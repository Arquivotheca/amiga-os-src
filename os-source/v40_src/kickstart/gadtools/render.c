/*** render.c *************************************************************
*
*   render.c	- Graphics routines for Gadget Toolkit
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: render.c,v 39.6 93/02/18 18:27:49 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Public: */
void __asm
LIB_DrawBevelBoxA (register __a0 struct RastPort *rport,
		  register __d0 WORD left,
		  register __d1 WORD top,
		  register __d2 WORD width,
		  register __d3 WORD height,
		  register __a1 struct TagItem *);

Class *initGTButtonIClass (void);
ULONG __asm
dispatchGTButtonI ( register __a0 Class *cl,
		     register __a2 Object *o,
		     register __a1 Msg msg );
void setGTButtonIAttrs ( Class *cl, Object *obj, Msg msg);
static VOID drawGTButtonI ( Class *cl, Object *o, struct impDraw *msg);

struct Border *MakeCycleBorder (struct CycleBorder *cycleborder,
    LONG height, struct VisualInfo *vi);

/*------------------------------------------------------------------------*/

STRPTR FrameIClassName = "frameiclass";

/*------------------------------------------------------------------------*/

#define IM(o)	((struct Image *)(o))	/* transparent base class */

/*------------------------------------------------------------------------*/

struct Image *getBevelImage( LONG left, LONG top, LONG width, LONG height, ULONG type )
{
    return((struct Image *)NewObject(NULL, FrameIClassName,
                                     IA_Left, left,
                                     IA_Top, top,
                                     IA_Width, width,
                                     IA_Height, height,
                                     IA_FrameType, type & FRAMETYPE_MASK,
                                     IA_Recessed, type & FRAMETYPE_RECESSED,
                                     IA_EdgesOnly, TRUE,
                                     TAG_DONE ) );
}

/*------------------------------------------------------------------------*/


