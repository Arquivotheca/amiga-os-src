/*
	DoSpecial.c for HP_PaintJet.
	David Berezowski - April/88.

	Handles those difficult escape sequences that cannot be performed
	with a simple translation.

	Originally written by Nick V. Flor of Hewlett Packard - SDD
	Modified by David Berezowski of Commodore-Amiga
*/

#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

#define PLU 4   /* ie. ESC&a-00V */
#define RLF 4   /* ie. ESC&a-000V */
#define PEN             3
#define HALF            4
#define MAX_TEXT_COLORS 8

#define PITCH			12
#define LPI			21
#define INIT_LEN		30
/*
	0-3	\033&d@			- underline off
	4-8	\033(s0B		- boldface off
	9-13	\033&k0S		- normal pitch
	14-17	\033(0U			- U.S. char set
	18-22	\033&l6D		- 6 lpi
	23-29	\033&v000S		- black text
*/
char InitSeq[INIT_LEN] =
	"\033&d@\033(s0B\033&k0S\033(0U\033&l6D\033&v000S";

#define PAGE		3
#define FORM_LEN	7
/*
	0-6		\033&l066F	- text length of 66 lines
*/
char InitForm[FORM_LEN] = "\033&l066F";

static char SlctPen[] = "\033&v000S";

DoSpecial(CmdPtr, OutBuf, VLine, CurrVMI, CRLFFlag, ParmBuf)
char OutBuf[];
UWORD *CmdPtr;
BYTE *VLine, *CurrVMI, *CRLFFlag;
UBYTE ParmBuf[];
{
	extern struct PrinterData *PD;
	extern char ReverseLF[], PartialUp[];

	static char sp;
	int pp = PD->pd_Preferences.PaperLength;
	int pg = PAGE;
	int pn, i, j;

	i = j = 0;

	switch (*CmdPtr) {

	case aVERP0: /* 8 LPI */
		sp = '8';
		return(0);
	break;

	case aVERP1: /* 6 LPI */
		sp = '6';
		return(0);
	break;

	case aRIN: /* Initialize */
		while (i < INIT_LEN) {
			OutBuf[i++] = InitSeq[j++];
		}
		if (PD->pd_Preferences.PrintSpacing == SIX_LPI) {
			sp = '6';
		}
		else {
			sp = '8';
		}
		OutBuf[LPI] = sp; /* set spacing */
		if (PD->pd_Preferences.PrintPitch == ELITE) {
			OutBuf[PITCH] = '4';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			OutBuf[PITCH] = '2';
		}
		AddNumber(pp, InitForm, pg);
		j = 0;
		while (j < FORM_LEN) {
			OutBuf[i++] = InitForm[j++];
		}
		return(i);
	break;

	case aSLPP: /* Set Form Length */
		AddNumber(pp, InitForm, pg);
		while(j < FORM_LEN) {
			OutBuf[i++] = InitForm[j++];
		}
		return(i);
	break;

	case aRI: /* Reverse_lf */
	DoaRI:
		if (sp == '8') { /* 8 LPI */
			ReverseLF[RLF] = '0';
			ReverseLF[RLF + 1] = '9';
			ReverseLF[RLF + 2] = '0';
		}
		else { /* 6 LPI */
			ReverseLF[RLF]  = '1';
			ReverseLF[RLF + 1] = '2';
			ReverseLF[RLF + 2] = '0';
		}
        return(0);
	break;

	case aSFC: /* Set Foreground Color */
		if (ParmBuf[0] == 39) {
			ParmBuf[0] = 30; /* default(39) is Black(30) */
		}
		if (ParmBuf[0] >= 30 && ParmBuf[0] <=38) {
			while (SlctPen[i]) {
				OutBuf[i] = SlctPen[i];
				++i;
			}
			pn = ParmBuf[0] - 30;
			pn %= MAX_TEXT_COLORS;
			AddNumber(pn, OutBuf, PEN);
			return(i);
		}
	break;

	case aSUS2: /* Superscript On */
		if (*VLine == 0) {
			*CmdPtr = aPLU;
			goto DoaPLU;
		}
		else if (*VLine < 0) {
			*CmdPtr = aRI;
			*VLine = 1;
			goto DoaRI;
		}
	break;

	case aSUS1: /* Superscript Off */
		if (*VLine > 0) {
			*CmdPtr = aPLD;
			*VLine = 0;
			return(0);
		}
	break;

	case aSUS4: /* Subscript On */
		if (*VLine == 0) {
			*CmdPtr = aPLD;
			*VLine = (-1);
		}
		else if (*VLine > 0) {
			*CmdPtr = aIND;
			*VLine = (-1);
		}
		return(0);
	break;

	case aSUS3: /* Subscript Off */
		if (*VLine < 0) {
			*CmdPtr = aPLU;
			*VLine = (-1);
			goto DoaPLU;
		}
	break;

	case aSUS0: /* Normalize */
		if (*VLine > 0) {
			*CmdPtr = aPLD;
			*VLine = 0;
			return(0);
		}
		else if (*VLine < 0) {
			*CmdPtr = aPLU;
			*VLine = (-1);
			goto DoaPLU;
		}
	break;

	case aPLU: /* Partial Line Up */
	DoaPLU:
		if (sp == '8') { /* 8 LPI */
			PartialUp[PLU] = '4';
			PartialUp[PLU+1] = '5';
		}
		else { /* 6 LPI */
			PartialUp[PLU] = '6';
			PartialUp[PLU+1] = '0';
		}
		++(*VLine);
		return(0);
	break;

	case aPLD: /* Partial Line Down */
		--(*VLine);
		return(0);
	break;
	}
	return(0);
}

/*
 * AddNumber:
 *     Adds a 3 digit number to a buffer.
 *
 * Entry:
 *     Number -- The number in question.
 *     Buffer -- The buffer to put it in.
 *     Start  -- Where to start insertion.
 *
 */
AddNumber(Number, Buffer, Start)
int Number, Start;
char Buffer[];
{
	int Remain;

	Remain = Number % 10;
	Buffer[Start + 2] = (char)(Remain + '0');
	Number /= 10;
	Remain = Number % 10;
	Buffer[Start + 1] = (char)(Remain + '0');
	Number /= 10;
	Buffer[Start] = (char)(Number + '0');
}
