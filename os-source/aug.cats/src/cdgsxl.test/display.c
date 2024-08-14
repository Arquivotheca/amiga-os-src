/*************

	Display.c
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
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>	// for setmem()
#include "stdio.h"

#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"

#include "cdxl/debugsoff.h"

/*	// Uncomment to get debug output turned on
#define KPRINTF
#include "cdxl/debugson.h"
*/


IMPORT struct GfxBase		* GfxBase;
IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct Library		* GameBase;
IMPORT struct Custom		  far custom;

ULONG		  smartgfx = 0x12345678;
IMPORT BOOL	  DoScreen;

struct Screen	* CDXLScreen;
struct Window	* CDXLWindow;

VOID CloseDisplay( DISP_DEF * disp_def );
int kprintf(const char *, ...);
int DoILBM( UBYTE * filename, DISP_DEF * disp_def );
VOID	CopRoutine( VOID );	// In CopRoutine.a


struct Interrupt CopInt;
struct UCopList	* old_ucl,ucl;
BOOL	CopIntAdded;
BYTE	CopSigBit = -1;
ULONG	CopSignal;

/*
 * Data that gets passed into our copper interrupt routine.
 */
typedef	struct IntData {
    struct Task	* Task;
    ULONG	  SigMask;

} INTDATA;

INTDATA CopIntData;

/*
 * Set up the data that will get sent into the copper interrupt.
 */
SetUpIntData( INTDATA * intdata )
{
    if ( ( CopSigBit = AllocSignal(-1) ) != -1 ) {
	CopSignal = intdata->SigMask = (1L << CopSigBit);
	intdata->Task = FindTask( NULL );
	return( TRUE );
    }

    return( FALSE );

} // SetUpIntData()


/*
 * Add a copper interrupt that will signal us to call ChangeVPBitMap().
 */
VOID
AddCopInt( DISP_DEF * disp_def, struct UCopList * ucl, struct Interrupt * copint, INTDATA * intdata )
{
    setmem( ucl, sizeof ( *ucl ), 0 );

    CINIT ( ucl, 50 );
    CMOVE ( ucl, custom.intena, (INTF_SETCLR|INTF_COPER) );
//  CWAIT ( ucl, 0, 0 );
    CMOVE ( ucl, custom.intreq, (INTF_SETCLR|INTF_COPER) );
    CEND ( ucl );

    old_ucl = disp_def->vp->UCopIns;
    disp_def->vp->UCopIns = ucl;

    copint->is_Node.ln_Type = NT_INTERRUPT;
    copint->is_Node.ln_Pri = 0;
    copint->is_Node.ln_Name = "";
    copint->is_Data = intdata;
    copint->is_Code = CopRoutine;

    AddIntServer( INTB_COPER, copint );

    CopIntAdded = TRUE;

    if ( disp_def->Flags & DISP_SCREEN ) {
	RethinkDisplay();
    }

} // AddCopInt()


ULONG
GetModeID( DISP_DEF * disp_def )
{
    /* Check that the ModeID can handle doublebuffering.
     * If not, then find the best ModeID that can.
     */
    struct DisplayInfo	disp;
    ULONG		ModeID = disp_def->ModeID;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, ModeID) ) {

	if ( !(disp.PropertyFlags & DIPF_IS_DBUFFER) ) {

	    ModeID = BestModeID(BIDTAG_NominalWidth, disp_def->Width,
	    BIDTAG_NominalHeight, disp_def->Height,
	    BIDTAG_Depth, disp_def->Depth,
	    /* Keep the DIPF_ properties of the original
	     * ModeID, but ensure we have DBUFFER
	     * properties too.
	     */
	    BIDTAG_SourceID, ModeID,
	    BIDTAG_DIPFMustHave, DIPF_IS_DBUFFER,
	    TAG_END);
	}
    }

    return( disp_def->ModeID = ModeID );

} // GetModeID()


