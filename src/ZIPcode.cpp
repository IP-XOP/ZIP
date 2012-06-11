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


int ZIPencode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
	string destMem;
	BCInt szSrc;
	unsigned char *pChar;
	unsigned char gzipHeader[10];
	unsigned long crc32;
	
	dest = NewHandle(0);
	if(dest == NULL)
		return NOMEM;
	
	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}

	//zero the object that will hold the zipped string
	destMem.clear();
	
	memset(gzipHeader, 0, 10);
	gzipHeader[0] = 0x1f;
	gzipHeader[1] = 0x8b;
	gzipHeader[2] = 8;
	gzipHeader[9] = 255;
	//write the GZIP header
	destMem.append((const char*) gzipHeader, sizeof(char) * 10);
	
	szSrc = GetHandleSize(p->src);
	
	//get a pointer to the start of the data you want to zip.
	pChar = (unsigned char*)*(p->src);
	
	if(err = encode_zip(destMem, pChar, szSrc, &crc32))
		goto done;
	
	//remove the header and tail of the zlib format, in order to add in the gzip headers.
	destMem.erase(10, sizeof(unsigned char)* 2);
	destMem.erase((destMem.size()/sizeof(unsigned char)) - 4, sizeof(unsigned char) * 4);

	destMem.append((const char*) &crc32, sizeof(unsigned long) * 1);
	szSrc = szSrc % (2^32);
	destMem.append((const char*) &szSrc, sizeof(unsigned long) * 1);
	
	if(err = PtrToHand((Ptr)destMem.data(), &dest, destMem.size()))
		goto done;
	
done:	
	if(p->src)
		DisposeHandle(p->src);
	
	p->dest = NULL;	// Init to NULL
	if (err != 0) {
		if (dest != NULL)
			DisposeHandle(dest);
		return err;
	}
	p->dest = dest;

	return err;
}




int ZIPdecode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
	string destMem;
	
	unsigned char *pChar;
	BCInt szSrc;
	
	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		p->dest = NULL;
		goto done;
	}
	szSrc = GetHandleSize(p->src);
	dest = NewHandle(0);
	if(dest == NULL)
		return NOMEM;
		
	//copy over the data by locking and unlocking the handle
	//wasteful of memory
	pChar = (unsigned char*)*(p->src);	

	if(err = decode_zip(destMem, pChar, szSrc))
		goto done;

	if(err = PtrToHand((Ptr)destMem.data(), &dest, destMem.size()))
		goto done;
		
	
done:		
	if(p->src)
		DisposeHandle(p->src);

	p->dest = NULL;	// Init to NULL
	if (err != 0) {
		if (dest != NULL)
			DisposeHandle(dest);
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

int encode_zip(string &dest, const unsigned char* src, BCInt szSrc, unsigned long *crcRet) {
   int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
	unsigned long crc = crc32(0L, Z_NULL, 0);
	*crcRet = 0;

	 
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION,Z_DEFLATED,15,8,Z_DEFAULT_STRATEGY);
	
    if (ret != Z_OK)
        return PROBLEM_UNZIPPING;
		
	//size of zip source
	BCInt szSrcRead = 0;

    /* compress until end of file */
    do {
		if((szSrc - szSrcRead) > CHUNK){
			memcpy(in, src + szSrcRead, sizeof(char) * CHUNK);
			strm.avail_in = CHUNK;
			szSrcRead += strm.avail_in;
		} else {
			memcpy(in, src + szSrcRead, sizeof(char)*(szSrc - szSrcRead));
			strm.avail_in = szSrc - szSrcRead;
			szSrcRead += strm.avail_in;
		}
		
		crc = crc32(crc, in, strm.avail_in);
		*crcRet = crc;
		
        flush = (szSrcRead == szSrc) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
			
			dest.append((const char*) out, sizeof(char) * have);

		} while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return 0;
}


/* Decompress from source to dest until stream ends.
   decode_zip returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int decode_zip(string &dest, const unsigned char *src, BCInt szSrc) {
   int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm,15+32);
    if (ret != Z_OK)
        return PROBLEM_UNZIPPING;

	//size of zip source
	BCInt szSrcRead = 0;
	
    /* decompress until deflate stream ends or end of file */
    do {
		if((szSrc - szSrcRead) > CHUNK){
			memcpy(in, src + szSrcRead, sizeof(char) * CHUNK);
			strm.avail_in = CHUNK;
			szSrcRead += strm.avail_in;
		} else {
			memcpy(in, src + szSrcRead, sizeof(char)*(szSrc - szSrcRead));
			strm.avail_in = szSrc - szSrcRead;
			szSrcRead += strm.avail_in;
		}
		
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return PROBLEM_UNZIPPING;
            }
            have = CHUNK - strm.avail_out;
			
			//write to the destination source
			dest.append((const char*) out, sizeof(char) * have);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
//    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
	return ret == Z_STREAM_END ? 0 : PROBLEM_UNZIPPING;
return 0;
}




