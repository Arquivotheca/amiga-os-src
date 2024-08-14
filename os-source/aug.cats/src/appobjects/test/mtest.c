/* mtest.c
 * test program for multi-line text gadget class
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/cghooks.h>
#include <intuition/sghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/icclass.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>
#include <utility/hooks.h>
#include <libraries/gadtools.h>

#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>

extern struct Library *DOSBase, SysBase;
struct Library *IntuitionBase, *GfxBase, *UtilityBase, *GadToolsBase;
struct Library *AppObjectsBase;

#define	V(x)	((VOID *)(x))

struct NewMenu GTMenu[] =
{
    {NM_TITLE,	"Project",},
    {NM_ITEM,	"Save",			"S", 0, 0, V(0),},
    {NM_ITEM,	"Quit",			"Q", 0, 0, V(1),},

    {NM_END, 0, 0, 0, 0, 0},
};

struct Window *win;
struct Gadget *gad1, gad2;

UBYTE mbuff1[256] = "This is a test of the ability to do a wrapping string gadget.\nI will put some more initial text into the gadget to help with the problem of wrapping.";

struct TagItem wintags[] =
{
    WA_MenuHelp, TRUE,
    TAG_DONE,
};

struct ExtNewWindow NewWindow =
{
    0, 0, 400, 180,		/* Window placement rectangle */
    0, 1,			/* DetailPen, BlockPen */
    CLOSEWINDOW |		/* IDCMPFlags */
    GADGETUP | ACTIVEWINDOW |
    MENUPICK | IDCMP_MENUHELP |
    IDCMPUPDATE,

    NOCAREREFRESH | ACTIVATE |	/* Window flags */
    SIMPLE_REFRESH | WINDOWDRAG |
    WINDOWDEPTH | WINDOWCLOSE |
    WINDOWSIZING | WFLG_NW_EXTENDED,
    NULL,			/* First gadget */
    NULL,			/* Menu checkmark */
    NULL,			/* Window title */
    NULL,			/* Screen pointer */
    NULL,			/* Custom bitmap */
    200, 60,			/* Minimum width & height */
    1024, 1024,			/* Maximum width & height */
    WBENCHSCREEN,		/* Screen type */
    wintags,
};

BOOL OpenLibraries (VOID)
{
    if (IntuitionBase = OpenLibrary ("intuition.library", 36))
    {
	if (GfxBase = OpenLibrary ("graphics.library", 36))
	{
	    if (UtilityBase = OpenLibrary ("utility.library", 36))
	    {
		if (AppObjectsBase = OpenLibrary ("appobjects.library", 36))
		{
		    if (GadToolsBase = OpenLibrary ("gadtools.library", 36))
		    {
			return (TRUE);
		    }
		    else
			printf ("Couldn't open gadtools.library\n");

		    CloseLibrary (GadToolsBase);
		    GadToolsBase = NULL;
		}
		else
		    printf ("Couldn't open appobjects.library\n");

		CloseLibrary (UtilityBase);
		UtilityBase = NULL;
	    }
	    else
		printf ("Couldn't open utility.library\n");

	    CloseLibrary (GfxBase);
	    GfxBase = NULL;
	}
	else
	    printf ("couldn't open graphics.library version 36\n");

	CloseLibrary (IntuitionBase);
	IntuitionBase = NULL;
    }
    else
	printf ("couldn't open intuition.library version 36\n");

    return (FALSE);
}

VOID CloseLibraries (VOID)
{

    if (GadToolsBase)
	CloseLibrary (GadToolsBase);
    if (AppObjectsBase)
	CloseLibrary (AppObjectsBase);
    if (UtilityBase)
	CloseLibrary (UtilityBase);
    if (IntuitionBase)
	CloseLibrary (IntuitionBase);
    if (GfxBase)
	CloseLibrary (GfxBase);
}

VOID OpenFile (STRPTR name, STRPTR * buff, ULONG * bsize)
{
    BPTR fh;

    if (fh = Open (name, MODE_OLDFILE))
    {
	LONG num, doclen, len;

	if ((num = Seek (fh, 0L, OFFSET_END)) >= 0L)
	{
	    STRPTR buffer = NULL;

	    /* Get the length of the document file, and go back to front */
	    doclen = Seek (fh, 0L, OFFSET_BEGINNING);

	    /* Allocate a work buffer */
	    if (buffer = (STRPTR) AllocVec ((doclen + 2L), MEMF_CLEAR))
	    {
		if ((len = Read (fh, buffer, doclen)) > 0L)
		{
		    printf ("successful read (%ld)\n", len);
		    *buff = buffer;
		    *bsize = doclen;
		}
	    }
	}

	Close (fh);
    }
}

BOOL CreateGTMenus (struct Window *win, VOID *vi)
{
    struct Menu *menu;

    if (menu = CreateMenus (GTMenu, GTMN_FrontPen, 0, TAG_DONE))
    {
	if (LayoutMenus (menu, vi, TAG_DONE))
	{
	    SetMenuStrip (win, menu);
	    return (TRUE);
	}
    }

    return (FALSE);
}

