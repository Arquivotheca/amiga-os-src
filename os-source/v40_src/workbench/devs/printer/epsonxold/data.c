/*
	Data.c table for EpsonXOld (copy of EpsonX) driver.
	David Berezowski - March/88.

	Modified to remove all special control codes
	by Bryce Nesbitt - Thu Jun  2 18:07:53 EDT 1988
	This is a generic, minimal Epson compatible driver

	(Copy of EpsonX driver except for ExtendedCharTable & other mods).
*/

char *CommandTable[] ={
	"\375\033@\375",/* 00 aRIS reset			*/
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\015\012",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/

			/* 05 aSGR0 normal char set		*/
	"\0335\033-\376\033F",
	"\0334",	/* 06 aSGR3 italics on			*/
	"\0335",	/* 07 aSGR23 italics off		*/
	"\033-\001",	/* 08 aSGR4 underline on		*/
	"\033-\376",	/* 09 aSGR24 underline off		*/
	"\033E",	/* 10 aSGR1 boldface on			*/
	"\033F",	/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/

			/* 14 aSHORP0 normal pitch (10 cpi)	*/
	"\022\033W\376\033T",
	"\017",		/* 15 aSHORP2 elite on	(12 cpi)	*/
	"\022",		/* 16 aSHORP1 elite off			*/
	"\017",		/* 17 aSHORP4 condensed fine on (15,16.67,17.1 cpi) */
	"\022",		/* 18 aSHORP3 condensed fine off	*/
	"\033W\001",	/* 19 aSHORP6 enlarge on		*/
	"\033W\376",	/* 20 aSHORP5 enlarge off		*/

	"\033E",	/* 21 aDEN6 shadow print on		*/
	"\033F",	/* 22 aDEN5 shadow print off		*/
	"\033G",	/* 23 aDEN4 double strike on		*/
	"\033H",	/* 24 aDEN3 double strike off		*/
	"\033E",	/* 25 aDEN2 NLQ on			*/
	"\033F",	/* 26 aDEN1 NLQ off			*/

	"\033S\376",	/* 27 aSUS2 superscript on		*/
	"\033T",	/* 28 aSUS1 superscript off		*/
	"\033S\001",	/* 29 aSUS4 subscript on		*/
	"\033T",	/* 30 aSUS3 subscript off		*/
	"\033T",	/* 31 aSUS0 normalize the line		*/
	"\377",		/* 32 aPLU partial line up		*/
	"\377",		/* 33 aPLD partial line down		*/

	/* Warning - the font commands don't work on all printers.
	   don't send them unless you need them.  For example, 
	   the Gemini uses this code to set the location of the
	   first line on the page. */
        "\033R\376",    /* 34 aFNT0 Typeface 0                  */
        "\033R\001",    /* 35 aFNT1 Typeface 1                  */
        "\033R\002",    /* 36 aFNT2 Typeface 2                  */
        "\033R\003",    /* 37 aFNT3 Typeface 3                  */
        "\033R\004",    /* 38 aFNT4 Typeface 4                  */
        "\033R\005",    /* 39 aFNT5 Typeface 5                  */
        "\033R\006",    /* 40 aFNT6 Typeface 6                  */
        "\033R\007",    /* 41 aFNT7 Typeface 7                  */
        "\033R\010",    /* 42 aFNT8 Typeface 8                  */
        "\033R\011",    /* 43 aFNT9 Typeface 9                  */
        "\033R\012",    /* 44 aFNT10 Typeface 10                */

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
	"\0332",	/* 56 aVERP1 1/6" line spacing		*/
	"\033C",	/* 57 aSLPP set form length		*/
	"\033N",	/* 58 aPERF perf skip n (n > 0)		*/
	"\033O",	/* 59 aPERF0 perf skip off		*/

	"\377",		/* 60 aLMS set left margin		*/
	"\377",		/* 61 aRMS set right margin		*/
	"\377",		/* 62 aTMS set top margin 		*/
	"\377",		/* 63 aBMS set bottom margin		*/
	"\377",		/* 64 aSTBM set T&B margins		*/
	"\377",		/* 65 aSLRM set L&R margins		*/
	"\377",		/* 66 aCAM clear margins		*/

	"\377",		/* 67 aHTS set horiz tab		*/
	"\377",		/* 68 aVTS set vert tab			*/
	"\377",		/* 69 aTBC0 clear horiz tab		*/
	"\377",		/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\377",		/* 73 aTBCALL clear all h & v tabs	*/
			/* 74 aTBSALL set default tabs		*/
	"\033D\010\020\030\040\050\060\070\100\110\120\130\376",
	"\377",		/* 75 aEXTEND extended commands		*/
	"\377"		/* 76 aRAW next 'n' chars are raw	*/
};

char *ExtendedCharTable[] = { /* uses backspace to make best chars */
" ", "!", "c\010|", "L\010-", "o", "Y\010-", "|", "S",
"\"", "c", "a", "`", "~", "-", "r", "-",
"*", "+\010_", "2", "3", "'", "u", "P", ".",
",", "1", "o", "'", "/", "/", "/", "?",
"A\010`", "A\010'", "A\010^", "A\010~", "A\010\042", "A", "A", "C\010,",
"E\010`", "E\010'", "E\010^", "E\010\042", "I\010`", "I\010'", "I\010^", "I\010\042",
"D", "N\010~", "O\010`", "O\010'", "O\010^", "O\010~", "O\010\042", "x",
"O\010/", "U\010`", "U\010'", "U\010^", "U\010\042", "Y\010'", "T", "3",
"a\010`", "a\010'", "a\010^", "a\010~", "a\010\042", "a", "a", "c\010,",
"e\010`", "e\010'", "e\010^", "e\010\042", "i\010`", "i\010'", "i\010^", "i\010\042",
"d", "n\010~", "o\010`", "o\010'", "o\010^", "o\010~", "o\010\042", "-\010:",
"o\010/", "u\010`", "u\010'", "u\010^", "u\010\042", "y\010'", "t", "y\010\042"
};

