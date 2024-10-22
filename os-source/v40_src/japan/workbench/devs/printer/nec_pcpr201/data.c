/*
 * data.c --- table for NEC_PCPR201 driver
 *
 */

char *CommandTable[] =
{
	"\033c1",         /* 00 aRIS reset                   */ /* ESCc1 */
	"\377",           /* 01 aRIN initialize              */ /* dospecial.c */
	"\012",           /* 02 aIND linefeed                */ /* LF */
	"\015\012",       /* 03 aNEL CRLF                    */ /* CR+LF */
	"\033r\012",      /* 04 aRI reverse LF               */ /* ESCr+LF */

	"\034c,,0.",      /* 05 aSGR0 normal char set        */ /* FSc,,0. */
	"\034c,,2.",      /* 06 aSGR3 italics on             */ /* FSc,,2. */
	"\034c,,0.",      /* 07 aSGR23 italics off           */ /* FSc,,0. */
	"\033X\033_1",    /* 08 aSGR4 underline on           */ /* ESCX+ESC_1 */
	"\033Y",          /* 09 aSGR24 underline off         */ /* ESCY */
	"\034c,,1.",      /* 10 aSGR1 boldface on            */ /* FSc,,1. */
	"\034c,,0.",      /* 11 aSGR22 boldface off          */ /* FSc,,0. */
	"\377",           /* 12 aSFC set foreground color    */
	"\377",           /* 13 aSBC set background color    */

	"\033H",          /* 14 aSHORP0 normal pitch         */ /* ESCH (HD pika) */
	"\033E",          /* 15 aSHORP2 elite on             */ /* ESCE */
	"\033H",          /* 16 aSHORP1 elite off            */ /* ESCH (HD pika) */
	"\033Q",          /* 17 aSHORP4 condensed fine on    */ /* ESCQ */
	"\033H",          /* 18 aSHORP3 condensed fine off   */ /* ESCH (HD pika) */
	"\033e22",        /* 19 aSHORP6 enlarge on           */ /* ESCe## (double) */
	"\033e11",        /* 20 aSHORP5 enlarge off          */ /* ESCe## (single) */

	"\034c,,4.",      /* 21 aDEN6 shadow print on        */ /* FSc,,4. */
	"\034c,,0.",      /* 22 aDEN5 shadow print off       */ /* FSc,,0. */
	"\033!",          /* 23 aDEN4 double strike on       */ /* ESC! */
	"\033\042",       /* 24 aDEN3 double strike off      */ /* ESC" */
	"\033d1",         /* 25 aDEN2 NLQ on                 */ /* ESCd1 (Draft off) */
	"\033d0",         /* 26 aDEN1 NLQ off                */ /* ESCd0 (Draft on) */

	"\033s1",         /* 27 aSUS2 superscript on         */ /* ESCs1 */
	"\033s0",         /* 28 aSUS1 superscript off        */ /* ESCs0 */
	"\033s2",         /* 29 aSUS4 subscript on           */ /* ESCs2 */
	"\033s0",         /* 30 aSUS3 subscript off          */ /* ESCs0 */
	"\377",           /* 31 aSUS0 normalize the line     */
	"\377",           /* 32 aPLU partial line up         */
	"\377",           /* 33 aPLD partial line down       */

	"\377",           /* 34 aFNT0 Typeface 0             */
	"\377",           /* 35 aFNT1 Typeface 1             */
	"\377",           /* 36 aFNT2 Typeface 2             */
	"\377",           /* 37 aFNT3 Typeface 3             */
	"\377",           /* 38 aFNT4 Typeface 4             */
	"\377",           /* 39 aFNT5 Typeface 5             */
	"\377",           /* 40 aFNT6 Typeface 6             */
	"\377",           /* 41 aFNT7 Typeface 7             */
	"\377",           /* 42 aFNT8 Typeface 8             */
	"\377",           /* 43 aFNT9 Typeface 9             */
	"\377",           /* 44 aFNT10 Typeface 10           */

	"\033P",          /* 45 aPROP2 proportional on       */ /* ESCP */
	"\033H",          /* 46 aPROP1 proportional off      */ /* ESCH (HD pika) */
	"\033H",          /* 47 aPROP0 proportional clear    */ /* ESCH (HD pika) */
	"\377",           /* 48 aTSS set proportional offset */
	"\377",           /* 49 aJFY5 auto left justify      */
	"\377",           /* 50 aJFY7 auto right justify     */
	"\377",           /* 51 aJFY6 auto full jusitfy      */
	"\377",           /* 52 aJFY0 auto jusity off        */
	"\377",           /* 53 aJFY3 letter space           */
	"\377",           /* 54 aJFY1 word fill              */

	"\033B",          /* 55 aVERP0 1/8" line spacing     */ /* ESCB */
	"\033A",          /* 56 aVERP1 1/6" line spacing     */ /* ESCA */
	"\033v70.",       /* 57 aSLPP set form length        */ /* ESCv##. */
	"\377",           /* 58 aPERF perf skip n (n > 0)    */
	"\377",           /* 59 aPERF0 perf skip off         */

	"\033L",          /* 60 aLMS set left margin         */ /* ESCL */
	"\033/",          /* 61 aRMS set right margin        */ /* ESC/ */
	"\377",           /* 62 aTMS set top margin          */
	"\377",           /* 63 aBMS set bottom margin       */ /* dospecial.c */
	"\377",           /* 64 aSTBM set T&B margins        */ /* dospecial.c */
	"\377",           /* 65 aSLRM set L&R margins        */ /* dospecial.c */
	"\377",           /* 66 aCAM clear margins           */ /* dospecial.c */

	"\033(",          /* 67 aHTS set horiz tab           */ /* ESC( */
	"\377",           /* 68 aVTS set vert tab            */
	"\033)",          /* 69 aTBC0 clear horiz tab        */ /* ESC) */
	"\0332",          /* 70 aTBC3 clear all horiz tabs   */ /* ESC2 */
	"\377",           /* 71 aTBC1 clear vert tab         */
	"\377",           /* 72 aTBC4 clear all vert tabs    */
	"\377",           /* 73 aTBCALL clear all h & v tabs */
	"\377",           /* 74 aTBSALL set default tabs     */

	"\377",           /* 75 aEXTEND extended commands    */
	"\377"            /* 76 aRAW next 'n' chars are raw  */
};
#if 0
char *ExtendedCharTable[] = {
	" ",                      /* NBSP */
	"\033R\007[\033R\\0",     /* i */
	"c\010|",                 /* c| */
	"\033R\003#\033R\\0",     /* L- */
	"\033R\005$\033R\\0",     /* o */
	"\033R\010\\\\\033R\\0",  /* Y- */
	"|",                      /* | */
	"\033R\002@\033R\\0",     /* SS */

	"\033R\001~\033R\\0",     /* " */
	"c",                      /* copyright */
	"\033S\\0a\010_\033T",    /* a_ */
	"<",                      /* << */
	"~",                      /* - */
	"-",                      /* SHY */
	"r",                      /* registered trademark */
	"-",                      /* - */

	"\033R\001[\033R\\0",     /* degrees */
	"+\010_",                 /* +_ */
	"\033S\\0002\033T",       /* 2 */
	"\033S\\0003\033T",       /* 3 */
	"'",                      /* ' */
	"u",                      /* u */
	"P",                      /* reverse P */
	"\033S\\000.\033T",       /* . */

	",",                      /* , */
	"\033S\\0001\033T",       /* 1 */
	"\033R\001[\033R\\0\010-",/* o_ */
	">",	                  /* >> */
	"\033$\274",              /* ESC$+0xbc si */
	"\033$\275",              /* ESC$+0xbd su */
	"\033$\276",              /* ESC$+0xbe se */
	"\033R\007]\033R\\0",     /* upside down ? */

	"\033$\300",              /* ESC$+0xc0 ta */
	"\033$\301",              /* ESC$+0xc1 ti */
	"\033$\302",              /* ESC$+0xc2 tu */
	"\033$\303",              /* ESC$+0xc3 te */
	"\033$\304",              /* ESC$+0xc4 to */
	"\033$\305",              /* ESC$+0xc5 na */
	"\033$\306",              /* ESC$+0xc6 ni */
	"\033$\307",              /* ESC$+0xc7 nu */

	"\033$\310",              /* ESC$+0xc8 ne */
	"\033$\311",              /* ESC$+0xc9 no */
	"\033$\312",              /* ESC$+0xca ha */
	"\033$\313",              /* ESC$+0xcb hi */
	"\033$\314",              /* ESC$+0xcc hu */
	"\033$\315",              /* ESC$+0xcd he */
	"\033$\316",              /* ESC$+0xce ho */
	"\033$\317",              /* ESC$+0xcf ma */

	"\033$\320",              /* ESC$+0xd0 mi */
	"\033$\321",              /* ESC$+0xd1 mu */
	"\033$\322",              /* ESC$+0xd2 me */
	"\033$\323",              /* ESC$+0xd3 mo */
	"\033$\324",              /* ESC$+0xd4 ya */
	"\033$\325",              /* ESC$+0xd5 yu */
	"\033$\326",              /* ESC$+0xd6 yo */
	"\033$\327",              /* ESC$+0xd7 ra */

	"\033$\330",              /* ESC$+0xd8 ri */
	"\033$\331",              /* ESC$+0xd9 ru */
	"\033$\332",              /* ESC$+0xda re */
	"\033$\333",              /* ESC$+0xdb ro */
	"\033$\334",              /* ESC$+0xdc wa */
	"\033$\335",              /* ESC$+0xdd n */
	"\033$\336",              /* ESC$+0xde dakuten */
	"\033$\337",              /* ESC$+0xdf maru */

	"\033$\340",              /* ESC$+0xe0 */
	"\033$\341",              /* ESC$+0xe1 */
	"\033$\342",              /* ESC$+0xe2 */
	"\033$\343",              /* ESC$+0xe3 */
	"\033$\344",              /* ESC$+0xe4 */
	"\033$\345",              /* ESC$+0xe5 */
	"\033$\346",              /* ESC$+0xe6 */
	"\033$\347",              /* ESC$+0xe7 */

	"\033$\350",              /* ESC$+0xe8 */
	"\033$\351",              /* ESC$+0xe9 */
	"\033$\352",              /* ESC$+0xea */
	"\033$\353",              /* ESC$+0xeb */
	"\033$\354",              /* ESC$+0xec */
	"\033$\355",              /* ESC$+0xed */
	"\033$\356",              /* ESC$+0xee */
	"\033$\357",              /* ESC$+0xef */

	"\033$\360",              /* ESC$+0xf0 */
	"\033$\361",              /* ESC$+0xf1 */
	"\033$\362",              /* ESC$+0xf2 */
	"\033$\363",              /* ESC$+0xf3 */
	"\033$\364",              /* ESC$+0xf4 */
	"\033$\365",              /* ESC$+0xf5 */
	"\033$\366",              /* ESC$+0xf6 */
	"\033$\367"               /* ESC$+0xf7 */

	"\033$\370",              /* ESC$+0xf8 */
	"\033$\371",              /* ESC$+0xf9 */
	"\033$\372",              /* ESC$+0xfa */
	"\033$\373",              /* ESC$+0xfb */
	"\033$\374",              /* ESC$+0xfc */
	"\033$\375",              /* ESC$+0xfd */
	"\033$\376",              /* ESC$+0xfe */
	"\033$\377"               /* ESC$+0xff */
};
#endif
