/* alphacom 101 command table */

/****** printer.device/printers/Alphacom_Pro101_functions ****************
 *
 *   NAME
 *   Alphacom AlphaPro101 functions implemented: 
 *  
 *	aRIS, aIND, aNEL, aRI, aSGR0, aSGR4, aSGR24, aSGR1, aSGR22,
 *	aSHORP0, aSHORP1, aSHORP2, aSHORP3, aSHORP4,
 *	aDEN3, aDEN4, aDEN5, aDEN6,
 *	aVERP0, aVERP1, aSLPP, aLMS, aRMS, aTMS, aBMS
 *
 *      special functions implemented:
 *
 *      aRIN, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,
 *      aSLRM, aCAM, aLMS, aRMS, aVERP0, aVERP1
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

         "\033\037\014",/* normal pitch */
         "\033\037\012",/* elite on*/
         "\033\037\014",/* elite off*/
         "\033\037\010",/* condensed on*/
         "\033\037\014",/* condensed off*/
         "\377",        /* enlarged on*/
         "\377",        /* enlarged off*/

         "\033W",       /*shadow print on*/
         "\033&",       /*shadow print off*/
         "\033O",       /*doublestrike on*/
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
                              
         "\377",        /*proportional on*/
         "\377",        /*proportional off*/
         "\377",        /*proportional clear*/
         "\377",        /*set prop offset*/
         "\377",        /*auto left justify on*/
         "\377",        /*auto right justify on*/
         "\377",        /*auto full justify on*/
         "\377",       /*auto justify/center off*/
         "\377",        /*place holder */
         "\377",       /*auto center on*/

         "\033\036\007",/* 1/8" line space*/
         "\033\036\011",/* 1/6" line spacing*/
         "\033\014",    /* set form length n */
         "\377",        /* perf skip n */
         "\377",        /* Perf skip off */
                        
         "\0339",       /* Left margin set */
         " \0330",       /* Right margin set */
         "\033T",       /* Top margin set */
         "\033L",       /* Bottom marg set */
         "\377",        /* T&B margin set   STBM      ESC[Pn1;Pn2r */
         "\377",        /* L&R margin set   SLRM      ESC[Pn1;Pn2s */
         "\377",        /* Clear margins */

         "\377",	/* Set horiz tab */
         "\377",	/* Set vertical tab */
         "\377",	/* Clr horiz tab */
         "\377",	/* Clear all h tabs */ 
         "\377",	/* Clear vertical tab */
         "\377",        /* Clr all v tabs */
         "\377",        /* Clr all h & v tabs */
	 "\377",	/* set default tabs */
	 "\377"		/* extended commands */
};

char *ExtendedCharTable[] = {
" ",
"!",
"c\\8/",
"L",
"o",
"Y\\8-",
"!",
"{",
"\"",
"^", 
"a",
"'",
"~",
"-",
"\\\\",
"-",
"`",
"+\\8_",
"\\27D2\\27U",
"\\27D3\\27U",
"'",
"u",
"|",
"\\27D.\\27U",
",",
"\\27D1\\27U",
"`\\8-",
"'",
"<",
">",
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
"a",
"a",
"a",
"a",
"a",
"a",
"a\\8`",
"c",
"e",
"e",
"e",
"e",
"i",
"i",
"i",
"i",
"d",
"n",
"o",
"o",
"o",
"o",
"o",
"/",
"o\\8/",
"u",
"u",
"u",
"u",
"y",
"t",
"y"
};
