
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/regions.h>
#include <graphics/displayinfo.h>
#include <graphics/monitor.h>
#include <graphics/videocontrol.h>
#include <hardware/custom.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <libraries/gadtools.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "edit.h"
#include "pe_custom.h"
#include "pe_utils.h"

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>


#define SysBase ed->ed_SysBase


/*****************************************************************************/


#define OUT 0
#define MID 1
#define IN  2

/*  Size of handles in ticks */
#define HANDLETICKS 1024

/* The reason we require color zero to be black is because the "unusable"
 * area of the edit screen is the area outside of MaxOScan.  Part of that
 * area is drawn in black, and part of it is drawn in pen zero, even though
 * it is outside of the bitmap.  By making pen zero black, the two regions
 * appear identical, and the user can get a better understanding of the
 * usable area.
 * Note that we invert all the rendering so things still appear to have
 * black text on a gray background.
 */
struct ColorSpec far colors[] =
{
    {0,3,3,3,},	    /* Color zero must be close to black ! */
    {1,10,10,10,},
    {~0,0,0,0,},
};


/* We want color to be not completely black in order to allow folks to see when
 * they are pushing the origin of their display too much to the right. If the
 * background color is not black, pushing the display too far will result in
 * a very noticeable change in screen colors. If the screen is completely black,
 * then the color change will be much less pronounce and more easily
 * overlooked. Pushing the display too far is not good for the monitor, and
 * can also result in an unreadable Workbench (all black)
 */


/*****************************************************************************/


#define MIN_DENISE mspc->DeniseMinDisplayColumn
#define TOTCLKS mspc->total_colorclocks
#define MINROW  mspc->min_row
#define HBSTRT 2
#define HBSTOP (((MIN_DENISE + 1) >> 1) - 1)
#define HSSTRT ((((HBSTOP+1) - (HBSTRT-1)) / 3) + HBSTRT)
#define HSSTOP (((((HBSTOP+1) - (HBSTRT-1)) / 3) << 1) + HBSTRT)
#define VBSTRT (TOTCLKS)
#define VBSTOP ((MINROW * TOTCLKS) - TOTCLKS)
#define VSSTRT ((MINROW / 3) * TOTCLKS)
#define VSSTOP (((MINROW / 3) << 1) * TOTCLKS)

#define TOTYCLKS (mspc->total_rows * TOTCLKS)



/*****************************************************************************/


#define OK_ID     0
#define CANCEL_ID 1
#define UP_ID     2
#define DOWN_ID   3
#define LEFT_ID   4
#define RIGHT_ID  5


/*****************************************************************************/


APTR NewPrefsObject(EdDataPtr ed, STRPTR name, ULONG data,...)
{
    return (NewObjectA(NULL,name, (struct TagItem *) &data));
}


/*****************************************************************************/


VOID CheckSyncBounds(EdDataPtr ed, struct MonitorEntry *me)
{
struct MonitorSpec *mspc;

    mspc = me->me_MonitorInfo.Mspc;

    if (mspc && mspc->ms_Special)
    {
        if (me->me_HStop > HBSTOP)
            me->me_HStop = HBSTOP;

        if (me->me_HStart < HBSTRT)
            me->me_HStart = HBSTRT;

        if (me->me_VStop > VBSTOP)
            me->me_VStop = VBSTOP;

        if (me->me_VStart < VBSTRT)
            me->me_VStart = VBSTRT;
    }
}


/*****************************************************************************/


VOID SyncLeft(EdDataPtr ed, struct MonitorSpec *mspc)
{
struct AnalogSignalInterval *hsync;
struct Custom               *custom = (struct Custom *)0xdff000;

    if (mspc && mspc->ms_Special)
    {
	hsync = &mspc->ms_Special->hsync;

        if (hsync->asi_Stop < HBSTOP)
        {
            hsync->asi_Start++;
            hsync->asi_Stop++;

            custom->hsstrt = hsync->asi_Start;
            custom->hsstop = hsync->asi_Stop;
        }
    }
}


/*****************************************************************************/


VOID SyncRight(EdDataPtr ed, struct MonitorSpec *mspc)
{
struct AnalogSignalInterval *hsync;
struct Custom               *custom = (struct Custom *)0xdff000;

    if (mspc && mspc->ms_Special)
    {
	hsync = &mspc->ms_Special->hsync;

        if (hsync->asi_Start > HBSTRT)
        {
            hsync->asi_Start--;
            hsync->asi_Stop--;

            custom->hsstrt = hsync->asi_Start;
            custom->hsstop = hsync->asi_Stop;
        }
    }
}


/*****************************************************************************/


VOID SyncUp(EdDataPtr ed, struct MonitorSpec *mspc)
{
struct AnalogSignalInterval *vsync;
struct Custom               *custom = (struct Custom *)0xdff000;

    if (mspc && mspc->ms_Special)
    {
        vsync = &mspc->ms_Special->vsync;

        if (vsync->asi_Stop < VBSTOP)
        {
            vsync->asi_Start += TOTCLKS;
            vsync->asi_Stop  += TOTCLKS;

            custom->vsstrt = vsync->asi_Start / TOTCLKS;
            custom->vsstop = vsync->asi_Stop / TOTCLKS;
        }
    }
}


/*****************************************************************************/


VOID SyncDown(EdDataPtr ed, struct MonitorSpec *mspc)
{
struct AnalogSignalInterval *vsync;
struct Custom               *custom = (struct Custom *)0xdff000;

    if (mspc && mspc->ms_Special)
    {
        vsync = &mspc->ms_Special->vsync;

        if (vsync->asi_Start > VBSTRT)
        {
            vsync->asi_Start -= TOTCLKS;
            vsync->asi_Stop  -= TOTCLKS;

            custom->vsstrt = vsync->asi_Start / TOTCLKS;
            custom->vsstop = vsync->asi_Stop / TOTCLKS;
        }
    }
}


/*****************************************************************************/