struct Border *MakeCycleBorder(struct CycleBorder *cycleborder,
                               LONG gadheight, struct VisualInfo *vi )
{
    WORD *p;
    LONG height;

    /* Fill out the the border structure: */

    /* glyph height is always at least 9 */
    height = max(gadheight-5, 9);
    cycleborder->cb_Border.BackPen = DESIGNTEXT;
    cycleborder->cb_Border.LeftEdge = LEFTTRIM+2;
    cycleborder->cb_Border.TopEdge = (gadheight-height)/2;
    /* LeftEdge, TopEdge, DrawMode are already zero */
    cycleborder->cb_Border.Count = CB_COUNT;
    cycleborder->cb_Border.XY = cycleborder->cb_Points;

    /* Fill out the border points, based on the four coordinates: */
    p = cycleborder->cb_Points;

    *p++ = 7;		*p++ = 0;
    *p++ = 7;		*p++ = 5;
    *p++ = 5;		*p++ = 3;
    *p++ = 10;		*p++ = 3;
    *p++ = 8;		*p++ = 5;
    *p++ = 8;		*p++ = 1;
    *p++ = 7;		*p++ = 0;
    *p++ = 1;		*p++ = 0;
    *p++ = 0;		*p++ = 1;
    *p++ = 0;		*p++ = height-2;
    *p++ = 1;		*p++ = height-1;
    *p++ = 1;		*p++ = 1;
    *p++ = 1;		*p++ = height-1;
    *p++ = 7;		*p++ = height-1;
    *p++ = 7;		*p++ = height-2;
/** NOTE WELL:  The last post-increment is deliberately missing (optimization) **/
    *p++ = 8;		*p = height-2;

    return((struct Border *)cycleborder);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/DrawBevelBoxA ***************************************
*
*   NAME
*	DrawBevelBoxA -- draw a bevelled box. (V36)
*	DrawBevelBox -- varargs stub for DrawBevelBoxA(). (V36)
*
*   SYNOPSIS
*	DrawBevelBoxA(rport, left, top, width, height, tagList);
*	              A0     D0    D1   D2     D3      A1
*
*	VOID DrawBevelBoxA(struct RastPort *, WORD, WORD, WORD, WORD,
*	                   struct TagItem *taglist);
*
*	DrawBevelBox(rport, left, top, width, height, firsttag, ...);
*
*	VOID DrawBevelBox(struct RastPort *, WORD, WORD, WORD, WORD,
*	                  Tag, ...);
*
*   FUNCTION
*	This function renders a bevelled box of specified dimensions
*	and type into the supplied RastPort.
*
*   INPUTS
*	rport - RastPort into which the box is to be drawn.
*	left - left edge of the box.
*	top - top edge of the box.
*	width - width of the box.
*	height - height of the box.
*	tagList - pointer to an array of tags providing extra parameters
*
*   TAGS
*	GTBB_Recessed (BOOL) - Set to anything for a recessed-looking box.
*			If absent, the box defaults, it would be raised. (V36)
*
*	GTBB_FrameType (ULONG) - Determines what kind of box this function
*			renders. BBFT_BUTTON generates a box like what is
*			used around GadTools BUTTON_KIND gadgets. BBFT_RIDGE
*			generates a box like what is used around GadTools
*			STRING_KIND gadgets. Finally, BBFT_ICONDROPBOX
*			generates a box suitable for a standard icon drop
*			box imagery. (defaults to BBFT_BUTTON). (V39)
*
*	GT_VisualInfo (APTR) - You MUST supply the value you obtained
*			from an earlier call to GetVisualInfoA() with this
*			tag. (V36)
*
*   NOTES
*	DrawBevelBox() is a rendering operation, not a gadget. That
*	means you must refresh it at the appropriate time, like any
*	other rendering operation.
*
*   SEE ALSO
*	GetVisualInfoA(), <libraries/gadtools.h>
*
******************************************************************************
*
* Created:  10-Oct-89, Peter Cherna
* Modified: 14-Feb-90, Peter Cherna
*
*/

void __asm
LIB_DrawBevelBoxA( register __a0 struct RastPort *rport,
		   register __d0 WORD left,
		   register __d1 WORD top,
		   register __d2 WORD width,
		   register __d3 WORD height,
		   register __a1 struct TagItem *taglist )
{
ULONG              frameType;
struct VisualInfo *vi;
struct Image      *bevelimage;

    /* Get tough and require GT_VisualInfo or else don't draw anything */
    if (vi = (struct VisualInfo *)getGTTagData(GT_VisualInfo, NULL, taglist))
    {
        frameType = getGTTagData(GTBB_FrameType,BBFT_BUTTON,taglist);

	if (findGTTagItem(GTBB_Recessed,taglist))
	    frameType |= FRAMETYPE_RECESSED;

	if (bevelimage = getBevelImage(0,0,width,height,frameType))
	{
	    DrawImageState(rport,bevelimage,left,top,IDS_NORMAL,vi->vi_DrawInfo);
	    DisposeObject(bevelimage);
	}
    }
}

/*------------------------------------------------------------------------*/

struct LocalObjectData
{
    struct IntuiText *lod_IntuiText;
};

/*------------------------------------------------------------------------*/

Class *initGTButtonIClass( void )
{
Class *cl;

    if (cl = MakeClass(NULL,FrameIClassName,NULL,sizeof(struct LocalObjectData),0))
    {
	cl->cl_Dispatcher.h_SubEntry = dispatchGTButtonI;
	cl->cl_Dispatcher.h_Entry    = callCHook;
	cl->cl_Dispatcher.h_Data     = GadToolsBase;
    }
    return(cl);
}


/*------------------------------------------------------------------------*/

ULONG ASM dispatchGTButtonI(REG(a0) Class *cl,
		            REG(a2) Object *o,
		            REG(a1) Msg msg)
{
ULONG   result = 1;
Object *newobj;

    switch (msg->MethodID)
    {
	case IM_DRAW     :
	case IM_DRAWFRAME: SSM( cl, o, msg );
                           drawGTButtonI( cl, o, (struct impDraw *)msg );
                           break;

	case OM_SET      : SSM( cl, o, msg );
                           setGTButtonIAttrs( cl, o, msg );
                           break;

	case OM_NEW      : if (result = (ULONG)(newobj = (Object *) SSM( cl, o, msg )) )
                               setGTButtonIAttrs( cl, newobj, msg );
                           break;

	default          : result = (ULONG) SSM( cl, o, msg );
    }

    return(result);
}


/*------------------------------------------------------------------------*/

VOID setGTButtonIAttrs(Class *cl, Object *obj, Msg msg)
{
struct LocalObjectData *idata = INST_DATA( cl, obj );
struct TagItem         *taglist = ((struct opSet *)msg)->ops_AttrList;

    idata->lod_IntuiText = (struct IntuiText *)getTagData( MYIA_IText,(ULONG)idata->lod_IntuiText,taglist);
}

/*------------------------------------------------------------------------*/

static VOID drawGTButtonI(Class *cl, Object *o, struct impDraw *msg)
{
struct IBox             box;
BOOL                    selected;
UBYTE                   penText = 1;
UBYTE                   penShine = 2;
UBYTE                   penShadow = 1;
UBYTE                   penFillText = 1;
struct DrawInfo        *di;
struct LocalObjectData *idata = INST_DATA(cl,o);

    /* Only use DrawInfo if present */
    if ( di = msg->imp_DrInfo)
    {
	penText     = di->dri_Pens[TEXTPEN];
	penShine    = di->dri_Pens[SHINEPEN];
	penShadow   = di->dri_Pens[SHADOWPEN];
	penFillText = di->dri_Pens[FILLTEXTPEN];
    }

    /* get Image.Left/Top/Width/Height */
    box       = *IM_BOX( IM(o) );
    box.Left += msg->imp_Offset.X;
    box.Top  += msg->imp_Offset.Y;

    selected = ((msg->imp_State == IDS_SELECTED) || (msg->imp_State == IDS_INACTIVESELECTED));

    {
	/* This is ugly but efficient:  for each border on the list,
	 * the (unused) BackPen field will be DESIGNSHINE or DESIGNSHADOW
	 * or 0, indicating that this border was intended to be drawn
	 * with shine or shadow or something else.
	 * We key off this and set the FrontPen to SHINEPEN or SHADOWPEN
	 * or leave it alone, respectively.  If the gadget is selected,
	 * we invert the assignment of SHINEPEN and SHADOWPEN.
	 * The actual pen values of SHINEPEN and SHADOWPEN have been
	 * placed in the image->PlanePick and PlaneOnOff using
	 * IA_FGPen and IA_BGPen respectively.
	 */
	UBYTE designShine;
	UBYTE designShadow;
	UBYTE designText;
	struct Border *border;

	border = (struct Border *)IM(o)->ImageData;
	designShine = penShine;
	designShadow = penShadow;
	designText = penText;
	if (selected)
	{
	    /* Since we're going to change designText anyways, we can use
	     * it as a temp-variable
	     */
	    designText = designShine;
	    designShine = designShadow;
	    designShadow = designText;
	    designText = penFillText;
	}

	while (border)
	{
	    DP(("dBCI: coloring border at $%lx\n", border));
	    if (border->BackPen == DESIGNSHINE)
	    {
		border->FrontPen = designShine;
	    }
	    else if (border->BackPen == DESIGNSHADOW)
	    {
		border->FrontPen = designShadow;
	    }
	    else if (border->BackPen == DESIGNTEXT)
	    {
		border->FrontPen = designText;
	    }
	    /* else leave FrontPen alone */
	    border = border->NextBorder;
	}
    }

    if (idata->lod_IntuiText)
    {
	struct IntuiText *itext;

	DP(("dBCI: Printing Text '%s'\n", idata->lod_IntuiText->IText));
	for (itext = idata->lod_IntuiText; itext; itext = itext->NextText)
	{
	    itext->FrontPen = penText;
	    if ( selected )
	    {
		itext->FrontPen = penFillText;
	    }
	}
	PrintIText(msg->imp_RPort, idata->lod_IntuiText, box.Left, box.Top);
    }

    /* Then draw the border on top */
    if ( IM(o)->ImageData )
    {
	DrawBorder(msg->imp_RPort, ((struct Border *)IM(o)->ImageData),
	           box.Left, box.Top);
    }
}
