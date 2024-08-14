#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdetc.c : text edit support procedures for HDML              */
/*                                                                      */
/*              Designed    by  H.Tayama        07/16/1986              */
/*              Coded       by  H.Tayama        07/16/1986              */
/*              Modified    by  Y.Katogi        03/01/1990              */
/*                                                                      */
/*      (C) Copyright 1986-90 ERGOSOFT Corp.                            */
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
/*      1 UWORD hdmlclear(cb)                                           */
/*      2 UWORD hdmpget(chl,chp,cn)                                     */
/*      3 UWORD hdmpput(chl,*chp,cb)                                    */
/*      4 UWORD cdcvt(mode,cb)                                          */
/*      5 UWORD geteleinf(cb)                                           */
/*      6 UWORD zenncmp(cptr,prest)                                     */
/*      7 UWORD hanncmp(code)                                           */
/*                                                                      */
/************************************************************************/
#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egberr.h"
#include  "egcerr.h"
#ifdef UNIX
#include  "egbhdml.h"
#endif    /* UNIX */

#define egccep(P)       P->acbebp+(P->acbcpn-1)*3

static UWORD    getstrinf();

extern UBYTE    jta[];
#ifndef UNIX_EGK
/****************************************************************/
/*                                                              */
/*      global data area                                        */
/*                                                              */
/****************************************************************/


UWORD           elbff[3 * (MAXCLN + 20)];       /* element buffer        */
UBYTE           scbff[MAXCHN];         /* screen buffer         */
UBYTE           orbff[MAXCHN];         /* original buffer       */
UBYTE           hpbff[MAXHMP];         /* homonym page buffer   */
UBYTE           hebff[MAXHOM];         /* homonym element buf.  */
UBYTE           hmbff[MAXHML];         /* homonym buffer        */
UBYTE           hymbff[MAXCL * 2 + 1]; /* homonym yomi buffer   */
WORDID          widbff[MAXHOM];        /* homonym word id. buf. */
#endif    /* UNIX_EGK */

/****************************************************************/
/*                                                              */
/*  1   hdmlclear : initialinze communication block             */
/*                                                              */
/****************************************************************/
UWORD
#ifdef UNIX_EGK
hdmlclear(cb, egb_hdml)
    ACB             cb;                /* initialize system values */
    EGB_HDML*       egb_hdml;          /* HDML real buffer. */
#else
hdmlclear(cb)
    ACB             cb;                /* initialize system values             */
#endif    /* UNIX_EGK */
{
    cb->acbsmd = 0;                 /* system mode                          */
    cb->acbtpn = 1;                 /* total phrase number                  */
    cb->acbcpn = 1;                 /* current phrase number                */
    cb->acbcrs = 0;                 /* cursor start position                */
    cb->acbcre = 0;                 /* cursor end position                  */
    cb->acbrmj = 0;                 /* romaji start position                */
    cb->acbtpg = 1;                 /* total page number                    */
    cb->acbcpg = 1;                 /* current page number                  */
    cb->acbchm = 1;                 /* current homonym number               */
    cb->acbhyl = 0;                 /* homonym yomi length                  */
    /* initialize buffers                   */
#ifdef UNIX_EGK
    egb_hdml->elbff[0] = egb_hdml->elbff[1] = egb_hdml->elbff[2] = 0;
    egb_hdml->hymbff[0] = 0;
    /* set pointer of buffers               */
    cb->acbebp = egb_hdml->elbff;   /* pointer to element buffer            */
    cb->acbsbp = egb_hdml->scbff;   /* pointer to screen buffer             */
    cb->acbobp = egb_hdml->orbff;   /* pointer to origenal buffer           */
    cb->acbhpp = egb_hdml->hpbff;   /* pointer to homonym page buffer       */
    cb->acbhep = egb_hdml->hebff;   /* pointer to homonym element buffer    */
    cb->acbhbp = egb_hdml->hmbff;   /* pointer to homonym buffer            */
    cb->acbwid = egb_hdml->widbff;  /* pointer to homonym word id. buffer   */
    cb->acbhym = egb_hdml->hymbff;  /* pointer to homonym yomi buffer       */
#else
    elbff[0] = elbff[1] = elbff[2] = 0;
    hymbff[0] = 0;
    /* set pointer of buffers               */
    cb->acbebp = elbff;             /* pointer to element buffer            */
    cb->acbsbp = scbff;             /* pointer to screen buffer             */
    cb->acbobp = orbff;             /* pointer to origenal buffer           */
    cb->acbhpp = hpbff;             /* pointer to homonym page buffer       */
    cb->acbhep = hebff;             /* pointer to homonym element buffer    */
    cb->acbhbp = hmbff;             /* pointer to homonym buffer            */
    cb->acbwid = widbff;            /* pointer to homonym word id. buffer   */
    cb->acbhym = hymbff;            /* pointer to homonym yomi buffer       */
#endif    /* UNIX_EGK */
    return (NORMAL);                /* normal return                        */
}
/*PAGE*/