static BOOL CanChangeSync(EdDataPtr ed, struct MonitorSpec *mspc)
{
struct AnalogSignalInterval *hsync;
struct AnalogSignalInterval *vsync;

    if (mspc && mspc->ms_Special)
    {
        hsync = &mspc->ms_Special->hsync;
        vsync = &mspc->ms_Special->vsync;

        if ((vsync->asi_Stop < VBSTOP)
        ||  (vsync->asi_Start > VBSTRT)
        ||  (hsync->asi_Start > HBSTRT)
        ||  (hsync->asi_Stop < HBSTOP))
        {
            return(TRUE);
        }
    }

    return (FALSE);
}


/*****************************************************************************/


VOID SyncAbs(EdDataPtr ed, struct MonitorSpec *mspc, UWORD x, UWORD y)
{
struct AnalogSignalInterval *hsync;
struct AnalogSignalInterval *vsync;
struct Custom               *custom = (struct Custom *)0xdff000;
UWORD                        diff;

    if (mspc && mspc->ms_Special)
    {
        hsync = &mspc->ms_Special->hsync;
        vsync = &mspc->ms_Special->vsync;

        diff = hsync->asi_Stop - hsync->asi_Start;
        hsync->asi_Start = HBSTRT + x;
        hsync->asi_Stop  = hsync->asi_Start + diff;

        diff = vsync->asi_Stop - vsync->asi_Start;
        vsync->asi_Start = VBSTRT + y;
        vsync->asi_Stop  = vsync->asi_Start + diff;

        custom->hsstrt = hsync->asi_Start;
        custom->hsstop = hsync->asi_Stop;
        custom->vsstrt = vsync->asi_Start / TOTCLKS;
        custom->vsstop = vsync->asi_Stop / TOTCLKS;
    }
}


/*****************************************************************************/


WORD RectWidth(struct Rectangle *rect)
{
    return(rect->MaxX - rect->MinX + 1);
}


/*****************************************************************************/


WORD RectHeight(struct Rectangle *rect)
{
    return(rect->MaxY - rect->MinY + 1);
}


/*****************************************************************************/


VOID ShiftRect(struct Rectangle *rect, WORD dx, WORD dy)
{
    rect->MinX += dx;
    rect->MaxX += dx;
    rect->MinY += dy;
    rect->MaxY += dy;
}


/*****************************************************************************/


WORD Modulo(WORD num, WORD div)
{
WORD result;

    result = num % div;

    if (result < 0)
    {
	result += div;
    }

    return(result);
}


/*****************************************************************************/


BOOL StartSizing(EdDataPtr ed, WORD x, WORD y)
{
WORD              midh, midv;
struct Rectangle *rect;

    rect = &ed->Current;
    midh = (rect->MinX + rect->MaxX - ed->HandleWidth) >> 1;
    midv = (rect->MinY + rect->MaxY - ed->HandleHeight) >> 1;

    ed->vert  = 0;
    ed->horiz = 0;

    if ((x >= rect->MinX) && (x <= rect->MinX + ed->HandleWidth))
	ed->horiz = 1;
    else if ((x >= midh) && (x <= midh + ed->HandleWidth))
	ed->horiz = 2;
    else if ((x >= rect->MaxX - ed->HandleWidth) && (x <= rect->MaxX))
	ed->horiz = 3;

    if ((y >= rect->MinY) && (y <= rect->MinY + ed->HandleHeight))
	ed->vert = 1;
    else if ((y >= midv) && (y <= midv + ed->HandleHeight))
	ed->vert = 2;
    else if ((y >= rect->MaxY - ed->HandleHeight) && (y <= rect->MaxY))
	ed->vert = 3;

    if ((ed->horiz == 0) || (ed->vert == 0))
        return(FALSE);

    ed->Undo = ed->Prev = ed->Current;
    ed->PrevHandles = ed->CurrentHandles;

    /*  Round off X and Y to view units: */
    x -= Modulo(x,ed->xscale);
    y -= Modulo(y,ed->yscale);

    ed->OldX = x;
    ed->OldY = y;

    ed->Delta.MinX = ed->Current.MinX - x;
    ed->Delta.MaxX = ed->Current.MaxX - x;
    ed->Delta.MinY = ed->Current.MinY - y;
    ed->Delta.MaxY = ed->Current.MaxY - y;

    if ((ed->horiz == 2) && (ed->vert == 2))
    {
	ed->LoLim.MaxX = ed->MaxOScan.MinX;
	ed->HiLim.MinX = ed->MaxOScan.MaxX;
	ed->LoLim.MaxY = ed->MaxOScan.MinY;
	ed->HiLim.MinY = ed->MaxOScan.MaxY;
    }
    else
    {
	ed->HiLim.MinX = ed->Current.MaxX - ed->NomWidth + 1;
	ed->LoLim.MaxX = ed->Current.MinX + ed->NomWidth - 1;
	ed->HiLim.MinY = ed->Current.MaxY - ed->NomHeight + 1;
	ed->LoLim.MaxY = ed->Current.MinY + ed->NomHeight - 1;
    }

    return(TRUE);
}


/*****************************************************************************/


VOID DrawRect(EdDataPtr ed, struct Rectangle *rect)
{
UWORD array[8];

    Move(ed->ed_EditRP,rect->MinX,rect->MinY);

    array[0] = array[2] = rect->MaxX;
    array[1] = array[7] = rect->MinY;
    array[3] = array[5] = rect->MaxY;
    array[4] = array[6] = rect->MinX;
    array[7]++;

    PolyDraw(ed->ed_EditRP,4,array);
}


/*****************************************************************************/


/* Calculate the coordinates of the handles of a given rectangle.
 */

