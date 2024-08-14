/* windowclass.c
 *
 */

#include "amigaguidebase.h"
#include "windowclass.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DS(x)	;

/*****************************************************************************/

#define	WINDOWCLASS	"windowclass"

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | \
			IDCMP_VANILLAKEY | IDCMP_RAWKEY | \
			IDCMP_IDCMPUPDATE

/*****************************************************************************/

struct WindowClass
{
    Class *wc_ModelClass;
};

/*****************************************************************************/

static Tag model_attrs[] =
{
    DTA_Sync,
    WOA_Sync,			/* THIS WAS MISSING */

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
    struct Window		*wo_Window;
    struct Menu			*wo_Menu;
    VOID			*wo_VI;
    struct Process		*wo_Process;
    APTR			 wo_OldWinPtr;
    struct AppWindow		*wo_AppWindow;
    struct Gadget		*wo_First;
    Object			*wo_Model;
    Object			*wo_DataObj;
    Object			*wo_IC;
    struct MinList		 wo_List;	/* List of gadgets */
    struct Image		*wo_ArrowImg[NUM_ARROWS];
    UBYTE			 wo_Title[128];
    ULONG			*wo_Methods;
    struct DTMethod		*wo_Trigger;
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
    {NM_ITEM, ITEM_OPEN, MMC_OPEN, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_SAVE_AS, MMC_SAVEAS, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_PRINT, MMC_PRINT, 0},
    {NM_ITEM, ITEM_ABOUT, MMC_ABOUT, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_QUIT, MMC_QUIT, 0},

    {NM_TITLE, MENU_EDIT, MMC_NOP, 0},
    {NM_ITEM, ITEM_MARK, MMC_MARK, 0},
    {NM_ITEM, ITEM_COPY, MMC_COPY, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_SELECT_ALL, MMC_SELECTALL, 0},
    {NM_ITEM, ITEM_CLEAR_SELECTED, MMC_CLEARSELECTED, 0},

#if 0
    {NM_TITLE, MENU_SEARCH, MMC_NOP, 0},
    {NM_ITEM, ITEM_FIND, MMC_FIND, 0},
    {NM_ITEM, ITEM_FIND_NEXT, MMC_NEXT, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_GO_TO_LINE, MMC_GOTO, 0},
    {NM_ITEM, MSG_NOTHING, MMC_NOP, 0},
    {NM_ITEM, ITEM_SET_BOOKMARK, MMC_SETBOOKMARK, 0},
    {NM_ITEM, ITEM_GOTO_BOOKMARK, MMC_GOTOBOOKMARK, 0},
#endif

    {NM_TITLE, MENU_WINDOW, MMC_NOP, 0},
    {NM_ITEM, ITEM_MINIMIZE, MMC_MINIMIZE, 0},
    {NM_ITEM, ITEM_NORMAL, MMC_NORMAL, 0},
    {NM_ITEM, ITEM_MAXIMIZE, MMC_MAXIMIZE, 0},

#if 0
    {NM_TITLE, MENU_EXTRAS, MMC_NOP, 0},
    {NM_ITEM, ITEM_EXECUTE_COMMAND, MMC_EXECUTE, 0},
#endif

    {NM_TITLE, MENU_PREFS, MMC_NOP, 0},
    {NM_ITEM, ITEM_SAVE_AS_DEFAULTS, MMC_SAVE_AS_DEFAULTS, 0},

    {NM_END, MSG_NOTHING, MMC_NOP, 0},
};

/*****************************************************************************/

BOOL SetMenuItemState (struct AmigaGuideLib * ag, struct WindowObj * wo, ULONG command, BOOL state)
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

