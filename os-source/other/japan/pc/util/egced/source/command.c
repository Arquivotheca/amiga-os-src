#ifndef lint
static char     rcsid[]="$Id: command.c,v 40.0 93/06/08 11:00:04 brummer Exp $";
#endif
/*
*
*     command.c  :   command functions
*
*            vfFind()       : data search main routine
*            vfModify()     : data modify routine
*            vfDelete()     : data delete routine
*            vfInsert()     : data move routine
*            vfEntry()      : data entry routine
*            vfChange()     : data swap routine
*            ifQuit()       : quit process routine
*            vfHinshiEdit() : hinshi edit routine
*            vfPrintCtrl()  : print control routine
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by 	   T.Koide	 1992.09.14
*
*/

#include    <stdio.h>
#include    <string.h>
#include    "egced.h"
#include    "crt.h"
#include    "keybrd.h"

#ifndef UNIX
#include    <conio.h>
#define     puts      cputs
#define     printf    cprintf
#endif

/*----------------------------------------------*/
/*    data find main routine                    */
/*----------------------------------------------*/
VOID    vfFind(sDAT)
    sDATASET    *sDAT;   /* pointer to data buffer        */
{
    /*    messages    */
    static char    ubFind[]  = " 検  索 ";
    static char    ubErace[] = "                    ";
    static char    *ubMes[]  = {
        "↓  検索対象入力    ",
        "みつかりません. 何か",
        "キーを押してください",
    };
    UBYTE    ubTarget[80],    /* target string buffer        */
             dummy;           /* dummy                       */
    int      iComp;           /* compare result              */
    WORD     wIndex;          /* index number of target      */
    BOOL     bFind;           /* flag                        */


    /*---------- show message ----------*/
    vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubFind);
    NORMCSR();
    CSRXY(57, 8); puts(ubMes[0]);
    do {
        bFind = FALSE;
        BEEP();
        /*---------- get target yomi ---------*/
        CSRXY(57, 9); puts(ubErace);
        CSRXY(57, 9); 
        CSRON();
        vfGetString(ubTarget, YOMISIZ);
        CSROFF();
        /*----- too large string -----*/
        if (strlen(ubTarget) > YOMISIZ){
            bFind = TRUE;
            /*----- too large string, over flame -----*/
            if (strlen(ubTarget) > 20){
                CLRSCR();
                vfSetMonitor();
                vfDisplay(sDAT, NO_CSR_MOVE);
				vfBoxDisplay(sDAT);
                vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
                CSR(32);
#endif
                CSRXY(63, 7); puts(ubFind);
                NORMCSR();
                CSRXY(57, 8); puts(ubMes[0]);
            }
        }
        /*----- if string length = 0 stop find process -----*/
        else if (strlen(ubTarget) < 1){
            vfRemoveBox();
            return;
        }
        /*----- check string type -----*/
        else {
          bFind = bfStrChk(ubTarget, YOMI, YOMISIZ);
          bFind = (sDAT->sRecBuf[sDAT->IndexNum[0]]->ubYomi[0] != ubTarget[0]);
        }
    } while (bFind); /*----- end of do~while -----*/

    /*---------- get index number of target ----------*/
    wIndex = wfIndexSearch(sDAT, ubTarget);

    /*---------- no target ----------*/
    if (wIndex == NO_TARGET){

        /*----- search near point -----*/
        for(wIndex = 0; wIndex < sDAT->IndexMax-1; wIndex++){
            iComp = wfMemCmp(ubTarget, 
                     sDAT->sRecBuf[sDAT->IndexNum[wIndex]]->ubYomi, YOMISIZ);
            if (iComp < 0) break;
        }
        /*----- show error message -----*/
        BEEP();
        CSRXY(57, 8);puts(ubMes[1]);
        CSRXY(57, 9);puts(ubMes[2]);
        vfGetKey(&dummy);
        vfRemoveBox();
        /*----- set near point -----*/
        sDAT->wIndexNum = wIndex;
        sDAT->wDispNum = wIndex;
        sDAT->iCsrY = 0;
    } 
    /*---------- find target ----------*/
    else {
        /*----- set target index -----*/
        vfRemoveBox();
        sDAT->wIndexNum = wIndex;
        sDAT->wDispNum = wIndex;
        sDAT->iCsrY = 0;
    }
}

