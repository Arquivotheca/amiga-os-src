/***********

    XlEdit.c

    W.D.L 930518

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
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <utility/tagitem.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>

#include <libraries/asl.h>

#include "devices/cd.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/blitdef.h"
#include "cdxl/xledit.h"
#include "cdxl/runcdxl_protos.h"

#include "xledit_rev.h"

#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"


#define GAD_LEFT	15
#define	GAD_TOP		6
#define	GAD_HT		14

#define	PANEL_HT	140
#define	PANEL_DEPTH	2
#define	PANEL_HOME	(GAD_HT+(2*GAD_TOP))

#define	XL_TOP_BORDER	4

#define SCREEN_MODE_ID	100
#define OPEN_ID		101
#define PLAY_ID		102
#define QUIT_ID		103
#define STILL_ID	104
#define FRAME_ID	105
#define FRAME_DISP_ID	106
#define STEP_L_ID	107
#define STEP_R_ID	108
#define INFO_ID		109
#define SAVE_ID		110
#define OPEN_NEW_ID	111
#define CLOSE_ID	112
#define SAVE_ILBM_ID	113
#define SAVE_VID_RAW_ID	114
#define SAVE_AUD_8SVX_ID 115
#define SAVE_AUD_RAW_ID	116
#define FRAME_LOAD_ID	117
#define FRAME_DEL_ID	118
#define FRAME_UNDEL_ID	119
#define SAVE_AS_ID	120
#define CREATE_ID	121
#define SAVE_AS_FLIK_ID	122





struct Gadget * FrameGad;

int init( BOOL iff );
VOID closedown( VOID );
ResetIntData( DISP_DEF * );
VOID UpDateFrame( ULONG Frame );
VOID SelectXlEdit( XLEDIT * xledit );

BOOL	DisplayIsPal;

// argument parsing 
#define TEMPLATE    "FROM/A,X/K/N,Y/K/N,VOL/K/N,VIEW/S,BLIT/S,BACK/K,MULTIPAL/S,XLSPEED/K/N,NOXLEEC/S,XLPAL/S,LACE/S,NONLACE/S,HIRES/S,LORES/S,BOXIT/S,SDBL/S,NTSC/S,PAL/S,DEFMON/S,NOPOINTER/S,XLMODEID/S,ENDDELAY/K/N,LOOP/K/N,CDXL/S,DOSXL/S,NOPROMOTE/S" VERSTAG "Wayne D. Lutz ...In House Testing Version..."
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
#define	OPT_SDBL	16
#define	OPT_NTSC	17
#define	OPT_PAL 	18
#define	OPT_DEFMON	19
#define	OPT_NOPOINT	20
#define	OPT_XLMODEID	21
#define	OPT_ENDDELAY	22
#define	OPT_LOOP	23
#define	OPT_CDXL	24
#define	OPT_DOSXL	25
#define	OPT_NOPROMOTE	26
#define	OPT_COUNT	27

IMPORT CDXLOB	* CDXL_OB;	// Global CDXLOB
DISP_DEF	* DisplayDef;	// Global DISP_DEF
LONG		  RetCode;	// Global return code

//XLEDIT		* XlCur;

XLCONTROL	  XlControl;

IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct GfxBase		* GfxBase;
IMPORT ULONG  			  CopSignal;

struct Library			* GadToolsBase;
struct Library			* AslBase;

struct Task			* CDXLTask,* parent;
ULONG				  ParentSig,RexxSig;

STATIC struct Image		* PanelFrame;
STATIC struct FileRequester	* FileRequester;


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;

struct Library * UtilityBase;


int CXBRK(void) { return(0); }		/* Disable SASC CTRL/C handling */
int chkabort(void) { return(0); }	/* Indeed */

UBYTE	FileNameBuf[256];

// Error messages.
STATIC UBYTE * XLError[] = {
    "OK",
    "Required filename missing",
    "Error while reading file",
    "Couldn't open file",
    "Not enough memory for operation",
    "Could not open cd/cdtv device",
    "Could not open audio device",
    "Could not open window",
    "Could not open screen",
    "Specified CDXL file is not a standard PAN file",
    "Operation failed",
    "Error while writting file",
    "Supplied filename is too long",
    "Can't open file",
};

struct TextAttr Topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

STATIC
UWORD pens[] =
{
	0, /* DETAILPEN */
	1, /* BLOCKPEN	*/
	1, /* TEXTPEN	*/
	2, /* SHINEPEN	*/
	1, /* SHADOWPEN	*/
	3, /* FILLPEN	*/
	1, /* FILLTEXTPEN	*/
	0, /* BACKGROUNDPEN	*/
	2, /* HIGHLIGHTTEXTPEN	*/

	1, /* BARDETAILPEN	*/
	2, /* BARBLOCKPEN	*/
	1, /* BARTRIMPEN	*/

	(UWORD)~0,
};

STATIC struct NewMenu PanelNM[] = 
{
    { NM_TITLE, "Project", NULL, NULL, NULL, NULL }, 
	{ NM_ITEM, "Open...", "O", NM_ITEMDISABLED, NULL, (APTR)OPEN_ID },
	{ NM_ITEM, "Open New...", "N", NULL, NULL, (APTR)OPEN_NEW_ID },
	{ NM_ITEM, "Save", "S", NM_ITEMDISABLED, NULL, (APTR)SAVE_ID },
	{ NM_ITEM, "Save As...", "A", NULL, NULL, (APTR)SAVE_AS_ID },
	{ NM_ITEM, "Create...", "R", NULL, NULL, (APTR)CREATE_ID },
	{ NM_ITEM, "Save As .Flik...", "R", NULL, NULL, (APTR)SAVE_AS_FLIK_ID },
	{ NM_ITEM, NM_BARLABEL },
	{ NM_ITEM, "Close", "C", NULL, NULL, (APTR)CLOSE_ID },
	{ NM_ITEM, "Quit", "Q", NULL, NULL, (APTR)QUIT_ID },
    { NM_TITLE, "Frame", NULL, NULL, NULL, NULL }, 
	{ NM_ITEM, "Save   ", NULL, NULL, NULL, NULL },
	    { NM_SUB, "Video as ILBM... ", "I", NULL, NULL, (APTR)SAVE_ILBM_ID },
	    { NM_SUB, "Video as RAW... ", "R", NM_ITEMDISABLED, NULL, (APTR)SAVE_VID_RAW_ID },
	    { NM_SUB, "Audio as 8SVX... ", "V", NULL, NULL, (APTR)SAVE_AUD_8SVX_ID },
	    { NM_SUB, "Audio as RAW... ", "A", NM_ITEMDISABLED, NULL, (APTR)SAVE_AUD_RAW_ID },
	{ NM_ITEM, "Load", "L", NM_ITEMDISABLED, NULL, (APTR)FRAME_LOAD_ID },
	{ NM_ITEM, "Delete", "D", NULL, NULL, (APTR)FRAME_DEL_ID },
	{ NM_ITEM, "UnDelete", "U", NULL, NULL, (APTR)FRAME_UNDEL_ID },

    { NM_END },
};

struct Screen		* PanelScreen;
struct Window		* PanelWindow;
struct VisualInfo	* PanelVI;
struct Gadget		* PanelGlist;
struct Menu		* PanelMenu;


UBYTE *
ErrorString( int rc )
{

    if ( rc >= (sizeof (XLError)/sizeof (UBYTE *)) ) {
	rc = RC_FAILED;
    }

    return( XLError[ rc ] );

} // ErrorString()

/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * ( note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it )
 */
VOID
StripIntuiMessages( struct MsgPort * mp, struct Window * win )
{
    struct IntuiMessage *msg;
    struct Node *succ;

    if ( !(mp && win ) )
	return;

    msg = ( struct IntuiMessage * ) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ ) {

	if( msg->IDCMPWindow ==  win ) {

	    /* Intuition is about to free this message.
	     * Make sure that we have politely sent it back.
	     */
	    Remove( ( struct Node * )msg );

	    ReplyMsg( ( struct Message * )msg );
	}
	    
	msg = ( struct IntuiMessage * ) succ;
    }

} // StripIntuiMessages()


VOID
ClosePanel( VOID )
{

    if ( FileRequester ) {
	FreeAslRequest(FileRequester);
	FileRequester = NULL;
    }

    if ( PanelFrame ) {
	DisposeObject( PanelFrame );
	PanelFrame = NULL;
    }

    if ( PanelWindow ) {
	if ( PanelMenu ) {
	    ClearMenuStrip( PanelWindow );
	    FreeMenus( PanelMenu );
	    PanelMenu = NULL;
	}
	Forbid();
	StripIntuiMessages( PanelWindow->UserPort, PanelWindow );
	ModifyIDCMP( PanelWindow, 0L );
	Permit();
	CloseWindow( PanelWindow );
	PanelWindow = NULL;
    }

    if ( PanelGlist ) {
	FreeGadgets( PanelGlist);
	PanelGlist = NULL;
    }

    if ( PanelScreen ) {
	if ( PanelVI ) {
	    FreeVisualInfo( PanelVI );
	    PanelVI = NULL;
	}
	CloseScreen( PanelScreen );
	PanelScreen = NULL;
    }

} // ClosePanel()