VOID CalcHandles(EdDataPtr ed, struct Rectangle *rect, struct Handles *handles)
{
    /* First, we calculate three Points.  The one called OUT is
     * the upper-left point just inside the framing rectangle
     */
    handles->pos[OUT].x = rect->MinX+1;
    handles->pos[OUT].y = rect->MinY+1;

    /* The second Point MID, which holds the upper-left coordinate
     * of the center handle.
     */
    handles->pos[MID].x = ( rect->MinX + rect->MaxX - ed->HandleWidth ) >> 1;
    handles->pos[MID].y = ( rect->MinY + rect->MaxY - ed->HandleHeight ) >> 1;

    /* The third Point, IN, holds the upper-left coordinate just inside
     * the lower-right handle
     */
    handles->pos[IN].x = rect->MaxX-ed->HandleWidth;
    handles->pos[IN].y = rect->MaxY-ed->HandleHeight;

    /* The upper-left coordinates of the nine handles can be derived
     * from combinations of the three points.
     */
}


/*****************************************************************************/


/* Given one or two Handles structures (h2 may be NULL), draw the
 * active handle as indicated by globals 'horiz' and 'vert'.
 */

VOID MoveHandle(EdDataPtr ed, struct Handles *h1, struct Handles *h2)
{
struct Rectangle        rect;
WORD                    left, top;
struct RegionRectangle *rrect;

    ClearRegion(ed->ed_EditRegion);
    rect.MinX = h1->pos[ed->horiz-1].x;
    rect.MinY = h1->pos[ed->vert-1].y;
    rect.MaxX = rect.MinX+ed->HandleWidth-1;
    rect.MaxY = rect.MinY+ed->HandleHeight-1;

    OrRectRegion(ed->ed_EditRegion,&rect);

    if (h2)
    {
	rect.MinX = h2->pos[ed->horiz-1].x;
	rect.MinY = h2->pos[ed->vert-1].y;
	rect.MaxX = rect.MinX+ed->HandleWidth-1;
	rect.MaxY = rect.MinY+ed->HandleHeight-1;

	XorRectRegion(ed->ed_EditRegion,&rect);
    }

    left  = ed->ed_EditRegion->bounds.MinX;
    top   = ed->ed_EditRegion->bounds.MinY;
    rrect = ed->ed_EditRegion->RegionRectangle;
    while (rrect)
    {
	RectFill(ed->ed_EditRP,(left+rrect->bounds.MinX),(top+rrect->bounds.MinY),
                               (left+rrect->bounds.MaxX),(top+rrect->bounds.MaxY));
	rrect = rrect->Next;
    }
}


/*****************************************************************************/


/* Function to handle a MOUSEMOVE event to location x, y, representing
 * the sizing or moving of a frame.
 * Moves the 'Current' rectangle accordingly, with limit checking.
 * Affects globals Current, Prev.
 */

VOID DoSizing(EdDataPtr ed, WORD x, WORD y)
{
    /*  Round off X and Y to view units: */
    x -= Modulo( x, ed->xscale );
    y -= Modulo( y, ed->yscale );

    /*  Act depending on which handle is being held: */
    if ( ed->horiz == 1 )
    {
	/*  A left-side handle: */
	ed->Current.MinX = x + ed->Delta.MinX;
	if ( ed->Current.MinX < ed->LoLim.MinX )
	{
	    ed->Current.MinX = ed->LoLim.MinX;
	}
	else if ( ed->Current.MinX > ed->HiLim.MinX )
	{
	    ed->Current.MinX = ed->HiLim.MinX;
	}
    }
    else if ( ed->horiz == 3 )
    {
	/*  A right-side handle: */
	ed->Current.MaxX = x + ed->Delta.MaxX;
	if ( ed->Current.MaxX < ed->LoLim.MaxX )
	{
	    ed->Current.MaxX = ed->LoLim.MaxX;
	}
	else if ( ed->Current.MaxX > ed->HiLim.MaxX )
	{
	    ed->Current.MaxX = ed->HiLim.MaxX;
	}
    }

    if ( ed->vert == 1 )
    {
	/*  A top-edge handle: */
	ed->Current.MinY = y + ed->Delta.MinY;
	if ( ed->Current.MinY < ed->LoLim.MinY )
	{
	    ed->Current.MinY = ed->LoLim.MinY;
	}
	else if ( ed->Current.MinY > ed->HiLim.MinY )
	{
	    ed->Current.MinY = ed->HiLim.MinY;
	}
    }
    else if ( ed->vert == 3 )
    {
	/*  A bottom-edge handle: */
	ed->Current.MaxY = y + ed->Delta.MaxY;
	if ( ed->Current.MaxY < ed->LoLim.MaxY )
	{
	    ed->Current.MaxY = ed->LoLim.MaxY;
	}
	else if ( ed->Current.MaxY > ed->HiLim.MaxY )
	{
	    ed->Current.MaxY = ed->HiLim.MaxY;
	}
    }
    else if ( ( ed->vert == 2 ) && ( ed->horiz == 2 ) )
    {
	/*  The dead-center handle needs different
	    treatment, since we move, not size: */
	ed->Current.MinX = x + ed->Delta.MinX;
	ed->Current.MaxX = x + ed->Delta.MaxX;
	ed->Current.MinY = y + ed->Delta.MinY;
	ed->Current.MaxY = y + ed->Delta.MaxY;
	/*  Check for violation of limits.  Note that if we're
	    moving the TxtOScan rect, we don't want to check
	    the inner limits, since the inner limit is one of
	    size, not place: */
	if ( ed->Current.MinX < ed->LoLim.MinX )
	{
	    ed->Current.MaxX += ed->LoLim.MinX - ed->Current.MinX;
	    ed->Current.MinX = ed->LoLim.MinX;
	}
	else if ( ed->Current.MinX > ed->HiLim.MinX )
	{
	    ed->Current.MaxX -= ed->Current.MinX - ed->HiLim.MinX;
	    ed->Current.MinX = ed->HiLim.MinX;
	}

	if ( ed->Current.MaxX < ed->LoLim.MaxX )
	{
	    ed->Current.MinX += ed->LoLim.MaxX - ed->Current.MaxX;
	    ed->Current.MaxX = ed->LoLim.MaxX;
	}
	else if ( ed->Current.MaxX > ed->HiLim.MaxX )
	{
	    ed->Current.MinX -= ed->Current.MaxX - ed->HiLim.MaxX;
	    ed->Current.MaxX = ed->HiLim.MaxX;
	}

	if ( ed->Current.MinY < ed->LoLim.MinY )
	{
	    ed->Current.MaxY += ed->LoLim.MinY - ed->Current.MinY;
	    ed->Current.MinY = ed->LoLim.MinY;
	}
	else if ( ed->Current.MinY > ed->HiLim.MinY )
	{
	    ed->Current.MaxY -= ed->Current.MinY - ed->HiLim.MinY;
	    ed->Current.MinY = ed->HiLim.MinY;
	}

	if ( ed->Current.MaxY < ed->LoLim.MaxY )
	{
	    ed->Current.MinY += ed->LoLim.MaxY - ed->Current.MaxY;
	    ed->Current.MaxY = ed->LoLim.MaxY;
	}
	else if (ed->Current.MaxY > ed->HiLim.MaxY )
	{
	    ed->Current.MinY -= ed->Current.MaxY - ed->HiLim.MaxY;
	    ed->Current.MaxY = ed->HiLim.MaxY;
	}
    }

    /*  Only do graphics if the rect changed: */
    if ( ( ed->Current.MinY != ed->Prev.MinY ) || ( ed->Current.MaxX != ed->Prev.MaxX ) ||
	( ed->Current.MinX != ed->Prev.MinX ) || ( ed->Current.MaxY != ed->Prev.MaxY ) )
    {
	/*  Draw new, erase old ( magic of XOR's ): */
	CalcHandles(ed,&ed->Current, &ed->CurrentHandles );
	DrawRect(ed, &ed->Current );
	DrawRect(ed, &ed->Prev );
	MoveHandle(ed, &ed->CurrentHandles, &ed->PrevHandles );
	ed->Prev = ed->Current;
	ed->PrevHandles = ed->CurrentHandles;
    }

    ed->OldX = x;
    ed->OldY = y;
}


