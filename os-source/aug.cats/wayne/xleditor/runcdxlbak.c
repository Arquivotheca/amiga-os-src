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

#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()
#include "stdio.h"

#include "devices/cd.h"

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/blitdef.h"
#include "cdxl/runcdxl_protos.h"
#include "cdxl/runcdxl.h"
#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
/*
#define KPRINTF
#include "cdxl/debugson.h"
*/

struct CDInfo __aligned cdinfo,saveinfo;

struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;

struct Process * parent;

STATIC	struct MinList	XList;

STATIC	LONG	XLSignal	= -1;
STATIC	LONG	XLSignalBit	= -1;

ULONG	Count;

CDXLOB * CDXL_OB;	// Global CDXLOB


STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;

STATIC struct IOStdReq	* CDDeviceMReq;

STATIC LONG	CDPortSignal	= -1;

IMPORT struct Custom far custom;
IMPORT ULONG	CopSignal;


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

    kprintf("Opening freeanim.library\n");
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    kprintf("After opening freeanim.library\n");

    return( RC_OK );

} // init()


/*
 * Send a CD_INFO command to cd.device. The info is stored in the cdinfo structure.
 */
BOOL
GetCDInfo( struct CDInfo * cdi )
{
    SendIOR( CDDeviceMReq, CD_INFO, NULL, sizeof ( *cdi ), cdi );
    WaitIO( (struct IORequest *)CDDeviceMReq );

    if ( !(CDDeviceMReq->io_Error ) ) {
	kprintf("\nReadXLSpeed= %ld, MaxSpeed= %ld\nSectorSize= %ld, XLECC= %ld\nStatus= %ld\n\n",
	cdi->ReadXLSpeed,cdi->MaxSpeed,cdi->SectorSize,cdi->XLECC,cdi->Status);

	return( TRUE );
    } else {
	printf("CD_INFO ERROR!!! io_Error= %ld\n",CDDeviceMReq->io_Error);
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

    WaitIO( (struct IORequest *)CDDeviceMReq );

    if ( CDDeviceMReq->io_Error ) {
	printf("\n!!!CD_CONFIG ERROR!!! io_Error= %ld\n\n",CDDeviceMReq->io_Error);
	return( FALSE );
    }

    GetCDInfo( &cdinfo );

    return( TRUE );

} // CDConfig()


/*
 * Take a Pan structure and determine what kind of display to open.
 */
VOID
Pan2Display( PAN * pan, DISP_DEF * disp_def, ULONG flags )
{
    LONG				pi;
    int					i,ht = NTSC_HEIGHT;
    struct	Rectangle	drect;
    struct DisplayInfo	disp;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    ht = PAL_HEIGHT;
    }

    disp_def->Depth = pan->PixelSize;

    pi = PI_VIDEO( pan );

    disp_def->ModeID = NULL;

    if( pi == PIV_HAM ) {
	disp_def->ModeID |= HAM;
    } else if( pi == PIV_AVM ) {
	disp_def->ModeID |= HIRES;
    } else if ( (pi == PIV_STANDARD) && (pan->PixelSize == 6) ) {
	disp_def->ModeID |= EXTRA_HALFBRITE;
    }

    // High Resolution guess
    if ( pan->XSize >= 640 ) {
	disp_def->ModeID |= HIRES;
    }

    i = SCREEN_WIDTH;
    if ( pan->XSize >= i) {
	disp_def->ModeID |= HIRES;
    } else {
	i >>= 1;
    }

    if ( pan->XSize > i)
	disp_def->Flags |= DISP_OVERSCANX;

    i = ht;

    // InterLace guess
    if ( pan->YSize >= i ) {
	disp_def->ModeID |= LACE;
    } else {
	i >>= 1;
    }

    if ( pan->YSize > i)
	disp_def->Flags |= DISP_OVERSCANY;

    if ( disp_def->Flags & DISP_OVERSCAN ) {
	QueryOverscan( disp_def->ModeID, &drect, OSCAN_MAX );

	disp_def->NominalWidth = (drect.MaxX - drect.MinX) + 1;
	disp_def->NominalHeight = (drect.MaxY - drect.MinY) + 1;

	if ( disp_def->Flags & DISP_OVERSCANX )
	    disp_def->Left = drect.MinX;

	if ( disp_def->Flags & DISP_OVERSCANY )
	    disp_def->Top = drect.MinY;

    } else {
	disp_def->NominalWidth = SCREEN_WIDTH;
	disp_def->NominalHeight = ht;

	if ( !(disp_def->ModeID & HIRES) )
	    disp_def->NominalWidth >>= 1;

	if ( !(disp_def->ModeID & LACE) )
	    disp_def->NominalHeight >>= 1;
    }

    /*
     * If we are blitting from a buffer to our display, open the display
     * full width. Else open a display only as big as the CDXL.
     */
    if ( flags & CDXL_BLIT ) {
	disp_def->Width = disp_def->NominalWidth;
	disp_def->Height = disp_def->NominalHeight;
    } else {
	disp_def->Width = pan->XSize;
	disp_def->Height = pan->YSize;
    }

} // Pan2Display


