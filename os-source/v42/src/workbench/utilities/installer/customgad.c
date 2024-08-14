/*
 *		customgad.c
 *
 *	This module formats some custom gadget images.  Note that this is only
 *	used in the 2.0+ version of Installer.  For the universal Installer
 *	this entire module is dropped by the conditional compilation stuff.
 *
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */


/* ========================================================================= *
 * CustomGad.c - custom gadgets for installer buttons                        *
 * By Talin & Joe Pearce. (c) 1991 Sylvan Technical Arts                     *
 * ========================================================================= */

#ifdef ONLY2_0

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>

#include "functions.h"

#include "xfunctions.h"
#include <string.h>

#include "installer.h"
#include "wild.h"
#include "window.h"

extern struct Library	*GfxBase,
						*IntuitionBase,
						*GadToolsBase;

extern APTR				visinfo;

extern WORD				darkest_color,
						lightest_color,
						text_color,
						highlight_color;

extern struct TextFont	*UFont;

static long __asm __saveds buttonhook(
	register __a0 struct Hook *ihook,
	register __a1 long *msg,
	register __a2 struct Gadget *g);
static long __asm __saveds arrowhook(
	register __a0 struct Hook *ihook,
	register __a1 long *msg,
	register __a2 struct Gadget *g);

struct Hook ButtonHook = { { NULL, NULL }, (ULONG (*)())buttonhook, };
struct Hook ArrowHook  = { { NULL, NULL }, (ULONG (*)())arrowhook, };

static	UWORD	pat[2] = { 0x8888, 0x2222 };

struct TagItem bevel_tags[] = 
{
	GTBB_Recessed,	TRUE,
	GT_VisualInfo,	NULL,
	TAG_DONE,		0
};


void write_text (struct RastPort *rp, struct Gadget *gad, char *text)
{	WORD		text_width,
				text_offset,
				text_length;

	text_length = strlen(text);
	text_width = TextLength(rp,text,text_length);
	text_offset = (gad->Width - text_width) / 2;

	SetAPen(rp,text_color);
	Move(rp,
		gad->LeftEdge + text_offset,
		gad->TopEdge + (gad->Height - rp->TxHeight)/2 +	rp->TxBaseline + 1);
	Text (rp, text, text_length);
}

	/*  Draw innerds of gauge.  */

static void drawbutton(struct RastPort *rp,struct Gadget *g,WORD mode)
{
	char	*str = (char *)g->SpecialInfo;

	bevel_tags[1].ti_Data = (LONG)visinfo;

	SetDrMd(rp,JAM1);
	SetFont(rp,UFont);

	if (mode == 1)
	{
		SetAPen(rp,highlight_color);
		RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
			g->TopEdge+g->Height-1);
		write_text(rp,g,str);
		DrawBevelBoxA(rp,g->LeftEdge,g->TopEdge,g->Width,g->Height,&bevel_tags[0]);
	}
	else
	{
		SetAPen(rp,0);
		RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
			g->TopEdge+g->Height-1);
		write_text(rp,g,str);
		DrawBevelBoxA(rp,g->LeftEdge,g->TopEdge,g->Width,g->Height,&bevel_tags[1]);

		if (mode == 2)
		{
			SetAPen(rp,darkest_color);
			SetAfPt(rp,pat,1);
			RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
				g->TopEdge+g->Height-1);
		}
	}
}

	/*  Draw arrow  */

void renderarrow(struct RastPort *rp, struct Gadget *g)
{
	SetAPen(rp,darkest_color);
	SetDrMd(rp,JAM1);
	BltTemplate((void *)g->SpecialInfo,0,2,rp,g->LeftEdge+7,g->TopEdge+3,10,5);
}

static void drawarrow(struct RastPort *rp,struct Gadget *g,WORD mode)
{
	bevel_tags[1].ti_Data = (LONG)visinfo;

	SetDrMd(rp,JAM1);

	if (mode == 1)
	{
		SetAPen(rp,highlight_color);
		RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
			g->TopEdge+g->Height-1);
		renderarrow(rp, g);
		DrawBevelBoxA(rp,g->LeftEdge,g->TopEdge,g->Width,g->Height,&bevel_tags[0]);
	}
	else
	{
		SetAPen(rp,0);
		RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
			g->TopEdge+g->Height-1);
		renderarrow(rp, g);
		DrawBevelBoxA(rp,g->LeftEdge,g->TopEdge,g->Width,g->Height,&bevel_tags[1]);

		if (mode == 2)
		{
			SetAPen(rp,darkest_color);
			SetAfPt(rp,pat,1);
			RectFill(rp,g->LeftEdge,g->TopEdge,g->LeftEdge+g->Width-1,
				g->TopEdge+g->Height-1);
		}
	}
}

	/*  GM_RENDER method handler  */

static void buttondraw(struct gpRender *msg,struct Gadget *g)
{
	if (msg->gpr_Redraw == GREDRAW_REDRAW)
	{
		drawbutton(msg->gpr_RPort,g,g->Flags & GADGDISABLED ? 2 : 0);
	}
}

	/*  GM_HITTEST method handler  */

