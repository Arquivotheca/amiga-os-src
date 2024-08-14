/*******************

    SaveAsCDXL.c

      930602

********************/


#include <exec/types.h>
#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>

//#include <devices/cd.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>	// for setmem()

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/blitdef.h"
#include "cdxl/xledit.h"
#include "cdxl/runcdxl_protos.h"


#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"

IMPORT UBYTE			  FileNameBuf[256];
IMPORT struct IntuitionBase	* IntuitionBase;


SaveFrame( BPTR fh, XLEDIT * xledit )
{
    CDXLOB	* cdxlob = xledit->cdxlob;
    PAN		* pan;
    int		  ret = RC_OK;

    if ( xledit->frameflags[cdxlob->FrameNum] & XL_FRAME_DELETED ) {
	return( RC_OK );
    }

    pan = (PAN *)&cdxlob->Buffer[cdxlob->curVideo];

    pan->Frame = cdxlob->FrameNum;

    if ( Write( fh, cdxlob->Buffer[cdxlob->curVideo], cdxlob->FrameSize ) != cdxlob->FrameSize ) {
	return( RC_WRITE_ERROR );
    }

    return( ret );

} // SaveFrame()


SaveXL( XLEDIT * xledit, UBYTE * name, LONG numframes )
{
    BPTR	  fh = NULL;
    LONG	  ret;


    if( !(fh = Open( name, MODE_NEWFILE )) ) {
	ret = RC_CANT_OPEN;
	printf("Could NOT Open '%ls'\n",name);
	goto exit;
    }

    do {
	if ( ret = SaveFrame( fh, xledit ) ) {
	    goto exit;
	}

	if ( --numframes && (ret = StepFrame( xledit, TRUE )) ) {
	    kprintf("BREAKING numframes= %ld, FrameNum= %ld , NumFrames= %ld\n",
		numframes,xledit->cdxlob->FrameNum,xledit->cdxlob->NumFrames);
	    break;
	}
	UpDateFrame( xledit->cdxlob->FrameNum );

    } while ( numframes && ( xledit->cdxlob->FrameNum < (xledit->cdxlob->NumFrames-1) ) );

exit:

    if( fh ) {
	Close( fh );
    }

    return( ret );


} // SaveXL()


SaveAsCDXL( XLEDIT * xledit, struct Window * win )
{
    int			  ret = RC_OK;
    ULONG		  start,end,numframes;
    struct Requester	  sleepReq;
    BOOL		  sleeping;

    if ( !(xledit && xledit->cdxlob) ) {
	DisplayBeep( win->WScreen );
	return( RC_FAILED );
    }

    kprintf("SaveAsCDXL() ENTERED with curVideo= %ld\n",
	xledit->cdxlob->curVideo);

    start = end = xledit->cdxlob->FrameNum;

    if ( !RangeReq( xledit, win, &start, &end ) ) {
	return( RC_OK );
    }

    if ( FileReq( win ) ) {

	InitRequester( &sleepReq );
	sleeping = Request( &sleepReq, win );
	SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );

	start = (start > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : start;

	numframes = (end - start) + 1;
	numframes = (numframes < 1) ? 1 : numframes;
	numframes = (numframes > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : numframes;

//	if ( start != xledit->cdxlob->FrameNum ) {
	    GoToFrame( xledit, start );
	    UpDateFrame( xledit->cdxlob->FrameNum );
//	}

	ret = SaveXL( xledit, FileNameBuf, numframes );

	if ( sleeping )
	    EndRequest( &sleepReq, win );

	ClearPointer( win );
    }

    return( ret );

} // SaveAsCDXL()

