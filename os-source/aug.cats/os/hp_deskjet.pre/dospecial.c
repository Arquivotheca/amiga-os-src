/*** deskjet/dospecial.c **********************************************
 *
 * dospecial.c -- Handle any sequences which cannot be handled via data
 *
 *	$Id: dospecial.c,v 38.0 92/01/16 11:58:21 eric Exp $
 *
 *	Copyright (c) 1987,1991, Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 * 37.3 - fix InitForm Top margin (change '1' '0' '0' '2' to 'l' '0' '0' '2')
 * 37.4 - set ped_PrintMode when text received
 **********************************************************************/

#include "exec/types.h"
#include "devices/printer.h"
#include "devices/prtbase.h"
#include "intuition/preferences.h"

#define LPI		7
#define CPI		15
#define QUALITY		17
#define INIT_LEN	30
#define LPP		7
#define FORM_LEN	11
#define LEFT_MARG	3
#define RIGHT_MARG	7
#define MARG_LEN	12
#define PAGELEN_SET	8
#define LPI_SET		3
#define LEN_LEN		12

#define LPI_SIX		6
#define LPI_EIGHT	8

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

	static UWORD textlength, topmargin;
	int x, y, j, k, l;

	int lpi = LPI_SIX;
	int pprsz;

	/*
		00-03	\033&d@		Underline off
		04-08	\033&l6D	6 lines per inch
		09-11	\033(s		Begin Font Selection
		12-13	0b		Medium stroke weight
		14-16	10h		Primary Pitch (10)
		17-18	1q		Print Quality (Draft)
		19-20	0p		Primary Spacing (fixed)
		21-22	0s		Primary Style (upright)
		23-24	3t		Primary Font (Courier)
		25-26	0u		Font Placement (Normal)
		27-29	12V		Point Size (12)
	*/

	static char initThisPrinter[INIT_LEN+1] =
		{27, '&', 'd', '@', 27, '&','l','6','D',27, '(','s','0','b','1','0','h','1','q','0','p','0','s','3','t','0','u','1','2','V'};

	/*	 0   1234   56789   01234567890123456789	*/
	/*       0		    1         2			*/


	/* InitForm - Top Margin 2 lines plus # of lines of text	*/
	static char initForm[FORM_LEN+1] = {27,'&','l','0','0','2','e','0','0','0','F'};

	/* Margins left, and right					*/

	static char initMarg[MARG_LEN+1] = {27,'&','a','0','0','0','l','0','0','0','M',13};

	/* Top Margin set, and # of lines of text			*/

	static char initTMarg[] = {27,'&','l','0','0','0','e','0','0','0','F'};

	/* Tell printer about paper length				*/
	/* Change LPI too so printer won't be confused.			*/

	/**
	 ** SIZE      |	LPI (6)	      |	LPI (8)
	 ** -------------------------------------
	 ** US_LETTER |	66	      |	88		(11 * LPI)
	 ** US_LEGAL  |	84	      |	112		(14 * LPI)
	 ** EURO_A4   |	70	      |	93
	 **
	 **/

	static char initLength[LEN_LEN+1] = {27,'&','l','6','D',27,'&','l','0','6','6','P'};

	x = y = j = k = l = 0;

	if (*command == aRIN) {

		/* Tell printer what size paper to use	*/

		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			lpi = LPI_EIGHT;
		}

		pprsz = PD->pd_Preferences.PaperSize;

		switch(pprsz)
		{
			case US_LETTER:
				pprsz = 11 * lpi;
				break;

			case US_LEGAL:
				pprsz = 14 * lpi;
				break;

			case EURO_A4:
				pprsz = 70;		/* SIX_LPI	*/

				if(lpi == EIGHT_LPI)
				{
					pprsz = 93;	/* EIGHT_LPI	*/
				}
				break;

			default:
				pprsz = 0;
		}

		/* Send LPI, and page length sequence for paper sizes
		 * US_LEGAL, US_LETTER, and A4 sizes.  For all other
		 * sizes (e.g., CUSTOM), do no send this sequence.
		 * Allow possibility of user selecting some paper size
		 * manually (and using CUSTOM size from preferences).
		 */

		if(pprsz)
		{
			while(k < LEN_LEN)
			{
				outputBuffer[x++] = initLength[k++];
			}
			numberString(pprsz, PAGELEN_SET, outputBuffer);

			if(lpi == LPI_EIGHT)
			{
				outputBuffer[LPI_SET] = '8';
			}
		}

		/* Rest of init string */

		while(l < INIT_LEN) {
			outputBuffer[x++] = initThisPrinter[l++];
		}
		outputBuffer[x++] = '\015';


		if (lpi == LPI_EIGHT) {
			outputBuffer[LPI + k] = '8';
		}

		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[CPI + k] = '2';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[CPI + k] = '5';
		}

		if (PD->pd_Preferences.PrintQuality == LETTER) {
			outputBuffer[QUALITY + k] = '2';
		}

		j = x; /* set the formlength = textlength, top margin = 2 */
		textlength = PD->pd_Preferences.PaperLength;
		topmargin = 2;

		while (y < FORM_LEN) {
			outputBuffer[x++] = initForm[y++];
		}
		numberString(textlength, j + LPP, outputBuffer);

		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aSLRM) {
		j = x;
		y = 0;
		while(y < MARG_LEN) {
			outputBuffer[x++] = initMarg[y++];
		}
		numberString(Parms[0] - 1, j + LEFT_MARG, outputBuffer);
		numberString(Parms[1] - 1, j + RIGHT_MARG, outputBuffer);
		return(x);
	}

	if ((*command == aSUS2) && (*vline == 0)) {
		*command = aPLU;
		*vline = 1;
		return(0);
	}

	if ((*command == aSUS2) && (*vline < 0)) {
		*command = aRI;
		*vline = 1;
		return(0);
	}

	if ((*command == aSUS1) && (*vline > 0)) {
		*command = aPLD;
		*vline = 0;
		return(0);
	}

	if ((*command == aSUS4) && (*vline == 0)) {
		*command = aPLD;
		*vline = -1;
		return(0);
	}

	if ((*command == aSUS4) && (*vline > 0)) {
		*command = aIND;
		*vline = -1;
		return(0);
	}

	if ((*command == aSUS3) && (*vline < 0)) {
		*command = aPLU;
		*vline = 0;
		return(0);
	}

	if(*command == aSUS0) {
		if (*vline > 0) {
			*command = aPLD;
		}
		if (*vline < 0) {
			*command = aPLU;
		}
		*vline = 0;
		return(0);
	}

	if (*command == aPLU) {
		(*vline)++;
		return(0);
	}

	if (*command == aPLD){
		(*vline)--;
		return(0);
	}

	if (*command == aSTBM) {
		if (Parms[0] == 0) {
			Parms[0] = topmargin;
		}
		else {
			topmargin = --Parms[0];
		}

		if (Parms[1] == 0) {
			Parms[1] = textlength;
		}
		else {
			textlength=Parms[1];
		}
		while (x < 11) {
			outputBuffer[x] = initTMarg[x];
			x++;
		}
		numberString(Parms[0], 3, outputBuffer);
		numberString(Parms[1] - Parms[0], 7, outputBuffer);
		return(x);
	}

	if (*command == aSLPP) {
		while(x < 11) {
			outputBuffer[x] = initForm[x];
			x++;
		}
		/*restore textlength, margin*/
		numberString(topmargin, 3, outputBuffer);
		numberString(textlength, 7, outputBuffer);
		return(x);
	}

	if (*command == aRIS) {
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}