/*-----------------------------------------------*/
/*    data delete routine                        */
/*-----------------------------------------------*/
VOID    vfDelete(sDAT)
    sDATASET    *sDAT;    /* pointer to data buffer    */
{
    /*    messages    */
    static char     ubDel[]  = " 削  除 ";
    static char     ubMes[]  = "↓  削除対象 <Y / N>";
    static char     ubPrcs[] = "削除中です. しばらくお待ちください.";
    static char     ubErr[]  = "これ以上は削除できません.";
           char     ubAnswer;
           UBYTE    dummy;        /* support flag             */
           WORD     wNumber;      /* delete data index        */
		   sREC48  *Hndl;
		   UBYTE   kanjibuf[66];


    /*---------- data index setting ----------*/
    wNumber = sDAT->wDispNum + sDAT->iCsrY;

	/*---------- no more delete ----------*/
	if (sDAT->IndexMax <= 1){
#ifdef UNIX
        CSRRVS();
#endif
		vfMakeRvsBox(10, 10, 38, 12, RED);
		CSRXY(12, 11); puts(ubErr);
		vfGetKey(&dummy);
		return;
	}

    /*---------- target display ----------*/
    BEEP(); 
	Hndl = sDAT->sRecBuf[sDAT->IndexNum[wNumber]];
    CSRXY(3, 3 + sDAT->iCsrY);
    printf("%4d   ", sDAT->wIndexNum + 1);
    CSRXY(8, 3 + sDAT->iCsrY);
    printf("%-12s", Hndl->ubYomi);
    CSRXY(20, 3 + sDAT->iCsrY);
    printf(" ");
    CSRXY(21, 3 + sDAT->iCsrY);
	memcpy(kanjibuf,Hndl->ubKanji,32);
	kanjibuf[32] = 0x00;
	printf("%-32s", kanjibuf);

    /*---------- message display ----------*/ 
    vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubDel);
    NORMCSR();
    CSRXY(57, 8); puts(ubMes);
    CSRXY(57, 9); printf("%-12s", Hndl->ubYomi);
    CSRXY(69, 9); puts(" ");
    /*---------- support ----------*/
    do {
#ifdef		UNIX
		CSRXY(70, 9);
#endif
        CSRON();
        vfGetKey(&ubAnswer);
    } while (!(ubAnswer == 'y' || ubAnswer == 'Y' || ubAnswer == 'ﾝ'
             || ubAnswer == 'n' || ubAnswer == 'N' || ubAnswer == 'ﾐ'
             || ubAnswer == ESC));
    CSROFF();
    switch (ubAnswer){
        /*---------- do it! ----------*/
        case   'y': case    'Y': case    'ﾝ':
            /*----- show message -----*/
#ifdef UNIX
            CSRRVS();
#endif
            vfMakeRvsBox(10, 10, 48, 12, YELLOW);
            CSRXY(12, 11); 
            puts(ubPrcs);
            NORMCSR();
            vfDataMove(sDAT, DELETE);
			vfGetIndex(sDAT);

            /*----- processing -----*/
            if (sDAT->wIndexNum == sDAT->IndexMax){
                sDAT->iCsrY--;
                if (sDAT->iCsrY < 0) sDAT->iCsrY = 0;
                sDAT->wIndexNum--;
                if (sDAT->wIndexNum < 0) sDAT->wIndexNum = 0;
                if (sDAT->wDispNum == sDAT->IndexMax){
                    sDAT->wDispNum--;
                    if (sDAT->wDispNum < 0) sDAT->wDispNum = 0;
                }
            }
            vfRemoveBox();
            break;
        default    :
            /*---------- stop delete process ----------*/
            vfRemoveBox();
            break;
    }
}

