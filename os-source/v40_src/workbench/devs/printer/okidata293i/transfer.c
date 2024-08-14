/*
	Transfer routine for Okidata_292I driver.
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
	extern struct PrinterExtendedData *PED;

	static UWORD bit_table[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	union colorEntry *ColorInt;
	UBYTE *bptr, *yptr, *mptr, *cptr, Black, Yellow, Magenta, Cyan;
	UBYTE *dmatrix, dvalue, threshold, y16;
	UWORD x, width, sx, *sxptr, bit, x2;

	/* pre-compute */
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;

	/* printer specific */
	if (PED->ped_YDotsInch == 288) {
		BufOffset *= y & 1;
		y /= 2;
	}
	else {
		BufOffset = 0;
	}
	bit = bit_table[y & 7];
	y16 = (y % 16) >> 3;
	bptr = ptr + y16 + colors[0] + BufOffset;
	yptr = ptr + y16 + colors[1] + BufOffset;
	mptr = ptr + y16 + colors[2] + BufOffset;
	cptr = ptr + y16 + colors[3] + BufOffset;
	x2 = x * 2;

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) {
		dvalue = threshold ^ 15;
		bptr += x2;
		do { /* for all source pixels */
			/* pre-compute intensity values for Black component */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				if (Black > dvalue) {
					*bptr |= bit;
				}
				bptr += 2; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			do { /* for all source pixels */
				/* pre-compute intensity values for Black component */
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Black > dmatrix[x & 3]) {
						*(bptr + x2) |= bit;
					}
					x++; /* done 1 more printer pixel */
					x2 += 2;
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
						*(bptr + x2) |= bit;
					}
					else  { /* black not rendered */
						if (Yellow > dvalue) {
							*(yptr + x2) |= bit;
						}
						if (Magenta > dvalue) {
							*(mptr + x2) |= bit;
						}
						if (Cyan > dvalue) {
							*(cptr + x2) |= bit;
						}
					}
					++x; /* done 1 more printer pixel */
					x2 += 2;
				} while (--sx);
			} while (--width);
		}
	}
}
