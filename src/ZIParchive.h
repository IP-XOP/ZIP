/*
 *  Connections.h
 *  iPeek
 *
 *  Created by andrew on 17/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStandardHeaders.h"
#include "unzip.h"
#include <map>

#ifdef _WINDOWS_
double roundf(double val);
#endif

using namespace std;
/*
 ZIParchive, a C++ class to wrap all UNZIP operations that we're interested in
 */

class ZIParchive {
public:

	ZIParchive();
	
	int ZIParchiveOpen(const char* theFile);
	
	~ZIParchive();
	
	int listfiles(MemoryStruct &buf);
	
	int listCurrentFileInfo(MemoryStruct &buf);
	
	int seek(unsigned long pos);
	
	unsigned long tell();
	
	int isEOF();
	
	int openAFile(const char* fileinArchive);
	
	void closeAFile();
	
	long read(unsigned char *buf, unsigned long buflen);

	//the zip file that's been opened
	unzFile openFile;

private:
};



#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct ZIPa_openArchiveStruct {
	Handle zipFileToBeOpened;
	double result;			// not actually used.
}ZIPa_openArchiveStruct, *ZIPa_openArchiveStructPtr;

typedef struct ZIPa_closeArchiveStruct {
	double zipFileToBeClosed;
	double result;			// not actually used.
}ZIPa_closeArchiveStruct, *ZIPa_closeArchiveStructPtr;

typedef struct ZIPa_lsStruct {
	double zipFileToList;
	Handle result;			
}ZIPa_lsStruct, *ZIPa_lsStructPtr;

typedef struct ZIPa_openStruct {
	Handle whichFile;
	double zipFileToDeal;
	double result;
}ZIPa_openStruct, *ZIPa_openStructPtr;

typedef struct ZIPa_closeStruct {
	double zipFileToDeal;
	double result;
}ZIPa_closeStruct, *ZIPa_closeStructPtr;

typedef struct ZIPa_infoStruct {
	double zipFileToDeal;
	Handle result;
}ZIPa_infoStruct, *ZIPa_infoStructPtr;

struct ZIPa_readRuntimeParams {
	// Flag parameters.
	
	// Main parameters.
	
	// Parameters for simple main group #0.
	int zipFileToDealEncountered;
	double zipFileToDeal;
	int zipFileToDealParamsSet[1];
	
	// Parameters for simple main group #1.
	int outputEncountered;
	char output[MAX_OBJ_NAME+1];
	int outputParamsSet[1];
		
	// Parameters for simple main group #2.
	int numbytesEncountered;
	double numbytes;
	int numbytesParamsSet[1];
	
	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct ZIPa_readRuntimeParams ZIPa_readRuntimeParams;
typedef struct ZIPa_readRuntimeParams* ZIPa_readRuntimeParamsPtr;

int ExecuteZIPa_read(ZIPa_readRuntimeParamsPtr p);
int RegisterZIPa_read(void);
int ZIPa_openArchive(ZIPa_openArchiveStructPtr);
int ZIPa_closeArchive(ZIPa_closeArchiveStructPtr);
int ZIPa_ls(ZIPa_lsStructPtr);
int ZIPa_open(ZIPa_openStructPtr);
int ZIPa_close(ZIPa_closeStructPtr);
int ZIPa_info(ZIPa_infoStructPtr);
#pragma pack()