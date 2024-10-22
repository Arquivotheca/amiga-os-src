/* imagewriterII   */
/*************************************************************************
 *
 *   NAME
 *   ImagewriterII functions implemented:
 *
 *      aRIS, aRin, aIND, aNEL, aRI, aSGR0, aSGR4, aSGR24, aSGR1, aSGR22,
 *      aSHORP0, aSHORP2, aSHORP1, aSHORP3, aSHORP4, aSHORP5, aSHORP6,
 *      aDEN1, aDen2, aDEN3, aDEN4,
 *      aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,
 *      aFNT0, aFNT1, aFNT2, aFNT3, aFNT4, aFNT5, aFNT6, aFNT7,
 *      aPROP2, aPROP1,
 *      aVERP0, aVERP1,
 *      aLMS,aCAM,aTBC3,aTBSALL
 *
 *   special functions implemented:
 *
 *      aRIN, aSHORP2, aSHORP1, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,
 *      aPROP2, aPROP1, aPROP0, aVERP0, aVERP1, aLMS,
 *      aCAM
 *
 ************************************************************************/

char *CommandTable[] ={
         "\375\033c\375", /*reset              aRIS     ESCc      */
         "\377",        /*initialize         aRIN     ESC#1           !!!!*/
         "\033l1\012\033l0",        /* lf                aIND     ESCD      */
         "\015\012",    /* return,lf         aNEL     ESCE      */
         "\033l1\033r\033f\033l0", /* reverse lf  aRI      ESCM      */

               /*normal char set    SGR 0    ESC[0m */
         "\033Y\033\042",
         "\377",        /*italics on         SGR 3    ESC[3m    */
         "\377",        /*italics off        SGR 23   ESC[23m   */
         "\033X",       /*underline on       SGR 4    ESC[4m    */
         "\033Y",       /*underline off      SGR 24   ESC[24m   */
         "\033!",       /*boldface on        SGR 1    ESC[1m    */
         "\033\042",    /*boldface off       SGR 22   ESC[22m   */
         "\377",        /* set foreground color */
         "\377",        /* set background color */

            /* normal char set  SHORP0    ESC[0w */
         "\033N\033z\017",
         "\377",       /*elite on            SHORP2    ESC[2w         !!!!*/
         "\377",       /*elite off           SHORP1    ESC[1w         !!!!*/
         "\033q",       /*condensed(fine) on SHORP4    ESC[4w         !!!!*/
         "\377",        /*condensed off      SHORP3    ESC[3w         !!!!*/
         "\016",        /*enlarged on        SHORP6    ESC[6w         !!!!*/
         "\017",        /*enlarged off       SHORP5    ESC[5w         !!!!*/

         "\377",        /*shadow print on    DEN6     ESC[6"z */
         "\377",        /*shadow print off   DEN5     ESC[5"z */
         "\377",        /*doublestrike on    DEN4     ESC[4"z */
         "\377",        /*doublestrike off   DEN3     ESC[3"z */
         "\033a2",      /* NLQ on            DEN2     ESC[2"z */
         "\033a1",      /* NLQ off           DEN1     ESC[1"z */

         "\033x",       /*superscript on     aSUS2    ESC[2v          !!!!*/
         "\033z",       /*superscript off    aSUS1    ESC[1v          !!!!*/
         "\033y",       /*subscript on       aSUS4    ESC[4v          !!!!*/
         "\033z",       /*subscript off      aSUS3    ESC[3v          !!!!*/
         "\033z",       /*normalize          aSUS0    ESC[0v          !!!!*/
         "\377",        /* partial line up   aPLU     ESCL            !!!!*/
         "\377",        /* partial line down aPLD     ESCK            !!!!*/

         "\033Z\007\376",                /*US char set      FNT0 ESC(B */
         "\033Z\001\376\033D\006\376",   /*French char set  FNT1 ESC(R */
         "\033Z\003\376\033D\004\376",   /*German char set  FNT2 ESC(K */
         "\033Z\004\376\033D\003\376",   /*UK char set      FNT3 ESC(A */
         "\033Z\005\376\033D\002\376",   /*Danish I char setFNT4 ESC E */
         "\033Z\002\376\033D\005\376",   /*Sweden char set  FNT5 ESC(H */
         "\033Z\006\376\003D\001\376",   /*Italian char set FNT6 ESC(Y */
         "\033D\007\376",                /*Spanish char set FNT7 ESC(Z */
         "\377",                  /*Japanese char set  FNT8     ESC(J */
         "\377",                  /*Norweign char set  FNT9     ESC(6 */
         "\377",                  /*Danish II char set FNT10    ESC(C */

         "\377",                /*proportional on    PROP2  ESC[2p     !!!!*/
         "\377",                /*proportional off   PROP1  ESC[1p     !!!!*/
         "\377",                /*proportional clear PROP0  ESC[0p     */
         "\377",                /*set prop offset    aTSS              */
         "\377",                /*auto left justify  aJFY5  ESC[5 F */
         "\377",                /*auto right justify aJFY7  ESC[7 F */
         "\377",                /*auto full justify  aJFY6  ESC[6 F */
         "\377",                /*auto justify/center off aJFY0  ESC[0 F */
         "\377",                /*place holder       aJFY3  ESC[3 F */
         "\377",                /*auto center on     aJFY1  ESC[2 F */

         "\033B",               /* 1/8" line space   aVERP0     ESC[0z !!!!*/
         "\033A",               /* 1/6" line spacing aVERP1     ESC[1z !!!!*/
         "\377",                /* set form length   aSLPP      ESC[Pnt*/
         "\377",                /* perf skip n       aPERF      ESC[nq */
         "\377",                /* perf skip off     aPERF0     ESC[0q */

         "\377",        /* Left margin set           aLMS       ESC[2x  !!!!*/
         "\377",        /* Right margin set          aRMS       ESC[3x */
         "\377",        /* top margin set            aTMS       ESC[4x */
         "\377",        /* Bottom marg set           aBMS       ESC[5x */
         "\377",        /* T&B margin set            aSTBM      ESC[Pn1;Pn2r*/
         "\377",        /* L&R margin set            aSLRM      ESC[Pn1;Pn2s*/
         "\377",        /* Clear margins             aCAM       ESC[0x */

         "\377",        /* Set horiz tab             aHTS       ESCH   */
         "\377",        /* Set vertical tab          aVTS       ESCJ   */
         "\377",        /* Clr horiz tab             aTBC0      ESC[0g */

                        /* Clear all h tabs          aTBC3      ESC[3g */
         "\0330",

         "\377",        /* Clr vertical tab          aTBC1      ESC[1g */

         "\377",        /* Clr all v tabs            aTBC4      ESC[4g */

         "\0330",        /* Clr all h & v tabs        aTBCALL    ESC#4 */

                        /* set default tabs          aTBSALL    ESC#5 */
         "\033(009,017,025,033,041,049,057,065,073.",
         "\377"        /* entended command           aEXTEND    ESC[n"x */

		"\377"			/* set raw mode (dummy)		aRAW		ESC[Pn"r */
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


	Amer	\033Z\007\\000
	Brit	\033Z\004\\000\033D\003\\000
	Germ	\033Z\003\\000\033D\004\\000
	Fren	\033Z\001\\000\033D\006\\000
	Swed	\033Z\002\\000\033D\005\\000
	Ital	\033Z\006\\000\033D\001\\000
	Span	\033D\007\\000
	Dani	\033Z\005\\000\033D\002\\000

*/
	" ",			/*    ok */
	"\033D\007\\000\133\033Z\007\\000", /* !  ok */
	"c\010/",		/* "   */
	"\033Z\004\\000\033D\003\\000\043\033Z\007\\000", /* #  ok */
	"\033\170o\033\172",	/* $  ok */
	"Y\010\055",		/* %  ok */
	"|",			/* &  ok */
	"\033Z\001\\000\033D\006\\000\135\033Z\007\\000", /* '  ok */

	"\033Z\001\\000\033D\006\\000\176\033Z\007\\000", /* (  ok */
	"c",			/* )  ok */
	"\033\170a\033\172",	/* *  ok */
	"`",			/* +  ok */
	"~",			/* ,   */
	"-",			/* -  ok */
	"r",			/* .   */
	"\033&\314\033$",	/* /  ok */

	"\033Z\006\\000\033D\001\\000\133\033Z\007\\000", /* 0  ok */
	"+\010_",		/* 1  ok */
	"\033\1702\033\172",	/* 2  ok */
	"\033\1703\033\172",	/* 3  ok */
	"\047",			/* 4  ok */
	"u",			/* 5  ok */
	"P",			/* 6  ok */
	"\033D\007\\000\173\033Z\007\\000", /* 7  ok */

	",",			/* 8  ok */
	"\033\1701\033\172",	/* 9  ok */
	"\033\1700\033\172",	/* :  ok */
	"'",			/* ;  ok */
	"/",			/* <  ok */
	"/",			/* =  ok */
	"/",			/* >  ok */
	"\033D\007\\000\135\033Z\007\\000", /* ?  ok */

	"A",			/* @  ok */
	"A",			/* A  ok */
	"A",			/* B  ok */
	"A",			/* C  ok */
	"\033Z\003\\000\033D\004\\000\133\033Z\007\\000", /* D  ok */
	"\033Z\002\\000\033D\005\\000\135\033Z\007\\000", /* E  ok */
	"\033Z\005\\000\033D\002\\000\133\033Z\007\\000", /* F  ok */
	"C\010,",		/* G  ok */

	"E",			/* H  ok */
	"E",			/* I  ok */
	"E",			/* J  ok */
	"E",			/* K  ok */
	"I",			/* L  ok */
	"I",			/* M  ok */
	"I",			/* N  ok */
	"I",			/* O  ok */

	"D\010-",		/* P  ok */
	"\033D\007\\000\\\\\033Z\007\\000", /* Q   */
	"O",			/* R   */
	"O",			/* S   */
	"O",			/* T   */
	"O",			/* U   */
	"\033Z\003\\000\033D\004\\000\\\\\033Z\007\\000", /* V   */
	"x",			/* W   */

	"\033Z\005\\000\033D\002\\000\\\\\033Z\007\\000", /* X   */
	"U",			/* Y   */
	"U",			/* Z   */
	"U",			/* [   */
	"\033Z\003\\000\033D\004\\000\135\033Z\007\\000", /* \   */
	"Y",			/* ]   */
	"T",			/* ^   */
	"\033Z\003\\000\033D\004\\000\176\033Z\007\\000", /* _   */

	"\033Z\006\\000\033D\001\\000\173\033Z\007\\000", /* `  ok */
	"a\010\'",		/* a  ok */
	"a\010^",		/* b  ok */
	"a\010~",		/* c  ok */
	"\033Z\003\\000\033D\004\\000\173\033Z\007\\000", /* d  ok */
	"\033Z\002\\000\033D\005\\000\175\033Z\007\\000", /* e  ok */
	"\033Z\005\\000\033D\002\\000\173\033Z\007\\000", /* f  ok */
	"\033Z\006\\000\033D\001\\000\\\\\033Z\007\\000", /* g   */

	"\033Z\001\\000\033D\006\\000\175\033Z\007\\000", /* h   */
	"\033Z\001\\000\033D\006\\000\173\033Z\007\\000", /* i   */
	"e\010^",		/* j   */
	"\033Z\001\\000\033D\006\\000\176\010e\033Z\007\\000", /* k   */
	"\033Z\006\\000\033D\001\\000\176\033Z\007\\000", /* l   */
	"i\010\'",		/* m   */
	"i\010^",		/* n   */
	"\033Z\001\\000\033D\006\\000\176\010i\033Z\007\\000", /* o   */

	"d",			/* p   */
	"\033D\007\\000\174\033Z\007\\000", /* q   */
	"\033Z\007\\000\140\010o", /* r   */
	"o\010\'",		/* s   */
	"o\010^",		/* t   */
	"o\010~",		/* u   */
	"\033Z\001\\000\033D\006\\000\176\010o\033Z\007\\000", /* v   */
	"/",			/* w   */

	"\033Z\005\\000\033D\002\\000\174\033Z\007\\000", /* x   */
	"\033Z\001\\000\033D\006\\000\174\033Z\007\\000", /* y   */
	"u\010\'",		/* z   */
	"u\010^",		/* {   */
	"\033Z\003\\000\033D\004\\000\175\033Z\007\\000", /* |   */
	"y\010\'",		/* }   */
	"t",			/* ~   */
	"\033Z\001\\000\033D\006\\000\176\010y\033Z\007\\000" /*   */
};

