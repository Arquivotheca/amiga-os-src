/* misc.c
 *
 */

#define	STRINGARRAY	TRUE

#include "wdisplay.h"

LONG DeadKeyConvert (struct IntuiMessage * msg, STRPTR kbuffer, LONG kbsize, struct KeyMap * kmap)
{
    struct InputEvent ievent;

    if (msg->Class != RAWKEY)
	return (-2);
    ievent.ie_NextEvent = NULL;
    ievent.ie_Class = IECLASS_RAWKEY;
    ievent.ie_SubClass = 0;
    ievent.ie_Code = msg->Code;
    ievent.ie_Qualifier = msg->Qualifier;
    ievent.ie_position.ie_addr = *((APTR *) msg->IAddress);
    return (RawKeyConvert (&ievent, kbuffer, kbsize, kmap));
}

static struct IntuiText ET =
{2, 1, JAM1, 10, 15, &Topaz80, NULL, NULL};

static struct IntuiText oktext =
{
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE,
    AUTOTOPEDGE, &Topaz80, " OK ", NULL
};

/* NewWindow Structures */
static struct NewWindow NW =
{
    0, 0, 1, 1, -1, -1,
    NULL, WFLG_BORDERLESS, NULL, NULL, BASENAME, NULL, NULL,
    0, 0, 0, 0, WBENCHSCREEN,
};

STRPTR GetWDisplayString (struct AppInfo * ai, LONG id)
{
    STRPTR local = NULL;
    UWORD i;

    i = 0;
    while (!local)
    {
	if (AppStrings[i].as_ID == id)
	    local = AppStrings[i].as_Str;
	i++;
    }

    if (LocaleBase)
    {
	return (GetCatalogStr (ai->ai_Catalog, id, local));
    }

    return (local);
}

VOID CLIMsg (STRPTR msg)
{
    BPTR fh;

    if (fh = Open ("*", MODE_NEWFILE))
    {
	Write (fh, msg, strlen (msg));
	Write (fh, "\n", 1L);
	Close (fh);
    }
}

VOID NotifyUser (struct AppInfo * ai, LONG id)
{
    WORD arwidth, arheight = 30, arwadj = 15;
    struct Window *win;
    STRPTR msg;

    if (msg = GetWDisplayString (ai, id))
    {
	if (ai->ai_Flags & AIF_WORKBENCH)
	{
	    if (IntuitionBase->lib_Version < 36)
	    {
		arheight = 60;
		arwadj = 50;
	    }

	    ET.IText = msg;
	    arwidth = IntuiTextLength (&ET) + arwadj;

	    /* We need a title, dude */
	    if (win = OpenWindow (&NW))
	    {
		AutoRequest (win, &ET, NULL, &oktext, 0, 0, arwidth, arheight);
		CloseWindow (win);
	    }
	}
	else
	{
	    /* Print CLI message */
	    CLIMsg (msg);
	}
    }
}

/* safely close a window that shares a message port with another window */
VOID CloseWindowSafely (struct Window * win)
{
    struct IntuiMessage *msg, *succ;

    Forbid ();

    msg = (struct IntuiMessage *) win->UserPort->mp_MsgList.lh_Head;
    while (succ = (struct IntuiMessage *) msg->ExecMessage.mn_Node.ln_Succ)
    {
	if (msg->IDCMPWindow == win)
	{
	    Remove ((struct Node *) msg);
	    ReplyMsg ((struct Message *) msg);
	}
	msg = succ;
    }
    win->UserPort = NULL;
    ModifyIDCMP (win, NULL);

    Permit ();

    /* clear the menu strip */
    if (win->MenuStrip)
    {
	ClearMenuStrip (win);
    }

    /* close the window */
    CloseWindow (win);
}
