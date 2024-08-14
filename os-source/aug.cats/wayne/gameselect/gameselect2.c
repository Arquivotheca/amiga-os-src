/***********

    GameSelect.c

    W.D.L 930618

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
#include <exec/interrupts.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <utility/tagitem.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuitionbase.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>

#include <hardware/intbits.h>

#include <libraries/asl.h>
#include <libraries/lowlevel.h>

#include "devices/cd.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "disp_def.h"
#include "retcodes.h"

#include "gameselect_rev.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "debugson.h"

#define	GAMESELVER	VERSTAG " Wayne D. Lutz"

UBYTE	* Version = GAMESELVER;


// argument parsing 
#define TEMPLATE    "FROM/A"
#define	OPT_FROM	0
#define	OPT_COUNT	1

#define GAD_LEFT	15
#define	GAD_TOP		15
#define	GAD_HT		(attr->ta_YSize + 4)

#define	PANEL_DEPTH	2


#define FIRST_ID	100

#define LMB_DOWN (!((*((BYTE *) 0xbfe001) & 192)==192))
#define S(MASK) (MASK & state )

// protos

VOID CloseDisplay( DISP_DEF * disp_def );
int  DoDisplay( UBYTE * ilbmname );
int  DoQuery( UBYTE * filename, DISP_DEF * disp_def );
int  ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile, LONG );
int  PlayTrack( ULONG track );
VOID CDDeviceTerm( VOID );
BOOL CDDeviceInit( ULONG * opened );
VOID CDAudioTerm( VOID );


UBYTE outcon[80] = 
	{"CON:10/10/600/200/GameSelect Output Window/AUTO/CLOSE/WAIT"};

UBYTE * DoWB = "execute >nil: cd0:s/startup-sequence.wb";

BOOL	DisplayIsPal;


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


DISP_DEF		  DisplayDef;
UBYTE			* ILBMname;

struct Screen		* PanelScreen;
struct Window		* PanelWindow;

struct Screen		* MaskScreen;

struct VisualInfo	* PanelVI;
struct Gadget		* PanelGlist;
struct Image		* PanelFrame;

struct Library		* GadToolsBase;
struct Library		* UtilityBase;
struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;


STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;

STATIC struct IOStdReq	* CDDeviceMReq;

STATIC LONG	CDPortSignal	= -1;


typedef struct DataNode
{
    struct DataNode *Next;
    ULONG Size;
    ULONG Type;

} DATA_NODE;

typedef struct GameConfig
{
    DATA_NODE	  dnode;
    UBYTE	* name;
    UBYTE	* command;

} GAMECONFIG;


GAMECONFIG * HeadConfig;

struct Rectangle MarqueeRect;
struct Gadget	* CurrentGad;
SHORT  CurrentID = FIRST_ID;

struct TextAttr Topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};


void __regargs _CXBRK(void) { }  /* Disable SAS CTRL/C handling */
void __regargs chkabort(void) { }  /* really */
void __regargs Chk_Abort(void) { }  /* truly */


/*
 * Close every thing down.
 */