/*
 * Free the CDXLOB and its buffers
 */
VOID
CDXLFreeOb( CDXLOB * CDXL_ob )
{
    int i;

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
    if ( !(disp_def->Flags & DISP_BACKGROUND) )
	Pan2Display( &pan, disp_def, flags );

    Close( File );
    File = NULL;

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
    CDXL_ob->FrameSize = FRAME_SIZE( &pan );

    kprintf("CDXL_ob->FrameSize= %ld\n",CDXL_ob->FrameSize);

    CDXL_ob->CMapSize = CMAP_SIZE( &pan );
    CDXL_ob->AudioSize = pan.AudioSize;

    CDXL_ob->BufSize = CDXL_ob->FrameSize;

    CDXL_ob->StartSector = CDXLSector;
    CDXL_ob->NumFrames = ( Size / CDXL_ob->FrameSize );

    /*
     * Set 2 buffers.
     */
    for ( j = 0; j < 2; j++ ) {

	if( !(CDXL_ob->Buffer[j] = AllocMem( CDXL_ob->BufSize, MEMF_CHIP|MEMF_CLEAR ) ) ) {
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
	CDXL_ob->rect.MinX = (max(disp_def->NominalWidth,disp_def->Width) >> 1) - (pan.XSize >> 1);
    }

    /*
     * If a top value is specified use it. Else center.
     */
    if ( ti = FindTagItem( XLTAG_Top, inti )  ) {
	CDXL_ob->rect.MinY = ti->ti_Data;
    } else {
	// Center it;
	CDXL_ob->rect.MinY = (max(disp_def->NominalHeight,disp_def->Height) >> 1) - (pan.YSize >> 1);
    }

    // Clip it.
    CDXL_ob->rect.MinX = min( CDXL_ob->rect.MinX, disp_def->Width );
    CDXL_ob->rect.MinY = min( CDXL_ob->rect.MinY, disp_def->Height );

    // If display will be scan doubled
    if ( sdbl ) {
	D(PRINTF("SDBL... Before MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
	CDXL_ob->rect.MinY >>= 2;
	disp_def->Top >>= 2;
	D(PRINTF("AFTER MinY= %ld, Top= %ld\n",CDXL_ob->rect.MinY,disp_def->Top);)
    }

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

	if ( !(disp_def->Flags & DISP_OVERSCANX) )
	    disp_def->Left = CDXL_ob->rect.MinX;

	if ( !(disp_def->Flags & DISP_OVERSCANY) )
	    disp_def->Top = CDXL_ob->rect.MinY;
    }

    return( RC_OK );

error:

    printf("CDXLObtain error ret= %ld\n",ret);

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
	    if ( CDDeviceMReq = CreateStdIO( CDPort ) ) {
		// Try to open cd.device. If can't, try for cdtv.device.

		i = (CDXL_ob->flags & CDTV_DEVICE) ? 1 : 0;

		for ( ; i < 2; i++ ) {
		    if ( !OpenDevice( device_name[i], 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
			CDDevice = CDDeviceMReq->io_Device;
			break;
		    }
		}
	    }
	}

	if ( !CDDevice ) {
	    printf("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
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
	kprintf("GOT cd.device\n");
	GetCDInfo( &cdinfo );
	saveinfo = cdinfo;	// structure copy

	CDXL_ob->NumSectors = CDXL_ob->FibSize / cdinfo.SectorSize;

	if ( CDXL_ob->ReadXLSpeed !=  cdinfo.ReadXLSpeed ) {
	    if ( CDXL_ob->ReadXLSpeed > cdinfo.MaxSpeed )
		CDXL_ob->ReadXLSpeed = cdinfo.MaxSpeed;

	    CDConfig( TAGCD_READXLSPEED, CDXL_ob->ReadXLSpeed, TAG_END );
	}

	// Set Errror Correction
	CDConfig( TAGCD_XLECC, (CDXL_ob->flags & CDXL_XLEEC) ? 1 : 0, TAG_END );

    } else {
	kprintf("GOT cdtv.device\n");
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

    if ( xllist->mlh_Head )
	xllist->mlh_Head->mln_Pred = NULL;

    if ( xllist->mlh_TailPred )
	xllist->mlh_TailPred->mln_Succ = NULL;

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
	AbortIO( (struct IORequest *) CDDeviceMReq );

	WaitIO(  (struct IORequest *) CDDeviceMReq );

	// Then a SEEK is required to force the drive to stop (sometimes necessary)
	CDDeviceMReq->io_Command = CD_SEEK;
	CDDeviceMReq->io_Offset  = 0;
	CDDeviceMReq->io_Length  = 0;
	CDDeviceMReq->io_Data    = NULL;

	DoIO( (struct IORequest *) CDDeviceMReq );

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
	// Start the audio the first time thru.
	if ( !Count ) 
	    StartAudio();
    }

    Count++;
//    kprintf("%ld: 0x%lx  ",Count,xl);
    CDXL_OB->curVideo ^= 1;

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
	kprintf("SendIOR() cmd= %ld io_Offset= %ld, io_Length= %ld\n",
	    req->io_Command,req->io_Offset,req->io_Length);
    }

    SendIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	printf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
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

    kprintf("NewCD_XL xl= 0x%lx\n",xl);

    AddTail( (struct List *)first, (struct Node *)xl );

    return( RC_OK );

} // NewCD_XL()


/*
 * Conatins the main loop which handles the CDXL playback.
 */
PlayCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    LONG	LastFrame,NumColors;
    ULONG	SignalMask,DBufSignal;
    ULONG	Signal;
    SHORT	curbm,curvid,DstX,DstY,Wid,Ht;
    BOOL	Need2Change,Safe2Change;
    BLITDEF	BDef;
    int		ret = RC_OK;

    /*
     * The signal for the CDXL interrupt to ping us with.
     */
    if ( ( XLSignalBit = AllocSignal( -1 ) ) == -1 ) {
	return( RC_NO_MEM );
    }

    XLSignal = ( 1 << XLSignalBit );

    /*
     * The signal from the copper interrupt telling us to call ChangeVPBitMap().
     */
    DBufSignal = CopSignal;

    curvid = 0;
    curbm = 1;
    Count = 0;
    Need2Change = Safe2Change = FALSE;
    NumColors = min(CDXL_ob->CMapSize >> 1,disp_def->vp->ColorMap->Count);
    LastFrame = CDXL_ob->NumFrames - 2;

    /*
     * Wait for signals from:
     *	the CDXL interrupt (XLSignal),
     *	the user telling us to abort (CDXL_ob->KillSig),
     *	the device telling us something is wrong (CDPortSignal),
     *	the copper interrupt telling us to Call ChangeVPBitMap() (DBufSignal).
     */
    SignalMask = ( XLSignal|CDXL_ob->KillSig|CDPortSignal|DBufSignal);

    DstX = CDXL_ob->rect.MinX;
    DstY = CDXL_ob->rect.MinY;

    Wid = (CDXL_ob->rect.MaxX - CDXL_ob->rect.MinX) - CDXL_ob->xoff;
    Ht = (CDXL_ob->rect.MaxY - CDXL_ob->rect.MinY) - CDXL_ob->yoff;


    /*
     * If we are blitting from a buffer to the display, Initialize a
     * BLITDEF structure. The philosophy here is to do all of the
     * blitter calculations once, before the CDXL is actually running.
     * In this way we can hopefully gain an edge.
     */
    if ( CDXL_ob->flags & CDXL_BLIT ) {
	InitBDef (&BDef, &CDXL_ob->bm[0], CDXL_ob->xoff,CDXL_ob->yoff,
	    disp_def->bm[0], DstX, DstY, Wid, Ht, NULL );
    }

    CDXL_OB = CDXL_ob;	// Set the Global ptr

    /*
     * Start the CDXL. Things are done differently depending upon whether
     * we have cd.device opened or cdtv.device.
     */
    if ( CDXL_ob->flags & CDTV_DEVICE ) {
	SendIOR(CDDeviceMReq,CDTV_READXL,CDXL_ob->StartSector,CDXL_ob->NumSectors,&XList);
//	SendIOR(CDDeviceMReq,CDTV_READXL,CDXL_ob->StartSector,CDXL_ob->NumSectors,XList.mlh_Head);
    } else {
	SendIOR(CDDeviceMReq,CD_READXL,CDXL_ob->StartSector * cdinfo.SectorSize,CDXL_ob->NumSectors * cdinfo.SectorSize,&XList);
//	SendIOR(CDDeviceMReq,CD_READXL,CDXL_ob->StartSector * cdinfo.SectorSize,CDXL_ob->NumSectors * cdinfo.SectorSize,XList.mlh_Head);
    }

    /*
     * Our main CDXL loop. Continue until we get a kill signal from the user,
     * (CDXL_ob->KillSig) or from the device (CDPortSignal), or until we reach
     * the last frame.
     */
    while ( Count < LastFrame ) {
	Signal = Wait( SignalMask );

	if ( Signal & (CDXL_ob->KillSig|CDPortSignal) ) {
	    // Got a kill signal.
	    break;
	}

	/*
	 * Recieved a signal from our CDXL interrupt telling us that it
	 * has finished with one of our transfer lists.
	 */
	if ( Signal & XLSignal ) {

	    if ( CDXL_ob->flags & CDXL_BLIT ) {

		BlitBDef( &BDef, &CDXL_ob->bm[curvid], disp_def->bm[curbm], NULL );
		/*
		 * Uncomment if you would rather use BltBitMap() as opposed
		 * to the BLITDEF stuff.
		 *
		 * BltBitMap( &CDXL_ob->bm[curvid],0,0,disp_def->bm[curbm],
		 *    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);
		 */
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
//		    LoadRGB4( disp_def->vp, CDXL_ob->CMap[CDXL_ob->curVideo^1],NumColors );
		    LoadRGB4( disp_def->vp, CDXL_ob->CMap[curvid^1],NumColors );
		    Safe2Change = FALSE;
		}

		Need2Change = FALSE;

		if ( CDXL_ob->flags & CDXL_BLIT ) {
		    curbm ^= 1;
		}
	    } else if ( !(CDXL_ob->flags & CDXL_MULTI_PALETTE) ) {
		// If changing palette every frame, we must keep in sync with copper
		Safe2Change = TRUE;
	    }
	}

    }

    if ( Count ) {
	if ( CDXL_ob->AudioSize )
	    StopAudio();
    }

    return( ret );

} // PlayCDXL()


