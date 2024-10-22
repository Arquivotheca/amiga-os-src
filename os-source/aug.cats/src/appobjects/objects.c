/* objects.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * Translate object array into a window with gadgets
 *
 */

#include "appobjectsbase.h"

#define	OBJECT	Object

struct TagItem mutex_map[] =
{
    {GA_ID, GTMX_Active},
    {TAG_DONE,}
};

/* External functions */
VOID NewList (struct List *);
struct Node *__asm FindNameI (register __a0 struct List *, register __a1 STRPTR);
VOID InitLayoutWorkData (struct WorkData * pwd);
VOID FreeLayoutWorkData (struct WorkData * pwd);

/* Public functions */
struct ObjectInfo *__saveds __asm NewObjListA (register __a1 struct TagItem *);
VOID __saveds __asm DisposeObjList (register __a1 struct ObjectInfo *);

/* Private functions */
BOOL ProcessInfo (struct WorkData * pwd, struct OBJECT * objl, struct TagItem *);
struct ObjectNode *CreateObject (struct WorkData * pwd, struct OBJECT * obj, struct TagItem *);
VOID AdjustInner (struct IBox * box, struct OBJECT * obj, struct TagItem *);
VOID DisposeObj (struct ObjectNode * con);
struct WorkData *AllocWorkData (struct ObjectInfo *, struct TagItem *);
VOID FreeWorkData (struct WorkData *);
BOOL computeObjectDomain (struct WorkData * pwd, struct Object * cob, struct IBox * box, BOOL doit);

USHORT chip MagnifyData[] =
{
    0x0000, 0x0000, 0x03F8, 0x0000, 0x1C07, 0x0000, 0x30A1, 0x8000,
    0x3201, 0x8000, 0x3001, 0x8000, 0x1C07, 0x0000, 0x03FB, 0xC000,
    0x0000, 0xF000, 0x0000, 0x3C00, 0x0000, 0x0000, 0x0000, 0x0000
};

struct Image Magnify =
{
    0, 0,			/* XY origin relative to container TopLeft */
    24, 12,			/* Image width and height in pixels */
    2,				/* number of bitplanes in Image */
    MagnifyData,		/* pointer to ImageData */
    0x0001, 0x0000,		/* PlanePick and PlaneOnOff */
    NULL			/* next Image structure */
};

/****** appshell.library/NewObjListA ***************************************
*
*   NAME
*	NewObjListA - Create a window environment based on an object array
*
*   SYNOPSIS
*	oi = NewObjListA (attrs);
*	d0		  a1
*
*	struct ObjectInfo *oi;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function will create a window environment from an object
*	array.
*
*	This will modify the following fields.
*
*	    NewWindow
*		Width, Height, MinWidth, MinHeight, MaxWidth, MaxHeight,
*		Title, IDCMPFlags, FirstGadget, Screen
*
*   EXAMPLE
*
*	\* Established elsewhere *\
*	struct Screen *screen;
*	struct NewWindow *main_new;
*	STRPTR *def_text;
*
*	struct Window *win;
*	struct ObjectInfo *oi;
*
*	\* Using a stack based stub *\
*	if (oi = NewObjList (
*	    	    APSH_Screen,	screen,
*	   	    APSH_DefText,	def_text,
*	   	    APSH_NewWindow,	main_new,
*	   	    APSH_Objects,	main_objects,
*		    TAG_DONE))
*	{
*	    \* Try to open the window *\
*	    if (win = OpenWindowTagList (main_new, oi->oi_WindowAttrs))
*	    {
*		\* Refresh the GadTools gadgets *\
*		GT_RefreshWindow (win, NULL);
*
*		\* Do application stuff... *\
*
*		\*--- Close the window ---*\
*		RemoveGList (win, oi->oi_GList, -1);
*		CloseWindowSafely (win);
*	    }
*
*	    \* Free the object information *\
*	    DisposeObjList (oi);
*	}
*
*   INPUTS
*	attrs	- Pointer to TagItem array of attributes.
*
*		The following tags are required.
*
*		APSH_Screen
*		Pointer to the screen that the window will be opened in.
*
*		APSH_TextAttr
*		Pointer to the text attribute to use for the objects.  If
*		NULL, then will use the screen's font attribute.
*
*		APSH_DefText
*		Pointer to the text table to obtain the text label
*		information from.
*
*		APSH_NewWindow
*		Pointer to the NewWindow structure to place the objects in.
*
*		APSH_NewWindowTags
*		Pointer to any tags that the application plans on passing
*		to OpenWindowTags.  Only required if the application
*		actually uses it.
*
*		APSH_Objects
*		Pointer to the object array to create.
*
*		The following tags are optional.
*
*		APSH_WinText
*		Pointer to the window specific text table to obtain the
*		text label information from.
*
*   RESULT
*	oi	- Pointer to a ObjectInfo structure if successful.
*
*		  NULL if failure.
*
*   SEE ALSO
*	DisposeObjList()
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

struct ObjectInfo *__saveds __asm NewObjListA (register __a1 struct TagItem * attrs)
{
    struct ObjectInfo *oi;
    BOOL retval = FALSE;

    /* Attempt to allocate an ObjectInfo structure */
    if (oi = (struct ObjectInfo *)
	AllocVec (sizeof (struct ObjectInfo), MEMF_CLEAR))
    {
	ULONG tidata;

	/* Initialize the object list */
	NewList (&(oi->oi_ObjList));

	/* Get the screen pointer */
	if (oi->oi_Screen = (struct Screen *) GetTagData (APSH_Screen, NULL, attrs))
	{
	    tidata = (ULONG) oi->oi_Screen->Font;
	}

	/* Get the text attribute */
	oi->oi_TextAttr = (struct TextAttr *) GetTagData (APSH_TextAttr, tidata, attrs);

	/* Get the NewWindow structure */
	oi->oi_NewWindow = (struct NewWindow *) GetTagData (APSH_NewWindow, NULL, attrs);

#if 1
	/* Window table is default table */
	if (oi->oi_TextTable = (STRPTR) GetTagData (APSH_WinText, NULL, attrs))
	{
	    /* Get the text table */
	    oi->oi_LocalText = (STRPTR *) GetTagData (APSH_DefText, NULL, attrs);
	}
	else
	{
	    /* Get the text table */
	    oi->oi_TextTable = (STRPTR *) GetTagData (APSH_DefText, NULL, attrs);
	}
#else
	/* Get the text table */
	oi->oi_TextTable = (STRPTR *) GetTagData (APSH_DefText, NULL, attrs);

	/* Get the per window text table */
	oi->oi_LocalText = (STRPTR *) GetTagData (APSH_WinText, NULL, attrs);
#endif

	/* Get the object array */
	oi->oi_Objects = (struct OBJECT *) GetTagData (APSH_Objects, NULL, attrs);

	/* Make sure everything was properly passed */
	if (oi->oi_Screen &&
	    oi->oi_TextAttr &&
	    oi->oi_NewWindow &&
	    oi->oi_TextTable &&
	    oi->oi_Objects)
	{
	    /* Get text/draw render information */
	    if (oi->oi_DrInfo = GetScreenDrawInfo (oi->oi_Screen))
	    {
		/* Get GadTools visual information */
		if (oi->oi_VisualInfo =
		    GetVisualInfo (oi->oi_Screen, TAG_DONE))
		{
		    /* Establish the IDCMP flags that we think we need */
		    oi->oi_NewWindow->IDCMPFlags = IDCMP_flagF;

		    /* Open the desired font */
		    if (oi->oi_TextFont = OpenFont (oi->oi_TextAttr))
		    {
			struct NewWindow *nw = oi->oi_NewWindow;
			struct WorkData *pwd;
			struct TagItem *tag;

			/* Clear the min fields */
			nw->MinWidth = nw->MinHeight = NOT_SET;

			/* See if NewWindow attributes were passed */
			if ((tag = FindTagItem (APSH_NewWindowTags, attrs)) &&
			    (tag->ti_Data))
			{
			    oi->oi_WindowAttrs =
			      CloneTagItems ((struct TagItem *) tag->ti_Data);
			}

			/* Initialize the temporary RastPort */
			InitRastPort (&(oi->oi_RastPort));

			/* Set the font for the RastPort */
			SetFont (&(oi->oi_RastPort), oi->oi_TextFont);

			/* Initialize GadTools context */
			oi->oi_GList = NULL;
			oi->oi_Gadgets = CreateContext (&(oi->oi_GList));

			/* Initialize view & size information */
			{
			    ULONG mode;

			    oi->oi_Size = SYSISIZE_HIRES;

			    /* Get the mode id of the screen */
			    if ((mode = GetVPModeID (&(oi->oi_Screen->ViewPort))) != INVALID_ID)
			    {
				/* Fill in the window rectangle */
				QueryOverscan (mode, &(oi->oi_View), OSCAN_TEXT);

				/* Adjust */
				oi->oi_View.MaxX++;
				oi->oi_View.MaxY++;

				if (oi->oi_View.MaxY < 400)
				{
				    oi->oi_Size = SYSISIZE_MEDRES;
				}

				if (oi->oi_View.MaxX < 640)
				{
				    oi->oi_Size = SYSISIZE_LOWRES;
				}
			    }
			}

			/* Setup our working data */
			if (oi->oi_Gadgets &&
			    (pwd = AllocWorkData (oi, NULL)))
			{
			    /* process information */
			    if (ProcessInfo (pwd, oi->oi_Objects, NULL))
			    {
				struct IBox *r;	/* work rectangle */

				/* get pointers to the work areas */
				r = &(pwd->pwd_Work2);

				if (nw->MinWidth == NOT_SET)
				{
				    nw->MinWidth = r->Width;
				}

				if (nw->MinHeight == NOT_SET)
				{
				    nw->MinHeight =
				      oi->oi_Screen->WBorTop + (oi->oi_Screen->Font->ta_YSize + 1);
				}

				nw->MaxWidth = nw->MaxHeight = (-1);

				nw->FirstGadget = oi->oi_GList;
				nw->Screen = oi->oi_Screen;

				/* point at the newly created gadget list */
				oi->oi_Gadgets = oi->oi_GList;

				/* Indicate success */
				retval = TRUE;
			    }

			    /* Free temporary work data */
			    FreeWorkData (pwd);
			}
		    }		/* End of open desired font */
		}		/* End of visualinfo */
	    }			/* End of drawinfo */
	}			/* End of check needed attributes */

	/* Clean failure path */
	if (retval == FALSE)
	{
	    DisposeObjList (oi);
	    oi = NULL;
	}
    }				/* End of allocate ObjectInfo */

    return (oi);
}

