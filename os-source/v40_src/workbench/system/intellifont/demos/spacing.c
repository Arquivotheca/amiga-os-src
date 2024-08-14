#define  DEBUG
#ifdef   DEBUG
#define  D(a)	kprintf a
#else
#define  D(a)
#endif
#include <exec/types.h>
#include <dos/rdargs.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include <string.h>
#undef    strchr
#undef    strcmp
#undef    strcpy
#undef    strrchr
#undef    memset

extern struct Library *SysBase;
extern struct Library *DOSBase;

#define  TEMPLATE	"NAME,TITLE"

#define  O_NAME		0
#define  O_TITLE	1
#define  O_COUNT	2

struct Library *GfxBase = 0;
struct Library *IntuitionBase = 0;
struct Library *DiskfontBase = 0;
struct RDArgs *RDArgs = 0;
struct TextFont *Font = 0;

#define  X2	640
#define  Y2	400
struct Window *Window = 0;
struct IntuiMessage *IM;

void
closeFont()
{
    if (Font)
	CloseFont(Font);
    Font = 0;
}

void
endGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format) {
	D((format, arg1, arg2, arg3, arg4));
	printf(format, arg1, arg2, arg3, arg4);
    }
    if (Window)
	CloseWindow(Window);
    closeFont();
    if (RDArgs)
	FreeArgs(RDArgs);
    if (DiskfontBase)
	CloseLibrary(DiskfontBase);
    if (IntuitionBase)
	CloseLibrary(IntuitionBase);
    if (GfxBase)
	CloseLibrary(GfxBase);
    if (format)
	exit(5);
    exit(0);
}


struct Window *
myOpenWindowTags(tag)
LONG tag;
{
    return (OpenWindowTagList(0, (struct TagItem *) & tag));
}


void
main()
{
    ULONG *options[O_COUNT];
    struct TextAttr textAttr;
    int y;
    char legend[128], *title;

    GfxBase = OpenLibrary("graphics.library", 0);
    if (!GfxBase)
	endGame("ERROR: cannot open \"graphics.library\"\n");
    IntuitionBase = OpenLibrary("intuition.library", 0);
    if (!IntuitionBase)
	endGame("ERROR: cannot open \"intuition.library\"\n");
    DiskfontBase = OpenLibrary("diskfont.library", 0);
    if (!DiskfontBase)
	endGame("ERROR: cannot open \"diskfont.library\"\n");

    memset(options, 0, sizeof(options));
    RDArgs = ReadArgs(TEMPLATE, (LONG *) options, 0);
    if (!RDArgs)
	endGame("ERROR: invalid arguments\n");

    if (options[O_NAME])
	textAttr.ta_Name = (char *) options[O_NAME];
    else
	textAttr.ta_Name = "CGTimes.font";
    textAttr.ta_Flags = textAttr.ta_Style = 0;

    if (options[O_TITLE])
	title = (char *) options[O_TITLE];
    else
	title = textAttr.ta_Name;

    Window = myOpenWindowTags(
	    WA_Left, 0,
	    WA_Top, 0,
	    WA_Width, X2,
	    WA_Height, Y2,
	    WA_IDCMP, CLOSEWINDOW,
	    WA_DepthGadget, TRUE,
	    WA_CloseGadget, TRUE,
	    WA_Title, title,
	    TAG_DONE);

    if (!Window)
	endGame("OpenWindow failed\n");

    SetAPen(Window->RPort, 1);
    SetDrMd(Window->RPort, JAM1);

    textAttr.ta_YSize = 4;
    y = Window->BorderTop + 4;
    while (y < Y2) {
	textAttr.ta_YSize++;
	Font = OpenDiskFont(&textAttr);
	if (!Font)
	    endGame("OpenDiskFont %s %ld failed\n", textAttr.ta_Name,
		    textAttr.ta_YSize);

	SetFont(Window->RPort, Font);
	Move(Window->RPort, 20, y);
	sprintf(legend, "%3ld oo xx oxo hnh ijij aeiou 0123456789 qjy\305\264 "
		"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		textAttr.ta_YSize);
	Text(Window->RPort, legend, strlen(legend));
	closeFont();
	y += textAttr.ta_YSize + 1;
    }

    /* wait until done before returning */
    for (;;) {
	while (IM = (struct IntuiMessage *) GetMsg(Window->UserPort)) {
	    if (IM->Class & CLOSEWINDOW) {
		endGame(0);
	    }
	    ReplyMsg((struct Message *) IM);
	}
	WaitPort(Window->UserPort);
    }
}
