#ifndef lint
static char     rcsid[] = "$Id: load.c,v 1.5 91/06/12 21:27:29 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      LOAD : EGConvert   utility parts                                */
/*                                                                      */
/*              Designed    by  Y.Katogi        08/27/1990              */
/*              Coded       by  Y.Katogi        08/27/1990              */
/*                                                                      */
/*      (C) Copyright 1991 ERGOSOFT Corp.                               */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                         --- NOTE ---                                 */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egckcnv.h"
#include "egcdef.h"
#include "egcdict.h"
#include "egcpart.h"
#include "egcproto.h"

#define    GFILE    "dict88.dic"
UBYTE     *egcopengrmdic();
VOID       egcclosegrmdic();
VOID       egcclosegrmdic();
EGCFILE    gref;
UBYTE     *grtp;

/***********************************************************/
NONREC
WORD egcload(loadfp, seqrecval, pnorec, plddic)
    EGCFILE         loadfp;            /* load file pointer */
    UBYTE           seqrecval;         /* sequencial file rec value version1 */
    UWORD          *pnorec;            /* number of load records */
    DICM           *plddic;
/***********************************************************/
{
    register UWORD  ylen, klen;
    WORD            st;
    UWORD           recv = 0;
    struct{
        UBYTE yomi[12];
        UBYTE hinshi[4];
        UBYTE kanji[64];
    } ldrec;

    egcmemset(ldrec.yomi, (UBYTE)0x00, 12);
    egcmemset(ldrec.hinshi, (UBYTE)0x00, 4);
    egcmemset(ldrec.kanji, (UBYTE)0x00, 64);
    recv = 0;
    while ((st = egcfread((UBYTE *)&ldrec, (WORD)seqrecval, 1, loadfp))
               == NORMAL) {
        for (ylen = 0; ylen < MAXJYOMI && ldrec.yomi[ylen] != 0; ylen++);
        for (klen = 0; klen < MAXJKANJI && ldrec.kanji[klen] != 0; klen++);
        switch (st = egcwrite(ldrec.yomi, ylen, ldrec.kanji, 
                      klen, ldrec.hinshi, plddic)) {
            case NORMAL  : 
                recv++; 
                break;
            case ERWRI12 : 
                fprintf(stdout, "over homonym : %12s %32s\n", 
                        ldrec.yomi, ldrec.kanji);
                break;
            case ERWRI32 : 
                fprintf(stdout, "skip writing : %12s %32s\n", 
                        ldrec.yomi, ldrec.kanji);
                break;
            case ERWRI02 :
                fprintf(stdout, "yomi string error : %12s %32s\n",
                        ldrec.yomi, ldrec.kanji);
                break;
            default      : 
                *pnorec = recv; 
                return    (st);
        }
        egcmemset(ldrec.yomi, (UBYTE)0x00, 12);
        egcmemset(ldrec.kanji, (UBYTE)0x00, 32);
    }
    *pnorec = recv;
    return (st);
}                                      /* end of egcload */
/*PAGE*/

/***********************************************************/
NONREC
UBYTE * egcopengrmdic()
/***********************************************************/
{
    UBYTE           gdhead[FHNUM][2];
    UWORD           gsize;
    register REG    i;

    if ((gref = egcfopen(GFILE, OPENMODE)) == (EGCFILE)NULL) {
        goto err_exit;
    }
    for (i = 0; i < FHNUM; i++) {
        if (egcfread(gdhead[i], (WORD)sizeof(UBYTE), 2, gref) == ERROR){
            goto err_exit;
        }
    }
    gsize = getint((UBYTE *) gdhead[FHLAST]);
    if ((grtp = egcnewptr((REG) gsize)) == (UBYTE *)NULL) {
        goto err_exit;
    }
    if (egcfseek(gref, 0L, 0) == ERROR) {
        goto err_exit;
    }
    if (egcfread(grtp, gsize, 1, gref) == ERROR) {
        goto err_exit;
    }
    return (grtp);

err_exit:
    egcclosegrmdic();
    return (NULL);
}
/***********************************************************/
NONREC
VOID egcclosegrmdic()
/***********************************************************/
{
    if (grtp) {
        egcfreeptr(grtp);
        grtp = (UBYTE *)NULL;
    }
    if (gref) {
        egcfclose(gref);
        gref = (EGCFILE)NULL;
    }
}
