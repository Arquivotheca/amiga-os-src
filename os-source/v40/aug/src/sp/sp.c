/* sp.c
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "sp_rev.h"

char *version = VERSTAG;

extern struct Library *SysBase, *DOSBase;

struct Library *IntuitionBase;
struct Library *GfxBase;

#define	MINCOLORWIDTH	10
#define	MINCOLORHEIGHT	5

struct PalEx
{
    struct SignalSemaphore pe_Lock;	/* shared semaphore for arbitration 	 */
    UWORD pe_FirstFree;		/* index of first available color	 */
    UWORD pe_NFree;		/* number of colors available 		 */
    UWORD pe_FirstShared;	/* index of first shared color		 */
    UWORD pe_NShared;		/* number of shared colors		 */
    UWORD *pe_RefCnt;		/* one byte per pal entry		 */
    UBYTE *pe_AllocList;	/* allocation information		 */
    struct ViewPort *pe_ViewPort;	/* back pointer to viewport		 */
    UWORD pe_SharableColors;	/* the number of sharable colors.	 */
};


VOID SetAllFree (struct Window * win, struct ColorMap * cm, ULONG *table)
{
    struct PalEx *pe;
    UWORD i, colors;

    colors = 1 << win->WScreen->BitMap.Depth;

    /* See if it has a PalEx */
    if (pe = (struct PalEx *) cm->PalExtra)
    {
	ObtainSemaphore (&pe->pe_Lock);
	for (i = 0; i < colors; i++)
	{
	    if (pe->pe_RefCnt[i] == 0)
	    {
		switch (i)
		{
		    case 17:
		    case 18:
		    case 19:
			pe->pe_RefCnt[i]++;
			break;

		    default:
			SetRGB32 (&win->WScreen->ViewPort, i, table[0], table[1], table[2]);
			break;
		}
	    }
	}
	ReleaseSemaphore (&pe->pe_Lock);
    }
}

VOID DrawPalette (struct Window * win, struct ColorMap * cm, UWORD bestrows, UWORD bestcolumns, UWORD colorWidth, UWORD colorHeight)
{
    struct PalEx *pe;
    UWORD i, j, x, y;
    UBYTE buffer[20];
    UWORD colors;

    colors = 1 << win->WScreen->BitMap.Depth;

    /* See if it has a PalEx */
    if (pe = (struct PalEx *) cm->PalExtra)
    {
	ObtainSemaphore (&pe->pe_Lock);

#if 0
	printf ("  FirstFree := %d\n", pe->pe_FirstFree);
	printf ("    NumFree := %d\n", pe->pe_NFree);
	printf ("FirstShared := %d\n", pe->pe_FirstShared);
	printf ("  NumShared := %d\n", pe->pe_NShared);
	printf ("NumSharable := %d\n", pe->pe_SharableColors);
	printf ("\n");
#endif

	SetDrMd (win->RPort, JAM2);
	SetBPen (win->RPort, 0);

	for (i = j = 0, x = 2, y = 1; i < colors; i++)
	{
	    SetAPen (win->RPort, i);
	    RectFill (win->RPort,
		      win->BorderLeft + x + (win->RPort->TxWidth * 2) + 4,
		      win->BorderTop + y,
		      win->BorderLeft + x + (win->RPort->TxWidth * 2) + 4 + colorWidth - 1,
		      win->BorderTop + y + colorHeight - 1);

	    sprintf (buffer, "%d", i);
	    SetAPen (win->RPort, 1);

	    Move (win->RPort, win->BorderLeft + x + 2, win->BorderTop + y + win->RPort->TxBaseline + 2);
	    Text (win->RPort, buffer, strlen (buffer));

	    if (pe)
	    {
		sprintf (buffer, "%d:%d", pe->pe_RefCnt[i], pe->pe_AllocList[i]);
		Move (win->RPort, win->BorderLeft + x + (win->RPort->TxWidth * 2) + 8, win->BorderTop + y + win->RPort->TxBaseline + 2);
		Text (win->RPort, buffer, strlen (buffer));
	    }

	    y += colorHeight;
	    j++;
	    if (j >= bestrows)
	    {
		j = 0;
		y = 1;
		x += colorWidth + (win->RPort->TxWidth * 2) + 4;
	    }
	}
	ReleaseSemaphore (&pe->pe_Lock);
    }
}

