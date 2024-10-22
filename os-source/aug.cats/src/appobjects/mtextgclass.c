/* mtextgclass.c
 * multiline text gadget
 *
 * Based on textgadget.c written by Talin.
 * BOOPSI by David N. Junod, 27-Sep-90
 *
 */

#include "appobjectsbase.h"
#include "mtextgclass.h"
#include <clib/macros.h>
#include <clib/keymap_protos.h>

/* mtextclass.c */
Class *initMTextGClass (VOID);
ULONG freeMTextGClass (Class * cl);
ULONG __saveds dispatchMTextG (Class * cl, Object * o, VOID * msg);
ULONG setMTextGAttrs (Class * cl, Object * o, VOID * msg);
ULONG getMTextGAttrs (Class * cl, Object * o, VOID * msg);
ULONG renderMTextG (Class * cl, Object * o, VOID *);
ULONG activeMTextG (Class * cl, Object * o, VOID *);
ULONG handleMTextG (Class * cl, Object * o, VOID *);
ULONG inactiveMTextG (Class * cl, Object * o, VOID *);
ULONG handleMTextGmouse (Class * cl, Object * o, VOID *);
ULONG PerformCMD (Class * cl, Object * o, struct gpInput * msg);
ULONG __saveds handleText (CPTR hook, struct TGWork * tw);
ULONG checkMouse (Class * cl, Object * o, VOID *);
VOID setupTGWork (Class * cl, Object * o, VOID *);
VOID stringDelete (struct TGWork * tw, UWORD qualifier);
VOID stringBackspace (struct TGWork * tw, UWORD qualifier);
VOID adjust_scroll (Class * cl, Object * o, VOID *);
VOID SetupIBox (Class * cl, Object * o, VOID *);
WORD xyfind (WORD cx, WORD cy, struct Gadget * g);
WORD TextWrap (UBYTE * s, struct TextFont * f, WORD pixels, WORD * flag);
WORD clamp (WORD min, WORD cur, WORD max);
int paste_text (char *text, int textlength, char *buffer, WORD * numchars, WORD * pos, WORD * anchor);
int ReadHighlightText (void);
int WriteHighlightText (void);
VOID clearTGW (struct TGWork * tw, struct TextInfo * ti);
WORD findWord (struct TGWork * tw, BOOL dir, BOOL delete);

#define IEQUALIFIER_SHIFT	(IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define IEQUALIFIER_ALT		(IEQUALIFIER_LALT | IEQUALIFIER_RALT)

#define	MYCLASSID	"mtextgclass"
#define	SUPERCLASSID	GADGETCLASS

#define	G(o)	((struct Gadget *)(o))

Class *initMTextGClass (VOID)
{
    ULONG __saveds dispatchMTextG ();
    ULONG hookEntry ();
    struct MTextG *mt;
    Class *MakeClass ();
    Class *cl;

    /* Create our work area */
    if (mt = (struct MTextG *) AllocVec (sizeof (struct MTextG), MEMF_CLEAR))
    {
	/* Initialize the main text handling edit hook */
	mt->EditHook.h_Entry = hookEntry;
	mt->EditHook.h_SubEntry = handleText;

	/* Create the class */
	if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL,
			    sizeof (struct TextInfo), 0))
	{
	    /* Fill in the callback hook */
	    cl->cl_Dispatcher.h_Entry = hookEntry;
	    cl->cl_Dispatcher.h_SubEntry = dispatchMTextG;
	    cl->cl_UserData = (ULONG) mt;

	    /* Make the class public & available */
	    AddClass (cl);

	    return (cl);
	}

	FreeVec ((APTR) mt);
    }
    return (NULL);
}

ULONG freeMTextGClass (Class * cl)
{
    ULONG retval = FALSE;

    /* Make sure something was passed */
    if (cl)
    {
	struct MTextG *mt = (struct MTextG *) cl->cl_UserData;

	if (retval = (ULONG) FreeClass (cl))
	{
	    /* Free the work area */
	    if (mt)
	    {
		FreeVec ((APTR) mt);
	    }
	}
    }

    return (retval);
}

