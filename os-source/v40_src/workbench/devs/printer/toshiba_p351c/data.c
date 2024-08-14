/*
	Data.c table for Toshiba_P351C driver.
	David Berezowski - March/88.
*/

char *CommandTable[] ={
	"\033\032I",	/* 00 aRIS reset			*/
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\012\015",	/* 03 aNEL CRLF				*/
	"\033\012",	/* 04 aRI reverse LF			*/

			/* 05 aSGR0 normal char set		*/
	"\033\024\033J\033M",
	"\033\022",	/* 06 aSGR3 italics on			*/
	"\033\024",	/* 07 aSGR23 italics off		*/
	"\033I",	/* 08 aSGR4 underline on		*/
	"\033J",	/* 09 aSGR24 underline off		*/
	"\033K2",	/* 10 aSGR1 boldface on			*/
	"\033M",	/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/

			/* 14 aSHORP0 normal pitch		*/
	"\033E12\033]\033\042",
			/* 15 aSHORP2 elite on			*/
	"\033E10\033]\033\042",
	"\033E12",	/* 16 aSHORP1 elite off			*/
	"\033[\033E12",	/* 17 aSHORP4 condensed fine on		*/
	"\033]",	/* 18 aSHORP3 condensed fine off	*/
	"\033!",	/* 19 aSHORP6 enlarge on		*/
	"\033\042",	/* 20 aSHORP5 enlarge off		*/

	"\033Q",	/* 21 aDEN6 shadow print on		*/
	"\033R",	/* 22 aDEN5 shadow print off		*/
	"\033K2",	/* 23 aDEN4 double strike on		*/
	"\033M",	/* 24 aDEN3 double strike off		*/
	"\033*2",	/* 25 aDEN2 NLQ on			*/
	"\033*0",	/* 26 aDEN1 NLQ off			*/

	"\377",		/* 27 aSUS2 superscript on		*/
	"\377",		/* 28 aSUS1 superscript off		*/
	"\377",		/* 29 aSUS4 subscript on		*/
	"\377",		/* 30 aSUS3 subscript off		*/
	"\377",		/* 31 aSUS0 normalize the line		*/
	"\033D",	/* 32 aPLU partial line up		*/
	"\033U",	/* 33 aPLD partial line down		*/

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

	"\033$",	/* 45 aPROP2 proportional on		*/
	"\033%",	/* 46 aPROP1 proportional off		*/
	"\377",		/* 47 aPROP0 proportional clear		*/
	"\377",		/* 48 aTSS set proportional offset	*/
	"\377",		/* 49 aJFY5 auto left justify		*/
	"\377",		/* 50 aJFY7 auto right justify		*/
	"\377",		/* 51 aJFY6 auto full jusitfy		*/
	"\377",		/* 52 aJFY0 auto jusity off		*/
	"\377",		/* 53 aJFY3 letter space		*/
	"\377",		/* 54 aJFY1 word fill			*/

	"\033\036\007",	/* 55 aVERP0 1/8" line spacing		*/
	"\033\036\011",	/* 56 aVERP1 1/6" line spacing		*/
	"\377",		/* 57 aSLPP set form length		*/
	"\377",		/* 58 aPERF perf skip n (n > 0)		*/
	"\377",		/* 59 aPERF0 perf skip off		*/

	"\0339",	/* 60 aLMS set left margin		*/
	"\0330",	/* 61 aRMS set right margin		*/
	"\033+",	/* 62 aTMS set top margin		*/
	"\033-",	/* 63 aBMS set bottom margin		*/
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
	"\033(08,16,24,32,40,48,56,64,72,80,88,96,A4,B2,C0,C8,D6,E4,F2.",

	"\377",		/* 75 aEXTEND extended commands		*/
	"\377",		/* 76 aRAW next 'n' chars are raw	*/
};

/*
	" ", "!", "c", "L",
	"o", "Y", "|", "S",
	"\"", "c", "a", "<",
	"~", "-", "r", "-",
	"*", "+", "2", "3",
	"'", "u", "P", ".",
	",", "1", "o", ">",
	"/", "/", "/", "?",
	"A", "A", "A", "A",
	"A", "A", "A", "C",
	"E", "E", "E", "E",
	"I", "I", "I", "I",
	"D", "N", "O", "O",
	"O", "O", "O", "x",
	"O", "U", "U", "U",
	"U", "Y", "P", "B",
	"a", "a", "a", "a",
	"a", "a", "a", "c",
	"e", "e", "e", "e",
	"i", "i", "i", "i",
	"d", "n", "o", "o",
	"o", "o", "o", "/",
	"o", "u", "u", "u",
	"u", "y", "p", "y"
*/
char *ExtendedCharTable[] = {
	" ", "i", "\264", "\243",
	"o", "\260", "|", "\251",
	"\276", "\253", "a", "<",
	"~", "-", "\252", "-",
	"\246", "+\010_", "2", "3",
	"\240", "\245", "\257", ".",
	",", "1", "\246\010-", ">",
	"\254", "\256", "\255", "?",
	"A\010\244", "A\010\240", "A\010^", "A\010~",
	"\261", "A\010\246", "A", "C\010,",
	"E\010\244", "E\010\240", "E\010^", "E\010\042",
	"I\010\244", "I\010\240", "I\010^", "I\010\042",
	"D", "N\010~", "O\010\244", "O\010\240",
	"O\010^", "O\010~", "\262", "x",
	"O\010/", "U\010\244", "U\010\240", "U\010^",
	"\263", "Y\010\240", "T", "\271",
	"\241", "a\010\240", "a\010^", "a\010~",
	"\266", "a\010\246", "a", "\242",
	"\275", "\273", "e\010^", "e\010\042",
	"i\010\244", "i\010\240", "i\010^", "i\010\042",
	"d", "n\010~", "o\010\244", "o\010\240",
	"o\010^", "o\010~", "\267", "-\010:",
	"o\010/", "\274", "u\010\240", "u\010^",
	"\270", "y\010\240", "t", "y\010\042"
};
