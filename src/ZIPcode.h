#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

int decode_zip(string &input, string &output);
int encode_zip(string &input, string &output);

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

extern "C" int ZIPencode (ZIPencoderStructPtr p);
extern "C" int ZIPdecode (ZIPencoderStructPtr p);
