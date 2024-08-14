/* Okidata_292I */

/****** printer.device/printers/Okidata_292I_functions ********************
 *
 *   NAME
 *   Okidata_292I functions implemented: 
 *  
 *      aRIS, aIND, aNEL,
 *		aSGR0, aSGR1, aSGR3, aSGR4, aSGR22, aSGR23, aSGR24, 
 *      aSHORP0, aSHORP1, aSHORP2, aSHORP3, aSHORP4, aSHORP5, aSHORP6,
 *      aDEN1, aDEN2, aDEN3, aDEN4,
 *      aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,
 *      aFNT0, aFNT1, aFNT2, aFNT3, aFNT4, aFNT5, aFNT6, aFNT7, aFNT8
 *      aFNT9, aFNT10,
 *      aPROP1, aPROP2, aJFY6, aJFY0,
 *      aVERP0, aVERP1, aSLPP, aPERF, aPERF0,
 *      aTBC3, aTBC4, aTBCALL, aTBSALL
 *
 *      special functions implemented:
 *      aRIN, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4, aCAM
 *      aPLU, aPLD, aVERP0, aVERP1, aSLRM, aIND
 *
 ************************************************************************/

char *CommandTable[] ={
	"\375\033@\375",	/*reset              RIS      ESCc */
	"\377",			/*initialize*/
	"\377",			/* lf                IND      ESCD */
	"\015\012",    /* return,lf         NEL      ESCE */
	"\033\012",		/* reverse lf        RI       ESCM */

	/*normal char set    SGR 0    ESC[0m */
	"\033%H\033-\376\033F",
	"\033%G",        /*italics on         SGR 3    ESC[3m */
	"\033%H",        /*italics off        SGR 23   ESC[23m */
	"\033-\001",   /*underline on       SGR 4    ESC[4m */
	"\033-\376",   /*underline off      SGR 24   ESC[24m */
	"\033E",       /*boldface on        SGR 1    ESC[1m */
	"\033F",       /*boldface off       SGR 22   ESC[22m */
	"\377",        /* set foreground color */
	"\377",        /* set background color */

	
	/* normal pitch      SHORP    ESC[0w */
	"\022\033W\376",
	"\033:",       /*elite on           SHORP    ESC[2w */
	"\022",        /*elite off          SHORP    ESC[1w */
	"\017",        /*condensed(fine) on SHORP    ESC[4w */
	"\022",        /*condensed off      SHORP    ESC[3w */
	"\033W\001",   /*enlarged on        SHORP    ESC[6w */
	"\033W\376",   /*enlarged off       SHORP    ESC[5w */

	"\377",	/*shadow print on    DEN6     ESC[6"z */
	"\377",	/*shadow print off   DEN5     ESC[5"z */
	"\033G",       /*doublestrike on    DEN4     ESC[4"z */
	"\033H",       /*doublestrike off   DEN3     ESC[3"z */
	"\033I3",   /* NLQ on            DEN2     ESC[2"z */
	"\033I1",   /* NLQ off 	     DEN1     ESC[1"z */

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
                              
	"\033%P",   /*proportional on    PROP     ESC[2p */
	"\033%Q",   /*proportional off   PROP     ESC[1p */
	"\377",        /*proportional clear PROP     ESC[0p */
	"\377",        /*set prop offset    TSS */
	"\377",        /*auto left justify  JFY5     ESC[5 F */
	"\377",        /*auto right justify JFY7     ESC[7 F */
	"\033M\001",   /*auto full justify  JFY6     ESC[6 F */
	"\033M\376",	/*auto justify/center off   ESC[0 F */
	"\377",        /*place holder       JFY3     ESC[3 F */
	"\377",	/*auto center on     JFY2     ESC[2 F */

	"\0330",       /* 1/8" line space   VERP     ESC[0z */
	"\033A\014\0332", /* 1/6" line spacing VERP     ESC[1z */
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
	"\377",   /* Clr all v tabs    TBC 4    ESC[4g */
	"\033D\376",	/* Clr all h & v tabs  	      ESC#4 */
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
	" ",			/*    ok */
	"\255",			/* !  ok */
	"\0336\233\0337",	/* "  ok */
	"\0336\234\0337",	/* #  ok */
	"\033S\\000o\033T",	/* $  ok */
	"\0336\235\0337",	/* %  ok */
	"\174",			/* &  ok */
	"\0336\025\0337",	/* '  ok */

	"\371",			/* (  ok */
	"c",			/* )  ok */
	"\246",			/* *  ok */
	"\256",			/* +  ok */
	"\252",			/* ,  ok */
	"-",			/* -  ok */
	"r",			/* .  ok */
	"\304",			/* /  ok */

	"\370",			/* 0  ok */
	"\361",			/* 1  ok */
	"\033S\\0002\033T", 	/* 2  ok */
	"\033S\\0003\033T", 	/* 3  ok */
	"'",			/* 4  ok */
	"\346",			/* 5  ok */
	"P",			/* 6  ok */
	"\371",			/* 7  ok */

	",",			/* 8  ok */
	"\033S\\0001\033T", 	/* 9  ok */
	"\247",			/* :  ok */
	"\257",			/* ;  ok */
	"\254",			/* <  ok */
	"\253",			/* =  ok */
	"/",			/* >  ok */
	"\250",			/* ?  ok */

	"A",			/* @  ok */
	"A",			/* A  ok */
	"A",			/* B  ok */
	"A",			/* C  ok */
	"\0336\216\0337",	/* D  ok */
	"\0336\217\0337",	/* E  ok */
	"\0336\222\0337",	/* F  ok */
	"\0336\200\0337",	/* G  ok */

	"E",			/* H  ok */
	"\0336\220\0337",	/* I  ok */
	"E",			/* J  ok */
	"E",			/* K  ok */
	"I",			/* L  ok */
	"I",			/* M  ok */
	"I",			/* N  ok */
	"I",			/* O  ok */

	"D",			/* P  ok */
	"\245",			/* Q  ok */
	"O",			/* R  ok */
	"O",			/* S  ok */
	"O",			/* T  ok */
	"O",			/* U  ok */
	"\0336\231\0337",	/* V  ok */
	"x",			/* W  ok */

	"\355",			/* X  ok */
	"U",			/* Y  ok */
	"U",			/* Z  ok */
	"U",			/* [  ok */
	"\0336\232\0337",	/* \  ok */
	"Y",			/* ]  ok */
	"T",			/* ^  ok */
	"\341",			/* _  ok */

	"\0336\205\0337",	/* `  ok */
	"\240",			/* a  ok */
	"\0336\203\0337",	/* b  ok */
	"a",			/* c  ok */
	"\0336\204\0337",	/* d  ok */
	"\0336\206\0337",	/* e  ok */
	"\0336\221\0337",	/* f  ok */
	"\0336\207\0337",	/* g  ok */

	"\0336\212\0337",	/* h  ok */
	"\0336\202\0337",	/* i  ok */
	"\0336\210\0337",	/* j  ok */
	"\0336\211\0337",	/* k  ok */
	"\0336\215\0337",	/* l  ok */
	"\241",			/* m  ok */
	"\0336\214\0337",	/* n  ok */
	"\0336\213\0337",	/* o  ok */

	"d",			/* p  ok */
	"\244",			/* q  ok */
	"\0336\225\0337",	/* r  ok */
	"\242",			/* s  ok */
	"\0336\223\0337",	/* t  ok */
	"o",			/* u  ok */
	"\0336\224\0337",	/* v  ok */
	"/",			/* w  ok */

	"o",			/* x  ok */
	"\0336\227\0337",	/* y  ok */
	"\243",			/* z  ok */
	"\0336\226\0337",	/* {  ok */
	"\0336\201\0337",	/* |  ok */
	"y",			/* }  ok */
	"t",			/* ~  ok */
	"\0336\230\0337"	/*    ok */
};

