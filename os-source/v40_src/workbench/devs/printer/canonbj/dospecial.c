/**********************************************************/
/* Copyright   ©1991 Wolf-Juergen Faust                   */
/* Am Dorfgarten 10                                       */
/* W-6000 Frankfurt 50   Canon BJ 10 driver               */
/* Germany                                                */
/* FIDO: 2:243/43.5 (Wolf Faust)                          */
/* UUCP: cbmvax.commodore.com!cbmehq!cbmger!venus!wfaust  */
/* Tel: +(49) 69 5486556                                  */
/*                                                        */
/* This File is NOT PART OF THE DISTRIBUTION package !!!  */
/*          DO NOT SPREAD THIS SOURCE FILE !!!            */
/*                                                        */
/* Versions required:                                     */
/* SAS/C 5.10a                                            */
/**********************************************************/

#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <clib/all_protos.h>

UBYTE charsize = 0;

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

	int x = 0, y = 0;

/*

 0- 1   ESC T   cancel super/subscript mode
 2- 4   ESC - 0 cancel underline
 5- 7   ESC _ 0 cancel overscoring
 8- 9   ESC H   cancel double strike
10-11   ESC F   cancel emphased mode (bold mode)
12-14   ESC U 0 bi-directional
15-16   ESC R   set all tabs to default setting
17-19   ESC P 0	cancel PS Mode
20-22   ESC I charsize+font+HQ  select 10cpi HQ font (resident or download)
23-24   ESC '2' set 1/6 linefeed (don't use ESC A n!)
25-25   \015		carriage return

*/

	static char initThisPrinter[26] = { 27,'T',27,'-',0xfe,27,
		'_',0xfe,27,'H',27,'F',27,'U',0xfe,27,'R',27,'P',0xfe,27,'I',2,27,'2',13};

	static char initMarg[] = {0xfd,0x1b,'X','L','R',0x0d};

	static UBYTE quality = 2, font = 0, prop = 0;

	if (*command == aRIN) { /* initialize */
		while(x < 26) {
			if ((outputBuffer[x] = initThisPrinter[x]) == -2) {
				outputBuffer[x] = 0;
			}
			x++;
		}

		*currentVMI = 30; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[24] = '0';
			*currentVMI = 22;
		}

		prop = 0;
		quality = (PD->pd_Preferences.PrintQuality == LETTER) ? 2 : 0;

		switch(PD->pd_Preferences.PrintPitch)
		{
			case FINE:
				charsize = 16;
				break;
			case ELITE:
				charsize = 8;
				break;
			case PICA:
			default:
				charsize = 0;
				break;
		}

		outputBuffer[22] = quality+charsize+font;

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command=aSLRM;
	}

	if (*command == aCAM) { /* cancel margins */
		y = PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
		if (PD->pd_Preferences.PrintPitch == PICA)   /* pica */
		{
			Parms[1] = (10 * y) / 10;
		}    /* elite */
		else if (PD->pd_Preferences.PrintPitch == ELITE)
		{
			Parms[1] = (12 * y) / 10;
		}
		else /* fine */
		{
			Parms[1] = ((17 * y) / 10) + 2;
		}
		Parms[0] = 1;
		y = 0;
		*command = aSLRM;
	}

	if (*command == aSLRM) { /* set left&right margins */
		PD->pd_PWaitEnabled = 253; /* wait after this character */
		if (Parms[0] == 0)
		{
			initMarg[3] = 0;
		}
		else
		{
			initMarg[3] = Parms[0];
		}
		initMarg[4] = Parms[1];
		while (y < 6)
		{
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
		quality = 2;
		prop = font = charsize = 0;
	}

	if (*command == aSHORP0) { /* normal pitch */
		charsize = 0;
	}

	if (*command == aSHORP1) { /* elite off */
		charsize = 0;
	}

	if (*command == aSHORP2) { /* elite on */
		charsize = 8;
	}

	if (*command == aSHORP1) { /* condensed fine off */
		charsize = 0;
	}

	if (*command == aSHORP2) { /* condensed fine on */
		charsize = 16;
	}

	if (*command == aDEN2) { /* NLQ on */
		quality = 2;
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'I';
		outputBuffer[x++] = charsize + 2 + font;
		if (prop)
		{
			outputBuffer[x++] = 27;
			outputBuffer[x++] = 'P';
			outputBuffer[x++] = 1;
		}
		return(x);
	}

	if (*command == aDEN1) { /* NLQ off */
		quality = 0;
		outputBuffer[x++] = 27;
		outputBuffer[x++] = 'I';
		outputBuffer[x++] = charsize + font;
		if (prop)
		{
			outputBuffer[x++] = 27;
			outputBuffer[x++] = 'P';
			outputBuffer[x++] = 1;
		}
		return(x);
	}

	if (*command == aPROP2) { /* proportional on */
		prop = 1;
	}
	if (*command == aPROP1) { /* proportional off */
		prop = 0;
	}

	switch(*command)
	{
		case aFNT0:
			charsize = 0;
			font = 0;
			prop = 0;
			quality = 2;
			break;
		case aFNT1:
			charsize = 8;
			font = 0;
			prop = 0;
			quality = 2;
			break;
		case aFNT2:
			charsize = 16;
			font = 0;
			prop = 0;
			quality = 2;
			break;
		case aFNT3:
			charsize = 0;
			font = 0;
			prop = 1;
			quality = 2;
			break;
		case aFNT4:
			charsize = 0;
			font = 4;
			prop = 0;
			quality = 0;
			break;
		case aFNT5:
			charsize = 8;
			font = 4;
			prop = 0;
			quality = 0;
			break;
		case aFNT6:
			charsize = 16;
			font = 4;
			prop = 0;
			quality = 0;
			break;
		case aFNT7:
			charsize = 0;
			font = 4;
			prop = 0;
			quality = 2;
			break;
		case aFNT8:
			charsize = 8;
			font = 4;
			prop = 0;
			quality = 2;
			break;
		case aFNT9:
			charsize = 16;
			font = 4;
			prop = 0;
			quality = 2;
			break;
		case aFNT10:
			charsize = 0;
			font = 4;
			prop = 1;
			quality = 2;
			break;
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

ClosePrint(ior)
struct printerIO *ior;
{
	if (PD->pd_Preferences.PaperType == SINGLE)
	{
		if  (PED->ped_PrintMode)  /* if data has been printed */
		{
			(*(PD->pd_PWrite))("\014",1); /* eject page */
			(*(PD->pd_PBothReady))(); /* wait for it to finish */
			PED->ped_PrintMode = 0; /* no data to print */
		}
	}
	return(0);
}
