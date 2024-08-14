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
*/
FloydPixelArray(PInfo, flag)
struct PrtInfo *PInfo;
int flag;
{
	extern struct PrinterData *PD;

	union colorEntry *ColorInt, *Dest1Int, *Dest2Int;
	UWORD width, sx, *sxptr;
	BYTE dvalue, Yellow, Magenta, Cyan, ety2, ety3, etm2, etm3, etc2, etc3;

	if (flag) { /* yexe or yrxe */
		FixScalingVars(PInfo);
	}

	sxptr = PInfo->pi_ScaleX;
	width = PInfo->pi_width;
	ColorInt = PInfo->pi_ColorInt;
	Dest1Int = PInfo->pi_Dest1Int;
	Dest2Int = PInfo->pi_Dest2Int;
	dvalue = 7; /* 0-7 no dot, 8-15 dot */

	if (PD->pd_Preferences.PrintShade == SHADE_COLOR) { /* if a color print */
		do { /* for all source pixels */
			sx = *sxptr++;
			do { /* use this pixel 'sx' times */
				/*
					Compute value for each color component by
					rounding the error term and adding in the
					original color component
				*/
				Yellow = (Dest1Int->colorSByte[PCMYELLOW] + 4) / 8 +
					ColorInt->colorSByte[PCMYELLOW];
				Magenta = (Dest1Int->colorSByte[PCMMAGENTA] + 4) / 8 +
					ColorInt->colorSByte[PCMMAGENTA];
				Cyan = (Dest1Int->colorSByte[PCMCYAN] + 4) / 8 +
					ColorInt->colorSByte[PCMCYAN];

				/* calculate pixel color and error term */
				if (Yellow > dvalue) { /* if yellow dot */
					Dest1Int->colorSByte[PCMYELLOW] = 15; /* on */
					ety2 = (Yellow - 15) * 2;
					ety3 = ety2 + Yellow - 15;
				}
				else { /* no yellow dot */
					Dest1Int->colorSByte[PCMYELLOW] = 0; /* off */
					ety2 = Yellow * 2;
					ety3 = ety2 + Yellow;
				}
				if (Magenta > dvalue) { /* if magenta dot */
					Dest1Int->colorSByte[PCMMAGENTA] = 15; /* on */
					etm2 = (Magenta - 15) * 2;
					etm3 = etm2 + Magenta - 15;
				}
				else { /* no magenta dot */
					Dest1Int->colorSByte[PCMMAGENTA] = 0; /* off */
					etm2 = Magenta * 2;
					etm3 = etm2 + Magenta;
				}
				if (Cyan > dvalue) { /* if cyan dot */
					Dest1Int->colorSByte[PCMCYAN] = 15; /* on */
					etc2 = (Cyan - 15) * 2;
					etc3 = etc2 + Cyan - 15;
				}
				else { /* no cyan dot */
					Dest1Int->colorSByte[PCMCYAN] = 0; /* off */
					etc2 = Cyan * 2;
					etc3 = etc2 + Cyan;
				}
				GetBlack(Dest1Int, PInfo->pi_flags);

				/* distribute error 3/8 down */
				Dest2Int->colorSByte[PCMYELLOW] += ety3;
				Dest2Int->colorSByte[PCMMAGENTA] += etm3;
				Dest2Int->colorSByte[PCMCYAN] += etc3;
				/* distribute error 2/8 (1/4) down and right */
				Dest2Int++; /* advance to next pixel down (down right) */
				Dest2Int->colorSByte[PCMYELLOW] += ety2;
				Dest2Int->colorSByte[PCMMAGENTA] += etm2;
				Dest2Int->colorSByte[PCMCYAN] += etc2;
				/* distribute error 3/8 right */
				Dest1Int++; /* advance to next pixel (right) */
				Dest1Int->colorSByte[PCMYELLOW] += ety3;
				Dest1Int->colorSByte[PCMMAGENTA] += etm3;
				Dest1Int->colorSByte[PCMCYAN] += etc3;
			} while (--sx);
			ColorInt++;
		} while (--width);
	}
	else { /* SHADE_GREYSCALE */
		do { /* for all source pixels */
			sx = *sxptr++;
			do { /* use this pixel 'sx' times */
				Yellow = (Dest1Int->colorSByte[PCMBLACK] + 4) / 8;
				Yellow += ColorInt->colorSByte[PCMBLACK];

				/* calculate pixel color and error term */
				if (Yellow > dvalue) { /* if black dot */
					Dest1Int->colorSByte[PCMBLACK] = 15; /* on */
					ety2 = (Yellow - 15) * 2;
					ety3 = ety2 + Yellow - 15;
				}
				else { /* no black dot */
					Dest1Int->colorSByte[PCMBLACK] = 0; /* off */
					ety2 = Yellow * 2;
					ety3 = ety2 + Yellow;
				}

				/* distribute error 3/8 down */
				Dest2Int->colorSByte[PCMBLACK] += ety3;
				/* distribute error 2/8 (1/4) down and right */
				Dest2Int++;
				Dest2Int->colorSByte[PCMBLACK] = ety2;
				/* distribute error 3/8 right */
				Dest1Int++;
				Dest1Int->colorSByte[PCMBLACK] += ety3;
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
