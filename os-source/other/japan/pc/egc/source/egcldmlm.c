/************************************************************************/
/*                                                                      */
/*      EGCLDMLm : EGConvert Low-level Data Manupulation Language       */
/*                                                                      */
/*              Designed    by  I.Iwata         01/29/1987              */
/*              Coded       by  I.Iwata         01/29/1987              */
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
/*  <public>                                                            */
/*      1  egctxcvt(sec,srcl,dst,dstl,sep,sepl,fixl,cmode)              */
/*      2  egcclcvt(src,srcl,dstr,dsep,dlid,dnbr,cmode)                 */
/*      3  egclearn(dlid)                                               */
/*      4  egclearn2(dlid,dnbr,tnbr)                                    */
/*      5  egcwdadd(ystr,yl,kstr,kl,uh)                                 */
/*      6  egcwddel(ystr,yl,kstr,kl)                                    */
/*      7  egccllearn(kStr,kl1mkl2,yStr,yl1,yl2,)                       */
/*                                                                      */
/*  <private>                                                           */
/*      1  DecodeElem()                                                 */
/*      2  MakeCvtFmt()                                                 */
/*      3  ClauseSort()                                                 */
/*      4  CompClause()                                                 */
/*                                                                      */
/************************************************************************/
#ifdef UNIX
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/dir.h>
#endif
#include    "edef.h"
#include    "egcenv.h"
#include    "egckcnv.h"
#include    "egcerr.h"
#include    "egcdef.h"
#include    "egcproto.h"
#include    "egcdict.h"
#include    "egcudep.h"
#include    <memory.h>

/*  char definition (shift JIS)  */
/*  row                          */
#define    SETTOU      0x41                 /* A */
#define    JIRITU      0x42                 /* B */
#define    SETUBI      0x43                 /* C */
#define    JYOSU       0x44                 /* D */
#define    KIGOU       0x45                 /* E */
#define    FUZOKU      0x47                 /* G */
#define    NOCONV      0x48                 /* H */
#define    KANJI       0x4A                 /* J */
#define    WAKACHI     0x57                 /* wakachi_go 'W' */

#define    HANKANA     0x03                 /* hankaku kana */
#define    HKANA       0x02                 /* hirakana     */
#define    KKANA       0x01                 /* katakana     */
#define    HANKAKU     0x00                 /* hankaku      */
#define    KUGIRI      0x04                 /* delimiter    */
#define    MAXNUM      8
#define    MAXHML      1024                 /* Maximum homonym buffer length */

#ifdef  DOS_EGBRIDGE
#include    "egcdctrl.h"
    WORD            dummy;
#endif

/*
** static functions
*/
static UWORD    DecordElem(
#ifdef P_TYPE
UBYTE*, UBYTE*,  UWORD*, UWORD*, UWORD*, UWORD*, UBYTE, UWORD
#endif
);
static WORD     MakeCvtFmt(
#ifdef  P_TYPE
UBYTE*, UWORD, UBYTE*, UWORD*
#endif
);
static VOID     ClauseSort(
#ifdef  P_TYPE
UBYTE*, WORDID*, UBYTE*, UBYTE*, UBYTE
#endif
);
static BOOL     CompClause(
#ifdef  P_TYPE
UWORD, UBYTE*, WORDID*, UWORD, UBYTE*, UBYTE*, WORDID*, BOOL, UWORD, UWORD*
#endif
);
static UBYTE    cvtZHclause(
#ifdef  P_TYPE
UBYTE*, UWORD, UBYTE*, WORDID*, UWORD
#endif
);

/*-------------------------------------------------------------------*/
/*        egctxcvt : egc text level conversion                       */
/*-------------------------------------------------------------------*/
NONREC
WORD    egctxcvt(src, srcl, dst, dstl, sep, sepl, fixl, cmode)
    UBYTE          *src;               /* -> source string                   */
    UWORD           srcl;              /* source string length               */
    UBYTE          *dst;               /* -> destination string              */
    UWORD          *dstl;              /* -> destination string length       */
    UWORD          *sep;               /* -> element buffer                  */
    UWORD          *sepl;              /* -> element number                  */
    UWORD          *fixl;              /* -> yomi fixed length               */
    UBYTE           cmode;             /* convert mode (0:batch 1:real time) */
                                       /*              (2:bufferd batch)     */
{                                      /* f/c = 0:normal !=0 error */
#ifdef MAC
    static  UBYTE   tempstr[1024];     /* temporary string area              */
    static  UBYTE   skanji[3072];      /* kanji string area                  */
#else
#ifdef  BHREAD
    UBYTE           tempstr[1024];
    static UBYTE    skanji[3072];
#else
    UBYTE           tempstr[1024];     /* temporary string area              */
    UBYTE           skanji[3072];      /* kanji string area                  */
#endif
#endif
    UWORD           fixcl;             /* convert fixed length               */
    UWORD           lkanji;            /* output kanji string length         */
    UWORD           fmtlen;            /* kanatokanji format length          */
    UWORD           maxlen;            /* max output length                  */
#ifdef  BHREAD
    BOOL            decode = TRUE;
#endif
    UBYTE           mode;

#ifdef  BHREAD
    if (cmode == 2) {
        decode = FALSE;
        mode = cmode;
    } else {
        decode = TRUE;
        mode = cmode;
    }
#else
    if (cmode != 0 && cmode != 1) {
        *fixl = 0;
        *dstl = 0;
        *sepl = 0;
        return  ERROR;
    } else {
        mode = cmode;
    }
#endif  /* BHREAD */

#ifdef  DOS_EGBRIDGE
    maxlen = 1024;
#else
    maxlen = 1024;
#endif
    *dstl = *sepl = *fixl = (UWORD) 0;
    if (MakeCvtFmt(src, srcl, tempstr, &fmtlen) != NORMAL) {
        return    ERTXC03;
    }

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_READ_OPEN)) {
        return  dummy;
    }
