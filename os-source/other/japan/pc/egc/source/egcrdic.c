/************************************************************************/
/*                                                                      */
/*      EGCRDIC : EGConvert Read DICtionary library                     */
/*                                                                      */
/*              Designed    by  I.Iwata         11/08/1986              */
/*              Coded       by  I.Iwata         11/08/1986              */
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
/*----------------------------------------------------------------------*/
/*  included functions                                                  */
/*                                                                      */
/*      1  st=egcfind(yomi,yomil,smode,dmode,dic)                       */
/*      2  st=egcnext(dic)                                              */
/*      3  st=egcread(yomi,yomil,kanji,kanjil,hinshi,dic)               */
/*                                                                      */
/************************************************************************/
#ifdef    UNIX
#include <stdio.h>
#endif
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcexp.h"
#include "egcudep.h"
#include "egcproto.h"
#include "egcdict.h"

REG             egcplock = TRUE;
static int      not_skip_plo = FALSE;
/*@@@_HY
Next 2 variables are added on 1993/03/02,
becouse if egcnext is called, lost PL/PLO data.
HY_@@@*/
UBYTE           plop[DSBLKL];
UBYTE           dsop[DSBLKL];
/*@@@*/

/*-----------------------------------------------------------*/
/*   egcfind :  find dictionary word                         */
/*-----------------------------------------------------------*/
NONREC
WORD egcfind(yomi, yomil, smode, dmode, dic)
    UBYTE          *yomi;
    UWORD           yomil;
    UBYTE           smode;
    UBYTE           dmode;
    DICRP           dic;
{
    UWORD           segn;
    WORD            st;
    register PIE    iep;
    register DICRP  dp;
    static WORD     NextErr = FALSE;
    static WORD     Err25 = FALSE;
    if (YTypeChk(yomi) & (ILLEGAL_YOMI | NEUTRAL_YOMI))
        return (ERFND02);           /* yomi string error */
    if ((yomil == 0) || (yomil > MAXJYOMI))
        return (ERFND03);           /* yomi length error */
    dp = dic;
    dp->smode = smode;
    dp->dmode = dmode;
    if (dp->dmode == 0 || dp->dmode == 1) {
        dp->segq = getint(dicprty[0].pds->hbuhdr.uesegq);
        if (YTypeChk(yomi) == EXTRA_YOMI)
            segn = 0;
        else
            segn = dicprty[0].pds->hbuhdr.uduidx[(UINT) (*yomi - 0xB1)];
        iep = dicprty[0].pds->hbuhdr.udidxp + segn;
    }
    else {
        dp->segq = dicstat[dp->dmode - 1].pds->hbsegq;
        if (YTypeChk(yomi) == EXTRA_YOMI)
            segn = 0;
        else
            segn = dicstat[dp->dmode-1].pds->hbuhdr.uduidx[(UINT)(*yomi-0xB1)];
        iep = dicstat[dp->dmode - 1].pds->hbuhdr.udidxp + segn;
    }
    dic->yomil = (UBYTE) yomil;
    egcmemmove(dp->yomi, yomi, yomil);
    egcmemset(dic->yomi + dic->yomil, 0, MAXJYOMI - dic->yomil);
    segn++;
    if (memncmp(dp->yomi, iep->ieyomi, 4) >= 0) {
        while (memncmp(dp->yomi, iep->ieyomi, 4) >= 0) {
            iep++, segn++;
        }
        iep--, segn--;
    }
    while (segn > 1 && memncmp(iep[-1].ieyomi, iep[0].ieyomi, 4) == 0) {
        UBYTE           iyomi[MAXJYOMI], cur_yomi[MAXJYOMI];
        UBYTE          *plmpt, *p;
        REG             dupll, uniql;
        plmpt = getlseg((dp->dmode == 0 || dp->dmode == 1) ? 0 : dp->dmode - 1,
                        getint(iep[0].ieplp),
                        getint(iep[1].ieplp) - getint(iep[0].ieplp));
        plmpt += 2;
        p = readpwrd(plmpt, &dupll, &uniql);
        egcmemset(cur_yomi, 0x00, MAXJYOMI);
        egcmemmove(cur_yomi, iep->ieyomi, dupll);
        egcmemmove(cur_yomi + dupll, p, uniql);
        egcmemset(iyomi, 0x00, MAXJYOMI);
        egcmemmove(iyomi, yomi, yomil);
        if (memncmp(cur_yomi, iyomi, MAXJYOMI) <= 0)
            break;
        segn--, iep--;
    }
    dp->segno = segn;
    egcmemmove(dp->yomi, iep->ieyomi, sizeof(iep->ieyomi));
    dp->yomi[sizeof(iep->ieyomi)] = 0;
    dp->recno = 0;
    dp->recq = 0;
    while (TRUE) {
        if ((NextErr = egcnext((DICRP) dp)) && (NextErr != ERNXT25)) {
            break;
        }
        if (NextErr == ERNXT25) {
            Err25 = TRUE;
            continue;
        }
        st = memncmp(dp->yomi, yomi, yomil);
        if (dp->smode) {
            if (yomil == dp->yomil) {   /* not found error */
                if (st >= 0) {
                    return ((st) ? ERFND01 : (Err25) ? ERNXT25 : FALSE);
                }
            }
        }
        else if (st >= 0) {
            return ((Err25) ? ERNXT25 : FALSE);
        }
    }
    return ((Err25) ? ERNXT25 : ERFND01);
}

