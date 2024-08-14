#include <exec/types.h>
#include <graphics/rastport.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"
#include "internal.h"

#include "printer_iprotos.h"

#define CPDEBUG	0
#define TPDEBUG	0
#define CBDEBUG	0
#define CMDEBUG	0

/*
	Initialize the Ham array to RGB values.
	(this allows any xstart,ystart Ham picture to be printed)
*/
InitHamArray(PInfo)
struct PrtInfo *PInfo;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	UWORD *HamBuf;
	union colorEntry *cm, *ArrayInt, oldcolor;
	struct RastPort *rp;
	WORD x, xend, y, yend, yinc;
	int color, color2, additive;

	int HamNormal;
	int HamMBlue;
	int HamMRed;
	int HamShift;
	BOOL Ham8;
	UBYTE bitsave;

	cm = PInfo->pi_ColorMap;
	rp = PInfo->pi_rp;

	/* CAS changed 15 to 255 */
	additive = (PED->ped_ColorClass & PCC_ADDITIVE) ? 0 : 255;
	additive ^= (PD->pd_Preferences.PrintImage == IMAGE_NEGATIVE) ? 255 : 0;

	/* CAS
	 * will shift modify values left 4 for HAM6 and add to 8 bit color
	 * will shift modify values left 2 for HAM8 and add to 8 bit color
	 */
	if(rp->BitMap->Depth <= 6)
	{
		HamShift = 4;		/* must shift mod values left 4 */
		HamNormal = 16;
		HamMBlue = 32;
		HamMRed = 48;
		Ham8 = FALSE;
	}
	else
	{
		HamShift = 2;		/* must shift mod values left 2 */
		HamNormal = 64;
		HamMBlue = 128;
		HamMRed = 192;
		Ham8 = TRUE;
	}


	if (PInfo->pi_flags & PRT_INVERT) { /* inverted */
		ArrayInt = PInfo->pi_ColorInt;
		y = PInfo->pi_xstart;
		yend = y - PInfo->pi_width;
		yinc = -1;
		xend = PInfo->pi_ystart;
	}
	else { /* non-inverted */
		ArrayInt = PInfo->pi_HamInt;
		y = PInfo->pi_ystart;
		yend = y + PInfo->pi_height;
		yinc = 1;
		xend = PInfo->pi_xstart;
	}

	if (xend) { /* if not starting at left edge */
		for (; y != yend; y += yinc) { /* for each row */
			oldcolor.colorLong = cm->colorLong; /* init to background color */
			HamBuf = PInfo->pi_HamBuf; /* read entire line into HamBuf */
			ReadPixelLine(PInfo->pi_rp, 0, y, xend, HamBuf, PInfo->pi_temprp,
					PInfo->pi_flags & PRT_NOBLIT);
			/* compute color of first used pixel */
			for (x=0; x<xend; x++) {
				/* get color lookup code */
				if ((color = *HamBuf++) < HamNormal ) { /* normal lookup */
					oldcolor.colorLong = (cm + color)->colorLong;
				}
				/* CAS */
				/* If Ham8, preserve low 2 bits of old color */
				else if(Ham8) { /* modify one of the color components */
					/* CAS - use additive first so low
					 * bits will be zero, ready for ORing
					 * in of old low bits or dup nibble
					 */
					color2 = ((color ^ additive) << HamShift) & 0xff;
					if (color < HamMBlue) { /* modify blue/yellow */
						bitsave = oldcolor.colorByte[PCMYELLOW] & 0x03;
						oldcolor.colorByte[PCMYELLOW] = (color2 | bitsave);
					}
					else if (color < HamMRed) { /* modify red/cyan */
						bitsave = oldcolor.colorByte[PCMCYAN] & 0x03;
						oldcolor.colorByte[PCMCYAN] = color2 | bitsave;
					}
					else { /* modify green/magenta */
						bitsave = oldcolor.colorByte[PCMMAGENTA] & 0x03;
						oldcolor.colorByte[PCMMAGENTA] = color2;
					}
				}
				/* If Ham6, duplicate new color in low nibble */
				else { /* modify one of the color components */
					color2 = (color << HamShift) ^ additive;
					if (color < HamMBlue) { /* modify blue/yellow */
						oldcolor.colorByte[PCMYELLOW] = color2 | (color2>>4);
					}
					else if (color < HamMRed) { /* modify red/cyan */
						oldcolor.colorByte[PCMCYAN] = color2 | (color2>>4);
					}
					else { /* modify green/magenta */
						oldcolor.colorByte[PCMMAGENTA] = color2 | (color>>4);
					}
				}
			}
			/* calculate black */
			GetBlack(&oldcolor, PInfo->pi_flags);
			ArrayInt->colorLong = oldcolor.colorLong; /* save pixel color */
			ArrayInt++;
		}
	}
	else { /* starting at left edge (simple case) */
		for (; y != yend; y += yinc) { /* for each row */
			/* init to background color */
			ArrayInt->colorLong = cm->colorLong;
			/* calculate black */
			GetBlack((union colorEntry *)&ArrayInt->colorLong, PInfo->pi_flags);
			ArrayInt++; /* go on to next row */
		}
	}
}

