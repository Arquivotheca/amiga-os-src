#ifndef lint
static char     rcsid[]="$Id: display.c,v 40.0 93/06/08 10:58:34 brummer Exp $";
#endif
/*
*
*     display.c  :   data display routhin
*
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by		   T.Koide   1992.09.14
*
*/


#include    <stdio.h>
#include    <string.h>
#include    "egced.h"
#include    "crt.h"
#include    "keybrd.h"

#ifdef PC9801
#include    <conio.h>
#define     printf    cprintf
#define     puts      cputs
#endif

/*---------------------------------------------*/
/*    word data display routine                */
/*---------------------------------------------*/

VOID    vfDisplay(sDAT, ubKey)
	sDATASET    *sDAT;         /* pointer to data buffer      */
    UBYTE       ubKey;         /* key code (ascii)            */
{
	static int		dummy1 = 1;	/*	display mode flag			*/
	static WORD		dummy2 = 1;	/*	display mode flag			*/
    int 			i, j;
	int				iBefore,	/*	base cursor point			*/
					iCsrY;		/*	vertical cursor point 		*/
	register WORD	wDispNum, 	/*	top index on display		*/
					wIndexNum;	/*	current index number		*/
	register sREC48	*Hndl;
	char	buf[100], yomibuf[13], kanjibuf[66];
	static char		ubNoMore[] = 
		"***************** No more record *****************";
	static char		ubSpace[] = 
		"                                                  ";

#ifdef PC9801
    if (ubKey == ESC && sDAT->VertHin == YES)    return;
#endif

	memset(buf, 0, 100);
	memset(kanjibuf, 0, 66);
	/*----- display data set -----*/

	if(sDAT->wIndexNum > sDAT->IndexMax - 1) {
		    sDAT->iCsrY = CSR_TOP;
		    sDAT->wIndexNum = INDEX_MIN;
		    sDAT->wDispNum = INDEX_MIN;
    }

	wDispNum  = sDAT->wDispNum;
	iCsrY 	  = sDAT->iCsrY;
	wIndexNum = sDAT->wIndexNum;

	if (ubKey == UP)
		iBefore = iCsrY + 1;
	else if(ubKey == DWN)
		iBefore = iCsrY - 1;
	else
		iBefore = iCsrY;
#ifdef	PC9801
	CSR(0);
#endif
	CSROFF();

	if (dummy1 == sDAT->wDispNum
	 && dummy1 != dummy2 - 1 
	 && dummy2 == sDAT->IndexMax
	 && (ubKey == UP  || 
		 ubKey == DWN || 
		 ubKey == RGT || 
		 ubKey == LFT)){
#ifdef	PC9801
		j = 3 + iBefore;
		Hndl = sDAT->sRecBuf[sDAT->IndexNum[wDispNum + iBefore]];
		memset(buf,0,100);
		memcpy(yomibuf,	Hndl->ubYomi, 12);
		memcpy(kanjibuf, Hndl->ubKanji,64);
		yomibuf[12] = 0x00;
		if( strlen(kanjibuf) < 32){
			sprintf(buf, "%4d %-12s %-32s",
				wIndexNum+iBefore-iCsrY+1,
				yomibuf,
				kanjibuf);
		} else {
			kanjibuf[30] = 0x00;
			sprintf(buf, "%4d %-12s %-30s..",
				wIndexNum+iBefore-iCsrY+1,
				yomibuf,
				kanjibuf);
		}

		if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) 
			DrawText(3, j, buf, strlen(buf), EGLCYAN);
		else
			DrawText(3, j, buf, strlen(buf), EGLWHITE);
#else
		j = 3 + iBefore;
		Hndl = sDAT->sRecBuf[sDAT->IndexNum[wDispNum + iBefore]];
		memset(buf,0,100);
		memcpy(yomibuf,	Hndl->ubYomi, 12);
		memcpy(kanjibuf, Hndl->ubKanji,64);
		yomibuf[12] = 0x00;
        CSRXY(3, j); printf("%4d   ", wIndexNum + iBefore - iCsrY + 1);
        CSRXY(8, j); printf("%-12s", yomibuf);
        CSRXY(20, j); printf(" ");
		if(strlen(kanjibuf) < 32) {
        	CSRXY(21, j); printf("%-32s", kanjibuf);
		} else {
			kanjibuf[30] = 0x00;
        	CSRXY(21, j); printf("%-30s.. ",kanjibuf);
		}
#endif
    } else {
#ifdef PC9801
		for (i = 0; i < DISPMAX; i++){
			if ( wDispNum + i > sDAT->IndexMax-1){
				DrawText(3, 3 + i, ubNoMore, strlen(ubNoMore), EGLWHITE);
				for ( j = i + 1; j < DISPMAX; j++){
					DrawText(3, 3 + j, ubSpace, strlen(ubSpace), EGLBLACK);
				}
				break;
			}
			Hndl = sDAT->sRecBuf[sDAT->IndexNum[wDispNum + i]];
			memset(buf, 0, 100);
			memcpy(yomibuf, Hndl->ubYomi, 12);
			memcpy(kanjibuf, Hndl->ubKanji,64);
			yomibuf[12] = 0x00;
			if(strlen(kanjibuf) < 32){
				sprintf(buf, "%4d %-12s %-32s",
					sDAT->wIndexNum - sDAT->iCsrY + i + 1,
					yomibuf,
					kanjibuf);
			} else {
				kanjibuf[30] = 0x00;
				sprintf(buf, "%4d %-12s %-30s..",
					sDAT->wIndexNum - sDAT->iCsrY + i + 1,
					yomibuf,
					kanjibuf);
			}
			if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) 
				DrawText(3, 3 + i, buf, strlen(buf), EGLCYAN);
			else
				DrawText(3, 3 + i, buf, strlen(buf), EGLWHITE);

		}