void NoObjectLoaded (struct AmigaGuideLib * ag, struct WindowObj * wo)
{

    SetMenuItemState (ag, wo, MMC_SAVEAS, FALSE);
    SetMenuItemState (ag, wo, MMC_PRINT, FALSE);
    SetMenuItemState (ag, wo, MMC_MARK, FALSE);
    SetMenuItemState (ag, wo, MMC_COPY, FALSE);
    SetMenuItemState (ag, wo, MMC_SELECTALL, FALSE);
    SetMenuItemState (ag, wo, MMC_CLEARSELECTED, FALSE);
    SetMenuItemState (ag, wo, MMC_FIND, FALSE);
    SetMenuItemState (ag, wo, MMC_NEXT, FALSE);
    SetMenuItemState (ag, wo, MMC_GOTO, FALSE);
    SetMenuItemState (ag, wo, MMC_SETBOOKMARK, FALSE);
    SetMenuItemState (ag, wo, MMC_GOTOBOOKMARK, FALSE);
    SetMenuItemState (ag, wo, MMC_MINIMIZE, FALSE);
    SetMenuItemState (ag, wo, MMC_NORMAL, FALSE);
    SetMenuItemState (ag, wo, MMC_MAXIMIZE, FALSE);
    SetMenuItemState (ag, wo, MMC_EXECUTE, FALSE);
}

/*****************************************************************************/