/*
	Converts a row of pixels to hedley scan convert mode
*/
/* CAS - added <<4 to masks and changed values to double-byte values */
ScanConvertPixelArray(PInfo, y)
struct PrtInfo *PInfo;
UWORD y;
{
	union colorEntry *ColorInt;
	UWORD width, color;

	ColorInt = PInfo->pi_ColorInt;
	width = PInfo->pi_width;
	do {
		if (y & 1) { /* odd lines */
			color = (ColorInt->colorByte[PCMMAGENTA] & (8<<4)) ? 0x22 : 0;
			color |= (ColorInt->colorByte[PCMYELLOW] & (1<<4)) ? 0x11 : 0;
		}
		else { /* even lines */
			color = (ColorInt->colorByte[PCMCYAN] & (8<<4)) ? 0x22 : 0;
			color |= (ColorInt->colorByte[PCMYELLOW] & (8<<4)) ? 0x11 : 0;
		}
		if (color == 0x11) { /* kludge to an even number */
			ColorInt->colorByte[PCMBLACK] = 0x66;
		}
		else {
			ColorInt->colorByte[PCMBLACK] = color * 0x44 + color; /* x5 */
		}
		ColorInt++;
	} while (--width);
}

/*
	Converts a row of pixels (stored as indexes into color registers)
	into color intensities.
	CAS - added count so can average on the fly.  Color bytes
	too small to do in-byte summing
*/
ConvertPixelArray(PInfo, y, sum, count)
struct PrtInfo *PInfo;
UWORD y;
int sum;
UWORD count;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;
	ULONG cavg;
	UWORD *RowBuf; /* row of color indecies */
	union colorEntry *cm, *ColorInt, oldcolor, *HamInt;
	UWORD width, count1, u;
	int color, color2, additive;

	BOOL Ham8;
	int HamNormal;
	int HamShift;
	UBYTE bitsave;

	cm = PInfo->pi_ColorMap;
	width = PInfo->pi_width;
	RowBuf = PInfo->pi_RowBuf;
	ColorInt = PInfo->pi_ColorInt;
	HamInt = PInfo->pi_HamInt;

	/* CAS - changed 15 to 255 */
	additive = (PED->ped_ColorClass & PCC_ADDITIVE) ? 0 : 255;
	additive ^= (PD->pd_Preferences.PrintImage == IMAGE_NEGATIVE) ? 255 : 0;


	if(PInfo->pi_rp->BitMap->Depth <= 6)
	{
		HamShift = 4;		/* shifted up 4 bits */
		HamNormal = 16;
		Ham8 = FALSE;
	}
	else
	{
		HamShift = 2;		/* shifted up 2 bits */
		HamNormal = 64;
		Ham8 = TRUE;
	}


	count1 = count+1;

#if CPDEBUG
	kprintf("CPA: PInfo=$%lx, y=%ld, flags=$%lx, width=%ld",
		PInfo, y, PInfo->pi_flags, width);
