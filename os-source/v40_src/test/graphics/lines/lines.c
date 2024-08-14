/****************************************

	lines.c - April 1993 - Mark Dolecki

	Hack program to experiment with:
x		user copper lists
x		Ieee trancendental math
x		Regions and layer clipping

****************************************/
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/copper.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/gels.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
#include <hardware/custom.h>
#include <intuition/intuition.h>
#include <libraries/asl.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include <clib/mathieeesingtrans_protos.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/mathieeesingtrans_pragmas.h>
#include <libraries/mathieeesp.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#ifdef LATTICE
int CXBRK(void) { return(0); }
int chkabort(void) { return(0); }
#endif

#define NUMCOLORS 16
#define GELSIZE 4

UBYTE *vers = "\0$VER: lines 1.0";

extern struct DOSBase *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct LayersBase *LayersBase = NULL;
struct AslBase *AslBase = NULL;
struct GadToolsBase *GadToolsBase = NULL;
struct Library *MathIeeeSingTransBase = NULL;

extern struct Custom custom;

struct Window *win = NULL;
struct Screen *scr = NULL;
struct ScreenModeRequester *scrmodereq = NULL;
int return_code;

void loadCopper(void);
void processWindow(void);
void OpenView(void);
void OpenLibs(void);
void CloseLibs(void);
void QUIT(void);

void loadCopper()
{
	register UWORD i;
	struct ViewPort *viewport = NULL;
	struct UCopList *uCopList = NULL;
	BOOL error = FALSE;
	struct TagItem uCopTags[] =
	{
		{ VTAG_USERCLIP_SET, NULL },
		{ VTAG_END_CM, NULL }
	};
	UWORD spectrum[] =
		{
			0x000, 0x001, 0x002, 0x003, 0x004,
			0x005, 0x006, 0x007, 0x008, 0x009,
			0x00a, 0x00b, 0x00c, 0x00d, 0x00e,
			0x00f, 0x00f, 0x00e, 0x00d, 0x00c,
			0x00b, 0x00a, 0x009, 0x008, 0x007,
			0x006, 0x005, 0x004, 0x003, 0x002,
			0x001, 0x000
		};


	uCopList = (struct UCopList *)AllocMem(sizeof(struct UCopList),MEMF_PUBLIC | MEMF_CLEAR);
	if (! uCopList) {
		printf("Couldn't allocate copperlist\n");
		QUIT();
	}
	else {
		CINIT(uCopList, (scr->Height * 2 ) + 2);
		for (i = 0; i < scr->Height; i++) {
			CWAIT(uCopList, i, 0);
			CMOVE(uCopList, custom.color[0], spectrum[RangeRand(32)]);
		}
		CEND(uCopList);
		printf("\n");

		viewport = ViewPortAddress(win);
		Forbid();
		viewport->UCopIns = uCopList;
		Permit();
		error = VideoControl(viewport->ColorMap, uCopTags);
		RemakeDisplay();
		if (error) {
			printf("Error when setting copper list clipping\n");
			QUIT();
		}
	}
}



VOID processWindow()
{
struct IntuiMessage *msg;

	FOREVER {
		Wait(1L << win->UserPort->mp_SigBit);
		while (NULL != (msg = (struct IntuiMessage *)GT_GetIMsg(win->UserPort)))
		{
			if (msg->Class == IDCMP_CLOSEWINDOW)
			{
				GT_ReplyIMsg(msg);
				return;
			}
			else {
				GT_ReplyIMsg(msg);
			}
		}
	}
}



