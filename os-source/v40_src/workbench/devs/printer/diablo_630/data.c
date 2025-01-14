/* diablo 630 command table */

/****** printer.device/printers/Diablo_630_functions ********************
 *
 *   NAME
 *   Diablo 630 functions implemented: 
 *  
 *      aRIS, aIND, aNEL, aRI, aSGR0, aSGR4, aSGR24, aSGR1, aSGR22,
 *      aSHORP0, aSHORP1, aSHORP2, aSHORP3, aSHORP4,
 *      aDEN3, aDEN4, aDEN5, aDEN6, aPLU, aPLD
 *      aPROP0, aPROP1, aPROP2, aTSS, aJFY5, aJFY0, aJFY1,  aJFY7
 *      aVERP0, aVERP1, aSLPP, aLMS, aRMS, aTMS, aBMS, aCAM,
 *      aHTS, aVTS, aTBC0, aTBC3, aTBC1, aTBCALL, aTBSALL
 *
 *      special functions implemented:
 *      aRIN, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,aSLRM, aSTBM, aSFC
 *
 ************************************************************************/

char *CommandTable[]={
         "\375\033\015P\375",   /*reset*/
         "\377",        /*initialize*/  
         "\012",        /* lf                IND      ESCD */
         "\015\012",    /* return,lf         NEL      ESCE */
         "\033\012",    /* reverse lf        RI       ESCM */

			/*normal char set    SGR 0 */
         "\033R\033&",
         "\377",        /*italics on*/
         "\377",        /*italics off*/
         "\033E",       /*underline on*/
         "\033R",       /*underline off */
         "\033O",       /*boldface on*/
         "\033&",       /*boldface off*/
         "\377",        /* set foreground color */
         "\377",        /* set background color */

         "\033\037\015",/* normal pitch */
         "\033\037\013",/* elite on*/
         "\033\037\015",/* elite off*/
         "\033\037\011",/* condensed on*/
         "\033\037\015",/* condensed off*/
         "\377",        /* enlarged on*/
         "\377",        /* enlarged off*/

         "\033W",       /*shadow print on*/
         "\033&",       /*shadow print off*/
         "\033O",      /*doublestrike on*/
         "\033&",       /*doublestrike off*/
         "\377",        /* NLQ on*/
         "\377",        /* NLQ off*/

         "\377",        /*superscript on*/
         "\377",        /*superscript off*/
         "\377",        /*subscript on*/
         "\377",        /*subscript off*/
	 "\377",	/* normalize */
         "\033D",       /* partial line up   PLU      ESCL */
         "\033U",       /* partial line down PLD      ESCK */

         "\377",        /*US char set */
         "\377",        /*French char set*/
         "\377",        /*German char set*/
         "\377",        /*UK char set*/
         "\377",        /*Danish I char set*/
         "\377",        /*Sweden char set*/
         "\377",        /*Italian char set*/
         "\377",        /*Spanish char set*/
         "\377",        /*Japanese char set*/
         "\377",        /*Norweigen char set*/
         "\377",        /*Danish II char set*/
                              
         "\033P",       /*proportional on*/
         "\033Q",       /*proportional off*/
         "\033S",       /*proportional clear*/
         "\377",        /*set prop offset*/
         "\033M",       /*auto left justify on*/
         "\377",        /*auto right justify on*/
         "\377",        /*auto full justify on*/
         "\033X",       /*auto justify/center off*/
         "\377",        /*place holder */
         "\033=",       /*auto center on*/

         "\033\036\007",/* 1/8" line space*/
         "\033\036\011",/* 1/6" line spacing*/
         "\033\014",    /* set form length n */
         "\377",        /* perf skip n */
         "\377",        /* Perf skip off */
                        
         "\0339",       /* Left margin set */
         "\0330",       /* Right margin set */
         "\033T",       /* Top margin set */
         "\033L",       /* Bottom marg set */
         "\377",        /* T&B margin set   STBM      ESC[Pn1;Pn2r */
         "\377",        /* L&R margin set   SLRM      ESC[Pn1;Pn2s */
         "\033\011\001\0339\033\011\176\0330\033C\015",/* Clear margins */

         "\0331",       /* Set horiz tab */
         "\033-",       /* Set vertical tab */
         "\0338",       /* Clr horiz tab */
         "\0332",       /* Clear all h tabs */ 
         "\377",       /* Clear vertical tab */
         "\377",        /* Clr all v tabs    TBC 4 */
         "\0332",       /* Clr all h & v tabs */
			/* set default tabs */
	"\0332\033\011\011\0331\033\011\021\0331\033\011\031\0331\033\011\041\0331\033\011\051\0331\033\011\061\0331\033\011\071\0331\033\011\101\0331\033\011\111\0331\033\011\121\0331\033\011\131\0331",
	"\377"		/* extended commands */
};

char *ExtendedCharTable[] = {
" ",
"!",
"\\27Y",
"L",
"o",
"Y\\8-",
"|",
"S",
"\"",
"c", 
"a",
"`",
"\\27Z",
"-",
"r",
"-",
"*",
"+\\8_",
"\\27D2\\27U",
"\\27D3\\27U",
"'",
"u",
"P",
"\\27D.\\27U",
",",
"\\27D1\\27U",
"*\\8-",
"'",
"/",
"/",
"/",
"?",
"A",
"A",
"A",
"A",
"A",
"A",
"A",
"C",
"E",
"E",
"E",
"E",
"I",
"I",
"I",
"I",
"D\\8-",
"N",
"O",
"O",
"O",
"O",
"O",
"x",
"O\\8/",
"U",
"U",
"U",
"U",
"Y",
"T",
"3",
"a\\8`",
"a",
"a\\8^",
"a",
"a",
"a",
"a",
"c",
"e\\8`",
"e",
"e\\8^",
"e",
"i\\8`",
"i",
"i\\8^",
"i",
"d",
"n",
"o\\8`",
"o",
"o\\8^",
"o",
"o",
"/",
"o\\8/",
"u\\8`",
"u",
"u\\8^",
"u",
"y",
"t",
"y"
};
