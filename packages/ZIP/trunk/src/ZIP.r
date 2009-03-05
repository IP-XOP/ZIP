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
	"Symbolic path doesn't point to folder.",
	//[5]
	"Cannot open ZIP file",
	//[6]
	"Cannot change directory",
	//[7]
	"Problem while extracting",
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
	}
};

resource 'XOPC' (1100) {
	{
		"ZIPfile",								// Name of operation.
		XOPOp+UtilOP+compilableOp,			// Operation's category.
	}
};
