#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ZIPcode.h"
#include <iostream>
#include "zlib.h"
#include <assert.h>
#include <string>
using namespace std;



#define CHUNK 16384


extern "C" int ZIPencode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
	BCInt szSrc;
	unsigned char *pChar;
    string input;
    string output;

	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}

	//zero the object that will hold the zipped string
	output.clear();
	
	szSrc = WMGetHandleSize(p->src);
    input.resize(szSrc); // make sure we have enough space!
    
	//get a pointer to the start of the data you want to zip.
	pChar = (unsigned char*)*(p->src);
    memcpy(&input[0], pChar, szSrc);

    if(err = encode_zip(input, output))
		goto done;
	
	if(err = WMPtrToHand((Ptr)output.data(), &dest, output.size()))
		goto done;
	
done:	
	if(p->src)
		WMDisposeHandle(p->src);
	
	p->dest = NULL;	// Init to NULL
	if (err != 0) {
		if (dest != NULL)
			WMDisposeHandle(dest);
		return err;
	}
	p->dest = dest;

	return err;
}


extern "C" int ZIPdecode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
    string input;
	string output;
    
    output.clear();
	
	unsigned char *pChar;
	BCInt szSrc;
	
	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		p->dest = NULL;
		goto done;
	}
	szSrc = WMGetHandleSize(p->src);
    input.resize(szSrc); // make sure we have enough space!

    pChar = (unsigned char*)*(p->src);
    memcpy(&input[0], pChar, szSrc);

	if(err = decode_zip(input, output))
		goto done;

	if(err = WMPtrToHand((Ptr)output.data(), &dest, output.size()))
		goto done;
		
	
done:		
    WMDisposeHandle(p->src);

	p->dest = NULL;	// Init to NULL
	if (err != 0) {
		if (dest != NULL)
			WMDisposeHandle(dest);
		return err;
	}
	p->dest = dest;	
	return err;
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */

int encode_zip(string &input, string &output) {
    int ret;
    
    z_stream zs;
    
    /* allocate deflate state */
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    
    // default value of 15 for window size, add 16 to write a simple
    // gzip header and trailer around the compressed data instead of a zlib wrapper
    ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        return PROBLEM_ZIPPING;

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();           // set the z_stream's input

    char outbuffer[32768];

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (output.size() < zs.total_out) {
            // append the block to the output string
            output.append(outbuffer,
                        zs.total_out - output.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        return PROBLEM_ZIPPING;
    }
    return 0;
}


/* Decompress from source to dest until stream ends.
   decode_zip returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int decode_zip(string &input, string &output) {
    int ret;
    
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    // Add 32 to windowBits to enable zlib and gzip decoding with automatic header
    // detection
    if (inflateInit2(&zs, 32) != Z_OK)
        return PROBLEM_UNZIPPING;

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();

    char outbuffer[32768];

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (output.size() < zs.total_out) {
            output.append(outbuffer,
                             zs.total_out - output.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        return PROBLEM_UNZIPPING;
    }

    return 0;
}
