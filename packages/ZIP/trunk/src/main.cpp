/*
 base64 - an XOP designed to encode/decode base64 strings
 */
//string check = base64encode("weldon")

#include "ZIP.h"
#include "zlib.h"

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
	XOPInit(ioRecHandle);							// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);							// Set entry point for future calls.
	
	if (igorVersion < 504)
		SetXOPResult(REQUIRES_IGOR_504);
	else
		SetXOPResult(0L);
}

int ZIPencode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
	MemoryStruct destMem;
	long szSrc;
	unsigned char *pChar;
	int hState;
	
	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		p->dest = NULL;
		goto done;
	}
	szSrc = GetHandleSize(p->src);
	//copy over the data by locking and unlocking the handle
	//wasteful of memory
	hState = MoveLockHandle(p->src);
	
	pChar = (unsigned char*)*(p->src);

	if(err = encode_zip(destMem, pChar, szSrc)){		
		HSetState(p->src, hState ); 
		goto done;
	}
	
	dest = NewHandle(0);
	if(err = PtrToHand((Ptr)destMem.getData(), &dest, destMem.getMemSize()))
		goto done;
		
	p->dest = dest;
	
done:	
	if(err)
		if(dest)
			DisposeHandle(dest);	
	if(p->src)
		DisposeHandle(p->src);
	
	return err;
}




int ZIPdecode(ZIPencoderStruct *p){
	int err = 0;
	
	Handle dest = NULL;
	MemoryStruct destMem;
	
	unsigned char *pChar;
	long szSrc;
	int hState;
	
	if(p->src == NULL){
		err = NULL_STRING_HANDLE;
		p->dest = NULL;
		goto done;
	}
	szSrc = GetHandleSize(p->src);
	
	//copy over the data by locking and unlocking the handle
	//wasteful of memory
	hState = MoveLockHandle(p->src);
	
	pChar = (unsigned char*)*(p->src);	

	if(err = decode_zip(destMem, pChar, szSrc)){
		HSetState(p->src, hState ); 
		goto done;
	}
	
	dest = NewHandle(0);
	if(err = PtrToHand((Ptr)destMem.getData(), &dest, destMem.getMemSize()))
		goto done;
		
	p->dest = dest;
	
done:	
	if(err)
		if(dest)
			DisposeHandle(dest);	
	if(p->src)
		DisposeHandle(p->src);
	
	return err;
}
