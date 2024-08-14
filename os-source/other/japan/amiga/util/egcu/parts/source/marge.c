#ifndef lint
static char     rcsid[] = "$Id: marge.c,v 3.4 91/06/20 20:33:50 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      Marge : EGConvert utility parts                                 */
/*                                                                      */
/*              Designed    by  Y.Katogi        08/27/1990              */
/*              Coded       by  Y.Katogi        08/27/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
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
#include "egckcnv.h"
#include "egcdef.h"
#include "egcldpro.h"

#define lgint(p)  (UWORD) (((UWORD) (*p) << 8) + (UWORD) (*(p + 1)));
static WORD     mread(
#ifdef DOS_EGC
    UBYTE          *buf,
    UWORD           cnt,
    EGCFILE         fp
#endif
);

/***********************************************************/
NONREC
WORD egcmarge(mainname, username, expert)
    UBYTE          *mainname;
    UBYTE          *username;
    UBYTE          *expert;
/*     main user dictionary marge                          */
/***********************************************************/
{
    EGCFILE         mfp, ufp, efp;
    LONG            xs, xl, gt;
    LONG            dsoff = 0L;
    UWORD           segq = 0;
    LONG            cnt, mod;
    LONG            i;
    UBYTE           bbuf[1024];

    mfp = ufp = efp = (EGCFILE)NULL;
    if ((mfp = egcfopen(mainname, OPENMODE)) == (EGCFILE)NULL) {
        i = ERMAR01;
        goto err_exit;
    }
    if ((ufp = egcfopen(username, OPENMODE)) == (EGCFILE)NULL) {
        i = ERMAR02;
        goto err_exit;
    }
    if ((efp = egcfopen(expert, CREATMODE)) == (EGCFILE)NULL) {
        i = ERMAR03;
        goto err_exit;
    }
    if (mread(bbuf, 16, mfp) != NORMAL) {
        i = ERMAR11;
        goto err_exit;
    }
    segq = (UWORD) ((UWORD) bbuf[1] << 8) + (UWORD) bbuf[0];
    dsoff = (LONG) lgint(&bbuf[4]);
    gt = (LONG) segq *DSBLKL / 16 + dsoff;      /* get block count */
    if (egcfwrite(bbuf, sizeof(UBYTE), 16, efp) == ERROR) {
        i = ERMAR23;
        goto err_exit;
    }
    cnt = ((gt - 1) * 16) / sizeof(bbuf);
    mod = ((gt - 1) * 16) % sizeof(bbuf);
    for (i = 0; i < cnt; i++) {
        if (mread(bbuf, sizeof(bbuf), mfp) != NORMAL) {
            i = ERMAR11;
            goto err_exit;
        }
        if (egcfwrite(bbuf, sizeof(bbuf), 1, efp) == ERROR) {
            i = ERMAR23;
            goto err_exit;
        }
    }
    if (mread(bbuf, (UWORD)mod, mfp) != NORMAL) {
        i = ERMAR12;
        goto err_exit;
    }
    if (egcfwrite(bbuf, (UWORD) mod, 1, efp) == ERROR) {
        i = ERMAR23;
        goto err_exit;
    }
    if (mread(bbuf, 128, ufp) != NORMAL) {
        i = ERMAR12;
        goto err_exit;
    }
    xs = (LONG) bbuf[0];
    xl = (LONG) bbuf[1];
    bbuf[2] = 0;
    bbuf[3] = 0;
    bbuf[4] = 0;
    bbuf[5] = 0;
    bbuf[6] = 0;
    bbuf[8] = 0;
    bbuf[9] = (BYTE) (xs + xl);
    if (egcfwrite(bbuf, 128, 1, efp) == ERROR) {
        i = ERMAR23;
        goto err_exit;
    }
    cnt = (xl * 128) / sizeof(bbuf);
    mod = (xl * 128) % sizeof(bbuf);
    for (i = 0; i < cnt; i++) {
        if (mread(bbuf, sizeof(bbuf), ufp) != NORMAL) {
            i = ERMAR12;
            goto err_exit;
        }
        if (egcfwrite(bbuf, sizeof(bbuf), 1, efp) == ERROR) {
            i = ERMAR23;
            goto err_exit;
        }
    }
    if (mread(bbuf, (UWORD) mod, ufp) != NORMAL) {
        i = ERMAR12;
        goto err_exit;
    }
    if (egcfwrite(bbuf, (UWORD) mod, 1, efp) == ERROR) {
        i = ERMAR23;
        goto err_exit;
    }
    i = NORMAL;

err_exit:
    if (mfp)
        egcfclose(mfp);
    if (ufp)
        egcfclose(ufp);
    if (efp)
        egcfclose(efp);
    return ((WORD) i);
}
static
WORD            mread(buf, cnt, fp)
    UBYTE          *buf;
    UWORD           cnt;
    EGCFILE         fp;
{
    WORD            i = NORMAL;

    if (egcfread(buf, sizeof(UBYTE), (WORD) cnt, fp) == ERROR) {
        i = ERMAR12;
        goto err_r;
    }
    /*
    if (ferror(fp) != NULL || feof(fp) != NULL) {
        i = ERMAR11;
        goto err_r;
    }
    */
err_r:
    return (i);
}
