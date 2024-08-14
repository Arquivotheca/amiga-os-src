/*************

    RunCDXL.c

    W.D.L 930330

**************/

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
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>

#include <libraries/lowlevel.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()
#include "stdio.h"

#include "devices/cd.h"

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/asyncXL.h"
#include "cdxl/runcdxl_protos.h"
#include "cdxl/xlm.h"

#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
/**/
#define KPRINTF
//#include "cdxl/debugson.h"


struct CDInfo __aligned cdinfo,saveinfo;

struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;

struct Process * parent;
ASYNCXLFILE * xlfile;

STATIC	struct MinList	XList;
STATIC	struct MinNode	* Pred,* Succ;

	LONG	XLSignal	= -1;
STATIC	LONG	XLSignalBit	= -1;

ULONG	Count;

CDXLOB * CDXL_OB;	// Global CDXLOB

BOOL	AudioSignalTask;

STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;

STATIC struct IOStdReq	* CDDeviceMReq;

STATIC LONG	CDPortSignal	= -1;

IMPORT struct Custom far custom;
IMPORT ULONG	CopSignal;

IMPORT struct CIA far ciaa;

/*
 * Close every thing down.
 */
STATIC VOID
closedown( VOID )
{
    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

    if ( LowLevelBase )
	CloseLibrary( LowLevelBase );

} // closedown()


/*
 * Open all of the required libraries. If iff is TRUE, then open iffparse.
 */
STATIC
init( BOOL iff )
{
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library!\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library!\n");
	return( RC_FAILED );
    }

    if ( iff ) {
	D(PRINTF("opening iffparse.library!\n");)
	if(!(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	    printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
	}
    }

    LowLevelBase = OpenLibrary( "lowlevel.library", 39L );

    D(PRINTF("Opening freeanim.library\n");)
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    D(PRINTF("After opening freeanim.library\n");)

    return( RC_OK );

} // init()


VOID
StopCDXL( struct IOStdReq * req )
{
    if ( req ) {
	// This correctly terminates a CDXL
	if ( !CheckIO( (struct IORequest *)req ) ) {
	    AbortIO( (struct IORequest *)req );
	}
	WaitIO( (struct IORequest *)req );

/*	Removed 930831
	// Then a SEEK is required to force the drive to stop (sometimes necessary)
	req->io_Command = CD_SEEK;
	req->io_Offset  = 0;
	req->io_Length  = 0;
	req->io_Data    = NULL;

	SendIO( (struct IORequest *)req );
	WaitIO( (struct IORequest *)req );
*/
    }

} // StopCDXL()


/*
 * Send a CD_INFO command to cd.device. The info is stored in the cdinfo structure.
 */
BOOL
GetCDInfo( struct CDInfo * cdi )
{
    SendIOR( CDDeviceMReq, CD_INFO, NULL, sizeof ( *cdi ), cdi );
    WaitIO( (struct IORequest *)CDDeviceMReq );

    if ( !(CDDeviceMReq->io_Error ) ) {
	D(PRINTF("\nReadXLSpeed= %ld, MaxSpeed= %ld\nSectorSize= %ld, XLECC= %ld\nStatus= %ld\n\n",
	cdi->ReadXLSpeed,cdi->MaxSpeed,cdi->SectorSize,cdi->XLECC,cdi->Status);)

	return( TRUE );
    } else {
	printf("CD_INFO ERROR!!! io_Error= %ld\n",CDDeviceMReq->io_Error);
	kprintf("CD_INFO ERROR!!! io_Error= %ld\n",CDDeviceMReq->io_Error);
	return( FALSE );
    }

} // GetCDInfo()


/*
 * Send a CD_CONFIG command to cd.device.
 */
BOOL
CDConfig( ULONG tag, ... )
{
    SendIOR( CDDeviceMReq, CD_CONFIG, NULL, 0, &tag );

    D(PRINTF("CDConfig() Calling WaitIO() ior= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx, ln_Type= 0x%lx\n",
	CDDeviceMReq,CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,CDDeviceMReq->io_Message.mn_Node.ln_Type);)

    WaitIO( (struct IORequest *)CDDeviceMReq );

    D(PRINTF("CDConfig() After WaitIO() ior= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx, ln_Type= 0x%lx\n",
	CDDeviceMReq,CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,CDDeviceMReq->io_Message.mn_Node.ln_Type);)

    if ( CDDeviceMReq->io_Error ) {
	printf("\n!!!CD_CONFIG ERROR!!! io_Error= %ld\n\n",CDDeviceMReq->io_Error);
	kprintf("\n!!!CD_CONFIG ERROR!!! io_Error= %ld\n\n",CDDeviceMReq->io_Error);
	return( FALSE );
    }

    GetCDInfo( &cdinfo );

    return( TRUE );

} // CDConfig()


/*
 * Take a Pan structure and determine what kind of display to open.
 */
VOID
Pan2Display( PAN * pan, DISP_DEF * disp_def, ULONG flags, struct TagItem * inti )
{
    LONG		pi;
    int			i,ht = NTSC_HEIGHT;
    struct Rectangle	drect;
    struct DisplayInfo	disp;
    struct TagItem	* ti;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    ht = PAL_HEIGHT;
    }

    disp_def->Depth = pan->PixelSize;

    pi = PI_VIDEO( pan );

//    disp_def->ModeID = NULL;

    if ( !(disp_def->Flags & DISP_BACKGROUND) || 
	(disp_def->Flags & DISP_XLMODEID) ) {

	if( pi == PIV_HAM ) {
	    disp_def->ModeID |= HAM;
	} else if( pi == PIV_AVM ) {
	    disp_def->ModeID |= HIRES;
	} else if ( (pi == PIV_STANDARD) && (pan->PixelSize == 6) ) {
	    disp_def->ModeID |= EXTRA_HALFBRITE;
	}
    }

    // High Resolution guess
    i = SCREEN_WIDTH;
    if ( pan->XSize >= i) {
	if ( !(disp_def->Flags & DISP_BACKGROUND) || (disp_def->Flags & DISP_XLMODEID) )
	    disp_def->ModeID |= HIRES;
    } else {
	i >>= 1;
    }

    if ( pan->XSize > i)
	disp_def->Flags |= DISP_OVERSCANX;

    i = ht;

    // InterLace guess
    if ( pan->YSize >= i ) {
	if ( !(disp_def->Flags & DISP_BACKGROUND) || (disp_def->Flags & DISP_XLMODEID) )
	    disp_def->ModeID |= LACE;
    } else {
	i >>= 1;
    }

    if ( pan->YSize > i)
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
     * Tag that tells us to force a HAM/NONHAM display.
     */
    if ( ti = FindTagItem( XLTAG_HAM, inti ) ) {
	if ( ti->ti_Data ) {
	    disp_def->ModeID |= HAM;
	    disp_def->ModeID &= ~EXTRA_HALFBRITE;
	} else {
	    disp_def->ModeID &= ~HAM;
	}
    }

    /*
     * Tag that tells us to force a NONHAM/HAM display.
     */
    if ( ti = FindTagItem( XLTAG_NONHAM, inti ) ) {
	if ( ti->ti_Data ) {
	    disp_def->ModeID &= ~HAM;
	} else {
	    disp_def->ModeID |= HAM;
	    disp_def->ModeID &= ~EXTRA_HALFBRITE;
	}
    }

    /*
     * Tag that tells us to force a EHB/NONEHB display.
     */
    if ( ti = FindTagItem( XLTAG_EHB, inti ) ) {
	if ( ti->ti_Data ) {
	    disp_def->ModeID |= EXTRA_HALFBRITE;
	    disp_def->ModeID &= ~HAM;
	} else {
	    disp_def->ModeID &= ~EXTRA_HALFBRITE;
	}
    }

    /*
     * Tag that tells us to force a NONEHB/EHB display.
     */
    if ( ti = FindTagItem( XLTAG_NONEHB, inti ) ) {
	if ( ti->ti_Data ) {
	    disp_def->ModeID &= ~EXTRA_HALFBRITE;
	} else {
	    disp_def->ModeID |= EXTRA_HALFBRITE;
	    disp_def->ModeID &= ~HAM;
	}
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

	/*
	 * Tag that tells us to force an NTSC display.
	 */
	if ( (ti = FindTagItem( XLTAG_NTSC, inti ))
	  && ti->ti_Data ) {
		disp_def->ModeID &= ~MONITOR_ID_MASK;
		disp_def->ModeID |= NTSC_MONITOR_ID;
	}

	/*
	 * Tag that tells us to force a PAL display.
	 */
	if ( (ti = FindTagItem( XLTAG_PAL, inti ))
	  && ti->ti_Data ) {
		disp_def->ModeID &= ~MONITOR_ID_MASK;
		disp_def->ModeID |= PAL_MONITOR_ID;
	}

	/*
	 * Tag that tells us to force a DEFAULT display.
	 */
	if ( (ti = FindTagItem( XLTAG_DEFMON, inti ))
	  && ti->ti_Data ) {
		disp_def->ModeID &= ~MONITOR_ID_MASK;
	}

    disp_def->NominalWidth = SCREEN_WIDTH;
    disp_def->NominalHeight = ht;

    if ( !(disp_def->Flags & DISP_BACKGROUND) ) {
	disp_def->Width = pan->XSize;
	disp_def->Height = pan->YSize;
	D(PRINTF("1 Setting disp_def->Height= %ld\n",disp_def->Height);)
    }

    if ( disp_def->Flags & DISP_OVERSCAN ) {

	QueryOverscan( disp_def->ModeID, &drect, OSCAN_MAX );

	if ( disp_def->Flags & DISP_OVERSCANX ) {
	    D(PRINTF("Pan2Display() DISP_OVERSCANX\n");)
	    disp_def->NominalWidth = (drect.MaxX - drect.MinX) + 1;
	    disp_def->Left = drect.MinX + (disp_def->NominalWidth - disp_def->Width);

	} else if ( !(disp_def->ModeID & HIRES) ) {
	    disp_def->NominalWidth >>= 1;
	}

	if ( disp_def->Flags & DISP_OVERSCANY ) {
	    D(PRINTF("Pan2Display() DISP_OVERSCANY\n");)
	    disp_def->NominalHeight = (drect.MaxY - drect.MinY) + 1;
	    disp_def->Top = drect.MinY + (disp_def->NominalHeight - disp_def->Height);

	} else if ( !(disp_def->ModeID & LACE) ) {
	    disp_def->NominalHeight >>= 1;
	}

	D(PRINTF("Pan2Display() DISP_OVERSCAN, NominalWidth= %ld, NominalHeight= %ld\nWidth= %ld, Height= %ld\n MinX= %ld, MaxX= %ld, MinY= %ld, MaxY= %ld\n",
	    disp_def->NominalWidth,disp_def->NominalHeight,
	    disp_def->Width,disp_def->Height,
	    drect.MinX,drect.MaxX,drect.MinY,drect.MaxY);)
    } else {

	if ( !(disp_def->ModeID & HIRES) )
	    disp_def->NominalWidth >>= 1;

	if ( !(disp_def->ModeID & LACE) )
	    disp_def->NominalHeight >>= 1;
    }

    D(PRINTF("Pan2Display() Left= %ld, Top= %ld\n",disp_def->Left,disp_def->Top);)

    if ( disp_def->Flags & DISP_BACKGROUND )
	return;

    /*
     * If we are blitting from a buffer to our display, open the display
     * full width. Else open a display only as big as the CDXL.
     */
    if ( flags & CDXL_BLIT ) {
	disp_def->Width = max(disp_def->NominalWidth,pan->XSize);
	disp_def->Height = max(disp_def->NominalHeight,pan->YSize);
/*
	if ( disp_def->ModeID & LORESSDBL_KEY ) {
	    disp_def->Height = max( disp_def->height, pan->YSize << 1 );
	}
*/
	D(PRINTF("2 Setting disp_def->Height= %ld\n",disp_def->Height);)
    } else {
	disp_def->Width = pan->XSize;
	disp_def->Height = pan->YSize;
	D(PRINTF("3 Setting disp_def->Height= %ld\n",disp_def->Height);)
    }


} // Pan2Display


