/*************

    Flik.c

    W.D.L 930613

**************/


#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>

#include <devices/cd.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <math.h>	// For min() & max()
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


/*
 * Take a FLIKHEAD structure and determine what kind of display to open.
 */
VOID
Flik2Display( FLIKHEAD * head, DISP_DEF * disp_def, ULONG flags, struct TagItem * inti )
{
    int			i,ht = NTSC_HEIGHT;
    struct Rectangle	drect;
    struct DisplayInfo	disp;
    struct TagItem	* ti;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    ht = PAL_HEIGHT;
    }

    disp_def->Depth = head->depth;


    D(PRINTF("\nFlik2Display ENTERED\n");)

    if ( !(disp_def->Flags & DISP_BACKGROUND) || 
	(disp_def->Flags & DISP_XLMODEID) ) {
	disp_def->ModeID = head->modeID;
	D(PRINTF("Flik2Display 1 head->modeID= 0x%lx\n",head->modeID);)
    }

    // High Resolution guess
    i = SCREEN_WIDTH;
    if ( head->width < i) {
	i >>= 1;
    }

    if ( head->width > i)
	disp_def->Flags |= DISP_OVERSCANX;

    i = ht;

    // InterLace guess
    if ( head->height < i ) {
	i >>= 1;
    }

    if ( head->height > i)
	disp_def->Flags |= DISP_OVERSCANY;


    /*
     * Tag that tells us to force an interlace/noninterlace display.
     */
    if ( ti = FindTagItem( XLTAG_LACE, inti ) ) {
	if ( ti->ti_Data ) 
	    disp_def->ModeID |= LACE;
	else
	    disp_def->ModeID &= ~LACE;
    }

    /*
     * Tag that tells us to force a noninterlace/interlace display.
     */
    if ( ti = FindTagItem( XLTAG_NONLACE, inti ) ) {
	if ( ti->ti_Data ) 
	    disp_def->ModeID &= ~LACE;
	else
	    disp_def->ModeID |= LACE;
    }

    /*
     * Tag that tells us to force a HIRES/LORES display.
     */
    if ( ti = FindTagItem( XLTAG_HIRES, inti ) ) {
	if ( ti->ti_Data ) 
	    disp_def->ModeID |= HIRES;
	else
	    disp_def->ModeID &= ~HIRES;
    }

    /*
     * Tag that tells us to force a LORES/HIRES display.
     */
    if ( ti = FindTagItem( XLTAG_LORES, inti ) ) {
	if ( ti->ti_Data ) 
	    disp_def->ModeID &= ~HIRES;
	else
	    disp_def->ModeID |= HIRES;
    }

    /*
     * Tag that tells us to force a scan doubled/single display.
     */
    if ( ti = FindTagItem( XLTAG_SDBL, inti ) ) {
	if ( ti->ti_Data ) {
	    disp_def->ModeID |= LORESSDBL_KEY;
	} else {
	    disp_def->ModeID &= ~LORESSDBL_KEY;
	}
    }

    if ( ((ti = FindTagItem( XLTAG_NoPromote, inti )) && ti->ti_Data) || 
       (disp_def->ModeID & LORESSDBL_KEY) || !(flags & CDXL_BLIT) ) {

	disp_def->ModeID &= ~MONITOR_ID_MASK;
	if ( ht == PAL_HEIGHT ) {
	    disp_def->ModeID |= PAL_MONITOR_ID;
	} else {
	    disp_def->ModeID |= NTSC_MONITOR_ID;
	}
    }

    disp_def->NominalWidth = SCREEN_WIDTH;
    disp_def->NominalHeight = ht;

    if ( !(disp_def->Flags & DISP_BACKGROUND) ) {
	disp_def->Width = head->width;
	disp_def->Height = head->height;
    }

    if ( disp_def->Flags & DISP_OVERSCAN ) {

	QueryOverscan( disp_def->ModeID, &drect, OSCAN_MAX );

	if ( disp_def->Flags & DISP_OVERSCANX ) {
	    D(PRINTF("Flik2Display() DISP_OVERSCANX\n");)
	    disp_def->NominalWidth = (drect.MaxX - drect.MinX) + 1;
	    disp_def->Left = drect.MinX + (disp_def->NominalWidth - disp_def->Width);

	} else if ( !(disp_def->ModeID & HIRES) ) {
	    disp_def->NominalWidth >>= 1;
	}

	if ( disp_def->Flags & DISP_OVERSCANY ) {
	    D(PRINTF("Flik2Display() DISP_OVERSCANY\n");)
	    disp_def->NominalHeight = (drect.MaxY - drect.MinY) + 1;
	    disp_def->Top = drect.MinY + (disp_def->NominalHeight - disp_def->Height);

	} else if ( !(disp_def->ModeID & LACE) ) {
	    disp_def->NominalHeight >>= 1;
	}

	D(PRINTF("Flik2Display() DISP_OVERSCAN, NominalWidth= %ld, NominalHeight= %ld\nWidth= %ld, Height= %ld\n MinX= %ld, MaxX= %ld, MinY= %ld, MaxY= %ld\n",
	    disp_def->NominalWidth,disp_def->NominalHeight,
	    disp_def->Width,disp_def->Height,
	    drect.MinX,drect.MaxX,drect.MinY,drect.MaxY);)
    } else {

	if ( !(disp_def->ModeID & HIRES) )
	    disp_def->NominalWidth >>= 1;

	if ( !(disp_def->ModeID & LACE) )
	    disp_def->NominalHeight >>= 1;
    }

    D(PRINTF("Flik2Display() Left= %ld, Top= %ld\n",disp_def->Left,disp_def->Top);)

    D(PRINTF("Flik2Display disp_def->ModeID= 0x%lx\n",disp_def->ModeID);)

    if ( disp_def->Flags & DISP_BACKGROUND )
	return;

    /*
     * If we are blitting from a buffer to our display, open the display
     * full width. Else open a display only as big as the CDXL.
     */
    if ( flags & CDXL_BLIT ) {
	disp_def->Width = disp_def->NominalWidth;
	disp_def->Height = disp_def->NominalHeight;
    } else {
	disp_def->Width = head->width;
	disp_def->Height = head->height;
    }

} // Flik2Display


