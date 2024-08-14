/*
	Transfer routine for Okidata92.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr)
struct PrtInfo *PInfo;
UWORD y;
UBYTE *ptr;
{
	UBYTE *dmatrix, Black, dvalue, threshold;
	union colorEntry *ColorInt;
	UWORD x, width, sx, *sxptr, bit;

	/* printer non-specific */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;

	/* printer specific */
	ptr += x;
	bit = 1 << (y % 7);

	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15;
		do { /* for all source pixels */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				if (Black > dvalue) {
					*ptr |= bit;
				}
				ptr++;
			} while (--sx);
		} while (--width);
	}
	else { /* greyscale */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		do { /* for all sourcev pixels */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				dvalue = dmatrix[x & 3];
				if (Black > dvalue) {
					*ptr |= bit;
				}
				ptr++;
				x++; /* done 1 more printer pixel */
			} while (--sx);
		} while (--width);
	}
}
