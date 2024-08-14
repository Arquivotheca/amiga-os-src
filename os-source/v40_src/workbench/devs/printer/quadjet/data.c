/*
	Data.c table for Quadram_QuadJet driver.
	David Berezowski - March/88.
*/

char *CommandTable[] ={
	"\377",		/* 00 aRIS reset			*/
	"\377",		/* 01 aRIN initialize			*/
	"\012",		/* 02 aIND linefeed			*/
	"\012\015",	/* 03 aNEL CRLF				*/
	"\377",		/* 04 aRI reverse LF			*/

	"\033R",	/* 05 aSGR0 normal char set		*/
	"\377",		/* 06 aSGR3 italics on			*/
	"\377",		/* 07 aSGR23 italics off		*/
	"\377",		/* 08 aSGR4 underline on		*/
	"\377",		/* 09 aSGR24 underline off		*/
	"\033D",	/* 10 aSGR1 boldface on			*/
	"\033R",	/* 11 aSGR22 boldface off		*/
	"\377",		/* 12 aSFC set foreground color		*/
	"\377",		/* 13 aSBC set background color		*/

	"\033N",	/* 14 aSHORP0 normal pitch		*/
	"\377",		/* 15 aSHORP2 elite on			*/
	"\377",		/* 16 aSHORP1 elite off			*/
	"\377",		/* 17 aSHORP4 condensed fine on		*/
	"\377",		/* 18 aSHORP3 condensed fine off	*/
	"\033L",	/* 19 aSHORP6 enlarge on		*/
	"\033N",	/* 20 aSHORP5 enlarge off		*/

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
	"\377",		/* 33 aPLD partial line down	*/

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

	"\033F\010",	/* 55 aVERP0 1/8" line spacing		*/
	"\033F\006",	/* 56 aVERP1 1/6" line spacing		*/
	"\377",		/* 57 aSLPP set form length		*/
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
	"\377",		/* 70 aTBC3 clear all horiz tabs	*/
	"\377",		/* 71 aTBC1 clear vert tab		*/
	"\377",		/* 72 aTBC4 clear all vert tabs		*/
	"\377",		/* 73 aTBCALL clear all h & v tabs	*/
	"\377",		/* 74 aTBSALL set default tabs		*/

	"\377",		/* 75 aEXTEND extended commands		*/
	"\377",		/* 76 aRAW next 'n' chars are raw	*/
};

char *ExtendedChars[] = {
	" ",					/* NBSP*/
	"\255",					/* upside down ! */
	"\233",					/* c| */
	"\234",					/* L- */
	"o",					/* o */
	"\235",					/* Y- */
	"\263",					/* | */
	"S",					/* SS */

	"\042",					/* " */
	"c", 					/* copyright */
	"\246",					/* a_ */
	"\256",					/* << */
	"\252",					/* - */
	"-",					/* SHY */
	"r",					/* registered trademark */
	"-",					/* - */

	"*",					/* degrees */
	"\361",					/* +_ */
	"\375",					/* 2 */
	"3",					/* 3 */
	"'",					/* ' */
	"u",					/* u */
	"P",					/* reverse P */
	"\372",					/* . */

	",",					/* , */
	"1",					/* 1 */
	"\247",					/* o_ */
	"\257",					/* >> */
	"\254",					/* 1/4 */
	"\253",					/* 1/2 */
	"/",					/* 3/4 */
	"\250",					/* upside down ? */

	"A",					/* `A */
	"A",					/* 'A */
	"A",					/* ^A */
	"A",					/* ~A */
	"\216",					/* "A */
	"\217",					/* oA */
	"\222",					/* AE */
	"\200",					/* C, */

	"E",					/* `E */
	"\220",					/* 'E */
	"E",					/* ^E */
	"E",					/* "E */
	"I",					/* `I */
	"I",					/* 'I */
	"I",					/* ^I */
	"I",					/* "I */

	"D",					/* -D */
	"\245",					/* ~N */
	"O",					/* `O */
	"O",					/* 'O */
	"O",					/* ^O */
	"O",					/* ~O */
	"\224",					/* "O */
	"x",					/* x */

	"0",					/* 0 */
	"U",					/* `U */
	"U",					/* 'U */
	"U",					/* ^U */
	"U",					/* "U */
	"Y",					/* 'Y */
	"T",					/* Thorn */
	"\341",					/* B */

	"\205",					/* `a */
	"\240",					/* 'a */
	"\203",					/* ^a */
	"a",					/* ~a */
	"\204",					/* "a */
	"\206",					/* oa */
	"\221",					/* ae */
	"\207",					/* c, */

	"\212",					/* `e */
	"\202",					/* 'e */
	"\210",					/* ^e */
	"\211",					/* "e */
	"\215",					/* `i */
	"\241",					/* 'i */
	"\214",					/* ^i */
	"\213",					/* "i */

	"\353",					/* d */
	"\244",					/* ~n */
	"\225",					/* `o */
	"\242",					/* 'o */
	"\223",					/* ^o */
	"o",					/* ~o */
	"\231",					/* "o */
	"\366",					/* :- */

	"\355",					/* o/ */
	"\227",					/* `u */
	"\243",					/* 'u */
	"\226",					/* ^u */
	"\201",					/* "u */
	"y",					/* 'y */
	"t",					/* thorn */
	"\230"					/* "y */
};
