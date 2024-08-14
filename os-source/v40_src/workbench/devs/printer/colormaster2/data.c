/*
	Data.c table for CalComp_ColorMaster driver.
	David Berezowski - March/88.
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

			/* 14 aSHORP0 normal pitch		*/
	"\033P\022\033W\376",
			/* 15 aSHORP2 elite on			*/
	"\033M\022\033W\376",
	"\033P",	/* 16 aSHORP1 elite off			*/
			/* 17 aSHORP4 condensed fine on		*/
	"\017\033P\033W\376",
	"\022",		/* 18 aSHORP3 condensed fine off	*/
	"\033W\001",	/* 19 aSHORP6 enlarge on		*/
	"\033W\376",	/* 20 aSHORP5 enlarge off		*/

	"\377",		/* 21 aDEN6 shadow print on		*/
	"\377",		/* 22 aDEN5 shadow print off		*/
	"\033G",	/* 23 aDEN4 double strike on		*/
	"\033H",	/* 24 aDEN3 double strike off		*/
	"\377",		/* 25 aDEN2 NLQ on			*/
	"\377",		/* 26 aDEN1 NLQ off			*/

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

	"\033p1",	/* 45 aPROP2 proportional on		*/
	"\033p0",	/* 46 aPROP1 proportional off		*/
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
	"\377",		/* 58 aPERF perf skip n (n > 0)		*/
	"\377",		/* 59 aPERF0 perf skip off		*/

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
	"\033D\376",	/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\033D\376",	/* 73 aTBCALL clear all h & v tabs	*/
			/* 74 aTBSALL set default tabs		*/
	"\033D\010\020\030\040\050\060\070\100\110\120\130\376",

	"\377",		/* 75 aEXTEND extended commands		*/
	"\377"		/* 76 aRAW next 'n' chars are raw	*/
};

char *ExtendedCharTable[] = {
" ",						/* $A0 */
"\\27R\\7!\\27R\\0",
"c\\8/",
"\\27R\\3#\\27R\\0",
"\\27R\\5$\\27R\\0",
"\\27R\\8\\\\\\27R\\0",
"|",
"\\27R\\2@\\27R\\0",
"\\27R\\7{\\27R\\0",
"c", 
"\\27S\\000a\\8_\\27T",
"`",
"~",
"-",
"r",
"-",						/* $AF */

"\\27R\\1[\\27R\\0",		/* $B0 */
"+\\8_",
"\\27S\\0002\\27T",
"\\27S\\0003\\27T",
"'",
"u",
"P",
"\\27S\\000.\\27T",
",",
"\\27S\\0001\\27T",
"\\27R\\1[\\27R\\0\\8-",
"'",
"\\27S\\0001\\27T\\8-\\8\\27S\\0014\\27T",
"\\27S\\0001\\27T\\8-\\8\\27S\\0012\\27T",
"\\27S\\0003\\27T\\8-\\8\\27S\\0014\\27T",
"\\27R\\7]\\27R\\0",		/* $BF */

"A",						/* $C0 */
"A",
"A",
"A",
"\\27R\\2[\\27R\\0",
"\\27R\\4]\\27R\\0",
"\\27R\\4[\\27R\\0",
"C",
"E",
"\\27R\\9@\\27R\\0",
"E",
"E",
"I",
"I",
"I",
"I",						/* $CF */

"D\\8-",					/* $D0 */
"\\27R\\7\\\\\\27R\\0",
"O",
"O",
"O",
"O",
"\\27R\\2\\\\\\27R\\0",
"x",
"\\27R\\4\\\\\\27R\\0",
"U",
"U",
"U",
"\\27R\\2]\\27R\\0",
"Y",
"T",
"\\27R\\2~\\27R\\0",		/* $DF */

"\\27R\\1@\\27R\\0",		/* $E0 */
"\\27R\\4}\\27R\\0",
"a\\8`",
"a\\8'",
"\\27R\\2{\\27R\\0",
"a\\8\\27R\\1[\\27R\\0",
"\\27R\\4{\\27R\\0",
"c",
"\\27R\\1}\\27R\\0",
"\\27R\\6]\\27R\\0",
"e\\8\\27S\\000^\\27T",
"e",
"\\27R\\6~\\27R\\0",
"i\\8'",
"i\\8\\27S\\000^\\27T",
"i\\8\\27R\\7{\\27R\\0",	/* $EF */

"d",						/* $F0 */
"\\27R\\7|\\27R\\0",
"\\27R\\6|\\27R\\0",
"o\\8'",
"o\\8\\27S\\000^\\27T",
"o\\8\\27R\\7{\\27R\\0",
"\\27R\\2|\\27R\\0",
"/",
"\\27R\\4|\\27R\\0",
"\\27R\\6\\96\\27R\\0",
"u\\8'",
"u\\8\\27S\\000^\\27T",
"\\27R\\2}\\27R\\0",
"y\\8'",
"t",
"y\\8\\27R\\7{\\27R\\0"		/* $FF */
};
