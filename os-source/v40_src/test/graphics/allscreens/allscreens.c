#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

struct Window *win = NULL;
struct Screen *scr = NULL;
ULONG id = 0;
LONG depth = 1;
LONG startdepth = 1;
LONG enddepth = 8;

void openLibs(void);
void closeLibs(void);
void dPrintf(char *);
void KPrintF(char *s, ...);
void printInfo(struct Screen *, struct Window *);
void doDraw(struct Window *);
void doScreens(void);
void QUIT(void);

void openLibs()
{
	if (! (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))) {
		dPrintf("Couldn't open intuition library\n");
		QUIT();
	}
	if (! (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L))) {
		dPrintf("Couldn't open graphics library\n");
		QUIT();
	}
}

void closeLibs()
{
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
	}
	if (IntuitionBase) {
		CloseLibrary((struct Library *)IntuitionBase);
	}
}

void dPrintf(char *s)
{
	printf("%s",s);
	KPrintF("%s",s);
}

void printInfo(struct Screen *scr, struct Window *win)
{
	struct NameInfo *buffer = NULL;
	long result = 0;

	buffer = (struct NameInfo *)AllocMem(sizeof(struct NameInfo),MEMF_PUBLIC | MEMF_CLEAR);
	if (buffer) {
	   result = GetDisplayInfoData(NULL, (UBYTE *)buffer, sizeof(struct NameInfo), DTAG_NAME, (ULONG)id);
		if (result) {
			KPrintF("Display ID = 0x%08lx -- ",id);
			printf("Display ID = 0x%08lx -- ",id);
			KPrintF("%s\n", buffer->Name);
			printf("%s\n", buffer->Name);
		}
		else {
			KPrintF("\tGetDisplayInfoData failed for following ID:\n");
			printf("\tGetDisplayInfoData failed for following ID:\n");
			KPrintF("\tDisplay ID = 0x%08lx\n",id);
			printf("\tDisplay ID = 0x%08lx\n",id);
		}
	}
	KPrintF("\tProperties of this mode are:  \n");
	printf("\tProperties of this mode are:  \n");
	KPrintF("\tDepth = %ld\n",depth);
	printf("\tDepth = %ld\n",depth);

	if (buffer) {
		FreeMem(buffer, sizeof(struct NameInfo));
		buffer = NULL;
	}
}

void doDraw(struct Window *win)
{
	SetRast(win->RPort,1);
}

void doScreens()
{
	ULONG count = 0;
	ULONG errorcode = 0;

	for (depth = startdepth; depth <= enddepth; depth++) {
		dPrintf("#################\n");
		KPrintF("### Depth = %ld ###\n",depth);
		printf("### Depth = %ld ###\n",depth);
		dPrintf("#################\n");
		id = NextDisplayInfo(INVALID_ID);
		while (id != INVALID_ID) {
			if (NULL != (scr = OpenScreenTags(NULL,
					SA_DisplayID, 		id,
					SA_Depth, 			depth,
					SA_ShowTitle, 		FALSE,
					SA_Interleaved, 	TRUE,
					SA_ErrorCode,		&errorcode,
					TAG_END))) {
				if (NULL != (win = OpenWindowTags(NULL,
					WA_CustomScreen, 	scr,
					WA_Activate, 		TRUE,
					WA_Borderless, 	TRUE,
					WA_Backdrop, 		TRUE,
					WA_SmartRefresh, 	TRUE,
					TAG_END))) {
					count++;
					KPrintF("%ld> ",count);
	 				printf("%ld> ",count);
					printInfo(scr,win);
					doDraw(win);
				}
				else {
					dPrintf("*** Couldn't open window for following display ID\n");
					count++;
	 				printf("%ld> ",count);
					KPrintF("%ld> ",count);
					printInfo(scr,win);
				}
			}
			else {
				dPrintf("*** Couldn't open screen for following display ID\n");
				count++;
				KPrintF("%ld> ",count);
 				printf("%ld> ",count);
				printInfo(scr,win);
				KPrintF("\tError Code = %ld\n",errorcode);
				printf("\tError Code = %ld\n",errorcode);
				switch ((long)errorcode) {
					case OSERR_NOMONITOR:
						dPrintf("\tOSERR_NOMONITOR\n");
						break;
					case OSERR_NOCHIPS:
						dPrintf("\tOSERR_NOCHIPS\n");
						break;
					case OSERR_NOMEM:
						dPrintf("\tOSERR_NOMEM\n");
						break;
					case OSERR_NOCHIPMEM:
						dPrintf("\tOSERR_NOCHIPMEM\n");
						break;
					case OSERR_PUBNOTUNIQUE:
						dPrintf("\tOSERR_PUBNOTUNIQUE\n");
						break;
					case OSERR_UNKNOWNMODE:
						dPrintf("\tOSERR_UNKNOWNMODE\n");
						break;
					case OSERR_TOODEEP:
						dPrintf("\tOSERR_TOODEEP\n");
						break;
					case OSERR_ATTACHFAIL:
						dPrintf("\tOSERR_ATTACHFAIL\n");
						break;
					default:
						break;
				}
			}

			if (win) {
				CloseWindow(win);
				win = NULL;
			}
			if (scr) {
				CloseScreen(scr);
				scr = NULL;
			}

			id = NextDisplayInfo(id);
		}
	}
}

void QUIT()
{
	if (win) {
		CloseWindow(win);
	}
	if (scr) {
		CloseScreen(scr);
	}
	closeLibs();
	exit(0);
}

void main(int argc, char *argv[])
{
	dPrintf(" *** AllScreens 1.0 ***\n");

	if (argc > 1) {
		startdepth = atoi(argv[1]);
		enddepth = startdepth;
		printf("startdepth = %ld\n",startdepth);
	}
	openLibs();

	doScreens();

	QUIT();
}
