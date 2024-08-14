#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/asl_protos.h>
#include <clib/disk_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define GetString( g )      ((( struct StringInfo * )g->SpecialInfo )->Buffer  )
#define GetNumber( g )      ((( struct StringInfo * )g->SpecialInfo )->LongInt )
#define GD_scrmode                             0
#define GDX_scrmode                            0
#define win_CNT 1

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct AslBase *AslBase = NULL;
struct GadToolsBase *GadToolsBase = NULL;

struct Screen *testscr = NULL;
struct ScreenModeRequester *scrmodereq = NULL;
/* struct Window *win = NULL; */

struct Screen         *Scr = NULL;
UBYTE                 *PubScreenName = "Workbench";
APTR                   VisualInfo = NULL;
struct Window         *winWnd = NULL;
struct Gadget         *winGList = NULL;
struct Gadget         *winGadgets[1];
UWORD                  winLeft = 39;
UWORD                  winTop = 65;
UWORD                  winWidth = 225;
UWORD                  winHeight = 70;
UBYTE                 *winWdt = (UBYTE *)"ScrToBack";
struct TextAttr       *Font, Attr;
UWORD                  FontX, FontY;
UWORD                  OffX, OffY;
struct TextFont       *winFont = NULL;
BOOL useDefaults = TRUE;
BOOL toggle = FALSE;

struct IntuiText winIText[] = {
	1, 0, JAM1,103, 49, NULL, (UBYTE *)"Press ESC to start/stop", NULL,
	1, 0, JAM1,100, 60, NULL, (UBYTE *)"toggling screen.", NULL };

#define win_TNUM 2

UWORD winGTypes[] = {
	BUTTON_KIND
};

struct NewGadget winNGad[] = {
	12, 10, 16, 11, (UBYTE *)"Set/Change Screen Mode", NULL, GD_scrmode, PLACETEXT_RIGHT, NULL, NULL
};

ULONG winGTags[] = {
	(TAG_DONE)
};


void OpenLibs(void);
void CloseLibs(void);
void QUIT(void);
void doScreenMode(void);
void openDisplay(void);
void KPrintF(STRPTR,...);
void doScreenMode(void);

UWORD ComputeX(UWORD);
UWORD ComputeY(UWORD);
void ComputeFont(UWORD,UWORD);
void SetupScreen(void);
void CloseDownScreen(void);
void winRender(void);
void HandlewinIDCMP(void);
void OpenwinWindow(void);
void ClosewinWindow(void);
void scrmodeClicked(void);
void winVanillaKey(ULONG);
void winGadgetUp(void);
void doDisplay(void);

void OpenLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		printf("Couldn't open intuition.library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L))) {
		printf("Couldn't open graphics.library\n");
		QUIT();
	}
	if (! (AslBase = (struct AslBase *)OpenLibrary("asl.library",0L))) {
		printf("Couldn't open asl.library\n");
		QUIT();
	}
	if (! (GadToolsBase = (struct GadToolsBase *)OpenLibrary("gadtools.library",0L))) {
		printf("Couldn't open gadtools.library\n");
		QUIT();
	}
}

void CloseLibs()
{
	if (GadToolsBase) {
		CloseLibrary((struct Library *)GadToolsBase);
	}
	if (AslBase) {
		CloseLibrary((struct Library *)AslBase);
	}
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
		CloseLibrary((struct Library *)IntuitionBase);
	}
}

void QUIT()
{
	if (scrmodereq) {
		FreeAslRequest(scrmodereq);
	}
	if (testscr) {
		CloseScreen(testscr);
	}
	ClosewinWindow();
	CloseDownScreen();
	CloseLibs();
	exit(0);
}

UWORD ComputeX( UWORD value )
{
	return(( UWORD )((( FontX * value ) + 4 ) / 8 ));
}

UWORD ComputeY( UWORD value )
{
	return(( UWORD )((( FontY * value ) + 4 ) / 8 ));
}

