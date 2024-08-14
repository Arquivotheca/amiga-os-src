#ifndef lint
static char     rcsid[]="$Id: comsub.c,v 3.4.1.1 91/06/25 21:07:11 katogi GM Locker: katogi $";
#endif
/*
*
*     comsub.c  :   sub functions for command
*
*        ifSprt()        : process support routine
*        wfIndexSearch() : data search sub routine
*        wfHomonym()     : homonym check routine
*        vfBufClr()      : buffer clear routine
*                YStrChk()               : yomi string check
*                YTypeChk()              : yomi type check
*        bfStrChk()      : string check routine
*        vfDataMove()    : data block move routine
*        vfSort()        : data sort routine
*        vfPrintCtrl()   : print control routine
*        vfPrintMrg()    : print main routine
*        vfPageHead()    : header print routine
*        vfPageFeed()    : page feed routine
*        vfPrintScript() : data print routine
*        vfInitHin()     : hinshi buffer clear routine
*        vfChkHin()      : hinshi check routine
*        vfAddHinType()  : hinshi type adding routine
*                ubDataMask()
*                vfGetIndex()
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*                Modified    by        T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include    <string.h>
#include    "egced.h"
#include    "crt.h"
#include    "keybrd.h"

#ifndef UNIX
/* #include    <jctype.h> */
#include    <conio.h>
#define     printf    cprintf
#endif

/*******************************************************************/
/*       Tables                                                    */
/*******************************************************************/
/*
**   character type table
*/
UBYTE           chrtbl[256] =
{
/*  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
    0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 1 */
    6, 1, 5, 1, 1, 1, 1, 1, 6, 5, 1, 1, 6, 1, 1, 1,     /* 2 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 3 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 4 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 5, 1, 1,     /* 5 */
    6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 6 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 5, 1, 6,     /* 7 */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     /* 8 */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     /* 9 */
    6, 6, 6, 5, 6, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,     /* a */
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* b */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* c */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,     /* d */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     /* e */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7      /* f */
};


FILE    *fp;        /* file pointer for print out    */

/*-----------------------------------------------*/
/*    command process support routine            */
/*-----------------------------------------------*/
int        ifSprt()
{
    static char    ubSprt[] = " ÄìséééµéðééóéééüH <Y/N> ";
    int     iAns;
    char    ubKey;
    
    BEEP();
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeRvsBox(8, 8, 42, 10, RED);
    CSRXY(10, 9); 
    printf(ubSprt);
    NORMCSR();
    do {
	vfGetKey(&ubKey);
    } while (ubKey != 'y' && ubKey != 'Y' && ubKey != '' &&
	     ubKey != 'n' && ubKey != 'N' && ubKey != '' &&
	     ubKey != ESC);
    /*---------- do it! ----------*/
    if (ubKey == 'y' || ubKey == 'Y' || ubKey == '')
	    iAns = YES;
    /*---------- stop ----------*/
    else    iAns = NO;
    return    iAns;
}

