/**********************************************************/
/* Copyright   ©1991 Wolf-Juergen Faust                   */
/* Am Dorfgarten 10                                       */
/* W-6000 Frankfurt 50   Canon BJ 10 driver               */
/* Germany                                                */
/* FIDO: 2:243/43.5 (Wolf Faust)                          */
/* UUCP: cbmvax.commodore.com!cbmehq!cbmger!venus!wfaust  */
/* Tel: +(49) 69 5486556                                  */
/*                                                        */
/* This File is NOT PART OF THE DISTRIBUTION package !!!  */
/*          DO NOT SPREAD THIS SOURCE FILE !!!            */
/*                                                        */
/* Versions required:                                     */
/* SAS/C 5.10a                                            */
/**********************************************************/

char *CommandTable[] =
{
                        /* 00 aRIS reset                        */
        "\033[K\001\376\001",
        "\377",         /* 01 aRIN initialize                   */
        "\377",         /* 02 aIND linefeed                     */
        "\012\015",     /* 03 aNEL CRLF                         */
        "\377",         /* 04 aRI reverse LF                    */

                        /* 05 aSGR0 normal char set             */
        "\033-\376\033F",
        "\377",         /* 06 aSGR3 italics on                  */
        "\377",         /* 07 aSGR23 italics off                */
        "\033-\001",    /* 08 aSGR4 underline on                */
        "\033-\376",    /* 09 aSGR24 underline off              */
        "\033E",        /* 10 aSGR1 boldface on                 */
        "\033F",        /* 11 aSGR22 boldface off               */
        "\377",         /* 12 aSFC set foreground color         */
        "\377",         /* 13 aSBC set background color         */

                        /* 14 aSHORP0 normal pitch              */
        "\022\033W\376",
                        /* 15 aSHORP2 elite on                  */
        "\033:\033W\376",
        "\022",         /* 16 aSHORP1 elite off                 */
                        /* 17 aSHORP4 condensed fine on         */
        "\033W\376\017",
        "\022",         /* 18 aSHORP3 condensed fine off        */
        "\033W\001",    /* 19 aSHORP6 enlarge on                */
        "\033W\376",    /* 20 aSHORP5 enlarge off               */

        "\033[@\002\376\376\002",   /* 21 aDEN6 shadow print on             */
        "\033[@\002\376\376\001",   /* 22 aDEN5 shadow print off            */
        "\033G",        /* 23 aDEN4 double strike on            */
        "\033H",        /* 24 aDEN3 double strike off           */
        "\377",         /* 25 aDEN2 NLQ on                      */
        "\377",         /* 26 aDEN1 NLQ off                     */

        "\033S\376",    /* 27 aSUS2 superscript on              */
        "\033T",        /* 28 aSUS1 superscript off             */
        "\033S\001",    /* 29 aSUS4 subscript on                */
        "\033T",        /* 30 aSUS3 subscript off               */
        "\033T",        /* 31 aSUS0 normalize the line          */
        "\377",         /* 32 aPLU partial line up              */
        "\377",         /* 33 aPLD partial line down    */

        "\033I\002",    /* 34 aFNT0 Typeface 0                  */
        "\033I\012",    /* 35 aFNT1 Typeface 1                  */
        "\033I\022",    /* 36 aFNT2 Typeface 2                  */
        "\033I\003",    /* 37 aFNT3 Typeface 3                  */
        "\033I\004",    /* 38 aFNT4 Typeface 4                  */
        "\033I\014",    /* 39 aFNT5 Typeface 5                  */
        "\033I\024",    /* 40 aFNT6 Typeface 6                  */
        "\033I\006",    /* 41 aFNT7 Typeface 7                  */
        "\033I\016",    /* 42 aFNT8 Typeface 8                  */
        "\033I\026",    /* 43 aFNT9 Typeface 9                  */
#if 1
        "\033I\007",	/* 44 aFNT10 Typeface 10                */
#else
        "\033I\007\000$VER: CanonBJ10 1.2 (27.11.91)",  /* 44 aFNT10 Typeface 10*/
#endif

        "\033P\001",    /* 45 aPROP2 proportional on            */
        "\033P\376",    /* 46 aPROP1 proportional off           */
        "\377",         /* 47 aPROP0 proportional clear         */
        "\377",         /* 48 aTSS set proportional offset      */
        "\377",         /* 49 aJFY5 auto left justify           */
        "\377",         /* 50 aJFY7 auto right justify          */
        "\377",         /* 51 aJFY6 auto full jusitfy           */
        "\377",         /* 52 aJFY0 auto jusity off             */
        "\377",         /* 53 aJFY3 letter space                */
        "\377",         /* 54 aJFY1 word fill                   */

        "\0330",        /* 55 aVERP0 1/8" line spacing		*/
        "\0332",        /* 56 aVERP1 1/6" line spacing		*/
        "\377",         /* 57 aSLPP set form length             */
        "\377",         /* 58 aPERF perf skip n (n > 0)         */
        "\033O",        /* 59 aPERF0 perf skip off              */

        "\377",         /* 60 aLMS set left margin              */
        "\377",         /* 61 aRMS set right margin             */
        "\377",         /* 62 aTMS set top margin               */
        "\377",         /* 63 aBMS set bottom margin            */
        "\377",         /* 64 aSTBM set T&B margins             */
        "\377",         /* 65 aSLRM set L&R margins             */
        "\377",         /* 66 aCAM clear margins                */

        "\377",         /* 67 aHTS set horiz tab                */
        "\377",         /* 68 aVTS set vert tab                 */
        "\377",         /* 69 aTBC0 clear horiz tab             */
        "\033D\376",    /* 70 aTBC3 clear all horiz tabs        */
        "\377",         /* 71 aTBC1 clear vert tab              */
        "\033B\376",	/* 72 aTBC4 clear all vert tabs         */
#if 1
	/* Changed per Wolf Faust Apr-21-92 */
			/* 73 aTBCALL clear all h & v tabs	*/
	"\033B\376\033D\376",
	"033R",		/* 74 aTBSALL set default tabs		*/
#else
        "\033R\376",    /* 73 aTBCALL clear all h & v tabs      */
                        /* 74 aTBSALL set default tabs          */
"\033D\010\020\030\040\050\060\070\100\110\120\130\140\150\160\170\200\376",
#endif
        "\377",         /* 75 aEXTEND extended commands         */
        "\377",         /* 76 aRAW next 'n' chars are raw       */
};