/****************************************************************/
/*                                                              */
/*  2   hdmpget                                                 */
/*                                                              */
/****************************************************************/
UWORD
hdmpget(chl, chp, cb)
    UWORD          *chl;               /* character counter            */
    UBYTE          *chp;               /* character save adrs top      */
    ACB             cb;                /* communication block          */
{
    WORD            st;

    cb->acbrsp = 0;
    *chl = *(egccep(cb) + 1);
    egcmemmove(chp, cb->acbsbp + egcsel(cb), *chl);
    return (NORMAL);
}

/****************************************************************/
/*                                                              */
/*  3   hdmpput                                                 */
/*                                                              */
/****************************************************************/
UWORD
hdmpput(chl, chp, cb)
    UWORD           chl;               /* character counter            */
    UBYTE          *chp;               /* character save adrs top      */
    ACB             cb;                /* communication block          */
{
    cb->acbrsp = 0;
    if ((egcsbl(cb) + chl - *(egccep(cb) + 1)) > cb->acbmik) {
        cb->acbrsp = LIMITOVER;
        return (ERROR);
    }
    egcdelt(0, *(egccep(cb) + 1), cb);
    egcinst(0, chp, chl, cb);
    return (NORMAL);
}
/*PAGE*/

/****************************************************************/
/*                                                              */
/*  4   cdcvt(mode,cb)                                          */
/*                                                              */
/*      mode                                                    */
/*             0 = asc(hankaku)                                 */
/*             1 = jis(zenkaku hira)                            */
/*             2 = jis(zenkaku kana)                            */
/*             3 = romaji cvt (hankaku)                         */
/*             4 = romaji cvt (zenkaku hira)                    */
/*             5 = romaji cvt (zenkaku kana)                    */
/*                                                              */
/****************************************************************/
UWORD
cdcvt(mode, cb)
    UBYTE           mode;              /* mode                         */
    ACB             cb;                /* -> communication block       */
{
    register UBYTE *sbp;               /* -> screen buffer    */
    UWORD           clen;              /* convert length      */
    UBYTE           work[210];         /* data store area     */
    UBYTE           works[210];        /* data store area     */
    UWORD           ctype;             /* code type           */
    UBYTE          *di;                /* dist pointer        */
    UWORD           crspos;            /* cursol position     */
    UWORD           cnbr;
    UWORD           tnbr;
    UBYTE           rmode;
    WORD            st;

    ctype = geteleinf((UBYTE) 1, cb);   /* code type get        */
    sbp = egcsel(cb) + cb->acbsbp;  /* current screen buffer */
    crspos = *(egccep(cb) + 1) - cb->acbcre;
    if ((ctype >= 4) && (ctype <= 10)) {
        if (mode == 0) {
            st = egcjacvt(sbp, cb->acbcre, work, &clen);
            di = work;
        }
        else if ((mode == 1) || (mode == 2)) {
            st = egcjacvt(sbp, cb->acbcre, work, &clen);
            mode = 3 - mode;
            st = egcajcvt(work, clen, works, &clen, mode);
            di = works;
        }
        else if ((ctype >= 4) && (ctype <= 6) && (mode >= 4))
            rmode = (6 - mode);
        else
            return (NOTCVTCHRTYPE);
    }
    else if ((ctype >= 1) && (ctype <= 3)) {
        if ((mode == 1) || (mode == 2)) {
            mode = 3 - mode;
            st = egcajcvt(sbp, cb->acbcre, work, &clen, mode);
            di = work;
        }
        else if (mode == 3)
            rmode = 0;
        else if (mode == 4 || mode == 5)
            rmode = (6 - mode);
        else
            return (NOTCVTCHRTYPE);
    }
    else
        return (NOTCVTCHRTYPE);
    if (mode >= 3) {
        st = egcrmcvt(cb->acbsbp + egcsel(cb), cb->acbcre,
                      work, &cnbr, &tnbr, rmode);
        if (tnbr == 0)
            return (NOTCVTCHRTYPE);
        di = work;
        clen = tnbr;
    }
    if (egcsbl(cb) + clen - cb->acbcre > cb->acbmik)
        return (LIMITOVER);
    egcdelt(0, cb->acbcre, cb);
    egcinst(0, di, clen, cb);
    cb->acbrmj = cb->acbcre = *(egccep(cb) + 1) - crspos;
    return (NORMAL);                /* normal return */
}
/*PAGE*/

