/************************************************************************/
/*                                                                      */
/*      EGCDIOS : EGConvert Dictionary I/O System                       */
/*                                                                      */
/*              Designed    by  I.Iwata         01/01/1985              */
/*              Coded       by  I.Iwata         01/01/1985              */
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
#include "egcldpro.h"

/*
**   n-bytes strings (UBYTE strings) compare
*/
#ifdef  MS_DOS
#define memncmp(S,T,C)  strncmp(S,T,C)
#endif

#define DEFAULT -100
#define FULLPLO -200
#define FULLDSO -300
#define FULLHOM -400
#define FULLYOM -500
#define NOEXTY  -600

/*
**  locale functions
*/
static BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
chkMultiSeg(
#ifdef  P_TYPE
WORD, REG*, PIE*, UBYTE*, REG
#endif
);
static VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
wtblsort(
#ifdef  P_TYPE
REG, PPR
#endif
);
static WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
choice_err(
#ifdef  P_TYPE
WORD, UWORD
#endif
);

/*****************************************************************************/
/*     gloval variable                                                       */
/*****************************************************************************/
UBYTE           dbstat;               /* true if yomi found in PLO           */
UWORD           dbsegn;               /* segment no (GI element number)      */
UBYTE           dbdupl;               /* duplicate length of yomi            */
UWORD           dbrecn;               /* PLO record number                   */
UWORD           dbmrec;               /* DS record number                    */
UWORD           dburec;               /* DSO record number                   */
UWORD           dbks;                 /* tan-kanji to be returned next time  */
UWORD           dbkq;                 /* quantity of tan-kanji               */
DBSLTBL         dbSlt[MAXSLTVAL];     /* learning homonym buffer             */
UBYTE           dbSltNo;              /* SLT value                           */
UBYTE           dbSltCnt;             /* SLT counter                         */
UBYTE           dbws;                 /* homonym to be returned next time    */
UBYTE           dbwq;                 /* quantity of homonym                 */
UBYTE          *dbwp;                 /* pointer to 1st element              */
UBYTE          *dbmbp;                /* pointer to PLO segment              */
UBYTE          *dbmrp;                /* pointer to PLO record               */
UBYTE          *dbkrp;                /* pointer to DSO record               */
UBYTE          *dbheap;               /* start of heap                       */
UBYTE          *dbfree;               /* end of free space of heap           */
PTNODTBL        dbcurt;               /* pointer to node table last allocat  */
UWORD           dbdsos[DSOEXT];       /* DSO block number list               */
UBYTE          *dbftbl[FHNUM];        /* dict88 vector table                 */
UBYTE           maxslt;               /* SLT elements max value              */
#ifdef CANON
UBYTE           maxdso;
#endif
#ifndef DOS_EGBRIDGE
BOOL            dellearn_flag = 1;    /* default true */
#endif
/*PAGE*/
/********************************************************************/
/*      tables                                                      */
/********************************************************************/
/*
** bit convertion table
*/
static UBYTE    bittbl[8] = {128, 64, 32, 16, 8, 4, 2, 1};
/*
** hinshi bit encode and decode table
*/
static UBYTE    dcdtbl[MAXCODE + 1][4] = {
    {0x00, 0x00, 0x00, 0x00}, /* 1 */
    {0x20, 0x00, 0x00, 0x00}, /* 2 */
    {0x10, 0x00, 0x00, 0x00}, /* 3 */
    {0x04, 0x00, 0x00, 0x00}, /* 4 */
    {0x40, 0x00, 0x00, 0x00}, /* 5 */
    {0x40, 0x00, 0x08, 0x00}, /* 6 */
    {0x40, 0x00, 0x20, 0x00}, /* 7 */
    {0x08, 0x00, 0x00, 0x00}, /* 8 */
    {0x00, 0x00, 0x40, 0x00}, /* 9 */
    {0x00, 0x00, 0x20, 0x00}, /* 10 */
    {0x00, 0x02, 0x00, 0x00}, /* 11 */
    {0x00, 0x00, 0x02, 0x00}, /* 12 */
    {0x00, 0x02, 0x08, 0x00}, /* 13 */
    {0x08, 0x02, 0x00, 0x00}, /* 14 */
    {0x08, 0x00, 0x20, 0x00}, /* 15 */
    {0xff, 0xff, 0xff, 0xff}, /* 16 */
    {0x00, 0x01, 0x00, 0x00}, /* 17 */
    {0x00, 0x00, 0x80, 0x00}, /* 18 */
    {0x01, 0x00, 0x00, 0x00}, /* 19 */
    {0x00, 0x80, 0x00, 0x00}, /* 20 */
    {0x00, 0x40, 0x00, 0x00}, /* 21 */
    {0x00, 0x20, 0x00, 0x00}, /* 22 */
    {0x02, 0x00, 0x00, 0x00}, /* 23 */
    {0x00, 0x00, 0x10, 0x00}, /* 24 */
    {0x00, 0x00, 0x08, 0x00}, /* 25 */
    {0x00, 0x00, 0x04, 0x00}, /* 26 */
    {0x00, 0x00, 0x01, 0x00}, /* 27 */
    {0x00, 0x00, 0x00, 0x80}, /* 28 */
    {0x00, 0x00, 0x00, 0x40}, /* 29 */
    {0x00, 0x00, 0x00, 0x20}, /* 30 */
    {0x00, 0x00, 0x00, 0x10}, /* 31 */
    {0x00, 0x00, 0x00, 0x08}, /* 32 */
    {0x00, 0x00, 0x00, 0x04}, /* 33 */
    {0x00, 0x00, 0x00, 0x01}, /* 34 */
    {0x00, 0x00, 0x00, 0x02}, /* 35 */
    {0x60, 0x00, 0x00, 0x00}, /* 36 */
    {0x50, 0x00, 0x00, 0x00}, /* 37 */
    {0x40, 0x40, 0x00, 0x00}, /* 38 */
    {0x40, 0x20, 0x00, 0x00}, /* 39 */
    {0x40, 0x02, 0x00, 0x00}, /* 40 */
    {0x40, 0x00, 0x28, 0x00}, /* 41 */
    {0x08, 0x80, 0x00, 0x00}, /* 42 */
    {0x08, 0x40, 0x00, 0x00}, /* 43 */
    {0x09, 0x00, 0x00, 0x00}, /* 44 */
    {0x08, 0x20, 0x00, 0x00}, /* 45 */
    {0x08, 0x00, 0x08, 0x00}, /* 46 */
    {0x08, 0x00, 0x04, 0x00}, /* 47 */
    {0x08, 0x00, 0x02, 0x00}, /* 48 */
    {0x08, 0x00, 0x01, 0x00}, /* 49 */
    {0x08, 0x00, 0x00, 0x80}, /* 50 */
    {0x08, 0x00, 0x00, 0x40}, /* 51 */
    {0x08, 0x00, 0x00, 0x20}, /* 52 */
    {0x08, 0x00, 0x00, 0x10}, /* 53 */
    {0x08, 0x00, 0x00, 0x08}, /* 54 */
    {0x08, 0x00, 0x00, 0x04}, /* 55 */
    {0x08, 0x00, 0x00, 0x02}, /* 56 */
    {0x08, 0x00, 0x00, 0x01}, /* 57 */
    {0x00, 0x02, 0x20, 0x00}, /* 58 */
    {0x08, 0x02, 0x20, 0x00}, /* 59 */
    {0x08, 0x02, 0x08, 0x00}, /* 60 */
    {0x30, 0x00, 0x00, 0x00}, /* 61 */
    {0x00, 0x04, 0x00, 0x00}, /* tankan */
    {0x00, 0x08, 0x00, 0x00}  /* bunsetu */
};
/*PAGE*/
/*
** jiritsu-go setsubi-go connect table
*/
static UBYTE    sctbl[7][2] = {
    {173, 176},
    {174, 0},
    {170, 0},
    {178, 182},
    {172, 0},
    {172, 0},
    {172, 0}
};
/*
** setsubi-go hinshi table
*/
static UBYTE    shtbl[7][4] = {
    {0x00, 0x40, 0x00, 0x00},
    {0x00, 0x40, 0x00, 0x00},
    {0x00, 0x02, 0x00, 0x00},
    {0x00, 0x20, 0x08, 0x00},
    {0x00, 0x40, 0x08, 0x00},
    {0x00, 0x40, 0x20, 0x00},
    {0x00, 0x80, 0x00, 0x00}
};
/*PAGE*/

/********************************************************************/
/* findpwrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "findpwrd" searches PL and PLO. "findpwrd" will be called by   */
/*   "egcktok"with "opt" = 0. In this case "findpwrd" will find all */
/*   jiritsu-go whose kana notation agree with foword substring of  */
/*   kana specified by "yomip" and "yomil", and search result will  */
/*   be stored in "wordtbl". "findpwrd" will be call by "finddict", */
/*   "deletedict" and "adddict" with "opt" = 1. In this case        */
/*   "findpwrd" will search only PLO and find the jiritsu-go whose  */
/*   kana notation agree completely with kana string specified by   */
/*   "yomip" and "yomil", and search result will be stored in       */
/*   "dbstat","dbsegn","dbmbp","dbmrp","dbrecn" and "dbdupll".      */
/* return                                                           */
/*   number of jiritsu-go found                                     */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findpwrd(yomip, yomil, wordtbl, opt)
    UBYTE          *yomip;             /* pointer to kana string  */
    UBYTE           yomil;             /* length of kana string   */
    PPR             wordtbl;           /* pointer to search result table */
    UBYTE           opt;               /* search option           */

