
// Operation template: tar_append string:tarFile, string:theFile
// Runtime param structure for tar_append operation.
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
struct tar_appendRuntimeParams {
	// Flag parameters.
	
	// Parameters for /O flag group.
	int OFlagEncountered;
	// There are no fields for this group because it has no parameters.
	
	// Main parameters.
	
	// Parameters for simple main group #0.
	int tarFileEncountered;
	Handle tarFile;
	int tarFileParamsSet[1];
	
	// Parameters for simple main group #1.
	int theFileEncountered;
	Handle theFile[100];						// Optional parameter.
	int theFileParamsSet[100];
		
	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct tar_appendRuntimeParams tar_appendRuntimeParams;
typedef struct tar_appendRuntimeParams* tar_appendRuntimeParamsPtr;
#pragma pack()		// Reset structure alignment to default.


int Registertar_append(void);
int Executetar_append(tar_appendRuntimeParamsPtr p);