void ComputeFont( UWORD width, UWORD height )
{
	Forbid();
	Font = &Attr;
	Font->ta_Name = (STRPTR)GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name;
	Font->ta_YSize = FontY = GfxBase->DefaultFont->tf_YSize;
	FontX = GfxBase->DefaultFont->tf_XSize;
	Permit();

	OffX = Scr->WBorLeft;
	OffY = Scr->RastPort.TxHeight + Scr->WBorTop + 1;

	if ( width && height ) {
		if (( ComputeX( width ) + OffX + Scr->WBorRight ) > Scr->Width )
			goto UseTopaz;
		if (( ComputeY( height ) + OffY + Scr->WBorBottom ) > Scr->Height )
			goto UseTopaz;
	}
	return;

UseTopaz:
	Font->ta_Name = (STRPTR)"topaz.font";
	FontX = FontY = Font->ta_YSize = 8;
}

void SetupScreen( void )
{
	if ( ! ( Scr = LockPubScreen( PubScreenName ))) {
		printf("Couldn't lock workbench screen\n");
		QUIT();
	}

	ComputeFont( 0, 0 );

	if ( ! ( VisualInfo = GetVisualInfo( Scr, TAG_DONE ))) {
		printf("Couldn't get visual info for screen\n");
		QUIT();
	}
}

void CloseDownScreen( void )
{
	if ( VisualInfo ) {
		FreeVisualInfo( VisualInfo );
		VisualInfo = NULL;
	}

	if ( Scr        ) {
		UnlockPubScreen( NULL, Scr );
		Scr = NULL;
	}
}

void winRender( void )
{
	struct IntuiText	it;
	UWORD			cnt;

	ComputeFont( winWidth, winHeight );


	for ( cnt = 0; cnt < win_TNUM; cnt++ ) {
		CopyMem(( char * )&winIText[ cnt ], ( char * )&it, (long)sizeof( struct IntuiText ));
		it.ITextFont = Font;
		it.LeftEdge  = OffX + ComputeX( it.LeftEdge ) - ( IntuiTextLength( &it ) >> 1 );
		it.TopEdge   = OffY + ComputeY( it.TopEdge ) - ( Font->ta_YSize >> 1 );
		PrintIText( winWnd->RPort, &it, 0, 0 );
	}
}

void HandlewinIDCMP( void )
{
	struct IntuiMessage	*m;
	ULONG class;
	ULONG code;

	FOREVER {
		if ( m = GT_GetIMsg( winWnd->UserPort )) {
			class = m->Class;
			code = m->Code;
			GT_ReplyIMsg( m );

			switch (class) {
				case	IDCMP_REFRESHWINDOW:
					GT_BeginRefresh( winWnd );
					winRender();
					GT_EndRefresh( winWnd, TRUE );
					break;
				case	IDCMP_CLOSEWINDOW:
					QUIT();
					break;
				case	IDCMP_VANILLAKEY:
					winVanillaKey(code);
					break;
				case	IDCMP_GADGETUP:
					winGadgetUp();
					break;
				default:
					break;
			}
		}
		if ((toggle) && (testscr)) {
			ScreenToFront(testscr);
			ScreenToBack(testscr);
		}
	}
}

void OpenwinWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		wleft = winLeft, wtop = winTop, ww, wh;

	ComputeFont( winWidth, winHeight );

	ww = ComputeX( winWidth );
	wh = ComputeY( winHeight );

	if (( wleft + ww + OffX + Scr->WBorRight ) > Scr->Width ) {
		wleft = Scr->Width - ww;
	}
	if (( wtop + wh + OffY + Scr->WBorBottom ) > Scr->Height ) {
		wtop = Scr->Height - wh;
	}

	if ( ! ( winFont = OpenDiskFont( Font ))) {
		printf("Couldn't open font\n");
		QUIT();
	}

	if ( ! ( g = CreateContext( &winGList ))) {
		printf("Couldn't create context\n");
		QUIT();
	}

	for( lc = 0, tc = 0; lc < win_CNT; lc++ ) {

		CopyMem((char * )&winNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = Font;
		ng.ng_LeftEdge   = OffX + ComputeX( ng.ng_LeftEdge );
		ng.ng_TopEdge    = OffY + ComputeY( ng.ng_TopEdge );
		ng.ng_Width      = ComputeX( ng.ng_Width );
		ng.ng_Height     = ComputeY( ng.ng_Height);

		winGadgets[ lc ] = g = CreateGadgetA((ULONG)winGTypes[ lc ], g, &ng, ( struct TagItem * )&winGTags[ tc ] );

		while( winGTags[ tc ] ) {
			tc += 2;
		}
		tc++;

		if ( ! g ) {
			printf("Problem allocating gadgets\n");
			QUIT();
		}
	}

	if ( ! ( winWnd = OpenWindowTags( NULL,
				WA_Left,	wleft,
				WA_Top,		wtop,
				WA_Width,	ww + OffX + Scr->WBorRight,
				WA_Height,	wh + OffY + Scr->WBorBottom,
				WA_IDCMP,	BUTTONIDCMP|IDCMP_CLOSEWINDOW|IDCMP_VANILLAKEY|IDCMP_REFRESHWINDOW,
				WA_Flags,	WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE,
				WA_Gadgets,	winGList,
				WA_Title,	winWdt,
				WA_ScreenTitle,	"ScrToBack",
				WA_AutoAdjust,	TRUE,
				TAG_DONE ))) {
		printf("Couldn't open window\n");
		QUIT();
	}

	GT_RefreshWindow( winWnd, NULL );
	winRender();
}


void ClosewinWindow( void )
{
	if (winWnd ) {
		CloseWindow( winWnd );
		winWnd = NULL;
	}

	if (winGList) {
		FreeGadgets( winGList );
		winGList = NULL;
	}

	if (winFont) {
		CloseFont( winFont );
		winFont = NULL;
	}
}

void winVanillaKey(ULONG code)
{
	if (code == 0x1B) {
		toggle ^= 1;
	}
}

void winGadgetUp()
{
	doScreenMode();
}

void doScreenMode()
{
	BOOL result = FALSE;

	if (scrmodereq) {
		FreeAslRequest(scrmodereq);
		scrmodereq = NULL;
	}

	if (scrmodereq = AllocAslRequestTags(ASL_ScreenModeRequest,
		TAG_END)) {
		if (! (result = AslRequestTags(scrmodereq,
			ASLSM_PubScreenName, "Workbench",
			ASLSM_DoWidth, TRUE,
			ASLSM_DoHeight, TRUE,
			ASLSM_DoDepth, TRUE,
			ASLSM_DoOverscanType, TRUE,
			ASLSM_DoAutoScroll, TRUE,
			ASLSM_PropertyMask, 0,
			TAG_END))) {
			KPrintF("User cancelled requester\n");
			FreeAslRequest(scrmodereq);
			scrmodereq = NULL;
		}
	}
	doDisplay();
}

void doDisplay()
{
	if (testscr) {
		CloseScreen(testscr);
		testscr = NULL;
	}

	if (scrmodereq) {
		KPrintF("Using user choices\n");
		if (! (testscr = OpenScreenTags(NULL,
			SA_DisplayID, scrmodereq->sm_DisplayID,
			SA_Width, scrmodereq->sm_DisplayWidth,
			SA_Height, scrmodereq->sm_DisplayHeight,
			SA_Depth, scrmodereq->sm_DisplayDepth,
			SA_Overscan, scrmodereq->sm_OverscanType,
			SA_AutoScroll, scrmodereq->sm_AutoScroll,
			SA_Title, "TEST SCREEN",
			TAG_END))) {
			printf("Couldn't open test screen\n");
			QUIT();
		}
	}
	if (! scrmodereq) {
		KPrintF("Using defaults\n");
		if (! (testscr = OpenScreenTags(NULL,TAG_END))) {
			printf("Couldn't open screen\n");
			QUIT();
		}
	}
	ScreenToBack(testscr);
}

void main()
{
	OpenLibs();

	SetupScreen();
	OpenwinWindow();
	HandlewinIDCMP();

	QUIT();
}