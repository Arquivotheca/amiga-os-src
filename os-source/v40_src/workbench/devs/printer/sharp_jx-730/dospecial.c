/*
	DoSpecial for Sharp_JX-730 driver.
	David Berezowski - Dec/89.
*/

#include "exec/types.h"
#include "devices/printer.h"
#include "devices/prtbase.h"

#define INIT_LEN	16

/*
	00-00	\017		enlarge off (SI)
	01-02	\033Y		underline off
	03-04	\033\042	boldface off
	05-07	\033d0		draft (vs standard print mode)
	08-09	\0332		clear all h and v tabs
	10-12	\033s0		superscript off
	13-13	\015		carriage-return
	14-15	\033H		pica pitch
*/

UBYTE initThisPrinter[INIT_LEN] =
    "\017\033Y\033\042\033d0\0332\033s0\015\033H";

UBYTE ISOcolorTable[10] = {48, 50, 52, 54, 49, 51, 53, 55, 48, 48};
			/*  K   R   G   Y   B   M   C   W   K   K */

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
UBYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	int x = 0;
	if (*command == aRIN) {
		while(x < INIT_LEN) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	/* Set Left and Right Margins */
	if (*command == aSLRM) {
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'L';
		outputBuffer[x++] = (Parms[0] / 100) | '0';
		outputBuffer[x++] = ((Parms[0] / 10) % 10) | '0';
		outputBuffer[x++] = (Parms[0] % 10) | '0';
		/*
		   to set a right margin of n, you must set
		   the right margin to n+1 on this printer
		*/
		if (Parms[1] < 999) Parms[1]++;
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = '/';
		outputBuffer[x++] = (Parms[1] / 100) | '0';
		outputBuffer[x++] = ((Parms[1] / 10) % 10) | '0';
		outputBuffer[x++] = (Parms[1] % 10) | '0';
		return(x);
	}

	if (*command == aSFC) { /* Set Foreground/Background Color */
		if (Parms[0] > 37) {
			Parms[0] = 30; /* set defaults */
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = ISOcolorTable[Parms[0] - 30];
		return(x);
	}

	if (*command == aPLU) {
		if (*vline == 0) {
			*vline = 1;
			*command = aSUS2;
			return(0);
		}
		if (*vline < 0) {
			*vline = 0;
			*command = aSUS3;
			return(0);
		}
		return(-1);
	}

	if (*command == aPLD) {
		if (*vline == 0) {
			*vline = -1;
			*command = aSUS4;
			return(0);
		}
		if (*vline > 0) {
			*vline = 0;
			*command = aSUS1;
			return(0);
		}
		return(-1);
	}

	if (*command == aSUS0) {
		*vline = 0;
	}
	if (*command == aSUS1) {
		*vline = 0;
	}
	if (*command == aSUS2) {
		*vline = 1;
	}
	if (*command == aSUS3) {
		*vline = 0;
	}
	if (*command == aSUS4) {
		*vline = -1;
	}

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}