/*-----------------------------------------------------------*/
/*   egcnext :  serach next dictionary word                  */
/*-----------------------------------------------------------*/
NONREC
WORD egcnext(dic)
    DICRP           dic;
{
    register DICRP  dp;
    register UBYTE *wp;
    PIE             iep;
    REG             uniql;
    REG             dupll;
    UBYTE          *p;
    UBYTE           c;
    WORD            PloOk;
    WORD            ErrorID = FALSE;

    not_skip_plo = FALSE;
    dp = dic;
    if (dp->segno > (dp->segq + 1))
        return (ERNXT01);           /* not found error */
    while (TRUE) {
        PloOk = 1;
        if (dp->recno >= dp->recq) {
            iep = (dp->dmode <= 1) ?
                dicprty[0].pds->hbuhdr.udidxp + (int) (dp->segno - 1) :
                dicstat[dp->dmode - 1].pds->hbuhdr.udidxp +
                (int) (dp->segno - 1);
            for (; PloOk = CheckPLOSeg((DICRP) dp); iep++, dp->segno++) {
                if (dp->segno > dp->segq) {
                    return (ERNXT01);   /* not found error */
                }
                if (dp->dmode == 0) {
                    p = getlseg(0, getint(iep->ieplp), PLBLKL / 16);
                } else {
                    if (dp->dmode == 1) {
#ifdef  PMPLO
                        p = GetPlop(dicprty[0].pds, dp->segno);
#else
                        p = dicprty[0].pds->hbuhdr.udplop+getint(iep->ieplop);

#endif
                    } else {
                        p = getlseg(dp->dmode-1,getint(iep->ieplp),PLBLKL/16);
                    }
                }
/*@@@_HY add on 1993/03/02, becouse lost PL/PLO data HY_@@@*/
			    egcmemmove(plop, p, DSBLKL);
				dp->plp = plop;
/*@@@*/
                dp->recq = getint(plop);
                dp->recno = 0;
                if (dp->recq > 0) {
                    egcmemmove(dp->yomi, iep->ieyomi, sizeof(iep->ieyomi));
                    dp->plp = plop + 2;
                    dp->dsp = (dp->dmode == 0) ?
                        getdsrec(0, dp->segno, (UWORD) 1) :
                        ((dp->dmode == 1) ?
                         getdsorec(dp->segno, (UWORD) 1) :
                         getdsrec(dp->dmode - 1, dp->segno, (UWORD) 1));
                    iep++, dp->segno++;
/*@@@_HY add on 1993/03/02, becouse lost PL/PLO data HY_@@@*/
                    egcmemmove(dsop, dp->dsp, DSBLKL);
                    dp->dsp = dsop;
/*@@@*/
                    break;
                }
            }
        }
        if (!PloOk) {
            ErrorID = ERNXT25;
            if (not_skip_plo == TRUE)
                return (ErrorID);
            continue;
        }
        p = readpwrd(dp->plp, &dupll, &uniql);
        dp->yomil = (UBYTE) (dupll + uniql);
        egcmemmove(dp->yomi + dupll, p, uniql);
        egcmemset(dp->yomi + dp->yomil, 0, MAXJYOMI - dp->yomil);
        egcmemmove(dp->hinshi, readplh(dp->plp), 4);
        dp->plp = p + uniql;
        dp->recno++;
        if (((dp->dmode) == 0) ||
            (dp->hinshi[0] != 0) ||
            (dp->hinshi[1] != 0) ||
            (dp->hinshi[2] != 0) ||
            (dp->hinshi[3] != 0))
            break;
    }
    wp = dp->dsp;
    if (dp->dmode == 0 || dp->dmode >= 2) {
        if (*wp >= 0xC0) {
            dp->dbwq = 1;
        }
        else {
            dp->dbwq = *(wp++);
        }
        wp = skipkanji(dp->dbwq, wp);
        switch (*wp & 0xC0) {
        case 64:
            c = *(wp++) - (UBYTE)64;
            break;
        case 128:
            wp++;
            c = *(wp++) + (UBYTE)64;
            break;
        default:
            c = 0;
        }
        wp += (UWORD) c << 1;
        dp->dbws = 0;
        dp->dbks = 0;
    }
    else {
        dp->dbwq = *(wp++);
        wp = skipkanji(dp->dbwq, wp);
        dp->dbws = 0;
    }
    if (dp->recno > 1)
        dp->dsp = wp;
    return (ErrorID);
}

