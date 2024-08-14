/* Okidata_292I special commands */

/****** printer.device/printers/Okidata_292I_special_functions ***********
 *
 *   NAME
 *   Okidata_292I special functions implemented:
 * 
 ************************************************************************/

#include	"exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define NUMINIT	22

extern struct PrinterData *PD;
extern char *CommandTable[];

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];

{
	int x, y;
	char *point, inchar;
	static unsigned char initMarg[] = "\033Xlr";
	/*
		Cmds are:

	    0 -  2	ESC%H		- stop italics
	    3 -  5	ESC-NULL	- stop underline
	    6 -  7	ESCF		- stop emphasized
	    8 - 10	ESCWNULL	- stop double width
	   11 - 12	ESCH		- stop enhanced
	   13 - 15	ESCI1		- utility (draft) quality
	   16 - 17	ESCT		- stop super/sub script
	   18 - 20	ESC%Q		- proportional spacing off
	   21 - 21	CR			- carriage-return (no lf)
	*/
	static char initThisPrinter[NUMINIT] =
		"\033%H\033-\376\033F\033W\376\033H\033I1\033T\033%Q\015";
   static BYTE ISOcolorTable[10]= {0,5,6,4,3,1,2,0};

	x = y = 0;

	if (*command == aRIN) { /* initialize */
		while (x < NUMINIT) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		outputBuffer[5] = 0; /* stop under line */
		outputBuffer[10] = 0; /* double width off */

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[15] = 0;
		}

		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = 0;
			*currentVMI = 27; /* 216/27 => 8 */
		}
		else { /* 6 LPI */
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = 'A';
			outputBuffer[x++] = '\014';
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = '2';
			*currentVMI=36; /* 216/36 => 6 */
		}

		if (PD->pd_Preferences.PrintPitch == PICA) {
			outputBuffer[x++] = '\022'; /* 10 CPI */
		}
		else if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = ':'; /* 12 CPI */
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[x++] = '\017'; /* 17 CPI */
		}

		point = CommandTable[aTBSALL];
		while ((inchar = point[y++]) !=0) {
			if (inchar == -2) {
				inchar = '\000'; /*kludge for 0 */
			}
			outputBuffer[x++] = inchar;
		}
		y=0;

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command=aSLRM;
	}

	if (*command == aCAM) {
		Parms[0] = 1;
		Parms[1] = 255;
		*command = aSLRM;
	}

	if (*command == aSLRM) { /* set left and right margins */
		initMarg[2] = Parms[0];
		initMarg[3] = Parms[1];
		while (y < 4) {
			outputBuffer[x++] = initMarg[y++];
		}
		return(x);
	}

	if (*command == aPLU) { /* partial line up */
		if (*vline == 0) {
			*vline = 1;
			*command = aSUS2; /* superscript on */
			return(0);
		}
		if (*vline < 0) {
			*vline = 0;
			*command = aSUS3; /* subscript off */
			return(0);
		}
		return(-1);
	}

	if (*command == aPLD) { /* partial line down */
		if (*vline == 0) {
			*vline = -1;
			*command = aSUS4; /* subscript on */
			return(0);
		}
		if (*vline > 0) {
			*vline = 0;
			*command = aSUS1; /* superscript off */
			return(0);
		}
		return(-1);
	}

	if (*command == aSUS0) { /* normalize the line */
		*vline = 0;
	}
	else if (*command == aSUS1) { /* superscript off */
		*vline = 0;
	}
	else if (*command == aSUS2) { /* superscript on */
		*vline = 1;
	}
	else if (*command == aSUS3) { /* subscript off */
		*vline = 0;
	}
	else if (*command == aSUS4) { /* subscript on */
		*vline = -1;
	}

	if (*command == aVERP0) { /* 1/8 inch line spacing */
		*currentVMI=27;
	}
	else if (*command == aVERP1) { /* 1/6 inch line spacing */
		*currentVMI=36;
	}

	if (*command == aIND) { /* linefeed */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'J';
		outputBuffer[x++] = *currentVMI;
		return(x);
	}

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled=253;
	}

	if (*command == aSFC) {
		if (Parms[0] == 39) {
			Parms[0] = 30; /* set defaults */
		}
		if (Parms[0] > 37) {
			return(0); /* ni or background color change */
		}
        outputBuffer[x++] = '\033';
        outputBuffer[x++] = 'r';
        outputBuffer[x++] = ISOcolorTable[Parms[0] - 30];
	   return(x);
   }

	return(0);
}
