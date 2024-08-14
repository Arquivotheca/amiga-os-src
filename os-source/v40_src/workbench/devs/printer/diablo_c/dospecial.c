/*
	DoSpecial for Diablo_C-150 driver.
	David Berezowski - June/88.
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define INITLEN		1
#define TABLEN		34

#define LMARG		2
#define RMARG		7
#define MARGLEN		10
/*
	00-04	\033l00\015	- set left margin to '00'	LMARG
	05-09	\033r00\015	- set right margin to '00'	RMARG
*/
UBYTE MargBuf[MARGLEN] = "\033l00\015\033r00\015";

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
UBYTE *currentVMI; /* used for color on this printer */
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	int x = 0, y= 0;
	static BYTE ISOcolorTable[10] =
		{49, 51, 53, 52, 55, 50, 54, 48, 49, 49};
	/*       K   R   G   Y   B   M   C   W   K   K */

	/*
		00-00	\015	- carriage return
	*/
	static char initThisPrinter[INITLEN] = "\015";
	static unsigned char initTabs[TABLEN] =
		"\033i9 17 25 33 41 49 57 65 73 81 89\015";

	if (*command == aRIN) {
		while(x < INITLEN) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		while (y < TABLEN) {
			outputBuffer[x++] = initTabs[y++];
		}
		y = 0;

		*currentVMI = 0x70; /* white background, black text */

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aCAM) {
		Parms[0] = 1;
		Parms[1] = (90 * 10 + 5) / 10; /* max is 9.0 inches @ 10 cpi */
		*command = aSLRM;
	}

	if (*command == aSLRM) {
		CalcMarg(Parms[0], Parms[1]);
		while (y < MARGLEN) {
			outputBuffer[x++] = MargBuf[y++];
		}
		return(x);
	}

	if (*command == aSFC) { /* set foreground/background color */
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
		outputBuffer[x++] = '@';
		outputBuffer[x++] = ISOcolorTable[*currentVMI & 15];
		outputBuffer[x++] = ISOcolorTable[(*currentVMI & 240) / 16];
		return(x);
	}

	if (*command == aRIS) {
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}

CalcMarg(left, right)
int left, right;
{
	int i, j, offset, max;

	/*
		The minimum left margin on the Diablo_C-150 is .5 inches.
		Thus a left margin of 1 (ie. no left margin) is 5/10 => .5.
		The maximum print width is 9.0 inches.
	*/
	offset = 40;
	max = (90 * 10 + 5) / 10;
	if ((i = (left * 10 + offset + 5) / 10) > max) {
		i = max;
	}
	MargBuf[LMARG + 0] = ((i % 100) / 10) + '0';
	MargBuf[LMARG + 1] = (i % 10) + '0';
	if ((i = (right * 10 + offset + 15) / 10) > max) {
		i = max;
	}
	MargBuf[RMARG + 0] = ((i % 100) / 10) + '0';
	MargBuf[RMARG + 1] = (i % 10) + '0';
	return(MARGLEN);
}
