#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memutils.h"

int decode_zip(MemoryStruct &dest, const unsigned char *src, long szSrc);
int encode_zip(MemoryStruct &dest, const unsigned char *src, long szSrc);

/*
in encode/decode.cpp
*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct ZIPencoderStruct {
	Handle src;
	Handle dest;			//the string containing the content
}ZIPencoderStruct, *ZIPencoderStructPtr;
#include "XOPStructureAlignmentReset.h"

int ZIPencode (ZIPencoderStructPtr p);
int ZIPdecode (ZIPencoderStructPtr p);
