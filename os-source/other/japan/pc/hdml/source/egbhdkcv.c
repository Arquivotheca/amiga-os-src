#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdkcv.c :kanatokanji conversion subroutines for HDML         */
/*                                                                      */
/*              Designed    by  I.Iwata         12/03/1986              */
/*              Coded       by  I.Iwata         07/14/1986              */
/*              Modified    by  Y.Katogi        03/12/1990              */
/*                                                                      */
/*      (C) Copyright 1985-90 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*      include subroutines                                             */
/*                                                                      */
/*      1 UWORD  readhom(cb)                                            */
/*      2 UWORD  movhom(dir,cb)                                         */
/*      3 UWORD  fixhom(hnbr,cb)                                        */
/*      4 UWORD  agehom(cb)                                             */
/*      5 UWORD  canhom(cb)                                             */
/*      6 UWORD  curhomchk(cb)                                          */
/*                                                                      */
/************************************************************************/
#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egcerr.h"
#include  "egberr.h"

#define egccep(P)       P->acbebp+(P->acbcpn-1)*3
/*PAGE*/

/*-----------------------------------------------------------------*/
/*  1   readhom : read homonym                                     */
/*-----------------------------------------------------------------*/
UWORD 
readhom(cb)
    ACB             cb;                /* ->communication block        */
{
    WORD            st;                /* status                       */
    UWORD           hnbr;              /* total homonym number         */
    UWORD           plen;              /* homonym page   length        */
    UWORD           pnbr;              /* total page number            */
    UWORD           i;                 /* counter                      */
    WORD            cmpst;             /* compare status               */
    register UBYTE  count;             /* counter                  */
    register UBYTE *hpp;               /* -> homonym page buffer   */

    cmpst = strncmp(cb->acbobp + egcoel(cb), cb->acbhym, cb->acbhyl);
    if ((*(egccep(cb) + 2) == cb->acbhyl) && !(cmpst))
        return (NORMAL);
    cb->acbsmd &= ~CSMAGE;
    egcmemmove(cb->acbhbp, cb->acbsbp + egcsel(cb), *(egccep(cb) + 1));
    *(cb->acbhep) = *(egccep(cb) + 1);
    hnbr = 1;
    st = -1;
    while (st < 0) {
        st = egcclcvt(cb->acbobp + egcoel(cb),
                      *(egccep(cb) + 2),
                      cb->acbhbp,
                      cb->acbhep,
                      cb->acbwid,
                      &hnbr,
                      (UBYTE) 2);
    }
    if (st >= ERCLC01)
        return (st);                /* no homonym then exit         */
    if (hdmerr(st) == ERROR)
        return (DISKERR);
    cb->acbcpg = 1;                 /* set current page    number   */
    cb->acbchm = 1;                 /* set current homonym number   */
    egcmemmove(cb->acbhym, cb->acbobp + egcoel(cb), (UWORD) (*(egccep(cb) + 2)));
    cb->acbhyl = *(egccep(cb) + 2);
    hpp = cb->acbhpp;
    pnbr = 1;
    plen = count = 0;
    for (i = 0; i < hnbr; i++) {
        plen += *(cb->acbhep + i);
        if (((plen > cb->acbspg) || (count >= PAGEHOMS)) && (count)) {
            *hpp++ = count;
            pnbr++;
            plen = *(cb->acbhep + i);
            count = 0;
        }
        *hpp = ++count;
    }
    cb->acbtpg = pnbr;
    return (NORMAL);
}
/*PAGE*/

/*-----------------------------------------------------------------*/
/*  2   movhom  : move homonym                                     */
/*-----------------------------------------------------------------*/
UWORD 
movhom(dir, cb)
    UBYTE           dir;               /* TRUE:right; FALSE:left               */
    ACB             cb;                /* eptr->edit record                    */
{
    register UBYTE *cpb;               /* ->  current page buffer              */

    cpb = cb->acbhpp + cb->acbcpg - 1;
    if (dir == DRRIGHT) {
        if (cb->acbchm < *cpb) {
            cb->acbchm++;
        }
        else {
            if (cb->acbcpg < cb->acbtpg) {
                cb->acbcpg++;
            }
            else {
                cb->acbcpg = 1;
            }
            cb->acbchm = 1;
        }
    }
    else if (dir == DRLEFT) {
        if (cb->acbchm > 1) {
            cb->acbchm--;
        }
        else {
            if (cb->acbcpg > 1) {
                cb->acbcpg--;
                cb->acbchm = *(cpb - 1);
            }
            else {
                cb->acbcpg = cb->acbtpg;
                cb->acbchm = *(cb->acbhpp + cb->acbtpg - 1);
            }
        }
    }
    else if (dir == DRUP) {
        if (cb->acbcpg > 1) {
            cb->acbcpg--;
            cb->acbchm = *(cpb - 1);
        }
        else {
            cb->acbcpg = cb->acbtpg;
            cb->acbchm = *(cb->acbhpp + cb->acbtpg - 1);
        }
    }
    else if (dir == DRDOWN) {
        if (cb->acbcpg < cb->acbtpg) {
            cb->acbcpg++;
        }
        else {
            cb->acbcpg = 1;
        }
        cb->acbchm = 1;
    }
    else
        return (SUBCMDERR);
    return (NORMAL);
}
/*PAGE*/

