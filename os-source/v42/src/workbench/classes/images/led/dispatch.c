/* led image
 * Copyright (c) 1994 Commodore International Services, Co.
 * Written by David N. Junod
 *
 */

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <images/led.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************/

static char SegmentArray[11][7] =
{
    {1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 0, 0},
    {1, 1, 0, 1, 1, 0, 1},
    {1, 1, 1, 1, 0, 0, 1},
    {0, 1, 1, 0, 0, 1, 1},
    {1, 0, 1, 1, 0, 1, 1},
    {1, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0}
};

/*****************************************************************************/

static LONG setAttrsMethod (Class * cl, struct Image * im, struct opSet * msg, BOOL init)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, im);
    struct TagItem *tags = msg->ops_AttrList;
    LONG segs, time, refresh;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    /* Initialize the variables */
    if (init)
    {
	im->Width = im->Height = 40;
	segs = time = refresh = 1;
	od->od_Pairs = 2;
    }
    else
    {
	/* Let the super class handle it first */
	segs = time = 0;
	refresh = DoSuperMethodA (cl, (Object *) im, msg);
    }

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case LED_Pairs:
		od->od_Pairs = (WORD) tidata;
		time = refresh = 1;
		break;

	    case LED_Values:
		/* Copy the values into our array */
		CopyMem ((APTR)tidata, (APTR)od->od_Values, sizeof (WORD) * od->od_Pairs);
		time = refresh = 1;
		break;

	    case LED_Colon:
		if (tidata)
		    od->od_Flags |= ODF_COLON;
		else
		    od->od_Flags &= ~ODF_COLON;
		break;

	    case LED_Negative:
		if (tidata)
		    od->od_Flags |= ODF_NEGATIVE;
		else
		    od->od_Flags &= ~ODF_NEGATIVE;
		refresh = 1;
		break;

	    case LED_Signed:
		if (tidata)
		    od->od_Flags |= ODF_SIGNED;
		else
		    od->od_Flags &= ~ODF_SIGNED;
		segs = refresh = 1;
		break;

	    case SYSIA_DrawInfo:
		od->od_DrawInfo = (struct DrawInfo *) tidata;
		break;

	    case IA_BGPen:
		od->od_BackgroundPen = tidata;
		refresh = 1;
		break;

	    case IA_FGPen:
		od->od_TextPen = tidata;
		refresh = 1;
		break;

	    case IA_Width:
		im->Width = (WORD) tidata;
		segs = refresh = 1;
		break;

	    case IA_Height:
		im->Height = (WORD) tidata;
		segs = refresh = 1;
		break;
	}
    }

    /* Make sure we have valid pens */
    if (od->od_BackgroundPen == -1)
	od->od_BackgroundPen = (LONG)od->od_DrawInfo->dri_Pens[BACKGROUNDPEN];
    if (od->od_TextPen == -1)
	od->od_TextPen = (LONG)od->od_DrawInfo->dri_Pens[TEXTPEN];

    /* Recalculate segment positioning */
    if (segs)
    {
	struct Rectangle *r;	/* Current rectangle */
	WORD hsw;		/* Horizontal segment width */
	WORD hsh;		/* Horizontal segment height */
	WORD vsw;		/* Vertical segment width */
	WORD vsh;		/* Vertical segment height */
	WORD ho;		/* Horizontal offset */
	WORD vo;		/* Vertical offset */

	/* Calculate segment sizes */
	ho = ((6 + 2 + 6) * od->od_Pairs) + ((od->od_Pairs - 1) * 5);
	if (od->od_Flags & ODF_SIGNED)
	    ho += 5;
	od->od_VCell = im->Height / 12;
	od->od_HCell = im->Width / ho;
	hsw = od->od_HCell * 6;
	vsh = im->Height / 2; // od->od_VCell * 6;
	vo = hsh = od->od_VCell;
	ho = vsw = od->od_HCell;
	od->od_HSW = hsw;
	od->od_VSW = vsw;

	/* 0 - horizontal */
	r = &od->od_Segments[0];
	r->MinX = ho;
	r->MinY = 0;
	r->MaxX = hsw - ho - 1;
	r->MaxY = r->MinY + od->od_VCell - 1;

	/* 1 - vertical */
	r = &od->od_Segments[1];
	r->MinX = hsw - ho;
	r->MinY = vo;
	r->MaxX = hsw - 1;
	r->MaxY = vsh - vo;

	/* 2 - vertical */
	r = &od->od_Segments[2];
	r->MinX = hsw - ho;
	r->MinY = vsh + 1;
	r->MaxX = hsw - 1;
	r->MaxY = im->Height - vo - 1;

	/* 3 - horizontal */
	r = &od->od_Segments[3];
	r->MinX = ho;
	r->MinY = im->Height - od->od_VCell;
	r->MaxX = hsw - ho - 1;
	r->MaxY = r->MinY + od->od_VCell - 1;

	/* 4 - vertical */
	r = &od->od_Segments[4];
	r->MinX = 0;
	r->MinY = vsh + 1;
	r->MaxX = ho - 1;
	r->MaxY = im->Height - vo - 1;

	/* 5 - vertical */
	r = &od->od_Segments[5];
	r->MinX = 0;
	r->MinY = vo;
	r->MaxX = ho - 1;
	r->MaxY = vsh - vo;

	/* 6 - horizontal */
	r = &od->od_Segments[6];
	r->MinX = ho;
	r->MinY = vsh - od->od_VCell + 1;
	r->MaxX = hsw - ho - 1;
	r->MaxY = r->MinY + od->od_VCell - 1;

	/* 7 - top colon */
	r = &od->od_Segments[7];
	r->MinX = 0;
	r->MinY = (od->od_VCell * 3) - 1;
	r->MaxX = ho - 1;
	r->MaxY = r->MinY + od->od_VCell;

	/* 8 - bottom colon */
	r = &od->od_Segments[8];
	r->MinX = 0;
	r->MinY = (od->od_VCell * 9) - 1;
	r->MaxX = ho - 1;
	r->MaxY = r->MinY + od->od_VCell;

	/* 9 - negative */
	r = &od->od_Segments[9];
	r->MinX = 0;
	r->MinY = vsh - vo + 1;
	r->MaxX = (od->od_HCell * 3) - 1;
	r->MaxY = r->MinY + od->od_VCell - 1;
    }

    /* If the time changed, then we need to fill in the lit segment array */
    if (time)
    {
	UBYTE i, j, k;
	WORD digit;

	for (i = k = 0; i < 8; i++)
	{
	    digit = od->od_Values[i] / 10;
	    digit = (digit < 0 || digit > 9) ? 0 : digit;
	    for (j = 0; j < 7; j++)
		od->od_LitSegments[k][j] = SegmentArray[digit][j];
	    k++;

	    digit = od->od_Values[i] % 10;
	    digit = (digit < 0 || digit > 9) ? 0 : digit;
	    for (j = 0; j < 7; j++)
		od->od_LitSegments[k][j] = SegmentArray[digit][j];
	    k++;
	}
    }

    return (refresh);
}

