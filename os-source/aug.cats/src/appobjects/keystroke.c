/* keystroke.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Translate object array into a window with gadgets
 *
 */

#include "appobjectsbase.h"

void kprintf (void *,...);

/* Create */
#define	DC(x)	;

/* Delete */
#define	DD(x)	;

/* Keystroke */
#define	DK(x)	;

#define	DB(x)	;

struct ObjectNode *
ReleaseKey (struct ObjectInfo * oi, struct Window * win, BOOL abort)
{
    struct ObjectNode *on;

    /* Get the ObjectNode from the cache */
    DK (kprintf ("RK (0x%lx,0x%lx,%ld)\n", oi, win, (LONG) abort));
    if (on = oi->oi_PriorObj)
    {
	struct Gadget *gad = on->on_Gadget;

	DK (kprintf ("obj 0x%lx gad 0x%lx\n", on, gad));

	/* Got a key release */
	on->on_Current = on->on_Funcs[EG_RELEASE];

	/* Figure out what to do to the gadget */
	switch (on->on_Object.o_Type)
	{
	    case OBJ_Button:
	    case OBJ_Select:
	    case OBJ_GImage:
		if (gad->Activation & GACT_RELVERIFY)
		{
		    USHORT pos;

		    DB (kprintf("key release\n"));
		    pos = RemoveGList (win, gad, 1L);
		    gad->Flags &= ~SELECTED;
		    AddGList (win, gad, (LONG) pos, 1L, NULL);
		    RefreshGList (gad, win, NULL, 1);
		}
		break;
	}

	oi->oi_PriorObj = NULL;
	oi->oi_PriorKey = 0;

	DK (kprintf ("Rlse obj 0x%lx cmd %ld\n", on, on->on_Current));

	if (abort)
	{
	    on->on_Current = 0L;
	}
    }

    return (on);
}