/*---------------------------------------------*/
/*    data move routine                        */
/*---------------------------------------------*/
VOID    vfInsert(sDAT)
    sDATASET    *sDAT;     /* pointer to data buffer        */
{
    /*    messages    */
	static char	   ubEmsg[] = "マスク中は実行することはできません. ";
    static char    ubInst[] = " 挿  入 ";
    static char    ubPrcs[] = "移動中です. しばらくお待ちください.";
    static char    *ubMes[] = {
        "同じ読みの範囲内で移", 
        "動して挿入先を指定し", 
        "てください.",
    };
    
    sREC48    sTemp;              /* temporary data buffer     */
    UBYTE     ubKey;              /* key code (ascii)          */
    int       iInst,              /* support flag              */
              iPoint1,            /* base data index           */
              iPoint2;            /* insert point              */
    WORD      wTop, wEnd,         /* homonym top & end index   */
              wHomonym;           /* number of homonym         */
    BOOL      bInst;              /* insert flag               */
    REG       i;
    int			VertHinFlag;
	char		yomibuf[13];
	char		kanjibuf[66];

	/*----------------- mask check -----------------*/
	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); 
		puts(ubEmsg);
		vfGetKey(&ubKey);
		return;
	}

	VertHinFlag = sDAT->VertHin;
	sDAT->VertHin = NO;

    /*----------    message dispaly    ----------*/
    BEEP();
    vfMakeBox(55, 7, 77, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubInst);
    NORMCSR();
    for (i = 0; i < 3; i++){
        CSRXY(57, 8 + i); 
        puts(ubMes[i]);
    }
    /*----------    get base data    ----------*/
    for (i = 0; i < YOMISIZ; i++)
        sTemp.ubYomi[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
        sTemp.ubHinshi[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
        sTemp.ubKanji[i] = sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i];

    iPoint1 = sDAT->wIndexNum;

    /*----------    homonym check    ----------*/
    wHomonym = wfHomonym(sDAT, &wTop, &wEnd);
    if (wHomonym == 1){
        vfRemoveBox();
        return;
    }

	memset(kanjibuf,0x00,66);
	memcpy(yomibuf,sDAT->sRecBuf[iPoint1]->ubYomi,12);
	yomibuf[12] = 0x00;
	memcpy(kanjibuf,sDAT->sRecBuf[iPoint1]->ubKanji,64);
	if (strlen(kanjibuf) > 44){
		kanjibuf[42] = '.';
		kanjibuf[43] = '.';
		kanjibuf[44] = 0x00;
	}
	CSRXY(3,18);
	printf("  挿入データ %4d %-12s %-44s", iPoint1 + 1,yomibuf,kanjibuf);

    /*----- get insert point -----*/
    do {
        bInst = TRUE;

        vfGetKey(&ubKey);
        vfCsrMove(sDAT, ubKey);
        vfDisplay(sDAT, ubKey);
        if (ubKey == RET){
            if (sDAT->wIndexNum >= wTop && sDAT->wIndexNum < wEnd)
                bInst = FALSE;
            else {
                BEEP();
                bInst = TRUE;
            }
        } 
#ifndef UNIX
        else if (ubKey == ESC){
            NORMCSR();
            vfRemoveBox();
            return;
        }
#endif

    } while(bInst);

	sDAT->VertHin = VertHinFlag;

	/*---------- set jump point ----------*/
	iPoint2 = sDAT->wIndexNum;

	memset(kanjibuf,0x00,66);
	memcpy(kanjibuf,sDAT->sRecBuf[iPoint2]->ubKanji,64);
	if (strlen(kanjibuf) > 44){
		kanjibuf[42] = '.';
		kanjibuf[43] = '.';
		kanjibuf[44] = 0x00;
	}
	CSRXY(3,18);
	printf("   挿入先 :  %4d %-12s %-44s", iPoint2 + 1,yomibuf,kanjibuf);
    
    /*----------    support    process ----------*/
    iInst = ifSprt();
    /*----------    stop insert process    ----------*/
    if (iInst == NO){
        vfRemoveBox();
        return;
    }
    /*---------- show message ----------*/
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeRvsBox(10, 10, 48, 12, YELLOW);
    CSRXY(12, 11); 
    puts(ubPrcs);
    NORMCSR();

    /*---------- set delete point ----------*/
    sDAT->wIndexNum = iPoint1;

    /*---------- delete base data ----------*/
    vfDataMove(sDAT, DELETE);

    /*---------- set new index ----------*/
    sDAT->wIndexNum = iPoint2;

    /*---------- if new index is not last index number ----------*/
    if (sDAT->wIndexNum != sDAT->wIndexMax)
        vfDataMove(sDAT, ENTRY);
    /*--------- else ---------*/
    else
        sDAT->wIndexMax++;
    /*--------- data copy ----------*/
    for (i = 0; i < YOMISIZ; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i] = sTemp.ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubHinshi[i] = sTemp.ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i] = sTemp.ubKanji[i];
    /*----------    finish processing    ----------*/
    vfRemoveBox();
}