char *ExtendedCharTable[] = {
        " ",                                    /* NBSP*/
        "\033^\255",                            /* i */
        "c\010|",                               /* c| */
        "\033^\234",                            /* L- */
        "o",                                    /* o */
        "\033^\235",                            /* Y- */
        "\033^\263",                            /* | */
        "\033^\025",                            /* SS */

        "\042",                                 /* " */
        "c",                                    /* copyright */
        "\033^\246",                            /* a_ */
        "\033^\256",                            /* << */
        "\176",                                 /* - */
        "-",                                    /* SHY */
        "r",                                    /* registered trademark */
        "-",                                    /* - */

        "\033^\370",                            /* degrees */
        "\033^\361",                            /* +_ */
        "\033S\\0002\033T",                     /* 2 */
        "\033S\\0003\033T",                     /* 3 */
        "'",					/* ' */
        "\033^\346",                            /* u */
        "P",                                    /* reverse P */
        "\033S\\000.\033T",                     /* . */

        ",",                                    /* , */
        "\033S\\0001\033T",                     /* 1 */
        "\033^\370\010-",                       /* o_ */
        "\033^\257",                            /* >> */
        "\033S\\0001\033T\010-\010\033S\0014\033T",     /* 1/4 */
        "\033S\\0001\033T\010-\010\033S\0012\033T",     /* 1/2 */
        "\033S\\0003\033T\010-\010\033S\0014\033T",     /* 3/4 */
        "\033^\250",                            /* upside down ? */

        "A\010`",                               /* `A */
        "A\010'",				/* 'A */
        "A\010^",                               /* ^A */
        "A\010~",                               /* ~A */
        "\033^\216",                            /* "A */
        "\033^\217",                            /* oA */
        "\033^\222",                            /* AE */
        "\033^\200",                            /* C, */

        "E\010`",                               /* `E */
        "\033^\220",                            /* 'E */
        "E\010^",                               /* ^E */
        "E\010\033S\\000\042\033T",             /* "E */
        "I\010`",                               /* `I */
        "I\010`",                               /* 'I */
        "I\010^",                               /* ^I */
        "I\010\033S\\000\042\033T",             /* "I */

        "D\010-",                               /* -D */
        "\033^\245",                            /* ~N */
        "O\010`",                               /* `O */
        "O\010'",                               /* 'O */
        "O\010^",                               /* ^O */
        "O\010~",                               /* ~O */
        "\033^\231",                            /* "O */
        "\033^\372",                            /* x */

        "\033^\\0",                             /* 0 */
        "U\010`",                               /* `U */
        "U\010'",                               /* 'U */
        "U\010^",                               /* ^U */
        "\033^\232",                            /* "U */
        "Y\010'",                               /* 'Y */
        "T",                                    /* Thorn */
        "\033^\341",                            /* B */

        "\033^\205",                            /* `a */
        "\033^\240",                            /* 'a */
        "\033^\203",                            /* ^a */
        "a\010~",                               /* ~a */
        "\033^\204",                            /* "a */
        "\033^\206",                            /* oa */
        "\033^\221",                            /* ae */
        "\033^\207",                            /* c, */

        "\033^\212",                            /* `e */
        "\033^\202",                            /* 'e */
        "e\010^",                               /* ^e */
        "\033^\211",                            /* "e */
        "\033^\215",                            /* `i */
        "\033^\241",                            /* 'i */
        "\033^\214",                            /* ^i */
        "\033^\213",                            /* "i */

        "d",                                    /* d */
        "\033^\244",                            /* ~n */
        "\033^\225",                            /* `o */
        "\033^\242",                            /* 'o */
        "\033^\223",                            /* ^o */
        "o\010~",                               /* ~o */
        "\033^\224",                            /* "o */
        "\033^\366",                            /* :- */

        "o\010/",                               /* o/ */
        "\033^\227",                            /* `u */
        "\033^\243",                            /* 'u */
        "\033^\226",                            /* ^u */
        "\033^\201",                            /* "u */
        "y\010'",                               /* 'y */
        "t",                                    /* thorn */
        "\033^\230"                             /* "y */
};
