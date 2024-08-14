/*
	Transfer routine for CBM_Epson driver.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, BufOffset)
struct PrtInfo *PInfo;
UWORD y;	/* row # */
UBYTE *ptr;	/* ptr to buffer */
ULONG BufOffset;
{
	extern struct PrinterExtendedData *PED;

	UBYTE *dmatrix, Black, dvalue, threshold;
	union colorEntry *ColorInt;
	UWORD x, width, sx, *sxptr, bit;

	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos; /* x offset */
	ColorInt = PInfo->pi_ColorInt; /* array of color intensities */
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width; /* width of SOURCE pixels */

	/* printer specific code. */
	if (PED->ped_YDotsInch == 216) {
		BufOffset *= y % 3;
		y /= 3;
	}
	else if (PED->ped_YDotsInch == 144) {
		BufOffset *= y & 1;
		y /= 2;
	}
	else {
		BufOffset = 0;
	}
	bit = 1 << ((y & 7) ^ 7); /* bit value is a constant */
	ptr += x + BufOffset;

	if (threshold = PInfo->pi_threshold) { /* if thresholding */
		dvalue = threshold ^ 15; /* dither value is a constant */
		do { /* loop through all SOURCE pixels */
			/* get color info */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++; /* advance to next SOURCE pixel */

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				/* if pixel should be rendered */
				if (Black > dvalue) {
					*ptr |= bit; /* set bit in the buf */
				}
				ptr++; /* advance to next DESTINATION byte */
			} while (--sx);
		} while (--width);
	}
	/* not thresholding, grey-scaling */
	else { /* init pointer to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		do { /* loop through all SOURCE pixels */
			/* get color info */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++; /* advance to next SOURCE pixel */

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				dvalue = dmatrix[x & 3]; /* get dither value */
				/* if pixel should be rendered */
				if (Black > dvalue) {
					*ptr |= bit; /* set bit in the buf */
				}
				ptr++; /* advance to next DESTINATION byte */
				x++; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
}