/*-----------------------------------------------*/
/*    data modify routine                        */
/*-----------------------------------------------*/
VOID    vfModify(sDAT)
    sDATASET    *sDAT;     /* pointer to data buffer */
{
    /*    messages    */
	static char	   ubEmsg[] = "マスク中は実行することはできません. ";
    static char    ubMod[]   = " 修  正 ";
    static char    ubErace[] = "             ";
    static char    *ubMes[]  = {
        "←  修正してください",
        "読み：半角12文字以内",
        "漢字：全角 8文字以内",
        "漢字：全角15文字以内",
		"漢字：全角32文字以内",

    };
    
    REG     		i;
    BOOL    		bModify;        /* flag                     */
	unsigned int    head,           /* minimum string length    */
            		tail;           /* maximum string length    */
    int        		iModify;        /* support flag             */
    UBYTE   		ubNewDat[80];   /* string buffer            */
	UBYTE   		dummy;


	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); puts(ubEmsg);
		vfGetKey(&dummy);
		return;
	}

    /*---------- yomi or kanji ? ----------*/
    if (sDAT->iCsrX == 0){
        head = 1; tail = 13;
    } else {
        head = 1; tail = 65;
    }
    /*---------- show message ----------*/
    BEEP();
    vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubMod);
    NORMCSR();
    CSRXY(57, 8); puts(ubMes[0]);
    do {
        bModify = FALSE;
        /*----- erace current data -----*/
        if (sDAT->iCsrX == 0){
            CSRXY(8, sDAT->iCsrY + 3); puts(ubErace);
        }
        else {
			CSRXY(8, 18); puts(ubErace);
			CSRXY(20, 18); puts(ubErace);
			CSRXY(32, 18); puts(ubErace);
			CSRXY(40, 18); puts(ubErace);
			CSRXY(52, 18); puts(ubErace);
			CSRXY(64, 18); puts(ubErace);

        }
        /*----- clear string buffer -----*/
        vfBufClr(ubNewDat, 80);
        /*----- set cursor -----*/
        if (sDAT->iCsrX == 0){
            CSRXY(57, 9); puts(ubMes[1]);
            CSRXY(8, sDAT->iCsrY + 3);
        } else {
			CSRXY(57,9);
			if(sDAT->iKnjLen == KNJSIZ32 )
				puts(ubMes[2]);
			else if(sDAT->iKnjLen == KNJSIZ48)
				puts(ubMes[3]);
			else
				puts(ubMes[4]);
			CSRXY(8, 18);

        }
        /*----- get string -----*/
        CSRON();
        vfGetString(ubNewDat, (sDAT->iCsrX == 0)? YOMISIZ : sDAT->iKnjLen);
        CSROFF();
        /*----- too large string -----*/
        if (strlen(ubNewDat) > tail){
            bModify = TRUE;
            BEEP();
            CLRSCR();
            vfSetMonitor();
            vfDisplay(sDAT, NO_CSR_MOVE);
			vfBoxDisplay(sDAT);
            vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
            CSR(32);
#endif
            CSRXY(63, 7); puts(ubMod);
            NORMCSR();
            CSRXY(57, 8); puts(ubMes[0]);
        } 
        /*----- if string lenght = 0 stop modify process -----*/
        else if (strlen(ubNewDat) < head){
            vfRemoveBox();
            return;
        } 
        /*----- check string type -----*/
        else {
			if (sDAT->iCsrX == 0) {
				if ( YStrChk(ubNewDat, strlen(ubNewDat)) == TRUE)
					bModify = FALSE;
				else
					bModify = TRUE;
			}
			else
				bModify = FALSE;

            if (! bModify && (sDAT->iCsrX == 0))
                bModify = (sDAT->sRecBuf[0]->ubYomi[0] != ubNewDat[0]);
        }
    } while (bModify);    /*----- end of do~while -----*/

    /*---------- support process ----------*/
    iModify = ifSprt();
    if (iModify == NO){
        vfRemoveBox();
        return;
    }

    /*---------- data copy (case yomi) ----------*/
    if (sDAT->iCsrX == 0){
        /*----- clear original data -----*/
        for (i = 0; i < YOMISIZ; i++){
           sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i] = 0x00;
        }

        /*----- copy -----*/
        strcpy(sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi, ubNewDat);

        /*----- sort -----*/
		if (sDAT->wIndexMax != 1)
        	vfSort(sDAT);
    }
    /*---------- data copy (case kanji) ----------*/
    else {
        /*----- clear original data -----*/
        for (i = 0; i < sDAT->iKnjLen; i++){
            sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i] = 0x00;
        }
        /*----- copy -----*/
        strcpy(sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji, ubNewDat);
    }
    vfRemoveBox();
}

