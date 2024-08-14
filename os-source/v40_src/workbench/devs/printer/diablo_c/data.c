/*
	Data.c table for Diablo_C-150 driver.
	David Berezowski - June/88.
*/

char *CommandTable[]={
			/* 00 aRIS reset			*/
	"\375\033\015P\375",
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\015\012",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/
	
	"\377",		/* 05 aSGR0 normal char set		*/
	"\377",		/* 06 aSGR3 italics on			*/
	"\377",		/* 07 aSGR23 italics off		*/
	"\377",		/* 08 aSGR4 underline on		*/
	"\377",		/* 09 aSGR24 underline off		*/
	"\377",		/* 10 aSGR1 boldface on			*/
	"\377",		/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/
	
	"\377",		/* 14 aSHORP0 normal pitch		*/
	"\377",		/* 15 aSHORP2 elite on			*/
	"\377",		/* 16 aSHORP1 elite off			*/
	"\377",		/* 17 aSHORP4 condensed fine on		*/
	"\377",		/* 18 aSHORP3 condensed fine off	*/
	"\377",		/* 19 aSHORP6 enlarge on		*/
	"\377",		/* 20 aSHORP5 enlarge off		*/
	
	"\377",		/* 21 aDEN6 shadow print on		*/
	"\377",		/* 22 aDEN5 shadow print off		*/
	"\377",		/* 23 aDEN4 double strike on		*/
	"\377",		/* 24 aDEN3 double strike off		*/
	"\377",		/* 25 aDEN2 NLQ on			*/
	"\377",		/* 26 aDEN1 NLQ off			*/
	
	"\377",		/* 27 aSUS2 superscript on		*/
	"\377",		/* 28 aSUS1 superscript off		*/
	"\377",		/* 29 aSUS4 subscript on		*/
	"\377",		/* 30 aSUS3 subscript off		*/
	"\377",		/* 31 aSUS0 normalize the line		*/
	"\377",		/* 32 aPLU partial line up		*/
	"\377",		/* 33 aPLD partial line down		*/
	
	"\377",		/* 34 aFNT0 Typeface 0			*/
	"\377",		/* 35 aFNT1 Typeface 1			*/
	"\377",		/* 36 aFNT2 Typeface 2			*/
	"\377",		/* 37 aFNT3 Typeface 3			*/
	"\377",		/* 38 aFNT4 Typeface 4			*/
	"\377",		/* 39 aFNT5 Typeface 5			*/
	"\377",		/* 40 aFNT6 Typeface 6			*/
	"\377",		/* 41 aFNT7 Typeface 7			*/
	"\377",		/* 42 aFNT8 Typeface 8			*/
	"\377",		/* 43 aFNT9 Typeface 9			*/
	"\377",		/* 44 aFNT10 Typeface 10		*/
	
	"\377",		/* 45 aPROP2 proportional on		*/
	"\377",		/* 46 aPROP1 proportional off		*/
	"\377",		/* 47 aPROP0 proportional clear		*/
	"\377",		/* 48 aTSS set proportional offset	*/
	"\377",		/* 49 aJFY5 auto left justify		*/
	"\377",		/* 50 aJFY7 auto right justify		*/
	"\377",		/* 51 aJFY6 auto full jusitfy		*/
	"\377",		/* 52 aJFY0 auto jusity off		*/
	"\377",		/* 53 aJFY3 letter space		*/
	"\377",		/* 54 aJFY1 word fill			*/
	
	"\377",		/* 55 aVERP0 1/8" line spacing		*/
	"\377",		/* 56 aVERP1 1/6" line spacing		*/
	"\033\014",	/* 57 aSLPP set form length		*/
	"\377",		/* 58 aPERF perf skip n (n > 0)		*/
	"\377",		/* 59 aPERF0 perf skip off		*/
	
	"\0339",	/* 60 aLMS set left margin		*/
	"\0330",	/* 61 aRMS set right margin		*/
	"\377",		/* 62 aTMS set top margin		*/
	"\377",		/* 63 aBMS set bottom margin		*/
	"\377",		/* 64 aSTBM set T&B margins		*/
	"\377",		/* 65 aSLRM set L&R margins		*/
	"\377",		/* 66 aCAM clear margins		*/
	
	"\0331",	/* 67 aHTS set horiz tab		*/
	"\377",		/* 68 aVTS set vert tab			*/
	"\0338",	/* 69 aTBC0 clear horiz tab		*/
	"\0332",	/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\0332",	/* 73 aTBCALL clear all h & v tabs	*/
			/* 74 aTBSALL set default tabs		*/
	"\033i9,17,25,33,41,49,57,65,73,81,89,97,105,113,121,129",

	"\377",		/* 75 aEXTEND extended commands		*/
	"\377"		/* 76 aRAW next 'n' chars are raw	*/
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
	"!",			/* !  ok */
	"\174\010c",		/* "  ok */
	"\323",			/* #  ok */
	"\324",			/* $  ok */
	"-\010Y",		/* %  ok */
	"|",			/* &  ok */
	"\335\010S",		/* '  ok */

	"\310 ",		/* (  ok */
	"c",			/* )  ok */
	"\314a",		/* *  ok */
	"`",			/* +  ok */
	"\305 ",		/* ,  ok */
	"-",			/* -  ok */
	"r",			/* .  ok */
	"\305 ",		/* /  ok */

	"\312 ",		/* 0  ok */
	"\314+",		/* 1  ok */
	"2", 			/* 2  ok */
	"3", 			/* 3  ok */
	"\302 ",		/* 4  ok */
	"\330",			/* 5  ok */
	"P",			/* 6  ok */
	"\335",			/* 7  ok */

	",",			/* 8  ok */
	"1", 			/* 9  ok */
	"\314o",		/* :  ok */
	"'",			/* ;  ok */
	"/",			/* <  ok */
	"/",			/* =  ok */
	"/",			/* >  ok */
	"\334",			/* ?  ok */

	"\301A",		/* @  ok */
	"\302A",		/* A  ok */
	"\303A",		/* B  ok */
	"\304A",		/* C  ok */
	"\310A",		/* D  ok */
	"\312A",		/* E  ok */
	"\322",			/* F  ok */
	"\313C",		/* G  ok */

	"\301E",		/* H  ok */
	"\302E",		/* I  ok */
	"\303E",		/* J  ok */
	"\310E",		/* K  ok */
	"\301I",		/* L  ok */
	"\302I",		/* M  ok */
	"\303I",		/* N  ok */
	"\310I",		/* O  ok */

	"-\010D",		/* P  ok */
	"\304N",		/* Q  ok */
	"\301O",		/* R  ok */
	"\302O",		/* S  ok */
	"\303O",		/* T  ok */
	"\304O",		/* U  ok */
	"\310O",		/* V  ok */
	"x",			/* W  ok */

	"0",			/* X  ok */
	"\301U",		/* Y  ok */
	"\302U",		/* Z  ok */
	"\303U",		/* [  ok */
	"\310U",		/* \  ok */
	"\302Y",		/* ]  ok */
	"T",			/* ^  ok */
	"\333",			/* _  ok */

	"\301a",		/* `  ok */
	"\302a",		/* a  ok */
	"\303a",		/* b  ok */
	"\304a",		/* c  ok */
	"\310a",		/* d  ok */
	"\312a",		/* e  ok */
	"\321",			/* f  ok */
	"\313c",		/* g  ok */

	"\301e",		/* h  ok */
	"\302e",		/* i  ok */
	"\303e",		/* j  ok */
	"\310e",		/* k  ok */
	"\301i",		/* l  ok */
	"\302i",		/* m  ok */
	"\303i",		/* n  ok */
	"\310i",		/* o  ok */

	"d",			/* p  ok */
	"\304n",		/* q  ok */
	"\301o",		/* r  ok */
	"\302o",		/* s  ok */
	"\303o",		/* t  ok */
	"\304o",		/* u  ok */
	"\310o",		/* v  ok */
	"/",			/* w  ok */

	"\311o",		/* x  ok */
	"\301u",		/* y  ok */
	"\302u",		/* z  ok */
	"\303u",		/* {  ok */
	"\310u",		/* |  ok */
	"\302y",		/* }  ok */
	"t",			/* ~  ok */
	"\310y"			/*    ok */
};