/*****************************************************************************/

static LONG drawMethod (Class * cl, struct Image * im, struct impDraw * msg)
{
    struct ClassLib *cb = (struct ClassLib *) cl->cl_UserData;
    struct objectData *od = INST_DATA (cl, im);
    struct RastPort *rp;
    struct Rectangle *r;
    WORD i, j, k;
    LONG tx, ty;
    UBYTE bpen;

    /* Get the things that we need */
    rp = msg->imp_RPort;

    /* Compute the size */
    tx = (LONG) msg->imp_Offset.X;
    ty = (LONG) msg->imp_Offset.Y;

    /* Get the background pen */
    bpen = rp->BgPen; // od->od_BackgroundPen;

    /* Draw the sign */
    if (od->od_Flags & ODF_SIGNED)
    {
	if (od->od_Flags & ODF_NEGATIVE)
	    SetAPen (rp, od->od_TextPen);
	else
	    SetAPen (rp, bpen);
	r = &od->od_Segments[9];
	RectFill (rp, tx + (LONG)r->MinX, ty + (LONG)r->MinY, tx + (LONG)r->MaxX, ty + (LONG)r->MaxY);
	tx += (od->od_HCell * 5);
    }

    /* Draw Segments */
    for (i = 0, k = od->od_Pairs << 1; i < k; i++)
    {
	for (j = 0; j < 7; j++)
	{
	    if (od->od_LitSegments[i][j])
		SetAPen (rp, od->od_TextPen);
	    else
		SetAPen (rp, bpen);
	    r = &od->od_Segments[j];
	    RectFill (rp, tx + (LONG)r->MinX, ty + (LONG)r->MinY, tx + (LONG)r->MaxX, ty + (LONG)r->MaxY);
	}

	tx += od->od_HSW + (od->od_HCell << 1);
	if ((i & 1) && (i < k - 1))
	{
	    if (od->od_Flags & ODF_COLON)
		SetAPen (rp, od->od_TextPen);
	    else
		SetAPen (rp, bpen);
	    r = &od->od_Segments[7];
	    RectFill (rp, tx + (LONG)r->MinX, ty + (LONG)r->MinY, tx + (LONG)r->MaxX, ty + (LONG)r->MaxY);
	    r = &od->od_Segments[8];
	    RectFill (rp, tx + (LONG)r->MinX, ty + (LONG)r->MinY, tx + (LONG)r->MaxX, ty + (LONG)r->MaxY);
	    tx += od->od_HCell + od->od_HCell + od->od_HCell;
	}
    }

    return (0);
}

/*****************************************************************************/

static LONG newMethod (Class * cl, struct Image * im, struct opSet * msg)
{
    struct Image *newobj;

    /* Create the new object */
    if (newobj = (struct Image *) DoSuperMethodA (cl, (Object *) im, msg))
    {
	/* Update the attributes */
	setAttrsMethod (cl, newobj, msg, TRUE);
    }

    return ((LONG) newobj);
}

/*****************************************************************************/

LONG ASM ClassDispatcher (REG (a0) Class * cl, REG (a1) ULONG * msg, REG (a2) struct Image * im)
{
    switch (*msg)
    {
	case OM_NEW:
	    return newMethod (cl, im, (struct opSet *) msg);

	case OM_SET:
	case OM_UPDATE:
	    return setAttrsMethod (cl, im, (struct opSet *) msg, FALSE);

	case IM_DRAW:
	    return drawMethod (cl, im, (struct impDraw *) msg);

	default:
	    return DoSuperMethodA (cl, (Object *) im, (Msg *) msg);
    }
}