/*----------------------------------------------*/
/*    data entry routine                        */
/*----------------------------------------------*/
VOID    vfEntry(sDAT)
    sDATASET    *sDAT;        /* pointer to data buffer        */
{
    /*    messages    */
	static char    ubEmsg[] = "マスク中は実行することはできません. ";
    static char    ubEntry[] = " 登  録 ";
    static char    ubErace[] = "                    ";
    static char    *ubMes[]  = {
        "↓   読 み の 登 録",
        "半角 1 - 12 字以内",
        "←   漢 字 の 登 録",
        "全角 1 - 8 字以内",
        "全角 1 - 15 字以内",
		"全角 1 - 32 字以内",
    };

    REG     i;
    BOOL    bEntry;          /* entry flag               */
    int     iEntry;          /* support flag             */
    UBYTE   ubNewDat[80];    /* buffer for entry data    */
	UBYTE	dummy;

	/*---------check mask---------*/
	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); puts(ubEmsg);
		vfGetKey(&dummy);
		return;
	}

    /*---------- show message ---------*/
    BEEP();
    vfMakeBox(55, 7, 77, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubEntry);
    NORMCSR();
    CSRXY(57, 8); puts(ubMes[0]);
    CSRXY(57, 10); puts(ubMes[1]);

    do {
        bEntry = FALSE;

        /*----- clear data buffer -----*/
        vfBufClr(ubNewDat, 80);

        /*----- get entry data (yomi) -----*/
        CSRXY(57, 10); puts(ubErace);
        CSRXY(57, 10); 
        CSRON();        vfGetString(ubNewDat, YOMISIZ);
        CSROFF();

        /*----- too large string -----*/
        if (strlen(ubNewDat) > 20){
            bEntry = TRUE;
            BEEP();
            CLRSCR();
            vfSetMonitor();
            vfDisplay(sDAT, 0);
			vfBoxDisplay(sDAT);
            vfMakeBox(57, 7, 77, 11);
#ifndef    UNIX
            CSR(32);
#endif
            CSRXY(63, 7); puts(ubEntry);
            NORMCSR();
            CSRXY(57, 8); puts(ubMes[0]);
            CSRXY(57, 10); puts(ubMes[1]);
        } 
        /*----- if string length = 0 stop entry process -----*/
        else if (strlen(ubNewDat) < 1) {
            vfRemoveBox();
            return;
        } 

        /*----- check string type -----*/
        else {
			if ( YStrChk(ubNewDat, YOMISIZ) == TRUE)
				bEntry = FALSE;
			else
				bEntry = TRUE;

            if (! bEntry && (sDAT->iCsrX == 0))
                bEntry = (sDAT->sRecBuf[0]->ubYomi[0] != ubNewDat[0]);
        }
    } while (bEntry);    /*----- end of do~while -----*/

    /*---------- support process ----------*/
    iEntry = ifSprt();
    if (iEntry == NO){
        vfRemoveBox();
        return;
    }
    /*---------- block data move ----------*/
    vfDataMove(sDAT, ENTRY);

    /*---------- clear current data buffer(yomi) ----------*/
    for (i = 0; i < YOMISIZ; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi[i] = 0x00;

    /*---------- data copy ----------*/
    strcpy(sDAT->sRecBuf[sDAT->wIndexNum]->ubYomi, ubNewDat);

    /*---------- clear current data buffer(kanji) ----------*/
    for (i = 0; i < sDAT->iKnjLen; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji[i] = 0x00;

    /*---------- sort ----------*/
	if(sDAT->wIndexMax != 1)
    	vfSort(sDAT);

    /*---------- set cursor ----------*/
    sDAT->iCsrX = 1;
	vfGetIndex(sDAT);
    vfDisplay(sDAT, 0);
	vfBoxDisplay(sDAT);

    /*---------- show message ----------*/
    BEEP();
    vfRemoveBox();
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeBox(55, 8, 77, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 8); puts(ubEntry);
    NORMCSR();
    CSRXY(57, 9); puts(ubMes[2]);
    CSRXY(57, 10);
	if(sDAT->iKnjLen == KNJSIZ32)
		puts(ubMes[3]);
	else if(sDAT->iKnjLen == KNJSIZ48)
		puts(ubMes[4]);
	else
		puts(ubMes[5]);
	
    do {
        bEntry = FALSE;
        /*----- clear string buffer -----*/
        vfBufClr(ubNewDat, 80);
        /*----- clear current kanji data -----*/
        CSRXY(21, sDAT->iCsrY + 3); puts(ubErace);
        CSRXY(33, sDAT->iCsrY + 3); puts(ubErace);
		CSRXY(8, 18); puts(ubErace);
		CSRXY(25, 18); puts(ubErace);
		CSRXY(42, 18); puts(ubErace);
		CSRXY(56, 18); puts(ubErace);

        /*----- get new data -----*/
        CSRON();
        CSRXY(8, 18); 
        vfGetString(ubNewDat, sDAT->iKnjLen);
        CSROFF();
        /*----- too large string -----*/
        if (strlen(ubNewDat) > 65){
            bEntry = TRUE;
            BEEP();
            CLRSCR();
            vfSetMonitor();
            vfDisplay(sDAT, 0);
			vfBoxDisplay(sDAT);
            vfMakeBox(55, 7, 77, 10);
#ifndef    UNIX
            CSR(32);
#endif
            CSRXY(63, 7); puts(ubEntry);
            NORMCSR();
            CSRXY(57, 8); puts(ubMes[2]);
            CSRXY(57, 9); puts(ubMes[3]);
        } 

        /*----- if string length = 0 stop entry process -----*/
        else if (strlen(ubNewDat) < 1) {
            bEntry = TRUE;
        } 

        /*----- check string type -----*/
        else {
			bEntry = FALSE;
        }
    } while (bEntry);    /*----- end of do~while -----*/

    /*---------- data copy ----------*/
    strcpy(sDAT->sRecBuf[sDAT->wIndexNum]->ubKanji, ubNewDat);

    /*---------- clear current hinshi data ----------*/
    for (i = 0; i < HINSIZ; i++)
        sDAT->sRecBuf[sDAT->wIndexNum]->ubHinshi[i] = 0x00;

    /*---------- remove message box ---------*/
    vfRemoveBox();

    /*--------- hinshi entry ----------*/
    vfHinshiEdit(sDAT);
}

/*---------------------------------------------*/
/*    data swap routine                        */
/*---------------------------------------------*/
VOID    vfChange(sDAT)
    sDATASET    *sDAT;        /* pointer to data buffer        */
{
    /*    messages    */
	static char    ubEmsg[] = "マスク中は実行することはできません. ";
    static char    ubChange[] = {
        " 交  換 ",
    };
    static char    ubPrcs[]   = "交換中です. しばらくお待ちください.";
    static char    *ubMes[]   = {
        "同じ読みの範囲内で移",
        "動して交換先を指定し",
        "てください.",
    };
    
    sREC48      sTemp1,         /* temporary data buffer          */
                sTemp2;         /* temporary data buffer          */
    int         iAction;        /* support flag                   */
    UBYTE       ubKey;          /* key code buffer                */
	UBYTE		dummy;
    WORD        wPoint1,        /* base point                     */
                wPoint2,        /* swaping point                  */
                wTop, wEnd,     /* homonym top & end index        */
                wHomonym;       /* number of homonym              */
    BOOL        bChange;        /* change flag                    */
    REG         i;
	int			VertHinFlag;
	char		yomibuf[13];
	char		kanjibuf[66];


	/*---------check mask-----------*/
	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); puts(ubEmsg);
		vfGetKey(&dummy);
		return;
	}

	VertHinFlag = sDAT->VertHin;
	sDAT->VertHin = NO;
	

    /*---------- message display ----------*/
    BEEP();
    vfMakeBox(55,7, 77, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubChange);
    NORMCSR();
    for (i = 0; i < 3; i++){
        CSRXY(57, 8 + i); 
        puts(ubMes[i]);
    }
    /*---------- get base data ----------*/
    wPoint1 = sDAT->wIndexNum;
    for (i = 0; i < YOMISIZ; i++){
        sTemp1.ubYomi[i] = sDAT->sRecBuf[wPoint1]->ubYomi[i];
    }
    for (i = 0; i < HINSIZ; i++){
        sTemp1.ubHinshi[i] = sDAT->sRecBuf[wPoint1]->ubHinshi[i];
    }
    for (i = 0; i < sDAT->iKnjLen; i++){
        sTemp1.ubKanji[i] = sDAT->sRecBuf[wPoint1]->ubKanji[i];
    }
    /*----------    homonym check    ----------*/
    wHomonym = wfHomonym(sDAT, &wTop, &wEnd);
    if (wHomonym == 1){
        vfRemoveBox();
        return;
    }

	memset(kanjibuf,0x00,66);
	memcpy(yomibuf,sDAT->sRecBuf[wPoint1]->ubYomi,12);
	yomibuf[12] = 0x00;
	memcpy(kanjibuf,sDAT->sRecBuf[wPoint1]->ubKanji,64);
	if (strlen(kanjibuf) > 44){
		kanjibuf[42] = '.';
		kanjibuf[43] = '.';
		kanjibuf[44] = 0x00;
	}
	CSRXY(3,18);
	printf("交換元データ %4d %-12s %-44s", wPoint1 + 1,yomibuf,kanjibuf);

    /*---------- get swaping point ----------*/
    do {
        bChange = TRUE;
        vfGetKey(&ubKey);
        vfCsrMove(sDAT, ubKey);
        vfDisplay(sDAT, ubKey);
        if (ubKey == RET){
            if (sDAT->wIndexNum >= wTop && sDAT->wIndexNum < wEnd)
                bChange = FALSE;
            else {
                BEEP();
                bChange = TRUE;
            }
        }
        else if (ubKey == ESC){
            NORMCSR();
            vfRemoveBox();
			sDAT->VertHin = VertHinFlag;
            return;
        }
    } while(bChange);

	sDAT->VertHin = VertHinFlag;

	/*------ get swap point ------*/
	wPoint2 = sDAT->wIndexNum;

	/*------ swap point data on display -----*/
	memset(kanjibuf,0x00,66);
	memcpy(yomibuf,sDAT->sRecBuf[wPoint2]->ubYomi,12);
	yomibuf[12] = 0x00;
	memcpy(kanjibuf,sDAT->sRecBuf[wPoint2]->ubKanji,64);
	if (strlen(kanjibuf) > 44){
		kanjibuf[42] = '.';
		kanjibuf[43] = '.';
		kanjibuf[44] = 0x00;
	}
	CSRXY(3,18);
	printf("交換先データ %4d %-12s %-44s", wPoint2 + 1,yomibuf,kanjibuf);

    /*---------- support process ----------*/
    iAction = ifSprt();
    if (iAction == NO){
        vfRemoveBox();
        return;
    }
    /*---------- show message ----------*/
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeRvsBox(10, 10, 48, 12, YELLOW);
    CSRXY(12, 11);
    puts(ubPrcs);
    NORMCSR();

    /*---------- get swap point data ----------*/
    for (i = 0; i < YOMISIZ; i++){
        sTemp2.ubYomi[i] = sDAT->sRecBuf[wPoint2]->ubYomi[i];
    }
    for (i = 0; i < HINSIZ; i++){
        sTemp2.ubHinshi[i] = sDAT->sRecBuf[wPoint2]->ubHinshi[i];
    }
    for (i = 0; i < sDAT->iKnjLen; i++){
        sTemp2.ubKanji[i] = sDAT->sRecBuf[wPoint2]->ubKanji[i];
    }

    /*---------- data swap ----------*/
    for (i = 0; i < YOMISIZ; i++)
        sDAT->sRecBuf[wPoint1]->ubYomi[i] = sTemp2.ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
        sDAT->sRecBuf[wPoint1]->ubHinshi[i] = sTemp2.ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
        sDAT->sRecBuf[wPoint1]->ubKanji[i] = sTemp2.ubKanji[i];

    for (i = 0; i < YOMISIZ; i++)
        sDAT->sRecBuf[wPoint2]->ubYomi[i] = sTemp1.ubYomi[i];
    for (i = 0; i < HINSIZ; i++)
        sDAT->sRecBuf[wPoint2]->ubHinshi[i] = sTemp1.ubHinshi[i];
    for (i = 0; i < sDAT->iKnjLen; i++)
        sDAT->sRecBuf[wPoint2]->ubKanji[i] = sTemp1.ubKanji[i];

    /*----- remove message box -----*/
    vfRemoveBox();
}

