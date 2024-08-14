#include <exec/types.h>
#include <graphics/rastport.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "printer_iprotos.h"

#define FCYDEBUG	0
#define FCRDEBUG	0

extern struct PrinterExtendedData *PED;

/*
	COLOR CORRECTION ALGORITHM - HOW IT WORKS

	Note :	Y - Yellow, M - Magenta, C - Cyan
			R - Red, G - Green, B - Blue
	The formula works as follows (for B):
	B15 is a combination of M15/C15.  However, this leads to a purple
	rather than a blue.  Therefore B15 is convoluted to be M8/C15 which
	produces a relatively solid blue shade.(ie. M' = M - C / 2).
	As we get away from M15/C15 (Blue) and approach	M15/C0 (Magenta)
	or M0/C15 (Cyan) we want to approach subtracting nothing from M.
	Since YMC is black we first get the minimum of YMC and subtract it
	from YMC.  This reduces the color to only 2 components instead of the
	original 3.  We now take the minimum of the two components we are
	interested in (M and C in this case), divide it by 2 and subtract it
	from the ORIGINAL component that we are interested in fixing
	(M in this case).  This procedure only affects the shade we are
	interested in fixing (Blue in this case, ie. combinations of
	M and C) and leaves all other shades unchanged.
*/

/*
	Fixes the colors in the printer color map.
*/
FixColorMap(PInfo)
struct PrtInfo *PInfo;
{
	if (PED->ped_ColorClass & PCC_ADDITIVE) {
		FixColorsRGB(PInfo, PInfo->pi_ColorMap,
			1 << PInfo->pi_rp->BitMap->Depth);
	}
	else {
		FixColorsYMC(PInfo, PInfo->pi_ColorMap,
			1 << PInfo->pi_rp->BitMap->Depth);
	}
}

/*
	Fixes the colors of a row of pixels.
*/
FixColorsPixelArray(PInfo)
struct PrtInfo *PInfo;
{
	if (PED->ped_ColorClass & PCC_ADDITIVE) {
		FixColorsRGB(PInfo, PInfo->pi_ColorInt, PInfo->pi_width);
	}
	else {
		FixColorsYMC(PInfo, PInfo->pi_ColorInt, PInfo->pi_width);
	}
}

FixColorsYMC(PInfo, ce, n)
struct PrtInfo *PInfo;
union colorEntry *ce;
int n;
{
	UWORD black, yellowblack, magentablack, cyanblack;
	UWORD rflag, gflag, bflag;

	rflag = (PInfo->pi_PrefsFlags & CORRECT_RED) ? 1 : 0;
	gflag = (PInfo->pi_PrefsFlags & CORRECT_GREEN) ? 1 : 0;
	bflag = (PInfo->pi_PrefsFlags & CORRECT_BLUE) ? 1 : 0;

	do {
#if FCYDEBUG
		kprintf("(YMCK)before=%01lx%01lx%01lx%01lx, ",
			ce->colorByte[PCMYELLOW], ce->colorByte[PCMMAGENTA],
			ce->colorByte[PCMCYAN], ce->colorByte[PCMBLACK]);
#endif /* FCYDEBUG */
		/* get original black */
		black = ce->colorByte[PCMBLACK];
		/* get original yellow - black */
		yellowblack = (ce->colorByte[PCMYELLOW] - black) / 2;
		/* get original magenta - black */
		magentablack = (ce->colorByte[PCMMAGENTA] - black) / 2;
		/* get original cyan - black */
		cyanblack = (ce->colorByte[PCMCYAN] - black) / 2;
		if (rflag) {	/* if want to fix red */
			ce->colorByte[PCMYELLOW] -=
				yellowblack < magentablack ?
					yellowblack : magentablack;
		}
		if (gflag) {	/* if want to fix green */
			ce->colorByte[PCMCYAN] -=
				cyanblack < yellowblack ?
					cyanblack : yellowblack;
		}
		if (bflag) {	/* if want to fix blue */
			ce->colorByte[PCMMAGENTA] -=
				magentablack < cyanblack ?
					magentablack : cyanblack;
		}
#if FCYDEBUG
		kprintf("(YMCK)after=%01lx%01lx%01lx%01lx\n",
			ce->colorByte[PCMYELLOW], ce->colorByte[PCMMAGENTA],
			ce->colorByte[PCMCYAN], ce->colorByte[PCMBLACK]);
#endif /* FCYDEBUG */
		ce++;
	} while (--n);
}

FixColorsRGB(PInfo, ce, n)
struct PrtInfo *PInfo;
union colorEntry *ce;
int n;
{
	UWORD red, green, blue, rflag, gflag, bflag;

	rflag = (PInfo->pi_PrefsFlags & CORRECT_RED) ? 1 : 0;
	gflag = (PInfo->pi_PrefsFlags & CORRECT_GREEN) ? 1 : 0;
	bflag = (PInfo->pi_PrefsFlags & CORRECT_BLUE) ? 1 : 0;

	do {
#if FCRDEBUG
		kprintf("(RGBK)before=%01lx%01lx%01lx%01lx, ",
			ce->colorByte[PCMRED], ce->colorByte[PCMGREEN],
			ce->colorByte[PCMBLUE], ce->colorByte[PCMBLACK]);
#endif /* FCRDEBUG */
		red = ce->colorByte[PCMRED];
		green = ce->colorByte[PCMGREEN];
		blue = ce->colorByte[PCMBLUE];
		/* if want to fix red and there's some red in this color */
		if (rflag && red>green && red>blue) {
			red -= green > blue ? green : blue;
			ce->colorByte[PCMBLUE] += red / 2;
		}
		/* if want to fix green and there's some green in this color */
		if (gflag && green>red && green>blue) {
			green -= red > blue ? red : blue;
			ce->colorByte[PCMRED] += green / 2;
		}
		/* if want to fix blue and there's some blue in this color */
		if (bflag && blue>red && blue>green) {
			blue -= red > green ? red : green;
			ce->colorByte[PCMGREEN] += blue / 2;
		}
#if FCRDEBUG
		kprintf("(RGBK)after=%01lx%01lx%01lx%01lx\n",
			ce->colorByte[PCMRED], ce->colorByte[PCMGREEN],
			ce->colorByte[PCMBLUE], ce->colorByte[PCMBLACK]);
#endif /* FCRDEBUG */
		ce++;
	} while (--n);
}
