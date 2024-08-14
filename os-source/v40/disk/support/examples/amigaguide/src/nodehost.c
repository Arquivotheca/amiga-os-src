/* nodehost.c
 * Written by David N. Junod
 *
 * Example of a Dynamic Node Host.  This example is useful for determining
 * what nodes an AmigaGuide database is calling when it brings up the
 * "Can't locate node" requester.
 *
 * Compile with:
 *
 * lc -L+lib:debug.lib -cfist -ms -v -y nodehost
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <libraries/amigaguide.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/amigaguide_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/amigaguide_pragmas.h>

/*****************************************************************************/

#define	DB(x)	x

#define	ASM	__asm __saveds
#define	REG(x)	register __ ## x

/*****************************************************************************/

extern void kprintf (void *,...);
ULONG ASM dispatchAmigaGuideHost (REG (a0) struct Hook * h, REG (a2) STRPTR db, REG (a1) Msg msg);

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase, *GfxBase, *UtilityBase, *AmigaGuideBase;

/*****************************************************************************/

main (int argc, char **argv)
{
    struct Hook hook;
    APTR hh;

    if (IntuitionBase = OpenLibrary ("intuition.library", 39))
    {
	GfxBase = OpenLibrary ("graphics.library", 39);
	UtilityBase = OpenLibrary ("utility.library", 39);

	if (AmigaGuideBase = OpenLibrary ("amigaguide.library", 39))
	{
	    /* Initialize the hook */
	    hook.h_Entry = dispatchAmigaGuideHost;

	    /* Add the AmigaGuideHost to the system */
	    if (hh = AddAmigaGuideHostA (&hook, "ExampleHost", NULL))
	    {
		printf ("Added AmigaGuideHost 0x%lx\n", hh);

		/* Wait until we're told to quit */
		Wait (SIGBREAKF_CTRL_C);

		printf ("Remove AmigaGuideHost 0x%lx", hh);

		/* Try removing the host */
		while (RemoveAmigaGuideHostA (hh, NULL) > 0)
		{
		    /* Wait a while */
		    printf (".");
		    Delay (250);
		}
		printf ("\n");
	    }
	    else
	    {
		printf ("Couldn't add AmigaGuideHost\n");
	    }

	    /* close the library */
	    CloseLibrary (AmigaGuideBase);
	}
	else
	{
	    printf ("couldn't open amigaguide.library V39\n");
	}

	CloseLibrary (UtilityBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
    }
    else
    {
	printf ("requires 3.0\n");
    }
}

/*****************************************************************************/

#define	TEMP_NODE "This AmigaGuideHost is an example, that can also\nbe used as a debugging tool.\n"
#define	LINK "Link: "

/*****************************************************************************/

struct TextAttr TOPAZ8  = {"topaz.font", 8, NULL, NULL};
struct TextAttr TOPAZ8B = {"topaz.font", 8, FSF_BOLD, NULL};

/*****************************************************************************/

/* We really need the screen, rectangle, and pen spec. */
VOID Display (struct opNodeIO * onm)
{
    struct TagItem *attrs = onm->onm_Attrs;
    struct IntuiText it1 = {NULL};
    struct IntuiText it2 = {NULL};
    struct Rectangle *rect = NULL;
    struct NewWindow nw = {NULL};
    struct Screen *scr = NULL;
    struct IntuiMessage *msg;
    struct Window *win;
    UWORD *pens = NULL;
    BOOL going = TRUE;
    LONG width = 0L;
    WORD w = 640;
    WORD h = 200;
    WORD dif;

    /* Get attributes, could be NULL */
    if (attrs)
    {
	scr = (struct Screen *) GetTagData (HTNA_Screen, NULL, attrs);
	pens = (UWORD *) GetTagData (HTNA_Pens, NULL, attrs);
	rect = (struct Rectangle *) GetTagData (HTNA_Rectangle, NULL, attrs);
    }

    /* Prepare the IntuiText */
    it1.FrontPen = it2.FrontPen = ((pens) ? pens[SHADOWPEN] : 1);
    it1.DrawMode = it2.DrawMode = JAM1;
    it1.ITextFont = &TOPAZ8;
    it2.ITextFont = &TOPAZ8B;
    it1.IText = LINK;
    it2.IText = onm->onm_Node;

    /* Get the width of the first string */
    width = IntuiTextLength (&it1);
    it2.LeftEdge = (SHORT) width;

    /* Add in the length of the node name */
    width += IntuiTextLength (&it2);

    /* Link the text */
    it1.NextText = &it2;

    /* Prepare the window */
    nw.IDCMPFlags = IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS;
    nw.Flags = WFLG_BORDERLESS | WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH \
      |WFLG_ACTIVATE;
    nw.Width = 8 + width + 8;
    nw.Height = 16;
    nw.Screen = scr;
    nw.Type = (scr) ? CUSTOMSCREEN : WBENCHSCREEN;

    /* Cache the screen size */
    if (scr)
    {
	w = scr->Width;
	h = scr->Height;
    }

    /* See if we have a open help window */
    if (rect)
    {
	/* Center the window within the help window */
	nw.LeftEdge = rect->MinX + ((rect->MaxX - nw.Width) / 2);
	nw.TopEdge = rect->MinY + ((rect->MaxY - nw.Height) / 2);
    }
    /* No help window, so go off the screen */
    else if (scr)
    {

	/*
	 * Center the window horizontally under the mouse and place it
	 * vertically over the mouse position.
	 */
	nw.LeftEdge = scr->MouseX - (nw.Width / 2);
	nw.TopEdge = scr->MouseY - (nw.Height - 2);

	/* Make sure the window can open */
	nw.LeftEdge = (nw.LeftEdge < 0) ? 0 : nw.LeftEdge;
	nw.TopEdge = (nw.TopEdge < 0) ? 0 : nw.TopEdge;
    }

    /* Make sure window is on-screen */
    dif = (nw.LeftEdge + nw.Width) - w;
    nw.LeftEdge = (dif > 0) ? nw.LeftEdge - dif : nw.LeftEdge;
    dif = (nw.TopEdge + nw.Height) - h;
    nw.TopEdge = (dif > 0) ? nw.TopEdge - dif : nw.TopEdge;

    /* Open the temporary window */
    if (win = OpenWindow (&nw))
    {
	/* Clear the window background */
	SetAPen (win->RPort, ((pens) ? pens[SHADOWPEN] : 1));
	RectFill (win->RPort, 0, 0, (win->Width - 1), (win->Height - 1));
	SetAPen (win->RPort, ((pens) ? pens[SHINEPEN] : 2));
	RectFill (win->RPort, 1, 1, (win->Width - 2), (win->Height - 2));

	/* Print the text */
	PrintIText (win->RPort, &it1, 8, 4);

	/* Keep on going til the going gets tough */
	while (going)
	{
	    /* Wait around for something eventful */
	    Wait (1L << win->UserPort->mp_SigBit);

	    /* Pull each message and handle it */
	    while (msg = (struct IntuiMessage *) GetMsg (win->UserPort))
	    {
		switch (msg->Class)
		{
		    case IDCMP_MOUSEBUTTONS:
			/* Stop if we were touched */
			if (msg->Code == SELECTDOWN)
			{
			    going = FALSE;
			}
			break;

		    case IDCMP_VANILLAKEY:
			/* Stop on significant keypress */
			if ((msg->Code == 27) || (msg->Code == 13))
			{
			    going = FALSE;
			}
			break;
		}

		ReplyMsg ((struct Message *) msg);
	    }
	}

	/* Close the window */
	CloseWindow (win);
    }
}

/*****************************************************************************/

/* This is your AmigaGuideHost dispatch hook.  It will never run on your own process. */
ULONG ASM dispatchAmigaGuideHost (REG (a0) struct Hook * h, REG (a2) STRPTR db, REG (a1) Msg msg)
{
    struct opNodeIO *onm = (struct opNodeIO *) msg;
    struct opFindHost *ofh;
    ULONG retval = 0;

    switch (msg->MethodID)
    {
	    /* Does this node belong to you? */
	case HM_FINDNODE:
	    ofh = (struct opFindHost *) msg;
	    DB (kprintf ("Find [%s] in %s\n", ofh->ofh_Node, db));

	    /* See if they want to find our table of contents */
	    if ((Stricmp (ofh->ofh_Node, "main")) == 0)
	    {
		/*
		 * Return TRUE to indicate that it's your node, otherwise
		 * return FALSE.
		 */
		DB (kprintf ("found main\n"));
		retval = TRUE;
	    }
	    else
	    {
		/* Display the name of the node */
		Display (onm);

		/*
		 * Return TRUE to indicate that it's your node, otherwise
		 * return FALSE.
		 */
		retval = FALSE;
		DB (kprintf ("didn't find\n"));
	    }
	    break;

	    /* Open a node. */
	case HM_OPENNODE:
	    DB (kprintf ("Open [%s] in %s\n", onm->onm_Node, db));

	    /* See if they want to display our table of contents */
	    if ((Stricmp (onm->onm_Node, "main")) == 0)
	    {
		/* Provide the contents of the node */
		onm->onm_DocBuffer = TEMP_NODE;
		onm->onm_BuffLen = strlen (TEMP_NODE);
	    }
	    else
	    {
		/* Display the name of the node */
		Display (onm);

		/*
		 * Indicate that we want the node removed from our database,
		 * and that we handled the display of the node
		 */
		onm->onm_Flags |= (HTNF_CLEAN | HTNF_DONE);
	    }

	    /* Indicate that we were able to open the node */
	    retval = TRUE;
	    break;

	    /* Close a node, that has no users. */
	case HM_CLOSENODE:
	    DB (kprintf ("Close [%s] in %s\n", onm->onm_Node, db));

	    /* Indicate that we were able to close the node */
	    retval = TRUE;
	    break;

	    /* Free any extra memory */
	case HM_EXPUNGE:
	    DB (kprintf ("Expunge [%s]\n", db));
	    break;

	default:
	    DB (kprintf ("Unknown method %ld\n", msg->MethodID));
	    break;
    }

    return (retval);
}
