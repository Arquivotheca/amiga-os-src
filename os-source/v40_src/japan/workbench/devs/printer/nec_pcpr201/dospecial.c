/*
 * dospecial.c --- Handle any sequences which cannot be handled via data
 *
 */

#include "exec/types.h"
#include "devices/printer.h"
#include "devices/prtbase.h"
#include "intuition/preferences.h"

#include "clib/jcc_protos.h"
#include "pragmas/jcc_pragmas.h"

#include "string.h"

#define ESCP_SUPER 0	/* for using ESCP/Super emulate mode */

extern struct Library *JCCBase;

#if ESCP_SUPER
#define INIT_LEN    20
#else
#define INIT_LEN    26
#endif
#define LPI_SIX     6
#define LPI_EIGHT   8
#define MARG_LEN    10
#define LEFT_MARG   2
#define RIGHT_MARG  7
#define HTAB_LEN    2

char ShiftCode = 'E';	/* shift code for back up */

/**********    debug macros     **********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
#define bug kprintf
#if MYDEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

int DoSpecial(UWORD *, char *, BYTE *, BYTE *, BYTE *, UBYTE *);
int numberString(UBYTE, int, char *);
int ConvFunc(UBYTE *, UBYTE, int);
int Open(struct printerIO *);
int Close(struct printerIO *);

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

	int x, y;
	int htab;
	int mstart;

	int lpi = LPI_SIX;
	int pprsz;

	/*
		00-05	\034c,,0.   Normal char set (italics, shadow, boldface off)
		06-07	\033Y       Underline off
		08-09	\033A       6 lines per inch
		10-11	\033f       Normal LF direction
		12-13	\033\042    Double stroke off
		14-15	\033H       HD pika
		16-19	\033e11     Enlarge off
		20-22	\033d1      NLQ on
		23-25	\033s0      Script off
    	*/
#if ESCP_SUPER
	static char initThisPrinter[] =
	"\033Y\033A\033f\033\042\033H\033e11\033d1\033s0";
#else
	static char initThisPrinter[] =
	"\034c,,0.\033Y\033A\033f\033\042\033H\033e11\033d1\033s0";
