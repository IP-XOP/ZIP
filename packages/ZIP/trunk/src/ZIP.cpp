#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ZIP.h"
#include <iostream>
#include "zlib.h"
#include <assert.h>
using namespace std;

#define CHUNK 16384


/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */

int encode_zip(MemoryStruct &dest, const unsigned char* src, long szSrc) {
   int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	
    if (ret != Z_OK)
        return PROBLEM_UNZIPPING;
		
	//size of zip source
	size_t szSrcRead = 0;
	dest.reset();

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
			
			try{
				dest.WriteMemoryCallback(out, sizeof(char), have);
			} catch (bad_alloc&){
				return NOMEM;
			}
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
int decode_zip(MemoryStruct &dest, const unsigned char *src, long szSrc) {
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
	size_t szSrcRead = 0;
	dest.reset();
	
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
			try{
				dest.WriteMemoryCallback(out, sizeof(char), have);
			} catch (bad_alloc&){
				return NOMEM;
			}
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
//    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
	return ret == Z_STREAM_END ? 0 : PROBLEM_UNZIPPING;
return 0;
}