/*---------------------------------------------*/
/*    data sort routine                        */
/*---------------------------------------------*/
VOID    vfSort(sDAT)
    sDATASET    *sDAT;    /* pointer to data buffer   */
{
    REG     i, 
	    iPoint;       /* point of move            */
    int     iComp;        /* data compare result      */
    sREC48  sTemp;        /* temporary data buffer    */
    static char    ubMes[] = "òéæéªÆåéé. éééé¡é¿æéé¡éééó.";

    /*---------- message display ----------*/
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeRvsBox(7, 10, 51, 12, BLUE);
    CSRXY(9, 11); 
    printf(ubMes);
    NORMCSR();
    /*---------- get current data ----------*/
    for (i = 0; i < YOMISIZ; i++)
	sTemp.ubYomi[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
	sTemp.ubHinshi[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
	sTemp.ubKanji[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i];
    /*---------- delete current data ----------*/
    vfDataMove(sDAT, DELETE);

    /*---------- search adding point ----------*/

    for(iPoint = 0; iPoint < sDAT->wIndexMax-1; iPoint++){
	iComp = wfMemCmp(sTemp.ubYomi,
				 sDAT->sRecBuf[iPoint]->ubYomi, YOMISIZ);
	if (iComp < 0) break;
    }

    /*---------- set adding point ----------*/
    sDAT->wIndexNum = sDAT->wDispNum = iPoint;
    sDAT->iCsrY = 0;
    
	/*---------- if point is last number of index ----------*/

    if (iPoint == sDAT->wIndexMax-1 && iComp > 0){
		sDAT->wIndexMax++;
	sDAT->wIndexNum++;
	sDAT->wDispNum++;
    } else {

	vfDataMove(sDAT, ENTRY);

    }

    /*---------- add data ----------*/
    for (i = 0; i < YOMISIZ; i++)
	sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i] = sTemp.ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
	sDAT->sRecBuf[sDAT->wIndexNum]->ubHinshi[i] = sTemp.ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
	sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i] = sTemp.ubKanji[i];
}
/*------------------------------------------*/
/*      data block move routine                                 */
/*------------------------------------------*/
VOID    vfDataMove(sDAT, mode)
	sDATASET        *sDAT;          /*      pointer to data buffer          */
	int                     mode;           /*      move mode (ENTRY or DELETE)     */
{
	WORD    wMoveSet,               /*      starting index          */
			wMoveCnt;               /*      move count                      */
	REG             i;
	
	switch(mode){
		case    DELETE:
			wMoveSet = sDAT->IndexNum[sDAT->wIndexNum];
			wMoveCnt = sDAT->wIndexMax - wMoveSet;
			do {
				for(i = 0; i < YOMISIZ; i++)
				sDAT->sRecBuf[wMoveSet]->ubYomi[i]
					= sDAT->sRecBuf[wMoveSet+1]->ubYomi[i];
				for(i = 0; i < HINSIZ; i++)
				sDAT->sRecBuf[wMoveSet]->ubHinshi[i]
					= sDAT->sRecBuf[wMoveSet+1]->ubHinshi[i];
				for(i = 0; i < sDAT->iKnjLen; i++)
				sDAT->sRecBuf[wMoveSet]->ubKanji[i]
					= sDAT->sRecBuf[wMoveSet+1]->ubKanji[i];
				wMoveSet ++;
			} while (wMoveCnt--);
			sDAT->wIndexMax--;
			sDAT->IndexMax--;

			break;
		case    ENTRY:
			wMoveSet = sDAT->wIndexMax-1;
			wMoveCnt = wMoveSet - sDAT->wIndexNum;
			do {
				for(i = 0; i < YOMISIZ; i++)
				sDAT->sRecBuf[wMoveSet + 1]->ubYomi[i]
					= sDAT->sRecBuf[wMoveSet]->ubYomi[i];
				for(i = 0; i < HINSIZ; i++)
				sDAT->sRecBuf[wMoveSet + 1]->ubHinshi[i]
					= sDAT->sRecBuf[wMoveSet]->ubHinshi[i];
				for(i = 0; i < sDAT->iKnjLen; i++)
				sDAT->sRecBuf[wMoveSet + 1]->ubKanji[i]
					= sDAT->sRecBuf[wMoveSet]->ubKanji[i];
				wMoveSet--;
			} while (wMoveCnt--);
			sDAT->wIndexMax++;
			sDAT->IndexMax++;
			break;
	}
	
}

/*-----------------------------------------------*/
/*    data search sub routine                    */
/*-----------------------------------------------*/
WORD    wfIndexSearch(sDAT, ubTarget)
	const    sDATASET    *sDAT;
    UBYTE       *ubTarget;
{
    WORD    wTargetNum;     /* target index number       */
    int     iComp;          /* compare result            */
    BOOL    bResult;        /* check flag                */
	REG             n;

	n = sDAT->IndexMax;
	wTargetNum = 0;
	do {
	iComp = wfMemCmp(ubTarget, 
			sDAT->sRecBuf[sDAT->IndexNum[wTargetNum]]->ubYomi, YOMISIZ);

		if (! iComp){
			bResult = TRUE;
			break;
		} else{
			bResult = FALSE;
		}
		wTargetNum++;
	} while(n--);

	/*---------- no target ----------*/
	if (!bResult){
		return  NO_TARGET;
	/*---------- find target ---------*/
	} else{
		return  wTargetNum;
	}
}

