/*
	Transfer routine for CalComp ColorMaster driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, pcm)
struct PrtInfo *PInfo;	/* ptr to PrtInfo struct */
UWORD y;				/* row # */
UBYTE *ptr;				/* ptr to buffer */
UWORD pcm;				/* pcm color code (Yellow, Magenta, or Cyan) */
{
	extern struct PrinterData *PD;

	static UBYTE bit_table[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	UBYTE *dmatrix, Color, dvalue, threshold, bit;
	union colorEntry *ColorInt;
	UWORD x, width, sx, *sxptr;

	/* pre-compute */
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos; /* get starting x position */
	ColorInt = PInfo->pi_ColorInt; /* get ptr to color intensities */
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width; /* get # of source pixels */

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) {
		dvalue = threshold ^ 15; /* yes, so pre-compute dither value */
		do { /* for all source pixels */
			/* pre-compute intensity value for this color */
			Color = ColorInt->colorByte[pcm];
			ColorInt++; /* bump ptr for next time */

			sx = *sxptr++;

			/* dither and render pixel */
			do { /* use this pixel 'sx' times */
				if (Color > dvalue) { /* if we should render this color */
					/* set bit in buffer */
					*(ptr + (x >> 3)) |= bit_table[x & 7];
				}
				++x; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		do { /* for all source pixels */
			/* pre-compute intensity value for this color */
			Color = ColorInt->colorByte[pcm];
			ColorInt++; /* bump ptr for next time */

			sx = *sxptr++;
	
			/* dither and render pixel */
			do { /* use this pixel 'sx' times */
				dvalue = dmatrix[x & 3]; /* pre-compute dither value */
				if (Color > dvalue) { /* if we should render this color */
					/* set bit in buffer */
					*(ptr + (x >> 3)) |= bit_table[x & 7];
				}
				++x; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
}
