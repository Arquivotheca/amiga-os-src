/*** cbm_epson/data.c *********************************************
 *
 * data.c -- table data
 *
 *	$Id: data.c,v 1.5 91/07/08 17:30:06 darren Exp $
 *
 *	Copyright (c) 1988,1991 Commodore-Amiga, Inc.
 *	All Rights Reserved
 *
 **********************************************************************/
/*
	Data.c table for CBM_MPS-1000 driver.
	David Berezowski - May/88.
*/

char *CommandTable[] ={
			/*reset              RIS      ESCc */
	"\375\033@\375",
	"\377",        /*initialize*/
	"\377",        /* lf                IND      ESCD */
	"\015\012",    /* return,lf         NEL      ESCE */
	"\377",        /* reverse lf        RI       ESCM */

			/*normal char set    SGR 0    ESC[0m */
	"\033-\376\033F",
	"\377",        /*italics on         SGR 3    ESC[3m */
	"\377",        /*italics off        SGR 23   ESC[23m */
	"\033-\001",   /*underline on       SGR 4    ESC[4m */
	"\033-\376",   /*underline off      SGR 24   ESC[24m */
	"\033E",       /*boldface on        SGR 1    ESC[1m */
	"\033F",       /*boldface off       SGR 22   ESC[22m */
	"\377",        /* set foreground color */
	"\377",        /* set background color */

			/* normal char set   SHORP    ESC[0w */
	"\022\033W\376",
			/*elite on           SHORP    ESC[2w */
	"\033:",
	"\022",		/*elite off          SHORP    ESC[1w */
	"\017",		/*condensed(fine) on SHORP    ESC[4w */
	"\022",        /*condensed off      SHORP    ESC[3w */
	"\033W\001",   /*enlarged on        SHORP    ESC[6w */
	"\033W\376",   /*enlarged off       SHORP    ESC[5w */

	"\377",		/*shadow print on    DEN6     ESC[6"z */
	"\377",		/*shadow print off   DEN5     ESC[5"z */
	"\033G",       /*doublestrike on    DEN4     ESC[4"z */
	"\033H",       /*doublestrike off   DEN3     ESC[3"z */
	"\033x\001",   /* NLQ on            DEN2     ESC[2"z */
	"\033x\376",   /* NLQ off 	     DEN1     ESC[1"z */

	"\033S\376",   /*superscript on              ESC[2u */
	"\033T",       /*superscript off	      ESC[1u */
	"\033S\001",   /*subscript on   	      ESC[4u */
	"\033T",       /*subscript off  	      ESC[3u */
	"\033T",	/*normalize		      ESC[0u */
	"\377",        /* partial line up   PLU      ESCL */
	"\377",        /* partial line down PLD      ESCK */

	"\377",   /*US char set        FNT0     ESC(B */
	"\377",   /*French char set    FNT1     ESC(R */
	"\377",   /*German char set    FNT2     ESC(K */
	"\377",   /*UK char set        FNT3     ESC(A */
	"\377",   /*Danish I char set  FNT4     ESC E */
	"\377",   /*Sweden char set    FNT5     ESC(H */
	"\377",   /*Italian char set   FNT6     ESC(Y */
	"\377",   /*Spanish char set   FNT7     ESC(Z */
	"\377",   /*Japanese char set  FNT8     ESC(J */
	"\377",   /*Norweign char set  FNT9     ESC(6 */
	"\377",   /*Danish II char set FNT10    ESC(C */

	"\033P\001",   /*proportional on    PROP     ESC[2p */
	"\033P\376",   /*proportional off   PROP     ESC[1p */
	"\377",        /*proportional clear PROP     ESC[0p */
	"\377",        /*set prop offset    TSS */
	"\377",        /*auto left justify  JFY5     ESC[5 F */
	"\377",        /*auto right justify JFY7     ESC[7 F */
	"\033M\001",   /*auto full justify  JFY6     ESC[6 F */
	"\033M\376",	/*auto justify/center off   ESC[0 F */
	"\377",        /*place holder       JFY3     ESC[3 F */
	"\377",	/*auto center on     JFY2     ESC[2 F */

	"\0330",       /* 1/8" line space   VERP     ESC[0z */
	"\0332",       /* 1/6" line spacing VERP     ESC[1z */
	"\033C",       /* set form length   SLPP     ESC[Pnt */
	"\033N",       /* perf skip n 		      ESC[nq */
	"\033O",       /* perf skip off              ESC[0q */

	"\377",        /* Left margin set  	      ESC[2x */
	"\377",        /* Right margin set   	      ESC[3x */
	"\377",        /* top margin set 	      ESC[4x */
	"\377",        /* Bottom marg set 	      ESC[5x */
	"\377",        /* T&B margin set   STBM      ESC[Pn1;Pn2r */
	"\377",        /* L&R margin set   SLRM      ESC[Pn1;Pn2s */
	"\377",        /* Clear margins 	      ESC[0x */

	"\377",	/* Set horiz tab     HTS      ESCH */
	"\377",        /* Set vertical tab  VTS      ESCJ */
	"\377",	/* Clr horiz tab     TBC 0    ESC[0g */
	"\033D\376",	/* Clear all h tabs  TBC 3    ESC[3g */ 
	"\377",        /* Clr vertical tab  TBC 1    ESC[1g */
	"\033B\376",   /* Clr all v tabs    TBC 4    ESC[4g */
			/* Clr all h & v tabs  	      ESC#4 */
	"\033D\376\033B\376",
			/* set default tabs  	      ESC#5 */
	"\033D\010\020\030\040\050\060\070\100\110\120\130\376",
	"\377"		/* entended command */
};

