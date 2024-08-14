/* prep window test program */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <libraries/gadtools.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/icon_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "hdtoolbox_rev.h"

#include "refresh.h"
#include "protos.h"
#include <clib/alib_protos.h>

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
       struct Library *IconBase;
       struct Library *GadToolsBase;

char St506Text[6];			// from newprep.c

extern struct WBStartup *WBenchMsg;

#define BG_COLOR	0
#define GAD_COLOR	0
#define BORDER_COLOR	1
#define DS_COLOR	1

char *Vers = VERSTAG;

ULONG colors[4] = { BG_COLOR,GAD_COLOR,BORDER_COLOR,DS_COLOR };

void main (int,char **);

//
void *vi = NULL;
UWORD XtraTop;
extern struct Gadget *glist1;
//

struct TextAttr TOPAZ80 = {	/* 39.13 */
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};

struct TextFont *tfont = NULL;	/* 39.13 */

void PutTextFont(struct StringExtend *tf)	/* 39.13 */
{
	tf->Font = tfont;
}

void
main(argc,argv)
	int argc;
	char *argv[];
{
	struct Window *w;
	struct DiskObject *diskobj = NULL;
	char *str,*wbname;

	struct Screen *screen = NULL;		// for gadtools
	UWORD ZoomSize[4] = { 0, 0, 200, 0 };	// for OpenWindowTags()

#ifdef MEMCHECK
	mem_init(2048L);
#endif
	IntuitionBase = (struct IntuitionBase *)
			OpenLibrary("intuition.library",37L);
	if (!IntuitionBase)
		_exit(200);

	GfxBase = (struct GfxBase *)
		  OpenLibrary("graphics.library",37L);
	if (!GfxBase)
	{
		CloseLibrary((struct Library *) IntuitionBase);
		_exit(100);
	}
	IconBase = OpenLibrary("icon.library",37L);
	if (!IconBase)
	{
		CloseLibrary((struct Library *) IntuitionBase);
		CloseLibrary((struct Library *) GfxBase);
		_exit(50);
	}

// These stuff is for gadtools.library and 2.0 types gadgetes.
	GadToolsBase = OpenLibrary("gadtools.library",37L);
	if (!GadToolsBase)
	{
		CloseLibrary((struct Library *) IntuitionBase);
		CloseLibrary((struct Library *) GfxBase);
		CloseLibrary((struct Library *) IconBase);
		_exit(50);
	}

	/* Open topaz 8 font: 39.13 */
	if (!(tfont = OpenFont(&TOPAZ80)))
	{
		CloseLibrary((struct Library *) IntuitionBase);
		CloseLibrary((struct Library *) GfxBase);
		CloseLibrary((struct Library *) IconBase);
		CloseLibrary((struct Library *) GadToolsBase);
		_exit(50);
	}

	screen = LockPubScreen(NULL);
	if (!screen)
	{
		CloseLibrary((struct Library *) IntuitionBase);
		CloseLibrary((struct Library *) GfxBase);
		CloseLibrary((struct Library *) IconBase);
		CloseLibrary((struct Library *) GadToolsBase);
		CloseFont(tfont);
		_exit(50);
	}
	vi = GetVisualInfo(screen, TAG_DONE);
	if (!vi)
	{
		CloseLibrary((struct Library *) IntuitionBase);
		CloseLibrary((struct Library *) GfxBase);
		CloseLibrary((struct Library *) IconBase);
		CloseLibrary((struct Library *) GadToolsBase);
		CloseFont(tfont);
		UnlockPubScreen(NULL, screen);
		_exit(50);
	}
//

	/* parse arguements, read (optionally) scsi device name,addrs,luns */
	/* works for both WB and CLI */
	if (argc > 1)
	{
		if (argc > 5)
		{
			printf("Bad arguements!\n");
			_exit(20);
		}
	        if (*argv[1] == '?') {
                  printf("Usage: %s [driver.device] [MaxID] [MaxLUN] [XT.device]\n",
                    argv[0]);
                  _exit(0);
                  }

		stccpy(scsi_device,argv[1],33);
		if (argc > 2)
			scsi_addrs  = atoi(argv[2]) + 1;
		if (argc > 3)
			scsi_luns   = atoi(argv[3]) + 1;
		if (argc > 4)
			stccpy(xt_name,argv[4],7);
	} else {
		if (argc == 1)
		{
			/* read WBObject anyway to get defaults */
			wbname = argv[1];
		} else if (argc == 0) {
			/* argv == WBenchMsg */
			wbname = WBenchMsg->sm_ArgList->wa_Name;
		}
		if (wbname && (diskobj = GetDiskObject(wbname)))
		{
			str = FindToolType(diskobj->do_ToolTypes,
					   "SCSI_DEVICE_NAME");
			if (str)
				stccpy(scsi_device,str,33);

			str = FindToolType(diskobj->do_ToolTypes,
					   "SCSI_MAX_ADDRESS");
			if (str)
				scsi_addrs = atoi(str) + 1;

			str = FindToolType(diskobj->do_ToolTypes,
					   "SCSI_MAX_LUN");
			if (str)
				scsi_luns = atoi(str) + 1;

			str = FindToolType(diskobj->do_ToolTypes,
					   "XT_NAME");
			if (str)
				stccpy(xt_name,str,7);
		}
	}

	/* set st-506/xt gadget on prep screen to right text */
//	St506_IText.IText = xt_name;
	strcpy(St506Text,xt_name);

	/* null out so we can use HandleScreen (which adds them) */
//	gad = NewWindowStructure1.FirstGadget;
//	NewWindowStructure1.FirstGadget = NULL;

//	w = OpenWindow(&NewWindowStructure1);

	/* For new gadtools type gadgets */
	XtraTop = screen->WBorTop + (screen->Font->ta_YSize + 1) - 11;
	ZoomSize[3] = XtraTop + 11;
	do_reposition(&IntuiTextList2);
	do_repositionRequester(&RequesterStructure6);
	do_repositionRequester(&RequesterStructure7);
	do_repositionRequester(&RequesterStructure77);
	do_repositionRequester(&RequesterStructure9);
	do_repositionRequester(&RequesterStructure11);
	do_repositionRequester(&RequesterStructure13);
//	PutTextFont(&StringExt);
	StringExt.Font = tfont;

	w = OpenWindowTags(&NewWindowStructure1,
			WA_Zoom, ZoomSize,
			WA_InnerWidth, NewWindowStructure1.Width - 8,
			WA_InnerHeight, NewWindowStructure1.Height - 24,
			WA_AutoAdjust, TRUE,
			WA_PubScreen, screen,
			TAG_DONE);

	GT_RefreshWindow(w,NULL);
//
	if (w) {
		struct Gadget *gtmp = NULL;
//		SetMenuStrip(w,&MenuList1);	// Right now, no menu.

		HandleWindowNew(w,NULL,&glist1,
				NewWindowStructure1.Title,
				NewWindowStructure1.IDCMPFlags,	// 39.10
				HandlePrep,
				NULL,
				PrepInit,
				CreateAllGadgets1,&gtmp,NULL);
//		ClearMenuStrip(w);		// Right now, no menu.

		/* we're exiting, clean up */
		FreeEverything();
		CloseWindow(w);
	}

	if (diskobj)
		FreeDiskObject(diskobj);

	/* Clean up gadtools stuff */
	FreeVisualInfo(vi);
	CloseLibrary((struct Library *) GadToolsBase);
	if (screen) UnlockPubScreen(NULL, screen);
	if (tfont) CloseFont(tfont);	/* 39.13 */
//
	CloseLibrary((struct Library *) IconBase);
	CloseLibrary((struct Library *) GfxBase);
	CloseLibrary((struct Library *) IntuitionBase);

#ifdef MEMCHECK
	mem_free();
#endif
}

