#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdelb.c :  text edit support procedures for HDML             */
/*                                                                      */
/*              Designed    by  I.Iwata         07/09/1986              */
/*              Coded       by  I.Iwata         07/09/1986              */
/*              Modified    by  K.Nakamura      10/18/1988              */
/*                                                                      */
/*      (C) Copyright 1985-88 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*      include subrutins                                               */
/*                                                                      */
/*      1  UWORD hdmkey(chp,cb)                                         */
/*      2  UWORD rvcvt(cb)                                              */
/*      3  UWORD hdmdel(dir,cb)                                         */
/*      4  UWORD movchar(dir,cb)                                        */
/*      5  UWORD ntojis(asrc,spt,ept,adst)                              */
/*                                                                      */
/************************************************************************/

#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egberr.h"
#include "egcerr.h"

#define egccep(P)       P->acbebp+(P->acbcpn-1)*3
/*PAGE*/

/*******************************************************************/
/*                                                                 */
/*  5  numeric to jis-code conversion                              */
/*                                                                 */
/*******************************************************************/
UWORD 
ntojis(asrc, spt, ept, adst, cb)
    UBYTE          *asrc;              /* source ptr */
    UWORD           spt, ept;          /* start / end pt */
    UBYTE          *adst;              /* destination pointer */
    ACB             cb;                /* communication block */
{
    register UBYTE  chr;
    register UWORD  i, j, k;
    UBYTE           jcd[5];
    UBYTE           jcdtmp[2];
    UBYTE           c = 4;             /* max character       */
    UWORD           dstl;
    UWORD           st;

    if (((ept - spt) >= 8 && (cb->acbimd & IMDZEN) && *(asrc + spt + 4) == 0x81
         && (*(asrc + spt + 5) == 0x43 || *(asrc + spt + 5) == 0x41))
        || ((ept - spt) >= 4 && (!(cb->acbimd & IMDZEN))
            && *(asrc + spt + 2) == ','))
        c = 5;
    i = spt;
    j = 0;
    while ((i < ept) && (j < c)) {
        if (c == 5 && j == 2) {
            i++;
            j++;
            if (cb->acbimd & IMDZEN)
                ++i;
            continue;
        }
        chr = *(asrc + i);
        if ((chr >= '0') && (chr <= '9'))
            jcd[j] = chr - '0';
        else if ((chr >= 'a') && (chr <= 'f'))
            jcd[j] = chr - 'a' + 10;
        else if ((chr >= 'A') && (chr <= 'F'))
            jcd[j] = chr - 'A' + 10;
        else if (chr == 0x82) {
            i++;
            chr = *(asrc + i);
            if ((chr >= 0x4f) && (chr <= 0x58))
                jcd[j] = chr - 0x4f;
            else if ((chr >= 0x60) && (chr <= 0x65))
                jcd[j] = chr - 0x60 + 10;
            else if ((chr >= 0x81) && (chr <= 0x86))
                jcd[j] = chr - 0x81 + 10;
            else
                return (0);
        }
        else
            return (0);
        i++;
        j++;
    }
    if (j < c)
        return (2);                 /* continue */
    i = (jcd[0] << 4) + jcd[1];
    j = (jcd[c - 2] << 4) + jcd[c - 1];
    /* kuten jis */
    if (c == 5 && (i >= 0x01) && (i <= 0x83) && (j >= 0x01) && (j <= 0x94)) {
        jcdtmp[0] = (jcd[0] * 10) + jcd[1] + 0x20;
        jcdtmp[1] = (jcd[c - 2] * 10) + jcd[c - 1] + 0x20;
        st = egcjscvt(jcdtmp, (UWORD) 2, adst, &dstl);
    }
    /* jis */
    else if (c == 4 && (i >= 0x20) && (i <= 0x7f) && (j >= 0x20) && (j <= 0x7f)) {
        jcdtmp[0] = i;
        jcdtmp[1] = j;
        st = egcjscvt(jcdtmp, (UWORD) 2, adst, &dstl);
    }
    /* sift jis */
    else if (c == 4 && ((i >= 0x81) && (i <= 0x9f)
               || (i >= 0xe0) && (i <= 0xfc)) && (j >= 0x40) && (j <= 0xfc)) {
        k = ((i << 8) + j);
        *adst = k >> 8;
        *(adst + 1) = k;
    }
    else
        return (0);

    return (1);
}
/*******************************************************************/
/*                                                                 */
/* 1  hdmkey : hdml key input                                      */
/*                                                                 */
/*******************************************************************/
UWORD 
hdmkey(chp, cb)
    UBYTE          *chp;               /* --> char(zen/han)               */
    ACB             cb;                /* --> communication block         */
{
    register UBYTE *achar;
    register UBYTE *cpp;               /* -> current position             */
    UWORD           i;
    UWORD          *p;
    UWORD           nbrb;              /* number of charcter byte         */
    UWORD           jst;               /* jis code status                 */
    UBYTE           jcode[4];          /* jis code char recieve area      */
    WORD            st = 0;            /* romaji status                   */
    UWORD           tnbr, cnbr;        /* total/convert number            */
    UBYTE           rmode;
    UBYTE           dhok;
    UBYTE           rstra[16];         /* romaji conversion recieve area  */
    UWORD           iop;

    cb->acbrsp = 0;
    tnbr = 0;
    for (p = cb->acbebp, i = 1; i <= cb->acbtpn; p += 3, ++i) {
        tnbr += (*p) ? ((*(p + 1) > *(p + 2)) ? *(p + 1) : *(p + 2)) : *(p + 1);
    }
    if (tnbr >= cb->acbmik) {
        cb->acbrsp = LIMITOVER;
        return (ERROR);
    }
    cpp = egcsel(cb) + cb->acbsbp;
    achar = chp;
    nbrb = (cb->acbimd & IMDZEN) ? 2 : 1;
    iop = cb->acbcre;
    dhok = FALSE;
    if ((nbrb == 2) && (*achar == 0x81)
        && ((*(achar + 1) == 0x4a) || (*(achar + 1) == 0x4b)) && (cb->acbcre >= 2)) {
        if (*(achar + 1) == 0x4a) {
            dhok = dhcheck(cpp + cb->acbcre - 2, (UBYTE) TRUE);
        }
        else if (*(achar + 1) == 0x4b) {
            dhok = dhcheck(cpp + cb->acbcre - 2, (UBYTE) FALSE);
        }
    }
    if (dhok) {
        iop = cb->acbcre - 2;
    }
    else {
        egcinst(cb->acbcre, achar, nbrb, cb);
        cb->acbcre += nbrb;
    }
    jst = st = 0;
    if (cb->acbimd & IMDJCD) {      /* jis-code conversion */
        jst = ntojis(cpp, cb->acbrmj, cb->acbcre, jcode, cb);
        if (jst == 0) {
            return (NORMAL);
        }
        else if (jst == 1) {
            egcdelt(cb->acbrmj, cb->acbcre - cb->acbrmj, cb);
            if (iop > cb->acbrmj)
                iop = cb->acbrmj;
            egcinst(cb->acbrmj, jcode, 2, cb);
            cb->acbcre = cb->acbrmj + 2;
            cb->acbrmj = cb->acbcre;
        }
        else
            jst = 1;
    }
    if ((cb->acbimd & IMDRMJ) && (jst == 0)
        && (!((*achar == 0x81) && (*(achar + 1) == 0x67)))) {
        if (cb->acbimd & IMDZEN) {
            if (cb->acbimd & IMDHRC)
                rmode = 2;
            else
                rmode = 1;
        }                           /* end of IMDZEN */
        else
            rmode = 0;
        if (cb->acbcre - cb->acbrmj > 10)
            cb->acbrmj = egcpos(DRRIGHT, cb->acbsbp + egcsel(cb),
                                cb->acbcre - 10);
        st = egcrmcvt(cpp + cb->acbrmj, cb->acbcre - cb->acbrmj,
                      rstra, &cnbr, &tnbr, rmode);
        if ((cb->acbmik == 26)      /* add dic/del dic mode */
            &&(tnbr > 2)
            && ((egcsbl(cb) - (cb->acbcre - cb->acbrmj) + tnbr) > 24)) {
            egcdelt(cb->acbrmj, cb->acbcre - cb->acbrmj, cb);
            cb->acbcre = cb->acbrmj;
            cb->acbrsp = LIMITOVER;
            return (ERROR);
        }
        if (st != 0) {
            egcdelt(cb->acbrmj, cb->acbcre - cb->acbrmj, cb);
            if (iop > cb->acbrmj)
                iop = cb->acbrmj;
            egcinst(cb->acbrmj, rstra, tnbr, cb);
            cb->acbcre = cb->acbrmj + tnbr;
            if (st == 1)
                cb->acbrmj = cb->acbcre;
            else if (st == 3)
                cb->acbrmj += cnbr;
        }
    }                               /* end of IMDRMJ */
    else {
        if (jst == 0)
            cb->acbrmj = cb->acbcre;
    }
    cb->acbcrs = iop;
    if (st == 2) {
        cb->acbrsp = NOTROMAJICVT;
        return (ERROR);
    }
    else
        return (NORMAL);
}
/*PAGE*/

