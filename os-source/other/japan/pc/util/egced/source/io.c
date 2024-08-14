#ifndef lint
static char     rcsid[]="$Id: io.c,v 40.0 93/06/08 11:00:24 brummer Exp $";
#endif
/*
*
*     io.c  :   file input & output functions
*
*        bfReadRec()  : read data
*        vfWriteRec() : write data
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by        T.Koide   1992.09.14
*
*/

#include    <stdio.h>
#include    "egced.h"
#include    "crt.h"

/*-------------------------------------------------------*/
/*    data write                                         */
/*-------------------------------------------------------*/
BOOL    bfWriteRec(sDAT)
    sDATASET    *sDAT;
{
    REG     i;
    BOOL    bRslt;
    int     iSave;
    UBYTE   dummy;
	UBYTE	ubKey;
	static  char    ubEmsg[] = "マスク中は実行することはできません. ";
    static  char    ubSaving[] = 
        "保存中です.しばらくお待ちください.  ";

	/*----------- mask check ------------*/
	if(sDAT->ubMask[1] != -1 && sDAT->ubMask[0] != 0x00) {
		BEEP();
		vfMakeRvsBox(10, 10, 48, 12, RED);
		CSRXY(12, 11); puts(ubEmsg);
		vfGetKey(&ubKey);
		return FALSE;
	}

    iSave = ifSprt();
    if (iSave == NO)    return    FALSE;

    if((sDAT->fp = fopen(sDAT->ubFile, "wb")) == NULL){
        bRslt = FALSE;
        goto    err;
    }
#ifdef		UNIX
	CSRRVS();
#endif
    vfMakeRvsBox(10, 10, 48, 12, MAGENTA);
    CSRXY(12, 11); puts(ubSaving);
    
    for (i = 0; i < sDAT->wIndexMax; i++){
        bRslt = fwrite(sDAT->sRecBuf[i], sDAT->iDicLen, 1, sDAT->fp);
        if (bRslt != 1){
            bRslt = FALSE;
            break;
        } else {
            bRslt = TRUE;
        }
    }
    
    fclose(sDAT->fp);
    
err:
    if (! bRslt){
        vfMakeBox(56, 7, 78, 10);
        CSRXY(58, 8); puts("保存に失敗しました.");
        CSRXY(58, 9); puts("何かキーを押してく");
        CSRXY(58, 10); puts("ださい");
        vfGetKey(&dummy);
        vfRemoveBox();
    }
    
    return    bRslt;
}

BOOL    bfReadRec(sDAT)
    sDATASET    *sDAT;
{
    REG     i;
    BOOL    bReadRslt;       /* return value                        */
    long    lReadCnt;        /* normal:DICSIZ  error:!DICSIZ        */

    fprintf(stdout, "\n\n  ファイルを読み込み中です...\n  ");
    if ((FILE *)NULL == (sDAT->fp = fopen(sDAT->ubFile, "rb"))){
        fprintf(stderr, "  NO FILE  : %s\n", sDAT->ubFile);
        return    FALSE;
    }
    for (i = 0; i < sDAT->wIndexMax; i++){
        lReadCnt = fread(sDAT->sRecBuf[i], sDAT->iDicLen, 1, sDAT->fp);
        if (!lReadCnt){
            bReadRslt = FALSE;
            break;
        } else{
            bReadRslt = TRUE;
        }
		sDAT->IndexNum[i] = i;
        if (!((i+1) % 500)) fprintf(stdout, "#");
    }

	sDAT->IndexMax = sDAT->wIndexMax;

    fclose(sDAT->fp);
    
    return  bReadRslt;
}

