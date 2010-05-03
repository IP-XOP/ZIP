/*
 *  tar_extract.h
 *  ZIP
 *
 *  Created by Andrew Nelson on 11/03/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct tar_extractRuntimeParams {
	// Flag parameters.
	
	// Parameters for /O flag group.
	int OFlagEncountered;
	// There are no fields for this group because it has no parameters.
	
	// Main parameters.
	
	// Parameters for simple main group #0.
	int pathEncountered;
	Handle path;
	int pathParamsSet[1];
	
	// Parameters for simple main group #1.
	int tarFileEncountered;
	Handle tarFile;
	int tarFileParamsSet[1];
		
	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct tar_extractRuntimeParams tar_extractRuntimeParams;
typedef struct tar_extractRuntimeParams* tar_extractRuntimeParamsPtr;
#pragma pack()		// Reset structure alignment to default.

int Registertar_extract(void);
int Executetar_extract(tar_extractRuntimeParamsPtr p);