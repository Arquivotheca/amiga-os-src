/* ======================================================================== *
 *  Arrows.c -- a BOOPSI class implementing increment / decrement arrows
 *                             By Talin.
 *              Copyright © 1991 Sylvan Technical Arts
 * ======================================================================== */

#include "std_headers.h"
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/classes.h>
#include <intuition/imageclass.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>
#include <clib/alib_protos.h>
#include <math.h>

#include "arrows.h"

extern struct Library	*IntuitionBase,
						*UtilityBase,
						*GadToolsBase,
						*GfxBase;

#define INCREMENT		GACT_ALTKEYMAP			/* overload this gadget flag	*/

#define clamp(a,b,c)	min( max( a, b ), c )	/* constrain b between a and c	*/

struct ArrowButton {
	ULONG				min,					/* minimum value				*/
						max,					/* maximum value				*/
						current;				/* current internal value		*/
	UWORD				rate,					/* rate of increment			*/
						rate_increase,			/* % speed increase per tick	*/
						rate_max;				/* maximum speed				*/
};

void __asm AsymBevel(	register __a0 struct RastPort *rp,
						register __a1 struct IBox *b,
						register __d0 LONG ulpen,
						register __d1 LONG lrpen,
						register __d2 LONG fillpen,
						register __d3 WORD side );

void SetupGadgetIBox( struct Gadget *gadget, struct IBox *domain, struct IBox *box )
{
	box->Left = gadget->LeftEdge;
	box->Top = gadget->TopEdge;
	box->Width = gadget->Width;
	box->Height = gadget->Height;

	if (gadget->Flags & GRELRIGHT)	box->Left += domain->Width;
	if (gadget->Flags & GRELBOTTOM) box->Top += domain->Height;

	if (gadget->Flags & GRELWIDTH)  box->Width += domain->Width;
	if (gadget->Flags & GRELHEIGHT) box->Height += domain->Height;
}

	/*  GM_RENDER method handler  */

static void arrowdraw(Class *cl, struct gpRender *msg,struct Gadget *g)
{
	if (msg->gpr_Redraw == GREDRAW_REDRAW || msg->gpr_Redraw == GREDRAW_UPDATE)
	{	struct IBox			left_box,
							right_box;
		struct GadgetInfo	*gi = msg->gpr_GInfo;
		struct RastPort		*rp = msg->gpr_RPort;
		UWORD				*pens = gi->gi_DrInfo->dri_Pens;
		BOOL				dec_selected = 0,
							inc_selected = 0;
		WORD				symbol_diag_w,
							symbol_diag_h;
		WORD				x_center,
							y_center;
		WORD				symbol_height,
							symbol_y1,
							symbol_y2,
							symbol_y3,
							symbol_y4;
		WORD				sx1,
							sx2;

		SetupGadgetIBox( g, &gi->gi_Domain, &right_box);

		if (g->Flags & GFLG_SELECTED)
		{	if (g->Activation & INCREMENT) inc_selected = TRUE;
			else dec_selected = TRUE;
		}

		left_box = right_box;
		left_box.Width = right_box.Width / 2;
		right_box.Width = right_box.Width - left_box.Width;
		right_box.Left += left_box.Width;

		symbol_diag_w = MIN( left_box.Width - 5, 7);
		symbol_diag_w = MIN( symbol_diag_w, left_box.Height / 3 );
		symbol_diag_h = clamp(3, (left_box.Height - 3) / 2, symbol_diag_w );

		symbol_height = symbol_diag_h + symbol_diag_h - (right_box.Height & 1);

		x_center = (left_box.Width - symbol_diag_w) / 2;
		y_center = (right_box.Height - symbol_height) / 2;

			/* Draw 1st arrow button */

		symbol_height--;
		symbol_diag_h--;
		symbol_diag_w--;
		symbol_y1 = left_box.Top + y_center;
		symbol_y2 = symbol_y1 + symbol_diag_h;
		symbol_y4 = symbol_y1 + symbol_height - symbol_diag_h;
		symbol_y3 = symbol_y1 + symbol_height;

		AsymBevel(	rp, &left_box,
					pens[ dec_selected ? SHADOWPEN : SHINEPEN ],
					pens[ dec_selected ? SHINEPEN : SHADOWPEN ],
					pens[ dec_selected ? FILLPEN : BACKGROUNDPEN ],
					0 );

			/* draw inner part of right-hand box */

		SetAPen(rp,pens[ TEXTPEN ]);

		sx1 = left_box.Left + 1 + x_center;
		sx2 = sx1 + symbol_diag_w;

		Move( rp, sx1, symbol_y2 );
		Draw( rp, sx2, symbol_y1 );
		Move( rp, sx1, symbol_y4 );
		Draw( rp, sx2, symbol_y3 );

		Move( rp, sx1 + 1, symbol_y2 );
		Draw( rp, sx2, symbol_y1 + 1 );
		Move( rp, sx1 + 1, symbol_y4 );
		Draw( rp, sx2, symbol_y3 - 1 );

		AsymBevel(	rp, &right_box,
					pens[ inc_selected ? SHADOWPEN : SHINEPEN ],
					pens[ inc_selected ? SHINEPEN : SHADOWPEN ],
					pens[ inc_selected ? FILLPEN : BACKGROUNDPEN ],
					1 );

			/* draw inner part of right-hand box */

		SetAPen(rp,pens[ TEXTPEN ]);

		sx1 = right_box.Left + x_center;
		sx2 = sx1 + symbol_diag_w;

		Move( rp, sx2, symbol_y2 );
		Draw( rp, sx1, symbol_y1 );
		Move( rp, sx2, symbol_y4 );
		Draw( rp, sx1, symbol_y3 );

		Move( rp, sx2 - 1, symbol_y2 );
		Draw( rp, sx1, symbol_y1 + 1 );
		Move( rp, sx2 - 1, symbol_y4 );
		Draw( rp, sx1, symbol_y3 - 1 );
	}
}

