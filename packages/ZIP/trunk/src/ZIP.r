#include "XOPStandardHeaders.r"

resource 'vers' (1) {						/* XOP version info */
	0x01, 0x10, final, 0x00, 0,				/* version bytes and country integer */
	"1.10",
	"1.10, � 1996-2004 WaveMetrics, Inc., all rights reserved."
};

resource 'vers' (2) {						/* Igor version info */
	0x05, 0x04, release, 0x00, 0,			/* version bytes and country integer */
	"5.04",
	"(for Igor Pro 5.04 or later)"
};

resource 'STR#' (1100) {					/* custom error messages */
	{
	//[1]
	"ZIP requires Igor Pro 5.04 or later.",
	//[2]
	"One of the input strings is NULL.",
	//[3]
	"A problem was experienced while unzipping.",
	//[4]
	"Path doesn't point to folder.",
	//[5]
	"Cannot open ZIP file",
	//[6]
	"Cannot change directory",
	//[7]
	"Problem while extracting",
	//[8]
	"Problem while zipping",
	//[9]
	"Can't open the tarfile",
	//[10]
	"You need to supply a file or folder",
	//[11]
	"Problem appending to tar file",
	//[12]
	"Can't locate file in archive",
	//[13]
	"No ZIP archive by that file handle",
	//[14]
	"A problem was encountered whilst trying to list files",
	}
};

resource 'STR#' (1101) {					// Misc strings for XOP.
	{
		"-1",								// This item is no longer supported by the Carbon XOP Toolkit.
		"No Menu Item",						// This item is no longer supported by the Carbon XOP Toolkit.
		"ZIP Help",					// Name of XOP's help file.
	}
};

resource 'XOPI' (1100) {
	XOP_VERSION,							// XOP protocol version.
	DEV_SYS_CODE,							// Development system information.
	0,										// Obsolete - set to zero.
	0,										// Obsolete - set to zero.
	XOP_TOOLKIT_VERSION,					// XOP Toolkit version.
};

resource 'XOPF' (1100) {
	{
		"ZIPencode",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		HSTRING_TYPE,						// Return value type.
		{									
		HSTRING_TYPE,					
		},
		"ZIPdecode",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		HSTRING_TYPE,						// Return value type.
		{									
		HSTRING_TYPE,										
		},
		"ZIPa_openArchive",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{	
			HSTRING_TYPE,										
		},
		"ZIPa_closeArchive",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{									
			NT_FP64,										
		},
		"ZIPa_ls",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		HSTRING_TYPE,						// Return value type.
		{									
			NT_FP64,										
		},
		"ZIPa_open",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{									
			NT_FP64,
			HSTRING_TYPE,
		},
		"ZIPa_close",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{									
			NT_FP64,
		},
		"ZIPa_info",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		HSTRING_TYPE,						// Return value type.
		{									
			NT_FP64,
		},		
	}
};

resource 'XOPC' (1100) {
	{
		"ZIPfile",								// Name of operation.
		XOPOp+UtilOP+compilableOp,			// Operation's category.
		"ZIPzipfiles",
		XOPOp+UtilOP+compilableOp,			// Operation's category.
		"tar_append",
		XOPOp+UtilOP+compilableOp,			// Operation's category.
		"tar_extract",
		XOPOp+UtilOP+compilableOp,			// Operation's category.
		"ZIPa_read",
		XOPOp+UtilOP+compilableOp,			// Operation's category.
	}
};
