/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * windowclass.c
 * Written by David N. Junod
 *
 */

/*****************************************************************************/

#include "clipview.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DS(x)	;

/*****************************************************************************/

#define	WINDOWCLASS	"windowclass"

/*****************************************************************************/

struct WindowClass
{
    Class *wc_ModelClass;
};

/*****************************************************************************/

static struct IBox ac[] =
{
    {-49, -9, 16, 10},		/* Left */
    {-17, -31, 18, 10},		/* Up */
    {-33, -9, 16, 10},		/* Right */
    {-17, -20, 18, 10},		/* Down */
};

/*****************************************************************************/

static Tag model_attrs[] =
{
    DTA_Sync,
    WOA_Sync,

    WOA_TopVert,
    WOA_VisVert,
    WOA_TotVert,

    WOA_TopHoriz,
    WOA_VisHoriz,
    WOA_TotHoriz,

    ICA_TARGET,
    ICA_MAP,

    TAG_DONE,
};

/*****************************************************************************/

static struct TagItem prop_tags[] =
{
    {GA_RelVerify, TRUE},
    {GA_Immediate, TRUE},
    {GA_FollowMouse, TRUE},
    {PGA_Total, 1},
    {PGA_Visible, 1},
    {PGA_Top, 0},
    {PGA_NewLook, TRUE},
    {TAG_DONE,},
};

/*****************************************************************************/

#define	HORIZ_SLIDER	0
#define	VERT_SLIDER	1
#define	NUM_SLIDERS	2

#define	LEFT_ARROW	0
#define	UP_ARROW	1
#define	RIGHT_ARROW	2
#define	DOWN_ARROW	3
#define	NUM_ARROWS	4

struct WindowObj
{
    struct Window	*wo_Window;
    struct Menu		*wo_Menu;
    struct Menu		*wo_CommandMenu;
    struct Process	*wo_Process;
    APTR		 wo_OldWinPtr;
    struct Gadget	*wo_First;
    Object		*wo_Model;
    Object		*wo_DataObject;
    Object		*wo_IC;
    struct MinList	 wo_List;	/* List of gadgets */
    struct Image	*wo_ArrowImg[NUM_ARROWS];
    UBYTE		 wo_Title[128];
    ULONG		*wo_Methods;
    struct DTMethod	*wo_Trigger;
};

/*****************************************************************************/

struct EdMenu
{
    UBYTE em_Type;		/* NM_XXX from gadtools.h */
    LONG em_Label;
    ULONG em_Command;
    UWORD em_ItemFlags;
};

/*****************************************************************************/

struct EdMenu newmenu[] =
{
    {NM_TITLE, MENU_PROJECT, MMC_NOP, 0},
    {NM_ITEM, ITEM_SAVE_AS, MMC_SAVEAS, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_PRINT, MMC_PRINT, 0},
    {NM_ITEM, ITEM_ABOUT, MMC_ABOUT, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_QUIT, MMC_QUIT, 0},

    {NM_TITLE, MENU_EDIT, MMC_NOP, 0},
    {NM_ITEM, ITEM_MARK, MMC_MARK, 0},
    {NM_ITEM, ITEM_COPY, MMC_COPY, 0},

    {NM_END, MSG_NOTHING, MMC_NOP, 0},
};

/*****************************************************************************/

struct Menu *localizemenus (struct GlobalData * gd, struct EdMenu * em)
{
    struct NewMenu *nm;
    struct Menu *menus;
    UWORD i;

    i = 0;
    while (em[i++].em_Type != NM_END)
    {
    }

    if (!(nm = AllocVec (sizeof (struct NewMenu) * i, MEMF_CLEAR | MEMF_PUBLIC)))
	return (NULL);