/*
 * clipit - passed width and height of a display, and the text, std, and
 *          max overscan rectangles for the mode, clipit fills in the
 *          spos (screen pos) and dclip rectangles to use in centering.
 *          Centered around smallest containing user-editable oscan pref,
 *          with dclip confined to legal maxoscan limits.
 *          Screens which center such that their top is below text
 *          oscan top, will be moved up.
 *          If a non-null uclip is passed, that clip is used instead.
 */
void clipit(SHORT wide, SHORT high,
	    struct Rectangle *spos, struct Rectangle *dclip,
	    struct Rectangle *txto, struct Rectangle *stdo,
	    struct Rectangle *maxo, struct Rectangle *uclip)
{
struct  Rectangle *besto;
SHORT	minx, maxx, miny, maxy;
SHORT	txtw, txth, stdw, stdh, maxw, maxh, bestw, besth;

    /* get the txt, std and max widths and heights */
    txtw = txto->MaxX - txto->MinX + 1;
    txth = txto->MaxY - txto->MinY + 1;
    stdw = stdo->MaxX - stdo->MinX + 1;
    stdh = stdo->MaxY - stdo->MinY + 1;
    maxw = maxo->MaxX - maxo->MinX + 1;
    maxh = maxo->MaxY - maxo->MinY + 1;

    if((wide <= txtw)&&(high <= txth))
	{
	besto = txto;
	bestw = txtw;
	besth = txth;

	D(bug("Best clip is txto\n"));
	}
    else
	{
	besto = stdo;
	bestw = stdw;
	besth = stdh;

	D(bug("Best clip is stdo\n"));
	}

