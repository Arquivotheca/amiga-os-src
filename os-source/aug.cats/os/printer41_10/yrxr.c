#include <exec/types.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0	0
#define DEBUG1  0

YReduceXReduce(PInfo, pass)
struct PrtInfo *PInfo;
int pass;
{
	extern struct PrinterData *PD;

	int err;
	UWORD y, ypos, sy, sy2, height, ymult, ymod, count;
	WORD ety, ety2;
	BOOL Floyd;

#if DEBUG0
	kprintf("yrxr: enter\n");
#endif
	/* init local variables */
	err = 0;
	ypos = 0;
	y = PInfo->pi_ystart;
	ymult = PInfo->pi_ymult;
	ymod = PInfo->pi_ymod;
	ety = ety2 = PInfo->pi_ety;
	height = PInfo->pi_height;
	Floyd = (PInfo->pi_PrefsFlags & FLOYD_DITHERING) &&
				(PD->pd_Preferences.PrintShade != SHADE_BW);
	/* can not anti-alias a yrxr image */
	PInfo->pi_PrefsFlags &= ~ANTI_ALIAS;

	/* for each source row */
	do {
		FixScalingVars(PInfo);
		/* read entire row of color code info in RowBuf */
		MyReadPixelArray(y, PInfo, PInfo->pi_RowBuf);
		/* convert entire row to YMCB intensity values */
		count = 0;
		ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 0, count);
		y++;
		/* calc # of rows to compress */
		sy = sy2 = Scale(ymult, ymod, ety2, &ety);
#if DEBUG1
	kprintf("yrxr: sy=%ld ymult=%ld ymod=%ld ety2=%ld ety=%ld\n",
			sy, ymult, ymod, ety2, ety);
#endif

		while (--sy2) { /* total up intensity values */
			/* read entire row of color code info in RowBuf */
			MyReadPixelArray(y, PInfo, PInfo->pi_RowBuf);
			/* convert entire row to YMCB intensity values */
			ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 1, ++count);
			y++;
		}
		/* Hedley scan convert */
		if (PInfo->pi_PrefsFlags & GREY_SCALE2) {
			ScanConvertPixelArray(PInfo, y);
		}
		/* compact entire row of pixels */
		CompactPixelArray(PInfo, sy);
		/* if want to fix color(s) and in HAM mode and doing a color dump */
		if ((PInfo->pi_PrefsFlags & CORRECT_RGB_MASK) &&
			(PInfo->pi_flags & PRT_HAM) &&
			(PD->pd_Preferences.PrintShade == SHADE_COLOR)) {
			FixColorsPixelArray(PInfo); /* fix all colors for this row */
		}
		if (Floyd) {
			FloydPixelArray(PInfo, 0);
		}
		/* scale(1:1), dither, and render entire row of pixels */
		TransferPixelArray(ypos, PInfo, pass);
		/* restore ptrs */
		if (Floyd) {
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
	kprintf("yrxr: exit\n");
#endif
	return(err);
}
