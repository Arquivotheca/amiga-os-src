/*
	Density module for Toshiba_P351SX driver.
	David Berezowski - April/88
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

/* (30.5cm/ft) * (10mm/cm) */
#define MMPERFT		305

/* Margins: minus 1/2" width, minus 1" height (== US diffs)	*/
/*								*/
/* Left            Right           Top           Bottom		*/
/* 50/300 = 1/6    100/300 = 1/3   60/300 = 1/5  60/300 = 1/5	*/

/* Half inches per ft */
#define HALFINCHPERFT 24

/* Inches per ft */
#define INCHPERFT 12

/* MM per 1/2 inch (round up [12.7->13]) */
#define HALFINCH ((MMPERFT + INCHPERFT)/HALFINCHPERFT)

/* MM per inch (round up) */
#define INCH ((MMPERFT + INCHPERFT)/INCHPERFT)

/* Sparse table for paper sizes - offset constant */
#define PSIZE_OFFSET 16


SetDensity(density_code)
ULONG density_code;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	static int XDPI[8] = {180, 180, 360, 180, 360, 360, 360, 360};
	static int YDPI[8] = {180, 180, 180, 360, 360, 360, 360, 360};
	static char codes[8] = {';', ';', 29, ';', 29, 29 , 29, 29};

#if 1
	/* Paper sizes in inches	*/
	static int PaperXSizes[] = {
		80,			/* US_LETTER 8.0 x 10.0*/
		80,			/* US_LEGAL  8.0 x 13.0*/
		80,			/* N_TRACTOR == LETTER */
		135,			/* W_TRACTOR == 13.5 x */
		80,			/* CUSTOM    == LETTER */
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

	UWORD psize;
        ULONG maxwidth;

	psize = PD->pd_Preferences.PaperSize;
	maxwidth =  (ULONG)PaperXSizes[psize/PSIZE_OFFSET];
	density_code /= SPECIAL_DENSITY1;
	if(psize <= CUSTOM)
	{
		PED->ped_MaxColumns = maxwidth;
		PED->ped_MaxXDots = maxwidth * XDPI[density_code] / 10;
	}
	else
	{
		PED->ped_MaxColumns = maxwidth * 100 / 254;	/* 25.4 mm per inch */
		PED->ped_MaxXDots = (maxwidth * INCHPERFT * XDPI[density_code])/MMPERFT;
	}
#else
	PED->ped_MaxColumns = PD->pd_Preferences.PaperSize == W_TRACTOR ? 135 : 80;
	density_code /= SPECIAL_DENSITY1;
	/* default is 80 chars (8.0 in.), W_TRACTOR is 135 chars (13.5 in.) */
	PED->ped_MaxXDots = (XDPI[density_code] * PED->ped_MaxColumns) / 10;
#endif
	PED->ped_XDotsInch = XDPI[density_code];
	if ((PED->ped_YDotsInch = YDPI[density_code]) == 360) {
		PED->ped_NumRows = 48;
	}
	else {
		PED->ped_NumRows = 24;
	}
	return(codes[density_code]);
}