{
/*--- static variables ---*/
    static UBYTE    nullstr[4] = {0x00, 0x00, 0x00, 0x00};

/*--- automatic variables ---*/
    UBYTE           yomi[MAXJYOMI + 1];
    UWORD           mrecn[MAXWDTBL];
    UBYTE          *mrecp[MAXWDTBL];
    UBYTE          *urecp[MAXWDTBL];
    UWORD           urecn[MAXWDTBL];
    UWORD           recno;
    UWORD           recq;
    REG             i;
    REG             dupll;
    REG             uniql;
    REG             matchls;
    REG             matchl;
    REG             myl;
    REG             nyl;
    REG             wordc;
    UBYTE           dirno;
    REG             idxn;
    UBYTE           accplo;
    PPR             wpt;
    UBYTE          *plmpt;
    UBYTE          *plompt;
    UBYTE          *mpt;
    UBYTE          *nmpt;
    PIE             idxp;
    register UBYTE *p;
    register UBYTE *q;
    DICIDTBL       *pdinfo;
    PTR_MDICT       pdarry;
    REG             previdx;
	BOOL			isMultiSeg = FALSE;
    UWORD           yType = NORMAL_YOMI;
    UBYTE           ExtArea = (UBYTE)TRUE;

    /*--- begin of findpwrd ---*/
    dbstat = FALSE;
    if ((yType = YTypeChk(yomip)) & (ILLEGAL_YOMI | NEUTRAL_YOMI)) {
        return  (UBYTE)0;
    }
    wordc = 0;
    wpt = wordtbl;
    nyl = myl = (yomil > MAXJYOMI) ? MAXJYOMI : yomil;
    egcmemmove(yomi, yomip, (UWORD) myl);
    while (myl > 0) {
        pdinfo = &dicprty[MAXAPENDIC - 1];
        for (dirno = 0; dirno < MAXAPENDIC; dirno++, pdinfo--) {
            if (pdinfo->idno == (WORD)-1) {
                if (dirno == MAXAPENDIC - 1) {
                    nyl = 0;
                }
                continue;
            }
            pdarry = (pdinfo->idno == 0) ?
                dicprty[0].pds : dicstat[pdinfo->idno].pds;
            if (pdarry->suspended == (UBYTE)SUSPEND) {
                if (dirno == MAXAPENDIC - 1) {
                    nyl = 0;
                }
                continue;
            }
            egcmemset((UBYTE *) mrecn, 0x00, sizeof(mrecn));
            egcmemset((UBYTE *) urecn, 0x00, sizeof(urecn));
            if (yType == NORMAL_YOMI)
                idxn = pdarry->hbuhdr.uduidx[(UINT)(*yomip - 0xB1)];
            else
                idxn = (UINT)0;
            idxp = pdarry->hbuhdr.udidxp + idxn;
            *(yomi + myl) = 0;

            i = ++idxn;
            if ((previdx = memncmp(yomi, idxp->ieyomi, 4)) >= 0) {
                while (memncmp(yomi, idxp->ieyomi, 4) >= 0) {
                    i++, idxp++;
                }
                i--, idxp--;
            }
            else if (i == 1 && previdx < 0) {
                if (dirno == MAXAPENDIC - 1) {
                    nyl --;
                    ExtArea = (UBYTE)FALSE;
                }
                continue;
            }

            isMultiSeg = chkMultiSeg(pdinfo->idno, &i, &idxp, yomip, myl);

            p = q = (UBYTE *) idxp;
            q += sizeof(GIEREC);
            dbsegn = (UWORD) i;
            plmpt = getlseg(pdinfo->idno, getint(((PIE) p)->ieplp),
                         getint(((PIE) q)->ieplp) - getint(((PIE) p)->ieplp));
            if (plmpt == (UBYTE*)NULL) {
                return  (UBYTE)0;
            }
            if (pdinfo->idno == 0) {
#ifdef  PMPLO
                plompt = dbmbp = GetPlop(pdarry, dbsegn);
#else
                plompt = dbmbp
                    = pdarry->hbuhdr.udplop +
                    getint(pdarry->hbuhdr.udidxp[(UINT) (dbsegn - 1)].ieplop);
#endif
            }
            p = ((PIE) p)->ieyomi;
            q = yomi;
            i = 4;
            while ((i) && (*q) && (*q == *p)) {
                i--; q++; p++;
            }
            dbdupl = (UBYTE) (q - yomi);
            matchls = (REG) (q - yomi);
            if (isMultiSeg) {
                nyl = myl - 1;
            }
            else {
                nyl = ((matchls == 4) || (!(*p))) ? matchls - 1 : matchls;
            }
            isMultiSeg = FALSE;
            for (accplo = opt; accplo <= 1; accplo++) {
                if ((pdinfo->idno != 0) && (accplo >= 1))
                    break;
                matchl = matchls;
                mpt = (accplo) ? plompt : plmpt;
                recq = getint(mpt);
                mpt += 2;
                recno = 1;
                while (recno <= recq) {
                    p = readpwrd(mpt, &dupll, &uniql);
                    nmpt = p + uniql;
                    if (matchl >= dupll) {
                        q = yomi + dupll;
                        while ((uniql) && (*p == *q)) {
                            uniql--;
                            p++;
                            q++;
                        }
                        matchl = q - yomi;
                        if ((!(uniql)) && (matchl)) {
                            if (!opt) {
                                i = matchl - 1;
                                if (!(accplo)) {
                                    *(mrecn + i) = recno;
                                    *(mrecp + i) = mpt;
                                }
                                else {
                                    *(urecn + i) = recno;
                                    *(urecp + i) = mpt;
                                }
                            }
                            else {
                                if (!(*q)) {
                                    dbstat = TRUE;
                                    break;
                                }
                                else {
                                    dbdupl = (UBYTE) matchl;
                                }
                            }
                        }
                        else if ((!(*q)) || (*q < *p)) {
                            break;
                        }
                    }
                    mpt = nmpt;
                    recno++;
                }
            }
            if (!opt) {
                for (i = myl - 1; i >= nyl; i--) {
                    if ((*(mrecn + i) || *(urecn + i)) && (wordc < MAXDWORD)) {
                        wpt->jrdicid = pdinfo->idno;
                        wpt->jrwordl = (UBYTE)i + (UBYTE)1;
                        wpt->jrsegno = dbsegn;
                        egcmemmove(wpt->jrmhbit, (wpt->jrmrecn = *(mrecn + i))?
                            readplh(*(mrecp + i)) : nullstr, 4);
                        egcmemmove(wpt->jruhbit, (wpt->jrurecn = *(urecn + i))?
                            readplh(*(urecp + i)) : nullstr, 4);
                        wordc++;
                        wpt++;
                        if (yType == EXTRA_YOMI)
                            break;
                    }
                }
            }
            else {
                dbrecn = recno;
                dbmrp = mpt;
                if (pdinfo->idno == 0)
                    goto ebreak;
            }
        }
        myl = nyl;
    }
ebreak:
    if (wordc > 1) {
        wtblsort(wordc, wordtbl);
    } else if (ExtArea == (UBYTE)FALSE && yType == EXTRA_YOMI) {
        wordc = 0xFF;
    }

    return ((UBYTE) wordc);
}
/*PAGE*/

/************************************************************************/
NONREC
static VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

wtblsort(wordc, wtbl)
    REG             wordc;
    PPR             wtbl;
/************************************************************************/
{
    PPR             wpt;
    UBYTE           wtmp[sizeof(PLREC)];
    UBYTE           noch;

    do {
        noch = TRUE;
        for (wpt = wtbl + 1; wpt < wtbl + wordc; wpt++) {
            if (((wpt - 1)->jrwordl <= wpt->jrwordl) &&
                ((wpt - 1)->jrdicid > wpt->jrdicid)) {
                egcmemmove((UBYTE *) wtmp, (UBYTE *) (wpt - 1), sizeof(PLREC));
                egcmemmove((UBYTE *) (wpt - 1), (UBYTE *) wpt, sizeof(PLREC));
                egcmemmove((UBYTE *) wpt, (UBYTE *) wtmp, sizeof(PLREC));
                noch = FALSE;
            }
        }
    } while (noch == FALSE);
}
/*PAGE*/

/************************************************************************/
/* findfwrd                                                             */
/*                                                                      */
/* function                                                             */
/*   "findfwrd" searches dict88 fuzoku-go table and finds all fuzoku-go */
/*   whose kana notation agree with a foword substring of kana specified*/
/*   by "yomip" and "yomil".                                            */
/* return                                                               */
/*   number of fuzoku-go found                                          */
/************************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findfwrd(yomip, yomil, fwrdtbl)
    UBYTE          *yomip;             /* pointer to kana string */
    UBYTE           yomil;             /* length of kana string */
    FWORDREC        fwrdtbl[MAXFWORD]; /* pointer to serach result table */
{
/*--- automatic variables ---*/
    PTFWORD         frp;
    register UBYTE *p;
    register UBYTE *q;
    UBYTE          *r;
    UBYTE          *cpt;
    UBYTE          *ept;
    UBYTE           fchar;
    REG             i;
    REG             wordct;
    UBYTE           flen;
    UBYTE           fcnt;

/*--- begin of findfwrd ---*/
    wordct = 0;
    fchar = *(yomip++);
    yomil--;
    frp = fwrdtbl;
    if ((fchar >= 0xa6) && (fchar <= 0xdd)) {
        p = dbftbl[FHFUZOKU];
        q = p + ((fchar - 0xa6) << 1);
        cpt = p + getint(q);
        ept = p + getint(q + 2);
        while (cpt < ept) {
            fcnt = (UBYTE)((*cpt) >> 4);
            flen = (UBYTE)((*cpt) & 0x0f);
            if (flen <= yomil) {
                q = yomip;
                r = (p = cpt + 1) + flen;
                while ((p < r) && (*q == *p)) {
                    q++;
                    p++;
                }
                if (p == r) {
                    for (i = fcnt; i--;) {
                        frp->frwordl = flen + (UBYTE)1;
                        frp->frcol = *(p++);
                        frp->frrow = *(p++);
                        frp++;
                        wordct++;
                    }
                }
                else {
                    if (*q < *p)
                        break;
                }
            }
            cpt += 1 + flen + (fcnt << 1);
        }
    }
    return ((UBYTE) wordct);
}
/*PAGE*/

/********************************************************************/
/* findswrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "findswrd" will search dict88 setubi-go table and find all     */
/*   setubi-go which satisfay following conditions                  */
/*   1. agree with a foword substring of kana specified by "yomip"  */
/*      and "yomil".                                                */
/*   2. can be connected to "prerow".                               */
/*   search result will be stored in "swordtbl".                    */
/* return                                                           */
/*   number of setsubi-go found                                     */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findswrd(yomip, yomil, prerow, swordtbl)
    UBYTE          *yomip;             /* pointer to kana string */
    UBYTE           yomil;             /* length of kana */
    UBYTE           prerow;            /* connect matrix row number */
    PLREC           swordtbl[MAXDWORD];/* pointer to search result table */
{
/*--- automatic variables ---*/
    register UBYTE *p;
    register UBYTE *q;
    UBYTE           fchar;
    UBYTE           sflag;
    UBYTE          *cpt;
    UWORD           n;
    REG             wordct;
    UBYTE           c;
    PPR             wrp;

/*--- start of findswrd ---*/
    wordct = 0;
    fchar = *yomip;
    if ((fchar >= 0xdd) || (fchar <= 0xb0)) {
        return ((UBYTE) 0);
    }
    if (n = getint((p = dbftbl[FHSETSUBI]) + ((fchar - 0xb1) << 1))) {
        cpt = p + n;
        wrp = swordtbl;
        while (1) {
            if ((c = *cpt) <= yomil) {
                p = yomip;
                q = cpt + 1;
                while ((c) && (*p == *q)) {
                    p++;
                    q++;
                    c--;
                }
                if (!c) {
                    sflag = *(q++) - (UBYTE)1;
                    if ((sctbl[(int) sflag][0] == prerow)
                        || (sctbl[(int) sflag][1] == prerow)) {
                        wrp->jrsegno = sflag;
                        wrp->jrmrecn = q - dbftbl[FHSETSUBI];
                        wrp->jrwordl = *cpt;
                        egcmemmove(wrp->jrmhbit, shtbl[(int) sflag], 4);
                        memset4(wrp->jruhbit, (UBYTE) 0x00);
                        wrp++;
                        wordct++;
                    }
                }
                else {
                    if (*p < *q)
                        break;
                }
            }
            else {
                if (fchar < *(cpt + 1))
                    break;
            }
            cpt += *cpt + 6;
        }
        return ((UBYTE) wordct);
    }
    else {
        return ((UBYTE) 0);
    }
}
/*PAGE*/
/**********************************************************************/
/* findkwrd                                                           */
/*                                                                    */
/* function                                                           */
/*   "findkwrd" searches dict88 bunnrui-fuka-go table and get connect */
/*   matrix row number of the word whose kana notation was specified  */
/*   by "yomip" and "yomil". Search result will be stored in "rowtbl" */
/* return                                                             */
/*   number of connect matrix row number                              */
/**********************************************************************/
NONREC
REG
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findkwrd(yomip, yomil, rowtbl)
    UBYTE          *yomip;             /* pointer to kana */
    UBYTE           yomil;             /* length of kana */
    UBYTE           rowtbl[MAXGWORD];  /* table of connect matrix row */
{
/*--- automatic variables ---*/
    UBYTE          *krec;
    UBYTE          *ept;
    register UBYTE *p;
    register UBYTE *q;
    UBYTE          *r;
    REG             i;
    REG             wordct;
    UBYTE           klen;
    UBYTE           kcnt;

    /*--- begin of findkwrd ---*/
    krec = dbftbl[FHKAHEN];
    ept = dbftbl[FHKAHEN + 1];
    wordct = 0;
    while (krec < ept) {
        kcnt = (UBYTE)((*krec) >> 4);
        klen = (UBYTE)((*krec) & (UBYTE)0x0f);
        if (klen == yomil) {
            p = yomip;
            q = krec + 1;
            r = p + yomil;
            while ((*p == *q) && (p < r)) {
                p++;
                q++;
            }
            if (p == r) {
                for (i = kcnt; i--;)
                    *(rowtbl + wordct++) = *(q++);
            }
            else if (*p < *q) {
                break;
            }
        }
        krec += 1 + klen + kcnt;
    }
    return (wordct);
}
/*PAGE*/