/*********************************************************************/
/*                                                                   */
/*  5   geteleinf(mode,cb)                                           */
/*                                                                   */
/*      mode                                                         */
/*           0: all curent element                                   */
/*           1: until cursol end                                     */
/*                                                                   */
/*                                                                   */
/*      --- get current element type information                     */
/*                                                                   */
/*      f/V                                                          */
/*       1 = all numelic                                 (hankaku)   */
/*       2 = all numelic alph kigou                      (hankaku)   */
/*       3 = all numelic alph kigou kana                 (hankaku)   */
/*                                                                   */
/*       4 = all numelic                                 (zenkaku)   */
/*       5 = all numelic alph                            (zenkaku)   */
/*       6 = all numelic alph kigou                      (zenkaku)   */
/*       7 = all numelic alph kigou hiragana             (zenkaku)   */
/*       8 = all numelic alph kigou katakana             (zenkaku)   */
/*       9 = all numelic alph kigou katakana hiragana    (zenkaku)   */
/*                                                                   */
/*       10= all numelic alph kigou katakana hiragana    (zen/han)   */
/*                                                                   */
/*       0 = error                                                   */
/*                                                                   */
/*********************************************************************/
UWORD   geteleinf(mode,cb)
    UBYTE   mode;
    ACB     cb;         /* communication block       */
{
    UWORD   sblen;      /* screen element length   */

    if (mode) sblen = cb->acbcre;
    else      sblen = *(egccep(cb) + 1);

    if (sblen>100) return(0);  

    return  (getstrinf(cb->acbsbp+egcsel(cb),sblen));
}

/*PAGE*/
/*********************************************************************/
/*                                                                   */
/*  6  zenncmp(aptr,prest)                                           */
/*                                                                   */
/*  f/v  1 = numelic                                                 */
/*       2 = alph                                                    */
/*       3 = kigou                                                   */
/*       4 = hiragana                                                */
/*       5 = katakana                                                */
/*       0 = other                                                   */
/*                                                                   */
/*********************************************************************/
UWORD   zenncmp(str)
    UBYTE*  str;
{
    UWORD   st = 0;

    if ((*str == 0x82) && (*(str + 1) >= 0x4f) && (*(str + 1) <= 0x58)) {
        st = 1;
    }
    else if ((*str == 0x82)
            && ( ((*(str+1) >= 0x60) && (*(str + 1) <= 0x79)) ||
                ((*(str + 1) >= 0x81) && (*(str + 1) <= 0x9a)) )) {
        st = 2;
    }
    else if ( (*str == 0x81)
            && (*(str + 1) >= 0x40) 
            && (*(str + 1) <= 0x9e)
            && (jta[*(str + 1) - 0x40 - ((*(str + 1) >= 0x80)? 1:0)] != 0)) {
        st = 3;
    }
    else if ( (*str == 0x82)
            && (*(str + 1) >= 0x9f)
            && (*(str + 1) <= 0xf1) ) {
        st = 4;
    }
    else if ( (*str == 0x83)
            && (*(str + 1) >= 0x40)
            && (*(str + 1) <= 0x93) ) {
        st = 5;
    }
    return(st);
}

/*********************************************************************/
/*                                                                   */
/*  7  hanncmp(code)                                                 */
/*                                                                   */
/*  f/v  1 = all numelic                                             */
/*       2 = all alph                                                */
/*       3 = all kana                                                */
/*       0 = other                                                   */
/*                                                                   */
/*********************************************************************/
UWORD   hanncmp(code)
    UBYTE   code;
{
    UWORD   st = 0;

    if ((code >= 0x30) && (code <= 0x39))       st = 1;
    else if ((code >= 0x20) && (code <= 0x7e))  st = 2;
    else if ((code >= 0xa1) && (code <= 0xdf))  st = 3;

    return  (st);
}

