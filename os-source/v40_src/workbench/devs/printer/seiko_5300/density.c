/*
	Density module for Seiko_5300.
	David Berezowski - June/88

      Printer	DPI	Normal Tractor	Wide Tractor
      -------	---	--------------	------------
	5301	152	1224x1280 (A)	1188x1280 (A4)
	5312	203	2048x3074 (B)	2048x3328 (B long)
	5303	240	1928x2174 (A)	1872x2344 (A4)
	5504	300	2475x2715 (A)	2475x3210 (A double cut)
	5514	300	3225x4500 (B)	3225x4980 (B double cut)
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
	extern char StartCmd[];

	/* SPECIAL_DENSITY     0    1    2    3    4    5    6    7 */
	static int XDPI[8] = {152, 152, 203, 240, 300, 300, 300, 300};
	static UBYTE codes[8] = {'1', '1', '7', '3', '3', '3', '3', '3'};
	static int MaxX[16] = {1224, 1224, 2048, 1928, 2475, 3225, 3225, 3225,
			      1188, 1188, 2048, 1872, 2475, 3225, 3225, 3225};

	static int MaxY[16] = {1280, 1280, 3074, 2174, 2715, 4500, 4500, 4500,
			      1280, 1280, 3328, 2344, 3210, 4980, 4980, 4980};
	int *xptr, *yptr;

	if (PD->pd_Preferences.PaperSize != W_TRACTOR)
	{
		xptr = &MaxX[0];
		yptr = &MaxY[0];
	}
	else
	{
		xptr = &MaxX[8];
		yptr = &MaxY[8];
	}
	density_code /= SPECIAL_DENSITY1;

        if (PD->pd_Preferences.PaperSize == EURO_A4)
	{
	    PED->ped_MaxXDots = ((210 - HALFINCH) * INCHPERFT * XDPI[density_code]) / MMPERFT;
	    PED->ped_MaxYDots = ((297 - INCH) * INCHPERFT * XDPI[density_code]) / MMPERFT;
	}
	else
	{
	    PED->ped_MaxXDots = xptr[density_code];
	    PED->ped_MaxYDots = yptr[density_code];
	}

	PED->ped_XDotsInch = PED->ped_YDotsInch = XDPI[density_code];

	return(codes[density_code]);
}