/*-----------------------------------------------------------------*/
/*  3   fixhom  : fix homonym                                      */
/*-----------------------------------------------------------------*/
UWORD 
fixhom(hnbr, cb)
    UBYTE           hnbr;              /* homonym number               */
    ACB             cb;                /* -> communicaton block        */
{
    register UBYTE *chm;               /* -> current homonym           */
    UBYTE          *cpg;               /* -> current homonym page      */
    register UBYTE *hep;               /* -> current homonym element   */
    UWORD           i, j;              /* counter                      */
    UWORD           chl;               /* character length             */
    UWORD           st;                /* status                       */

    cpg = cb->acbhpp;
    hep = cb->acbhep;
    chm = cb->acbhbp;
    if (cb->acbhyl == 0)
        return (NOHOMONYM);
    for (i = 1; i < cb->acbcpg; ++i) {
        for (j = 1; j <= *(cb->acbhpp + i - 1); ++j) {
            chm += *(hep + j - 1);
        }
        hep += *cpg++;
    }
    if (hnbr > *cpg)
        return (NOHOMONYM);
    if (hnbr == 0)
        hnbr = cb->acbchm;
    else
        cb->acbchm = hnbr;
    for (j = 1; j < hnbr; ++j) {
        chm += *(hep + j - 1);
    }
    chl = *(hep + j - 1);
    st = hdmpput(chl, chm, cb);
    if (cb->acbcpg == 1 && hnbr == 1)
        cb->acbsmd &= ~CSMAGE;
    else
        cb->acbsmd |= CSMAGE;
    return (NORMAL);
}
/*PAGE*/
/*----------------------------------------------------------------------*/
/*      agehom  : age homonym                                           */
/*                                  @@@ mod 03/12/1990                  */
/*----------------------------------------------------------------------*/
agehom(cb)
    ACB             cb;                /* -> communication block       */
{
    register UWORD  i;                 /* counter                      */
    register UWORD  thnbr;             /* total homonym number         */
    UWORD           hnbr;              /* homonym number               */
    WORD            st;                /* status                       */

    cb->acbsmd &= ~CSMAGE;
    hnbr = 0;
    thnbr = 0;
    for (i = 1; i <= cb->acbtpg; ++i) {
        if (i == cb->acbcpg)
            hnbr = thnbr + cb->acbchm;
        thnbr += *(cb->acbhpp + i - 1);
    }

/*HY Next line modefied at 22/09/1992 by H.Yanata HY*/
/*HY as kansuji learning                          HY*/
/*HY
    if ((hnbr == 0) || (cb->acbwid[hnbr - 1].wisegno == 0))
HY*/
    if (hnbr == 0)
        return (NOAGEHOMONYM);
    st = -1;
    while (st < 0) {
#if 1
        st = egclearn2(cb->acbwid, thnbr, hnbr - 1);
#else
        st = egclearn(cb->acbwid);
#endif
        if (st >= ERCLL01)
            return (st);
        st = hdmerr(st);
    }
    *(cb->acbhym) = 0;
    return (NORMAL);
}

/*-----------------------------------------------------------------*/
/*  5   canhom  : cancel homonym                                   */
/*-----------------------------------------------------------------*/
UWORD 
canhom(cb)
    ACB             cb;                /* -> communication block   */
{
    cb->acbsmd &= ~CSMAGE;
    cb->acbhyl = 0;
    cb->acbrmj = cb->acbcre;
    return (NORMAL);
}

/*-----------------------------------------------------------------*/
/*  6   current homonym                                            */
/*-----------------------------------------------------------------*/
UWORD 
curhomchk(cb)
    ACB             cb;                /* -> communicaton block        */
{
    register UBYTE *chm;               /* -> current homonym           */
    UBYTE          *cpg;               /* -> current homonym page      */
    register UBYTE *hep;               /* -> current homonym element   */
    UWORD           i, j;              /* counter                      */
    UWORD           chl;               /* character length             */
    WORD            st;
    WORD            cmpst;
    UWORD           status;

    cpg = cb->acbhpp;
    hep = cb->acbhep;
    chm = cb->acbhbp;
    for (i = 1; i < cb->acbcpg; ++i) {
        for (j = 1; j <= *(cb->acbhpp + i - 1); ++j) {
            chm += *(hep + j - 1);
        }
        hep += *cpg++;
    }
    for (j = 1; j < cb->acbchm; ++j) {
        chm += *(hep + j - 1);
    }
    chl = *(hep + j - 1);
    cmpst = strncmp(cb->acbsbp + egcsel(cb), chm, chl);
    if (cmpst)
        status = 0;
    else
        status = 1;
    return (status);
}
