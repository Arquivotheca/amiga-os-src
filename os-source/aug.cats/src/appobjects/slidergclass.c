/* slidergclass.c
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <utility/tagitem.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

Class *initSliderGClass (VOID);
ULONG freeSliderGClass (Class * cl);

extern struct Library *SysBase, *DOSBase;
extern struct Library *IntuitionBase, *GfxBase, *UtilityBase;

#define G(o)		((struct Gadget *)(o))
#define POINTLONG(pt)	(*((ULONG *)&(pt)))

ULONG __saveds dispatchSliderG (Class * cl, Object * o, Msg msg);
ULONG initializeSliderG (Class * cl, Object * o, struct opSet * ops);
ULONG renderSliderG (Class * cl, Object * o, struct gpRender * msg);
ULONG setSliderGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getSliderGAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG activeSliderG (Class * cl, Object * o, struct gpInput * msg);
ULONG handleSliderG (Class * cl, Object * o, struct gpInput * msg);
ULONG inactiveSliderG (Class * cl, Object * o, struct gpGoInactive * msg);
LONG GetVisualState (struct Gadget * g, struct GadgetInfo * gi);
VOID computeDomain (Class *, Object *, Msg, struct IBox *, struct IBox *, ULONG);

/* classface.asm prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);
ULONG notifyAttrChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...);

#define MYCLASSID	"slidergclass"
#define SUPERCLASSID	GADGETCLASS
#define	AFRAMEICLASS	"aframeiclass"

struct localObjData
{
    Object *lod_PropG;		/* Prop gadget */
    Object *lod_Frame;		/* Class allocated frame */

    struct
    {
	WORD V;
	WORD H;
    } lod_FMargins;		/* Margins for frame */

    struct IBox lod_Domain;	/* Domain of the gadget */
};

#define	LSIZE	(sizeof (struct localObjData))

Class *initSliderGClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LSIZE, 0L))
    {
	/* Fill in the callback hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchSliderG;

	/* Make the class public */
	AddClass (cl);
    }

    /* Return the public class */
    return (cl);
}

ULONG freeSliderGClass (Class * cl)
{
    /* Try to free the public class */
    return ((ULONG) FreeClass (cl));
}

static VOID translateInput (Class *cl, Object * o1, Object * o2, struct gpInput * msg)
{
    struct localObjData *lod = INST_DATA(cl, o1);

    msg->gpi_Mouse.X -= (G(o2)->LeftEdge - lod->lod_Domain.Left);
    msg->gpi_Mouse.Y -= (G(o2)->TopEdge - lod->lod_Domain.Top);
}

static ULONG propagateHit (Object * o, struct localObjData * lod, struct gpHitTest * msg)
{
    struct IBox mouse;
    struct IBox gbox;
    int origx;
    int origy;

    gadgetBox (G(o), &(msg->gpht_GInfo->gi_Domain), &(lod->lod_Domain), NULL, NULL);
    origx = msg->gpht_Mouse.X + lod->lod_Domain.Left;
    origy = msg->gpht_Mouse.Y + lod->lod_Domain.Top;

    gadgetBox (G(lod->lod_PropG), &(msg->gpht_GInfo->gi_Domain), &gbox, NULL, NULL);
    mouse.Left = msg->gpht_Mouse.X = (origx - gbox.Left);
    mouse.Top = msg->gpht_Mouse.Y = (origy - gbox.Top);
    gbox.Left = gbox.Top = 0;
    if (PointInBox (&mouse, &gbox) && DM (lod->lod_PropG, msg))
    {
        if (G (lod->lod_PropG)->Flags & GADGDISABLED)
        {
            return (0);
        }
        return (GMR_GADGETHIT);
    }
    return (0L);
}

static void SetPropGadget (struct localObjData *lod)
{
    G(lod->lod_PropG)->LeftEdge	= lod->lod_Domain.Left + (lod->lod_FMargins.V * 2);
    G(lod->lod_PropG)->TopEdge	= lod->lod_Domain.Top + (lod->lod_FMargins.H * 2);
    G(lod->lod_PropG)->Width	= lod->lod_Domain.Width - (lod->lod_FMargins.V * 4);
    G(lod->lod_PropG)->Height	= lod->lod_Domain.Height - (lod->lod_FMargins.H * 4);
}

