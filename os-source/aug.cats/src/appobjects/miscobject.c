/* miscobject.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Misc. object functions
 *
 */

#include "appobjectsbase.h"

void kprintf (void *,...);
struct Node *__asm FindNameI (register __a0 struct List *, register __a1 STRPTR);
ULONG __saveds __asm RefreshObjList (register __a1 struct ObjectInfo *, register __a2 struct Window *, register __a3 struct TagItem *);

#define	DA(x)	;
#define	DR(x)	;
#define	DD(x)	;
#define	DS(x)	;

VOID dumpList (struct List * list)
{

    if (list->lh_TailPred != (struct Node *) list)
    {
	struct Node *node, *nxtnode;
	struct ObjectNode *on;

	kprintf ("----------------------------\n");
	node = list->lh_Head;
	while (nxtnode = node->ln_Succ)
	{
	    on = (struct ObjectNode *) node;

	    kprintf ("[%s]\n", node->ln_Name);

	    node = nxtnode;
	}
    }
    else
    {
	kprintf ("List is empty\n");
    }
}

/****** appshell.library/IsGadToolObj **************************************
*
*   NAME
*	IsGadToolObj - Determine type of object.
*
*   SYNOPSIS
*	IsGadToolObj (o);
*		      a1
*
*	struct Object *o;
*
*   FUNCTION
*	This function is used to determine if an ObjectNode is a boopsi
*	object or a GadTools object.
*
*   INPUTS
*	on	- Pointer to an ObjectNode.
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

BOOL __saveds __asm IsGadToolObj (register __a1 struct Object * o)
{
    BOOL retval = TRUE;

    /* Make sure it is one of ours */
    switch (o->o_Type)
    {
	case OBJ_Button:
	case OBJ_Display:
	case OBJ_Select:
	case OBJ_Dropbox:
	case OBJ_GImage:
	case OBJ_Scroller:
	case OBJ_MultiText:
	case OBJ_reserved2:
	case OBJ_DirString:
	case OBJ_DirNumeric:
	case OBJ_boopsi:
	case OBJ_Image:
	case OBJ_reserved3:
	case OBJ_BevelIn:
	case OBJ_BevelOut:
	case OBJ_DblBevelIn:
	case OBJ_DblBevelOut:
	case OBJ_Window:
	case OBJ_VFill:
	case OBJ_HFill:
	case OBJ_Group:
	case OBJ_VGroup:
	case OBJ_HGroup:
	case OBJ_MGroup:

	case OBJ_View:
	case OBJ_MListView:
	case OBJ_Column:
	    retval = FALSE;
	    break;
    }

    return (retval);
}

/* Need a way to add an object list to an existing list without having to
 * have the window open.
 *
 * In that case, set the AOIF_TAILOR flag in the oi_Flags field.
 */

