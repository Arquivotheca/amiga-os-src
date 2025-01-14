/*
	Data.c table for Tektronix_4696 driver.
	David Berezowski - May/88.
*/

char *CommandTable[]={
			/* 00 aRIS reset			*/
	"\375\033c\375",
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\015\012",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/
	
		
	"\021",		/* 05 aSGR0 normal char set		*/
	"\377",		/* 06 aSGR3 italics on			*/
	"\377",		/* 07 aSGR23 italics off		*/
	"\022",		/* 08 aSGR4 underline on		*/
	"\021",		/* 09 aSGR24 underline off		*/
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
	"\033L",	/* 57 aSLPP set form length		*/
	"\377",		/* 58 aPERF perf skip n (n > 0)		*/
	"\377",		/* 59 aPERF0 perf skip off		*/
	
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
	"\0332",	/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\0332",	/* 73 aTBCALL clear all h & v tabs	*/
	"\377",		/* 74 aTBSALL set default tabs		*/

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

	" ",		/* $a0 */
	"\323",		/* $a1 */
	"\301",		/* $a2 */
	"\375",		/* $a3 */
	"\275",		/* $a4 */
	"\216\040",	/* $a5 */
	"\302",		/* $a6 */
	"\276",		/* $a7 */
	"\277",		/* $a8 */
	"\216\041",	/* $a9 */
	"\216\042",	/* $aa */
	"\216\043",	/* $ab */
	"\316",		/* $ac */
	"-",		/* $ad */
	"\216\045",	/* $ae */
	"\300",		/* $af */

	"\346",		/* $b0 */
	"\347",		/* $b1 */
	"\262",		/* $b2 */
	"\263",		/* $b3 */
	"\216\047",	/* $b4 */
	"\330",		/* $b5 */
	"\216\050",	/* $b6 */
	"\376",		/* $b7 */
	"\216\051",	/* $b8 */
	"\261",		/* $b9 */
	"\216\052",	/* $ba */
	"\216\053",	/* $bb */
	"\216\054",	/* $bc */
	"\216\055",	/* $bd */
	"\216\056",	/* $be */
	"\322",		/* $bf */

	"\216\057",	/* $c0 */
	"\216\060",	/* $c1 */
	"\216\061",	/* $c2 */
	"\216\062",	/* $c3 */
	"\241",		/* $c4 */
	"\243",		/* $c5 */
	"\245",		/* $c6 */
	"\216\063",	/* $c7 */
	"\216\064",	/* $c8 */
	"\216\065",	/* $c9 */
	"\216\066",	/* $ca */
	"\216\067",	/* $cb */
	"\216\070",	/* $cc */
	"\216\071",	/* $cd */
	"\216\072",	/* $ce */
	"\216\073",	/* $cf */

	"\216\074",	/* $d0 */
	"\320",		/* $d1 */
	"\216\075",	/* $d2 */
	"\216\076",	/* $d3 */
	"\216\077",	/* $d4 */
	"\216\046",	/* $d5 */
	"\253",		/* $d6 */
	"\216\131",	/* $d7 */
	"0",		/* $d8 */
	"\216\101",	/* $d9 */
	"\216\102",	/* $da */
	"\216\103",	/* $db */
	"\256",		/* $dc */
	"\216\104",	/* $dd */
	"\216\127",	/* $de */
	"\273",		/* $df */

	"\247",		/* $e0 */
	"\216\106",	/* $e1 */
	"\216\107",	/* $e2 */
	"\216\110",	/* $e3 */
	"\242",		/* $e4 */
	"\244",		/* $e5 */
	"\246",		/* $e6 */
	"\250",		/* $e7 */
	"\252",		/* $e8 */
	"\251",		/* $e9 */
	"\216\111",	/* $ea */
	"\216\112",	/* $eb */
	"\216\113",	/* $ec */
	"\216\114",	/* $ed */
	"\216\115",	/* $ee */
	"\216\116",	/* $ef */

	"\216\117",	/* $f0 */
	"\321",		/* $f1 */
	"\216\120",	/* $f2 */
	"\216\121",	/* $f3 */
	"\216\122",	/* $f4 */
	"\216\123",	/* $f5 */
	"\254",		/* $f6 */
	"\335",		/* $f7 */
	"\255",		/* $f8 */
	"\272",		/* $f9 */
	"\216\124",	/* $fa */
	"\216\125",	/* $fb */
	"\257",		/* $fc */
	"\216\126",	/* $fd */
	"\216\105",	/* $fe */
	"\216\130"	/* $ff */
};