static long anyhit(struct gpHitTest *msg,struct Gadget *g)
{
	return GMR_GADGETHIT;
}

	/*  GM_HANDLEINPUT method handler  */

static BOOL down;

static long buttonhandle(struct gpInput *msg,struct Gadget *g)
{
	long 	result;
	BOOL	inside;
	WORD	x = msg->gpi_Mouse.X,
			y = msg->gpi_Mouse.Y;

	if (msg->gpi_IEvent->ie_Class == IECLASS_RAWMOUSE)
	{
		inside = (x >= 0 && y >= 0 && x < g->Width && y < g->Height);

		if (msg->gpi_IEvent->ie_Code == (UBYTE)(IECODE_LBUTTON+IECODE_UP_PREFIX))
		{
			result = GMR_NOREUSE;
			if (inside) 
				result |= GMR_VERIFY;
			return result;
		}

		if (down != inside)
		{	struct RastPort	*rp;

			if (rp = ObtainGIRPort(msg->gpi_GInfo))
			{
				drawbutton(rp,g,down = inside);
				ReleaseGIRPort(rp);
			}
		}
	}

	return GMR_MEACTIVE;
}

static void buttonactive(struct gpGoInactive *msg,struct Gadget *g)
{
	struct RastPort *rp;

	if (rp = ObtainGIRPort(msg->gpgi_GInfo))
	{
		drawbutton(rp,g,1);
		down = 1;
		ReleaseGIRPort(rp);
	}
}

static void buttoninactive(struct gpGoInactive *msg,struct Gadget *g)
{
	struct RastPort *rp;

	if (rp = ObtainGIRPort(msg->gpgi_GInfo))
	{
		drawbutton(rp,g,0);
		ReleaseGIRPort(rp);
	}
}

	/*  GM_RENDER method handler  */

static void arrowdraw(struct gpRender *msg,struct Gadget *g)
{
	if (msg->gpr_Redraw == GREDRAW_REDRAW)
	{
		drawarrow(msg->gpr_RPort,g,g->Flags & GADGDISABLED ? 2 : 0);
	}
}

	/*  GM_HANDLEINPUT method handler  */

static long arrowhandle(struct gpInput *msg,struct Gadget *g)
{
	BOOL	inside;
	WORD	x = msg->gpi_Mouse.X,
			y = msg->gpi_Mouse.Y;

	if (msg->gpi_IEvent->ie_Class == IECLASS_RAWMOUSE)
	{
		inside = (x >= 0 && y >= 0 && x < g->Width && y < g->Height);

		if (msg->gpi_IEvent->ie_Code == (UBYTE)(IECODE_LBUTTON+IECODE_UP_PREFIX))
		{
			if (inside) 
				return GMR_NOREUSE | GMR_VERIFY;
			return GMR_REUSE;
		}

		if (down != inside)
		{
			struct RastPort	*rp;

			if (rp = ObtainGIRPort(msg->gpi_GInfo))
			{
				drawarrow(rp,g,down = inside);
				ReleaseGIRPort(rp);
			}
		}
	}

	return GMR_MEACTIVE;
}

static void arrowactive(struct gpGoInactive *msg,struct Gadget *g)
{
	struct RastPort *rp;

	if (rp = ObtainGIRPort(msg->gpgi_GInfo))
	{
		drawarrow(rp,g,1);
		down = 1;
		ReleaseGIRPort(rp);
	}
}

static void arrowinactive(struct gpGoInactive *msg,struct Gadget *g)
{
	struct RastPort *rp;

	if (rp = ObtainGIRPort(msg->gpgi_GInfo))
	{
		drawarrow(rp,g,0);
		ReleaseGIRPort(rp);
	}
}

	/*  All custom gadget input flows through here.  */

static long __asm __saveds buttonhook(
	register __a0 struct Hook *ihook,
	register __a1 long *msg,
	register __a2 struct Gadget *g)
{
	switch (*msg)
	{	
	case GM_RENDER:
		buttondraw((void *)msg,g);
		break;

	case GM_HITTEST:
		return anyhit((void *)msg,g);

	case GM_GOACTIVE:
		buttonactive((void *)msg,g);
		return GMR_MEACTIVE;

	case GM_HANDLEINPUT:
		return buttonhandle((void *)msg,g);

	case GM_GOINACTIVE:
		buttoninactive((void *)msg,g);
		break;
	}

	return 1;
}

static long __asm __saveds arrowhook(
	register __a0 struct Hook *ihook,
	register __a1 long *msg,
	register __a2 struct Gadget *g)
{
	switch (*msg)
	{
	case GM_RENDER:
		arrowdraw((void *)msg,g);
		break;

	case GM_HITTEST:
		return anyhit((void *)msg,g);

	case GM_GOACTIVE:
		arrowactive((void *)msg,g);
		return GMR_MEACTIVE;

	case GM_HANDLEINPUT:
		return arrowhandle((void *)msg,g);

	case GM_GOINACTIVE:
		arrowinactive((void *)msg,g);
		break;
	}

	return 1;
}

#endif