struct Menu *localizemenus (struct AmigaGuideLib * ag, struct WindowObj * wo, struct EdMenu * em)
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
	    nm[i].nm_Label = GetAmigaGuideStringLVO (ag, em[i].em_Label);
	}
	else if (em[i].em_Command == MMC_NOP)
	{
	    nm[i].nm_Label = NM_BARLABEL;
	}
	else if (em[i].em_Type != NM_END)
	{
	    nm[i].nm_CommKey = GetAmigaGuideStringLVO (ag, em[i].em_Label);
	    nm[i].nm_Label = &nm[i].nm_CommKey[2];
	    if (nm[i].nm_CommKey[0] == ' ')
	    {
		nm[i].nm_CommKey = NULL;
	    }
	}
    }

    if (menus = CreateMenusA (nm, NULL))
    {
	if (!(layoutmenus (ag, wo->wo_VI, menus,
			   GTMN_NewLookMenus, TRUE,
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

Class *initWindowClass (struct AmigaGuideLib * ag)
{
    struct WindowClass *wc;
    Class *cl;

    if (wc = AllocVec (sizeof (struct WindowClass), MEMF_CLEAR))
    {
	if (wc->wc_ModelClass = initWindowMClass (ag))
	{
	    if (cl = MakeClass (NULL, GADGETCLASS, NULL, sizeof (struct WindowObj), 0L))
	    {
		cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatchWindowClass;
		cl->cl_Dispatcher.h_Data = wc;
		cl->cl_UserData = (ULONG) ag;
		return (cl);
	    }
	    freeWindowMClass (wc->wc_ModelClass);
	}
	FreeVec (wc);
    }
    return (NULL);
}

/*****************************************************************************/

ULONG freeWindowClass (struct AmigaGuideLib * ag, Class * cl)
{
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
    struct AmigaGuideLib *ag = (struct AmigaGuideLib *) cl->cl_UserData;
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
		if (initWindowObject (ag, cl, newobj, (struct opSet *) msg))
		{
		    setWindowClassAttrs (ag, cl, newobj, (struct opSet *) msg);
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
		if (wo->wo_IC = NewObject (NULL, ICCLASS,
					   ICA_TARGET, (ULONG) wom->wom_Object,
					   ICA_MAP, (ULONG) wom->wom_Map,
					   TAG_DONE))
		{
		    DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) wo->wo_IC);
		    wo->wo_DataObj = wom->wom_Object;

		    /* Link the object to the window */
		    SetDTAttrs (wo->wo_DataObj, NULL, NULL,
				GA_Left, wo->wo_Window->BorderLeft + 2,
				GA_Top, wo->wo_Window->BorderTop + 1,
				GA_RelWidth, -(wo->wo_Window->BorderLeft + wo->wo_Window->BorderRight + 4),
				GA_RelHeight, -(wo->wo_Window->BorderTop + wo->wo_Window->BorderBottom + 2),
				ICA_TARGET, o,
				TAG_DONE);

		    AddDTObject (wo->wo_Window, NULL, wo->wo_DataObj, -1);
		    syncMenuItems (ag, cl, o, wo);
		}
	    }
	    break;

	case WOM_REMVIEW:
	    if (wo->wo_DataObj)
	    {
		/* Remove the object from the window */
		RemoveDTObject (wo->wo_Window, wo->wo_DataObj);

		DoMethod (o, OM_REMMEMBER, (LONG) wo->wo_DataObj);
		DoMethod (wo->wo_Model, OM_REMMEMBER, (LONG) wo->wo_IC);
		DoMethod (wo->wo_IC, OM_DISPOSE);

		NoObjectLoaded (ag, wo);

		wo->wo_DataObj = NULL;
		wo->wo_IC = NULL;
	    }
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = setWindowClassAttrs (ag, cl, o, (struct opSet *) msg);
	    break;

	case OM_GET:
	    retval = getWindowClassAttr (ag, cl, o, (struct opGet *) msg);
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
	    /* Clear the menu */
	    if (wo->wo_Menu)
	    {
		ClearMenuStrip (wo->wo_Window);
		FreeMenus (wo->wo_Menu);
	    }

	    /* Remove the AppWindow */
	    if (wo->wo_AppWindow)
		RemoveAppWindow (wo->wo_AppWindow);

	    /* Close the window */
	    if (wo->wo_Window)
	    {
		/* Restore the window pointer */
		wo->wo_Process->pr_WindowPtr = wo->wo_OldWinPtr;

		CloseWindowSafely (ag, wo->wo_Window);
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

	    DisposeObject (wo->wo_Model);

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

BOOL initWindowObject (struct AmigaGuideLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct WindowClass *wc = (struct WindowClass *) cl->cl_Dispatcher.h_Data;
    struct TagItem *attrs = msg->ops_AttrList;
    struct WindowObj *wo = INST_DATA (cl, o);
    struct Gadget *prev = NULL;
    struct DrawInfo *drinfo;
    struct MsgPort *awport;
    struct MsgPort *idcmp;
    BOOL success = FALSE;
    struct Screen *scr;
    STRPTR title;
    Object *ic;
    WORD i;

    static struct IBox ac[] =
    {
	{-49, -9, 16, 10},	/* Left */
	{-17, -31, 18, 10},	/* Up */
	{-33, -9, 16, 10},	/* Right */
	{-17, -20, 18, 10},	/* Down */
    };

    WORD relw, relh;

    /* Initialize the list */
    NewList ((struct List *) & wo->wo_List);

    /* Cache the parent process */
    wo->wo_Process = (struct Process *) FindTask (NULL);

    /* Get the information that we need */
    scr = (struct Screen *) GetTagData (WA_PubScreen, NULL, attrs);
    drinfo = (struct DrawInfo *) GetTagData (WOA_DrawInfo, NULL, attrs);
    awport = (struct MsgPort *) GetTagData (WOA_AWPort, NULL, attrs);
    idcmp = (struct MsgPort *) GetTagData (WOA_IDCMPPort, NULL, attrs);
    title = (STRPTR) GetTagData (WOA_Title, NULL, attrs);
    wo->wo_VI = (VOID *) GetTagData (WOA_VisualInfo, NULL, attrs);

    /* Make sure we have everything we need */
    if (scr && drinfo)
    {
	/* Need some glue to hook all the objects together */
	if (!(wo->wo_Model = NewObject (wc->wc_ModelClass, NULL, TAG_DONE)))
	    return (FALSE);

	/*********************/
	/* Create the arrows */
	/*********************/
	for (i = 0, success = TRUE; (i < NUM_ARROWS) && success; i++)
	{
	    if (wo->wo_ArrowImg[i] = NewObject (NULL, "sysiclass",
						SYSIA_DrawInfo, drinfo,
						SYSIA_Which, LEFTIMAGE + i,
						TAG_DONE))
		{
		    if (i % 2)
			/* ODD */
			ac[i].Left = -(wo->wo_ArrowImg[i]->Width - 1);
		    else
			/* EVEN */
			ac[i].Top = -(wo->wo_ArrowImg[i]->Height - 1);
		    ac[i].Width = wo->wo_ArrowImg[i]->Width;
		    ac[i].Height = wo->wo_ArrowImg[i]->Height;
		}
		else
		    success = FALSE;
	}

	if (success)
	{
	    /* Fix the positioning of the horizontal buttons */
	    ac[2].Left = -(ac[1].Width + ac[2].Width - 1);
	    ac[0].Left = ac[2].Left - ac[0].Width;
	    relw = -(scr->WBorLeft + ac[0].Width + ac[2].Width + ac[1].Width);

	    /* Fix the positioning of the vertical buttons */
	    ac[3].Top = -(ac[2].Height + ac[3].Height - 1);
	    ac[1].Top = ac[3].Top - ac[1].Height;
	    relh = -(scr->BarHeight + ac[1].Height + ac[3].Height + ac[0].Height + 3);

	    for (i = 0; (i < NUM_ARROWS) && success; i++)
	    {
		if (prev = NewObject (NULL, "buttongclass",
				      GA_RelRight, (ULONG) ac[i].Left,
				      GA_RelBottom, (ULONG) ac[i].Top,
				      GA_Width, (ULONG) ac[i].Width,
				      GA_Height, (ULONG) ac[i].Height,
				      GA_Image, wo->wo_ArrowImg[i],
				      GA_ID, (ULONG) (LEFT_GID + i),
				      GA_Immediate, TRUE,
				      GA_RelVerify, TRUE,
				      ((prev) ? GA_Previous : TAG_IGNORE), prev,
				      ICA_TARGET, wo->wo_Model,
				      ICA_MAP, model_map[i],
				      TAG_DONE))
		{
		    if (!wo->wo_First)
			wo->wo_First = prev;
		    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		}
		else
		    success = FALSE;
	    }
	}

	if (success)
	{
	    success = FALSE;

	    /* Create the horizontal scroller */
	    if (prev = NewObject (NULL, "propgclass",
				  GA_Left,		scr->WBorLeft - 1,
				  GA_RelBottom,		(LONG) (ac[0].Top + 2),
				  GA_RelWidth,		(LONG) relw,
				  GA_Height,		(LONG) (ac[0].Height - 4),
				  GA_Previous,		prev,
				  GA_BottomBorder,	TRUE,
				  GA_ID,		HORIZ_GID,
				  PGA_Freedom,		FREEHORIZ,
				  ICA_TARGET,		wo->wo_Model,
				  ICA_MAP,		hg_map,
				  PGA_Borderless,	((scr->BitMap.Depth == 1) ? FALSE : TRUE),
				  TAG_MORE,		prop_tags))
	    {
		DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		if (ic = NewObject (NULL, ICCLASS, ICA_TARGET, prev, ICA_MAP, hm_map, TAG_DONE))
		    DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);

		/* Vertical scroller */
		if (prev = NewObject (NULL, "propgclass",
				      GA_RelRight,	(LONG) (ac[1].Left + 4),
				      GA_Top,		scr->BarHeight + 2,
				      GA_Width,		(LONG) (ac[1].Width - 8),
				      GA_RelHeight,	(LONG) relh,
				      GA_Previous,	prev,
				      GA_RightBorder,	TRUE,
				      GA_ID,		VERT_GID,
				      PGA_Freedom,	FREEVERT,
				      ICA_TARGET,	wo->wo_Model,
				      ICA_MAP,		vg_map,
				      PGA_Borderless,	((scr->BitMap.Depth == 1) ? FALSE : TRUE),
				      TAG_MORE,		prop_tags))
		{
		    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		    if (ic = NewObject (NULL, ICCLASS, ICA_TARGET, prev, ICA_MAP, vm_map, TAG_DONE))
			DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);

		    /* Open the window */
		    if (wo->wo_Window = openwindowtags (ag,
							WA_AutoAdjust,	TRUE,
							WA_CloseGadget,	TRUE,
							WA_DepthGadget,	TRUE,
							WA_DragBar,	TRUE,
							WA_SizeGadget,	TRUE,
							WA_SizeBRight,	TRUE,
							WA_SizeBBottom,	TRUE,
							WA_SimpleRefresh,TRUE,
							WA_BusyPointer,	TRUE,
							WA_MinWidth,	80L,
							WA_MinHeight,	scr->BarHeight + 38,
							WA_MaxWidth,	-1L,
							WA_MaxHeight,	-1L,
							WA_Gadgets,	wo->wo_First,
							WA_NewLookMenus,TRUE,
							WA_Title,	title,
							TAG_MORE,	attrs))
		    {
			/* Set the window pointer */
			wo->wo_OldWinPtr = wo->wo_Process->pr_WindowPtr;
			wo->wo_Process->pr_WindowPtr = wo->wo_Window;

			/* Make it share the IDCMP port */
			wo->wo_Window->UserPort = idcmp;
			ModifyIDCMP (wo->wo_Window, IDCMP_FLAGS);

			/* Are we supposed to be an AppWindow? */
			if (awport)
			    wo->wo_AppWindow = AddAppWindowA (0, (ULONG) wo, wo->wo_Window, awport, NULL);

			/* Add the menus to the window */
			if (wo->wo_Menu = localizemenus (ag, wo, newmenu))
			{
			    NoObjectLoaded (ag, wo);
			    SetMenuStrip (wo->wo_Window, wo->wo_Menu);
			}

			success = TRUE;
		    }
		}
	    }
	}
    }
    return (success);
}


/*****************************************************************************/

ULONG syncMenuItems (struct AmigaGuideLib * ag, Class * cl, Object * o, struct WindowObj * wo)
{
    UWORD i;

    wo->wo_Trigger = NULL;
    if (wo->wo_Methods = (ULONG *) GetDTMethods (wo->wo_DataObj))
    {
	for (i = 0; wo->wo_Methods[i] != ~0; i++)
	{
	    switch (wo->wo_Methods[i])
	    {
		case DTM_SELECT:
		    SetMenuItemState (ag, wo, MMC_MARK, TRUE);
		    break;

		case DTM_CLEARSELECTED:
		    SetMenuItemState (ag, wo, MMC_CLEARSELECTED, TRUE);
		    break;

		case DTM_COPY:
		    SetMenuItemState (ag, wo, MMC_COPY, TRUE);
		    break;

		case DTM_PRINT:
		    SetMenuItemState (ag, wo, MMC_PRINT, TRUE);
		    break;

		case DTM_WRITE:
		    SetMenuItemState (ag, wo, MMC_SAVEAS, TRUE);
		    break;
	    }
	}
    }

    SetMenuItemState (ag, wo, MMC_MINIMIZE, TRUE);
    SetMenuItemState (ag, wo, MMC_NORMAL, TRUE);
    SetMenuItemState (ag, wo, MMC_MAXIMIZE, TRUE);
    return (1);
}

/*****************************************************************************/

ULONG setWindowClassAttrs (struct AmigaGuideLib * ag, Class * cl, Object * o, struct opSet * msg)
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
	    retval = DoMethodA (wo->wo_Model, (Msg) msg);
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
		syncMenuItems (ag, cl, o, wo);
		break;
	}
    }
    return (retval);
}

/*****************************************************************************/

ULONG getWindowClassAttr (struct AmigaGuideLib * ag, Class * cl, Object * o, struct opGet * msg)
{
    struct WindowObj *wo = INST_DATA (cl, o);
    ULONG retval = 1L;

    /* See if it belongs to the model */
    if (TagInArray (msg->opg_AttrID, model_attrs))
	return (DoMethodA (wo->wo_Model, (Msg) msg));

    /* It might belong to us */
    switch (msg->opg_AttrID)
    {
	case WOA_Window:
	    *msg->opg_Storage = (ULONG) wo->wo_Window;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, (Msg) msg);
	    break;
    }

    return (retval);
}
