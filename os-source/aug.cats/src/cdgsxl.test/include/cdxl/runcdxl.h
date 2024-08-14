/***********

 RunCDXL.h

 W.D.L 930330

************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

#ifndef	RUNCDXL_H	// [
#define	RUNCDXL_H

#define	XLTAG_Dummy		(TAG_USER + 32)

#define	XLTAG_XLFile		(XLTAG_Dummy+1)
// ti_Data points to CDXL filename.

#define	XLTAG_View		(XLTAG_Dummy+2)
// Open a View as opposed to a screen, ti_Data is a BOOL, default FALSE.

#define	XLTAG_Blit		(XLTAG_Dummy+3)
/* If ti_Data is TRUE, load CDXL image into a buffer, then blit to
 * display bitmap. Otherwize, just load CDXL image into display bitmap.
 * default FALSE.
 */

#define	XLTAG_Background	(XLTAG_Dummy+4)
/* ti_Data points to ILBM filename to display in background.
 * If specified, XLTAG_Blit is set.
 */

#define	XLTAG_Left		(XLTAG_Dummy+5)
// Left edge of CDXL image. If not specified, will center.

#define	XLTAG_Top		(XLTAG_Dummy+6)
// Top edge of CDXL image. If not specified, will center. (Valid only for XLTAG_View).

#define	XLTAG_Volume		(XLTAG_Dummy+7)
// Volume of CDXL audio, default 64.

#define	XLTAG_MultiPalette	(XLTAG_Dummy+8)
// If ti_Data is TRUE, will load new palette for each frame, defualt FALSE.

#define	XLTAG_KillSig		(XLTAG_Dummy+9)
// ti_Data is a ULONG signal which will abort CDXL playback.

#define	XLTAG_XLSpeed		(XLTAG_Dummy+10)
// ti_Data is XLSpeed playback speed. Currently supports 75 and 150.

#define	XLTAG_XLEEC		(XLTAG_Dummy+11)
// If ti_Data is TRUE, turn on error correction, default FALSE. (BOOL).

#define	XLTAG_XLPalette		(XLTAG_Dummy+12)
/* If ti_Data is TRUE, use palette stored in CDXL. Useful if 
 * XLTAG_Background is specified, default FALSE. (BOOL).
 */

#define	XLTAG_LACE		(XLTAG_Dummy+13)
// Open interlaced display. Will override ILBM or CDXL setting. (BOOL).

#define	XLTAG_NONLACE		(XLTAG_Dummy+14)
// Open noninterlaced display. Will override ILBM or CDXL setting. (BOOL).

#define	XLTAG_HIRES		(XLTAG_Dummy+15)
// Open HIRES display. Will override ILBM or CDXL setting. (BOOL).

#define	XLTAG_LORES		(XLTAG_Dummy+16)
// Open LORES display. Will override ILBM or CDXL setting. (BOOL).

#define	XLTAG_Boxit		(XLTAG_Dummy+17)
/* Draw a box with color 0 around CDXL image. May be needed to avoid
 * HAM problems. (BOOL).
 */

#define	XLTAG_CDTV		(XLTAG_Dummy+18)
// Open cdtv.device as opposed to cd.device


// return codes

#define RC_OK			0	/* No error */
#define RC_MISSING_FILE		1	/* Required filename missing */
#define RC_READ_ERROR		2	/* Error while reading file */
#define RC_CANT_FIND		3	/* Can't find file */
#define RC_NO_MEM		4	/* Not enough memory for operation */
#define RC_NO_CDDEVICE		5	/* Could not open cd/cdtv device */
#define RC_NO_AUDIODEVICE	6	/* Could not open audio device */
#define RC_NO_WIN		7	/* Could not open window */
#define RC_NO_SC		8	/* Could not open screen */
#define RC_BAD_PAN		9	/* File is not a standard PAN file */
#define RC_FAILED		10	/* Something failed */

int RunCDXL( ULONG tag, ... );

#endif				// ]