#endif

#if (defined(DOS_EGBRIDGE) && !defined(BHREAD))
    fixcl = kanatokanji(fmtlen, tempstr, &lkanji, src, mode);
#else
    fixcl = kanatokanji(fmtlen, tempstr, &lkanji, skanji, mode);
#endif

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_READ_CLOSE)) {
        return  dummy;
    }
#endif

#ifdef  BHREAD
    if (decode) {
#endif

#if (defined(DOS_EGBRIDGE) && !defined(BHREAD))
        *fixl = DecordElem(src, tempstr, dstl, sep, sepl,
                           &fixcl, cmode, maxlen);
#else
        *fixl = DecordElem(skanji, dst, dstl, sep, sepl,
                           &fixcl, cmode, maxlen);
#endif

#ifdef  BHREAD
    } else {
        *fixl = 0;
        *dstl = 0;
        *sepl = 0;
    }
#endif

    if (*fixl == (UWORD)0xFFFF) {
        *fixl = (UWORD)0;
        if (*dstl == (UWORD)0xFFFF) {
            *dstl = (UWORD)0;
            return  ERTXC10;
        } else {
            *sepl = (UWORD)0;
            return  ERTXC11;
        }
    }

#if (defined(DOS_EGBRIDGE) && !defined(BHREAD))
    egcmemmove(dst, tempstr, 1024);
#endif

#ifdef  BHREAD
    egcreconv(0);
#endif

    return    NORMAL;
}

/*-------------------------------------------------------------------*/
/*        egcclcvt : egc clause level convertion                     */
/*-------------------------------------------------------------------*/
NONREC
WORD    egcclcvt(src, srcl, dstr, dsep, dlid, dnbr, cmode)
    UBYTE          *src;               /* source string                */
    UWORD           srcl;              /* source string length         */
    UBYTE          *dstr;              /* destination string           */
    UBYTE          *dsep;              /* destination separator        */
    PTWORDID        dlid;              /* destination learning id      */
    UWORD          *dnbr;              /* nbr of destinations          */
    UBYTE           cmode;             /* convert mode(0:clause 1:homonym
                                        * 2:homonym special) */
 /* f/c = 0:normal !=0 error */
{
    register UWORD  i, j, m;
    WORDID          wtemp;
    UBYTE           stemp[MAXJKANJI * 2];
    UBYTE           btemp;
    BOOL            sflag;
    BOOL            ntokflag = TRUE;
    UBYTE           dst[80];
    UWORD           ltemp;
    UWORD           c = (UWORD)0;
    UWORD           ascl;
    BOOL            tk;
    UWORD           numl = 0;
    BOOL            clauseStat, ntokStat;
    UBYTE           busyNum = dicprty[0].pds->hbuhdr.busyNum;
    BOOL            busyNumFlag = TRUE;
    UWORD           gobil = (UWORD)0;
    UWORD           katacllen = 0;
    UBYTE           cvtHch = dicprty[0].pds->hbuhdr.cvtHch;
    UBYTE           cvtHal = dicprty[0].pds->hbuhdr.cvtHal;
    BOOL            ZHcvt = FALSE;

#ifndef DOS_EGBRIDGE
    UBYTE           tbaray[256];
#endif

    if (srcl == 0)
        return (egcerr(ERCLC03));                    /* yomi length error */

    if (strcheck(src, srcl) == ERROR)
        return  egcerr(ERCLC02);

    ascl = jistoasc(src, src + srcl, dst);
    if (ntokflag = NtoKCheck(src, srcl, &numl)) {
        if ((srcl > 32) || (ascl > 16)) {
            return (egcerr(ERCLC03));                /* yomi length error */
        }
    } else {
        if (srcl > MAXCL * 2)
            return (egcerr(ERCLC03));                /* yomi length error */
    }

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_READ_OPEN)) {
        return (dummy);
    }
#endif

    egcjacvt(src, srcl, stemp, &ltemp);
    clauseStat = findclause((UBYTE)ltemp, stemp);   /* convertion */

    if (clauseStat && !ntokflag) {
#ifdef  DOS_EGBRIDGE
        if (dummy = CallDhundle(DH_READ_CLOSE)) {
            return  dummy;
        }
#endif
        return  egcerr(ERCLC01);                    /* conversion error */
    }

    if (cmode != 2) {
        i = j = m = 0;
    } else {
        if (ntokflag) {
            i = 1; m = 0;
        } else {
            i = m = 1;
        }
        j = dsep[0];
        dlid[0].wisegno = SETWIDSEG(0);
    }

    clauseStat = FALSE;
    ntokStat = TRUE;
#ifndef DOS_EGBRIDGE
    egcmemset(tbaray, 0x00, 256);