int main (int argc, char **argv)
{
    struct IntuiMessage *imsg;
    struct DisplayInfo di;
    struct Rectangle rect;
    struct ColorMap *cm;
    struct Screen *scr;
    struct Window *win;
    BOOL going = TRUE;
    ULONG table[3];
    ULONG modeid;

    UWORD availWidth, availHeight;
    LONG colorWidth, colorHeight;
    UWORD bestcolumns, bestrows;
    UWORD width, height;
    UWORD columns, rows;
    ULONG resX, resY;
    LONG bestratio;
    UWORD divisor;
    UWORD colors;
    LONG ratio;

    if (IntuitionBase = OpenLibrary ("intuition.library", 39))
    {
	GfxBase = OpenLibrary ("graphics.library", 39);

	if (scr = LockPubScreen (NULL))
	{
	    cm = scr->ViewPort.ColorMap;

	    if ((modeid = GetVPModeID (&scr->ViewPort)) != INVALID_ID)
	    {
		QueryOverscan (modeid, &rect, OSCAN_TEXT);
		rect.MaxX++;
		rect.MaxY++;

		GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, modeid);
		resX = di.Resolution.x * 256;
		resY = di.Resolution.y;
		availWidth = 80;
		availHeight = rect.MaxY - (scr->BarHeight + scr->WBorBottom + 1);
		bestrows = 0;
		bestcolumns = 0;
		bestratio = 0x00800000;
		colors = 1 << scr->BitMap.Depth;

		while (colors > 0)
		{
		    divisor = 1;
		    do
		    {
			if (colors % divisor == 0)
			{
			    columns = colors / divisor;
			    rows = divisor;

			    colorWidth = availWidth / columns - 3;
			    colorHeight = availHeight / rows - 1;
			    if ((colorWidth >= MINCOLORWIDTH) && (colorHeight >= MINCOLORHEIGHT))
			    {
				ratio = (colorWidth * resX) / (colorHeight * resY);
				if (ABS (256 - ratio) < ABS (256 - bestratio))
				{
				    /* if aspect ratio better than current best */
				    bestcolumns = columns;
				    bestrows = rows;
				    bestratio = ratio;
				}
			    }
			}

			divisor++;
		    }
		    while (divisor <= colors);

		    if (bestcolumns > 0)
			break;

		    colors--;
		}

		if (bestcolumns <= 0)
		    return (NULL);

		colorWidth = availWidth / bestcolumns;
		colorHeight = availHeight / bestrows;

		width = colorWidth * bestcolumns + 5 + ((16 + 4) * 2);
		height = colorHeight * bestrows + 3;

		if (win = OpenWindowTags (NULL,
					  WA_InnerWidth, width,
					  WA_InnerHeight, height,
					  WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_VANILLAKEY,
					  WA_Title, "SP",
					  WA_DragBar, TRUE,
					  WA_DepthGadget, TRUE,
					  WA_CloseGadget, TRUE,
					  WA_SmartRefresh, TRUE,
					  WA_PubScreen, scr,
					  TAG_DONE))
		{
		    DrawPalette (win, cm, bestrows, bestcolumns, colorWidth, colorHeight);

		    while (going)
		    {
			Wait (1L << win->UserPort->mp_SigBit);

			while (imsg = (struct IntuiMessage *) GetMsg (win->UserPort))
			{
			    switch (imsg->Class)
			    {
				case IDCMP_VANILLAKEY:
				    switch (imsg->Code)
				    {
					case 'q':
					case 'Q':
					case 27:
					    going = FALSE;
					    break;

					case 'b':
					case 'B':
					    GetRGB32 (cm, 0, 1, table);
					    SetAllFree (win, cm, table);
					    break;
				    }
				    break;

				case IDCMP_MOUSEBUTTONS:
				    if (imsg->Code == SELECTDOWN)
					DrawPalette (win, cm, bestrows, bestcolumns, colorWidth, colorHeight);
				    break;

				case IDCMP_CLOSEWINDOW:
				    going = FALSE;
				    break;
			    }

			    ReplyMsg ((struct Message *) imsg);
			}
		    }

		    CloseWindow (win);
		}
	    }

	    UnlockPubScreen (NULL, scr);
	}

	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
    }
}