#endif
	if (PInfo->pi_flags & PRT_HAM) { /* ham picture */
#if CPDEBUG
		kprintf("HAM, ");
#endif
		if (!(PInfo->pi_flags & PRT_INVERT)) {
			oldcolor.colorLong = (PInfo->pi_HamInt + y)->colorLong;
		}
		if (sum) {
			do {
				if (PInfo->pi_flags & PRT_INVERT) {
					oldcolor.colorLong = ColorInt->colorLong;
				}
				color = *RowBuf++; /* get color lookup code */
				if (color < HamNormal) { /* normal lookup */
					oldcolor.colorLong = (cm + color)->colorLong;
				}
				else { /* modify one of the color components */
					/* CAS - duplicate of code above */
					color2 = ((color ^ additive) << HamShift) & 0xff;
					if(Ham8) {
					    switch(color >> 6) {
						case 1 : /* modify blue/yellow */
							bitsave = oldcolor.colorByte[PCMYELLOW] & 0x03;
							oldcolor.colorByte[PCMYELLOW] = color2 | bitsave;
							break;
						case 2 : /* modify red/cyan */
							bitsave = oldcolor.colorByte[PCMCYAN] & 0x03;
							oldcolor.colorByte[PCMCYAN] = color2 | bitsave;
							break;
						default : /* modify green/magenta */
							bitsave = oldcolor.colorByte[PCMMAGENTA] & 0x03;
							oldcolor.colorByte[PCMMAGENTA] = color2 | bitsave;
							break;
					    }
					}
					else { /* Ham6 */
					    switch(color >> 4) {
						case 1 : /* modify blue/yellow */
							oldcolor.colorByte[PCMYELLOW] = color2 | (color2>>4);
							break;
						case 2 : /* modify red/cyan */
							oldcolor.colorByte[PCMCYAN] = color2 | (color2>>4);
							break;
						default : /* modify green/magenta */
							oldcolor.colorByte[PCMMAGENTA] = color2 | (color2>>4);
							break;
					    }
					}

					/* re-calculate black */
					GetBlack(&oldcolor, PInfo->pi_flags);
				}
				/* CAS - average this pixel in on the fly */
				for(u=0; u<4; u++) {
					cavg = ((ColorInt->colorByte[u] * count)
					   + oldcolor.colorByte[u]) / (count1);
					ColorInt->colorByte[u] = cavg;
				}
				ColorInt++;
			} while (--width);
		}
		else {
			do {
				if (PInfo->pi_flags & PRT_INVERT) {
					oldcolor.colorLong = ColorInt->colorLong;
				}
				color = *RowBuf++; /* get color lookup code */
				if (color < HamNormal) { /* normal lookup */
					oldcolor.colorLong = (cm + color)->colorLong;
				}
				else { /* modify one of the color components */
					/* CAS - duplicate of code above */
					color2 = ((color ^ additive) << HamShift) & 0xff;
					if(Ham8) {
					    switch(color >> 6) {
						case 1 : /* modify blue/yellow */
							bitsave = oldcolor.colorByte[PCMYELLOW] & 0x03;
							oldcolor.colorByte[PCMYELLOW] = color2 | bitsave;
							break;
						case 2 : /* modify red/cyan */
							bitsave = oldcolor.colorByte[PCMCYAN] & 0x03;
							oldcolor.colorByte[PCMCYAN] = color2 | bitsave;
							break;
						default : /* modify green/magenta */
							bitsave = oldcolor.colorByte[PCMMAGENTA] & 0x03;
							oldcolor.colorByte[PCMMAGENTA] = color2 | bitsave;
							break;
					    }
					}
					else { /* Ham6 */
					    switch(color >> 4) {
						case 1 : /* modify blue/yellow */
							oldcolor.colorByte[PCMYELLOW] = color2 | (color2>>4);
							break;
						case 2 : /* modify red/cyan */
							oldcolor.colorByte[PCMCYAN] = color2 | (color2>>4);
							break;
						default : /* modify green/magenta */
							oldcolor.colorByte[PCMMAGENTA] = color2 | (color2>>4);
							break;
					    }
					}

					/* re-calculate black */
					GetBlack(&oldcolor, PInfo->pi_flags);
				}
				ColorInt->colorLong = oldcolor.colorLong;
				ColorInt++;
			} while (--width);
		}
	}
	else { /* non HAM picture */
#if CPDEBUG
		kprintf("!HAM (sum=%ld), ",sum);
#endif
#define DEBUGCAS 0
		if (sum) {
			do {
				/* CAS - average this pixel in on the fly */
				oldcolor.colorLong = (cm + *RowBuf++)->colorLong;
				for(u=0; u<4; u++) {
				cavg = ((ColorInt->colorByte[u] * count)
					   + oldcolor.colorByte[u]) / (count1);
				ColorInt->colorByte[u] = cavg;
				}

#if DEBUGCAS
		kprintf("%08lx ",ColorInt->colorLong);
#endif
				ColorInt++;
			} while (--width);
		}
		else {
			do {
				ColorInt->colorLong = (cm + *RowBuf++)->colorLong;
#if DEBUGCAS
		kprintf("%08lx ",ColorInt->colorLong);
#endif
				ColorInt++;
			} while (--width);
#if DEBUGCAS

		kprintf("\n");
#endif
		}
	}
