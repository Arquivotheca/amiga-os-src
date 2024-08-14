#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/asl.h>
#include <clib/alib_stdio_protos.h>
#include <clib/asl_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#pragma libcall GfxBase WriteChunkyPixels 420 4A3210807


#define DEBUG TRUE
#define MAXDEPTH 8
#define NUMREFRESH 3
#define RECTWIDTH 10
#define RECTHEIGHT 10

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

struct Screen *TestScreen = NULL;
struct Window *TestWindow1 = NULL;
struct Window *TestWindow2 = NULL;
struct Window *TestWindow3 = NULL;
struct RastPort *rp1 = NULL;
struct RastPort *rp2 = NULL;
struct RastPort *rp3 = NULL;
struct BitMap   *tempbm1 = NULL;
struct BitMap   *tempbm2 = NULL;
struct BitMap   *tempbm3 = NULL;
struct RastPort *temprp1 = NULL;
struct RastPort *temprp2 = NULL;
struct RastPort *temprp3 = NULL;
UBYTE pixelarray[((((RECTWIDTH+15)>>4)<<4)*(RECTHEIGHT + 1))];
struct ScreenModeRequester *smr;
ULONG PixCheckSum = 0;
int NumTests = 0;
int numbytes = 0;
int depth = 0;

void OpenLibs(void);
void ChooseScreenMode(void);
void KillTestWindow(struct Window *, struct RastPort *, struct BitMap *);
ULONG GetChecksum(struct Window *);
void PrintScreenAttrs(struct Screen *);
void DoTest(int,int,int,int,int);
void QUIT(void);
//extern void KPrintF(STRPTR,...);

void OpenLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L)))
	{
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L)))
	{
		QUIT();
	}
}

void ChooseScreenMode()
{
	if (!(smr = (struct ScreenModeRequester *)AllocAslRequestTags( ASL_ScreenModeRequest,
		ASLSM_TitleText, "Choose Screen Mode",
		TAG_DONE )))
	{
		printf("Could not allocate asl request tags\n");
		QUIT();
	}
	if (!(AslRequest(smr,0L)))
	{
		printf("No screen mode chosen - program aborted.\n");
		FreeAslRequest(smr);
		QUIT();
	}
}

void KillTestWindow(struct Window *wnd,struct RastPort *rp, struct BitMap *bm)
{
	int i = 0;

#ifdef DEBUG
	printf("KillTestWindow()\n");
#endif /* DEBUG */

	if (rp) {
		SetWrMsk(rp,(ULONG)-1);
	}
	if (wnd) {
		if (bm) {
			for (i = 0; i < depth; i++) {
				FreeRaster((PLANEPTR)bm->Planes[i],(ULONG)wnd->Width, (ULONG)wnd->Height);
         }
			FreeMem(bm,sizeof(struct BitMap));
		}
		if (rp) {
			FreeMem(rp,sizeof(struct RastPort));
		}
	   CloseWindow(wnd);
	}
}

ULONG GetChecksum(struct Window *win)
{
	int x = 0;
	int y = 0;
	ULONG checksum = 0;

	for (x = 0; x <= (win->Width); x++) {
		for (y = 0; y <= (win->Height); y++) {
			checksum = checksum + (ULONG)ReadPixel(win->RPort,x,y);
		}
	}
	return(checksum);
}

void PrintScrAttrs(struct Screen *scr)
{
	printf("scr->LeftEdge   %d\n",scr->LeftEdge);
	printf("scr->TopEdge    %d\n",scr->TopEdge);
	printf("scr->Width      %d\n",scr->Width);
	printf("scr->Height     %d\n",scr->Height);
	printf("scr->Flags      %d\n",scr->Flags);
	printf("scr->WBorTop    %d\n",scr->WBorTop);
	printf("scr->WBorLeft   %d\n",scr->WBorLeft);
	printf("scr->WBorRight  %d\n",scr->WBorRight);
	printf("scr->WBorBottom %d\n",scr->WBorBottom );
}

