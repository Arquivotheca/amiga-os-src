/*
	Density module for HP_ThinkJet.
	David Berezowski - May/87
*/

#include <exec/types.h>
#include "../printer/printer.h"
#include "../printer/prtbase.h"

SetDensity(density_code)
ULONG density_code;
{
	extern struct PrinterExtendedData *PED;
	extern char StartCmd[];

	/* SPECIAL_DENSITY     0   1   2    3    4    5    6    7 */
	static int XDPI[8] = {96, 96, 192, 192, 192, 192, 192, 192};
	static char codes[8][4] = {
	"0640", "0640", "1280", "1280", "1280", "1280", "1280", "1280"
	};
	static int MaxX[8] = {640, 640, 1280, 1280, 1280, 1280, 1280, 1280};

	/* Note that the THINKJET has this concept of 640 dots wide at
	 * 96 DPI, and 1280 dots wide at 192 DPI - this is a printer
	 * limitation, so papersize support is not relevant.
	 */

	density_code /= SPECIAL_DENSITY1;
	PED->ped_MaxXDots = MaxX[density_code];
	PED->ped_XDotsInch = XDPI[density_code];
	StartCmd[3] = codes[density_code][0];
	StartCmd[4] = codes[density_code][1];
	StartCmd[5] = codes[density_code][2];
	StartCmd[6] = codes[density_code][3];
}
