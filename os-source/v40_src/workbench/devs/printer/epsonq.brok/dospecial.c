/*
	DoSpecial for EpsonQ driver.
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

	int x = 0, y = 0;
	/*
		00-01	\0335		italics off
		02-04	\033-\000	underline off
		05-06	\033F		boldface off
		07-08	\033P		elite off
		09-09	\022		condensed fine off
		10-12	\033W\000	enlarge off
		13-14	\033H		double strike off
		15-17	\033x\000	NLQ off
		18-19	\033T		normalize the line
		20-22	\033R\000	US char set
		23-25	\033p0		proportional off
		26-27	\0332		6 lpi
		28-28	\015		carriage-return
	*/
	static char initThisPrinter[29] =
"\0335\033-\376\033F\033P\022\033W\376\033H\033x\376\033T\033R\376\033p0\0332\015";
	static char initMarg[] = "\375\033lL\033QR\375";
	static BYTE ISOcolorTable[] = {0, 5, 6, 4, 3, 1, 2, 0};

	if (*command == aRIN) { /* initialize */
		while(x < 29) {
			if ((outputBuffer[x] = initThisPrinter[x]) == -2) {
				outputBuffer[x] = 0;
			}
			x++;
		}

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[17] = 1;
		}

		*currentVMI = 30; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[27] = '0';
			*currentVMI = 22;
		}

		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[8] = 'M';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[9] = 15;
		}

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command=aSLRM;
	}

	if (*command == aCAM) { /* cancel margins */
		y = PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
		if (PD->pd_Preferences.PrintPitch == PICA) {
			Parms[1] = (10 * y) / 10;
		}
		else if (PD->pd_Preferences.PrintPitch == ELITE) {
			Parms[1] = (12 * y) / 10;
		}
		else { /* fine */
			Parms[1] = (17 * y) / 10;
		}
		Parms[0] = 1;
		y = 0;
		*command = aSLRM;
	}

	if (*command == aSLRM) { /* set left&right margins */
		PD->pd_PWaitEnabled = 253; /* wait after this character */
		if (Parms[0] == 0) {
			initMarg[3] = 0;
		}
		else {
			initMarg[3] = Parms[0] - 1;
		}
		initMarg[6] = Parms[1];
		while (y < 8) {
			outputBuffer[x++] = initMarg[y++];
		}
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
		*currentVMI = 22;
	}

	if (*command == aVERP1) { /* 6 LPI */
		*currentVMI = 30;
	}

	if (*command == aSFC) { /* set foreground/background color */
		if (Parms[0] == 39) {
			Parms[0] = 30; /* set default (black) */
		}
		if (Parms[0] > 37) {
			return(0); /* ni or background color change */
		}
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'r';
		outputBuffer[x++] = ISOcolorTable[Parms[0] - 30];
		return(x);
	}

	if (*command == aSLPP) { /* set form length */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'C';
		outputBuffer[x++] = Parms[0];
		return(x);
	}

	if (*command == aPERF) { /* perf skip n */
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'N';
		outputBuffer[x++] = Parms[0];
		return(x);
	}

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}
