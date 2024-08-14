/*** cbm_epson/dospecial.c *********************************************
 *
 * dospecial.c -- do anything which cannot be handled via data 
 *
 *	$Id: dospecial.c,v 1.5 91/07/08 14:19:56 darren Exp $
 *
 *	Copyright (c) 1988,1991 Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 **********************************************************************/

#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

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
	extern char *CommandTable[];

	int x = 0, y = 0, z;
	char *point, inchar;
	static unsigned char initMarg[4] = "\033Xlr";
	/*
		00-02	\033x\001	NLQ on
		03-04	\0332		6 lpi
		05-05	\022		condensed off
		06-08	\0335\000	auto linefeed off
		09-11	\033-\000	underline off
		12-13	\033F		boldface off
		14-14	\015		carriage return
		15-17	\033W\000	enlarge off
		18-19	\033T		subscript off
	*/
	static char initThisPrinter[20] = 
		"\033x\001\0332\022\0335\000\033-\000\033F\015\033W\000\033T";

	if(*command==aRIN) {
		while(x < 20) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}
		if (PD->pd_Preferences.PrintQuality == DRAFT) {
			outputBuffer[2] = 0;
		}
		*currentVMI = 36; /* assume 1/6 line spacing */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[4] = '0';
			*currentVMI=27;
		}
		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[x++]='\033';
			outputBuffer[x++] = ':';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[5] = '\017';
		}

		point = CommandTable[aTBSALL];
		while ((inchar = point[y++]) !=0) {
			if (inchar == -2) {
				inchar= 0; /*kludge for 0 */
			}
			outputBuffer[x++] = inchar;
		}
		y=0;

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command=aSLRM;
	}

	if(*command==aCAM) {
		Parms[0] = 1;
		z = PED->ped_MaxColumns;

		if (PD->pd_Preferences.PrintPitch == FINE) {
			Parms[1] = (z * 12)/10;
		}
		else if (PD->pd_Preferences.PrintPitch == ELITE) {
			Parms[1] = (z * 17)/10;
		}
		else {
			Parms[1] = z; 
		}
		*command = aSLRM;
	}

if(*command==aSLRM) {
        initMarg[2]=Parms[0];
        initMarg[3]=Parms[1];
        while(y<4)outputBuffer[x++]=initMarg[y++];
	return(x);
}

if(*command==aPLU) {
	if((*vline)==0){(*vline)=1; *command=aSUS2; return(0);}
	if((*vline)<0){(*vline)=0; *command=aSUS3; return(0);}
	return(-1);
}
if(*command==aPLD) {
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


if(*command==aIND) {
	outputBuffer[x++]='\033';
	outputBuffer[x++]='J';
	outputBuffer[x++]= *currentVMI;
	return(x);
}
if(*command==aRIS) PD->pd_PWaitEnabled=253;

return(0);
}
