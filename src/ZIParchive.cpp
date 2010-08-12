/*
 *  ZIParchive.cpp
 *  ZIP
 *
 *  Created by Andrew Nelson on 11/08/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
/*
 variable id
 id = ZIPa_openArchive("foobar:Users:anz:Desktop:Archive.zip")
 print ZIPa_ls(id)
 print ZIPa_open(id, "globalfit_test.pxp")
 
 string buf=""
 
 ZIPa_read id, buf, 10; print V_flag; print buf
 
 ZIPa_closeArchive(id)
 */
#include "XOPStandardHeaders.h"
#include "memutils.h"
#include "ZIParchive.h"
#include "error.h"

#include <map>
#include <algorithm>
#include <vector>
#include <string.h>
#include <ctime>

using namespace std;
std::map<long, ZIParchive> openZIPfiles;

/*
ZIParchive, a C++ class to wrap all UNZIP operations that we're interested in
 */

ZIParchive::ZIParchive(){
	openFile = NULL;
}

int ZIParchive::ZIParchiveOpen(const char* theFile){
	int err = 0;
	openFile = unzOpen(theFile);
	if(!openFile)
		err = 1;
	return err;
}
	
ZIParchive::~ZIParchive(){
	int err;
	if(openFile){
		err = unzCloseCurrentFile(openFile);
		err = unzClose(openFile);	
	}
};

void ZIParchive::closeAFile(){
	if(openFile)
		unzCloseCurrentFile(openFile);
}

int ZIParchive::openAFile(const char* fileinArchive){
	int err = 0;
	if(err = unzLocateFile(openFile, fileinArchive, 0))
	   return err;
	if(err = unzOpenCurrentFile(openFile))
		return err;
	return err;
}

long ZIParchive::read(unsigned char* buf, unsigned long buflen){
	long err = -1;
	memset(buf, 0, sizeof(char) * buflen);
	
	if(openFile)
		err = unzReadCurrentFile(openFile, buf, buflen);
	
	return err;
}

int ZIParchive::listCurrentFileInfo(MemoryStruct &buf){
	return 0;
}

int ZIParchive::listfiles(MemoryStruct &buf){
	int err = 0;
	int status = UNZ_OK;
	char filename_inzip[MAX_PATH_LEN + 1];
	unz_file_info file_info;
	
	buf.reset();
	closeAFile();
	
	if(err = unzGoToFirstFile(openFile))
		return err;
	
	for( ; status == UNZ_OK ; ){
		if(err = unzGetCurrentFileInfo(openFile, &file_info, filename_inzip, sizeof(char) * MAX_PATH_LEN, NULL, 0, NULL, 0))
			return err;
		buf.append(filename_inzip, sizeof(char), strlen(filename_inzip));
		if(!buf.getData())
			return 1;
		buf.append(":", sizeof(char), 1);
		if(!buf.getData())
			return 1;
		
		status = unzGoToNextFile(openFile);
	}
	
	return 0;
}

int ZIParchive::seek(unsigned long pos){
	int err = 0;
	err = unzSetOffset(openFile, pos);
	
	return err;
}

unsigned long ZIParchive::tell(){
	return unzGetOffset(openFile);
}

int ZIParchive::isEOF(){
	return unzeof(openFile);
}


/*
 All the XOP functions/operations we're interested in
 */
int ExecuteZIPa_read(ZIPa_readRuntimeParamsPtr p){
	int err = 0;
	int err2 = 0;
	long fileID = 0;
	unsigned char *buf = NULL;
	unsigned long buflen = 0;
	long bytesRead = -1;
	extern map<long, ZIParchive> openZIPfiles;
	
	// Main parameters.
	if (p->zipFileToDealEncountered) 
		fileID = (long)roundf(p->zipFileToDeal);
	
	if (p->outputEncountered) {
		if (p->outputParamsSet[0]) {
			int dataTypePtr;
			if(err = VarNameToDataType(p->output, &dataTypePtr))
				goto done;
			if(dataTypePtr){
				err = OH_EXPECTED_STRING;
				goto done;
			}
		}
	}
	
	if (p->numbytesEncountered)
		buflen = (long)roundf(p->numbytes);
		 		   
	if((openZIPfiles.find(fileID) == openZIPfiles.end())){
		err2 = NO_ZIP_BY_THAT_HANDLE;
		goto done;
	}
	
	buf = new (nothrow) unsigned char [buflen];
	if(!buf){
		err = NOMEM;
		goto done;
	}
	
	bytesRead = openZIPfiles[fileID].read(buf, buflen);
	if(bytesRead < 0)
		goto done;
	
	if(err = StoreStringDataUsingVarName(p->output, (const char*) buf, bytesRead))
		goto done;   
		   
done:
	if(buf)
		delete[] buf;
	if(err || bytesRead < 0 || err2)
		StoreStringDataUsingVarName(p->output, "", 0);
	
	SetOperationNumVar("V_flag", (double) bytesRead);
	
	if(err || err2) 
		SetOperationNumVar("V_flag", -1);
	return err;
}

int
RegisterZIPa_read(void)
{
	const char* cmdTemplate;
	const char* runtimeNumVarList;
	const char* runtimeStrVarList;
	
	// NOTE: If you change this template, you must change the ZIPa_readRuntimeParams structure as well.
	cmdTemplate = "ZIPa_read number:zipFileToDeal, varname:output, number:numbytes";
	runtimeNumVarList = "V_Flag";
	runtimeStrVarList = "";
	return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(ZIPa_readRuntimeParams), (void*)ExecuteZIPa_read, 0);
}


