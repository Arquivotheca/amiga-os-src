/*
	Transfer routine for CalComp_ColorMaster2.
	David Berezowski - October/87
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, colors, RowSize)
struct PrtInfo *PInfo;
UWORD y;	/* row # */
UBYTE *ptr;	/* ptr to buffer */
ULONG *colors; /* indexes to color buffers */
UWORD RowSize; /* size of one row buffer */
{
	extern struct PrinterData *PD;

	static UBYTE bit_table[] = {128, 64, 32, 16, 8, 4, 2, 1};
	union colorEntry *ColorInt;
	UBYTE *dmatrix, *yptr, *mptr, *cptr;
	UBYTE dvalue, Yellow, Magenta, Cyan, threshold, bit;
	UWORD x, x3, width, sx, *sxptr;

	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);

	if (PD->pd_Preferences.PrintShade == SHADE_COLOR) { /* color */
		yptr = ptr + colors[0] + y * RowSize;
		mptr = ptr + colors[1] + y * RowSize;
		cptr = ptr + colors[2] + y * RowSize;
		do { /* for each source pixel */
			/* pre-compute intensity values for each color component */
			Yellow = ColorInt->colorByte[PCMYELLOW];
			Magenta = ColorInt->colorByte[PCMMAGENTA];
			Cyan = ColorInt->colorByte[PCMCYAN];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				dvalue = dmatrix[x & 3];
				x3 = x >> 3;
				bit = bit_table[x & 7];
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
			} while (--sx);
		} while (--width);
	}
	else { /* doing a b&w or greyscale printout */
		ptr += colors[0] + y * RowSize;
		if (threshold = PInfo->pi_threshold) { /* if thresholding */
			dvalue = threshold ^ 15;
			do { /* for each source pixel */
				/* pre-compute intensity values for Black component */
				Yellow = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Yellow > dvalue) {
						*(ptr + (x >> 3)) |= bit_table[x & 7];
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
		else { /* grey-scale */
			do { /* for each source pixel */
				/* pre-compute intensity values for Black component */
				Yellow = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
       	            dvalue = dmatrix[x & 3];
					if (Yellow > dvalue) {
						*(ptr + (x >> 3)) |= bit_table[x & 7];
					}
					++x; /* done 1 more printer pixel */
				} while (--sx);
			} while (--width);
		}
	}
}
