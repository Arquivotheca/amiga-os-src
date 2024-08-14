/*
	Transfer routine for Xerox_4020 driver.
	David Berezowski - October/87.
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, colors)
struct PrtInfo *PInfo;
UWORD y;	/* row # */
UBYTE *ptr;	/* ptr to buffer */
UWORD *colors; /* indexes to color buffers */
{
	extern struct PrinterData *PD;

	static UWORD bit_table[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	UBYTE *dmatrix, *bptr, *yptr, *mptr, *cptr;
	UBYTE dvalue, Black, Yellow, Magenta, Cyan, threshold;
	UBYTE bit, y3;
	union colorEntry *ColorInt;
	UWORD x, x3, width, sx, *sxptr;

	/* pre-compute */
	/* printer specific */
	y3 = y & 3;
	bptr = ptr + colors[y3];
	yptr = ptr + colors[4 + y3];
	mptr = ptr + colors[8 + y3];
	cptr = ptr + colors[12 + y3];
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos; /* get starting x position */
	ColorInt = PInfo->pi_ColorInt; /* get ptr to color intensities */
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width; /* get # of source pixels */

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15; /* yes, so pre-compute dither value */
		do { /* for all source pixels */
			/* pre-compute intensity values for Black component */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++; /* bump ptr for next time */

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				/* if should render black */
				if (Black > dvalue) {
					/* set bit in black buffer */
					*(bptr + (x >> 3)) |= bit_table[x & 7];
				}
				++x; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + (y3 << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			do { /* for all source pixels */
				/* pre-compute intensity values for Black */
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					/* if should render black */
					if (Black > dmatrix[x & 3]) {
						/* set bit in black buffer */
						*(bptr + (x >> 3)) |=
							bit_table[x & 7];
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
		else { /* color */
			do { /* for all source pixels */
				/* pre-compute intensity vals for each color */
				Black = ColorInt->colorByte[PCMBLACK];
				Yellow = ColorInt->colorByte[PCMYELLOW];
				Magenta = ColorInt->colorByte[PCMMAGENTA];
				Cyan = ColorInt->colorByte[PCMCYAN];
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					/* pre-compute 'byte to set' value */
					x3 = x >> 3;
					/* pre-compute 'bit to set' val */
					bit = bit_table[x & 7];
					/* pre-compute dither value */
					dvalue = dmatrix[x & 3];
					/* if should render black */
					if (Black > dvalue) {
						/* set bit in black buffer */
						*(bptr + x3) |= bit;
					}
					/* black not rendered, check color */
					else  {
						/* if should render yellow */
						if (Yellow > dvalue) {
							/* set bit in Y buf */
							*(yptr + x3) |= bit;
						}
						/* if should render magenta */
						if (Magenta > dvalue) {
							/* set bit in M buf */
							*(mptr + x3) |= bit;
						}
						/* if should render cyan */
						if (Cyan > dvalue) {
							/* set bit in C buf */
							*(cptr + x3) |= bit;
						}
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