FlikObtain( UBYTE * CDXLFile, DISP_DEF * disp_def, CDXLOB **CDXLob, ULONG flags, struct TagItem * inti )
{
    struct FileLock	    * FileLock;
    struct FileInfoBlock    * fib;
    struct TagItem 	    * ti;
    CDXLOB		    * CDXL_ob;
    ULONG		      Size;
    LONG		      fsLock;
    LONG		      File;
    UBYTE		      fibbuf[ ( sizeof( struct FileInfoBlock ) + 3 ) ];
    ULONG		      CDXLSector;
    int			      ret,i,j,ImageStart;
    FLIKHEAD		      head;
    BOOL		      sdbl;

    *CDXLob = CDXL_ob = NULL;

    if( !(CDXLFile ) )
	return( RC_MISSING_FILE );

    File = NULL;

    if ( ! ( fsLock = Lock( CDXLFile, ACCESS_READ ) ) ) {
	printf("Could not Lock '%ls'\n",CDXLFile);
	ret = RC_CANT_FIND;
	goto error;
    }

    FileLock = (struct FileLock *) BADDR( fsLock );

    // Pluck the sector right out of public part of the structure
    CDXLSector = FileLock->fl_Key;

    // Get size of file.
    // (Force longword alignment of fib pointer.)
    fib = (struct FileInfoBlock *) ( (LONG) ( fibbuf + 3 ) & ~3L );
    Examine( fsLock, fib );
    Size = fib->fib_Size;

    UnLock( fsLock );
    fsLock = NULL;

    if ( ! ( File = Open( CDXLFile, MODE_OLDFILE ) ) ) {
	printf("Could not open '%ls'\n",CDXLFile);
	ret = RC_CANT_FIND;
	goto error;
    }

    // Get the first FLIKHEAD
    if ( Read( File, &head, sizeof (head) ) != sizeof (head) ) {
	ret = RC_READ_ERROR;
	goto error;
    }

    if( head.type != FLIK ) {
	ret = RC_BAD_PAN;
	goto error;
    }


    /*
     * If we are NOT loading an ILBM for the background, determine
     * what kind of display to open based upon the FLIKHEAD. Else we
     * determine the display from the ILBM.
     */
//    if ( (disp_def->Flags & DISP_XLMODEID) || !(disp_def->Flags & DISP_BACKGROUND) )
	Flik2Display( &head, disp_def, flags, inti );

    CenterDisplay( disp_def );
/*
    Close( File );
    File = NULL;
*/
    if( !(CDXL_ob = AllocMem( sizeof( CDXLOB ), MEMF_CLEAR ) ) ) {
	ret = RC_NO_MEM;
	goto error;
    }

    CDXL_ob->FibSize = Size;

    // get the playspeed
    CDXL_ob->ReadXLSpeed = head.playspeed;

    // if the file does NOT specify multipal, override user setting
    if ( !(head.flags & FLIK_MULTI_PAL ) ) {
	flags &= ~CDXL_MULTI_PALETTE;
    } else {
	flags |= CDXL_MULTI_PALETTE;
	// Mark whether each frame has the data ready to pass to LoadRGB32(),
	// else it will be in a format of UBYTE ctable[numcolors][3]. The 3 
	// being RGB. if NON RGB32 format, the colordata will be converted
	// into RBG32 format.
	if ( head.flags & FLIK_FRAME_RGB32 )
	    flags |= CDXL_RGB32;
    }

    CDXL_ob->flags = flags|CDXL_FLIK;

    CDXL_ob->FrameSize = head.framesize;
    CDXL_ob->AudioSize = head.audiosize;
    CDXL_ob->CMapSize = head.colortablesize;
    CDXL_ob->PlaneSize = ROW_SIZE (head.width) * head.height;
    CDXL_ob->ImageSize = CDXL_ob->PlaneSize * head.depth;


    if( !(CDXL_ob->colortable32 = AllocMem( CDXL_ob->CMapSize, MEMF_CLEAR )) ) {
	ret = RC_NO_MEM;
	goto error;
    }


#ifdef	OUTT	// [
    CDXL_ob->BufSize = CDXL_ob->FrameSize;

    // I need to ALWAYS allocate space for the colortable.
    // If FLIK_MULTI_PAL then FrameSize will already account
    // for the size of the colortable. If not, then I must
    // specifically add the colortablesize to the bufsize.
    if ( !(head.flags & FLIK_MULTI_PAL) ) {
	CDXL_ob->BufSize += CDXL_ob->CMapSize;
    }
#else		// ][

#ifdef	OUTT		// {

    // 930721. Always account for CMapSize, even if FLIK_MULTI_PAL. If
    // FLIK_FRAME_RGB32, then head.framesize should be equal to the equation
    // below, else the framesize will be less. Use the calculation below
    // so that all cases will be accounted for. This is because I do always
    // need space to load in at least the head.colortable.

    CDXL_ob->BufSize = sizeof (FLIKFRAME) + CDXL_ob->ImageSize+CDXL_ob->AudioSize+CDXL_ob->CMapSize;
#else			// }{

    CDXL_ob->BufSize = CDXL_ob->FrameSize;


#endif			// }

#endif		// ]

    CDXL_ob->StartSector = CDXLSector;

    CDXL_ob->NumFrames = head.numframes;

    D(PRINTF("head.numframes= %ld\n",head.numframes);)
						    			// Seek past the FLIKHEAD
    if ( (flags & CDXL_DOSXL) && !(CDXL_ob->xlfile = OpenAsyncXL( CDXLFile, CDXL_ob, head.headsize ) ) ) {
	D(PRINTF("Could NOT open AsyncXL FILE\n");)
	ret = RC_NO_MEM; // ??
	goto error;
    }


    /*
     * Set 2 buffers.
     */
    for ( j = 0; j < 2; j++ ) {

	if( !(flags & CDXL_DOSXL) && !(CDXL_ob->Buffer[j] = AllocMem( CDXL_ob->BufSize, MEMF_CHIP|MEMF_CLEAR ) ) ) {
	    ret = RC_NO_MEM;
	    goto error;
	}

	/*
	 * Point the audio to the correct offset into the buffer.
	 */
	if( CDXL_ob->AudioSize ) {
	    CDXL_ob->audio[j] = (UBYTE *)CDXL_ob->Buffer[j] + sizeof (FLIKFRAME) + 
		CDXL_ob->ImageSize;
	}

	ImageStart = sizeof (FLIKFRAME);

	if ( CDXL_ob->flags & CDXL_MULTI_PALETTE ) {
	    /*
	     * Point the CMAP to the correct offset into the buffer.
	     */
	    CDXL_ob->CMap[j] = (UWORD *)(CDXL_ob->Buffer[j] + ImageStart + 
		CDXL_ob->ImageSize + CDXL_ob->AudioSize);
	}

	/*
	 * Since the bitmap MUST be the same size as the CDXL,
	 * AllocBitMap() can NOT be used as it will round the width
	 * up according to the restrictions of the OS.
	 */
	InitBitMap(&CDXL_ob->bm[j],head.depth,head.width,head.height);

	/*
	 * Point the Video to the correct offset into the buffer.
	 */
	CDXL_ob->Video[j] = CDXL_ob->Buffer[j] + ImageStart;

	/*
	 * Set up the bitmap planes to point into our buffer.
	 */
	for ( i = 0; i < CDXL_ob->bm[j].Depth; i++ ) {
	    CDXL_ob->bm[j].Planes[i] = (PLANEPTR)(CDXL_ob->Video[j] + i * CDXL_ob->PlaneSize);
	}

    }

    kprintf("FrameSize= %ld, ImageSize= %ld, AudioSize= %ld, CMapSize= %ld\n",
	CDXL_ob->FrameSize,CDXL_ob->ImageSize,CDXL_ob->AudioSize,CDXL_ob->CMapSize);

    kprintf("Bufsize= %ld, ImageStart= %ld, buffer= 0x%lx, CMap= 0x%lx\n",
	CDXL_ob->BufSize,ImageStart,CDXL_ob->Buffer[1],CDXL_ob->CMap[1]);

#ifdef	OUTT	// [
    if( Read( File, CDXL_ob->CMap[0], CDXL_ob->CMapSize ) != CDXL_ob->CMapSize ) {
	ret = RC_READ_ERROR;
	goto error;
    }

    CopyMem( CDXL_ob->CMap[0], CDXL_ob->CMap[1], CDXL_ob->CMapSize );
#else		// ][
    if( Read( File, CDXL_ob->colortable32, CDXL_ob->CMapSize ) != CDXL_ob->CMapSize ) {
	ret = RC_READ_ERROR;
	goto error;
    }

#endif		// ]

    // Seek into the first frame, past the first part of the flikframe
    Seek( File, ImageStart, OFFSET_CURRENT );

    Read( File, CDXL_ob->bm[0].Planes[0], CDXL_ob->ImageSize );
    BltBitMap( &CDXL_ob->bm[0],0,0,&CDXL_ob->bm[1],0,0,head.width,head.height,0xC0,0xFF,NULL);
    Close( File );
    File = NULL;

    /*
     * If a volume is specified use it.
     */
    if ( ti = FindTagItem( XLTAG_Volume, inti )  ) {
	CDXL_ob->Volume = ti->ti_Data;
    } else {
	CDXL_ob->Volume = 64;
    }

    if ( (ti = FindTagItem( XLTAG_SDBL, inti )) && ti->ti_Data ) {
	sdbl = TRUE;
    } else {
	sdbl = FALSE;
    }

    /*
     * If a left value is specified use it. Else center.
     */
    if ( ti = FindTagItem( XLTAG_Left, inti )  ) {
	CDXL_ob->rect.MinX = ti->ti_Data;
    } else {
	// Center it;
	CDXL_ob->rect.MinX = (max(disp_def->NominalWidth,disp_def->Width) >> 1) - (head.width >> 1);
    }

    /*
     * If a top value is specified use it. Else center.
     */
    if ( ti = FindTagItem( XLTAG_Top, inti )  ) {
	CDXL_ob->rect.MinY = ti->ti_Data;
    } else {
	// Center it;
	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) - (sdbl ? (head.height << 1) : head.height) )/2;
