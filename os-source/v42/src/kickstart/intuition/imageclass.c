/*** imageclass.c **************************************************************
 *
 * imageclass.c -- image class
 *
 *  $Id: imageclass.c,v 40.0 94/02/15 17:40:33 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"

#include <hardware/blit.h>
#include <utility/pack.h>

/*****************************************************************************/

#define D(x)	;
#define D2(x)	;

/*****************************************************************************/

/* The only reason we get away with this is that we took care
 * that the methods for imageclass don't assume that the
 * image is really an object. */
ULONG sendCompatibleImageMessage (struct Image *image, ULONG msg1, ...)
{
    D (printf ("sCIM\n"));
    if (!image)
	return (0);

    if (image->Depth != CUSTOMIMAGEDEPTH)
    {
	D (printf ("      compatibility mode\n"));
	/* cache imageclass in IBase */
	return (CM (fetchIBase ()->ImageClassPtr, (Object *) image, (Msg) &msg1));
    }
    else
    {
	return (SM ((Object *) image, (Msg) &msg1));
    }
}

/*****************************************************************************/

/* instance data for image class
 *
 * NOTE WELL: this class carefully cheats and does not
 * assume that an image it is passed is really an Object.
 * This lets us use it for compatible, old-style images.
 *
 * So BE CAREFUL. */
struct localData
{
    struct Image lod_Image;
};

#define IM(o)	((struct Image *)(o))

/*****************************************************************************/

ULONG imagePackTable[] =
{
    PACK_STARTTABLE (IA_Dummy),
    PACK_ENTRY (IA_Dummy, IA_LEFT, Image, LeftEdge, PKCTRL_WORD),
    PACK_ENTRY (IA_Dummy, IA_TOP, Image, TopEdge, PKCTRL_WORD),
    PACK_ENTRY (IA_Dummy, IA_WIDTH, Image, Width, PKCTRL_WORD),
    PACK_ENTRY (IA_Dummy, IA_HEIGHT, Image, Height, PKCTRL_WORD),
    PACK_ENTRY (IA_Dummy, IA_FGPEN, Image, PlanePick, PKCTRL_UBYTE),
    PACK_ENTRY (IA_Dummy, IA_BGPEN, Image, PlaneOnOff, PKCTRL_UBYTE),
    PACK_ENTRY (IA_Dummy, IA_DATA, Image, ImageData, PKCTRL_ULONG),
    PACK_ENDTABLE,
};

/*****************************************************************************/

/* Since omSet() is nothing more than a call to PackStructureTags(),
 * it's smaller to take it in-line.  It's clearer to leave it as a macro.
 */
#define omSet( cl, o, msg ) \
    PackStructureTags( IM(o), imagePackTable, ((struct opSet *)msg)->ops_AttrList )

/*****************************************************************************/

/* process old-style images with coolness */
static ULONG imDraw (Object *o, struct impDraw *msg)
{
    extern USHORT BPattern[];
    ULONG retval;

#if 1
    if (msg->MethodID == IM_DRAWFRAME)
    {
	/* try using true class's DRAW method	*/
	msg->MethodID = IM_DRAW;
	retval = SM (o, msg);
	msg->MethodID = IM_DRAWFRAME;
	return (retval);
    }

    /* all states start with a imDraw */
    /* I give up on trying to do XOR for SELECTED and ghosting.
     * If a state is unrecognized by the subclass, then
     * it should be NORMAL.
     * For DISABLED state, this still doesn't help
     * anything to do ghosting here
     */
    drawImageGrunt (msg->imp_RPort, IM (o),
		    msg->imp_Offset.X, msg->imp_Offset.Y, 0x00C0);
#else
    /* forgive my sloppy minterms
     * I believe that one of them is copy, one is XOR
     */
    minterm = (msg->imp_State == IDS_SELECTED) ? (ANBC | ABNC) : 0x00C0;

    /* all states start with a imDraw */
    drawImageGrunt (msg->imp_RPort, IM (o),
		    msg->imp_Offset.X, msg->imp_Offset.Y, minterm);

    switch (msg->imp_State)
    {
	case IDS_NORMAL:
	    /* it's OK */
	    break;
	case IDS_SELECTED:
	    /* already OK, by using minterm */
	    break;
	case IDS_DISABLED:
	    /* need to ghost stuff */
	    /* ZZZ: needed to pick a pen to ghost with.
	 * elected to use planeonoff for now
	 */
	    b = (struct IBox *) &IM (o)->LeftEdge;
	    BlastPattern (msg->imp_RPort, b->Left, b->Top,
			  b->Left + b->Width - 1, b->Top + b->Height - 1,
			  BPattern, 1, IM (o)->PlaneOnOff, 0, JAM1);
	    break;
	case IDS_BUSY:
	case IDS_INDETERMINANT:
	    /* no op?  same as NORMAL */
	    break;
    }
#endif
    return 0;
}

/*****************************************************************************/

static ULONG imErase (Object *o, struct impErase *msg)
{
    ULONG width, height;
    ULONG left, top;

    width = IM (o)->Width;
    height = IM (o)->Height;

    if (msg->MethodID == IM_ERASEFRAME)
    {
	width = msg->imp_Dimensions.Width;
	height = msg->imp_Dimensions.Height;
    }

    /* jimm: 4/16/90 add in offset like I should have	*/
    left = IM (o)->LeftEdge + msg->imp_Offset.X;
    top = IM (o)->TopEdge + msg->imp_Offset.Y;

    /* ZZZ: should probably check validity */
    EraseRect (msg->imp_RPort, left, top, left + width - 1, top + height - 1);

    return 0;
}

