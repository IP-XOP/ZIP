/*
 *  tar_extract.cpp
 *  ZIP
 *
 *  Created by Andrew Nelson on 11/03/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "tar_extract.h"
#include "error.h"
#include <libtar.h>


#include "XOPStandardHeaders.h"

int Registertar_extract(void) {
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the tar_extractRuntimeParams structure as well.
	cmdTemplate = "tar_extract/O string:path, string:tarFile";
	runtimeNumVarList = "V_flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(tar_extractRuntimeParams), (void*)Executetar_extract, 0);
}

int Executetar_extract(tar_extractRuntimeParamsPtr p){
	int err = 0;
 
	char HFSoutputPath[MAX_PATH_LEN + 1]; // Output full path
	char outputPath[MAX_PATH_LEN + 1]; // Output full path
	
	char HFSpathToFile[MAX_PATH_LEN + 1];
	char posixPathToFile[MAX_PATH_LEN + 1]; 
	
	TAR *t = NULL;
	
	if (p->tarFileEncountered) {
		if(!p->tarFile){
			err = NULL_STRING_HANDLE;
			goto done;
		}
	}
	
	if(p->path){
		if(!p->path){
			err = NULL_STRING_HANDLE;
			goto done;
		}
	}
	
	memset(HFSpathToFile, 0, sizeof(char) * (MAX_PATH_LEN + 1));
	if(err = GetCStringFromHandle(p->tarFile, HFSpathToFile, MAX_PATH_LEN))
		goto done;
	
	//now need to check if the string to the file is really a file.
	if(!FullPathPointsToFile(HFSpathToFile)){
		err = CANT_OPEN_TARFILE;
		goto done;
	}
	//convert to POSIX
	if(err = HFSToPosixPath(HFSpathToFile, posixPathToFile, 0))
		goto done;

	//
	//get the place where we're supposed to write the tar contents
	if(err = GetCStringFromHandle(p->path, HFSoutputPath, MAX_PATH_LEN))
			goto done;
	
	//check if the output path is a directory
	if(!FullPathPointsToFolder(HFSoutputPath)){
		err = SYMBOLIC_PATH_ISNT_FOLDER;
		goto done;
	}
	//convert to POSIX
	if(err = HFSToPosixPath(HFSoutputPath, outputPath, 1))
		goto done;
	
	//open the tar file
	if(tar_open(&t, posixPathToFile, NULL, O_RDONLY, 0x777, TAR_GNU)){
		err = errno; //PROBLEM_EXTRACTING;
		errno = 0;
		goto done;
	}
	
	if(tar_extract_all(t, outputPath)){
		err = errno;//PROBLEM_EXTRACTING, return the error number
		goto done;
	}
	
done:
	if(t)
		tar_close(t);
	if(err)
		SetOperationNumVar("V_flag", 1);
	else
		SetOperationNumVar("V_flag", 0);
	
	return err;
}