#if CPDEBUG
	kprintf("CPA: exit\n");
#endif
}

/*
	Compact an entire row of pixels; used for x reductions.
*/
CompactPixelArray(PInfo, sytotal)
struct PrtInfo *PInfo; /* printer info */
UWORD sytotal;
{
	union colorEntry *ColorInt1, *ColorInt2;
	UWORD width, sx, sx1, sx2, *sxptr;
	ULONG Black, Yellow, Magenta, Cyan;	/* CAS - were UWORD */

	width = PInfo->pi_width;
	ColorInt1 = ColorInt2 = PInfo->pi_ColorInt;
	sxptr = PInfo->pi_ScaleX;
#if CMDEBUG
	kprintf("CM: width=%ld, sytotal=%ld, ", width, sytotal);
#endif /* CMDEBUG */
	/* if color OR non-color and printer has no black */
	if ((PInfo->pi_flags & PRT_BW_BLACKABLE) != PRT_BW_BLACKABLE) {
		do {
			Black = Yellow = Magenta = Cyan = 0;
			sx = sx2 = *sxptr++;
			do {
				/* get color info */
				Black += ColorInt1->colorByte[PCMBLACK];
				Yellow += ColorInt1->colorByte[PCMYELLOW];
				Magenta += ColorInt1->colorByte[PCMMAGENTA];
				Cyan += ColorInt1->colorByte[PCMCYAN];
				ColorInt1++;
			} while (--sx2);
			/* average color values */
			sx1 = sx; /* CAS - not + sytotal, y already averaged */
			sx2 = sx1 >> 1;
			ColorInt2->colorByte[PCMBLACK] = (Black + sx2) / sx1;
			ColorInt2->colorByte[PCMYELLOW] = (Yellow + sx2) / sx1;
			ColorInt2->colorByte[PCMMAGENTA] = (Magenta + sx2) / sx1;
			ColorInt2->colorByte[PCMCYAN] = (Cyan + sx2) / sx1;
			ColorInt2++;
			width -= sx;
		} while (width);
	}
	else { /* non-color and printer has black */
		do {
			Black = 0;
			sx = sx2 = *sxptr++;
			do {
				/* get color info */
				Black += ColorInt1->colorByte[PCMBLACK];
				ColorInt1++;
			} while (--sx2);
			/* average color values */
			sx1 = sx; /* CAS - not + sytotal, y already averaged */
			ColorInt2->colorByte[PCMBLACK] = (Black + (sx1 >> 1)) / sx1;
			ColorInt2++;
			width -= sx;
		} while (width);
	}
	Force1to1Scaling(PInfo);
}

