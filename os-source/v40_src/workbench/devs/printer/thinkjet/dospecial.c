/*
	Special functions for HP_ThinkJet driver.
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
		00-03	\033&d@		underline off
		04-08	\033(s0B	boldface off
		09-13	\033&k0S	normal pitch
		14-18	\033&l6D	6 lpi
		19-19	\r		carriage-return
	*/
	static char initThisPrinter[20] = "\033&d@\033(s0B\033&k0S\033&l6D\r";
	int x;

	x = 0;
	if (*command == aRIN || *command == aRIS) { /* initialize/reset */
		while (x < 20) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		*currentVMI = 6; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			*currentVMI = 8;
			outputBuffer[17] = '8';
		}
		return(x);
	}

	if (*command == aVERP0) { /* 8 lpi */
		*currentVMI = 8;
	}
	else if (*command == aVERP1) { /* 6 lpi */
		*currentVMI = 6;
	}

	if (*command == aSLPP) { /* set form length */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = '&';
		outputBuffer[x++] = 'l';
		outputBuffer[x++] = Parms[0];
		outputBuffer[x++] = 'P';
		return(x);
	}

	return(0);
}
