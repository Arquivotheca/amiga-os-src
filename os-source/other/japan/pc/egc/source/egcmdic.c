/************************************************************************/
/*                                                                      */
/*      EGCMDIC : EGConvert Making DICtionary library                   */
/*                                                                      */
/*              Designed    by  I.Iwata         02/25/1987              */
/*              Coded       by  I.Iwata         02/25/1987              */
/*              Modified    by  H.Yanata        01/01/1992              */
/*                                                                      */
/*      (C) Copyright 1985-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
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
#include "egcexp.h"
#include "egcudep.h"
#include "egcproto.h"
#include "egcdict.h"

UBYTE           LFILE[32];
UBYTE           wfile[] = "kjdictw.dic";

#ifdef DOS_EGC
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
/*
 * if MS-DOS, don't use egcnewptr in egcmios.c
 */
#define egcnewptr(p) malloc(p)
#define egcfreeptr(p) free(p)
#endif

/*
**  static finctions
*/
static BOOL egcflush(
#ifdef P_TYPE
DICMP   dic
#endif
);
static VOID DicMpInit(
#ifdef P_TYPE
DICMP   dp
#endif
);
static BOOL MakeNullBlock(
#ifdef P_TYPE
UBYTE   ExtCnt,
DICMP   dp
#endif
);

