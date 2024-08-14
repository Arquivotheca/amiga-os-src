/* windowclass.c
 * Copyright © 1992 Commodore-Amiga, Inc.
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <string.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include "windowclass.h"
#include "windowclass_iprotos.h"

/*****************************************************************************/

#define ASM				__asm
#define REG(x)				register __ ## x
#define MEMORY_FOLLOWING(ptr)		((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)	((void *)( ((ULONG)ptr) + n ))

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object *obj, Msg message);
ULONG __stdargs DoMethod (Object *obj, unsigned long MethodID, ...);
ULONG __stdargs DoSuperMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs DoSuperMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs CoerceMethodA (struct IClass *cl, Object *obj, Msg message);
ULONG __stdargs CoerceMethod (struct IClass *cl, Object *obj, unsigned long MethodID, ...);
ULONG __stdargs SetSuperAttrs (struct IClass *cl, Object *obj, unsigned long Tag1, ...);

/*****************************************************************************/
/* Data structures required for our view window class                        */
/*****************************************************************************/

/* Global data required by our View Window class */
struct ViewWindowClass
{
    /* ROM libraries used by the View Window class */
    struct Library	*wc_SysBase;
    struct Library	*wc_DOSBase;
    struct Library	*wc_GfxBase;
    struct Library	*wc_IntuitionBase;
    struct Library	*wc_UtilityBase;
    struct Library	*wc_WorkbenchBase;

    /* Data needed by the View Window class */
    Class		*wc_ModelClass;
};

/*****************************************************************************/

#define	DOSBase		 wc->wc_DOSBase
#define	GfxBase		 wc->wc_GfxBase
#define	IntuitionBase	 wc->wc_IntuitionBase
#define	UtilityBase	 wc->wc_UtilityBase
#define	WorkbenchBase	 wc->wc_WorkbenchBase

/*****************************************************************************/

struct ModelObj
{
    LONG		 mo_VInc;
    LONG		 mo_TotVert;
    LONG		 mo_VisVert;
    LONG		 mo_TopVert;

    LONG		 mo_HInc;
    LONG		 mo_TotHoriz;
    LONG		 mo_VisHoriz;
    LONG		 mo_TopHoriz;

    ULONG		 mo_ID;
};

/*****************************************************************************/

struct WindowObj
{
    struct Screen	*wo_Screen;
    struct DrawInfo	*wo_DrInfo;
    struct MsgPort	*wo_IDCMP;		/* Shared IDCMP port (if non-NULL) */
    struct Window	*wo_Window;		/* Window pointer */
    struct AppWindow	*wo_AppWindow;
    struct Gadget	*wo_First;
    Object		*wo_Model;
    struct Image	*wo_ArrowImg[NUM_ARROWS];
    struct Gadget	*wo_VSlider;		/* Vertical slider */
    struct Gadget	*wo_HSlider;		/* Horizontal slider */
    UBYTE		 wo_Title[128];
    APTR		 wo_UserData;
};

/*****************************************************************************/
/* Data required for our view window class                                   */
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
    WOA_TopVert,
    WOA_VisVert,
    WOA_TotVert,
    WOA_IncVert,
    WOA_DecVert,
    WOA_VertInc,

    WOA_TopHoriz,
    WOA_VisHoriz,
    WOA_TotHoriz,
    WOA_IncHoriz,
    WOA_DecHoriz,
    WOA_HorizInc,

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

static struct TagItem leftm_map[] =
{
    {GA_ID, WOA_DecHoriz},
    {TAG_DONE,},
};

static struct TagItem upm_map[] =
{
    {GA_ID, WOA_DecVert},
    {TAG_DONE,},
};

static struct TagItem rightm_map[] =
{
    {GA_ID, WOA_IncHoriz},
    {TAG_DONE,},
};

static struct TagItem downm_map[] =
{
    {GA_ID, WOA_IncVert},
    {TAG_DONE,},
};

static struct TagItem *model_map[] =
{
    leftm_map,
    upm_map,
    rightm_map,
    downm_map,
};

/*****************************************************************************/

static struct TagItem vm_map[] =
{
    {WOA_TopVert, PGA_Top},
    {WOA_VisVert, PGA_Visible},
    {WOA_TotVert, PGA_Total},
    {TAG_DONE,},
};