struct Gadget	*
TraverseGList ( ULONG ID, struct Gadget * GList)
{
    while (GList) {
	if (GList->GadgetID == ID)	
	    return( GList );

	GList = GList->NextGadget;
    }		

    return ( NULL );

} // TraverseGList()


struct Gadget	*
GetGadget( struct Window * Window, ULONG ID )
{
    return ( TraverseGList ( ID, Window->FirstGadget ) );

} // GetGadget()


VOID
OffGad( struct Gadget * gad, struct Window * win, struct Requester * req )
{
    if ( !(gad->Flags & GADGDISABLED) )
	OffGadget( gad, win, req );

} // OffGad()


VOID
OnGad( struct Gadget * gad, struct Window * win, struct Requester * req )
{
    if ( gad->Flags & GADGDISABLED )
	OnGadget( gad, win, req );

} // OnGad()


struct Gadget *
GadCreate( struct NewGadget * ng, struct VisualInfo * vi, struct Gadget * last,
 ULONG kind, ULONG tag, ... )
{
	ng->ng_VisualInfo = vi;

	return( CreateGadgetA(kind,last,ng,(struct TagItem *)&tag) );

} // GadCreate()


PrepMenu( struct VisualInfo * vi, struct Window * win )
{
    D(PRINTF("PrepMenu ENTERED\n");)

    if ( PanelMenu = CreateMenus( PanelNM, TAG_END ) ) {
	D(PRINTF("PrepMenu 1\n");)

	LayoutMenus( PanelMenu, vi, GTMN_NewLookMenus, TRUE, TAG_END );
	D(PRINTF("PrepMenu 2\n");)

	SetMenuStrip( win, PanelMenu );
	D(PRINTF("PrepMenu RC_OK\n");)
	return( RC_OK );
    }

    D(PRINTF("PrepMenu RC_NO_MEM\n");)

    return( RC_NO_MEM );

} // PrepMenu()


PrepGadgets( struct VisualInfo * vi, struct Gadget **glist )
{
    struct Gadget	* prev;
    struct NewGadget	  ng;
    struct IntuiText	  it;

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof ( it ), 0 );

    prev = CreateContext(glist);

    ng.ng_LeftEdge = GAD_LEFT;
    ng.ng_TopEdge = GAD_TOP;
//    ng.ng_Width = 120;
    ng.ng_Height = GAD_HT;
    ng.ng_TextAttr = it.ITextFont = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;

    ng.ng_GadgetText = it.IText = "_Quit";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = QUIT_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_Open";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = OPEN_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_Play";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = PLAY_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore, 	'_', 
	TAG_DONE );

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_Still";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = STILL_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore,	'_', 
	TAG_DONE );


    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "Step _<";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = STEP_L_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore,	'_', 
	TAG_DONE );

    if ( prev ) {
	prev->Activation |= GADGIMMEDIATE;
    }

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "Step _>";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = STEP_R_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore,	'_', 
	TAG_DONE );

    if ( prev ) {
	prev->Activation |= GADGIMMEDIATE;
    }

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_Info.";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = INFO_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore,	'_', 
	TAG_DONE );

    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_Frame";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = FRAME_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GA_Disabled,	TRUE,
	GT_Underscore,	'_', 
	TAG_DONE );

    ng.ng_GadgetText = it.IText = "";
//    ng.ng_LeftEdge += (ng.ng_Width + 12 + IntuiTextLength( &it ));
    ng.ng_LeftEdge += ng.ng_Width;
    ng.ng_Width = 32 + 16;
    ng.ng_GadgetID = FRAME_DISP_ID;
//    ng.ng_Flags = PLACETEXT_LEFT;
#ifdef	OUTT	// [
    prev = GadCreate( &ng, vi, prev, INTEGER_KIND, 
	GA_Disabled,		TRUE,
	GTIN_MaxChars,		4,
	STRINGA_Justification,	STRINGCENTER,
	TAG_DONE );
#else		// ][

    prev = GadCreate( &ng, vi, prev, NUMBER_KIND, 
	GTNM_Number,		0,
	GTNM_Border,		TRUE,
	GTNM_Justification,	GTJ_CENTER,
	GTNM_Format,		"%ld",
	GTNM_FrontPen,		1,
	GTNM_BackPen,		0,
	TAG_DONE );
#endif		// ]

    FrameGad = prev;

    if ( !prev ) {
	FreeGadgets( *glist );
	*glist = NULL;
	return( FALSE );
    }

    return( TRUE );

} // PrepGadgets()


STATIC LONG PanelY = PANEL_HOME;

SetHomePos( int y )
{
    int OldY = PanelY;

    PanelY = y;

    return( OldY );

} // SetHomePos()


VOID
HomePanel( VOID )
{
    int ht = DisplayIsPal ? PAL_HEIGHT : NTSC_HEIGHT;

    ht >>= 1;

    ScreenPosition( PanelScreen, SPOS_ABSOLUTE, 
	PanelScreen->LeftEdge, ht-PanelY, 0, 0);

    D(PRINTF("HomePanel() ht= %ld, PanelY= %ld, ht-PanelY= %ld\n",
	ht,PanelY,ht-PanelY);)

} // HomePanel()


OpenPanel( VOID )
{
    struct ColorSpec	colors[] = 
	{
	    {0,0xA,0xB,0xC},
	    {1,0x0,0x0,0x0},
	    {2,0xF,0xF,0xF},
	    {3,0x0,0x8,0xF},
	    {-1,0,0,0}
	};


    if ( PanelScreen = OpenScreenTags(NULL,
//	SA_Top,		PANEL_HOME,
	SA_Height,	PANEL_HT,
	SA_Width,	640,
	SA_Depth,	PANEL_DEPTH,
	SA_DisplayID,	HIRES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	SA_Colors,	colors,
	SA_Pens,	pens,
	SA_AutoScroll,	TRUE,
	TAG_DONE) )
    {

	D(PRINTF("OpenPanel() PanelScreen->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, Depth= %ld, PanelScreen= 0x%lx\n",
	    PanelScreen->LeftEdge,PanelScreen->TopEdge,PanelScreen->Width,
	    PanelScreen->Height,PanelScreen->RastPort.BitMap->Depth,PanelScreen);)


	PanelVI = GetVisualInfo( PanelScreen, TAG_DONE );
	if ( PanelVI && PrepGadgets( PanelVI, &PanelGlist ) &&
	   ( PanelWindow = OpenWindowTags( NULL,
	    WA_Width,		PanelScreen->Width,
	    WA_Height,		PanelScreen->Height,
	    WA_IDCMP,		IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK,
	    WA_Flags,		BACKDROP | BORDERLESS,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	PanelScreen,
	    WA_Gadgets,		PanelGlist,
	    WA_NewLookMenus,	TRUE,
	    TAG_DONE) )) {

	    if ( PanelFrame = NewObject( NULL, "frameiclass",
		IA_FrameType, FRAME_ICONDROPBOX,
		IA_Recessed, TRUE,
		IA_Width, PanelWindow->Width,
		IA_Height, (2*GAD_TOP)+GAD_HT,
		TAG_DONE ) ) {

		DrawImage( PanelWindow->RPort, PanelFrame, 0, 0 );
		RefreshGList( PanelGlist, PanelWindow, NULL, -1 );
	    }

	    D(PRINTF("OpenPanel() PanelWindow->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, WScreen= 0x%lx\n",
		PanelWindow->LeftEdge,PanelWindow->TopEdge,PanelWindow->Width,
		PanelWindow->Height,PanelWindow->WScreen);)

	    if ( !PrepMenu( PanelVI, PanelWindow ) ) {
		HomePanel();
		ScreenToFront( PanelScreen );
		return( RC_OK );
	    }
	}

//	CloseScreen( PanelScreen );
//	PanelScreen = NULL;
	ClosePanel();

	return( RC_NO_WIN );
    }

    return( RC_NO_SC );

} // OpenPanel()


/***********************************************************************/
/* We append 'name' onto 'path', adding a slash if necessary.
** Returns FALSE if not able to append within size constraints.
*/