/*****************************************************************************/

/* you could also have HITFRAME get run as HITTEST,
 * akin to DRAWFRAME, but this is easier for everyone. */
static ULONG imHitTest (Object *o, struct impHitTest *msg)
{
    struct IBox box;

    box = *IM_BOX (IM (o));

    if (msg->MethodID == IM_HITFRAME)
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

    return (ULONG)(ptInBox (msg->imp_Point, &box));
}

/*****************************************************************************/

static ULONG omGet (Class *cl, Object *o, struct opGet *msg)
{
    struct TagItem gettags[2];

#if 0
    /* NO NEED TO DO THIS, since rootclass has no OM_GETtable things */
    if (IM (o)->Depth == CUSTOMIMAGEDEPTH)
    {
	retval = SSM (cl, o, msg);
    }
    /* Now we override with class-specific get's */
#endif

    gettags[0].ti_Tag = msg->opg_AttrID;
    gettags[0].ti_Data = (ULONG) msg->opg_Storage;
    gettags[1].ti_Tag = TAG_DONE;

    return (UnpackStructureTags (IM (o), imagePackTable, gettags));
}

/*****************************************************************************/

/* ********** INTUITION ENTRY POINTS *********/
void DrawImage (struct RastPort *rport, struct Image *image, LONG xoffset, LONG yoffset)
{
    D (printf ("DI\n"));
    DrawImageState (rport, image, xoffset, yoffset, IDS_NORMAL, NULL);
}

/*****************************************************************************/

/* pack short point components into a longword */
#define POINT(x,y)	(((y) & 0xffff) | (x<<16))

/*****************************************************************************/

/*** intuition.library/DrawImageState ***/
void DrawImageState (struct RastPort *rport, struct Image *image, LONG xoffset, LONG yoffset, ULONG state, struct DrawInfo *drinfo)
{
    ULONG offset = POINT (xoffset, yoffset);
    struct RastPort clonerp;

    D (printf ("DIS\n"));

    cloneRP (&clonerp, rport);
    while (image)
    {
	sendCompatibleImageMessage (image, IM_DRAW, &clonerp, offset, state, drinfo);
	image = image->NextImage;
    }
}

/*****************************************************************************/

/*** intuition.library/PointInImage ***/
BOOL PointInImage (ULONG point, struct Image *image)
{
    /* assuming struct Point passes as a long */
    if (!image)
	return (TRUE);
    return (BOOL) sendCompatibleImageMessage (image, IM_HITTEST, point);
}

/*****************************************************************************/

/*** intuition.library/EraseImage ***/
void EraseImage (struct RastPort *rport, struct Image *image, LONG xoffset, LONG yoffset)
{
    ULONG offset = POINT (xoffset, yoffset);
    struct RastPort clonerp;

    cloneRP (&clonerp, rport);
    while (image)
    {
	sendCompatibleImageMessage (image, IM_ERASE, &clonerp, offset);
	image = image->NextImage;
    }
}

/*****************************************************************************/

static ULONG Dispatcher (Class *cl, Object *o, Msg msg)
{
    Object *newobj;

    D (printf ("Dispatcher, class %lx, object %lx, msg %lx\n", cl, o, msg));

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* have "extra size" parameter */
	    if (newobj = (Object *) SSM (cl, o, msg))
	    {
		/* this is really an object */
		o = (Object *) & ((struct localData *) INST_DATA (cl, newobj))->lod_Image;

		/* set 'o' to point to new object, so I don't get confused	*/

		/* mandatory init */
		IM (o)->Depth = CUSTOMIMAGEDEPTH;
		IM (o)->Width = 80;	/* random numbers */
		IM (o)->Height = 40;

		omSet (cl, newobj, msg);
	    }

	    return ((ULONG) newobj);

	case OM_SET:
#if 0
	    /* pass first to superclass,
	 * but ONLY if this is really an image *object*
	 *
	 * NO NEED TO DO THIS, since rootclass has no OM_SETtable things
	 */
	    if (IM (o)->Depth == CUSTOMIMAGEDEPTH)
		SSM (cl, o, msg);
#endif
	    omSet (cl, o, (struct opSet *) msg);
	    return ((ULONG) 1);

	case OM_GET:
	    return omGet (cl, o, (struct opGet *) msg);

	    /* image class methods.
     * this class mimics old style struct Image
     */
	case IM_DRAW:		/* draw with state 	*/
	case IM_DRAWFRAME:	/* ignore frame dims	*/
	    return imDraw (o, (struct impDraw *) msg);

	case IM_ERASE:
	case IM_ERASEFRAME:
	    return imErase (o, (struct impErase *) msg);

	case IM_HITFRAME:
	case IM_HITTEST:
	    return imHitTest (o, (struct impHitTest *) msg);

#if 0
	case OM_DISPOSE:
#endif
	default:
	    return (SSM (cl, o, msg));
    }
}

/*****************************************************************************/

Class *initImageClass (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    extern UBYTE *ImageClassName;
    extern UBYTE *RootClassName;

    D (printf ("initImageClass\n"));
#if 1
    /* cache a pointer to this public class where Intution can get it quickly.
     * I won't let it go away ever. */
    return (IBase->ImageClassPtr = makePublicClass (ImageClassName, RootClassName,
						    sizeof (struct localData), Dispatcher));
#else
    return (makePublicClass (ImageClassName, RootClassName, sizeof (struct localData), Dispatcher));
#endif
}