#else
        for (i = 0; i < DISPMAX; i++){
			if ( wDispNum + i > sDAT->IndexMax-1){
                CSRXY(3, 3 + i);
                puts(ubNoMore);
                for ( j = i + 1; j < DISPMAX; j++){
                    CSRXY(3, 3 + j);
                    puts(ubSpace);
                }
                break;
            }
            j = 3 + i;
            Hndl = sDAT->sRecBuf[sDAT->IndexNum[wDispNum + i]];
			memset(buf, 0, 100);
			memcpy(yomibuf, Hndl->ubYomi, 12);
			memcpy(kanjibuf, Hndl->ubKanji,64);
			yomibuf[12] = 0x00;
            CSRXY(3, j); printf("%4d   ", wDispNum + i + 1);
            CSRXY(8, j); printf("%-12s", yomibuf);
            CSRXY(20, j); printf(" ");
			if(strlen(kanjibuf) < 32) {
            	CSRXY(21, j); printf("%-32s", kanjibuf);
        	} else {
				kanjibuf[30] = 0x00;
            	CSRXY(21, j); printf("%-30s..", kanjibuf);
			}
		}
#endif
    }
    /*---------- current data display (reverce) ----------*/
#ifdef	PC9801
	j = 3 + iCsrY;
	if (sDAT->iCsrX == 0){
		Hndl = sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]];
		memcpy(yomibuf,	Hndl->ubYomi, 12);
		yomibuf[12] = 0x00;
		sprintf(buf, "%-12s ", yomibuf);
		DrawText(8, j, buf, strlen(buf), EGLCYAN|EGLREVERSE);
	} else{
		Hndl = sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]];
		memset(buf, 0, 100);
		memcpy(kanjibuf, Hndl->ubKanji, 64);
		if(strlen(kanjibuf) < 32) {
			sprintf(buf, "%-32s", kanjibuf);
		} else {
			kanjibuf[30] = 0x00;
			sprintf(buf, "%-30s..", kanjibuf);
		}
			DrawText(21, j, buf, strlen(buf), EGLCYAN|EGLREVERSE);
	}

#else
    j = 3 + iCsrY;
    Hndl = sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]];
	memcpy(yomibuf,	Hndl->ubYomi, 12);
	yomibuf[12] = 0x00;
	memcpy(kanjibuf,Hndl->ubKanji, 64);
	if(strlen(kanjibuf) > 32) {
		kanjibuf[30] = '.';
		kanjibuf[31] = '.';
		kanjibuf[32] = 0x00;
	}

    if (sDAT->iCsrX == 0){
        CSRRVS();
        CSRXY(8, j); printf("%-12s", yomibuf);
        CSRXY(20, j); printf(" ");
        NORMCSR();
        CSRXY(21, j); printf("%-32s", kanjibuf);
    } else{
        CSRRVS();
        CSRXY(21, j); printf("%-32s", kanjibuf);
        NORMCSR();
        CSRXY(8, j); printf("%-12s", yomibuf);
        CSRXY(20, j); printf(" ");
    }