#endif

    while (!clauseStat || !ntokStat) {
        if (ntokflag && clauseStat) {
            if (busyNumFlag) {
                numtokansu(busyNum, src, numl, stemp, &ltemp);
                busyNumFlag = FALSE;
            } else {
                if (m == (UWORD)busyNum) {
                    m++;
                    if (m == MAXNUM)
                        ntokStat = TRUE;
                    continue;
                }
                numtokansu((UBYTE)m, src, numl, stemp, &ltemp);
                m++;
            }
            btemp = (UBYTE)ltemp;
            wtemp.wisegno = 0;
            wtemp.wiseqno = (m == (UWORD)0)? (busyNum + (UBYTE)1) : (UBYTE)m;
            wtemp.wirecno = 0;
        } else if (! clauseStat) {
            if (clauseStat = readclause(&btemp, stemp, &wtemp, &tk)) {
                if (ntokStat) {
                    btemp = cvtZHclause(src, srcl, stemp, &wtemp, katacllen);
                    if (btemp)
                        ZHcvt = TRUE;
                }
            }
        }
        if ((!clauseStat && (btemp <= (MAXJKANJI * 2)))
            || (clauseStat && !ntokStat && ntokflag) || ZHcvt) {
            if (GETWIDSEG(wtemp.wisegno, wtemp.wiseqno)
                && IsIncluding(stemp, btemp, KKANA)) {
                register UWORD  i;
                register UWORD  vi = 0;
                katacllen = 0;
                for (i = 0; i < btemp; i += 2) {
                    if (((*(stemp + i) == 0x83) &&
                        (*(stemp + i + 1) >= 0x40 && *(stemp + i + 1) <= 0x96))
                        || (*(stemp + i) == 0x81 && *(stemp + i + 1) == 0x5b)){
                        if (*(stemp + i) == 0x83 && *(stemp + i + 1) == 0x94)
                            vi += 2;
                    } else {
                        break;
                    }
                }
                if ((btemp + vi) == srcl)
                    katacllen = i + vi;
                wtemp.wirecno |= PLBLANK;
            }
            sflag = FALSE;
            if (i > 0) {
                sflag = CompClause(i, stemp, &wtemp, btemp,
                                   dstr, dsep, dlid, clauseStat, gobil, &c);
            }
            if (!sflag
                    && !(ntokflag && (GETWIDSEQ(wtemp.wiseqno) == DSBLANK))) {
                if (j + (UWORD)btemp > MAXHML) {
                    break;
                }
                egcmemmove(dstr + j, stemp, (UWORD)btemp);
                if (clauseStat && ntokflag) {
                    egcajcvt(src + numl, srcl - numl, 
                                dstr + j + btemp, &ltemp, (UBYTE)2);
                    gobil = ltemp;
                    btemp += (UBYTE)ltemp;
                }
                j += btemp;
                egcmemmove((UBYTE*)(dlid + i), (UBYTE*)&wtemp, sizeof(WORDID));
                dsep[i] = btemp;
                tbaray[i] = ((tk)? (UBYTE)1 : (UBYTE)2);
                i ++;
                if (i >= 255) {
                    break;
                }
            }
            else if ((cmode == 2) &&
                     ((c == 0) &&
                      (GETWIDSEG(dlid[0].wisegno, dlid[0].wiseqno) == 0))) {
                egcmemmove((UBYTE *) dlid, (UBYTE *) & wtemp, sizeof(WORDID));
            }
            if (clauseStat && (! ntokStat) && ntokflag && cmode == 0) {
                break;
            }
        }

        clauseStat = (cmode) ? clauseStat : TRUE;
        if (cmode == 0 && i > 0) {
            UWORD   al, kl, l;
            if (IsIncluding(src, srcl, HANKAKU)) {
                egcmemmove(dstr, src, srcl);
                dsep[0] = (UBYTE)srcl;
            } else if ((al = IsStrAlpha(src, srcl))
                    || (kl = IsStrKKana(src, srcl))) {
                if ((al && cvtHal) || (kl && cvtHch)) {
                    l = (al && cvtHal)? al : kl;
                    egcjacvt(src, l, dstr, (UWORD*)&btemp);
                    egcmemmove(dstr + btemp, src + l, srcl - l);
                    dsep[0] = btemp + (UBYTE)(srcl - l);
                } else {
                    egcmemmove(dstr, src, srcl);
                    dsep[0] = (UBYTE)srcl;
                }
            }
            else if (katacllen && cvtHch) {
                if (btemp = cvtZHclause(src, srcl, dstr, dlid, katacllen))
                    dsep[0] = btemp;
            }
            break;
        }
        if (clauseStat) {
            ntokStat = FALSE;
        }
        if (!ntokflag || (ntokflag && (m == MAXNUM))) {
            ntokStat = TRUE;
        }
        wtemp.wisegno = 0;
        wtemp.wiseqno = 0;
        wtemp.wirecno = 0;
    }

    *dnbr = i;

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_READ_CLOSE)) {
        return  dummy;
    }
#endif

    ClauseSort(dstr, dlid, dsep, tbaray, cmode);

    return (NORMAL);
}
#ifndef DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/*      egclearn : homonym learning (old version)                   */
/*------------------------------------------------------------------*/
NONREC
WORD egclearn(dlid)
    PTWORDID        dlid;              /* -> destination larning id */
{
    if (GETWIDSEG(dlid->wisegno, dlid->wiseqno) == 0)
        return (ERLER02);           /* input error */
    agedict(dlid);
    return (NORMAL);
}
#endif  /* DOS_EGBRIDGE */
/*------------------------------------------------------------------*/
/*      egclearn2 : homonym learning (new version)                  */
/*------------------------------------------------------------------*/
NONREC
WORD    egclearn2(dlid, dnbr, tnbr)
    PTWORDID        dlid;              /* -> destination learning id */
    UWORD           dnbr;              /* nbr of destinations        */
    UWORD           tnbr;              /* target of learning number  */
{
    UWORD           segno;
    UBYTE           seqno;
    UWORD           recno;
    register UWORD  i;
    BOOL            formula = FALSE;

    if (tnbr >= dnbr)
        return (egcerr(ERLN202));   /* input error */

    segno = GETWIDSEG(dlid[tnbr].wisegno, dlid[tnbr].wiseqno);
    seqno = GETWIDSEQ(dlid[tnbr].wiseqno);
    recno = dlid[tnbr].wirecno;

    if (segno == 0) {
        if ((seqno && seqno != DSBLANK) && recno == 0) {
            for (i = 0; i < dnbr; i ++) {
                segno = GETWIDSEG(dlid[i].wisegno, dlid[i].wiseqno);
                recno = GETWIDREC(dlid[i].wirecno);
                if (segno == 0 && recno == 0)
                    break;
            }
            dicprty[0].pds->hbuhdr.busyNum = seqno - (UBYTE)1;
        	formula = TRUE;
        }
    }
    if (seqno == DSBLANK && recno == 0) {
        dicprty[0].pds->hbuhdr.cvtHch = (UBYTE)FALSE;
      	formula = TRUE;
    } else if (seqno == 0 && recno == PLBLANK) {
        dicprty[0].pds->hbuhdr.cvtHch = (UBYTE)TRUE;
      	formula = TRUE;
    } else if (recno == (PLBLANK | ALPHID)) {
        dicprty[0].pds->hbuhdr.cvtHal = (UBYTE)TRUE;
      	formula = TRUE;
    } else if (recno & ALPHID) {
        dicprty[0].pds->hbuhdr.cvtHal = (UBYTE)FALSE;
      	formula = TRUE;
    }

    if (formula) {
#ifdef  BHREAD
        egcreconv(1);
#endif
#ifdef  DOS_EGBRIDGE
        if (dummy = CallDhundle(DH_UPDATE_OPEN)) {
            return (dummy);
        }
#endif
        if (ageformula() == ERROR) {
            return  egcerr(ERLN205);
        }
        if (recno & ~ALPHID) {
            dlid[tnbr].wirecno &= ~ALPHID;
            agedict2(dlid, dnbr, tnbr);
        }
#ifdef  DOS_EGBRIDGE
        if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
            return  dummy;
        }
#endif
        return  NORMAL;
    }

    if (dlid[tnbr].wirecno & PLBLANK) {
        dicprty[0].pds->hbuhdr.cvtHch = (UBYTE)FALSE;
        dlid[tnbr].wirecno &= ~PLBLANK;
    }

    recno = GETWIDREC(dlid[tnbr].wirecno);
    if (((dlid[tnbr].wirecno) & 0xF000) != 0)
        return (egcerr(ERLN203));   /* selected expert input error */

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_OPEN)) {
        return (dummy);
    }