static int TestHit(struct gpInput *msg,struct Gadget *g)
{	struct IBox         gbox;
	short               x = msg->gpi_Mouse.X,
                        y = msg->gpi_Mouse.Y;

	SetupGadgetIBox(g,&msg->gpi_GInfo->gi_Domain,&gbox);

	if (x >= 0 && y >= 0 && x < gbox.Width && y < gbox.Height)
		return (x > gbox.Width / 2) ? ARROWSTATE_INC : ARROWSTATE_DEC;

	return 0;
}

	/*  GM_HANDLEINPUT method handler  */

static long arrowhandle(Class *cl, struct gpInput *msg,struct Gadget *g)
{
	struct RastPort	*rp;
	long			result = GMR_MEACTIVE;
	BOOL			render = FALSE,				/* TRUE if we need to re-render	*/
					update = FALSE,				/* TRUE if we need to re-calc	*/
					notify = FALSE;				/* TRUE if we need to broadcast	*/
	WORD			verify = g->Activation & GACT_RELVERIFY ? GMR_VERIFY : 0;
	struct ArrowButton 	*ab = (struct ArrowButton *)INST_DATA( cl, g );
	static			timer_delay;

		/* Handle input messages */

    if (msg->MethodID == GM_GOACTIVE)		/* classify type of hit         */
	{
		if (msg->gpi_IEvent == NULL) return GMR_NOREUSE;
		ab->rate = 100;

		g->Flags |= GFLG_SELECTED;
		if (TestHit(msg,g) == ARROWSTATE_INC) g->Activation |= INCREMENT;
		else g->Activation &= ~INCREMENT;

		timer_delay = 2;

		render = notify = update = TRUE;
	}

	if (msg->gpi_IEvent->ie_Class == IECLASS_RAWMOUSE)
	{
		if (msg->gpi_IEvent->ie_Code == (UBYTE)(IECODE_LBUTTON+IECODE_UP_PREFIX))
		{
			g->Flags &= ~GFLG_SELECTED;
			result = GMR_NOREUSE | verify;
			render = TRUE;
		}
		else if (msg->gpi_IEvent->ie_Code == (UBYTE)IECODE_RBUTTON)
		{
				/* check for menu button hit */

			g->Flags &= ~GFLG_SELECTED;
            result = GMR_REUSE | verify;
            render = TRUE;
        }
        else
        {	WORD		oldflags = g->Flags,
        				hit = TestHit(msg,g),
        				current_state;

        	current_state = (g->Activation & INCREMENT) ? ARROWSTATE_INC : ARROWSTATE_DEC;

        	if (current_state == hit) g->Flags |= GFLG_SELECTED;
        	else g->Flags &= ~GFLG_SELECTED;

			if (g->Flags != oldflags) render = TRUE;
        }
	}
	else if (msg->gpi_IEvent->ie_Class == IECLASS_TIMER)
	{
		if (timer_delay > 0) timer_delay--;
		else
		{
			update = TRUE;
			ab->rate = ab->rate * ab->rate_increase / 100;
			if (ab->rate > ab->rate_max) ab->rate = ab->rate_max;
		}
	}

		/* increment or decrement internal value */

	if (update)
	{	LONG			rate = ab->rate / 100;
		LONG			newval;

		if (rate < 1) rate = 1;
		if (!(g->Activation & INCREMENT)) rate = -rate;
		newval = clamp(ab->min,ab->current + rate,ab->max);
		if (newval != ab->current)
		{	ab->current = newval;
			notify = TRUE;
		}
	}

		/* re-render the gadget */

	if (render && (rp = ObtainGIRPort(msg->gpi_GInfo)))
	{
		DoMethod((Object *)g,GM_RENDER,msg->gpi_GInfo,rp,GREDRAW_UPDATE);
		ReleaseGIRPort(rp);
	}

		/* notify others that we changed */

	if (notify)
	{	struct TagItem	output_tags[ 4 ],
						*tagout = output_tags;
		WORD			flags = OPUF_INTERIM;

			/* build a conditional tag list */

		tagout->ti_Tag  = GA_ID;
		tagout->ti_Data = g->GadgetID;
		tagout++;

		tagout->ti_Tag  = ARROW_Current;
		tagout->ti_Data = ab->current;
		tagout++;

		if (result & GMR_VERIFY)
		{	tagout->ti_Tag  = ARROW_State;
			tagout->ti_Data = 0;
			tagout++;
			flags = 0;
		}
 		else if (msg->MethodID == GM_GOACTIVE)
		{	tagout->ti_Tag  = ARROW_State;
			tagout->ti_Data = g->Activation & INCREMENT ? ARROWSTATE_INC : ARROWSTATE_DEC;
			tagout++;
		}

		tagout->ti_Tag = TAG_DONE;

			/* send notification */

		DoMethod( (Object *)g, OM_NOTIFY, output_tags, msg->gpi_GInfo, flags );
	}

	return result;
}