BOOL
AppendPath( UBYTE * path, UBYTE * name, LONG len )
{
    UBYTE	lastch = 0;

    if ( ( strlen(path) + strlen(name) + 2 ) > len )
	return ( FALSE );

    if ( *path )  {
	lastch = path[ strlen(path)-1 ];
	if ( (lastch != ':') && (lastch != '/') )
	    strcat( path, "/");
    }

    strcat( path, name );
    return ( TRUE );

}  /* AppendPath() */


#define	CALC_GAD_LEFT	7
#define	CALC_GAD_TOP	(Topaz80.ta_YSize + 5)
#define	CALC_GAD_HT	(GAD_HT - 3)
#define	CALC_GAD_WID	(IntuiTextLength( &it ) + 16)

#define	CALC_STR_GAD_ID	50
#define	OK_ID		51
#define	CANCEL_ID	52
#define	ALL_CLEAR_ID	53
#define	BACK_ID		54

CalcReq( struct Window * window, struct VisualInfo * vi, LONG * retval )
{
    struct Window	* win;
    struct Gadget	* glist = NULL,* prev,* gad;
    UBYTE		* ptr,* buttontext[] = {"1","2","3","4","5","6","7","8","9","0","."};
    struct IntuiMessage	* imsg;
    struct NewGadget	  ng;
    struct IntuiText	  it;
    BOOL		  quit = FALSE;
    struct Requester	  sleepReq;
    BOOL		  sleeping;
    int			  i,wid,pos,buflen,panelY,ret = FALSE;

    InitRequester( &sleepReq );
    sleeping = Request( &sleepReq, window );
    SetWindowPointer( window, WA_BusyPointer, TRUE, TAG_DONE );

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof ( it ), 0 );

    prev = CreateContext( &glist );

    ng.ng_LeftEdge = CALC_GAD_LEFT;
    ng.ng_Height = CALC_GAD_HT + 1;
    ng.ng_TextAttr = it.ITextFont = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;
    ng.ng_TopEdge = CALC_GAD_TOP;
    ng.ng_GadgetText = "";
    it.IText = "1";
    ng.ng_Width = 4 * CALC_GAD_WID;
    ng.ng_GadgetID = CALC_STR_GAD_ID;

    prev = GadCreate( &ng, vi, prev, STRING_KIND, 
	STRINGA_MaxChars,	8,
	STRINGA_Justification,	STRINGCENTER,
	STRINGA_TextVal,	"0",
	TAG_DONE );

    ((struct StringInfo *)prev->SpecialInfo)->Buffer[0] = '0';
    ++((struct StringInfo *)prev->SpecialInfo)->BufferPos;

    ng.ng_Height = CALC_GAD_HT;

    ng.ng_TopEdge = CALC_GAD_TOP + (3 * CALC_GAD_HT) + 3;

    for ( i = 0; prev && (i < 9); i++ ) {
	ng.ng_GadgetText = it.IText = buttontext[i];
	ng.ng_Width = CALC_GAD_WID;
	ng.ng_GadgetID = i+1;
	if ( !(prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE )) ) {
	    break;
	}

	if ( (i < 8) && !(ng.ng_GadgetID % 3) ) {
	    ng.ng_LeftEdge = CALC_GAD_LEFT;
	    ng.ng_TopEdge -= CALC_GAD_HT;
	} else {
	    ng.ng_LeftEdge += ng.ng_Width;
	    wid = ng.ng_LeftEdge + ng.ng_Width;
	}
    }

    D(PRINTF("prev= 0x%lx, GadgetText= 0x%lx, IText= '%ls'\n",
	prev,prev->GadgetText,prev->GadgetText->IText);)

    ng.ng_GadgetText = it.IText = "C";
    ng.ng_Width = CALC_GAD_WID;
    ng.ng_GadgetID = ALL_CLEAR_ID;
    ng.ng_Height = CALC_GAD_HT*2;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_TopEdge += ng.ng_Height;
    ng.ng_GadgetText = it.IText = "<";
    ng.ng_Width = CALC_GAD_WID;
    ng.ng_GadgetID = BACK_ID;
    ng.ng_Height = CALC_GAD_HT*2;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_Height = CALC_GAD_HT + 1;

    ng.ng_TopEdge = CALC_GAD_TOP + (5 * CALC_GAD_HT) + 5;
    ng.ng_LeftEdge += ng.ng_Width;
    ng.ng_GadgetText = it.IText = "_Cancel";
    ng.ng_Width = CALC_GAD_WID-10;
    ng.ng_LeftEdge -= ng.ng_Width;
    ng.ng_GadgetID = CANCEL_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    wid = (ng.ng_LeftEdge + ng.ng_Width + CALC_GAD_LEFT); // wind width

    ng.ng_LeftEdge = CALC_GAD_LEFT;
    ng.ng_GadgetText = it.IText = "_OK";
    ng.ng_Width = CALC_GAD_WID-10;
    ng.ng_GadgetID = OK_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );


    ng.ng_Height = CALC_GAD_HT;
    ng.ng_TopEdge = CALC_GAD_TOP + (4 * CALC_GAD_HT) + 3;
    ng.ng_LeftEdge = CALC_GAD_LEFT;
    ng.ng_GadgetText = it.IText = buttontext[i++]; // "0"
    ng.ng_Width = 2*(CALC_GAD_WID);
    ng.ng_GadgetID = i;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_LeftEdge += ng.ng_Width;
    ng.ng_GadgetText = it.IText = buttontext[i++]; // "."
    ng.ng_Width = CALC_GAD_WID;
    ng.ng_GadgetID = i;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND,
	GT_Underscore, '_',
	GA_Disabled,TRUE,
	TAG_DONE );


    if ( prev && (win = OpenWindowTags( NULL,
	WA_Width,		wid,
//	WA_Width,		wid + CALC_GAD_LEFT,
//	WA_Width,		ng.ng_LeftEdge + CALC_GAD_LEFT,
//	WA_Width,		50,
//	WA_Height,		100,
//	WA_Height,		CALC_GAD_TOP + (4 * CALC_GAD_HT) + 6,
//	WA_Height,		ng.ng_TopEdge + ng.ng_Height + 4,
	WA_Height,		CALC_GAD_TOP + (6 * CALC_GAD_HT) + 10,
	WA_Left,		window->MouseX - (wid >> 1),
	WA_Top,			20,
	WA_IDCMP,		IDCMP_GADGETUP|IDCMP_GADGETDOWN|
				IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK|
				IDCMP_CLOSEWINDOW|IDCMP_ACTIVEWINDOW,

//	WA_Flags,		BACKDROP | BORDERLESS,
	WA_Activate,		TRUE,
	WA_CustomScreen,	PanelScreen,
	WA_Gadgets,		glist,
	WA_DragBar,		TRUE,
	WA_DepthGadget,		TRUE,
	WA_CloseGadget,		TRUE,
	WA_RMBTrap,		TRUE,
	TAG_DONE)) ) {

	panelY = SetHomePos( win->TopEdge + win->Height );
	HomePanel();

	    while ( !quit ) {

		ActivateGadget( GetGadget( win, CALC_STR_GAD_ID ), win, NULL );

		Wait( 1 << win->UserPort->mp_SigBit );

		while ( imsg = GT_GetIMsg( win->UserPort ) ) {

		    switch ( imsg->Class )  {
			case IDCMP_CLOSEWINDOW:
			    quit = TRUE;
			    break;

			case IDCMP_GADGETUP:
			    D(PRINTF("%ld\n",((struct Gadget *)imsg->IAddress)->GadgetID);)

			    switch ( ((struct Gadget *)imsg->IAddress)->GadgetID ) {

				case OK_ID:
				case CALC_STR_GAD_ID:

				    gad = GetGadget( win, CALC_STR_GAD_ID );
				    ptr = ((struct StringInfo *)gad->SpecialInfo)->Buffer;

				    if ( *ptr ) {
					*retval = atol( ptr );
				    } else {
					*retval = 0;
				    }

				    quit = TRUE;
				    ret = TRUE;
				    break;

				case CANCEL_ID:
				    quit = TRUE;
				    break;

				case BACK_ID:
				    gad = GetGadget( win, CALC_STR_GAD_ID );
				    if ( !(pos = ((struct StringInfo *)gad->SpecialInfo)->BufferPos ))
					break;
				    ptr = &((struct StringInfo *)gad->SpecialInfo)->Buffer[pos];
				    strcpy(ptr-1,ptr);
				    --((struct StringInfo *)gad->SpecialInfo)->BufferPos;
				    RefreshGList(gad,win,NULL,1);
				    break;

				case ALL_CLEAR_ID:
				    gad = GetGadget( win, CALC_STR_GAD_ID );
				    ((struct StringInfo *)gad->SpecialInfo)->BufferPos = 0;
				    *((struct StringInfo *)gad->SpecialInfo)->Buffer = 0;
				    RefreshGList(gad,win,NULL,1);
				    break;

				default:
				    gad = GetGadget( win, CALC_STR_GAD_ID );
				    buflen = strlen( ((struct StringInfo *)gad->SpecialInfo)->Buffer );
				    D(PRINTF("buflen= %ld\n",buflen);)
				    if ( buflen+1 < ((struct StringInfo *)gad->SpecialInfo)->MaxChars ) {
					for ( i = buflen; i >= ((struct StringInfo *)gad->SpecialInfo)->BufferPos; i-- )
					    ((struct StringInfo *)gad->SpecialInfo)->Buffer[i+1] = ((struct StringInfo *)gad->SpecialInfo)->Buffer[i];

					D(PRINTF("2 ((struct Gadget *)imsg->IAddress)= 0x%lx, ((struct Gadget *)imsg->IAddress)->GadgetText= 0x%lx\n((struct Gadget *)imsg->IAddress)->GadgetText->IText= 0x%lx - '%ls'\nbuttontext[((struct Gadget *)imsg->IAddress)->GadgetID-1]= '%ls'\n",
						((struct Gadget *)imsg->IAddress),
						((struct Gadget *)imsg->IAddress)->GadgetText,
						((struct Gadget *)imsg->IAddress)->GadgetText->IText,
						((struct Gadget *)imsg->IAddress)->GadgetText->IText,
						buttontext[((struct Gadget *)imsg->IAddress)->GadgetID-1]);)

//					((struct StringInfo *)gad->SpecialInfo)->Buffer[ ((struct StringInfo *)gad->SpecialInfo)->BufferPos ] = *((struct Gadget *)imsg->IAddress)->GadgetText->IText;
					((struct StringInfo *)gad->SpecialInfo)->Buffer[ ((struct StringInfo *)gad->SpecialInfo)->BufferPos ] = *buttontext[((struct Gadget *)imsg->IAddress)->GadgetID-1];

					((struct StringInfo *)gad->SpecialInfo)->BufferPos++;

					RefreshGList(gad,win,NULL,1);
				    } else  {
					/* could not insert */
					DisplayBeep( win->WScreen );
				    }

				    break;
			    }
			    break;
		    }
		    GT_ReplyIMsg(imsg);

		    if (quit)
			break;
		}
	    }

	Forbid();
	StripIntuiMessages( win->UserPort, win );
	ModifyIDCMP( win, 0L );
	Permit();
	CloseWindow( win );
	SetHomePos( panelY );
    }

    if ( glist )
	FreeGadgets( glist );

    if ( sleeping )
	EndRequest( &sleepReq, window );

    ClearPointer( window );

    HomePanel();

    return( ret );

} // CalcReq()