void DoPlot(struct Window *win, struct RastPort *rp, struct RastPort *tmprp) {
int count = 0;
int x = 0;
int y = 0;
int i = 0;

#ifdef DEBUG
	printf("DoPlot\n");
#endif /* DEBUG */

	for (x = 10; x <= (win->Width - 21); x += 11) {
		for (y = 20; y <= (win->Height - 11); y += 11) {

//			count = WritePixelArray8(rp, x, y, x + 10, y + 10, pixelarray, tmprp);
			count = WriteChunkyPixels(rp, x, y, x + 10, y + 10, pixelarray,16 );
			for (i = 0; i < numbytes; i++) {
				pixelarray[i] = i;
			}
		}
	}
}

void DoTest(int depth,int layered,int refresh,int randomSize,int maskval)
{
	ULONG wflags = 0;
	ULONG Mask = 0;
	int i = 0;

#ifdef DEBUG
	printf("Display ID = 0x%08lx\n",(LONG)smr->sm_DisplayID);
#endif /* DEBUG */
	printf("Display ID = 0x%08x\n",smr->sm_DisplayID);

	/* Open Screen with depth of depth */
	if (!(TestScreen = OpenScreenTags(NULL,
		SA_DisplayID, (smr->sm_DisplayID),
		SA_Depth, depth,
		SA_Title, "Test Screen",
  		TAG_END)))
	{
  		printf("Could not open test screen");
		QUIT();
	}

	printf("Screen Attributes:\n");
	PrintScrAttrs(TestScreen);

	printf("%3ld> ", (LONG)depth);
	printf("%3d> ", depth);

	switch (layered)
	{
		case 0:
#ifdef DEBUG
			printf("Non-layered ");
#endif /* DEBUG */
			printf("Non-layered ");
			break;
		case 1:
#ifdef DEBUG
			printf("Layered     ");
#endif /* DEBUG */
			printf("Layered     ");
			break;
		case 2:
#ifdef DEBUG
			printf("Obscured    ");
#endif /* DEBUG */
			printf("Obscured    ");
		default:
			break;
	}
	switch (refresh)
	{
		case 0:
#ifdef DEBUG
			printf("Simple Refresh ");
#endif /* DEBUG */
			printf("Simple Refresh ");
			wflags = wflags || WFLG_SIMPLE_REFRESH;
			break;
		case 1:
#ifdef DEBUG
			printf("Smart Refresh  ");
#endif /* DEBUG */
			printf("Smart Refresh  ");
			wflags = wflags || WFLG_SMART_REFRESH;
			break;
		case 2:
#ifdef DEBUG
			printf("SuperBitMap    ");
#endif /* DEBUG */
			printf("SuperBitMap    ");
			wflags = wflags || WFLG_SUPER_BITMAP;
			break;
		default:
			break;
	}
	switch (maskval)
	{
		case 0:
#ifdef DEBUG
	printf("Mask value = 0xAE ");
#endif /* DEBUG */
			printf("Mask value = 0xAE ");
			Mask = 0xAE;
			break;
		case 1:
#ifdef DEBUG
	printf("Mask value = 0x43 ");
#endif /* DEBUG */
			printf("Mask value = 0x43 ");
			Mask = 0x43;
			break;
		case 2:
#ifdef DEBUG
			printf("Mask value = -1   ");
#endif /* DEBUG */
			printf("Mask value = -1   ");
			Mask = -1;
			break;
		default:
			break;
	}
#ifdef DEBUG
	printf("\n");
#endif /* DEBUG */
	printf("\n");

	/* Open Windows on the screen */

	if (! (TestWindow1 = OpenWindowTags(NULL,
		WA_Flags, wflags,
		WA_Backdrop, TRUE,
		WA_Title, "Test Window 1 -",
		WA_CustomScreen, TestScreen,
		TAG_END)))
	{
#ifdef DEBUG
		printf("Couldn't open TestWindow1\n");
#endif /* DEBUG */
		QUIT();
	}
	else {
		rp1 = TestWindow1->RPort;
		SetWrMsk(rp1,Mask);
		tempbm1 = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_CHIP || MEMF_CLEAR);
		if (! tempbm1) {
			QUIT();
		}
		InitBitMap(tempbm1,depth,(LONG)TestWindow1->Width,(LONG)TestWindow1->Height);
		temprp1 = (struct RastPort *)AllocMem(sizeof(struct RastPort), MEMF_CHIP || MEMF_CLEAR);
		if (! temprp1) {
			QUIT();
		}
		InitRastPort(temprp1);
		/* copy rp1 info to temprp1 */
		temprp1->AreaPtrn = rp1->AreaPtrn;
		temprp1->AreaInfo = rp1->AreaInfo;
		temprp1->GelsInfo = rp1->GelsInfo;
		temprp1->Mask = rp1->Mask;
		temprp1->FgPen = rp1->FgPen;
		temprp1->BgPen = rp1->BgPen;
		temprp1->AOlPen = rp1->AOlPen;
		temprp1->DrawMode = rp1->DrawMode;
		temprp1->AreaPtSz = rp1->AreaPtSz;
		temprp1->linpatcnt = rp1->linpatcnt;
		temprp1->Flags = rp1->Flags;
		temprp1->LinePtrn = rp1->LinePtrn;
		temprp1->cp_x = rp1->cp_x;
		temprp1->cp_y = rp1->cp_y;
		temprp1->PenWidth = rp1->PenWidth;
		temprp1->PenHeight = rp1->PenHeight;

		temprp1->BitMap = tempbm1;
		temprp1->Layer = NULL;
		temprp1->BitMap->BytesPerRow = numbytes;
		for (i = 0; i < depth; i++) {
			tempbm1->Planes[i] = NULL;
		}
		for (i = 0; i < depth; i++) {
			tempbm1->Planes[i] = (PLANEPTR)AllocRaster((ULONG)TestWindow1->Width, (ULONG)TestWindow1->Height);
			if (!tempbm1->Planes[i]) {
#ifdef DEBUG
				printf("Couldn't allocate plane for tempbm1\n");
#endif /* DEBUG */
				QUIT();
			}
		}
	}

	if (layered > 0)
	{
		if (! (TestWindow2 = OpenWindowTags(NULL,
			WA_SizeGadget, FALSE,
			WA_Flags, wflags,
			WA_Left, 50,
			WA_Top, 50,
			WA_Width, 50,
			WA_Height, 50,
			WA_Title, "Test Window 2 -",
			WA_CustomScreen, TestScreen,
			TAG_END)))
		{
#ifdef DEBUG
		printf("Couldn't open TestWindow2\n");
#endif /* DEBUG */
			QUIT();
		}
		else {
			rp2 = TestWindow2->RPort;
			SetWrMsk(rp2,Mask);
			tempbm2 = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_CHIP || MEMF_CLEAR);
			if (! tempbm2) {
				QUIT();
			}
			InitBitMap(tempbm2,depth,(LONG)TestWindow2->Width,(LONG)TestWindow2->Height);
			temprp2 = (struct RastPort *)AllocMem(sizeof(struct RastPort), MEMF_CHIP || MEMF_CLEAR);
			if (!temprp2) {
				QUIT();
			}
			InitRastPort(temprp2);
			/* copy rp2 info to temprp2 */
			temprp2->AreaPtrn = rp2->AreaPtrn;
			temprp2->AreaInfo = rp2->AreaInfo;
			temprp2->GelsInfo = rp2->GelsInfo;
			temprp2->Mask = rp2->Mask;
			temprp2->FgPen = rp2->FgPen;
			temprp2->BgPen = rp2->BgPen;
			temprp2->AOlPen = rp2->AOlPen;
			temprp2->DrawMode = rp2->DrawMode;
			temprp2->AreaPtSz = rp2->AreaPtSz;
			temprp2->linpatcnt = rp2->linpatcnt;
			temprp2->Flags = rp2->Flags;
			temprp2->LinePtrn = rp2->LinePtrn;
			temprp2->cp_x = rp2->cp_x;
			temprp2->cp_y = rp2->cp_y;
			temprp2->PenWidth = rp2->PenWidth;
			temprp2->PenHeight = rp2->PenHeight;

			temprp2->BitMap = tempbm2;
			temprp2->Layer = NULL;
			temprp2->BitMap->BytesPerRow = numbytes;
			for (i = 0; i < depth; i++) {
				tempbm2->Planes[i] = NULL;
			}
			for (i = 0; i < depth; i++) {
				tempbm2->Planes[i] = (PLANEPTR)AllocRaster((ULONG)TestWindow2->Width, (ULONG)TestWindow2->Height);
				if (!tempbm2->Planes[i]) {
#ifdef DEBUG
				printf("Couldn't allocate plane for tempbm2\n");
#endif /* DEBUG */
					QUIT();
				}
			}
		}
   }

	if (layered > 1)
	{
		if (! (TestWindow3 = OpenWindowTags(NULL,
			WA_SizeGadget, FALSE,
			WA_Flags, wflags,
			WA_Left, 49,
			WA_Top, 49,
			WA_Width, 100,
			WA_Height, 100,
			WA_Title, "Test Window 3 -",
			WA_CustomScreen, TestScreen,
			TAG_END)))
		{
#ifdef DEBUG
		printf("Couldn't open TestWindow3\n");
#endif /* DEBUG */
			QUIT();
		}
		else {
			rp3 = TestWindow3->RPort;
			SetWrMsk(rp3,Mask);
			tempbm3 = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_CHIP || MEMF_CLEAR);
			if (! tempbm3) {
				QUIT();
			}
			InitBitMap(tempbm3,depth,(LONG)TestWindow3->Width,(LONG)TestWindow3->Height);
			temprp3 = (struct RastPort *)AllocMem(sizeof(struct RastPort), MEMF_CHIP || MEMF_CLEAR);
			if (!temprp3) {
				QUIT();
			}
			InitRastPort(temprp3);
			/* copy rp3 info to temprp3 */
			temprp3->AreaPtrn = rp3->AreaPtrn;
			temprp3->AreaInfo = rp3->AreaInfo;
			temprp3->GelsInfo = rp3->GelsInfo;
			temprp3->Mask = rp3->Mask;
			temprp3->FgPen = rp3->FgPen;
			temprp3->BgPen = rp3->BgPen;
			temprp3->AOlPen = rp3->AOlPen;
			temprp3->DrawMode = rp3->DrawMode;
			temprp3->AreaPtSz = rp3->AreaPtSz;
			temprp3->linpatcnt = rp3->linpatcnt;
			temprp3->Flags = rp3->Flags;
			temprp3->LinePtrn = rp3->LinePtrn;
			temprp3->cp_x = rp3->cp_x;
			temprp3->cp_y = rp3->cp_y;
			temprp3->PenWidth = rp3->PenWidth;
			temprp3->PenHeight = rp3->PenHeight;

			temprp3->BitMap = tempbm3;
			temprp3->Layer = NULL;
			temprp3->BitMap->BytesPerRow = numbytes;
			for (i = 0; i < depth; i++) {
				tempbm3->Planes[i] = NULL;
			}
			for (i = 0; i < depth; i++) {
				tempbm3->Planes[i] = (PLANEPTR)AllocRaster((ULONG)TestWindow3->Width, (ULONG)TestWindow3->Height);
				if (!tempbm3->Planes[i]) {
#ifdef DEBUG
				printf("Couldn't allocate plane for tempbm3\n");
#endif /* DEBUG */
					QUIT();
				}
			}
		}
   }


	/**************** Do plotting *******************/
   if (rp1) {
#ifdef DEBUG
	printf("Entering plot routine\n");
#endif /* DEBUG */
		DoPlot(TestWindow1, rp1, temprp1);
		PixCheckSum = GetChecksum(TestWindow1);
#ifdef DEBUG
	printf("PixCheckSum = 0x%08lx\n",PixCheckSum);
#endif /* DEBUG */
		printf("\tChecksum for window 1 = 0x%08x\n", PixCheckSum);
		PixCheckSum = 0;
	}

   if (rp2) {
#ifdef DEBUG
	printf("Entering plot routine\n");
#endif /* DEBUG */
		DoPlot(TestWindow2, rp2, temprp2);
		PixCheckSum = GetChecksum(TestWindow2);
#ifdef DEBUG
	printf("PixCheckSum = 0x%08lx\n",PixCheckSum);
#endif /* DEBUG */
		printf("\tChecksum for window 2 = 0x%08x\n", PixCheckSum);
		PixCheckSum = 0;
	}

   if (rp3) {
#ifdef DEBUG
	printf("Entering plot routine\n");
#endif /* DEBUG */
		DoPlot(TestWindow3, rp3, temprp3);
		PixCheckSum = GetChecksum(TestWindow3);
#ifdef DEBUG
	printf("PixCheckSum = 0x%08lx\n",PixCheckSum);
#endif /* DEBUG */
		printf("\tChecksum for window 3 = 0x%08x\n", PixCheckSum);
		PixCheckSum = 0;
	}

	/* Close all of the open windows and the screen */