void
FreeEverything ()
{
	register struct Drive *d,*nextd = NULL;
	register struct DriveDef *dd,*nextdd = NULL;

	for (d = FirstDrive; d; d = nextd)
	{
		nextd = d->NextDrive;
		FreeRDB(d->rdb);
		FreeRDB(d->oldrdb);

		FreeBad(d->bad);
		FreeMem((char *) d, sizeof(*d));
	}

	for (dd = SCSI_Defs; dd; dd = nextdd)
	{
		nextdd = dd->Succ;
		FreeRDB(dd->Initial_RDB);
		FreeMem((char *) dd, sizeof(*dd));
	}

	for (dd = St506_Defs; dd; dd = nextdd)
	{
		nextdd = dd->Succ;
		FreeRDB(dd->Initial_RDB);
		FreeMem((char *) dd, sizeof(*dd));
	}

	FreeCommit();
}

/* allocate 80 chars per line of commit text that will be used */

int
AllocCommit ()
{
	register short i;

	if (CommitText[FIRST_LINE] =
		AllocMem((LAST_LINE - FIRST_LINE + 1) * 80,
			 MEMF_CLEAR|MEMF_PUBLIC))
	{
		for (i = FIRST_LINE+1; i <= LAST_LINE; i++)
			CommitText[i] = CommitText[FIRST_LINE] +
					(i - FIRST_LINE) * 80;

		return TRUE;
	}
	return FALSE;
}