FileReq( struct Window * window )
{
    struct Requester		  sleepReq;
    BOOL			  sleeping;
    int				  panelY,ret = FALSE;

    D(PRINTF("FileReq() ENTERED\n");)

    InitRequester( &sleepReq );
    D(PRINTF("FileReq() 1\n");)

    sleeping = Request( &sleepReq, window );
    D(PRINTF("FileReq() 2\n");)

    SetWindowPointer( window, WA_BusyPointer, TRUE, TAG_DONE );
    D(PRINTF("FileReq() 3\n");)

/*
    ScreenPosition( PanelScreen, SPOS_ABSOLUTE, 
	PanelScreen->LeftEdge, PANEL_HT, 0, 0);
*/

    panelY = SetHomePos( PANEL_HT );
    HomePanel();


    D(PRINTF("FileReq() 4\n");)

    if ( FileRequester || (FileRequester = (struct FileRequester *)AllocAslRequest( ASL_FileRequest, TAG_END )) ) {
	D(PRINTF("FileReq() 5\n");)

	if ( AslRequestTags( FileRequester, ASLFR_Screen, window->WScreen, TAG_END ) ) {

            // OK was selected
	    *FileNameBuf = 0;
	    strcpy( FileNameBuf, FileRequester->fr_Drawer );

//	    strcat( FileNameBuf, FileRequester->fr_File );
	    if ( !AppendPath( FileNameBuf, FileRequester->fr_File, sizeof (FileNameBuf) ) )  {
		ret = FALSE; // Filename too long
	    } else {

		D(PRINTF("'%ls' - '%ls', '%ls'\n",
		    FileRequester->fr_Drawer,FileRequester->fr_File,FileNameBuf);)

		ret = TRUE;
	    }

        } else {

            if (IoErr()) {
		D(PRINTF("Could not present FileRequester!\n");)
            } else {
		D(PRINTF("Cancel was selected.\n");)
            }

        }

//        FreeAslRequest(FileRequester);
    } else {
	D(PRINTF("Could not create FileRequester!\n");)
    }

    if ( sleeping )
	EndRequest( &sleepReq, window );

    ClearPointer( window );

    SetHomePos( panelY );
    HomePanel();

    return( ret );

} // FileReq()


VOID
UpDateFrame( ULONG Frame )
{

    GT_SetGadgetAttrs( FrameGad, PanelWindow, NULL, GTNM_Number, Frame, TAG_END  );

} // UpDateFrame()


#define	MIN_ALLOC_SIZE	8

DATA_NODE *
DNodeFree( DATA_NODE * dnode )
{
    DATA_NODE * next;

    D(PRINTF("DNodeFree() 1 dnode= 0x%lx, Size= %ld\n",dnode,dnode->Size);)

    next = dnode->Next;

    FreeMem( dnode, dnode->Size );

    D(PRINTF("DNodeFree() END\n");)

    return( next );

} // DNodeFree()


DATA_NODE *
AllocDataNode( int insize, ULONG flags )
{
    DATA_NODE	* dnode;
    int		  realsize = insize;

    D(PRINTF("AllocDataNode ENTERED with insize= %ld\n",insize);)

    if ( insize % MIN_ALLOC_SIZE )
	realsize = ( (insize / MIN_ALLOC_SIZE) + 1) * MIN_ALLOC_SIZE;

    if ( dnode = AllocMem( realsize, flags ) ) {
	dnode->Size = realsize;
    }

    D(PRINTF("AllocDataNode(): allocated 0x%lx, realsize %ld, dnode->Size %ld\n",
	dnode, realsize, dnode->Size);)

    return( dnode );

} // AllocDataNode()


/****************************************************************************/
/* Returns the preceding item or NULL if no predecessor or item not found.
*/

DATA_NODE *
DNodePred( DATA_NODE * head, DATA_NODE * node )
{
    DATA_NODE	* dn;

    if ( head == node )
	return ( NULL );

    for ( dn = head; dn; dn = dn->Next ) {
	if ( dn->Next == node )
	    return ( dn );
    }

    return ( NULL );

} // DNodePred()


/****************************************************************************/
/* Inserts 'node' into the list after 'pred'.  If pred==NULL, add item at
** head of the list.
*/

VOID
DNodeInsert( DATA_NODE **head, DATA_NODE * node, DATA_NODE * pred )
{

    D(PRINTF("DNodeInsert() ENTERED with head= 0x%lx, node= 0x%lx, pred= 0x%lx\n",
	head,node,pred);)

    D(PRINTF("DNodeInsert() 1 node->Size= 0x%lx\n",node->Size);)

    if ( pred == NULL ) {
	pred = (DATA_NODE *)head;
	D(PRINTF("DNodeInsert() 2 node->Size= 0x%lx\n",node->Size);)
    }

    node->Next = pred->Next;

    pred->Next = node;

} // DNodeInsert()


/****************************************************************************/
/* Unlinks an item from the list.  Returns the preceding item.
*/

DATA_NODE *
DNodeRemove( DATA_NODE **head, DATA_NODE * node )
{
    register DATA_NODE * pred = NULL;

    if ( *head == node ) {
	*head = node->Next;
    } else if ( pred = DNodePred( *head, node ) ) {
	pred->Next = node->Next;
    }

    return (pred);

} // DNodeRemove()


VOID
DNodeListFree( DATA_NODE * dnode )
{
    DATA_NODE	* dnode1;

    while( dnode ) {
	dnode1 = dnode->Next;
	D(PRINTF("DNodeListFree: freeing 0x%lx size %ld\n", dnode, dnode->Size ))
	FreeMem( dnode, dnode->Size );
	dnode = dnode1;
    }

} // DNodeListFree()