/* unz_s contain internal information about the zipfile
I want to store references to the open unzFile in a map, need a file number for that.
 this can be created using fileno(FILE), but that is the second element in the OPAQUE unzFile structure.
 The fact that is opaque probably means I'm not meant to find this out.
 THe following structure is the first two elements of the unz_s structure (unzip.c).
 */
typedef struct unzFILEREF{
		zlib_filefunc_def z_filefunc;
		voidpf filestream;        /* io structore of the zipfile */
}unzFILEREF;

int ZIPa_openArchive(ZIPa_openArchiveStructPtr p){
	int err = 0;
	int err2 = 0;
	extern map<long, ZIParchive> openZIPfiles;
	char zipfilename[MAX_PATH_LEN+1];
	char temppath[MAX_PATH_LEN + 1];
	long fileRef;
	ZIParchive theZipFile;
	unzFILEREF *s;
	
	p->result = 0;
	memset(zipfilename, 0, MAX_PATH_LEN+1);
	
	if (!p->zipFileToBeOpened){
		err = NULL_STRING_HANDLE;
		goto done;
	}
		
	if(err = GetCStringFromHandle(p->zipFileToBeOpened, temppath, MAX_PATH_LEN))
		goto done;
	if(err = GetNativePath(temppath, zipfilename))
			goto done;
#ifdef _MACINTOSH_
	HFSToPosixPath(zipfilename, zipfilename, 0);
#endif
	
	if(theZipFile.ZIParchiveOpen(zipfilename)){
		err2 = CANNOT_OPEN_ZIPFILE;
		goto done;
	}
	if(unzGoToFirstFile(theZipFile.openFile)){
		err2 = PROBLEM_UNZIPPING;
		goto done;
	}
	
	s = (unzFILEREF*)theZipFile.openFile;
	
	fileRef = fileno((FILE*)s->filestream);
	openZIPfiles[fileRef] = theZipFile;
	theZipFile.openFile = NULL;
	
	p->result = fileRef;
done:
	if(err2 || err)
		p->result = -1;

	if(p->zipFileToBeOpened)
		DisposeHandle(p->zipFileToBeOpened);
	
	return err;
};

int ZIPa_closeArchive(ZIPa_closeArchiveStructPtr p){
	int err = 0;
	int err2 = 0;
	long fileID = 0;
	extern map<long, ZIParchive> openZIPfiles;
	fileID = (long)roundf(p->zipFileToBeClosed);
	
	if(fileID == -1){
		openZIPfiles.clear();
		return err;
	}
	if((openZIPfiles.find(fileID) != openZIPfiles.end()))
		openZIPfiles.erase(fileID);
	else
		err2 = NO_ZIP_BY_THAT_HANDLE;

		
	return err;
};

int ZIPa_ls(ZIPa_lsStructPtr p){
	int err = 0;
	int err2 = 0;
	long fileID;
	MemoryStruct buf;
	
	Handle theFileNames = NULL;
	
	extern map<long, ZIParchive> openZIPfiles;
	fileID = (long)roundf(p->zipFileToList);
			
	p->result = NULL;
	
	if((openZIPfiles.find(fileID) == openZIPfiles.end())){
		err2 = NO_ZIP_BY_THAT_HANDLE;
		goto done;
	}
	
	if(err2 = openZIPfiles[fileID].listfiles(buf)){
		err = PROBLEM_LISTING_FILES;
		goto done;
	}
	
	if(err = PtrToHand((Ptr)buf.getData(), &theFileNames, buf.getMemSize()))
	   goto done;
	   
done:
	if((err2 || err) && theFileNames)
		DisposeHandle(theFileNames);
	else
		p->result = theFileNames;
	
	return err;
};

int ZIPa_open(ZIPa_openStructPtr p){
	int err = 0;
	int err2 = 0;
	char filename_inZIP[MAX_PATH_LEN + 1];
	long fileID;
	
	extern map<long, ZIParchive> openZIPfiles;
	fileID = (long)roundf(p->zipFileToDeal);
	
	if(!p->whichFile){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	if(err = GetCStringFromHandle(p->whichFile, filename_inZIP, MAX_PATH_LEN))
	   goto done;
	
	if((openZIPfiles.find(fileID) == openZIPfiles.end())){
		err2 = NO_ZIP_BY_THAT_HANDLE;
		goto done;
	}
	
	openZIPfiles[fileID].closeAFile();
	
	if(openZIPfiles[fileID].openAFile(filename_inZIP)){
		err2 = CANT_FIND_IN_ZIP;
		goto done;
	}
	
done:
	if(p->whichFile)
		DisposeHandle(p->whichFile);
	p->result = err2;
	return err;
};

int ZIPa_close(ZIPa_closeStructPtr p){	
	int err = 0;
	long fileID;
	
	extern map<long, ZIParchive> openZIPfiles;
	fileID = (long)roundf(p->zipFileToDeal);
		
	if((openZIPfiles.find(fileID) == openZIPfiles.end())){
		err = NO_ZIP_BY_THAT_HANDLE;
		goto done;
	}
	
	openZIPfiles[fileID].closeAFile();
	
done:
	return err;
};

int ZIPa_info(ZIPa_infoStructPtr){
	int err = 0;
	return err;
};

#ifdef _WINDOWS_
double roundf(double val){
	double retval;
	if(val>0){
		if(val-floor(val) < 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	} else {
		if(val-floor(val) <= 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	}
	return retval;
}
#endif