    while (i--)
    {
	nm[i].nm_Type = em[i].em_Type;
	nm[i].nm_Flags = em[i].em_ItemFlags;
	nm[i].nm_UserData = (APTR) em[i].em_Command;

	if (em[i].em_Type == NM_TITLE)
	{
	    nm[i].nm_Label = GetString (gd, em[i].em_Label);
	}
	else if (em[i].em_Command == MMC_NOP)
	{
	    nm[i].nm_Label = NM_BARLABEL;
	}
	else if (em[i].em_Type != NM_END)
	{
	    nm[i].nm_CommKey = GetString (gd, em[i].em_Label);
	    nm[i].nm_Label = &nm[i].nm_CommKey[2];
	    if (nm[i].nm_CommKey[0] == ' ')
	    {
		nm[i].nm_CommKey = NULL;
	    }
	}
    }

    if (menus = CreateMenusA (nm, NULL))
    {
	if (!(layoutmenus (gd, menus, GTMN_NewLookMenus, TRUE,
			   TAG_DONE)))
	{
	    FreeMenus (menus);
	    menus = NULL;
	}
    }

    FreeVec (nm);

    return (menus);
}

/*****************************************************************************/

void DetachCommandMenu (struct GlobalData * gd, struct WindowObj * wo)
{
    struct Menu *m;

    if (wo->wo_CommandMenu)
    {
	if (m = wo->wo_Menu)
	{
	    while (m->NextMenu)
	    {
		if (m->NextMenu == wo->wo_CommandMenu)
		{
		    m->NextMenu = NULL;
		    break;
		}
		m = m->NextMenu;
	    }
	}
	FreeMenus (wo->wo_CommandMenu);
	wo->wo_CommandMenu = NULL;
    }
}

/*****************************************************************************/

BOOL SetMenuItemState (struct GlobalData * gd, struct WindowObj * wo, ULONG command, BOOL state)
{
    struct MenuItem *mi;
    struct Menu *m;

    if (m = wo->wo_Menu)
    {
	while (m)
	{
	    mi = m->FirstItem;
	    while (mi)
	    {
		if (((ULONG) GTMENUITEM_USERDATA (mi)) == command)
		{
		    if (state)
			mi->Flags |= ITEMENABLED;
		    else
			mi->Flags &= ~ITEMENABLED;
		    return (TRUE);
		}
		mi = mi->NextItem;
	    }
	    m = m->NextMenu;
	}
    }
    return (FALSE);
}

/*****************************************************************************/

void NoObjectLoaded (struct GlobalData * gd, struct WindowObj * wo)
{

    SetMenuItemState (gd, wo, MMC_SAVEAS, FALSE);
    SetMenuItemState (gd, wo, MMC_PRINT, FALSE);
    SetMenuItemState (gd, wo, MMC_MARK, FALSE);
    SetMenuItemState (gd, wo, MMC_COPY, FALSE);
}

/*****************************************************************************/

Class *initWindowClass (struct GlobalData * gd)
{
    struct WindowClass *wc;
    Class *cl;

    if (wc = AllocVec (sizeof (struct WindowClass), MEMF_CLEAR))
    {
	if (wc->wc_ModelClass = initWindowMClass (gd))
	{
	    if (cl = MakeClass (NULL, GADGETCLASS, NULL, sizeof (struct WindowObj), 0L))
	    {
		cl->cl_Dispatcher.h_Entry = dispatchWindowClass;
		cl->cl_Dispatcher.h_Data = wc;
		cl->cl_UserData = (ULONG) gd;
		return (cl);
	    }
	    freeWindowMClass (wc->wc_ModelClass);
	}
	FreeVec (wc);
    }
    return (NULL);
}

/*****************************************************************************/

ULONG freeWindowClass (Class * cl)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
    struct WindowClass *wc;
    ULONG retval = FALSE;

    if (cl)
    {
	wc = (struct WindowClass *) cl->cl_Dispatcher.h_Data;
	if (retval = (ULONG) FreeClass (cl))
	{
	    freeWindowMClass (wc->wc_ModelClass);
	    FreeVec (wc);
	}
    }
    return (retval);
}

/*****************************************************************************/

