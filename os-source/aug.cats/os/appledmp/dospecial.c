/*************************************************************************
 *
 *   NAME
 *   ImagewriterII special functions
 *
 ************************************************************************/

#include "exec/types.h"
#include "devices/printer.h"
#include "devices/prtbase.h"

extern struct PrinterData *PD;

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	int x = 0;
	int temp = 0;
	static char pica = 0; /* if 0 then we are pica */
	static char prop = 0; /* if 0 then we are NOT proportional */
	/* order is  BLACK,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE */
	static BYTE ISOcolorTable[10] = {0, 2, 5, 1, 3, 6, 3, 1};
	static char initThisPrinter[] = "\033c\033l0\033A\015\033s4";

	if(*command == aRIS) {
		PD->pd_PWaitEnabled=253;
	}
	else if(*command == aRIN) { /* initialize */
		while (x < 11) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		pica = prop = 0; /* set us back to pica and no prop */
		if (PD->pd_Preferences.PrintQuality != DRAFT) {
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = 'a';
			outputBuffer[x++] = '2'; /* select near letter quality */
		}
		*currentVMI=24; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) { /* wrong again */
			outputBuffer[x++] = '\033';
			outputBuffer[x++] = 'B';
			*currentVMI=18;
		}	
		outputBuffer[x++] = '\033';
		if (PD->pd_Preferences.PrintPitch == PICA) {
			outputBuffer[x++] = 'N';
		}
		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[x++] = 'E';
			pica = 1;
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[x++]='q';
			pica = 2;
		}
		Parms[0]= PD->pd_Preferences.PrintLeftMargin;
		*command=aLMS; /* left margin set */
	}

	/* left margin set or right margin set */
	if ((*command == aLMS) || (*command == aSLRM)) {
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'L';
		outputBuffer[x++] = (--Parms[0]) / 100 + '0';
		outputBuffer[x++] = (Parms[0] % 100) / 10 + '0';
		outputBuffer[x++] = (Parms[0] % 10) + '0';
		return(x);
	}

	if(*command == aCAM) { /* clear all margins */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'L';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = '0';
		return(x);
	}

	if (*command == aSUS0) { /* normalize the line */
		if (*vline > 0) {
			*command = aPLD;
			*vline = 1; 
		}
		else if (*vline < 0) {
			*command = aPLU;
			*vline = -1;
		}
		else {
			return(0);
		}
	}

	if (*command == aPLU) { /* partial line up */
		(*vline)++;
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'l';
		outputBuffer[x++] = '1';
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'T';
		temp = *currentVMI / 2;
		outputBuffer[x++] = (temp % 100) / 10 + '0';
		outputBuffer[x++] = (temp % 10) + '0';
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'r';
		outputBuffer[x++] = '\012';
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'f';
		outputBuffer[x++] = '\033';
		if (*currentVMI == 24) {
			outputBuffer[x++] = 'A';
		}
		else {
			outputBuffer[x++] = 'B';
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'l';
		outputBuffer[x++] = '0';
		return(x);
	}

	if (*command == aPLD) { /* partial line down */
		(*vline)--;
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'l';
		outputBuffer[x++] = '1';
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'T';
		temp = (*currentVMI) / 2;
		outputBuffer[x++] = (temp % 100) / 10 + '0';
		outputBuffer[x++] = (temp % 10) + '0';
		outputBuffer[x++] = '\012';
		outputBuffer[x++] = '\033';
		if (*currentVMI == 24) {
			outputBuffer[x++] = 'A';
		}
		else {
			outputBuffer[x++] ='B';
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'l';
		outputBuffer[x++] = '0';
		return(x);
	}

	if (*command == aVERP0) {
		*currentVMI=18;
	}

	if (*command == aVERP1) {
		*currentVMI=24;
	}

	if (*command == aSHORP2) { /* elite on */
		pica = 1;
		outputBuffer[x++] = '\033';
		if (prop == 0) {
			outputBuffer[x++] = 'E';
		}
		else {
			outputBuffer[x++]='P';
		}
		return(x);
	}

	/* elite or condensed off */
	if ((*command == aSHORP1) || (*command == aSHORP3)) {
		pica = 0;
		outputBuffer[x++] = '\033';
		if (prop == 0) {
			outputBuffer[x++] = 'N';
		}
		else {
			outputBuffer[x++] = 'p';
		}
		return(x);
	}

	if (*command == aSHORP4) { /* condensed on */
		pica = 2;
	}

	if (*command == aPROP0) {
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 's';
		outputBuffer[x++] = '4';
		return(x);
	}

	if (*command == aPROP2) { /* prop on */
		prop = 1;
		outputBuffer[x++] = '\033';
		if (pica == 2) {
			outputBuffer[x++] = 'q';
		}
		else if (pica == 1) {
			outputBuffer[x++] = 'P';
		}
		else {
			outputBuffer[x++] = 'p';
		}
		return(x);
	}

	if (*command==aPROP1) { /* prop off */
		prop = 0;
		outputBuffer[x++] = '\033';
		if (pica == 2) {
			outputBuffer[x++] = 'q';
		}
		else if (pica == 1) {
			outputBuffer[x++] = 'E';
		}
		else {
			outputBuffer[x++] = 'N';
		}
		return(x);
	}

	if (*command == aTSS) {
		if (Parms[0] == 0) {
			Parms[0] = 5;
		}
		if (Parms[0] > 10) {
			Parms[0] = 10;
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 's';
		outputBuffer[x++] = (--Parms[0]) + '0';
		return(x);
	}

	if (*command == aSFC) {
		/* if default or white */
		if (Parms[0] == 39 || Parms[0] == 37) {
			Parms[0] = 30; /* make black */
		}
		if (Parms[0] > 37) {
			return(0); /* NI or background color change */
		}
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'K';
		outputBuffer[x++] = ISOcolorTable[Parms[0] - 30] + '0';
		return(x);
	}
	return(0);
}
