
/* *** ascii2pc.c ***********************************************************
 * 
 * Routines that send ASCII to the PC
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  --------------------------------------------
 * 16 Dec 86  =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"


extern	UBYTE	AmigaToPCTable[];



/* === ASCII to PC Keycode ============================================== */
UBYTE ASCIIToPCTable[128 * 2] =
		{
		PC_CONTROL | PC_SHIFT, 3, /* ^@ */
 
		PC_CONTROL, 30,		/* ^A */
		PC_CONTROL, 48,		/* ^B */
		PC_CONTROL, 46,		/* ^C */
		PC_CONTROL, 32,		/* ^D */
		PC_CONTROL, 18,		/* ^E */
		PC_CONTROL, 33,		/* ^F */
		PC_CONTROL, 34,		/* ^G */
		PC_CONTROL, 35,		/* ^H */
		PC_CONTROL, 23,		/* ^I */
		PC_CONTROL, 50,		/* ^J ??? Was equal to 36, now 50 for \n from Notepad */
		PC_CONTROL, 37,		/* ^K */
		PC_CONTROL, 38,		/* ^L */
		PC_CONTROL, 50,		/* ^M */
		PC_CONTROL, 49,		/* ^N */
		PC_CONTROL, 24,		/* ^O */
		PC_CONTROL, 25,		/* ^P */
		PC_CONTROL, 16,		/* ^Q */
		PC_CONTROL, 19,		/* ^R */
		PC_CONTROL, 31,		/* ^S */
		PC_CONTROL, 20,		/* ^T */
		PC_CONTROL, 22,		/* ^U */
		PC_CONTROL, 47,		/* ^V */
		PC_CONTROL, 17,		/* ^W */
		PC_CONTROL, 45,		/* ^X */
		PC_CONTROL, 21,		/* ^Y */
		PC_CONTROL, 44,		/* ^Z */
		PC_NOTHING, 1,		/* ESCAPE KEY */
		PC_NOTHING, 0,		/* ??? 28 */
		PC_NOTHING, 0,		/* ??? 29 */
		PC_NOTHING, 0,		/* ??? 30 */
		PC_NOTHING, 0,		/* ??? 31 */
		PC_NOTHING, 57,		/*   */
		PC_SHIFT,  2,		/* ! */
		PC_SHIFT, 40,		/* " */
		PC_SHIFT,  4,		/* # */
		PC_SHIFT,  5,		/* $ */
		PC_SHIFT,  6,		/* % */
		PC_SHIFT,  8,		/* & */
		PC_NOTHING, 40,		/* ' */
		PC_SHIFT, 10,		/* ( */
		PC_SHIFT, 11,		/* ) */
		PC_SHIFT,  9,		/* * */
		PC_SHIFT, 13,		/* + */
		PC_NOTHING, 51,		/* , */
		PC_NOTHING, 12,		/* - */
		PC_NOTHING, 52,		/* . */
		PC_NOTHING, 53,		/* / */
		PC_NOTHING, 11,		/* 0 */
		PC_NOTHING,  2,		/* 1 */
		PC_NOTHING,  3,		/* 2 */
		PC_NOTHING,  4,		/* 3 */
		PC_NOTHING,  5,		/* 4 */
		PC_NOTHING,  6,		/* 5 */
		PC_NOTHING,  7,		/* 6 */
		PC_NOTHING,  8,		/* 7 */
		PC_NOTHING,  9,		/* 8 */
		PC_NOTHING, 10,		/* 9 */
		PC_SHIFT, 39,		/* : */
		PC_NOTHING, 39,		/* ; */
		PC_SHIFT, 51,		/* < */
		PC_NOTHING, 13,		/* = */
		PC_SHIFT, 32,		/* > */
		PC_SHIFT, 53,		/* ? */

		PC_SHIFT,  3,		/* @ */
		PC_SHIFT, 30,		/* A */
		PC_SHIFT, 48,		/* B */
		PC_SHIFT, 46,		/* C */
		PC_SHIFT, 32,		/* D */
		PC_SHIFT, 18,		/* E */
		PC_SHIFT, 33,		/* F */
		PC_SHIFT, 34,		/* G */
		PC_SHIFT, 35,		/* H */
		PC_SHIFT, 23,		/* I */
		PC_SHIFT, 36,		/* J */
		PC_SHIFT, 37,		/* K */
		PC_SHIFT, 38,		/* L */
		PC_SHIFT, 50,		/* M */
		PC_SHIFT, 49,		/* N */
		PC_SHIFT, 24,		/* O */
		PC_SHIFT, 25,		/* P */
		PC_SHIFT, 16,		/* Q */
		PC_SHIFT, 19,		/* R */
		PC_SHIFT, 31,		/* S */
		PC_SHIFT, 20,		/* T */
		PC_SHIFT, 22,		/* U */
		PC_SHIFT, 47,		/* V */
		PC_SHIFT, 17,		/* W */
		PC_SHIFT, 45,		/* X */
		PC_SHIFT, 21,		/* Y */
		PC_SHIFT, 44,		/* Z */
		PC_NOTHING, 26,		/* [ */
		PC_NOTHING, 43,		/* \ */
		PC_NOTHING, 27,		/* ] */
		PC_SHIFT,  7,		/* ^ */
		PC_SHIFT, 12,		/* _ */

		PC_NOTHING, 41,		/* ` */
		PC_NOTHING, 30,		/* a */
		PC_NOTHING, 48,		/* b */
		PC_NOTHING, 46,		/* c */
		PC_NOTHING, 32,		/* d */
		PC_NOTHING, 18,		/* e */
		PC_NOTHING, 33,		/* f */
		PC_NOTHING, 34,		/* g */
		PC_NOTHING, 35,		/* h */
		PC_NOTHING, 23,		/* i */
		PC_NOTHING, 36,		/* j */
		PC_NOTHING, 37,		/* k */
		PC_NOTHING, 38,		/* l */
		PC_NOTHING, 50,		/* m */
		PC_NOTHING, 49,		/* n */
		PC_NOTHING, 24,		/* o */
		PC_NOTHING, 25,		/* p */
		PC_NOTHING, 16,		/* q */
		PC_NOTHING, 19,		/* r */
		PC_NOTHING, 31,		/* s */
		PC_NOTHING, 20,		/* t */
		PC_NOTHING, 22,		/* u */
		PC_NOTHING, 47,		/* v */
		PC_NOTHING, 17,		/* w */
		PC_NOTHING, 45,		/* x */
		PC_NOTHING, 21,		/* y */
		PC_NOTHING, 44,		/* z */
		PC_SHIFT, 26,		/* { */
		PC_SHIFT, 43,		/* | */
		PC_SHIFT, 27,		/* } */
		PC_SHIFT, 41,		/* ~ */

		PC_NOTHING, 14,		/* DEL KEY */
		};