void OpenView()
{
	if (! (	scrmodereq = AllocAslRequestTags(ASL_ScreenModeRequest,
		ASLSM_TitleText, "Lines screen mode",
/*		ASLSM_DoWidth, TRUE, */
/*		ASLSM_DoHeight, TRUE, */
		ASLSM_DoDepth, TRUE,
		ASLSM_DoOverscanType, TRUE,
		ASLSM_DoAutoScroll, TRUE,
		ASLSM_PropertyFlags, NULL,
		ASLSM_PropertyMask, NULL,
		ASLSM_PubScreenName, "Workbench",
		TAG_DONE))) {
		printf("Couldn't allocate screen mode requester\n");
		QUIT();
	}
	if (FALSE == (AslRequestTags(scrmodereq,
		TAG_DONE))) {
		if (! (scr = OpenScreenTags(NULL,
			SA_ShowTitle, FALSE,
			SA_Interleaved, TRUE,
			TAG_DONE))) {
			printf("Couldn't open screen\n");
			QUIT();
		}
		if (! (win = OpenWindowTags(NULL,
			WA_CustomScreen, scr,
			WA_Backdrop, TRUE,
			WA_Borderless, TRUE,
			WA_Activate, TRUE,
			WA_RMBTrap, TRUE,
			WA_CloseGadget, TRUE,
			WA_IDCMP, IDCMP_CLOSEWINDOW,
			TAG_DONE))) {
			printf("Couldn't open window\n");
			QUIT();
		}
	}
	else {
		if (! (scr = OpenScreenTags(NULL,
			SA_ShowTitle, FALSE,
			SA_Width,		scrmodereq->sm_DisplayWidth,
			SA_Height,		scrmodereq->sm_DisplayHeight,
			SA_Depth,		scrmodereq->sm_DisplayDepth,
			SA_DisplayID,	scrmodereq->sm_DisplayID,
			SA_Overscan,	scrmodereq->sm_OverscanType,
			SA_AutoScroll,	scrmodereq->sm_AutoScroll,
			SA_Interleaved, TRUE,
			TAG_DONE))) {
			printf("Couldn't open screen\n");
			QUIT();
		}
		if (! (win = OpenWindowTags(NULL,
			WA_CustomScreen, scr,
			WA_Backdrop, TRUE,
			WA_Borderless, TRUE,
			WA_Activate, TRUE,
			WA_RMBTrap, TRUE,
			WA_CloseGadget, TRUE,
			WA_IDCMP, IDCMP_CLOSEWINDOW,
			TAG_DONE))) {
			printf("Couldn't open window\n");
			QUIT();
		}
	}
}

void OpenLibs()
{
	if (! (MathIeeeSingTransBase = (struct Library *)OpenLibrary("mathieeesingtrans.library",0L))) {
		printf("Couldn't open mathieeesingtrans library\n");
		QUIT();
	}
	if (! (GadToolsBase = (struct GadToolsBase *)OpenLibrary("gadtools.library",0L))) {
		printf("Couldn't open Gadtools library\n");
		QUIT();
	}
	if (! (AslBase = (struct AslBase *)OpenLibrary("asl.library",38L))) {
		printf("Couldn't open Asl library\n");
		QUIT();
	}
	if (! (LayersBase = (struct LayersBase *)OpenLibrary("layers.library",0L))) {
		printf("Couldn't open layers library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L))) {
		printf("Couldn't open graphics library\n");
		QUIT();
	}
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		printf("Couldn't open intuition library\n");
		QUIT();
	}
}

void CloseLibs()
{
	if (MathIeeeSingTransBase)	{
		CloseLibrary(MathIeeeSingTransBase);
	}
	if (GadToolsBase) {
		CloseLibrary((struct Library *)GadToolsBase);
	}
	if (AslBase) {
		CloseLibrary((struct Library *)AslBase);
	}
	if (LayersBase) {
		CloseLibrary((struct Library *)LayersBase);
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
	struct ViewPort *viewport = NULL;

	if (scrmodereq) {
		FreeAslRequest(scrmodereq);
	}
	if (win) {
		viewport = ViewPortAddress(win);
		if (viewport->UCopIns) {
			WaitBlit();
			FreeVPortCopLists(viewport);
			viewport->UCopIns = NULL;
			RemakeDisplay();
		}
		CloseWindow(win);
	}
	if (scr) {
		CloseScreen(scr);
	}
	CloseLibs();
	exit(0);
}

void main()
{
	OpenLibs();
	OpenView();
	loadCopper();
	processWindow();
	QUIT();
}