#include <exec/types.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0	0

YEnlargeXReduce(PInfo, pass)
struct PrtInfo *PInfo;
int pass;
{
	extern struct PrinterData *PD;

	int err;
	UWORD y, ypos, sy, height, ymult, ymod, *buf, sy2, width, count;
	WORD ety, ety2;
	BOOL Alias, Floyd;

#if DEBUG0
	kprintf("yexr: enter\n");
#endif
	/* init local variables */
	err = 0;
	ypos = 0;
	y = PInfo->pi_ystart;
	ymult = PInfo->pi_ymult;
	ymod = PInfo->pi_ymod;
	ety = ety2 = PInfo->pi_ety;
	height = PInfo->pi_height;
	Alias = PInfo->pi_PrefsFlags & ANTI_ALIAS;
	Floyd = (PInfo->pi_PrefsFlags & FLOYD_DITHERING) &&
				(PD->pd_Preferences.PrintShade != SHADE_BW);

	/* for each source row */
	do {
		FixScalingVars(PInfo);
		if (Alias) {
			AliasSwapBufs(PInfo);
			/* if on last line and there is not a line below us */
			if ((height == 1) && !(PInfo->pi_flags & PRT_BELOW)) {
				width = PInfo->pi_width;
				buf = PInfo->pi_BotBuf;
#if DEBUG0
				kprintf("yexr: filling BotBuf @ %lx with %lx, %ld times\n",
					buf, ~0, width);
#endif
				do { /* fill BotBuf with an invalid value */
					*buf++ = ~0;
				} while (--width);
			}
			else {
				/* get entire row (below) of color code info into BotBuf */
				MyReadPixelArray(y + 1, PInfo, PInfo->pi_BotBuf);
			}
		}
		else {
			/* read entire row of color code info in RowBuf */
			MyReadPixelArray(y, PInfo, PInfo->pi_RowBuf);
		}
		/* convert entire row to YMCB intensity values */
		ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 0, 0);
		/* Hedley scan convert */
		if (PInfo->pi_PrefsFlags & GREY_SCALE2) {
			ScanConvertPixelArray(PInfo, y);
		}
		/* compact entire row of pixels */
		CompactPixelArray(PInfo, 0);
		/* if want to fix color(s) and in HAM mode and doing a color dump */
		if ((PInfo->pi_PrefsFlags & CORRECT_RGB_MASK) &&
			(PInfo->pi_flags & PRT_HAM) &&
			(PD->pd_Preferences.PrintShade == SHADE_COLOR)) {
			FixColorsPixelArray(PInfo); /* fix all colors for this row */
		}
		sy = sy2 = Scale(ymult, ymod, ety2, &ety);
		do { /* use this row 'sy' times */
			/* CANNOT ALIAS AND FLOYD! */
			if (Alias) {
				AliasPixelArray(sy2 - sy, sy2, PInfo, 0);
			}
			else if (Floyd) {
				FloydPixelArray(PInfo, 0);
			}
			/* scale(1:1), dither, and render entire row of pixels */
			TransferPixelArray(ypos, PInfo, pass);
			/* restore ptrs */
			/* restore ptrs */
			if (Alias) {
				SwapInts(PInfo);
			}
			else if (Floyd) {
				SwapInts(PInfo);
				FloydSwapDest(PInfo);
			}
			/* dump buffer if neccessary */
			if (err = CheckBuf(++ypos, PInfo->pi_render, 0)) {
				break;
			}
		} while (--sy);
		if (err) {
			break;
		}
		y++;
	} while (--height);
	if (!err) {
		/* dump partial buffer if neccessary */
		err = CheckBuf(ypos, PInfo->pi_render, 1);
	}
#if DEBUG0
	kprintf("yexr: exit\n");
#endif
	return(err);
}