/*------------------------------------------------------------------*/
/*  YStrChk : yomi string check                                     */
/*------------------------------------------------------------------*/
BOOL    YStrChk(yomip, yomil)
    UBYTE*  yomip;
    UWORD   yomil;
{
    register UWORD  i;
    register UWORD  ctype;

    for (i = 0; i < yomil; i ++, yomip ++) {
	ctype = YTypeChk(yomip);
	if ((ctype == ILLEGAL_YOMI) ||
	    (ctype == NEUTRAL_YOMI && i == 0))
	    return  FALSE;
    }
    return  TRUE;
}

/*------------------------------------------------------------------*/
/*  YTypeChk : Yomi Type Check                                      */
/*------------------------------------------------------------------*/
UWORD   YTypeChk(yomip)
    UBYTE*    yomip;
{
    register UBYTE  ch = chrtbl[*yomip];
	UWORD           yType = NORMAL_YOMI;

	if (ch == CHNORMAL) {
	if (*yomip < 0x7f || *yomip == 0xa5)
	    yType = EXTRA_YOMI;
	else
	    yType = (UBYTE)NORMAL_YOMI;
    } else if (ch == CHSUB || ch == CHKINSOK) {
	yType = NEUTRAL_YOMI;
    } else {
	yType = ILLEGAL_YOMI;
    }
    return  yType;
}

/*--------------------------------------------*/
/*    string check routine                    */
/*--------------------------------------------*/
BOOL    bfStrChk(str, iType, iRange)
    UBYTE    *str;         /* pointer to string                */
    int      iType;        /* string type (yomi & kanji)       */
    int      iRange;       /* range of string length           */
{
    REG     i;
    int     iLength, iRslt1,iRslt2;

	iLength = strlen(str);
	
#ifndef UNIX
if (iType == YOMI){
		if (iLength > YOMISIZ) return   TRUE;
		for (i = 0; i < iLength; i++){
			//iRslt1 = iskana(str[i]);
			iRslt1 = (str[i] < 0x21 || str[i] > 0xdf)? FALSE:TRUE;
			/*----- string is not kana -----*/
			if (!iRslt1){
				BEEP();
				return  TRUE;
			}
		}
		return  FALSE;
	} else {
		if (iLength > iRange)   return  TRUE;
		for (i = 0; i < iLength; i+=2){
			iRslt1 = iskanji(str[i]);
			iRslt2 = iskanji2(str[i+1]);
			/*----- string is not kanji -----*/
			if (!iRslt1 || !iRslt2){
				BEEP();
				return  TRUE;
			}
		}
		return  FALSE;
	}

#else
    iLength = strlen(str);
    if (iType == YOMI){
	if (iLength > YOMISIZ) return    TRUE;
	for (i = 0; i < iLength; i++){
	    iRslt1 = iskana(str[i]);
	    /*----- string is not kana -----*/
	    if (!iRslt1){
		BEEP();
		return    TRUE;
	    }
	}
	return    FALSE;
    } else {
	if (iLength > iRange)    return    TRUE;
	for (i = 0; i < iLength; i+=2){
	    iRslt1 = iskanji(str[i], str[i+1]);
	    /*----- string is not kanji -----*/
	    if (!iRslt1){
		BEEP();
		return    TRUE;
	    }
	}
	return    FALSE;
    }
#endif

}

/*-----------------------------------------------*/
/*    string buffer clear routine                */
/*-----------------------------------------------*/
VOID    vfBufClr(buf, n)
    UBYTE    *buf;        /* pointer to string buffer    */
    int      n;           /* size of buffer              */
{
    REG     i;
    for (i = 0; i < n; i++)    buf[i] = 0x00;
}