VOID
FreeXlEdit( XLEDIT * xledit )
{
    XLEDIT * ptr;

    kprintf("FreeXlEdit() 1 xledit= 0x%lx, Size= %ld, XlHead= 0x%lx, Size= %ld\n",
	xledit,xledit->dnode.Size,XlControl.XlHead,XlControl.XlHead->dnode.Size);

    ptr = (XLEDIT *)DNodeRemove( (DATA_NODE **)&XlControl.XlHead, (DATA_NODE *)xledit );

    kprintf("FreeXlEdit() 2 XlHead= 0x%lx, Size= %ld\n",
	XlControl.XlHead,XlControl.XlHead ? XlControl.XlHead->dnode.Size : -1);

    if ( XlControl.XlCur == xledit ) {
	if ( !(XlControl.XlCur = ptr) )
	    SelectXlEdit( (XLEDIT *)xledit->dnode.Next );
    }

    kprintf("FreeXlEdit() 3 XlHead= 0x%lx, Size= %ld\n",
	XlControl.XlHead,XlControl.XlHead ? XlControl.XlHead->dnode.Size : -1);

    if ( XlControl.XlParent == xledit ) {
	XlControl.XlParent = NULL;
    }

    kprintf("FreeXlEdit() 4 XlHead= 0x%lx, Size= %ld\n",
	XlControl.XlHead,XlControl.XlHead ? XlControl.XlHead->dnode.Size : -1);


    DNodeFree( (DATA_NODE *)xledit );

    kprintf("FreeXlEdit() END XlHead= 0x%lx, Size= %ld\n",
	XlControl.XlHead,XlControl.XlHead ? XlControl.XlHead->dnode.Size : -1);


} // FreeXlEdit()


XLEDIT *
AllocXlEdit( CDXLOB * cdxlob )
{
    XLEDIT	* xledit,txledit;
    LONG	  size;

    size = sizeof (txledit) + (sizeof (*txledit.frameflags) * cdxlob->NumFrames);

    D(PRINTF("AllocXlEdit() size= %ld, sizeof (txledit)= %ld, sizeof (*txledit.frameflags)= %ld, cdxlob->NumFrames= %ld\n",
	size,sizeof (txledit),sizeof (*txledit.frameflags),cdxlob->NumFrames );)

    if ( xledit = (XLEDIT *)AllocDataNode( size, MEMF_CLEAR ) ) {
	xledit->frameflags = (USHORT *)(xledit+1);
    }

    return( xledit );

} // AllocXlEdit()


VOID
ClearIDCMP( struct Window * win )
{
    if ( win ) {
	StripIntuiMessages( win->UserPort, win );
	win->UserPort = NULL;
	ModifyIDCMP( win, 0L );
    }

} // ClearIDCMP()


VOID
SetIDCMP( struct Window * win )
{
    if ( win ) {
	win->UserPort = PanelWindow->UserPort;
	ModifyIDCMP( win, win->IDCMPFlags | IDCMP_ACTIVEWINDOW );
    }

} // SetIDCMP()


VOID
ClearIDCMPAll( VOID )
{
    XLEDIT	* xledit;

    for ( xledit = XlControl.XlHead; xledit; xledit = (XLEDIT *)xledit->dnode.Next ) {
	if ( xledit->disp_def.screen )
	    ClearIDCMP( xledit->disp_def.screen->FirstWindow );
    }

} // ClearIDCMPAll()


XLEDIT *
Win2Xledit( struct Window * win )
{
    XLEDIT		* xledit;
    struct Window	* w;

    for ( xledit = XlControl.XlHead; xledit; xledit = (XLEDIT *)xledit->dnode.Next ) {
	if ( xledit->disp_def.screen ) {
	    for ( w = xledit->disp_def.screen->FirstWindow; w; w = w->NextWindow ) {
		if ( w == win )
		    return( xledit );
	    }
	}
    }

    return( NULL );

} // Win2Xledit()


VOID
SetIDCMPAll( VOID )
{
    XLEDIT	* xledit;

    for ( xledit = XlControl.XlHead; xledit; xledit = (XLEDIT *)xledit->dnode.Next ) {
	if ( xledit->disp_def.screen )
	    SetIDCMP( xledit->disp_def.screen->FirstWindow );
    }

} // SetIDCMPAll()


VOID
EraseXlBorderTop( DISP_DEF * disp_def )
{
    struct Screen	* sc;
    struct Window	* win;
    struct RastPort	* rp;

    if ( !(disp_def && (sc = disp_def->screen) && (win = sc->FirstWindow)) ) {
	DisplayBeep( NULL );
	return;
    }

    rp = &sc->RastPort;

    SetABPenDrMd( rp, 0, 0, JAM2 );

    RectFill( rp, 0, 0, win->Width, min ( XL_TOP_BORDER-2, win->Height ) );
    ChangeVPBitMap(disp_def->vp, disp_def->bm[1], disp_def->dbuf);
    RectFill( rp, 0, 0, win->Width, min ( XL_TOP_BORDER-2, win->Height ) );
    ChangeVPBitMap(disp_def->vp, disp_def->bm[0], disp_def->dbuf);

} // EraseXlBorderTop()


VOID
DrawXlBorderTop( DISP_DEF * disp_def )
{
    struct Screen	* sc;
    struct Window	* win;
    struct ColorMap	* cm;
    struct RastPort	* rp;
    ULONG		  ctable[MAX_CMAP][3],color;
    int			  num,i,pen,y;

    if ( !(disp_def && (sc = disp_def->screen) && (win = sc->FirstWindow)) ) {
	DisplayBeep( NULL );
	return;
    }

    cm = sc->ViewPort.ColorMap;
    num = cm->Count;

    GetRGB32( cm, 0, num, ctable );

    // Get lightest pen
    pen = 0;
    color = 0;
    for ( i = 0; i < num; i++ ) {
	if ( ((ctable[i][0]&0xFF) + (ctable[i][1]&0xFF) + (ctable[i][2]&0xFF)) > color ) {
	    color = ((ctable[i][0]&0xFF) + (ctable[i][1]&0xFF) + (ctable[i][2]&0xFF));
	    pen = i;
	}
    }

    D(PRINTF("DrawXlBorder() pen= %ld, color= 0x%lx: 0x%lx 0x%lx 0x%lx\n",
	pen,color,ctable[pen][0],ctable[pen][1],ctable[pen][2]);)

    rp = &sc->RastPort;

    SetABPenDrMd( rp, pen, pen, JAM2 );

    if ( disp_def == &XlControl.XlCur->disp_def ) {
	y = min ( XL_TOP_BORDER-2, win->Height );
	D(PRINTF("XlCur y= %ld\n",y);)
    } else {
	y = min ( XL_TOP_BORDER-2, win->Height ) >> 1;
	D(PRINTF("!XlCur y= %ld\n",y);)
    }

    RectFill( rp, 0, 0, win->Width, y );
    ChangeVPBitMap(disp_def->vp, disp_def->bm[1], disp_def->dbuf);
    RectFill( rp, 0, 0, win->Width, y );
    ChangeVPBitMap(disp_def->vp, disp_def->bm[0], disp_def->dbuf);

} // DrawXlBorderTop()


VOID
GhostPanel( VOID )
{
    OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
    OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
    OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
    OffGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
    OffGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );

} // GhostPanel()


VOID
SelectXlEdit( XLEDIT * xledit )
{
    XLEDIT * xled;

    if ( XlControl.XlCur != xledit ) {
	xled = XlControl.XlCur;
	XlControl.XlCur = xledit;

	if ( xled ) {
	    EraseXlBorderTop( &xled->disp_def );
	    DrawXlBorderTop( &xled->disp_def );
	}

	if ( XlControl.XlCur ) {
	    DrawXlBorderTop( &xledit->disp_def );

	    if ( XlControl.XlCur->cdxlob->FrameNum < (XlControl.XlCur->cdxlob->NumFrames-1) ) {
		OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
	    } else {
		OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
		OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
	    }

	    if ( XlControl.XlCur->cdxlob->FrameNum > 0 ) {
		OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
	    } else {
		OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
	    }

	    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );
	}
    }

    if ( !XlControl.XlCur )
	GhostPanel();

} // SelectXlEdit()


