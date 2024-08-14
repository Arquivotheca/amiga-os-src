/* brother printer command table */

/****** printer.device/printers/Brother_HL-15XL_functions ****************
 *
 *   NAME
 *   Brother_HR-15XL functions implemented: 
 *  
 *      aRIS, aIND, aNEL, aRI, aSGR0, aSGR4, aSGR24, aSGR1, aSGR22,
 *      aSHORP0, aSHORP1, aSHORP2, aSHORP3, aSHORP4,
 *      aDEN3, aDEN4, aDEN5, aDEN6,
 *      aPROP0, aPROP1, aPROP2, aTSS,
 *      aVERP0, aVERP1, aSLPP, aLMS, aRMS, aTMS, aBMS, aCAM,
 *      aHTS, aVTS, aTBC0, aTBC3, aTBC4, aTBCALL, aTBSALL
 *
 *      special functions implemented:
 *
 *      aRIN, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4, aPLU, aPLD, aSLRM, aSFC
 *
 ************************************************************************/

char *CommandTable[]={
         "\375\033\015P\375",   /* reset              RIS      ESCc */
         "\377",        /* initialize*/
         "\012",        /* lf                IND      ESCD */
         "\015\012",    /* return,lf         NEL      ESCE */
         "\033\012",    /* reverse lf        RI       ESCM */

         "\033R\033&",  /* normal char set    SGR 0    ESC[0m 	*/
         "\377",        /* italics on         SGR 3    ESC[3m 	NA */
         "\377",        /* italics off        SGR 23   ESC[23m	NA */
         "\033E",       /* underline on       SGR 4    ESC[4m 	*/
         "\033R",       /* underline off      SGR 24   ESC[24m 	*/
         "\033F",       /* boldface on        SGR 1    ESC[1m 	*/
         "\033&",       /* boldface off       SGR 22   ESC[22m 	*/
         "\377",        /* set foreground color 		special */
         "\377",        /* set background color 		NA */

         "\033\037\015",/* normal pitch      DECSHORP ESC[0w */
         "\033\037\013",/*elite on           DECSHORP ESC[2w */
         "\033\037\015",/*elite off          DECSHORP ESC[1w */
         "\033\037\011",/*condensed on       GSM (special) */
         "\033\037\015",/*condensed off      GSM (special) */
         "\377",        /*enlarged on        GSM (special) 	NA */
         "\377",        /*enlarged off       GSM (special) 	NA */


         "\033W",       /*shadow print on*/
         "\033&",       /*shadow print off*/
         "\033O",       /*doublestrike on*/
         "\033&",       /*doublestrike off*/
         "\377",        /* NLQ on				NA */
         "\377",        /* NLQ off				NA */

         "\377",       /*superscript on    		 	special */
         "\377",       /*superscript off    		 	special */
         "\377",       /*subscript on      			special */
         "\377",       /*subscript off     			special */
	 "\377",       /* normalize 				special */
         "\033D",      /* partial line up   PLU      ESCL */
         "\033U",      /* partial line down PLD      ESCK */

         "\377",        /*US char set                 ESC(B */
         "\377",        /*French char set             ESC(R */
         "\377",        /*German char set             ESC(K */
         "\377",        /*UK char set                 ESC(A */
         "\377",        /*Danish I char set           ESC E */
         "\377",        /*Sweden char set             ESC(H */
         "\377",        /*Italian char set   FNT 6 */
         "\377",        /*Spanish char set   FNT 7 */
         "\377",        /*Japanese char set  FNT 8 */
         "\377",        /*Norweigen char set     FNT 9 */
         "\377",        /*Danish II char set*/
                              
         "\033P",       /*proportional on */
         "\033Q",       /*proportional off*/
         "\033S",       /*proportional clear*/
         "\377",	/*set prop offset    TSS */
         "\377",        /*auto left justify  JFY 5 */
         "\377",        /*auto right justify JFY 7 */
         "\377",        /*auto full justify  JFY 3,6 */
         "\377",        /*auto justify/center off   JFY 0 */
         "\377",	/*place holder */
         "\377",        /*auto center on     JFY 1 */

         "\033\036\007",/* 1/8" line space   DECVERP  ESC[0z */
         "\033\036\011",/* 1/6" line spacing DECVERP  ESC[1z */
         "\033\014",    /* set form length   DECSLPP  ESC[Pnt */
         "\377",       /* perf skip n */
         "\377",       /* perf skip off */
                        
         "\0339",       /* Left margin set   DECSLRM  ESC[Pn1;Pn2s */
         " \0330",       /* Right margin set */
         "\033T",       /* Top margin set    DECSTBM  ESC[Pn1;Pn2r */
         "\033L",       /* Bottom marg set */
         "\377",        /* T&B margin set   STBM      ESC[Pn1;Pn2r */
         "\377",        /* L&R margin set   SLRM      ESC[Pn1;Pn2s */
       			/* Clear margins */
         "\033\011\001\0339\033\011\120\0330\033C\015",


         "\0331",      /* Set horiz tab     HTS      ESCH */
         "\033-",      /* Set vertical tab  VTS      ESCJ */
         "\0338",      /* Clr horiz tab     TBC 0    ESC0g */
         "\0332",      /* Clear all h tabs  TBC 3    ESC3g */ 
         "\377",       /* Clr vertical tab  TBC 1    ESC1g */
         "\377",       /* Clr all v tabs    TBC 4    ESC4g */
         "\0332",      /* Clr all h & v tabs */
			/* Set default tabs */
         "\0332\033\011\011\0331\033\011\021\0331\033\011\031\0331\033\011\041\0331\033\011\051\0331\033\011\061\0331\033\011\071\0331\033\011\101\0331\033\011\111\0331\033\011\121\0331\033\011\131\0331\015",
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
"~",
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
"A\\8`",
"A\\8\\27Z",
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
"a\\8\\27Z",
"a\\8^",
"a",
"a",
"a",
"a",
"c",
"e\\8`",
"e\\8\\27Z",
"e\\8^",
"e",
"i\\8`",
"i\\8\\27Z",
"i\\8^",
"i",
"d",
"n",
"o\\8`",
"o\\8\\27Z",
"o\\8^",
"o",
"o",
"/",
"o\\8/",
"u\\8`",
"u\\8\\27Z",
"u\\8^",
"u",
"y\\8\\27Z",
"t",
"y"
};