/*---------------------------------------------*/
/*    print out control routine                */
/*---------------------------------------------*/
VOID    vfPrintCtrl(sDAT)
    sDATASET    *sDAT;        /* pointer to data buffer        */
{
    /*    messages    */
	static char    ubEmsg[] = "マスク中は実行することはできません. ";
    static char    ubPrint[]  = " 印  刷 ";
    static char    ubStart[]  = "開始位置指定";
    static char    ubEnd[]    = "終了位置指定";
    static char    *ubRange[] = {
         "  中  止  ", 
         " 全 単 語 ", 
         " 範囲指定 ",
    };
    static char    *ubOrder[] = {
        "移動して[RET]を","押してください",
    };
    static char    ubPrinting[] = 
        "印刷中です.しばらくお待ちください.";
    REG      i;
    UBYTE    ubKey;           /* key code buffer                       */
    int      iCsr = 0,        /* cursor point of inside message box    */
             iAction;         /* support flag                          */
    WORD     wStart,          /* print starting index                  */
             wEnd;            /* print ending index                    */

	/*---------- check mask----------*/
	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); puts(ubEmsg);
		vfGetKey(&ubKey);
		return;
	}

    /*---------- show message ----------*/
    CSROFF();
    vfMakeBox(56, 7, 76, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 7); puts(ubPrint);

    /*---------- get print mode ----------*/
    do {
        NORMCSR();
        for (i = 0; i < 3; i++){
            CSRXY(62, 8 + i); 
            puts(ubRange[i]);
        }
        CSRXY(62, 8 + iCsr);
        CSRRVS();
        puts(ubRange[iCsr]);
        vfGetKey(&ubKey);
        if (ubKey == ESC){
            NORMCSR();
            vfRemoveBox();
            return;
        }
        if (ubKey == DWN){
            iCsr++;
            if (iCsr > 2) iCsr = 0;
        }
        if (ubKey == UP){
            iCsr--;
            if (iCsr < 0) iCsr = 2;
        }
    } while(ubKey != RET);

    /*---------- switch print mode ----------*/
    switch(iCsr){

        /*----- stop print process -----*/
        case    STOP_PROCESS:    /*0*/
            break;

        /*----- select all -----*/
        case    SELECT_ALL:        /*1*/

            /*----- show support message -----*/
            iAction = ifSprt();
            if (iAction == NO){
                vfRemoveBox();
                return;
            }

            /*----- set all range -----*/
            wStart = 0; wEnd = sDAT->wIndexMax - 1;

            /*----- show printing message -----*/
#ifdef UNIX
            CSRRVS();
#endif
            vfMakeRvsBox(10, 10, 46, 12, YELLOW);
            CSRXY(12, 11); 
            puts(ubPrinting);
            NORMCSR();

            /*----- start printing -----*/
            vfPrintMrg(sDAT, wStart, wEnd);
            break;

        /*----- select range -----*/
        case    SELECT_RANGE:    /*2*/

            /*----- show message -----*/
            BEEP();
            vfMakeBox(56, 7, 76, 10);
#ifndef    UNIX
            CSR(32);
#endif
            CSRXY(61, 7); puts(ubStart);
            NORMCSR();
            for (i = 0; i < 2; i++){
                CSRXY(60, 8 + i); 
                puts(ubOrder[i]);
            }

            /*----- get starting index -----*/
            do {
                vfGetKey(&ubKey);
                vfCsrMove(sDAT, ubKey);
                vfDisplay(sDAT, ubKey);
				vfBoxDisplay(sDAT);
            } while(ubKey != RET);

            /*----- set starting index -----*/
            wStart = sDAT->wIndexNum;

            /*----- show message -----*/
#ifndef    UNIX
            CSR(32);
#endif
            BEEP();
            CSRXY(61, 7); puts(ubEnd);
            NORMCSR();

            /*----- get ending index -----*/
            do {
                vfGetKey(&ubKey);
                vfCsrMove(sDAT, ubKey);
                vfDisplay(sDAT, ubKey);
				vfBoxDisplay(sDAT);
            } while(ubKey != RET);

            /*----- set ending index -----*/
            wEnd = sDAT->wIndexNum;

            /*----- support process -----*/
            iAction = ifSprt();
            if (iAction == NO){
                vfRemoveBox();
                return;
            }

            /*----- show printing message -----*/
#ifdef UNIX
            CSRRVS();
#endif
            vfMakeRvsBox(10, 10, 46, 12, YELLOW);
            CSRXY(12, 11); 
            puts(ubPrinting);

            /*----- start printing -----*/
            if (wStart <= wEnd)
                vfPrintMrg(sDAT, wStart, wEnd);
            else
                vfPrintMrg(sDAT, wEnd, wStart);
            break;
    }
    /*---------- remove message box ----------*/
    vfRemoveBox();
}

