#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

int decode_zip(string &dest, const unsigned char *src, BCInt szSrc);
int encode_zip(string &dest, const unsigned char *src, BCInt szSrc, unsigned long *crc);

/*
in encode/decode.cpp
*/
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct ZIPencoderStruct {
	Handle src;
	UserFunctionThreadInfoPtr tp;
	Handle dest;			//the string containing the content
}ZIPencoderStruct, *ZIPencoderStructPtr;
#pragma pack()

int ZIPencode (ZIPencoderStructPtr p);
int ZIPdecode (ZIPencoderStructPtr p);
