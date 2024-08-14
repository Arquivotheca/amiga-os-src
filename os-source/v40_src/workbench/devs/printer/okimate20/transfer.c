/*
	Transfer routine for Okimate_20.
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
	union colorEntry *ColorInt;
	UBYTE *dmatrix, *yptr, *mptr, *cptr, y24, bit;
	UBYTE dvalue, Yellow, Magenta, Cyan, threshold;
	UWORD x, x3, width, sx, *sxptr;

	/* pre-compute */
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	/* pre-compute ptr to dither matrix */
	dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);

	/* printer specific */
	bit = bit_table[y & 7];
	y24 = (y % 24) >> 3;


	if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
		yptr = ptr + y24 + colors[0];
		mptr = ptr + y24 + colors[1];
		cptr = ptr + y24 + colors[2];
		x3 = x * 3;
		do { /* for all source pixels */
			/* compute intensity values for each color component */
			Yellow = ColorInt->colorByte[PCMYELLOW];
			Magenta = ColorInt->colorByte[PCMMAGENTA];
			Cyan = ColorInt->colorByte[PCMCYAN];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				dvalue = dmatrix[x & 3];
				if (Yellow > dvalue) {
					*(yptr + x3) |= bit;
				}
				if (Magenta > dvalue) {
					*(mptr + x3) |= bit;
				}
				if (Cyan > dvalue) {
					*(cptr + x3) |= bit;
				}
				++x; /* done 1 more printer pixel */
				x3 += 3;
			} while (--sx);
		} while (--width);
	}
	else { /* b&w or grey_scale printout */
		ptr += y24+ colors[0];
		/* are we thresholding? */
		if (threshold = PInfo->pi_threshold) {
			/* yes, so pre-compute dither value */
			dvalue = threshold ^ 15;
			do { /* for all source pixels */
				Yellow = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Yellow > dvalue) {
						*ptr |= bit;
					}
					ptr += 3;
				} while (--sx);
			} while (--width);
		}
		else { /* greyscale */
			do { /* for all source pixels */
				Yellow = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					dvalue = dmatrix[x & 3];
					if (Yellow > dvalue) {
						*ptr |= bit;
					}
					ptr += 3;
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