/********************************************************************/
/* findgwrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "findgwrd" searches dict88 katsuyou-gobi table of "hinshi" and */
/*   finds all katsuyou-gobi whose kana notation agree with a foword*/
/*   substring of kana specified by "yomip" and "yomil". Serach     */
/*   result will be stored in "rowtbl" and "lentbl"                 */
/* return                                                           */
/*   number of katsuyou-gobi found                                  */
/********************************************************************/
NONREC
REG
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findgwrd(hinshi, yomip, yomil, rowtbl, lentbl)
    UBYTE           hinshi;            /* hinshi code */
    UBYTE          *yomip;             /* pointer to kana */
    UBYTE           yomil;             /* length of kana */
    UBYTE           rowtbl[MAXGWORD];  /* connect matrix row */
    UBYTE           lentbl[MAXGWORD];  /* length of gobi */
{
/*--- automatic variables ---*/
    register UBYTE *p;
    register UBYTE *q;
    UBYTE          *grec;
    UBYTE          *ept;
    UBYTE          *r;
    REG             i;
    REG             gcnt;
    REG             glen;
    REG             wordct;

/*--- begin of findgwrd ---*/
    wordct = 0;
    grec = *(dbftbl + (i = FHKEIYOU - HCKEIYOU + hinshi));
    ept = *(dbftbl + i + 1);
    while (grec < ept) {
        gcnt = (*grec) >> 4;
        glen = (*grec) & 0x0f;
        if (glen <= (REG)yomil) {
            q = grec + 1;
            r = (p = yomip) + glen;
            while ((*p == *q) && (p < r)) {
                p++;
                q++;
            }
            if (p == r) {
                for (i = gcnt; i--;) {
                    *(lentbl + wordct) = (UBYTE) glen;
                    *(rowtbl + wordct) = *(q++);
                    wordct++;
                }
            }
            else if (*p < *q) {
                break;
            }
        }
        grec += 1 + glen + gcnt;
    }
    return (wordct);
}
/*PAGE*/

/********************************************************************/
/* clconnect                                                        */
/*                                                                  */
/* function                                                         */
/*   "clconnect" reads the dict88 connect matrix and checks         */
/*   the bit specified by "row" and "col".                          */
/* return                                                           */
/*   TRUE  : bit is on   (connectable)                              */
/*   FALSE : bit is off  (not connectable)                          */
/********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

clconnect(row, col)
    UBYTE           row;               /* connect matrix row number */
    UBYTE           col;               /* connect matrix colum number */
{
    register UBYTE *pt;

    pt = dbftbl[FHCONECT] + ((UWORD) (row - 1) << 4) + ((col - 1) >> 3);
    return ((BOOL) (*pt & *(bittbl + ((col - 1) & 0x07))));
}
/*PAGE*/

/********************************************************************/
/* terminate                                                        */
/*                                                                  */
/* function                                                         */
/*   "terminate" reads the dict88 clause end table and check the bit*/
/*   specified by "row".                                            */
/* return                                                           */
/*   TRUE  : bit is on  (clause end)                                */
/*   FALSE : bit is off (not clause end)                            */
/********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

terminate(row)
    UBYTE           row;               /* connect matrix row number */
{
    register UBYTE *pt;
    register REG    term;

    if (row > 171) {
        term = TRUE;
    }
    else if (!row) {
        term = TRUE;
    }
    else {
        pt = dbftbl[FHTERM] + ((--row) >> 3);
        term = *pt & *(bittbl + (row & 0x07));
    }
    return ((BOOL) term);
}
/*PAGE*/

/********************************************************************/
/* readswrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "readswrd" reads kanji notation of a setusbi-go stored in      */
/*   dict88 setubi-go tabel.                                        */
/* return                                                           */
/*   kanji length                                                   */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readswrd(off, buf)
    UWORD           off;               /* offset from the start of setsubi-go
                                        * record table */
    UBYTE          *buf;               /* pointer to kanji notation of
                                        * setsubi-go (output) */
{
/*--- automatic variavles ---*/
    register UBYTE *p;

/*--- begin of readswrd ---*/
    egcmemmove(buf, p = dbftbl[FHSETSUBI] + off, 4);
    if (p[3]) {
        return ((UBYTE) 4);
    }
    else {
        return ((UBYTE) 2);
    }

}
/*PAGE*/

/********************************************************************/
/* lookslt                                                          */
/*                                                                  */
/* function                                                         */
/*   "lookslt" return studied information.                          */
/* return                                                           */
/*   TRUE : exist             FALSE : none                          */
/********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

lookslt(idno, segno, mrecn, urecn)
    WORD            idno;              /* dictionary ID number */
    UWORD           segno;             /* segment number of PL,PLO */
    UWORD           mrecn;             /* record number in PL segment */
    UWORD           urecn;             /* record number in PLO segment */
{
/*--- automatic variavles ---*/
    register UBYTE *sltptr;
    register REG    i;
    UWORD           recno;
    PTR_MDICT       pdarry;

/*--- begin of lookslt ---*/
    i = idno;
    pdarry = dicprty[0].pds;
    recno = (mrecn) ? mrecn : MAXPLE + urecn;
#ifdef    PMSLT
    sltptr = GetSltp(pdarry, (segno - 1) * maxslt * 2);
#else
    sltptr = pdarry->hbuhdr.udsltp + ((segno - 1) * (maxslt * 2));
#endif
    for (i = maxslt; i--; sltptr += 2) {
        if ((getint(sltptr) & 0x1FF) == recno)
            return (TRUE);
    }
    return (FALSE);
}
/*PAGE*/

/********************************************************************/
/* readslt                                                          */
/*                                                                  */
/* function                                                         */
/*   "readslt" for 'selectclause'                                   */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readslt(idno, seg, mrec, urec)
    WORD            idno;
    UWORD           seg;
    UWORD           mrec;
    UWORD           urec;
{
/*--- automatic variavles ---*/
    UBYTE           seqno;
    REG             i;
    register UBYTE *sltptr;
    UWORD           sltval;
    UWORD           recno;
    PTR_MDICT       pdarry;

/*--- begin of readslt ---*/
    pdarry = dicprty[0].pds;
    dbSlt[0].FmtNo = 0;
    dbsegn = seg;
    dbSltCnt = 0;
    dbSltNo = 0;
    dbws = 0;
    dbks = 0;
    dbmrec = mrec;
    dburec = urec;
    if (idno != 0)
        return;
#ifdef    PMSLT
    sltptr = GetSltp(pdarry, (seg - 1) * maxslt * 2);
#else
    sltptr = pdarry->hbuhdr.udsltp + ((seg - 1) * (maxslt * 2));
#endif
   for (i = maxslt; i > 0; i--, sltptr += 2) {
        sltval = getint(sltptr);
        recno = sltval & 0x01FF;
        if (sltval == 0x0000)
            break;
        seqno = (UBYTE) (sltval >> 9);
        if (recno == mrec) {
            dbSlt[0].RecNo = (seqno < (UBYTE) 64) ? recno : urec;
            dbSlt[0].SegNo = seg;
            dbSlt[0].SeqNo = seqno;
            dbSltNo = 1;
            break;
        }
        else if (recno == urec + MAXPLE) {
            dbSlt[0].RecNo = urec;
            dbSlt[0].SegNo = seg;
            dbSlt[0].SeqNo = seqno;
            dbSltNo = 1;
            break;
        }
    }
}
/*PAGE*/

/********************************************************************/
/* DimSet                                                           */
/*                                                                  */
/* function                                                         */
/*   "DimSet" for 'findclause'                                      */
/*   "DimSet" must be called before "readdwrd" is called.           */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

DimSet(cltype, lefon, qclfrm)
    PTCLTYPE        cltype;
    BYTE           lefon;
    UBYTE           qclfrm;
{
/*--- automatic reference ---*/
    PTCLTYPE        ct;
    UBYTE           Sltc;
    UBYTE           Fmtc;
    UWORD           SltVal;
    UWORD           RecNo;
    UBYTE           SeqNo;
    UBYTE          *sltptr;
    UBYTE           sltq;
    PTR_MDICT       pdarry;
    PDSLT           pslt;

/*--- begin of DimSet ---*/
    pdarry = dicprty[0].pds;
    pslt = &dbSlt[0];
    sltq = 0;
    if (lefon != -1) {
        ct = cltype + lefon;
#ifdef    PMSLT
        sltptr = GetSltp(pdarry, (ct->ctsegno - 1) * maxslt * 2);
#else
        sltptr = pdarry->hbuhdr.udsltp + ((ct->ctsegno - 1) * (maxslt * 2));
#endif
        for (Sltc = 0; Sltc < maxslt; Sltc++) {
            SltVal = (UWORD) (getint(sltptr + Sltc * 2));
            if (SltVal == 0x0000)
                break;
            RecNo = SltVal & 0x1FF;
            SeqNo = (UBYTE) (SltVal >> 9);
            ct = cltype + lefon;
            Fmtc = lefon;
            for ( ; Fmtc < qclfrm; Fmtc++, ct++) {
                if (ct->ctdicid != 0)
                    continue;
                if (RecNo == ct->ctmrecn) {
                    pslt->RecNo = (SeqNo < (UBYTE) 64) ? RecNo : ct->cturecn;
                    pslt->SegNo = ct->ctsegno;
                    pslt->SeqNo = SeqNo;
                    pslt->FmtNo = Fmtc;
                    pslt++;
                    sltq++;
                }
                else if (RecNo == (ct->cturecn + MAXPLE)) {
                    pslt->RecNo = ct->cturecn;
                    pslt->SegNo = ct->ctsegno;
                    pslt->SeqNo = SeqNo;
                    pslt->FmtNo = Fmtc;
                    pslt++;
                    sltq++;
                }
                if (sltq >= MAXSLTVAL)
                    break;
            }
        }
    }
    dbSltNo = sltq;
    dbSltCnt = 0;
}
/*PAGE*/

/********************************************************************/
/* SelectSeg                                                        */
/*                                                                  */
/* function                                                         */
/*   "SelectSeg"  Select element number on the format-table.        */
/* return                                                           */
/*      found       :number                                         */
/*      not found   :-1                                             */
/********************************************************************/
NONREC
BYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

SelectSeg(cf, clq)
    PTCLTYPE        cf;
    UBYTE           clq;
{
/*--- automatic reference ---*/
    PTCLTYPE        cltype;
    UBYTE          *sltptr;
    UBYTE           Fmtc;
    UBYTE           Sltc;
    UWORD           mFmtRec;
    UWORD           uFmtRec;
    UWORD           CmpRec;
    UWORD           SltVal;
    PTR_MDICT       pdarry;

/*--- begin of SelectSeg ---*/
    pdarry = dicprty[0].pds;
    cltype = cf;
    for (Fmtc = 0; Fmtc < clq; Fmtc++, cltype++) {
        if (cltype->ctdicid != 0)
            continue;
#ifdef    PMSLT
        sltptr = GetSltp(pdarry, (cltype->ctsegno - 1) * maxslt * 2);
#else
        sltptr = pdarry->hbuhdr.udsltp 
            + ((cltype->ctsegno - 1) * (maxslt * 2));
#endif
        mFmtRec = cltype->ctmrecn;
        uFmtRec = cltype->cturecn;
        for (Sltc = 0; Sltc < maxslt; Sltc++) {
            SltVal = (UWORD) (getint(sltptr + Sltc * 2));
            if (SltVal == 0x0000)
                break;
            CmpRec = SltVal & 0x01FF;
            if ((mFmtRec == CmpRec) || ((uFmtRec + MAXPLE) == CmpRec))
                return (Fmtc);
        }
    }
    return (-1);
}
/*PAGE*/