#endif

	/* InitLength - 70 (A4) of lines of text. */
	static char initLength[] = "\033v70.";

	/* Margins left, and right */
	static char initMarg[] = "\033L000\033/000";

	/* Horizen tab */
	static char initTab[] = "\033(";

	/* Tell printer about paper length */
	/**
	 ** SIZE      | LPI (6) | LPI (8)         | Real size
	 ** -----------------------------------------------------
	 ** US_LETTER | 66      | 88   (11 * LPI) |
	 ** US_LEGAL  | 84      | 112  (14 * LPI) |
	 ** A4        | 70      | 93              | (210 x 297 mm)
	 ** A5        |         |                 | (148 x 210 mm)
	 ** B4        |         |                 | (257 x 364 mm)
	 ** B5        |         |                 | (182 x 257 mm)
	 ** POSTCARD  |         |                 | (100 x 148 mm)
	 **/

	x = y = 0;

	D(bug("command:%ld,", *command));

	/* Tell printer what size paper to use	*/
	if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI)
	{
		lpi = LPI_EIGHT;
	}

    switch (PD->pd_Preferences.PaperSize)
	{
		case US_LETTER:
			pprsz = 11 * lpi;
			break;
		case US_LEGAL:
			pprsz = 14 * lpi;
			break;
    	case EURO_A4:
			pprsz = 70;			/* SIX_LPI */
    		if(lpi == EIGHT_LPI)
			{
				pprsz = 93;		/* EIGHT_LPI */
			}
			break;
		/* In this point, have to use CUSTOM. */
//		case EURO_A5:
//		case EURO_B4:
//		case EURO_B5:
//		case POSTCARD:
		default:
			pprsz = 0;
	}

	if (*command == aRIN)
	{
		while (x < INIT_LEN)
		{
			outputBuffer[x++] = initThisPrinter[y++];
		}

		/*
		 * Send LPI, and page length sequence for paper sizes
		 * US_LEGAL, US_LETTER, and A4 sizes.  For all other
		 * sizes (e.g., CUSTOM), do no send this sequence.
		 * Allow possibility of user selecting some paper size
		 * manually (and using CUSTOM size from preferences).
		 */

		/*** Set paper length ***/
		if (pprsz == 0)
			pprsz = PD->pd_Preferences.PaperLength;

#if ESCP_SUPER
#else
		D(bug("pprsz = %ld\n", pprsz));
		if (pprsz && pprsz < 100)
		{
			y = 0;
			while (y < 5)
			{
				outputBuffer[x++] = initLength[y++];
			}

			outputBuffer[2 + INIT_LEN] = pprsz/10 + '0';
			outputBuffer[3 + INIT_LEN] = (pprsz - (pprsz/10 * 10)) + '0';

			if (lpi == LPI_EIGHT)
			{
				outputBuffer[9] = 'B';
			}
		}
#endif
		/*** Set print pitch ***/
		if (PD->pd_Preferences.PrintPitch == ELITE)
		{
			outputBuffer[15] = 'E';
		}
		else if (PD->pd_Preferences.PrintPitch == FINE)
		{
			outputBuffer[15] = 'Q';
		}

		/*** Set print quality ***/
		if (PD->pd_Preferences.PrintQuality == DRAFT)
		{
			outputBuffer[22] = '0';
		}

		/*** Set left and right margin ***/
		mstart = x;
		y = 0;
		while (y < MARG_LEN)
		{
			outputBuffer[x++] = initMarg[y++];
		}

		numberString(PD->pd_Preferences.PrintLeftMargin, mstart + LEFT_MARG, outputBuffer);
		numberString(PD->pd_Preferences.PrintRightMargin, mstart + RIGHT_MARG, outputBuffer);

		/*** Set horizen tab ***/
		y = 0;
		while (y < HTAB_LEN)
		{
			outputBuffer[x++] = initTab[y++];
		}
		y = 0;
		htab = PD->pd_Preferences.PaperSize == W_TRACTOR ? (136/8-1) : (80/8-1);
		while (y < htab)
		{
			y++;
			numberString(8 * y,	x, outputBuffer);
			x += 3;
			outputBuffer[x++] = ',';
		}
		outputBuffer[x-1] = '.';	/* end of horiaen tab */

		return(x);
	}

	if (*command == aLMS)	/* Set left margin */
	{
		y = 0;
		while (y < 5)
		{
			outputBuffer[x++] = initMarg[y++];
		}
		numberString(Parms[0] - 1, LEFT_MARG, outputBuffer);
		return(x);
	}

	if (*command == aRMS)	/* Set right margin */
	{
		y = 5;
		while (y < MARG_LEN)
		{
			outputBuffer[x++] = initMarg[y++];
		}
		numberString(Parms[0] - 1, LEFT_MARG, outputBuffer);
		return(x);
	}

	if (*command == aCAM)	/* Clear left and right margin */
	{
		/*** Clear bottom margin with using VFU ***/
		if (pprsz == 0)
			pprsz = PD->pd_Preferences.PaperLength;

		D(bug("pprsz = %ld\n", pprsz));

		y = 0;
		while (y < 4)
		{
			outputBuffer[x++] = initLength[y++];
		}

		if (pprsz && pprsz < 100)
		{
			outputBuffer[2] = pprsz/10 + '0';
			outputBuffer[3] = (pprsz - (pprsz/10 * 10)) + '0';
		}
		outputBuffer[x++] = ',';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = '0';
		outputBuffer[x++] = '.';

		/*** Clear left and right margin ***/
		Parms[0] = PD->pd_Preferences.PrintLeftMargin;
		Parms[1] = PD->pd_Preferences.PrintRightMargin;
		*command = aSLRM;
	}

	if (*command == aSLRM)	/* Set left and right margin */
	{
		mstart = x;
		y = 0;
		while (y < MARG_LEN)
		{
			outputBuffer[x++] = initMarg[y++];
		}
		numberString(Parms[0] - 1, mstart + LEFT_MARG, outputBuffer);
		numberString(Parms[1] - 1, mstart + RIGHT_MARG, outputBuffer);
		return(x);
	}

	if (*command == aSTBM)	/* Set top and bottom margin */
	{
		Parms[0] = Parms[1];	/* No top margin for this printer */
		*command = aBMS;
	}

	if (*command == aBMS)	/* Set bottom margin */
	{
		/*** Set bottom margin with using VFU ***/
		if (pprsz == 0)
			pprsz = PD->pd_Preferences.PaperLength;

		D(bug("pprsz = %ld\n", pprsz));

		y = 0;
		while (y < 4)
		{
			outputBuffer[x++] = initLength[y++];
		}

		if (pprsz && pprsz < 100)
		{
			outputBuffer[2] = pprsz/10 + '0';
			outputBuffer[3] = (pprsz - (pprsz/10 * 10)) + '0';
		}
		outputBuffer[x++] = ',';
		outputBuffer[x++] = Parms[0]/10 + '0';
		outputBuffer[x++] = (Parms[0] - (Parms[0]/10 * 10)) + '0';
		outputBuffer[x++] = '.';
//D(bug("x = %ld\n", x));
//D(bug("buf = %02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx\n",outputBuffer[0],outputBuffer[1],outputBuffer[2],outputBuffer[3],outputBuffer[4],outputBuffer[5],outputBuffer[6],outputBuffer[7]));
		return(x);
	}

	if (*command == aHTS)	/* Set horizen tab */
	{
		y = 0;
		while (y < HTAB_LEN)
		{
			outputBuffer[x++] = initTab[y++];
		}
		y = 0;
		while (Parms[y] && Parms[y] < 136)
		{
			numberString(Parms[y], x, outputBuffer);
			x += 3;
			outputBuffer[x++] = ',';
			y++;
		}
		outputBuffer[x-1] = '.';	/* end of horiaen tab */
		return(x);
	}

	if (*command == aTBC0)	/* Clear horizen tab */
	{
		y = 0;
		while (y < HTAB_LEN)
		{
			outputBuffer[x++] = initTab[y++];
		}
		outputBuffer[x-1] = ')';
		y = 0;
		while (Parms[y] && Parms[y] < 136)
		{
			numberString(Parms[y], x, outputBuffer);
			x += 3;
			outputBuffer[x++] = ',';
			y++;
		}
		outputBuffer[x-1] = '.';	/* end of horiaen tab */
		return(x);
	}

	if (*command == aSLPP)	/* Set form length */
	{
		y = 0;
		while (y < 5)
		{
			outputBuffer[x++] = initLength[y++];
		}
		outputBuffer[2] = Parms[0]/10 + '0';
		outputBuffer[3] = (Parms[0] - (Parms[0]/10 * 10)) + '0';
		return(x);
	}

	if (*command == aSHORP0
	 || *command == aSHORP1
	 || *command == aSHORP3)		/* HD pika */
	{
		ShiftCode = 'H';
		return(0);
	}

	if (*command == aSHORP2)		/* elite */
	{
		ShiftCode = 'E';
		return(0);
	}

	if (*command == aSHORP4)		/* condensed fine */
	{
		ShiftCode = 'Q';
		return(0);
	}

	if (*command == aPROP2)			/* propotional */
	{
		ShiftCode = 'P';
		return(0);
	}

	if (*command == aRIS)
	{
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}