/****** appshell.library/AddObjList **************************************
*
*   NAME
*	AddObjList - Add a list of objects to the window
*
*   SYNOPSIS
*	success = AddObjList (oi, win, attrs);
*	d0		      a1  a2   a3
*
*	ULONG success;
*	struct ObjectInfo *oi;
*	struct Window *win;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function is used to add an object or list of objects to
*	a window.  The window must be open.
*
*   INPUTS
*	oi	- Pointer to the window's ObjectInfo.
*	win	- Valid, open, window.
*	attrs	- Additional arguments.
*
*		  APSH_Objects (required) pointer to the first object
*		  to add to the window.
*
*		  APSH_NumArgs (optional) number of objects to add to
*		  the window.  If this attribute is not specified,
*		  then the NULL terminated list will be added.
*
*   RESULT
*	Returns the number of objects added to the window.
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

ULONG __saveds __asm AddObjList (
	register __a1 struct ObjectInfo * oi,
	register __a2 struct Window * win,
	register __a3 struct TagItem * attrs)
{
    ULONG cnt = 0L;
    ULONG max;

    DS (kprintf ("AddObjList   0x%lx\n", AddObjList));

    /* Make sure we have all our pointers, at least */
    if (oi && win && attrs)
    {
	struct Object *objl;

	/* Make sure we have an object list to work with */
	if (objl = (struct Object *) GetTagData (APSH_Objects, NULL, attrs))
	{
	    struct NewWindow *nw = oi->oi_NewWindow;
	    struct Gadget *anchor = oi->oi_GList;
	    struct WorkData *pwd;

	    /* Get the number of arguments to add */
	    if ((max = GetTagData (APSH_NumArgs, 9999L, attrs)) < 1)
		max = 9999;

	    /* See if we have a current gadget list */
	    if (anchor)
	    {
		struct Gadget *next, *cur = anchor;

		/* Remove the current gadget list */
		RemoveGList (win, anchor, -1);

		/* Find the last gadget */
		while (next = cur->NextGadget)
		{
		    /* Get the next one */
		    cur = next;
		}

		/* Set the last gadget */
		oi->oi_Gadgets = cur;
	    }

	    /* Prepare the ObjectInfo structure */
	    {
		struct List *list = &(oi->oi_ObjList);
		struct Object *ob = NULL;

		/* Make sure we have a list */
		if (list->lh_TailPred != (struct Node *) list)
		{
		    struct Node *node, *nxtnode;

		    /* Start at the very beginning */
		    node = list->lh_Head;

		    /* Step through the list */
		    while (nxtnode = node->ln_Succ)
		    {
			ob = &(((struct ObjectNode *)node)->on_Object);
			ob->o_NextObject = objl;

			/* Get the next node in the list */
			node = nxtnode;
		    }

		    /* Start at the very beginning */
		    node = list->lh_Head;

		    /* Initialize the pointers */
		    ob = &(((struct ObjectNode *)node)->on_Object);

		    /* Step through the list */
		    while ((nxtnode = node->ln_Succ) &&
			   (ob->o_Type != OBJ_Window))
		    {
			ob = &(((struct ObjectNode *)node)->on_Object);

			/* Get the next node in the list */
			node = nxtnode;
		    }
		}

		/* Do we have a window object? */
		if (ob && (ob->o_Type == OBJ_Window))
		{
		    ob->o_NextObject = objl;
		    oi->oi_Objects = ob;
		}
		else
		{
		    /* Start where they told us */
		    oi->oi_Objects = objl;
		}
	    }

	    /* Fill in the rest */
	    oi->oi_Window = win;
	    oi->oi_Flags |= (AOIF_REFRESH | AOIF_REMOVED);

	    /* Allocate a temporary work session */
	    if (oi->oi_Gadgets && (pwd = AllocWorkData (oi, NULL)))
	    {
		struct Object *cob = objl;
		struct ObjectNode *con;
		BOOL retval = TRUE;

		/* Set the real beginning */
		oi->oi_Objects = objl;

		/* Continue while we may */
		while (cob && (cnt < max) && retval)
		{
		    /* Create the object */
		    if (con = CreateObject (pwd, cob, NULL))
		    {
			/* Add the object to the list */
			AddTail (&(oi->oi_ObjList), (struct Node *) con);

			/* Increment the number added */
			cnt++;
		    }
		    else
		    {
			/* Sorry guys, can't continue. */
			retval = FALSE;
		    }

		    /* Get the next object in the array */
		    cob = cob->o_NextObject;

		}		/* End of while objects to add */

		/* Okay, where we successful? */
		if (retval)
		{
		    /* Update the important information */
		    oi->oi_Gadgets = oi->oi_GList;
		    nw->FirstGadget = oi->oi_GList;
		    nw->Screen = oi->oi_Screen;

		    /* Add the gadgets back to the window */
		    AddGList (win, oi->oi_GList, -1, -1, NULL);

		    /* Refresh the gadgets */
		    RefreshGList (win->FirstGadget, win, NULL, -1);

		    /* Refresh the GadTools stuff */
		    GT_RefreshWindow (win, NULL);
		}

		/* Free the temporary work data */
		FreeWorkData (pwd);

	    }			/* End of if AllocWorkData */

	    /* Clear our temporary flags */
	    oi->oi_Flags &= ~(AOIF_REFRESH | AOIF_REMOVED);

	}			/* End of if get APSH_Objects */
    }				/* End of if correct parameters */

    return (cnt);
}

