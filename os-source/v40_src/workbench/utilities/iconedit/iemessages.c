
/* includes */
#include <exec/types.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* application includes */
#include "iemain.h"
#include "iemessages.h"
#include "ieio.h"
#include "iemenus.h"
#include "iemisc.h"
#include "ieutils.h"
#include "iegads.h"
#include "sp_arrows.h"
#include "sp_tools.h"

/*****************************************************************************/

extern struct Library *GadToolsBase;
extern struct Library *SysBase;
extern struct Library *IntuitionBase;
extern struct Library *DOSBase;
extern struct Library *IconBase;
extern struct Library *GfxBase;

/*****************************************************************************/

VOID SetImage (WindowInfoPtr wi, USHORT code);

static SHORT WhichPort (WindowInfoPtr wi, SHORT x, SHORT y)
{
    struct SketchPad *sp = wi->wi_sketch;
    SHORT tx, ty, bx, by;

    tx = sp->LeftEdge - 2;
    bx = tx + (sp->Gad.Width + 4);
    ty = sp->TopEdge - 1;
    by = ty + (sp->Gad.Height + 2);

    if ((x >= tx && x <= bx) && (y >= ty && y <= by))
    {
	return (1);
    }

    tx = (wi->w_LO + 77) - 4 + 16;
    bx = tx + ICON_WIDTH + 8;
    ty = wi->topborder + IMG1TOP - 2;
    by = ty + ICON_HEIGHT + 4;

    if ((x >= tx && x <= bx) && (y >= ty && y <= by))
    {
	return (2);
    }

    ty = wi->topborder + IMG2TOP - 2;
    by = ty + ICON_HEIGHT + 4;

    if ((x >= tx && x <= bx) && (y >= ty && y <= by))
    {
	return (3);
    }

    return (0);
}

/*****************************************************************************/