BOOL ProcessInfo (struct WorkData * pwd, struct OBJECT * objl,
		   struct TagItem * tl)
{
    struct ObjectInfo *oi = pwd->pwd_OI;
    BOOL retval = TRUE;

    /* process object list */
    if (objl)
    {
	struct OBJECT *current_obj;
	struct ObjectNode *obj_node;

	/* initialize the current object pointer */
	current_obj = objl;

	/* loop thru the object array and process each object */
	while (current_obj && retval)
	{
	    if (obj_node = CreateObject (pwd, current_obj, NULL))
	    {
		/* adjust the inner size of the window */
		AdjustInner (&(pwd->pwd_Work2), &(obj_node->on_Object), NULL);

		/* add the object node to the list */
		Enqueue (&(oi->oi_ObjList), (struct Node *) obj_node);
	    }
	    else
	    {
		/* unable to create the gadget */
		retval = FALSE;
	    }
	    current_obj = current_obj->o_NextObject;
	}
    }

    return (retval);
}

struct TagItem *SwapNameForPointer
 (
      struct ObjectInfo * oi,	/* Structure describing object array */
      struct ObjectNode * con,	/* Current object node */
      struct TagItem * tagi,	/* Tag list for object */
      ULONG otag,		/* Tag for Name */
      ULONG ntag		/* Tag for pointer */
)
{
    BOOL Found_Selected = FALSE;
    struct TagItem *dn, *gv, *btagi = tagi;
    struct ObjectNode *don = NULL;
    STRPTR name = NULL;
    struct List *list;

    if (dn = FindTagItem (otag, tagi))
    {
	/* Get the name of the list display gadget */
	name = (STRPTR) dn->ti_Data;

	/* Get a pointer to the object list */
	list = &(oi->oi_ObjList);

	/* See if we can locate the object */
	if (name &&
	    (don = (struct ObjectNode *) FindNameI (list, name)))
	{
	    /* Show that we where able to fix up selected */
	    Found_Selected = TRUE;

	    /* See if the show selected already exists */
	    if (gv = FindTagItem (ntag, tagi))
	    {
		gv->ti_Data = (ULONG) don->on_Gadget;
	    }
	    else
	    {
		struct TagItem tt[2];

		/* Insert a new tag */
		tt[0].ti_Tag = ntag;
		tt[0].ti_Data = (ULONG) don->on_Gadget;
		tt[1].ti_Tag = TAG_MORE;
		tt[1].ti_Data = (ULONG) btagi;

		/* Consolodate the tag list */
		con->on_Object.o_Tags = CloneTagItems (tt);
		FreeTagItems (btagi);
		btagi = tagi = con->on_Object.o_Tags;
	    }
	}
    }

    /* Make sure it doesn't point at garbage */
    if (!(Found_Selected) &&
	(dn = FindTagItem (ntag, tagi)))
    {
	dn->ti_Data = NULL;
    }

    return (btagi);
}