/********************************************************************/
/* LearnSkip                                                        */
/*                                                                  */
/* function                                                         */
/*   "LearnSkip"  learn information. if learning word found, skip.  */
/* return   no value                                                */
/********************************************************************/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

LearnSkip(idno, seqno)
    WORD            idno;
    UWORD          *seqno;
{
/*--- automatic reference ---*/
    register UBYTE  ic;
    UWORD           srchRec;
    PDSLT           pslt;

/*--- begin of LearnSkip ---*/
    pslt = &dbSlt[0];
    if (idno != 0) {
        *seqno = dbws;
        dbws++;
        return (0);
    }
    srchRec = (dbws < 64) ? dbmrec : dburec;

    for (ic = 0; ic < dbSltNo; ic++, pslt++) {
        if ((pslt->RecNo != srchRec) ||
            (pslt->SeqNo != dbws) ||
            (pslt->SegNo != dbsegn))
            continue;
        dbws++;
        if (dbws >= dbwq) {
            dbws++;
            dbwp = skipkanji(1, dbwp);
            return (1);
        }
        dbwp = skipkanji(1, dbwp);
        ic = 0;
    }
    *seqno = dbws;
    dbws++;
    return (0);
}
/*PAGE*/

/********************************************************************/
/* SetCLearn                                                        */
/*                                                                  */
/* function                                                         */
/*   "SetCLearn" return learning information.                       */
/* return                                                           */
/*   format-table pointer                                           */
/********************************************************************/
NONREC
PTCLTYPE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

SetCLearn(cf1, cf2, newcl)
    PTCLTYPE        cf1;
    PTCLTYPE        cf2;
    UBYTE          *newcl;
{
/*--- automatic variables ---*/
    PTCLTYPE        cltype;

/*--- begin of SetCLearn ---*/
    cltype = cf2;
    if ((dbSltNo) && (dbSltCnt < dbSltNo)) {
        cltype = cf1 + dbSlt[dbSltCnt].FmtNo;
        dbsegn = cltype->ctsegno;
        *newcl = TRUE;
    }
    else if (*newcl) {
        *newcl = FALSE;
        dbws = 0;
        dbks = 0;
        dbsegn = cltype->ctsegno;
        dbmrec = cltype->ctmrecn;
        dburec = cltype->cturecn;
    }
    return (cltype);
}
/*PAGE*/

/********************************************************************/
/* readdwrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "readdwrd" returns a homonym one by one.                       */
/* return                                                           */
/*   sequece number of the homonym retured                          */
/*     0   - 255   homonym stored in main dictionary                */
/*     256 - 511   homonym stored in user dictionary                */
/*     512         end of homonym                                   */
/********************************************************************/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readdwrd(idno, yomil, yomi, kanjil, kanji, hbit, selected)
    WORD            idno;              /* dic id */
    UBYTE           yomil;             /* length of kana */
    UBYTE          *yomi;              /* pointer to kana */
    UBYTE          *kanjil;            /* pointer to length of kanji (output)*/
    UBYTE          *kanji;             /* pointer to kanji (output) */
    UBYTE          *hbit;              /* pointer to hinshi (output) */
    UBYTE          *selected;          /* Is word learned ?         */
{
/*--- automatic variables ---*/
    UWORD           recno;
    UWORD           segno;
    UWORD           seqno;
    UBYTE           kqTmp;
    UBYTE           temp[MAXJYOMI * 4];
    UBYTE           templ;

/*--- begin of readdwrd ---*/
    *selected = (UBYTE)FALSE;
    if ((dbSltCnt < dbSltNo) && (idno == 0)) {
        recno = dbSlt[dbSltCnt].RecNo;
        segno = dbSlt[dbSltCnt].SegNo;
        seqno = dbSlt[dbSltCnt].SeqNo;
        dbws = 0;
        ++dbSltCnt;
        if ((seqno == 63) || (seqno == 127)) {
            *kanjil = (UBYTE) cvtatoj(2, yomi, yomil, kanji);
            memset4(hbit, (UBYTE) 0xFF);
            return (seqno);
        }
        else if (seqno < 63) {      /* Case Main dict */
            if ((dbwp = getdsrec((WORD) 0, segno, recno)) == (UBYTE*)NULL) {
                return  512;
            }
            dbwq = ((*dbwp) & (UBYTE) 0xC0) ? (UBYTE) 1 : *(dbwp++);
            readjwrd(skipkanji((UBYTE) seqno, dbwp),
                     yomil, yomi, kanjil, kanji, hbit);
        }
        else {                      /* Case User dict */
            dbwp = getdsorec((UWORD) segno, recno);
            dbwq = *(dbwp++) + (UBYTE)64;
            readjwrd(skipkanji((UBYTE) (seqno - 64), dbwp), yomil, yomi,
                     kanjil, kanji, hbit);
        }
        *selected = (UBYTE)TRUE;
    }
    else {
        if (dbmrec != 0) {
            if (dbws == 0) {
                if ((dbwp = getdsrec(idno, dbsegn, dbmrec)) == (UBYTE*)NULL) {
                    return  512;
                }
                if (*dbwp == 0)
                    dbws = 63;
                dbwq = ((*dbwp) & (UBYTE) 0xC0) ? (UBYTE) 1 : *(dbwp++);
            }
            if ((dbws < dbwq) && !LearnSkip(idno, &seqno)) {
                dbwp = readjwrd(dbwp, yomil, yomi, kanjil, kanji, hbit);
            }
            else {
                if (dbks == 0) {
                    kqTmp = (UBYTE)((*dbwp) & 0xC0);
                    dbkq = (kqTmp == 0x40) ? ((*dbwp) & 0x3F) :
                        (kqTmp == 0x80) ? (*(++dbwp) + 64) :
                        0;
                    dbwp++;
                }
                if (dbks < dbkq) {
                    seqno = dbwq;
                    *kanjil = 2;
                    *kanji = *(dbwp++);
                    *(kanji + 1) = *(dbwp++);
                    memset4(hbit, (UBYTE) 0x00);
                    ++dbks;
                }
                else {
                    dbws = 64;
                    dbmrec = 0;
                }
            }
        }
        if ((dbmrec == 0) && (dburec != 0)) {
            if (idno != 0) {
                dburec = 0;
                return (512);
            }
            if ((dbws == 64) || (dbws == 0)) {
                if (dbws == 0)
                    dbws = 64;
                dbwp = getdsorec(dbsegn, dburec);
                dbwq = *(dbwp++) + (UBYTE)64;
            }
            if ((dbws < dbwq) && !LearnSkip(idno, &seqno)) {
                dbwp = readjwrd(dbwp, yomil, yomi, kanjil, kanji, hbit);
            }
            else {
                dburec = 0;
            }
        }
        if ((dbmrec == 0) && (dburec == 0)) {
            return (512);           /* To next format-table. */
        }
    }
    if (*kanjil == 2) {
        templ = (UBYTE) cvtatoj(2, yomi, yomil, temp);
        if ((templ == 2) && (!memncmp(kanji, temp, templ)))
            seqno = 63;
    }
    return ((seqno < 64) ? (UWORD) seqno :
            (seqno < 128) ? (UWORD) seqno + 256 : 512);
}
/*PAGE*/

#ifndef DOS_EGBRIDGE
/********************************************************************/
/* agedict (old version)                                            */
/*                                                                  */
/* function                                                         */
/*   "agedict" registers the id of the homonym selected by user     */
/*   in SLT.                                                        */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

agedict(wordid)
    PTWORDID        wordid;            /* pointer to the id of homonym */
{
/*--- automatic variavles ---*/
    register REG    i;
    register UBYTE *p;
    UBYTE          *sltptr;
    UBYTE           found = 0;
    PTR_MDICT       pdarry;

/*---- begin of agedict ---*/
    pdarry = dicprty[0].pds;
    if ((GETWIDSEQ(wordid->wiseqno) == 63) ||
        (GETWIDSEQ(wordid->wiseqno) == 127) ||
        (GETWIDDID(wordid->wirecno) != 0))
        return;
#ifdef    PMSLT
    sltptr = p = GetSltp(pdarry,
        (GETWIDSEG(wordid->wisegno, wordid->wiseqno) - 1) * maxslt * 2);
#else
    sltptr = p = pdarry->hbuhdr.udsltp +
        ((GETWIDSEG(wordid->wisegno, wordid->wiseqno) - 1) * (maxslt * 2));
#endif
    for (i = maxslt; i--;) {
        if ((getint(p) & 0x1FF) == GETWIDREC(wordid->wirecno)) {
            found++;
            break;
        }
        p += 2;
    }
    if (found) {
        if (i > 0) {
            egcmemmove(p, p + 2, (UWORD) i * 2);
        }
        setint(sltptr + (maxslt - 1) * 2, 0);
    }
    if (GETWIDSEQ(wordid->wiseqno)) {
        egcmemmove(sltptr + 2, sltptr, (maxslt - 1) * 2);
        setint(sltptr,
               ((UWORD) (GETWIDSEQ(wordid->wiseqno)) << 9) +
               GETWIDREC(wordid->wirecno));
    }
#ifdef    PMSLT
    PutSltp(pdarry, 
        (GETWIDSEG(wordid->wisegno, wordid->wiseqno) - 1) * maxslt * 2);
#else
    putuseg(0, pdarry->hbuhdr.udsltp,
            (UWORD) getint(pdarry->hbuhdr.uesltl),
            (UWORD) getint(pdarry->hbuhdr.ueslts));
#endif
}
#endif

/*PAGE*/
/********************************************************************/
/* agedict2 (new version)                                           */
/*                                                                  */
/* function                                                         */
/*   "agedict2" registers the id of the homonym selected by user    */
/*   in SLT.                                                        */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

