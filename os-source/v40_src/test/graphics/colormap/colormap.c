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

#define NUMENTRIES 255

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct AslBase *AslBase = NULL;
struct GadToolsBase *GadToolsBase = NULL;

struct Screen *scr = NULL;
struct Window *win = NULL;

void OpenLibs(void);
void CloseLibs(void);
void QUIT(void);
void openDisplay(void);
void doTest(void);

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
	CloseLibs();
	exit(0);
}

void doTest()
{
	struct ColorMap *cm = NULL;
	int i = 0;

	for (i = 0; i < 10000; i++) {
		cm = GetColorMap(NUMENTRIES);
		FreeColorMap(cm);
	}
}

void main()
{
	OpenLibs();
	doTest();
	QUIT();
}