//STATIC
VOID
closedown( VOID )
{
    if ( MaskScreen ) {
	CloseScreen( MaskScreen );
	MaskScreen = NULL;
    }

    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    if ( GadToolsBase )
	CloseLibrary( GadToolsBase );

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
init( BOOL iff )
{
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library! V39\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library V39!\n");
	return( RC_FAILED );
    }

    if ( !(GadToolsBase = OpenLibrary( "gadtools.library", 39L )) ) {
	printf("Could NOT open gadtools.library V39!\n");
	return( RC_FAILED );
    }

    if ( !(LowLevelBase = OpenLibrary( "lowlevel.library", 39L )) ) {
	printf("Could NOT open lowlevel.library!\n");
	return( RC_FAILED );
    }


    if ( iff ) {
	D(PRINTF("opening iffparse.library!\n");)
	if(!(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	    printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
	}
    }

    D(PRINTF("Opening freeanim.library\n");)
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    D(PRINTF("After opening freeanim.library\n");)

    return( RC_OK );

} // init()


VOID
ClosePanel( VOID )
{

    if ( PanelFrame ) {
	DisposeObject( PanelFrame );
	PanelFrame = NULL;
    }

    if ( DisplayDef.Flags & DISP_OPEN ) {
	if ( PanelVI ) {
	    FreeVisualInfo( PanelVI );
	    PanelVI = NULL;
	}
	CloseDisplay( &DisplayDef );

	if ( PanelGlist ) {
	    FreeGadgets( PanelGlist);
	    PanelGlist = NULL;
	}
	PanelScreen = NULL;
	PanelWindow = NULL;
	return;
    }

    if ( PanelWindow ) {
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


struct Gadget *
GadCreate( struct NewGadget * ng, struct VisualInfo * vi, struct Gadget * last,
 ULONG kind, ULONG tag, ... )
{
	ng->ng_VisualInfo = vi;

	return( CreateGadgetA(kind,last,ng,(struct TagItem *)&tag) );

} // GadCreate()


Config2Gadgets( struct VisualInfo * vi, struct Gadget **glist )
{
    GAMECONFIG		* config;
    struct TextAttr	* attr = &Topaz80;
    struct Gadget	* prev;
    struct NewGadget	  ng;
    struct IntuiText	  it;
    SHORT		  len,cnt,id = FIRST_ID,max = 0,min = PanelScreen->Width;

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof (it), 0 );

    *glist = NULL;

    if ( !(prev = CreateContext(glist)) )
	return( FALSE );

    it.ITextFont = attr;

    cnt = 0;
    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	it.IText = config->name;
	len = IntuiTextLength( &it );
	it.LeftEdge = (PanelScreen->Width >> 1) - (len >> 1);
	max = (len > max) ? len : max;
	min = (it.LeftEdge < min) ? it.LeftEdge : min;
	cnt++;
    }


    ng.ng_LeftEdge = min;
//    ng.ng_TopEdge = (PanelScreen->Height >> 1) - ((cnt * (GAD_HT+4)) >> 1) + 20;
    ng.ng_TopEdge = 60;
    ng.ng_Height = GAD_HT;
    ng.ng_TextAttr = it.ITextFont = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;

    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	ng.ng_GadgetText = config->name;
	ng.ng_Width = max + 16;
	ng.ng_GadgetID = id++;
	if ( prev = GadCreate( &ng, vi, prev, BUTTON_KIND, TAG_DONE ) ) {
	    prev->UserData = (APTR)config;
	}
	ng.ng_TopEdge += GAD_HT + 4;
    }

    if ( !prev ) {
	FreeGadgets( *glist );
	*glist = NULL;
	return( FALSE );
    }

    return( TRUE );

} // Config2Gadgets()


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
RubberBox( struct RastPort * rp, struct Rectangle * rect )
{
    int x1,y1,x2,y2;
    int i;

    x1 = rect->MinX;
    y1 = rect->MinY;
    x2 = rect->MaxX;
    y2 = rect->MaxY;

    if (x1 > x2) {
	i = x1;
	x1 = x2;
	x2 = i;
    }

    if (y1 > y2) {
	i = y1;
	y1 = y2;
	y2 = i;
    }

    Move(rp,x1,y1);
    Draw(rp,x2,y1);
    Move(rp,x2,y1+1);
    Draw(rp,x2,y2);
    Move(rp,x2-1,y2);
    Draw(rp,x1,y2);
    Move(rp,x1,y2-1);
    Draw(rp,x1,y1+1);

} // RubberBox()


#ifdef	OUTT	// [
VOID
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL redraw )
{
    static int patidx = 0;
    static int pat[4] = {0xcccc,0x6666,0x3333,0x9999};
    struct Rectangle trect;

    trect.MinX = rect->MinX - 1;
    trect.MaxX = rect->MaxX + 1;
    trect.MinY = rect->MinY - 1;
    trect.MaxY = rect->MaxY + 1;

    SetDrPt( rp, pat[patidx] );
    RubberBox( rp, rect );
    RubberBox( rp, &trect );

    if ( redraw ) {

	if ( (++patidx) == 4 )
		patidx = 0;

	SetDrPt( rp,pat[patidx] );

	RubberBox( rp, rect );
	RubberBox( rp, &trect );
    }

    SetDrPt( rp, 0xffff );

} // Marquee()
#else		// ][