numberString(Param, x, outputBuffer)
UBYTE Param;
int x;
char outputBuffer[];
{
	if (Param > 199) {
		outputBuffer[x++] = '2';
		Param -= 200;
	}
	else if (Param > 99) {
		outputBuffer[x++] = '1';
		Param -= 100;
	}
	else {
		outputBuffer[x++] = '0'; /* always return 3 digits */
	}

	if (Param > 9) {
		outputBuffer[x++] = Param / 10 + '0';
	}
	else {
		outputBuffer[x++] = '0';
	}

	outputBuffer[x] = Param % 10 + '0';
	return(0);
}

ConvFunc(buf, c, flag)
char *buf, c;
int flag; /* expand lf into lf/cr flag (0-yes, else no ) */
{
	if (c == '\014') { /* if formfeed (page eject) */
		PED->ped_PrintMode = 0; /* no data to print */
	}
	else if(c >= 0x21) {
		PED->ped_PrintMode = 1; /* data to print 37.4 */
	}
	return(-1); /* pass all chars back to the printer device */
}

Close(ior)
struct printerIO *ior;
{
	if (PED->ped_PrintMode) { /* if data has been printed */
		(*(PD->pd_PWrite))("\014",1); /* eject page */
		(*(PD->pd_PBothReady))(); /* wait for it to finish */
		PED->ped_PrintMode = 0; /* no data to print */
	}
	return(0);
}