/*******************************************************************/
/*                                                                 */
/* 2  reverce convert sub                                          */
/*                                                                 */
/*******************************************************************/
UWORD 
rvcvt(dir, cb)
    UBYTE           dir;               /* direction                    */
    ACB             cb;                /* -> communication block       */
{
    register UBYTE *chp;               /* -> original buffer           */
    register UWORD *cep;               /* -> element  buffer           */
    UWORD           cpn;               /* current phrase number        */
    WORD            st;

    cpn = cb->acbcpn;
    if (dir == DRLEFT)
        cpn--;
    else if (dir == DRRIGHT)
        cpn++;
    if ((!cpn) || (cpn > cb->acbtpn))
        return (NOTCURRENTPHRASE);
    cb->acbcpn = cpn;
    chp = egcoel(cb) + cb->acbobp;
    cep = egccep(cb);
    egcdelt(0, *(cep + 1), cb);
    egcinst(0, chp, *(cep + 2), cb);
    *cep = 0;
    if (dir == DRHOME)
        cb->acbcre = 0;
    else if (dir == DRLAST)
        cb->acbcre = *(cep + 2);
    else if (dir == DRLEFT)
        cb->acbcre += *(cep + 2);

    if (cb->acbcpn > 1 && *(cep - 3) == 0) {
        cb->acbcre += *(cep - 2);
        *(cep + 1) += *(cep - 2);
        *(cep + 2) += *(cep - 1);
        egcmemmove((UBYTE *) (cep - 3), (UBYTE *) cep,
                    (UWORD) ((cb->acbtpn - cb->acbcpn + 1) * 6));
        cb->acbcpn--;
        cb->acbtpn--;
        cep -= 3;
    }
    if (cb->acbtpn > cb->acbcpn && *(cep + 3) == 0) {
        *(cep + 4) += *(cep + 1);
        *(cep + 5) += *(cep + 2);
        egcmemmove((UBYTE *) cep, (UBYTE *) (cep + 3),
                    (UWORD) ((cb->acbtpn - cb->acbcpn) * 6));
        cb->acbtpn--;
    }
    cb->acbrmj = cb->acbcrs = cb->acbcre;
    return (NORMAL);
}
/*PAGE*/

