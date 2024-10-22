/*
	Transfer routine for Nec24 driver.
	David Berezowski - March/88.
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"
#include "../printer/prtgfx.h"

Transfer(PInfo, y, ptr, colors, Buf2Offset)
struct PrtInfo *PInfo;
UWORD y;	/* row # */
UBYTE *ptr;	/* ptr to buffer */
UWORD *colors; /* indexes to color buffers */
ULONG Buf2Offset;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static UWORD bit_table[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	union colorEntry *ColorInt;
	UBYTE *bptr, *yptr, *mptr, *cptr, Black, Yellow, Magenta, Cyan;
	UBYTE *dmatrix, dvalue, threshold;
	UWORD x, width, sx, *sxptr, color, bit, x3, ymod;

	/* pre-compute */
	/* printer non-specific, MUST DO FOR EVERY PRINTER */
	x = PInfo->pi_xpos;
	ColorInt = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	/* printer specific */
	x3 = x * 3;
	ymod = y % PED->ped_NumRows;
	if (PED->ped_YDotsInch == 360) {
		ymod /= 2;
		Buf2Offset *= y & 1;
	}
	else {
		Buf2Offset = 0;
	}
	bit = bit_table[ymod & 7];
	ptr += ymod / 8;
	bptr = ptr + colors[0] + Buf2Offset;
	yptr = ptr + colors[1] + Buf2Offset;
	mptr = ptr + colors[2] + Buf2Offset;
	cptr = ptr + colors[3] + Buf2Offset;

	/* pre-compute threshold; are we thresholding? */
	if (threshold = PInfo->pi_threshold) { /* thresholding */
		dvalue = threshold ^ 15;
		bptr += x3;
		do { /* for all source pixels */
			/* pre-compute intensity values for each Black component */
			Black = ColorInt->colorByte[PCMBLACK];
			ColorInt++;

			sx = *sxptr++;

			do { /* use this pixel 'sx' times */
				if (Black > dvalue) {
					*bptr |= bit;
				}
				bptr += 3;
			} while (--sx);
		} while (--width);
	}
	else { /* not thresholding, pre-compute ptr to dither matrix */
		dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
		if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) { 
			bptr += x3;
			do { /* for all source pixels */
				/* pre-compute intensity values for each Black component */
				Black = ColorInt->colorByte[PCMBLACK];
				ColorInt++;

				sx = *sxptr++;

				do { /* use this pixel 'sx' times */
					if (Black > dmatrix[x & 3]) {
						*bptr |= bit;
					}
					x++; /* done 1 more printer pixel */
					bptr += 3;
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
						*(bptr + x3) |= bit;
					}
					else  { /* black not rendered */
						if (Yellow > dvalue) {
							*(yptr + x3) |= bit;
						}
						if (Magenta > dvalue) {
							*(mptr + x3) |= bit;
						}
						if (Cyan > dvalue) {
							*(cptr + x3) |= bit;
						}
					}
					x++; /* done 1 more printer pixel */
					x3 += 3;
				} while (--sx);
			} while (--width);
		}
	}
}
