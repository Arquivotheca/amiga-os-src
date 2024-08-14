/*
	Transfer routine for Canon_PJ-1080A driver.
	David Berezowski - March/88
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

	static UBYTE bit_table[8] =
		{0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};
	UBYTE *dmatrix, *bptr, *gptr, *rptr;
	UBYTE dvalue, White, Blue, Green, Red, threshold, color, bit;
	union colorEntry *ColorInt;
	UWORD x, width, sx, *sxptr, byte;

	/* pre-compute */
	/* printer specific */
	rptr = ptr + colors[0];
	gptr = ptr + colors[1];
	bptr = ptr + colors[2];
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos; /* get starting x position */
	ColorInt = PInfo->pi_ColorInt; /* get ptr to color intensities */
	sxptr = PInfo->pi_ScaleX; /* get ptr to x scale info */
	width = PInfo->pi_width; /* get # of source pixels */

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15; /* yes, so pre-compute dither value */
		do { /* for all source pixels */
			/* pre-compute intensity values for White component */
			White = ColorInt->colorByte[PCMBLACK];
			ColorInt++; /* bump ptr for next time */

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				byte = x >> 3; /* pre-compute 'byte to modify' offset */
				bit = bit_table[x & 7]; /* pre-compute 'bit to clear' value */
				if (White <= dvalue) { /* if should not render white */
					*(bptr + byte) &= bit; /* clear bit in blue buffer */
					*(gptr + byte) &= bit; /* clear bit in green buffer */
					*(rptr + byte) &= bit; /* clear bit red buffer */
				}
				++x; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			do { /* for all source pixels */
				/* pre-compute intensity values for White component */
				White = ColorInt->colorByte[PCMBLACK];
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					byte = x >> 3; /* pre-compute 'byte to modify' offset */
					/* pre-compute 'bit to clear' value */
					bit = bit_table[x & 7];
					if (White <= dmatrix[x & 3]) { /* if should not render white */
						*(bptr + byte) &= bit; /* clear bit in blue buf */
						*(gptr + byte) &= bit; /* clear bit in green buf */
						*(rptr + byte) &= bit; /* clear bit red buf */
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
		else { /* color */
			do { /* for all source pixels */
				/* pre-compute intensity values for each color component */
				White = ColorInt->colorByte[PCMBLACK];
				Blue = ColorInt->colorByte[PCMYELLOW];
				Green = ColorInt->colorByte[PCMMAGENTA];
				Red = ColorInt->colorByte[PCMCYAN];
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					byte = x >> 3; /* pre-compute 'byte to modify' offset */
					/* pre-compute 'bit to clear' value */
					bit = bit_table[x & 7];
					dvalue = dmatrix[x & 3]; /* pre-compute dither value */
					if (White <= dvalue) { /* if should not render white */
						if (Blue <= dvalue) { /* if should not render blue */
							*(bptr + byte) &= bit; /* clear bit in blue */
						}
						if (Green <= dvalue) { /* if should not render green */
							*(gptr + byte) &= bit; /* clear bit in green */
						}
						if (Red <= dvalue) { /* if should not render red */
							*(rptr + byte) &= bit; /* clear bit in red */
						}
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