#ifdef DEBUG
	printf("Closing TestWindow1\n");
#endif /* DEBUG */
	if (rp1) {
		SetWrMsk(rp1, -1);
	}
	KillTestWindow(TestWindow1, temprp1, tempbm1);
	TestWindow1 = NULL;
	temprp1 = NULL;
	tempbm1 = NULL;

#ifdef DEBUG
	printf("Closing TestWindow2\n");
#endif /* DEBUG */
	if (rp2) {
		SetWrMsk(rp2, -1);
	}
	KillTestWindow(TestWindow2, temprp2, tempbm2);
	TestWindow2 = NULL;
	temprp2 = NULL;
	tempbm2 = NULL;

#ifdef DEBUG
	printf("Closing TestWindow3\n");
#endif /* DEBUG */
	if (rp3) {
		SetWrMsk(rp3, -1);
	}
	KillTestWindow(TestWindow3, temprp3, tempbm3);
	TestWindow3 = NULL;
	temprp3 = NULL;
	tempbm3 = NULL;



	/* Close the test screen */

	if (TestScreen) {
#ifdef DEBUG
   	printf("Closing screen %08lx\n", TestScreen);
#endif /* DEBUG */
	  	CloseScreen(TestScreen);
		TestScreen = NULL;
   }
	if (rp1) {
#ifdef DEBUG
		printf("Setting RastPort pointer rp1 to NULL\n");
#endif /* DEBUG */
		rp1 = NULL;
	}
	if (rp2) {
#ifdef DEBUG
		printf("Setting RastPort pointer rp2 to NULL\n");
#endif /* DEBUG */
		rp2 = NULL;
	}
	if (rp3) {
#ifdef DEBUG
		printf("Setting RastPort pointer rp3 to NULL\n");
#endif /* DEBUG */
		rp3 = NULL;
	}
}