/*********************************************************************/
/*                                                                   */
/*      getstrinf(str,len)                                           */
/*                                                                   */
/*      --- get current element type information                     */
/*                                                                   */
/*      f/V                                                          */
/*       1 = all numelic                                 (hankaku)   */
/*       2 = all numelic alph kigou                      (hankaku)   */
/*       3 = all numelic alph kigou kana                 (hankaku)   */
/*                                                                   */
/*       4 = all numelic                                 (zenkaku)   */
/*       5 = all numelic alph                            (zenkaku)   */
/*       6 = all numelic alph kigou                      (zenkaku)   */
/*       7 = all numelic alph kigou hiragana             (zenkaku)   */
/*       8 = all numelic alph kigou katakana             (zenkaku)   */
/*       9 = all numelic alph kigou katakana hiragana    (zenkaku)   */
/*                                                                   */
/*       10= all numelic alph kigou katakana hiragana    (zen/han)   */
/*                                                                   */
/*       0 = error                                                   */
/*                                                                   */
/*********************************************************************/
static UWORD   getstrinf(str,len)
    UBYTE*  str;
    UWORD   len;
{
    UWORD   zenncmp();
    UWORD   hanncmp();

    UWORD   zenst=0;
    UWORD   zenprest=0;
    UWORD   hanst=0;
    UWORD   hanprest=0;
    UWORD   st;

    while (len--) {
        if (zenhanchk(str) == ZENKAKU) {
            zenst = zenncmp(str++);
            str++;
            len--;

            if (zenst == 0) {
                return(0);
            }
            else if (zenprest <= 4 && zenst <= 4) {
                if (zenprest <= zenst)
                    zenprest = zenst; 
            }
            else if (zenprest <= 3 || zenprest == 5) {
                if (zenst == 4)
                    zenprest = 6;
                else
                    zenprest = 5;
            }
            else
                zenprest = 6;
        }
        else {
            hanst = hanncmp(*str++);
            if (hanst == 0) {
                return  (0);
            }
            else if (hanprest < hanst) {
                hanprest = hanst;
            }
        }
    }

    if ((zenprest <= 6) && (hanprest == 0)) {
        st = zenprest + 3;
    }
    else if ((hanprest <= 3) && (zenprest == 0)) {
        st = hanprest;
    }
    else if ((hanprest <= 3) && (zenprest <= 6)) {
        st = 10;
    }
    else {
        st = 0;
    }
    return  (st);
}

/*********************************************************************/
/*                                                                   */
/*      zenhanchk(code)                                              */
/*                                                                   */
/*********************************************************************/
UWORD   zenhanchk(code)
    UBYTE*  code;
{
    UWORD   st = HANKAKU;   /*hannkaku */

    if (((*code >= JV1FR ) && (*code <= JV1TO )) ||
        ((*code >= JV2FR ) && (*code <= JV2TO )))
    st = ZENKAKU;           /* zennkaku */

    return  (st);
}

/****************************************************************/
/*                                                              */
/*      addkana(cb)                                             */
/*                                                              */
/****************************************************************/
UWORD   addkana(cb)
    ACB    cb;             /* -> communication block       */
{
    UWORD    i, ii;
    UWORD    sbl;
    UWORD    addlen;
    UBYTE    hbit[4];
    UBYTE    yomi[30];
    UWORD    yomilen;
    UWORD    st;
    UWORD    found = 0;

    sbl = egcsbl(cb);

    i = 0;
    while(i < sbl) {
        if (zenhanchk(cb->acbsbp + i) == HANKAKU) {
            i++;
        }
        else {
            if (zenncmp(cb->acbsbp + i) == 5) {
                found = 1;
                break;
            }
            i += 2;
        }
    }

    if (found == 0)     return  (NOADDWORD);

    i = 0;

    while(i < sbl) {
        if ((getstrinf(cb->acbsbp + i, 2) == 8)
                && (zenncmp(cb->acbsbp+i) == 5)) {
            for (ii = 2; ii < (sbl - i + 2); ii += 2) {
                if (((*(cb->acbsbp + i + ii - 2) != 0x81) ||
                    (*(cb->acbsbp + i + ii - 1) != 0x5b))
                    && ((getstrinf(cb->acbsbp + i, ii) != 8) ||
                        (zenncmp(cb->acbsbp+i+ii-2)!=5)))
                break;
            }
            addlen = ii - 2;
            for (ii = 0; ii < 4; ii++)
                hbit[ii] = 0;
            hbit[0] |= 0x08;        /* meishi2 on   */
            egcjacvt(cb->acbsbp + i, addlen, yomi, &yomilen);
            if (yomilen <= 12) {
                st = egcwdadd(yomi, yomilen, cb->acbsbp + i, addlen, hbit);
            }
            else
                st = 0;
            if (st)
                return  (DICFULLERR);

            i += addlen;
        }
        if (zenhanchk(cb->acbsbp + i) == HANKAKU)   i++;
        else                                        i += 2;
    }
    return  (NORMAL);
}

