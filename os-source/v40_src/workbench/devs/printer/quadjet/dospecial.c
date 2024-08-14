/*
	Special functions for Quadram_QuadJet driver.
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
		00-02	\033F\006	1/6" line spacing
		03-04	\033N		enlarge characters off
		05-07	\033C\000	black text
	*/
	static char initThisPrinter[8] = "\033F\006\033N\033C\000";
	int x = 0;

	if (*command == aRIN || *command == aRIS) { /* initialize/reset */
		while (x < 8) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		*currentVMI = 6; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[2] = 8;
			*currentVMI = 8;
		}
		return(x);
	}

	if (*command == aVERP0) { /* 8 lpi */
		*currentVMI = 8;
	}
	else if (*command == aVERP1) { /* 6 lpi */
		*currentVMI = 6;
	}
	else if (*command == aSFC) { /* set foreground/background color */
		if (Parms[0] == 39) {
			Parms[0] = 30; /* default foreground to black */
		}
		else if (Parms[0] > 37) {
			return(0);
		}
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = Parms[0] - 30;
		return(x);
	}

	return(0);
}
