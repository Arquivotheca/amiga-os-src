/*
	Data.c table for Sharp_JX-730 driver.
	David Berezowski - Nov/89.
*/

char *CommandTable[]={
			/* 00 aRIS reset			*/
	"\375\033N\033@\375\033N",
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\015\012",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/
	
	/* 05 aSGR0 normal char set				*/
	"\033Y\033\042",
	"\377",		/* 06 aSGR3 italics on			*/
	"\377",		/* 07 aSGR23 italics off		*/
	"\033_1\033X",	/* 08 aSGR4 underline on		*/
	"\033Y",	/* 09 aSGR24 underline off		*/
	"\033!",	/* 10 aSGR1 boldface on			*/
	"\033\042",	/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/
	
	"\033H\017",	/* 14 aSHORP0 normal pitch		*/
	"\033E",	/* 15 aSHORP2 elite on			*/
	"\033H",	/* 16 aSHORP1 elite off			*/
	"\033Q",	/* 17 aSHORP4 condensed fine on		*/
	"\033H",	/* 18 aSHORP3 condensed fine off	*/
	"\016",		/* 19 aSHORP6 enlarge on		*/
	"\017",		/* 20 aSHORP5 enlarge off		*/
	
	"\377",		/* 21 aDEN6 shadow print on		*/
	"\377",		/* 22 aDEN5 shadow print off		*/
	"\377",		/* 23 aDEN4 double strike on		*/
	"\377",		/* 24 aDEN3 double strike off		*/
	"\033d1",	/* 25 aDEN2 NLQ on			*/
	"\033d0",	/* 26 aDEN1 NLQ off			*/
	
	"\033s1",	/* 27 aSUS2 superscript on		*/
	"\033s0",	/* 28 aSUS1 superscript off		*/
	"\033s2",	/* 29 aSUS4 subscript on		*/
	"\033s0",	/* 30 aSUS3 subscript off		*/
	"\033s0",	/* 31 aSUS0 normalize the line		*/
	"\033s1",	/* 32 aPLU partial line up		*/
	"\033s2",	/* 33 aPLD partial line down		*/
	
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
	
	"\033B",	/* 55 aVERP0 1/8" line spacing		*/
	"\033A",	/* 56 aVERP1 1/6" line spacing		*/
	"\377",		/* 57 aSLPP set form length		*/
	"\377",		/* 58 aPERF perf skip n (n > 0)		*/
	"\377",		/* 59 aPERF0 perf skip off		*/
	
	"\377",		/* 60 aLMS set left margin		*/
	"\377",		/* 61 aRMS set right margin		*/
	"\377",		/* 62 aTMS set top margin		*/
	"\377",		/* 63 aBMS set bottom margin		*/
	"\377",		/* 64 aSTBM set T&B margins		*/
	"\377",		/* 65 aSLRM set L&R margins		*/
			/* 66 aCAM clear margins		*/
	"\033L001\033/999",
	
	"\377",		/* 67 aHTS set horiz tab		*/
	"\377",		/* 68 aVTS set vert tab			*/
	"\377",		/* 69 aTBC0 clear horiz tab		*/
	"\0332",	/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\0332",	/* 73 aTBCALL clear all h & v tabs	*/
			/* 74 aTBSALL set default tabs		*/
"\033(008,016,024,032,040,048,056,064,072,080,088,096,104,112,120,128,136.",

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

	"\033G \033N",		/* $a0 */
	"\033G!\033N",		/* $a1 */
	"\033G\233\033N",	/* $a2 */
	"\033G\234\033N",	/* $a3 */
	"\033Go\033N",		/* $a4 */
	"\033G\235\033N",	/* $a5 */
	"\033G\263\033N",	/* $a6 */
	"\033GS\033N",		/* $a7 */
	"\033G\034\033N",	/* $a8 */
	"\033Gc\033N",		/* $a9 */
	"\033G\246\033N",	/* $aa */
	"\033G\256\033N",	/* $ab */
	"\033G\252\033N",	/* $ac */
	"\033G-\033N",		/* $ad */
	"\033Gr\033N",		/* $ae */
	"\033G-\033N",		/* $af */

	"\033G\370\033N",	/* $b0 */
	"\033G\361\033N",	/* $b1 */
	"\033G\375\033N",	/* $b2 */
	"\033G3\033N",		/* $b3 */
	"\033G'\033N",		/* $b4 */
	"\033G\346\033N",	/* $b5 */
	"\033GP\033N",		/* $b6 */
	"\033G\371\033N",	/* $b7 */
	"\033G,\033N",		/* $b8 */
	"\033G1\033N",		/* $b9 */
	"\033G\247\033N",	/* $ba */
	"\033G\257\033N",	/* $bb */
	"\033G\254\033N",	/* $bc */
	"\033G\253\033N",	/* $bd */
	"\033G4\033N",		/* $be */
	"\033G\250\033N",	/* $bf */

	"\033GA\033N",		/* $c0 */
	"\033GA\033N",		/* $c1 */
	"\033GA\033N",		/* $c2 */
	"\033GA\033N",		/* $c3 */
	"\033G\216\033N",	/* $c4 */
	"\033G\217\033N",	/* $c5 */
	"\033G\222\033N",	/* $c6 */
	"\033G\200\033N",	/* $c7 */
	"\033GE\033N",		/* $c8 */
	"\033G\220\033N",	/* $c9 */
	"\033GE\033N",		/* $ca */
	"\033GE\033N",		/* $cb */
	"\033GI\033N",		/* $cc */
	"\033GI\033N",		/* $cd */
	"\033GI\033N",		/* $ce */
	"\033GI\033N",		/* $cf */

	"\033GD\033N",		/* $d0 */
	"\033G\245\033N",	/* $d1 */
	"\033GO\033N",		/* $d2 */
	"\033GO\033N",		/* $d3 */
	"\033GO\033N",		/* $d4 */
	"\033GO\033N",		/* $d5 */
	"\033GO\033N",		/* $d6 */
	"\033Gx\033N",		/* $d7 */
	"\033G0\033N",		/* $d8 */
	"\033GU\033N",		/* $d9 */
	"\033GU\033N",		/* $da */
	"\033GU\033N",		/* $db */
	"\033G\232\033N",	/* $dc */
	"\033GY\033N",		/* $dd */
	"\033GI\033N",		/* $de */
	"\033G\341\033N",	/* $df */

	"\033Ga\033N",		/* $e0 */
	"\033G\240\033N",	/* $e1 */
	"\033G\203\033N",	/* $e2 */
	"\033G\205\033N",	/* $e3 */
	"\033G\204\033N",	/* $e4 */
	"\033G\206\033N",	/* $e5 */
	"\033G\221\033N",	/* $e6 */
	"\033G\207\033N",	/* $e7 */
	"\033Ge\033N",		/* $e8 */
	"\033G\202\033N",	/* $e9 */
	"\033G\210\033N",	/* $ea */
	"\033G\211\033N",	/* $eb */
	"\033Gi\033N",		/* $ec */
	"\033Gi\033N",		/* $ed */
	"\033G\214\033N",	/* $ee */
	"\033G\215\033N",	/* $ef */

	"\033Gd\033N",		/* $f0 */
	"\033G\244\033N",	/* $f1 */
	"\033Go\033N",		/* $f2 */
	"\033Go\033N",		/* $f3 */
	"\033G\223\033N",	/* $f4 */
	"\033G\225\033N",	/* $f5 */
	"\033G\224\033N",	/* $f6 */
	"\033G\366\033N",	/* $f7 */
	"\033G0\033N",		/* $f8 */
	"\033Gu\033N",		/* $f9 */
	"\033Gu\033N",		/* $fa */
	"\033G\226\033N",	/* $fb */
	"\033G\201\033N",	/* $fc */
	"\033Gy\033N",		/* $fd */
	"\033GI\033N",		/* $fe */
	"\033G\230"		/* $ff */
};
