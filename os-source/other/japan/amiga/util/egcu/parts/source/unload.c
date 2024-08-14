/************************************************************************/
/*                                                                      */
/*      UNLOAD : EGConvert unload utility parts                         */
/*                                                                      */
/*              Designed    by  H.Yanata        08/12/1992              */
/*              Coded       by  H.Yanata        08/12/1992              */
/*                                                                      */
/*      (C) Copyright 1990-1992 ERGOSOFT Corp.                          */
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
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcdict.h"
#include "egcexp.h"
#include "egcudep.h"
#include "egcpart.h"
#include "egcproto.h"
#include "egcldpro.h"

/*--------------------------------------------------------------*/
/*  egcunload : Unload dictionary (simple version)             */
/*--------------------------------------------------------------*/
BOOL    egcunload(fp, size, count, start, next)
    EGCFILE         fp;             /* unload file pointer */
    UBYTE           size;           /* sequencial file size */
    LONG*           count;          /* number of unload records */
    UBYTE*          start;          /* start point  */
    UBYTE*          next;           /* next label */
{
    register UWORD   i = 0;
    DICR             dr;
    struct {
        UBYTE   yomi[MAXJYOMI];
        UBYTE   hinshi[4];
        UBYTE   kanji[MAXJKANJI];
    } seqb;
    UBYTE            yomi[MAXJYOMI + 1];
    UBYTE            kanji[MAXJKANJI + 1];
    UWORD            yomil;
    UWORD            kanjil;
    UBYTE            hinshi[4];
    WORD             NextErr;
    UWORD            index = 0;

    if (egcfind(start, (UBYTE)1, (UBYTE)0, (UBYTE)0, &dr)) {
        *count = 0;
        return  ERROR;
    }

    do {
        if (egcread(yomi, &yomil, kanji, &kanjil, hinshi, &dr)) {
            if ((NextErr = egcnext(&dr)) == NORMAL) {
                egcread(yomi, &yomil, kanji, &kanjil, hinshi, &dr);
            } else {
                break;
            }
        }
        if (! memncmp(next, yomi, 1))
            break;
        egcmemset(seqb.yomi, 0x00, MAXJYOMI);
        egcmemset(seqb.hinshi, 0x00, 4);
        egcmemset(seqb.kanji, 0x00, MAXJKANJI);
        egcmemmove(seqb.yomi, yomi, yomil);
        egcmemmove(seqb.hinshi, hinshi, 4);
        egcmemmove(seqb.kanji, kanji, kanjil);
        if (egcfwrite((UBYTE *)&seqb, size, 1, fp) == ERROR) {
            *count = i;
            return  ERROR;
        }
        i ++;
    } while(TRUE);
    *count = i;
    return    NORMAL;
}
