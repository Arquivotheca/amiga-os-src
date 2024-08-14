/*
	DoSpecial for Okimate-20 driver.
	David Berezowski - March/88.
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
	extern struct PrinterExtendedData *PED;

	int x = 0, i = 0;
	static char initThisPrinter[] =
		"\033I\001\022\0330\033%H\033-\376\015\033W";
	static BYTE ISOcolTable[10] = {2, 1, 0, 0, 2, 1, 2, 0, 2, 2};

	if (*command == aRIN) {
		while(x < 15) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		outputBuffer[11] = '\000';
		outputBuffer[x++] = '\000';

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[2] = '\002';
		}

		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = ':';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[x++] = '\017';
		}

		*currentVMI = 27; /* assume 8 lpi (27/216 = 1/8) */
		if (PD->pd_Preferences.PrintSpacing == SIX_LPI) {
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = 'A';
			outputBuffer[x++] = '\014';
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = '2';
			*currentVMI = 36; /* 36/216 = 1/6 */
		}
		return(x);
	}

	if (*command == aNEL) {
		outputBuffer[x++] = '\015';
		outputBuffer[x++] = '\012';
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

	if (*command == aVERP0) {
		*currentVMI = 27;
	}
	if (*command == aVERP1) {
		*currentVMI = 36;
	}

	return(0);
}