ULONG ASM dispatchWindowClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
    struct WindowObj *wo = INST_DATA (cl, o);
    ULONG retval = 0L;
    Object *ostate;
    Object *member;
    Object *newobj;
    WORD i;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		if (initWindowObject (gd, cl, newobj, (struct opSet *) msg))
		{
		    setWindowClassAttrs (gd, cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case WOM_ADDVIEW:
	    {
		struct womView *wom = (struct womView *) msg;

		DoMethod (o, OM_ADDMEMBER, (LONG) wom->wom_Object);
		if (wo->wo_IC = newobject (gd, NULL, ICCLASS,
					   ICA_TARGET, (ULONG) wom->wom_Object,
					   ICA_MAP, (ULONG) wom->wom_Map,
					   TAG_DONE))
		{
		    DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) wo->wo_IC);
		    wo->wo_DataObject = wom->wom_Object;

		    /* Set the view size */
		    setdtattrs (gd, wo->wo_DataObject, NULL,
				GA_Left, wo->wo_Window->BorderLeft + 2,
				GA_Top, wo->wo_Window->BorderTop + 1,
				GA_RelWidth, -(wo->wo_Window->BorderLeft + wo->wo_Window->BorderRight + 4),
				GA_RelHeight, -(wo->wo_Window->BorderTop + wo->wo_Window->BorderBottom + 2),
				ICA_TARGET, o,
				TAG_DONE);

		    /* Add the DataType object to the window */
		    AddDTObject (wo->wo_Window, NULL, wo->wo_DataObject, -1);
		    syncMenuItems (gd, cl, o, wo);
		}
	    }
	    break;

	case WOM_REMVIEW:
	    if (wo->wo_DataObject)
	    {
		RemoveDTObject (wo->wo_Window, wo->wo_DataObject);

		DoMethod (o, OM_REMMEMBER, (LONG) wo->wo_DataObject);
		DoMethod (wo->wo_Model, OM_REMMEMBER, (LONG) wo->wo_IC);
		DoMethod (wo->wo_IC, OM_DISPOSE);

		NoObjectLoaded (gd, wo);

		wo->wo_DataObject = NULL;
		wo->wo_IC = NULL;
	    }
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = setWindowClassAttrs (gd, cl, o, (struct opSet *) msg);
	    break;

	case OM_GET:
	    retval = getWindowClassAttr (gd, cl, o, (struct opGet *) msg);
	    break;

	case OM_ADDMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_ADDTAIL, &wo->wo_List);
	    break;

	case OM_REMMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_REMOVE, &wo->wo_List);
	    break;

	case OM_DISPOSE:
	    /* Detach the prior command menu */
	    DetachCommandMenu (gd, wo);

	    /* Clear the menu */
	    if (wo->wo_Menu)
	    {
		ClearMenuStrip (wo->wo_Window);
		FreeMenus (wo->wo_Menu);
	    }

	    /* Close the window */
	    if (wo->wo_Window)
	    {
		/* Restore the window pointer */
		wo->wo_Process->pr_WindowPtr = wo->wo_OldWinPtr;

		CloseWindow (wo->wo_Window);
	    }

	    /* Delete all the objects that were attached to use */
	    ostate = (Object *) wo->wo_List.mlh_Head;
	    while (member = NextObject (&ostate))
	    {
		DoMethod (member, OM_REMOVE);
		DoMethodA (member, msg);
	    }

	    /* Get rid of the arrow images */
	    for (i = 0; i < NUM_ARROWS; i++)
		if (wo->wo_ArrowImg[i])
		    DisposeObject (wo->wo_ArrowImg[i]);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

struct TagItem leftm_map[] =
{
    {GA_ID, WOA_DecHoriz},
    {TAG_DONE,},
};

struct TagItem upm_map[] =
{
    {GA_ID, WOA_DecVert},
    {TAG_DONE,},
};

struct TagItem rightm_map[] =
{
    {GA_ID, WOA_IncHoriz},
    {TAG_DONE,},
};

struct TagItem downm_map[] =
{
    {GA_ID, WOA_IncVert},
    {TAG_DONE,},
};

struct TagItem *model_map[] =
{
    leftm_map,
    upm_map,
    rightm_map,
    downm_map,
};

/*****************************************************************************/

