/* texticlass.c
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
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

#define IM(o)   ((struct Image *)(o))	/* transparent base class */
#define ITEXT(ptr) ((struct IntuiText *)(ptr))

#define MYCLASSID       "texticlass"
#define SUPERCLASSID    (IMAGECLASS)

Class *initTextIClass (VOID);
ULONG freeTextIClass (Class * cl);
ULONG __saveds dispatchTextI (Class * cl, Object * o, Msg msg);
ULONG frameTextI (Class *cl, Object *o, struct impFrameBox *msg);
ULONG setTextIAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getTextIAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG drawTextI (Class * cl, Object * o, struct impDraw * msg);
WORD aTextExtent (struct RastPort *, STRPTR, LONG, struct TextExtent *);
UWORD GetLabelKeystroke (STRPTR label);
static VOID getContentsExtent (Class *cl, Object *o, struct DrawInfo * drinfo);

/* Class prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

#if 0
struct Image
{
  /* Offset relative to the container */
    SHORT LeftEdge;
    SHORT TopEdge;

  /* Contains the text extent of the string */
    SHORT Width;
    SHORT Height;

  /* Maintained by boopsi */
    SHORT Depth;

  /* Pointer to a NULL terminated text string */
    USHORT *ImageData;

  /* We use this for the foreground color */
    UBYTE PlanePick;

  /* We use this for the background color */
    UBYTE PlaneOnOff;

  /* Pointer to the next image.  Ignored by this class. */
    struct Image *NextImage;
};
#endif

struct localObjData
{
    /* Font to use */
    struct TextFont *lod_Font;

    /* The key that is underlined */
    UWORD lod_Key;

    /* DrawMode */
    UBYTE lod_Mode;
};

Class *initTextIClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID,
			SUPERCLASSID, NULL,
			sizeof(struct localObjData), 0))
    {
	/* Fill in the callback hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchTextI;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;
	cl->cl_UserData = 0xFEEDFACE;

	/* Make the class public */
	AddClass (cl);
    }

    /* Return a pointer to the class */
    return (cl);
}

ULONG freeTextIClass (Class * cl)
{

    /* Try to free the public class */
    return ((ULONG)FreeClass (cl));
}

ULONG __saveds dispatchTextI (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod;
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have our superclass create it */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		struct DrawInfo *drinfo;

		/* Get the DrawInfo */
		drinfo = (struct DrawInfo *) GetTagData (SYSIA_DrawInfo, NULL, attrs);

		/* Get the instance data */
		lod = INST_DATA(cl, newobj);

		/* Establish defaults */
		IM(newobj)->PlanePick = 1;
		lod->lod_Mode = JAM1;

		/* Set the attributes */
		setTextIAttrs(cl, newobj, (struct opSet *) msg);

		/* Get the bounding rectangle of the label */
		getContentsExtent (cl, newobj, drinfo);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getTextIAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DSM (cl, o, msg);

	    /* Call our set routines */
	    retval += setTextIAttrs (cl, o, (struct opSet *) msg);
	    break;

	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw */
	    retval = drawTextI (cl, o, (struct impDraw *) msg);
	    break;

	case IM_FRAMEBOX:
	    retval = frameTextI (cl, o, (struct impFrameBox *) msg);
	    break;

	/* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG frameTextI (Class *cl, Object *o, struct impFrameBox *msg)
{
    struct IBox *ob;

    /* Get the bounding rectangle of the label */
    getContentsExtent (cl, o, msg->imp_DrInfo);

    /* Make sure we have a destination box */
    if (ob = msg->imp_ContentsBox)
    {
	ob->Width = IM(o)->Width;
	ob->Height = IM(o)->Height;
    }

    return (1L);
}

/* Set attributes of an object */
ULONG setTextIAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case IA_FGPen:
		IM(o)->PlanePick = (UBYTE) tidata;
		break;

	    case IA_BGPen:
		IM(o)->PlaneOnOff = (UBYTE) tidata;
		break;

	    /* Must be a TextFont pointer. */
	    case IA_Font:
		/* Set the font */
		lod->lod_Font = (struct TextFont *) tidata;
		break;

	    /* Drawing mode to use */
	    case IA_Mode:
		lod->lod_Mode = (UBYTE) tidata;
		break;

	    case IA_Data:
		IM(o)->ImageData = (USHORT *) tidata;
		lod->lod_Key = GetLabelKeystroke ((STRPTR) tidata);
		break;
	}
    }

    return (1L);
}

/* Inquire attributes of an object */
ULONG getTextIAttrs (Class * cl, Object * o, struct opGet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case IA_Font:
	    *msg->opg_Storage = (ULONG) lod->lod_Font;
	    break;

	case IA_Mode:
	    *msg->opg_Storage = (ULONG) lod->lod_Mode;
	    break;

	/* Let the superclass try */
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }

    return (1L);
}