/*****************************************************************************/


VOID DrawHandles(EdDataPtr ed, struct Handles *handles)
{
ULONG row, col;
WORD  left, top;

    for (row = OUT; row <= IN; row++)
    {
	for (col = OUT; col <= IN; col++)
	{
	    left = handles->pos[row].x;
	    top  = handles->pos[col].y;
	    RectFill(ed->ed_EditRP,left,top,left+ed->HandleWidth-1,top+ed->HandleHeight-1);
	}
    }
}


/*****************************************************************************/


/* We have all the rectangles expressed as they come from the database,
 * in which the upper-left of TxtOScan is nailed to ( 0,0 ), and the
 * others are relative to that.  We want it all relative to the
 * upper-left of MaxOScan nailed at ( 0,0 ).
 */

VOID ConvertToDisplayStyle(EdDataPtr ed)
{
WORD x, y;

    x = -ed->ed_CurrentMon->me_DimensionInfo.MaxOScan.MinX;
    y = -ed->ed_CurrentMon->me_DimensionInfo.MaxOScan.MinY;

    ed->StdOScan = ed->ed_CurrentMon->me_DimensionInfo.StdOScan;
    ed->TxtOScan = ed->ed_CurrentMon->me_DimensionInfo.TxtOScan;
    ed->MaxOScan = ed->ed_CurrentMon->me_DimensionInfo.MaxOScan;

    ShiftRect(&ed->MaxOScan,x,y);
    ShiftRect(&ed->StdOScan,x,y);
    ShiftRect(&ed->TxtOScan,x,y);
}


/*****************************************************************************/


/* Converts the user's changes to numbers in the style of the graphics
 * database, for all modes of the same MonitorEntry ( virtual monitor ).
 * Now that the user has finished editing the rectangles, we have the
 * new numbers in screen-relative coordinates, in which the upper left
 * of the MaxOScan rectangle is nailed at ( 0,0 ).  We've got to convert
 * these rectangles to Graphics-Database format, in which the TxtOScan
 * has upper-left nailed at ( 0,0 ).  Of course, we have to do scaling
 * between different modes of the same virtual monitor, too.
 */