/*--------------------------------------------------------------*/
/*      egccreat : create dictionary                            */
/*--------------------------------------------------------------*/
NONREC
WORD egccreate(newmain, dic)
    UBYTE          *newmain;
    DICMP           dic;
{
	UBYTE			mp[PLALIN];

    egcmemset((UBYTE *) dic, 0x00, sizeof(DICM));
    if ((dic->lf = egcfopen(newmain, CREATMODE)) == (EGCFILE)0)
        goto err_cr_exit;

	egcmemset(mp, 0x00, PLALIN);

    if (egcfwrite(mp, PLALIN, 1, dic->lf) == ERROR)
        goto err_cr_exit;
    if (egcfseek(dic->lf, (LONG) PLALIN, 0) == ERROR)
        goto err_cr_exit;

    if ((dic->wf = egcfopen(wfile, CREATMODE)) == (EGCFILE)0)
        goto err_cr_exit;
    if ((dic->lblk = (UBYTE*)egcnewptr(PLBLKL)) == (UBYTE*)NULL)
        goto err_cr_exit;
    if ((dic->lrec = (UBYTE*)egcnewptr(PLBLKL)) == (UBYTE*)NULL)
        goto err_cr_exit;
    if ((dic->wblk = (UBYTE*)egcnewptr(DSBLKL)) == (UBYTE*)NULL)
        goto err_cr_exit;
    if ((dic->wrec = (UBYTE*)egcnewptr(DSBLKL * 2)) == (UBYTE*)NULL)
        goto err_cr_exit;
    if ((dic->tkbuf = (UBYTE*)egcnewptr((64 + 255) * 2)) == (UBYTE*)NULL)
        goto err_cr_exit;
    dic->preyomi[0] = 0;
    dic->preyomil = 0;
    dic->segno = 0;
    dic->lblen = 0;
    dic->lblkl = 2;
    dic->lbcnt = 0;
    dic->lrecl = 0;
    dic->lrcnt = 0;
    dic->wblkl = 0;
    dic->wrecl = 1;
    dic->homcnt = 0;
    dic->wconcl = 0;
    dic->tkcnt = 0;
    dic->first = TRUE;
    return (NORMAL);
err_cr_exit:
    creatfree(dic);
    return (ERCRE01);               /* main dict create error */
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
static VOID     creatfree(dp)
    DICMP           dp;
/*--------------------------------------------------------------*/
{
    if (dp->lf) {
        (VOID) egcfclose(dp->lf);
        dp->lf = (EGCFILE)0;
    }
    if (dp->wf) {
        (VOID) egcfclose(dp->wf);
        dp->wf = (EGCFILE)0;
    }
    if (dp->lblk) {
        (VOID) egcfreeptr(dp->lblk);
        dp->lblk = NULL;
    }
    if (dp->lrec) {
        (VOID) egcfreeptr(dp->lrec);
        dp->lrec = NULL;
    }
    if (dp->wblk) {
        (VOID) egcfreeptr(dp->wblk);
        dp->wblk = NULL;
    }
    if (dp->wrec) {
        (VOID) egcfreeptr(dp->wrec);
        dp->wrec = NULL;
    }
    if (dp->tkbuf) {
        (VOID) egcfreeptr(dp->tkbuf);
        dp->tkbuf = NULL;
    }
}

/*--------------------------------------------------------------*/
/*      egcwrite : write dictionary                             */
/*--------------------------------------------------------------*/
NONREC
WORD egcwrite(yomi, yomil, kanji, kanjil, hinshi, dic)
    UBYTE          *yomi;
    UWORD           yomil;
    UBYTE          *kanji;
    UWORD           kanjil;
    UBYTE          *hinshi;
    DICMP           dic;
{
    REG             i;
    BOOL            yomibreak;
    register DICMP  dp;
    register UBYTE *wp;
    static UBYTE    hbitx[4] = {0xff, 0xff, 0xff, 0xff};
    UBYTE           krecl;
    UBYTE           kanac;
    UBYTE           kana[32];
    static BOOL     extra = TRUE;
    static UBYTE    ExtCnt = 0;
    /*@@@ Mod 19/05/1992 H.Yanata 
    static UBYTE    ExtPnt[] = {0x21, 0x41, 0x5C, 0xB1};
    static UBYTE    ExtPnt[] = {0x21, 0x41, 0x5C, 0xB1, 0xDD};
    @@@*/
                             /* "."   "?"   "["   "-"   ""  */
    static UBYTE    ExtPnt[] = {0x2E, 0x3F, 0x5B, 0xB0, 0xDD};

    /* begin of egcwrite */
    if (! YStrChk(yomi, yomil))    return  ERWRI02; /* illegal yomi string */

    dp = dic;
    wp = dp->wrec;

    /*@@@ modefied at 28/08/1992 H.Yanata
            for few extra data.          @@@*/
    if (extra && (ExtPnt[ExtCnt] < *yomi)) {       /* make extra yomi block */
        if (dp->first) {
            do {
                if (MakeNullBlock(ExtCnt, dp) == ERROR)
                    return  ERWRI31;
                if (++ ExtCnt > 3) {
                    extra = FALSE;
                    break;
                }
            } while(ExtPnt[ExtCnt] < *yomi);
        } else {
            dp->lrecl += makelrec(dp->dupll, dp->preyomil, dp->preyomi,
                                  dp->lhinshi, dp->lrec + dp->lrecl);
            krecl = makekrec(dp->prekanjil, (UBYTE *) dp->prekanji,
                             hbitx, wp + dp->wconcl);
            dp->wrecl = krecl + dp->wconcl;
            dp->wconcl = dp->wrecl;
            dp->lrcnt++;
            dp->lbcnt += dp->lrcnt;
            egcmemmove(dp->lblk + dp->lblkl, dp->lrec, dp->lrecl);
            egcmemmove(dp->wblk + dp->wblkl, wp, krecl);
            dp->lblkl += dp->lrecl;
            dp->wblkl += dp->wconcl;
            if (++ExtCnt > 3)
                extra = FALSE;
            if (egcflush(dp) == ERROR)
                return  ERWRI31;
            DicMpInit(dp);
            while (ExtPnt[ExtCnt] < *yomi) {
                if (MakeNullBlock(ExtCnt, dp) == ERROR)
                    return  ERWRI31;
                if (++ ExtCnt > 3) {
                    extra = FALSE;
                    break;
                }
            }
        }
    }

    if ((dp->first)
        || ((dp->preyomil == (UBYTE)yomil) 
        && (memncmp(yomi, dp->preyomi, yomil) == 0))) {
        yomibreak = FALSE;
    }
    else {
        yomibreak = TRUE;
    }
    if (yomibreak) {
        if (dp->homcnt == 1) {
            krecl = makekrec(dp->prekanjil, (UBYTE *) dp->prekanji,
                             hbitx, wp + dp->wconcl);
            if ((kanac = *(wp + dp->wconcl + krecl - 2)) <= 2) {
                kanac = (UBYTE) cvtatoj(kanac, dp->preyomi,
                                        dp->preyomil, kana);
                if ((dp->prekanjil != kanac)
                    || (memncmp(dp->prekanji, kana, (UWORD) dp->prekanjil))) {
                    egcmemmove(kana, dp->prekanji, (UWORD) dp->prekanjil);
                    kanac = *(kana + dp->prekanjil - 2);
                    *(kana + dp->prekanjil - 2) = 0;
                    krecl = makekrec(dp->prekanjil, kana, hbitx,
                                     wp + dp->wconcl);
                    *(wp + dp->wconcl + krecl - 2) = kanac;
                }
            }
            if (dp->wconcl + krecl > DSBLKL) {
                return (ERWRI32);
            }
            dp->wrecl = krecl + dp->wconcl;
        }
        else {
            *(wp + dp->wconcl) = (UBYTE) dp->homcnt;
        }
        if (dp->tkcnt > 0) {
            if (dp->tkcnt < 64) {
                *(wp + dp->wrecl++) = (UBYTE) (dp->tkcnt + 64);
            }
            else {
                *(wp + dp->wrecl++) = 128;
                *(wp + dp->wrecl++) = (UBYTE) (dp->tkcnt - 64);
            }
            egcmemmove(wp + dp->wrecl, dp->tkbuf, dp->tkcnt << 1);
            dp->wrecl += dp->tkcnt << 1;
        }
        dp->wconcl = dp->wrecl;
        dp->wrecl++;
        dp->homcnt = 0;
        dp->tkcnt = 0;
        dp->lrecl += makelrec(dp->dupll, dp->preyomil, dp->preyomi,
                              dp->lhinshi, dp->lrec + dp->lrecl);
        dp->lrcnt++;
        if ((dp->wblkl + dp->wconcl >= DSBLKL) ||
            (dp->lblkl + dp->lrecl >= PLBLKL) ||
            (dp->lbcnt == MAXPLE) ||
            (dp->lblkl == 2 &&
             ((dp->lblkl + dp->lrecl >= (PLBLKL - MAXJYOMI * 2)) ||
              (dp->wblkl + dp->wconcl >= DSBLKL - MAXJKANJI * 2)))) {
            if (dp->segno >= MAXDSEG - 1) {
                return (ERWRI14);   /* over segments error  */
            }
            if (dp->wblkl > 0 && dp->lblkl > 2 && dp->lbcnt > 0) {
                if (egcflush(dp) == ERROR)
                    return (ERWRI31);
            }
            else {
                if (dp->lblkl > 2) {
					if (egcflush(dp) == ERROR)
						return (ERWRI31);
                    dp->wblkl = 0;
                    dp->lblkl = 2;
                    dp->lbcnt = 0;
                }
                egcmemmove(dp->lblk + dp->lblkl, dp->lrec, dp->lrecl);
                dp->lblkl += dp->lrecl;
                dp->lrecl = 0;
                dp->lbcnt += dp->lrcnt;
                dp->lrcnt = 0;
                egcmemmove(dp->wblk + dp->wblkl, wp, dp->wconcl);
                dp->wblkl += dp->wconcl;
                dp->wconcl = 0;
                dp->wrecl = 1;
                if (egcflush(dp) == ERROR)
                    return (ERWRI31);
            }
            dp->wblkl = 0;
            dp->lblkl = 2;
            dp->lbcnt = 0;
        }
        if (yomil <= 4) {
            egcmemmove(dp->lblk + dp->lblkl, dp->lrec, dp->lrecl);
            dp->lblkl += dp->lrecl;
            dp->lrecl = 0;
            dp->lbcnt += dp->lrcnt;
            dp->lrcnt = 0;
            egcmemmove(dp->wblk + dp->wblkl, wp, dp->wconcl);
            dp->wblkl += dp->wconcl;
            dp->wconcl = 0;
            dp->wrecl = 1;
        }
    }
    if (yomil > 0 && (dp->first || yomibreak)) {
        i = 0;
        while (dp->preyomi[i] == yomi[i]) {
            if ((yomil > (UWORD) i) && (dp->preyomil > (UBYTE) i)) {
                i++;
            }
            else {
                break;
            }
        }
        dp->dupll = (UBYTE) i;
        egcmemmove(dp->preyomi, yomi, yomil);
        dp->preyomil = (UBYTE) yomil;
        egcmemset(dp->lhinshi, (UBYTE) 0, 4);
    }
    dp->lhinshi[0] |= hinshi[0];
    dp->lhinshi[1] |= hinshi[1];
    dp->lhinshi[2] |= hinshi[2];
    dp->lhinshi[3] |= hinshi[3];
    if (hinshi[0] || hinshi[1] || hinshi[2] || hinshi[3]) {
        if (dp->homcnt >= 63) {
            return (ERWRI12);       /* over homonyms error  */
        }
        if (dp->lbcnt == MAXPLE) {
            return (ERWRI12);       /* over homonyms error       */
        }
        else {
            krecl = makekrec((UBYTE) kanjil, kanji, hinshi, wp + dp->wrecl);
            if ((kanac = *(wp + dp->wrecl + krecl - 2)) <= 2) {
                kanac = (UBYTE) cvtatoj(kanac, yomi, yomil, kana);
                if ((kanjil != kanac)
                    || (memncmp(kanji, kana, (UWORD) kanjil))) {
                    egcmemmove(kana, kanji, (UWORD) kanjil);
                    kanac = *(kana + kanjil - 2);
                    *(kana + kanjil - 2) = 0;
                    krecl = makekrec((UBYTE) kanjil, kana,
                                     hinshi, wp + dp->wrecl);
                    *(wp + dp->wrecl + krecl - 2) = kanac;
                }
            }
            if (dp->wrecl + krecl > DSBLKL) {
				return (ERWRI32);
			}
			dp->wrecl += krecl;
            dp->homcnt++;
            egcmemmove(dp->prekanji, kanji, kanjil);
            dp->prekanjil = (UBYTE) kanjil;
        }
    }
    else if (kanjil != 2) {
        return (ERWRI06);           /* hinshi code error    */
    }
    else if (dp->tkcnt > 64 + 255) {
        return (ERWRI13);           /* over tan-kanji error */
    }
    else {
        dp->tkbuf[dp->tkcnt << 1] = kanji[0];
        dp->tkbuf[(dp->tkcnt << 1) + 1] = kanji[1];
        dp->tkcnt++;
    }
    dp->first = FALSE;

    return (NORMAL);
}

/*--------------------------------------------------------------*/
/*      egceof : end of dictionary file                         */
/*--------------------------------------------------------------*/
NONREC
WORD egceof(dic)
    DICMP           dic;
{
    extern UBYTE   *grtp;
    register DICMP  dp;
    UWORD           i;
    LONG            j = 0L;
    static UBYTE    hbitz[4] = {0x00, 0x00, 0x00, 0x00};
    MDHD            mhead;
    WORD            ret;

    dp = dic;
    if ((ret = egcwrite(NULL, 0, NULL, 0, hbitz, dp)) != ERWRI06 &&
        ret != NORMAL)
        goto err_w_exit;
    if (egcflush(dp) == ERROR)
        goto err_w_exit;
    egcmemset((UBYTE *) & mhead, 0x00, sizeof(MDHD));
    mhead.mhsegq = (UBYTE) (dp->segno & 0xFF);
    mhead.mhsegqh = (UBYTE) (dp->segno >> 8);
    vers_ltoc(egcgetsysverno(), mhead.mhstrver);

    /*--- Calculate PL offset and set structure ---*/
    setint(mhead.mhploff, PLALIN / 16);
    /*--- Calculate DS offset and set structure ---*/
    j = (LONG) (dp->lblen + 1) * (PLALIN / 16);
    j += (LONG) (DSBLKL / 16) - 1;
    j /= (LONG) DSBLKL / 16;
    j *= (LONG) DSBLKL / 16;
	i = (UWORD) j;
    setint(mhead.mhdsoff, i);
    /*--- Write PL & DS to main dict ---*/
    egcmemset(dp->lblk, (UBYTE) 0, PLBLKL);
    if (egcfwrite(dp->lblk, (REG) ((j - dp->lblen) * 16), 1, dp->lf)
        == ERROR)
        goto err_w_exit;
    if (egcfseek(dp->lf, (LONG) j << 4, 0) == ERROR)
        goto err_w_exit;
    if (egcfseek(dp->wf, (LONG) 0, 0) == ERROR)
        goto err_w_exit;
    for (i = dp->segno; i--;) {
        if (egcfread(dp->wblk, DSBLKL, 1, dp->wf) == ERROR)
            goto err_w_exit;
        if (egcfwrite(dp->wblk, DSBLKL, 1, dp->lf) == ERROR)
            goto err_w_exit;
    }
    /*--- Calculate GT offset and set structure ---*/
    j += (LONG) dp->segno * (DSBLKL / 16);
	i = (j >= 0x0000FFFF) ? 0xFFFF : (UWORD) j;
    setint(mhead.mhgtoff, i);
    /*--- Write GT to main dict ---*/
    if (egcfseek(dp->lf, (LONG) j << 4, 0) == ERROR)
        goto err_w_exit;
    if (egcfwrite(grtp, 1, getint(grtp + FHLAST * 2), dp->lf)
        == ERROR)
        goto err_w_exit;
    /*--- Write HEADER to main dict ---*/
    if (egcfseek(dp->lf, (LONG) 0, 0) == ERROR)
        goto err_w_exit;
    if (egcfwrite((UBYTE *) & mhead, PLALIN, 1, dp->lf) == ERROR)
        goto err_w_exit;

    creatfree(dp);
    egcfdel(wfile);

    return (NORMAL);

err_w_exit:
    creatfree(dp);
    egcfdel(wfile);
    return (ERROR);
}

/*--------------------------------------------------------------*/
/*      egcflush : internal routine for flush buffer            */
/*--------------------------------------------------------------*/
NONREC static
BOOL            egcflush(dic)
    DICMP           dic;
{
    register DICMP  dp;

    dp = dic;
    egcmemset(dp->wblk + dp->wblkl, (UBYTE) 0, DSBLKL - dp->wblkl);
    if (egcfwrite(dp->wblk, DSBLKL, 1, dp->wf) == ERROR)
        return (ERROR);
    setint(dp->lblk, dp->lbcnt);
    egcmemset(dp->lblk + dp->lblkl, (UBYTE) 0, PLBLKL - dp->lblkl);
    dp->lblkl = ((dp->lblkl + PLALIN - 1) / PLALIN) * PLALIN;
    if (egcfwrite(dp->lblk, dp->lblkl, 1, dp->lf) == ERROR)
        return (ERROR);
    dp->lblen += dp->lblkl / PLALIN;
    dp->segno++;
    return (NORMAL);
}

/*--------------------------------------------------------------*/
/*      egcndic : new user dictionary                           */
/*--------------------------------------------------------------*/
NONREC
WORD egcndic(mpath, upath, plosize, sltvalue)
    UBYTE          *mpath;
    UBYTE          *upath;
    UWORD           plosize;           /* input unit byte */
    UBYTE           sltvalue;
{
    EGCFILE         usrfp = (EGCFILE)0;
    UBYTE          *p = NULL;
    UBYTE          *q;
    UBYTE          *indexblk = NULL;
    PUD             puhead = NULL;
    UWORD           i;
    UWORD           segq, idxq, sltq, ploq, dsoq;

#ifndef MAC
    UBYTE           fullpath[MAXPATH];
    extern REG      egcerrno;
    MDICT_INFO      newuser_dict;
    PTR_MDICT       pdarry;

    pdarry = dicstat[0].pds = &newuser_dict;
    pdarry->fp = 0;
    sprintf(fullpath, "./%s", mpath);
    strcpy(pdarry->path, fullpath);
    if (openmseg(0, pdarry) != NORMAL)
        return (ERROR);
#else
    extern DICIDTBL dicstat[MAXOPENDIC];
    extern UBYTE    iobp[2][DSBLKL + 2];
    PTR_MDICT       pdarry;
	UBYTE			Ppath[64];

	pdarry = dicstat[0].pds;

	strcpy(Ppath, mpath);
	c2pstr(Ppath);
	egcmemmove(&(pdarry->path), Ppath, Length(Ppath) + 1);
	if (openmseg(0, pdarry) != NORMAL)
        return (ERROR);
	pdarry->iobp = &iobp[0][0];
	if (readmseg(0, pdarry) != NORMAL) {
		closemseg(0, pdarry);
        return (ERROR);
	}
#endif

    if ((pdarry->hbsegq == 0) || (getint(getlseg(0, 0, (PLBLKL / 16))) == 0))
        goto err_exit;
 
    /*--- Make GI ---*/
    if ((indexblk = (UBYTE *) egcnewptr(GETIDXSIZE(MAXDSEG))) == NULL)
        goto err_exit;
    if ((puhead = (PUD) egcnewptr(UDBLKL)) == NULL)
        goto err_exit;

    egcmemset((UBYTE *) indexblk, 0x00, GETIDXSIZE(MAXDSEG));
    egcmemset((UBYTE *) puhead, 0x00, UDBLKL);
    MakeGIndex(indexblk, pdarry->hbsegq);
    MakeUIndex(puhead, indexblk, pdarry->hbsegq);
    MakeHeader(puhead, sltvalue, plosize, pdarry->hbsegq);
    segq = pdarry->hbsegq;
    idxq = UDBLKL * getint(puhead->ueidxl);
    sltq = UDBLKL * getint(puhead->uesltl);
    ploq = UDBLKL * getint(puhead->ueplol);
    dsoq = DSOBLKL * segq;

    if ((usrfp = egcfopen(upath, CREATMODE)) == (EGCFILE)0)
        goto err_exit;
    if (egcfwrite((UBYTE *) puhead, 1, UDBLKL, usrfp) == ERROR)
        goto err_exit;
    (VOID) egcfreeptr((UBYTE *) puhead);
    puhead = NULL;

    if (egcfwrite(indexblk, 1, idxq, usrfp) == ERROR)
        goto err_exit;
    (VOID) egcfreeptr(indexblk);
    indexblk = NULL;

    if ((p = (UBYTE *) egcnewptr(sltq)) == NULL)
        goto err_exit;
    egcmemset(p, 0, sltq);
    if (egcfwrite(p, 1, sltq, usrfp) == ERROR)
        goto err_exit;
    (VOID) egcfreeptr(p);
    p = NULL;

    if ((p = (UBYTE *) egcnewptr(ploq)) == NULL)
        goto err_exit;
    egcmemset(p, 0, ploq);
    if (egcfwrite(p, 1, ploq, usrfp) == ERROR)
        goto err_exit;
    (VOID) egcfreeptr(p);
    p = NULL;

    if ((q = p = (UBYTE *) egcnewptr(dsoq)) == NULL)
        goto err_exit;
    egcmemset(p, 0, dsoq);
    for (i = 0; i < segq; i++) {
        setint(q, 2);
        q += DSOBLKL;
    }
    if (egcfwrite(p, 1, dsoq, usrfp) == ERROR)
        goto err_exit;
    (VOID) egcfreeptr(p);
    p = NULL;

    closemseg(0, pdarry);
    if (usrfp) {
        (VOID) egcfclose(usrfp);
        usrfp = (EGCFILE)0;
    }
    return (NORMAL);

err_exit:
    if (puhead) {
        (VOID) egcfreeptr((UBYTE *) puhead);
        puhead = NULL;
    }
    if (indexblk) {
        (VOID) egcfreeptr(indexblk);
        indexblk = NULL;
    }
    if (p) {
        (VOID) egcfreeptr(p);
        p = NULL;
    }
    if (usrfp) {
        (VOID) egcfclose(usrfp);
        usrfp = (EGCFILE)0;
    }
    closemseg(0, pdarry);
    return (ERROR);
}                                      /* end of egcndic */


/***********************************************************/
NONREC
VOID MakeGIndex(indexblk, msegq)
    UBYTE          *indexblk;
    UWORD           msegq;
/***********************************************************/
{
    REG             dupll, uniql;
    PIE             iep;
    UBYTE           yomilen;
    UBYTE          *plsegp, *lp, *p;
    UBYTE           yomi[12];
    UWORD           plmark;
    UWORD           dsn;
    UWORD           i, yomiq;

    /*--- Make GI ---*/
    egcmemset(indexblk, 0, (UWORD) GETIDXSIZE(msegq));
    iep = (PIE) indexblk;
    plmark = 0;
    for (dsn = 0; dsn < msegq; dsn++) {
        plsegp = getlseg(0, plmark, (PLBLKL / 16));
        yomiq = getint(plsegp);
        p = readpwrd(plsegp + 2, &dupll, &uniql);
        egcmemmove(yomi + dupll, p, uniql);
        yomilen = (UBYTE) (dupll + uniql);
        lp = p + uniql;
        egcmemset(iep->ieyomi, 0, 4);
        egcmemmove(iep->ieyomi, yomi, yomilen);
        setint(iep->ieplp, plmark);
        setint(iep->ieplop, (UWORD) dsn * 2);
        i = yomiq - 1;
        while (i--) {
            p = readpwrd(lp, &dupll, &uniql);
            egcmemmove(yomi + dupll, p, uniql);
            yomilen = (UBYTE) (dupll + uniql);
            lp = p + uniql;
        }
        plmark += (((lp - plsegp) + PLALIN - 1) / PLALIN);
        iep++;
    }
    egcmemset(iep->ieyomi, 0, 4);
    iep->ieyomi[0] = 0xFF;
    setint(iep->ieplp, plmark);
    setint(iep->ieplop, msegq * (UWORD) 2);
}
/*PAGE*/
/***********************************************************/
NONREC
VOID MakeUIndex(puhead, indexblk, msegq)
    PUD             puhead;
    UBYTE          *indexblk;
    UWORD           msegq;
/***********************************************************/
{
    UWORD           i, j;
    PIE             iep;
    UBYTE           c;

/*--- Calculate & make UI  ---*/
    egcmemset((UBYTE *) puhead, 0, UDBLKL);
    for (i = 0, iep = (PIE) indexblk; i < msegq; i++, iep++) {
        c = iep->ieyomi[0];
        if (YTypeChk(&c) == EXTRA_YOMI) {
            continue;
        }
        if (iep->ieyomi[1]) {
            c++;
        }
        for (j = c - 0xB1; j < 0xDD - 0xB1; j++) {
            puhead->uduidx[j] = (i > (UWORD)0xFF)?    (UBYTE)0xFF : (UBYTE)i;
        }
    }
}
/*PAGE*/
/***********************************************************/
NONREC
VOID MakeHeader(puhead, sltvalue, plosize, segval)
    PUD             puhead;
    UWORD           plosize;           /* input unit byte */
    UBYTE           sltvalue;
    UWORD           segval;
/***********************************************************/
{
    UWORD           idxsz, sltsz;
    UWORD           eidxs, eidxl, eslts, esltl, eplos, eplol, edsos;

    if (plosize < PLOBLKL)
        plosize = PLOBLKL;
    else if (plosize % UDBLKL)
        plosize = (plosize / UDBLKL + 1) * UDBLKL;

    if (sltvalue < SLTDFL)
        sltvalue = SLTDFL;
    else if (sltvalue > MAXSLTVAL)
        sltvalue = MAXSLTVAL;

    idxsz = (((segval + 1) * sizeof(GIEREC) + UDBLKL - 1) / UDBLKL) * UDBLKL;
    sltsz = ((segval * sltvalue * 2 + UDBLKL - 1) / UDBLKL) * UDBLKL;
    eidxs = 1;
    eidxl = idxsz / UDBLKL;
    eslts = eidxs + eidxl;
    esltl = sltsz / UDBLKL;
    eplos = eslts + esltl;
    eplol = plosize / UDBLKL;
    edsos = eplos + eplol;

    puhead->udidxs = (UBYTE) eidxs;
    puhead->udidxl = (UBYTE) eidxl;

    if ((plosize < 0x8000) && (sltvalue < MAXSLTVAL)) {
        puhead->udslts = (UBYTE) eslts;
        puhead->udsltl = (UBYTE) esltl;
        puhead->udplos = (UBYTE) eplos;
        puhead->udplol = (UBYTE) eplol;
        puhead->uddsos = (UBYTE) edsos;
    }
	else {
		puhead->udslts = 255;
		puhead->udsltl = 255;
		puhead->udplos = 255;
		puhead->udplol = 255;
		puhead->uddsos = 255;
	}
	setint(puhead->udexts, edsos + (segval * (DSOBLKL / UDBLKL)));
    setint(puhead->ueidxs, eidxs);
    setint(puhead->ueidxl, eidxl);
    setint(puhead->ueslts, eslts);
    setint(puhead->uesltl, esltl);
    setint(puhead->ueplos, eplos);
    setint(puhead->ueplol, eplol);
    setint(puhead->uedsos, edsos);
    if (segval < (UWORD) 0x00FF) {
        puhead->udsegq = (UBYTE) segval;
    }
    else {
        puhead->udsegq = 0xFF;
    }
	vers_ltoc(egcgetsysverno(), puhead->udstrver);
	vers_ltoc(egcgetmaindicver(NULL), puhead->uddicver);
    setint(puhead->uesegq, segval);
    setint(puhead->udmaxslt, sltvalue);
}

/***********************************************************/
NONREC
VOID egcfdel(wfile)
    UBYTE          *wfile;
/***********************************************************/
{
    unlink(wfile);
}

/*--------------------------------------------------------------*/
/*  DicMpInit : Initialize dictionary making recode             */
/*--------------------------------------------------------------*/
static VOID DicMpInit(dp)
    DICMP   dp;
{
    dp->preyomi[0] = 0;
    dp->preyomil = 0;
    dp->lblkl = 2;
    dp->lbcnt = 0;
    dp->lrecl = 0;
    dp->lrcnt = 0;
    dp->wblkl = 0;
    dp->wrecl = 1;
    dp->homcnt = 0;
    dp->wconcl = 0;
    dp->tkcnt = 0;
    dp->first = TRUE;
 }

/*--------------------------------------------------------------*/
/*  MakeNullBlock : Making NULL PL block                        */
/*--------------------------------------------------------------*/
static BOOL MakeNullBlock(ExtCnt, dp)
    UBYTE   ExtCnt;
    DICMP   dp;
{
    static struct _lbl {
        UBYTE   ym[MAXJYOMI];
        UBYTE   kj[MAXJKANJI];
        UBYTE   hn[4];
    } label[] = {
        {"!", "üI"},
        {"/", "ü^"},
        {"@", "üù"},
        {"\\", "üÅ"},
    };
    static UBYTE    lblhn[4] = {0x00, 0x01, 0x00, 0x00};
    register UBYTE* wp = dp->wblk;
    register UBYTE* lp = dp->lblk;

    dp->lbcnt = 1;
    dp->lrcnt = 1;
    dp->lblkl += makelrec(0, 1, label[ExtCnt].ym, lblhn, lp + dp->lblkl);
    *(wp + dp->wblkl) = 0xF1;
    egcmemmove(wp + dp->wblkl + 1, label[ExtCnt].kj, 2);
    dp->wblkl += 3;
    *(dp->lblk) = 1;
    if (egcflush(dp) == ERROR) {
        return  ERROR;
    }
    DicMpInit(dp);

    return  NORMAL;
}