agedict2(wordid, allnbr, setnbr)
    PTWORDID        wordid;            /* pointer to the id of homonym */
    UWORD           allnbr;            /* all id number of homonym */
    UWORD           setnbr;            /* set id number of homonym */
{
/*--- automatic variavles ---*/
    PTWORDID        pw;
    register UBYTE  i;
    register UBYTE *sp;
    UBYTE          *sltptr;
    UWORD           CmpSegno, CmpSeqno;
    UWORD           SetSegno;
    UBYTE           SetSeqno;
    UWORD           SetRecno;
    UWORD           CntID;
    UWORD           uwp;
    BOOL            found = FALSE;
    PTR_MDICT       pdarry;

/*---- begin of agedict2 ---*/
    pdarry = dicprty[0].pds;
    SetSegno = GETWIDSEG((wordid + setnbr)->wisegno, 
        (wordid + setnbr)->wiseqno);
    SetSeqno = GETWIDSEQ((wordid + setnbr)->wiseqno);
    SetRecno = GETWIDREC((wordid + setnbr)->wirecno);
    if (GETWIDDID((wordid + setnbr)->wirecno) != 0)
        return;
    if (SetSegno == 0)
        return;
    found = FALSE;
#ifdef    PMSLT
    sltptr = sp = GetSltp(pdarry, (SetSegno - 1) * maxslt * 2);
#else
    sltptr = sp = pdarry->hbuhdr.udsltp + ((SetSegno - 1) * (maxslt * 2));
#endif
    uwp = (UWORD) getint(sp);
    if (((uwp & 0x1FF) != SetRecno) || (((UBYTE) (uwp >> 9)) != SetSeqno)) {
        for (i = 2, sp += 2; i <= maxslt - (UBYTE)1; sp += 2, i++) {
            uwp = (UWORD) getint(sp);
            if (((uwp & 0x1FF) == SetRecno) &&
                ((UBYTE) (uwp >> 9) == SetSeqno))
                break;
        }
        egcmemmove(sltptr + 2, sltptr, (i - 1) * 2);
        setint(sltptr, ((UWORD) SetSeqno << 9) + SetRecno);
#ifdef    PMSLT
        PutSltp(pdarry, (SetSegno - 1) * maxslt * 2);
#else
        putuseg(0, pdarry->hbuhdr.udsltp,
                (UWORD) getint(pdarry->hbuhdr.uesltl),
                (UWORD) getint(pdarry->hbuhdr.ueslts));
#endif
    }
    for (CntID = 0, pw = wordid; CntID < allnbr; pw++, CntID++) {
        CmpSegno = GETWIDSEG(pw->wisegno, pw->wiseqno);
        CmpSeqno = GETWIDSEQ(pw->wiseqno);
        if ((CmpSegno == SetSegno) || (CmpSegno == 0) ||
            (GETWIDDID(pw->wirecno) != 0))
            continue;
#ifdef    PMSLT
        sltptr = sp = GetSltp(pdarry, (CmpSegno - 1) * maxslt * 2);
#else
        sltptr = sp = pdarry->hbuhdr.udsltp + (CmpSegno - 1) * (maxslt * 2);
#endif
        for (i = 0; i < maxslt; sp += 2, i++) {
            if (((UWORD) getint(sp) & 0x1FF) != GETWIDREC(pw->wirecno))
                continue;
            egcmemmove(sp, sp + (UBYTE) 2, (maxslt - i - 1) * 2);
            setint(sltptr + (maxslt - 1) * 2, (UWORD) 0);
            found = TRUE;
        }
#ifdef    PMSLT
        if (found)
            PutSltp(pdarry, (CmpSegno - 1) * maxslt * 2);
#endif
    }

#ifndef    PMSLT
    if (found) {
        putuseg(0, pdarry->hbuhdr.udsltp,
                (UWORD) getint(pdarry->hbuhdr.uesltl),
                (UWORD) getint(pdarry->hbuhdr.ueslts));
    }
#endif
}
/*PAGE*/

/********************************************************************/
/* ageformula                                                       */
/*                                                                  */
/* function                                                         */
/*   "ageformula" puts formula out to dictionary header             */
/*                                                                  */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
ageformula()
{
    PTR_UDICT   pdarry = dicprty[0].pds;
    return  putuseg(0, (UBYTE *) & pdarry->hbuhdr, (UWORD) 1, 0);
}

/********************************************************************/
/* finddict                                                         */
/*                                                                  */
/* function                                                         */
/*   "finddict"  searches user dictionary and locates the homonyms  */
/*   whose kana notation are given by "yomi" and "yomilen".         */
/* return                                                           */
/*   number of homonyms.                                            */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

finddict(yomi, yomilen)
    UBYTE          *yomi;              /* pointer to kana */
    UBYTE           yomilen;           /* length of kana */
{
/*--- begin of finddict ---*/
    findpwrd(yomi, yomilen, (PPR) 0, (UBYTE) 1);
    if (dbstat) {
        return (*(dbkrp = getdsorec(dbsegn, dbrecn)));
    }
    else {
        return ((UBYTE) 0);
    }
}
/*PAGE*/

/********************************************************************/
/* readdict                                                         */
/*                                                                  */
/* funhction                                                        */
/*   "readdict" returns a homonym located by "finddict". homonym to */
/*   be reurn is specified by "seqno". Range of "seqno" must be from*/
/*   1 to the cardinarity of homonyms.                              */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
#ifndef DOS_EGBRIDGE
NONREC
UBYTE*    readdict(seqno, yomip, yomil, kanjip, kanjil)
    UBYTE           seqno;             /* sequence number of homonym */
    UBYTE          *yomip;             /* pointer to kana */
    UBYTE           yomil;             /* length of yomi */
    UBYTE          *kanjip;            /* pointer to kanji (output) */
    UBYTE          *kanjil;            /* pointer to length of kanji (output) */
{
/*--- automatic variables ----*/
    UBYTE           h[4];

/*--- begin of readdict ---*/
    return    readjwrd(skipkanji((UBYTE)(seqno - (UBYTE)1), dbkrp + 1), 
                       yomil, yomip, kanjil, kanjip, h);
}
#endif
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readdict2(seqno, yomip, yomil, kanjip, kanjil, hinshi)
    UBYTE           seqno;             /* sequence number of homonym */
    UBYTE          *yomip;             /* pointer to kana */
    UBYTE           yomil;             /* length of yomi */
    UBYTE          *kanjip;            /* pointer to kanji (output) */
    UBYTE          *kanjil;            /* pointer to length of kanji (output) */
    UBYTE          *hinshi;            /* pointer to hinshi bit (output) */
{
/*--- begin of readdict ---*/
    return    readjwrd(skipkanji((UBYTE)(seqno - (UBYTE)1), dbkrp + 1), 
                       yomil, yomip, kanjil, kanjip, hinshi);
}
/*PAGE*/

/********************************************************************/
/* deletedict                                                       */
/*                                                                  */
/* function                                                         */
/*   "deletedict" deletes a jiritsu-go strored in user dictionary.  */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

deletedict(yomi, yomilen, kanji, kanjilen)
    UBYTE          *yomi;              /* pointer to kana */
    UBYTE           yomilen;           /* length of kana */
    UBYTE          *kanji;             /* pointer to kanji */
    UBYTE           kanjilen;          /* length of kanji */
{
/*--- automatic variables ---*/
    register UBYTE *krecp;
    register UBYTE *kelmp;
    UWORD           blkl;
    PTR_MDICT       pdarry;
    WORD            delStat;

/*--- begin of deletedict ---*/
    pdarry = dicprty[0].pds;
    findpwrd(yomi, yomilen, (PPR) 0, (UBYTE) 1);
    if (dbstat) {
        krecp = getdsorec(dbsegn, dbrecn);
        if (kelmp = findkanji(krecp, yomilen, yomi, kanjilen, kanji)) {
            delkanji(krecp, kelmp);
            makemidashi(krecp, yomi, yomilen);
            if (!(*krecp)) {
                blkl = getint(pdarry->hbuhdr.uddsop);
                egcmemmove(krecp, krecp + 1,
                           blkl - (krecp + 1 - pdarry->hbuhdr.uddsop));
                setint(pdarry->hbuhdr.uddsop, --blkl);
            }
            putwoseg();
            deluslt(dbsegn);
            delStat = NORMAL;
        } else {
            delStat = ERROR;
        }
    } else {
        delStat = ERROR;
    }
    return  delStat;
}
/*PAGE*/

/********************************************************************/
/* adddict                                                          */
/*                                                                  */
/* function                                                         */
/*   "adddict" add a jiritsu-go to user dictionary.                 */
/* return                                                           */
/*   TRUE  : unable to add because of overflow                      */
/*   FALSE : ok                                                     */
/********************************************************************/
NONREC
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

