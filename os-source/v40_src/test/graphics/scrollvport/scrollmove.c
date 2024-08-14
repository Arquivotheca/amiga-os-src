#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/clip.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/displayinfo.h>
#include <graphics/rastport.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAXRADIUS 100
#define AREA_SIZE 200

extern struct DOSBase *DOSBase;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct AslBase *AslBase = NULL;
struct LayersBase *LayersBase = NULL;
struct GadToolsBase *GadToolsBase = NULL;

struct ScreenModeRequester *scrmodereq = NULL;
BOOL useDefaults = TRUE;
struct Screen *scr = NULL;
struct Window *win = NULL;

void OpenLibs(void);
void CloseLibs(void);
void QUIT(void);
void doScreenModeReq(void);
void doDisplay(void);
void doTest(void);

void OpenLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		printf("Couldn't open intuition library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L))) {
		printf("Couldn't open graphics library\n");
		QUIT();
	}
	if (! (AslBase = (struct AslBase *)OpenLibrary("asl.library",0L))) {
		printf("Couldn't open asl library\n");
		QUIT();
	}
	if (! (LayersBase = (struct LayersBase *)OpenLibrary("layers.library",0L))) {
		printf("Couldn't open layers library\n");
		QUIT();
	}
	if (! (GadToolsBase = (struct GadToolsBase *)OpenLibrary("gadtools.library",0L))) {
		printf("Couldn't open gadtools library\n");
		QUIT();
	}
}

void CloseLibs()
{
	if (GadToolsBase) {
		CloseLibrary((struct Library *)GadToolsBase);
	}
	if (LayersBase) {
		CloseLibrary((struct Library *)LayersBase);
	}
	if (AslBase) {
		CloseLibrary((struct Library *)AslBase);
	}
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
		CloseLibrary ((struct Library *)IntuitionBase);
	}
}

void QUIT()
{
	if (scrmodereq) {
		FreeAslRequest(scrmodereq);
	}
	if (win) {
		CloseWindow(win);
	}
	if (scr) {
		CloseScreen(scr);
	}
	CloseLibs();
	exit(0);
}

void doScreenModeReq()
{
	BOOL result = FALSE;

	if (scrmodereq = AllocAslRequestTags(ASL_ScreenModeRequest,
		TAG_END)) {
		if (result = AslRequestTags(scrmodereq,
			ASLSM_PubScreenName, "Workbench",
			ASLSM_DoWidth, TRUE,
			ASLSM_DoHeight, TRUE,
			ASLSM_DoDepth, TRUE,
			ASLSM_DoOverscanType, TRUE,
			ASLSM_DoAutoScroll, TRUE,
			ASLSM_PropertyFlags, NULL,
			TAG_END)) {
			useDefaults = FALSE;
		}
	}
}

void doDisplay()
{
	if (useDefaults) {
		if (NULL == (scr = OpenScreenTags(NULL,
			SA_Depth, 4L,
			SA_Interleaved, TRUE,
			SA_ShowTitle, FALSE,
			TAG_END))) {
			printf("Couldn't open screen\n");
			QUIT();
		}
		else {
			if (NULL == (win = OpenWindowTags(NULL,
				WA_CustomScreen, scr,
				WA_Title, "Press Esc to QUIT",
				WA_Activate, TRUE,
				WA_IDCMP, IDCMP_VANILLAKEY,
				TAG_END))) {
				printf("Couldn't open window\n");
				QUIT();
			}
		}

	}
	else {
		if (NULL == (scr = OpenScreenTags(NULL,
			SA_DisplayID, scrmodereq->sm_DisplayID,
			SA_Width, scrmodereq->sm_DisplayWidth,
			SA_Height, scrmodereq->sm_DisplayHeight,
			SA_Depth, scrmodereq->sm_DisplayDepth,
			SA_Overscan, scrmodereq->sm_OverscanType,
			SA_AutoScroll, scrmodereq->sm_AutoScroll,
			SA_Interleaved, TRUE,
			SA_ShowTitle, FALSE,
			TAG_END))) {
			printf("Couldn't open screen\n");
			QUIT();
		}
		else {
			if (NULL == (win = OpenWindowTags(NULL,
				WA_CustomScreen, scr,
				WA_Activate, TRUE,
				WA_Title, "Press Esc to QUIT",
				WA_IDCMP, IDCMP_VANILLAKEY,
				TAG_END))) {
				printf("Couldn't open window\n");
				QUIT();
			}
		}
	}
}

void doTest()
{
	struct IntuiMessage *imsg = NULL;
	ULONG class = 0;
	ULONG code = 0;


	FOREVER {
		if (NULL == (imsg = GT_GetIMsg(win->UserPort))) {
			scr->ViewPort.RasInfo->RxOffset = RangeRand(scr->Width/2);
			scr->ViewPort.RasInfo->RyOffset = RangeRand(scr->Height/2);
			ScrollVPort(&scr->ViewPort);
		}
		else {
			class = imsg->Class;
			code =imsg->Code;
			GT_ReplyIMsg(imsg);
			switch(class) {
				case IDCMP_VANILLAKEY:
					if (code == 0x1B) {
						QUIT();
					}
					break;
				default:
					break;
			}
		}
	}
}

void main()
{
	OpenLibs();

	doScreenModeReq();
	doDisplay();

	doTest();

	QUIT();
}