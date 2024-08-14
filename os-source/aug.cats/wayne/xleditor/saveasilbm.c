/*******************

    SaveAsILBM.c

      930527

********************/


#include <exec/types.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "iffp/ilbm.h"
#include "iffp/ilbmapp.h"


#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/blitdef.h"
#include "cdxl/xledit.h"
#include "cdxl/runcdxl_protos.h"

#define BODYBUFSZ	5004


IMPORT struct Library * GfxBase;
IMPORT struct Library * IFFParseBase;

IMPORT UBYTE	FileNameBuf[256];


/* saveilbm
 *
 * Given an ILBMInfo with a currently available (not-in-use)
 *   ParseInfo->iff IFFHandle, a BitMap ptr,
 *   modeid, widths/heights, colortable, ncolors, bitspergun,
 *   masking, transparent color, optional chunklists, and filename,
 *   will save the bitmap as an ILBM.
 *
 *  if bitspergun=4,  colortable is words, each with nibbles 0RGB
 *  if bitspergun=8,  colortable is byte guns of RGBRGB etc. (like a CMAP)
 *  if bitspergun=32, colortable is ULONG guns of RGBRGB etc.
 *     Only the high eight bits of each gun will be written to CMAP.
 *     Four bit guns n will be saved as nn
 *
 * The struct Chunk *chunklist is for chunks you wish written
 * other than BMHD, CMAP, and CAMG (they will be screened out)
 * because they are calculated and written separately
 *
 * Returns 0 for success, or an IFFERR
 */
LONG
saveilbm( struct ILBMInfo *ilbm, struct BitMap *bitmap, ULONG modeid,
 WORD width, WORD height, WORD pagewidth, WORD pageheight,
 APTR colortable, UWORD ncolors, UWORD bitspergun,
 WORD masking, WORD transparentColor,
 struct Chunk *chunklist1, struct Chunk *chunklist2, UBYTE *filename )
{
    struct IFFHandle	* iff;
    struct Chunk	* chunk;
    UBYTE		* bodybuf;
    ULONG		  chunkID;
    LONG		  size, error = 0L;

    iff = ilbm->ParseInfo.iff;

    if ( !(modeid & 0xFFFF0000) )
	modeid &= OLDCAMGMASK;

    if( !(bodybuf = AllocMem(BODYBUFSZ,MEMF_PUBLIC)) ) {
	message(SI(MSG_IFFP_NOMEM));
	return(IFFERR_NOMEM);
    }

    if( !(error = openifile(ilbm, filename, IFFF_WRITE)) ) {
	D(bug("Opened %s for write\n",filename));

	error = PushChunk(iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN);

	D(bug("After PushChunk FORM ILBM - error = %ld\n", error));

        initbmhd(&ilbm->Bmhd, bitmap, masking, cmpByteRun1, transparentColor,
            		width, height, pagewidth, pageheight, modeid);

	D(bug("Error before putbmhd = %ld\n",error));

	CkErr(putbmhd(iff,&ilbm->Bmhd));	

	if(colortable)	CkErr(putcmap(iff,colortable,ncolors,bitspergun));

	ilbm->camg = modeid;
	D(bug("before putcamg - error = %ld\n",error));
	CkErr(putcamg(iff,&modeid));

	D(bug("Past putBMHD, CMAP, CAMG - error = %ld\n",error));

	/* Write out chunklists 1 & 2 (if any), except for
	 * any BMHD, CMAP, or CAMG (computed/written separately)
	 */
	for(chunk = chunklist1; chunk; chunk = chunk->ch_Next) {
	    D(bug("chunklist1 - have a %.4s\n",&chunk->ch_ID));
	    chunkID = chunk->ch_ID;
	    if((chunkID != ID_BMHD)&&(chunkID != ID_CMAP)&&(chunkID != ID_CAMG)) {
		size = chunk->ch_Size==IFFSIZE_UNKNOWN ?
			strlen(chunk->ch_Data) : chunk->ch_Size;
		D(bug("Putting %.4s\n",&chunk->ch_ID));
		CkErr(PutCk(iff, chunkID, size, chunk->ch_Data));
	    }
	}

	for(chunk = chunklist2; chunk; chunk = chunk->ch_Next) {
	    chunkID = chunk->ch_ID;
	    D(bug("chunklist2 - have a %.4s\n",&chunk->ch_ID));

	    if((chunkID != ID_BMHD)&&(chunkID != ID_CMAP)&&(chunkID != ID_CAMG)) {
		size = chunk->ch_Size==IFFSIZE_UNKNOWN ?
			strlen(chunk->ch_Data) : chunk->ch_Size;
		D(bug("Putting %.4s\n",&chunk->ch_ID));
		CkErr(PutCk(iff, chunkID, size, chunk->ch_Data));
	    }
	}

	/* Write out the BODY
	 */
	CkErr(putbody(iff, bitmap, NULL, &ilbm->Bmhd, bodybuf, BODYBUFSZ));

	D(bug("Past putbody - error = %ld\n",error));


	CkErr(PopChunk(iff));	/* close out the FORM */
	closeifile(ilbm);	/* and the file */
    }