VOID ConvertToGfxDbStyle(EdDataPtr ed, BOOL text)
{
WORD  TxtWidth, TxtHeight;
Point shift, size;
struct MonitorEntry *me;

    me = ed->ed_CurrentMon;

    TxtWidth  = RectWidth(&ed->TxtOScan);
    TxtHeight = RectHeight(&ed->TxtOScan);

    /*  TxtOScan must be no bigger than StdOScan.  Let size measure
	this excess, or zero if ok: */
    if ((size.x = (TxtWidth - RectWidth(&ed->StdOScan))) <= 0)
	size.x = 0;

    if ((size.y = (TxtHeight - RectHeight(&ed->StdOScan))) <= 0)
	size.y = 0;

    if (text)
    {
	ed->StdOScan.MaxX += size.x;
	ed->StdOScan.MaxY += size.y;
    }
    else
    {
	ed->TxtOScan.MaxX -= size.x;
	ed->TxtOScan.MaxY -= size.y;
    }

    /*  TxtOScan must remain inside StdOScan.  Let shift measure
	the offset needed to make it so.  A positive offset indicates
	that TxtOScan must be increased ( or StdOScan decreased ), and
	vice versa: */
    if ((shift.x = (ed->StdOScan.MinX - ed->TxtOScan.MinX)) > 0 )
    {
    }
    else if ((shift.x = (ed->StdOScan.MaxX - ed->TxtOScan.MaxX)) < 0)
    {
    }
    else
	shift.x = 0;

    if ((shift.y = (ed->StdOScan.MinY - ed->TxtOScan.MinY)) > 0)
    {
    }
    else if ((shift.y = (ed->StdOScan.MaxY - ed->TxtOScan.MaxY)) < 0)
    {
    }
    else
	shift.y = 0;


    if (text)
    {
	ShiftRect(&ed->StdOScan,(WORD)(-shift.x),(WORD)(-shift.y));
    }
    else
    {
	ShiftRect(&ed->TxtOScan,shift.x,shift.y);
    }

    /*  By this point, we have legal TxtOScan and StdOScan
	rectangles ( S contains T ) in display-relative coordinates. */

    /*  We know that the ViewPosition added to the MaxOScan upper
	limit ( in ViewResolution units ) is the dViewPos constant
	( calculated earlier ).  However, the rectangles are
	MaxOScan-relative, and not yet TxtOScan-relative, so we
	should use -TxtOScan.Min in place of MaxOScan.Min.
	Use this to figure out the ViewPosition: */

    ed->ViewPos.x = ed->dViewPos.x + ( ( ( LONG )ed->TxtOScan.MinX ) * me->me_DisplayInfo.Resolution.x ) / me->me_MonitorInfo.ViewResolution.x;
    ed->ViewPos.y = ed->dViewPos.y + ( ( ( LONG )ed->TxtOScan.MinY ) * me->me_DisplayInfo.Resolution.y ) / me->me_MonitorInfo.ViewResolution.y;

    /*  Since the ViewPos can only be measured in integral units
	of ViewResolution, it's possible to have a few pixels rounded
	off here.  The result will be"that the StdOScan and TxtOScan
	rectangles will be shifted left ( or up ) from what the user
	intended, by the amount needed to achieve this granularity.
	However, the MaxOScan rectangle is not free to move by this
	small amount ( being fixed in position, but always described
	as ViewPos-relative ), so we have to account for this rounding.  */

    /*  Measure how much the ViewPosition shifted, in mode resolution: */
    shift.x = ((me->me_MonitorInfo.ViewPosition.x - ed->ViewPos.x) * me->me_MonitorInfo.ViewResolution.x) / me->me_DisplayInfo.Resolution.x;
    shift.y = ((me->me_MonitorInfo.ViewPosition.y - ed->ViewPos.y) * me->me_MonitorInfo.ViewResolution.y) / me->me_DisplayInfo.Resolution.y;

    /*  Shift MaxOScan accordingly: */
    ShiftRect(&me->me_DimensionInfo.MaxOScan, shift.x, shift.y );

    /*  Throw the TxtOScan and StdOScan rectangles back into
	TxtOScan-relative coordinates: */
    shift.x = -ed->TxtOScan.MinX;
    shift.y = -ed->TxtOScan.MinY;
    ShiftRect(&ed->TxtOScan,shift.x,shift.y);
    ShiftRect(&ed->StdOScan,shift.x,shift.y);

    me->me_DimensionInfo.TxtOScan   = ed->TxtOScan;
    me->me_DimensionInfo.StdOScan   = ed->StdOScan;
    me->me_MonitorInfo.ViewPosition = ed->ViewPos;
}


/*****************************************************************************/


VOID PrintCurrentSize(EdDataPtr ed)
{
char             buffer[40];
struct RastPort *rp;
UWORD            len;

    rp = ed->ed_EditRP;

    SetDrMd(rp,JAM2);

    sprintf(buffer,GetString(&ed->ed_LocaleInfo,MSG_OSCN_EDITPOS),ed->Current.MinX,ed->Current.MinY);
    len = strlen(buffer);
    Move(rp,(RectWidth(&ed->MaxOScan)-TextLength(rp,buffer,len)) / 2,
            RectHeight(&ed->MaxOScan)-ed->HandleHeight-64);
    Text(rp,buffer,len);

    sprintf(buffer,GetString(&ed->ed_LocaleInfo,MSG_OSCN_EDITSIZE),RectWidth(&ed->Current),RectHeight(&ed->Current));
    len = strlen(buffer);
    Move(rp,(RectWidth(&ed->MaxOScan)-TextLength(rp,buffer,len)) / 2,
            RectHeight(&ed->MaxOScan)-ed->HandleHeight-55);
    Text(rp,buffer,len);

    SetDrMd(rp,COMPLEMENT);
}


/*****************************************************************************/


VOID PrintInstructions(EdDataPtr ed, AppStringsID instr)
{
UWORD  y;
UWORD  len;
STRPTR str;

    str = GetString(&ed->ed_LocaleInfo,instr);
    y   = ed->HandleHeight+25;

    while (TRUE)
    {
        len = 0;
        while ((str[len] != 0) && (str[len] != '\n'))
            len++;

        if (len > 1)
        {
            Move(ed->ed_EditRP,(RectWidth(&ed->MaxOScan)-TextLength(ed->ed_EditRP,str,len)) / 2,y);
            Text(ed->ed_EditRP,str,len);
        }

        if (str[len] == 0)
            break;

        str  = (STRPTR)&str[len+1];
        y   += 9;
    }

    PrintCurrentSize(ed);
}


/*****************************************************************************/


VOID DrawGadget(EdDataPtr ed, struct Window *wp, struct Gadget *gad, AppStringsID name)
{
struct RastPort *rp;
WORD             x0,y0,x1,y1;

    rp = wp->RPort;
    SetAPen(rp,wp->WScreen->RastPort.FgPen);
    SetBPen(rp,wp->WScreen->RastPort.BgPen);

    x0 = gad->LeftEdge;
    y0 = gad->TopEdge;
    x1 = x0+gad->Width-1;
    y1 = y0+gad->Height-1;

    RectFill(rp,x0,y0,x1,y0);
    RectFill(rp,x0,y1,x1,y1);
    RectFill(rp,x0,y0+1,x0+1,y1-1);
    RectFill(rp,x1-1,y0+1,x1,y1-1);
    CenterLine(ed,wp,name,x0,y1-4,gad->Width);
}


/*****************************************************************************/


struct Screen *OpenPrefsScreen(EdDataPtr ed, ULONG tag1, ...)
{
    return(OpenScreenTagList(NULL,(struct TagItem *) &tag1));
}


/*****************************************************************************/


