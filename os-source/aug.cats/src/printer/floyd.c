#include <exec/types.h>
#include <intuition/intuition.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "printer_iprotos.h"

#define DEBUGFPA	0
#define DEBUGFSI	0
#define DEBUGFTD	0

/*
	Floyd-Steinburg error correction

	12/93 - changed for 8-bits-per-gun thanks to help from Tom Rokicki
*/
FloydPixelArray(PInfo, flag)
struct PrtInfo *PInfo;
int flag;
{
	extern struct PrinterData *PD;

	union colorEntry *ColorInt, *Dest1Int, *Dest2Int;
	UWORD width, sx, *sxptr;
	/* CAS - These were BYTE */
	WORD Yellow, Magenta, Cyan, X;
	WORD ety2, ety3, etm2, etm3, etc2, etc3, dvalue;

	if (flag) { /* yexe or yrxe */
		FixScalingVars(PInfo);
	}

	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	ColorInt = PInfo->pi_ColorInt;
	Dest1Int = PInfo->pi_Dest1Int;
	Dest2Int = PInfo->pi_Dest2Int;
	dvalue = 127; /* CAS - Used to be 7 with 0-7 no dot, 8-15 dot */

	if (PD->pd_Preferences.PrintShade == SHADE_COLOR) { /* if a color print */
		do { /* for all source pixels */
			sx = *sxptr++;
			do { /* use this pixel 'sx' times */
				/*
				Compute value for each color component by
				rounding the error term and adding in the
				original color component
				*/

				/* CAS - Changed ColorSByte to ColorByte */
				/* TR  - moves scaling to later in loop */
				Yellow = 
				 Dest1Int->colorSByte[PCMYELLOW] +
				 ColorInt->colorByte[PCMYELLOW];
				Magenta =
				 Dest1Int->colorSByte[PCMMAGENTA] +
				 ColorInt->colorByte[PCMMAGENTA];
				Cyan =
				 Dest1Int->colorSByte[PCMCYAN] +
				 ColorInt->colorByte[PCMCYAN];


				/* calculate pixel color and error term */
				if (Yellow > dvalue) { /* if yellow dot */
				/* CAS - changed 15's to 255 */
				/* CAS - changed colorSByte to colorByte */
					Dest1Int->colorByte[PCMYELLOW] = 255; /* on */
					X = Yellow - 255;
					ety3 = ((X + (X << 1)) + 6) >> 3;
					ety2 = X - (ety3 << 1);
				}
				else { /* no yellow dot */
					Dest1Int->colorByte[PCMYELLOW] = 0; /* off */
					X = Yellow * 2;
					ety3 = ((X + (X << 1)) + 6) >> 3;
					ety2 = X - (ety3 << 1);
				}
				if (Magenta > dvalue) { /* if magenta dot */
					Dest1Int->colorByte[PCMMAGENTA] = 255; /* on */
					X = Magenta - 255;
					etm3 = ((X + (X << 1)) + 6) >> 3;
					etm2 = X - (etm3 << 1);
				}
				else { /* no magenta dot */
					Dest1Int->colorByte[PCMMAGENTA] = 0; /* off */
					X = Magenta * 2;
					etm3 = ((X + (X << 1)) + 6) >> 3;
					etm2 = X - (etm3 << 1);
				}
				if (Cyan > dvalue) { /* if cyan dot */
					Dest1Int->colorByte[PCMCYAN] = 255; /* on */
					X = Cyan - 255;
					etc3 = ((X + (X << 1)) + 6) >> 3;
					etc2 = X - (etc3 << 1);
				}
				else { /* no cyan dot */
					Dest1Int->colorByte[PCMCYAN] = 0; /* off */
					X = Cyan * 2;
					etc3 = ((X + (X << 1)) + 6) >> 3;
					etc2 = X - (etc3 << 1);
				}
				GetBlack(Dest1Int, PInfo->pi_flags);

				/* CAS - changed colorSByte to colorByte */
				/* distribute error 3/8 down */
				Dest2Int->colorByte[PCMYELLOW] += ety3;
				Dest2Int->colorByte[PCMMAGENTA] += etm3;
				Dest2Int->colorByte[PCMCYAN] += etc3;
				/* distribute error 2/8 (1/4) down and right */
				Dest2Int++; /* advance to next pixel down (down right) */
				Dest2Int->colorByte[PCMYELLOW] += ety2;
				Dest2Int->colorByte[PCMMAGENTA] += etm2;
				Dest2Int->colorByte[PCMCYAN] += etc2;
				/* distribute error 3/8 right */
				Dest1Int++; /* advance to next pixel (right) */
				Dest1Int->colorByte[PCMYELLOW] += ety3;
				Dest1Int->colorByte[PCMMAGENTA] += etm3;
				Dest1Int->colorByte[PCMCYAN] += etc3;
			} while (--sx);
			ColorInt++;
		} while (--width);
	}
	/* CAS - changed colorSByte to ColorByte */
	else { /* SHADE_GREYSCALE */
		do { /* for all source pixels */
			sx = *sxptr++;
			do { /* use this pixel 'sx' times */
				Yellow =
				  Dest1Int->colorSByte[PCMBLACK] +
				  ColorInt->colorByte[PCMBLACK];

				/* calculate pixel color and error term */
				/* CAS - changed 15's to 255's */
				if (Yellow > dvalue) { /* if black dot */
					Dest1Int->colorByte[PCMBLACK] = 255; /* on */
					X = Yellow - 255;
					ety3 = ((X + (X << 1)) + 6) >> 3;
					ety2 = X - (ety3 << 1);
				}
				else { /* no black dot */
					Dest1Int->colorByte[PCMBLACK] = 0; /* off */
					X = Yellow * 2;
					ety3 = ((X + (X << 1)) + 6) >> 3;
					ety2 = X - (ety3 << 1);

				}

				/* distribute error 3/8 down */
				Dest2Int->colorByte[PCMBLACK] += ety3;
				/* distribute error 2/8 (1/4) down and right */
				Dest2Int++;
				Dest2Int->colorByte[PCMBLACK] = ety2;
				/* distribute error 3/8 right */
				Dest1Int++;
				Dest1Int->colorByte[PCMBLACK] += ety3;
			}
			while (--sx);
			ColorInt++;
		} while (--width);
	}

	SwapInts(PInfo); /* swap ColorInt and Dest1Int ptrs */

	if (flag) {
		Force1to1Scaling(PInfo); /* force 1:1 scaling */
	}
}

FloydSwapDest(PInfo)
struct PrtInfo *PInfo;
{
	union colorEntry *Dest2Int;

	/*swap ptrs */
	Dest2Int = PInfo->pi_Dest2Int;
	PInfo->pi_Dest2Int = PInfo->pi_Dest1Int;
	PInfo->pi_Dest1Int = Dest2Int;
	/* clear first entry */
	PInfo->pi_Dest2Int->colorLong = 0;
}