void QUIT()
{
#ifdef DEBUG
	printf("QUIT()...\n");
#endif /* DEBUG */

	if (smr) FreeAslRequest(smr);

	if (TestScreen) {
#ifdef DEBUG
   	printf("Closing screen %08lx\n", TestScreen);
#endif /* DEBUG */
	  	CloseScreen(TestScreen);
		TestScreen = NULL;
   }

	if (GfxBase) {
	        CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
	        CloseLibrary((struct Library *)IntuitionBase);
	}
   exit(0);
}

void main()
{
	int layered = 0;
	int refresh = 0;
	int randomSize = 0;
	int maskval = 0;
	int i = 0;

	OpenLibs();

	numbytes = ((((RECTWIDTH+15)>>4)<<4)*(RECTHEIGHT + 1));
	for (i = 0; i < numbytes; i++) {
		pixelarray[i] = i;
	}

	printf("wpixel v1.0 - WritePixelArray8 test\n");
	printf("wpixel v1.0 - WritePixelArray8 test\n");

	ChooseScreenMode();

	/* Quick test! */
/*	depth = 8;
	DoTest(8,0,0,0,0);
	DoTest(8,1,0,0,1);
	DoTest(8,2,0,0,2); */

	/* The 'tie up the machine' test */
	for (depth = 1; depth <= 8; depth++) {
		for (layered = 0; layered <= 2; layered++) {
			for (refresh = 0; refresh < NUMREFRESH; refresh++) {
				for (maskval = 0; maskval < 3; maskval++) {
					NumTests++;
					DoTest(depth, layered, refresh, randomSize, maskval);
				}
			}
		}
	}

#ifdef DEBUG
	printf("total tests = %ld\n", (LONG)NumTests);
#endif /* DEBUG */
	QUIT();
}