/*--------------------------------------------*/
/*    quit control routine                    */
/*--------------------------------------------*/
int        ifQuit(sDAT)
    sDATASET    *sDAT;       /* pointer to data buffer        */
{
    /*    messages    */
	static char     ubEmsg[] = "マスク中は実行することはできません. ";
    static char    *ubQuit     = " 終了確認 ";
    static char    *ubSelect[] = {
        "   中  止   ",
        "セーブ後終了",
        "  強制終了  ",
    };
    REG      i;
    int      iSelect,        /* select mode                         */
             iCsr = 0;       /* cursor point of inside messaage box */
    UBYTE    ubKey;          /* key code                            */

    /*---------- show message ----------*/
    CSROFF();
    BEEP();
    vfMakeBox(56, 7, 76, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(62, 7); puts(ubQuit);
    NORMCSR();
    /*---------- get select mode ----------*/
    do {
        NORMCSR();
        CSROFF();
        for (i = 0; i < 3; i++){
            CSRXY(61, 8 + i); 
            puts(ubSelect[i]);
        }
        CSRRVS();
        CSRXY(61, 8 + iCsr);
        puts(ubSelect[iCsr]);
        vfGetKey(&ubKey);
        if (ubKey == ESC){
            NORMCSR();
            vfRemoveBox();
            return    EDIT;
        }
        if (ubKey == DWN){
            iCsr++;
            if (iCsr > 2) iCsr = 0;
        }
        if (ubKey == UP){
            iCsr--;
            if (iCsr < 0) iCsr = 2;
        }
    } while(ubKey != RET);
    /*---------- switch quit mode ----------*/
    switch(iCsr){
        /*----- stop quit process -----*/
        case    STOP_PROCESS:
            iSelect = EDIT;
            break;
        /*----- quit after saving -----*/
        case    QUIT_AFTER_SAVE:
			if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
				iSelect = EDIT;
				BEEP();
				vfMakeRvsBox(10, 10, 48, 12, RED);
				CSRXY(12, 11); puts(ubEmsg);
				vfGetKey(&ubKey);
			} else if (sDAT->iReadOnly == YES){
				iSelect = EDIT;
				vfDispReadOnly();
			} else {
				iSelect = QUIT;
				bfWriteRec(sDAT);
			}
			break;

        /*----- quit ignore -----*/
        case    QUIT_IGNORE:
            iSelect = QUIT;
            break;
    }

    /*---------- remove message box ----------*/
    vfRemoveBox();

    /*---------- return select value ----------*/
    NORMCSR();
    return    iSelect;
}