ULONG __saveds dispatchMTextG (Class * cl, Object * o, Msg msg)
{
    struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
    struct gpRender *gpr = (struct gpRender *) msg;
    struct TextInfo *ti = INST_DATA (cl, o);
    struct DrawInfo *drinfo;
    struct StringExtend *se;
    ULONG retval = 0L;
    Object *newobj;

    /* See what they want us to do */
    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* A penspec is required */
	    if (drinfo = (struct DrawInfo *) GetTagData (GA_DRAWINFO, NULL, attrs))
	    {
		/* Create a new object (gadget) */
		if (newobj = (Object *) DSM (cl, o, msg))
		{
		    /* Get a pointer to the instance data */
		    ti = INST_DATA (cl, newobj);

		    /* Set the special information field to point to our info */
		    G (newobj)->SpecialInfo = ti;

		    /* Show that we honor a string extension */
		    G (newobj)->Activation |= STRINGEXTEND;

		    /* Get the maximum characters */
		    ti->MaxChars = (WORD) GetTagData (STRINGA_MaxChars, 10L, attrs);

		    /* Get the buffer */
		    ti->Buffer = (UBYTE *) GetTagData (STRINGA_Buffer, NULL, attrs);

		    /* See if a buffer was passed */
		    if (ti->Buffer == NULL)
		    {
			/* No buffer, let's allocate one */
			if (!(ti->Buffer = (UBYTE *) AllocVec ((ULONG) (ti->MaxChars + 2), MEMF_CLEAR)))
			{
			    /* Send message to dispose of this object */
			    DSM (cl, o, (Msg) OM_DISPOSE);

			    /* Get out, quickly! */
			    return (NULL);
			}

			/* Show that we allocated the buffer */
			ti->Flags |= TXT_MYBUFFER;
		    }

		    /* Get the undo buffer */
		    ti->UndoBuffer = (UBYTE *) GetTagData (STRINGA_UndoBuffer, NULL, attrs);

		    /* See if an undo buffer was passed */
		    if (ti->UndoBuffer == NULL)
		    {
			/* No undo buffer, let's allocate one */
			if (!(ti->UndoBuffer = (UBYTE *)AllocVec ((ULONG) (ti->MaxChars + 2), MEMF_CLEAR)))
			{
			    /* Send message to dispose of this object */
			    DSM (cl, o, (Msg) OM_DISPOSE);

			    /* Get out, quickly! */
			    return (NULL);
			}

			/* Show that we allocated the buffer */
			ti->Flags |= TXT_MYUNDO;
		    }

		    /* Establish some default values */
		    se = ti->Extension = &(ti->SE);
		    se->Font = drinfo->dri_Font;
		    /* ((struct GfxBase *) GfxBase)->DefaultFont; */

		    /* Determine pens---should get from penspec */
		    se->ActivePens[0] = se->Pens[0] = drinfo->dri_Pens[TEXTPEN];
		    se->ActivePens[1] = se->Pens[1] = drinfo->dri_Pens[BACKGROUNDPEN];
#if 0
		    se->ActivePens[0] = drinfo->dri_Pens[hifilltextPen];
		    se->ActivePens[1] = drinfo->dri_Pens[hifillPen];
#endif
		    ti->CursorPens[0] = ti->HighPens[0] = drinfo->dri_Pens[FILLTEXTPEN];
		    ti->CursorPens[1] = ti->HighPens[1] = drinfo->dri_Pens[FILLPEN];

		    /* Set our other attributes */
		    setMTextGAttrs (cl, newobj, msg);
		}
		return ((ULONG) newobj);
	    }
	    break;

	case OM_SET:
	case OM_UPDATE:
	    /* Have the super class do it's stuff */
	    retval = DSM (cl, o, msg);

	    /* Adjust our stuff */
	    retval += setMTextGAttrs (cl, o, msg);

	    /* Refresh visuals if I'm a true class */
	    if ((retval) && (OCLASS (o) == cl))
	    {
		struct GadgetInfo *gi = ((struct opSet *) msg)->ops_GInfo;
		struct RastPort *rp;

		/* Get a pointer the rastport */
		if (rp = ObtainGIRPort (gi))
		{
		    struct gpRender dgpr = {NULL};

		    /* Force a redraw */
		    dgpr.MethodID = GM_RENDER;
		    dgpr.gpr_GInfo = gi;
		    dgpr.gpr_RPort = rp;
		    dgpr.gpr_Redraw = GREDRAW_REDRAW;
		    DM (o, &dgpr);

		    /* Release the rastport */
		    ReleaseGIRPort (rp);

		    retval = 0L;
		}
	    }
	    break;

	case OM_GET:
	    retval = (ULONG) getMTextGAttrs (cl, o, msg);
	    break;

	case GM_HITTEST:
	    retval = (ULONG) GMR_GADGETHIT;
	    break;

	case GM_RENDER:
	    if (gpr->gpr_Redraw == GREDRAW_REDRAW)
	    {
		/* Prepare the box size */
		SetupIBox (cl, o, msg);

		/* Render the gadget */
		retval = renderMTextG (cl, o, msg);
	    }
	    break;

	case GM_GOACTIVE:
	    retval = activeMTextG (cl, o, msg);
	    break;

	case GM_HANDLEINPUT:
	    retval = handleMTextG (cl, o, msg);
	    break;

	case GM_GOINACTIVE:
	    retval = inactiveMTextG (cl, o, msg);

	    /* Tell others that there are changes */
	    notifyAttrChanges (o, gpr->gpr_GInfo, 0,
			       GA_ID, G (o)->GadgetID,
			       STRINGA_TextVal, (ULONG) ti->Buffer,
			       TAG_END);
	    break;

	case OM_DISPOSE:
	    /* See if we allocated the buffer */
	    if (ti->Flags & TXT_MYBUFFER)
	    {
		/* Free the buffer */
		FreeVec ((APTR) ti->Buffer);
	    }

	    /* See if we allocated the undo buffer */
	    if (ti->Flags & TXT_MYUNDO)
	    {
		/* Free the buffer */
		FreeVec ((APTR) ti->UndoBuffer);
	    }

	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG activeMTextG (Class * cl, Object * o, struct gpInput * msg)
{
    struct TextInfo *ti = INST_DATA (cl, o);

    if (ti->Flags & TXT_READONLY)
    {
	return (GMR_NOREUSE);
    }

    /* Compute our domain */
    SetupIBox (cl, o, msg);

    /* Do we have an undo buffer? */
    if (ti->UndoBuffer)
    {
	/* Copy the current contents into the undo buffer */
	CopyMem (ti->Buffer, ti->UndoBuffer, ti->MaxChars);
    }

    /* Reset the undo buffer position */
    ti->UndoPos = ti->BufferPos;

    /* Reset the highlight anchor */
    ti->AnchorPos = ti->BufferPos;

    /* Show that we're selected */
    G (o)->Flags |= SELECTED;

    /* check for first SELECT... */
    if (msg->gpi_IEvent)
    {
	checkMouse (cl, o, msg);
    }

    /* Update the display... */
    adjust_scroll (cl, o, msg);

    return (GMR_MEACTIVE);
}

ULONG inactiveMTextG (Class * cl, Object * o, struct gpInput * msg)
{
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;

    /* Clear the selected flag */
    G (o)->Flags &= ~SELECTED;

    /* Not dragging */
    mt->tgWork.Dragging = FALSE;

    /* Render the gadget */
    adjust_scroll (cl, o, msg);

    return (1L);
}

/* handle input */
ULONG handleMTextG (Class * cl, Object * o, struct gpInput * msg)
{
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;
    struct TextInfo *ti = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    struct StringExtend *se = ti->Extension;
    ULONG retval;
    UBYTE keybuf;

    /* Setup the work areas */
    setupTGWork (cl, o, msg);

    /* Determine what to do... */
    switch (ie->ie_Class)
    {
	case IECLASS_RAWKEY:
	    /* Ignore key releases */
	    if (ie->ie_Code & IECODE_UP_PREFIX)
	    {
		return (GMR_MEACTIVE);
	    }

	    if (mt->tgWork.PrevBuffer != mt->tgWork.WorkBuffer)
	    {
		CopyMem (mt->tgWork.PrevBuffer, mt->tgWork.WorkBuffer,
			 mt->tgWork.NumChars + 1);
	    }

	    /* Convert the scan code to a character */
	    mt->tgWork.Code = '\0';
	    if (MapRawKey (ie, (char *) &keybuf, 1, ti->AltKeyMap) == 1)
	    {
		mt->tgWork.Code = keybuf;
	    }

	    /* Handle the keyboard input through the main editing hook */
	    callHook (&mt->EditHook, &(mt->tgWork), SGH_KEY);

	    /* See if there is a custom edit hook for this gadget */
	    if (se && se->EditHook)
	    {
		/* Call the user edit hook */
		callHook (se->EditHook, &(mt->tgWork), SGH_KEY);
	    }

	    /* Perform the action requested */
	    retval = PerformCMD (cl, o, msg);
	    break;

	case IECLASS_TIMER:
	case IECLASS_RAWMOUSE:
	    retval = handleMTextGmouse (cl, o, msg);
	    break;

	default:
	    retval = GMR_MEACTIVE;
	    break;
    }

    return (retval);
}

ULONG __saveds handleText (CPTR hook, struct TGWork * tw)
{
    struct TextInfo *ti = tw->TextInfo;
    struct StringExtend *se = ti->Extension;
    struct InputEvent *ie = tw->IEvent;
    ULONG retval = GMR_MEACTIVE;
    BOOL keyput = FALSE;
    WORD p1, p2;

    /* Handle the highlighting if needed... */
    if (ti->Flags & TXT_HIGHLIGHT)
    {
	if (tw->BufferPos > tw->AnchorPos)
	{
	    p1 = tw->AnchorPos;
	    p2 = tw->BufferPos;
	}
	else
	{
	    p1 = tw->BufferPos;
	    p2 = tw->AnchorPos;
	}
    }

    /* Clear the actions */
    tw->Actions |= SGA_REDISPLAY;

    switch (ie->ie_Code)
    {
	    /* Delete backward */
	case RAW_BKSP:
	    stringBackspace (tw, ie->ie_Qualifier);
	    break;

	    /* Delete forward */
	case RAW_DEL:
	    stringDelete (tw, ie->ie_Qualifier);
	    break;

	    /* Cancel form */
	case RAW_ESC:
	    tw->Code = 27;
	    tw->EditOp = EO_ENTER;
	    tw->Actions |= SGA_END;
	    break;

	    /* Complete form */
	case RAW_ENTER:
	case RAW_RETURN:
	    if (ie->ie_Qualifier & IEQUALIFIER_CONTROL)
	    {
		keyput = TRUE;
		tw->Code = RETURN_KEY;
	    }
	    else
	    {
		tw->Code = '\n';
		tw->EditOp = EO_ENTER;
		tw->Actions |= SGA_END;
	    }
	    break;

	case RAW_TAB:
	    if ((tw->Gadget->Flags & GFLG_TABCYCLE) &&
		!(ie->ie_Qualifier & IEQUALIFIER_CONTROL))
	    {
		tw->Code = '\t';
		tw->EditOp = EO_ENTER;
		tw->Actions |= SGA_END;
		tw->Actions |= ((ie->ie_Qualifier & IEQUALIFIER_SHIFT) ? SGA_PREVACTIVE : SGA_NEXTACTIVE);
	    }
	    else
	    {
		keyput = TRUE;
		tw->Code = '\t';
	    }
	    break;

	    /* Cursor keys */
	case RAW_UP:
	case RAW_DOWN:
	case RAW_LEFT:
	case RAW_RIGHT:
	    {
		WORD displines;
		WORD cpos, xpos;
		WORD y;

		/* Number of lines in the display */
		displines = ti->Domain.Height / se->Font->tf_YSize;

		if (ie->ie_Qualifier & IEQUALIFIER_CONTROL)
		{
		    ti->Flags |= TXT_HIGHLIGHT;
		}
		else
		{
		    /* Are we highlighting? */
		    if (ti->Flags & TXT_HIGHLIGHT)
		    {
			if (ie->ie_Code == RAW_LEFT || ie->ie_Code == RAW_UP)
			{
			    tw->BufferPos = p1;
			}
			else if (ie->ie_Code == RAW_LEFT || ie->ie_Code == RAW_DOWN)
			{
			    tw->BufferPos = p2;
			}
		    }

		    /* Turn off highlighting */
		    ti->Flags &= ~TXT_HIGHLIGHT;
		}

		/* Tell everyone what we're doing */
		tw->EditOp = EO_MOVECURSOR;

		/* Navigate */
		switch (ie->ie_Code)
		{
		    case RAW_LEFT:
			if (ie->ie_Qualifier & IEQUALIFIER_ALT)
			{
			    tw->BufferPos = xyfind (0, tw->CurY, tw->Gadget);
			}
			else if (ie->ie_Qualifier & IEQUALIFIER_SHIFT)
			{
			    tw->BufferPos += findWord (tw, FALSE, FALSE);
			}
			else if (tw->BufferPos > 0)
			{
			    tw->BufferPos--;
			}

			tw->Actions |= SGA_FIX;
			break;

		    case RAW_RIGHT:
			if (ie->ie_Qualifier & IEQUALIFIER_ALT)
			{
			    xpos = tw->Gadget->LeftEdge + tw->Gadget->Width;
			    tw->BufferPos = xyfind (xpos, tw->CurY, tw->Gadget);
			}
			else if (ie->ie_Qualifier & IEQUALIFIER_SHIFT)
			{
			    tw->BufferPos += findWord (tw, TRUE, FALSE);
			}
			else if (tw->BufferPos < tw->NumChars)
			{
			    tw->BufferPos++;
			}

			tw->Actions |= SGA_FIX;
			break;

		    case RAW_UP:
			if (ie->ie_Qualifier & IEQUALIFIER_ALT)
			{
			    ti->ScrollPosition = 0;
			    tw->BufferPos = 0;
			    ti->BufferPos = ti->FirstVisible = 0;
			}
			else if (ie->ie_Qualifier & IEQUALIFIER_SHIFT)
			{
			    y = ti->ScrollPosition;

			    tw->BufferPos = xyfind (tw->SeekX, y, tw->Gadget);
			}
			else
			{
			    if (tw->CurY > 0)
			    {
				cpos = xyfind (tw->SeekX, tw->CurY - 1, tw->Gadget);

				/* if there was a change */
				if (tw->BufferPos != cpos)
				{
				    tw->BufferPos = cpos;
				}
			    }
			    else
			    {
				tw->EditOp = EO_NOOP;
			    }
			}
			break;

		    case RAW_DOWN:
			if (ie->ie_Qualifier & IEQUALIFIER_ALT)
			{
			    WORD bottop = (ti->LinesInBuffer - displines);

			    tw->BufferPos = tw->NumChars;

			    ti->ScrollPosition = (bottop <= 0) ? 0 : (bottop - 1);
			}
			else if (ie->ie_Qualifier & IEQUALIFIER_SHIFT)
			{
			    y = ti->ScrollPosition + displines - 1;

			    if (y < 0)
				y = 0;

			    tw->BufferPos = xyfind (tw->SeekX, y, tw->Gadget);
			}
			else
			{
			    cpos = xyfind (tw->SeekX, tw->CurY + 1, tw->Gadget);

			    /* See if there was a change */
			    if (tw->BufferPos != cpos)
			    {
				tw->BufferPos = cpos;
			    }
			    else
			    {
				tw->EditOp = EO_NOOP;
			    }
			}
		}

		/* if not in highlight mode, update the Anchor */
		if (!(ti->Flags & TXT_HIGHLIGHT))
		{
		    tw->AnchorPos = tw->BufferPos;
		}
	    }
	    break;

	    /* Undo */
	case RAW_U:
	case RAW_Q:
	    if (ie->ie_Qualifier & IEQUALIFIER_RCOMMAND)
	    {
		tw->EditOp = EO_RESET;

		if (ti->UndoBuffer)
		{
		    CopyMem (ti->UndoBuffer, tw->WorkBuffer, ti->MaxChars);
		    tw->BufferPos = ti->UndoPos;
		    tw->NumChars = strlen (tw->WorkBuffer);
		    ti->ScrollPosition = 0;
		    ti->Flags &= ~TXT_HIGHLIGHT;
		}
	    }
	    else
	    {
		/* Need to translate input */
		keyput = TRUE;
	    }
	    break;

	    /* Cut */
	case RAW_X:
	    if (ie->ie_Qualifier & IEQUALIFIER_RCOMMAND)
	    {
		clearTGW (tw, ti);
	    }
	    else
	    {
		/* Need to translate input */
		keyput = TRUE;
	    }
	    break;

	    /* Copy */
	case RAW_C:
	    if (ie->ie_Qualifier & IEQUALIFIER_RCOMMAND)
	    {
	    }
	    else
	    {
		/* Need to translate input */
		keyput = TRUE;
	    }
	    break;

	    /* Paste */
	case RAW_V:
	    if (ie->ie_Qualifier & IEQUALIFIER_RCOMMAND)
	    {
	    }
	    else
	    {
		/* Need to translate input */
		keyput = TRUE;
	    }
	    break;

	    /* Erase */
	case RAW_E:
	    if (ie->ie_Qualifier & IEQUALIFIER_RCOMMAND)
	    {
		clearTGW (tw, ti);
	    }
	    else
	    {
		/* Need to translate input */
		keyput = TRUE;
	    }
	    break;

	default:
	    /* Put control handling here */

	    /* Say that we can handle translated keyboard input */
	    keyput = TRUE;
	    tw->Actions &= ~SGA_REDISPLAY;
	    break;
    }

    /* See if we should handle translated keyboard input */
    if (keyput && tw->Code)
    {
	/* Out of room? */
	if ((tw->NumChars >= ti->MaxChars - 1) &&
	    (!(tw->Modes & SGM_REPLACE) || tw->NumChars == tw->BufferPos))
	{
	    tw->Actions |= SGA_BEEP;
	    tw->Actions &= ~SGA_REDISPLAY;
	}
	else
	{
	    WORD i, bump;

	    /* Mark as redisplay */
	    tw->Actions |= SGA_REDISPLAY;

	    /* Set cursor */
	    bump = 1;

	    /* actually should REPLACE the highlighted text */
	    if ((ti->Flags & TXT_HIGHLIGHT) && (p2 > p1))
	    {
		CopyMem (tw->WorkBuffer + p2, tw->WorkBuffer + p1 + 1, tw->NumChars + 1 - p2);

		/* should actually do this as a paste... */
		tw->WorkBuffer[p1] = tw->Code;
		tw->BufferPos = tw->AnchorPos = p1 + 1;
		tw->NumChars += p1 - p2 + 1;
		tw->Actions |= SGA_FIX;
		tw->EditOp = EO_REPLACECHAR;
	    }
	    else
	    {
		for (i = tw->NumChars; i >= tw->BufferPos; i--)
		{
		    tw->WorkBuffer[i + 1] = tw->WorkBuffer[i];
		}
		tw->NumChars++;

		tw->WorkBuffer[tw->BufferPos] = tw->Code;
		tw->BufferPos += bump;
		tw->Actions |= SGA_FIX;
		tw->EditOp = EO_INSERTCHAR;
	    }

	    ti->Flags &= ~TXT_HIGHLIGHT;
	}
    }

    return (retval);
}

ULONG PerformCMD (Class * cl, Object * o, struct gpInput * msg)
{
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;
    struct TextInfo *ti = INST_DATA (cl, o);
    ULONG retval = GMR_MEACTIVE;

    /* Beep */
    if (mt->tgWork.Actions & SGA_BEEP)
    {
	DisplayBeep (msg->gpi_GInfo->gi_Screen);
    }

    /* Update the gadget */
    if (mt->tgWork.Actions & SGA_USE)
    {
	if (mt->tgWork.NumChars > ti->NumChars)
	{
	    ti->LastVisible += (mt->tgWork.NumChars - ti->NumChars);
	}

	ti->NumChars = mt->tgWork.NumChars;
	ti->BufferPos = mt->tgWork.BufferPos;
	ti->AnchorPos = mt->tgWork.AnchorPos;

	if (mt->tgWork.PrevBuffer)
	{
	    strcpy (mt->tgWork.PrevBuffer, mt->tgWork.WorkBuffer);
	}
    }

    if (mt->tgWork.Actions & SGA_REDISPLAY)
    {
	/* clear the highlight if quitting the gadget */
	if (mt->tgWork.Actions & (SGA_REUSE | SGA_END))
	{
	    ti->Flags &= ~TXT_HIGHLIGHT;
	}

	/* re-render the gadget */
	adjust_scroll (cl, o, msg);

	/* and fix the position of the cursor... */
	if (mt->tgWork.Actions & SGA_FIX)
	    mt->tgWork.SeekX = mt->tgWork.CurX;
    }

    if (mt->tgWork.Actions & SGA_END)
    {
	*msg->gpi_Termination = mt->tgWork.Code;

#if 0
	retval = GMR_VERIFY | ((mt->tgWork.Actions & SGA_REUSE) ? GMR_REUSE : GMR_NOREUSE);
#else
	retval = (GMR_NOREUSE | GMR_VERIFY);
#endif

	if (mt->tgWork.Actions & SGA_NEXTACTIVE)
	{
	    retval |= GMR_NEXTACTIVE;
	}
	if (mt->tgWork.Actions & SGA_PREVACTIVE)
	{
	    retval |= GMR_PREVACTIVE;
	}

    }

    return (retval);
}

ULONG handleMTextGmouse (Class * cl, Object * o, struct gpInput * msg)
{
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;
    struct Gadget *g = G (o);
    struct GadgetInfo *gi = msg->gpi_GInfo;
    struct TextInfo *ti = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    struct StringExtend *se = ti->Extension;
    ULONG retval = GMR_MEACTIVE;
    WORD displines;
    WORD x, y;
    WORD y_in;

    /* Compute number of lines in the display */
    displines = ti->Domain.Height / se->Font->tf_YSize;

    y = msg->gpi_Mouse.Y / se->Font->tf_YSize;
    if (msg->gpi_Mouse.Y < 0)
	y--;
    x = msg->gpi_Mouse.X;

    y_in = (y >= 0 && y < ti->LinesInDisplay);

    y = clamp (-1, y, displines);
    y += ti->ScrollPosition;
    if (y < 0)
	y = 0;

    switch (ie->ie_Code)
    {
	case SELECTDOWN:
	    if (hitGadget (g, gi))
	    {
		mt->tgWork.Dragging = TRUE;

		ti->BufferPos = xyfind (x, y, g);

		/* extended select */
		if (!(ie->ie_Qualifier & IEQUALIFIER_SHIFT))
		{
		    ti->AnchorPos = ti->BufferPos;
		}

		mt->tgWork.SeekX = mt->tgWork.CurX;

		/* redraw the view */
		adjust_scroll (cl, o, msg);
	    }
	    else
	    {
		/* Outside of our region, so get out of here... */
		retval = GMR_REUSE;
	    }

	    break;

	case SELECTUP:
	    mt->tgWork.Dragging = FALSE;
	    break;

	case MENUDOWN:
	case MENUUP:
	    retval = GMR_REUSE;
	    break;

	default:
	    /* What's timer doing in mouse? */
	    if (mt->tgWork.Dragging &&
		(((ie->ie_Class == IECLASS_RAWMOUSE) && y_in) ||
		 ((ie->ie_Class == IECLASS_TIMER) && !y_in)))
	    {
		WORD cpos;

		cpos = xyfind (x, y, g);
		if (ti->BufferPos != cpos)	/* if there was a change		 */
		{
		    ti->BufferPos = cpos;	/* then change it				 */

		    /* redraw the view */
		    adjust_scroll (cl, o, msg);
		}
		mt->tgWork.SeekX = mt->tgWork.CurX;
		ti->Flags |= TXT_HIGHLIGHT;
	    }
	    break;
    }

    return (retval);
}

VOID setupTGWork (Class * cl, Object * o, struct gpInput * msg)
{
    struct MTextG *mt = (struct MTextG *) cl->cl_UserData;
    struct Gadget *g = G (o);
    struct TextInfo *ti = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    struct StringExtend *se = ti->Extension;

    /* Initialize the work area */
    mt->tgWork.Gadget = g;
    mt->tgWork.TextInfo = ti;
    mt->tgWork.WorkBuffer = mt->tgWork.PrevBuffer = ti->Buffer;
    mt->tgWork.Modes = SGM_NOWORKB;
    mt->tgWork.IEvent = ie;
    mt->tgWork.Code = '\0';
    mt->tgWork.BufferPos = ti->BufferPos;
    mt->tgWork.NumChars = ti->NumChars;
    mt->tgWork.Actions = SGA_USE;
    mt->tgWork.LongInt = ti->LongInt;	/* Not used here... :-) */
    mt->tgWork.GadgetInfo = msg->gpi_GInfo;
    mt->tgWork.EditOp = EO_NOOP;
    mt->tgWork.AnchorPos = ti->AnchorPos;

    /* Use extended buffer, if available */
    if (se && se->WorkBuffer)
    {
	mt->tgWork.WorkBuffer = se->WorkBuffer;
	mt->tgWork.Modes &= ~SGM_NOWORKB;
    }
}

ULONG checkMouse (Class * cl, Object * o, struct gpInput * msg)
{

    /* Setup the work areas */
    setupTGWork (cl, o, msg);

    /* Check to see our status */
    return (handleMTextGmouse (cl, o, msg));
}

/* re-draw the text, scrolling to keep the cursor in view if neccessary */
VOID adjust_scroll (Class * cl, Object * o, Msg msg)
{
    struct TextInfo *ti = INST_DATA (cl, o);

    /* If any characters to view */
    if ((ti->LastVisible > 0) && (ti->BufferPos >= ti->LastVisible))
    {
	/* If cursor below window, scroll up */
	ti->ScrollPosition++;
    }

    if (ti->BufferPos < ti->FirstVisible)
    {
	/* If cursor above window, scroll down */
	ti->ScrollPosition--;
    }

    /* Redraw */
    renderMTextG (cl, o, msg);

    /*
     * since the variables "FirstVisible" and "LastVisible" are set by
     * renderMTextG, we need to call it each time we adjust the scroll
     * position.
     */

    /*
     * (SCROLL UP) If any characters to view and cursor is still below window
     */
    while (ti->LastVisible > 0 && ti->BufferPos >= ti->LastVisible)
    {
	/* Adjust */
	ti->ScrollPosition++;

	/* Redraw */
	renderMTextG (cl, o, msg);
    }

    /* (SCROLL DOWN) if cursor STILL above window */
    while (ti->BufferPos < ti->FirstVisible)
    {
	/* Adjust */
	ti->ScrollPosition--;

	/* Redraw */
	renderMTextG (cl, o, msg);
    }
}

ULONG setMTextGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct TextInfo *ti = INST_DATA (cl, o);
    struct StringExtend *se = ti->Extension;
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate = tags;
    struct TagItem *tag;
    ULONG refresh = FALSE;
    ULONG tidata;

    /* Loop through the tags */
    while (tag = NextTagItem (&tstate))
    {
	/* Get a pointer to the tag data */
	tidata = tag->ti_Data;

	/* Figure out what to do */
	switch (tag->ti_Tag)
	{
	    case STRINGA_WorkBuffer:
		se->WorkBuffer = (UBYTE *) tidata;
		break;

	    case STRINGA_BufferPos:
		ti->BufferPos = tidata;
		refresh = TRUE;
		break;

	    case STRINGA_DispPos:
		ti->DispPos = tidata;
		refresh = TRUE;
		break;

	    case STRINGA_AltKeyMap:
		if (ti->AltKeyMap = (struct KeyMap *) tidata)
		    G (o)->Activation |= ALTKEYMAP;
		else
		    G (o)->Activation &= ~ALTKEYMAP;
		break;

	    case STRINGA_Font:
		se->Font = (struct TextFont *) tidata;
		refresh = TRUE;
		break;

	    case STRINGA_Pens:
		se->Pens[0] = tidata & 0xF;
		se->Pens[1] = (tidata >> 8) & 0xF;
		refresh = TRUE;
		break;

	    case STRINGA_ActivePens:
		se->ActivePens[0] = tidata & 0xF;
		se->ActivePens[1] = (tidata >> 8) & 0xF;
		refresh = TRUE;
		break;

	    case STRINGA_EditHook:
		se->EditHook = (struct Hook *) tidata;
		break;

	    case STRINGA_TextVal:
		/* Make sure we have a place to put the value */
		if (ti->Buffer && (tidata != (ULONG) ti->Buffer))
		{
		    ULONG i, j;

		    j = strlen ((STRPTR) tidata);

		    /* Compute length of string to copy */
		    i = (ULONG) MIN ((ti->MaxChars - 1), j);

		    /* Copy the value into the buffer */
		    CopyMem ((APTR) tidata, (APTR) ti->Buffer, i);

		    /* Adjust our world view... */
		    ti->NumChars = strlen (ti->Buffer);
		    ti->BufferPos = 0;
		    ti->Buffer[i] = 0;
		}

		refresh = TRUE;
		break;

	    case CGTA_HighPens:
		ti->HighPens[0] = tidata & 0xF;
		ti->HighPens[1] = (tidata >> 8) & 0xF;
		refresh = TRUE;
		break;

	    case CGTA_DisplayOnly:
		if (tidata)
		{
		    ti->Flags |= TXT_READONLY;
		}
		else
		{
		    ti->Flags &= ~TXT_READONLY;
		}
		break;

	    case CGTA_Top:
		ti->ScrollPosition = (WORD) tidata;
		refresh = 1;
		break;
	}
    }

    return (refresh);
}

