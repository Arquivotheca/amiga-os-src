/*
	Special functions for Canon_PJ-1080A driver.
	David Berezowski - March/88
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	/*
		0 - 2	\033-\376	underline off
		3 - 4	\033H		boldface off
		5 - 7	\033W\376	enlarge off
		8 - 9	\0332		1/6" line spacing
	*/
	static char initThisPrinter[10] = "\033-\376\033H\033W\376\0332";
	int x;

	x = 0;
	if (*command == aRIN) { /* initialize */
		while (x < 10) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		outputBuffer[2]='\000'; /* turn \376 into \000 */
		outputBuffer[7]='\000'; /* turn \376 into \000 */

		*currentVMI=36; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[9] = '0';
			*currentVMI=27;
		}
		return(x);
	}

	if (*command == aVERP0) { /* 8 lpi */
		*currentVMI=27;
	}
	else if (*command == aVERP1) { /* 6 lpi */
		*currentVMI=36;
	}

	if (*command == aSFC) { /* set foreground/background color */
		if (Parms[0] == 39) {
			Parms[0] = 30; /* default foreground to black */
		}
		else if (Parms[0] == 49) {
			Parms[0] = 47; /* default background to white */
		}
		outputBuffer[x++] = 27;
		if (Parms[0] < 40) { /* set foregound color */
			outputBuffer[x++] = 'V';
		}
		else { /* set background color */
			outputBuffer[x++] = 'g';
			Parms[0] -= 10;
		}
		outputBuffer[x++] = Parms[0] + 18;
		return(x);
	}

	if (*command == aSLPP) { /* set form length */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = Parms[0];
		return(x);
	}

	if (*command == aPERF) { /* set perf skip */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'N';
		outputBuffer[x++] = Parms[0];
		return(x);
	}

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled=253;
	}

	return(0);
}