ULONG __saveds dispatchSliderG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct RastPort *rp;
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Allow our superclass to create the base */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		/* Initialize the complete set */
		if (!(initializeSliderG (cl, newobj, (struct opSet *) msg)))
		{
		    /* free what's there if failure	 */
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    /* Return the new object */
	    retval = (ULONG) newobj;
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DSM (cl, o, msg);

	    /* Call our set routines */
	    retval += setSliderGAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr = {NULL};

		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_REDRAW;

		    /* Redraw... */
		    DM (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_HITTEST:
	    retval = propagateHit (o, lod, (struct gpHitTest *) msg);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
            translateInput (cl, o, lod->lod_PropG, (struct gpInput *) msg);
	    retval = DM (lod->lod_PropG, msg);
	    break;

	case GM_GOINACTIVE:
	    retval = DM (lod->lod_PropG, msg);
	    break;

	case GM_RENDER:
	    retval = DSM (cl, o, msg);

	    if (((struct gpRender *) msg)->gpr_Redraw == GREDRAW_REDRAW)
	    {
		computeDomain (cl, o, msg, &(lod->lod_Domain), NULL, NULL);
		SetPropGadget (lod);
	    }

	    renderSliderG (cl, o, (struct gpRender *) msg);
	    break;

	case OM_DISPOSE:
	    if (lod->lod_Frame)
		DM (lod->lod_Frame, msg);
	    if (lod->lod_PropG)
		DM (lod->lod_PropG, msg);
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG initializeSliderG (Class * cl, Object * o, struct opSet * ops)
{
    struct localObjData *lod = INST_DATA (cl, o);
    ULONG success = FALSE;
    struct PropInfo *si;

    if (lod->lod_PropG = NewObject (NULL, PROPGCLASS,
				    TAG_MORE, ops->ops_AttrList,
				    TAG_DONE))
    {
        si = (struct PropInfo *) G(lod->lod_PropG)->SpecialInfo;
        if ((G(o)->Activation & 0xF0))
        {
            si->Flags |= PROPNEWLOOK;
	    success = TRUE;
        }
        else
        {
            si->Flags |= PROPBORDERLESS;
            if (lod->lod_Frame = NewObject (NULL, AFRAMEICLASS,
#if 0
                                            IA_EdgesOnly, TRUE,
#endif
                                            TAG_DONE))
            {
                DoMethod (lod->lod_Frame, OM_GET, IA_LINEWIDTH, &(lod->lod_FMargins));
		success = TRUE;
            }
        }
        setSliderGAttrs (cl, o, ops);
    }
    return (success);
}

static Tag prop_attrs[] =
{
    PGA_Top,
    PGA_Visible,
    PGA_Total,
    PGA_VertPot,
    PGA_VertBody,
    PGA_HorizPot,
    PGA_HorizBody,
    GA_ID,
    TAG_DONE,
};

/* Set attributes of an object */
ULONG setSliderGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *otags;

    otags = msg->ops_AttrList;
    if (msg->ops_AttrList = CloneTagItems (otags))
    {
	if (FilterTagItems (msg->ops_AttrList, prop_attrs, TAGFILTER_AND))
	{
	    if (msg->MethodID == OM_NEW)
		DoMethod (lod->lod_PropG, OM_SET, (LONG) msg->ops_AttrList, (LONG) msg->ops_GInfo);
	    else
		DM (lod->lod_PropG, msg);
	}
	FreeTagItems (msg->ops_AttrList);
    }
    msg->ops_AttrList = otags;

    SetPropGadget (lod);

    if (FindTagItem (GA_Selected, msg->ops_AttrList) ||
	FindTagItem (GA_Disabled, msg->ops_AttrList))
    {
	return (1L);
    }
    return (0L);
}

ULONG renderSliderG (Class * cl, Object * o, struct gpRender * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct GadgetInfo *gi = msg->gpr_GInfo;
    struct IBox *fbox = &(lod->lod_Domain);
    struct RastPort *rp;
    ULONG retval = 0L;

    /* Get the rastport that we're going to write into. */
    if (lod->lod_Frame &&
	((fbox->Width > 1) && (fbox->Height > 1)) &&
	(rp = ObtainGIRPort (msg->gpr_GInfo)))
    {
	struct impDraw imp = {NULL};

	/* Prep the message */
	imp.MethodID = IM_DRAWFRAME;
	imp.imp_RPort = rp;
	imp.imp_State = IDS_NORMAL;
	imp.imp_DrInfo = gi->gi_DrInfo;

	/* See if we're doing a full draw */
	if ((msg->MethodID == GM_RENDER) &&
	    (msg->gpr_Redraw == GREDRAW_REDRAW))
	{
	    /* Prep the message */
	    imp.imp_Offset.X = fbox->Left;
	    imp.imp_Offset.Y = fbox->Top;
	    imp.imp_Dimensions.Width = fbox->Width;
	    imp.imp_Dimensions.Height = fbox->Height;

	    /* Set the state */
	    imp.imp_State = IDS_NORMAL;

	    /* Draw the button's frame */
	    DM (lod->lod_Frame, &imp);
	}

	/* Indicate success */
	retval = 1L;

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    DM (lod->lod_PropG, msg);

    return (retval);
}
