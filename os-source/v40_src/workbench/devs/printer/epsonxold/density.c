/*** epsonXOld/density.c ***********************************************
 *
 * density.c -- calc density settings (maximums)
 *
 *	$Id: density.c,v 1.2 91/07/10 17:01:49 darren Exp $
 *
 *	Copyright (c) 1987,1991 Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 **********************************************************************/

/*
	Density module for EpsonXOld driver.
	David Berezowski - October/87

	Modified for Star Micronics printers by Bryce Nesbitt 
	Fri Jun  3 13:19:18 EDT 1988
*/

#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <intuition/preferences.h>


#ifdef  DEBUG
#define D(a) KPrintF a
#else
#define D(a)
#endif


/* (30.5cm/ft) * (10mm/cm)					*/
#define MMPERFT		305

/* Half inches per ft						*/

#define HALFINCHPERFT 24

/* Inches per ft						*/

#define INCHPERFT 12

/* Characters per ft at 10 CPI					*/

#define CHARSPERFT 120

/* MM per 1/2 inch 						*/

#define HALFINCH ((MMPERFT)/HALFINCHPERFT)

/* MM per inch							*/

#define INCH ((MMPERFT)/INCHPERFT)

/* Sparse table for paper sizes - offset constant		*/

#define PSIZE_OFFSET 16


SetDensity(density_code)
ULONG density_code;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	/* SPECIAL_DENSITY       0    1    2    3    4    5    6    7 */
	static int XDPI[8] =   { 60,  60, 120, 120, 240, 120, 240, 240};
	static char codes[8] = {'K', 'K', 'Y', 'L', 'Z', 'y', 'z', 'Z'};
	/* y and z are the Star equivalents of Epson's Y and Z */

	static int PaperXSizes[] = {
	/* Paper sizes in inches x 10		*/

		80,			/* US_LETTER */
		80,			/* US_LEGAL  */
		80,			/* N_TRACTOR */
		136,			/* W_TRACTOR */
		80,			/* CUSTOM    */

	/* Euro Ax sizes follow						*/
	/* Paper size in millimeters - 1/2 inch 			*/
	/* Subtract 1/2 inch to be consistent with US size behavior	*/
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

	/* Calculate max dots based on paper size selection */

	UWORD psize;
        ULONG maxwidth;
	ULONG maxcols;

	/* reference array (constant steps of 16) */
	/* Any new in-between sizes will be rounded down */

	psize = PD->pd_Preferences.PaperSize;

	maxwidth =  (ULONG)PaperXSizes[psize/PSIZE_OFFSET];

	D(("psize = %ld  maxwidth = %ld\n",(long)psize,(long)maxwidth));

	/* Math - be precise as possible!  No change in precision for	*/
	/* existing INCH specified paper sizes				*/

	density_code /= SPECIAL_DENSITY1;

	if(psize <= CUSTOM)
	{
		PED->ped_MaxColumns = maxwidth;

		/* default is 80 chars (8.0 in.), W_TRACTOR is 136 chars (13.6 in.) */

		PED->ped_MaxXDots = (XDPI[density_code] * maxwidth) / 10;

	}
	else
	{
		/* calculate max X dots based on millimeters		*/

		/* Millimeters:						*/
		/*							*/
		/* (X mm) * 12 inches/ft * dots/inch			*/
		/* ------------------------------ = Dots		*/
		/* 305 mm/ft						*/


		PED->ped_MaxXDots =
			(maxwidth * INCHPERFT *	XDPI[density_code])/MMPERFT;

		/* calculate max columns based on millimeters, and	*/
		/* 10 CPI like inch based calculations above.		*/
		/* Plus round-up to nearest whole column position so we */
		/* don't clip when printing graphics			*/

                /*							*/
		/* (X mm) * 12 inches/ft * 10 chars/inch +		*/
		/* -------------------------------------     = Chars	*/
		/* 305 mm/ft						*/

		maxcols = ((maxwidth * CHARSPERFT)+(MMPERFT - 1))/MMPERFT;

		/* A0 sizes can overflow UBYTE MaxColumns.		*/

		/* Maximum based on wide tractor size, and allows for	*/
		/* the CLEAR MARGINS sequence to work at 17 CPI without	*/
		/* overflowing the UBYTE value understood by the Epson	*/
		/* margin set sequence.					*/

		PED->ped_MaxColumns = maxcols <= 136 ? maxcols : 136;
	}
		
	D(("ped_MaxXDots = %ld ped_MaxColumns = %ld\n",
		(long)PED->ped_MaxXDots, (long)PED->ped_MaxColumns));

	PED->ped_XDotsInch = XDPI[density_code];
	return(codes[density_code]);
}
