/*
	DoSpecial for CalComp_ColorMaster driver.
	David Berezowski - March/88.
	(copy of EpsonX driver except for removal of aSFC and addition of
	 Close and ConvFunc routines)
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define LMARG	3
#define RMARG	6
#define MARGLEN	9

#define CONDENSED	7
#define PITCH		9
#define QUALITY		17
#define LPI		24
#define INITLEN		26

extern struct PrinterData *PD;
extern struct PrinterExtendedData *PED;

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	int x = 0, y = 0;
	/*
		00-00	\375	wait
		01-03	\033lL	set left margin
		04-06	\033Qq	set right margin
		07-07	\375	wait
		08-08	\015	carriage return
	*/
	static char initMarg[MARGLEN] = "\375\033lL\033Qq\375\015";
	/*
		00-01	\0335		italics off
		02-04	\033-\000	underline off
		05-06	\033F		boldface off
		07-07	\022		cancel condensed mode
		08-09	\033P		select pica (10 cpi)
		10-12	\033W\000	enlarge off
		13-14	\033H		doublestrike off
		15-17	\033x\000	draft
		18-19	\033T		super/sub script off
		20-22	\033p0		proportional off
		23-24	\0332		6 lpi
		25-25	\015		carriage return
	*/
	static char initThisPrinter[INITLEN] =
"\0335\033-\000\033F\022\033P\033W\000\033H\033x\000\033T\033p0\0332\015";

	if (*command == aRIN) {
		while (x < INITLEN) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[QUALITY] = 1;
		}

		*currentVMI = 36; /* assume 1/6 line spacing (36/216 => 1/6) */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[LPI] = '0';
			*currentVMI = 27; /* 27/216 => 1/8 */
		}

		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[PITCH] = 'M';
		}
		else if (PD->pd_Preferences.PrintPitch == CONDENSED) {
			outputBuffer[CONDENSED] = '\017'; /* condensed */
			outputBuffer[PITCH] = 'P'; /* pica condensed */
		}

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
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

	if (*command == aSLRM) { /* set left and right margins */
		PD->pd_PWaitEnabled = 253;
		if (Parms[0] == 0) {
			initMarg[LMARG] = 0;
		}
		else {
			initMarg[LMARG] = Parms[0] - 1;
		}
		initMarg[RMARG] = Parms[1];
		while (y < MARGLEN) {
			outputBuffer[x++] = initMarg[y++];
		}
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

	if (*command == aIND) { /* lf */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'J';
		outputBuffer[x++] = *currentVMI;
		return(x);
	}

	if (*command == aRI) { /* reverse lf */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'j';
		outputBuffer[x++] = *currentVMI;
		return(x);
	}

	if (*command == aRIS) {
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}

ConvFunc(buf, c, flag)
char *buf, c;
int flag; /* expand lf into lf/cr flag (0-yes, else no ) */
{
	if (c == '\014') { /* if formfeed (page eject) */
		PED->ped_PrintMode = 0; /* no data to print */
	}
	return(-1); /* pass all chars back to the printer device */
}

Close(ior)
struct printerIO *ior;
{
	if(PED->ped_PrintMode) { /* if data has been printed */
		(*(PD->pd_PWrite))("\014",1); /* eject page */
		(*(PD->pd_PBothReady))(); /* wait for it to finish */
		PED->ped_PrintMode = 0; /* no data to print */
	}
	return(0);
}
