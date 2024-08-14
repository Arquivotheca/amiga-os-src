/*******************

    SaveAsFLIK.c

    W.D.L 930612

********************/


#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>

#include <devices/cd.h>

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
#include "cdxl/flik.h"
#include "cdxl/runcdxl_protos.h"


#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"

IMPORT UBYTE			  FileNameBuf[256];
IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct GfxBase		* GfxBase;


#ifdef	OUTT	// [
sep_rgb4(color,rgb)
int color;
int rgb[3];
{
    int i;

    D(PRINTF("sep_rgb4() color= 0x%lx ",color);)

    for (i = 2; i >= 0; i--) {
	rgb[i] = color & 0x0f;
	    color >>= 4;
    }

    D(PRINTF("rgb[0]= 0x%lx, [1]= 0x%lx, [2]= 0x%lx\n",rgb[0],rgb[0],rgb[2]);)

} // sep_rgb4()


rgb4_to_8( int color, UBYTE cm[1][3] )
{
    int rgb[3];

    sep_rgb4( color, rgb );

    cm[0][0] = rgb[0] << 4;
    cm[0][1] = rgb[1] << 4;
    cm[0][2] = rgb[2] << 4;

} // rgb4_to_8()


RGB4_to_colortable( UWORD * cmap, ULONG * table, int num )
{
    int i;
    int rgb[3];
    ULONG c,r,g,b,
    UBYTE cm[1][3]

    for ( i = 0; i < num; i++ ) {
	rgb4_to_8( cmap[i], cm )

	c = cm[i][0];
	r = c | (c << 8) | (c << 16) | (c << 24);

	c = cm[i][1];
	g = c | (c << 8) | (c << 16) | (c << 24);

	c = cm[i][2];
	b = c | (c << 8) | (c << 16) | (c << 24);


    }

} // RGB4_to_colortable()
#endif		// ]


WriteFlikHead( BPTR fh, XLEDIT * xledit, ULONG numframes, ULONG flags )
{
    CDXLOB	* cdxlob = xledit->cdxlob;
    FLIKHEAD	  head;
    ULONG	  numcolors = cdxlob->CMapSize >> 1;
    ULONG	* colortable = NULL;
    int		  ret = RC_OK;


    setmem( &head, sizeof( head ), 0 );

    head.type = FLIK;
    head.framesize = sizeof(FLIKFRAME) + cdxlob->ImageSize + cdxlob->AudioSize;

    kprintf("WriteFlikHead() ImageSize= %ld, numcolors= %ld\n",cdxlob->ImageSize,numcolors);

    // could maybe use GetVPModeID( disp_def.vp )
    head.modeID = xledit->disp_def.ModeID;

    head.numframes = numframes;

/*
 * The format of the table passed to this function is a series of records,
 * each with the following format:
 *
 *        1 Word with the number of colors to load
 *        1 Word with the first color to be loaded.
 *        3 longwords representing a left justified 32 bit rgb triplet.
 *        The list is terminated by a count value of 0.
 *
 *   examples:
 *        ULONG table[]={1l<<16+0,0xffffffff,0,0,0}
 *          loads color register 0 with 100% red.
 *
 *        ULONG table[]={256l<<16+0,r1,g1,b1,r2,g2,b2,.....0}
 *          can be used to load an entire 256 color palette.
 */

    head.colortablesize = (2 * sizeof (WORD)) +
	(numcolors * (3 * sizeof (ULONG))) + sizeof (ULONG);

    head.headsize = sizeof(head) + head.colortablesize;

    if ( flags & FLIK_MULTI_PAL ) {
	head.framesize += head.colortablesize;
	head.flags |= FLIK_MULTI_PAL;
    }
/*
    head.maxcompsize = 0;
    head.firstcompsize = 0;
*/
    head.audiosize = cdxlob->AudioSize;

    head.width = cdxlob->rect.MaxX - cdxlob->rect.MinX;
    head.height = cdxlob->rect.MaxY - cdxlob->rect.MinY;
    head.depth = cdxlob->bm[0].Depth;

    head.compression = 0;

    head.playspeed = cdxlob->ReadXLSpeed;

    if ( !(colortable = AllocMem( head.colortablesize, MEMF_CLEAR) ) ) {
	return( RC_NO_MEM );
    }

    kprintf("\n---------\nDepth= %ld\n",head.depth);

#ifndef	OUTT	// [

    GetRGB32( xledit->disp_def.vp->ColorMap, 0L, numcolors, &colortable[1] );
    colortable[0] = numcolors << 16 + 0;

#else		// ][

    RGB4_to_colortable( cdxlob->CMap, colortable, numcolors );

#endif		// ]

    if ( Write( fh, &head, sizeof( head ) ) != sizeof( head ) ) {
	return( RC_WRITE_ERROR );
    }

    if ( Write( fh, colortable, head.colortablesize ) != head.colortablesize ) {
	return( RC_WRITE_ERROR );
    }

    FreeMem( colortable, head.colortablesize );

    return( ret );

} // WriteFlikHead()


SaveFlikFrame( BPTR fh, XLEDIT * xledit, ULONG flags )
{
    CDXLOB	* cdxlob = xledit->cdxlob;
    FLIKFRAME	  frame;
    int		  ret = RC_OK;

    if ( xledit->frameflags[cdxlob->FrameNum] & XL_FRAME_DELETED ) {
	return( RC_OK );
    }

    frame.framenum = cdxlob->FrameNum;
//    frame.nextcompsize = 0;

    if ( Write( fh, &frame, sizeof( frame ) ) != sizeof( frame ) ) {
	return( RC_WRITE_ERROR );
    }

    if ( Write( fh, cdxlob->Video[xledit->cdxlob->curVideo], cdxlob->ImageSize ) != cdxlob->ImageSize ) {
	return( RC_WRITE_ERROR );
    }

    if ( Write( fh, cdxlob->audio[xledit->cdxlob->curVideo], cdxlob->AudioSize ) != cdxlob->AudioSize ) {
	return( RC_WRITE_ERROR );
    }

    if ( flags & FLIK_MULTI_PAL ) {
	// Write out the framecolortable
    }

    return( ret );

} // SaveFlikFrame()


SaveFLIK( XLEDIT * xledit, UBYTE * name, ULONG numframes )
{
    BPTR	  fh = NULL;
    LONG	  ret;


    if( !(fh = Open( name, MODE_NEWFILE )) ) {
	ret = RC_CANT_OPEN;
	printf("Could NOT Open '%ls'\n",name);
	goto exit;
    }

    if ( ret = WriteFlikHead( fh, xledit, numframes, NULL ) ) {
	printf("Could NOT write the FLIKHEAD\n");
	goto exit;
    }

    do {
	if ( ret = SaveFlikFrame( fh, xledit, NULL ) ) {
	    printf("Error writing FLIKFRAME\n");
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


} // SaveFLIK()


SaveAsFLIK( XLEDIT * xledit, struct Window * win )
{
    int			  ret = RC_OK;
    ULONG		  start,end,numframes;
    struct Requester	  sleepReq;
    BOOL		  sleeping;

    if ( !(xledit && xledit->cdxlob) ) {
	DisplayBeep( win->WScreen );
	return( RC_FAILED );
    }

    kprintf("SaveAsFLIK() ENTERED with curVideo= %ld\n",
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

	ret = SaveFLIK( xledit, FileNameBuf, numframes );

	if ( sleeping )
	    EndRequest( &sleepReq, win );

	ClearPointer( win );
    }

    return( ret );

} // SaveAsFLIK()