ULONG drawTextI (Class *cl, Object *o, struct impDraw *msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    STRPTR label = (STRPTR) IM(o)->ImageData;
    struct DrawInfo *di = msg->imp_DrInfo;
    struct RastPort *rp = msg->imp_RPort;
    WORD len = strlen (label);
    struct TextFont *tf;
    WORD left, top;
    WORD i;

    /* Clear the key */
    lod->lod_Key = NULL;

    /* Set the font */
    tf = (lod->lod_Font) ? lod->lod_Font : di->dri_Font;
    SetFont (rp, tf);

    /* Figure out our coordinates */
    top = msg->imp_Offset.Y + IM (o)->TopEdge + rp->TxBaseline;
    left = msg->imp_Offset.X + IM (o)->LeftEdge;

    /* See if we have frame information */
    if (msg->MethodID == IM_DRAWFRAME)
    {
	top += ((msg->imp_Dimensions.Height - IM(o)->Height) > 0) ?  ((msg->imp_Dimensions.Height - IM(o)->Height) / 2) : 0;
	left += ((msg->imp_Dimensions.Width - IM(o)->Width) > 0) ? ((msg->imp_Dimensions.Width - IM(o)->Width) / 2) : 0;
    }

    /* Set the colors */
    SetAPen (rp, IM(o)->PlanePick);
    SetBPen (rp, IM(o)->PlaneOnOff);

    /* Set the drawing mode */
    SetDrMd (rp, lod->lod_Mode);

    /* Move to the start */
    Move (rp, left, top);

    /* Step through string */
    for (i = 0; i < (len - 1); i++)
    {
	/* Is this an '_' sign? */
	if (label[i] == '_')
	{
	    WORD bot = (top + rp->TxHeight - rp->TxBaseline);
	    WORD mark;

	    /* Draw the first part of the string */
	    Text (rp, label, i);

	    /* Remember where we are */
	    mark = rp->cp_x;

	    /* Draw the underscore */
	    Move (rp, mark, bot);
	    Draw (rp, (mark + TextLength (rp, &label[(i + 1)], 1L) - 2), bot);

	    /* Return to where we were */
	    Move (rp, mark, top);

	    /*
	     * Draw the rest of the string.  This one is done last so that the
	     * cursor could be positioned after the text.
	     */
	    Text (rp, &label[(i + 1)], (len - i - 1));

	    /* Return the underlined character */
	    lod->lod_Key = (UWORD) label[i];
	}
    }

    /* Do we have an underscore? */
    if (!lod->lod_Key)
    {
	/* Didn't find an '_' sign */
	Text (rp, label, len);
    }

    return (1L);
}

UWORD GetLabelKeystroke (STRPTR label)
{
    LONG count = (label) ? strlen (label) : 0L;
    LONG i;

    /* Search for an _ sign */
    for (i = 0; i < (count - 1); i++)
    {
	/* Did we find an _ sign? */
	if (label[i] == '_')
	{
	    return ((UWORD)label[(i + 1)]);
	}
    }

    return (0);
}

#if 1
/* TextExtent that honors the '_' as being a non-printable character (once) */
WORD
aTextExtent(struct RastPort * rp, STRPTR string, LONG count, struct TextExtent * te)
{
    WORD retval = FALSE;
    STRPTR buffer;
    LONG i;

    /* Allocate a temporary buffer */
    if (buffer = AllocVec ((count + 1), MEMF_CLEAR))
    {
	/* Step through string */
	for (i = 0; i < count; i++)
	{
            /* Is this an '_' sign? */
            if (string[i] == '_')
            {
		/* Add the rest of the label to the buffer */
		strcat (buffer, &string[(i + 1)]);

		/* Adjust the length of the string. */
		count--;
		break;
	    }
	    else
	    {
		/* Copy each character over, until we reach the _ mark */
		buffer[i] = string[i];
	    }
        }

        /* Get the extent */
        TextExtent(rp, buffer, count, te);

	/* Free the temporary buffer */
	FreeVec (buffer);

	/* Show that we were successful */
	retval = TRUE;
    }

    /* Return whatever textextent returned */
    return (retval);
}

#else
/* TextExtent that honors the '_' as being a non-printable character (once) */
WORD aTextExtent (struct RastPort * rp, STRPTR string, LONG count, struct TextExtent * te)
{
    WORD retval;
    LONG i;

    /* Get the extent */
    retval = TextExtent (rp, string, count, te);

    /* Search for an '_' sign */
    for (i = 0; i < (count - 1); i++)
    {
	/* Did we find an '_' sign? */
	if (string[i] == '_')
	{
	    /* Get the width of the '_' sign */
	    WORD w = TextLength (rp, "_", 1);

	    /* Adjust the extent */
	    te->te_Width -= w;
	    te->te_Extent.MaxX -= w;
	    te->te_Height++;
	    te->te_Extent.MaxY++;
	    break;
	}
    }

    /* Return whatever textextent returned */
    return (retval);
}
#endif

static VOID
getContentsExtent (Class *cl, Object * o, struct DrawInfo * drinfo)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TextExtent te = {NULL};
    struct RastPort rp;
    STRPTR label;

    /* maybe look at some flags to handle other types of text someday */
    if (label = (STRPTR) IM(o)->ImageData)
    {
	/* Initialize the RastPort */
	InitRastPort (&rp);

	if (lod->lod_Font)
	{
	    SetFont (&rp, lod->lod_Font);
	}
	else if (drinfo && drinfo->dri_Font)
	{
	    SetFont (&rp, drinfo->dri_Font);
	}

	/* Get the rectangle for the label */
	aTextExtent (&rp, label, strlen(label), &te);

	/* Set the image structure */
	IM(o)->Width = te.te_Width;
	IM(o)->Height = te.te_Height;
    }
    else
    {
	IM(o)->Width = IM(o)->Height = 0;
    }
}
