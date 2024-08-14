/*
	Transfer routine for HP_PaintJet driver.
	David Berezowski - October/87
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
	UBYTE *dmatrix, *yptr, *mptr, *cptr;
	UBYTE dvalue, Black, Yellow, Magenta, Cyan, threshold, color, bit;
	union colorEntry *ColorInt;
	UWORD x, width, sx, *sxptr, byte;

	/* pre-compute */
	/* printer specific */
	cptr = ptr + colors[0];
	mptr = ptr + colors[1];
	yptr = ptr + colors[2];
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos; /* get starting x position */
	ColorInt = PInfo->pi_ColorInt; /* get ptr to color intensities */
	sxptr = PInfo->pi_ScaleX; /* get ptr to x scale info */
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
				byte = x >> 3; /* pre-compute 'byte to modify' offset */
				bit = bit_table[x & 7]; /* pre-compute 'bit to clear' value */
				if (Black > dvalue) { /* if should render black */
					*(yptr + byte) &= bit; /* clear bit in yellow buffer */
					*(mptr + byte) &= bit; /* clear bit in magenta buffer */
					*(cptr + byte) &= bit; /* clear bit cyan buffer */
				}
				++x; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
			do { /* for all source pixels */
				/* pre-compute intensity values for Black component */
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					byte = x >> 3; /* pre-compute 'byte to modify' offset */
					/* pre-compute 'bit to clear' value */
					bit = bit_table[x & 7];
					if (Black > dmatrix[x & 3]) { /* if should render black */
						*(yptr + byte) &= bit; /* clear bit in yellow buf */
						*(mptr + byte) &= bit; /* clear bit in magenta buf */
						*(cptr + byte) &= bit; /* clear bit cyan buf */
					}
					++x; /* done 1 more printer pixel */
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
				ColorInt++; /* bump ptr for next time */

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					byte = x >> 3; /* pre-compute 'byte to modify' offset */
					/* pre-compute 'bit to clear' value */
					bit = bit_table[x & 7];
					dvalue = dmatrix[x & 3]; /* pre-compute dither value */
					if (Black > dvalue) { /* if should render black */
						*(yptr + byte) &= bit; /* clear bit in yellow buf */
						*(mptr + byte) &= bit; /* clear bit in magenta buf */
						*(cptr + byte) &= bit; /* clear bit cyan buf */
					}
					else  { /* black not rendered, check color */
						if (Yellow > dvalue) { /* if should render yellow */
							*(yptr + byte) &= bit; /* clear bit in yellow */
						}
						if (Magenta > dvalue) { /* if should render magenta */
							*(mptr + byte) &= bit; /* clear bit in magenta */
						}
						if (Cyan > dvalue) { /* if should render cyan */
							*(cptr + byte) &= bit; /* clear bit in cyan */
						}
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