/*
	Transfer an entire pixel row to the render routine.  This is done by
	scaling, dithering/thresholding and conditionally rendering each pixel.
*/
TransferPixelArray(ypos, PInfo, pass)
UWORD ypos; /* row number */
struct PrtInfo *PInfo; /* printer info */
int pass;
{
	extern struct PrinterData *PD;

	union colorEntry *ColorInt, *Color12, *Color24=NULL;
	UWORD xpos, sx, *sxptr, width;
	UBYTE Black, Yellow, Magenta, Cyan; /* color intensity values */
	int (*render)();
	/* CAS - leave threshold15 - is for real old drivers */
	UBYTE threshold15, threshold, blackable, dvalue, *dmatrix;
	
	render = PInfo->pi_render;
#if TPDEBUG
kprintf("TPA: ypos=%ld, PInfo=$%lx, flags=$%lx, render=$%lx, Ver=%ld, Rev=%ld",
	ypos, PInfo, PInfo->pi_flags, render, PD->pd_SegmentData->ps_Version,
	PD->pd_SegmentData->ps_Revision);
#endif

	/* CAS - if wants 24-bit, leave colors alone
	 * else shift all color values down
	 */
        if(!(PED->ped_ColorClass & PCC_24BIT)) {
		width   = PInfo->pi_width;
		Color24 = PInfo->pi_ColorInt;
		Color12 = PInfo->pi_ColorTmp;
		do {
#if TPDEBUG
			kprintf("%08lx ",Color24->colorLong);
#endif
			Color12->colorLong = 
				((Color24->colorLong & 0xf0f0f0f0) >> 4); 
			Color12++;
			Color24++;
		} while (--width);
#if TPDEBUG
	kprintf("\n");
#endif
	Color24 = PInfo->pi_ColorInt;		  /* set  to start of Color24 */
	PInfo->pi_ColorInt = PInfo->pi_ColorTmp;  /* swap to start of Color12 */
	}

	/* if a V1.3 or greater driver */
	if (PD->pd_SegmentData->ps_Version >= 35) {
#if TPDEBUG
		kprintf("calling render once for entire line, ");
#endif
		(*render)(PInfo, pass, ypos, 1);
		if (Color24) PInfo->pi_ColorInt = Color24;  /* swap back */
		return;
	}


/* NOTE - for V1.3 and greater drivers, we are done.
 * Code below is just for older drivers.
 * Data already changed to 12-bit, so we'll use old code as is
 * but add swap back at the end
 */
#if TPDEBUG
	kprintf("TPA: Ver=%ld, Rev=%ld, <35\n", PD->pd_SegmentData->ps_Version,
		PD->pd_SegmentData->ps_Revision);
#endif
	dmatrix = PInfo->pi_dmatrix + ((ypos & 3) << 2);
	sxptr = PInfo->pi_ScaleX;
	threshold = PInfo->pi_threshold;
	threshold15 = threshold ^ 15;
	xpos = PInfo->pi_xpos;
	width = PInfo->pi_width;
	ColorInt = PInfo->pi_ColorInt;
	blackable = PInfo->pi_flags & PRT_BLACKABLE;

	 /* if non-color and printer has black */
	if ((PInfo->pi_flags & PRT_BW_BLACKABLE) == PRT_BW_BLACKABLE) {
#if TPDEBUG
		kprintf("non-color & printer has black, ");
#endif
		if (threshold) { /* thresholding */
#if TPDEBUG
			kprintf("thresholding, ");
#endif
			do { /* for each source column */
				sx = *sxptr++;
				/* get color info, if not null */
				if (Black = ColorInt->colorByte[PCMBLACK]) {
					/* scale, threshold, and render pixel */
					do { /* use this pixel 'sx' times */
						if (Black > threshold15) {
							(*render)(0, xpos, ypos, 1); /* render black */
						}
						++xpos; /* done 1 more printer pixel */
					} while (--sx);
				}
				else {
					xpos += sx; /* done sx more printer pixels */
				}
				ColorInt++;
			} while (--width);
		}
		else { /* not thresholding (ie. dithering) */
#if TPDEBUG
			kprintf("dithering, ");
#endif
			do { /* for each source column */
				sx = *sxptr++;
				/* get color info, if not null */
				if (Black = ColorInt->colorByte[PCMBLACK]) {
					/* scale, dither, and render pixel */
					do { /* use this pixel 'sx' times */
						if (Black > *(dmatrix + (xpos & 3))) {
							(*render)(0, xpos, ypos, 1); /* render black */
						}
						++xpos; /* done 1 more printer pixel */
					} while (--sx);
				}
				else {
					xpos += sx; /* done sx more printer pixels */
				}
				ColorInt++;
			} while (--width);
		}
	}
	else { /* color OR non-color and printer has no black */
#if TPDEBUG
		kprintf("color OR non-color and printer has no black, ");
#endif
		if (threshold) { /* thresholding */
#if TPDEBUG
			kprintf("thresholding, ");
#endif
			do {
				/* get color info */
				Yellow = ColorInt->colorByte[PCMYELLOW];
				Magenta = ColorInt->colorByte[PCMMAGENTA];
				Cyan = ColorInt->colorByte[PCMCYAN];
				/* scale, threshold, and render pixel */
				sx = *sxptr++;
				do { /* use this pixel 'sx' times */
					if (Yellow > threshold15) {
						(*render)(1, xpos, ypos, 1); /* render yellow */
					}
					if (Magenta > threshold15) {
						(*render)(2, xpos, ypos, 1); /* render magenta */
					}
					if (Cyan > threshold15) {
						(*render)(3, xpos, ypos, 1); /* render cyan */
					}
					++xpos; /* done 1 more printer pixel */
				} while (--sx);
				ColorInt++;
			} while (--width);
		}
		else { /* not thresholding (ie. dithering) */
#if TPDEBUG
			kprintf("dithering, ");
#endif
			do { /* for each source column */
				/* get color info */
				Black = ColorInt->colorByte[PCMBLACK];
				Yellow = ColorInt->colorByte[PCMYELLOW];
				Magenta = ColorInt->colorByte[PCMMAGENTA];
				Cyan = ColorInt->colorByte[PCMCYAN];
				/* scale, dither, and render pixel */
				sx = *sxptr++;
				do { /* use this pixel 'sx' times */
					/* get dither value */
					dvalue = *(dmatrix + (xpos & 3));
					if (blackable && Black > dvalue) {
						(*render)(0, xpos, ypos, 1); /* render black */
					}
					else { /* black not rendered */
						if (Yellow > dvalue) {
							(*render)(1, xpos, ypos, 1); /* render yellow */
						}
						if (Magenta > dvalue) {
							(*render)(2, xpos, ypos, 1); /* render magenta */
						}
						if (Cyan > dvalue) {
							(*render)(3, xpos, ypos, 1); /* render cyan */
						}
					}
					++xpos; /* done 1 more printer pixel */
				} while (--sx);
				ColorInt++;
			} while (--width);
		}
	}

	if (Color24) PInfo->pi_ColorInt = Color24;  /* swap back */
#if TPDEBUG
	kprintf("TPA: exit\n");
#endif

}