struct TagItem vm_map[] =
{
    {WOA_TopVert, PGA_Top},
    {WOA_VisVert, PGA_Visible},
    {WOA_TotVert, PGA_Total},
    {TAG_DONE,},
};

struct TagItem hm_map[] =
{
    {WOA_TopHoriz, PGA_Top},
    {WOA_VisHoriz, PGA_Visible},
    {WOA_TotHoriz, PGA_Total},
    {TAG_DONE,},
};

/*****************************************************************************/

struct TagItem vg_map[] =
{
    {PGA_Top, WOA_TopVert},
    {PGA_Visible, WOA_VisVert},
    {PGA_Total, WOA_TotVert},
    {TAG_DONE,},
};

struct TagItem hg_map[] =
{
    {PGA_Top, WOA_TopHoriz},
    {PGA_Visible, WOA_VisHoriz},
    {PGA_Total, WOA_TotHoriz},
    {TAG_DONE,},
};

/*****************************************************************************/

BOOL initWindowObject (struct GlobalData * gd, Class * cl, Object * o, struct opSet * msg)
{
    struct WindowClass *wc = (struct WindowClass *) cl->cl_Dispatcher.h_Data;
    struct TagItem *attrs = msg->ops_AttrList;
    struct WindowObj *wo = INST_DATA (cl, o);
    struct Gadget *prev = NULL;
    struct DrawInfo *drinfo;
    struct MsgPort *idcmp;
    BOOL success = FALSE;
    struct Screen *scr;
    STRPTR title;
    Object *ic;
    WORD i, j;

    /* Initialize the list */
    NewList ((struct List *) & wo->wo_List);

    /* Cache the parent process */
    wo->wo_Process = (struct Process *) FindTask (NULL);

    /* Get the data that we need */
    scr = (struct Screen *) GetTagData (WA_PubScreen, NULL, attrs);
    drinfo = (struct DrawInfo *) GetTagData (WOA_DrawInfo, NULL, attrs);
    idcmp = (struct MsgPort *) GetTagData (WOA_IDCMPPort, NULL, attrs);
    title = (STRPTR) GetTagData (WOA_Title, NULL, attrs);

    if (scr && drinfo)
    {
	/* Need some glue to hook all the objects together */
	if (!(wo->wo_Model = newobject (gd, wc->wc_ModelClass, NULL, TAG_DONE)))
	    return (FALSE);

	for (i = j = 0; i < NUM_ARROWS; i++)
	{
	    if (wo->wo_ArrowImg[i] = newobject (gd, NULL, "sysiclass",
						SYSIA_Size, 0,
						SYSIA_DrawInfo, drinfo,
						SYSIA_Which, LEFTIMAGE + i,
						TAG_DONE))
	    {
		if (prev = newobject (gd, NULL, "buttongclass",
				      GA_RelRight, (ULONG) ac[i].Left,
				      GA_RelBottom, (ULONG) ac[i].Top,
				      GA_Width, (ULONG) ac[i].Width,
				      GA_Height, (ULONG) ac[i].Height,
				      GA_Image, wo->wo_ArrowImg[i],
				      GA_ID, (ULONG) (LEFT_GID + i),
				      ((prev) ? GA_Previous : TAG_IGNORE), prev,
				      ICA_TARGET, wo->wo_Model,
				      ICA_MAP, model_map[i],
				      TAG_DONE))
		{
		    if (!wo->wo_First)
			wo->wo_First = prev;
		    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		    j++;
		}
	    }
	}

	if (prev)
	{
	    if (prev = newobject (gd, NULL, "propgclass",
				  GA_Left, 4L,
				  GA_RelBottom, -7L,
				  GA_RelWidth, -55L,
				  GA_Height, 6L,
				  GA_Previous, prev,
				  GA_BottomBorder, TRUE,
				  GA_ID, HORIZ_GID,
				  PGA_Freedom, FREEHORIZ,
				  ICA_TARGET, wo->wo_Model,
				  ICA_MAP, hg_map,
				  PGA_Borderless, ((scr->BitMap.Depth == 1) ? FALSE : TRUE),
				  TAG_MORE, prop_tags))
	    {
		DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		if (ic = newobject (gd, NULL, ICCLASS,
				    ICA_TARGET, prev,
				    ICA_MAP, hm_map,
				    TAG_DONE))
		{
		    DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);
		}

		if (prev = newobject (gd, NULL, "propgclass",
				      GA_RelRight, -13L,
				      GA_Top, scr->BarHeight + 2,
				      GA_Width, 10,
				      GA_RelHeight, -(scr->BarHeight + 35),
				      GA_Previous, prev,
				      GA_RightBorder, TRUE,
				      GA_ID, VERT_GID,
				      PGA_Freedom, FREEVERT,
				      ICA_TARGET, wo->wo_Model,
				      ICA_MAP, vg_map,
				      PGA_Borderless, ((scr->BitMap.Depth == 1) ? FALSE : TRUE),
				      TAG_MORE, prop_tags))
		{
		    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		    if (ic = newobject (gd, NULL, ICCLASS,
					ICA_TARGET, prev,
					ICA_MAP, vm_map,
					TAG_DONE))
		    {
			DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);

			if (wo->wo_Window = openwindowtags (gd,
							    WA_AutoAdjust, TRUE,
							    WA_CloseGadget, TRUE,
							    WA_DepthGadget, TRUE,
							    WA_DragBar, TRUE,
							    WA_SizeGadget, TRUE,
							    WA_SizeBRight, TRUE,
							    WA_SizeBBottom, TRUE,
							    WA_SimpleRefresh, TRUE,
							    WA_Activate, TRUE,
							    WA_BusyPointer, TRUE,
							    WA_MinWidth, 80L,
							    WA_MinHeight, scr->BarHeight + 38,
							    WA_MaxWidth, -1L,
							    WA_MaxHeight, -1L,
							    WA_Gadgets, wo->wo_First,
							    WA_NewLookMenus, TRUE,
							    WA_Title, title,
							    TAG_MORE, attrs))
			{
			    /* Set the window pointer */
			    wo->wo_OldWinPtr = wo->wo_Process->pr_WindowPtr;
			    wo->wo_Process->pr_WindowPtr = wo->wo_Window;

			    /* Add the menus to the window */
			    if (wo->wo_Menu = localizemenus (gd, newmenu))
			    {
				NoObjectLoaded (gd, wo);
				SetMenuStrip (wo->wo_Window, wo->wo_Menu);
			    }

			    success = TRUE;
			}
		    }
		}
	    }
	}
    }

    return (success);
}

