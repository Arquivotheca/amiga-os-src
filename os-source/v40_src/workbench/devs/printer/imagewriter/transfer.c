/*
	Transfer routine for ImagewriterII driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, colors, BufOffset)
struct PrtInfo *PInfo;
UWORD y;	/* row # */
UBYTE *ptr;	/* ptr to buffer */
UWORD *colors; /* indexes to color buffers */
ULONG BufOffset;
{
	extern struct PrinterData *PD;
	extern struct PrinterData *PED;

	union colorEntry *ColorInt;
	UBYTE *bptr, *yptr, *mptr, *cptr, Black, Yellow, Magenta, Cyan;
	UBYTE *dmatrix, dvalue, threshold;
	UWORD x, width, sx, *sxptr, bit;

	/* printer non-specific */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;

	/* printer specific */
	if (PED->ped_YDotsInch == 144) {
		BufOffset *= y & 1;
		y /= 2;
	}
	else {
		BufOffset = 0;
	}
	bit = 1 << (y & 7);
	bptr = ptr + colors[0] + BufOffset;
	yptr = ptr + colors[1] + BufOffset;
	mptr = ptr + colors[2] + BufOffset;
	cptr = ptr + colors[3] + BufOffset;

	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15;
		bptr += x;
		do {
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				if (Black > dvalue) {
					*bptr |= bit;
				}
				bptr++; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else {
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			do {
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Black > dmatrix[x & 3]) {
						*(bptr + x) |= bit;
					}
					x++; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
		else { /* color */
			do {
				Black = ColorInt->colorByte[PCMBLACK];
				Yellow = ColorInt->colorByte[PCMYELLOW];
				Magenta = ColorInt->colorByte[PCMMAGENTA];
				Cyan = ColorInt->colorByte[PCMCYAN];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					dvalue = dmatrix[x & 3];
					if (Black > dvalue) {
						*(bptr + x) |= bit;
					}
					else  { /* black not rendered */
						if (Yellow > dvalue) {
							*(yptr + x) |= bit;
						}
						if (Magenta > dvalue) {
							*(mptr + x) |= bit;
						}
						if (Cyan > dvalue) {
							*(cptr + x) |= bit;
						}
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
