/*
 ZIP - an XOP designed to zip and unzip strings and files.
 
 ZIP relies on the zlib library and Minizip.
 
 ZLIB was written by Jean-loup Gailly and Mark Adler
 http://www.zlib.net/
 Minizip was written by Gilles Vollant.
 http://www.winimage.com/zLibDll/minizip.html
 ZIP could not have been written without this free sourcecode.
 
 
 ZIPencode and ZIPdecode zip and unzip strings in the gz format.  This enables you to zip up files into the gz format, by reading the file into memory
 ZIPfile is an operation designed to unzip zip files that contain multiple files
 
 */
 
#include "ZIPcode.h"
#include "ZIPfile.h"
#include "zlib.h"

#ifdef _MACINTOSH_
#include "tar_append.h"
#include "tar_extract.h"
#endif

#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle);
#endif	
#ifdef _WINDOWS_
HOST_IMPORT void main(IORecHandle ioRecHandle);
#endif

static long
RegisterFunction()
{
	int funcIndex;
	
	funcIndex = GetXOPItem(0);			// Which function invoked ?
	switch (funcIndex) {
		case 0:							
			return((long)ZIPencode);	// This function is called using the direct method.
			break;
		case 1:
			return((long)ZIPdecode);
			break;
	}
	return NIL;
}

static long
RegisterOperations(void)		// Register any operations with Igor.
{
	int result;

	
	if (result = RegisterZIPfile())
		return result;
	if (result = RegisterZIPzipfiles())
		return result;
#ifdef _MACINTOSH_
	if (result = Registertar_append())
		return result;
	if (result = Registertar_extract())
		return result;
#endif
	// There are no more operations added by this XOP.
	
	return 0;
}


/*	XOPEntry()
 
 This is the entry point from the host application to the XOP for all
 messages after the INIT message.
 */
static void
XOPEntry(void)
{	
	long result = 0;
	
	switch (GetXOPMessage()) {
		case FUNCADDRS:
			result = RegisterFunction();	// This tells Igor the address of our function.
			break;
	}
	SetXOPResult(result);
}

/*	main(ioRecHandle)
 
 This is the initial entry point at which the host application calls XOP.
 The message sent by the host must be INIT.
 main() does any necessary initialization and then sets the XOPEntry field of the
 ioRecHandle to the address to be called for future messages.
 */
#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle)
#endif	
#ifdef _WINDOWS_
HOST_IMPORT void main(IORecHandle ioRecHandle)
#endif
{	
	int result;
	XOPInit(ioRecHandle);							// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);							// Set entry point for future calls.
	
	if (result = RegisterOperations()) {
		SetXOPResult(result);
#ifdef _MACINTOSH_
		return 0;
#endif
	}
	
	if (igorVersion < 504)
		SetXOPResult(REQUIRES_IGOR_504);
	else
		SetXOPResult(0L);
	
#ifdef _MACINTOSH_
	return 0;
#endif
}