    D(bug("TXTO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		txto->MinX,txto->MinY,txto->MaxX,txto->MaxY,txtw,txth));
    D(bug("STDO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		stdo->MinX,stdo->MinY,stdo->MaxX,stdo->MaxY,stdw,stdh));
    D(bug("MAXO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  maxw=%ld maxh=%ld\n",
		maxo->MinX,maxo->MinY,maxo->MaxX,maxo->MaxY,maxw,maxh));

    if(uclip)
	{
	*dclip = *uclip;
    	spos->MinX = uclip->MinX;
	spos->MinY = uclip->MinY;

	D(bug("UCLIP: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
    else
	{
	/* CENTER the screen based on best oscan prefs
 	* but confine dclip within max oscan limits
 	*
 	* FIX MinX first */
	spos->MinX = minx = besto->MinX - ((wide - bestw) >> 1);
	maxx = wide + minx - 1;
	if(maxx > maxo->MaxX)  maxx = maxo->MaxX;	/* too right */
	if(minx < maxo->MinX)  minx = maxo->MinX;	/* too left  */

	D(bug("DCLIP: minx adjust from %ld to %ld\n",spos->MinX,minx));

	/* FIX MinY */
	spos->MinY = miny = besto->MinY - ((high - besth) >> 1);
	/* if lower than top of txto, move up */
	spos->MinY = miny = min(spos->MinY,txto->MinY);
	maxy = high + miny - 1;
	if(maxy > maxo->MaxY)  maxy = maxo->MaxY;	/* too down  */
	if(miny < maxo->MinY)  miny = maxo->MinY;	/* too up    */

	D(bug("DCLIP: miny adjust from %ld to %ld\n",spos->MinY,miny));

	/* SET up dclip */
	dclip->MinX = minx;
	dclip->MinY = miny;
	dclip->MaxX = maxx;
	dclip->MaxY = maxy;

	D(bug("CENTER: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
}


VOID
CenterDisplay( DISP_DEF * disp_def )
{
    int			i,ht = NTSC_HEIGHT;
    struct Rectangle	drect;
    struct DisplayInfo	disp;

    if ( disp_def->Flags & DISP_BACKGROUND ) {
	struct Rectangle	txto,stdo,maxo,spos,dclip;

	QueryOverscan( disp_def->ModeID, &txto, OSCAN_TEXT );
	QueryOverscan( disp_def->ModeID, &stdo, OSCAN_STANDARD );
	QueryOverscan( disp_def->ModeID, &maxo, OSCAN_MAX );

	clipit( disp_def->Width, disp_def->Height, &spos, &dclip, &txto, &stdo,
	    &maxo,NULL);

	D(PRINTF("spos.MinX= %ld, MinY= %ld\n",spos.MinX,spos.MinY);)

	disp_def->Left = spos.MinX;
	disp_def->Top = spos.MinY;

	return;
    }


    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    ht = PAL_HEIGHT;
    }

    // High Resolution guess
    i = SCREEN_WIDTH;
    if ( !( (disp_def->Width >= i) || (disp_def->ModeID & HIRES) ) ) {
	i >>= 1;
    }

    if ( disp_def->Width > i)
	disp_def->Flags |= DISP_OVERSCANX;

    i = ht;

    // InterLace guess
    if ( !( (disp_def->Height >= i) || (disp_def->ModeID & LACE) ) ) {
	i >>= 1;
    }

    if ( disp_def->Height > i)
	disp_def->Flags |= DISP_OVERSCANY;

    if ( disp_def->Flags & DISP_OVERSCAN ) {

	QueryOverscan( disp_def->ModeID, &drect, OSCAN_MAX );

	if ( disp_def->Flags & DISP_OVERSCANX ) {
	    D(PRINTF("CenterDisplay() DISP_OVERSCANX\n");)
	    disp_def->NominalWidth = (drect.MaxX - drect.MinX) + 1;
	    disp_def->Left = drect.MinX + (disp_def->NominalWidth - disp_def->Width);
	}

	if ( disp_def->Flags & DISP_OVERSCANY ) {
	    D(PRINTF("CenterDisplay() DISP_OVERSCANY\n");)
	    disp_def->NominalHeight = (drect.MaxY - drect.MinY) + 1;
	    disp_def->Top = drect.MinY + (disp_def->NominalHeight - disp_def->Height);
	}

	D(PRINTF("CenterDisplay() DISP_OVERSCAN, NominalWidth= %ld, NominalHeight= %ld\nWidth= %ld, Height= %ld\n MinX= %ld, MaxX= %ld, MinY= %ld, MaxY= %ld\n",
	    disp_def->NominalWidth,disp_def->NominalHeight,
	    disp_def->Width,disp_def->Height,
	    drect.MinX,drect.MaxX,drect.MinY,drect.MaxY);)
    }

    D(PRINTF("CenterDisplay() Left= %ld, Top= %ld\n",disp_def->Left,disp_def->Top);)

} // CenterDisplay


/*
 * Free the CDXLOB and its buffers
 */
VOID
CDXLFreeOb( CDXLOB * CDXL_ob )
{
    int i;

    if ( xlfile ) {
	CloseAsyncXL( xlfile, CDXL_ob );
	xlfile = NULL;
    }

    if( CDXL_ob ) {
	for ( i = 0; i < 2; i++ ) {
	    if( CDXL_ob->Buffer[i] )
		FreeMem( CDXL_ob->Buffer[i], CDXL_ob->BufSize );
	}

	FreeMem( CDXL_ob, sizeof( CDXLOB ) );
    }

} // CDXLFreeOb();


/*
 * Allocate a CDXLOB and its buffers and do setup.
 */
CDXLObtain( UBYTE * CDXLFile, DISP_DEF * disp_def, CDXLOB **CDXLob, ULONG flags, struct TagItem * inti )
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
    WORD		      cmap[MAX_CMAP];
    PAN			      pan;
    BOOL		      sdbl;

    *CDXLob = CDXL_ob = NULL;

    if( !(CDXLFile ) )
	return( RC_MISSING_FILE );

    File = NULL;

    if ( ! ( fsLock = Lock( CDXLFile, ACCESS_READ ) ) ) {
	printf("Could not Lock '%ls'\n",CDXLFile);
	kprintf("Could not Lock '%ls'\n",CDXLFile);
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
	kprintf("Could not open '%ls'\n",CDXLFile);
	ret = RC_CANT_FIND;
	goto error;
    }

    // Get the first PAN
    if ( Read( File, &pan, PAN_SIZE ) != PAN_SIZE ) {
	ret = RC_READ_ERROR;
	goto error;
    }

    if( pan.Type != PAN_STANDARD ) {
	ret = RC_BAD_PAN;
	goto error;
    }

    if( Read( File, cmap, pan.ColorMapSize ) != pan.ColorMapSize ) {
	ret = RC_READ_ERROR;
	goto error;
    }

    /*
     * If we are NOT loading an ILBM for the background, determine
     * what kind of display to open based upon the PAN. Else we
     * determine the display from the ILBM.
     */
//    if ( (disp_def->Flags & DISP_XLMODEID) || !(disp_def->Flags & DISP_BACKGROUND) )
	Pan2Display( &pan, disp_def, flags, inti );

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

    // pan.Reserved holds the READXLSPEED.
    CDXL_ob->ReadXLSpeed = (pan.Reserved+1) * DEFAULT_XLSPEED;

    CDXL_ob->flags = flags;

    CDXL_ob->ImageSize = IMAGE_SIZE( &pan );
    CDXL_ob->PlaneSize = PLANE_SIZE( &pan );
//    CDXL_ob->FrameSize = FRAME_SIZE( &pan );
    CDXL_ob->FrameSize = pan.Size;	// To account for padding. W.D.L 930811
    CDXL_ob->CMapSize = CMAP_SIZE( &pan );
    CDXL_ob->AudioSize = pan.AudioSize;

    CDXL_ob->BufSize = CDXL_ob->FrameSize;

    CDXL_ob->StartSector = CDXLSector;
    CDXL_ob->NumFrames = ( Size / CDXL_ob->FrameSize );


    if ( (flags & CDXL_DOSXL) && !(xlfile = OpenAsyncXL( CDXLFile, CDXL_ob, 0 ) ) ) {
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
	    CDXL_ob->audio[j] = (UBYTE *)CDXL_ob->Buffer[j] + PAN_SIZE + CMAP_SIZE( &pan ) + IMAGE_SIZE( &pan );
	}

	/*
	 * Point the CMAP to the correct offset into the buffer.
	 */
	CDXL_ob->CMap[j] = (UWORD *)(CDXL_ob->Buffer[j] + PAN_SIZE);
	CopyMem( cmap, CDXL_ob->CMap[j], pan.ColorMapSize );

	/*
	 * Since the bitmap MUST be the same size as the CDXL,
	 * AllocBitMap() can NOT be used as it will round the width
	 * up according to the restrictions of the OS.
	 */
	InitBitMap(&CDXL_ob->bm[j],pan.PixelSize,pan.XSize,pan.YSize);

	ImageStart = PAN_SIZE + CMAP_SIZE( &pan );

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

    D(PRINTF("pan.YSize= %ld\n",pan.YSize);)

    D(PRINTF("\n-----------------------\nBuffer[0]= %ld, Video[0]= %ld, ImageStart= %ld\n,CMap[0]= %ld, CMapSize= %ld",
	CDXL_ob->Buffer[0],CDXL_ob->Video[0],ImageStart,CDXL_ob->CMap[0],CDXL_ob->CMapSize);)


    Read( File, CDXL_ob->bm[0].Planes[0], CDXL_ob->ImageSize );
    BltBitMap( &CDXL_ob->bm[0],0,0,&CDXL_ob->bm[1],0,0,pan.XSize,pan.YSize,0xC0,0xFF,NULL);
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
	D(PRINTF("SDBL\n");)
    } else {
	sdbl = FALSE;
	D(PRINTF("NON SDBL\n");)
    }

    /*
     * If a left value is specified use it. Else center.
     */
    if ( ti = FindTagItem( XLTAG_Left, inti )  ) {
	CDXL_ob->rect.MinX = ti->ti_Data;
    } else {
	// Center it;
	CDXL_ob->rect.MinX = (max(disp_def->NominalWidth,disp_def->Width) >> 1) - (pan.XSize >> 1);
    }

    /*
     * If a top value is specified use it. Else center.
     */
    if ( ti = FindTagItem( XLTAG_Top, inti )  ) {
	CDXL_ob->rect.MinY = ti->ti_Data;
    } else {
	// Center it;
	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) - (sdbl ? (pan.YSize << 1) : pan.YSize) )/2;
//	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) >> 1) - (sdbl ? pan.YSize : (pan.YSize >> 1));
//	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) >> (sdbl ? 2 : 1)) - pan.YSize;
	D(PRINTF("1 MinY = %ld, Ht= %ld, Ht>>1= %ld, YSize= %ld, YSize>>1= %ld\n",
	    CDXL_ob->rect.MinY,max(disp_def->NominalHeight,disp_def->Height),
		(max(disp_def->NominalHeight,disp_def->Height) >> 1),
		pan.YSize,(pan.YSize >> 1));)

	D(PRINTF("disp_def->Heigh= %ld, disp_def->NominalHeight= %ld\nmax(disp_def->NominalHeight,disp_def->Height)= %ld,(pan.YSize << 1)= %ld\n",
	    disp_def->Height,disp_def->NominalHeight,
	    max(disp_def->NominalHeight,disp_def->Height),
	    (pan.YSize << 1) );)

	D(PRINTF("Miny= %ld - %ld\n",
	    max(disp_def->NominalHeight,disp_def->Height),((pan.YSize << 1)/2) );)

    }



    // Clip it.
    CDXL_ob->rect.MinX = max(0,min( CDXL_ob->rect.MinX, disp_def->Width ));
    CDXL_ob->rect.MinY = max(0,min( CDXL_ob->rect.MinY, disp_def->Height ));

/*
    // If display will be scan doubled
    if ( sdbl ) {
	D(PRINTF("SDBL... Before MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
	CDXL_ob->rect.MinY >>= 2;
	disp_def->Top >>= 2;
	D(PRINTF("AFTER MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
    }
*/

    CDXL_ob->rect.MaxX = pan.XSize + CDXL_ob->rect.MinX;
    CDXL_ob->rect.MaxY = pan.YSize + CDXL_ob->rect.MinY;

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

    D(PRINTF("CDXLObtain... END MinY= %ld, Top= %ld, Height= %ld, YSize= %ld\n",
	CDXL_ob->rect.MinY,disp_def->Top,disp_def->Height,pan.YSize);)

    D(PRINTF("PAN_SIZE= %ld, CMapSize= %ld, ImageSize= %ld, AudioSize= %ld, FrameSize= %ld\n",
	PAN_SIZE,CDXL_ob->CMapSize,CDXL_ob->ImageSize,CDXL_ob->AudioSize,CDXL_ob->FrameSize);)

    Delay( 14 ); // Delay for cd.device prefetch doublespeed bug.

    return( RC_OK );

error:

    if ( xlfile ) {
	CloseAsyncXL( xlfile, CDXL_ob );
	xlfile = NULL;
    }

    D(PRINTF("CDXLObtain error ret= %ld\n",ret);)

    if( File )
	Close( File );

    if ( fsLock )
	UnLock( fsLock );

    CDXLFreeOb( CDXL_ob );

    return( ret );

} /* CDXLObtain() */


/*
 * Close cd/cdtv.device, depending upon which one was opened.
 */
VOID
CDDeviceTerm( CDXLOB * CDXL_ob )
{
    if ( CDDeviceMReq ) {
	if ( CDDevice ) {
	    if ( CDXL_ob && !(CDXL_ob->flags & CDTV_DEVICE) ) {

		// Reset CDConfig to how we found it.
		CDConfig(TAGCD_READXLSPEED,saveinfo.ReadXLSpeed,
			 TAGCD_XLECC,saveinfo.XLECC,
			 TAG_END );
	    }

	    CloseDevice( (struct IORequest *)CDDeviceMReq );
	    CDDevice = NULL;
	}


	if ( CDPort ) {
	    while( GetMsg( CDPort ) );
	}

	DeleteStdIO( CDDeviceMReq );
	CDDeviceMReq = NULL;
    }

    if ( CDPort ) {
	DeleteMsgPort( CDPort );
	CDPort = NULL;
    }

    CDPortSignal = -1;

} // CDDeviceTerm()


/*
 * Attempts to open CD/CDTV.device if not already opened.
 * Tries to open cd.device first, if not successful, tries for cdtv.device
 * Returns:
 *	retcode: (BOOL) reflects device's open state.
 *  *opened: (BOOL) TRUE if opened by this call.
 */
BOOL
CDDeviceInit( ULONG * opened, CDXLOB * CDXL_ob )
{
    UBYTE    * device_name[] = { CD_NAME, CDTV_NAME };
    int	      i;

    if ( opened )
	*opened = FALSE;

    if ( !CDDevice ) {
	D(PRINTF("CDDeviceInit() have to prep CDDevice!");)

	if ( CDPort = CreateMsgPort() ) {
	    D(PRINTF("CDDeviceInit() GOT CDPort\n");)
	    if ( CDDeviceMReq = CreateStdIO( CDPort ) ) {
		D(PRINTF("CDDeviceInit() GOT CDDeviceMReq\n");)
		// Try to open cd.device. If can't, try for cdtv.device.

		i = (CDXL_ob->flags & CDTV_DEVICE) ? 1 : 0;

		for ( ; i < 2; i++ ) {
		    if ( !OpenDevice( device_name[i], 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
			D(PRINTF("CDDeviceInit() Got a Device\n");)
			CDDevice = CDDeviceMReq->io_Device;
			break;
		    }
		}
	    }
	}

	if ( !CDDevice ) {
	    printf("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
	    CDPort, CDDeviceMReq, CDDevice );
	    kprintf("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
	    CDPort, CDDeviceMReq, CDDevice );

	    CDDeviceTerm( CDXL_ob );
	    return( FALSE );
	}

	CDPortSignal = ( 1 << CDPort->mp_SigBit );
	if ( opened )
	    *opened = TRUE;
    }

    // If we have cd.device as opposed to cdtv.device
    if ( i == 0 ) {
	D(PRINTF("GOT cd.device\n");)
	GetCDInfo( &cdinfo );
	saveinfo = cdinfo;	// structure copy

	CDXL_ob->NumSectors = CDXL_ob->FibSize / cdinfo.SectorSize;

	D(PRINTF("1 CDXL_ob->ReadXLSpeed= %ld, cdinfo.ReadXLSpeed= %ld\n",
	    CDXL_ob->ReadXLSpeed,cdinfo.ReadXLSpeed );)

	if ( CDXL_ob->ReadXLSpeed !=  cdinfo.ReadXLSpeed ) {
	    if ( CDXL_ob->ReadXLSpeed > cdinfo.MaxSpeed )
		CDXL_ob->ReadXLSpeed = cdinfo.MaxSpeed;

	    CDConfig( TAGCD_READXLSPEED, CDXL_ob->ReadXLSpeed, TAG_END );
	    CDXL_ob->ReadXLSpeed = cdinfo.ReadXLSpeed;

	    D(PRINTF("2 CDXL_ob->ReadXLSpeed= %ld, cdinfo.ReadXLSpeed= %ld\n",
		CDXL_ob->ReadXLSpeed,cdinfo.ReadXLSpeed );)

	}

	// Set Errror Correction
	CDConfig( TAGCD_XLECC, (CDXL_ob->flags & CDXL_XLEEC) ? 1 : 0, TAG_END );

    } else {
	D(PRINTF("GOT cdtv.device\n");)
	// Indicate that we are using cdtv.device
	CDXL_ob->flags |= CDTV_DEVICE;
	CDXL_ob->NumSectors = CDXL_ob->FibSize / DEFAULT_SECTOR_SIZE;
    }

    return( TRUE );	

} // CDDeviceInit()


/*
 * Free the list of struct CDXL or CDTV_CDXL, depending upon which device we opened.
 */
VOID
FreeXList( struct MinList * xllist, CDXLOB * CDXL_ob )
{
    struct CDXL * xl;

    if ( xllist->mlh_Head ) {
	xllist->mlh_Head->mln_Pred = Pred;
	Pred = NULL;
    }

    if ( xllist->mlh_TailPred ) {
	xllist->mlh_TailPred->mln_Succ = Succ;
	Succ = NULL;
    }


    while ( xl = (struct CDXL *)RemHead( (struct List *)xllist) ) {
	FreeMem( xl, (CDXL_ob->flags & CDTV_DEVICE) ? sizeof( CDTV_CDXL ) : sizeof( struct CDXL ) );
    }

} /* FreeXList() */


/*
 * Shut down everything CDXL related.
 */
VOID
CDXLTerm( CDXLOB * CDXL_ob )
{

    // Terminate the audio.
    QuitAudio();

    if( CDDeviceMReq ) {
	// This correctly terminates a CDXL
	if ( !CheckIO( (struct IORequest *) CDDeviceMReq ) ) {
	    D(PRINTF("CDXLTerm() Calling AbortIO() ior= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx, ln_Type= 0x%lx\n",
		CDDeviceMReq,CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,CDDeviceMReq->io_Message.mn_Node.ln_Type);)
	    AbortIO( (struct IORequest *) CDDeviceMReq );
	}

	D(PRINTF("CDXLTerm() Calling WaitIO() ior= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx, ln_Type= 0x%lx\n",
	    CDDeviceMReq,CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,CDDeviceMReq->io_Message.mn_Node.ln_Type);)

	WaitIO(  (struct IORequest *) CDDeviceMReq );

	D(PRINTF("CDXLTerm() After WaitIO() ior= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx, ln_Type= 0x%lx\n",
	    CDDeviceMReq,CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,CDDeviceMReq->io_Message.mn_Node.ln_Type);)

/*	Removed 930831
	// Then a SEEK is required to force the drive to stop (sometimes necessary)
	CDDeviceMReq->io_Command = CD_SEEK;
	CDDeviceMReq->io_Offset  = 0;
	CDDeviceMReq->io_Length  = 0;
	CDDeviceMReq->io_Data    = NULL;

	SendIO( (struct IORequest *) CDDeviceMReq );
	WaitIO(  (struct IORequest *) CDDeviceMReq );
*/
	CDDeviceTerm( CDXL_ob );
    }

    if(	XList.mlh_Head ) {
	FreeXList( &XList, CDXL_ob );
    }

    if( XLSignalBit != -1 ) {
	FreeSignal( XLSignalBit );
	XLSignalBit = -1;
    }
    XLSignal =  -1;

    CDXLFreeOb( CDXL_ob );

} /* CDXLTerm() */


/*
 *
 *  CDXLCallBack -- XL Callback function (runs as an interrupt)
 *
 */
__interrupt __asm __saveds
CDXLCallBack( register __a1 APTR intdata, register __a2 struct CDXL * xl )
{
    if ( CDXL_OB->AudioSize ) {
	// disable AUD0 interrupt
	custom.intreq = INTF_AUD0;

	CDXL_OB->curVideo ^= 1;
	Count++;

	if ( Count==1 ) {
	    StartAudio();
	} else {
	    // enable AUD0 interrupt
	    custom.intena = INTF_SETCLR|INTF_AUD0;
	}

    } else {
	CDXL_OB->curVideo ^= 1;
	Count++;
    }

    Signal( (struct Task *)parent, XLSignal );

    return( 0 );

} /* CDXLCallBack() */


/*
 *  SendIOR -- asynchronously execute a device command
 */
BOOL
SendIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data   = data;

    if ( (cmd == CDTV_READXL) || (cmd == CD_READXL) ) {
	D(PRINTF("SendIOR() cmd= %ld io_Offset= %ld, io_Length= %ld\n",
	    req->io_Command,req->io_Offset,req->io_Length);)
    }

    SendIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	printf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
	kprintf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()


/*
 * Call this if cdtv.device was opened as opposed to cd.device.
 */
NewCDTV_XL( struct MinList * first, UBYTE * buf, ULONG len, PF code )
{
    CDTV_CDXL * xl;

    if( !(xl = AllocMem( sizeof( CDTV_CDXL ) , MEMF_CLEAR ) ) )
	return( RC_NO_MEM );

    xl->Buffer = (char *)buf;
    xl->Length = len;
    xl->DoneFunc = (VOID(* )())code;

    AddTail( (struct List *)first, (struct Node *)xl );

    return( RC_OK );

} // NewCDTV_XL()


/*
 * Call this if cd.device was opened as opposed to cdtv.device.
 */
NewCD_XL( struct MinList * first, UBYTE * buf, ULONG len, PF code )
{
    struct CDXL * xl;

    if( !(xl = AllocMem( sizeof( struct CDXL ) , MEMF_CLEAR ) ) )
	return( RC_NO_MEM );

    xl->Buffer = (char *)buf;
    xl->Length = len;
    xl->IntCode = (VOID(* )())code;
    xl->IntData = NULL;

    D(PRINTF("NewCD_XL xl= 0x%lx\n",xl);)

    AddTail( (struct List *)first, (struct Node *)xl );

    return( RC_OK );

} // NewCD_XL()


BOOL
ButtonAbort( ULONG flags )
{

    if ( ((flags & CDXL_LMBABORT) && LMBDown() ) ||
	 ((flags & CDXL_RMBABORT) && RMBDown() ) ||
	 ((flags & CDXL_FIREABORT) && FireDown() ) ) {

	return( TRUE );
    }

    return( FALSE );

} // ButtonAbort()

#define S(MASK) (MASK & state )

BOOL
ReadPort( ULONG flags )
{
    ULONG    state;

    if ( flags & (CDXL_LMBABORT|CDXL_RMBABORT) )
	state = ReadJoyPort( 0 );

    if ( flags & CDXL_LMBABORT ) {
	if ( (JP_TYPE_MASK & state) == JP_TYPE_MOUSE) {
	    if (S(JPF_BTN2))
		return( TRUE );
	}
    }

    if ( flags & CDXL_RMBABORT ) {
	if ( (JP_TYPE_MASK & state) == JP_TYPE_MOUSE) {
	    if (S(JPF_BTN1))
		return( TRUE );
	}
    }

    if ( flags & CDXL_FIREABORT ) {
	state = ReadJoyPort( 1 );

	if ( ((JP_TYPE_MASK & state) == JP_TYPE_GAMECTLR) || ((JP_TYPE_MASK & state) == JP_TYPE_JOYSTK) ) {
	    if (S(JPF_BTN2))
		return( TRUE );
	}
    }

    return( FALSE );

} // ReadPort()


/*
 * Conatins the main loop which handles the CDXL playback.
 */
PlayCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    LONG	LastFrame,NumColors;
    ULONG	SignalMask,DBufSignal;
    ULONG	Signal;
#ifdef	DO_REBOOT // [
    ULONG	copcount;
#endif		// ]
    SHORT	curbm,curvid,DstX,DstY,Wid,Ht;
    BOOL	Need2Change,Safe2Change;
    BOOL	(* babort)( ULONG );
    int		ret = RC_OK,oldpri;

    /*
     * The signal for the CDXL interrupt to ping us with.
     */
    if ( ( XLSignalBit = AllocSignal( -1 ) ) == -1 ) {
	printf("PlayCDXL could NOT alloc XLSignalBit\n");
	kprintf("PlayCDXL could NOT alloc XLSignalBit\n");
	return( RC_NO_MEM );
    }

    XLSignal = ( 1 << XLSignalBit );

    if ( LowLevelBase ) {
	babort = ReadPort;
    } else {
	babort = ButtonAbort;
    }
    /*
     * The signal from the copper interrupt telling us to call ChangeVPBitMap().
     */
    DBufSignal = CopSignal;

    curvid = 0;
    curbm = 1;
    Count = 0;
    Need2Change = Safe2Change = FALSE;
    NumColors = min(CDXL_ob->CMapSize >> 1,disp_def->vp->ColorMap->Count);
    LastFrame = CDXL_ob->NumFrames-1;

    /*
     * Wait for signals from:
     *	the CDXL interrupt (XLSignal),
     *	the user telling us to abort (CDXL_ob->KillSig),
     *	the device telling us something is wrong (CDPortSignal),
     *	the copper interrupt telling us to Call ChangeVPBitMap() (DBufSignal).
     */
    SignalMask = ( XLSignal|CDXL_ob->KillSig|CDPortSignal|DBufSignal);

    DstX = CDXL_ob->rect.MinX;
    DstY = (disp_def->ModeID & LORESSDBL_KEY) ? 
	(CDXL_ob->rect.MinY>>1) : CDXL_ob->rect.MinY;

    Wid = (CDXL_ob->rect.MaxX - CDXL_ob->rect.MinX) - CDXL_ob->xoff;
    Ht = (CDXL_ob->rect.MaxY - CDXL_ob->rect.MinY) - CDXL_ob->yoff;

    D(PRINTF("DstX= %ld, DstY= %ld, Wid= %ld, Ht= %ld\n",
	DstX,DstY,Wid,Ht);)

    CDXL_OB = CDXL_ob;	// Set the Global ptr

    oldpri = SetTaskPri( FindTask( NULL ), 50L );

    CDXL_ob->flags |= CDXL_PLAYING;

    D(PRINTF("PlayCDXL() Count= %ld, LastFrame= %ld\n",Count,LastFrame);)
    D(PRINTF("PlayCDXL() Count= %ld, LastFrame= %ld\n",Count,LastFrame);)

    do { 

	D(PRINTF("TOP loops= %ld\n",CDXL_ob->loops);)

	SetSignal( 0, SignalMask );
	curvid = 0;
	curbm = 1;
	Count = 0;
	Need2Change = Safe2Change = FALSE;
	CDXL_ob->curVideo = CDXL_ob->curAudio = 0;
	/*
	 * Start the CDXL. Things are done differently depending upon whether
	 * we have cd.device opened or cdtv.device.
	 */
	if ( CDXL_ob->flags & CDTV_DEVICE ) {
	    SendIOR(CDDeviceMReq,CDTV_READXL,CDXL_ob->StartSector,CDXL_ob->NumSectors,&XList);
//	    SendIOR(CDDeviceMReq,CDTV_READXL,CDXL_ob->StartSector,CDXL_ob->NumSectors,XList.mlh_Head);
	} else {
	    SendIOR(CDDeviceMReq,CD_READXL,CDXL_ob->StartSector * cdinfo.SectorSize,CDXL_ob->NumSectors * cdinfo.SectorSize,&XList);
//	    SendIOR(CDDeviceMReq,CD_READXL,CDXL_ob->StartSector * cdinfo.SectorSize,CDXL_ob->NumSectors * cdinfo.SectorSize,XList.mlh_Head);
	}

    /*
     * Our main CDXL loop. Continue until we get a kill signal from the user,
     * (CDXL_ob->KillSig) or from the device (CDPortSignal), or until we reach
     * the last frame.
     */
#ifdef	DO_REBOOT // [
	copcount = 0;
#endif		// ]
	while ( 1 /*Count < LastFrame*/ ) {

	    if ( CDXL_ob->flags & CDXL_BUTTONABORTMASK ) {
		if ( babort( CDXL_ob->flags ) ) {
		    CDXL_ob->loops = 1;
		    ret = RC_ABORTED;
		    D(PRINTF("PlayCDXL BUTTON aborting\n");)
		    D(PRINTF("PlayCDXL BUTTON aborting\n");)
		    break;
		}
	    }

	    Signal = Wait( SignalMask );
#ifndef	OUTT	// [
	    if ( Signal & CDXL_ob->KillSig ) {
		D(PRINTF("// Got a kill signal. Count= %ld, LastFrame= %ld\n",
		    Count,LastFrame);)
		CDXL_ob->loops = 1;
		ret = RC_ABORTED;
		break;
	    }
	    if ( Signal & CDPortSignal ) {
		D(PRINTF("// Got a Reply signal from cd.device. Count= %ld, LastFrame= %ld\n",
		    Count,LastFrame);)
		break;
	    }

#else		// ][
	    if ( Signal & (CDXL_ob->KillSig|CDPortSignal) ) {
		D(PRINTF("// Got a kill signal. Signal= 0x%lx, CDPortSignal= 0x%lx\n",
		    Signal,CDPortSignal);)
		CDXL_ob->loops = 1;
		ret = RC_ABORTED;
		break;
	    }
#endif		// ]
	    /*
	     * Recieved a signal from our CDXL interrupt telling us that it
	     * has finished with one of our transfer lists.
	     */
	    if ( Signal & XLSignal ) {
#ifdef	DO_REBOOT // [
		copcount = 0;
#endif		// ]

		if ( CDXL_ob->flags & CDXL_BLIT ) {

		    BltBitMap( &CDXL_ob->bm[curvid],0,0,disp_def->bm[curbm],
			DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);

		    curvid = CDXL_ob->curVideo;

		} else {
		    curvid = curbm = CDXL_ob->curVideo;
		}
		Need2Change = TRUE;
	    }

	    /*
	     * Its time to call ChangeVPBitMap() 
	     */
	    if( (Signal & DBufSignal) || Safe2Change ) {
#ifdef	DO_REBOOT // [
		copcount++;
		if ( copcount > (60*15) ) {
		    CDXL_ob->loops = 1;
		    ret = RC_ABORTED;
		    D(PRINTF("Exiting loop\n");)
		    D(PRINTF("cd.device NOT responding... REBOOTING!! copcount= %ld, Count= %ld, LastFrame= %ld\n",
			copcount,Count,LastFrame);)
		    D(PRINTF("ior= 0x%lx, io_Command= 0x%lx, io_Flags= 0x%lx, io_Error= 0x%lx\nio_Actual= %ld, io_Length= %ld, io_Data= 0x%lx, io_Offset= %ld, ln_Type= 0x%lx\n",
			CDDeviceMReq,CDDeviceMReq->io_Command,
			CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,
			CDDeviceMReq->io_Actual,CDDeviceMReq->io_Length,
			CDDeviceMReq->io_Data,CDDeviceMReq->io_Offset,
			CDDeviceMReq->io_Message.mn_Node.ln_Type);)

		    ColdReboot();
		    break;
		}
#endif		// ]
		if ( Need2Change ) {
		    WaitBlit();

		    ChangeVPBitMap(disp_def->vp, disp_def->bm[curbm], disp_def->dbuf);

		    if ( CDXL_ob->flags & CDXL_MULTI_PALETTE ) {
			/*
			 * Since the PAN format that we are using only stores
			 * 4 bits per gun, might as well just use LoadRGB4.
			 */
			LoadRGB4( disp_def->vp, CDXL_ob->CMap[curvid^1],NumColors );
		    }
		    Safe2Change = FALSE;
		    Need2Change = FALSE;

		    if ( CDXL_ob->flags & CDXL_BLIT ) {
			curbm ^= 1;
		    }
		} else {
		    Safe2Change = TRUE;
		}
	    }

	}

	if ( Count ) {
	    D(PRINTF("Need2Change= %ld, Safe2Change= %ld\n",Need2Change,Safe2Change);)
	    AudioSignalTask = TRUE;
	    if ( CDXL_ob->AudioSize ) {
		if ( !ret ) {
		    Signal = NULL;
		    while ( !(Signal & (XLSignal|CDXL_ob->KillSig)) )
			Signal = Wait( SignalMask );
		}
		StopAudio();
	    }
	    AudioSignalTask = FALSE;
	}

	if ( (CDXL_ob->loops == -1) || (CDXL_ob->loops-1) ) {
	    D(PRINTF("Calling StopCDXL()!!!!\n");)
	    StopCDXL( CDDeviceMReq );
	}
	D(PRINTF("BOTTOM2 loops= %ld\n",CDXL_ob->loops);)

    } while ( (CDXL_ob->loops == -1) || --CDXL_ob->loops );

    CDXL_ob->flags &= ~CDXL_PLAYING;

    D(PRINTF("END loops= %ld\n",CDXL_ob->loops);)

    SetTaskPri( FindTask( NULL ), oldpri );

    return( ret );

} // PlayCDXL()



/*
 * Conatins the main loop which handles the DOSXL playback.
 */
PlayAsyncXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob, ASYNCXLFILE * xlfile )
{
    LONG	LastFrame,NumColors;
    ULONG	SignalMask,DBufSignal;
    ULONG	Signal;
    SHORT	curbm,curvid,DstX,DstY,Wid,Ht;
    BOOL	Need2Change,Safe2Change;
    BOOL	(* babort)( ULONG );
    LONG	bytesArrived;

    int		ret = RC_OK,oldpri;

    D(PRINTF("PlayAsyncXL() 1\n");)

    /*
     * The signal for the CDXL interrupt to ping us with.
     */
    if ( ( XLSignalBit = AllocSignal( -1 ) ) == -1 ) {
	return( RC_NO_MEM );
    }
    XLSignal = ( 1 << XLSignalBit );

    if ( LowLevelBase ) {
	babort = ReadPort;
    } else {
	babort = ButtonAbort;
    }

    /*
     * The signal from the copper interrupt telling us to call ChangeVPBitMap().
     */
    DBufSignal = CopSignal;

    curvid = 0;
    curbm = 1;
    Count = 0;
    Need2Change = Safe2Change = FALSE;
    NumColors = min(CDXL_ob->CMapSize >> 1,disp_def->vp->ColorMap->Count);
    LastFrame = CDXL_ob->NumFrames;

    /*
     * Wait for signals from:
     *	the CDXL interrupt (XLSignal),
     *	the user telling us to abort (CDXL_ob->KillSig),
     *	the device telling us something is wrong (CDPortSignal),
     *	the copper interrupt telling us to Call ChangeVPBitMap() (DBufSignal).
     */
    SignalMask = ( XLSignal|CDXL_ob->KillSig|CDPortSignal|DBufSignal);

    DstX = CDXL_ob->rect.MinX;
    DstY = (disp_def->ModeID & LORESSDBL_KEY) ? 
	(CDXL_ob->rect.MinY>>1) : CDXL_ob->rect.MinY;

    Wid = (CDXL_ob->rect.MaxX - CDXL_ob->rect.MinX) - CDXL_ob->xoff;
    Ht = (CDXL_ob->rect.MaxY - CDXL_ob->rect.MinY) - CDXL_ob->yoff;

    D(PRINTF("PlayAsyncXL() 2\n");)

    D(PRINTF("PlayAsyncXL() 3 Count= %ld, LastFrame= %ld\n",Count,LastFrame);)

    CDXL_OB = CDXL_ob;	// Set the Global ptr

    oldpri = SetTaskPri( FindTask( NULL ), 50L );
    CDXL_ob->flags |= CDXL_PLAYING;

    do { 

	D(PRINTF("TOP loops= %ld\n",CDXL_ob->loops);)

	// If we are relooping
	if ( Count ) {
//	    WaitPacket( xlfile );
	    Seek(xlfile->af_File,0,OFFSET_BEGINNING);
	}

	SetSignal( 0, SignalMask );
	curvid = 0;
	curbm = 1;
	Count = 0;
	Need2Change = Safe2Change = FALSE;
	CDXL_ob->curVideo = CDXL_ob->curAudio = 0;


	/* ask that the buffer be filled */
	SendAsync(xlfile,xlfile->af_Buffers[CDXL_ob->curVideo]);
	D(PRINTF("PlayAsyncXL() 4\n");)
	WaitPacket( xlfile );
	SendAsync(xlfile,xlfile->af_Buffers[CDXL_ob->curVideo]);

    /*
     * Our main CDXL loop. Continue until we get a kill signal from the user,
     * (CDXL_ob->KillSig) or from the device (CDPortSignal), or until we reach
     * the last frame.
     */
	while ( Count < LastFrame ) {

	    D(PRINTF("PlayAsyncXL() 5\n");)

	    if ( CDXL_ob->flags & CDXL_BUTTONABORTMASK ) {
		if ( babort( CDXL_ob->flags ) ) {
		    CDXL_ob->loops = 1;
		    ret = RC_ABORTED;
		    break;
		}
	    }

	    Signal = Wait( SignalMask );

	    D(PRINTF("PlayAsyncXL() 6\n");)

	    if ( Signal & CDXL_ob->KillSig ) {
		D(PRINTF("// Got a kill signal Signal= 0x%lx, CDXL_ob->KillSig= 0x%lx, CDPortSignal= 0x%lx\n",Signal,CDXL_ob->KillSig,CDPortSignal);)
		CDXL_ob->loops = 1;
		ret = RC_ABORTED;
		break;
	    }

	    /*
	     * Recieved a signal from our CDXL interrupt telling us that it
	     * has finished with one of our transfer lists.
	     */
	    if ( !(CDXL_ob->AudioSize) || (Signal & XLSignal) /*|| !Count*/ ) {
		D(PRINTF("PlayAsyncXL() 7\n");)

		WaitPacket( xlfile );

		bytesArrived = xlfile->af_Packet.sp_Pkt.dp_Res1;
		D(PRINTF("bytesArrived= %ld\n",bytesArrived);)

		if (bytesArrived <= 0) {
		    if (bytesArrived <= 0)
		    {
		        /* error, get out of here */
		        SetIoErr(xlfile->af_Packet.sp_Pkt.dp_Res2);
			CDXL_ob->loops = 1;
			ret = RC_READ_ERROR;
			D(PRINTF("Read Error Count= %ld, LastFrame= %ld\n",
			    Count,LastFrame);)
			break;
		    }
		}

		/* ask that the buffer be filled */
		SendAsync(xlfile,xlfile->af_Buffers[CDXL_ob->curVideo]);


		if ( CDXL_ob->flags & CDXL_BLIT ) {

		    BltBitMap( &CDXL_ob->bm[curvid],0,0,disp_def->bm[curbm],
			DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);

		    curvid = CDXL_ob->curVideo;

		} else {
		    curvid = curbm = CDXL_ob->curVideo;
		}
		Need2Change = TRUE;
	    }

	    /*
	     * Its time to call ChangeVPBitMap() 
	     */
	    if( (Signal & DBufSignal) || Safe2Change ) {
		if ( Need2Change ) {
		    WaitBlit();

		    ChangeVPBitMap(disp_def->vp, disp_def->bm[curbm], disp_def->dbuf);

		    if ( CDXL_ob->flags & CDXL_MULTI_PALETTE ) {
			/*
			 * Since the PAN format that we are using only stores
			 * 4 bits per gun, might as well just use LoadRGB4.
			 */
			LoadRGB4( disp_def->vp, CDXL_ob->CMap[curvid^1],NumColors );
		    }
		    Safe2Change = FALSE;
		    Need2Change = FALSE;

		    if ( CDXL_ob->flags & CDXL_BLIT ) {
			curbm ^= 1;
		    }
		} else {
		    Safe2Change = TRUE;
		}
	    }

	}

	if ( Count ) {
	    D(PRINTF("Need2Change= %ld, Safe2Change= %ld\n",Need2Change,Safe2Change);)
	    WaitPacket( xlfile );
	    if ( CDXL_ob->AudioSize ) {
		if ( !ret ) {
		    Signal = NULL;
		    while ( !(Signal & (XLSignal|CDXL_ob->KillSig)) )
			Signal = Wait( SignalMask );
		}
		StopAudio();
	    }
	}

	D(PRINTF("BOTTOM2 loops= %ld\n",CDXL_ob->loops);)

    } while ( (CDXL_ob->loops == -1) || --CDXL_ob->loops );

    CDXL_ob->flags &= ~CDXL_PLAYING;

    D(PRINTF("END loops= %ld\n",CDXL_ob->loops);)

    SetTaskPri( FindTask( NULL ), oldpri );

    return( ret );

} // PlayAsyncXL()


/*
 * Get ready to start playing the DOSXL.
 */
StartAsyncXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    int			ret;

    D(PRINTF("StartAsyncXL() 1\n");)

    if( !CDXL_ob )
	return( RC_FAILED );

    D(PRINTF("StartAsyncXL() 2\n");)

    if ( CDXL_ob->AudioSize ) {
	if ( ret = InitAudio( CDXL_ob ) )
	    return( ret );
    }

    D(PRINTF("StartAsyncXL() 3\n");)

    SetSignal( 0, CDPortSignal );	// Make sure signal is clear.

    /*
     * Call the main CDXL playing routine.
     */
    ret = PlayAsyncXL( disp_def, CDXL_ob, xlfile );

    D(PRINTF("StartAsyncXL() END ret= %ld\n",ret);)

    return( ret );

} // StartASyncXL()


/*
 * Get ready to start playing the CDXL.
 */
StartCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    int			ret,i;
    PFI			newxl;

    if( !CDXL_ob )
	return( RC_FAILED );

    Pred = Succ = NULL;
    NewList( (struct List *)&XList );

    if( CDXL_ob->flags & CDXL_DOSXL )
	return( StartAsyncXL( disp_def, CDXL_ob ) );

    if ( !CDDeviceInit( NULL, CDXL_ob ) ) {
	printf("StartCDXL CDDeviceInit() FAILED\n");
	kprintf("StartCDXL CDDeviceInit() FAILED\n");
	return( RC_NO_CDDEVICE );
    }

    if ( CDXL_ob->AudioSize ) {
	if ( ret = InitAudio( CDXL_ob ) ) {
	    printf("StartCDXL InitAudio() FAILED\n");
	    kprintf("StartCDXL InitAudio() FAILED\n");
	    return( ret );
	}
    }

    /*
     * We call a different transfer list allocation function depending
     * upon whether we have opened cd.device or cdtv.device.
     */
    if ( CDXL_ob->flags & CDTV_DEVICE ) {
	newxl = NewCDTV_XL;
    } else {
	newxl = NewCD_XL;
    }

    /*
     * We have 2 buffers so we need 2 transfer lists.
     */
    for ( i = 0; i < 2; i++ ) {
	if( newxl( &XList, CDXL_ob->Buffer[i], CDXL_ob->FrameSize,
	    CDXLCallBack )
	) {
	    ret = RC_NO_MEM;
	    printf("StartCDXL newxl FAILED!!\n");
	    kprintf("StartCDXL newxl FAILED!!\n");
	    break;
	}
    }

    if ( !ret )	{

	// Tie list together to give us an endless loop
	Pred = XList.mlh_Head->mln_Pred;
	Succ = XList.mlh_TailPred->mln_Succ;
	XList.mlh_Head->mln_Pred = XList.mlh_TailPred;
	XList.mlh_TailPred->mln_Succ = XList.mlh_Head;

	SetSignal( 0, CDPortSignal );	// Make sure signal is clear.

	/*
	 * Call the main CDXL playing routine.
	 */
	ret = PlayCDXL( disp_def, CDXL_ob );

    }

    return( ret );

} // StartCDXL()


/*
 * Draw a box in color 0 around the CDXL. May be necessary to avoid some
 * HAM problems.
 */
VOID
Boxit( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    struct RastPort rp;
    int				i;

    InitRastPort( &rp );
    SetABPenDrMd( &rp, 0, 0, JAM2 );

    for ( i = 0; i < 2; i++ ) {
	rp.BitMap = disp_def->bm[i];
	Move( &rp, max(CDXL_ob->rect.MinX - 1,0), max(CDXL_ob->rect.MinY - 1,0) );
	Draw( &rp, max(CDXL_ob->rect.MinX - 1,0), min(CDXL_ob->rect.MaxY,disp_def->Height) );
	Draw( &rp, min(CDXL_ob->rect.MaxX,disp_def->Width), min(CDXL_ob->rect.MaxY,disp_def->Height) );
	Draw( &rp, min(CDXL_ob->rect.MaxX,disp_def->Width), max(CDXL_ob->rect.MinY - 1,0) );
	Draw( &rp, max(CDXL_ob->rect.MinX - 1,0), max(CDXL_ob->rect.MinY - 1,0) );
    }

} // Boxit()


/*
 * If XLTAG_MSGPortName is specified, this task will get invoked which
 * will handle commands recieved from a port that is created with the
 * name specified.
 */
STATIC VOID __saveds
MsgPortTask( VOID )
{
    struct Task		* task = FindTask( NULL );
    struct MsgPort	* port;
    UBYTE		* portname = (UBYTE *)task->tc_UserData;
    XLMESSAGE		* xlmsg;
    ULONG		  portsig,signals;
    int			  ret = RC_OK;

    Forbid();

    D(PRINTF("MsgPortTask() ENTERED\n");)

    if ( !(port = CreatePort( portname, 0 )) )
	ret = RC_FAILED;

    D(PRINTF("MsgPortTask() 1 ret= %ld\n",ret);)

    task->tc_UserData = (APTR)ret;

    Signal( (struct Task *)parent, SIGBREAKF_CTRL_E );

    if ( !ret ) {
	Permit();
	D(PRINTF("MsgPortTask() 2\n");)

	portsig = 1 << port->mp_SigBit;

	FOREVER {
	    signals = Wait( portsig|SIGBREAKF_CTRL_C );

	    D(PRINTF("MsgPortTask() 3\n");)

	    if ( signals & portsig ) {
		D(PRINTF("MsgPortTask() 4 portsig\n");)

		while ( xlmsg = (XLMESSAGE *)GetMsg( port )) {
		    D(PRINTF("MsgPortTask() GOT xlmsg\n");)

		    if ( CDXL_OB->flags & CDXL_PLAYING ) {
			xlmsg->status = XLM_STATUS_PLAYING;
		    } else {
			xlmsg->status = NULL;
		    }
		    if ( xlmsg->command == XLM_CMD_ABORT ) {
			xlmsg->status |= XLM_STATUS_ABORTING;
			signals = SIGBREAKF_CTRL_C;
			ReplyMsg( (struct Message *)xlmsg );
			break;
		    }
		    ReplyMsg( (struct Message *)xlmsg );
		}
	    }

	    if ( signals & SIGBREAKF_CTRL_C ) {
		D(PRINTF("MsgPortTask() 5 CTRL_C\n");)

		Forbid();
		Signal( (struct Task *)parent, CDXL_OB->KillSig );
		break;
	    }
	}

	while ( xlmsg = (XLMESSAGE *)GetMsg( port )) {
	    xlmsg->status = XLM_STATUS_ABORTING;
	    if ( CDXL_OB->flags & CDXL_PLAYING )
		xlmsg->status |= XLM_STATUS_PLAYING;

	    ReplyMsg( (struct Message *)xlmsg );
	}

	DeletePort( port );
    }

} // MsgPortTask()


/*
 * Our CDXL entry point. Takes a taglist which defines the options.
 * See runcdxl.h for the definitions/meanings of these tags.
 */
RunCDXL( ULONG tag, ... )
{
    CDXLOB		* CDXL_ob;
    UBYTE		* cdxlfile,* ilbmfile;
    struct TagItem	* tagitem,* intagitem = (struct TagItem *)&tag;
    struct Task		* MSGTask;
    int			  ret,EndDelay;
    DISP_DEF		  disp_def ;
    ULONG		  flags = NULL;
    BOOL		(* babort)( ULONG );

    ilbmfile = NULL;
    setmem( &disp_def, sizeof (DISP_DEF) ,0 );

    // Get cdxlfile
    if ( !(tagitem = FindTagItem( XLTAG_XLFile, intagitem )) ||
      !(cdxlfile = (UBYTE *)tagitem->ti_Data) ) {
	printf("RunCDXL() MISSING XLFILE\n");
	kprintf("RunCDXL() \n");
	return( RC_MISSING_FILE );
    }

    /*
     * Tag for specifying an ILBM to load into the background. If
     * found, set flags to indicate that we must AllocBitMap() with
     * the dimensions found in the file and we must then use seperate
     * bitmaps/buffers to load the CDXL image into and then blit
     * them to the display.
     */
    if ( (tagitem = FindTagItem( XLTAG_Background, intagitem )) &&
      (ilbmfile = (UBYTE *)tagitem->ti_Data) ) {

	disp_def.Flags |= (DISP_ALLOCBM|DISP_INTERLEAVED|DISP_BACKGROUND);
	flags |= CDXL_BLIT;
    }

    /*
     * Tag for using the CDXL's idea of ModeID.
     */
    if ( (tagitem = FindTagItem( XLTAG_XLModeID, intagitem )) &&
      tagitem->ti_Data ) {
	disp_def.Flags |= DISP_XLMODEID;
    }

    if ( ret = init( (disp_def.Flags & DISP_BACKGROUND) ? TRUE : FALSE ) ) {
	printf("RunCDXL() Could NOT init()\n");
	kprintf("RunCDXL() Could NOT init()\n");
	closedown();
	return( RC_FAILED );
    }

    if ( LowLevelBase ) {
	babort = ReadPort;
    } else {
	babort = ButtonAbort;
    }

    /*
     * Tag for blitting from a buffer to our display. If this tag is
     * found, set flags telling the display opening routines to 
     * AllocBitMap() our bitmaps, and make them interleaved.
     */
    if ( (tagitem = FindTagItem( XLTAG_Blit, intagitem )) &&
      tagitem->ti_Data ) {
	disp_def.Flags |= DISP_ALLOCBM|DISP_INTERLEAVED;
	flags |= CDXL_BLIT;
	D(PRINTF("RunCDXL() CDXL_BLIT\n");)
    }

    /*
     * Tag for loading a new palette for each frame.
     */
    if ( (tagitem = FindTagItem( XLTAG_MultiPalette, intagitem )) &&
      tagitem->ti_Data ) {
	flags |= CDXL_MULTI_PALETTE;
    }

    if ( disp_def.Flags & DISP_BACKGROUND ) {
	// Query the ILBM to find what sort of display to open.
	if ( !DoQuery( ilbmfile, &disp_def ) ) {
	    ret = RC_CANT_FIND;
	    printf("RunCDXL() Could NOT find '%ls'()\n",ilbmfile);
	    kprintf("RunCDXL() Could NOT find '%ls'()\n",ilbmfile);
	}
    }

    /*
     * Tag that says to open a view instead of a screen.
     */
    if ( !(tagitem = FindTagItem( XLTAG_View, intagitem )) ||
      !(tagitem->ti_Data) ) {
	disp_def.Flags |= DISP_SCREEN;
    }


    /*
     * Tag that says to read the XL file through DOS
     */
    if ( !(tagitem = FindTagItem( XLTAG_DOSXL, intagitem )) ||
      !(tagitem->ti_Data) ) {
	flags &= ~CDXL_DOSXL;
    } else {
	flags |= CDXL_DOSXL;
    }

    /*
     * Tag that says to read the XL file through cd/cd.device
     */
    if ( tagitem = FindTagItem( XLTAG_CDXL, intagitem ) ) {
	if( tagitem->ti_Data ) {
	    flags &= ~CDXL_DOSXL;
	} else {
	    flags |= CDXL_DOSXL;
	}
    }

    /*
     * Allocate the CDXLOB structure. Note that a **CDXL_ob is being sent
     * in, and that the allocated CDXLOB will be returned in it.
     */
    if ( !ret && !(ret = CDXLObtain( cdxlfile, &disp_def, &CDXL_ob, flags, (struct TagItem *)&tag )) ) {

	/*
	 * Tag that says how many times to play the CDXL
	 */
	if ( tagitem = FindTagItem( XLTAG_Loop, intagitem ) ) {
	    if ( (CDXL_ob->loops = tagitem->ti_Data) < -1 )
		CDXL_ob->loops = -1;	// Infinite loop
	} else {
	    CDXL_ob->loops = 1;
	}

	/*
	 * Tag that specifies a signal that will abort playback.
	 */
	if ( tagitem = FindTagItem( XLTAG_KillSig, intagitem ) ) {
	    CDXL_ob->KillSig = tagitem->ti_Data;
	}

	/*
	 * Tag that specifies an override READXLSPEED.
	 */
	if ( tagitem = FindTagItem( XLTAG_XLSpeed, intagitem ) ) {
	    CDXL_ob->ReadXLSpeed = tagitem->ti_Data;
	}

	/*
	 * Tag that tells us to abort on Left Mouse Button
	 */
	if ( (tagitem = FindTagItem( XLTAG_LMBAbort, intagitem )) &&
	  tagitem->ti_Data ) {
	    CDXL_ob->flags |= CDXL_LMBABORT;
	}

	/*
	 * Tag that tells us to abort on Right Mouse Button
	 */
	if ( (tagitem = FindTagItem( XLTAG_RMBAbort, intagitem )) &&
	  tagitem->ti_Data ) {
	    CDXL_ob->flags |= CDXL_RMBABORT;
	}

	/*
	 * Tag that tells us to abort on Joystick FIre Button
	 */
	if ( (tagitem = FindTagItem( XLTAG_FIREAbort, intagitem )) &&
	  tagitem->ti_Data ) {
	    CDXL_ob->flags |= CDXL_FIREABORT;
	}

	/*
	 * Tag that tells us to turn Error Correction on.
	 */
	if ( (tagitem = FindTagItem( XLTAG_XLEEC, intagitem )) &&
	  tagitem->ti_Data ) {
	    CDXL_ob->flags |= CDXL_XLEEC;	// Turn on Error Correction
	}

	/*
	 * Tag that tells us to load a different palette for each frame.
	 */
	if ( (tagitem = FindTagItem( XLTAG_XLPalette, intagitem )) &&
	  tagitem->ti_Data ) {
	    disp_def.Flags |= DISP_XLPALETTE; // Use XL Palette
	}

	/*
	 * Tag that tells us to force an interlace/noninterlace display.
	 */
	if ( tagitem = FindTagItem( XLTAG_LACE, intagitem ) ) {
	    if ( tagitem->ti_Data ) 
		disp_def.ModeID |= LACE;
	    else
		disp_def.ModeID &= ~LACE;
	}

	/*
	 * Tag that tells us to force a noninterlace/interlace display.
	 */
	if ( tagitem = FindTagItem( XLTAG_NONLACE, intagitem ) ) {
	    if ( tagitem->ti_Data ) 
		disp_def.ModeID &= ~LACE;
	    else
		disp_def.ModeID |= LACE;
	}


	/*
	 * Tag that tells us to force a HAM/NONHAM display.
	 */
	if ( tagitem = FindTagItem( XLTAG_HAM, intagitem ) ) {
	    if ( tagitem->ti_Data ) {
		disp_def.ModeID |= HAM;
		disp_def.ModeID &= ~EXTRA_HALFBRITE;
	    } else {
		disp_def.ModeID &= ~HAM;
	    }
	}

	/*
	 * Tag that tells us to force a NONHAM/HAM display.
	 */
	if ( tagitem = FindTagItem( XLTAG_NONHAM, intagitem ) ) {
	    if ( tagitem->ti_Data ) {
		disp_def.ModeID &= ~HAM;
	    } else {
		disp_def.ModeID |= HAM;
		disp_def.ModeID &= ~EXTRA_HALFBRITE;
	    }
	}

	/*
	 * Tag that tells us to force a EHB/NONEHB display.
	 */
	if ( tagitem = FindTagItem( XLTAG_EHB, intagitem ) ) {
	    if ( tagitem->ti_Data )  {
		disp_def.ModeID |= EXTRA_HALFBRITE;
		disp_def.ModeID &= ~HAM;
	    } else {
		disp_def.ModeID &= ~EXTRA_HALFBRITE;
	    }
	}

	/*
	 * Tag that tells us to force a NONEHB/EHB display.
	 */
	if ( tagitem = FindTagItem( XLTAG_NONEHB, intagitem ) ) {
	    if ( tagitem->ti_Data ) {
		disp_def.ModeID &= ~EXTRA_HALFBRITE;
	    } else {
		disp_def.ModeID |= EXTRA_HALFBRITE;
		disp_def.ModeID &= ~HAM;
	    }
	}

	/*
	 * Tag that tells us to force a HIRES/LORES display.
	 */
	if ( tagitem = FindTagItem( XLTAG_HIRES, intagitem ) ) {
	    if ( tagitem->ti_Data ) 
		disp_def.ModeID |= HIRES;
	    else
		disp_def.ModeID &= ~HIRES;
	}

	/*
	 * Tag that tells us to force a LORES/HIRES display.
	 */
	if ( tagitem = FindTagItem( XLTAG_LORES, intagitem ) ) {
	    if ( tagitem->ti_Data ) 
		disp_def.ModeID &= ~HIRES;
	    else
		disp_def.ModeID |= HIRES;
	}

	/*
	 * Tag that tells us to force a scan doubled/single display.
	 */
	if ( tagitem = FindTagItem( XLTAG_SDBL, intagitem ) ) {
	    if ( tagitem->ti_Data ) {
		disp_def.ModeID |= LORESSDBL_KEY;
	    } else {
		disp_def.ModeID &= ~LORESSDBL_KEY;
	    }
	}

	/*
	 * Tag that tells us to force an NTSC display.
	 */
	if ( (tagitem = FindTagItem( XLTAG_NTSC, intagitem ))
	  && tagitem->ti_Data ) {
		disp_def.ModeID &= ~MONITOR_ID_MASK;
		disp_def.ModeID |= NTSC_MONITOR_ID;
	}

	/*
	 * Tag that tells us to force a PAL display.
	 */
	if ( (tagitem = FindTagItem( XLTAG_PAL, intagitem ))
	  && tagitem->ti_Data ) {
		disp_def.ModeID &= ~MONITOR_ID_MASK;
		disp_def.ModeID |= PAL_MONITOR_ID;
	}

	/*
	 * Tag that tells us to force a DEFAULT display.
	 */
	if ( (tagitem = FindTagItem( XLTAG_DEFMON, intagitem ))
	  && tagitem->ti_Data ) {
		disp_def.ModeID &= ~MONITOR_ID_MASK;
	}

	/*
	 * Tag for Turning OFF Intuitions pointer if DISP_SCREEN.
	 */
	if ( (tagitem = FindTagItem( XLTAG_Pointer, intagitem )) &&
	  !tagitem->ti_Data ) {
	    disp_def.Flags |= DISP_NOPOINTER;
	}

	/*
	 * Tag for SetFunction()-ing OpenWorkbench to avoid the system
	 * from forcing the Workbench screen to open when the CDXLScreen
	 * closes if its the only screen open in the system.
	 */
	if ( (tagitem = FindTagItem( XLTAG_Patch_OpenWB, intagitem )) &&
	  tagitem->ti_Data ) {
	    D(PRINTF("RunCDXL() DISP_PATCH_OPENWB\n");)
	    disp_def.Flags |= DISP_PATCH_OPENWB;
	}

	/*
	 * Wait for the intro animation to stop.
	 */
	D(PRINTF("Closing freeanim.library\n");)
	CloseLibrary( FreeAnimBase );
	D(PRINTF("After closing freeanim.library ret= %ld\n",ret);)
	FreeAnimBase = NULL;

	/*
	 * Open the display, specifying an optional ilbmfile.
	 */
	if ( !ret && !(ret = OpenDisplay( &disp_def, ilbmfile ) ) ) {
	    D(PRINTF("Got Display\n");)

	    if ( CDXL_ob->flags & CDXL_MULTI_PALETTE ) {
		// So the border doesn't flash colors
		VideoControlTags( disp_def.vp->ColorMap,
		    VTAG_BORDERBLANK_SET,TRUE,
		    TAG_END);
	    }

	    if ( !(disp_def.Flags & DISP_BACKGROUND) || (disp_def.Flags & DISP_XLPALETTE) ) {
		/*
		 * Since the PAN format that we are using only stores
		 * 4 bits per gun, might as well just use LoadRGB4.
		 */
		D(PRINTF("\n\n\nCalling LoadRGB4\n\n");)
		LoadRGB4( disp_def.vp, CDXL_ob->CMap[0], min(CDXL_ob->CMapSize >> 1,disp_def.vp->ColorMap->Count) );
	    }

	    /*
	     * Tag that says to draw a box in color 0 around the CDXL.
	     */
	    if ( (tagitem = FindTagItem( XLTAG_Boxit, intagitem )) &&
	      tagitem->ti_Data ) {
		Boxit( &disp_def, CDXL_ob );
	    }

	    parent = (struct Process *)FindTask( NULL );

	    MSGTask = NULL;

	    // Check if a MsgPort is being requested.
	    if ( (tagitem = FindTagItem( XLTAG_MSGPortName, intagitem )) &&
	      (UBYTE *)tagitem->ti_Data && *(UBYTE *)tagitem->ti_Data ) {

		D(PRINTF("ti_Data= '%ls'\n",tagitem->ti_Data);)

		Forbid();
		if ( MSGTask = CreateTask( "XLMsgTask", 30L, MsgPortTask, 8000L) ) {
		    MSGTask->tc_UserData = (APTR)tagitem->ti_Data;

		    D(PRINTF("Created TASK\n");)

		    SetSignal( 0, SIGBREAKF_CTRL_E ); // Make sure signal is clear.
		    Permit();
		    Wait( SIGBREAKF_CTRL_E );

		    if ( ret = (int)MSGTask->tc_UserData )
			MSGTask = NULL;

		    D(PRINTF("Created GOT CTRL_E ret= %ld\n",ret);)

		} else {
		    Permit();
		    ret = RC_FAILED;
		    printf("Could NOT Create TASK\n");
		    kprintf("Could NOT Create TASK\n");
		}
	    }


	    /*
	     * Tag for bypassing the low pass filter.
	     */
	    if ( (tagitem = FindTagItem( XLTAG_No_LowPass, intagitem )) &&
		  tagitem->ti_Data ) {
	        ciaa.ciapra |= CIAF_LED;
	    }

	    /*
	     * Go to it.
	     */
	    if ( !ret ) {
		ret = StartCDXL( &disp_def, CDXL_ob );
	    } else {
		kprintf("RunCDXL() ret= %ld, NOT calling StartCDXL\n",ret);
	    }

	    /*
	     * Re-enable the low pass filter.
	     */
	    if ( (tagitem = FindTagItem( XLTAG_No_LowPass, intagitem )) &&
		  tagitem->ti_Data ) {
	        ciaa.ciapra &= ~CIAF_LED;
	    }

	    if ( !ret && (tagitem = FindTagItem( XLTAG_EndDelay, intagitem )) &&
	      (EndDelay = tagitem->ti_Data) ) {

		while ( (EndDelay < 0) || EndDelay-- ) {
		    if ( CDXL_ob->flags & CDXL_BUTTONABORTMASK ) {
			if ( babort( CDXL_ob->flags ) ) {
			    ret = RC_ABORTED;
			    break;
			}
	    	    }

		    if ( Wait(CDXL_ob->KillSig|CopSignal) & CDXL_ob->KillSig )
			break;
		}
	    }

	    D(PRINTF("\nStartCDXL returned ret= %ld\n",ret);)

	    Forbid();
	    if ( MSGTask && FindTask("XLMsgTask") ) {
		if ( ret ) {
		    D(PRINTF("// KILL the task 1\n");)
		    Signal( MSGTask, SIGBREAKF_CTRL_C );
		}
		Permit();

		FOREVER {
		    D(PRINTF("WAITING on KillSig\n");)
		    Wait( CDXL_ob->KillSig );

		    D(PRINTF("GOT KillSig\n");)
		    if ( FindTask("XLMsgTask") ) {
			D(PRINTF("// KILL the task 2\n");)
			Signal( MSGTask, SIGBREAKF_CTRL_C );
		    } else {
			break;
		    }
		}
	    } else {
		Permit();
	    }

	    CloseDisplay( &disp_def );
	} else {
	    printf("Did NOT get display ret= %ld\n",ret);
	    kprintf("Did NOT get display ret= %ld\n",ret);
	}
	CDXLTerm( CDXL_ob );
    } else {
	printf("RunCDXL() CDXLObtain() returned %ld\n",ret);
	kprintf("RunCDXL() CDXLObtain() returned %ld\n",ret);
    }

    closedown();

    return( ret );

} // RunCDXL()