/*****************************************************************************/

ULONG syncMenuItems (struct GlobalData * gd, Class * cl, Object * o, struct WindowObj * wo)
{
    UWORD i;

    /* Detach the prior command menu */
    DetachCommandMenu (gd, wo);

    wo->wo_Trigger = NULL;
    if (wo->wo_Methods = (ULONG *) GetDTMethods (wo->wo_DataObject))
    {
	for (i = 0; wo->wo_Methods[i] != ~0; i++)
	{
	    switch (wo->wo_Methods[i])
	    {
		case DTM_WRITE:
		    SetMenuItemState (gd, wo, MMC_SAVEAS, TRUE);
		    break;

		case DTM_PRINT:
		    SetMenuItemState (gd, wo, MMC_PRINT, TRUE);
		    break;

		case DTM_SELECT:
		    SetMenuItemState (gd, wo, MMC_MARK, TRUE);
		    break;

		case DTM_COPY:
		    SetMenuItemState (gd, wo, MMC_COPY, TRUE);
		    break;

		case DTM_TRIGGER:
		    /* See if we have any trigger methods */
		    if (wo->wo_Trigger = GetDTTriggerMethods (wo->wo_DataObject))
		    {
			struct NewMenu *nm, *cnm;
			struct DTMethod *dtm;
			struct Menu *m;
			ULONG cnt;

			/* Count the number of menu items needed */
			dtm = wo->wo_Trigger;
			cnt = 0;
			while (dtm->dtm_Label)
			{
			    dtm++;
			    cnt++;
			}

			/* Only continue on if we counted any trigger methods */
			if (cnt)
			{
			    /* Make room for the header and tailer */
			    cnt += 2;

			    /* Allocate the menu items in one swoop */
			    if (nm = AllocVec (sizeof (struct NewMenu) * cnt, MEMF_CLEAR))
			    {
				dtm = wo->wo_Trigger;
				cnm = nm;

				/* Prepare the title */
				cnm->nm_Type  = NM_TITLE;
				cnm->nm_Label = "Commands";
				cnm++;

				while (dtm->dtm_Label)
				{
				    /* Create the menu item */
				    cnm->nm_Type  = NM_ITEM;
				    cnm->nm_Label = dtm->dtm_Label;
				    cnm->nm_UserData = (APTR) (1000 + dtm->dtm_Method);

				    /* Increment the pointers */
				    dtm++;
				    cnm++;
				}

				/* Prepare the terminator */
				cnm->nm_Type = NM_END;

				/* Create a new menu panel */
				if (wo->wo_CommandMenu = CreateMenusA (nm, NULL))
				{
				    /* Detach the current menu strip */
				    ClearMenuStrip (wo->wo_Window);

				    /* Add our new panel to the end */
				    if (m = wo->wo_Menu)
				    {
					while (m->NextMenu)
					    m = m->NextMenu;
					m->NextMenu = wo->wo_CommandMenu;
				    }

				    /* Layout the new menu */
				    if (!layoutmenus (gd, wo->wo_Menu, GTMN_NewLookMenus, TRUE, TAG_DONE))
				    {
					FreeMenus (wo->wo_CommandMenu);
					wo->wo_CommandMenu = NULL;
				    }

				    /* Reattach the menu strip */
				    SetMenuStrip (wo->wo_Window, wo->wo_Menu);
				}

				FreeVec (nm);
			    }
			}
		    }
		    break;
	    }
	}
    }
    return (1);
}