VOID
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL erase )
{
STATIC int	pen = 7;
    struct Rectangle trect;


    trect.MinX = rect->MinX - 1;
    trect.MaxX = rect->MaxX + 1;
    trect.MinY = rect->MinY - 1;
    trect.MaxY = rect->MaxY + 1;

    SetDrPt( rp, 0xffff );

    SetAPen( rp, erase ? 0 : pen );
    RubberBox( rp, rect );
    RubberBox( rp, &trect );
/*
    if ( ++pen > 3 )
	pen = 2;
*/

    pen = (pen == 7) ? 3 : 7;

#ifdef	OUTT	// [
    WaitTOF();
    SetAPen( rp, 3 );
    RubberBox( rp, rect );
    RubberBox( rp, &trect );

    if ( redraw ) {
	SetAPen( rp, 2 );
	RubberBox( rp, rect );
	RubberBox( rp, &trect );

	WaitTOF();
	SetAPen( rp, 3 );
	RubberBox( rp, rect );
	RubberBox( rp, &trect );
    }
#endif		// ]

} // Marquee()

#endif

VOID
CurrGad( struct Gadget * gad )
{
    if ( !gad ) {
	D(PRINTF("CurrGad NULL gad!!!\n");)
	return;
    }

    Marquee( PanelWindow->RPort, &MarqueeRect, TRUE);

    MarqueeRect.MinY = gad->TopEdge - 1;
    MarqueeRect.MaxY = MarqueeRect.MinY + gad->Height + 1;

    MarqueeRect.MinX = gad->LeftEdge - 1;
    MarqueeRect.MaxX = MarqueeRect.MinX + gad->Width + 1;

    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

    CurrentGad = gad;
    CurrentID = gad->GadgetID;

} // CurrGad()


VOID
PrevGad( VOID )
{
    int		    id;
    struct Gadget * gad;

    if ( CurrentGad ) {
	if ( (id = CurrentGad->GadgetID) == FIRST_ID ) {
	    for ( gad = CurrentGad; gad && gad->NextGadget; gad = gad->NextGadget );
    	    CurrGad( gad );
	} else {
    	    CurrGad( GetGadget( PanelWindow, --id ) );
	}
    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }

} // PrevGad()


BOOL
docommand( UBYTE *command, BOOL restart )
{
    struct TagItem tags[3];
    LONG fh;

    CDAudioTerm();

    ScreenToFront( MaskScreen );
    ClosePanel();

    WaitBlit();
    Delay( 50 );
    WaitBlit();

    if(fh = Open(outcon,MODE_OLDFILE)) {
    	tags[0].ti_Tag	= SYS_Input;
    	tags[0].ti_Data 	= fh;
    	tags[1].ti_Tag	= SYS_Output;
    	tags[1].ti_Data 	= NULL;
    	tags[2].ti_Tag	= TAG_DONE;

	if( (System(command, tags) ) == -1 )
	    Close(fh);
    } else {
	printf("docommand - Can't open output window\n");
    }

    if ( !restart )
	return( TRUE );

    DoDisplay( ILBMname );

//    Delay( 300 );

    if ( CDDeviceInit( NULL ) ) {
	PlayTrack( 4 );
	return( TRUE );
    }

    return( FALSE );

} // docommand()


BOOL
SelectGad( VOID )
{
    GAMECONFIG	* config;

    if ( CurrentGad && (config = (GAMECONFIG *)CurrentGad->UserData ) ) {
	CurrentGad->Flags |= SELECTED;
	RefreshGList( CurrentGad, PanelWindow, NULL, 1 );
	Delay( 20 );
	return( docommand( config->command, TRUE ) );
    }

    return( FALSE );

} // SelectGad()


VOID
NextGad( VOID )
{
    struct Gadget * gad;

    if ( CurrentGad && (gad = CurrentGad->NextGadget) ) {
    	    CurrGad( gad );
    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }

} // NextGad


