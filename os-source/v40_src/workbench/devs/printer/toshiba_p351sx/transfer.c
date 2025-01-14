/*
	Transfer routine for Toshiba_P351SX driver.
	David Berezowski - April/88.
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
	extern struct PrinterExtendedData *PED;

	static UWORD bit_table[6] = {32, 16, 8, 4, 2, 1};
	union colorEntry *ColorInt;
	UBYTE *bptr, *yptr, *mptr, *cptr, Black, Yellow, Magenta, Cyan;
	UBYTE *dmatrix, dvalue, threshold;
	UWORD x, width, sx, *sxptr, color, bit, x4;

	/* pre-compute */
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	/* printer specific */
	x4 = x * 4;
	if (PED->ped_YDotsInch == 360) {
		y /= 2;
		BufOffset *= y & 1;
	}
	else {
		BufOffset = 0;
	}
	bit = bit_table[y % 6];
	ptr += (y % 24) / 6;
	bptr = ptr + colors[0] + BufOffset;
	yptr = ptr + colors[1] + BufOffset;
	mptr = ptr + colors[2] + BufOffset;
	cptr = ptr + colors[3] + BufOffset;

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15;
		bptr += x4;
		do { /* for all source pixels */
			/* compute intensity vals for each Black component */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				if (Black > dvalue) {
					*bptr |= bit;
				}
				bptr += 4;
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) { 
			bptr += x4;
			do { /* for all source pixels */
				/* compute intensity vals for each Black component */
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Black > dmatrix[x & 3]) {
						*bptr |= bit;
					}
					x++; /* done 1 more printer pixel */
					bptr += 4;
				} while (--sx);
			} while (--width);
		}
		else { /* color */
			do { /* for all source pixels */
				/* pre-compute intensity values for each color component */
				Black = ColorInt->colorByte[PCMBLACK];
				Yellow = ColorInt->colorByte[PCMYELLOW];
				Magenta = ColorInt->colorByte[PCMMAGENTA];
				Cyan = ColorInt->colorByte[PCMCYAN];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					dvalue = dmatrix[x & 3];
					if (Black > dvalue) {
						*(bptr + x4) |= bit;
					}
					else  { /* black not rendered */
						if (Yellow > dvalue) {
							*(yptr + x4) |= bit;
						}
						if (Magenta > dvalue) {
							*(mptr + x4) |= bit;
						}
						if (Cyan > dvalue) {
							*(cptr + x4) |= bit;
						}
					}
					x++; /* done 1 more printer pixel */
					x4 += 4;
				} while (--sx);
			} while (--width);
		}
	}
}