/*-----------------------------------------------------------*/
/*   egcread :  read dictionary word                         */
/*-----------------------------------------------------------*/
NONREC
WORD egcread(yomi, yomil, kanji, kanjil, hinshi, dic)
    UBYTE          *yomi;
    UWORD          *yomil;
    UBYTE          *kanji;
    UWORD          *kanjil;
    UBYTE          *hinshi;
    DICRP           dic;
{
    register DICRP  dp;
    register UBYTE *wp;
    UBYTE           kanjils;
    WORD            st;

    dp = dic;
    if (dp->segno > (dp->segq + 1)) {
        return (ERRED01);           /* dict read error */
    }
    egcmemmove(yomi, dp->yomi, dp->yomil);
    *yomil = dp->yomil;
    wp = dp->dsp;
    if (dp->dmode == 0 || dp->dmode >= 2) {     /* Main or expert dict */
        if (*wp >= 0xC0) {
            dp->dbwq = 1;
        }
        else {
            dp->dbwq = *(wp++);
        }
        if (dp->dbws < dp->dbwq) {
            readjwrd(skipkanji(dp->dbws, wp),
                     dp->yomil, yomi, &kanjils, kanji, hinshi);
            *kanjil = (UWORD) kanjils;
            dp->dbws++;
            st = FALSE;
        }
        else {
            wp = skipkanji(dp->dbwq, wp);
            switch ((*wp) & 0xC0) {
            case 64:
                dp->dbkq = (UWORD) * (wp++) - 64;
                break;
            case 128:
                wp++;
                dp->dbkq = (UWORD) * (wp++) + 64;
                break;
            default:
                dp->dbkq = 0;
            }
            if (dp->dbks < dp->dbkq) {
                wp += (UWORD) (dp->dbks << 1);
                *kanjil = 2;
                *kanji = *(wp++);
                *(kanji + 1) = *wp;
                egcmemset(hinshi, (UBYTE) 0x00, 4);
                dp->dbks++;
                st = FALSE;
            }
            else {
                st = ERRED01;       /* dict read error */
            }
        }
    }
    else {                          /* User dict */
        dp->dbwq = *(wp++);
        if (dp->dbws < dp->dbwq) {
            readjwrd(skipkanji(dp->dbws, wp),
                     dp->yomil, yomi, &kanjils, kanji, hinshi);
            *kanjil = (UWORD) kanjils;
            dp->dbws++;
            st = FALSE;
        }
        else {
            st = ERRED01;           /* dict read error */
        }
    }
    if (hinshi[0] == 0xFF)
        egcmemmove(hinshi, dp->hinshi, 4);
    return (st);
}

/*-----------------------------------------------------------*/
/*   CheckPLOSeg :  check PLO segment                                            */
/*-----------------------------------------------------------*/
NONREC static
BOOL            CheckPLOSeg(dic)
    DICRP           dic;
{
    DICR            dummydp;
    PIE             iep;
    UBYTE          *p;
    UBYTE          *NextPLO;
    REG             uniql;
    REG             dupll;

    if (!egcplock)
        return (TRUE);
    egcmemmove((UBYTE *) & dummydp, (UBYTE *) dic, sizeof(dummydp));

    /* Main & Expert Dictionary : not Check.                     */
    if (dummydp.dmode != 1)
        return (TRUE);

    /* PLO segment No. END ? => TRUE return. */
    if ((dummydp.segno) > getint(dicprty[0].pds->hbuhdr.uesegq))
        return (TRUE);

    /* Now GI Segment pointer get.                       */
	iep = dicprty[0].pds->hbuhdr.udidxp + (int) (dummydp.segno - 1);

    /* Next PLO Segment pointer get.             */
#ifdef  PMPLO
    p = GetPlop(dicprty[0].pds, dic->segno);
#else
    p = dicprty[0].pds->hbuhdr.udplop + getint(iep->ieplop);
#endif

    dummydp.plp = p + 2;

    /* PLO counter value get & MAX Check.        */
    if ((dummydp.recq = getint(p)) > (MAXPLE - 1)) {
        dummydp.segno++;
        dummydp.recq = 0;
        dummydp.recno = 0;
        egcmemmove((UBYTE *) dic, (UBYTE *) & dummydp, sizeof(dummydp));
        return (FALSE);
    }
    if (dummydp.recq == 0)
        return (TRUE);

    /* Now PLO reading (loop) */
    egcmemmove(dummydp.yomi, iep->ieyomi, sizeof(iep->ieyomi));
    for (dummydp.recno = 0; dummydp.recno < dummydp.recq; dummydp.recno++) {
        dummydp.dsp = getdsorec(dummydp.segno, (UWORD) 1);
        p = readpwrd(dummydp.plp, &dupll, &uniql);
        dummydp.plp = p + uniql;
    }

    iep++;
#ifdef  PMPLO
    NextPLO = GetPlop(dicprty[0].pds, dic->segno + 1);
#else
    NextPLO = dicprty[0].pds->hbuhdr.udplop + getint(iep->ieplop);
#endif
    if (dummydp.plp != NextPLO) {
        dummydp.segno++;
        dummydp.recno++;
        egcmemmove((UBYTE *) dic, (UBYTE *) & dummydp, sizeof(dummydp));
        return (FALSE);
    }
    return (TRUE);
}
/*-----------------------------------------------------------*/
NONREC
WORD egcnextdb(dic)
    DICRP           dic;
/*-----------------------------------------------------------*/
{
    not_skip_plo = TRUE;
    return (egcnext(dic));
}