#endif
    /*---------- file data display ----------*/
    if (dummy1 != sDAT->wDispNum && sDAT->VertHin == YES){
        vfDispVertHin(sDAT);
    } else {
        if (sDAT->VertHin == NO){
            if (strlen(sDAT->ubFile) > 12){
                i = 0;
                while(sDAT->ubFile[i] != '\0')        i++;
                while(sDAT->ubFile[i] != DELIMITER)   i--;
                i++;
                CSRXY(65, 3);
                printf("%-12s",sDAT->ubFile + i);
            } else {
                CSRXY(65, 3);
                printf("%-12s",sDAT->ubFile);
            }
			CSRXY(65, 4);
			if(sDAT->iDicLen == DICSIZ32)
				printf("%-12s","32 byte");
			else if(sDAT->iDicLen == DICSIZ48)
				printf("%-12s","48 byte");
			else 
				printf("%-12s","80 byte");

			CSRXY(65, 5);
			printf("%-5d", sDAT->IndexMax);
		}
	}


    /*---------- hinshi dispaly ----------*/
    vfDispHin(sDAT);


    /*---------- set flags ----------*/
    dummy1 = sDAT->wDispNum;
    dummy2 = sDAT->IndexMax;

}


/*-----------------------------------------------*/
/*    kanji data display in Box                  */
/*-----------------------------------------------*/
VOID		vfBoxDisplay(sDAT)
	const	sDATASET	*sDAT;
{
	char		buf[100];
	char		kanjibuf[66];
	register sREC48 *Hndl;

	memset(buf,0x00,100);
	Hndl = sDAT->sRecBuf[sDAT->IndexNum[sDAT->wIndexNum]];
	memcpy(kanjibuf, Hndl->ubKanji, 64);
	kanjibuf[64] = 0x00;
	sprintf(buf, "%4d %-64s       ",
			sDAT->wIndexNum+1,
			kanjibuf);
#ifdef PC9801
	DrawText(3, 18, buf, strlen(buf), EGLWHITE);
#else
	CSRXY(3,18);
	printf("%4d ",sDAT->wIndexNum+1);
	CSRXY(8,18);
	printf("%-64s       ",kanjibuf);
#endif

}


/*-----------------------------------------------*/
/*    hinshi data display routine(horizontal)    */
/*-----------------------------------------------*/

VOID    vfDispHin(sDAT)
	const    sDATASET    *sDAT;
{
    WORD    wIndexNum;
    char    ubHinBuf[31 * 2 + 1];    /* 31 * 2byte + NULL        */
    REG     i, j, k;

    wIndexNum = sDAT->wIndexNum;

    NORMCSR();

	k = 0;
	for (i = 0; i < 4; i++){
		for (j = 0; j < 8; j++){
			if (i == 0 && j == 0) continue;		/*	ubHinshi[0][0] is empty	*/
			ubHinBuf[k] = (UBYTE)0x81;
			ubHinBuf[k + 1]= ((sDAT->sRecBuf[sDAT->IndexNum[wIndexNum]]->ubHinshi[i] << j) & 0x80)? (UBYTE)0x9c : (UBYTE)0x9b;
			k += 2;
		}
	}
	ubHinBuf[62] = (UBYTE)0x00;

#ifdef	PC9801
	DrawText(3, 23, ubHinBuf, strlen(ubHinBuf), EGLWHITE);
#else
    CSRXY(3, 23); 
    puts(ubHinBuf);
#endif
}