/****** appshell.library/LookupKeystroke ***********************************
*
*   NAME
*	LookupKeystroke - Determine if a keystroke is attached to a gadget.
*
*   SYNOPSIS
*	on = LookupKeystroke (oi, msg);
*	d0		      a1  a2
*
*	struct ObjectNode *on;
*	struct ObjectInfo *oi;
*	struct IntuiMessage *msg;
*
*   FUNCTION
*	This function will check a RAWKEY intuition message and determine
*	if the keystroke is attached to a AppObject gadget.  The comparision
*	is case-insensitive, but if any qualifiers other than SHIFT where
*	used, then the key is ignored.
*
*	If the keystroke does belong to a gadget, then following operations
*	occur.
*
*	Downpress
*	Gadget is activated in the appropriate manner.  The on_Current
*	field of the returned ObjectNode is set to on_Funcs[EG_DOWNPRESS].
*
*	Repeat
*	The on_Current field is set to on_Funcs[EG_HOLD].  If there is a
*	'repeat' visual state for the gadget, then it is refreshed.
*
*	Release
*	The on_Current field is set to on_Funcs[EG_RELEASE] and the gadget
*	is deactivated in the appropriate manner.
*
*	Only one key is visually activated at a time.
*
*   INPUTS
*	oi	- ObjectInfo structure returned by NewObjListA()
*	msg	- IntuiMessage for a RAWKEY event.
*
*   EXAMPLE
*
*	\* Inside your event loop *\
*	case RAWKEY:
*
*	    {
*		ULONG tidata = 0L;
*		struct ObjectNode *on;
*
*		if (on = LookupKeystroke (oi, msg))
*		{
*		    \* Get the command index associated with the action *\
*		    tidata = on->on_Current;
*		}
*		else
*		{
*		    \* Look up the keystroke in your own list *\
*		}
*
*		\* Do we have a command index? *\
*		if (tidata)
*		{
*		    \* Perform your command dispatching *\
*		}
*	    }
*
*	    break;
*
*   RETURNS
*	on	- If successful, then will return a pointer to the
*		  ObjectNode that the keystroke belongs to.
*		  Returns NULL if the keystroke doesn't belong to a gadget.
*
*   SEE ALSO
*	AbortKeystroke()
*
*   BUGS
*	Doesn't use an international toupper function.
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

#define	BADKEYS (IEQUALIFIER_CONTROL | IEQUALIFIER_LALT | IEQUALIFIER_RALT \
		 | IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND)

struct ObjectNode *__saveds __asm
LookupKeystroke (register __a1 struct ObjectInfo * oi,
		   register __a2 struct IntuiMessage * msg)
{
    struct Window *win = msg->IDCMPWindow;
    struct ObjectNode *on = NULL;

    DK (kprintf ("LUK(0x%lx, 0x%lx)\n", oi, msg));

    /* Make sure it is a keypress */
    if ((msg->Class == RAWKEY) && !(msg->Qualifier & BADKEYS))
    {
	struct InputEvent ie = {NULL};
	UBYTE keybuff;

	/* Cobble up our input event */
	ie.ie_Class = IECLASS_RAWKEY;
	ie.ie_SubClass = 0;
	ie.ie_Code = msg->Code;
	ie.ie_Qualifier = msg->Qualifier;
	ie.ie_position.ie_addr = *((APTR *) msg->IAddress);

	/* Map the key to a vanilla key */
	if (MapRawKey (&ie, (char *) &keybuff, 1, NULL) == 1)
	{
	    /* Not an international upper-caser!!! */
	    UWORD code = (UWORD) toupper (keybuff);

	    /* See if it matches our cached value */
	    if ((code == oi->oi_PriorKey) && oi->oi_PriorObj)
	    {
		/* Get the ObjectNode from the cache */
		on = oi->oi_PriorObj;
	    }
	    else
	    {
		struct List *list = &(oi->oi_ObjList);

		/* Release the prior key */
		ReleaseKey (oi, win, TRUE);

		/* Make sure it isn't an empty list */
		if (list->lh_TailPred != (struct Node *) list)
		{
		    struct Node *node, *next;

		    /* Start at the beginning */
		    node = list->lh_Head;

		    /*
		     * Step through the list until we find the key or reach the
		     * end of the list
		     */
		    while ((on == NULL) && (next = node->ln_Succ))
		    {
			/* Does it match? */
			if (code == ((struct ObjectNode *) node)->on_Object.o_Key)
			{
			    /* Got it! */
			    on = (struct ObjectNode *) node;
			}

			/* Go to the next node */
			node = next;
		    }
		}
	    }

	    /* See if we found an object that uses this key */
	    if (on)
	    {
		struct Gadget *gad = on->on_Gadget;

		/* Cache the values */
		oi->oi_PriorKey = code;
		oi->oi_PriorObj = on;

		if (msg->Qualifier & IEQUALIFIER_REPEAT)
		{
		    /* Got a key repeat */
		    on->on_Current = on->on_Funcs[EG_HOLD];

		    DK (kprintf ("Hold obj 0x%lx cmd %ld\n", on, on->on_Current));
		}
		else
		{
		    /* Got a key downpress */
		    on->on_Current = on->on_Funcs[EG_DOWNPRESS];

		    /* Figure out what to do to the gadget */
		    switch (on->on_Object.o_Type)
		    {
			case OBJ_Checkbox:
			    {
				USHORT pos;

				pos = RemoveGList (win, gad, 1L);
				gad->Flags ^= SELECTED;
				AddGList (win, gad, (LONG) pos, 1L, NULL);
				RefreshGList (gad, win, NULL, 1);
			    }
			    break;

			case OBJ_Integer:
			case OBJ_String:
			case OBJ_MultiText:
			    if (!(gad->Flags & GFLG_DISABLED))
			    {
				ActivateGadget (gad, win, NULL);
			    }
			    break;

			case OBJ_Button:
			case OBJ_Select:
			case OBJ_GImage:
			    if (gad->Activation & GACT_RELVERIFY)
			    {
				USHORT pos;

				DB (kprintf("downpress\n"));
				pos = RemoveGList (win, gad, 1L);
				gad->Flags |= SELECTED;
				AddGList (win, gad, (LONG) pos, 1L, NULL);
				RefreshGList (gad, win, NULL, 1);
			    }
			    else if ((gad->Activation & GACT_IMMEDIATE) &&
				     on->on_ExtraData)
			    {
				DB (kprintf("immediate downpress\n"));
				SetGadgetAttrs (on->on_ExtraData, win, NULL, GTMX_Active, gad->GadgetID, TAG_DONE);
			    }
			    break;
		    }
		    DK (kprintf ("Down obj 0x%lx cmd %ld\n", on, on->on_Current));
		}
	    }
	    else
	    {
		DK (kprintf ("no find\n"));
	    }
	}
	else if (msg->Code & IECODE_UP_PREFIX)
	{
	    on = ReleaseKey (oi, win, FALSE);
	}
    }
    else
    {
	on = ReleaseKey (oi, win, TRUE);
    }

    return (on);
}

VOID __saveds __asm
AbortKeystroke (register __a1 struct ObjectInfo * oi,
		  register __a2 struct Window * win)
{

    ReleaseKey (oi, win, TRUE);
}
