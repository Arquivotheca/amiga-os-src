
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/locale.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>

/* pragmas */
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* application includes */
#include "worldmap.h"
#include "pe_utils.h"

#define SysBase ed->ed_SysBase;

#pragma libcall GfxBase BltBitMapRastPort 25E 65432910809

/*****************************************************************************/

#define	DB(x)	x

/*****************************************************************************/

/* VERTICAL UNPACKER */
VOID unpacker (BYTE * source, BYTE * pDest, LONG BytesPerRow, LONG Rows)
{
    LONG minus128 = -128L;	/* get the compiler to generate a CMP.W */
    register LONG n;
    register BYTE c;
    LONG dstbytes;
    WORD column;
    BYTE *dest;
    WORD row;

    dstbytes = BytesPerRow * Rows;

    row = column = 0;
    dest = &pDest[0];

    while (dstbytes > 0)
    {
	n = (LONG) * source++;

	if (n >= 0L)
	{
	    n += 1L;

	    dstbytes -= n;

	    do
	    {
		*dest = *source++;
		dest += BytesPerRow;
		row++;
		if (row >= Rows)
		{
		    row = 0;
		    column++;
		    dest = &pDest[column];
		}
	    } while (--n > 0L);
	}
	else if (n != minus128)
	{
	    n = -n + 1L;

	    dstbytes -= n;

	    c = *source++;

	    do
	    {
		*dest = c;
		dest += BytesPerRow;
		row++;
		if (row >= Rows)
		{
		    row = 0;
		    column++;
		    dest = &pDest[column];
		}
	    } while (--n > 0L);
	}
    }
}

/*****************************************************************************/

BOOL InitMapData (EdDataPtr ed)
{
    register WORD i;
    ULONG msize;

    msize = (ULONG) (WorldI.BitMap->BytesPerRow * WorldI.BitMap->Rows * 4);
    if (WorldI.BitMap->Planes[0] = AllocVec (msize, MEMF_CLEAR | MEMF_CHIP))
    {
	unpacker (WorldI.Compress, WorldI.BitMap->Planes[0], (LONG) WorldI.BitMap->BytesPerRow, (LONG) WorldI.BitMap->Rows);
	msize = (ULONG) (MasksI.BitMap->BytesPerRow * MasksI.BitMap->Rows * 4);
	if (MasksI.BitMap->Planes[1] = AllocVec (msize, MEMF_CLEAR | MEMF_CHIP))
	{
	    unpacker (MasksI.Compress, MasksI.BitMap->Planes[1], (LONG) MasksI.BitMap->BytesPerRow, (LONG) MasksI.BitMap->Rows);
	}
	else
	{
	    FreeVec (WorldI.BitMap->Planes[0]);
	    WorldI.BitMap->Planes[0] = NULL;
	    return (FALSE);
	}

	/* Map it down to the first bitplane */
	MasksI.BitMap->Planes[0] = MasksI.BitMap->Planes[1];

	return (TRUE);
    }
    return (FALSE);
}

/*****************************************************************************/

VOID FreeMapData (EdDataPtr ed)
{
    register WORD i;

    FreeVec (WorldI.BitMap->Planes[0]);
    FreeVec (MasksI.BitMap->Planes[1]);
}

/*****************************************************************************/

#define TZ_SLICE (WM_IMAGEWIDTH / 24)

VOID PutMap (EdDataPtr ed)
{
    struct Window *win = ed->ed_Window;
    struct RastPort crp;
    WORD minterm;
    WORD h;

    if (ed->ed_VZoneScroller && (ed->ed_CurrentMsg.IAddress == ed->ed_VZoneScroller))
	ed->ed_ZoneYOffset = ed->ed_CurrentMsg.Code;

    /* Only do it if it changed */
    if ((ed->ed_Zone == ed->ed_PreviousZone) &&	(ed->ed_ZoneYOffset == ed->ed_PreviousYOffset))
	return;

    ed->ed_PreviousZone    = ed->ed_Zone;
    ed->ed_PreviousYOffset = ed->ed_ZoneYOffset;

    /* Clone the rastport */
    crp = *win->RPort;

    /* Copy the map over */
    BltBitMapRastPort (&World, 0, ed->ed_ZoneYOffset + 1,
		       &crp,
		       ed->ed_WM_LEFT + 2 + win->BorderLeft,
		       WM_TOP + 2 + win->BorderTop,
		       WM_WIDTH - 6 - WM_SCROLLERWIDTH,
		       ed->ed_ZoneHeight - 4,
		       0xC0);

    if (win->WScreen->BitMap.Depth == 1)
    {
	minterm = ANBC | ABNC;
    }
    else
    {
	minterm = 0x00C0;

	/* We only want to modify the second plane */
	crp.Mask = 0x02;

	/* Erase the previous highlight */
	SetAPen (&crp, 0);

	/* Clear the previously selected zone */
	RectFill (&crp, ed->ed_WM_LEFT + win->BorderLeft + 2,
		  WM_TOP + win->BorderTop + 2,
		  ed->ed_WM_LEFT + WM_WIDTH + win->BorderLeft - 3 - WM_SCROLLERWIDTH,
		  WM_TOP + ed->ed_ZoneHeight + win->BorderTop - 3);
    }

    h = MIN (ed->ed_ZoneHeight - 4, MasksI.Height - ed->ed_ZoneYOffset - 1);
    if (h > 0)
    {
	/* Draw the highlighted time zone */
	BltBitMapRastPort (MasksI.BitMap, Masks_images[ed->ed_Zone].i_Offset, ed->ed_ZoneYOffset + 1,
			   &crp,
			   ed->ed_WM_LEFT + win->BorderLeft + Masks_images[ed->ed_Zone].i_XPad + 2,
			   WM_TOP + win->BorderTop + 2,
			   Masks_images[ed->ed_Zone].i_Width,
			   h,
			   minterm);
    }
}

/*****************************************************************************/

VOID PickTimeZone (EdDataPtr ed, USHORT x, USHORT y)
{
    register WORD tx, bx;
    struct RastPort crp;
    register WORD i;

    InitRastPort (&crp);

    x -= ed->ed_Window->BorderLeft;
    if (x < ed->ed_WM_LEFT)
	x = ed->ed_WM_LEFT;
    if (x >= ed->ed_WM_LEFT + WM_WIDTH - WM_SCROLLERWIDTH - 3)
	x = ed->ed_WM_LEFT + WM_WIDTH - WM_SCROLLERWIDTH - 4;
    x -= ed->ed_WM_LEFT + 2;

    y -= ed->ed_Window->BorderTop;
    if (y < WM_TOP)
	y = WM_TOP;
    if (y >= WM_TOP + ed->ed_ZoneHeight - 3)
	y = WM_TOP + ed->ed_ZoneHeight - 4;
    y -= WM_TOP + 2;

    crp.BitMap = MasksI.BitMap;

    for (i = 0; i < MAX_ZONES; i++)
    {
	tx = Masks_images[i].i_XPad;
	bx = tx + Masks_images[i].i_Width;

	if ((x >= tx) && (x <= bx))
	{
	    if (ReadPixel (&crp, (Masks_images[i].i_Offset + x - tx), (y + ed->ed_ZoneYOffset)) != 0)
	    {
		ed->ed_PrefsWork.lp_GMTOffset = Masks_images[i].i_Data;
		ed->ed_Zone = i;
		return;
	    }
	}
    }
}
