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
#include "ZIParchive.h"

HOST_IMPORT int XOPMain(IORecHandle ioRecHandle);


static XOPIORecResult
RegisterFunction()
{
	int funcIndex;
	
	funcIndex = (int) GetXOPItem(0);			// Which function invoked ?
	switch (funcIndex) {
		case 0:							
			return((XOPIORecResult)ZIPencode);	// This function is called using the direct method.
			break;
		case 1:
			return((XOPIORecResult)ZIPdecode);
			break;
		case 2:
			return((XOPIORecResult)ZIPa_openArchive);
			break;
		case 3:
			return((XOPIORecResult)ZIPa_closeArchive);
			break;
		case 4:
			return ((XOPIORecResult)ZIPa_ls);
			break;
		case 5:
			return ((XOPIORecResult)ZIPa_open);
			break;
		case 6:
			return ((XOPIORecResult)ZIPa_close);
			break;
		case 7:
			return ((XOPIORecResult)ZIPa_info);
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
	if (result = RegisterZIPa_read())
		return result;
	// There are no more operations added by this XOP.
	
	return 0;
}


/*	XOPEntry()
 
 This is the entry point from the host application to the XOP for all
 messages after the INIT message.
 */
extern "C" void
XOPEntry(void)
{	
	XOPIORecResult result = 0;
	ZIPa_closeArchiveStruct p;
	
	switch (GetXOPMessage()) {
		case FUNCADDRS:
			result = RegisterFunction();	// This tells Igor the address of our function.
			break;
		case NEW:
			p.zipFileToBeClosed = -1;
			ZIPa_closeArchive(&p);
			
			break;
		case CLEANUP:
			p.zipFileToBeClosed = -1;
			ZIPa_closeArchive(&p);

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

HOST_IMPORT int XOPMain(IORecHandle ioRecHandle)
{	
	long result;
	XOPInit(ioRecHandle);							// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);							// Set entry point for future calls.
		
	if (result = RegisterOperations()) {
		SetXOPResult(result);
		return EXIT_FAILURE;
	}
	
	if (igorVersion < 900){
		SetXOPResult(IGOR_OBSOLETE);
		return EXIT_FAILURE;
	} else
		SetXOPResult(0L);
	
	return EXIT_SUCCESS;
}

