/*
	DoSpecial for Tektronix_4696 driver.
	David Berezowski - May/88.
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

/*
	00-01	\033H	- cmd header
	02-03	10	- # of tab stops (max is 10)
	04-05	10	- left margin (from left edge, min is 10)
	06-25	08	- intertab spacing of 8 chars (10 times)
	26-27	04	- right margin (from right edge, min is 04)
	28-28	\015	- carriage-return
*/
UBYTE MargBuf[29] = "\033H10100808080808080808080804\015";

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
UBYTE *currentVMI; /* used for color on this printer */
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	int x = 0, y= 0, z = 0;
	static BYTE ISOcolorTable[10] =
		{49, 51, 53, 52, 55, 50, 54, 48, 49, 49};
	/*       K   R   G   Y   B   M   C   W   K   K */
	/*
		00-00	/021	normal char set
		01-01	/015	carriage-return
	*/
	static char initThisPrinter[] = "\021\015";
	if (*command == aRIN) {
		while(x < 2) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		*currentVMI = 0x70; /* white background, black text */
		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aTBSALL) { /* Set Default Tabs */
		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aCAM) { /* Cancel Margins */
		Parms[0] = 1;
		Parms[1] = 85;
		*command = aSLRM;
	}

	/* Set Left and Right Margins (also handles tabs) */
	if (*command == aSLRM) {
		z = CalcMarg(Parms[0], Parms[1]);
		do {
			outputBuffer[x++] = MargBuf[y++];
		} while (--z);
		return(x);
	}

	if (*command == aSFC) { /* Set Foreground/Background Color */
		if (Parms[0] == 39) {
			Parms[0] = 30; /* set defaults */
		}
		if (Parms[0] == 49) {
			Parms[0] = 47;
		}
		if (Parms[0] < 40) {
			*currentVMI = (*currentVMI & 240) + (Parms[0] - 30);
		}
		else {
			*currentVMI = (*currentVMI & 15) + (Parms[0] - 40) *
				16;
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = ISOcolorTable[*currentVMI & 15];
		outputBuffer[x++] = ISOcolorTable[(*currentVMI & 240) / 16];
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

	if (*command == aSLPP) { /* set form length */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'L';
		outputBuffer[x++] = (Parms[0] / 10) | '0';
		outputBuffer[x++] = (Parms[0] % 10) | '0';
		return(x);
	}

	return(0);
}

CalcMarg(left, right)
int left, right;
{
	int x = 0, y = 0;

	if ((left += 9) > 85) {
		left = 85;
	}
	left &= ~1; /* must be an even # */
	if (right > 85) {
		right = 85;
	}
	right = 90 - right;
	right &= ~1; /* must be an even # */
	MargBuf[x++] = '\033';
	MargBuf[x++] = 'H'; /* cmd header */
	/* calc # of tabs (max of 10) */
	if ((y = (99 - left - right) / 8) > 10) {
		y = 10;
	}
	MargBuf[x++] = y / 10 + '0';
	MargBuf[x++] = (y % 10) + '0'; /* # of tabs */
	MargBuf[x++] = left / 10 + '0';
	MargBuf[x++] = (left % 10) + '0'; /* left margin */
	while (y > 0) {
		MargBuf[x++] = '0';
		MargBuf[x++] = '8'; /* 8 chars per tab */
		y--;
	}
	MargBuf[x++] = right / 10 + '0';
	MargBuf[x++] = (right % 10) + '0'; /* right margin */
	return(x);
}