#endif

    agedict2(dlid, dnbr, tnbr);

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
        return  dummy;
    }
#endif

#ifdef  BHREAD
    egcreconv(1);
#endif

    return (NORMAL);
}
/*PAGE*/

/*----------------------------------------------------------------------*/
/*     egcwdadd : add word to dictionary                                */
/*----------------------------------------------------------------------*/
NONREC
WORD    egcwdadd(ystr, yl, kstr, kl, uh)
    UBYTE          *ystr;              /* yomi string          */
    UWORD           yl;                /* yomi string length   */
    UBYTE          *kstr;              /* kanjj string         */
    UWORD           kl;                /* kanji string length  */
    UBYTE          *uh;                /* user hinsi joho      */
{
    register UWORD  i;
    WORD            st;
    BOOL            flag;
    UWORD           dstl;
    UBYTE           dst[32];
    UBYTE          *yomip;
    UBYTE*          pdest;
    UWORD           lnbr;
    UBYTE           ldst[1024];
    UBYTE           dcep[256];
    WORDID          wid[256];

    if (kl > MAXJKANJI)
        return (egcerr(ERWDA05));   /* kanji length error */

    if (!(uh[0] | uh[1] | uh[2] | uh[3]))
        return (egcerr(ERWDA06));   /* hinshi error */

    if ((st = egcjacvt(ystr, yl, dst, &dstl)) == ERROR) {
        return (egcerr(ERWDA02));   /* yomi string error */
    }
    if (dstl > MAXJYOMI)
        return (egcerr(ERWDA03));   /* yomi length error */

    yomip = dst;
    if (! YStrChk(yomip, dstl)) {
        return  egcerr(ERWDA02);
    }

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_OPEN)) {
        return  dummy;
    }
#endif

    flag = adddict(dst, (UBYTE) dstl, kstr, (UBYTE) kl, uh);

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
        return  dummy;
    }
#endif

    if (flag)
        return (egcerr(flag));   /* user dict full error */

    pdest = ldst;
    if (egcclcvt(ystr, yl, ldst, dcep, wid, &lnbr, 1) == NORMAL) {
        for (i = 0; i < lnbr; pdest += dcep[i++]) {
            if (memncmp(pdest, kstr, dcep[i]) == 0
                    && (UWORD)dcep[i] == kl) {
                if (egclearn2(wid, lnbr, i) != NORMAL) {
                    return (egcerr(ERCLL01));   /* learning error  */
                } else {
                    break;                      /* learning sucess */
                }
            }
        }
    }

#ifdef  BHREAD
    egcreconv(1);
#endif

    return (NORMAL);
}

/*-----------------------------------------------------------------*/
/*     egcwddel : delete word to dictionary                        */
/*-----------------------------------------------------------------*/
NONREC
WORD    egcwddel(ystr, yl, kstr, kl)
    UBYTE          *ystr;              /* yomi string         */
    UWORD           yl;                /* yomi string length  */
    UBYTE          *kstr;              /* kanjj string        */
    UWORD           kl;                /* kanji string length */
{
    UWORD           st;
    UWORD           dstl;
    UBYTE           dst[32];

    st = egcjacvt(ystr, yl, dst, &dstl);
    if (st)
        return (egcerr(ERWDD02));   /* yomi string error */
    if (dstl > MAXJYOMI)
        return (egcerr(ERWDD03));   /* yomi length error */

#ifndef UNIX
    if (!ctrlchk(kstr, kl))
        return (ERWDD04);           /* kanji string error */
#endif

    if (kl > MAXJKANJI)
        return (egcerr(ERWDD05));   /* kanji length error */

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_OPEN)) {
        return  dummy;
    }
#endif

    if (! finddict(dst, (UBYTE) dstl))
        return (egcerr(ERWDD20));   /* yomi not find error */

    if (deletedict(dst, (UBYTE) dstl, kstr, (UBYTE) kl) == ERROR)
        return  ERWDD21;           /* kanji not find error */

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
        return  dummy;
    }
#endif