/*
 * Get ready to start playing the CDXL.
 */
StartCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob )
{
    int			ret,i;
    PFI			newxl;

    if( !CDXL_ob )
	return( RC_FAILED );

    parent = (struct Process *)FindTask( NULL );

    NewList( (struct List *)&XList );

    if( !CDDeviceInit( NULL, CDXL_ob ) ) {
	return( RC_NO_CDDEVICE );
    }

    if ( CDXL_ob->AudioSize ) {
	if ( ret = InitAudio( CDXL_ob ) )
	    return( ret );
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
	    break;
	}
    }

    if ( !ret )	{

	// Tie list together to give us an endless loop
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
 * Our CDXL entry point. Takes a taglist which defines the options.
 * See runcdxl.h for the definitions/meanings of these tags.
 */
RunCDXL( ULONG tag, ... )
{
    CDXLOB		* CDXL_ob;
    UBYTE		* cdxlfile,* ilbmfile;
    struct TagItem	* tagitem,* intagitem = (struct TagItem *)&tag;
    int			  ret;
    DISP_DEF		  disp_def ;
    ULONG		  flags = NULL;

    ilbmfile = NULL;
    setmem( &disp_def, sizeof (DISP_DEF) ,0 );

    // Get cdxlfile
    if ( !(tagitem = FindTagItem( XLTAG_XLFile, intagitem )) ||
      !(cdxlfile = (UBYTE *)tagitem->ti_Data) ) {
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

    if ( ret = init( (disp_def.Flags & DISP_BACKGROUND) ? TRUE : FALSE ) ) {
	closedown();
	return( RC_FAILED );
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
    }

    /*
     * Tag to force opening of cdtv.device as opposed to cd.device.
     */
    if ( (tagitem = FindTagItem( XLTAG_CDTV, intagitem )) &&
      tagitem->ti_Data ) {
	flags |= CDTV_DEVICE;
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
	}
    }


    /*
     * Allocate the CDXLOB structure. Note that a **CDXL_ob is being sent
     * in, and that the allocated CDXLOB will be returned in it.
     */
    if ( !ret && !(ret = CDXLObtain( cdxlfile, &disp_def, &CDXL_ob, flags, (struct TagItem *)&tag )) ) {

	/*
	 * Tag that says to open a view instead of a screen.
	 */
	if ( !(tagitem = FindTagItem( XLTAG_View, intagitem )) ||
	  !(tagitem->ti_Data) ) {
	    disp_def.Flags |= DISP_SCREEN;
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
		disp_def.ModeID |= NTSC_MONITOR_ID;
	}

	/*
	 * Tag that tells us to force a PAL display.
	 */
	if ( (tagitem = FindTagItem( XLTAG_PAL, intagitem ))
	  && tagitem->ti_Data ) {
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
	 * Wait for the intro animation to stop.
	 */
	kprintf("Closing freeanim.library\n");
	CloseLibrary( FreeAnimBase );
	kprintf("After closing freeanim.library\n");
	FreeAnimBase = NULL;

	/*
	 * Open the display, specifying an optional ilbmfile.
	 */
	if ( !(ret = OpenDisplay( &disp_def, ilbmfile ) ) ) {

	    if ( !(disp_def.Flags & DISP_BACKGROUND) || (disp_def.Flags & DISP_XLPALETTE) ) {
		/*
		 * Since the PAN format that we are using only stores
		 * 4 bits per gun, might as well just use LoadRGB4.
		 */
		LoadRGB4( disp_def.vp, CDXL_ob->CMap[0], min(CDXL_ob->CMapSize >> 1,disp_def.vp->ColorMap->Count) );
	    }

	    /*
	     * Tag that says to draw a box in color 0 around the CDXL.
	     */
	    if ( (tagitem = FindTagItem( XLTAG_Boxit, intagitem )) &&
	      tagitem->ti_Data ) {
		Boxit( &disp_def, CDXL_ob );
	    }

	    /*
	     * Go to it.
	     */
	    ret = StartCDXL( &disp_def, CDXL_ob );
	    CloseDisplay( &disp_def );
	}
	CDXLTerm( CDXL_ob );
    }

    closedown();

    return( ret );

} // RunCDXL()