static BOOL HandleIMessage (struct IntuiMessage *rmsg)
{
    ULONG class = rmsg->Class;
    UWORD code = rmsg->Code;
    struct Gadget *gad = (struct Gadget *) rmsg->IAddress;
    struct Window *wp = rmsg->IDCMPWindow;
    WindowInfoPtr wi = (WindowInfoPtr) wp->UserData;
    struct DynamicImage *di;
    BOOL terminated = FALSE;
    BOOL iconchanged = FALSE;
    USHORT gid;

    /* Handle the cursor */
    HandleCursor (rmsg);

    switch (class)
    {
	case MENUPICK:
	    if (code != MENUNULL)
	    {
		struct Requester lock;
		struct Requester *r = &(lock);

		SetBusyPointer (wp);

		wp->Flags |= RMBTRAP;

		/* Clear the requester fields */
		InitRequester (r);

		/* Set the required fields */
		r->LeftEdge = (-1);	/* wi->BorderLeft; */
		r->TopEdge = (-1);	/* wi->BorderTop; */
		r->Width = 1;
		r->Height = 1;
		r->Flags = SIMPLEREQ | NOREQBACKFILL;

		/* Bring up the locking requester */
		Request (r, wp);

		terminated = HandleMenuEvent (wi, code);

		/* Unlock the window */
		EndRequest (r, wp);

		wp->Flags &= ~RMBTRAP;

		ClearPointer (wp);
	    }
	    break;

	case RAWKEY:
	    di = wi->wi_sketch->Adi;
	    switch (code)
	    {
		case 76:	/* up arrow */
		    SPSaveToUndo (wi->wi_sketch);
		    MoveImage (1, di);
		    iconchanged = TRUE;
		    break;
		case 77:	/* down arrow */
		    SPSaveToUndo (wi->wi_sketch);
		    MoveImage (3, di);
		    iconchanged = TRUE;
		    break;
		case 79:	/* right arrow */
		    SPSaveToUndo (wi->wi_sketch);
		    MoveImage (4, di);
		    iconchanged = TRUE;
		    break;
		case 78:	/* left arrow */
		    SPSaveToUndo (wi->wi_sketch);
		    MoveImage (2, di);
		    iconchanged = TRUE;
		    break;
	    }
	    break;

	case VANILLAKEY:
	    switch (code)
	    {
		case '/':
		    wi->wi_sketch->APen = 0;
		    wi->wi_sketch->BPen = 0;
		    SPSetAfPt (wi->wi_sketch, NULL, 0);
		    SPUpdateColors (wi);
		    break;
		case 'p':	/* set the FOREGROUND pen */
		    wi->wi_sketch->APen++;
		    SPUpdateColors (wi);
		    break;
		case 'P':	/* set the BACKGROUND pen */
		    wi->wi_sketch->BPen++;
		    SPSetAfPt (wi->wi_sketch, &SPDither, 1);
		    SPUpdateColors (wi);
		    break;
		case 'C':	/* clear the screen */
		    SPClear (wi->wi_sketch);
		    wi->changed |= CH_MAJOR;
		    break;
		case 'N':	/* select the NORMAL image */
		    SetImage (wi, 0);
		    break;
		case 'S':	/* select the SELECT image */
		    SetImage (wi, 1);
		    break;
		case 's':
		    SPSelectToolGad (wi->wi_sketch, 0);
		    break;
		case 'e':
		    SPSelectToolGad (wi->wi_sketch, 1);
		    break;
		case 'l':
		    SPSelectToolGad (wi->wi_sketch, 2);
		    break;
		case 'd':
		    SPSelectToolGad (wi->wi_sketch, 3);
		    break;
		case 'r':
		    SPSelectToolGad (wi->wi_sketch, 4);
		    break;
		case 'f':
		    SPSelectToolGad (wi->wi_sketch, 5);
		    break;
		case 'E':
		    SPSelectToolGad (wi->wi_sketch, 7);
		    break;
		case 'R':
		    SPSelectToolGad (wi->wi_sketch, 10);
		    break;
		case 'u':
		case 'U':
		    SPUndo (wi->wi_sketch);
		    wi->changed |= CH_MAJOR;
		    break;
		default:
		    break;
	    }
	    break;

	case GADGETDOWN:
	case GADGETUP:
	    gid = (gad->GadgetID & GADTOOLMASK);
	    switch (gid)
	    {
		case IE_SELECT:
		    SetImage (wi, code);
		    break;

		case SPSCROLL_ID:
		    SPScroll (wi->wi_sketch, rmsg);
		    wi->changed |= CH_MAJOR;
		    break;

		case SPTOOLS_ID:
		    SPSelectTool (wi->wi_sketch, rmsg);
		    break;

		case SPSKETCHPAD_ID:
		    SPDraw (wi->wi_sketch, rmsg);
		    wi->changed |= CH_MAJOR;
		    break;

		case SPCLEAR_ID:
		    SPClear (wi->wi_sketch);
		    wi->changed |= CH_MAJOR;
		    break;

		case SPUNDO_ID:
		    SPUndo (wi->wi_sketch);
		    wi->changed |= CH_MAJOR;
		    break;

		case SPPALETTE_ID:
		    if (rmsg->Qualifier & SHIFTED)
		    {
			SPSetBPen (wi->wi_sketch, code);
			SPSetAfPt (wi->wi_sketch, &SPDither, 1);
			SPUpdateColors (wi);
		    }
		    else if (rmsg->Qualifier & ALTED)
		    {
			SPSetBPen (wi->wi_sketch, code);
			SPSetAfPt (wi->wi_sketch, &SPVerBar, 1);
			SPUpdateColors (wi);
		    }
		    else
		    {
			SPSetAPen (wi->wi_sketch, code);
			SPSetBPen (wi->wi_sketch, 0);
			SPSetAfPt (wi->wi_sketch, NULL, 0);
			SPUpdateColors (wi);
		    }
		    break;
	    }
	    break;

	case REFRESHWINDOW:
	    GT_BeginRefresh (wp);
	    RefreshCustomImagery (wi);
	    GT_EndRefresh (wp, TRUE);
	    break;

	case CLOSEWINDOW:
	    terminated = TRUE;
	    break;
    }

    if (iconchanged)
    {
	wi->Adi = &wi->images[wi->currentwin];
	SPRefresh (wi->wi_sketch);
	RefreshImages (wi);
    }

    return (terminated);
}

