/* windowclass.c
 * The purpose of the window class is to simplify the creation of a window with
 * horizontal and vertical scroll bars in the borders.  It doesn't handle any
 * gadget communication, except for the communication of the arrow buttons to
 * the sliders.
 *
 */

#include "multiview.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DC(x)	;

/*****************************************************************************/

#define	WINDOWCLASS	"windowclass"

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM dispatchModel (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg);
ULONG ASM dispatchWindow (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg);

/*****************************************************************************/

/* Window model data */
struct modelData
{
    LONG mod_TotVert;
    LONG mod_VisVert;
    LONG mod_TopVert;

    LONG mod_TotHoriz;
    LONG mod_VisHoriz;
    LONG mod_TopHoriz;

    ULONG mod_ID;
};

/*****************************************************************************/

static Tag model_attrs[] =
{
    DTA_Sync,
    WOA_Sync,

    WOA_TopVert,
    WOA_VisibleVert,
    WOA_TotalVert,

    WOA_TopHoriz,
    WOA_VisibleHoriz,
    WOA_TotalHoriz,

    ICA_TARGET,
    ICA_MAP,

    TAG_DONE,
};

/*****************************************************************************/

struct TagItem leftm_map[] =
{
    {GA_ID,	WOA_DecHoriz},
    {TAG_DONE,},
};

struct TagItem upm_map[] =
{
    {GA_ID,	WOA_DecVert},
    {TAG_DONE,},
};

struct TagItem rightm_map[] =
{
    {GA_ID,	WOA_IncHoriz},
    {TAG_DONE,},
};

struct TagItem downm_map[] =
{
    {GA_ID,	WOA_IncVert},
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

struct TagItem vg_map[] =
{
    {PGA_Top,		WOA_TopVert},
    {PGA_Visible,	WOA_VisibleVert},
    {PGA_Total,		WOA_TotalVert},
    {TAG_DONE,},
};

struct TagItem hg_map[] =
{
    {PGA_Top,		WOA_TopHoriz},
    {PGA_Visible,	WOA_VisibleHoriz},
    {PGA_Total,		WOA_TotalHoriz},
    {TAG_DONE,},
};

/*****************************************************************************/

struct TagItem vm_map[] =
{
    {WOA_TopVert,	PGA_Top},
    {WOA_VisibleVert,	PGA_Visible},
    {WOA_TotalVert,	PGA_Total},
    {TAG_DONE,},
};

struct TagItem hm_map[] =
{
    {WOA_TopHoriz,	PGA_Top},
    {WOA_VisibleHoriz,	PGA_Visible},
    {WOA_TotalHoriz,	PGA_Total},
    {TAG_DONE,},
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static struct TagItem prop_tags[] =
{
    {GA_RelVerify,	TRUE},
    {GA_Immediate,	TRUE},
    {GA_FollowMouse,	TRUE},
    {PGA_Total,		1},
    {PGA_Visible,	1},
    {PGA_Top,		0},
    {PGA_NewLook,	TRUE},
    {TAG_DONE,},
};

/*****************************************************************************/

Class *initWindowClass (struct GlobalData * gd)
{
    Class *mcl, *cl;

    /* Create the window model class */
    if (mcl = MakeClass (NULL, MODELCLASS, NULL, sizeof (struct modelData), 0))
    {
	/* Initialize the window model class */
	mcl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatchModel;
	mcl->cl_UserData           = (ULONG) gd;

	/* Create the window class */
	if (cl = MakeClass (NULL, GADGETCLASS, NULL, NULL, 0L))
	{
	    /* Initialize the window class */
	    cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatchWindow;
	    cl->cl_Dispatcher.h_Data  = mcl;
	    cl->cl_UserData           = (ULONG) gd;
	    return (cl);
	}
	FreeClass (mcl);
    }

    return (NULL);
}

/*****************************************************************************/

ULONG freeWindowClass (Class * cl)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
    Class *mcl = (Class *) cl->cl_Dispatcher.h_Data;