char *ExtendedCharTable[] = {
/*
    " ", "!", "c", "L", "o", "Y", "|", "S",
    "\"", "c", "a", "`", "~", "-", "r", "-",
    "*", "+", "2", "3", "'", "u", "P", ".",
    ",", "1", "o", "'", "/", "/", "/", "?",
    "A", "A", "A", "A", "A", "A", "A", "C",
    "E", "E", "E", "E", "I", "I", "I", "I",
    "D", "N", "O", "O", "O", "O", "O", "x",
    "O", "U", "U", "U", "U", "Y", "T", "3",
    "a", "a", "a", "a", "a", "a", "a", "c",
    "e", "e", "e", "e", "i", "i", "i", "i",
    "d", "n", "o", "o", "o", "o", "o", "/",
    "o", "u", "u", "u", "u", "y", "t", "y"
};

*/
	" ",			/* NBSP */
	"\255",			/* i */
	"\0336\233\0337",	/* c| */
	"\0336\234\0337",	/* L- */
	"\033S\\000o\033T",	/* o */
	"\0336\235\0337",	/* Y- */
	"\174",			/* | */
	"\0336\025\0337",	/* SS */

	"\371",			/* " */
	"c",			/* copyright */
	"\246",			/* a_ */
	"\256",			/* << */
	"\252",			/* - */
	"-",			/* SHY */
	"r",			/* registered trademark */
	"\304",			/* - */

	"\370",			/* degrees */
	"\361",			/* +- */
	"\033S\\0002\033T", 	/* 2 */
	"\033S\\0003\033T", 	/* 3 */
	"'",			/* ' */
	"\346",			/* u */
	"P",			/* reverse P */
	"\371",			/* . */

	",",			/* , */
	"\033S\\0001\033T", 	/* 1 */
	"\247",			/* o_ */
	"\257",			/* >> */
	"\254",			/* 1/4 */
	"\253",			/* 1/2 */
	"/",			/* 3/4 */
	"\250",			/* upside down ? */

	"A",			/* `A */
	"A",			/* 'A */
	"A",			/* ^A */
	"A",			/* ~A */
	"\0336\216\0337",	/* "A */
	"\0336\217\0337",	/* oA */
	"\0336\222\0337",	/* AE */
	"\0336\200\0337",	/* C, */

	"E",			/* `E */
	"\0336\220\0337",	/* 'E */
	"E",			/* ^E */
	"E",			/* "E */
	"I",			/* `I */
	"I",			/* 'I */
	"I",			/* ^I */
	"I",			/* "I */

	"D",			/* -D */
	"\245",			/* ~N */
	"O",			/* `O */
	"O",			/* 'O */
	"O",			/* ^O */
	"O",			/* ~O */
	"\0336\231\0337",	/* "O */
	"x",			/* x */

	"\355",			/* O/ */
	"U",			/* `U */
	"U",			/* 'U */
	"U",			/* ^U */
	"\0336\232\0337",	/* "U */
	"Y",			/* 'Y */
	"T",			/* Thorn */
	"\341",			/* B */

	"\0336\205\0337",	/* `a */
	"\240",			/* 'a */
	"\0336\203\0337",	/* ^a */
	"a",			/* ~a */
	"\0336\204\0337",	/* "a */
	"\0336\206\0337",	/* oa */
	"\0336\221\0337",	/* ae */
	"\0336\207\0337",	/* c, */

	"\0336\212\0337",	/* `e */
	"\0336\202\0337",	/* 'e */
	"\0336\210\0337",	/* ^e */
	"\0336\211\0337",	/* "e */
	"\0336\215\0337",	/* `i */
	"\241",			/* 'i */
	"\0336\214\0337",	/* ^i */
	"\0336\213\0337",	/* "i */

	"d",			/* d */
	"\244",			/* ~n */
	"\0336\225\0337",	/* `o */
	"\242",			/* 'o */
	"\0336\223\0337",	/* ^o */
	"o",			/* ~o */
	"\0336\224\0337",	/* "o */
	"/",			/* :- */

	"\355",			/* o/ */
	"\0336\227\0337",	/* `u */
	"\243",			/* 'u */
	"\0336\226\0337",	/* ^u */
	"\0336\201\0337",	/* "u */
	"y",			/* `y */
	"t",			/* thorn */
	"\0336\230\0337"	/* "y */
};