VOID
SetUpCurrGad( VOID )
{
    CurrentGad = GetGadget( PanelWindow, CurrentID );
    MarqueeRect.MinY = CurrentGad->TopEdge - 1;
    MarqueeRect.MaxY = MarqueeRect.MinY + CurrentGad->Height + 1;

    MarqueeRect.MinX = CurrentGad->LeftEdge - 1;
    MarqueeRect.MaxX = MarqueeRect.MinX + CurrentGad->Width + 1;
    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

} // SetUpCurrGad()


OpenMaskScreen( VOID )
{
    struct ColorSpec	colors[] = 
			{
			    {0,0,0,0},
			    {-1,0,0,0}
			};

    if ( MaskScreen = OpenScreenTags(NULL,

	SA_Height,	4,
	SA_Depth,	1,

	SA_DisplayID,	LORES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
//	SA_Behind,	TRUE,
	SA_Colors,	colors,
	TAG_DONE) )
    {
	D(PRINTF("MaskScreen->Height= %ld, Rows= %ld\n",MaskScreen->Height,MaskScreen->RastPort.BitMap->Rows);)
	return( RC_OK );
    }

} // OpenMaskScreen()


OpenPanel( VOID )
{
    struct ColorSpec	colors[] = 
	{
	    {0,0xAA,0xAA,0xAA},
	    {1,0x00,0x00,0x00},
	    {2,0xFF,0xFF,0xFF},
	    {3,0x00,0x88,0xFF},
	    {-1,0,0,0}
	};

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


    if ( PanelScreen = OpenScreenTags(NULL,

//	SA_Height,	PANEL_HT,
	SA_Depth,	PANEL_DEPTH,

	SA_DisplayID,	HIRES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	SA_Colors,	colors,
	SA_Pens,	pens,
	TAG_DONE) )
    {

	D(PRINTF("OpenPanel() PanelScreen->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, Depth= %ld, PanelScreen= 0x%lx\n",
	    PanelScreen->LeftEdge,PanelScreen->TopEdge,PanelScreen->Width,
	    PanelScreen->Height,PanelScreen->RastPort.BitMap->Depth,PanelScreen);)

	PanelVI = GetVisualInfo( PanelScreen, TAG_DONE );

	if ( PanelVI && Config2Gadgets( PanelVI, &PanelGlist ) &&
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

	    SetPointer( PanelWindow, InvisiblePointer, 1, 1, 0, 0 );
//	    SetDrMd( PanelWindow->RPort, COMPLEMENT | JAM1 );
	    SetDrMd( PanelWindow->RPort, JAM1 );

	    if ( PanelFrame = NewObject( NULL, "frameiclass",
		IA_FrameType, FRAME_ICONDROPBOX,
		IA_Recessed, TRUE,
		IA_Width, PanelWindow->Width,
		IA_Height, PanelWindow->Height,
		TAG_DONE ) ) {

		DrawImage( PanelWindow->RPort, PanelFrame, 0, 0 );
		RefreshGList( PanelGlist, PanelWindow, NULL, -1 );
	    }

	    SetUpCurrGad();

	    D(PRINTF("OpenPanel() PanelWindow->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, WScreen= 0x%lx\n",
		PanelWindow->LeftEdge,PanelWindow->TopEdge,PanelWindow->Width,
		PanelWindow->Height,PanelWindow->WScreen);)

	    ScreenToFront( PanelScreen );
	    return( RC_OK );
	}

//	CloseScreen( PanelScreen );
//	PanelScreen = NULL;
	ClosePanel();

	return( RC_NO_WIN );
    }

    return( RC_NO_SC );

} // OpenPanel()


