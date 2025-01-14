/*
	DoSpecial for Toshiba_P351SX driver.
	David Berezowski - March/88.
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define VMI6	9	/* 6 lpi */
#define VMI8	7	/* 8 lpi */

UBYTE HexTable[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	int x = 0, y = 0;
	/*
		00-01	\033\024	italics off
		02-03	\033J		underline off
		04-05	\033M		boldface off
		06-08	\033*0		highspeed font (draft)
		09-10	\033]		condensed fine off
		11-12	\033\042	enlarge off
		13-14	\033R		shadow print off
		15-16	\033%		proportional off
		17-19	\033\036\011	6 lpi
		20-23	\033E12		10 cpi
		24-29	\033~S010	super/sub script off
		30	CR		carriage return
	*/
	static UBYTE initThisPrinter[31] =
"\033\024\033J\033M\033*0\033]\033\042\033R\033%\033\036\011\033E12\033~S010\r";
	/*
		00-03	\033CLL		move to position 'LL'
		04-05	\0339		set left margin
		06-09	\033CRR		move to position 'RR'
		10-11	\0330		set right margin
		12-12	\r		carriage return
	*/
	static UBYTE initMarg[13] = "\033CLL\0339\033CRR\0330\r";
	static UBYTE ISOcolorTable[8] =
		{'0', '3', '5', '1', '6', '2', '4', '0'};
		/*K    R    G    Y    B    M    C    W */

	if (*command == aRIN) { /* initialize */
		while(x < 31) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[8] = '2';
		}

		*currentVMI = VMI6; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[19] = 7;
			*currentVMI = VMI8;
		}

		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[23] = '0';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[10] = '[';
		}

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aCAM) { /* clear margins */
		Parms[0] = 1;
		Parms[1] = 160;
		*command = aSLRM;
	}

	if (*command == aSLRM) { /* set left&right margins */
		if (Parms[0] > 160) {
			Parms[0] = 160;
		}
		else if (Parms[0] == 0) {
			Parms[0] = 1;
		}
		if (Parms[1] > 160) {
			Parms[1] = 160;
		}
		Parms[0]--;
		Parms[1]--;
		initMarg[2] = HexTable[Parms[0] / 10];
		initMarg[3] = HexTable[Parms[0] % 10];
		initMarg[8] = HexTable[Parms[1] / 10];
		initMarg[9] = HexTable[Parms[1] % 10];
		do {
			outputBuffer[x++] = initMarg[y++];
		} while (y < 13);
		return(x);
	}

	if (*command == aPLU) { /* partial line up */
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

	if (*command == aPLD) { /* partial line down */
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

	if (*command == aSUS0) { /* normalize the line */
		*vline = 0;
	}

	if (*command == aSUS1) { /* superscript off */
		*vline = 0;
	}

	if (*command == aSUS2) { /* superscript on */
		*vline = 1;
	}

	if (*command == aSUS3) { /* subscript off */
		*vline = 0;
	}

	if (*command == aSUS4) { /* subscript on */
		*vline = -1;
	}

	if (*command == aVERP0) { /* 8 LPI */
		*currentVMI = VMI8;
	}

	if (*command == aVERP1) { /* 6 LPI */
		*currentVMI = VMI6;
	}

	if (*command == aSFC) { /* set foreground/background color */
		if (Parms[0] == 39) {
			Parms[0] = 30; /* set default (black) */
		}
		if (Parms[0] > 37) {
			return(0); /* ni or background color change */
		}
		outputBuffer[x++] = 27;
		outputBuffer[x++] = '~';
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = '2';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = ISOcolorTable[Parms[0] - 30];
		return(x);
	}

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}
