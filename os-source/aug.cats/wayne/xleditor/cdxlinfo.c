/*******************

    CDXLInfo.c

   W.D.L 930603

********************/


#include <exec/types.h>
#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuitionbase.h>


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


#define	INFO_TOP	(Topaz80.ta_YSize + 5)


#define INFO_GAD_LEFT	15
#define	INFO_GAD_TOP	(WIN_HT - INFO_GAD_HT - 4)
#define	INFO_GAD_HT	14

#define INFO_OK_ID	50
#define INFO_CANCEL_ID	51

//#define	WIN_HT		53
#define	WIN_HT		(53 - INFO_GAD_HT)
#define	WIN_WID		wid

#define	INFOTEXT	"Width Height Depth FrameSize ImageSize AudioSize CMapSize Frames"

IMPORT struct TextAttr		  Topaz80;

IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct Library		* GadToolsBase;
IMPORT struct GfxBase		* GfxBase;

struct Gadget * GadCreate( struct NewGadget * ng, struct VisualInfo * vi,
	struct Gadget * last, ULONG kind, ULONG tag, ... );


XLInfo( XLEDIT * xledit, struct Window * win )
{
    struct Requester	  sleepReq;
    BOOL		  sleeping;
    struct NewGadget	  ng;
    struct IntuiText	  it;
    int			  i,wid,ret = RC_OK,panelY;
    BOOL		  quit = FALSE;
    UBYTE		  buf[100];
    UBYTE		* str;
    struct Window	* infowin;
    struct IntuiMessage	* imsg;
    struct Gadget	* prev,* glist;
    struct VisualInfo	* vi;
    struct Image	* frame;

    if ( !(vi = GetVisualInfo( win->WScreen, TAG_DONE ) ) ) {
	DisplayBeep( NULL );
	return( FALSE );
    }

    kprintf("XLInfo() 1\n");

    InitRequester( &sleepReq );
    sleeping = Request( &sleepReq, win );
    SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof ( it ), 0 );

    kprintf("XLInfo() 2\n");

#ifdef	OUTT	// [
    prev = CreateContext( &glist );

    kprintf("XLInfo() prev= 0x%lx\n",prev);

    ng.ng_LeftEdge = INFO_GAD_LEFT;
    ng.ng_TopEdge = INFO_GAD_TOP;
    ng.ng_Height = INFO_GAD_HT;
    ng.ng_TextAttr = it.ITextFont = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;

    ng.ng_GadgetText = it.IText = "_OK";
    ng.ng_Width = IntuiTextLength( &it ) + 8;
    ng.ng_GadgetID = INFO_OK_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GT_Underscore,	'_', 
	TAG_DONE );
