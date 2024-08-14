#include <exec/types.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "printer_iprotos.h"

#define DEBUGAPA	0
#define DEBUGAPA2	0

AliasSwapBufs(PInfo)
struct PrtInfo *PInfo;
{
	UWORD *buf;

	buf = PInfo->pi_TopBuf;
	PInfo->pi_TopBuf = PInfo->pi_RowBuf;
	PInfo->pi_RowBuf = PInfo->pi_BotBuf;
	PInfo->pi_BotBuf = buf;
}

AliasPixelArray(y, sy, PInfo, flag)
UWORD y, sy;
struct PrtInfo *PInfo;
int flag;
{
	union colorEntry *ColorInt, *Dest1Int;
	UWORD *RowBuf, *TopBuf, *BotBuf;
	UWORD width, x, x2, *sxptr, sx, sx1, sy1, sxy1, sxy2, sy2;
	UWORD left, right, upleft, upright;
	UWORD up, dn, dnleft, dnright;
#if DEBUGAPA
	union colorEntry *Dest2Int;
#endif DEBUGAPA

	if (flag) { /* yexe or yrxe */
		FixScalingVars(PInfo);
	}

	RowBuf = PInfo->pi_RowBuf;
	TopBuf = PInfo->pi_TopBuf;
	BotBuf = PInfo->pi_BotBuf;
	ColorInt = PInfo->pi_ColorInt;
	Dest1Int = PInfo->pi_Dest1Int;
	width = PInfo->pi_width;
	sxptr = PInfo->pi_ScaleX;

#if DEBUGAPA
	kprintf("APA: enter, y=%ld, sy=%ld, RowBuf=%lx, TopBuf=%lx, BotBuf=%lx\n",
		y, sy, RowBuf, TopBuf, BotBuf);
	kprintf("ColorInt=%lx, Dest1Int=%lx, width=%ld, flag=%ld\n",
		ColorInt, Dest1Int, width, flag);
#endif DEBUGAPA

	sx = *sxptr++; /* transfer first pixel; ie. do not alias it */
	do {
		Dest1Int->colorLong = ColorInt->colorLong;
		Dest1Int++; /* advance to next destination pixel */
	} while (--sx);
	RowBuf++; /* advance to next source pixel on this line */
	TopBuf++; /* advance to next source pixel on line above us */
	BotBuf++; /* advance to next source pixel on line below us */
	ColorInt++; /* advance to next source pixel (color intensity) */
	width -=2; /* less the leftmost and rightmost pixel */
#if DEBUGAPA
	kprintf("APA: transferred first pixel, now doing main logic\n");
#endif DEBUGAPA

	do { /* for each source pixel (less the leftmost and rightmost one) */
#if DEBUGAPA
		kprintf("APA: computing directions\n");
#endif DEBUGAPA
		up = *TopBuf;
		dn = *BotBuf;
		left = *(RowBuf - 1);
		right = *(RowBuf + 1);
		upleft = *(TopBuf - 1);
		upright = *(TopBuf + 1);
		dnleft = *(BotBuf - 1);
		dnright = *(BotBuf + 1);

#if DEBUGAPA
		kprintf("APA: scaling pixel\n");
#endif DEBUGAPA
		x2 = sx = *sxptr++; /* use this pixel 'sx' times */

#if DEBUGAPA
		kprintf("APA: duplicating pixel %ld times\n", x2);
#endif DEBUGAPA
		do {
			Dest1Int->colorLong = ColorInt->colorLong;
			Dest1Int++; /* advance to next destination pixel */
		} while (--x2);

#if DEBUGAPA
		kprintf("APA: computing 'sx' values, Dest1Int=%lx\n", Dest1Int);
#endif DEBUGAPA
		sx1 = sx - 1;
		sy1 = sy - 1;
		sxy1 = (sx1 * sy1) / 2; /* MUST divide AFTER multiple */
		sxy2 = (sx1 * sy1 * 3) / 2; /* MUST divide AFTER multiple */
		sy2 = sy1 / 2;

		if (sy1) { /* if not 1:1 y scaling */
#if DEBUGAPA
			kprintf("APA: not 1:1 y scaling\n");
#endif DEBUGAPA
			/* top half and not 2:1 y scaling */
			if ((y <= sy2) && (sy1 > 1)) {
#if DEBUGAPA
				kprintf("APA: top half and not 2:1 y scaling\n");
#endif DEBUGAPA
				/* if diagonal upleft */
				if ((left == up) && ((upright != up) || (dnleft != up))) {
#if DEBUGAPA
					kprintf("APA: diagonal upleft\n");
					Dest2Int = Dest1Int;
#endif DEBUGAPA
					Dest1Int -= sx;
					x = x2 = (sxy1 - y * sx1) / sy1;
					while (x--) {
						/* use pixel to the left of us */
						Dest1Int->colorLong = (ColorInt - 1)->colorLong;
						Dest1Int++; /* advance to next destination pixel */
					}
					x = sx - x2;
					Dest1Int += x;
#if DEBUGAPA
					if (Dest1Int != Dest2Int) {
						kprintf("ERROR: Dest1Int=%lx, Dest2Int=%lx\n",
							Dest1Int, Dest2Int);
	kprintf("sx=%ld, sx1=%ld, sy1=%ld, sxy1=%ld, sxy2=%ld, sy2=%ld, x2=%ld\n",
		sx, sx1, sy1, sxy1, sxy2, sy2, x2);
						Debug();
					}
#endif DEBUGAPA
				}
				/* if diagonal upright */
				if ((right == up) && ((upleft != up) || (dnright != up))) {
#if DEBUGAPA
					kprintf("APA: diagonal upright\n");
					Dest2Int = Dest1Int;
#endif DEBUGAPA
					Dest1Int -= sx;
					x = x2 = (sxy1 + y * sx1) / sy1 + 1;
					Dest1Int += x;
					x = sx - x2;
					while (x--) {
						/* use pixel to the right of us */
						Dest1Int->colorLong = (ColorInt + 1)->colorLong;
						Dest1Int++; /* advance to next destination pixel */
					}
#if DEBUGAPA
					if (Dest1Int != Dest2Int) {
						kprintf("ERROR: Dest1Int=%lx, Dest2Int=%lx\n",
							Dest1Int, Dest2Int);
	kprintf("sx=%ld, sx1=%ld, sy1=%ld, sxy1=%ld, sxy2=%ld, sy2=%ld, x2=%ld\n",
		sx, sx1, sy1, sxy1, sxy2, sy2, x2);
						Debug();
					}
#endif DEBUGAPA
				}
			}
			else if (y > sy2) { /* bottom half */
#if DEBUGAPA
				kprintf("APA: bottom half\n");
#endif DEBUGAPA
				/* if diagonal downleft */
				if ((left == dn) && ((dnright != dn) || (upleft != dn))) {
#if DEBUGAPA
					kprintf("APA: diagonal downleft\n");
					Dest2Int = Dest1Int;
#endif DEBUGAPA
					Dest1Int -= sx;
					x = x2 = (-sxy1 + y * sx1) / sy1;
					while (x--) {
						/* use pixel to the left of us */
						Dest1Int->colorLong = (ColorInt - 1)->colorLong;
						Dest1Int++; /* advance to next destination pixel */
					}
					x = sx - x2;
					Dest1Int += x;
#if DEBUGAPA
					if (Dest1Int != Dest2Int) {
						kprintf("ERROR: Dest1Int=%lx, Dest2Int=%lx\n",
							Dest1Int, Dest2Int);
	kprintf("sx=%ld, sx1=%ld, sy1=%ld, sxy1=%ld, sxy2=%ld, sy2=%ld, x2=%ld\n",
		sx, sx1, sy1, sxy1, sxy2, sy2, x2);
						Debug();
					}
#endif DEBUGAPA
				}
				/* if diagonal downright */
				if ((right == dn) && ((dnleft != dn) || (upright != dn))) {
#if DEBUGAPA
					kprintf("APA: diagonal downright\n");
					Dest2Int = Dest1Int;
#endif DEBUGAPA
					Dest1Int -= sx;
					x = x2 = (sxy2 - y * sx1) / sy1 + 1;
					Dest1Int += x;
					x = sx - x2;
					while (x--) {
						/* use pixel to the right of us */
						Dest1Int->colorLong = (ColorInt + 1)->colorLong;
						Dest1Int++; /* advance to next destination pixel */
					}
#if DEBUGAPA
					if (Dest1Int != Dest2Int) {
						kprintf("ERROR: Dest1Int=%lx, Dest2Int=%lx\n",
							Dest1Int, Dest2Int);
	kprintf("sx=%ld, sx1=%ld, sy1=%ld, sxy1=%ld, sxy2=%ld, sy2=%ld, x2=%ld\n",
		sx, sx1, sy1, sxy1, sxy2, sy2, x2);
						Debug();
					}
#endif DEBUGAPA
				}
			}
		}
		RowBuf++; /* advance to next source pixel on this line */
		TopBuf++; /* advance to next source pixel on line above us */
		BotBuf++; /* advance to next source pixel on line below us */
		ColorInt++; /* advance to next source pixel (color intensity) */
#if DEBUGAPA
		kprintf("APA: bottom of loop, bumping pointers\n");
		kprintf("RowBuf=%lx, TopBuf=%lx, BotBuf=%lx\n",
			RowBuf, TopBuf, BotBuf);
		kprintf("Dest1Int=%lx, ColorInt=%lx\n", Dest1Int, ColorInt);
#endif DEBUGAPA
	} while (--width);
#if DEBUGAPA
	kprintf("APA: done main logic, transferring last pixel\n");
#endif DEBUGAPA

	sx = *sxptr++; /* transfer last pixel; ie. do not alias it */
	do {
		Dest1Int->colorLong = ColorInt->colorLong;
		Dest1Int++; /* advance to next destination pixel */
	} while (--sx);

#if DEBUGAPA
	kprintf("elapsed : RowBuf=%ld, ColorInt=%ld, Dest1Int=%ld\n",
		RowBuf - PInfo->pi_RowBuf, ColorInt - PInfo->pi_ColorInt,
		Dest1Int - PInfo->pi_Dest1Int);
#endif DEBUGAPA

	SwapInts(PInfo); /* swap ColorInt and Dest1Int ptrs */

	if (flag) {
		Force1to1Scaling(PInfo); /* force 1:1 scaling */
	}

#if DEBUGAPA
	kprintf("APA: bye\n");
#endif DEBUGAPA
}