void
FreeCommit ()
{
	if (CommitText[FIRST_LINE])
		FreeMem(CommitText[FIRST_LINE],
			(LAST_LINE - FIRST_LINE + 1) * 80);
	CommitText[FIRST_LINE] = NULL;
}


#if 0
void delay(long time)
{
	long i;

	for (i = 0L; i != time*2000; i++);
}
#endif

#if 0
int  window_size = 0;	/* 1 means window is small */
char OldWindowTitle[100];


/* handle menu selections - returns whether it made the window big */

int HandleMenus(struct Window *w, struct IntuiMessage *msg)
{
    UWORD menunumber;
    struct MenuItem *Item;
    int made_small = FALSE;
    int made_big   = FALSE;

    /* stop selections while handling - note: may still get a second */

    ClearMenuStrip(w);

    menunumber = msg->Code;
    while (menunumber != MENUNULL)
    {
	Item = ItemAddress(&MenuList1,menunumber);

	if (window_size == 0 && ITEMNUM(menunumber) == 0)
	{
	    /* make smaller */

	    if (w->Width == NewWindowStructure1.Width)
	    {
		SizeWindow(w, -NewWindowStructure1.Width + 104,
				-NewWindowStructure1.Height + 11);
		MenuItem1.Flags &= ~ITEMENABLED;
		MenuItem2.Flags |= ITEMENABLED;
	    }

	    made_small = TRUE;
	    window_size = 1;
	}

	else if (window_size == 1 && ITEMNUM(menunumber) == 1)
	{
	    /* make bigger */

	    if (w->Width != NewWindowStructure1.Width)
	    {
	        /* must first move to upper left corner.  REAL dangerous!! */

		MoveWindow(w,-(w->LeftEdge),-(w->TopEdge));
		SizeWindow(w, NewWindowStructure1.Width - 104,
				NewWindowStructure1.Height - 11);
		MenuItem1.Flags |= ITEMENABLED;
		MenuItem2.Flags &= ~ITEMENABLED;
	    }

	    made_big = TRUE;
	    window_size = 0;
	}

	menunumber = Item->NextSelect;
    }

    SetMenuStrip(w,&MenuList1);

    /* put up a little display, eat input other than menus */
    /* initial msg variable isn't needed, reuse */

    if (made_small)
    {
	LittleDisplay(w);

	while (made_small)
	{
	    WaitPort(w->UserPort);
	    while (made_small && (msg = (struct IntuiMessage *)GetMsg(w->UserPort)) != NULL)
	    {
	    /* exit when Handlemenus returns TRUE */

		if (msg->Class == MENUPICK)
		    made_small = !(HandleMenus(w,msg));
		ReplyMsg((struct Message *) msg);
	    }
	}
	SetWindowTitles(w,OldWindowTitle,(char *) -1);
    }

    return (made_big);
}

void LittleDisplay (struct Window *w)
{
    char *Title, *OldTitle = OldWindowTitle;

    Title = (char *)w->Title;

    do {
	*OldTitle++ = *Title;
    } while (*Title++);

    SetWindowTitles(w,"HDToolBox",(char *) -1);
}
#endif