/*---------------------------------------------*/
/*    homonym check routine                    */
/*---------------------------------------------*/
WORD    wfHomonym(sDAT, wTop, wEnd)
    sDATASET    *sDAT;        /* pointer to data buffer              */
    WORD        *wTop;        /* buffer for top index of homonym     */
    WORD        *wEnd;        /* buffer for last index of homonym    */
{
    REG      i, j, k;
    int      iHomonym;
    UBYTE    dummy;
    static   char    ubErr[] = " ô»éôééæìéééé± ";

	i = sDAT->wIndexMax;
	j = k = 0;
	do {
		iHomonym = strncmp((char *)sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi,
						   (char *)sDAT->sRecBuf[j]->ubYomi, 
						   YOMISIZ);
		/*---------- top index find ----------*/
		if ((k == 0) && (iHomonym == 0)){
			*wTop = (WORD)j;
			k = 1;
		/*---------- last index find ----------*/
		} else if ((k == 1) && (iHomonym != 0)){
			*wEnd = (WORD)j;
			break;
		}
		j++;
	} while(i--);
	
    /*---------- no homonym ----------*/
    if (((*wEnd) - (*wTop)) == 1){
	BEEP();
#ifdef UNIX
	CSRRVS(); 
#endif
	vfMakeRvsBox(16, 10, 42, 12, RED);
	CSRXY(18, 11); 
	puts(ubErr);
	vfGetKey(&dummy);
	NORMCSR();
    }
    
    /*---------- return number of homonym ----------*/
    return    ((*wEnd) - (*wTop));
}

/*-----------------------------------------------*/
/*    data print out main routine                */
/*-----------------------------------------------*/
VOID    vfPrintMrg(sDAT, wStart, wEnd)
    sDATASET    *sDAT;       /* pointer to data buffer        */
    WORD        wStart;      /* starting index                */
    WORD        wEnd;        /* ending index                  */
{
    REG      i, j;
    UBYTE    ubHinType[60 + 1];  /* hinsi type buffer         */

    if ((FILE *)NULL == (fp = fopen("prn", "w")))    return;
    /*---------- header out ----------*/
    vfPageHead(sDAT, 1);

    for (i = wStart, j = 0; i < wEnd + 1; i++, j++){
	/*---------- page feed and header out every 50 data ----------*/
	if (j != 0 && !(j % 50)){
	    vfPageFeed();
	    vfPageHead(sDAT, j % 50 + 1);
	}
	/*---------- check hinshi and add hinshi type ----------*/
	vfInitHin(ubHinType);
	vfChkHin(sDAT->sRecBuf[i], ubHinType);
	/*---------- data print out ----------*/
	vfPrintScript(sDAT->sRecBuf[i], ubHinType, i + 1);
    }
    /*---------- finish printing ----------*/
    vfPageFeed();
    fclose(fp);
}

/*--------------------------------------------*/
/*    header print out routine                */
/*--------------------------------------------*/
VOID    vfPageHead(sDAT, iPage)
    sDATASET    *sDAT;
    int         iPage;
{
    static char    ubPage[] = 
	"    FILE : [%12s]%36sPage : %d\n\n\n\n";
    static char    ubMember[] = 
	"    öìå    ôé            èÄÜ                            òiÄî\n\n";

    fprintf(fp, ubPage, sDAT->ubFile, " ", iPage);
    fprintf(fp, ubMember);
}

/*---------------------------------------------*/
/*    page feed routine                        */
/*---------------------------------------------*/
VOID    vfPageFeed()
{
    fprintf(fp, "%c", PAGE);
}

/*----------------------------------------------*/
/*    data print out routine                    */
/*----------------------------------------------*/
VOID    vfPrintScript(dat, hin, num)
    sREC48  *dat;
    UBYTE   *hin;
    WORD    num;
{
    REG     i, j;
    UBYTE    ubBuf[YOMISIZ + 1];
 
    for (i = 0; i < YOMISIZ; i++)
	ubBuf[i] = dat->ubYomi[i];
    ubBuf[i] = 0x00;
    /*---------- data rprint out ----------*/
    fprintf(fp, "    %4d    %-12s    %-32s", num, ubBuf, dat->ubKanji);
    /*---------- hinshi type print out ----------*/
    i = 0; while (hin[i++]);
    for (j = 1; j < i; j++){
	fprintf(fp, "%c", hin[j - 1]);
	/*---------- arrange hinshi type ----------*/
	if (!(j % 15) && hin[j]){
	    fprintf(fp, "\n%62s"," ");
	} 
    }
    fprintf(fp, "\n");

}