    /* Free the window class */
    if (FreeClass (cl))
    {
	/* Free the window model class */
	return (ULONG) FreeClass (mcl);
    }
    return (0);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static ULONG notifyChanges (struct GlobalData * gd, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

static ULONG setModelAttrs (struct GlobalData * gd, Class * cl, Object * o, struct opSet * msg)
{
    struct modelData *mod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0L;
    LONG newHoriz;
    ULONG tidata;
    LONG newVert;

    /* start with original value */
    newHoriz = mod->mod_TopHoriz;
    newVert = mod->mod_TopVert;

    /* Process tags */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;

	switch (tag->ti_Tag)
	{
/*	    case WOA_Sync: */
	    case DTA_Sync:
		retval = 1L;
		break;

	    case WOA_TotalVert:
		if (mod->mod_TotVert != tidata)
		{
		    mod->mod_TotVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisibleVert:
		if (mod->mod_VisVert != tidata)
		{
		    mod->mod_VisVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TopVert:
		newVert = tidata;
		break;

	    case WOA_DecVert:
		newVert--;
		break;

	    case WOA_IncVert:
		newVert++;
		break;

	    case WOA_TotalHoriz:
		if (mod->mod_TotHoriz != tidata)
		{
		    mod->mod_TotHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisibleHoriz:
		if (mod->mod_VisHoriz != tidata)
		{
		    mod->mod_VisHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TopHoriz:
		newHoriz = tidata;
		break;

	    case WOA_DecHoriz:
		newHoriz--;
		break;

	    case WOA_IncHoriz:
		newHoriz++;
		break;
	}
    }

    /* VERTICAL Validity check */
    if (mod->mod_VisVert <= 0)
    {
	mod->mod_VisVert = 1;
	retval = 1L;
    }
    if (mod->mod_TotVert <= 0)
    {
	mod->mod_TotVert = 1;
	retval = 1L;
    }
    if (newVert > (mod->mod_TotVert - mod->mod_VisVert))
	newVert = mod->mod_TotVert - mod->mod_VisVert;
    if (newVert < 0)
	newVert = 0;
    if (mod->mod_TopVert != newVert)
    {
	mod->mod_TopVert = newVert;
	retval = 1L;
    }

    /* HORIZONTAL Validity check */
    if (mod->mod_VisHoriz <= 0)
    {
	mod->mod_VisHoriz = 1;
	retval = 1L;
    }
    if (mod->mod_TotHoriz <= 0)
    {
	mod->mod_TotHoriz = 1;
	retval = 1L;
    }
    if (newHoriz > (mod->mod_TotHoriz - mod->mod_VisHoriz))
	newHoriz = mod->mod_TotHoriz - mod->mod_VisHoriz;
    if (newHoriz < 0)
	newHoriz = 0;
    if (mod->mod_TopHoriz != newHoriz)
    {
	mod->mod_TopHoriz = newHoriz;
	retval = 1L;
    }

    return (retval);
}

/*****************************************************************************/

static ULONG getModelAttr (Class * cl, Object * o, struct opGet * msg)
{
    struct modelData *mod = INST_DATA (cl, o);
    ULONG retval = 1L;

    switch (msg->opg_AttrID)
    {
	case WOA_TopVert:
	    *msg->opg_Storage = mod->mod_TopVert;
	    break;

	case WOA_VisibleVert:
	    *msg->opg_Storage = mod->mod_VisVert;
	    break;

	case WOA_TotalVert:
	    *msg->opg_Storage = mod->mod_TotVert;
	    break;

	case WOA_TopHoriz:
	    *msg->opg_Storage = mod->mod_TopHoriz;
	    break;

	case WOA_VisibleHoriz:
	    *msg->opg_Storage = mod->mod_VisHoriz;
	    break;

	case WOA_TotalHoriz:
	    *msg->opg_Storage = mod->mod_TotHoriz;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

ULONG ASM dispatchModel (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
    struct modelData *mod = INST_DATA (cl, o);
    struct opSet *ops = (struct opSet *) msg;
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have the superclass create the object */
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		/* Get the local object data */
		mod = INST_DATA (cl, newobj);

		/* Get the gadget ID */
		mod->mod_ID = GetTagData (GA_ID, NULL, ops->ops_AttrList);

		/* Initialize instance data */
		setModelAttrs (gd, cl, newobj, ops);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getModelAttr (cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		ULONG topvt = DTA_TopVert, otopv = mod->mod_TopVert;
		ULONG visvt = DTA_VisibleVert, ovisv = mod->mod_VisVert;
		ULONG totvt = DTA_TotalVert, ototv = mod->mod_TotVert;
		ULONG topht = DTA_TopHoriz, otoph = mod->mod_TopHoriz;
		ULONG visht = DTA_VisibleHoriz, ovish = mod->mod_VisHoriz;
		ULONG totht = DTA_TotalHoriz, ototh = mod->mod_TotHoriz;
		ULONG nmflag = 0L;

		/* Let the superclass see whatever it wants from OM_SET, such as ICA_TARGET, ICA_MAP, and so on.
		 * For OM_NOTIFY, however, we control all traffic and issue notification specifically for the
		 * attributes we're interested in. */
		if (msg->MethodID == OM_SET)
		    DoSuperMethodA (cl, o, msg);
		else
		    /* these flags aren't present in the message of OM_SET	 */
		    nmflag = ((struct opUpdate *) msg)->opu_Flags;

		/* change 'em, only if changed (or if a "non-interim" message. */
		if ((retval = setModelAttrs (gd, cl, o, ops)) || !(nmflag & OPUF_INTERIM))
		{
		    if (!(FindTagItem (DTA_Sync, ops->ops_AttrList) || FindTagItem (WOA_Sync, ops->ops_AttrList)))
		    {
			if (otopv == mod->mod_TopVert)
			    topvt = TAG_IGNORE;
			if (ovisv == mod->mod_VisVert)
			    visvt = TAG_IGNORE;
			if (ototv == mod->mod_TotVert)
			    totvt = TAG_IGNORE;
			if (otoph == mod->mod_TopHoriz)
			    topht = TAG_IGNORE;
			if (ovish == mod->mod_VisHoriz)
			    visht = TAG_IGNORE;
			if (ototh == mod->mod_TotHoriz)
			    totht = TAG_IGNORE;
		    }

		    /* Pass along GInfo, if any, so gadgets can redraw themselves.
		     *  Pass along opu_Flags, so that the application will know the
		     * difference between and interim message and a final message */
		    notifyChanges (gd, o, ops->ops_GInfo, (nmflag & OPUF_INTERIM),
				   GA_ID, mod->mod_ID,
				   topvt, mod->mod_TopVert,
				   visvt, mod->mod_VisVert,
				   totvt, mod->mod_TotVert,
				   topht, mod->mod_TopHoriz,
				   visht, mod->mod_VisHoriz,
				   totht, mod->mod_TotHoriz,
				   TAG_END);
		}
	    }
	    break;

	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

BOOL initWindowObject (struct GlobalData * gd, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *attrs = msg->ops_AttrList;
    struct Gadget *prev = NULL;
    BOOL success = FALSE;
    ULONG backdrop;
    LONG backfill;
    STRPTR title;
    Object *ic;
    ULONG top;
    WORD i;

    struct IBox ac[] =
    {
	{-49, -9, 16, 10},	/* Left */
	{-17, -31, 18, 10},	/* Up */
	{-33, -9, 16, 10},	/* Right */
	{-17, -20, 18, 10},	/* Down */
    };

    WORD relw, relh;

    /* Initialize the list */
    NewList ((struct List *) & gd->gd_List);

    backdrop = GetTagData (WOA_Backdrop, FALSE, attrs);
    title = (STRPTR) GetTagData (DTA_Title, NULL, attrs);
    gd->gd_WinFlags = GetTagData (WOA_Flags, NULL, attrs);

    if (gd->gd_Screen && gd->gd_DrawInfo)
    {
	/* Create the glue to hook all the objects together */
	if (!(gd->gd_Model = newobject (gd, (Class *) cl->cl_Dispatcher.h_Data, NULL, TAG_DONE)))
	    return (FALSE);

	/* Are we supposed to be a backdrop */
	if (backdrop)
	{
	    top = (gd->gd_WinFlags & WOAF_BACKDROP) ? (gd->gd_Screen->BarHeight + 1) : 0;
	    backfill = (LONG) ((gd->gd_WinFlags & WOAF_BITMAP) ? LAYERS_NOBACKFILL : LAYERS_BACKFILL);
	    if (gd->gd_Window = openwindowtags (gd,
						WA_Left,		0,
						WA_Top,			top,
						WA_Width,		gd->gd_Screen->Width,
						WA_Height,		gd->gd_Screen->Height - top,
						WA_SimpleRefresh,	TRUE,
						WA_NoCareRefresh,	TRUE,
						WA_Activate,		TRUE,
						WA_BusyPointer,		TRUE,
						WA_NewLookMenus,	TRUE,
						WA_Backdrop,		TRUE,
						WA_Borderless,		TRUE,
						WA_RMBTrap,		TRUE,
						WA_BackFill,		backfill,
						TAG_MORE, attrs))
	    {
		if (!(gd->gd_WinFlags & WOAF_BACKDROP))
		{
		    gd->gd_Window->RPort->Layer->Flags &= ~LAYERSIMPLE;
		    gd->gd_Window->RPort->Layer->Flags |= LAYERSMART;

		    gd->gd_Window->Flags &= ~WFLG_SIMPLE_REFRESH;
		}

/*		gd->gd_Window->Flags &= ~WFLG_RMBTRAP; */
		success = TRUE;
	    }
	}
	else
	{
	    /* Create the arrow imagry */
	    success = TRUE;
	    for (i = 0; (i < NUM_ARROWS) && success; i++)
	    {
		if (gd->gd_ArrowImg[i] = newobject (gd, NULL, "sysiclass",
						    SYSIA_DrawInfo, gd->gd_DrawInfo,
						    SYSIA_Which, LEFTIMAGE + i,
						    TAG_DONE))
		{
		    if (i % 2)
			/* ODD */
			ac[i].Left = -(gd->gd_ArrowImg[i]->Width - 1);
		    else
			/* EVEN */
			ac[i].Top = -(gd->gd_ArrowImg[i]->Height - 1);
		    ac[i].Width = gd->gd_ArrowImg[i]->Width;
		    ac[i].Height = gd->gd_ArrowImg[i]->Height;
		}
		else
		    success = FALSE;
	    }

	    /* Create the arrow buttons */
	    if (success)
	    {
		/* Fix the positioning of the horizontal buttons */
		ac[2].Left = -(ac[1].Width + ac[2].Width - 1);
		ac[0].Left = ac[2].Left - ac[0].Width;
		relw = -(gd->gd_Screen->WBorLeft + ac[0].Width + ac[2].Width + ac[1].Width);

		/* Fix the positioning of the vertical buttons */
		ac[3].Top = -(ac[2].Height + ac[3].Height - 1);
		ac[1].Top = ac[3].Top - ac[1].Height;
		relh = -(gd->gd_Screen->BarHeight + ac[1].Height + ac[3].Height + ac[0].Height + 3);

		for (i = 0; (i < NUM_ARROWS) && success; i++)
		{
		    if (prev = newobject (gd, NULL, "buttongclass",
					  GA_RelRight, (ULONG) ac[i].Left,
					  GA_RelBottom, (ULONG) ac[i].Top,
					  GA_Width, (ULONG) ac[i].Width,
					  GA_Height, (ULONG) ac[i].Height,
					  GA_Image, gd->gd_ArrowImg[i],
					  GA_ID, (ULONG) (LEFT_GID + i),
					  GA_Immediate, TRUE,
					  GA_RelVerify, TRUE,
					  ((prev) ? GA_Previous : TAG_IGNORE), prev,
					  ICA_TARGET, gd->gd_Model,
					  ICA_MAP, model_map[i],
					  TAG_DONE))
		    {
			if (!gd->gd_First)
			    gd->gd_First = prev;
			DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		    }
		    else
			success = FALSE;
		}
	    }

	    if (success)
	    {
		success = FALSE;

		/* Horizontal prop gadget */
		if (prev = gd->gd_HGad = newobject (gd, NULL, "propgclass",
						    GA_Left, gd->gd_Screen->WBorLeft - 1,
						    GA_RelBottom, (LONG) (ac[0].Top + 2),
						    GA_RelWidth, (LONG) relw,
						    GA_Height, (LONG) (ac[0].Height - 4),
						    GA_Previous, prev,
						    GA_BottomBorder, TRUE,
						    GA_ID, HORIZ_GID,
						    PGA_Freedom, FREEHORIZ,
						    PGA_Borderless, ((gd->gd_Screen->BitMap.Depth == 1) ? FALSE : TRUE),
						    ICA_TARGET, gd->gd_Model,
						    ICA_MAP, hg_map,
						    TAG_MORE, prop_tags))
		{
		    /* Add the object to the list */
		    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
		    if (ic = newobject (gd, NULL, ICCLASS, ICA_TARGET, prev, ICA_MAP, hm_map, TAG_DONE))
			DoMethod (gd->gd_Model, OM_ADDMEMBER, (LONG) ic);

		    /* Vertical prop gadget */
		    if (prev = gd->gd_VGad = newobject (gd, NULL, "propgclass",
							GA_RelRight, (LONG) (ac[1].Left + 4),
							GA_Top, gd->gd_Screen->BarHeight + 2,
							GA_Width, (LONG) (ac[1].Width - 8),
							GA_RelHeight, (LONG) relh,
							GA_Previous, prev,
							GA_RightBorder, TRUE,
							GA_ID, VERT_GID,
							PGA_Freedom, FREEVERT,
							PGA_Borderless, ((gd->gd_Screen->BitMap.Depth == 1) ? FALSE : TRUE),
							ICA_TARGET, gd->gd_Model,
							ICA_MAP, vg_map,
							TAG_MORE, prop_tags))
		    {
			/* Add the object to the list */
			DoMethod (o, OM_ADDMEMBER, (LONG) prev);
			if (ic = newobject (gd, NULL, ICCLASS, ICA_TARGET, prev, ICA_MAP, vm_map, TAG_DONE))
			    DoMethod (gd->gd_Model, OM_ADDMEMBER, (LONG) ic);

			if (gd->gd_Window = openwindowtags (gd,
							    WA_AutoAdjust,	TRUE,
							    WA_CloseGadget,	TRUE,
							    WA_DepthGadget,	TRUE,
							    WA_DragBar,		TRUE,
							    WA_SizeGadget,	TRUE,
							    WA_SizeBRight,	TRUE,
							    WA_SizeBBottom,	TRUE,
							    WA_SimpleRefresh,	TRUE,
							    WA_NoCareRefresh,	TRUE,
							    WA_Activate,	TRUE,
							    WA_BusyPointer,	TRUE,
							    WA_MinWidth,	80L,
							    WA_MinHeight,	gd->gd_Screen->BarHeight + 38,
							    WA_MaxWidth,	-1L,
							    WA_MaxHeight,	-1L,
							    WA_Gadgets,		gd->gd_First,
							    WA_NewLookMenus,	TRUE,
							    WA_Title,		title,
							    TAG_MORE, attrs))
			{
			    success = TRUE;
			}
		    }
		}
	    }
	}

	if (success)
	{
	    /* Set the window pointer */
	    gd->gd_OldWinPtr = gd->gd_Process->pr_WindowPtr;
	    gd->gd_Process->pr_WindowPtr = gd->gd_Window;

	    /* Make it share the IDCMP port */
	    gd->gd_Window->UserPort = gd->gd_IDCMPPort;
	    ModifyIDCMP (gd->gd_Window, IDCMP_FLAGS);

	    /* Add the menus to the window */
	    if (gd->gd_Menu = localizemenus (gd, newmenu))
	    {
		NoObjectLoaded (gd);

		if (backdrop)
		{
		    BackdropMenus (gd);
		}

		SetMenuStrip (gd->gd_Window, gd->gd_Menu);
	    }

	    if (gd->gd_AWPort)
		gd->gd_AppWindow = AddAppWindowA (0, (ULONG) gd, gd->gd_Window, gd->gd_AWPort, NULL);
	}
    }

    return (success);
}

/*****************************************************************************/

ULONG setWindowClassAttrs (struct GlobalData * gd, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *origtags;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0;

    /* Set aside the tags so that we can properly send them out */
    origtags = msg->ops_AttrList;
    msg->ops_AttrList = CloneTagItems (origtags);

    /* Handle model attributes first */
    if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_AND))
    {
	if (msg->MethodID == OM_NEW)
	    retval = DoMethod (gd->gd_Model, OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	else
	    retval = DoMethodA (gd->gd_Model, msg);
    }

    /* If this isn't a new, then pass along the attributes */
    if (msg->MethodID != OM_NEW)
    {
	/* re-clone, without re-allocating */
	RefreshTagItemClones (msg->ops_AttrList, origtags);
	if (FilterTagItems (msg->ops_AttrList, model_attrs, TAGFILTER_NOT))
	    DoMethod (o, OM_NOTIFY, msg->ops_AttrList, msg->ops_GInfo, NULL);
    }

    /* free clone and restore original */
    FreeTagItems (msg->ops_AttrList);
    msg->ops_AttrList = origtags;

    /* Process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	switch (tag->ti_Tag)
	{
	    case DTA_Sync:
		syncMenuItems (gd, cl, o);
		break;
	}
    }
    return (retval);
}

/*****************************************************************************/

ULONG getWindowAttr (struct GlobalData *gd, Class * cl, Object * o, struct opGet * msg)
{
    ULONG retval = 1L;

    /* See if it belongs to the model */
    if (TagInArray (msg->opg_AttrID, model_attrs))
	return (DoMethodA (gd->gd_Model, msg));

    /* It might belong to us */
    switch (msg->opg_AttrID)
    {
	case WOA_Window:
	    *msg->opg_Storage = (ULONG) gd->gd_Window;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}
/*****************************************************************************/

ULONG ASM dispatchWindow (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) cl->cl_UserData;
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
		DB (kprintf ("init window\n"));
		if (initWindowObject (gd, cl, newobj, (struct opSet *) msg))
		{
		    DB (kprintf ("set window attrs\n"));
		    setWindowClassAttrs (gd, cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }
	    DB (kprintf ("done initing window\n"));
	    retval = (ULONG) newobj;
	    break;

	case WOM_ADDVIEW:
	    {
		struct Window *win = gd->gd_Window;
		ULONG left, top, width, height;
		Tag wt, ht;

		if (win->Flags & WFLG_BACKDROP)
		{
		    left = 0;
		    top = 0;
		    width = win->Width;
		    height = win->Height;
		    wt = GA_Width;
		    ht = GA_Height;
		}
		else
		{
		    left = win->BorderLeft + 2;
		    top = win->BorderTop + 1;
		    width = -(win->BorderLeft + win->BorderRight + 4);
		    height = -(win->BorderTop + win->BorderBottom + 2);
		    wt = GA_RelWidth;
		    ht = GA_RelHeight;
		}

		/* Create an IC, connecting it to the DataType object */
		DoMethod (o, OM_ADDMEMBER, (LONG) gd->gd_DataObject);

		DC (kprintf ("new ic\n"));
		if (gd->gd_IC = newobject (gd, NULL, ICCLASS, ICA_TARGET, (ULONG) gd->gd_DataObject, TAG_DONE))
		{
		    DC (kprintf ("add member\n"));
		    DoMethod (gd->gd_Model, OM_ADDMEMBER, (LONG) gd->gd_IC);

		    DC (kprintf ("set dtobject attrs\n"));
		    setattrs (gd, (Object *) gd->gd_DataObject,
				  GA_Left, left,
				  GA_Top, top,
				  wt, width,
				  ht, height,
				  ICA_TARGET, o,
				  TAG_DONE);
		    DC (kprintf ("add dtobject\n"));
		    AddDTObject (gd->gd_Window, NULL, gd->gd_DataObject, -1);

		    DC (kprintf ("sync menus\n"));
		    syncMenuItems (gd, cl, o);

		    DC (kprintf ("object added\n"));
		}
	    }
	    break;

	case WOM_REMVIEW:
	    if (gd->gd_DataObject)
	    {
		/* Remove the object from the window */
		RemoveDTObject (gd->gd_Window, gd->gd_DataObject);

		/* Remove the object from the model */
		DoMethod (o, OM_REMMEMBER, (LONG) gd->gd_DataObject);
		DoMethod (gd->gd_Model, OM_REMMEMBER, (LONG) gd->gd_IC);
		DoMethod (gd->gd_IC, OM_DISPOSE);
		gd->gd_IC = NULL;

		NoObjectLoaded (gd);
	    }
	    break;

	case OM_UPDATE:
	case OM_SET:
	    return (setWindowClassAttrs (gd, cl, o, (struct opSet *) msg));
	    break;

	case OM_GET:
	    return (getWindowAttr (gd, cl, o, (struct opGet *) msg));

	case OM_ADDMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_ADDTAIL, &gd->gd_List);
	    break;

	case OM_REMMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_REMOVE, &gd->gd_List);
	    break;

	case OM_DISPOSE:
	    /* Clear the menu */
	    if (gd->gd_Menu)
	    {
		ClearMenuStrip (gd->gd_Window);
		FreeMenus (gd->gd_Menu);
		gd->gd_Menu = NULL;
	    }

	    /* Remove the AppWindow */
	    if (gd->gd_AppWindow)
		RemoveAppWindow (gd->gd_AppWindow);
	    gd->gd_AppWindow = NULL;

	    /* Close the window */
	    if (gd->gd_Window)
	    {
		/* Restore the window pointer */
		gd->gd_Process->pr_WindowPtr = gd->gd_OldWinPtr;
		CloseWindowSafely (gd, gd->gd_Window);
		gd->gd_Window = NULL;
	    }

	    /* Delete all the objects that were attached to use */
	    ostate = (Object *) gd->gd_List.mlh_Head;
	    while (member = NextObject (&ostate))
	    {
		DoMethod (member, OM_REMOVE);
		DoMethodA (member, msg);
	    }

	    /* Get rid of the arrow images */
	    for (i = 0; i < NUM_ARROWS; i++)
	    {
		if (gd->gd_ArrowImg[i])
		    DisposeObject (gd->gd_ArrowImg[i]);
		gd->gd_ArrowImg[i] = NULL;
	    }

	    /* Dispose of the model */
	    DisposeObject (gd->gd_Model);

	    gd->gd_HGad = gd->gd_VGad = NULL;
	    gd->gd_First = NULL;
	    gd->gd_Model = NULL;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}
