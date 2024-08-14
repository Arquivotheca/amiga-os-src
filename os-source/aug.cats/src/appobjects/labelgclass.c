/* labelgclass.c
 * framed command button gadget class
 *
 * Confidential Information: Commodore-Amiga, Inc.
 * Copyright (C) 1989, 1990 Commodore-Amiga, Inc.
 * All Rights Reserved
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
#include <libraries/apshattrs.h>
#include <libraries/appobjects.h>
#include <utility/tagitem.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <string.h>

extern struct Library *SysBase, *DOSBase;
extern struct Library *IntuitionBase, *GfxBase, *UtilityBase;

#define G(o)		((struct Gadget *)(o))

/* labelgclass.c */
Class *initLabelGClass (VOID);
BOOL freeLabelGClass (Class * cl);
ULONG __saveds dispatchLabelG (Class * cl, Object * o, Msg msg);
ULONG renderLabelG (Class * cl, Object * o, struct gpRender * msg);
ULONG setLabelGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getLabelGAttrs (Class * cl, Object * o, struct opGet * msg);
VOID computeDomain (Class *, Object *, struct gpRender *, struct IBox *, struct IBox *, ULONG);
UWORD GetLabelKeystroke (STRPTR label);

/* Class prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

#define MYCLASSID	"labelgclass"
#define SUPERCLASSID	"gadgetclass"
#define	INSTANCESIZE	(sizeof(struct localObjData))

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */
    struct Gadget *lod_Parent;	/* Parent object */
    struct Image *lod_Label;	/* Label image */
    UWORD lod_Flags;		/* Placement flags */
    UWORD lod_Keystroke;	/* Keystroke assigned to gadget */
};

#define	LODF_ADJUST	0x8000

Class *initLabelGClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, INSTANCESIZE, 0L))
    {
	/* Fill in the callback hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchLabelG;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;

	/* Make the class public */
	AddClass (cl);
    }

    return (cl);
}

BOOL freeLabelGClass (Class * cl)
{

    /* Try to free the public class */
    return (FreeClass (cl));
}