/*--------------------------------------------*/
/*    hinshi type buffer clear routine        */
/*--------------------------------------------*/
VOID    vfInitHin(hin)
    UBYTE    *hin;        /* pointer to hinshi type buffer    */
{
    REG        i;
    for (i = 0; i < 60 + 1; i++)
	hin[i] = 0x00;
}

/*----------------------------------------------------*/
/*    check hinshi and add hinshi type routine        */
/*----------------------------------------------------*/
VOID    vfChkHin(dat, hin)
    sREC48   *dat;
    UBYTE    *hin;
{

    /*---------- tankanji ----------*/
    if((dat->ubHinshi[0] == 0x00)
	&& (dat->ubHinshi[1] == 0x00)
	&& (dat->ubHinshi[2] == 0x00)
	&& (dat->ubHinshi[3] == 0x00))
	    vfAddHinType(hin, 0x92,0x50, 0x8a,0xbf, 0x20);
    /*---------- meishi 1 ----------*/
    if (dat->ubHinshi[0] & 0x40)
	vfAddHinType(hin, 0x96,0xbc, 0x82,0x50, 0x20);
    /*---------- jinmeii 1 ----------*/
    if (dat->ubHinshi[0] & 0x20)
	vfAddHinType(hin, 0x90,0x6c, 0x82,0x50, 0x20);
    /*---------- chimei 1 ----------*/
    if (dat->ubHinshi[0] & 0x10)
	vfAddHinType(hin, 0x92,0x6e, 0x82,0x50, 0x20);
    /*---------- meishi 2 ----------*/
    if (dat->ubHinshi[0] & 0x08)
	vfAddHinType(hin, 0x96,0xbc, 0x82,0x51, 0x20);
    /*---------- jinmeii 2 ----------*/
    if (dat->ubHinshi[0] & 0x04)
	vfAddHinType(hin, 0x90,0x6c, 0x82,0x51, 0x20);
    /*---------- chimei 2 ----------*/
    if (dat->ubHinshi[0] & 0x02)
	vfAddHinType(hin, 0x92,0x6e, 0x82,0x51, 0x20);
    /*---------- suushi ----------*/
    if (dat->ubHinshi[0] & 0x01)
	vfAddHinType(hin, 0x90,0x94, 0x8e,0x8c, 0x20);
    /*---------- settoushi ----------*/
    if (dat->ubHinshi[1] & 0x80)
	vfAddHinType(hin, 0x90,0xda, 0x93,0xaa, 0x20);
    /*---------- setubishi ----------*/
    if (dat->ubHinshi[1] & 0x40)
	vfAddHinType(hin, 0x90,0xda, 0x94,0xf6, 0x20);
    /*---------- jyosuushi----------*/
    if (dat->ubHinshi[1] & 0x20)
	vfAddHinType(hin, 0x8f,0x95, 0x90,0x94, 0x20);
    /*---------- suuji----------*/
    if (dat->ubHinshi[1] & 0x10)
	vfAddHinType(hin, 0x90,0x94, 0x8e,0x9a, 0x20);
    /*---------- fukushi ----------*/
    if (dat->ubHinshi[1] & 0x02)
	vfAddHinType(hin, 0x95,0x9b, 0x8e,0x8c, 0x20);
    /*---------- setuzokushi ----------*/
    if (dat->ubHinshi[1] & 0x01)
	vfAddHinType(hin, 0x90,0xda, 0x91,0xb1, 0x20);
    /*---------- rentaishi ----------*/
    if (dat->ubHinshi[2] & 0x80)
	vfAddHinType(hin, 0x98,0x41, 0x91,0xcc, 0x20);
    /*---------- keiyoushi ----------*/
    if (dat->ubHinshi[2] & 0x40)
	vfAddHinType(hin, 0x8c,0x60, 0x97,0x65, 0x20);
    /*---------- keiyoudoushi ----------*/
    if (dat->ubHinshi[2] & 0x20)
	vfAddHinType(hin, 0x8c,0x60, 0x93,0xae, 0x20);
    /*---------- sahendoushi ----------*/
    if (dat->ubHinshi[2] & 0x08)
	vfAddHinType(hin, 0x83,0x54, 0x95,0xcf, 0x20);
    /*---------- zahendoushi ----------*/
    if (dat->ubHinshi[2] & 0x04)
	vfAddHinType(hin, 0x83,0x55, 0x95,0xcf, 0x20);
    /*---------- itidandoushi ----------*/
    if (dat->ubHinshi[2] & 0x02)
	vfAddHinType(hin, 0x82,0x50, 0x92,0x69, 0x20);
    /*---------- kagyo5dan ----------*/
    if (dat->ubHinshi[2] & 0x01)
	vfAddHinType(hin, 0x83,0x4a, 0x8d,0x73, 0x20);
    /*---------- gagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x80)
	vfAddHinType(hin, 0x83,0x4b, 0x8d,0x73, 0x20);
    /*---------- sagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x40)
	vfAddHinType(hin, 0x83,0x54, 0x8d,0x73, 0x20);
    /*---------- tagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x20)
	vfAddHinType(hin, 0x83,0x5e, 0x8d,0x73, 0x20);
    /*---------- nagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x10)
	vfAddHinType(hin, 0x83,0x69, 0x8d,0x73, 0x20);
    /*---------- bagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x08)
	vfAddHinType(hin, 0x83,0x6f, 0x8d,0x73, 0x20);
    /*---------- magyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x04)
	vfAddHinType(hin, 0x83,0x7d, 0x8d,0x73, 0x20);
    /*---------- ragyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x02)
	vfAddHinType(hin, 0x83,0x89, 0x8d,0x73, 0x20);
    /*---------- wagyo5dan ----------*/
    if (dat->ubHinshi[3] & 0x01)
	vfAddHinType(hin, 0x83,0x8f, 0x8d,0x73, 0x20);
    /*---------- bunruifuka ----------*/
    if (dat->ubHinshi[2] & 0x10)
	vfAddHinType(hin, 0x95,0xaa, 0x95,0x73, 0x20);
}