/* ============================================================================ *
 *	set ArrowButton attributes
 * ============================================================================ */

static int setArrowButtonAttrs( Class *cl, Object *o, struct opSet *msg )
{	struct TagItem		*tags = msg->ops_AttrList,
						*tlist = tags;
	struct ArrowButton	*ab = INST_DATA( cl, o );
	ULONG				tagval;

		/* iterate through the tags and setup what values are needed */

	while (tags = NextTagItem(&tlist))
	{
		tagval = tags->ti_Data;
		switch (tags->ti_Tag) {

		case ARROW_Min:				ab->min = tagval; break;
		case ARROW_Max:				ab->max = tagval; break;
		case ARROW_Current:			ab->current = tagval; break;
		case ARROW_IncreaseRate:	ab->rate_increase = tagval + 100; break;
		case ARROW_MaxRate:     	ab->rate_max = tagval * 100; break;
		}
	}
	ab->current = clamp(ab->min,ab->current,ab->max);
	return 0;
}

/* ============================================================================ *
 *	dispatch function for the ArrowButton Class
 * ============================================================================ */

static long __asm __saveds ourhook (
	register __a0 Class *cl,
	register __a1 struct gpInput *msg,
	register __a2 struct Gadget *g,
	register __a6 struct Library *IntuitionBase)
{	long				refresh = 0;
	struct ArrowButton	*ab;
	static struct ArrowButton init_ab = { 0,100,0,100,10000 };

	switch ( msg->MethodID ) {

	case GM_RENDER:
		arrowdraw(cl, (struct gpRender *)msg, g);
		break;

	case GM_HITTEST:
		return GMR_GADGETHIT;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
		return arrowhandle(cl,msg,g);

	case GM_GOINACTIVE:
		break;

	case OM_NEW:
		g = (struct Gadget *)DoSuperMethodA( cl, (Object *)g, (Msg)msg);
		if (g == NULL) return NULL;

		ab = INST_DATA( cl, g );
		*ab = init_ab;
		setArrowButtonAttrs( cl, (Object *)g, (struct opSet *)msg );

		return (long)g;

	case OM_SET:
		DoSuperMethodA( cl, (Object *)g, (Msg)msg );	/* set attibutes		*/
	    refresh |= setArrowButtonAttrs( cl, (Object *)g, (struct opSet *)msg );
	    return 0;

	default:
		return (LONG) DoSuperMethodA( cl, (Object *)g, (Msg)msg );
	}
	return 0;
}

/* ============================================================================ *
 *	initArrowButtonClass: create data used by all Arrow Buttons
 * ============================================================================ */

Class *initArrowButtonClass(void)
{	Class *cl;

	if (cl = MakeClass(							/* Ask Intuition to make it		*/
		NULL, 									/* Class ID	(none)				*/
		"gadgetclass", NULL,					/* superclass is public, tho...	*/
		sizeof (struct ArrowButton),			/* Size of instance data		*/
		0 ))									/* No flags...					*/
	{
		cl->cl_Dispatcher.h_Entry =	(void *)ourhook;
		cl->cl_Dispatcher.h_SubEntry = NULL;
		cl->cl_Dispatcher.h_Data = 0L;
	}

	return cl;
}

LONG freeArrowButtonClass(Class *cl)
{
	return (LONG)FreeClass(cl);
}
