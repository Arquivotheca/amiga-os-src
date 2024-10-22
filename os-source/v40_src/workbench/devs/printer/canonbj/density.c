/**********************************************************/
/* Copyright   �1991 Wolf-Juergen Faust                   */
/* Am Dorfgarten 10                                       */
/* W-6000 Frankfurt 50   Canon BJ 10 driver               */
/* Germany                                                */
/* FIDO: 2:243/43.5 (Wolf Faust)                          */
/* UUCP: cbmvax.commodore.com!cbmehq!cbmger!venus!wfaust  */
/* Tel: +(49) 69 5486556                                  */
/*                                                        */
/* This File is NOT PART OF THE DISTRIBUTION package !!!  */
/*          DO NOT SPREAD THIS SOURCE FILE !!!            */
/*                                                        */
/* Versions required:                                     */
/* SAS/C 5.10a                                            */
/**********************************************************/

#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

#define	YSIZE		FALSE

/* (30.5cm/ft) * (10mm/cm) */
#define MMPERFT		305

/* Margins: minus 1/2" width, minus 1" height (== US diffs)	*/
/*								*/
/* Left            Right           Top           Bottom		*/
/* 50/300 = 1/6    100/300 = 1/3   60/300 = 1/5  60/300 = 1/5	*/

/* Half inches per ft */
#define HALFINCHPERFT	24

/* Inches per ft */
#define INCHPERFT	12

/* Characters per ft at 10 CPI */
#define	CHARSPERFT	120

/* MM per 1/2 inch (round up [12.7->13]) */
#define HALFINCH	((MMPERFT)/HALFINCHPERFT)
/* ((MMPERFT + INCHPERFT)/HALFINCHPERFT) */

/* MM per inch (round up) */
#define INCH		((MMPERFT)/INCHPERFT)
/* ((MMPERFT + INCHPERFT)/INCHPERFT) */

/* Sparse table for paper sizes - offset constant */
#define PSIZE_OFFSET	16

char __regargs SetDensity (ULONG density_code)
{
    extern struct PrinterData *PD;
    extern struct PrinterExtendedData *PED;

#if 1
    /* SPECIAL_DENSITY        0    1    2    3    4    5    6    7 */
    static int XDPI[8]   = {180, 180, 180, 120, 180, 240, 360, 360};
    static int YDPI[8]   = {180, 180, 180, 360, 360, 360, 180, 360};
    static char codes[8] = { 11,  11,  11,  13,  14,  15,  12,  16};
#else
    /* SPECIAL_DENSITY        0    1    2    3    4    5    6    7 */
    static int XDPI[8]   = {180, 360, 180, 120, 180, 240, 360, 360};
    static int YDPI[8]   = {180, 360, 180, 360, 360, 360, 180, 360};
    static char codes[8] = { 11,  16,  11,  13,  14,  15,  12,  16};
#endif

#if 1
    /* Paper sizes in inches	 */
    static int PaperXSizes[] =
    {
	80,			/* US_LETTER 8.0 x 10.0 */
	80,			/* US_LEGAL  8.0 x 13.0 */
	80,			/* N_TRACTOR == LETTER */
	136,			/* W_TRACTOR == 13.6 x 10.0 */
	80,			/* CUSTOM    == LETTER */
    /* Euro Ax sizes follow		 */
    /* Paper size in millimeters - 1/2 inch */
	841 - HALFINCH,		/* A0	 */
	594 - HALFINCH,		/* A1	 */
	420 - HALFINCH,		/* A2	 */
	297 - HALFINCH,		/* A3	 */
	210 - HALFINCH,		/* A4	 */
	148 - HALFINCH,		/* A5	 */
	105 - HALFINCH,		/* A6	 */
	74 - HALFINCH,		/* A7	 */
	52 - HALFINCH,		/* A8	 */
    };

#if YSIZE
    /* Paper sizes in inches	 */
    static int PaperYSizes[] =
    {
	10,			/* US_LETTER 8.0 x 10.0 */
	13,			/* US_LEGAL  8.0 x 13.0 */
	10,			/* N_TRACTOR == LETTER */
	10,			/* W_TRACTOR == LETTER */
	10,			/* CUSTOM    == LETTER */
    /* Euro Ax sizes follow		 */
    /* Paper size in millimeters - 1 inch */
	1189 - INCH,		/* A0	 */
	841 - INCH,		/* A1	 */
	594 - INCH,		/* A2	 */
	420 - INCH,		/* A3	 */
	297 - INCH,		/* A4	 */
	210 - INCH,		/* A5	 */
	148 - INCH,		/* A6	 */
	105 - INCH,		/* A7	 */
	74 - INCH,		/* A8	 */
    };

    ULONG maxheight;

#endif

    /* Calculate max dots based on paper size selection */
    UWORD psize;
    ULONG maxwidth;
    ULONG maxcols;

    psize = PD->pd_Preferences.PaperSize;
    maxwidth = (ULONG) PaperXSizes[psize / PSIZE_OFFSET];
#if YSIZE
    maxheight = (ULONG) PaperYSizes[psize / PSIZE_OFFSET];
#endif
    density_code /= SPECIAL_DENSITY1;
    if (psize <= CUSTOM)
    {
	PED->ped_MaxColumns = maxwidth;
	PED->ped_MaxXDots = (XDPI[density_code] * maxwidth) / 10;
#if YSIZE
	PED->ped_MaxYDots = maxheight * YDPI[density_code];
#endif
    }
    else
    {
	PED->ped_MaxXDots = (maxwidth * INCHPERFT * XDPI[density_code]) / MMPERFT;
	maxcols = ((maxwidth * CHARSPERFT)+(MMPERFT - 1))/MMPERFT;
	PED->ped_MaxColumns = maxcols <= 136 ? maxcols : 136;
#if YSIZE
	PED->ped_MaxYDots = (maxheight * INCHPERFT * YDPI[density_code]) / MMPERFT;
#endif
    }
#else

    PED->ped_MaxColumns = PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
    density_code /= SPECIAL_DENSITY1;

    /* default is 80 chars (8.0 in.), W_TRACTOR is 136 chars (13.6 in.) */
    PED->ped_MaxXDots = (XDPI[density_code] * PED->ped_MaxColumns) / 10;

#endif

    PED->ped_XDotsInch = XDPI[density_code];

    if ((PED->ped_YDotsInch = YDPI[density_code]) == 360)
	PED->ped_NumRows = 48;
    else
	PED->ped_NumRows = 24;

    return (codes[density_code]);
}