DoDisplay( UBYTE * ilbmname )
{
    int		  ret;

    setmem( &DisplayDef, sizeof (DISP_DEF), 0 );

    D(PRINTF("DoDisplay ENTERED with ilbmname= '%ls'\n",
	ilbmname ? ilbmname : "");)

    if ( !(ilbmname && *ilbmname) )
	return( OpenPanel() );

    DisplayDef.Flags |= DISP_SCREEN;
    DisplayDef.Flags |= (DISP_INTERLEAVED|DISP_BACKGROUND);
    DisplayDef.Flags |= DISP_CENTERX;
    DisplayDef.Flags |= DISP_NOPOINTER;

    if ( !DoQuery( ilbmname, &DisplayDef ) ) {
	D(PRINTF("DoDisplay() DoQuery returned FALSE\n");)
	return( OpenPanel() );
    }


    if ( !(ret = ScrWinOpen( &DisplayDef, ilbmname, NULL ) ) ) {

	PanelScreen = DisplayDef.screen;
	PanelWindow = PanelScreen->FirstWindow;

	if ( PanelVI = GetVisualInfo( PanelScreen, TAG_DONE ) ) {

	    if ( Config2Gadgets( PanelVI, &PanelGlist ) ) {
		AddGList( PanelWindow, PanelGlist, 0, -1, NULL );
		RefreshGList( PanelGlist, PanelWindow, NULL, -1 );
		SetUpCurrGad();
		return( RC_OK );
	    }
 	}

    }

    setmem( &DisplayDef, sizeof (DISP_DEF) ,0 );

    D(PRINTF("DoDisplay() END calling OpenPanel\n");)

    return( OpenPanel() );

} // DoDisplay()

/************************************************************************
**	NextToken.c - MEW 900430
**
**	Scans a whitespace-delimited string and returns the length of and a
**	pointer to the next token in the string.  Any whitespace enclosed
**	in quotes is considered part of the token.  Any whitespace preceeding
**	a token is ignored.  If there isn't another token, this returns NULL.
**
**	On exit, this function returns a pointer to the token in your string.
**	Also, your string pointer will be modified to point immediately after
**	the token, and the token's length is returned.
**
**	Please note: Your original string is not modified; ie: the token
**	string will NOT be null-terminated.
**	
*/