numberString(Param, x, outputBuffer)
UBYTE Param;
int x;
char outputBuffer[];
{
	if (Param > 199)
	{
		outputBuffer[x++] = '2';
		Param -= 200;
	}
	else if (Param > 99)
	{
		outputBuffer[x++] = '1';
		Param -= 100;
	}
	else
	{
		outputBuffer[x++] = '0'; /* always return 3 digits */
	}

	if (Param > 9)
	{
		outputBuffer[x++] = Param / 10 + '0';
	}
	else
	{
		outputBuffer[x++] = '0';
	}

	outputBuffer[x] = Param % 10 + '0';
	return(0);
}

struct JConversionCodeSet *jcc;

ConvFunc(buf, c, flag)
UBYTE *buf;
UBYTE c;
int flag;	/* expand lf into lf/cr flag (0-yes, else no) */
{
	LONG count;
	UBYTE outbuf[10];
	LONG i;
	LONG shift;

//	D(bug("c:%02lx ", c));

	switch(c)
	{
		case FF:	/* if formfeed (page eject) */
			PED->ped_PrintMode = 0; /* no data to print */
			return(-1);
		case LF:
			SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, TAG_DONE);
			return(-1);
	}

	count = JStringConvert(jcc, &c, outbuf, CTYPE_EUC, CTYPE_NECJIS, -1, 0);

	/* Save original shift code */
	GetJConversionAttrs(jcc, JCC_ShiftedIn, &shift, TAG_DONE);
	if (!shift)
	{
		if (outbuf[0] == ESC)
			if (outbuf[1] != ShiftCode)
				outbuf[1] = ShiftCode;
	}

//	if (shift == FALSE && outbuf[0] == ESC)
//		D(bug("%d:%02x%02x%02x",count, outbuf[0], outbuf[1], outbuf[2]));

	if (count == 1)
		return(-1);	/* pass all chars back to the printer device */

	for (i = 0; i < count; i++)
		buf[i] = outbuf[i];

	return(count);
}

Open(ior)
struct printerIO *ior;
{
	D(bug("Open...\n"));

	if (!(jcc = AllocConversionHandle(TAG_DONE)))
		return(-1);

	return(0);
}

Close(ior)
struct printerIO *ior;
{
	LONG shift = FALSE;
	UBYTE shiftout[4];

	D(bug("Close...\n"));

	GetJConversionAttrs(jcc, JCC_ShiftedIn, &shift, TAG_DONE);
	if (shift)		/* Make shift out code */
	{
		shiftout[0] = ESC;
		shiftout[1] = ShiftCode;
		shiftout[2] = CR;
	}

	if(jcc)
		FreeConversionHandle(jcc);

	if (PED->ped_PrintMode)		/* if data has been printed */
	{
		if (shift)		/* Need shift out */
		{
			(*(PD->pd_PWrite))(shiftout,3);
		}
//		(*(PD->pd_PWrite))("\014",1);	/* eject page */
		(*(PD->pd_PBothReady))();	/* wait for it to finish */
		PED->ped_PrintMode = 0;		/* no data to print */
	}

	return(0);
}
