/*
	Data.c table for Xerox_4020 driver.
	David Berezowski - March/88.
*/

char *CommandTable[]={
			/* 00 aRIS reset			*/
	"\375\033\015P\375",
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\015\012",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/
	
	"\033R",	/* 05 aSGR0 normal char set		*/
	"\377",		/* 06 aSGR3 italics on			*/
	"\377",		/* 07 aSGR23 italics off		*/
	"\033E",	/* 08 aSGR4 underline on		*/
	"\033R",	/* 09 aSGR24 underline off		*/
	"\377",		/* 10 aSGR1 boldface on			*/
	"\377",		/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/
	
	"\033F0\033&",	/* 14 aSHORP0 normal pitch		*/
	"\033F2",	/* 15 aSHORP2 elite on			*/
	"\033F0",	/* 16 aSHORP1 elite off			*/
	"\033F4",	/* 17 aSHORP4 condensed fine on		*/
	"\033F0",	/* 18 aSHORP3 condensed fine off	*/
	"\033W20",	/* 19 aSHORP6 enlarge on		*/
	"\033&",	/* 20 aSHORP5 enlarge off		*/
	
	"\377",		/* 21 aDEN6 shadow print on		*/
	"\377",		/* 22 aDEN5 shadow print off		*/
	"\377",		/* 23 aDEN4 double strike on		*/
	"\377",		/* 24 aDEN3 double strike off		*/
	"\033wa",	/* 25 aDEN2 NLQ on			*/
	"\033wb",	/* 26 aDEN1 NLQ off			*/
	
	"\033t",	/* 27 aSUS2 superscript on		*/
	"\033s",	/* 28 aSUS1 superscript off		*/
	"\033u",	/* 29 aSUS4 subscript on		*/
	"\033s",	/* 30 aSUS3 subscript off		*/
	"\033s",	/* 31 aSUS0 normalize the line		*/
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
	" ",			/* NBSP			ok */
	"\241",			/* ! (upside down)	ok */
	"\242",			/* c|			ok */
	"\243",			/* L			ok */
	"\033w\005$\033w\001",	/* o			ok */
	"\245",			/* Y			ok */
	"\326",			/* |			ok */
	"\247",			/* SS			ok */

	"\310 ",		/* "			ok */
	"c",			/* c (copyright)	ok */
	"\333",			/* a_			ok */
	"\253",			/* <<			ok */
	"\305 ",		/* -			ok */
	"-",			/* SHY			ok */
	"r",			/* r (registered)	ok */
	"\305 ",		/* -			ok */

	"\312 ",		/* o (degrees)		ok */
	"\261",			/* +_			ok */
	"2", 			/* 2			ok */
	"3", 			/* 3			ok */
	"\302 ",		/* '			ok */
	"\265",			/* u			ok */
	"P",			/* P (reversed)		ok */
	"\345",			/* .			ok */

	",",			/* ,			ok */
	"1", 			/* 1			ok */
	"\353",			/* o_			ok */
	"\273",			/* >>			ok */
	"/",			/* 1/4			ok */
	"/",			/* 1/2			ok */
	"/",			/* 3/4			ok */
	"\277",			/* ? (upside down)	ok */

	"\301A",		/* A`			ok */
	"\302A",		/* A'			ok */
	"\303A",		/* A^			ok */
	"\304A",		/* A~			ok */
	"\310A",		/* A"			ok */
	"\312A",		/* Ao			ok */
	"\341",			/* AE			ok */
	"\313C",		/* C,			ok */

	"\301E",		/* E`			ok */
	"\302E",		/* E'			ok */
	"\303E",		/* E^			ok */
	"\310E",		/* E"			ok */
	"\301I",		/* I`			ok */
	"\302I",		/* I'			ok */
	"\303I",		/* I^			ok */
	"\310I",		/* I"			ok */

	"-\010D",		/* D-			ok */
	"\304N",		/* N-			ok */
	"\301O",		/* O`			ok */
	"\302O",		/* O'			ok */
	"\303O",		/* O^			ok */
	"\304O",		/* O~			ok */
	"\310O",		/* O"			ok */
	"x",			/* x (times)		ok */

	"\351",			/* 0			ok */
	"\301U",		/* U`			ok */
	"\302U",		/* U'			ok */
	"\303U",		/* U^			ok */
	"\310U",		/* U"			ok */
	"\302Y",		/* Y'			ok */
	"T",			/* IP			ok */
	"\373",			/* B			ok */

	"\301a",		/* a`			ok */
	"\302a",		/* a'			ok */
	"\303a",		/* a^			ok */
	"\304a",		/* a~			ok */
	"\310a",		/* a"			ok */
	"\312a",		/* ao			ok */
	"\361",			/* ae			ok */
	"\313c",		/* c,			ok */

	"\301e",		/* e`			ok */
	"\302e",		/* e'			ok */
	"\303e",		/* e^			ok */
	"\310e",		/* e"			ok */
	"\301i",		/* i`			ok */
	"\302i",		/* i'			ok */
	"\303i",		/* i^			ok */
	"\310i",		/* i"			ok */

	"d",			/* d			ok */
	"\304n",		/* n~			ok */
	"\301o",		/* o`			ok */
	"\302o",		/* o'			ok */
	"\303o",		/* o^			ok */
	"\304o",		/* o~			ok */
	"\310o",		/* o"			ok */
	"\270",			/* -: (divide)		ok */

	"\371",			/* 0			ok */
	"\301u",		/* u`			ok */
	"\302u",		/* u'			ok */
	"\303u",		/* u^			ok */
	"\310u",		/* u"			ok */
	"\302y",		/* y'			ok */
	"t",			/* Ip			ok */
	"\310y"			/* y"			ok */
};