STATIC UBYTE *
NextToken( UBYTE ** str, int * len )
{
    UBYTE	*scan = *str, *tok = NULL;
    UBYTE	quote = 0;

    *len = 0;
    FOREVER  {
	switch ( *scan )  {
	    case 0:
		goto NextToken_exit;

	    case ' ':
	    case '\t':
		scan++;
		break;

	    case '\"':
		quote = *(scan++);
		goto NextToken_1;

	    default:
		goto NextToken_1;
	}
    }

NextToken_1:
    tok = scan;
    FOREVER  {
	switch ( *scan )  {

	    case 0:
		goto NextToken_exit;

	    case ' ':
	    case '\t':
		if ( !quote )
			goto NextToken_exit;
		scan++;
		break;

	    case '\"':
		if ( *(scan++) == quote )
			goto NextToken_exit;
		break;

	    default:
		scan++;
		break;
	}
	(*len)++;
    }

NextToken_exit:
    *str = scan;
    return ( tok );

}  /* NextToken() */


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
    if ( pred == NULL ) {
	pred = (DATA_NODE *)head;
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
	*pred = *node;
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


AddGameConfig( UBYTE * namebuf, UBYTE * commandbuf )
{
    GAMECONFIG	* config,* pred;
    LONG	  size = sizeof (GAMECONFIG) + strlen(namebuf) + 1 + strlen(commandbuf) + 1;

    if ( config = (GAMECONFIG *)AllocDataNode( size, MEMF_CLEAR ) ) {
	config->name = (UBYTE*)(config + 1);
	config->command = config->name + strlen(namebuf) + 1;
	strcpy( config->name, namebuf );
	strcpy( config->command, commandbuf );

	for ( pred = HeadConfig; pred && pred->dnode.Next; pred = (GAMECONFIG *)pred->dnode.Next );

	DNodeInsert( (DATA_NODE **)&HeadConfig, (DATA_NODE *)config, (DATA_NODE *)pred );

	return( RC_OK );
    }

    return( RC_NO_MEM );

} // AddGameConfig()


#define	BUF_LEN	100
LoadConfig( UBYTE * file )
{
    FILE		* fp;
    UBYTE		  inbuf[256],namebuf[BUF_LEN],commandbuf[BUF_LEN],*bufptr,* ptr;
    int			  i,len,ret;

    D(PRINTF("LoadConfig ENTERED\n");)

    if( !(fp = fopen(file,"r")) ) {
	D(PRINTF("Could not open file '%s'\n",file);)
	return( RC_CANT_FIND );
    }

    ret = RC_OK;

    while( !feof( fp ) ) {

	if( !(fgets(inbuf,sizeof(inbuf),fp)) ) {
	    fclose( fp );
	    return( RC_OK );
	}

	i = strlen( inbuf );
	inbuf[ i-1 ] = 0;
	bufptr = inbuf;

	D(PRINTF("bufptr= '%ls'\n",bufptr);)

#ifdef	OUTT	// [
	if ( ptr = NextToken( &bufptr, &len ) ) {
	    if ( *ptr == ';' )	// ignore comments (they start with ';')
		continue;

	    D(PRINTF("NextToken returned ptr= '%ls', len= %ld\n",ptr,len);)
	    if ( len < (BUF_LEN-1) ) {
		strncpy( namebuf, ptr, len );
		namebuf[len] = 0;
	    } else {
		continue;
	    }
	} else {
	    D(PRINTF("LoadConfig() namebuf.. NextToken returned NULL!!!\n");)
	    continue;
	}

	if ( ptr = NextToken( &bufptr, &len ) ) {
	    D(PRINTF("NextToken returned ptr= '%ls', len= %ld\n",ptr,len);)
	    if ( len < (BUF_LEN-1) ) {
		strncpy( commandbuf, ptr, len );
		commandbuf[len] = 0;
	    } else {
		continue;
	    }
	} else {
	    D(PRINTF("LoadConfig() commandbif.. NextToken returned NULL!!!\n");)
	    continue;
	}
#else		// ][

	// skip white space
	for ( ; *bufptr && ((*bufptr == ' ') || (*bufptr == '\t')); bufptr++ );
	if ( !*bufptr || (*bufptr == ';') )
	    continue;

	if ( ptr = strchr( bufptr, '=' ) ) {
	    len = (int)ptr - (int)bufptr;
	    if ( len < (BUF_LEN-1) ) {
		strncpy( namebuf, bufptr, len );
		namebuf[len] = 0;
	    } else {
		continue;
	    }

	    ptr++;
	    if ( !*ptr )
		continue;

	    strncpy( commandbuf, ptr, (BUF_LEN-1) );
	    commandbuf[(BUF_LEN-1)] = 0;
	} else {
	    continue;
	}
#endif		// ]

	if ( ret = AddGameConfig( namebuf, commandbuf ) ) {
	    D(PRINTF("LoadConfig() Out of Memory\n");)
	    break;
	}
    }

    fclose( fp );
    return( ret );

} // LoadConfig()


VOID
FreeConfig( VOID )
{
    DNodeListFree( (DATA_NODE *)HeadConfig );

} // FreeConfig()



STATIC BOOL
ReadPort( ULONG port )
{
    STATIC	ULONG state = 0L, oldstate;

    oldstate = state;
    state = ReadJoyPort( port );

    if (oldstate != state) {
	switch ( JP_TYPE_MASK & state ) {

	    case (JP_TYPE_GAMECTLR):
		D(PRINTF("\nGame controller:\n");)
		if (S(JPF_BTN1)) { 
		    D(PRINTF(" Right\n");)
//		    SelectGad();
		}
		if (S(JPF_BTN2)) { 
		    D(PRINTF(" Fire\n");)
		    SelectGad();
		}
		if (S(JPF_BTN3)) D(PRINTF(" JPF_BTN3\n");)
		if (S(JPF_BTN4)) D(PRINTF(" JPF_BTN4\n");)
		if (S(JPF_BTN6) && S(JPF_BTN4) && S(JPF_BTN1)) {
		    D(PRINTF(" left ear, Blue, Red\n");)
		    docommand( DoWB, FALSE );
		    return( TRUE );
		}
/*
		if (S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN7)) {
		    D(PRINTF(" left ear, Middle, right ear\n");)
		}
*/
		if (S(JPF_BTN7)) D(PRINTF(" Middle\n");)
		if (S(JPF_UP)) {
		    D(PRINTF(" Up direction\n");)
		    PrevGad();
		    break;
		}
		if (S(JPF_DOWN)) { 
		    D(PRINTF(" Down direction\n");)
		    NextGad();
		    break;
		}
		break;

	}
    }

    return( FALSE );

} // ReadPort()


BOOL
Check4ReBoot( VOID )
{
    STATIC	ULONG state = 0L, oldstate;

    oldstate = state;
    state = ReadJoyPort( 1 );

    if ( 1 /*(oldstate != state)*/ ) {
	switch ( JP_TYPE_MASK & state ) {
	    case (JP_TYPE_GAMECTLR):
		if (S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN7)) {
		    return( TRUE );
		}
	    break;
	}
    }

    return( FALSE );

} // Check4ReBoot()