/*
 * Open a screen and a window as defined by disp_def. Optionally
 * load an ILBM file into background.
 */
ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile )
{
    ULONG	tag;
    ULONG	flags;

    GetModeID( disp_def );

    /*
     * If we are opening a full sized screen as opposed to a screen
     * that is just the size of the CDXL dimensions.
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	flags = BMF_CLEAR|BMF_DISPLAYABLE;

	if ( disp_def->Flags & DISP_INTERLEAVED )
	    flags |= BMF_INTERLEAVED;

	if (!(disp_def->bm[0] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 1\n");)
	    disp_def->bm[1] = NULL;
	    return( RC_NO_MEM );
	}

	if (!(disp_def->bm[1] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 2\n");)

	    FreeBitMap( disp_def->bm[0] );
	    disp_def->bm[0] = NULL;

	    return( RC_NO_MEM );
	}
	tag = TAG_IGNORE;
    } else {
	tag = SA_Left;
    }

    if ( CDXLScreen = OpenScreenTags(NULL,
	SA_Width,	disp_def->Width,
	SA_Height,	disp_def->Height,
	SA_Depth,	disp_def->Depth,
	SA_BitMap,	disp_def->bm[0],
	SA_DisplayID,	disp_def->ModeID,
	SA_Interleaved,	(disp_def->Flags & DISP_INTERLEAVED) ? TRUE : FALSE,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	tag,		disp_def->Left,
	TAG_DONE) )
    {

	VideoControlTags( CDXLScreen->ViewPort.ColorMap, VC_IntermediateCLUpdate,0,0);
	VideoControlTags( CDXLScreen->ViewPort.ColorMap, VC_IntermediateCLUpdate_Query,&smartgfx,0);

	smartgfx=(smartgfx==0x12345678)?0:-1;
	kprintf("\nsmartgfx= '%ls'\n\n",
	    smartgfx ? "SMART" : "DUMB" );

	if ( CDXLWindow = OpenWindowTags( NULL,
	    WA_Width,		CDXLScreen->Width,
	    WA_Height,		CDXLScreen->Height,
	    WA_IDCMP,		NULL,	//IDCMP_MOUSEBUTTONS,
	    WA_Flags,		BACKDROP | BORDERLESS | RMBTRAP,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	CDXLScreen,
	    TAG_DONE) ) {

	    disp_def->vp = &CDXLScreen->ViewPort;

	    if ( disp_def->dbuf = AllocDBufInfo( disp_def->vp ) ) {

		if ( (disp_def->Flags & DISP_BACKGROUND) && ilbmfile && *ilbmfile ) {

		    if ( DoILBM( ilbmfile, disp_def ) )
			printf("Could not open '%ls'\n",ilbmfile);
		}

		/*
		 * Add the copper interrupt.
		 */
		if ( SetUpIntData( &CopIntData ) ) {
		    AddCopInt( disp_def, &ucl, &CopInt, &CopIntData );
		    ScreenToFront( CDXLScreen );
		    return( RC_OK );
		}
	    }
	    if ( disp_def->dbuf ) {
		FreeDBufInfo( disp_def->dbuf );
	    }
	    CloseWindow( CDXLWindow );
	}

	CloseScreen( CDXLScreen );
	CDXLScreen = NULL;

	if ( disp_def->Flags & DISP_ALLOCBM ) {
	    FreeBitMap( disp_def->bm[0] );
	    FreeBitMap( disp_def->bm[1] );
	    disp_def->bm[0] = disp_def->bm[1] = NULL;
	}
	return( RC_NO_WIN );
    }

    if ( disp_def->Flags & DISP_ALLOCBM ) {
	FreeBitMap( disp_def->bm[0] );
	FreeBitMap( disp_def->bm[1] );
	disp_def->bm[0] = disp_def->bm[1] = NULL;
    }

    return( RC_NO_SC );

} // ScrWinOpen()



/*********

 View Stuff

**********/

struct View		  View,* OldView;
struct ViewPort 	  ViewPort;
struct RasInfo		  RasInfo;
struct MonitorInfo	  Monitor;
struct ColorMap		* CMap;
struct ViewPortExtra	* Vpx;
struct ViewExtra	* Vx;


struct ViewExtra *
GetVX( ULONG ModeID )
{
    struct ViewExtra	* vx;

    if ( vx = GfxNew( VIEW_EXTRA_TYPE ) ) {
	if ( GetDisplayInfoData( NULL, (UBYTE *)&Monitor, sizeof(Monitor), DTAG_MNTR, ModeID ) ) {
	    vx->Monitor = Monitor.Mspc;
	}
    }

    return( vx );

} // GetVX()



/*
 * Open a view as defined by disp_def. Optionally
 * load an ILBM file into background.
 */