/*****************************************************************************/

static BOOL CompleteName (WindowInfoPtr wi, STRPTR name, BPTR lock, STRPTR buffer, ULONG buffLen)
{
    BOOL retval = FALSE;
    BPTR tlock;
    BPTR old;
    ULONG i;

    if (lock)
    {
	/* Make the supplied directory the current directory */
	old = CurrentDir (lock);

	/* See if the name points to a NULL string */
	if ((name) && (*name==NULL))
	    name = NULL;

	/* If there is a name, then let's get the complete name */
	if (name)
	{
	    NameFromLock (lock, buffer, buffLen);
	    AddPart (buffer, name, buffLen);
	}
	else
	{
	    /* We have a directory, so get the name from the lock */
	    NameFromLock (lock, buffer, buffLen);

	    /* See if we are a volume name */
	    i = strlen (buffer);
	    if (buffer[i-1]==':')
	    {
		AddPart (buffer, "Disk.info", buffLen);
		if (tlock = Lock (buffer, ACCESS_READ))
		{
		    UnLock (tlock);
		    buffer[i+4] = 0;
		}
		else
		    buffer[i] = 0;
	    }
	}

	/* Go back home */
	CurrentDir (old);

	/* Indicate success */
	retval = TRUE;
    }

    return retval;
}

/*****************************************************************************/

void ProcessIMessages (WindowInfoPtr wi)
{
    struct Window *wp;
    char tmp[255];
    BOOL terminated = FALSE;
    struct IntuiMessage *imsg, cmsg;
    struct AppMessage *amsg;
    struct WBArg *argptr;
    SHORT i;
    SHORT msx, msy, hole;
    struct DiskObject *dob;
    ULONG isig, asig, sigs, sigr;

    wp = wi->win;
    /* figure out what signals to watch for */
    isig = (1L << wp->UserPort->mp_SigBit);
    asig = (1L << wi->msgport->mp_SigBit);
    sigs = (isig | asig);

    /* loop until done... */
    while (!terminated)
    {
	sigr = Wait (sigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);

	/* handle IDCMP messages */
	msx = wp->MouseX;
	msy = wp->MouseY;

	if (sigr & SIGBREAKF_CTRL_C)
	    terminated = TRUE;

	if (sigr & SIGBREAKF_CTRL_F)
	{
	    if (wp->Flags & WFLG_ZOOMED)
		ZipWindow (wp);
	    WindowToFront (wp);
	    ActivateWindow (wp);
	}

	if (sigr & isig)
	{
	    while ((!terminated) &&
		   (imsg = (struct IntuiMessage *) GT_GetIMsg (wp->UserPort)))
	    {
		cmsg = *imsg;
		GT_ReplyIMsg (imsg);
		terminated = HandleIMessage (&cmsg);
	    }
	}

	/* Someone just dropped an Icon into our window. */
	if (sigr & asig)
	{
	    /* Make this the active window */
	    ActivateWindow (wp);

	    /* Pull all the App messages */
	    while (amsg = (struct AppMessage *) GetMsg (wi->msgport))
	    {
		/* Find out what hole they dropped it on */
		if ((hole = WhichPort (wi, msx, msy)) > 0)
		{
		    /* We just want the last one buckwheat */
		    argptr = amsg->am_ArgList;
		    for (i = 0; i < (amsg->am_NumArgs - 1); i++)
			argptr++;

		    /* Do we have it? */
		    if (argptr && CompleteName (wi, argptr->wa_Name, argptr->wa_Lock, tmp, sizeof (tmp)))
		    {
			/* Save the current image to the undo buffer */
			SPSaveToUndo (wi->wi_sketch);

			/* Change the current icon */
			if (hole == 1)
			{
			    /* Make this the current icon */

			    /* Open the named icon */
			    OpenNamedIcon (wi, tmp, FALSE);
			}
			/* Change the normal or selected image only */
			else
			{
			    /* Adjust the hole position */
			    hole -= 2;

			    /* Load the icon */
			    if (dob = LoadIcon (tmp, FALSE))
			    {
				LoadIAI (wi, 0, dob, hole);
				FreeDiskObject (dob);
			    }
			}
		    }
		    else
			DisplayBeep (NULL);
		}
		else
		    DisplayBeep (NULL);

		/* Reply to the App message */
		ReplyMsg ((struct Message *) amsg);
	    }
	}

	if (terminated)
	    terminated = (!(CheckForChanges (wi, MSG_IE_QUITANDZAP, MSG_IE_QUITANDZAP_GADS)));

    }
}