ULONG getMTextGAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct TextInfo *ti = INST_DATA (cl, o);
    ULONG retval = 1L;

    switch (msg->opg_AttrID)
    {
	case STRINGA_TextVal:
	    *msg->opg_Storage = (LONG) ti->Buffer;
	    break;

	case CGTA_Total:
	    *msg->opg_Storage = (LONG) ti->LinesInBuffer;
	    break;

	case CGTA_Visible:
	    *msg->opg_Storage = (LONG) ti->LinesInDisplay;
	    break;

	case CGTA_Top:
	    *msg->opg_Storage = (LONG) ti->ScrollPosition;
	    break;

	    /* Let the superclass try */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

#if 0

 /*
  * fields needed: ti (for flags and MaxChars) AnchorPos. BufferPos.
  * WorkBuffer. NumChars.
  */

int paste_text (char *text, int textlength, char *buffer, WORD * numchars, WORD * pos, WORD * anchor)
{
    WORD p1, p2;
    WORD oldlength;

    if (ti->Flags & TXT_HIGHLIGHT)
    {
	if (*pos > *anchor)
	{
	    p1 = *anchor;
	    p2 = *pos;
	}
	else
	{
	    p1 = *pos;
	    p2 = *anchor;
	}
    }
    else
	p1 = p2 = *pos;

    oldlength = p2 - p1;

    /* Q: If too large, should we just truncate??? */

    if (*numchars + textlength - oldlength + 1 > ti->MaxChars)
	return NULL;

    MoveMem (buffer + p2, buffer + p1 + textlength, *numchars - p2);
    CopyMem (text, buffer + p1, length);

    *anchor = *pos = p1;

    if (ti->Flags & TXT_HIGHLIGHT)
	*anchor = p1 + textlength;

    *numchars += textlength - oldlength;
    return TRUE;
}

ReadHighlightText ()
{				/* remove the gadget */
    copy p1 through p2 to buffer
    /* put back the gadget */
    /* activate it if already activate */
}

 WriteHighlightText ()
{				/* remove the gadget */

    paste_text
    /* put the gadget back */
    activate it if it was active...
}

#endif

VOID clearTGW (struct TGWork * tw, struct TextInfo * ti)
{

    if (!(tw->Modes & SGM_FIXEDFIELD))
    {
	tw->EditOp = EO_CLEAR;

	tw->WorkBuffer[0] = '\0';
	tw->BufferPos = 0;
	tw->NumChars = 0;
    }

    tw->BufferPos = 0;
    ti->ScrollPosition = 0;

    /* Turn off highlighting */
    ti->Flags &= ~TXT_HIGHLIGHT;
}

#define ispace(c)  ((c==' ')||(c=='\t')||(c=='\n'))

/* TRUE for forward, FALSE for backward */
WORD findWord (struct TGWork * tw, BOOL dir, BOOL delete)
{
    UBYTE *upper = tw->WorkBuffer + strlen (tw->WorkBuffer) - 1;
    UBYTE *cptr = tw->WorkBuffer + tw->BufferPos;
    UBYTE *lower = tw->WorkBuffer;
    WORD num_move = 0;

    if (dir)
    {
	if (ispace (*cptr))
	{
	    while ((cptr >= lower && cptr <= upper) && ispace (*cptr))
	    {
		num_move++;
		cptr++;
	    }

	    if (delete)
	    {
		num_move++;
	    }
	}
	else
	{
	    while ((cptr >= lower && cptr <= upper) && !ispace (*cptr))
	    {
		num_move++;
		cptr++;
	    }
	    while ((cptr >= lower && cptr <= upper) && ispace (*cptr))
	    {
		num_move++;
		cptr++;
	    }
	}

	if (delete)
	{
	    num_move--;
	}
    }
    else
    {
	cptr--;
	if (delete && ispace (*cptr))
	{
	}
	else
	{
	    while ((cptr >= lower && cptr <= upper) && ispace (*cptr))
	    {
		num_move--;
		cptr--;
	    }
	    while ((cptr >= lower && cptr <= upper) && !ispace (*cptr))
	    {
		num_move--;
		cptr--;
	    }
	}
    }

    return (num_move);
}

VOID stringDelete (struct TGWork * tw, UWORD qualifier)
{
    WORD i;

    /* a no-op for fixed field mode	 */
    if (tw->BufferPos < tw->NumChars &&
	!(tw->Modes & SGM_FIXEDFIELD))
    {
	WORD numkill;

	tw->EditOp = EO_DELFORWARD;
	tw->Actions |= SGA_FIX;

	if (qualifier & IEQUALIFIER_ALT)
	{
	    /* Delete to end of buffer */
	    tw->WorkBuffer[tw->BufferPos] = 0;
	    tw->NumChars = tw->BufferPos;

	    numkill = 0;
	}
	else if (qualifier & IEQUALIFIER_SHIFT)
	{
	    /* Delete to end of word */
	    numkill = ABS (findWord (tw, TRUE, TRUE));
	}
	else
	{
	    /* Delete one character */
	    numkill = 1;
	}

	if (numkill > 0)
	{
	    /* Delete something... */
	    for (i = tw->BufferPos; i <= (tw->NumChars - numkill); i++)
	    {
		tw->WorkBuffer[i] = tw->WorkBuffer[i + numkill];
	    }

	    tw->NumChars--;
	}
    }
    else
    {
	tw->Actions &= ~SGA_REDISPLAY;
    }
}

VOID stringBackspace (struct TGWork * tw, UWORD qualifier)
{

    /* Say that we're going to delete backward */
    tw->EditOp = EO_DELBACKWARD;

    if (tw->BufferPos <= 0)
    {
	tw->Actions &= ~SGA_REDISPLAY;
    }
    else
    {
	struct TextInfo *ti = tw->TextInfo;
	WORD i, numkill;

	/* Turn of highlighting */
	ti->Flags &= ~TXT_HIGHLIGHT;

	/* Delete backward */
	tw->Actions |= SGA_FIX;

	/* Figure out how many characters to delete */
	if (qualifier & IEQUALIFIER_ALT)
	{
	    /* Delete to beginning of buffer */
	    numkill = tw->BufferPos;
	}
	else if (qualifier & IEQUALIFIER_SHIFT)
	{
	    /* Delete to beginning of word */
	    numkill = ABS (findWord (tw, FALSE, TRUE));
	}
	else
	{
	    numkill = 1;
	}

	if (tw->Modes & SGM_FIXEDFIELD)
	{
	    tw->BufferPos -= numkill;
	}
	else
	{
	    for (i = tw->BufferPos; i <= tw->NumChars; i++)
	    {
		tw->WorkBuffer[i - numkill] = tw->WorkBuffer[i];
	    }

	    tw->BufferPos -= numkill;
	    tw->NumChars -= numkill;
	}
    }
}

/* Set up the GRELWIDTH / GRELHEIGHT calculation... */
VOID SetupIBox (Class * cl, Object * o, struct gpRender * gpr)
{
    /* last two are constraints and flags */
    computeDomain (cl, o, gpr, &(((struct TextInfo *) G (o)->SpecialInfo)->Domain), NULL, NULL);
}