BOOL EditEventLoop(EdDataPtr ed, struct Window *wp)
{
struct IntuiMessage *intuiMsg, copymsg;
ULONG                mask;
ULONG                sigs;
BOOL                 sizing = FALSE;
struct MonitorSpec  *mspc;
BOOL                 doArrows = FALSE;
struct Gadget       *lastGad;
UWORD                skipTick;

    mspc = ed->ed_CurrentMon->me_MonitorInfo.Mspc;

    mask = (1<<window->UserPort->mp_SigBit) | (1<<wp->UserPort->mp_SigBit) | SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_C;
    while (TRUE)
    {
	sigs = Wait(mask);

	if (sigs & SIGBREAKF_CTRL_F)
	{
	    ScreenToFront(wp->WScreen);
	    ActivateWindow(wp);
	}

        if (sigs & SIGBREAKF_CTRL_C)
        {
            Signal(FindTask(NULL),SIGBREAKF_CTRL_C);
            return(FALSE);
        }

	while (intuiMsg = GT_GetIMsg(window->UserPort))
	{
	    if (intuiMsg->Class != IDCMP_REFRESHWINDOW)
	    {
	        ScreenToFront(wp->WScreen);
	        ActivateWindow(wp);
	    }

	    GT_ReplyIMsg(intuiMsg);
            GT_BeginRefresh(window);
            RenderDisplay(ed);
            GT_EndRefresh(window,TRUE);
	}

	while (intuiMsg = (struct IntuiMessage *)GetMsg(wp->UserPort))
        {
            copymsg = *intuiMsg;
            ReplyMsg(intuiMsg);

            switch (copymsg.Class)
            {
                case RAWKEY      : if (copymsg.Code == 0x4f)
                                   {
                                       SyncLeft(ed,mspc);
                                   }
                                   else if (copymsg.Code == 0x4e)
                                   {
                                       SyncRight(ed,mspc);
                                   }
                                   else if (copymsg.Code == 0x4d)
                                   {
                                       SyncDown(ed,mspc);
                                   }
                                   else if (copymsg.Code == 0x4c)
                                   {
                                       SyncUp(ed,mspc);
                                   }
                                   break;

                case VANILLAKEY  : if ((copymsg.Code == 13) || (copymsg.Code == 'v'))
                                       return(TRUE);

                    		   if ((copymsg.Code == 27) || (copymsg.Code == 'b'))
                    		       return(FALSE);

                    		   break;

                case INACTIVEWINDOW: doArrows = FALSE;
                                     if (!sizing)
                                         break;

                case MOUSEBUTTONS: if (sizing && ((copymsg.Class == INACTIVEWINDOW) || (copymsg.Code == SELECTUP) || (copymsg.Code == MENUDOWN)))
                                   {
                                       sizing = FALSE;
                                       MoveHandle(ed, &ed->CurrentHandles, NULL );
                                       if ((copymsg.Class == INACTIVEWINDOW) || copymsg.Code == MENUDOWN)
                                       {
                                           DrawRect(ed, &ed->Current );
                                           ed->Current = ed->Undo;
                                           CalcHandles(ed,&ed->Current, &ed->CurrentHandles );
                                           DrawRect(ed, &ed->Current );
                                           DrawHandles(ed, &ed->CurrentHandles );
                                           PrintCurrentSize(ed);
                                       }
                                       else
                                       {
                                           DrawHandles(ed, &ed->CurrentHandles );
                                       }
                                   }
                                   else if (copymsg.Code == SELECTDOWN)
                                   {
                                       if (sizing = StartSizing(ed,copymsg.MouseX,copymsg.MouseY))
                                       {
                                           DrawHandles(ed,&ed->CurrentHandles);
                                           MoveHandle(ed,&ed->CurrentHandles,NULL);
                                       }
                                   }
                                   doArrows = FALSE;
                                   break;

                case MOUSEMOVE   : if (sizing)
                                   {
                                       DoSizing(ed,wp->MouseX,wp->MouseY);
                                       PrintCurrentSize(ed);
                                   }
                                   break;

                case INTUITICKS  : if (!doArrows)
                                       break;

                                   if (skipTick)
                                   {
                                       skipTick--;
                                       break;
                                   }

                                   if (!(lastGad->Flags & GFLG_SELECTED))
                                       break;

                                   copymsg.IAddress = lastGad;
                                   /* fall through */

                case GADGETDOWN  : doArrows = TRUE;
                                   lastGad = copymsg.IAddress;

                                   if (copymsg.Class == GADGETDOWN)
                                       skipTick = 4;

                                   switch (((struct Gadget *)copymsg.IAddress)->GadgetID)
                                   {
                                       case UP_ID    : SyncUp(ed,mspc);
                                                       break;

                                       case DOWN_ID  : SyncDown(ed,mspc);
                                                       break;

                                       case LEFT_ID  : SyncLeft(ed,mspc);
                                                       break;

                                       case RIGHT_ID : SyncRight(ed,mspc);
                                                       break;
                                   }
                                   break;

                case GADGETUP    : doArrows = FALSE;
                                   switch (((struct Gadget *)copymsg.IAddress)->GadgetID)
                                   {
                                       case OK_ID    : return(TRUE);
                                       case CANCEL_ID: return(FALSE);
                                   }
                                   break;
	    }
	}
    }
}


/*****************************************************************************/


static WORD down_vec[16] = {0,0, 4,4, 8,0, 7,0, 4,2, 1,0, 4,3, 7,0};
static WORD up_vec[16]   = {0,4, 4,0, 8,4, 7,4, 4,2, 1,4, 4,1, 7,4};
static WORD left_vec[16] = {5,0, 3,2, 5,4, 4,4, 0,2, 2,2, 4,0, 0,2};
static WORD right_vec[16]= {0,0, 2,2, 0,4, 1,4, 5,2, 2,2, 1,0, 5,2};


/*****************************************************************************/


