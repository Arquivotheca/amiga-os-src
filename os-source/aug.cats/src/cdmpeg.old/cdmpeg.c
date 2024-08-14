/*******************
      cdmpeg.c

    W.D.L 931014
********************/

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

#include <graphics/gfxbase.h>

#include <dos/dos.h>
#include <intuition/intuition.h>

#include <libraries/lowlevel.h>

#include "devices/cd.h"
#include <devices/mpeg.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/mpeg_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/realtime_pragmas.h>
#include <pragmas/mpeg_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>


#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "disp_def.h"
#include "retcodes.h"
#include "cdmpeg.h"

#include "cdmpeg_rev.h"


#include "debugsoff.h"
#define		KPRINTF
//#include "debugson.h"


// argument parsing 
#define TEMPLATE    "TRACK/A/N,BACK/K,LMBABORT/S,RMBABORT/S,FIREABORT/S,LACE/S,INFO/S" VERSTAG " Wayne D. Lutz"
#define OPT_TRACK	0
#define OPT_BACK	1
#define	OPT_LMBABORT	2
#define	OPT_RMBABORT	3
#define	OPT_FIREABORT	4
#define	OPT_LACE	5
#define	OPT_INFO	6
#define	OPT_COUNT	7


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;

BOOL			  DisplayIsPal;


int CXBRK(void) { return(0); }		/* Disable SASC CTRL/C handling */
int chkabort(void) { return(0); }	/* Indeed */

// Error messages.
STATIC UBYTE * ErrorMsg[] = {
    "No error",
    "Error while reading file",
    "Couldn't open file",
    "Not enough memory for operation",
    "Could not open cd/cdtv device",
    "Could not open window",
    "Could not open screen",
    "BAD MPEG Track specification.\nTry the INFO command line argument to see track info.",
    "Operation failed"
};


struct Library		* IntuitionBase;
struct Library		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;

STATIC struct Device	* MPEGDevice;
STATIC struct MsgPort	* MPEGPort;
STATIC struct IOMPEGReq	* MPEGReq;

STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;
STATIC struct IOStdReq	* CDDeviceMReq;

IMPORT ULONG	CopSignal;

/*
 * Close every thing down.
 */