//	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) >> 1) - (sdbl ? head.height : (head.height >> 1));
//	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) >> (sdbl ? 2 : 1)) - head.height;
	D(PRINTF("1 MinY = %ld, Ht= %ld, Ht>>1= %ld, wdith= %ld, height>>1= %ld\n",
	    CDXL_ob->rect.MinY,max(disp_def->NominalHeight,disp_def->Height),
		(max(disp_def->NominalHeight,disp_def->Height) >> 1),
		head.height,(head.height >> 1));)
    }

    // Clip it.
    CDXL_ob->rect.MinX = min( CDXL_ob->rect.MinX, disp_def->Width );
    CDXL_ob->rect.MinY = min( CDXL_ob->rect.MinY, disp_def->Height );

/*
    // If display will be scan doubled
    if ( sdbl ) {
	D(PRINTF("SDBL... Before MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
	CDXL_ob->rect.MinY >>= 2;
	disp_def->Top >>= 2;
	D(PRINTF("AFTER MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
    }
*/

    CDXL_ob->rect.MaxX = head.width + CDXL_ob->rect.MinX;
    CDXL_ob->rect.MaxY = head.height + CDXL_ob->rect.MinY;

    // Clip it.
    CDXL_ob->rect.MaxX = min( CDXL_ob->rect.MaxX, disp_def->Width );
    CDXL_ob->rect.MaxY = min( CDXL_ob->rect.MaxY, disp_def->Height );

    *CDXLob = CDXL_ob;	// Set return ptr

    /*
     * If we are useing the imbedded bitmaps for the display
     * (ie we are NOT blitting from the imbedded bitmaps to the
     * display), point the display bitmaps to the imbedded ones.
     */
    if ( !(disp_def->Flags & DISP_ALLOCBM) ) {
	disp_def->bm[0] = &CDXL_ob->bm[1];
	disp_def->bm[1] = &CDXL_ob->bm[0];

	if ( !(disp_def->Flags & DISP_OVERSCANX) ) {
	    disp_def->Left = CDXL_ob->rect.MinX;
	    D(PRINTF("Setting disp_def->Left 1 to %ld\n",disp_def->Left);)
	}

	if ( !(disp_def->Flags & DISP_OVERSCANY) && !(disp_def->Flags & DISP_SCREEN) ) {
	    disp_def->Top = CDXL_ob->rect.MinY;
	    D(PRINTF("Setting disp_def->Top 1 to %ld\n",disp_def->Top);)
	}
    }

    D(PRINTF("FlikObtain... END MinY= %ld, Top= %ld, Height= %ld, .height= %ld\n",
	CDXL_ob->rect.MinY,disp_def->Top,disp_def->Height,head.height);)

    D(PRINTF("ImageStart= %ld, CMapSize= %ld, ImageSize= %ld, AudioSize= %ld, FrameSize= %ld\n",
	ImageStart,CDXL_ob->CMapSize,CDXL_ob->ImageSize,CDXL_ob->AudioSize,CDXL_ob->FrameSize);)


    return( RC_OK );

error:

    if ( CDXL_ob && CDXL_ob->xlfile ) {
	CloseAsyncXL( CDXL_ob->xlfile, CDXL_ob );
	CDXL_ob->xlfile = NULL;
    }

    printf("FlikObtain error ret= %ld\n",ret);

    if( File )
	Close( File );

    if ( fsLock )
	UnLock( fsLock );

    CDXLFreeOb( CDXL_ob );

    return( ret );

} // FlikObtain()