#ifdef  BHREAD
    egcreconv(1);
#endif

    return (NORMAL);
}
/*PAGE*/

/*--------------------------------------------------------------*/
/*      egccllearn                                              */
/*--------------------------------------------------------------*/
NONREC
WORD    egccllearn(kStr, kl1, kl2, yStr, yl1, yl2)
    UBYTE          *kStr;              /* Kanji Start          */
    UWORD           kl1;               /* first  Kanjilength   */
    UWORD           kl2;               /* second Kanjilength   */
    UBYTE          *yStr;              /* Yomi Start           */
    UWORD           yl1;               /* first  Yomilength    */
    UWORD           yl2;               /* second Yomilength    */
{
    UBYTE           ascs[MAXCL * 2];   /* --> ascll-string     */
    UBYTE           delStr[MAXCL * 2]; /* delete Strings       */
    UBYTE           hinsi[4];          /* chack hinsi          */
    UBYTE           uh[4];             /* set hinsi            */
    UBYTE           tascs[MAXCL * 2];  /* --> temp ascllstring */
#ifdef MAC
    static	UBYTE	dst[1024];
    static	UBYTE	dcep[512];
    static	WORDID	wid[512];
#else
    UBYTE           dst[1024];
    UBYTE           dcep[512];
#ifdef  DOS_EGBRIDGE
    WORDID          wid[256];
#else
    WORDID          wid[512];
#endif
#endif
    UBYTE          *pdest;
    UBYTE           btempl;
    UWORD           kl;                /* kl1 + kl2            */
    UWORD           ascl;              /* F/Val ascstringlen   */
    UWORD           yl;                /* yl1 + yl2            */
    UWORD           countdic;
    UWORD           lnbr;
    UWORD           i;                 /* counter              */
    WORD            RetCode;
    UBYTE           kanji[MAXJKANJI + 1];
    UWORD           yomil, kanjil;
    UBYTE           hinshi[4];

    if (strcheck(yStr, yl1 + yl2) == ERROR)
        return  egcerr(ERCLL02);

    yl = yl1 + yl2;
    kl = kl1 + kl2;
    uh[0] = 0x00;
    uh[1] = 0x08;
    uh[2] = 0x00;
    uh[3] = 0x00;
    pdest = dst;
    if (yl2 != 0) {
        if (!(RetCode = egcclcvt(yStr + yl1, yl2, dst, dcep, wid, &lnbr, 1))) {
            for (i = 0; i < lnbr; pdest += dcep[i++]) {
                if (!memncmp(pdest, kStr + kl1, dcep[i])) {
                    break;
                }
            }
            if (i != lnbr && (GETWIDSEQ((wid + i)->wiseqno) != (UBYTE)63)) {
                memset(ascs, 0x00, MAXCL * 2);
                memset(kanji, 0x00, MAXJKANJI + 1);
                ascl = jistoasc(yStr + yl1, yStr + yl1 + yl2, ascs);
#ifdef  DOS_EGBRIDGE
                if (dummy = CallDhundle(DH_READ_OPEN)) {
                    return (dummy);
                }
#endif
                if (ReadFromWid(ascs, &ascl, kanji, &kanjil,
                                hinshi, wid + i) == NORMAL) {
                    egcajcvt(ascs, ascl, yStr + yl1, &yomil, (UBYTE)2);
                    kl = yl = 0;
                    yl = yl1 + yomil;
                    kl = kl1 + kanjil;
                    uh[0] |= hinshi[0];
                    uh[1] |= hinshi[1];
                    uh[2] |= hinshi[2];
                    uh[3] |= hinshi[3];
                }
#ifdef  DOS_EGBRIDGE
                if (dummy = CallDhundle(DH_READ_CLOSE)) {
                    return  dummy;
                }
#endif
            }
        }
    }

    if (kl > MAXJKANJI) {
        return  egcerr(ERCLL05);            /* kanji length error */
    }

    ascl = jistoasc(yStr, yStr + yl, ascs);
    if (ascl > MAXJYOMI) {
        return  egcerr(ERCLL03);            /* yomi length error  */
    }

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_OPEN)) {
        return  dummy;
    }
#endif

    egcmemmove(tascs, ascs, ascl);
    countdic = (UWORD) finddict(tascs, (UBYTE) ascl);
    for (i = (UWORD) 1; i <= countdic; i++) {
        readdict2((UBYTE) i, tascs, (UBYTE)ascl, delStr, &btempl, hinsi);
        if (hinsi[1] & 0x08) {
            if (deletedict(tascs, (UBYTE)ascl, delStr, btempl) == ERROR) {
#ifdef  DOS_EGBRIDGE
                if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
                    return  dummy;
                }
#endif
                return  ERROR;
            } else {
                break;
            }
        }
    }

    if (adddict(ascs, (UBYTE)ascl, kStr, (UBYTE)kl, uh)) {
#ifdef  DOS_EGBRIDGE
        if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
            return  dummy;
        }
#endif
        return  egcerr(ERCLL01);                /* clause learning error */
    }

#ifdef  DOS_EGBRIDGE
    if (dummy = CallDhundle(DH_UPDATE_CLOSE)) {
        return  dummy;
    }
#endif

#ifndef DOS_EGBRIDGE
    pdest = dst;
    if ((RetCode = egcclcvt(yStr, yl, dst, dcep, wid, &lnbr, 1)) == NORMAL) {
        for (i = 0; i < lnbr; pdest += dcep[i++]) {
            if (memncmp(pdest, kStr, dcep[i]) == 0
                    && (UWORD)dcep[i] == kl) {
                if (egclearn2(wid, lnbr, i) != NORMAL) {
                    return (egcerr(ERCLL01));   /* learning error  */
                } else {
                    break;                      /* learning sucess */
                }
            }
        }
    }
#endif

#ifdef  BHREAD
    egcreconv(1);