STATIC VOID
closedown( VOID )
{
    if ( IntuitionBase )
	CloseLibrary( IntuitionBase );

    if ( GfxBase )
	CloseLibrary( GfxBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

    if ( LowLevelBase )
	CloseLibrary( LowLevelBase );

} // closedown()


/*
 * Open all of the required libraries.
 */
STATIC
init( BOOL iff )
{
    if ( !(IntuitionBase = OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library V39!\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library V39!\n");
	return( RC_FAILED );
    }

    if(iff && !(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
    }

    D(PRINTF("init() IFFParseBase= 0x%lx\n",IFFParseBase);)

    if ( !(LowLevelBase = OpenLibrary( "lowlevel.library", 40L )) ) {
	printf("Could NOT open lowlevel.library!\n");
	return( RC_FAILED );
    }

    FreeAnimBase = OpenLibrary("freeanim.library",0L);

    return( RC_OK );

} // init()


VOID
MPEGDeviceTerm( VOID )
{
    if ( MPEGReq ) {
	if ( MPEGDevice ) {
	    CloseDevice( (struct IORequest *)MPEGReq );
	    MPEGDevice = NULL;
	}

	if ( MPEGPort ) {
	    while( GetMsg( MPEGPort ) );
	}

	DeleteIORequest( MPEGReq );
	MPEGReq = NULL;
    }

    if ( MPEGPort ) {
	DeleteMsgPort( MPEGPort );
	MPEGPort = NULL;
    }

} // MPEGDeviceTerm()


/*
 * Attempts to open cd32mpeg.device if not already opened.
 * Returns:
 *	retcode: (BOOL) reflects device's open state.
 *  *opened: (BOOL) TRUE if opened by this call.
 */
BOOL
MPEGDeviceInit( ULONG * opened )
{

    if ( opened )
	*opened = FALSE;

    if ( !MPEGDevice ) {
	D(PRINTF("MPEGDeviceInit() have to prep MPEGDevice!");)

	if ( MPEGPort = CreateMsgPort() ) {
	    D(PRINTF("MPEGDeviceInit() GOT MPEGPort\n");)

	    if ( MPEGReq = CreateIORequest( MPEGPort, sizeof (struct IOMPEGReq)) ) {
		D(PRINTF("MPEGDeviceInit() GOT MPEGDeviceMReq\n");)

		if ( !OpenDevice( MPEG_NAME, 0, (struct IORequest *)MPEGReq, 0 ) ) {
		    D(PRINTF("MPEGDeviceInit() Got a Device\n");)
		    MPEGDevice = MPEGReq->iomr_Req.io_Device;
		}
	    }
	}

	if ( !MPEGDevice ) {
	    D(PRINTF("MPEGDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
	    MPEGPort, MPEGReq, MPEGDevice );)

	    MPEGDeviceTerm();
	    return( FALSE );
	}

	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // MPEGDeviceInit()


/*
 * Close cd.device if opened.
 */
VOID
CDDeviceTerm( VOID )
{
    if ( CDDeviceMReq ) {
	if ( CDDevice ) {

	    CloseDevice( (struct IORequest *)CDDeviceMReq );
	    CDDevice = NULL;
	}

	if ( MPEGPort ) {
	    while( GetMsg( MPEGPort ) );
	}

	DeleteStdIO( CDDeviceMReq );
	CDDeviceMReq = NULL;
    }

    if ( CDPort ) {
	DeleteMsgPort( CDPort );
	CDPort = NULL;
    }

} // CDDeviceTerm()


/*
 * Attempts to open cd.device if not already opened.
 * Returns:
 *	retcode: (BOOL) reflects device's open state.
 *  *opened: (BOOL) TRUE if opened by this call.
 */
BOOL
CDDeviceInit( ULONG * opened )
{
    if ( opened )
	*opened = FALSE;

    if ( !CDDevice ) {
	D(PRINTF("CDDeviceInit() have to prep CDDevice!");)

	if ( CDPort = CreateMsgPort() ) {
	    D(PRINTF("CDDeviceInit() GOT CDPort\n");)
	    if ( CDDeviceMReq = CreateStdIO( CDPort ) ) {
		D(PRINTF("CDDeviceInit() GOT CDDeviceMReq\n");)
		// Try to open cd.device. If can't, try for cdtv.device.

		if ( !OpenDevice( CD_NAME, 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
		    D(PRINTF("CDDeviceInit() Got a Device\n");)
		    CDDevice = CDDeviceMReq->io_Device;
		}
	    }
	}

	if ( !CDDevice ) {
	    D(PRINTF("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
	    CDPort, CDDeviceMReq, CDDevice );)

	    CDDeviceTerm();
	    return( FALSE );
	}

	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // CDDeviceInit()


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

    SendIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	printf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
	kprintf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()



VOID
DoSearch( int direction )
{
    struct IOMPEGReq	req = *MPEGReq;	// structure copy

    req.iomr_Arg1 = 1 * direction;
    req.iomr_Arg2 = NULL;

    D(PRINTF("DoSearch direction= %ld\n",direction);)

//    SendIOR( (struct IOStdReq *)&req, MPEGCMD_SCAN, NULL, NULL, NULL );

    if ( SendIOR( (struct IOStdReq *)&req, MPEGCMD_SINGLESTEP, NULL, NULL, NULL ) )
	WaitIO( (struct IORequest *)&req );

    D(PRINTF("DoSearch END\n\n");)

} // DoSearch()


VOID
DoPause( VOID )
{
STATIC	BOOL		Paused = FALSE;
    struct IOMPEGReq	req = *MPEGReq;	// structure copy

    Paused = !Paused;

    req.iomr_Arg1 = Paused;
    req.iomr_Arg2 = NULL;

    if ( SendIOR( (struct IOStdReq *)&req, MPEGCMD_PAUSE, NULL, NULL, NULL ) )
	WaitIO( (struct IORequest *)&req );

} // DoPause()

#define S(MASK) (MASK & state )

BOOL
ReadPort( ULONG flags )
{
STATIC ULONG	oldstate = 0;
    ULONG 	state;

    if ( flags & (MPEG_LMBABORT|MPEG_RMBABORT) )
	state = ReadJoyPort( 0 );

    if ( flags & MPEG_LMBABORT ) {
	if ( (JP_TYPE_MASK & state) == JP_TYPE_MOUSE) {
	    if (S(JPF_BTN2))
		return( TRUE );
	}
    }

    if ( flags & MPEG_RMBABORT ) {
	if ( (JP_TYPE_MASK & state) == JP_TYPE_MOUSE) {
	    if (S(JPF_BTN1))
		return( TRUE );
	}
    }

#ifdef	OUTT	// [
    if ( flags & MPEG_FIREABORT ) {
	state = ReadJoyPort( 1 );

	if ( ((JP_TYPE_MASK & state) == JP_TYPE_GAMECTLR) || ((JP_TYPE_MASK & state) == JP_TYPE_JOYSTK) ) {
	    if (S(JPF_BTN2))
		return( TRUE );
	}
    }
#else		// ][

    state = ReadJoyPort( 1 );

    if ( flags & MPEG_FIREABORT ) {
	if ( ((JP_TYPE_MASK & state) == JP_TYPE_GAMECTLR) || ((JP_TYPE_MASK & state) == JP_TYPE_JOYSTK) ) {
	    if (S(JPF_BTN2))
		return( TRUE );
	}
    }

    if ( S(JPF_BUTTON_PLAY) && (state != oldstate) )
	DoPause();
/*
    if ( S(JPF_BUTTON_FORWARD) ) {
	DoSearch( 1 );
    } else if ( S(JPF_BUTTON_REVERSE) ) {
	DoSearch( -1 );
    }
*/
#endif		// ]

    oldstate = state;

    return( FALSE );

} // ReadPort()


VOID
DumpTOC( union CDTOC * toc, int len )
{
    int	i;

    printf("FirstTrack= %ld, LastTrack= %ld, LeadOut LSN= %ld\n",
	toc[0].Summary.FirstTrack,toc[0].Summary.LastTrack,
	toc[0].Summary.LeadOut.LSN);

    len = MIN (len,(toc[0].Summary.LastTrack+1));

    for ( i = toc[0].Summary.FirstTrack; i < len; i++ ) {
	printf("%ld: Track #%ld, Position LSN= %ld, CtlAdr= %ld\n",
	    i,toc[i].Entry.Track,toc[i].Entry.Position.LSN,
	    toc[i].Entry.CtlAdr);
    }

} // DumpTOC()


PlayMPEG( ULONG track, ULONG flags )
{
    union CDTOC			* toc;
    struct MPEGVideoParamsSet	  mvp;
    ULONG			  signalMask,mpegsig,signals;
    int				  len,ret;
    BOOL			  done;

    len = 100;

    if ( !(toc = AllocVec ((len+1) * sizeof (union CDTOC), MEMF_CLEAR)) )
	return( RC_NO_MEM );

    ret = SendIOR( CDDeviceMReq, CD_TOCLSN, NULL, len, toc );

    if ( !ret ) {
	ret = RC_FAILED;
	printf("Could NOT get TOC!\n");
	goto exit;
    }
    WaitIO( (struct IORequest *)CDDeviceMReq );

    if ( flags & MPEG_INFO )
	DumpTOC( toc, len );

    if ( (track <  toc[0].Summary.FirstTrack) || (track > toc[0].Summary.LastTrack) ) {
	ret = RC_BAD_TRACK;
	goto exit;
    }
//    toc[toc[0].Summary.LastTrack + 1].Entry.Position.LSN = toc[0].Summary.LeadOut.LSN;


    mvp.mvp_Fade = 65535;	// Fade level.  0 = no MPEG video at all, 65535 = full saturation
    mvp.mvp_DisplayType = 0;	// DisplayIsPal ? 3 : 4;	// 3 = PAL (50Hz), 4 = NTSC (60Hz)

    ret = SendIOR( (struct IOStdReq *)MPEGReq, MPEGCMD_SETVIDEOPARAMS, NULL,
	sizeof (struct MPEGVideoParamsSet), &mvp );

    if ( !ret ) {
	ret = RC_FAILED;
	printf("Could NOT SETVIDEOPARAMS!\n");
	printf("iomr_MPEGError= %ld\n",MPEGReq->iomr_MPEGError);
	goto exit;
    }
    WaitIO( (struct IORequest *)MPEGReq );

    MPEGReq->iomr_MPEGFlags = 0;
    MPEGReq->iomr_StreamType = MPEGSTREAM_SYSTEM;
    MPEGReq->iomr_Arg1 = 2328;
    MPEGReq->iomr_Arg2 = toc[track].Entry.Position.LSN;

    ret = SendIOR( (struct IOStdReq *)MPEGReq, MPEGCMD_PLAYLSN,
	toc[track].Entry.Position.LSN,
	toc[track + 1].Entry.Position.LSN - toc[track].Entry.Position.LSN,
	NULL );

    if ( !ret ) {
	ret = RC_FAILED;
	printf("Could NOT PLAYLSN!\n");
	printf("iomr_MPEGError= %ld\n",MPEGReq->iomr_MPEGError);
	goto exit;
    }
    ret = RC_OK;

    mpegsig = 1L << MPEGPort->mp_SigBit;

    signalMask = SIGBREAKF_CTRL_C | mpegsig | CopSignal;

    done = FALSE;
    while ( !done ) {

	signals = Wait( signalMask );

	if ( GetMsg( MPEGPort ) ) {
	    D(PRINTF("Got Msg from MPEGPort\n");)
	    done = TRUE;
	}

	if ( signals & SIGBREAKF_CTRL_C ) {
	    D(PRINTF("GOT a CTRL_C\n");)
	    if ( !CheckIO( (struct IORequest *)MPEGReq ) ) {
		D(PRINTF("Aborting!!\n");)
		AbortIO( (struct IORequest *)MPEGReq );
		WaitIO( (struct IORequest *)MPEGReq );
	    }
	    done = TRUE;
	}

	if ( signals & CopSignal ) {
	    if ( flags & MPEG_BUTTONABORTMASK ) {
		if ( ReadPort( flags ) ) {
		    D(PRINTF("Button Aborting!!\n");)
		    if ( !CheckIO( (struct IORequest *)MPEGReq ) ) {
			D(PRINTF("Aborting!!\n");)
			AbortIO( (struct IORequest *)MPEGReq );
			WaitIO( (struct IORequest *)MPEGReq );
		    }
		    done = TRUE;
		}
	    }
	}

    }

    if ( SendIOR( (struct IOStdReq *)MPEGReq, CMD_FLUSH, NULL, NULL, NULL ) ) {
	WaitIO( (struct IORequest *)MPEGReq );
    }

exit:

    FreeVec( toc );

    return( ret );

} // PlayMPEG()


VOID
main( LONG argc,char * argv[] )
{
    UBYTE		* ilbmfile;
    ULONG		  track,flags = NULL;
    struct DisplayInfo	  disp;
    int			  ret;
    DISP_DEF		  disp_def ;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }

    ilbmfile = (UBYTE *)opts[OPT_BACK];
    track = *(ULONG *)opts[OPT_TRACK];

    if ( ret = init( ilbmfile ? TRUE : FALSE ) ) {
	closedown();
	exit( RETURN_FAIL );
    }

    setmem( &disp_def, sizeof (DISP_DEF) ,0 );

    disp_def.Flags |= DISP_SCREEN|DISP_NOPOINTER|DISP_COP_INT;

    if ( ilbmfile ) {
	disp_def.Flags |= (DISP_INTERLEAVED|DISP_BACKGROUND|DISP_ALLOCBM);

	// Query the ILBM to find what sort of display to open.
	if ( !DoQuery( ilbmfile, &disp_def ) ) {
	    ret = RC_CANT_FIND;
	    PF("main RC_CANT_FIND");
	    closedown();
	    exit( RETURN_FAIL );
	}

    } else {
	disp_def.Width = 320;
	disp_def.Depth = 1;

	if ( opts[OPT_LACE] ) {
	    disp_def.Height = 400;
	    disp_def.ModeID = LORESLACE_KEY;
	} else {
	    disp_def.Height = 200;
	    disp_def.ModeID = LORES_KEY;
	}
    }

    D(PRINTF("ilbmfile= '%ls'\n", PFSTR(ilbmfile) );)

    DisplayIsPal = FALSE;
    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

    D(PRINTF("display type= '%ls', gfxbase says '%ls' - '%ls'\n",
	DisplayIsPal ? "PAL" : "NTSC",
	(GfxBase->DisplayFlags & PAL) ? "PAL" : "NTSC",
	(GfxBase->DisplayFlags & REALLY_PAL) ? "REALLY_PAL" : "REALLY_NTSC" );)

    D(PRINTF("After Query disp_def.Width= %ld, Height= %ld\n",
	disp_def.Width,disp_def.Height);)

    if ( opts[OPT_LMBABORT] )
	flags |= MPEG_LMBABORT;

    if ( opts[OPT_RMBABORT] )
	flags |= MPEG_RMBABORT;

    if ( opts[OPT_FIREABORT] )
	flags |= MPEG_FIREABORT;

    if ( opts[OPT_INFO] )
	flags |= MPEG_INFO;

    CDDeviceInit( NULL );
    MPEGDeviceInit( NULL );

    if ( !(CDDeviceMReq && MPEGReq) ) {
//	ret = RC_FAILED;
	ret = RC_OK;	// So no error messages are printed to the console
	goto exit;
    }

    D(PRINTF("\nCDDeviceMReq= 0x%lx, MPEGReq= 0x%lx\n",
	CDDeviceMReq,MPEGReq);)

    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;

    disp_def.ModeID &= ~MONITOR_ID_MASK;

    if ( !ret && !(ret = OpenDisplay( &disp_def, ilbmfile ) ) ) {

	ret = PlayMPEG( track, flags );

	CloseDisplay( &disp_def );
    }

exit:
    MPEGDeviceTerm();
    CDDeviceTerm();

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	printf("'%ls'\n",ErrorMsg[ret]);
	ret = RETURN_FAIL;
    }

    closedown();

    exit( ret );

} // main()
