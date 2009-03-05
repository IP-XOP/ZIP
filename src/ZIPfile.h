/*
 *  ZIPfile.h
 *  ZIP
 *
 *  Created by andrew on 5/03/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

// Runtime param structure for ZIPfile operation.
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct ZIPfileRuntimeParams {
	
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
	int fileEncountered;
	Handle file;
	int fileParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct ZIPfileRuntimeParams ZIPfileRuntimeParams;
typedef struct ZIPfileRuntimeParams* ZIPfileRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int ExecuteZIPfile(ZIPfileRuntimeParamsPtr p);