OpenCDXL( UBYTE * CDXLFile, DISP_DEF * disp_def, CDXLOB **CDXL_ob, ULONG flags, ULONG tag, ... )
{
    int ret;

    D(PRINTF("OpenCDXL ENTERED\n");)
    CDXLTerm( *CDXL_ob );

    D(PRINTF("OpenCDXL 1\n");)
    *CDXL_ob = NULL;

    D(PRINTF("OpenCDXL 2\n");)

    UpDateFrame( 0 );

    if ( disp_def->Flags & DISP_OPEN ) {
	CloseDisplay( disp_def );
	DisplayDef = NULL;	// Clear the Global ptr
    }

    setmem( disp_def, sizeof (DISP_DEF) , 0 );
    disp_def->Flags |= DISP_SCREEN|DISP_ALLOCBM|DISP_INTERLEAVED;

    if ( !(ret = CDXLObtain( CDXLFile, disp_def, CDXL_ob, flags, (struct TagItem *)&tag )) ) {
	struct TagItem	tagitem[] = 
		{
		    {NULL,	NULL},
		    {TAG_END,	NULL},
		};
	struct TagItem	tagmore = {TAG_MORE,NULL};

	if ( !XlControl.XlParent ) {
	    tagitem[0].ti_Tag = SA_FrontChild;
	    tagitem[0].ti_Data = (ULONG)PanelScreen;
	} else {
	    tagitem[0].ti_Tag = SA_Parent;
	    tagitem[0].ti_Data = (ULONG)XlControl.XlParent->disp_def.screen;
	}

	tagmore.ti_Data = (ULONG)tagitem;

	// Make the screen only as tall as it needs to be
	disp_def->Height = (*CDXL_ob)->rect.MaxY;

	/*
	 * Open the display, specifying an optional ilbmfile.
	 */

	CDXL_OB = *CDXL_ob;		// Set the Global ptr

	if ( !(ret = ScrWinOpen( disp_def, NULL, &tagmore ) ) ) {

	    DisplayDef = disp_def;	// Set the Global ptr

	    if ( !(disp_def->Flags & DISP_BACKGROUND) || (disp_def->Flags & DISP_XLPALETTE) ) {
		if ( (*CDXL_ob)->flags & CDXL_FLIK ) {
#ifdef	OUTT	// [
		    LoadRGB32(  disp_def->vp, (ULONG *)(*CDXL_ob)->CMap[0] );
#else		// ][
		    kprintf("Calling the initial LoadRGB32()\n");
		    LoadRGB32(  disp_def->vp, (ULONG *)(*CDXL_ob)->colortable32 );
#endif		// ]
		} else {
		    /*
		     * Since the PAN format that we are using only stores
		     * 4 bits per gun, might as well just use LoadRGB4.
		     */
		    LoadRGB4( disp_def->vp, (*CDXL_ob)->CMap[0], min((*CDXL_ob)->CMapSize >> 1,disp_def->vp->ColorMap->Count) );
		}
	    }

	    disp_def->Flags |= DISP_OPEN;

	    if ( (*CDXL_ob)->flags & CDXL_BLIT ) {
		SHORT DstX = (*CDXL_ob)->rect.MinX;
		SHORT DstY = (disp_def->ModeID & LORESSDBL_KEY) ? ((*CDXL_ob)->rect.MinY>>1) : (*CDXL_ob)->rect.MinY;
		SHORT Wid = ((*CDXL_ob)->rect.MaxX - (*CDXL_ob)->rect.MinX) - (*CDXL_ob)->xoff;
		SHORT Ht = ((*CDXL_ob)->rect.MaxY - (*CDXL_ob)->rect.MinY) - (*CDXL_ob)->yoff;

		BltBitMap( &(*CDXL_ob)->bm[0],0,0,disp_def->bm[0],
		    DstX,DstY,Wid,Ht,0xC0,0xFF,NULL);

	    } else {
		ChangeVPBitMap(disp_def->vp, disp_def->bm[0], disp_def->dbuf);
		ChangeVPBitMap(disp_def->vp, disp_def->bm[1], disp_def->dbuf);
	    }
	}

	SetIDCMP( disp_def->screen->FirstWindow );

    } else {
/*
	OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
	OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
	OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
	OffGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
	OffGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );
*/
    }
    return( ret );

} // OpenCDXL()


#ifndef	OUTT	// [