ViewOpen( DISP_DEF * disp_def, UBYTE * ilbmfile )
{
    ULONG	flags;

    GetModeID( disp_def );

    /*
     * If we are opening a full sized display as opposed to one
     * that is just the size of the CDXL dimensions.
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	flags = BMF_CLEAR|BMF_DISPLAYABLE;

	if ( disp_def->Flags & DISP_INTERLEAVED )
	    flags |= BMF_INTERLEAVED;

	if (!(disp_def->bm[0] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 1\n");)
	    return( RC_NO_MEM );
	}

	if (!(disp_def->bm[1] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, disp_def->bm[0] )) ) {

	    D(PRINTF("Could NOT AllocBitMap() 2\n");)
	    FreeBitMap( disp_def->bm[0] );
	    disp_def->bm[0] = NULL;
	    return( RC_NO_MEM );
	}
    }

    if ( CMap = GetColorMap( MAX ( (disp_def->Depth<<1), 256 ) ) ) {
	InitView( &View );

	if ( !(disp_def->Flags & DISP_ALLOCBM ) ) {
	    View.DxOffset += disp_def->Left;
	    View.DyOffset += disp_def->Top;
	}

	ViewPort.ColorMap = CMap;
	ViewPort.DHeight = disp_def->Height;
	ViewPort.DWidth = disp_def->Width;
	ViewPort.Modes = disp_def->ModeID;
	ViewPort.RasInfo = &RasInfo;
	RasInfo.BitMap = disp_def->bm[0];
	ViewPort.Next = View.ViewPort;
	View.ViewPort = &ViewPort;

	if ( Vx = GetVX( disp_def->ModeID ) )
	    GfxAssociate(&View,Vx);

	if ( Vpx = GfxNew(VIEWPORT_EXTRA_TYPE) ) {

	    VideoControlTags(
		CMap,VTAG_VIEWPORTEXTRA_SET,Vpx,
		VTAG_ATTACH_CM_SET,&ViewPort,
		VC_IntermediateCLUpdate,0,
		VC_IntermediateCLUpdate_Query,&smartgfx,
		TAG_END
	    );

	    smartgfx = ( smartgfx == 0x12345678 ) ? 0 : -1;
	    kprintf("\nsmartgfx= '%ls'\n\n",
		smartgfx ? "SMART" : "DUMB" );

	    disp_def->vp = &ViewPort;

	    if ( SetUpIntData( &CopIntData ) ) {
		AddCopInt( disp_def, &ucl, &CopInt, &CopIntData );
		MakeVPort( &View, &ViewPort );
		MrgCop( &View );

		if ( disp_def->dbuf = AllocDBufInfo( disp_def->vp ) ) {
		    if ( (disp_def->Flags & DISP_BACKGROUND) && ilbmfile && *ilbmfile ) {

			if ( DoILBM( ilbmfile, disp_def ) )
			    printf("Could not open '%ls'\n",ilbmfile);
		    }

		    OldView = GfxBase->ActiView;
		    LoadView( &View );

		    return( RC_OK );
		}
	    }
	    CloseDisplay( disp_def );
	    return( RC_NO_MEM );
	}

	FreeColorMap( CMap );
	CMap = NULL;
    }

    if ( disp_def->Flags & DISP_ALLOCBM ) {
	FreeBitMap( disp_def->bm[0] );
	FreeBitMap( disp_def->bm[1] );
	disp_def->bm[0] = disp_def->bm[1] = NULL;
    }

    return( RC_NO_MEM );

} // ViewOpen()



/* 
 * Open either a screen or a view.
 */
OpenDisplay( DISP_DEF * disp_def, UBYTE * ilbmfile )
{
    if ( disp_def->Flags & DISP_SCREEN ) {
	return( ScrWinOpen( disp_def, ilbmfile ) );
    } else {
	return( ViewOpen( disp_def, ilbmfile ) );
    }

} // OpenDisplay()


/* 
 *  Close screen or view.
 */
VOID
CloseDisplay( DISP_DEF * disp_def )
{

    if ( CopIntAdded ) {
	RemIntServer( INTB_COPER, &CopInt );
	disp_def->vp->UCopIns = old_ucl;
	RethinkDisplay();
	FreeCopList( ucl.FirstCopList );
	CopIntAdded = FALSE;
    }

    if ( CopSigBit != -1 ) {
	FreeSignal( CopSigBit );
	CopSigBit = -1;
    }

    if ( disp_def->dbuf )
	FreeDBufInfo( disp_def->dbuf );

    /* 
     * Close the screen and window.
     */
    if ( disp_def->Flags & DISP_SCREEN ) {
	if ( CDXLWindow ) {
	    CloseWindow( CDXLWindow );
	    CDXLWindow = NULL;
	}

	if ( CDXLScreen ) {
	    CloseScreen( CDXLScreen );
	    CDXLScreen = NULL;
	}
    } else {
	/* 
	 * We had a view not a screen.
	 */
	if ( OldView ) {
	    LoadView( OldView );
	    OldView = NULL;
	}

	if ( View.ViewPort )
	    FreeVPortCopLists( View.ViewPort );

	if ( Vpx ) {
	    GfxFree( Vpx );
	    Vpx = NULL;
	}

	if ( Vx ) {
	    GfxFree( Vx );
	    Vx = NULL;
	}

	if ( View.LOFCprList ) {
	    FreeCprList( View.LOFCprList );
	    View.LOFCprList = NULL;
	}

	if ( CMap ) {
	    FreeColorMap( CMap );
	    CMap = NULL;
	}
    }

    /* 
     * If we allocated these bitmaps with AllocBitMap(),
     * free them with FreeBitMap().
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	if ( disp_def->bm[0] )
	    FreeBitMap( disp_def->bm[0] );

	if ( disp_def->bm[1] )
	    FreeBitMap( disp_def->bm[1] );
    }

} // CloseDisplay()


VOID
Display2Front( DISP_DEF * disp_def )
{
    if ( disp_def->Flags & DISP_SCREEN ) {
	if ( CDXLScreen )
	    ScreenToFront( CDXLScreen );

    } else {
	if ( View.LOFCprList ) {
	    OldView = GfxBase->ActiView;
	    LoadView( &View );
	}
    }

} // Display2Front()