/*-----------------------------------------------*/
/*    hinshi edit routine                        */
/*-----------------------------------------------*/
VOID    vfHinshiEdit(sDAT)
    sDATASET    *sDAT;        /* pointer to data buffer            */
{
    /*    messages    */
    static char    ubEdit[] = " 品詞編集 ";
    static char    *ubMes[] = {
        "品詞編集してください",
        "[SPC] で BIT ON/OFF ",
    };
    UBYTE    ubKey;            /* key code                       */
    int      x = 0,            /* cursor point                   */
             shift = 1,        /* shift value                    */
             wDispNum,         /* temporary value                */
             iCsrY;            /* temporary value                */
    WORD     wIndexNum;        /* temporary value                */

    /*---------- set temporary value ----------*/
    wDispNum  = sDAT->wDispNum;
    iCsrY       = sDAT->iCsrY;
    wIndexNum = sDAT->wIndexNum;

    /*---------- show message ----------*/
    BEEP();
#ifdef UNIX
    CSRRVS();
#endif
    vfMakeBox(55, 8, 77, 11);
#ifndef    UNIX
    CSR(32);
#endif
    CSRXY(63, 8); puts(ubEdit);
    NORMCSR();
    CSRXY(57, 9); puts(ubMes[0]);
    CSRXY(57, 10); puts(ubMes[1]);

    do {
        CSROFF();
        vfDispHin(sDAT);
        CSRXY(3 + x * 2, 23);
#ifdef		UNIX
		CSRRVS();
#else
		CSR(7;32);
#endif
        puts((sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]]->ubHinshi[(x + 1) / 8]
                    & (0x80 >> shift))? "●" : "○");
        vfGetKey(&ubKey);

        /*----- green cursor move and bit on/off -----*/
        switch(ubKey){
            case    RGT:
                x ++; shift++;
                if (x > 30){
                    x = 0;
                    shift = 1;
                }
                if (shift > 7) shift = 0;
                break;
            case    LFT:
                x --; shift--;
                if (x < 0){
                    x = 30;
                    shift = 7;
                }
                if (shift < 0) shift = 7;
                break;
            case    SPC:
                if (x != BUNSETU && x != MISIYOU && x != SUUJI){
                    sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]]->ubHinshi[(x + 1) / 8]
                        ^= (0x80 >> shift);
                } else {
                    BEEP();
                }
                break;
            default    :
                break;
        }
    } while (ubKey != TAB && ubKey != ESC);
    CSROFF();
    vfRemoveBox();
}

/*--------------------------- end of command.c ------------------------------*/