CheckBuf(y, render, flag)
UWORD y;
int (*render)();
int flag;
{
	extern struct PrinterExtendedData *PED;

	UWORD mod;
	int err = 0;

	mod = y % PED->ped_NumRows;
#if CBDEBUG
	kprintf("CB: y=%ld, mod=%ld, flag=%ld ", y, mod, flag);
#endif
	/* if time to dump buffer OR we are forcing the issue */
	if ((!mod && !flag) || (mod && flag)) {
#if CBDEBUG
		kprintf("CB: DUMPED ");
#endif
		if (!mod) {
			mod = PED->ped_NumRows;
		}
		if ((err = (*render)(0, 0, mod, 2)) == 0) { /* dump it, if no error */
			(*render)(0, 0, 0, 3); /* clear and init buffer */
		}
	}
#if CBDEBUG
	kprintf("CB: exit\n");
#endif
	return(err);
}

GetBlack(ce, flags)
union colorEntry *ce;
UWORD flags;
{
	extern struct PrinterData *PD;

	ULONG black;	/* CAS - was UWORD */

	/* if doing a non-color picture */
	if (PD->pd_Preferences.PrintShade != SHADE_COLOR) {
		/* white = red * .299 + green * .587 + blue * .114 */
		black = (ce->colorByte[PCMCYAN] * 77 +
			ce->colorByte[PCMMAGENTA] * 150 +
			ce->colorByte[PCMYELLOW] * 29 + 128) >> 8;
	}
	else { /* if doing a color picture */
		/* black is the minimum of ymc */
		black = ce->colorByte[PCMYELLOW];
		if (ce->colorByte[PCMMAGENTA] < black) {
			black = ce->colorByte[PCMMAGENTA];
		}
		if (ce->colorByte[PCMCYAN] < black) {
			black = ce->colorByte[PCMCYAN];
		}
	}

	if (PD->pd_Preferences.LaceWB & 128) { /* Hedley 4-color hi-res */
		black >>= 2;
		black = black * 4 + black; /* x5 */
	}

	ce->colorByte[PCMBLACK] = black;
}

SwapScaleX(PInfo) /* swap ScaleX and ScaleXAlt vars */
struct PrtInfo *PInfo;
{
	UWORD *ptr, size;

	ptr = PInfo->pi_ScaleX;
	PInfo->pi_ScaleX = PInfo->pi_ScaleXAlt;
	PInfo->pi_ScaleXAlt = ptr;
	size = PInfo->pi_ScaleXSize;
	PInfo->pi_ScaleXSize = PInfo->pi_ScaleXAltSize;
	PInfo->pi_ScaleXAltSize = size;
}

FixScalingVars(PInfo)
struct PrtInfo *PInfo;
{
	/* restore width and scalex array if we messed them up */
	if (PInfo->pi_width != PInfo->pi_tempwidth) {
		PInfo->pi_width = PInfo->pi_tempwidth;
		SwapScaleX(PInfo);
	}
}

Force1to1Scaling(PInfo)
struct PrtInfo *PInfo;
{
	/* force 1:1 scaling if we have not already */
	if (PInfo->pi_width == PInfo->pi_tempwidth) {
		PInfo->pi_width = PInfo->pi_pc - PInfo->pi_xpos;
		SwapScaleX(PInfo);
	}
}

SwapInts(PInfo) /* swap ColorInt and Dest1Int ptrs */
struct PrtInfo *PInfo;
{
	union colorEntry *ce;

	ce = PInfo->pi_ColorInt;
	PInfo->pi_ColorInt = PInfo->pi_Dest1Int;
	PInfo->pi_Dest1Int = ce;
}


