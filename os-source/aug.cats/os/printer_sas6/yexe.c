#include <exec/types.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0	0
#define TCDEBUG	0

#if TCDEBUG
#include "timing.h"
#endif /* TCDEBUG */

YEnlargeXEnlarge(PInfo, pass)
struct PrtInfo *PInfo;
int pass;
{
#if TCDEBUG
	extern void ReadVertTime(), ElapsedVertTime(), PrintAvgTime();
#endif /* TCDEBUG */
	extern struct PrinterData *PD;

	int err;
	UWORD y, ypos, sy, height, ymult, ymod, *buf, sy2, width;
	WORD ety, ety2;
	BOOL Alias, Floyd;
#if TCDEBUG
	struct mytimeval RPATime, CPATime, FPATime, TPATime, CBTime, TotalTime;
	struct mytimeval rpa1, rpa2, cpa1, cpa2, fpa1, fpa2, tpa1, tpa2, cb1, cb2;
	RPATime.row = RPATime.col = RPATime.count = 0;
	CPATime.row = CPATime.col = CPATime.count = 0;
	FPATime.row = FPATime.col = FPATime.count = 0;
	TPATime.row = TPATime.col = TPATime.count = 0;
	CBTime.row = CBTime.col = CBTime.count = 0;
#endif /* TCDEBUG */
#if DEBUG0
	kprintf("yexe: enter\n");
#endif

	/* init local variables */
	err = 0;
	ypos = 0;
	y = PInfo->pi_ystart;
	height = PInfo->pi_height;
	ymult = PInfo->pi_ymult;
	ymod = PInfo->pi_ymod;
	ety = ety2 = PInfo->pi_ety;
	Alias = PInfo->pi_PrefsFlags & ANTI_ALIAS;
	Floyd = (PInfo->pi_PrefsFlags & FLOYD_DITHERING) &&
				(PD->pd_Preferences.PrintShade != SHADE_BW);

	/* for each source row */
	do {
		FixScalingVars(PInfo);
#if TCDEBUG
		Disable();
		ReadVertTime(&rpa1);
#endif /* TCDEBUG */
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
#if TCDEBUG
		ReadVertTime(&rpa2);
		Enable();
		ElapsedVertTime(&rpa1, &rpa2, &RPATime);
		Disable();
		ReadVertTime(&cpa1);
#endif /* TCDEBUG */
		/* convert entire row to YMCB intensity values */
		ConvertPixelArray(PInfo, y - PInfo->pi_ystart, 0);
#if TCDEBUG
		ReadVertTime(&cpa2);
		Enable();
		ElapsedVertTime(&cpa1, &cpa2, &CPATime);
#endif /* TCDEBUG */
		/* Hedley scan convert */
		if (PInfo->pi_PrefsFlags & GREY_SCALE2) {
			ScanConvertPixelArray(PInfo, y);
		}
		/* if want to fix color(s) and in HAM mode and doing a color dump */
		if ((PInfo->pi_PrefsFlags & CORRECT_RGB_MASK) &&
			(PInfo->pi_flags & PRT_HAM) &&
			(PD->pd_Preferences.PrintShade == SHADE_COLOR)) {
#if TCDEBUG
			Disable();
			ReadVertTime(&fpa1);
#endif /* TCDEBUG */
			FixColorsPixelArray(PInfo); /* fix all colors for this row */
#if TCDEBUG
			ReadVertTime(&fpa2);
			Enable();
			ElapsedVertTime(&fpa1, &fpa2, &FPATime);
#endif /* TCDEBUG */
		}
		sy = sy2 = Scale(ymult, ymod, ety2, &ety);
		do { /* use this row 'sy' times */
			/* CANNOT ALIAS AND FLOYD! */
			if (Alias) {
				AliasPixelArray(sy2 - sy, sy2, PInfo, 1);
			}
			else if (Floyd) {
				FloydPixelArray(PInfo, 1);
			}
#if TCDEBUG
			Disable();
			ReadVertTime(&tpa1);
#endif /* TCDEBUG */
			/* scale, dither, and render entire row of pixels */
			TransferPixelArray(ypos, PInfo, pass);
#if TCDEBUG
			ReadVertTime(&tpa2);
			Enable();
			ElapsedVertTime(&tpa1, &tpa2, &TPATime);
			Disable();
			ReadVertTime(&cb1);
#endif /* TCDEBUG */
			/* restore ptrs */
			if (Alias) {
				SwapInts(PInfo);
			}
			else if (Floyd) {
				SwapInts(PInfo);
				FloydSwapDest(PInfo);
			}
			/* restore ptrs */
			/* dump buffer if neccessary */
			if (err = CheckBuf(++ypos, PInfo->pi_render, 0)) {
				break;
			}
#if TCDEBUG
			ReadVertTime(&cb2);
			Enable();
			ElapsedVertTime(&cb1, &cb2, &CBTime);
#endif /* TCDEBUG */
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
#if TCDEBUG
	TotalTime.col = RPATime.col + CPATime.col + FPATime.col + TPATime.col +
		CBTime.col;
	TotalTime.row = TotalTime.col / 228;
	TotalTime.col = TotalTime.col % 228;
	TotalTime.row += RPATime.row + CPATime.row + FPATime.row + TPATime.row +
		CBTime.row;
	PrintAvgTime("\011MyReadPixelArray", &RPATime, &TotalTime);
	PrintAvgTime("\011ConvertPixelArray", &CPATime,  &TotalTime);
	PrintAvgTime("\011FixColorsPixelArray", &FPATime,  &TotalTime);
	PrintAvgTime("\011TransferPixelArray", &TPATime,  &TotalTime);
	PrintAvgTime("\011CheckBuf", &CBTime,  &TotalTime);
	kprintf("\011TotalTime=%ld:%ld\n", TotalTime.row, TotalTime.col);
#endif /* TCDEBUG */
#if DEBUG0
	kprintf("yexe: ypos=%ld, exit\n", ypos);
#endif
	return(err);
}