#endif

    return  NORMAL;
}
/*PAGE*/

/*-----------------------------------------------------------------*/
NONREC
WORD    egcerr(noerr)
	WORD    noerr;
/*-----------------------------------------------------------------*/
{
#ifdef  DOS_EGBRIDGE
    return  ERROR;
#else
    return  noerr;
#endif
}

/*-------------------------------------------------------------------*/
/*        DecodeElem : Decode element (egctxcvt sub)                 */
/*-------------------------------------------------------------------*/
static UWORD    DecordElem(skanji, dst, dstl, sep, sepl, fixcl, cmode, maxlen)
    UBYTE*    skanji;
    UBYTE*    dst;
    UWORD*    dstl;
    UWORD*    sep;
    UWORD*    sepl;
    UWORD*    fixcl;
    UBYTE     cmode;
    UWORD     maxlen;
{
    UBYTE           ctype = (UBYTE)JIRITU,
                    ptype;          /* current/previous type/mode */
    UWORD           i, j;
    register UWORD  k, l, m;
    UWORD           elnbr;
    UWORD           fixyl;          /* yomi fixed length                  */
    UWORD           di, si;         /* destination/source bunsetu length  */
    UWORD           dj, sj;         /* destination/source bunsetu length  */
    UBYTE           ntemp[MAXJKANJI + MAXTAIL * 2];
    UBYTE           tempstr[MAXJKANJI + MAXTAIL * 2];
    UWORD           numl = 0;
    UWORD           tmpNuml;
    BOOL            IsNum;
    UBYTE           busyNum = dicprty[0].pds->hbuhdr.busyNum;
    BOOL            hankana = dicprty[0].pds->hbuhdr.cvtHch;
    BOOL            hanalph = dicprty[0].pds->hbuhdr.cvtHal;

    di = si = fixyl = (UWORD) 0;
    elnbr = *((UWORD *) skanji);    /* element number */
    k = (UWORD) 2;                  /* skanji index */
    l = (UWORD) 0;                  /* dst index    */
    m = (UWORD) 0;                  /* sep index    */
    ptype = (UBYTE) 0;

    for (i = (UWORD) 0; i < elnbr; i++) {
        ctype = *(skanji + (k++));
        if (((ctype == (UBYTE) JIRITU) && (ptype != (UBYTE) SETTOU)) ||
             (ctype == (UBYTE) SETTOU) || (ctype == (UBYTE) SETUBI) ||
                (ctype == (UBYTE) JYOSU) || (ctype == (UBYTE) NOCONV) ||
                (ctype == (UBYTE) KIGOU) || (ctype == (UBYTE) KANJI)
                || (ctype == (UBYTE) WAKACHI)) {
            if (i > (UWORD) 0) {
                sep[m * 3] = (UWORD) ptype;
                sep[m * 3 + 1] = di;
                sep[m * 3 + 2] = si;
                if (++m > MAXCLAUSE) {
                    *sepl = (UWORD)0xFFFF;
                    goto    OVERFLOW;
                }
                di = si = (UWORD) 0;
            }
        }
        sj = dj = (UWORD) 0;
        if (ctype == (UBYTE) NOCONV) {
            IsNum = FALSE;
            dj = sj = (UWORD) (*(skanji + k) - 2);
            fixyl += (*fixcl) ? (UWORD) (*(skanji + k) - 2) : (UWORD) 0;
            (*fixcl) -= (UWORD) (*(skanji + k));
            k += (UWORD) 3;
        }
        else if (ctype == (UBYTE) KANJI) {
            IsNum = FALSE;
            sj = dj = (UWORD) (*(skanji + k));
            fixyl += (*fixcl) ? (UWORD) (*(skanji + k)) : (UWORD) 0;
            (*fixcl) -= (WORD) (*(skanji + k));
            k += (UWORD) 2;
        }
        else {
            IsNum = TRUE;
            j = (UWORD) (*(skanji + (k++)));
            dj = (UWORD) (*(skanji + (k++)));
            for (numl = 0; numl < dj; numl += 2) {
                UBYTE   HiNum, LowNum;
                HiNum  = *(skanji + k + j + numl);
                LowNum = *(skanji + k + j + numl + 1);
                if (HiNum != (UBYTE)0x82 
                    || (LowNum < (UBYTE)0x4f || (UBYTE)0x58 < LowNum)) {
                    IsNum = FALSE;
                    break;
                }
            }
            egcajcvt(skanji + k, j, tempstr, &sj, (UBYTE) 2);
            k += j;
            fixyl += (*fixcl) ? sj : (UWORD) 0;
        }
        if (IsNum && numl <= 32) {
            numtokansu(busyNum, skanji + k, numl, ntemp, &tmpNuml);
            if (l + tmpNuml > maxlen) {
                *dstl = (UWORD)0xFFFF;
                goto    OVERFLOW;
            }
            egcmemmove(dst + l, ntemp, tmpNuml);
            di += tmpNuml;
            si += sj;
            l  += tmpNuml;
            k  += dj;
        } else {
			register UWORD	kl = 0, ml = 0;
            if ((hankana && (kl = IsStrKKana(skanji + k, dj)))
                || (hanalph && (kl = IsStrAlpha(skanji + k, dj)))) {
                UWORD   zlen;
                ml = dj - kl;
				egcjacvt(skanji + k, kl, tempstr, &zlen);
                if (l + zlen > maxlen) {
                    *dstl = (UWORD)0xFFFF;
                    goto    OVERFLOW;
                }
                egcmemmove(dst + l, tempstr, zlen);
/* Next 1 line modefied at 1993/02/03 by H.Yanata
** becouse half size conversion break code
**
**              egcmemmove(dst + l + zlen, tempstr + kl, ml);
*/
                egcmemmove(dst + l + zlen, skanji + k + kl, ml);
				di += (zlen + ml);
                si += sj;
                l += (zlen + ml);
            } else {
                if ((l + dj) > maxlen) {
                    *dstl = (UWORD)0xFFFF;
                    goto    OVERFLOW;
                }
                di += dj;
                si += sj;
                egcmemmove(dst + l, skanji + k, dj);
                l += dj;
            }
            k += dj;
        }
        if (ctype == (UBYTE) NOCONV) {
            k++;
        }
        ptype = ctype;
    }                               /* end of for loop */
    sep[m * 3] = (UWORD) ctype;
    sep[m * 3 + 1] = di;
    sep[m * 3 + 2] = si;
    m++;

    if (cmode == (UBYTE) 1) {
        if ((m == (UWORD) 1) || ((m == (UWORD) 2) &&
                  ((ctype == (UBYTE) SETUBI) || (ctype == (UBYTE) FUZOKU)))) {
            m = (UWORD) 0;
            fixyl = (UWORD) 0;
            l = (UWORD) 0;
        }
    }

    *dstl = l;
    *sepl = m;
    return    fixyl;

OVERFLOW:
    return  (UWORD)0xFFFF;
}                                      /* end of main subroutine */
/*PAGE*/

