/****** printer.device/printers/generic_functions ************************
 *
 *   NAME
 *   generic functions implemented: none.
 *  
 ************************************************************************/

char *CommandTable[]={
	"\377",	/*reset*/
	"\377",	/*initialize*/  
	"\377",	/* lf                IND      ESCD */
	"\377",	/* return,lf         NEL      ESCE */
	"\377",	/* reverse lf        RI       ESCM */

	"\377",	/*normal char set    SGR 0 */
	"\377",	/*italics on*/
	"\377",	/*italics off*/
	"\377",	/*underline on*/
	"\377",	/*underline off */
	"\377",	/*boldface on*/
	"\377",	/*boldface off*/
	"\377",	/* set foreground color */
	"\377",	/* set background color */

	"\377",	/* normal pitch */
	"\377",	/* elite on*/
	"\377",	/* elite off*/
	"\377",	/* condensed on*/
	"\377",	/* condensed off*/
	"\377",	/* enlarged on*/
	"\377",	/* enlarged off*/

	"\377",	/*shadow print on*/
	"\377",	/*shadow print off*/
	"\377",	/*doublestrike on*/
	"\377",	/*doublestrike off*/
	"\377",	/* NLQ on*/
	"\377",	/* NLQ off*/

	"\377",	/*superscript on*/
	"\377",	/*superscript off*/
	"\377",	/*subscript on*/
	"\377",	/*subscript off*/
	"\377",	/* normalize */
	"\377",	/* partial line up   PLU      ESCL */
	"\377",	/* partial line down PLD      ESCK */

	"\377",	/*US char set */
	"\377",	/*French char set*/
	"\377",	/*German char set*/
	"\377",	/*UK char set*/
	"\377",	/*Danish I char set*/
	"\377",	/*Sweden char set*/
	"\377",	/*Italian char set*/
	"\377",	/*Spanish char set*/
	"\377",	/*Japanese char set*/
	"\377",	/*Norweigen char set*/
	"\377",	/*Danish II char set*/
                              
	"\377",	/*proportional on*/
	"\377",	/*proportional off*/
	"\377",	/*proportional clear*/
	"\377",	/*set prop offset*/
	"\377",	/*auto left justify on*/
	"\377",	/*auto right justify on*/
	"\377",	/*auto full justify on*/
	"\377",	/*auto justify/center off*/
	"\377",	/*place holder */
	"\377",	/*auto center on*/

	"\377",	/* 1/8" line space*/
	"\377",	/* 1/6" line spacing*/
	"\377",	/* set form length n */
	"\377",	/* perf skip n */
	"\377",	/* Perf skip off */
                        
	"\377",	/* Left margin set */
	"\377",	/* Right margin set */
	"\377",	/* Top margin set */
	"\377",	/* Bottom marg set */
	"\377",	/* T&B margin set   STBM      ESC[Pn1;Pn2r */
	"\377",	/* L&R margin set   SLRM      ESC[Pn1;Pn2s */
	"\377",	/* Clear margins */

	"\377",	/* Set horiz tab */
	"\377",	/* Set vertical tab */
	"\377",	/* Clr horiz tab */
	"\377",	/* Clear all h tabs */ 
	"\377",	/* Clear vertical tab */
	"\377",	/* Clr all v tabs */
	"\377",	/* Clr all h & v tabs */
	"\377",	/* set default tabs */
	"\377"	/* extended commands */
};