    FreeMem(bodybuf,BODYBUFSZ);

    return(error);

} // saveilbm()


SaveXLAsILBM( XLEDIT * xledit, struct Window * win )
{
    ULONG 	      modeid;
    UWORD	      count;
    int		      Wid,Ht,len,ret = RC_OK;
    struct ILBMInfo   ilbm;
    ULONG	      start,end,numframes;
    struct Requester  sleepReq;
    BOOL	      sleeping;
    UBYTE	      countbuf[50];
    struct Screen   * scr = xledit->disp_def.screen;
    Color32	    * colortable32;


    kprintf("SaveXLAsILBM() ENTERED\n");

    if( !IFFParseBase && !(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	printf("Could NOT open iffparse.library!\n");
	return( RC_FAILED );
    }

    kprintf("SaveXLAsILBM() 0\n");

    start = end = xledit->cdxlob->FrameNum;

    if ( !RangeReq( xledit, win, &start, &end ) ) {
	return( RC_OK );
    }

    if ( FileReq( win ) ) {

	kprintf("SaveXLAsILBM() 1\n");

	if ( !(ilbm.ParseInfo.iff = AllocIFF()) ) {
	    setmem( &ilbm, sizeof (ilbm) , 0 );
	    return( RC_NO_MEM );
	}

	InitRequester( &sleepReq );
	sleeping = Request( &sleepReq, win );
	SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );

	start = (start > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : start;

	numframes = (end - start) + 1;
	numframes = (numframes < 1) ? 1 : numframes;
	numframes = (numframes > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : numframes;

	GoToFrame( xledit, start );
	UpDateFrame( xledit->cdxlob->FrameNum );

	kprintf("SaveXLAsILBM() 2\n");

	modeid = GetVPModeID( &scr->ViewPort );

	count = scr->ViewPort.ColorMap->Count;

	if ( colortable32 = (Color32 *)AllocMem( sizeof (Color32) * count, MEMF_CLEAR) ) {

	    kprintf("SaveXLAsILBM() 3\n");

	    GetRGB32( scr->ViewPort.ColorMap, 0L, count, (ULONG *)colortable32 );

	    Wid = (xledit->cdxlob->rect.MaxX - xledit->cdxlob->rect.MinX);
	    Ht = (xledit->cdxlob->rect.MaxY - xledit->cdxlob->rect.MinY);

	    kprintf("SaveXLAsILBM() 4\n");

	    len = strlen( FileNameBuf );
	    do {
		FileNameBuf[len] = 0;
		sprintf( FileNameBuf, "%ls%ld", FileNameBuf, xledit->cdxlob->FrameNum );
		if ( ret = saveilbm( &ilbm, &xledit->cdxlob->bm[xledit->cdxlob->curVideo], modeid,
		    Wid, Ht, xledit->disp_def.NominalWidth, xledit->disp_def.NominalHeight,
		    colortable32, count, 32,
		    mskNone, 0,
		    NULL, NULL, FileNameBuf ) ) {

			numframes = 1; // Force an abort
		}

		if ( --numframes && (ret = StepFrame( xledit, TRUE )) ) {
		    kprintf("BREAKING numframes= %ld, FrameNum= %ld , NumFrames= %ld\n",
			numframes,xledit->cdxlob->FrameNum,xledit->cdxlob->NumFrames);
		    break;
		}
		UpDateFrame( xledit->cdxlob->FrameNum );

	    } while ( numframes && ( xledit->cdxlob->FrameNum < (xledit->cdxlob->NumFrames-1) ) );

	    kprintf("SaveXLAsILBM() 5\n");

	    FreeMem( colortable32, sizeof(Color32) * count );
	}

	kprintf("SaveXLAsILBM() 6\n");

	FreeIFF( ilbm.ParseInfo.iff );

	if ( sleeping )
	    EndRequest( &sleepReq, win );

	ClearPointer( win );
    }

    kprintf("SaveXLAsILBM() END ret= %ld\n",ret);

    return( ret );

} // SaveXLAsILBM()

