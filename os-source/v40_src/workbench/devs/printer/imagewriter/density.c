/*
	Density module for Apple Imagewriter
	David Berezowski - Aug/87
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

/* (30.5cm/ft) * (10mm/cm)					*/
#define MMPERFT		305

/* Margins: minus 1/2" width, minus 1" height (== US diffs)	*/
/*								*/
/* Left            Right           Top           Bottom		*/
/* 50/300 = 1/6    100/300 = 1/3   60/300 = 1/5  60/300 = 1/5	*/
/* 								*/

/* Half inches per ft */
#define HALFINCHPERFT 24

/* Inches per ft */
#define INCHPERFT 12

/* MM per 1/2 inch (round up [12.7->13]) */
#define HALFINCH ((MMPERFT + INCHPERFT)/HALFINCHPERFT)

/* MM per inch (round up) 25.4 */
#define INCH ((MMPERFT + INCHPERFT)/INCHPERFT)

/* Sparse table for paper sizes - offset constant */
#define PSIZE_OFFSET 16

/* Paper width 3 - 10 (maximum line length of 8) inches */
SetDensity(density_code)
ULONG density_code;
{
	extern struct PrinterExtendedData *PED;
	extern struct PrinterData *PD;

	/* SPECIAL_DENSITY     0   1   2   3    4    5    6    7 */
	static int XDPI[8] = {80, 80, 120, 144, 160, 120, 144, 160};
	static int YDPI[8] = {72, 72, 72, 72, 72, 144, 144, 144};
	static char codes[8] = {'N', 'N', 'q', 'p', 'P', 'q', 'p', 'P'};

#if 1
	/* Paper sizes in inches */
	static int PaperXSizes[] = {
		8,			/* US_LETTER 8.0 x 10.0*/
		8,			/* US_LEGAL  8.0 x 13.0*/
		8,			/* N_TRACTOR == LETTER */
		8,			/* W_TRACTOR == LETTER */
		8,			/* CUSTOM    == LETTER */
	/* Euro Ax sizes follow		*/
	/* Paper size in millimeters - 1/2 inch */
		841 - HALFINCH,		/* A0	*/
		594 - HALFINCH,		/* A1	*/
		420 - HALFINCH,		/* A2	*/
		297 - HALFINCH,		/* A3	*/
		210 - HALFINCH,		/* A4	*/
		148 - HALFINCH,		/* A5	*/
		105 - HALFINCH,		/* A6	*/
		74  - HALFINCH,		/* A7	*/
		52  - HALFINCH,		/* A8	*/
		};

	/* Paper sizes in inches */
	static int PaperYSizes[] = {
		10,			/* US_LETTER 8.0 x 10.0*/
		13,			/* US_LEGAL  8.0 x 13.0*/
		10,			/* N_TRACTOR == LETTER */
		10,			/* W_TRACTOR == 13.6 x 10.0 */
		10,			/* CUSTOM    == LETTER */
	/* Euro Ax sizes follow		*/
	/* Paper size in millimeters - 1 inch */
		1189 - INCH,		/* A0 	*/
		841  - INCH,	 	/* A1 	*/
		594  - INCH, 		/* A2 	*/
		420  - INCH, 		/* A3 	*/
		297  - INCH, 		/* A4	*/
		210  - INCH, 		/* A5	*/
		148  - INCH, 		/* A6	*/
		105  - INCH, 		/* A7	*/
		74   - INCH,		/* A8	*/
		};

	/* Calculate max dots based on paper size selection */
	UWORD psize;
        ULONG maxwidth,maxheight;

	/* reference array (constant steps of 16) */
	/* Any new in-between sizes will be rounded down */
	psize = PD->pd_Preferences.PaperSize;

	maxwidth =  (ULONG)PaperXSizes[psize/PSIZE_OFFSET];
	maxheight = (ULONG)PaperYSizes[psize/PSIZE_OFFSET];

	/* Math - be precise as possible!  No change in precision for	*/
	/* existing INCH specified paper sizes				*/
	density_code /= SPECIAL_DENSITY1;

	/* Inches:  Inches * DPI */
	if(psize <= CUSTOM)
	{
		PED->ped_MaxXDots = maxwidth * XDPI[density_code];
#if 0
		PED->ped_MaxYDots = maxheight * XDPI[density_code];
#endif
	}
	/* Millimeters: */
	/*					 */
	/* (X mm) * 12 inches/ft * dots/inch	 */
	/* ------------------------------ = Dots */
	/* 305 mm/ft				 */
	else
	{
		PED->ped_MaxXDots = (maxwidth * INCHPERFT * XDPI[density_code])/MMPERFT;
		PED->ped_MaxYDots = (maxheight * INCHPERFT * XDPI[density_code])/MMPERFT;
	}

#else
	density_code /= SPECIAL_DENSITY1;
	PED->ped_MaxXDots = XDPI[density_code] * 8; /* 8 inches */
#endif
	PED->ped_XDotsInch = XDPI[density_code];
	if ((PED->ped_YDotsInch = YDPI[density_code]) == 144) {
		PED->ped_NumRows = 16;
	}
	else {
		PED->ped_NumRows = 8;
	}
	return(codes[density_code]);
}