SendASCIIAsPCKeyCodes(c)
USHORT c;
{
	UBYTE flag, keycode;
	REGIST	SHORT	i;

	if (KeyQueuer)
		{
		if (c < 128)
			{
			c <<= 1;
			flag = ASCIIToPCTable[c];
			keycode = ASCIIToPCTable[c + 1];
			if (FlagIsSet(flag, PC_CONTROL)) 
				(*KeyQueuer)(PCCtrlCode);
			if (FlagIsSet(flag, PC_SHIFT)) 
				(*KeyQueuer)(PCLeftShiftCode);
			(*KeyQueuer)(keycode);
			(*KeyQueuer)(keycode | 0x80);
			if (FlagIsSet(flag, PC_SHIFT)) 
				(*KeyQueuer)(PCLeftShiftCode | 0x80);
			if (FlagIsSet(flag, PC_CONTROL)) 
				(*KeyQueuer)(PCCtrlCode | 0x80);
			}
		else
			{
			/* To send ASCII codes greater than or equal 
			 * to 128, we must send ALT-down, then the 
			 * down and up scancodes for the ascii of 
			 * a three-digit decimal number for the key, 
			 * followed by ALT up.
			 */
			(*KeyQueuer)(PCAltCode);

			i = 0;
			while (c >= 100) { i++;  c -= 100; }
			SendKeypadDigit(i);
			i = 0;
			while (c >= 10) { i++;  c -= 10; }
			SendKeypadDigit(i);
			SendKeypadDigit(c);

			(*KeyQueuer)(PCAltCode | 0x80);
			}
		}
}


SendKeypadDigit(digit)
REGIST	UBYTE	digit;
{
	switch (digit)
		{
		case 0:
			digit = PCKeypad0Code;
			break;
		case 1:
			digit = PCKeypad1Code;
			break;
		case 2:
			digit = PCKeypad2Code;
			break;
		case 3:
			digit = PCKeypad3Code;
			break;
		case 4:
			digit = PCKeypad4Code;
			break;
		case 5:
			digit = PCKeypad5Code;
			break;
		case 6:
			digit = PCKeypad6Code;
			break;
		case 7:
			digit = PCKeypad7Code;
			break;
		case 8:
			digit = PCKeypad8Code;
			break;
		case 9:
			digit = PCKeypad9Code;
			break;
		}
	(*KeyQueuer)(digit);
	(*KeyQueuer)(digit | 0x80);
}



/*???SendTextToPC(text)*/
/*???UBYTE *text;*/
/*???{*/
/*???	while (*text)*/
/*???		{*/
/*???		SendASCIIAsPCKeyCodes(*text);*/
/*???		text++;*/
/*???		}*/
/*???}*/


VOID	SendTextToPC(text)
REGIST	UBYTE	*text;
{
	REGIST	UBYTE	c;

	while (c = *text++) SendASCIIAsPCKeyCodes(AmigaToPCTable[c]);
}