adddict(yomi, yomilen, kanji, kanjilen, hbitstr)
    UBYTE          *yomi;              /* pointer to kana */
    UBYTE           yomilen;           /* length of kana */
    UBYTE          *kanji;             /* pointer to kanji */
    UBYTE           kanjilen;          /* length of kanji */
    UBYTE           hbitstr[4];        /* hinshi bit string */
{
/*--- automatic variables ----*/
    WORD            add_ok;
    UBYTE           adkelml;
    UBYTE           adaddl;
    UWORD           adblkl;
    register UBYTE *adaddp;
    register UBYTE *adkrecp;
    UBYTE           adkelm[MAXJKANJI + 16];
    UBYTE           adkanac;
    UBYTE           adkana[MAXJKANJI + 2];
    PTR_MDICT       pdarry;
    UWORD           eplol;

/*--- begin of adddict ---*/
    pdarry = dicprty[0].pds;
    eplol = (pdarry->hbuhdr.udplol == 255) ?
        (UWORD) (getint(pdarry->hbuhdr.ueplol)) :
        pdarry->hbuhdr.udplol;
    do {
        add_ok = NORMAL;
        if ((eplol * UDBLKL) -
            getint(pdarry->hbuhdr.udidxp
                   [(UINT) getint(pdarry->hbuhdr.uesegq)]
                   .ieplop) < 32) {
            add_ok = FULLPLO;
        }
        adkelml = makekrec(kanjilen, kanji, hbitstr, adkelm);
        if ((adkanac = *(adkelm + adkelml - 2)) <= 2) {
            adkanac = (UBYTE) cvtatoj(adkanac, yomi, yomilen, adkana);
            if ((kanjilen != adkanac)
                || (memncmp(kanji, adkana, (UWORD) kanjilen))) {
                egcmemmove(adkana, kanji, (UWORD) kanjilen);
                adkanac = *(adkana + kanjilen - 2);
                *(adkana + kanjilen - 2) = 0;
                adkelml = makekrec(kanjilen, adkana, hbitstr, adkelm);
                *(adkelm + adkelml - 2) = adkanac;
            }
        }
        if (findpwrd(yomi, yomilen, (PPR) 0, (UBYTE) 1) == (UBYTE)0xFF
            && YTypeChk(yomi) == EXTRA_YOMI) {
            add_ok = NOEXTY;
        }
        if (getint(dbmbp) == MAXPLOE) {
            add_ok = FULLYOM;
        }
        if (add_ok != NOEXTY)
            adkrecp = getdsorec(dbsegn, dbrecn);

        if (dbstat) {
            if (*adkrecp >= 63) {
                add_ok = FULLHOM;
            }
            if (adaddp = findkanji(adkrecp, yomilen, yomi, kanjilen, kanji)) {
                delkanji(adkrecp, adaddp);
            }
            adaddp = adkrecp + 1;
            adaddl = adkelml;
        }
        else {
            adaddp = adkrecp;
            adaddl = adkelml + (UBYTE)1;
        }
        adblkl = getint(pdarry->hbuhdr.uddsop);
#ifdef CANON
        if (adblkl + adaddl > (DSOBLKL - 2) * maxdso) {
#else
        if (adblkl + adaddl > (DSOBLKL - 2) * DSOEXT) {
#endif
            add_ok = FULLDSO;
        }
        if (add_ok != NORMAL) {
#ifdef  DOS_EGBRIDGE
            if (dbsegn)
#else
            if (dellearn_flag && dbsegn)
#endif
                add_ok = choice_err(add_ok, dbsegn);
            switch (add_ok) {
            case    FULLYOM:
                return  (ERWDA23);
            case    FULLHOM:
                return  (ERWDA12);
			case    FULLPLO:
				return  (ERWDA20);
			case    FULLDSO:
				return  (ERWDA21);
			case    NOEXTY:
			    return  (ERWDA24);
			case    DEFAULT:
			default:
				break;
			}
		}
    } while (add_ok != NORMAL);
    egcmemmove(adaddp + adaddl, adaddp,
               adblkl - (adaddp - pdarry->hbuhdr.uddsop));
    setint(pdarry->hbuhdr.uddsop, adblkl + adaddl);
    if (dbstat) {
        *adkrecp += 1;
    }
    else {
        *(adaddp++) = 1;
    }

    egcmemmove(adaddp, adkelm, (UWORD) adkelml);
    putwoseg();
    makemidashi(adkrecp, yomi, yomilen);
    deluslt(dbsegn);
    return (FALSE);
}
/*PAGE*/

/*------------------------------------------------------------------*/
NONREC static
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

choice_err(err_add, csegn)
    WORD            err_add;
    UWORD           csegn;
/*------------------------------------------------------------------*/
{
    UWORD           startseg, loopseg, maxsegq;
    WORD            cho_ret = DEFAULT;
    BOOL            delres;

/*--- begin of choice_err ---*/
    maxsegq = getint(dicprty[0].pds->hbuhdr.uesegq);
    startseg = loopseg = csegn;
    do {
        delres = dellearn(loopseg);
        /* next segment */
        loopseg++;
        switch (delres) {
        case TRUE:                  /* delete success */
            goto cho_break;
        case FALSE:                 /* not delete success */
            /* check current segment, no more next segments */
            if ((err_add == FULLYOM) ||
                (err_add == FULLHOM) ||
                (err_add == FULLDSO) ||
                (err_add == NOEXTY)
                ) {
                cho_ret = err_add;
                goto cho_break;
            }
            /* if over last segment, next first segment */
            if (loopseg > maxsegq)
                loopseg = 1;
            break;
        }
    } while (loopseg != startseg);
    /* no more add word */
    if (err_add == FULLPLO)
        cho_ret = err_add;

cho_break:
    return (cho_ret);
}

/*------------------------------------------------------------------*/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

dellearn(delsegn)
    UWORD           delsegn;
/*------------------------------------------------------------------*/
{
/*--- automatic variavles ---*/
    REG             deyuni, deyrecl;
    UWORD           sltval, recval;
    UWORD           recq;
    UWORD           sltfound;
    UWORD           kanfound;
    UBYTE           seqval;
    UBYTE           seqq, tseq;
    UBYTE          *delrec;
    UBYTE           dely[MAXJYOMI + 1];
    UBYTE           delyl;
    UBYTE           maxdely[MAXJYOMI + 1];
    UBYTE           maxdelyl;
    UBYTE           delk[33];
    UBYTE           delkl;
    UWORD           recnt;
    UBYTE           hinsi[4];
    register REG    oldlevel;
    register UBYTE *sltptr;
    register UBYTE *deplop, *dedsop;
    PIE             ptrie;
    PTR_MDICT       pdarry;

/*--- begin of dellearn ---*/
    pdarry = dicprty[0].pds;
    ptrie = pdarry->hbuhdr.udidxp + (int) (delsegn - 1);
#ifdef  PMPLO
    deplop = GetPlop(pdarry, delsegn);
#else
	deplop = pdarry->hbuhdr.udplop + getint((UBYTE *) (ptrie->ieplop));
#endif
	egcmemmove(dely, ptrie->ieyomi, sizeof(ptrie->ieyomi));
	recq = getint(deplop);
	deplop += 2;
	for (recnt = 1; recnt <= recq; recnt++, deplop = delrec + deyuni) {
		delrec = readpwrd(deplop, &deyrecl, &deyuni);
		delyl = (UBYTE) (deyrecl + deyuni);
		egcmemmove(dely + deyrecl, delrec, deyuni);
		dely[delyl] = 0;
		egcmemmove(hinsi, readplh(deplop), 4);
		if (hinsi[1] & 0x08) {
			goto lrnfound;
		}
	}
    return (FALSE);                 /* Cannot delete learn word */

lrnfound:                           /* learning-word found */
#ifdef    PMSLT
    sltptr = GetSltp(pdarry, (delsegn - 1) * maxslt * 2);
    sltptr += ((maxslt * 2) - 2);
#else
    sltptr = pdarry->hbuhdr.udsltp + ((delsegn) * (maxslt * 2)) - 2;
#endif
    sltfound = FALSE;
    for (oldlevel = maxslt; oldlevel--; sltptr -= 2) {
        sltval = getint(sltptr);
        recval = sltval & 0x01FF;
        seqval = (UBYTE) (sltval >> 9);
        if ((seqval < (UBYTE) 64) || (recnt != recval))
            continue;
        sltfound = TRUE;
		egcmemmove(maxdely, dely, sizeof(dely));
		maxdelyl = delyl;
		break;
    }
    kanfound = FALSE;
    if (sltfound == TRUE) {         /* SLT found */
        kanfound = TRUE;
        readdict2(seqval, maxdely, maxdelyl, delk, &delkl, hinsi);
        deluslt(delsegn);
    }
    else {                          /* SLT not found */
        dedsop = getdsorec(delsegn, (UWORD) recnt);
        seqq = *dedsop++;
        for (tseq = 0; tseq < seqq; tseq++) {
            readjwrd(dedsop, delyl, dely, &delkl, &delk[0], hinsi);
            if (hinsi[1] & 0x08) {
                kanfound = TRUE;
                break;
            }
            dedsop = skipkanji(1, dedsop);
        }
    }
    if (kanfound == TRUE)           /* kanji found */
        deletedict(dely, delyl, delk, delkl);
    else                            /* kanji not found */
        return (FALSE);
    return (TRUE);                  /* delete success */
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* deluslt                                                          */
/*                                                                  */
/* fuction                                                          */
/*   "deluslt" is a subroutine of "adddict" and "deletedict".       */
/*   "deluslt" clear user slt without main slt for a SLT segment    */
/* return                                                           */
/*   none                                                           */
/*------------------------------------------------------------------*/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

deluslt(dbsegn)
    UWORD           dbsegn;
{
/*--- automatic variavles ---*/
    register REG    i;
    register UBYTE *sltptr;
    UBYTE          *p;
    PTR_MDICT       pdarry;
#ifndef PMSLT
    UDDH            uhdr;
#endif
/*--- begin of deluslt ---*/
    pdarry = dicprty[0].pds;
    if (dbsegn > getint(pdarry->hbuhdr.uesegq))
        return;
#ifdef    PMSLT
    sltptr = GetSltp(pdarry, (dbsegn - 1) * maxslt * 2);
    p = (sltptr += ((maxslt * 2) - 2));
#else
    p = sltptr = pdarry->hbuhdr.udsltp + ((dbsegn) * (maxslt * 2)) - 2;
#endif
    for (i = maxslt; i--;) {
        if ((getint(sltptr) >> 9) > 63) {
            egcmemmove(sltptr, sltptr + 2, (maxslt - i) * 2);
            setint(p, 0);
        }
        sltptr -= 2;
    }
#ifdef    PMSLT
    PutSltp(pdarry, (dbsegn - 1) * maxslt * 2);
#else
    uhdr = dicprty[0].pds->hbuhdr;
    putuseg(0, uhdr.udsltp,
            (UWORD) getint(pdarry->hbuhdr.uesltl),
            (UWORD) getint(pdarry->hbuhdr.ueslts));
#endif
}

/*PAGE*/

/*------------------------------------------------------------------*/
/* putwoseg                                                         */
/*                                                                  */
/* fuction                                                          */
/*   "putwoseg" is a subroutine of "adddict" and "deletedict".      */
/*   "putwoseg" write back a DSO segment and it's extention to user */
/*   dictionary.                                                    */
/* return                                                           */
/*   none                                                           */
/*------------------------------------------------------------------*/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

putwoseg()
{
/*--- automatic variables ----*/
    register UBYTE *p;
    UWORD           n;
    register REG    i;
    UBYTE           dmabuff[128];
    PTR_MDICT       pdarry;

/*--- begin of putwoseg ---*/
    pdarry = dicprty[0].pds;
    i = (getint(pdarry->hbuhdr.uddsop) - 1) / (DSOBLKL - 2);
    n = 0;
    p = dmabuff;
    while (1) {
        egcmemmove(p ,
            pdarry->hbuhdr.uddsop + ((DSOBLKL - 2) * (UWORD) i), DSOBLKL - 2);
        setint(p + DSOBLKL - 2, n);
        n = dbdsos[i];
        if (!n) {
            n = getint(pdarry->hbuhdr.udexts);
            setint(pdarry->hbuhdr.udexts, n + DSOBLKL / UDBLKL);
            putuseg(0, (UBYTE *) & pdarry->hbuhdr, (UWORD) 1, 0);
        }
        putuseg(0, p, (UWORD) (DSOBLKL / UDBLKL), n);
        if (i)
            i--;
        else
            break;
    }
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* makemidashi                                                      */
/*                                                                  */
/* function                                                         */
/*   "makemidashi" is a subroutine of "adddict" and "deletedict".   */
/*   "makemidashi" create/update a PLO record by summing up hinshi  */
/*   of corresponding homonyms in DSO.                              */
/* return                                                           */
/*   none                                                           */
/*------------------------------------------------------------------*/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makemidashi(krecp, yomi, yomilen)
    UBYTE          *krecp;             /* pointer to DSO record */
    UBYTE          *yomi;              /* pointer to kana */
    UBYTE           yomilen;           /* length of kana */
{
/*--- automatic variables ---*/
    UBYTE           kanjilen;
    UBYTE           llen;
    REG             dupll;
    REG             uniql;
    WORD            dlen;
    register REG    i;
    register UBYTE *p;
    UBYTE          *mrecep;
    UBYTE          *np;
    UBYTE           h[4];
    UBYTE           hbitstr[4];
    UBYTE           kanji[MAXJKANJI];
    UBYTE           label[32];
    PTR_MDICT       pdarry;

/*--- begin of makemidashi ---*/
    pdarry = dicprty[0].pds;
    memset4(hbitstr, (UBYTE) 0x00);
    p = krecp;
    i = *(p++);
    while (i--) {
        p = readjwrd(p, yomilen, yomi, &kanjilen, kanji, h);
        hbitstr[0] |= h[0];
        hbitstr[1] |= h[1];
        hbitstr[2] |= h[2];
        hbitstr[3] |= h[3];
    }
    p = dbmrp;
    if (!(*krecp)) {
        np = readpwrd(p, &dupll, &uniql);
        np += uniql;
        if (dbrecn < getint(dbmbp)) {
            p = np;
            np = readpwrd(p, &dupll, &uniql);
            yomilen = (UBYTE) dupll + (UBYTE) uniql;
            egcmemmove(yomi + dupll, np, uniql);
            egcmemmove(hbitstr, readplh(p), (UBYTE) 4);
            dbdupl = (dbdupl < (UBYTE) dupll) ? dbdupl : (UBYTE) dupll;
        }
        setint(dbmbp, getint(dbmbp) - 1);
    }
    if (!(hbitstr[0] | hbitstr[1] | hbitstr[2] | hbitstr[3])) {
        llen = 0;
    }
    else {
        llen = makelrec(dbdupl, yomilen, yomi, hbitstr, label);
    }
    if (dbstat) {
        p = readpwrd(p, &dupll, &uniql);
        mrecep = p + uniql;
    }
    else {
        mrecep = p;
        setint(dbmbp, getint(dbmbp) + 1);
    }
#ifdef  PMPLO
    PutPlop(pdarry, dbsegn, dbmbp, 2);
#endif
    dlen = llen - (mrecep - dbmrp);
    p = (UBYTE *) (pdarry->hbuhdr.udidxp + dbsegn - 1);
    if (dlen) {
        UWORD   movel;
#ifdef  PMPLO
        UWORD   moveFr, moveTo;
#endif
        movel = (UWORD)getint(pdarry->hbuhdr.
                        udidxp[(UINT) getint(pdarry->hbuhdr.uesegq)].ieplop)
                    - (UWORD)getint(((PIE) p)->ieplop)
                    - (UWORD)(mrecep - dbmbp);
#ifdef  PMPLO
        moveFr = mrecep - dbmbp;
        moveTo = dbmrp + llen - dbmbp;
        MovePlop(pdarry, dbsegn, moveFr, moveTo, movel);
#else
        egcmemmove(dbmrp + llen, mrecep, movel);
#endif
    }
    if (llen) {
        egcmemmove(dbmrp, label, (UWORD)llen);
#ifdef  PMPLO
        PutPlop(pdarry, dbsegn, dbmbp, (UBYTE)((dbmrp + llen) - dbmbp));
#endif
    }
    if (dlen) {
        p += sizeof(GIEREC);
        while (TRUE) {
         setint(((PIE) p)->ieplop, getint(((PIE) p)->ieplop) + dlen);
            if (((PIE) p)->ieyomi[0] == 0xff)
                break;
            p += sizeof(GIEREC);
        }
        putuseg(0, (UBYTE *) pdarry->hbuhdr.udidxp,
                (UWORD) pdarry->hbuhdr.udidxl, (UWORD) pdarry->hbuhdr.udidxs);
    }
#ifndef PMPLO
    putuseg(0, pdarry->hbuhdr.udplop,
            (UWORD) getint(pdarry->hbuhdr.ueplol),
            (UWORD) getint(pdarry->hbuhdr.ueplos));
#endif
}
/*PAGE*/

/********************************************************************/
/* findkanji                                                        */
/*                                                                  */
/* function                                                         */
/*   "findkanji" find the homonym in DSO.                           */
/* return                                                           */
/*   if found : pointer to homonym record.                          */
/*   else     : null                                                */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findkanji(krecp, yomilen, yomi, kanjilen, kanji)
    UBYTE          *krecp;             /* pointer to DSO record */
    UBYTE           yomilen;           /* length of kana        */
    UBYTE          *yomi;              /* pointer to kana       */
    UBYTE           kanjilen;          /* length of kanaji      */
    UBYTE          *kanji;             /* pointer to kanji      */
{
/*--- automatic variables ---*/
    register UBYTE *p;
    register UBYTE *q;
    UBYTE           kjq;
    UBYTE           kjl;
    UBYTE           kjc[MAXJKANJI];
    UBYTE           kjh[4];

/*--- begin of findkanji ---*/
    p = krecp;
    kjq = *(p++);
    while (kjq) {
        q = readjwrd(p, yomilen, yomi, &kjl, kjc, kjh);
        if ((kanjilen == kjl) && (!memncmp(kanji, kjc, (UWORD) kjl)))
            break;
        p = q;
        kjq--;
    }
    return ((kjq) ? p : 0);
}
/*PAGE*/

/********************************************************************/
/* delkanji                                                         */
/*                                                                  */
/* function                                                         */
/*   "delkanji" delete a homonym element in DSO.                    */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

delkanji(krecp, kelmp)
    UBYTE          *krecp;             /* pointer to DSO record */
    UBYTE          *kelmp;             /* pointer to homonym record */
{
/*--- automatic variables ---*/
    register UBYTE *q;
    register UWORD  blkl;
    PTR_MDICT       pdarry;

/*--- begin of delkanji ---*/
    pdarry = dicprty[0].pds;
    q = skipkanji((UBYTE) 1, kelmp);
    blkl = getint(pdarry->hbuhdr.uddsop);
    egcmemmove(kelmp, q, blkl - (q - pdarry->hbuhdr.uddsop));
    setint(pdarry->hbuhdr.uddsop, blkl - (q - kelmp));
    (*krecp)--;
}
/*PAGE*/

/********************************************************************/
/* makekrec                                                         */
/* function                                                         */
/*   "makekrec" creats DS,DSO homonym record.                       */
/* return                                                           */
/*   length of record created                                       */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makekrec(kanjilen, kanji, hbitstr, krec)
    UBYTE           kanjilen;          /* length of kanji */
    UBYTE          *kanji;             /* pointer to kanji */
    UBYTE          *krec;              /* pointer to new record */
    UBYTE           hbitstr[4];        /* hinshi bit string */
{
/*--- automatic varuables ---*/
    register UBYTE *p;
    register REG    i;
    UBYTE           chartype;

/*--- begin of makekrec ---*/
    chartype = 0;
    p = kanji;
    if (kanjilen != (UBYTE)((REG)kanjilen & ~(0x0001))) {
        chartype = 3;
        kanjilen++;
    }
    else {
        for (i = kanjilen >> 1; i; i--) {
            if ((*p != 0x82) || (*(p + 1) < 0x9f))
                break;
            p += 2;
        }
        if (!i) {
            chartype = 2;
            kanjilen = 2;
        }
        else {
            p = kanji;
            for (i = kanjilen >> 1; i; i--) {
                if (!((*p == 0x83) && (*(p + 1) <= 0x96)
                      || (*p == 0x81) && (*(p + 1) == 0x5B))) {
                    break;
                }
                p += 2;
            }
            if (!i) {
                chartype = 1;
                kanjilen = 2;
            }
        }
    }

    p = krec;
    i = ecdhinshi(hbitstr);

    if (i && (i < 16)) {
        if (kanjilen <= 30) {
            *(p++) = (UBYTE)(i << 4) + (UBYTE)(kanjilen >> 1); /* DSO type A */
        } else {
            *(p ++) = (UBYTE)0xF0;
            *(p ++) = kanjilen;
            *(p ++) = (UBYTE)i + (UBYTE)0x80;
        }
    } else {
        if (kanjilen <= 30) {
            *(p++) = (UBYTE)(kanjilen >> 1);      /* DSO type B or C */
        } else {
            *(p ++) = (UBYTE)0xF0;                /* DSO type D or E */
            *(p ++) = kanjilen;
        }
        if (i <= MAXCODE) {
            *(p ++) = (UBYTE)i + (UBYTE)0x80;     /* DSO type B or D */
        } else {
            egcmemmove(p, hbitstr, 4);        /* DSO type C or E */
            p += 4;
        }
    }
    if (chartype == 3) {
        *p = chartype;
        egcmemmove(p + 1, kanji, (UWORD) (kanjilen - 1));
    }
    else if (!chartype) {
        egcmemmove(p, kanji, (UWORD) kanjilen);
    }
    else {
        *p = chartype;
        *(p + 1) = 0x0;
    }
    return ((UBYTE) ((p - krec) + kanjilen));
}
/*PAGE*/

/********************************************************************/
/* readpwrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "readpwrd" read yomi data stored in PL,PLO record.             */
/* return                                                           */
/*   pointer to unique yomi                                         */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readpwrd(yrec, yll, ylr)
    UBYTE          *yrec;              /* pointer to PL,PLO record    */
    REG            *yll;               /* pointer to duplicate length */
    REG            *ylr;               /* pointer to unique length    */
{
/*--- automatic variables ---*/
    REG             d;
    register REG    c;
    register UBYTE *p;

/*--- begin of readpwrd ---*/
    p = yrec;
    if ((c = *(p++)) & 0x1F) {
        *yll = ((c & 0x1F) >> 3) + 1;
        *ylr = c & 0x07;
    }
    else {
        *ylr = (d = *(p++)) & 0x0F;
        *yll = d >> 4;
    }
    if (!(c &= 0xE0)) {
        if (*(p++) < 128)
            p += 3;
    }
    return (p);
}
/*PAGE*/

/********************************************************************/
/* readplh                                                          */
/*                                                                  */
/* function                                                         */
/*   "readplh" reads hinshi information stored in PL,PLO record.    */
/* return                                                           */
/*   pointer to hinshi bit string                                   */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readplh(yrec)
    UBYTE          *yrec;              /* pointer to PL,PLO record */
{
/*--- automatic variables ---*/
    register REG    c;
    register UBYTE *p;

/*--- begin of readplh ---*/
    p = yrec;
    c = *(p++);
    if (!(c & 0x1F))
        p++;
    if (!(c &= 0xE0)) {
        if ((c = *p) >= 128) {
            p = dcdtbl[c - 128];
        }
    }
    else {
        p = dcdtbl[c >> 5];
    }
    return (p);
}
/*PAGE*/

/********************************************************************/
/* makelrec                                                         */
/*                                                                  */
/* function                                                         */
/*   "makelrec" creats PL,PLO record.                               */
/* return                                                           */
/*   length of record created                                       */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makelrec(lnyomil, yomilen, yomi, hbitstr, lrec)
    UBYTE           lnyomil;           /* duplicate length of yomi */
    UBYTE           yomilen;           /* length of yomi */
    UBYTE          *yomi;              /* pointer to yomi */
    UBYTE           hbitstr[4];        /* hinshi bit string */
    UBYTE          *lrec;              /* pointer to new record */
{
/*--- automatic varuables ---*/
    UBYTE           lnyomir;
    register REG    hcode;
    register UBYTE *p;

/*--- begin of makelrec ---*/
    p = lrec;
    hbitstr[(HCTANKAN >> 3)] &= (~(0x80 >> (HCTANKAN & 0x07)));
    hcode = ecdhinshi(hbitstr);
    if (lnyomil > 4)
        lnyomil = 4;
    lnyomir = yomilen - lnyomil;
    if ((lnyomir <= 7)
        && (lnyomir != 0)
        && (lnyomil != 0)) {
        *(p++) = (UBYTE)((lnyomil - (UBYTE)1) << 3) + lnyomir;
    }
    else {
        *(p++) = (UBYTE)0;
        *(p++) = (UBYTE)(lnyomil << 4) + lnyomir;
    }
    if (hcode && (hcode < 8)) {
        *lrec |= hcode << 5;
    }
    else if (hcode <= MAXCODE) {
        *(p++) = (UBYTE) (hcode + 128);
    }
    else {
        egcmemmove(p, hbitstr, 4);
        p += 4;
    }
    if (lnyomir)
        egcmemmove(p, yomi + lnyomil, (UWORD) lnyomir);
    return ((UBYTE) ((p - lrec) + lnyomir));
}
/*PAGE*/

/********************************************************************/
/* getdsrec                                                         */
/*                                                                  */
/* function                                                         */
/*   "getdsrec" reads DS segemnt,and locate the specified DS record.*/
/* return                                                           */
/*   pointer to the DS record.                                      */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

getdsrec(idno, segno, recno)
    WORD            idno;              /* dic id */
    UWORD           segno;             /* DS segment number */
    UWORD           recno;             /* record number in DS segment */
{
/*--- automatic variables ---*/
    register UBYTE *p;
    register UWORD  n;
    REG             c;

/*--- begin of getdsrec ---*/
    if ((p = getwseg(idno, segno)) == (UBYTE*)NULL) {
        return  p;
    }
    for (n = recno - 1; n--;) {
        if (*p >= 0xc0)
            p = skipkanji((UBYTE) 1, p);
        else
            p = skipkanji(*p, p + 1);
        switch (*p & 0xc0) {
        case 64:
            c = *(p++) - 64;
            break;
        case 128:
            p++;
            c = *(p++) + 64;
            break;
        default:
            c = 0;
        }
        p += (UWORD) c << 1;
    }
    return (p);
}
/*PAGE*/

/********************************************************************/
/* getdsorec                                                        */
/*                                                                  */
/* function                                                         */
/*   "getdsorec" reads DSO segemnt and it's extention, and locate   */
/*   the specified DSO record.                                      */
/* return                                                           */
/*   pointer to the DSO record.                                     */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

getdsorec(segno, recno)
    UWORD           segno;             /* DSO segment number */
    UWORD           recno;             /* record number in DSO segment */
{
/*--- automatic variables ---*/
    register UBYTE *p;
    register REG    i;
    UWORD           n;
    PTR_MDICT       pdarry;
    UWORD           edsos;

/*--- begin of getdsorec ---*/
    pdarry = dicprty[0].pds;
    p = pdarry->hbuhdr.uddsop;
    edsos = (pdarry->hbuhdr.uddsos == 255) ?
        getint(pdarry->hbuhdr.uedsos) : pdarry->hbuhdr.uddsos;
    n = edsos + (UWORD) segno - 1;
    for (i = 0; i < DSOEXT; i++) {
        dbdsos[i] = n;
        if (n) {
            getuseg(0, p, (UWORD) (DSOBLKL / UDBLKL), n);
            p += DSOBLKL - 2;
            n = getint(p);
        }
    }
    p = pdarry->hbuhdr.uddsop + 2;
    for (n = recno - 1; n--;)
        p = skipkanji(*p, p + 1);
    return (p);
}
/*PAGE*/

/********************************************************************/
/* readjwrd                                                         */
/*                                                                  */
/* function                                                         */
/*   "readjwrd" reads the a homonym element stored in DS or DSO.    */
/* return                                                           */
/*   pointer to the end of homonym record just read.                */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readjwrd(krec, yomil, yomi, kanjil, kanji, hbitstr)
    UBYTE          *krec;              /* pointer to homonym record (input) */
    UBYTE           yomil;             /* length of yomi (input) */
    UBYTE          *yomi;              /* yomi of homonym (input) */
    UBYTE          *kanjil;            /* length of kanji (output) */
    UBYTE          *kanji;             /* kanji of homonym (output) */
    UBYTE          *hbitstr;        /* hinshi of homonym (output) */
{
/*--- automatic variables ---*/
    register REG    c;
    register UBYTE *p;
    UBYTE          *hp;

/*--- begin of readjwrd ---*/
    p = krec;

    if (*p != (UBYTE)0xF0) {              /* DSO type A,B,C */
        if (c = ((*p ++) >> 4)) {
            hp = dcdtbl[c];
        } else if ((c = *p) >= 128) {
            hp = dcdtbl[c - 128];
            p++;
        } else {
            hp = p;
            p += 4;
        }
    } else {                              /* DSO type D,E */
        p += 2;
        if (*p & 0x80) {
            hp = dcdtbl[*(p ++) - (UBYTE)0x80];
        } else {
            hp = p;
            p += 4;
        }
    }

    egcmemmove(hbitstr, hp, 4);

    if (*krec != (UBYTE)0xF0) {
        c = (((*krec) & 0x0f) << 1);
    } else {
        krec ++;
        c = *krec;
    }
    if (*p >= 0x20) {
        egcmemmove(kanji, p, (UWORD) c);
        *kanjil = (UBYTE) c;
    }
    else if (*p == 3) {
        egcmemmove(kanji, p + 1, (UWORD) (c - 1));
        *kanjil = (UBYTE) (c - 1);
    }
    else {
        *kanjil = (UBYTE) cvtatoj(*p, yomi, yomil, kanji);
    }
    return (p + c);
}
/*PAGE*/

/********************************************************************/
/* skipkanji                                                        */
/*                                                                  */
/* function                                                         */
/*   "skipkanji" skips as many homonym elements as are specified by */
/*   "kanjiq" from the current position pointed by "kanjiq".        */
/* return                                                           */
/*   pointer to the end of homonynm records skiped.                 */
/********************************************************************/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

skipkanji(kanjiq, kanjip)
    UBYTE           kanjiq;            /* number of homonyms to be skipped */
    UBYTE          *kanjip;            /* pointer to homonym record */
{
    register REG    c;
    register UBYTE *p;

    p = kanjip;
    while (kanjiq--) {
        if (*p != (UBYTE)0xF0) {
            if ((!((c = *(p ++)) & 0xf0)) && (*(p ++) < 128)) {
                p += 3;
            }
            p += ((c & 0x0f) << 1);
        } else {
            p ++;
            c = *(p ++);
            if (!(*(p ++) & 0x80)) {
                p += 3;
            }
            p += c;
        }
    }
    return (p);
}
/*PAGE*/

/********************************************************************/
/* ecdhinshi                                                        */
/*                                                                  */
/* function                                                         */
/*   "ecdhinshi" will encode the hinshi bit string.                 */
/* return                                                           */
/*   code number if the hinshi bit string cannot be encoded         */
/*  "MAXCODE" will be returned                                      */
/********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

ecdhinshi(hbitstr)
    UBYTE           hbitstr[4];        /* hinshi bit string */
{
    register REG    c;
    register UBYTE *p;

    p = dcdtbl[0];
    for (c = 0; c <= MAXCODE; c++) {
        if (!((*p ^ *hbitstr)
              | (*(p + 1) ^ *(hbitstr + 1))
              | (*(p + 2) ^ *(hbitstr + 2))
              | (*(p + 3) ^ *(hbitstr + 3))))
            break;
        p += 4;
    }
    return ((UBYTE) c);
}
/*PAGE*/

/****************************************************************/
/* cleanheap                                                    */
/*                                                              */
/* function                                                     */
/*   "cleanhean" cuts all clause analysis trees but the tree    */
/*   whose root node table is pointed by "treetbl".             */
/* return                                                       */
/*   none                                                       */
/****************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

cleanheap(treetbl)
    PTNODTBL        treetbl;           /* pointer to node table */
{
    register PTNODTBL   pt;
    register PTNODTBL   qt;
    PTNODE              pn;
    REG                 i;

    if (!treetbl) {
        dbfree = dbheap + HEAPSIZE;
        dbcurt = 0;
    }
    else {
        dbfree = (UBYTE *) treetbl;
        pt = treetbl;
        while (1) {
            pn = pt->nodes;
            qt = 0;
            for (i = pt->nbrnode; i--;) {
                if (pn->ndchain)
                    qt = (PTNODTBL) pn->ndchain;
                pn++;
            }
            if (!qt) {
                break;
            }
            else {
                pt = qt;
            }
        }
        dbcurt = pt;
    }
}
/*PAGE*/

/****************************************************************/
/* newtreept                                                    */
/*                                                              */
/* function                                                     */
/*   "newtreept" allocates the space for node table form heap.  */
/* return                                                       */
/*   pointer to the heap space allocated                        */
/****************************************************************/
NONREC
PTNODTBL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

newtreept(memsize)
    UWORD           memsize;           /* node table size */
{
    register UBYTE *spt;
    register UBYTE *ept;

    if (dbcurt) {
        if (dbcurt->nbrnode) {
            spt = (UBYTE *) & (dbcurt->nodes[(int) (dbcurt->nbrnode)]);
        }
        else {
            spt = (UBYTE *) dbcurt;
        }
    }
    else {
        spt = dbheap;
    }
    ept = spt + memsize;
    if ((spt > dbfree) && (ept > dbheap + HEAPSIZE)) {
        ept = (spt = dbheap) + memsize;
    }
    if ((spt <= dbfree) && (ept > dbfree)) {
        return ((PTNODTBL) 0);
    }
    dbcurt = (PTNODTBL) spt;
    return (dbcurt);
}
/*PAGE*/

/********************************************************************/
/* ReadFromWid                                                      */
/*                                                                  */
/* function                                                         */
/*   "ReadFromWid" reads dictionary on the basis of the word ID.    */
/*                                                                  */
/* return                                                           */
/*   NORMAL : find data                                             */
/*   ERROR  : no data or disk I/O error                             */
/********************************************************************/
BOOL 
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

ReadFromWid(yStr, yl, kStr, kl, hn, wid)
    UBYTE*  yStr;                               /* yomi string      */
    UWORD*  yl;                                 /* yomi length      */
    UBYTE*  kStr;                               /* kanji string     */
    UWORD*  kl;                                 /* kanji length     */
    UBYTE*  hn;                                 /* hinshi string    */
    WORDID* wid;                                /* word ID          */
{
    register UBYTE  *p, *q;
    register PPR    wp;
    PLREC           wdtbl[MAXDWORD];            /* word table       */
    UBYTE           pwrdCnt = 0;                /* PL/PLO word count*/
    UBYTE           tmpYl, tmpKl;               /* temporary        */
    UWORD           segno, seqno, recno;        /* word ID member   */
    BOOL            found = FALSE;              /* flag             */

    segno = GETWIDSEG(wid->wisegno, wid->wiseqno);
    seqno = (UWORD)GETWIDSEQ(wid->wiseqno);
    recno = GETWIDREC(wid->wirecno);
    tmpYl = (UBYTE)*yl;

    wp = wdtbl;
    pwrdCnt = findpwrd(yStr, tmpYl, wdtbl, 0);
    if (pwrdCnt && (pwrdCnt != (UBYTE)0xFF)) {
        while (pwrdCnt --) {
            if (wp->jrsegno == segno
                && (wp->jrmrecn == recno || wp->jrurecn == recno)) {
                found = TRUE;
                *yl = wp->jrwordl;
                break;
            }
            wp ++;
        }
        if (! found)
            return  ERROR;
    } else {
        return  ERROR;
    }
    if (! ((wid->wirecno) & 0xF000)) {
        if (seqno <= 63)
            p = getdsrec(0, segno, wp->jrmrecn);
        else
            p = getdsorec(segno, wp->jrurecn);
    } else {
        p = getdsrec(1, segno, recno);
    }
    if (p == (UBYTE*)NULL)  return  ERROR;          /* disk I/O error   */
    if (! (*p & (UBYTE)0xC0)) {                     /* one or some ?    */
        p ++;
    }
    q = skipkanji((UBYTE)(seqno & 0x003F), p);
    tmpYl = (UBYTE)*yl;
    readjwrd(q, tmpYl, yStr, &tmpKl, kStr, hn);
    *kl = (UWORD)tmpKl;
    if ((hn[0] & hn[1] & hn[2] & hn[3]) == (UBYTE)0xFF) {
        hn[0] = wp->jrmhbit[0] | wp->jruhbit[0];
        hn[1] = wp->jrmhbit[1] | wp->jruhbit[1];
        hn[2] = wp->jrmhbit[2] | wp->jruhbit[2];
        hn[3] = wp->jrmhbit[3] | wp->jruhbit[3];
    }

    return  NORMAL;
}

/********************************************************************/
/* chkMultiSeg                                                      */
/*                                                                  */
/* function                                                         */
/*   "chkMultiSeg" compare global index, and returns result.        */
/*                                                                  */
/* return                                                           */
/*   TRUE  : current GI is multi segment                            */
/*   FALSE : not multi segment                                      */
/********************************************************************/
static BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

chkMultiSeg(idno, segno, index, yptr, ylen)
    WORD    idno;
    REG*    segno;
    PIE*    index;
    UBYTE*  yptr;
    REG     ylen;
{
    BOOL    multiseg = FALSE;
    UBYTE  *p, *plmpt;
    UBYTE   cur_yomi[MAXJYOMI], iyomi[MAXJYOMI];
    REG     dupll, uniql;
    PIE     tmpidx = *index;

    while (*segno > 1
        && memncmp(tmpidx[-1].ieyomi, tmpidx[0].ieyomi, 4) == 0) {
        multiseg = TRUE;
        plmpt = getlseg(idno, getint(tmpidx[0].ieplp),
                        getint(tmpidx[1].ieplp) - getint(tmpidx[0].ieplp));
        if (plmpt == (UBYTE*)NULL) {
            return  FALSE;
        }
        plmpt += 2;
        p = readpwrd(plmpt, &dupll, &uniql);
        egcmemset(cur_yomi, 0x00, MAXJYOMI);
        egcmemmove(cur_yomi, tmpidx->ieyomi, dupll);
        egcmemmove(cur_yomi + dupll, p, uniql);
        egcmemset(iyomi, 0x00, MAXJYOMI);
        egcmemmove(iyomi, yptr, ylen);
        if (memncmp(cur_yomi, iyomi, MAXJYOMI) <= 0)
            break;
        (*segno)--, (*index)--; tmpidx--;
    }
    return  multiseg;
}
