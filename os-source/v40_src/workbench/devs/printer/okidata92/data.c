/****** printer.device/printers/Okidata 92 functions ********************
 *
 *   NAME
 *   OKIDATA U92 functions implemented: 
 *  
 *      aRIS, aIND, aNEL, aSGR0, aSGR4, aSGR24, aSGR1, aSGR22,
 *      aSHORP0, aSHORP1, aSHORP2, aSHORP3, aSHORP4, aSHORP5, aSHORP6,
 *      aDEN1, aDEN2, aDEN3, aDEN4,
 *      aSUS0, aSUS1, aSUS2, aSUS3, aSUS4,
 *      aFNT0, aFNT1, aFNT2, aFNT3, aFNT4, aFNT5, aFNT6, aFNT7, aFNT8
 *      aFNT9, aFNT10,
 *      aPROP1, aPROP2, aJFY6, aJFY0,
 *      aVERP0, aVERP1, aSLPP, aPERF, aPERF0,
 *      aTBC3, aTBC4, aTBCALL, aTBSALL
 *
 *      special functions implemented:
 *      aRIN, aSUS0, aSUS1, aSUS2, aSUS3, aSUS4, aCAM
 *      aPLU, aPLD, aVERP0, aVERP1, aSLRM, aIND
 *
 ************************************************************************/

char *CommandTable[] ={
	"\375\030\375",	/*reset              RIS      ESCc */
	"\377",	/*initialize*/
	"\377",	/* lf                IND      ESCD */
	"\015\012",	/* return,lf         NEL      ESCE */
	"\377",	/* reverse lf        RI       ESCM */
	
	"\033D\033I",	/*normal char set    SGR 0    ESC[0m */
	
	"\377",	/*italics on         SGR 3    ESC[3m */
	"\377",	/*italics off        SGR 23   ESC[23m */
	"\033C",	/*underline on       SGR 4    ESC[4m */
	"\033D",	/*underline off      SGR 24   ESC[24m */
	"\033T",	/*boldface on        SGR 1    ESC[1m */
	"\033I",	/*boldface off       SGR 22   ESC[22m */
	"\377",	/* set foreground color */
	"\377",	/* set background color */
	"\036",	/* normal char set   SHORP    ESC[0w */
	"\034",	/*elite on           SHORP    ESC[2w */
	"\036",	/*elite off          SHORP    ESC[1w */
	"\035",	/*condensed(fine) on SHORP    ESC[4w */
	"\036",	/*condensed off      SHORP    ESC[3w */
	"\037",	/*enlarged on        SHORP    ESC[6w */
	"\036",	/*enlarged off       SHORP    ESC[5w */
	
	"\377",	/*shadow print on    DEN6     ESC[6"z */
	"\377",	/*shadow print off   DEN5     ESC[5"z */
	"\033H",	/*doublestrike on    DEN4     ESC[4"z */
	"\033I",	/*doublestrike off   DEN3     ESC[3"z */
	"\0331",	/* NLQ on            DEN2     ESC[2"z */
	"\0330",	/* NLQ off 	     DEN1     ESC[1"z */
	
	"\033J",	/*superscript on              ESC[2u */
	"\033K",	/*superscript off	      ESC[1u */
	"\033L",	/*subscript on   	      ESC[4u */
	"\033M",	/*subscript off  	      ESC[3u */
	"\033K\033M",	/*normalize		      ESC[0u */
	"\377",	/* partial line up   PLU      ESCL */
	"\377",	/* partial line down PLD      ESCK */
	
	"\377",	/*US char set        FNT0     ESC(B */
	"\377",	/*French char set    FNT1     ESC(R */
	"\377",	/*German char set    FNT2     ESC(K */
	"\377",	/*UK char set        FNT3     ESC(A */
	"\377",	/*Danish I char set  FNT4     ESC E */
	"\377",	/*Sweden char set    FNT5     ESC(H */
	"\377",	/*Italian char set   FNT6     ESC(Y */
	"\377",	/*Spanish char set   FNT7     ESC(Z */
	"\377",	/*Japanese char set  FNT8     ESC(J */
	"\377",	/*Norweign char set  FNT9     ESC(6 */
	"\377",	/*Danish II char set FNT10    ESC(C */
	
	"\377",	/*proportional on    PROP     ESC[2p */
	"\377",	/*proportional off   PROP     ESC[1p */
	"\377",	/*proportional clear PROP     ESC[0p */
	"\377",	/*set prop offset    TSS */
	"\377",	/*auto left justify  JFY5     ESC[5 F */
	"\377",	/*auto right justify JFY7     ESC[7 F */
	"\377",	/*auto full justify  JFY6     ESC[6 F */
	"\377",	/*auto justify/center off   ESC[0 F */
	"\377",	/*place holder       JFY3     ESC[3 F */
	"\377",	/*auto center on     JFY2     ESC[2 F */
	
	"\0338",	/* 1/8" line space   VERP     ESC[0z */
	"\0336",	/* 1/6" line spacing VERP     ESC[1z */
	"\377",	/* set form length   SLPP     ESC[Pnt */
	"\377",	/* perf skip n 		      ESC[nq */
	"\377",	/* perf skip off              ESC[0q */
	
	"\377",	/* Left margin set  	      ESC[2x */
	"\377",	/* Right margin set   	      ESC[3x */
	"\377",	/* top margin set 	      ESC[4x */
	"\377",	/* Bottom marg set 	      ESC[5x */
	"\377",	/* T&B margin set   STBM      ESC[Pn1;Pn2r */
	"\377",	/* L&R margin set   SLRM      ESC[Pn1;Pn2s */
	"\377",	/* Clear margins 	      ESC[0x */
	
	"\377",	/* Set horiz tab     HTS      ESCH */
	"\377",	/* Set vertical tab  VTS      ESCJ */
	"\377",	/* Clr horiz tab     TBC 0    ESC[0g */
	"\033\011\015",	/* Clear all h tabs  TBC 3    ESC[3g */ 
	"\377",	/* Clr vertical tab  TBC 1    ESC[1g */
	"\377",	/* Clr all v tabs    TBC 4    ESC[4g */
	"\377",	/* Clr all h & v tabs  	      ESC#4 */	
	"\377",	/* set default tabs  	      ESC#5 */
	"\377"	/* entended command */
};