/*******************************************************************/
/*                                                                 */
/* 3  delete a character                                           */
/*                                                                 */
/*******************************************************************/
UWORD 
hdmdel(dir, cb)
    UBYTE           dir;               /* direction                    */
    ACB             cb;                /* -> communication block       */
{
    UWORD           chl, iop;          /* char length                  */
    register UBYTE *sbp;               /* -> scrren buffer             */
    register UWORD *cep;               /* -> element buffer            */

    cb->acbrsp = 0;
    if (egcsbl(cb) == 0) {
        cb->acbrsp = NOCHARCTER;
        return (ERROR);
    }
    cep = egccep(cb) + 1;
    sbp = egcsel(cb) + cb->acbsbp;
    if (dir == DRRIGHT) {           /* if DEL                  */
        if (cb->acbcre < *cep) {
            iop = egcpos((UBYTE) DRRIGHT, sbp, cb->acbcre + 1);
            chl = iop - cb->acbcre;
            egcdelt(cb->acbcre, chl, cb);
        }
    }
    else if (dir == DRLEFT) {       /* if BS                   */
        iop = egcpos((UBYTE) DRLEFT, sbp, cb->acbcre);
        chl = cb->acbcre - iop;
        if (chl > 0) {
            egcdelt(iop, chl, cb);
            cb->acbcre = iop;
            if (cb->acbrmj >= cb->acbcre)
                cb->acbrmj = cb->acbcre;
        }
    }
    else {
        cb->acbrsp = SUBCMDERR;
        return (ERROR);
    }
    if (cb->acbtpn > cb->acbcpn && *cep == 0) {
        egcupdate((UBYTE *) 0, 0, (UBYTE *) 0, 0, 1, 0, (UWORD *) 0, cb);
        if (*cep == 0)
            cb->acbsmd &= ~CSMCEL;
        else
            cb->acbsmd |= CSMCEL;
    }
    cb->acbcrs = cb->acbcre;
    return (NORMAL);
}
/*PAGE*/

/*******************************************************************/
/*                                                                 */
/* 4  move   a character                                           */
/*                                                                 */
/*******************************************************************/
UWORD 
movchar(dir, cb)
    UBYTE           dir;               /* direction                    */
    ACB             cb;                /* -> communication block       */
{
    register UBYTE *iop;               /* -> current screen  buffer    */
    register UWORD *cep;               /* -> current element buffer    */

    iop = cb->acbsbp + egcsel(cb);
    cep = egccep(cb) + 1;
    if (dir == DRHOME) {
        cb->acbcre = 0;
    }
    else if (dir == DRLAST) {
        cb->acbcre = *cep;
    }
    else if (dir == DRRIGHT) {
        if (cb->acbcre < *cep) {
            cb->acbcre = egcpos((UBYTE) DRRIGHT, iop, cb->acbcre + 1);
        }
        else
            return (MOVERANGEOVER);
    }
    else if (dir == DRLEFT) {
        if (cb->acbcre > 0) {
            cb->acbcre = egcpos((UBYTE) DRLEFT, iop, cb->acbcre);
        }
        else
            return (MOVERANGEOVER);
    }
    else if (dir == DRUP) {
        if (cb->acbcre > cb->acbswd - 1) {
            cb->acbcre = egcpos((UBYTE) DRRIGHT, iop,
                                cb->acbcre - (cb->acbswd - 1));
        }
        else {
            cb->acbcre = 0;
        }
    }
    else if (dir == DRDOWN) {
        if (cb->acbcre + cb->acbswd <= *cep) {
            cb->acbcre = egcpos((UBYTE) DRLEFT, iop, cb->acbcre + cb->acbswd);
        }
        else {
            cb->acbcre = *cep;
        }
    }
    cb->acbcrs = cb->acbcre;
    cb->acbrmj = cb->acbcre;
    return (NORMAL);
}
/*PAGE*/