VOID EditOverscan(EdDataPtr ed, BOOL text)
{
BOOL                         result = FALSE;
struct Screen               *sp;
struct Window               *wp;
struct Gadget                ok,cancel;
struct MonitorEntry         *mon;
struct MonitorSpec          *mspc;
struct AnalogSignalInterval *hsync, *vsync;
struct Custom               *custom = (struct Custom *)0xdff000;
UWORD                        initHStart, initHStop, initVStart, initVStop;
ULONG                        lock;
struct DrawInfo             *drawInfo;
struct Gadget                up;
struct Gadget                down;
struct Gadget                left;
struct Gadget                right;
struct TagItem               vtags[3];

struct Border upBorder    = {8, 7, 0, 0, JAM2, 8, up_vec, NULL};
struct Border downBorder  = {8, 7, 0, 0, JAM2, 8, down_vec, NULL};
struct Border leftBorder  = {11, 6, 0, 0, JAM2, 8, left_vec, NULL};
struct Border rightBorder = {13, 6, 0, 0, JAM2, 8, right_vec, NULL};

    mon = ed->ed_CurrentMon;

    /*  We'll need to track how much the view position changed:
	ViewPos.x and .y are the current view position: */
    ed->ViewPos.x = mon->me_MonitorInfo.ViewPosition.x;
    ed->ViewPos.y = mon->me_MonitorInfo.ViewPosition.y;

    /*  dViewPos.x and dViewPos.y measure (in ViewResolution units)
	the relationship between the MaxOScan upper left limit
	and the ViewPosition (their sum is always constant,
	and dViewPos is that sum): */
    ed->dViewPos.x = ed->ViewPos.x + (((LONG)mon->me_DimensionInfo.MaxOScan.MinX) * mon->me_DisplayInfo.Resolution.x) / mon->me_MonitorInfo.ViewResolution.x;
    ed->dViewPos.y = ed->ViewPos.y + (((LONG)mon->me_DimensionInfo.MaxOScan.MinY) * mon->me_DisplayInfo.Resolution.y) / mon->me_MonitorInfo.ViewResolution.y;

    ConvertToDisplayStyle(ed);

    ed->NomWidth     = RectWidth(&mon->me_DimensionInfo.Nominal);
    ed->NomHeight    = RectHeight(&mon->me_DimensionInfo.Nominal);
    ed->xscale       = mon->me_MonitorInfo.ViewResolution.x / mon->me_DisplayInfo.Resolution.x;
    ed->yscale       = mon->me_MonitorInfo.ViewResolution.y / mon->me_DisplayInfo.Resolution.y;
    ed->HandleWidth  = HANDLETICKS/mon->me_DisplayInfo.Resolution.x;
    ed->HandleHeight = HANDLETICKS/mon->me_DisplayInfo.Resolution.y;

    if (ed->ed_EditRegion = NewRegion())
    {
        hsync = NULL;
        mspc  = mon->me_MonitorInfo.Mspc;
        if (mspc && mspc->ms_Special)
        {
            hsync = &mspc->ms_Special->hsync;
            vsync = &mspc->ms_Special->vsync;

            initHStart = hsync->asi_Start;
            initHStop  = hsync->asi_Stop;
            initVStart = vsync->asi_Start;
            initVStop  = vsync->asi_Stop;

            hsync->asi_Start = mon->me_HStart;
            hsync->asi_Stop  = mon->me_HStop;
            vsync->asi_Start = mon->me_VStart;
            vsync->asi_Stop  = mon->me_VStop;
        }

        // For genlock and MPEG support...
        vtags[0].ti_Tag  = VTAG_CHROMA_PEN_SET;
        vtags[0].ti_Data = 1;
        vtags[1].ti_Tag  = VTAG_CHROMAKEY_SET;
        vtags[1].ti_Data = TRUE;
        vtags[2].ti_Tag  = TAG_DONE;

        if (sp = OpenPrefsScreen(ed,SA_DisplayID,    mon->me_ID,
                                    SA_Overscan,     OSCAN_MAX,
                                    SA_ShowTitle,    FALSE,
                                    SA_Colors,       colors,
                                    SA_Font,         &ed->ed_TextAttr,
                                    SA_Exclusive,    TRUE,
                                    SA_VideoControl, vtags,
                                    TAG_DONE))
        {
            if (drawInfo = GetScreenDrawInfo(sp))
            {
                if (hsync)  /* nudge HW in case gfx didn't do that for us */
                {
                    custom->hsstrt = mon->me_HStart;
                    custom->hsstop = mon->me_HStop;
                    custom->vsstrt = mon->me_VStart / TOTCLKS;
                    custom->vsstop = mon->me_VStop / TOTCLKS;
                }

                ok.NextGadget     = &cancel;
                ok.LeftEdge       = ed->HandleWidth+70;
                ok.TopEdge        = sp->Height-ed->HandleHeight-45;
                ok.Width          = 87;
                ok.Height         = 14;
                ok.Flags          = GFLG_GADGHCOMP;
                ok.Activation     = GACT_RELVERIFY;
                ok.GadgetType     = GTYP_BOOLGADGET;
                ok.GadgetRender   = NULL;
                ok.SelectRender   = NULL;
                ok.GadgetText     = NULL;
                ok.MutualExclude  = 0;
                ok.SpecialInfo    = NULL;
                ok.GadgetID       = OK_ID;

                cancel            = ok;
                cancel.NextGadget = NULL;;
                cancel.LeftEdge   = sp->Width-ed->HandleWidth-70-87;
                cancel.GadgetID   = CANCEL_ID;

                up.NextGadget     = &left;
                up.LeftEdge       = ed->HandleWidth+75;
                up.TopEdge        = ok.TopEdge - 10;
                up.Width          = 25;
                up.Height         = 20;
                up.Flags          = GFLG_GADGHCOMP;
                up.Activation     = GACT_IMMEDIATE | GACT_RELVERIFY;
                up.GadgetType     = GTYP_BOOLGADGET;
                up.GadgetRender   = &upBorder;
                up.SelectRender   = NULL;
                up.GadgetText     = NULL;
                up.MutualExclude  = 0;
                up.SpecialInfo    = NULL;
                up.GadgetID       = UP_ID;

                left.NextGadget     = &right;
                left.LeftEdge       = up.LeftEdge - 30;
                left.TopEdge        = up.TopEdge + up.Height;
                left.Width          = 30;
                left.Height         = 16;
                left.Flags          = GFLG_GADGHCOMP;
                left.Activation     = GACT_IMMEDIATE | GACT_RELVERIFY;
                left.GadgetType     = GTYP_BOOLGADGET;
                left.GadgetRender   = &leftBorder;
                left.SelectRender   = NULL;
                left.GadgetText     = NULL;
                left.MutualExclude  = 0;
                left.SpecialInfo    = NULL;
                left.GadgetID       = LEFT_ID;

                right.NextGadget     = &down;
                right.LeftEdge       = up.LeftEdge + up.Width;
                right.TopEdge        = left.TopEdge;
                right.Width          = 30;
                right.Height         = 16;
                right.Flags          = GFLG_GADGHCOMP;
                right.Activation     = GACT_IMMEDIATE | GACT_RELVERIFY;
                right.GadgetType     = GTYP_BOOLGADGET;
                right.GadgetRender   = &rightBorder;
                right.SelectRender   = NULL;
                right.GadgetText     = NULL;
                right.MutualExclude  = 0;
                right.SpecialInfo    = NULL;
                right.GadgetID       = RIGHT_ID;

                down.NextGadget     = NULL;
                down.LeftEdge       = up.LeftEdge;
                down.TopEdge        = left.TopEdge + left.Height;
                down.Width          = 25;
                down.Height         = 20;
                down.Flags          = GFLG_GADGHCOMP;
                down.Activation     = GACT_IMMEDIATE | GACT_RELVERIFY;
                down.GadgetType     = GTYP_BOOLGADGET;
                down.GadgetRender   = &downBorder;
                down.SelectRender   = NULL;
                down.GadgetText     = NULL;
                down.MutualExclude  = 0;
                down.SpecialInfo    = NULL;
                down.GadgetID       = DOWN_ID;

                if (CanChangeSync(ed,mspc))
                {
                    ok.LeftEdge       = cancel.LeftEdge;
                    cancel.TopEdge   += 16;
                    cancel.NextGadget = &up;
                }

                if (wp = OpenPrefsWindow(ed,WA_Left,         0,
                                            WA_Top,          0,
                                            WA_Width,        sp->Width,
                                            WA_Height,       sp->Height,
                                            WA_IDCMP,        IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW | IDCMP_RAWKEY | IDCMP_INTUITICKS,
                                            WA_Flags,        WFLG_BACKDROP | WFLG_REPORTMOUSE | WFLG_NOCAREREFRESH | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP,
                                            WA_CustomScreen, sp,
                                            WA_Gadgets,      &ok,
                                            TAG_DONE))
                {
                    result        = TRUE;
                    ed->ed_EditRP = &sp->RastPort;

                    SetABPenDrMd(ed->ed_EditRP,0,1,JAM2);
                    SetRast(ed->ed_EditRP,1);

                    if (cancel.NextGadget == &up)
                    {
                        RefreshGList(&up,wp,NULL,4);

                        DrawGadget(ed,wp,&up,MSG_NOTHING);
                        DrawGadget(ed,wp,&down,MSG_NOTHING);
                        DrawGadget(ed,wp,&left,MSG_NOTHING);
                        DrawGadget(ed,wp,&right,MSG_NOTHING);
                    }

                    DrawGadget(ed,wp,&ok,MSG_OK_GAD);
                    DrawGadget(ed,wp,&cancel,MSG_CANCEL_GAD);

                    ed->horiz = ed->vert = 0;

                    /*  LoLim and HiLim represent the low and high bounds of movement
                        for each edge ( Min and Max X and Y ).  The outer limit is always
                        the MaxOScan rectangle ( fixed ), but the inner limit is the nominal
                        size ( which must be set dynamically ).  Here we set the outer limit: */
                    ed->LoLim.MinX = ed->MaxOScan.MinX;
                    ed->HiLim.MaxX = ed->MaxOScan.MaxX;
                    ed->LoLim.MinY = ed->MaxOScan.MinY;
                    ed->HiLim.MaxY = ed->MaxOScan.MaxY;

                    if (text)
                    {
                        ed->Current = ed->TxtOScan;
                        PrintInstructions(ed,MSG_OSCN_TEXTEDIT);
                    }
                    else
                    {
                        ed->Current = ed->StdOScan;
                        PrintInstructions(ed,MSG_OSCN_GFXEDIT);
                    }

                    CalcHandles(ed,&ed->Current, &ed->CurrentHandles );
                    ed->Prev = ed->Current;
                    ed->PrevHandles = ed->CurrentHandles;

                    /*  Draw the frame, and all handles */
                    DrawRect(ed, &ed->Current );
                    DrawHandles(ed, &ed->CurrentHandles );

                    if (EditEventLoop(ed,wp))
                    {
                        if (text)
                            ed->TxtOScan = ed->Current;
                        else
                            ed->StdOScan = ed->Current;

                        ConvertToGfxDbStyle(ed,text);

                        if (hsync)
                        {
                            mon->me_HStart = hsync->asi_Start;
                            mon->me_HStop  = hsync->asi_Stop;
                            mon->me_VStart = vsync->asi_Start;
                            mon->me_VStop  = vsync->asi_Stop;
                        }
                    }

                    CloseWindow(wp);
                }

                if (hsync)
                {
                    hsync->asi_Start = initHStart;
                    hsync->asi_Stop  = initHStop;
                    vsync->asi_Start = initVStart;
                    vsync->asi_Stop  = initVStop;
                }

                FreeScreenDrawInfo(sp,drawInfo);
            }
	    CloseScreen(sp);

            if (hsync)
            {
                lock = LockIBase(0);

                /* if the monitor of the new current screen is the same as the
                 * one for the edit screen, gfx won't poke the hardware sync
                 * values, so we need to do this in order to avoid any edits
                 * done to affect the rest of the system.
                 */
                if (GfxBase->current_monitor == mspc)
                {
                    custom->hsstrt = initHStart;
                    custom->hsstop = initHStop;
                    custom->vsstrt = initVStart / TOTCLKS;
                    custom->vsstop = initVStop / TOTCLKS;
                }

                UnlockIBase(lock);
            }
	}
	DisposeRegion(ed->ed_EditRegion);
    }

    if (!result)
        DisplayBeep(NULL);
}