#define	ODSIZE	(sizeof(struct ObjectData))

struct ObjectData *CreateObjData (struct ObjectNode *on)
{
    struct ObjectData *od;

    if (od = (struct ObjectData *) AllocVec (ODSIZE, MEMF_CLEAR))
    {
	on->on_SystemData = od;
    }

    return (od);
}

struct ObjectNode *
CreateObject (struct WorkData * pwd, struct OBJECT * obj, struct TagItem * tl)
{
    struct NewGadget *ng = &(pwd->pwd_NewGadget);
    struct TagItem *tagi = NULL, *btagi = NULL, *wtag;
    struct ObjectInfo *oi = pwd->pwd_OI;
    struct ObjectData *od;
    struct IBox *r = NULL;
    struct ObjectNode *con;
    struct NewWindow *nw = oi->oi_NewWindow;
    struct List *list;
    UWORD kind, otype;
    ULONG msize, data;
    ULONG downfunc;

    /* calculate the size of the node */
    msize = sizeof (struct ObjectNode) +
      (strlen (obj->o_Name) + 2L);

    /* allocate the memory block */
    if (con = (struct ObjectNode *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	LONG leftt = GA_LEFT;
	LONG topt = GA_TOP;
	LONG widtht = GA_WIDTH;
	LONG heightt = GA_HEIGHT;

	/* set up the node portion of the object */
	con->on_Node.ln_Type = APSH_MHO_INTOBJ;	/* Intuition object */
	con->on_Node.ln_Pri = APSH_MH_HANDLER_P;
	con->on_Node.ln_Name = MEMORY_FOLLOWING (con);
	strcpy (con->on_Node.ln_Name, obj->o_Name);

	/* Set the current object information */
	pwd->pwd_CON = con;

	/* Copy the current object into the object field */
	con->on_Object = *obj;

	/* Make sure that the names are the same... */
	con->on_Object.o_Name = con->on_Node.ln_Name;

	/* clone the tags, if there are any */
	con->on_Object.o_Tags = NULL;
	if (obj->o_Tags)
	{
	    con->on_Object.o_Tags = CloneTagItems (obj->o_Tags);
	}

	/* Initialize a few variables */
	btagi = tagi = con->on_Object.o_Tags;
	otype = con->on_Object.o_Type;
	kind = otype - OBJ_Generic;

	/* Do we have any tags to backup? */
	if (obj->o_Tags)
	{
	    /* Make a backup of the tags */
	    con->on_OTags = CloneTagItems (obj->o_Tags);
	}

	/* Backup the rectangle */
	con->on_OBox = *(&(obj->o_Outer));

	/* Show that we have converted */
	con->on_Flags |= ONF_CONVERTED;

	/* Get the gadget label, if there is one! */
	ng->ng_GadgetText = NULL;
	if (obj->o_LabelID > 0L)
	{
	    if (con->on_Object.o_Flags & APSH_OBJF_GLOBALTEXT)
	    {
		if (oi->oi_LocalText)
		{
		    ng->ng_GadgetText = (UBYTE *)
		      (oi->oi_LocalText[obj->o_LabelID]);
		}
	    }
	    else
	    {
		ng->ng_GadgetText = (UBYTE *)
		  (oi->oi_TextTable[obj->o_LabelID]);
	    }
	}

	if (ng->ng_GadgetText)
	{
	    UWORD key;

	    /* See if the keystroke is embedded in the label */
	    if ((key = GetLabelKeystroke (ng->ng_GadgetText)))
	    {
		con->on_Object.o_Key = (toupper (key));
	    }
	}

	/* Get the object rectangle */
	r = &(con->on_Object.o_Outer);

	/* See if we should create the object */
	if (computeObjectDomain (pwd, obj, r, TRUE) &&
	    !(oi->oi_Flags & AOIF_TAILOR))
	{
	    /* set up the NewGadget structure */
	    ng->ng_LeftEdge = r->Left;
	    ng->ng_TopEdge = r->Top;
	    ng->ng_Width = r->Width;
	    ng->ng_Height = r->Height;

	    /* Get the GadTools flags */
	    ng->ng_Flags = GetTagData (APSH_GTFlags, NULL, tagi);

	    /* Remember who we are */
	    ng->ng_UserData = con;

	    /* Set up the command callbacks */
	    downfunc = GetTagData (APSH_ObjDown, obj->o_ObjectID, tagi);
	    con->on_Funcs[EG_DOWNPRESS] = GetTagData (APSH_ObjDown, NULL, tagi);
	    con->on_Funcs[EG_HOLD] = GetTagData (APSH_ObjHold, NULL, tagi);
	    con->on_Funcs[EG_RELEASE] = GetTagData (APSH_ObjRelease, obj->o_ObjectID, tagi);
	    con->on_Funcs[EG_DBLCLICK] = GetTagData (APSH_ObjDblClick, NULL, tagi);
	    con->on_Funcs[EG_ABORT] = GetTagData (APSH_ObjAbort, NULL, tagi);
	    con->on_Funcs[EG_ALTHIT] = GetTagData (APSH_ObjAltHit, NULL, tagi);
	    con->on_Funcs[EG_SHIFTHIT] = GetTagData (APSH_ObjShiftHit, NULL, tagi);
	    con->on_Funcs[EG_CREATE] = GetTagData (APSH_ObjCreate, NULL, tagi);
	    con->on_Funcs[EG_DELETE] = GetTagData (APSH_ObjDelete, NULL, tagi);
	    con->on_Funcs[EG_UPDATE] = GetTagData (APSH_ObjUpdate, NULL, tagi);

	    /* Set the gadget ID */
	    ng->ng_GadgetID = (UWORD) con->on_Object.o_ObjectID;
	    if (con->on_Object.o_ObjectID >= APSH_USER_ID)
	    {
		ng->ng_GadgetID = (UWORD) con->on_Object.o_ObjectID - APSH_USER_ID;
	    }

	    /* set up object based on type */
	    switch (otype)
	    {
		case OBJ_Generic:
		    break;

		case OBJ_Button:

		    /* Adjust */
		    if (!(ng->ng_Flags & 0x3DF))
		    {
			ng->ng_Flags |= PLACETEXT_IN;
		    }

		    /* Build tag list */
		    {
			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (GA_TEXT, (LONG) ng->ng_GadgetText,
					  GA_RelVerify, TRUE,
					  CGTA_LabelInfo, LABEL_CENTER,
					  TAG_MORE, tagi,
					  TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_Checkbox:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;
		    break;

		case OBJ_Integer:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;
		    break;

		case OBJ_Listview:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;

		    /* Get the hold to work */
		    nw->IDCMPFlags |= (LISTVIEWIDCMP);

		    if (tagi)
		    {
#if 1
			btagi = tagi =
			  SwapNameForPointer (oi, con, tagi, APSH_ShowSelected, GTLV_ShowSelected);
#else
			struct TagItem *dn;
			BOOL Found_Selected = FALSE;

			if (dn = FindTagItem (APSH_ShowSelected, tagi))
			{
			    struct ObjectNode *don;
			    STRPTR name;

			    /* Get the name of the list display gadget */
			    name = (STRPTR) dn->ti_Data;

			    /* Get a pointer to the object list */
			    list = &(oi->oi_ObjList);

			    /* See if we can locate the object */
			    if (name &&
				(don = (struct ObjectNode *) FindNameI (list, name)))
			    {
				struct TagItem *gv;

				/* Show that we where able to fix up selected */
				Found_Selected = TRUE;

				/* See if the show selected already exists */
				if (gv = FindTagItem (GTLV_ShowSelected, tagi))
				{
				    gv->ti_Data = (ULONG) don->on_Gadget;
				}
				else
				{
				    struct TagItem attrs[2];

				    /* Insert a new tag */
				    attrs[0].ti_Tag = GTLV_ShowSelected;
				    attrs[0].ti_Data = (ULONG) don->on_Gadget;
				    attrs[1].ti_Tag = TAG_MORE;
				    attrs[1].ti_Data = (ULONG) btagi;

				    /* Consolodate the tag list */
				    con->on_Object.o_Tags = CloneTagItems (attrs);
				    FreeTagItems (btagi);
				    btagi = tagi = con->on_Object.o_Tags;
				}
			    }
			}

			/* Make sure it doesn't point at garbage */
			if (!(Found_Selected) &&
			    (dn = FindTagItem (GTLV_ShowSelected, tagi)))
			{
			    dn->ti_Data = NULL;
			}
#endif
		    }
		    break;

		case OBJ_MX:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;
		    con->on_Funcs[EG_RELEASE] = NULL;
		    break;

		case OBJ_Number:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;

		    break;

		case OBJ_Cycle:
		    break;

		case OBJ_Palette:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;

		    tagi = &(pwd->pwd_Inserts[0]);
		    pwd->pwd_Inserts[0].ti_Tag = GTPA_Depth;
		    if (oi->oi_Screen)
		    {
			pwd->pwd_Inserts[0].ti_Data =
			  oi->oi_Screen->RastPort.BitMap->Depth;
		    }
		    else
			pwd->pwd_Inserts[0].ti_Data = 2;

		    if (r->Width <= r->Height)
		    {
			pwd->pwd_Inserts[1].ti_Tag = GTPA_IndicatorHeight;
			pwd->pwd_Inserts[1].ti_Data =
			  SCALE (pwd->pwd_sheight, pwd->pwd_fheight, 10L);
		    }
		    else
		    {
			pwd->pwd_Inserts[1].ti_Tag = GTPA_IndicatorWidth;
			pwd->pwd_Inserts[1].ti_Data =
			  SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, 20L);
		    }

		    pwd->pwd_Inserts[2].ti_Tag = TAG_MORE;
		    pwd->pwd_Inserts[2].ti_Data = (ULONG) btagi;

		    break;

		case OBJ_Scroller:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;
		    con->on_Funcs[EG_UPDATE] = downfunc;

#if 1
		    /* Get the hold to work */
		    nw->IDCMPFlags |= (LISTVIEWIDCMP);

		    if (!FindTagItem (PGA_FREEDOM, tagi))
		    {
			ULONG orient = FREEVERT;

			tagi = &(pwd->pwd_Inserts[0]);

			/* Set the orientation */
			pwd->pwd_Inserts[0].ti_Tag = PGA_FREEDOM;

			/* LORIENT_ */
			if ((r->Width < 0) || (r->Width > r->Height))
			{
			    orient = FREEHORIZ;
			}
			if (r->Height < 0)
			{
			    orient = FREEVERT;
			}

			pwd->pwd_Inserts[0].ti_Data = orient;

			/* Set the arrows */
			pwd->pwd_Inserts[1].ti_Tag = GTSC_Arrows;
			if (orient == FREEVERT)
			{
			    pwd->pwd_Inserts[1].ti_Data = (LONG)
			      SCALE (pwd->pwd_sheight, pwd->pwd_fheight, 8L);
			}
			else
			{
			    pwd->pwd_Inserts[1].ti_Data = (LONG)
			      SCALE (pwd->pwd_swidth, pwd->pwd_fwidth, 16L);
			}

			/* Point to any more information */
			pwd->pwd_Inserts[2].ti_Tag = TAG_MORE;
			pwd->pwd_Inserts[2].ti_Data = (ULONG) btagi;
		    }
#endif
		    break;

		case OBJ_Slider:
		    /* Set the downpress function */
		    con->on_Funcs[EG_DOWNPRESS] = downfunc;

		    /* Get the hold to work */
		    nw->IDCMPFlags |= (LISTVIEWIDCMP);

		    if (!FindTagItem (PGA_FREEDOM, tagi))
		    {
			ULONG flags = NULL;

			tagi = &(pwd->pwd_Inserts[0]);

			pwd->pwd_Inserts[0].ti_Tag = PGA_FREEDOM;
			if (r->Width <= r->Height)
			{
			    flags = PLACETEXT_ABOVE;
			    pwd->pwd_Inserts[0].ti_Data = LORIENT_VERT;
			}
			else
			{
			    flags = PLACETEXT_LEFT;
			    pwd->pwd_Inserts[0].ti_Data = LORIENT_HORIZ;
			}

			if (!(ng->ng_Flags & 0x3DF))
			{
			    ng->ng_Flags |= flags;
			}

			pwd->pwd_Inserts[1].ti_Tag = TAG_MORE;
			pwd->pwd_Inserts[1].ti_Data = (ULONG) btagi;
		    }
		    break;

		case OBJ_String:
		case OBJ_Text:
		    break;

		    /* Other gadgets */

		case OBJ_Display:
		    /* Build tag list */
		    {
			ULONG tag = TAG_IGNORE;

			/* See if there is an image label */
			if (data = GetTagData (APSH_ObjData, NULL, tagi))
			{
			    tag = GA_LABELIMAGE;
			}
			/* See if there is a text label */
			else if (data = (ULONG) ng->ng_GadgetText)
			{
			    tag = GA_TEXT;
			}

			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (CGTA_DisplayOnly, TRUE,
					  CGTA_LabelInfo, LABEL_CENTER,
					  tag, data,
					  CGTA_LabelInfo, LABEL_CENTER,
					  CGTA_FrameInfo, FRAME_CENTER,
					  TAG_MORE, tagi,
					  TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_reserved2:
		case OBJ_Select:
		    /* Build tag list */
		    {
			ULONG tag = TAG_IGNORE;

			/* See if there is an image label */
			if (data = GetTagData (APSH_ObjData, NULL, tagi))
			{
			    tag = GA_LABELIMAGE;
			}
			/* See if there is a text label */
			else if (data = (ULONG) ng->ng_GadgetText)
			{
			    tag = GA_TEXT;
			}

			/* Make an image pointer */
			btagi = tagi =
			    SwapNameForPointer (oi, con, tagi, APSH_GA_LabelImage, GA_LabelImage);

			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (CGTA_LabelInfo, LABEL_CENTER,
					  GA_RelVerify, TRUE,
					  tag, data,
					  CGTA_LabelInfo, LABEL_CENTER,
					  CGTA_FrameInfo, FRAME_CENTER,
					  TAG_MORE, tagi,
					  TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_Dropbox:
		    /* Build tag list */
		    {
			/* See if there is an image label */
			data = GetTagData (APSH_ObjData, NULL, tagi);

			if (con->on_Object.o_Flags & APSH_OBJF_DRAGGABLE)
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					GA_HIGHLIGHT, GADGHNONE,
					GA_LABELIMAGE, data,
					GA_Immediate, TRUE,
					GA_RelVerify, FALSE,
					IA_DOUBLEEMBOSS, TRUE,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					TAG_MORE, tagi,
					TAG_DONE);
			}
			else
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					GA_HIGHLIGHT, GADGHNONE,
					GA_LABELIMAGE, data,
					IA_DOUBLEEMBOSS, TRUE,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);
			}

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_GImage:
		    /* Build tag list */
		    {
			/* See if there is an image label */
			data = GetTagData (APSH_ObjData, NULL, tagi);

			if (con->on_Object.o_Flags & APSH_OBJF_DRAGGABLE)
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					GA_Immediate, TRUE,
					GA_RelVerify, FALSE,
					GA_LABELIMAGE, data,
					GA_HIGHLIGHT, GADGHNONE,
					CGTA_Borderless, TRUE,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					TAG_MORE, tagi,
					TAG_DONE);
			}
			else
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					GA_RelVerify, TRUE,
					GA_LABELIMAGE, data,
					CGTA_Borderless, TRUE,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					TAG_MORE, tagi,
					TAG_DONE);
			}

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_MultiText:

		    /* Create a place to store the data */
		    od = CreateObjData (con);

		    /* Build tag list */
		    {
			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (GA_RelVerify, TRUE,
					STRINGA_Font, oi->oi_TextFont,
					TAG_MORE, tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }
		    break;

		case OBJ_DirString:
		case OBJ_DirNumeric:
		    /* No downpress function */
		    con->on_Funcs[EG_RELEASE] =
		      GetTagData (APSH_ObjExtraRelease, obj->o_ObjectID, tagi);

		    /* Create a place to store the data */
		    od = CreateObjData (con);

		    break;

		    /* Images */

		case OBJ_Image:
		    /* Build tag list */
		    {
			/* See if there is an image label */
			data = GetTagData (APSH_ObjData, NULL, tagi);

			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (GA_RelVerify, TRUE,
					GA_LABELIMAGE, data,
					CGTA_Borderless, TRUE,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		    /* Borders */
		case OBJ_BevelIn:
		    /* Build tag list */
		    {
			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (IA_RECESSED, TRUE,
					IA_EDGESONLY, TRUE,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_BevelOut:
		    /* Build tag list */
		    {
			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (IA_EDGESONLY, TRUE,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }

		    break;

		case OBJ_Group:
		case OBJ_VGroup:
		case OBJ_HGroup:
		case OBJ_DblBevelIn:
		    /* Build tag list */
		    if ((otype == OBJ_DblBevelIn) ||
			(con->on_Object.o_Flags & APSH_OBJF_BORDER))
		    {
			if (!(ng->ng_Flags & 0x3DF))
			{
			    ng->ng_Flags |= PLACETEXT_ABOVEC;
			}

			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (IA_EDGESONLY, TRUE,
					IA_RECESSED, TRUE,
					IA_DOUBLEEMBOSS, TRUE,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }
		    break;

		case OBJ_DblBevelOut:
		    /* Build tag list */
		    {
			/* See if there is an image label */
			data = GetTagData (APSH_ObjData, NULL, tagi);

			if (con->on_Object.o_Flags & APSH_OBJF_DRAGGABLE)
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					GA_Immediate, TRUE,
					GA_RelVerify, FALSE,
					GA_HIGHLIGHT, GADGHNONE,
					IA_DOUBLEEMBOSS, TRUE,
					GA_LABELIMAGE, data,
					CGTA_LabelInfo, LABEL_CENTER,
					CGTA_FrameInfo, FRAME_CENTER,
					TAG_MORE, tagi,
					TAG_DONE);
			}
			else
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags =
			      MakeNewTagList (
					IA_EDGESONLY, TRUE,
					IA_DOUBLEEMBOSS, TRUE,
					GA_LABELIMAGE, data,
					CGTA_Transparent, TRUE,
					TAG_MORE, tagi,
					TAG_DONE);
			}

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;
		    }
		    break;

		case OBJ_boopsi:
		    /* Establish the function */
		    con->on_Funcs[EG_UPDATE] = downfunc;

		    /* Are there tags? */
		    if (tagi)
		    {
			/* Make an image pointer */
			btagi = tagi =
			  SwapNameForPointer (oi, con, tagi, APSH_GA_Image, GA_Image);

			btagi = tagi =
			  SwapNameForPointer (oi, con, tagi, APSH_GA_SelectRender, GA_SelectRender);
		    }
		    break;

		case OBJ_MGroup:
		    /* Establish the function */
		    con->on_Funcs[EG_UPDATE] = downfunc;

		    /* Remember the header object */
		    pwd->pwd_MO = con;

		    /* Create a place to store the data */
		    od = CreateObjData (con);

		    break;

		case OBJ_View:
		case OBJ_MListView:
		    /* Establish the function */
		    con->on_Funcs[EG_UPDATE] = downfunc;

		    /* Remember the header object */
		    pwd->pwd_LV = con;

		    /* Place the label above the list view */
		    if (!(ng->ng_Flags & 0x3DF))
		    {
			ng->ng_Flags |= PLACETEXT_ABOVE;
		    }
		    break;

		default:
		    /* It's one of the special dealies */
		    break;

	    }			/* end of switch (type) */

	    /* See if we're working with relative coordinates */
	    if (ng->ng_LeftEdge < 0)
		leftt = GA_RELRIGHT;
	    if (ng->ng_TopEdge < 0)
		topt = GA_RELBOTTOM;
	    if (ng->ng_Width < 0)
		widtht = GA_RELWIDTH;
	    if (ng->ng_Height < 0)
		heightt = GA_RELHEIGHT;

	    /* See if it is a GadTools object */
	    if (IsGadToolObj (&(con->on_Object)))
	    {
		LONG under;

		/* Let them provide an override */
		under = GetTagData (GT_Underscore, (ULONG)'_', tagi);

		/* Make a new tag list */
		con->on_Object.o_Tags = MakeNewTagList (
					GT_Underscore, under,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

		/* Free the old list */
		FreeTagItems (tagi);

		/* Remember the new list */
		btagi = tagi = con->on_Object.o_Tags;

		/* use GadTools for gadget creation */
		if (con->on_Gadget = pwd->pwd_Gad =
		    CreateGadgetA (kind, pwd->pwd_Gad, ng, tagi))
		{
#if 0
		    /* special adjustment for string gadgets */
		    if (con->on_Object.o_Type == OBJ_Integer ||
			con->on_Object.o_Type == OBJ_String)
		    {
			((struct StringInfo *)
			 pwd->pwd_Gad->SpecialInfo)->Extension->ActivePens[0]
			  = oi->oi_DrInfo->dri_Pens[hifilltextPen];

			((struct StringInfo *)
			 pwd->pwd_Gad->SpecialInfo)->Extension->ActivePens[1]
			  = oi->oi_DrInfo->dri_Pens[hifillPen];

		    }		/* end of if string gadget */
#endif
		}		/* end of if created gadget */
	    }
	    else
	    {
		STRPTR class_name = "listviewgclass";

		switch (otype)
		{
		    case OBJ_View:
			class_name = "viewgclass";
		    case OBJ_MListView:
			/* Make a new tag list */
			con->on_Object.o_Tags = MakeNewTagList (
					leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_Immediate, TRUE,
					GA_RelVerify, TRUE,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					ICA_TARGET, ICTARGET_IDCMP,
					SYSIA_Size, (LONG) oi->oi_Size,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;

			if (pwd->pwd_Header =
			    NewObjectA (NULL, class_name, tagi))
			{
			    con->on_Gadget = pwd->pwd_Gad = (struct Gadget *) pwd->pwd_Header;

			    /* Remember this header */
			    pwd->pwd_LV = con;

			    /* Successful */
			    if (ng->ng_GadgetText)
			    {
				con->on_ObjData = pwd->pwd_Gad = (struct Gadget *)
				      NewObject (NULL, "labelgclass",
					GA_Text, (ULONG) ng->ng_GadgetText,
					GA_Previous, (ULONG) pwd->pwd_Gad,
					APSH_GTFlags, (ULONG) ng->ng_Flags,
					IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					TAG_DONE);
			    }
			}
			break;

		    case OBJ_Column:
			if (pwd->pwd_Header && pwd->pwd_LV &&
			    (pwd->pwd_LV->on_Object.o_Group == con->on_Object.o_Group))
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags =
			      MakeNewTagList (
					CGTA_TextAttr, (LONG) oi->oi_TextAttr,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			    /* Free the old list */
			    FreeTagItems (tagi);

			    /* Remember the new list */
			    btagi = tagi = con->on_Object.o_Tags;

			    /* Create the column image */
			    if (con->on_Gadget = (struct Gadget *)
			      NewObjectA (NULL, "columniclass", tagi))
			    {
				/* Add the column */
				DoMethod (pwd->pwd_Header, LV_ADDCOLUMN, con->on_Gadget, NULL);

				/* Remember the header */
				con->on_ExtraData = pwd->pwd_Header;
			    }
			    break;
			}
			else
			{
			    /* Don't let this happen again. */
			    pwd->pwd_Header = NULL;
			    pwd->pwd_LV = NULL;
			}
			break;

		    case OBJ_MGroup:
			/* Make a new tag list */
			con->on_Object.o_Tags = MakeNewTagList (
					GA_UserData, (ULONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					ICA_TARGET, ICTARGET_IDCMP,
					TAG_MORE, (ULONG) tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;

			if (pwd->pwd_Header =
			    NewObjectA (NULL, "mutexgclass", tagi))
			{
			    con->on_Gadget = (struct Gadget *) pwd->pwd_Header;
			}
			break;

		    case OBJ_boopsi:
			if (wtag = FindTagItem (APSH_NameTag, tagi))
			{
			    class_name = (STRPTR) wtag->ti_Data;

			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					SYSIA_Size, (LONG) oi->oi_Size,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			    /* Free the old list */
			    FreeTagItems (tagi);

			    /* Remember the new list */
			    btagi = tagi = con->on_Object.o_Tags;

			    /* Object isn't a gadget. */
			    if (con->on_Object.o_Flags & APSH_OBJF_NOGADGET)
			    {
				con->on_Gadget = (struct Gadget *)
				    NewObjectA (NULL, class_name, tagi);
			    }
			    else
			    {
			    if (con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
				NewObjectA (NULL, class_name, tagi))
			    {
				/* Successful */
				if (ng->ng_GadgetText)
				{
				    con->on_ObjData = pwd->pwd_Gad = (struct Gadget *)
				      NewObject (NULL, "labelgclass",
					GA_Text, (ULONG) ng->ng_GadgetText,
					GA_Previous, (ULONG) pwd->pwd_Gad,
					APSH_GTFlags, (ULONG) ng->ng_Flags,
					IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					TAG_DONE);
				}
			    }
			    }
			}
			break;

		    case OBJ_Scroller:
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags = MakeNewTagList (
					leftt, (ULONG) ng->ng_LeftEdge,
					topt, (ULONG) ng->ng_TopEdge,
					widtht, (ULONG) ng->ng_Width,
					heightt, (ULONG) ng->ng_Height,
					GA_UserData, (ULONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (ULONG) oi->oi_DrInfo,
					GA_PREVIOUS, (ULONG) pwd->pwd_Gad,
					ICA_TARGET, ICTARGET_IDCMP,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			    /* Free the old list */
			    FreeTagItems (tagi);

			    /* Remember the new list */
			    btagi = tagi = con->on_Object.o_Tags;

			    if (con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
				NewObjectA (NULL, "scrollgclass", tagi))
			    {
				/* Successful */
				if (ng->ng_GadgetText)
				{
				    con->on_ObjData = pwd->pwd_Gad = (struct Gadget *)
				      NewObject (NULL, "labelgclass",
					GA_Text, (ULONG) ng->ng_GadgetText,
					GA_Previous, (ULONG) pwd->pwd_Gad,
					APSH_GTFlags, (ULONG) ng->ng_Flags,
					IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					TAG_DONE);
				}
			    }
			}
			break;

			/* Multiline text class */
		    case OBJ_MultiText:
			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					GA_TabCycle, TRUE,
					CGTA_TextAttr, (LONG) oi->oi_TextAttr,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;

			if (con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
			    NewObjectA (NULL, "mtextggclass", tagi))
			{
			    if (ng->ng_GadgetText)
			    {
				con->on_ObjData = pwd->pwd_Gad = (struct Gadget *)
				  NewObject (NULL, "labelgclass",
					   GA_Text, (ULONG) ng->ng_GadgetText,
					     GA_Previous, (ULONG) pwd->pwd_Gad,
					   APSH_GTFlags, (ULONG) ng->ng_Flags,
				   IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					     TAG_DONE);
			    }
			}
			break;

		    case OBJ_Group:
		    case OBJ_VGroup:
		    case OBJ_HGroup:
		    case OBJ_DblBevelIn:
			if ((otype == OBJ_DblBevelIn) ||
			    (con->on_Object.o_Flags & APSH_OBJF_BORDER))
			{
			    Object *obj = NULL;

			    /*
			     * See if we have a label.  Have to add it in front
			     * of the frame.
			     */
			    if (ng->ng_GadgetText)
			    {
				obj = pwd->pwd_Gad = (struct Gadget *)
				  NewObject (NULL, "labelgclass",
					GA_Text, (ULONG) ng->ng_GadgetText,
					GA_Previous, (ULONG) pwd->pwd_Gad,
					APSH_GTFlags, (ULONG) ng->ng_Flags,
					IA_Mode, (ULONG) JAM2,	/* HIGHLIGHTTEXTPEN */
					IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					TAG_DONE);
			    }

			    /* Make a new tag list */
			    con->on_Object.o_Tags =
			      MakeNewTagList (leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					CGTA_TextAttr, (LONG) oi->oi_TextAttr,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			    /* Free the old list */
			    FreeTagItems (tagi);

			    /* Remember the new list */
			    btagi = tagi = con->on_Object.o_Tags;

			    /* create the gadget itself */
			    if (con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
			      NewObjectA (NULL, "actiongclass", tagi))
			    {
				if (con->on_ObjData = obj)
				{
				    /* Indicate parent gadget */
				    SetAttrs (obj, CGTA_Parent, con->on_Gadget);
				}
			    }
			}
			break;

		    case OBJ_Button:
		    case OBJ_Select:
		    case OBJ_GImage:

			if (pwd->pwd_Header && pwd->pwd_MO &&
			    (pwd->pwd_MO->on_Object.o_Group == con->on_Object.o_Group))
			{
			    /* Make a new tag list */
			    con->on_Object.o_Tags =
			      MakeNewTagList (leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					CGTA_TextAttr, (LONG) oi->oi_TextAttr,
					ICA_TARGET, pwd->pwd_Header,
					ICA_MAP, mutex_map,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			    /* Free the old list */
			    FreeTagItems (tagi);

			    /* Remember the new list */
			    btagi = tagi = con->on_Object.o_Tags;

			    /* create the gadget itself */
			    con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
			      NewObjectA (NULL, "actiongclass", tagi);

			    DoMethod (pwd->pwd_Header, OM_ADDMEMBER, pwd->pwd_Gad);

			    con->on_Funcs[EG_DOWNPRESS] = obj->o_ObjectID;
			    con->on_Funcs[EG_DOWNPRESS] = GetTagData (APSH_ObjDown, NULL, tagi);

			    /* Remember the header */
			    con->on_ExtraData = pwd->pwd_Header;
			    break;
			}
			else
			{
			    /* Don't let this happen again. */
			    pwd->pwd_Header = NULL;
			    pwd->pwd_MO = NULL;
			}

		    case OBJ_Image:
		    case OBJ_Display:
		    case OBJ_reserved2:
		    case OBJ_Dropbox:
		    case OBJ_reserved3:
		    case OBJ_BevelIn:
		    case OBJ_BevelOut:
		    case OBJ_DblBevelOut:

			/* Make a new tag list */
			con->on_Object.o_Tags =
			  MakeNewTagList (leftt, (LONG) ng->ng_LeftEdge,
					topt, (LONG) ng->ng_TopEdge,
					widtht, (LONG) ng->ng_Width,
					heightt, (LONG) ng->ng_Height,
					GA_UserData, (LONG) con,
					GA_ID, (ULONG) ng->ng_GadgetID,
					GA_DRAWINFO, (LONG) oi->oi_DrInfo,
					GA_PREVIOUS, (LONG) pwd->pwd_Gad,
					CGTA_TextAttr, (LONG) oi->oi_TextAttr,
					TAG_MORE, (LONG) tagi,
					TAG_DONE);

			/* Free the old list */
			FreeTagItems (tagi);

			/* Remember the new list */
			btagi = tagi = con->on_Object.o_Tags;

			/* create the gadget itself */
			con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
			  NewObjectA (NULL, "actiongclass", tagi);
			break;

			/* Screwy-louey class */
		    case OBJ_DirString:
		    case OBJ_DirNumeric:
			/* Point the data at the image */
			con->on_Image = &Magnify;

			/* create the gadget itself */
			if (con->on_Gadget = pwd->pwd_Gad = (struct Gadget *)
			    NewObject (NULL, "actiongclass",
				       GA_LEFT, (LONG) ng->ng_LeftEdge,
				       GA_TOP, (LONG) ng->ng_TopEdge,
				       GA_WIDTH, 28L,
				       GA_HEIGHT, (LONG) ng->ng_Height,
				       GA_LABELIMAGE, con->on_Image,
				       GA_HIGHLIGHT, GADGHCOMP,
				       GA_Immediate, FALSE,
				       GA_RelVerify, TRUE,
				       GA_UserData, con,
				       GA_ID, (ULONG) ng->ng_GadgetID,
				       GA_DRAWINFO, oi->oi_DrInfo,
				       GA_PREVIOUS, pwd->pwd_Gad,
				       CGTA_LabelInfo, LABEL_CENTER,
				       TAG_END))
			{
			    struct ObjectNode *ccon;
			    struct OBJECT cob =
			    {NULL};

			    if (ng->ng_GadgetText)
			    {
				con->on_ObjData = pwd->pwd_Gad = (struct Gadget *)
				  NewObject (NULL, "labelgclass",
					   GA_Text, (ULONG) ng->ng_GadgetText,
					     GA_Previous, (ULONG) pwd->pwd_Gad,
					   APSH_GTFlags, (ULONG) ng->ng_Flags,
				   IA_FGPen, oi->oi_DrInfo->dri_Pens[TEXTPEN],
					     TAG_DONE);
			    }

			    /* Copy the original */
			    cob = *obj;

			    /* Set the correct type */
			    cob.o_Type = OBJ_String;
			    if (otype == OBJ_DirNumeric)
				cob.o_Type = OBJ_Integer;
			    cob.o_LabelID = NULL;

			    /* Tell the system not to scale the coordinates */
			    cob.o_Flags |= APSH_OBJF_NOADJUST;

			    /* correct the rectangle */
			    cob.o_Outer.Left = ng->ng_LeftEdge + con->on_Gadget->Width;
			    cob.o_Outer.Top = ng->ng_TopEdge;
			    cob.o_Outer.Width = ng->ng_Width - con->on_Gadget->Width;
			    cob.o_Outer.Height = ng->ng_Height;

			    /* Create the string/numeric portion */
			    if (ccon = CreateObject (pwd, &cob, NULL))
			    {
				/* Add it to the list */
				Enqueue (&(oi->oi_ObjList), (struct Node *) ccon);
			    }
			}
			break;
		}
	    }

	    /* See if this is the activate gadget */
	    if (con->on_Object.o_Flags & APSH_OBJF_ACTIVATE)
	    {
		oi->oi_Active = con->on_Gadget;
	    }
#if 0
	    /* If this gadget is draggable... */
	    if (con->on_Object.o_Flags & APSH_OBJF_DRAGGABLE)
	    {
		con->on_Gadget->Activation |= GADGIMMEDIATE;
		con->on_Gadget->Activation &= ~RELVERIFY;
	    }
#endif
	}
    }				/* end of if alloc (con) */

    /* return the object node */
    return (con);
}

VOID AdjustInner (struct IBox * box, struct OBJECT * obj,
		   struct TagItem * tl)
{
    struct IBox *obox = &(obj->o_Outer);

    if (obj->o_Type != OBJ_Window)
    {
	box->Width = MAX (box->Width, ABS (obox->Left + obox->Width));
	box->Height = MAX (box->Height, ABS (obox->Top + obox->Height));
    }
}

/* Remove the resources allocated for an object node */
VOID DisposeObj (struct ObjectNode * con)
{

    if (con)
    {
	struct OBJECT *o = &(con->on_Object);

	/* See if we have a frame to dispose of */
	if (con->on_Image)
	{
	    if (((struct Image *) con->on_Image)->Depth < 0)
	    {
		DisposeObject (con->on_Image);
	    }
	}

	/* See if we have a frame to dispose of */
	if (con->on_ObjData)
	{
	    DisposeObject (con->on_ObjData);
	}

	if (con->on_Gadget && !(IsGadToolObj (&(con->on_Object))))
	{
	    if (con->on_Object.o_Type != OBJ_Column)
	    {
		DisposeObject (con->on_Gadget);
	    }
	}

	/* Free the cloned tag items */
	if (o->o_Tags)
	{
	    FreeTagItems (o->o_Tags);
	}

	/* Free the cloned backup tag items */
	if (con->on_OTags)
	{
	    FreeTagItems (con->on_OTags);
	}

	if (con->on_SystemData)
	{
	    FreeVec (con->on_SystemData);
	}

	/* free the memory allocated for the node */
	FreeVec ((APTR) con);
    }
}

/****** appshell.library/DisposeObjList ***********************************
*
*   NAME
*	DisposeObjList - Delete a window environment
*
*   SYNOPSIS
*	DisposeObjList (oi);
*			 a1
*
*	struct ObjectInfo *oi;
*
*   FUNCTION
*	This function will delete the object list allocated for a window
*	environment.
*
*   INPUTS
*	oi	- ObjectInfo structure returned by NewObjListA()
*
*   SEE ALSO
*	NewObjListA()
*
*********************************************************************
*
* Created:  10-Oct-90, David N. Junod
*
*/

VOID __saveds __asm DisposeObjList (register __a1 struct ObjectInfo * oi)
{

    /* Make sure something was passed */
    if (oi)
    {
	struct List *list = &(oi->oi_ObjList);

	/* Free the GadTools gadgets */
	if (oi->oi_GList)
	{
	    FreeGadgets (oi->oi_GList);
	}

	/* Delete all the ObjectNodes */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;

	    node = list->lh_Head;
	    while (nxtnode = node->ln_Succ)
	    {
		/* Remove the object from the list */
		Remove (node);

		/* Dispose of the object */
		DisposeObj ((struct ObjectNode *) node);

		/* Get the next node in the list */
		node = nxtnode;

	    }			/* End of while objects */
	}			/* End of if list */

	/* Free Intuition drawing information */
	if (oi->oi_DrInfo)
	{
	    FreeScreenDrawInfo (oi->oi_Screen, oi->oi_DrInfo);
	}

	/* Free the GadTools visual information */
	if (oi->oi_VisualInfo)
	{
	    FreeVisualInfo (oi->oi_VisualInfo);
	}

	/* Free the cloned NewWindow attributes */
	if (oi->oi_WindowAttrs)
	{
	    FreeTagItems (oi->oi_WindowAttrs);
	}

	/* Free the font */
	if (oi->oi_TextFont)
	{
	    CloseFont (oi->oi_TextFont);
	}

	/* Free the ObjectInfo structure */
	FreeVec ((APTR) oi);
    }				/* End of if ObjectInfo */
}

struct WorkData *AllocWorkData (struct ObjectInfo * oi, struct TagItem * attrs)
{
    struct WorkData *pwd;

    /* Allocate the work data area */
    if (pwd = (struct WorkData *)
	AllocVec (sizeof (struct WorkData), MEMF_CLEAR))
    {
	struct NewGadget *ng;	/* work NewGadget structure */

	/* Remember our ObjectInfo structure */
	pwd->pwd_OI = oi;

	/* Initialize the next gadget pointer */
	pwd->pwd_Gad = oi->oi_Gadgets;

	/* initialize the default font information */
	pwd->pwd_swidth = DEF_WIDTH;
	pwd->pwd_sheight = DEF_HEIGHT;
	pwd->pwd_sbase = DEF_BASE;

	pwd->pwd_fwidth = (LONG) TextLength (&(oi->oi_RastPort), "n", 1);
	pwd->pwd_fheight = (LONG) oi->oi_TextFont->tf_YSize;
	pwd->pwd_fbase = (LONG) oi->oi_TextFont->tf_Baseline;

	/* Initialize the constant portion of the NewGadget structure */
	ng = &(pwd->pwd_NewGadget);
	ng->ng_TextAttr = oi->oi_TextAttr;
	ng->ng_VisualInfo = oi->oi_VisualInfo;

	/* Initialize the layout smarts */
	InitLayoutWorkData (pwd);
    }

    return (pwd);
}

VOID FreeWorkData (struct WorkData * pwd)
{

    if (pwd)
    {
	/* Free the layout information */
	FreeLayoutWorkData (pwd);

	/* Free the work data */
	FreeVec ((APTR) pwd);
    }
}