/*-----------------------------------------------*/
/*    add hinshi type routine                    */
/*-----------------------------------------------*/
VOID    vfAddHinType(hin, ubType0, ubType1, ubType2, ubType3, ubType4)
    UBYTE    *hin;
    UBYTE    ubType0;
    UBYTE    ubType1;
    UBYTE    ubType2;
    UBYTE    ubType3;
    UBYTE    ubType4;
{
    REG        iCount;
    
    iCount = 0;
    while (hin[iCount])    iCount++;
    hin[iCount + 0] = ubType0;
    hin[iCount + 1] = ubType1;
    hin[iCount + 2] = ubType2;
    hin[iCount + 3] = ubType3;
    hin[iCount + 4] = ubType4;
}

/*---------------------------------------------*/
/*    read only message display routine        */
/*---------------------------------------------*/
VOID    vfDispReadOnly()
{
    UBYTE          dummy;
    static BYTE    ubErr[] = " âtâ@âCâïéòæééé½ééé± ";
    
    BEEP();
    vfMakeRvsBox(16, 10, 46, 12, RED);
    CSRXY(18, 11);
#ifdef UNIX
    CSRRVS();
#endif
    puts(ubErr);
    vfGetKey(&dummy);
    NORMCSR();
}

/*------------------------------------------*/
/*      data masking routine                                    */
/*------------------------------------------*/
UBYTE   ubDataMask(ubNum)
	BYTE            *ubNum;                         /*      masking hinshi data number              */
{
	REG             i, iCsr, iTop, iSelect;
	UBYTE   ubKey;
	UBYTE   ubMode;
	static  char    ubMask[] = " â}âXâN ";
	static  char    *ubMaskMenu[] = {
		"   ë  Å£   ",
		"  û Äî éP  ","  Él û É  ",
		"  Æn û Äî  ","  û Äî éQ  ",
		"  Él û û  ","  îº ïµ û  ",
		"   Éö  Äî   ","  É ô¬ Äî  ",
		"  É ö÷ Äî  ","  Åò Éö Äî  ",
		"   ò¢  Äî   ","  É æ Äî  ",
		"  ÿA æ Äî  ","  î` ùe Äî  ",
		"  î`ùeô«Äî  ","  âTòô«Äî  ",
		"  âUòô«Äî  ","  êÆiô«Äî  ",
		"  âJìséTÆi  ","  âKìséTÆi  ",
		"  âTìséTÆi  ","  â^ìséTÆi  ",
		"  âiìséTÆi  ","  âoìséTÆi  ",
		"  â}ìséTÆi  ","  âëìséTÆi  ",
		"  âÅìséTÆi  ","  ò¬ùòsë  ",
	};
	
	vfMakeBox(31, 4, 45, 16);
#ifndef     UNIX
	CSR(7;32);
#endif
	CSRXY(35, 4); puts(ubMask);
	
	/*---------- get select mode ----------*/
	iTop = iCsr = iSelect = 0;
	do {
#ifndef     UNIX
		CSR(0);
#endif
	NORMCSR();
	CSROFF();
	
		for (i = 0; i < 11; i++){
			CSRXY(33, 5 + i); 
			puts(ubMaskMenu[iTop + i]);
		}
		CSRXY(33, 5 + iCsr);
#ifndef     UNIX
		CSR(7);
#endif
	CSRRVS();
	
		puts(ubMaskMenu[iTop + iCsr]);
		vfGetKey(&ubKey);
		if (ubKey == ESC){
			return  0x00;
		}
		if (ubKey == DWN){
			iSelect++;
			iCsr++;
			if (iSelect > 28)       iSelect = 28; 
			if (iCsr > 10){
				iCsr = 10;
				iTop ++;
				if (iTop >  18){
					BEEP();
					iTop = 18;
				}
			}
		}
		if (ubKey == UP){
			iSelect--;
			iCsr--;
			if (iSelect <= 0)       iSelect = 0; 
			if (iCsr < 0){
				iCsr = 0;
				iTop --;
				if (iTop < 0){
					BEEP();
					iTop = 0;
				}
			}
		}
	} while(ubKey != RET);
	
	switch(iSelect){
		case    NORMAL:         *ubNum = -1; ubMode =  0x00; break;     /*      0       */
		case    MEISI1:         *ubNum = 0; ubMode = 0x40; break;       /*      1       */
		case    JINMEI1:        *ubNum = 0; ubMode = 0x20; break;       /*      2       */
		case    CHIMEI1:        *ubNum = 0; ubMode = 0x10; break;       /*      3       */
		case    MEISI2:         *ubNum = 0; ubMode = 0x08; break;       /*      4       */
		case    JINMEI2:        *ubNum = 0; ubMode = 0x04; break;       /*      5       */
		case    CHIMEI2:        *ubNum = 0; ubMode = 0x02; break;       /*      6       */
		case    SUUSI:          *ubNum = 0; ubMode = 0x01; break;       /*      7       */
		case    SETTOU:         *ubNum = 1; ubMode = 0x80; break;       /*      8       */
		case    SETUBI:         *ubNum = 1; ubMode = 0x40; break;       /*      9       */
		case    JYOSUU:         *ubNum = 1; ubMode = 0x20; break;       /*      10      */
		case    FUKUSI:         *ubNum = 1; ubMode = 0x02; break;       /*      11      */
		case    SETUZOKU:       *ubNum = 1; ubMode = 0x01; break;       /*      12      */
		case    RENTAI:         *ubNum = 2; ubMode = 0x80; break;       /*      13      */
		case    KEIYOU:         *ubNum = 2; ubMode = 0x40; break;       /*      14      */
		case    KEIDOU:         *ubNum = 2; ubMode = 0x20; break;       /*      15      */
		case    SAHEN:          *ubNum = 2; ubMode = 0x08; break;       /*      16      */
		case    ZAHEN:          *ubNum = 2; ubMode = 0x04; break;       /*      17      */
		case    ICHIDAN:        *ubNum = 2; ubMode = 0x02; break;       /*      18      */
		case    KAGYO:          *ubNum = 2; ubMode = 0x01; break;       /*      19      */
		case    GAGYO:          *ubNum = 3; ubMode = 0x80; break;       /*      20      */
		case    SAGYO:          *ubNum = 3; ubMode = 0x40; break;       /*      21      */
		case    TAGYO:          *ubNum = 3; ubMode = 0x20; break;       /*      22      */
		case    NAGYO:          *ubNum = 3; ubMode = 0x10; break;       /*      23      */
		case    BAGYO:          *ubNum = 3; ubMode = 0x08; break;       /*      24      */
		case    MAGYO:          *ubNum = 3; ubMode = 0x04; break;       /*      25      */
		case    RAGYO:          *ubNum = 3; ubMode = 0x02; break;       /*      26      */
		case    WAGYO:          *ubNum = 3; ubMode = 0x01; break;       /*      27      */
		case    BUNFU:          *ubNum = 2; ubMode = 0x10; break;       /*      28      */
	}

	
	return  ubMode;
}