ULONG __saveds dispatchLabelG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod;
    struct gpRender *gpr = (struct gpRender *) msg;
    struct RastPort *rp;
    Object *newobj;
    ULONG retval = 0L;

    lod = INST_DATA (cl, o);

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have our superclass build the base */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		STRPTR label;

		/* Get the local object data */
		lod = INST_DATA (cl, newobj);

		/* Make sure we have enough data to get started */
		if ((label = (STRPTR) GetTagData (GA_Text, NULL, attrs)) &&
		    (lod->lod_Parent = (struct Gadget *)
		     GetTagData (GA_Previous, NULL, attrs)))
		{
		    /* See if we can create the label. */
		    if (lod->lod_Label = (struct Image *)
			NewObject (NULL, "texticlass",
				   IA_Data, label,
				   TAG_MORE, attrs,
				   TAG_DONE))
		    {
			/* Set the attributes */
			setLabelGAttrs (cl, newobj, (struct opSet *) msg);

			/* Set the return value */
			retval = (ULONG) newobj;
		    }
		}

		/* See if setup was successful */
		if (!retval)
		{
		    /* Free what's there if failure */
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		}
	    }

	    break;

	case OM_GET:
	    retval = getLabelGAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DSM (cl, o, msg);

	    /* Call our set routines */
	    retval += setLabelGAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr =
		    {NULL};

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
	    /* We are transparent */
	    retval = 0;
	    break;

	case GM_RENDER:
	    /* See if the domain has changed since last render */
	    if (!(lod->lod_Flags & LODF_ADJUST))
	    {
		struct GadgetInfo *gi = gpr->gpr_GInfo;
		struct Gadget *pg = lod->lod_Parent;
		struct Image *im = lod->lod_Label;
		struct IBox box, ibox;
		WORD left;
		WORD top;
		WORD width;
		WORD height;
		WORD or = 0;
		LONG lt = GA_Left, ld;
		LONG tt = GA_Top, td;
		LONG wt = GA_Width, wd;
		LONG ht = GA_Height, hd;

		/* Set the adjust flags */
		lod->lod_Flags |= LODF_ADJUST;

		/* Compute the domain of the parent box */
		computeDomain (cl, (Object *) lod->lod_Parent, gpr, &(box), NULL, NULL);

		/* Ask text for extent */
		DoMethod ((Object *) im, IM_FRAMEBOX, NULL, (LONG)&ibox, gi->gi_DrInfo, NULL);

		/* Initialize the coordinates */
		left = box.Left;
		top = box.Top;
		width = box.Width;
		height = box.Height;

		/* Calculate the new position */
		if (lod->lod_Flags & PLACETEXT_IN)
		{
		    top += ((height) ? ((height - im->Height) / 2) : 0);
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		}
		else if (lod->lod_Flags & PLACETEXT_ABOVE)
		{
		    top -= (im->Height + INTERHEIGHT);
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 1;
		}
		else if (lod->lod_Flags & PLACETEXT_ABOVEC)
		{
		    top -= (im->Height - (im->Height / 2));
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 1;
		}
		else if (lod->lod_Flags & PLACETEXT_ABOVEI)
		{
		    top -= (im->Height - im->Height - INTERHEIGHT);
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 1;
		}
		else if (lod->lod_Flags & PLACETEXT_BELOWI)
		{
		    top += (height - im->Height - INTERHEIGHT);
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 2;
		}
		else if (lod->lod_Flags & PLACETEXT_BELOWC)
		{
		    top += (height - (im->Height / 2));
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 2;
		}
		else if (lod->lod_Flags & PLACETEXT_BELOW)
		{
		    top += (height + INTERHEIGHT);
		    left += ((width) ? ((width - im->Width) / 2) : 0);
		    or = 2;
		}
		else if (lod->lod_Flags & PLACETEXT_RIGHT)
		{
		    top += ((height) ? ((height - im->Height) / 2) : 0);
		    left += (width + INTERWIDTH);
		    or = 3;
		}
		/* Default to PLACETEXT_LEFT */
		else
		{
		    top += ((height) ? ((height - im->Height) / 2) : 0);
		    left -= (im->Width + INTERWIDTH - 1);
		    or = 4;
		}

		/* Provide defaults */
		ld = (LONG)left;
		td = (LONG) (top + 1L);
		wd = (LONG)im->Width;
		hd = (LONG)im->Height;

		if (pg->Flags & GRELRIGHT)
		{
		    lt = GA_RelRight;
		    ld = -((LONG)gi->gi_Domain.Width - ld);
		}

		if (pg->Flags & GRELBOTTOM)
		{
		    tt = GA_RelBottom;
		    td = -((LONG)gi->gi_Domain.Height - td);
		}

		if (pg->Flags & GRELWIDTH)
		{
		    switch (or)
		    {
			/* Center within */
			case 0:
			    ld = pg->LeftEdge;
			    wt = GA_RelWidth;
			    wd = pg->Width;
			    break;

			/* Above */
			case 1:
			/* Below */
			case 2:
			    wt = GA_RelWidth;
			    wd = pg->Width;
			    ld = pg->LeftEdge;
			    break;

			/* Place text on the right */
			case 3:
			    lt = GA_RelRight;
			    ld = -((LONG)gi->gi_Domain.Width - ld);
			    break;
		    }
		}

		if (pg->Flags & GRELHEIGHT)
		{
		    switch (or)
		    {
			/* Center inside */
			case 0:
			    td = pg->TopEdge;
			    ht = GA_RelHeight;
			    hd = pg->Height;
			    break;

			/* Below */
			case 2:
			    tt = GA_RelBottom;
			    td = -((LONG)gi->gi_Domain.Height - td);
			    break;

			/* Place text on right */
			case 3:
			/* Place text on left */
			case 4:
			    ht = GA_RelHeight;
			    hd = pg->Height;
			    td = pg->TopEdge;
			    break;
		    }
		}

		/* Reset our information */
		SetGadgetAttrs (G(o), gi->gi_Window, gi->gi_Requester,
				lt, ld,
				tt, td,
				wt, wd,
				ht, hd,
				TAG_DONE);
	    }

	    /* Compute the domain of the box */
	    computeDomain (cl, o, gpr, &(lod->lod_Domain), NULL, NULL);

	    /* Render the button */
	    renderLabelG (cl, o, gpr);
	    break;

	case OM_DISPOSE:
	    /* See if we have a label to dispose */
	    if (lod->lod_Label)
	    {
		/* Delete our label */
		DisposeObject ((Object *) lod->lod_Label);
	    }

	    /* Pass it up to the superclass */
	    retval = (ULONG) DSM (cl, o, msg);
	    break;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

/* Inquire attributes of an object */
ULONG getLabelGAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	    /* Trigger keystroke */
	case CGTA_Keystroke:
	    *msg->opg_Storage = (ULONG) lod->lod_Keystroke;
	    break;

	    /* Let the superclass try */
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }

    return (1L);
}

/* Set attributes of an object */
ULONG setLabelGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;
    struct localObjData *lod;
    ULONG refresh = FALSE;

    lod = INST_DATA (cl, o);

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case CGTA_Parent:
		lod->lod_Parent = (struct Gadget *) tidata;
		break;

	    /* Get the placement flags */
	    case APSH_GTFlags:
		lod->lod_Flags = (UWORD) tidata;
		refresh = TRUE;
		break;
	}
    }

    return (refresh);
}

ULONG renderLabelG (Class * cl, Object * o, struct gpRender * msg)
{
    ULONG retval = 0L;

    /* See if we need to draw */
    if (msg->gpr_Redraw != GREDRAW_TOGGLE)
    {
	struct localObjData *lod = INST_DATA (cl, o);
	struct IBox *box = &(lod->lod_Domain);
	struct impDraw imp = {NULL};

	/* Build a draw message */
	imp.MethodID = IM_DRAWFRAME;
	imp.imp_RPort = msg->gpr_RPort;
	imp.imp_Offset.X = box->Left;
	imp.imp_Offset.Y = box->Top;
	imp.imp_State = IDS_NORMAL;
	imp.imp_DrInfo = msg->gpr_GInfo->gi_DrInfo;
	imp.imp_Dimensions.Width = box->Width;
	imp.imp_Dimensions.Height = box->Height;

	/* Draw the button's frame */
	DM ((Object *) lod->lod_Label, &imp);

	/* Show success... */
	retval = 1L;
    }

    return (retval);
}
