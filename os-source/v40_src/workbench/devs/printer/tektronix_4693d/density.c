/*
	Density module for Tektronix_4693D driver.
	David Berezowski - September/88.
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

/*
	Media dimensions and number of addressable dots.

	Type	Media Size	Print Size	Print Dots
	----	----------	----------	----------

	A	8.5 x 11 in.	8.13 x 8.31	2440 x 2492	USA
	Legal	8.5 x 14 in.	8.13 x 10.66	2440 x 3198	USA
	A4	210 x 297 mm.	200 x 229	2368 x 2700	UK
	A4 L	210 x 356 mm.	200 x 271	2368 x 3198	UK
*/
SetDensity(density_code)
ULONG density_code;
{
    extern struct PrinterData *PD;
    extern struct PrinterExtendedData *PED;

    long *ptr;    
    /* SPECIAL_DENSITY          0    1     2     3     4     5    6     7 */
    static long MaxYDotsA[8] = {750, 750, 1040, 1330, 1620, 1910, 2200, 2492};
    static long MaxYDotsL[8] = {750, 750, 1158, 1566, 1974, 2382, 2790, 3198};

    if (PD->pd_Preferences.PaperSize == US_LETTER ||
	PD->pd_Preferences.PaperSize == N_TRACTOR) {
	ptr = MaxYDotsA; /* letter (A) size */
    }
    else {
	ptr = MaxYDotsL; /* legal size */
    }
    PED->ped_MaxYDots = ptr[density_code / SPECIAL_DENSITY1];
}