typedef struct intdata
{
    LONG count1;
    LONG count2;

} INTDATA;

__interrupt __asm __saveds
VertBServer( register __a1 INTDATA * idata )
{

    if ( !(++(idata->count1) % 45) ) {
	D(PRINTF("* ");)
	if ( Check4ReBoot() ) {
	    if ( ++(idata->count2) > 3) {
		D(PRINTF("REBOOT!\n");)
		ColdReboot();
	    }
	} else {
	    idata->count2 = 0;
	}
    }

    return( 0 );

} // VertBServer()


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
	D(PRINTF("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()


/*
 *  DoIOR -- synchronously execute a device command
 */
BOOL
DoIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data   = data;

    DoIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	D(PRINTF("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()


/*
 * Send a CD_INFO command to cd.device. The info is stored in the cdinfo structure.
 */
BOOL
GetCDInfo( struct CDInfo * cdi )
{
    struct  IOStdReq	__aligned req = *CDDeviceMReq;

    DoIOR( &req, CD_INFO, NULL, sizeof ( struct CDInfo ), cdi );

    if ( !(req.io_Error ) ) {

	D(PRINTF("\nReadXLSpeed= %ld, MaxSpeed= %ld\nSectorSize= %ld, XLECC= %ld\nStatus= %ld\n\n",
	cdi->ReadXLSpeed,cdi->MaxSpeed,cdi->SectorSize,cdi->XLECC,cdi->Status);)

	return( TRUE );
    } else {
	D(PRINTF("CD_INFO ERROR!!! io_Error= %ld\n",req.io_Error);)
	return( FALSE );
    }

} // GetCDInfo()


BOOL
PauseCDDA( struct IOStdReq * req, int pause )
{
    req->io_Command = CD_PAUSE;
    req->io_Offset = NULL;
    req->io_Length = pause;	// pause is 1 to pause 0 to unpause
    req->io_Data   = NULL;

    DoIO( (struct IORequest *)req );

    if ( !(req->io_Error ) ) {

	return( TRUE );
    } else {
	D(PRINTF("CD_PAUSE ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    }

} // PauseCDDA()


PlayTrack( ULONG track )
{
    struct CDInfo   __aligned cdinfo;

    D(PRINTF("PlayTrack() ENTERED\n");)
    GetCDInfo( &cdinfo );

    if ( cdinfo.Status & CDSTSF_PAUSED ) {
	D(PRINTF("PlayTrack() UNPAUSING\n");)
	PauseCDDA( CDDeviceMReq, 0 ); // Unpause it
    }

    SendIOR( CDDeviceMReq, CD_PLAYTRACK, track, 1, NULL );

    if ( CDDeviceMReq->io_Error ) {
	D(PRINTF("\nPlayTracks io_Error= %ld\n\n",req.io_Error);)
	return( FALSE );
    }

    D(PRINTF("PlayTrack() END\n");)

    return( TRUE );

} // PlayTrack()


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

		if ( !OpenDevice( "cd.device", 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
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

	CDPortSignal = ( 1 << CDPort->mp_SigBit );
	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // CDDeviceInit()


VOID
CDAudioTerm( VOID )
{
    struct CDInfo __aligned cdinfo;

    D(PRINTF("CDAudioTerm() 1\n");)

    if( CDDeviceMReq ) {
	D(PRINTF("CDAudioTerm() 1.2\n");)

	GetCDInfo( &cdinfo );

	if ( cdinfo.Status & CDSTSF_PLAYING ) {

	    struct IOStdReq	__aligned req = *CDDeviceMReq;

	    D(PRINTF("CDAudioTerm() 1.2\n");)
//	    PauseCDDA( &req, 1 );

	    D(PRINTF("CDAudioTerm() 2\n");)
	    AbortIO( (struct IORequest *)CDDeviceMReq );
	    D(PRINTF("CDAudioTerm() 3\n");)

	    WaitIO(  (struct IORequest *)CDDeviceMReq );
	    D(PRINTF("CDAudioTerm() 4\n");)
//	    PauseCDDA( &req, 0 );
	}

	D(PRINTF("CDAudioTerm() 5\n");)

	CDDeviceTerm();
    }

    D(PRINTF("CDAudioTerm() END\n");)

} /* CDAudioTerm() */


VOID
main( LONG argc,char * argv[] )
{
    int			  ret,checkplay = 0,i=0;
    INTDATA		  idata;
    struct DisplayInfo	  disp;
    BOOL		  DoAudio;
    APTR		  TimerHandle;
    struct Interrupt	* vbint = NULL;
    struct CDInfo cdinfo;

    D(PRINTF("argc= %ld, argv[0]= '%ls', argv[1]= '%ls'\n",
	argc,argv[0],argv[1]);)

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );
    setmem( &idata, sizeof (idata) ,0 );

    if ( !(UtilityBase = OpenLibrary("utility.library", 0L)) ) {
	D(PRINTF("Could NOT open utility.library\n");)
	exit( RETURN_ERROR );
    }

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	D(PRINTF("main() ReadArgs FAILED\n");)
	exit( RETURN_ERROR );
    }

    ILBMname = (UBYTE *)opts[OPT_FROM];

    D(PRINTF("main() ILBMname from CommandLine= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( !(ILBMname && *ILBMname) )
	ILBMname = "Demos:GameSelect.pic";

    D(PRINTF("main() ILBMname= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( ret = init( (ILBMname && *ILBMname) ? TRUE : FALSE ) ) {

	closedown();
	exit( RETURN_ERROR );
    }

    ReadPort( 1 ); // Need to do this once outside of the interrupt

    if ( ret = LoadConfig( "Scripts:GameSelect.config" ) ) {
	printf("Error loading GameSelect.config file ret= %ld\n",ret);
	closedown();
	exit( RETURN_ERROR );
    }

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

    SetTaskPri( FindTask( NULL ), 5L );


    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;

    if ( ret = OpenMaskScreen() )
	goto exit;


    if ( !CDDeviceInit( NULL ) ) {
	printf("CDDeviceInit() FAILED\n");
	goto exit;
    }


/*
    if ( TimerHandle = (APTR)AddTimerInt( TimerRoutine, &count ) ) {
	StartTimerInt( TimerHandle, 90000, TRUE );
    }
*/

    /* Allocate memory for interrupt node */
    if (vbint = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR)) {
        /* initialize the node */
        vbint->is_Node.ln_Type = NT_INTERRUPT;
        vbint->is_Node.ln_Pri = 9;
        vbint->is_Node.ln_Name = "VertReset";
        vbint->is_Data = &idata;
        vbint->is_Code = (APTR)VertBServer;

        /* kick this interrupt server to life*/
        AddIntServer(INTB_VERTB, vbint);
    }

    if ( !DoDisplay( ILBMname ) ) {
//	Delay( 300 );
	DoAudio = PlayTrack( 4 );

	while ( !ReadPort( 1 ) ) {
	    WaitTOF();
	    if ( ++i > 10 ) {
		Marquee( PanelWindow->RPort, &MarqueeRect, FALSE );
		i = 0;
	    }
	    WaitTOF();
	    if ( DoAudio && !(++checkplay % 60) ) {
		GetCDInfo( &cdinfo );
		if ( !(cdinfo.Status & CDSTSF_PLAYING) ) {
		    WaitIO(  (struct IORequest *)CDDeviceMReq );
		    PlayTrack( 4 );
		}
	    }

	}
//	Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

	ClosePanel();
	Delay( 500 );
    }

exit:

/*
    if ( TimerHandle )
	RemTimerInt( TimerHandle );
*/
    CDAudioTerm();

    if ( vbint ) {
	RemIntServer(INTB_VERTB, vbint);
	FreeMem(vbint, sizeof(struct Interrupt));
    }

    FreeConfig();

    CloseLibrary( UtilityBase );
    closedown();

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	ret = RETURN_FAIL;
    }

    exit( ret );

} // main()