/*---------------------------------------------*/
/*              get masking data index                 */
/*---------------------------------------------*/
VOID    vfGetIndex(sDAT)
	sDATASET        *sDAT;
{
	static  char    ubEmsg[] = "  èYôûâfü[â^éæìéééé±. ";
			UBYTE   ubKey;
			REG             i,j;

	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		for( i=0, j=0; i < sDAT->wIndexMax;i++) {
			if(CHKMASK(sDAT->ubMask[0],
				sDAT->sRecBuf[i]->ubHinshi[sDAT->ubMask[1]])) {
				sDAT->IndexNum[j] = i;
				j++;
			}
		}
		if( j == 0 ){
			BEEP();
			vfMakeRvsBox(10, 10, 40, 12, RED);
			CSRXY(12, 11); 
			puts(ubEmsg);
			vfGetKey(&ubKey);
		    for( i = 0; i<sDAT->wIndexMax; i++) 
			sDAT->IndexNum[i] = i;
			sDAT->ubMask[1] = -1;
			sDAT->ubMask[0] = 0x00;
		    sDAT->IndexMax = sDAT->wIndexMax;
			return;
		} else {
			sDAT->IndexMax = j;
		}
	} else {
		for( i=0; i<sDAT->wIndexMax;i++) {
			sDAT->IndexNum[i] = i;
		}
		sDAT->IndexMax = sDAT->wIndexMax;
	}
}

/*---------------------------------------------*/
/*    memory compare                           */
/*---------------------------------------------*/
WORD    wfMemCmp(mem1, mem2, cnt)
    UBYTE    *mem1;
    UBYTE    *mem2;
    WORD      cnt;
{
    REG     i;

    for (i = 0; i < cnt; i++) {
	if (*mem1 > *mem2) return    1;
	if (*mem1 < *mem2) return   -1;
	mem1++; mem2++;
    }
    return    0;
}

#ifndef        UNIX
#define     Int23Nbr    0x23
BOOL        SetINT23 = FALSE;
void(interrupt  far *A_INT23)();

void(interrupt  far StopKey)()
{
}

VOID    SetStopKey(VOID)
{
    if (!SetINT23) {
	A_INT23 = _dos_getvect(Int23Nbr);
	_dos_setvect(Int23Nbr, StopKey);
	SetINT23 = TRUE;
    }
}

VOID    RcvStopKey(VOID)
{
    if (SetINT23) {
	_dos_setvect(Int23Nbr, A_INT23);
	SetINT23 = FALSE;
    }
}
#endif

/*------------------------------- end of comsub.c ---------------------------*/