/*---------------------------------------------*/
/*    hinshi data display routine(vertical)    */
/*---------------------------------------------*/
VOID    vfDispVertHin(sDAT)
	const    sDATASET    *sDAT;
{
    UBYTE      ubHinBuf[25];    /* 11 * 2byte + NULL     */
    REG        i, j;
    sREC48    *Hndl;
    static    char    *ubItem[] = {
        "名名人地副連形形サ一５他",
        "１２名名詞体容動変段段  ",
    };

#ifdef    PC9801
    CSR(36);
	DrawText(55, 1, ubItem[0], strlen(ubItem[0]), EGLCYAN);
	DrawText(55, 2, ubItem[1], strlen(ubItem[1]), EGLCYAN);
#else
    CSRXY(55, 1); puts(ubItem[0]);
    CSRXY(55, 2); puts(ubItem[1]);
#endif

	for (i = 0; i < DISPMAX; i ++){
		memset(ubHinBuf, 0x00, 22);
		Hndl = sDAT->sRecBuf[sDAT->IndexNum[sDAT->wDispNum + i]];
		for (j = 0; j < 24; j += 2)	ubHinBuf[j] = 0x81;
		if (sDAT->wDispNum + i > sDAT->IndexMax - 1){
			for (j = 1; j < 24; j += 2)	ubHinBuf[j] = 0x9b;
		} else {
			ubHinBuf[1] = (Hndl->ubHinshi[0] & 0x40)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[3] = (Hndl->ubHinshi[0] & 0x08)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[5] = (Hndl->ubHinshi[0] & 0x24)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[7] = (Hndl->ubHinshi[0] & 0x12)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[9] = (Hndl->ubHinshi[1] & 0x02)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[11] = (Hndl->ubHinshi[2] & 0x80)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[13] = (Hndl->ubHinshi[2] & 0x40)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[15] = (Hndl->ubHinshi[2] & 0x20)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[17] = (Hndl->ubHinshi[2] & 0x08)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[19] = (Hndl->ubHinshi[2] & 0x02)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[21] = (Hndl->ubHinshi[2] & 0x01
						|| Hndl->ubHinshi[3] & 0xff)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[23] = (Hndl->ubHinshi[0] & 0x01 
						|| Hndl->ubHinshi[1] & 0xfd
						|| Hndl->ubHinshi[2] & 0x10)? (UBYTE)0x9c:(UBYTE)0x9b;
			ubHinBuf[24] = 0x00;
		}
    	NORMCSR();
#ifdef	PC9801
		DrawText(55, 3 + i, ubHinBuf, strlen(ubHinBuf), EGLWHITE);
#else
    	CSRXY(55, 3 + i); puts(ubHinBuf);
#endif
    }
}

#ifdef	PC9801

/*--------------------------------------------------------------*/
/*	DrawText : draw target text									*/
/*--------------------------------------------------------------*/
VOID	DrawText(cdX, cdY, s, ls, attr)
WORD	cdX;
WORD	cdY;		/* draw starting position	*/
UBYTE	*s;			/* -> target text			*/
WORD	ls;			/* length of text			*/
UWORD	attr;		/* attribute				*/
{
	UWORD	register kch;
	WORD	register i;

	i = 0;
	while (i < ls) {
		kch = s[i];
		if (iskanji(kch)) {
			kch = (kch << 8) + s[i+1];
			DrawKnj(cdX, cdY, kch, attr);
			cdX++;
			i++;
		} else {
			DrawChr(cdX, cdY, kch, attr);
		}
		cdX++;
		i++;
	}
}

/*--------------------------------------------------------------*/
/*	DrawChr : draw character(st'd ASCII code)					*/
/*--------------------------------------------------------------*/
VOID	DrawChr(cdX, cdY, ch, attr)
WORD	cdX;
WORD	cdY;
UWORD	ch;
UWORD	attr;
{
	register UWORD	offset = (UWORD)(((cdX - 1) + (cdY - 1) * 80) << 1);
	*(UWORD __far *)(_VSEG_ADDR + offset) = (UWORD)(ch & 0x00FF);
	*(UBYTE __far *)(_ASEG_ADDR + offset) = (UBYTE)attr;
}

/*--------------------------------------------------------------*/
/*	DrawKnj : draw kanji code(st'd Shift JIS code)				*/
/*--------------------------------------------------------------*/
VOID	DrawKnj(cdX, cdY, jms, attr)
WORD	cdX;
WORD	cdY;
UWORD	jms;
UWORD	attr;
{
	UWORD	register jis;
	register UWORD	offset;

	jis = jmstojis(jms);
	jis = (jis >> 8) + ((jis & 0x00ff) << 8) - 0x20;

	offset = (UWORD)(((cdX - 1) + (cdY - 1) * 80) << 1);
	*(UWORD __far *)(_VSEG_ADDR + offset) = jis;
	*(UBYTE __far *)(_ASEG_ADDR + offset) = (UBYTE)attr;
	jis |= 0x8000;
	offset += 2;
	*(UWORD __far *)(_VSEG_ADDR + offset) = jis;
	*(UBYTE __far *)(_ASEG_ADDR + offset) = (UBYTE)attr;
}

#endif