/*****************************************************************************/

VOID SetImage (WindowInfoPtr wi, USHORT code)
{
    struct Window *wp;

    wp = wi->win;
    if (wi->img1->Flags & GADGHIMAGE)
    {
	wi->currentwin = code;
	if (wi->currentwin < 0 || wi->currentwin > 1)
	    wi->currentwin = 0;
	GT_SetGadgetAttrs (wi->mxgad, wp, NULL,
			   GTMX_Active, wi->currentwin,
			   TAG_DONE);
	wi->Adi = &wi->images[wi->currentwin];
	SPSetRepeat (wi->wi_sketch, wi->Adi,
		     (wi->w_LO + 77) + 16, wi->wintopx[wi->currentwin]);
	SPRefresh (wi->wi_sketch);
    }
    else if (code == 1)
    {
	GT_SetGadgetAttrs (wi->mxgad, wp, NULL,
			   GTMX_Active, 0,
			   TAG_DONE);
    }
}

/*****************************************************************************/

/* Handle cursor */
VOID HandleCursor (struct IntuiMessage *msg)
{
    struct Window *wp = msg->IDCMPWindow;
    WindowInfoPtr wi = (WindowInfoPtr) wp->UserData;
    struct RastPort crp = *(wp->RPort);
    struct SketchPad *s = wi->wi_sketch;
    struct Gadget *g = &(s->Gad);
    UBYTE buff[10];
    WORD x, y, cx, cy;
    static WORD sprite = (-1);

    if ((!(wp->Flags & WFLG_ZOOMED)) && (wp->Flags & WFLG_WINDOWACTIVE))
    {
	x = (msg->MouseX - g->LeftEdge);
	y = (msg->MouseY - g->TopEdge);
	cx = (x / s->MagX) + 1;
	cy = (y / s->MagY) + 1;

	/* Are we in the sketch domain? */
	if ((cx < 1) || (cy < 1) || (cx > ICON_WIDTH) || (cy > ICON_HEIGHT))
	{
	    /* Do we need to update the sprite imagry? */
	    if (sprite != (-1))
	    {
		ClearPointer (wp);
		sprite = (-1);
	    }

	    /* Clear the coordinate display */
	    CopyMem ("      \0", buff, 7);
/* compiler bug in SAS 5.10A, the above line should normally just be:
            strcpy(buff,"      ");
*/
	}
	else
	{
	    /* Do we need to update the sprite imagry? */
	    if (sprite != wi->w_Sprite)
	    {
		sprite = wi->w_Sprite;

		/* Which sprite do we need? */
		switch (sprite)
		{
		    case 0:
			SetCrossPointer (wp);
			break;

		    case 1:
			SetFillPointer (wp);
			break;
		}
	    }

	    /* Set the coordinate display */
	    sprintf (buff, "%2ld, %2ld", (LONG) cx, (LONG) cy);
	}

	/* Prepare the RastPort */
	SetAPen (&crp, wi->w_DrInfo->dri_Pens[FILLTEXTPEN]);
	SetBPen (&crp, wi->w_DrInfo->dri_Pens[FILLPEN]);
	SetDrMd (&crp, JAM2);
	Move (&crp, 380, 8);

	if ((!(wp->Flags & WFLG_ZOOMED)) && (wp->Flags & WFLG_WINDOWACTIVE))
	{
	    Text (&crp, buff, 6);
	}
    }
}
