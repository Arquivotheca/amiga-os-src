#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdclb.c : text convert support procedures for HDML           */
/*                                                                      */
/*              Designed    by  I.Iwata         07/09/1986              */
/*              Coded       by  I.Iwata         07/09/1986              */
/*              Modified    by  K.Nakamura      03/14/1990              */
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
/*      1 UWORD cvttext(mode,cb)                                        */
/*      2 UWORD movphr(dir,cb)                                          */
/*      3 UWORD crrphr(dir,cb)                                          */
/*      4 UWORD cllreq(cb)                                              */
/*      5 int   egcupdate(scr,scrl,org,orgl,oelen,nelen,ele,cb)         */
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
#define ZENKAKU  1      /* ZENNKAKU code	        */
#define HANKAKU  0      /* HANNKAKU code	        */

static VOID    gettrigernbr();

/*PAGE*/

/*------------------------------------------------------------------*/
/*  1   cvttext : convert text                                      */
/*------------------------------------------------------------------*/
UWORD 
cvttext(mode, cb)
    UBYTE           mode;              /* convert mode (0:force 1:free) */
    ACB             cb;                /* -> communication block       */
{
    WORD            st;                /* status                       */
    UBYTE           cvtstr[MAXCHN];    /* conversion receive buffer    */
    UBYTE           rcvstr[MAXCHN];    /* conversion receive buffer    */
    UWORD           rcvele[3 * MAXCLN * 2];     /* receive element buffer       */
    UWORD           rcveln;            /* receive element number       */
    register UWORD  cvtl;              /* convert length               */
    UWORD           rcvl;              /* receive length               */
    UWORD           fixl;              /* fixed convert length         */
    register UWORD *cep;               /* -> current element buffer    */

    cep = egccep(cb);
    if ((cvtl = *(cep + 1)) == 0)
        return (NOCHARCTER);
    if (mode != 0 && mode != 1)
        return (SUBCMDERR);
    egcmemmove(cvtstr, cb->acbsbp + egcsel(cb), cvtl);
    st = -1;
    while (st < 0) {
        st = egctxcvt(cvtstr, cvtl, rcvstr, &rcvl, rcvele, &rcveln, &fixl, mode);
        if (st >= ERTXC01)
            return (st);
        st = hdmerr(st);
    }
    if (mode == 1) {
        if (cvtl > fixl) {
            egcmemmove(rcvstr + rcvl, cvtstr + fixl, cvtl - fixl);
            *(rcvele + rcveln * 3) = *(rcvele + rcveln * 3 + 2) = 0;
            *(rcvele + rcveln * 3 + 1) = cvtl - fixl;
            rcveln++;
            rcvl += (cvtl - fixl);
        }
        else if (cvtl == fixl) {
            *(rcvele + rcveln * 3) = *(rcvele + rcveln * 3 + 1)
                = *(rcvele + rcveln * 3 + 2)
                = 0;
            rcveln++;
        }
    }
    if ((rcvl == 0) || (rcveln == 0)/* @@@ modify 1/5/87 @@@ */
        ||(cb->acbtpn + rcveln >= MAXCLN)
        || (egcsbl(cb) - cvtl + rcvl > cb->acbmik))
        return (LIMITOVER);
    egcupdate(rcvstr, rcvl, cvtstr, cvtl, (UWORD) 1, rcveln, rcvele, cb);
    if (mode == 0) {
        cb->acbcpn += rcveln;
        if (cb->acbcpn > cb->acbtpn) {
            cb->acbtpn++;
            cep = cb->acbebp + (cb->acbtpn - 1) * 3;
            *cep = *(cep + 1) = *(cep + 2) = 0;
        }
        cb->acbcre = 0;
    }
    else if (cvtl == fixl) {
        cb->acbcpn += (rcveln - 1);
        cb->acbcre = 0;
    }
    else {
        cb->acbcpn += (rcveln - 1);
        if (fixl > 0) {
            if (cb->acbcre <= fixl)
                cb->acbcre = 0;
            else if (cb->acbcre = cvtl)
                cb->acbcre = cvtl - fixl;
            else
                cb->acbcre -= fixl;
        }
    }
    cb->acbrmj = cb->acbcre;
    cb->acbcrs = 0;
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/*  2    movphr : move selection phrase                             */
/*------------------------------------------------------------------*/
UWORD 
movphr(dir, cb)
    UBYTE           dir;               /* direction                    */
    ACB             cb;                /* -> communication block       */
{
    cb->acbhyl = 0;
    if (dir == DRHOME) {            /* move home    */
        cb->acbcpn = 1;
        cb->acbcre = 0;
    }
    else if (dir == DRLAST) {       /* move last    */
        cb->acbcpn = cb->acbtpn;
        cb->acbcre = *(egccep(cb) + 1);
    }
    else if (dir == DRLEFT) {       /* move left    */
        if (cb->acbcpn > 1) {
            cb->acbcpn--;
        }
        else
            return (MOVERANGEOVER);
        cb->acbcre = *(egccep(cb) + 1);
    }
    else if (dir == DRRIGHT) {      /* move right */
        if (cb->acbcpn < cb->acbtpn) {
            cb->acbcpn++;
        }
        else
            return (MOVERANGEOVER);
        cb->acbcre = 0;
    }
    else
        return (SUBCMDERR);
    cb->acbrmj = cb->acbcrs = cb->acbcre;
    return (NORMAL);
}
/*PAGE*/
#define PHRASE  2                      /* correction phrase number */
#define MAXPHL  MAXCL*2                /* max phrase length        */

/*------------------------------------------------------------------*/
/*  3     crrphr : correction to current phrase                     */
/*------------------------------------------------------------------*/
UWORD 
crrphr(dir, cb)
    UBYTE           dir;               /* direction                          */
    ACB             cb;                /* -> communication block             */
{
    WORD            st;                /* status                             */
    WORD            stt;               /* status                             */
    WORD            err;               /* status                             */
    register UWORD  i;                 /* counter                            */
    UWORD           ol, sl;            /* screen/original current length     */
    UWORD           rcvl;              /* homonyme recieved length           */
    UWORD           cvtl;              /* convert length                     */
    UWORD           fixl;              /* fixed convert length               */
    UBYTE           dcep;              /* */
    UBYTE           cvtstr[1024];      /* conversion receive buffer          */
    UBYTE           rcvstr[1024];      /* conversion receive buffer          */
    UWORD           rcvele[3 * MAXCL]; /* receive element buffer             */
    UWORD           cvteln, rcveln;    /* convert/receive element number     */
    UWORD           dummycpn;          /* temp. current phrase number        */
    WORDID          dummy1;            /* dummy                              */
    UWORD           dummy2;            /* dummy                              */
    UWORD           sbl;
    UWORD           sel;               /* screen buffer of current element   */
    UWORD           oel;               /* original buffer of current element */
    register UWORD *cep;               /*-> element buffer of current element*/
    UWORD           trigernbr;
    UWORD           dhflg;

    sbl = egcsbl(cb);
    sel = egcsel(cb);
    oel = egcoel(cb);
    cep = egccep(cb);
    sl = *(cep + 1);
    ol = *(cep + 2);

    if (*(cep) == 0)
        return (NOCHCVT);
    if (*(cep + 3) == 0) {
        if (*(cep + 4) > 0) {
            stt = egcmemmove(cvtstr, cb->acbsbp + sel + sl,
                         (UWORD) (*(cep + 4)));
            stt = egcmemmove(rcvstr, cb->acbsbp + sel + sl,
                         (UWORD) (*(cep + 4)));
        }
        rcvele[0] = 0;
        rcvele[1] = rcvele[2] = *(cep + 4);
        cb->acbcpn++;
        egcupdate(cvtstr, *(cep + 4), rcvstr, *(cep + 4), 1, 1, rcvele, cb);
        cb->acbcpn--;
    }
    if (dir == DRRIGHT || dir == DRUP) {
        if (ol >= MAXPHL)
            return (NOCHCVT);
        if (cb->acbcpn == cb->acbtpn)
            return (NOCHCVT);
        if (*(cep + 4) == 0)
            return (NOCHCVT);
        if (zenhanchk(cb->acbobp + oel + ol) == HANKAKU)
            return (NOCHCVT);
        gettrigernbr(cb->acbobp + oel + ol, &trigernbr, &dhflg);
        if (trigernbr == 1 || trigernbr == 2 || trigernbr == 5)
            ol += 2;
        else
            return (NOCHCVT); 
    }
    else if (dir == DRLEFT || dir == DRDOWN) {
        if (ol <= 2)
            return (NOCHCVT);
        if (zenhanchk(cb->acbobp + oel + ol - 2) == HANKAKU)
            return (NOCHCVT);
        gettrigernbr(cb->acbobp + oel + ol - 2, &trigernbr, &dhflg);
        if (trigernbr == 1 || trigernbr == 2 || trigernbr == 5)
            ol -= 2;
        else
            return (NOCHCVT);
    }
    else if (dir == DRHOME) {
        if (*(cep + 3) == 0 && *(cep + 4) == 0)
            return (NOCHCVT);
    }
    if (dir == DRUP || dir == DRDOWN) {
        st = 1;
    }
    else {
        st = -1;
        while (st < 0) {
            st = egcclcvt(cb->acbobp + oel, ol,
                          rcvstr, &dcep, &dummy1, &dummy2, (UBYTE) 0);
            rcvl = (UWORD) dcep;
            if (st >= ERCLC01) {
                st = ERROR;
                break;
            }
            if ((*(cb->acbobp + oel) == 0x82)
                && ((*(cb->acbobp + oel + 1) >= 0xA0)
                    && (*(cb->acbobp + oel + 1) <= 0xEF))) {
                cb->acbebp[(cb->acbcpn - 1) * 3] = ELMAGE;
            }
            st = hdmerr(st);
        }
    }
    if (st == 1) {
        rcvl = ol;
        stt = egcmemmove(rcvstr, cb->acbobp + oel, rcvl);
    }
    if (((sbl + rcvl + 24) > cb->acbmik)        /* @@@ mod 87/09/18 */
        ||(hdmpput(rcvl, rcvstr, cb) == ERROR)) {
        return (LIMITOVER);
    }
    *(cep + 2) = ol;
    dummycpn = cb->acbcpn;
    cb->acbcpn++;
    cep += 3;
    oel += ol;
    cvteln = 1;
    if (dir != DRHOME && *cep == 0) {
        st = 0;
        if (dir == DRRIGHT || dir == DRUP) {
            egcdelt(0, 2, cb);
            *(cep + 2) -= 2;
            cvtl = 0;
            if (*(cep + 1) == 0 && cb->acbcpn < cb->acbtpn)
                st = 1;
        }
        else if (dir == DRLEFT) {
            cvtl = 2;
        }
        else if (dir == DRDOWN) {
            *(cep + 2) += 2;
            cvtl = 2;
        }
    }
    else {
        if (dir == DRRIGHT || dir == DRUP)
            *(cep + 2) -= 2;
        else if (dir == DRLEFT || dir == DRDOWN)
            *(cep + 2) += 2;
        st = 1;
        cvtl = *(cep + 2);
        cep += 3;
        if (dir != DRHOME) {
            for (i = cb->acbcpn + 1; i <= cb->acbtpn; i++, cep += 3) {
                if (i >= cb->acbcpn + PHRASE || *cep == 0
                    || *cep == 0x45 || *cep == 0x46
                    || *cep == 0x48 || *cep == 0x49)
                    break;
                cvtl += *(cep + 2);
                cvteln++;
            }
        }
    }
    if (cvtl > 0) {
        stt = egcmemmove(cvtstr, cb->acbobp + oel, cvtl);
        if (dir == DRUP || dir == DRDOWN) {
            rcvl = 0;
        }
        else {
            err = -1;
            while (err < 0) {
                err = egctxcvt(cvtstr, cvtl, rcvstr, &rcvl,
                               rcvele, &rcveln, &fixl, (UBYTE) 0);
                if (err >= ERTXC01)
                    return (st);
                err = hdmerr(err);
            }
        }
        if (rcvl > 0) {
            if (st)
                egcupdate(rcvstr, rcvl, cvtstr, cvtl, cvteln, rcveln, rcvele, cb);
            else
                egcupdate(rcvstr, rcvl, (UBYTE *) 0, 0, 0, rcveln, rcvele, cb);
            cb->acbcpn += rcveln;
            if (cb->acbcpn > cb->acbtpn) {
                cb->acbtpn++;
                cep = cb->acbebp + (cb->acbtpn - 1) * 3;
                *cep = *(cep + 1) = *(cep + 2) = 0;
            }
        }
        else {
            for (i = 0; i < cvteln; ++i) {
                rvcvt((UBYTE) DRHOME, cb);
                cb->acbcpn++;
            }
        }
    }
    else if (st)
        egcupdate((UBYTE *) 0, 0, (UBYTE *) 0, 0, 1, 0, (UWORD *) 0, cb);

    cb->acbcpn = dummycpn;          /* recover current phrase number */
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/*  4   cllreq   :                                                  */
/*------------------------------------------------------------------*/
UWORD 
cllreq(cb)
    ACB             cb;
{
    UWORD           kl1;
    UWORD           kl2;
    UWORD           yl1;
    UWORD           yl2;
    UWORD           clk;
    UWORD           cly;
    UWORD           i;
    UBYTE          *kTL;
    UBYTE          *yOL;
    UBYTE           clchkbuf[256];
    WORD            st;
    BOOL            SkipFlag;

    kl1 = kl2 = 0;
    yl1 = yl2 = 0;
    clk = cly = 0;
    st = ERROR;
    for (i = 1; i <= cb->acbtpn; i++) {
        if (cb->acbebp[(i - 1) * 3] == ELMAGE) {
            st = NORMAL;
            break;
        }
    }
    if (st == ERROR)
        return (NOELMAGE);
    st = -1;
    st = NORMAL;
    if (st == ERROR)
        return (DISKERR);
    else {
        for (i = 1; i <= cb->acbtpn; i++) {
            SkipFlag = FALSE;
            if (cb->acbebp[(i - 1) * 3] == ELMAGE) {
                SkipFlag = TRUE;
                kTL = cb->acbsbp;
                yOL = cb->acbobp;
                kl1 = cb->acbebp[(i - 1) * 3 + 1];
                yl1 = cb->acbebp[(i - 1) * 3 + 2];
                kTL += clk;
                yOL += cly;
                if ((i != cb->acbtpn)
                    && ((cb->acbebp[i * 3] == CSYUB)
                        || (cb->acbebp[i * 3] == CSYUC)
                        || (cb->acbebp[i * 3] == CSYUG)
                        || (cb->acbebp[i * 3] == ELMAGE))) {
                    kl2 = cb->acbebp[i * 3 + 1];
                    yl2 = cb->acbebp[i * 3 + 2];
                    st = egccllearn(kTL, kl1, kl2, yOL, yl1, yl2);
                }
                else
                    st = egccllearn(kTL, kl1, (WORD) 0, yOL, yl1, (WORD) 0);
            }
            clk += cb->acbebp[(i - 1) * 3 + 1];
            cly += cb->acbebp[(i - 1) * 3 + 2];
            if (SkipFlag && (cb->acbebp[i * 3] == ELMAGE)) {    /* Skip element */
                i++;
                clk += cb->acbebp[(i - 1) * 3 + 1];
                cly += cb->acbebp[(i - 1) * 3 + 2];
            }
        }
    }
    return (st);
}
/*PAGE*/

/*---------------------------------------------------------------*/
/*  5     egcupdate : update text buffers                        */
/*---------------------------------------------------------------*/
egcupdate(scr, scrl, org, orgl, oelen, nelen, ele, cb)
    UBYTE          *scr;               /* pointer to screen strings    */
    UWORD           scrl;              /* screen strings length        */
    UBYTE          *org;               /* pointer to original strings  */
    UWORD           orgl;              /* original strings length      */
    UWORD           oelen;             /* old element number           */
    UWORD           nelen;             /* new element number           */
    UWORD          *ele;               /* pointer to element strins    */
    ACB             cb;                /* communication block          */
{
    WORD            i;                 /* counter                      */
    WORD            st;                /* status                       */
    UWORD           sl = 0;            /* screen length                */
    UWORD           ol = 0;            /* original length              */
    register UWORD *ep;                /* ->element buffer             */
    UWORD           sel;               /* screen buf. current element  */
    UWORD           sbl;               /* screen buffer length         */
    UWORD           oel;               /* original buf. current element */
    UWORD           obl;               /* original buffer length       */
    register UWORD *cep;               /* ->current element buffer     */
    static UWORD    nulele[3] = {0, 0, 0};

    sel = egcsel(cb);
    oel = egcoel(cb);
    sbl = egcsbl(cb);
    obl = egcobl(cb);
    cep = egccep(cb);
    /* clear old buffers imfornation */
    for (i = 0, ep = cep; i < oelen; ++i, ep += 3) {
        sl += *(ep + 1);
        ol += *(ep + 2);
        *ep = *(ep + 1) = *(ep + 2) = 0;
    }
    if (sbl > sel + sl) {
        egcmemmove(cb->acbsbp + sel, cb->acbsbp + sel + sl,
                    sbl - sel - sl);
    }
    sbl -= sl;
    if (obl > oel + ol) {
        egcmemmove(cb->acbobp + oel, cb->acbobp + oel + ol, obl - oel - ol);
    }
    obl -= ol;
    /* mentenace element buffer */
    if (nelen > oelen) {
        if (cb->acbtpn > cb->acbcpn + oelen - 1) {
            egcmemmove((UBYTE *) (cep + nelen * 3),
                        (UBYTE *) (cep + oelen * 3),
                        (UWORD) ((cb->acbtpn - cb->acbcpn - oelen + 1) * 6));
        }
        for (i = 0, ep = cep; i < nelen - oelen; ++i, ep += 3) {
            egcmemmove((UBYTE *) ep, (UBYTE *) nulele, (UWORD) 6);
        }
        cb->acbtpn += (nelen - oelen);
    }
    else if (nelen < oelen) {
        if (cb->acbtpn > cb->acbcpn + oelen - 1) {
            egcmemmove((UBYTE *) (cep + nelen * 3),
                        (UBYTE *) (cep + oelen * 3),
                        (UWORD) ((cb->acbtpn - cb->acbcpn - oelen + 1) * 6));
        }
        cb->acbtpn -= (oelen - nelen);
    }
    /* insert new buffers imfornation */
    if (sbl > sel)
        egcmemmove(cb->acbsbp + sel + scrl, cb->acbsbp + sel, sbl - sel);
    if (obl > oel)
        egcmemmove(cb->acbobp + oel + orgl, cb->acbobp + oel, obl - oel);
    /* copy screen strings to buffer    */
    if (scrl > 0)
        egcmemmove(cb->acbsbp + sel, scr, scrl);
    /* copy original strings to buffer  */
    if (orgl > 0)
        egcmemmove(cb->acbobp + oel, org, orgl);
    /* copy element strings to buffer   */
    if (nelen > 0)
        egcmemmove((UBYTE *) cep, (UBYTE *) ele, nelen * 6);
}

/*--------------------------------------------------------------*/
/*  gettrigernbr(targetch,trigernbr,dhflg)                      */
/*--------------------------------------------------------------*/
static VOID    gettrigernbr(targetch, trigernbr, dhflg) 
UBYTE*  targetch;
UWORD*  trigernbr;
UWORD*  dhflg;
{
    static UBYTE  chrtbl[256] = {
    /*  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
        0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 0 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 1 */
        6, 1, 5, 1, 1, 1, 1, 5, 6, 5, 1, 1, 6, 1, 1, 1,     /* 2 */
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 3 */
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 4 */
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 5, 1, 1,     /* 5 */
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 6 */
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
    static UBYTE   DHHira[88] = {
        /* 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f */
        /* $829F to $82BF */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        /* $82C0 to $82DF */
        0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2,
        0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0,
        /* $82E0 to $82F1 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static UBYTE    DHKana[88] = {
        /* 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f, */
        /* $8340 to $835F */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        /* $8360 to $837F */
        1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0,
        2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0,
        /* $8380 to $8396 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    UBYTE   asc[2];       /* ascii     work area           */
    UWORD   zw;           /* zennkaku  code                */

    if (zenhanchk(targetch) == ZENKAKU) {
        jistoasc(targetch, targetch + 2, asc);
        zw = ((*targetch) << 8) + (*(targetch + 1));
        if (zw == 0X8345)
           *dhflg = 1;                             /*                    */
        else if ((zw >= 0X829F) && (zw < 0X82F2))  /* kanachoku hiragana?  */
           *dhflg = DHHira[zw - 0X829F];
        else if ((zw >= 0X8340) && (zw < 0X8397))  /* kanachoku katakana?  */
           *dhflg = DHKana[zw - 0X8340];
        else
           *dhflg = 0;                             /*    other             */
    } else {
        asc[0] = *targetch;
        *dhflg = 0;
    }

    *trigernbr = (WORD)chrtbl[asc[0]];
}