/*****************************************************************************/

ULONG setWindowClassAttrs (struct GlobalData * gd, Class * cl, Object * o, struct opSet * msg)
{
    struct WindowObj *wo = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *origtags;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0;
    ULONG tidata;

    /* Set aside the tags so that we can properly send them out */
    origtags = msg->ops_AttrList;
    msg->ops_AttrList = CloneTagItems (origtags);

    /* Handle model attributes first */
    if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_AND))
    {
	if (msg->MethodID == OM_NEW)
	    retval = DoMethod (wo->wo_Model, OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	else
	    retval = DoMethodA (wo->wo_Model, msg);
    }

    /* If this is a new, then pass along the attributes */
    if (msg->MethodID != OM_NEW)
    {
	/* re-clone, without re-allocating */
	RefreshTagItemClones (msg->ops_AttrList, origtags);

	if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_NOT))
	{
	    DoMethod (o, OM_NOTIFY, msg->ops_AttrList, msg->ops_GInfo, NULL);
	}
    }

    /* free clone and restore original */
    FreeTagItems (msg->ops_AttrList);
    msg->ops_AttrList = origtags;

    /* Process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case WOA_Title:
		if (tidata)
		    strcpy (wo->wo_Title, (STRPTR) tidata);
		else
		    wo->wo_Title[0] = 0;
		SetWindowTitles (wo->wo_Window, wo->wo_Title, ((UBYTE *) (~0)));
		break;

	    case DTA_Sync:
	    case WOA_Sync:
		syncMenuItems (gd, cl, o, wo);
		break;
	}
    }
    return (retval);
}

/*****************************************************************************/

ULONG getWindowClassAttr (struct GlobalData * gd, Class * cl, Object * o, struct opGet * msg)
{
    struct WindowObj *wo = INST_DATA (cl, o);
    ULONG retval = 1L;

    /* See if it belongs to the model */
    if (TagInArray (msg->opg_AttrID, model_attrs))
	return (DoMethodA (wo->wo_Model, msg));

    /* It might belong to us */
    switch (msg->opg_AttrID)
    {
	case WOA_Window:
	    *msg->opg_Storage = (ULONG) wo->wo_Window;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}