/****** appshell.library/RemoveObjList *******************************
*
*   NAME
*	RemoveObjList - Remove a list of objects from the window.
*
*   SYNOPSIS
*	success = RemoveObjList (oi, win, attrs);
*	d0			 a1  a2   a3
*
*	ULONG success;
*	struct ObjectInfo *oi;
*	struct Window *win;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function is used to remove an object or list of objects
*	to window.  The window must be open.
*
*   INPUTS
*	oi	- Pointer to the window's ObjectInfo.
*	win	- Valid, open, window.
*	attrs	- Additional arguments.  If no attributes are passed
*		  and the AOIF_REMOVE bit of oi_Flags is set, then
*		  the entire object list is removed using
*		  RemoveGList().
*
*		  APSH_NameTag (required) name of the first object to
*		  remove.
*
*		  APSH_NumArgs (optional) number of objects to remove
*		  from the window.  If this attribute is not
*		  specified, then the NULL terminated list will be
*		  removed.
*
*
*   RESULT
*	Always returns 0L.
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

ULONG __saveds __asm RemoveObjList (
	register __a1 struct ObjectInfo * oi,
	register __a2 struct Window * win,
	register __a3 struct TagItem * attrs)
{
    /* Make sure we have all our pointers, at least */
    if (oi && win)
    {
	if (attrs)
	{
	    /*
	     * Get the APSH_NameTag to start with, and APSH_NumArgs for the
	     * number of objects to remove
	     */
	    STRPTR name;

	    /* Get the name of the object to start with */
	    if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, attrs))
	    {
		struct List *list = &(oi->oi_ObjList);
		struct Node *node, *nxtnode;
		WORD num = 0, max;

		/* Get the number of objects to remove */
		max = (WORD) GetTagData (APSH_NumArgs, 9999L, attrs);

		/* See if the object is in the list */
		if (node = FindNameI (list, name))
		{
		    struct IBox *nb = (struct IBox *) & (win->LeftEdge);
		    ULONG flags = oi->oi_Flags;
		    struct ObjectNode *con;

		    /* Clear the window box */
		    oi->oi_WinBox = *nb;
		    oi->oi_WinBox.Width++;

		    /* Has the list been removed? */
		    if (!(oi->oi_Flags & AOIF_REMOVED) && oi->oi_GList)
		    {
			/* Remove the list */
			RemoveGList (win, oi->oi_GList, -1);

			/* Don't remove it again */
			oi->oi_Flags |= AOIF_REMOVED;
		    }

		    /* Step through the list */
		    while ((num < max) && (nxtnode = node->ln_Succ))
		    {
			/* Cast it to an object node */
			con = (struct ObjectNode *) node;

			/* Show that it has been deleted */
			con->on_Flags |= ONF_DELETED;

			/* Get on with it */
			num++;
			node = nxtnode;
		    }

		    /* Make sure the gadgets get refreshed */
		    oi->oi_Flags |= AOIF_REMOVE;

		    /* Refresh the view */
		    RefreshObjList (oi, win, attrs);

		    /* Restore the flags */
		    oi->oi_Flags = flags;
		}
	    }
	}
	else if (oi->oi_Flags & AOIF_REMOVE)
	{
	    /* Copy the window box size */
	    oi->oi_WinBox = *((struct IBox *) & (win->LeftEdge));

	    /* Has the list been removed? */
	    if (!(oi->oi_Flags & AOIF_REMOVED) && oi->oi_GList)
	    {
		/* Remove the list */
		RemoveGList (win, oi->oi_GList, -1);

		/* Don't remove it again */
		oi->oi_Flags |= AOIF_REMOVED;
	    }
	}
    }

    return (0L);
}