/*-------------------------------------------------------------------*/
/*        MakeCvtFmt : make format for kanatokanji (egctxcvt sub)    */
/*-------------------------------------------------------------------*/
static WORD    MakeCvtFmt(src, srcl, dst, dstl)
    UBYTE*    src;                          /* source string            */
    UWORD     srcl;                         /* source string length     */
    UBYTE*    dst;                          /* destnation buffer        */
    UWORD*    dstl;                         /* destnation length        */
{
    UBYTE           ctype, ptype = HKANA; /* current/previous type/mode */
    register UWORD  i = 0, j = 0, k = 0;
    UWORD           cvtlen;

    if (srcl == (UWORD)0 || srcl > 1024) {
        return    ERROR;    /* yomi length error */
    }
    k = 0;
    while (i < srcl) {
        ctype = codecheck(ptype, src + i);
        if ((k >= (MAXCL + 1)) || ((k >= MAXCL) && (ctype != HANKAKU))) {
            k = 0;
            if (((src[i] == 0xDE) || (src[i] == 0xDF)) && (ptype == HANKAKU)) {
                dst[j + 1] = dst[j - 1];
                dst[j - 1] = KUGIRI;
                dst[j] = KUGIRI;
                j += 2;
                k += 2;
            }
            else {
                dst[j ++] = KUGIRI;
                if (ctype != HKANA) {
                    dst[j ++] = KUGIRI;
                    k ++;
                }
            }
        }
        else if (ctype != ptype) {
            dst[j ++] = KUGIRI;
            k = 0;
            if (ptype == HKANA)
                k ++;
            else if (ctype != HKANA) {
                dst[j ++] = KUGIRI;
                k ++;
            }
        }
        if (ctype == HKANA) {
            egcjacvt(src + i, 2, dst + j, &cvtlen);
            j += cvtlen;
            i += 2;
        }
        else {
            dst[j ++] = *(src + (i ++));
            k ++;
            if (ctype == KKANA) {
                dst[j ++] = *(src + (i ++));
                k ++;
            }
        }
        ptype = ctype;
    }                               /* end of while loop */

    if (ctype != (UBYTE) HKANA) {
        dst[j ++] = KUGIRI;         /* last delimiter */
    }
    *dstl = j;
    return    NORMAL;
}

