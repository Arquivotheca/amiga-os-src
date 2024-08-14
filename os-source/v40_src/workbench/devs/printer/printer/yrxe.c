#include <exec/types.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0	0

YReduceXEnlarge(PInfo, pass)
struct PrtInfo *PInfo;
int pass;
{
	extern struct PrinterData *PD;

	int err;
	UWORD y, ypos, sy, sy2, height, ymult, ymod, *buf, width;
	WORD ety, ety2;
	BOOL Alias, Floyd;

#if DEBUG0
	kprintf("yrxe: enter\n");
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
				kprintf("yexe: filling BotBuf @ %lx with %lx, %ld times\n",
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
			/* read entire row of color code info into RowBuf */
			MyReadPixelArray(y, PInfo, PInfo->pi_RowBuf);
		}
		/* convert entire row to YMCB intensity values */
		ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 0);
		y++;
		/* calc # of rows to compress */
		sy = sy2 = Scale(ymult, ymod, ety2, &ety);
		while (--sy2) { /* total up intensity values */
			/* read entire row of color code info in RowBuf */
			MyReadPixelArray(y, PInfo, PInfo->pi_RowBuf);
			/* convert entire row to YMCB intensity values */
			ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 1);
			y++;
		}
		sy2 = sy; /* re-load 'sy2' */
		/* average intensity values */
		AvgPixelArray(PInfo, sy);
		/* Hedley scan convert */
		if (PInfo->pi_PrefsFlags & GREY_SCALE2) {
			ScanConvertPixelArray(PInfo, y);
		}
		/* if want to fix color(s) and in HAM mode and doing a color dump */
		if ((PInfo->pi_PrefsFlags & CORRECT_RGB_MASK) &&
			(PInfo->pi_flags & PRT_HAM) &&
			(PD->pd_Preferences.PrintShade == SHADE_COLOR)) {
			FixColorsPixelArray(PInfo); /* fix all colors for this row */
		}
		/* CANNOT ALIAS AND FLOYD! */
		if (Alias) {
			AliasPixelArray(sy2 - sy, sy2, PInfo, 1);
		}
		else if (Floyd) {
			FloydPixelArray(PInfo, 1);
		}
		/* scale, dither, and render entire row of pixels */
		TransferPixelArray(ypos, PInfo, pass);
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
		height -= sy; /* done sy more screen rows */
	} while (height);
	if (!err) {
		/* dump partial buffer if neccessary */
		err = CheckBuf(ypos, PInfo->pi_render, 1);
	}
#if DEBUG0
	kprintf("yrxe: exit\n");
#endif
	return(err);
}

/*
	Calculate average values for a row of pixels
*/
AvgPixelArray(PInfo, total)
struct PrtInfo *PInfo;
UWORD total;
{
	union colorEntry *ColorInt;
	UWORD width, total2;

	ColorInt = PInfo->pi_ColorInt;
	width = PInfo->pi_width;
	total2 = total >> 1;
	 /* if color or non-color and printer has no black */
	if ((PInfo->pi_flags & PRT_BW_BLACKABLE) != PRT_BW_BLACKABLE) {
		do {
			ColorInt->colorByte[PCMBLACK] =
				(ColorInt->colorByte[PCMBLACK] + total2) / total;
			ColorInt->colorByte[PCMYELLOW] =
				(ColorInt->colorByte[PCMYELLOW] + total2) / total;
			ColorInt->colorByte[PCMMAGENTA] =
				(ColorInt->colorByte[PCMMAGENTA] + total2) / total;
			ColorInt->colorByte[PCMCYAN] =
				(ColorInt->colorByte[PCMCYAN] + total2) / total;
			ColorInt++;
		} while (--width);
	}
	else { /* non color and printer has black */
		do {
			ColorInt->colorByte[PCMBLACK] =
				(ColorInt->colorByte[PCMBLACK] + total2) / total;
			ColorInt++;
		} while (--width);
	}
}