static struct TagItem hm_map[] =
{
    {WOA_TopHoriz, PGA_Top},
    {WOA_VisHoriz, PGA_Visible},
    {WOA_TotHoriz, PGA_Total},
    {TAG_DONE,},
};

/*****************************************************************************/

static struct TagItem vg_map[] =
{
    {PGA_Top, WOA_TopVert},
    {PGA_Visible, WOA_VisVert},
    {PGA_Total, WOA_TotVert},
    {TAG_DONE,},
};

static struct TagItem hg_map[] =
{
    {PGA_Top, WOA_TopHoriz},
    {PGA_Visible, WOA_VisHoriz},
    {PGA_Total, WOA_TotHoriz},
    {TAG_DONE,},
};

/*****************************************************************************/
/* Stubs for system functions that require tags                              */
/*****************************************************************************/

static APTR newobject (struct ViewWindowClass * wc, Class * cl, STRPTR name, Tag tag1,...)
{
    return (NewObjectA (cl, name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

static struct Window *openwindowtags (struct ViewWindowClass * wc, Tag tag1,...)
{
    return (OpenWindowTagList (NULL, (struct TagItem *) & tag1));
}

/*****************************************************************************/

static ULONG notifychanges (struct ViewWindowClass * wc, Object * o, void * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/
/* Model class used to wire the components together                          */
/*****************************************************************************/

static ULONG setModelClassAttrs (struct ViewWindowClass * wc, Class * cl, Object * o, struct opSet * msg)
{
    struct ModelObj *mo = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0L;
    LONG newHoriz;
    ULONG tidata;
    LONG newVert;

    /* start with original value */
    newHoriz = mo->mo_TopHoriz;
    newVert = mo->mo_TopVert;

    /* Process tags */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case WOA_TotVert:
		if (mo->mo_TotVert != tidata)
		{
		    mo->mo_TotVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisVert:
		if (mo->mo_VisVert != tidata)
		{
		    mo->mo_VisVert = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TopVert:
		newVert = tidata;
		break;

	    case WOA_DecVert:
		newVert -= mo->mo_VInc;
		break;

	    case WOA_IncVert:
		newVert += mo->mo_VInc;
		break;

	    case WOA_VertInc:
		mo->mo_VInc = tidata;
		break;

	    case WOA_TotHoriz:
		if (mo->mo_TotHoriz != tidata)
		{
		    mo->mo_TotHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_VisHoriz:
		if (mo->mo_VisHoriz != tidata)
		{
		    mo->mo_VisHoriz = tidata;
		    retval = 1L;
		}
		break;

	    case WOA_TopHoriz:
		newHoriz = tidata;
		break;

	    case WOA_DecHoriz:
		newHoriz -= mo->mo_HInc;
		break;

	    case WOA_IncHoriz:
		newHoriz += mo->mo_HInc;
		break;

	    case WOA_HorizInc:
		mo->mo_HInc = tidata;
		break;
	}
    }

    /* VERTICAL Validity check */
    if (mo->mo_VisVert <= 0)
    {
	mo->mo_VisVert = 1;
	retval = 1L;
    }
    if (mo->mo_TotVert <= 0)
    {
	mo->mo_TotVert = 1;
	retval = 1L;
    }
    if (newVert > (mo->mo_TotVert - mo->mo_VisVert))
	newVert = mo->mo_TotVert - mo->mo_VisVert;
    if (newVert < 0)
	newVert = 0;
    if (mo->mo_TopVert != newVert)
    {
	mo->mo_TopVert = newVert;
	retval = 1L;
    }

    /* HORIZONTAL Validity check */
    if (mo->mo_VisHoriz <= 0)
    {
	mo->mo_VisHoriz = 1;
	retval = 1L;
    }
    if (mo->mo_TotHoriz <= 0)
    {
	mo->mo_TotHoriz = 1;
	retval = 1L;
    }
    if (newHoriz > (mo->mo_TotHoriz - mo->mo_VisHoriz))
	newHoriz = mo->mo_TotHoriz - mo->mo_VisHoriz;
    if (newHoriz < 0)
	newHoriz = 0;
    if (mo->mo_TopHoriz != newHoriz)
    {
	mo->mo_TopHoriz = newHoriz;
	retval = 1L;
    }
    return (retval);
}

/*****************************************************************************/

static ULONG getModelClassAttr (struct ViewWindowClass * wc, Class * cl, Object * o, struct opGet * msg)
{
    struct ModelObj *mo = INST_DATA (cl, o);
    ULONG retval = 1L;

    switch (msg->opg_AttrID)
    {
	case WOA_TopVert:
	    *msg->opg_Storage = mo->mo_TopVert;
	    break;

	case WOA_VisVert:
	    *msg->opg_Storage = mo->mo_VisVert;
	    break;

	case WOA_TotVert:
	    *msg->opg_Storage = mo->mo_TotVert;
	    break;

	case WOA_TopHoriz:
	    *msg->opg_Storage = mo->mo_TopHoriz;
	    break;

	case WOA_VisHoriz:
	    *msg->opg_Storage = mo->mo_VisHoriz;
	    break;

	case WOA_TotHoriz:
	    *msg->opg_Storage = mo->mo_TotHoriz;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

static ULONG ASM dispatchModelClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ViewWindowClass *wc = (struct ViewWindowClass *) cl->cl_UserData;
    struct ModelObj *mo = INST_DATA (cl, o);
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
		mo = INST_DATA (cl, newobj);

		/* Provide reasonable defaults */
		mo->mo_HInc = mo->mo_VInc = 1;

		/* Get the gadget ID */
		mo->mo_ID = GetTagData (GA_ID, NULL, ops->ops_AttrList);

		/* Initialize instance data */
		setModelClassAttrs (wc, cl, newobj, ops);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = (ULONG) getModelClassAttr (wc, cl, o, (struct opGet *) msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    if (!DoSuperMethod (cl, o, ICM_CHECKLOOP))
	    {
		ULONG visvt = WOA_VisVert, ovisv = mo->mo_VisVert;
		ULONG totvt = WOA_TotVert, ototv = mo->mo_TotVert;
		ULONG visht = WOA_VisHoriz, ovish = mo->mo_VisHoriz;
		ULONG totht = WOA_TotHoriz, ototh = mo->mo_TotHoriz;
		ULONG nmflag = 0L;

		/*
		 * Let the superclass see whatever it wants from OM_SET, such as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY, however, we control all traffic and issue notification specifically for the attributes we're interested in.
		 */
		if (msg->MethodID == OM_SET)
		    DoSuperMethodA (cl, o, msg);
		else
		    nmflag = ((struct opUpdate *) msg)->opu_Flags;

		/* change 'em, only if changed (or if a "non-interim" message. */
		if ((retval = setModelClassAttrs (wc, cl, o, ops)) || !(nmflag & OPUF_INTERIM))
		{
		    /* Only filter for interim events */
		    if (nmflag & OPUF_INTERIM)
		    {
			if (ovisv == mo->mo_VisVert)
			    visvt = TAG_IGNORE;
			if (ototv == mo->mo_TotVert)
			    totvt = TAG_IGNORE;
			if (ovish == mo->mo_VisHoriz)
			    visht = TAG_IGNORE;
			if (ototh == mo->mo_TotHoriz)
			    totht = TAG_IGNORE;
		    }

		    /*
		     * Pass along GInfo, if any, so gadgets can redraw themselves.  Pass along opu_Flags, so that the application will know the difference between an interim message and a final message
		     */
		    notifychanges (wc, o, ops->ops_GInfo, (nmflag & OPUF_INTERIM),
				   GA_ID, mo->mo_ID,
				   WOA_TopVert, mo->mo_TopVert,
				   visvt, mo->mo_VisVert,
				   totvt, mo->mo_TotVert,
				   WOA_TopHoriz, mo->mo_TopHoriz,
				   visht, mo->mo_VisHoriz,
				   totht, mo->mo_TotHoriz,
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
/* Group gadget class used to manage the view window compononents            */
/*****************************************************************************/

static ULONG setViewWindowClassAttrs (struct ViewWindowClass * wc, Class * cl, Object * o, struct opSet * msg)
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

	    case WOA_UserData:
		wo->wo_UserData = (APTR) tidata;
		break;
	}
    }
    return (retval);
}

/*****************************************************************************/

static ULONG getViewWindowClassAttr (struct ViewWindowClass * wc, Class * cl, Object * o, struct opGet * msg)
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

	case WOA_UserData:
	    *msg->opg_Storage = (ULONG) wo->wo_UserData;
	    break;

	default:
	    /* I don't recognize this one, let the superclass try */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

static BOOL initWindowObject (struct ViewWindowClass * wc, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *attrs = msg->ops_AttrList;
    struct WindowObj *wo = INST_DATA (cl, o);
    struct Gadget *prev = NULL;
    struct MsgPort *awport;
    struct MsgPort *iport;
    BOOL borderless;
    ULONG iflags;
    Object *ic;
    BOOL gzz;
    WORD i;

    /* We have to have a Screen */
    if ((wo->wo_Screen = (struct Screen *) GetTagData (WA_PubScreen, NULL, attrs)) ||
	(wo->wo_Screen = (struct Screen *) GetTagData (WA_CustomScreen, NULL, attrs)))
    {
	/* We have to have a DrawInfo */
	if (wo->wo_DrInfo = GetScreenDrawInfo (wo->wo_Screen))
	{
	    /* Need some glue to hook all the objects together */
	    if (wo->wo_Model = newobject (wc, wc->wc_ModelClass, NULL, TAG_DONE))
	    {
		/* Do we want borderless propgadgets? */
		borderless = TRUE;
		if ((IntuitionBase->lib_Version < 39) || (wo->wo_Screen->BitMap.Depth == 1))
		    borderless = FALSE;

		/* They are a waste, but they are supported here... */
		if (gzz = (BOOL) GetTagData (WA_GimmeZeroZero, FALSE, attrs))
		    ((struct Gadget *) o)->GadgetType |= GTYP_GZZGADGET;

		/*********************/
		/* Create the arrows */
		/*********************/
		for (i = 0; i < NUM_ARROWS; i++)
		{
		    if (wo->wo_ArrowImg[i] = newobject (wc, NULL, "sysiclass",
							SYSIA_Size, 0,
							SYSIA_DrawInfo, wo->wo_DrInfo,
							SYSIA_Which, LEFTIMAGE + i,
							TAG_DONE))
		    {
			if (prev = newobject (wc, NULL, "buttongclass",
					      GA_RelRight, (ULONG) ac[i].Left,
					      GA_RelBottom, (ULONG) ac[i].Top,
					      GA_Width, (ULONG) ac[i].Width,
					      GA_Height, (ULONG) ac[i].Height,
					      GA_Image, wo->wo_ArrowImg[i],
					      GA_ID, (ULONG) (LEFT_GID + i),
					      GA_GZZGadget, gzz,
					      ((prev) ? GA_Previous : TAG_IGNORE), prev,
					      ICA_TARGET, wo->wo_Model,
					      ICA_MAP, model_map[i],
					      TAG_DONE))
			{
			    if (!wo->wo_First)
				wo->wo_First = prev;
			    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
			}
		    }
		}

		if (prev)
		{
		    /* Create the scrollers */
		    if (prev = wo->wo_HSlider = newobject (wc, NULL, "propgclass",
							   GA_Left, 4L,
							   GA_RelBottom, -7L,
							   GA_RelWidth, -55L,
							   GA_Height, 6L,
							   GA_Previous, prev,
							   GA_BottomBorder, TRUE,
							   GA_ID, HORIZ_GID,
							   GA_GZZGadget, gzz,
							   PGA_Freedom, FREEHORIZ,
							   ICA_TARGET, wo->wo_Model,
							   ICA_MAP, hg_map,
							   PGA_Borderless, borderless,
							   TAG_MORE, prop_tags))
		    {
			DoMethod (o, OM_ADDMEMBER, (LONG) prev);
			if (ic = newobject (wc, NULL, ICCLASS,
					    ICA_TARGET, prev,
					    ICA_MAP, hm_map,
					    TAG_DONE))
			{
			    DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);
			}

			if (prev = wo->wo_VSlider = newobject (wc, NULL, "propgclass",
							       GA_RelRight, -13L,
							       GA_Top, wo->wo_Screen->BarHeight + 2,
							       GA_Width, 10,
							       GA_RelHeight, -(wo->wo_Screen->BarHeight + 35),
							       GA_Previous, prev,
							       GA_RightBorder, TRUE,
							       GA_ID, VERT_GID,
							       GA_GZZGadget, gzz,
							       PGA_Freedom, FREEVERT,
							       ICA_TARGET, wo->wo_Model,
							       ICA_MAP, vg_map,
							       PGA_Borderless, borderless,
							       TAG_MORE, prop_tags))
			{
			    DoMethod (o, OM_ADDMEMBER, (LONG) prev);
			    if (ic = newobject (wc, NULL, ICCLASS,
						ICA_TARGET, prev,
						ICA_MAP, vm_map,
						TAG_DONE))
			    {
				DoMethod (wo->wo_Model, OM_ADDMEMBER, (LONG) ic);
			    }

			    /* What IDCMP flags */
			    iflags = GetTagData (WOA_IDCMP, NULL, attrs);
			    iport = (struct MsgPort *) GetTagData (WOA_IDCMPPort, NULL, attrs);

			    /* Open the window */
			    if (wo->wo_Window = openwindowtags (wc,
								WA_AutoAdjust,	TRUE,
								WA_CloseGadget,	TRUE,
								WA_DepthGadget,	TRUE,
								WA_DragBar,	TRUE,
								WA_SizeGadget,	TRUE,
								WA_SizeBRight,	TRUE,
								WA_SizeBBottom,	TRUE,
								WA_NewLookMenus,TRUE,
								WA_MinWidth,	80L,
								WA_MinHeight,	wo->wo_Screen->BarHeight + 38,
								WA_Gadgets,	wo->wo_First,
								((iport) ? TAG_IGNORE : WA_IDCMP), iflags,
								TAG_MORE,	attrs))
			    {
				/* Do we share an IDCMP port? */
				if (wo->wo_IDCMP = iport)
				{
				    /* Make it share the IDCMP port */
				    wo->wo_Window->UserPort = wo->wo_IDCMP;
				    ModifyIDCMP (wo->wo_Window, iflags);
				}

				/* Are we supposed to be an AppWindow? */
				if (awport = (struct MsgPort *) GetTagData (WOA_AWPort, NULL, attrs))
				    wo->wo_AppWindow = AddAppWindowA (0, (ULONG) wo, wo->wo_Window, awport, NULL);

				/* Set the window attributes */
				setViewWindowClassAttrs (wc, cl, o, msg);

				/* Indicate success */
				return TRUE;
			    }
			}
		    }
		}

		/* Dispose of the model */
		DisposeObject (wo->wo_Model);
	    }

	    /* Free the DrawInfo */
	    FreeScreenDrawInfo (wo->wo_Screen, wo->wo_DrInfo);
	}
    }

    return FALSE;
}

/*****************************************************************************/

static void CloseWindowSafely (struct ViewWindowClass * wc, struct Window * win)
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
	ClearMenuStrip (win);

    /* close the window */
    CloseWindow (win);
}

/*****************************************************************************/

static ULONG ASM dispatchViewWindowClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ViewWindowClass *wc = (struct ViewWindowClass *) cl->cl_UserData;
    struct WindowObj *wo = INST_DATA (cl, o);
    Object *newobj;
    ULONG retval;
    WORD i;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		if (initWindowObject (wc, cl, newobj, (struct opSet *) msg))
		{
		    setViewWindowClassAttrs (wc, cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = setViewWindowClassAttrs (wc, cl, o, (struct opSet *) msg);
	    break;

	case OM_GET:
	    retval = getViewWindowClassAttr (wc, cl, o, (struct opGet *) msg);
	    break;

	case OM_DISPOSE:
	    /* Remove the AppWindow */
	    if (wo->wo_AppWindow)
		RemoveAppWindow (wo->wo_AppWindow);

	    /* Close the window */
	    if (wo->wo_Window)
	    {
		/* Close the window */
		if (wo->wo_IDCMP)
		    CloseWindowSafely (wc, wo->wo_Window);
		else
		    CloseWindow (wo->wo_Window);
	    }

	    /* Get rid of the arrow images */
	    for (i = 0; i < NUM_ARROWS; i++)
		if (wo->wo_ArrowImg[i])
		    DisposeObject (wo->wo_ArrowImg[i]);

	    if (wo->wo_DrInfo)
		FreeScreenDrawInfo (wo->wo_Screen, wo->wo_DrInfo);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/
/* Application interface for the view window class                           */
/*****************************************************************************/

/****** windowclass/initViewWindowClass ******************************
*
*    NAME
*	initViewWindowClass - Initialize the boopsi View Window class.
*
*    SYNOPSIS
*	wclass = initViewWindowClass ()
*
*	Class *initViewWindowClass (void);
*
*    FUNCTION
*	This function initializes the private boopsi View Window
*	class for application use.
*
*    NOTES
*	Do NOT make this a public boopsi class!
*
*    RESULT
*	wclass - Pointer to an Intuition class structure.
*
*    SEE ALSO
*	freeViewWindowClass()
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

Class *initViewWindowClass (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Process *pr = (struct Process *) SysBase->ThisTask;
    struct ViewWindowClass *wc;
    Class *cl;

    /* 2.04 is the minimum requirement for these functions */
    if (SysBase->LibNode.lib_Version < 37)
    {
	pr->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
	return NULL;
    }

    /* Allocate the instance data need for the view window class */
    if (wc = AllocVec (sizeof (struct ViewWindowClass), MEMF_CLEAR))
    {
	/* Open all the ROM libraries that we may need */
	wc->wc_SysBase       = SysBase;
	wc->wc_DOSBase       = OpenLibrary ("dos.library", 37);
	wc->wc_GfxBase       = OpenLibrary ("graphics.library", 37);
	wc->wc_IntuitionBase = OpenLibrary ("intuition.library", 37);
	wc->wc_UtilityBase   = OpenLibrary ("utility.library", 37);
	wc->wc_WorkbenchBase = OpenLibrary ("workbench.library", 37);

	/* Make the view model class */
	if (wc->wc_ModelClass = MakeClass (NULL, MODELCLASS, NULL, sizeof (struct ModelObj), 0))
	{
	    /* Initialize the view model class */
	    wc->wc_ModelClass->cl_Dispatcher.h_Entry = dispatchModelClass;
	    wc->wc_ModelClass->cl_UserData = (ULONG) wc;

	    /* Make the view window class */
	    if (cl = MakeClass (NULL, GADGETCLASS, NULL, sizeof (struct WindowObj), 0L))
	    {
		/* Initialize the class hook */
		cl->cl_Dispatcher.h_Entry = dispatchViewWindowClass;
		cl->cl_UserData = (ULONG) wc;
		return (cl);
	    }

	    /* Free the model class */
	    FreeClass (wc->wc_ModelClass);
	}

	/* Free the instance data */
	FreeVec (wc);
    }

    /* Any failure means not enough memory */
    pr->pr_Result2 = ERROR_NO_FREE_STORE;

    return (NULL);
}

/****** windowclass/initViewWindowClass ******************************
*
*    NAME
*	freeViewWindowClass - Free the boopsi View Window class.
*
*    SYNOPSIS
*	freeViewWindowClass (cl);
*
*	void freeViewWindowClass (Class *);
*
*    FUNCTION
*	This function is used to free the private boopsi View
*	Window class that was initialized by initViewWindowClass().
*
*    INPUTS
*	cl -- Pointer to the boopsi class that was returned by
*	    initViewWindowClass().  NULL is valid value.
*
*    SEE ALSO
*	intViewWindowClass()
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

ULONG freeViewWindowClass (Class * cl)
{
    struct ViewWindowClass *wc;
    ULONG retval = FALSE;

    /* We always check to make sure we at least have a pointer */
    if (cl)
    {
	/* Get a pointer to our global data */
	wc = (struct ViewWindowClass *) cl->cl_UserData;

	/* Try freeing the class (it won't free if there are open windows) */
	if (retval = (ULONG) FreeClass (cl))
	{
	    /* Free the view window model class */
	    FreeClass (wc->wc_ModelClass);

	    /* Close the ROM libraries */
	    CloseLibrary (wc->wc_WorkbenchBase);
	    CloseLibrary (wc->wc_UtilityBase);
	    CloseLibrary (wc->wc_IntuitionBase);
	    CloseLibrary (wc->wc_GfxBase);
	    CloseLibrary (wc->wc_DOSBase);

	    /* Free the view window class data */
	    FreeVec (wc);
	}
    }
    return (retval);
}

/****** windowclass/OpenViewWindow **********************************
*
*    NAME
*	OpenViewWindow - Open a boopsi View Window.
*
*    SYNOPSIS
*	window = OpenViewWindow (cl, tags, ...);
*
*	struct Window *OpenViewWindow (Class *, Tag, ...);
*
*    FUNCTION
*	Opens an Intuition window of the given dimensions and position,
*	with the properties specified in the attribute list.
*
*	The window will have a horizontal scroller in the bottom
*	window border and a vertical scroller in the right window
*	border.
*
*    INPUTS
*	cl -- Pointer to the boopsi class that was returned by
*	    initViewWindowClass().
*
*	tags -- Pointer to an array of tags providing additional
*	    parameters.
*
*    TAGS
*	WA_PubScreen (struct Screen *) -- Pointer to the public
*	    screen to open on.
*
*	    Default is NULL.  Applicability is (I).
*
*	WA_CustomScreen (struct Screen *) -- Pointer to the custom
*	    screen to open on.
*
*	    Default is NULL.  Applicability is (I).
*
*	WOA_IDCMP (ULONG) -- IDCMP flags.
*
*	    Default is NULL.  Applicability is (I).
*
*	WOA_IDCMPPort (struct MsgPort *) -- Pointer to a shared
*	    message port for IDCMP messages.
*
*	    Default is NULL.  Applicability is (I).
*
*	WOA_AWPort (struct MsgPort *) -- Pointer to a message
*	    port if the window is to become an AppWindow.
*
*	    Default is NULL.  Applicability is (I).
*
*	WOA_Title (STRPTR) -- Pointer to string to use as the
*	    window title.
*
*	    Default is NULL.  Applicability is (ISG).
*
*	WOA_UserData (APTR) -- Pointer to application data.
*	    The window->UserData field is taken by this
*	    class, so per-window application data must be
*	    stored using this tag.
*
*	    Default is NULL.  Applicability is (ISG).
*
*	WOA_VertInc (LONG) -- Amount to adjust the
*	    vertical top by when the Up and Down arrows
*	    are pressed.
*
*	    Default is 1.  Applicability is (IS).
*
*	WOA_TopVert (LONG) -- Top for the vertical
*	    scroller.
*
*	    Default is 0.  Applicability is (ISGNU).
*
*	WOA_VisVert (LONG) -- Number of visible rows.
*
*	    Default is 0.  Applicability is (ISGNU).
*
*	WOA_TotVert (LONG) -- Total number of rows.
*
*	    Default is NULL.  Applicability is (ISGNU).
*
*	WOA_HorizInc (LONG) -- Amount to adjust the
*	    horizontal top by when the Left and Right
*	    arrows are pressed.
*
*	    Default is 1.  Applicability is (IS).
*
*	WOA_TopHoriz (LONG) -- Top for the horizontal
*	    scroller.
*
*	    Default is 0.  Applicability is (ISGNU).
*
*	WOA_VisHoriz (LONG) -- Number of visible columns.
*
*	    Default is 0.  Applicability is (ISGNU).
*
*	WOA_TotHoriz (LONG) -- Total number of columns.
*
*	    Default is NULL.  Applicability is (ISGNU).
*
*	WOA_Window (struct Window *) -- Obtain a pointer
*	    to the window structure associated with the
*	    object.
*
*	    Default is NULL.  Applicability is (G).
*
*    NOTES
*	The following boolean window tags are set to TRUE by the window
*	class and therefore should not be used by the application:
*
*	    WA_AutoAdjust, WA_CloseGadget, WA_DepthGadget, WA_DragBar,
*	    WA_SizeGadget, WA_SizeBRight, WA_SizeBBottom and
*	    WA_NewLookMenus.
*
*	The following window tags are used by the window class and
*	therefore should not be used by the application:
*
*	    WA_MinWidth, WA_MinHeight, WA_Gadgets, and WA_IDCMP.
*
*	When the user is manipulating the scrollers of a view window,
*	the values are sent to the application via the
*	IDCMP_IDCMPUPDATE Intuition message class.  The application
*	can then find the values using the utility.library/FindTagItem()
*	function.  The values to search for are:
*
*	    WOA_TopHoriz (LONG) -- Horizontal top.
*	    WOA_TopVert (LONG) -- Vertical top.
*
*	To get the most up-to-date values, it is best to use the
*	GetCurrentTopValues() function.
*
*    SEE ALSO
*	CloseViewWindow()
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

struct Window *OpenViewWindow (Class *cl, Tag tag1,...)
{
    struct ViewWindowClass *wc;
    struct Window *w = NULL;
    Object *o;

    /* Make sure we have the class */
    if (cl)
    {
	/* Get a pointer to our global data */
	wc = (struct ViewWindowClass *) cl->cl_UserData;

	/* Create a new object */
	if (o = NewObjectA (cl, NULL, (struct TagItem *) & tag1))
	{
	    /* Get a pointer to the window */
	    GetAttr (WOA_Window, o, (ULONG *) & w);

	    /* Cache the Object pointer */
	    w->UserData = (BYTE *) o;
	}
    }
    return w;
}

/****** windowclass/SetViewWindowAttrs *******************************
*
*    NAME
*	SetViewWindowAttrs - Set boopsi View Window attributes.
*
*    SYNOPSIS
*	SetViewWindowAttrs (cl, w, tags, ...)
*
*	void SetViewWindowAttrs (Class *, struct Window *, Tag, ...);
*
*    FUNCTION
*	This function is used to set attributes pertaining to
*	View windows.
*
*	See OpenViewWindow() for a list of the settable attributes.
*
*    INPUTS
*	cl -- Pointer to the boopsi class that was returned by
*	    initViewWindowClass().
*
*	w -- Pointer to a view window.
*
*	tags -- Pointer to an array of tags providing additional
*	    parameters.
*
*    SEE ALSO
*	OpenViewWindow()
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

void SetViewWindowAttrs (Class * cl, struct Window * w, Tag tag1,...)
{
    struct ViewWindowClass *wc;

    /* Make sure we have a class pointer */
    if (cl)
    {
	/* Get a pointer to our global data */
	wc = (struct ViewWindowClass *) cl->cl_UserData;

	/* I use SetGadgetAttrs so that IDCMP_IDCMPUPDATE works */
	SetGadgetAttrsA ((struct Gadget *) w->UserData, w, NULL, (struct TagItem *) & tag1);
    }
}

/****** windowclass/GetCurrentTopValues *******************************
*
*    NAME
*	GetCurrentTopValues - Get the current Top values.
*
*    SYNOPSIS
*	GetCurrentTopValues (cl, w, toph, topv)
*
*	void GetCurrentTopValues (Class *, struct Window *,
*				  ULONG *, ULONG *);
*
*    FUNCTION
*	Used to quickly obtain the current horizontal and vertical top
*	values of a view window.
*
*	Call this function whenever you obtain an IDCMP_IDCMPUPDATE
*	message containing a WOA_TopVert or WOA_TopHoriz attribute.
*
*    INPUTS
*	cl -- Pointer to the boopsi class that was returned by
*	    initViewWindowClass().
*
*	w -- Pointer to a view window.
*
*	toph -- Where to store the horizontal top value.
*
*	topv -- Where to store the vertical top value.
*
*    SEE ALSO
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

void GetCurrentTopValues (Class * cl, struct Window * w, ULONG * toph, ULONG * topv)
{
    struct ViewWindowClass *wc;
    struct WindowObj *wo;

    /* Make sure we have a class pointer */
    if (cl)
    {
	/* Get a pointer to the data that we need */
	wc = (struct ViewWindowClass *) cl->cl_UserData;
	wo = INST_DATA (cl, (Object *) w->UserData);

	GetAttr (PGA_Top, wo->wo_VSlider, topv);
	GetAttr (PGA_Top, wo->wo_HSlider, toph);
    }
}

/****** windowclass/CloseViewWindow *******************************
*
*    NAME
*	CloseViewWindow - Close a boopsi View Window.
*
*    SYNOPSIS
*	CloseViewWindow (cl, w)
*
*	void CloseViewWindow (Class *cl, struct Window *);
*
*    FUNCTION
*	Close a boopsi View Window opened by OpenViewWindow().
*
*    INPUTS
*	cl -- Pointer to the boopsi class that was returned by
*	    initViewWindowClass().
*
*	w -- Pointer to the view window to close.  A NULL value is valid.
*
*    SEE ALSO
*	OpenViewWindow()
*
**********************************************************************
*
* Created:  6-Aug-92, David N. Junod
*
*/

void CloseViewWindow (Class * cl, struct Window * w)
{
    struct ViewWindowClass *wc;

    /* Make sure we have a class pointer */
    if (cl)
    {
	/* Get a pointer to our global data */
	wc = (struct ViewWindowClass *) cl->cl_UserData;

	if (w)
	    DisposeObject ((Object *) w->UserData);
    }
}
