/*
 *  tar_append.cpp
 *  igortar
 *
 *  Created by Andrew Nelson on 10/03/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"
#include "tar_append.h"
#include "error.h"
#include "libtar.h"

int Registertar_append(void) {
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the tar_appendRuntimeParams structure as well.
	cmdTemplate = "tar_append/O string:tarFile, string[100]:theFile";
	runtimeNumVarList = "";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(tar_appendRuntimeParams), (void*)Executetar_append, 0);
}

int Executetar_append(tar_appendRuntimeParamsPtr p){
	int err = 0;
	
	TAR *t = NULL;
	FILE *theTarFile = NULL;
	
	char HFSpathToTarFile[MAX_PATH_LEN + 1];
	char tarFileName[MAX_FILENAME_LEN + 1];
	char tarDirName[MAX_PATH_LEN + 1];
	char posixPathToTarFile[MAX_PATH_LEN + 1];
	bool fileAlreadyExists = false;
	bool isFolder = false;
	bool isFile = false;
	char HFSpathToAppendFileOrFolder[MAX_PATH_LEN + 1];
	char posixPathToAppendFileOrFolder[MAX_PATH_LEN + 1];
	char appendFileName[MAX_FILENAME_LEN + 1];
	char appendDirName[MAX_PATH_LEN + 1];
	int numfiles, ii;
	// Main parameters.
	
	if (p->tarFileEncountered) {
		// Parameter: p->tarFile (test for NULL handle before using)
		if(!p->tarFile){
			err = NULL_STRING_HANDLE;
			goto done;
		}
	}
	
	if (p->theFileEncountered) {
		// Parameter: p->theFile (test for NULL handle before using)
		int* paramsSet = &p->theFileParamsSet[0];
		Handle strH;
		
		for(ii = 0; ii < 100 ; ii++) {
			if (paramsSet[ii] == 0)
				break;		// No more parameters.
			strH = p->theFile[ii];
			if (strH == NULL) {
				err = USING_NULL_STRVAR;
				goto done;
			}
			numfiles += 1;
		}
	}
	
	memset(HFSpathToTarFile, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	if(err = GetCStringFromHandle(p->tarFile, HFSpathToTarFile, MAX_PATH_LEN))
		goto done;
	
	//now need to check if the string to the file is really a preexisting file.
	if(FullPathPointsToFile(HFSpathToTarFile)){
		//the path is a file, hopefully a tar file.  When the tar library is called it should barf if it's not.
		fileAlreadyExists = true;
	} else {
		//check if the directory part of the HFS path points to a directory!
		//obtain the directory part and the file part.
		if(err = GetDirectoryAndFileNameFromFullPath(HFSpathToTarFile, tarDirName, tarFileName))
			goto done;
		
		//now got the directory, and filename.  Check if the directory really is a directory
		if(!FullPathPointsToFolder(tarDirName)){
			err = SYMBOLIC_PATH_ISNT_FOLDER;
			goto done;
		}
		//got to make sure that you have a valid file name
		if(strlen(tarFileName) < 0){
			err = CANT_OPEN_TARFILE;
			goto done;
		}
		fileAlreadyExists = false;
		//now got to make a file of that name.
		
	}
	//convert the HFS tar path to POSIX tar path
	if(err = HFSToPosixPath(HFSpathToTarFile, posixPathToTarFile, 0))
		goto done;
	
	//now create a tar file
	if(!fileAlreadyExists || p->OFlagEncountered){
		theTarFile = fopen(posixPathToTarFile, "w");
		if(!theTarFile){
			err = CANT_OPEN_TARFILE;
			goto done;
		}
		fclose(theTarFile);	
	}
	
	//open the tar file.
	if(tar_open(&t, posixPathToTarFile, NULL, O_WRONLY, 0644, TAR_GNU)){
		err = errno;
		errno = 0;
		goto done;
	}
	
	
	////////////////////////////////////////////////////////////
	//now got to see if the directory/file to be appended exists.
	////////////////////////////////////////////////////////////
	
	for (ii = 0 ; ii < numfiles && err == 0 ; ii++){
		if(err = GetCStringFromHandle(p->theFile[ii], HFSpathToAppendFileOrFolder, MAX_PATH_LEN))
			goto done;
		
		isFolder = FullPathPointsToFolder(HFSpathToAppendFileOrFolder);
		isFile = FullPathPointsToFile(HFSpathToAppendFileOrFolder);
		
		if(! (isFolder || isFile)){
			err = NOT_A_FILE_OR_FOLDER;
			goto done;
		}
		
		//if you are appending file(s), then need to append a filename
		if(isFile && (err = GetDirectoryAndFileNameFromFullPath(HFSpathToAppendFileOrFolder, appendDirName, appendFileName)))
			goto done;
		
		//convert the HFS path to a POSIX PATH
		if(err = HFSToPosixPath(HFSpathToAppendFileOrFolder, posixPathToAppendFileOrFolder, isFolder))
			goto done;
		
		
		//now lets append the tree, if it is one
		if(isFolder && tar_append_tree(t, posixPathToAppendFileOrFolder, ".")){
			err = PROBLEM_APPENDING_TO_TAR;
			goto done;
		}
		//append a file if it is one
		if(isFile && tar_append_file(t, posixPathToAppendFileOrFolder, appendFileName)){
			err = PROBLEM_APPENDING_TO_TAR;
			goto done;
		}
	}
done:
	if(t)
		tar_close(t);
	
	return err;
}