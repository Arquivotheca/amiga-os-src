/*
	DoSpecial for Okidata_92 driver.
	David Berezowski - June/88.
*/

#include "exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

#define PITCH	4
#define QUALITY	6
#define LPI	12
#define INITLEN	14

#define LMARG	3
#define MARGLEN	6

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
	extern struct PrinterData *PD;

	int x = 0, y = 0, z = 0;
	/*
		00-05	\033%C001	- set left margin to 1.
	*/
	static char initMarg[MARGLEN] = "\033%C001";
	/*
		00-01	\033D	- underline off
		02-03	\033I	- boldface off
		04-04	\036	- 10 cpi
		05-06	\0330	- draft
		07-08	\033K	- superscript off
		09-10	\033M	- subscript off
		11-12	\0336	- 6 lpi
		13-13	\015	- carriage-return
	*/
	static char initThisPrinter[INITLEN] =
		"\033D\033I\036\0330\033K\033M\0336\015";
	static UBYTE pitch = 100;

	if (*command == aRIS) { /* reset */
		PD->pd_PWaitEnabled = 253;
	}

	if (*command==aRIN) { /* init */
		while (x < INITLEN) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[QUALITY] = '1';
		}

		*currentVMI = 36; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[LPI] = '8';
			*currentVMI = 27;
		}

		if (PD->pd_Preferences.PrintPitch == PICA) {
			pitch = 100;
		}
		else if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[PITCH] = '\034';
			pitch = 120;
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[PITCH] = '\035';
			pitch = 171;
		}

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		*command=aSLRM;
	}

	if (*command == aCAM) { /* clear margins */
		Parms[0] = 1;
		*command = aSLRM;
	}

	if (*command == aSLRM) { /* set left&right margin(s) */
		/* convert from 120ths/inch to chars @ current cpi */
		z = (Parms[0] - 1) * 1200 / pitch + 1;
		initMarg[LMARG] = ((z % 1000) / 100) | '0';
		initMarg[LMARG + 1] = ((z % 100) / 10) | '0';
		initMarg[LMARG + 2] = (z % 10) | '0';
		while (y < MARGLEN) {
			outputBuffer[x++] = initMarg[y++];
		}
		return(x);
	}

	if (*command == aSLPP) { /* set form length */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = 'F';
		outputBuffer[x++] = ((Parms[0] % 100) / 10) | '0';
		outputBuffer[x++] = (Parms[0] % 10) | '0';
		return(x);
	}

if(*command==aPLU) { /* partial line up */
	if((*vline)==0){(*vline)=1; *command=aSUS2; return(0);}
	if((*vline)<0){(*vline)=0; *command=aSUS3; return(0);}
	return(-1);
}
if(*command==aPLD) { /* partial line down */
	if((*vline)==0){(*vline)=(-1); *command=aSUS4; return(0);}
	if((*vline)>0){(*vline)=0; *command=aSUS1; return(0);}
	return(-1);
}

if(*command==aSUS0) *vline=0;
if(*command==aSUS1) *vline=0;
if(*command==aSUS2) *vline=1;
if(*command==aSUS3) *vline=0;
if(*command==aSUS4) *vline=(-1);

if(*command==aVERP0) *currentVMI=27;

if(*command==aVERP1) *currentVMI=36;

	if (*command==aIND) { /* linefeed */
		outputBuffer[x++] = '\033';
		outputBuffer[x++] = '\022';
		return(x);
	}
 
	/* normal char set, elite off, condensed off, enlarge off */
	if (*command == aSHORP0 || *command == aSHORP1 ||
		*command == aSHORP3 || *command == aSHORP5) {
		pitch = 100;
	}
	else if (*command == aSHORP2) { /* ELITE, 12 cpi */
		pitch = 120;
	}
	else if (*command == aSHORP4) { /* FINE, 17.1 cpi */
		pitch = 171;
	}

	return(0);
}