LoadCDXL( struct Window * win, ULONG flags )
{
    int		  ret = RC_OK,size;
    XLEDIT	  txledit,* xledit;

    if ( FileReq( win ) ) {

	setmem( &txledit, sizeof (txledit), 0 );

	if ( !(ret = OpenCDXL( FileNameBuf, &txledit.disp_def, &txledit.cdxlob, flags, XLTAG_Top, XL_TOP_BORDER )) ) {

	    if ( xledit = AllocXlEdit( txledit.cdxlob ) ) {

		size = xledit->dnode.Size;
		CopyMem( &txledit, xledit, sizeof (XLEDIT) );
		xledit->frameflags = (USHORT *)(xledit+1);
		xledit->dnode.Size = size;

		DNodeInsert( (DATA_NODE **)&XlControl.XlHead, (DATA_NODE *)xledit, (DATA_NODE *)XlControl.XlCur );

		SelectXlEdit( xledit );

		D(PRINTF("LoadCDXL 3 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)

		if ( !XlControl.XlParent ) {
		    XlControl.XlParent = XlControl.XlCur;
		} else {
		    while ( xledit = (XLEDIT *)DNodePred( (DATA_NODE *)XlControl.XlHead, (DATA_NODE *)xledit ) ) {
			ScreenDepth( xledit->disp_def.screen, 
			    SDEPTH_TOBACK|SDEPTH_INFAMILY, NULL );
		    }
		}
		D(PRINTF("LoadCDXL 4 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)

		OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );

	    } else {
		ret = RC_NO_MEM;
		CloseDisplay( &txledit.disp_def );
		CDXLTerm( txledit.cdxlob );
	    }
	}
    }

    if ( ret )
	printf("'%ls'\n",XLError[ret]);

    return( ret );

} // LoadCDXL()

#else		// ][

LoadCDXL( struct Window * win, ULONG flags )
{
    int		  ret = RC_OK;
    XLEDIT	* xledit;

    if ( FileReq( win ) ) {
	if ( xledit = AllocXlEdit() ) {

	    D(PRINTF("LoadCDXL 1 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)

	    DNodeInsert( (DATA_NODE **)&XlControl.XlHead, (DATA_NODE *)xledit, (DATA_NODE *)XlControl.XlCur );
	    D(PRINTF("LoadCDXL 2 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)

	    if ( ret = OpenCDXL( FileNameBuf, &xledit->disp_def, &xledit->cdxlob, flags, XLTAG_Top, XL_TOP_BORDER ) ) {
		printf("'%ls'\n",XLError[ret]);
		FreeXlEdit( xledit );
	    } else {
		OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
		OnGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );

		SelectXlEdit( xledit );

//		XlControl.XlCur = xledit;

		D(PRINTF("LoadCDXL 3 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)

		if ( !XlControl.XlParent ) {
		    XlControl.XlParent = XlControl.XlCur;
		} else {
		    while ( xledit = (XLEDIT *)DNodePred( (DATA_NODE *)XlControl.XlHead, (DATA_NODE *)xledit ) ) {
			ScreenDepth( xledit->disp_def.screen, 
			    SDEPTH_TOBACK|SDEPTH_INFAMILY, NULL );
		    }
		}
		D(PRINTF("LoadCDXL 4 xledit= 0x%lx, Size= %ld\n",xledit,xledit->dnode.Size);)
	    }
	} else {
	    ret = RC_NO_MEM;
	}
    }

    return( ret );

} // LoadCDXL()

#endif		// ]


STATIC VOID __saveds
CDXLProc( VOID )
{
    int	KillBit;

    kprintf("CDXLProc 1 CDXL_OB= 0x%lx, CDXL_OB->FrameNum= %ld\n",
	CDXL_OB,CDXL_OB->FrameNum);

    EraseXlBorderTop( DisplayDef );

    D(PRINTF("CDXLProc 2\n");)

    if ( !ResetIntData( DisplayDef ) && (KillBit = AllocSignal( -1 )) != -1 ) {

	D(PRINTF("CDXLProc 3\n");)

	CDXL_OB->KillSig = 1 << KillBit;

	if ( RetCode = StartCDXL( DisplayDef, CDXL_OB ) ) {
	    D(PRINTF("'%ls'\n",XLError[RetCode]);)
	} else {
	    GoToFrame( XlControl.XlCur, CDXL_OB->FrameNum );
	}
	QuitAudio();

	FreeSignal( KillBit );
    } else {
	D(PRINTF("Could NOT allocate SigBit\n");)
    }

    kprintf("CDXLProc 4 CDXL_OB= 0x%lx, CDXL_OB->FrameNum= %ld, RetCode= %ld\n",
	CDXL_OB,CDXL_OB->FrameNum,RetCode);

    DrawXlBorderTop( DisplayDef );

    Forbid();

    CDXLTask = NULL;

    StopCopInt( DisplayDef );

    OnGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
    OnGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );

    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
    OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
    OffGad( GetGadget( PanelWindow, STILL_ID ), PanelWindow, NULL );

    if ( CDXL_OB->FrameNum < (CDXL_OB->NumFrames-1) ) {
	OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
	OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
    } else {
	OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
    }

    if ( CDXL_OB->FrameNum > 0 )
	OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );

    SetIDCMPAll();

} // CDXLProc()


VOID
KillCDXL( CDXLOB * CDXL_ob )
{
    Forbid();

    if ( CDXL_ob && CDXLTask ) {
	ResetIntData( DisplayDef );
	Signal( CDXLTask, CDXL_ob->KillSig );
    }
    Permit();

    while ( CDXLTask ) {
	WaitTOF();
    }

} // KillCDXL()


VOID
HandlePanel( VOID )
{
    ULONG		  mask,frame,CopSig = NULL;
    ULONG		  signals,PanelSig = (1 << PanelWindow->UserPort->mp_SigBit);
    BOOL		  quit = FALSE;
    DISP_DEF		  disp_def;
    USHORT		  step_id = NULL;
    int			  ret = RC_FAILED;
    struct IntuiMessage	* imsg;
    CDXLOB		* CDXL_ob = NULL;
    struct Process	* proc;
    XLEDIT		* xledit = NULL;

    setmem( &disp_def, sizeof (DISP_DEF) , 0 );

    mask = PanelSig|ParentSig|RexxSig;

    UpDateFrame( 0 );

    while ( !quit ) {

	ActivateWindow( PanelWindow );
	signals = Wait(mask);

	D(PRINTF("signals= 0x%lx, ParentSig= 0x%lx, PanelSig= 0x%lx, CopSig= 0x%lx\n",
		signals,ParentSig,PanelSig,CopSig);)

	if ( signals & RexxSig ) {
	    ProcessRexxMsgs();
	}

	if ( signals & ParentSig ) {
	    D(PRINTF("signals= 0x%lx, ParentSig= 0x%lx, PanelSig= 0x%lx\n",
		signals,ParentSig,PanelSig);)

	    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );
	}

	if ( signals & CopSig ) {
	    D(PRINTF("CopSig step_id= %ld\n",step_id);)
	    switch( step_id ) {
		case STEP_L_ID:
		    D(PRINTF("CopSig STEP_L_ID\n");)
		    StepFrame( XlControl.XlCur, FALSE );
		    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );

		    break;


		case STEP_R_ID:
		    D(PRINTF("CopSig STEP_R_ID\n");)
		    StepFrame( XlControl.XlCur, TRUE );
		    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );
		    break;
	    }
	}

	while ( (signals & PanelSig) && (imsg = GT_GetIMsg( PanelWindow->UserPort )) ) {

	    switch ( imsg->Class ) {
		case IDCMP_ACTIVEWINDOW:
		    if ( imsg->IDCMPWindow != PanelWindow ) {
			if ( xledit = Win2Xledit( imsg->IDCMPWindow ) ) {
			    SelectXlEdit( xledit );
			}
		    }

		case INTUITICKS:
		    D(PRINTF("INTUITICKS\n");)
		    ModifyIDCMP( PanelWindow, PanelWindow->IDCMPFlags & ~INTUITICKS );
		    if ( step_id ) {
			ResetIntData( &XlControl.XlCur->disp_def );
			mask |= (CopSig = CopSignal);
		    }
		    break;

		case MOUSEBUTTONS:
		    if ( step_id ) {
			mask &= ~CopSig;
			CopSig = NULL;
			StopCopInt( &XlControl.XlCur->disp_def );
			step_id = NULL;
			D(PRINTF("UP L_ID|R_ID \n");)
		    }
		    break;

		case IDCMP_GADGETDOWN:
		    switch ( ((struct Gadget *)imsg->IAddress)->GadgetID ) {
			case STEP_L_ID:
			    StepFrame( XlControl.XlCur, FALSE );
			    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );

			    step_id = ((struct Gadget *)imsg->IAddress)->GadgetID;
			    D(PRINTF("DOWN L_ID step_id= %ld\n",step_id);)
			    ModifyIDCMP( PanelWindow, PanelWindow->IDCMPFlags | INTUITICKS );
			    break;


			case STEP_R_ID:
			    StepFrame( XlControl.XlCur, TRUE );
			    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );

			    D(PRINTF("DOWN R_ID step_id= %ld\n",step_id);)
			    step_id = ((struct Gadget *)imsg->IAddress)->GadgetID;
			    ModifyIDCMP( PanelWindow, PanelWindow->IDCMPFlags | INTUITICKS );
			    break;

		    }
		    break;

		case IDCMP_GADGETUP:
		    switch ( ((struct Gadget *)imsg->IAddress)->GadgetID ) {
			case FRAME_ID:
			    if ( !XlControl.XlCur ) {
				DisplayBeep( PanelScreen );
				break;
			    }
			    if ( CalcReq( PanelWindow, PanelVI, &frame) ) {
				    GoToFrame( XlControl.XlCur, frame );
				    UpDateFrame( XlControl.XlCur->cdxlob->FrameNum );
			    }
			    if ( XlControl.XlCur->cdxlob->FrameNum >= (XlControl.XlCur->cdxlob->NumFrames-1) ) {
				OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
				OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
			    } else {
				OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );

				if ( XlControl.XlCur->cdxlob->FrameNum < 1 ) {
				    OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				} else {
				    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				}
			    }
			    break;

			case SCREEN_MODE_ID:
			    D(PRINTF("SCREENMODE_ID\n");)
//			    ChooseScreenMode( window, &scdef );
			    break;

			case OPEN_ID:
			    if ( XlControl.XlCur )
				KillCDXL( XlControl.XlCur->cdxlob );
#ifndef	OUTT	// [
			    LoadCDXL( PanelWindow, CDXL_DOSXL|CDXL_BLIT );
			    D(PRINTF("OPEN_ID XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);)

#else		// ][
			    if ( FileReq( PanelWindow ) ) {
				if ( ret = OpenCDXL( FileNameBuf, &disp_def, &CDXL_ob, CDXL_DOSXL|CDXL_BLIT, NULL ) ) {
				    printf("'%ls'\n",XLError[ret]);
				} else {
				    OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );
				}
			    }
#endif		// ]
			    break;

			case PLAY_ID:
			    KillCDXL( XlControl.XlCur->cdxlob );
			    D(PRINTF("PLAY_ID 1\n");)
			    DisplayDef = &XlControl.XlCur->disp_def;	// Set the Global ptr
			    CDXL_OB = XlControl.XlCur->cdxlob;		// Set the Global ptr

			    OffGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
			    OffGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );
			    OffGad( (struct Gadget *)imsg->IAddress, PanelWindow, NULL );
			    OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
			    OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
			    OnGad( GetGadget( PanelWindow, STILL_ID ), PanelWindow, NULL );
			    WaitBlit();
			    Delay( 5 );

			    D(PRINTF("PLAY_ID 2\n");)

			    Forbid();
			    ClearIDCMPAll();

			    D(PRINTF("PLAY_ID 3\n");)

			    if ( proc = (struct Process *)CreateNewProcTags(
				NP_Entry,	CDXLProc,
				NP_Priority,	90L,
				NP_StackSize,	8*1024,
				NP_Name,	"CDXLProcess",
				TAG_END ) )
			    {

				CDXLTask = &proc->pr_Task;
			    }

			    D(PRINTF("PLAY_ID 4\n");)

			    StripIntuiMessages( PanelWindow->UserPort, PanelWindow );
			    D(PRINTF("PLAY_ID 5\n");)

			    Permit();

			    break;

			case QUIT_ID:
			    if ( XlControl.XlCur ) {
				kprintf("QUIT_ID 1 XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);
				KillCDXL( XlControl.XlCur->cdxlob );
				kprintf("QUIT_ID 2 XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);
			    }
			    quit = TRUE;
			    kprintf("QUIT_ID\n");
			    break;

			case STILL_ID:
			    KillCDXL( XlControl.XlCur->cdxlob );
			    OffGad( (struct Gadget *)imsg->IAddress, PanelWindow, NULL );
			    break;

			case STEP_L_ID:
			case STEP_R_ID:
			    mask &= ~CopSig;
			    CopSig = NULL;
			    StopCopInt( &XlControl.XlCur->disp_def );
			    step_id = NULL;
			    D(PRINTF("UP L_ID|R_ID \n");)

			    if ( XlControl.XlCur->cdxlob->FrameNum >= (XlControl.XlCur->cdxlob->NumFrames-1) ) {
				OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
				OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
			    } else {
				OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );

				if ( XlControl.XlCur->cdxlob->FrameNum < 1 ) {
				    OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				} else {
				    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				}
			    }

			    break;

			case INFO_ID:
			    if ( XlControl.XlCur ) {
				XLInfo( XlControl.XlCur, PanelWindow );
			    }
			    break;
		    }
		    break;

		case MENUPICK:
		    if ( imsg->Code != MENUNULL ) {
		      switch ( (int)(GTMENUITEM_USERDATA( ItemAddress( PanelWindow->MenuStrip, imsg->Code) )) ) {
			case QUIT_ID:
			    if ( XlControl.XlCur )
				KillCDXL( XlControl.XlCur->cdxlob );
			    quit = TRUE;
			    D(PRINTF("QUIT_ID\n");)
			    break;


			case OPEN_NEW_ID:
			    if ( XlControl.XlCur )
				KillCDXL( XlControl.XlCur->cdxlob );

			    LoadCDXL( PanelWindow, CDXL_DOSXL|CDXL_BLIT );

			    break;

			case OPEN_ID:
#ifdef	OUTT	// [
			    if ( XlControl.XlCur )
				KillCDXL( XlControl.XlCur->cdxlob );
			    D(PRINTF("DIRECTORY_ID\n");)
			    if ( FileReq( PanelWindow ) ) {
				if ( ret = OpenCDXL( FileNameBuf, &disp_def, &CDXL_ob, CDXL_DOSXL|CDXL_BLIT, NULL ) ) {
				    printf("'%ls'\n",XLError[ret]);
				} else {
				    OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, FRAME_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, INFO_ID ), PanelWindow, NULL );
				}
			    }
#endif		// ]
			    break;

			case SAVE_ILBM_ID:
			    if ( XlControl.XlCur ) {
				KillCDXL( XlControl.XlCur->cdxlob );
				SaveXLAsILBM( XlControl.XlCur, PanelWindow );
			    }
			    break;

			case SAVE_AUD_8SVX_ID:
			    if ( XlControl.XlCur ) {
				KillCDXL( XlControl.XlCur->cdxlob );
				SaveXLAs8SVX( XlControl.XlCur, PanelWindow );

				if ( XlControl.XlCur->cdxlob->FrameNum >= (XlControl.XlCur->cdxlob->NumFrames-1) ) {
				    OffGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				    OffGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				} else {
				    OnGad( GetGadget( PanelWindow, STEP_R_ID ), PanelWindow, NULL );
				    OnGad( GetGadget( PanelWindow, PLAY_ID ), PanelWindow, NULL );

				    if ( XlControl.XlCur->cdxlob->FrameNum < 1 ) {
					OffGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				    } else {
					OnGad( GetGadget( PanelWindow, STEP_L_ID ), PanelWindow, NULL );
				    }
				}
			    }
			    break;

			case CLOSE_ID:
			    kprintf("MENUPICK CLOSE_ID:\n");
			    if ( XlControl.XlCur ) {
				kprintf("MENUPICK CLOSE_ID: 1\n");
				if ( XlControl.XlCur->disp_def.Flags & DISP_OPEN ) {
				    kprintf("MENUPICK CLOSE_ID: 2\n");

				    CloseDisplay( &XlControl.XlCur->disp_def );
				    kprintf("MENUPICK CLOSE_ID: 3\n");
				    setmem( &XlControl.XlCur->disp_def, sizeof (DISP_DEF) , 0 );
				}
				kprintf("MENUPICK CLOSE_ID: 4\n");

				CDXLTerm( XlControl.XlCur->cdxlob );
				kprintf("MENUPICK CLOSE_ID: 5\n");
				FreeXlEdit( XlControl.XlCur );
				kprintf("MENUPICK CLOSE_ID: 6\n");
			    }
			    break;


			case FRAME_DEL_ID:
			    if ( XlControl.XlCur ) {
				ULONG    start,end,numframes,i;

				CDXL_ob = XlControl.XlCur->cdxlob;
				start = end = CDXL_ob->FrameNum;

				if ( !RangeReq( XlControl.XlCur, PanelWindow, &start, &end ) ) {
				    break;
				}

				start = (start > CDXL_ob->NumFrames) ? CDXL_ob->NumFrames : start;

				numframes = (end - start) + 1;
				numframes = (numframes < 1) ? 1 : numframes;
				numframes = (numframes > CDXL_ob->NumFrames) ? CDXL_ob->NumFrames : numframes;

				if ( start != CDXL_ob->FrameNum ) {
				    GoToFrame( XlControl.XlCur, start );
				    UpDateFrame( CDXL_ob->FrameNum );
				}

				i = CDXL_ob->FrameNum;
				do {
				    XlControl.XlCur->frameflags[i++] |= XL_FRAME_DELETED;
				} while ( --numframes );

				DrawFrame( XlControl.XlCur );
			    }
			    break;


			case FRAME_UNDEL_ID:
			    if ( XlControl.XlCur ) {
				ULONG    start,end,numframes,i;

				CDXL_ob = XlControl.XlCur->cdxlob;

				start = end = CDXL_ob->FrameNum;

				if ( !RangeReq( XlControl.XlCur, PanelWindow, &start, &end ) ) {
				    break;
				}

				start = (start > CDXL_ob->NumFrames) ? CDXL_ob->NumFrames : start;

				numframes = (end - start) + 1;
				numframes = (numframes < 1) ? 1 : numframes;
				numframes = (numframes > CDXL_ob->NumFrames) ? CDXL_ob->NumFrames : numframes;

				if ( start != CDXL_ob->FrameNum ) {
				    GoToFrame( XlControl.XlCur, start );
				    UpDateFrame( CDXL_ob->FrameNum );
				}

				i = CDXL_ob->FrameNum;
				do {
				    XlControl.XlCur->frameflags[i++] &= ~XL_FRAME_DELETED;
				} while ( --numframes );

				DrawFrame( XlControl.XlCur );
			    }
			    break;

			case SAVE_AS_ID:
			    if ( !((xledit = XlControl.XlCur) && (CDXL_ob = xledit->cdxlob)) ) {
				DisplayBeep( PanelScreen );
				break;
			    }
			    ret = SaveAsCDXL( xledit, PanelWindow );
			    break;

			case SAVE_AS_FLIK_ID:
			    if ( !((xledit = XlControl.XlCur) && (CDXL_ob = xledit->cdxlob)) ) {
				DisplayBeep( PanelScreen );
				break;
			    }
			    ret = SaveAsFLIK( xledit, PanelWindow );
			    break;

			case CREATE_ID:
			    ret = XLCreate( XlControl.XlCur, PanelWindow );
			    break;
		    }
		}
		HomePanel();
		break;
	    }

	    GT_ReplyIMsg(imsg);

	    if (quit)
		break;
	}
    }

    kprintf("HandlePanel() W\n");

    while ( XlControl.XlCur ) {
	kprintf("HandlePanel() X XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);

	if ( XlControl.XlCur->disp_def.Flags & DISP_OPEN ) {
	    CloseDisplay( &XlControl.XlCur->disp_def );
	    setmem( &XlControl.XlCur->disp_def, sizeof (DISP_DEF) , 0 );
	}

	kprintf("HandlePanel() Y XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);

	CDXLTerm( XlControl.XlCur->cdxlob );

	kprintf("HandlePanel() Z XlControl.XlCur= 0x%lx, Size= %ld\n",XlControl.XlCur,XlControl.XlCur->dnode.Size);
	FreeXlEdit( XlControl.XlCur );
    }

    kprintf("HandlePanel() END\n");

} // HandlePanel()


VOID
main( LONG argc,char * argv[] )
{
    int			ret;
    int			parsigbit;
    struct DisplayInfo	disp;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );

    if ( !(UtilityBase = OpenLibrary("utility.library", 0L)) ) {
	printf("Could NOT open utility.library\n");
	exit( RETURN_ERROR );
    }

    rdargs = ReadArgs(TEMPLATE, opts, NULL);
/*
    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }
*/

    D(PRINTF("main() 1\n");)

    if ( (ret = init( FALSE )) ||
	!(GadToolsBase = OpenLibrary( "gadtools.library", 39L )) ||
    	!(AslBase = OpenLibrary("asl.library",37))
    ) {
	D(PRINTF("main() 2\n");)

	if ( GadToolsBase )
	    CloseLibrary( GadToolsBase );

	closedown();
	exit( RETURN_ERROR );
    }

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

    D(PRINTF("main() 3\n");)

    parent = FindTask( NULL );

    if ( (parsigbit = AllocSignal( -1 )) != -1 ) {

	D(PRINTF("main() 4\n");)

	ParentSig = 1 << parsigbit;

	RexxSig = RxInit();

	if ( !OpenPanel() ) {
	    D(PRINTF("main() 5\n");)

	    HandlePanel();

	    ClosePanel();
	}

	RxTerm();

	FreeSignal( parsigbit );
    }

    CloseLibrary( GadToolsBase );
    CloseLibrary( AslBase );
    CloseLibrary( UtilityBase );
    closedown();

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	printf("'%ls'\n",XLError[ret]);
	ret = RETURN_FAIL;
    }

    exit( ret );

} // main()