/* PAGE */
#ifndef  DOS_EGBRIDGE
/*-------------------------------------------------------------------*/
/*      ClauseSort : move tankanji to backward (egcclcvt sub)        */
/*-------------------------------------------------------------------*/
static VOID     ClauseSort(src, dlid, sep, tb, cmd)
    UBYTE*  src;                        /* word block                   */
    WORDID* dlid;                       /* word ID block                */
    UBYTE*  sep;                        /* separate information block   */
    UBYTE*  tb;                         /* tankanji information         */
    UBYTE   cmd;                        /* convertion mode              */
{
    register UWORD      i = 0;
    UBYTE               tk;
    UWORD               srcl = 0, tksrcl = 0;
    UWORD               dlidl = 0, tkdlidl = 0;
    UWORD               sepl = 0, tksepl = 0;
    UBYTE               tmpsrc[1024];
    WORDID              tmpdlid[256];
    UBYTE               tmpsep[256];
    register WORDID*    srcdlid = dlid;
    UBYTE*              srcsrc = src;
    UBYTE*              srcsep = sep;
    register WORDID*    dstdlid = tmpdlid;
    UBYTE*              dstsrc = tmpsrc;
    UBYTE*              dstsep = tmpsep;

    if (cmd == (UBYTE)2) {
        i++;
        egcmemmove((UBYTE*)dstdlid, (UBYTE*)srcdlid, sizeof(WORDID));
        srcdlid++;
        dstdlid++;
        egcmemmove(dstsrc, srcsrc, sep[0]);
        srcsrc += sep[0];
        dstsrc += sep[0];
        egcmemmove(dstsep, srcsep, sizeof(UBYTE));
        srcsep += sizeof(UBYTE);
        dstsep += sizeof(UBYTE);
        sepl += sizeof(UBYTE);
        srcl += sep[0];
        dlidl += sizeof(WORDID);
    }

    while (tk = tb[i]) {
        if (tk == 1) {
            egcmemmove(dstsrc + tksrcl, srcsrc, sep[i]);
            egcmemmove((UBYTE*)dstdlid + tkdlidl,
                       (UBYTE*)srcdlid, sizeof(WORDID));
            egcmemmove(dstsep + tksepl, srcsep, sizeof(UBYTE));
            tksrcl += (UWORD)sep[i];
            tkdlidl += sizeof(WORDID);
            tksepl += sizeof(UBYTE);
        } else {
            egcmemmove(dstsrc + sep[i], dstsrc, tksrcl);
            egcmemmove(dstsrc, srcsrc, sep[i]);
            egcmemmove((UBYTE*)dstdlid + sizeof(WORDID),
                       (UBYTE*)dstdlid, tkdlidl);
            egcmemmove((UBYTE*)dstdlid, (UBYTE*)srcdlid, sizeof(WORDID));
            egcmemmove(dstsep + sizeof(UBYTE), dstsep, tksepl);
            egcmemmove(dstsep, srcsep, sizeof(UBYTE));
            dstsrc += sep[i];
            dstdlid++;
            dstsep += sizeof(UBYTE);
        }
        srcsrc += sep[i];
        srcdlid++;
        srcsep += sizeof(UBYTE);
        srcl += sep[i];
        dlidl += sizeof(WORDID);
        sepl += sizeof(UBYTE);
        i++;
    }
    egcmemmove(src, tmpsrc, srcl);
    egcmemmove((UBYTE*)dlid, (UBYTE*)tmpdlid, dlidl);
    egcmemmove(sep, tmpsep, sepl);
}
#endif  /* DOS_EGBRIDGE */
/* PAGE */
/*------------------------------------------------------------------*/
/*  CompClause : Compare clause and change word id (egcclcvt sub)   */
/*------------------------------------------------------------------*/
static BOOL CompClause(clcnt, clw, wid, cll,
                       dstr, dsep, dlid, clst, gobil, chgn)
    UWORD       clcnt;                  /* clause count             */
    UBYTE*      clw;                    /* current clause string    */
    WORDID*     wid;                    /* current word id          */
    UWORD       cll;                    /* clause length            */
    UBYTE*      dstr;                   /* egcclcvt destination buffer      */
    UBYTE*      dsep;                   /* egcclcvt destination separater   */
    WORDID*     dlid;                   /* egcclcvt word id buffer  */
    BOOL        clst;                   /* egcclcvt clause status   */
    UWORD       gobil;                  /* numeral clause gobi length       */
    UWORD*      chgn;                   /* changed id number        */
{
    register UWORD      wloc;           /* word location            */
    register UWORD      wcnt;           /* word search counter      */
    register WORDID*    ptlid;          /* word id pointer          */
    UBYTE               wlen;           /* word length              */
    BOOL                sflag;          /* word search flag         */
    LONG                wold;           /* weight of old word       */
    LONG                wnew;           /* weight of new word id    */

    ptlid = dlid;
    wloc = 0;
    sflag = FALSE;
    for (wcnt = 0; (wcnt < clcnt) && (!sflag); wcnt++) {
        if (! clst)
            wlen = dsep[wcnt];
        else
            wlen = dsep[wcnt] - (UBYTE)gobil;   /* 0 < gobil < 19 */
        if ((UWORD)wlen == cll) {
            register UBYTE  i;
            sflag = TRUE;
            for (i = 0; (i < wlen) && (sflag); i++) {
                if (dstr[wloc + i] != clw[i]) {
                    sflag = FALSE;
                }
            }
            if (sflag) {
                wold = GETWIDVAL(ptlid->wisegno,
                                 ptlid->wiseqno,
                                 ptlid->wirecno);
                wnew = GETWIDVAL(wid->wisegno,
                                 wid->wiseqno,
                                 wid->wirecno);
                if (wnew && GETWIDSEQ(wid->wiseqno) != 63 && (wold > wnew)) {
                    egcmemmove((UBYTE*)ptlid, (UBYTE*)wid, sizeof(WORDID));
                }
                *chgn = wcnt;
            }
        }
        wlen = dsep[wcnt];
        wloc += wlen;
        ptlid++;
    }
    return  sflag;
}
/* PAGE */
/*------------------------------------------------------------------*/
/*  cvtZHclause : Zenkaku-Hankaku conversion (egcclcvt sub)         */
/*------------------------------------------------------------------*/
static UBYTE    cvtZHclause(src, srcl, dst, dstid, cllen)
    UBYTE*  src;                    /* source string                */
    UWORD   srcl;                   /* source length                */
    UBYTE*  dst;                    /* destnation buffer            */
    WORDID* dstid;                  /* word id buffer               */
    UWORD   cllen;                  /* conversion length            */
{
    register UWORD  i;
    UWORD           l;
    UBYTE           cvtl = 0;
    BOOL            IncHanKana  = FALSE; /* flag of hankana_char in string  */
    BOOL            IncKataKana = FALSE; /* flag of katakana_char in string */
    BOOL            IncHanKaku  = FALSE; /* flag of hankaku_char in string  */

    IncHanKana  = IsIncluding(src, srcl, HANKANA);
    IncKataKana = IsIncluding(src, srcl, KKANA);
    IncHanKaku  = IsIncluding(src, srcl, HANKAKU);

    if (IncKataKana || IncHanKana || (cllen && !IncHanKaku)) {
        if (IncKataKana || (cllen && !IncHanKaku)) {
            if (cllen) {
                i = cllen;
            } else if (IncKataKana) {
                i = IsStrKKana(src, srcl);
            }
            egcjacvt(src, i, dst, &l);
            dstid->wisegno = 0;
            dstid->wiseqno = 0;
            dstid->wirecno = PLBLANK;
        } else if (IncHanKana) {
            for (i = 0; i < srcl; i ++) {
                if (!IsIncluding(src + i, 1, HANKAKU))
                    break;
            }
            egcajcvt(src, i, dst, &l, 1);
            dstid->wisegno = 0;
            dstid->wiseqno = DSBLANK;
            dstid->wirecno = 0;
        }
        egcmemmove(dst + l, src + i, srcl - i);
        cvtl = (UBYTE)(l + (srcl - i));
    } else {
        cvtl = 0;
    }
    return  cvtl;
}
