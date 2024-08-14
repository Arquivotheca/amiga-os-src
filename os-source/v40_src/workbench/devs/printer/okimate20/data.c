/*
	Data.c table for Okimate-20 driver.
	David Berezowski - March/88.
*/

char *CommandTable[]= {
			/* 00 aRIS reset			*/
       "\033W\376\022\033A\014\0332\033I\001\033%H\033-\376\033T\033C\376\011",
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\377",		/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/
	
			/* 05 aSGR0 normal char set		*/
	"\033%H\033-\376",
	"\033%G",	/* 06 aSGR3 italics on			*/
	"\033%H",	/* 07 aSGR23 italics off		*/
	"\033-\001",	/* 08 aSGR4 underline on		*/
	"\033-\376",	/* 09 aSGR24 underline off		*/
	"\377",		/* 10 aSGR1 boldface on			*/
	"\377",		/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/
	
	"\022\033W\376",/* 14 aSHORP0 normal pitch		*/
	"\033:",	/* 15 aSHORP2 elite on			*/
	"\022",		/* 16 aSHORP1 elite off			*/
	"\017",		/* 17 aSHORP4 condensed fine on		*/
	"\022",		/* 18 aSHORP3 condensed fine off	*/
	"\033W\001",	/* 19 aSHORP6 enlarge on		*/
	"\033W\376",	/* 20 aSHORP5 enlarge off		*/
	
	"\377",		/* 21 aDEN6 shadow print on		*/
	"\377",		/* 22 aDEN5 shadow print off		*/
	"\377",		/* 23 aDEN4 double strike on		*/
	"\377",		/* 24 aDEN3 double strike off		*/
	"\033I\002",	/* 25 aDEN2 NLQ on			*/
	"\033I\001",	/* 26 aDEN1 NLQ off			*/
	
	"\033S\376",	/* 27 aSUS2 superscript on		*/
	"\033T",	/* 28 aSUS1 superscript off		*/
	"\033S\001",	/* 29 aSUS4 subscript on		*/
	"\033T",	/* 30 aSUS3 subscript off		*/
	"\033T",	/* 31 aSUS0 normalize the line		*/
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
	
	"\0330",	/* 55 aVERP0 1/8" line spacing		*/
			/* 56 aVERP1 1/6" line spacing		*/
	"\033A\014\0332",
	"\033C",	/* 57 aSLPP set form length		*/
	"\033N\001",	/* 58 aPERF perf skip n (n > 0)		*/
	"\033O",	/* 59 aPERF0 perf skip off		*/
	
	"\377",		/* 60 aLMS set left margin		*/
	"\377",		/* 61 aRMS set right margin		*/
	"\377",		/* 62 aTMS set top margin		*/
	"\377",		/* 63 aBMS set bottom margin		*/
	"\377",		/* 64 aSTBM set T&B margins		*/
	"\377",		/* 65 aSLRM set L&R margins		*/
	"\377",		/* 66 aCAM clear margins		*/
	
	"\377",		/* 67 aHTS set horiz tab		*/
	"\377",		/* 68 aVTS set vert tab			*/
	"\377",		/* 69 aTBC0 clear horiz tab		*/
	"\033D\376",	/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\033D\376",	/* 72 aTBC4 clear all vert tabs		*/
	"\377",		/* 73 aTBCALL clear all h & v tabs	*/
			/* 74 aTBSALL set default tabs		*/
	"\033D\010\020\030\040\050\060\070\100\110\120\376",

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
*/
	" ",			/*    ok */
	"\255",			/* !  ok */
	"\0336\233\0337",	/* "  ok */
	"\0336\234\0337",	/* #  ok */
	"\033S\\000o\033T",	/* $  ok */
	"\0336\235\0337",	/* %  ok */
	"\174",			/* &  ok */
	"\0336\025\0337",	/* '  ok */

	"\"",			/* (  ok */
	"c",			/* )  ok */
	"\246",			/* *  ok */
	"\256",			/* +  ok */
	"\252",			/* ,  ok */
	"-",			/* -  ok */
	"r",			/* .  ok */
	"\304",			/* /  ok */

	"\370",			/* 0  ok */
	"\361",			/* 1  ok */
	"\033S\\0002\033T",	/* 2  ok */
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