VOID HandleApp (struct Window * win)
{
    BOOL going = TRUE;

    while (going)
    {
	struct IntuiMessage *msg;
	struct Gadget *gad;
	struct TagItem *tags;
	STRPTR text;
	struct StringInfo *si;

	/* Wait for something to happen */
	Wait (1 << win->UserPort->mp_SigBit);

	/* Handle the messages */
	while (msg = (struct IntuiMessage *) GetMsg (win->UserPort))
	{
	    switch (msg->Class)
	    {
		case IDCMP_MENUHELP:
		    {
			WORD mnum, inum, snum;

			mnum = MENUNUM (msg->Code);
			inum = ITEMNUM (msg->Code);
			snum = SUBNUM (msg->Code);

			printf ("menu help %d %d %d\n", mnum, inum, snum);
		    }
		    break;

		case IDCMP_CLOSEWINDOW:
		    going = FALSE;
		    break;

		case IDCMPUPDATE:
		    if (tags = (struct TagItem *) msg->IAddress)
		    {
			text = GetTagData (STRINGA_TextVal, NULL, tags);
			printf ("Text 0x%lx [%s]\n", (ULONG) text, text);
		    }
		    break;


		case IDCMP_GADGETUP:
		    gad = (struct Gadget *) msg->IAddress;
		    printf ("Gadget %d Up, code %d\n",gad->GadgetID, msg->Code);
		    si = (struct StringInfo *) gad->SpecialInfo;
		    text = si->Buffer;
		    printf ("gad 0x%lx si 0x%lx 0x%lx [%s]\n", gad, si, text, text);

		    /* Close the window on ESC */
		    if (msg->Code == 27)
		    {
			going = FALSE;
		    }
		    break;

		case IDCMP_GADGETDOWN:
		    gad = (struct Gadget *) msg->IAddress;
		    printf ("Gadget Down %d\n", gad->GadgetID);
		    break;
	    }

	    ReplyMsg ((struct Message *) msg);
	}
    }
}


VOID main (int argc, char **argv)
{
    struct Screen *scr = NULL;
    struct DrawInfo *di = NULL;
    VOID *vi = NULL;
    STRPTR buff = mbuff1;
    ULONG bsize = 1024L;
    STRPTR title = "MultiLine Text Gadget";

    if (OpenLibraries ())
    {
	LONG left = 8L;
	LONG top = 36L;
	LONG width = (-36L);
	LONG height = (-50L);

	if (argc > 1)
	{
	    title = argv[1];

	    OpenFile (title, &buff, &bsize);
	}

	if (scr = LockPubScreen (NULL))
	{
	    left = (LONG) scr->WBorLeft + 8L;
	    top = (LONG) (scr->WBorTop + scr->Font->ta_YSize) + 4L;
	    width = -(left + 18L + 8L);
	    height = -(top + (LONG) (scr->WBorBottom) + 4L);

	    /* Get the DisplayInfo */
	    di = GetScreenDrawInfo (scr);
	    vi = GetVisualInfo (scr, TAG_DONE);

	    /* Create the gadget */
	    gad1 = (struct Gadget *) NewObject (NULL, (ULONG) "mtextggclass",
						GA_DRAWINFO, (LONG) di,
						GA_LEFT, left,
						GA_TOP, top,
						GA_RELWIDTH, width,
						GA_RELHEIGHT, height,
						GA_ID, 1L,
						GA_IMMEDIATE, TRUE,
						GA_RELVERIFY, TRUE,
						STRINGA_MaxChars, bsize,
						STRINGA_TextVal, (LONG) buff,
						ICA_TARGET, ICTARGET_IDCMP,
						TAG_DONE);

	    if (gad1)
	    {
		/* Point at the gadget we just created */
		NewWindow.FirstGadget = gad1;

		/* Set the title */
		NewWindow.Title = title;

		/* Attempt to open the window */
		if (win = OpenWindow (&NewWindow))
		{
		    /* Create the menus */
		    CreateGTMenus (win, vi);

		    /* Handle the fruity app */
		    HandleApp (win);

		    /* Close the window */
		    if (win->MenuStrip)
		    {
			struct Menu *menu = win->MenuStrip;

			ClearMenuStrip (win);

			FreeMenus (menu);
		    }
		    CloseWindow (win);
		    win = NULL;
		}
		else
		{
		    printf ("Can't get window\n");
		}

		/* Get rid of the gadgets */
		DisposeObject (gad1);
	    }
	    else
	    {
		printf ("can't get gadget\n");
	    }

	    FreeVisualInfo (vi);
	    FreeScreenDrawInfo (scr, di);
	    UnlockPubScreen (NULL, scr);
	}
	else
	{
	    printf ("can't lock default public screen\n");
	}

	if (buff != mbuff1)
	{
	    printf ("free buffer\n");
	    FreeVec (buff);
	}

	/* Close the libraries */
	CloseLibraries ();
    }
}