/*
    ng.ng_GadgetText = it.IText = "_Cancel";
    ng.ng_Width = IntuiTextLength( &it ) + 8;
    ng.ng_LeftEdge = WIN_WID - ng.ng_Width - INFO_GAD_LEFT;
    ng.ng_GadgetID = INFO_CANCEL_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );
*/
#endif		// ]

    it.IText = INFOTEXT;
    it.LeftEdge = INFO_GAD_LEFT;
    it.TopEdge = INFO_TOP;
    it.DrawMode = JAM1;
    it.FrontPen = 1;
    wid = IntuiTextLength( &it ) + (2*INFO_GAD_LEFT);

    if ( /*prev &&*/ (infowin = OpenWindowTags( NULL,
	WA_Width,		WIN_WID,
//	WA_Height,		ng.ng_TopEdge + ng.ng_Height + 4,
	WA_Height,		WIN_HT,
	WA_Left,		win->MouseX - (WIN_WID >> 1),
	WA_Top,			20,
	WA_IDCMP,		IDCMP_GADGETUP|IDCMP_CLOSEWINDOW,
	WA_Activate,		TRUE,
	WA_CustomScreen,	win->WScreen,
//	WA_Gadgets,		glist,
	WA_DragBar,		TRUE,
	WA_DepthGadget,		TRUE,
	WA_CloseGadget,		TRUE,
	WA_RMBTrap,		TRUE,
	WA_Title,		"CDXL Info",
	TAG_DONE)) ) {

	PrintIText( infowin->RPort, &it, 0, 0 );

	if ( frame = NewObject( NULL, "frameiclass",
	    IA_FrameType, FRAME_DEFAULT,
	    IA_Recessed, TRUE,
	    IA_Width, infowin->Width - (2*INFO_GAD_LEFT),
//	    IA_Height, (WIN_HT - INFO_TOP - Topaz80.ta_YSize - 4),
	    IA_HEIGHT, Topaz80.ta_YSize + 3,
	    TAG_DONE ) ) {

	    DrawImage( infowin->RPort, frame, INFO_GAD_LEFT, INFO_TOP + Topaz80.ta_YSize+1 );
	    WaitBlit();
	    DisposeObject( frame );
	}

	str = INFOTEXT;
	i = 0;

	it.IText = buf;
	it.TopEdge = INFO_TOP + Topaz80.ta_YSize + 3;
	it.LeftEdge += 8;

	sprintf( buf, "%ld", xledit->cdxlob->rect.MaxX - xledit->cdxlob->rect.MinX );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, str, 6);
	sprintf( buf, "%ld", xledit->cdxlob->rect.MaxY - xledit->cdxlob->rect.MinY );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=6], 7);
	sprintf( buf, "%ld", xledit->cdxlob->bm[0].Depth );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=7], 6);
	sprintf( buf, "%ld", xledit->cdxlob->FrameSize );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=6], 10);
	sprintf( buf, "%ld", xledit->cdxlob->ImageSize );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=6], 10);
	sprintf( buf, "%ld", xledit->cdxlob->AudioSize );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=10], 10);
	sprintf( buf, "%ld", xledit->cdxlob->CMapSize );
	PrintIText( infowin->RPort, &it, 0, 0 );

	it.LeftEdge += TextLength( infowin->RPort, &str[i+=10], 9);
	sprintf( buf, "%ld", xledit->cdxlob->NumFrames );
	PrintIText( infowin->RPort, &it, 0, 0 );

	panelY = SetHomePos( infowin->TopEdge + infowin->Height );

	HomePanel();

	while ( !quit ) {
	    Wait( 1 << infowin->UserPort->mp_SigBit );

	    while ( imsg = GT_GetIMsg( infowin->UserPort ) ) {

		switch ( imsg->Class )  {
		    case IDCMP_CLOSEWINDOW:
			quit = TRUE;
			ret = FALSE;
			break;


		    case IDCMP_GADGETUP:
			D(PRINTF("%ld\n",((struct Gadget *)imsg->IAddress)->GadgetID);)

			switch ( ((struct Gadget *)imsg->IAddress)->GadgetID ) {

			    case INFO_OK_ID:
				quit = TRUE;
				ret = TRUE;
				break;

			    case INFO_CANCEL_ID:
				quit = TRUE;
				ret = FALSE;
				break;
			}
		}
		GT_ReplyIMsg(imsg);

		if (quit)
		    break;
	    }

	}

	Forbid();
	StripIntuiMessages( infowin->UserPort, infowin );
	ModifyIDCMP( infowin, 0L );
	Permit();
	CloseWindow( infowin );

	SetHomePos( panelY );

    } else {
/*
	if ( !prev ) {
	    ret = RC_NO_MEM;
	} else*/ {
	    ret = RC_NO_WIN;
	}
	DisplayBeep( win->WScreen );
    }
/*
    if ( glist )
	FreeGadgets( glist );
*/
    if ( sleeping )
	EndRequest( &sleepReq, win );

    ClearPointer( win );

    HomePanel();

    FreeVisualInfo( vi );

    return( ret );

} // XLInfo()


