/***********

    cdsgxl.c

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

// Tab size is 8!

#include <exec/types.h>
#include <dos/dos.h>
#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <string.h>	// for setmem()

#include "cdxl/runcdxl.h"
#include "cdgsxl_rev.h"

// argument parsing 
#define TEMPLATE    "FROM/A,X/K/N,Y/K/N,VOL/K/N,VIEW/S,BLIT/S,BACK/K,MULTIPAL/S,XLSPEED/K/N,XLEEC/S,XLPAL/S,LACE/S,NONLACE/S,HIRES/S,LORES/S,BOXIT/S,CDTV/S" VERSTAG " Wayne D. Lutz"
#define OPT_FROM	0
#define	OPT_X		1
#define	OPT_Y		2
#define	OPT_VOL		3
#define	OPT_VIEW	4
#define	OPT_BLIT	5
#define	OPT_BACK	6
#define	OPT_MULTIPAL	7
#define	OPT_XLSPEED	8
#define	OPT_XLEEC	9
#define	OPT_XLPAL	10
#define	OPT_LACE	11
#define	OPT_NONLACE	12
#define	OPT_HIRES	13
#define	OPT_LORES	14
#define	OPT_BOXIT	15
#define	OPT_CDTV	16
#define	OPT_COUNT	17

LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


int CXBRK(void) { return(0); }		/* Disable SASC CTRL/C handling */
int chkabort(void) { return(0); }	/* Indeed */

// Error messages.
STATIC UBYTE * XLError[] = {
    "No error",
    "Required filename missing",
    "Error while reading file",
    "Couldn't open file",
    "Not enough memory for operation",
    "Could not open cd/cdtv device",
    "Could not open audio device",
    "Could not open window",
    "Could not open screen",
    "Specified CDXL file is not a standard PAN file",
    "Operation failed"
};


VOID
main( LONG argc,char * argv[] )
{
    int    ret;
    LONG   xlspeed,left,top,vol;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }

    if ( opts[OPT_XLSPEED] )
	xlspeed = *(LONG *)opts[OPT_XLSPEED];

    if ( opts[OPT_X] )
	left = *(LONG *)opts[OPT_X];

    if ( opts[OPT_Y] )
	top = *(LONG *)opts[OPT_Y];

    if ( opts[OPT_VOL] )
	vol = *(LONG *)opts[OPT_VOL];

    ret = RunCDXL(
		XLTAG_XLFile,		opts[OPT_FROM],
		XLTAG_View,		opts[OPT_VIEW],
		XLTAG_Blit,		opts[OPT_BLIT],
		XLTAG_MultiPalette,	opts[OPT_MULTIPAL],
		XLTAG_Background,	opts[OPT_BACK],
		XLTAG_KillSig,		SIGBREAKF_CTRL_C,
		XLTAG_XLEEC,		opts[OPT_XLEEC],
		XLTAG_XLPalette,	opts[OPT_XLPAL],
		XLTAG_Boxit,		opts[OPT_BOXIT],
		XLTAG_CDTV,		opts[OPT_CDTV],

		opts[OPT_X] ? XLTAG_Left : TAG_IGNORE,		left,
		opts[OPT_Y] ? XLTAG_Top : TAG_IGNORE, 		top,
		opts[OPT_VOL] ? XLTAG_Volume : TAG_IGNORE, 	vol,
		opts[OPT_XLSPEED] ? XLTAG_XLSpeed : TAG_IGNORE,	xlspeed,
		opts[OPT_LACE] ? XLTAG_LACE : TAG_IGNORE, 	TRUE,
		opts[OPT_NONLACE] ? XLTAG_NONLACE : TAG_IGNORE,	TRUE,
		opts[OPT_HIRES] ? XLTAG_HIRES : TAG_IGNORE,	TRUE,
		opts[OPT_LORES] ? XLTAG_LORES : TAG_IGNORE,	TRUE,
		TAG_END
	    );

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	printf("'%ls'\n",XLError[ret]);
	ret = RETURN_FAIL;
    }

    exit( ret );

} // main()