/****** appshell.library/RefreshObjList *******************************
*
*   NAME
*	RefreshObjList - Refresh a windows' objects.
*
*   SYNOPSIS
*	success = RefreshObjList (oi, win, attrs);
*	d0			  a1  a2   a3
*
*	ULONG success;
*	struct ObjectInfo *oi;
*	struct Window *win;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function is used to refresh an object or list of objects
*	for a window.  The window must be open.
*
*   INPUTS
*	oi	- Pointer to the window's ObjectInfo.
*	win	- Valid, open, window.
*	attrs	- Additional arguments.  No attributes are defined.
*
*   RESULT
*	Returns TRUE if the gadget addresses have changed.
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

ULONG __saveds __asm RefreshObjList (
	register __a1 struct ObjectInfo * oi,
	register __a2 struct Window * win,
	register __a3 struct TagItem * attrs)
{
    ULONG retval = FALSE;
    struct ObjectData *od;

    DS (kprintf ("RefreshObjList 0x%lx\n", RefreshObjList));
    DD (kprintf ("RefreshObjList 0x%lx, 0x%lx, 0x%lx\n", oi, win, attrs));

    /* Make sure we have all our pointers, at least */
    if (oi && win && (oi->oi_Flags & AOIF_REMOVE))
    {
	struct NewWindow *nw = oi->oi_NewWindow;

	/* Do we have relative GadTools gadgets? */
	if (oi->oi_Flags & AOIF_REMOVED)
	{
	    struct IBox *nb = (struct IBox *) & (win->LeftEdge);
	    struct IBox *ob = &(oi->oi_WinBox);

	    /* Did the window change in size? */
	    if ((nb->Width != ob->Width) ||
		(nb->Height != ob->Height))
	    {
		struct Object *objects = NULL;
		struct Gadget *oGList;
		struct List tlist;

		DD (kprintf ("Removing the object list\n"));

		/* Initialize the temporary list */
		NewList (&tlist);

		/* Remember the old gadget list so that we can free it */
		oGList = oi->oi_GList;

		/* Get rid of old stuff */
		oi->oi_Active = NULL;
		oi->oi_PriorObj = NULL;

		/* Free the gadgets ******************************************** */
		DD (kprintf ("free the gadgets\n"));
		{
		    struct List *list = &(oi->oi_ObjList);

		    /* Make sure we have a list */
		    if (list->lh_TailPred != (struct Node *) list)
		    {
			struct Object *prev = NULL, *cur;
			struct Node *node, *nxtnode;
			struct ObjectNode *con;

			/* Start at the beginning */
			node = list->lh_Head;

			/* Initialize the pointers */
			con = (struct ObjectNode *) node;
			objects = &(con->on_Object);

			/* Step through the list */
			while (nxtnode = node->ln_Succ)
			{
			    /* Remove the node from the old list */
			    Remove (node);

			    /* Cast the current node to an ObjectNode */
			    con = (struct ObjectNode *) node;

			    /* Add the node to the temporary list */
			    AddTail (&tlist, node);

			    if (!(con->on_Flags & ONF_DELETED))
			    {
				/* Get a pointer to the object */
				cur = &(con->on_Object);

				/* Is there an old tag list? */
				if (cur->o_Tags)
				{
				    /* Free it */
				    FreeTagItems (cur->o_Tags);
				    cur->o_Tags = NULL;
				}

				/* Is there a very old tag list? */
				if (con->on_OTags)
				{
				    /* Restore it */
				    cur->o_Tags = con->on_OTags;
				    con->on_OTags = NULL;
				}

				/* Restore the old coordinates also */
				con->on_Object.o_Outer = *(&(con->on_OBox));

				/* Is this a Window object? */
				if (cur->o_Type == OBJ_Window)
				{
				    /* We can't adjust this one */
				    cur->o_Flags |= APSH_OBJF_NOADJUST;

				    /* Copy over the current window box */
				    con->on_Object.o_Outer = *nb;
				    nw->Width = nb->Width;
				    nw->Height = nb->Height;
				}

				/* Set the next object pointer */
				if (prev)
				{
				    prev->o_NextObject = cur;
				}
				cur->o_NextObject = NULL;

				/* Set the previous pointer */
				prev = cur;
			    }

			    /* Get a pointer to the next node */
			    node = nxtnode;
			}
		    }

		    /* Clear out the old list */
		    NewList (list);
		}

		/* Recalculate the gadgets ************************************* */
		DD (kprintf ("recalculate the gadgets\n"));
		if (objects)
		{
		    struct WorkData *pwd;

		    /* Set up the GadTools context */
		    oi->oi_GList = NULL;
		    oi->oi_Gadgets = CreateContext (&(oi->oi_GList));

		    /* Setup working context */
		    oi->oi_Objects = objects;
		    oi->oi_Window = win;
		    oi->oi_Flags |= AOIF_REFRESH;

		    /* Allocate the temporary work area */
		    if (oi->oi_Gadgets && (pwd = AllocWorkData (oi, NULL)))
		    {
			/* Remake the gadgets */
			if (ProcessInfo (pwd, objects, NULL))
			{
			    /* Remember this... */
			    oi->oi_Gadgets = oi->oi_GList;
			    nw->FirstGadget = oi->oi_GList;
			    nw->Screen = oi->oi_Screen;

			    /* Clear the gadgets' graphics (brutal) ************************ */
			    DD (kprintf ("clear the gadgets 0x%lx\n", win));
			    {
				WORD h = MIN (ob->Height, nb->Height) - win->BorderBottom - 1;
				WORD w = MIN (ob->Width, nb->Width) - win->BorderRight - 1;
				struct RastPort crp = *win->RPort;
				WORD tx = win->BorderLeft;
				WORD ty = win->BorderTop;

				SetAPen (&crp, 0);
				SetDrMd (&crp, JAM1);
				DD (kprintf ("RectFill %ld,%ld,%ld,%ld\n",
				   (LONG) tx, (LONG) ty, (LONG) w, (LONG) h));

				RectFill (&crp, tx, ty, w, h);
			    }

			    /* Add the gadgets to the window */
			    AddGList (win, oi->oi_GList, -1, -1, NULL);

			    /* Refresh the gadgets */
			    RefreshGList (win->FirstGadget, win, NULL, -1);

			    /* Refresh the GadTools borders */
			    GT_RefreshWindow (win, NULL);
			}

			/* Free the work area */
			FreeWorkData (pwd);
		    }

		    /* Clear refresh state */
		    oi->oi_Flags &= ~AOIF_REFRESH;
		}

		/*
		 * Get rid of tlist here also.  Remember, I moved them to a
		 * separate list so that we can copy over current data attribs
		 * also. *******************************************************
		 */
		DD (kprintf ("get rid of the other list\n"));
		{
		    struct List *list = &(tlist);

		    /* Do we have an old gadget list */
		    if (oGList)
		    {
			/* Free the old GadTools gadgets */
			FreeGadgets (oGList);
		    }

		    /* Do we have a list */
		    if (list->lh_TailPred != (struct Node *) list)
		    {
			struct Node *node, *nxtnode;

			/* Step through the list */
			node = list->lh_Head;
			while (nxtnode = node->ln_Succ)
			{
			    /* Remove the node from the list */
			    Remove (node);

			    /* Get rid of the object */
			    DisposeObj ((struct ObjectNode *) node);

			    /* Get a pointer to the next node */
			    node = nxtnode;
			}
		    }
		}

		/* Addresses have changed... */
		retval = TRUE;
	    }
	    else
	    {
		/* Add the list back in */
		DD (kprintf ("AddGList 0x%lx 0x%lx\n", win, oi->oi_GList));
		AddGList (win, oi->oi_GList, -1L, -1L, NULL);
	    }

	    /* Don't remove it again */
	    oi->oi_Flags &= ~AOIF_REMOVED;
	}
    }

    DD (kprintf ("RefreshObjList exit\n"));
    return (retval);
}
