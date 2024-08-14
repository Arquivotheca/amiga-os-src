/************************************************************************/
/*                                                                      */
/*      EGCKTOK : EGConvert Kana TO Kanji                               */
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
#include "egcdef.h"
#include "egckcnv.h"
#include "egcexp.h"
#include "egcudep.h"
#include "egcproto.h"

extern DICIDTBL dicstat[MAXOPENDIC];
extern DICIDTBL dicprty[MAXAPENDIC];

/*******************************************************************/
/*            Global Variables                                     */
/*******************************************************************/
/*
**  input string contorol
*/
UBYTE          *ystart;                /* current pointer to input string   */
UBYTE          *yend;                  /* pointer to end of input string    */
UBYTE           spoint;                /* second clause start point         */
UBYTE           currow;                /* row of proceeding clause          */
UBYTE           dqtff;                 /* double quote flip flop            */
UBYTE           sqtff;                 /* single quote flip flop            */
/*
**  output string contorol
*/
UBYTE          *kstart;                /* current pointer to output string  */
UBYTE          *kend;                  /* pointer to end of output buffer   */
UWORD           wordct;                /* word count                        */
#ifdef  BHREAD
UBYTE           prvkcnv;               /* previous kanji convert length     */
UBYTE           prvycnv;               /* previous yomi convert kength      */
UWORD           prvWc;                 /* previous word count               */
#endif
/*
**  clause type table
*/
UBYTE           clausq;                /* clause type quantity              */
CLAUSREC        clstbl[MAXCLAUS + 1];  /* clasue type tabel                 */
/*
**  clause format table
*/
UBYTE           clfrmq = 0;            /* clause format quantity            */
CLTYPE          clftbl[MAXPTYPE];      /* clause format table               */
/*
**  jiritsu-go / setsubi-go description
*/
UBYTE           wdclas;                /* word type                         */
UBYTE           wdhead;                /* settou-go length                  */
UBYTE           wdwrdq;                /* word quantity                     */
UBYTE          *wdyptr;                /* pointer to yomi of word           */
UBYTE           wdmask[4];             /* hinshi mask                       */
PLREC           wdwtbl[MAXWDTBL];      /* word table                        */
/*
**  external variables shared by "findclause" and "readclause"
*/
UBYTE           newcl = TRUE;
UBYTE           kkana;                 /* convert to kata-kana              */
UBYTE           hkana;                 /* convert to hira-kana              */
UBYTE           khankaku;              /* convert to kata_hankaku           */
UBYTE           hhankaku;              /* convert to hira_hankaku           */
UBYTE           cllen;                 /* clause yomi length                */
UBYTE           clkana[MAXCL + 10];    /* clause yomi                       */
UBYTE           cltypr;                /* clause format count               */
/*
**  external variavles shared by "makenode" and "setnode"
*/
UBYTE           nct;                   /* counter of nodes                  */
PTNODE          npt;                   /* pointer to node                   */
NODEREC         node;                  /* skelton of node                   */
/*
**  external variavles shared by "makeptbl", "makefclause" and "setclause"
*/
PTNODE          jwnd;                  /* pointer to root node              */
UBYTE           wpath[MAXLEVEL];       /* clause analysis tree path         */
UWORD           setuji;                /* type of setsuji                   */
UBYTE           xclausq;               /* clause type quantity (STATEX)     */
BOOL            xflag;                 /* STATEX flag                       */
/*
**  external variavles shared by "priority" and "getplength"
*/
UBYTE           sclen;                 /* 2nd clause length                 */
/*
**  external variavles shared by "kakatokanji", "selectclause", "cvtclause"
**   and "makeptbl"
*/
extern UWORD    dbsegn;                /* segment value in egcdios.c        */
extern UBYTE    maxslt;                /* max slt value in egcdios.c        */
UBYTE           jsclen;                /* josusi or setubi clause length    */
UBYTE           jsjlen;                /* josusi or setubi jiritsugo length */
UBYTE           jshinshi;              /* josusi or setubi hinshi code      */
UBYTE           jsrow;                 /* josusi or setubi row              */
UBYTE           jsflag;                /* josusi or setubi flag             */
UBYTE           jsclausq;              /* josusi or setubi clause quantity  */
UWORD           jsmrecn;               /* setubi rec number                 */

/*
**  external variavles shared by "selectclause"
*/
UBYTE           cmode;                 /* kana to kanji convert mode        */
/*PAGE*/

/*******************************************************************/
/*       Tables                                                    */
/*******************************************************************/
/*
**   character type table
*/
UBYTE           chrtbl[256] =
{
/*  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
    0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     /* 1 */
    6, 1, 5, 1, 1, 1, 1, 1, 6, 5, 1, 1, 6, 1, 1, 1,     /* 2 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 3 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 4 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 5, 1, 1,     /* 5 */
    6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /* 6 */
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
/*
**   taigen connect matrix
*/
static UBYTE    distbl[12][11] =
{
/*   ft  jm  cm  dm  jm  kn  su  st  sb  js  nm */
    {02, 02, 02, 02, 02, 02, 02, 02, 13, 02, 02},       /* 171 renyou */
    {14, 02, 02, 02, 02, 02, 13, 02, 13, 02, 13},       /* 172 futsuu */
    {14, 02, 02, 02, 16, 02, 02, 02, 14, 02, 02},       /* 173 jinmei */
    {14, 02, 14, 02, 02, 02, 02, 02, 02, 02, 15},       /* 174 chimei */
    {02, 02, 02, 13, 02, 02, 02, 02, 13, 02, 02},       /* 175 daimei */
    {14, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02},       /* 176 jinmei */
    {13, 02, 15, 02, 02, 15, 02, 02, 02, 02, 02},       /* 177 kenmei */
    {13, 02, 02, 02, 02, 02, 15, 02, 02, 15, 02},       /* 178 suu    */
    {14, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02},       /* 179 settou */
    {13, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02},       /* 180 setsubi */
    {13, 02, 02, 02, 02, 02, 14, 02, 14, 02, 14},       /* 181 josuu  */
    {13, 02, 02, 02, 02, 02, 02, 02, 02, 15, 15}        /* 182 numeral */
};
/*PAGE*/
static UBYTE    rowtbl[183] =
{
/*  0    1    2    3    4    5    6    7    8    9 */
    000, 006, 006, 004, 006, 000, 006, 006, 006, 006,   /* 00 */
    006, 006, 002, 006, 000, 006, 006, 002, 006, 006,   /* 01 */
    006, 006, 006, 006, 003, 006, 006, 003, 006, 000,   /* 02 */
    006, 000, 000, 000, 006, 006, 000, 006, 000, 006,   /* 03 */
    000, 000, 006, 000, 000, 000, 006, 000, 000, 006,   /* 04 */
    000, 006, 000, 000, 000, 000, 006, 004, 000, 006,   /* 05 */
    006, 000, 000, 000, 006, 006, 006, 006, 000, 000,   /* 06 */
    006, 000, 000, 006, 006, 006, 000, 006, 000, 006,   /* 07 */
    000, 006, 006, 000, 000, 000, 006, 000, 006, 000,   /* 08 */
    000, 000, 000, 000, 006, 000, 006, 006, 000, 006,   /* 09 */
    003, 006, 000, 000, 000, 004, 000, 000, 004, 000,   /* 10 */
    000, 000, 000, 000, 000, 000, 003, 000, 000, 000,   /* 11 */
    000, 000, 000, 000, 000, 006, 000, 000, 000, 000,   /* 12 */
    000, 000, 003, 000, 000, 004, 000, 000, 000, 000,   /* 13 */
    000, 000, 004, 000, 006, 000, 000, 000, 000, 000,   /* 14 */
    000, 000, 006, 006, 006, 005, 005, 003, 000, 000,   /* 15 */
    000, 003, 000, 006, 000, 003, 003, 000, 000, 006,   /* 16 */
    000, 001, 001, 001, 001, 001, 001, 001, 001, 001,   /* 17 */
    001, 001, 001                                       /* 18 */
};
UBYTE    dbacod[4][2] = {
    {0x00, 0x00},
    {0x82, 0xa8},
    {0x82, 0xb2},
    {0x91, 0xe6}
};                                     /* settou-go kanji code "o" "go" "dai" */
/*PAGE*/

/**********************************************************************/
/* kanatokanji                                                        */
/*                                                                    */
/* function                                                           */
/*   "kanatokanji" converts given kana string to kanji string.        */
/* return                                                             */
/*   none                                                             */
/**********************************************************************/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

kanatokanji(kanalen, kana, kanjilen, kanji, mod)
    UWORD           kanalen;           /* length of input string */
    UBYTE          *kana;              /* pointer to input string */
    UWORD          *kanjilen;          /* pointer to length of output string */
    UBYTE          *kanji;             /* pointer to ouput string */
    UBYTE           mod;
{
/* automatic variables */
    register PTCLAUS clause;
    register PTNODTBL jtree;
    PTNODTBL        stree = (PTNODTBL)0;
    register UBYTE  i;
    UBYTE          *kanaend;
    UBYTE           maxclen = 3;
    UBYTE           maxpq = 0;
    BOOL            first;
    UBYTE*          ystp;
    UBYTE*          kstp;
#ifdef  BHREAD
    static UBYTE    CnvStr[512] = {0};
    static UWORD    yCnvLen = 0;
    static UWORD    kCnvLen = 0;
    static UWORD    wdCount = 0;
    BOOL            bakword = FALSE;
    UWORD           kcvtl, ycvtl;
#endif

    cmode = mod;
    ystp = ystart = kana;
    kstp = kstart = kanji + 2;
    kend = kanji + MAXOUTBUF;
    wordct = 0;
    dqtff = 0;
    sqtff = 0;
    jtree = 0;
    kanaend = kana + kanalen;
    jsclen = 0;
    jsflag = 0;

#ifdef  BHREAD
    prvycnv = prvkcnv = 0;
    prvWc = 0;
    if ((yCnvLen <= kanalen)
        && (memncmp(ystart, CnvStr, yCnvLen) == 0)
            && (dicprty[0].pds->reconv == FALSE)) {
        ystart += yCnvLen;
        kstart += kCnvLen;
    } else {
        yCnvLen = kCnvLen = wdCount = 0;
        egcmemset(CnvStr, 0x00, 512);
    }
#endif

    while (ystart < kanaend) {
        cleanheap(jtree);
        yend = (jsclen) ? ystart + jsclen :
            ((kanaend <= ystart + MAXKCNV) ? kanaend : ystart + MAXKCNV);
        spoint = 0;
        if (!jtree) {
            currow = 0;
            if (jtree = newjtree((UBYTE) 0)) {
                makeftree(jtree, (UBYTE) 0, (UBYTE) 1);
            }
            else {
                jtree = wakachi();
            }
        }
        if (!makeptbl(jtree)) {
            if (jtree->nodes[0].ndclass == CSYUB) {
                jtree = wakachi();
            }
            else {
                jtree->nodes[0].ndterm = 3;
            }
            currow = 0;
            makeptbl(jtree);
        }
        clause = clstbl;
        first = TRUE;
        jsclausq = clausq;
        for (i = 1; i <= clausq; i++) {
            if ((first) && (clause->clstat != STATEX)) {
                first = FALSE;
                maxclen = (clause->clendpt < 3) ? (UBYTE) 3
                    : (UBYTE) clause->clendpt;
                maxpq = (clause->clstat == STATET) ? (UBYTE)128 : (UBYTE)0;
            }
            if (clause->clstat == STATEP) {
                if (clause->clendpt != spoint) {
                    if ((maxclen - (UBYTE)3) > clause->clendpt)
                        maxpq = 128;
                    spoint = clause->clendpt;
                    if ((spoint == 1 && maxpq >= 18) || maxpq >= 20) {
                        clausq = (UBYTE) i;
                        break;
                    }
                    if (clause->clrow == 108)
                        maxpq = 128;
                    if (stree = newjtree(spoint)) {
                        makeftree(stree, spoint, (UBYTE) 1);
                        if (stree->nodes[0].ndclass != CSYUB)
                            maxpq = 128;
                    }
                }
                clause->clchain = stree;
            }
            priority(clause);
            if ((clause->clprtygrm + clause->clprtysel > maxpq) &&
                (clause->clstat != STATEX)) {
                maxpq = clause->clprtygrm + clause->clprtysel;
            }
            clause++;
        }
        clause = selectclause();
        if (mod == 1 || mod == 2) {
            if (((ystart + clause->clprtylen + 3 > kanaend)
                && (!jsclen || jsflag))
                    || (*(chrtbl + *(kanaend - 1)) == CHKINSOK)) {
#ifdef  BHREAD
                prvycnv = prvkcnv = 0;
                prvWc = 0;
#endif
                break;
            }
        }
        if (cvtclause(clause, jtree)) {
            break;
        }
        if (jsflag) {
            jtree = 0;
            jsflag = 0;
        }
        else {
            jtree = clause->clchain;
            currow = clause->clrow;
        }
        ystart += clause->clendpt;
    }

#ifdef  BHREAD
    wdCount += wordct;
    *(UWORD *) kanji = wdCount;
    yCnvLen = (ystart - kana);
    ycvtl = ystart - kana;
    kcvtl = kstart - kanji;
    if ((0 < yCnvLen && yCnvLen <= 512)
            && ((mod == 2) || (yCnvLen - prvycnv > 0))) {
        bakword = TRUE;
        yCnvLen -= prvycnv;
        wdCount -= prvWc;
        ycvtl -= prvycnv;
        kcvtl -= prvkcnv;
        egcmemmove(CnvStr, kana, yCnvLen);
    }
    if (kstart - kanji == 2) {
        kCnvLen = 0;
    } else {
        kCnvLen = (kstart - kanji - 2);
        if (bakword)
            kCnvLen -= prvkcnv;
    }
    *kanjilen = kcvtl;
    return  ycvtl;
#else
    *(UWORD *) kanji = wordct;
    *kanjilen = kstart - kanji;
    return ((UWORD) (ystart - kana));
#endif
}
/*PAGE*/
/**********************************************************************/
/* findclause                                                         */
/*                                                                    */
/* function                                                           */
/*   "findclause" gets all clause format of given kana string.        */
/* return                                                             */
/*   TRUE  :  no clause created                                       */
/*   FALSE :  ok                                                      */
/**********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

findclause(yomilen, yomi)
    UBYTE           yomilen;           /* length of yomi */
    UBYTE          *yomi;              /* address of yomi */
{
/*--- automatic variables ---*/
    UBYTE           clauseno;
    PTNODTBL        jtree;
    PTCLAUS         clause;
    register PTNODE node;
    register PTCLTYPE cltype;

/*--- begin of findclause ---*/
    if (yomilen > MAXCL)
        return (TRUE);

    cllen = yomilen;
    egcmemmove(clkana, yomi, (UWORD) yomilen);
    cleanheap((PTNODTBL) 0);
    ystart = yomi;
    yend = yomi + yomilen;
    currow = 255;
    if (jtree = newjtree((UBYTE) 0)) {
        makeftree(jtree, (UBYTE) 0, (UBYTE) 1);
        makeptbl(jtree);
    }
    else {
        clausq = 0;
    }
    cltype = clftbl;
    clfrmq = 0;
    if (clausq) {
        clause = clstbl;
        for (clauseno = clausq; clauseno--; clause++) {
            if ((clause->clendpt == yomilen) || (*yomi < 0x80)) {
                node = clause->cljwnd;
                if ((clfrmq == 0)
                    || (node->nddicid != cltype->ctdicid)
                    || (node->ndlength != cltype->ctjlen)
                    || (node->ndhdlen != cltype->ctalen)) {
                    if (clfrmq == 0) {
                        cltype = clftbl;
                    }
                    else if (clfrmq < MAXPTYPE) {
                        cltype++;
                    }
                    else {
                        break;
                    }
                    cltype->ctdicid = node->nddicid;
                    cltype->ctclass = node->ndclass;
                    cltype->ctalen = node->ndhdlen;
                    cltype->ctjlen = node->ndlength;
                    cltype->ctsegno = node->ndsegno;
                    cltype->ctmrecn = 0;
                    cltype->cturecn = 0;
                    memset4(cltype->ctmhbit, (UBYTE) 0x00);
                    memset4(cltype->ctuhbit, (UBYTE) 0x00);
                    clfrmq++;
                }
                if (node->ndmrecn) {
                    cltype->ctmrecn |= node->ndmrecn;
                    cltype->ctmhbit[(int) ((node->ndhinshi) >> 3)] |=
                        (UBYTE) (0x80) >> ((node->ndhinshi) & 0x07);
                }
                if (node->ndurecn) {
                    cltype->cturecn |= node->ndurecn;
                    cltype->ctuhbit[(int) ((node->ndhinshi) >> 3)] |=
                        (UBYTE) (0x80) >> ((node->ndhinshi) & 0x07);
                }
            }
        }
    }
    cltypr = 0;
    DimSet(clftbl, SelectSeg(clftbl, clfrmq), clfrmq);
    newcl = TRUE;
    kkana = hkana =
        (YTypeChk(yomi) != ILLEGAL_YOMI)? (UBYTE)TRUE : (UBYTE)FALSE;
    khankaku = hhankaku =
        (YTypeChk(yomi) == EXTRA_YOMI)? (UBYTE)TRUE : (UBYTE)FALSE;
    return (FALSE);
}
/*PAGE*/
/**********************************************************************/
/* readclause                                                         */
/* function                                                           */
/*   "readclause" returns a homonym of a clause format generated by   */
/*   "findclause".                                                    */
/* return                                                             */
/*   TRUE  : no clause left                                           */
/*   FALSE : otherwise                                                */
/**********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

readclause(clen, clause, wordid, tankan)
    UBYTE          *clen;              /* length of clause */
    UBYTE          *clause;            /* kanji string of clause */
    PTWORDID        wordid;            /* word id */
    BOOL           *tankan;            /* TRUE : kanji FALSE : word */
{
/*--- automatic variables ---*/
    UWORD           seqno = 0;
    UBYTE           jwlen = 0;
    UBYTE           jylen = 0;
    UBYTE           aylen = 0;
    UBYTE           found = 0;
    UBYTE           hbitstr[4];
    PTCLTYPE        cltype = (PTCLTYPE)NULL;
    register UBYTE *kmark = (UBYTE*)NULL;
    UBYTE          *jmark = (UBYTE*)NULL;
    UBYTE          *p = (UBYTE*)NULL;
    UBYTE           jword[MAXJKANJI];
    BOOL            tankj = FALSE;
    UWORD           didtmp = 0;

/*--- begin of readclause ---*/
    found = 0;
    if (cltypr < clfrmq) {
        while ((!found) && (cltypr < clfrmq)) {
            cltype = SetCLearn(&clftbl[0], &(clftbl[(int) cltypr]), &newcl);
            aylen = cltype->ctalen;
            jylen = cltype->ctjlen;
            jmark = clkana + aylen;
            if (cltype->ctclass == CSYUB) {
                UBYTE   learned;
                seqno = readdwrd(cltype->ctdicid, jylen, jmark,
                                 &jwlen, jword, hbitstr, &learned);
                if (seqno == 512) {
                    cltype++;
                    cltypr++;
                    newcl = TRUE;
                    continue;
                }
                p = (seqno < 256) ? cltype->ctmhbit : cltype->ctuhbit;
                if (((hbitstr[0] & p[0]) | (hbitstr[1] & p[1]) |
                     (hbitstr[2] & p[2]) | (hbitstr[3] & p[3])) ||
                    (((cllen == jylen) && (jwlen == 2)) &&
                     (hbitstr[0] | hbitstr[1] | hbitstr[2] | hbitstr[3])) ||
                    (hbitstr[0] == 0xFF && hbitstr[1] == 0xFF &&
                     hbitstr[2] == 0xFF && hbitstr[3] == 0xFF)) {
                    found = TRUE;
                    if ((!(hbitstr[0] | hbitstr[1] | hbitstr[2] | hbitstr[3]) 
                            || hbitstr[1] == 0x04) && !learned)
                        tankj = TRUE;
                    else
                        tankj = FALSE;
                    wordid->wisegno = SETWIDSEG(cltype->ctsegno);
                    wordid->wirecno = (cltype->ctmrecn)
                        ? cltype->ctmrecn
                        : cltype->cturecn + MAXPLE;
                    wordid->wiseqno = SETWIDSEQ(cltype->ctsegno, seqno);
                    didtmp = cltype->ctdicid << 12;
                    wordid->wirecno |= didtmp;
                    break;
                }
                else if ((cllen == jylen) && (jwlen == 2)) {
                    tankj = TRUE;
                    wordid->wisegno = 0;
                    wordid->wiseqno = 0;
                    wordid->wirecno = 0;
                    didtmp = cltype->ctdicid << 12;
                    wordid->wirecno |= didtmp;
                    found = TRUE;
                    break;
                }
            }
            else {
                found = TRUE;
                tankj = TRUE;
                wordid->wisegno = 0;
                wordid->wirecno = SETWIDSEQ(0, 63);
                if (YTypeChk(clkana) == EXTRA_YOMI) {
                    wordid->wirecno = ALPHID;
                } else {
                    wordid->wirecno = 0x0000;
                }
                wordid->wiseqno = 0;
                cltypr++;
                break;
            }
        }                           /* while */
    }
    kmark = clause;
    if (found) {
        if (aylen) {
            *(kmark++) = dbacod[(int) aylen][0];
            *(kmark++) = dbacod[(int) aylen][1];
        }
        if (cltype->ctclass == CSYUB) {
            egcmemmove(kmark, jword, (UWORD) jwlen);
            kmark += jwlen;
        }
        else {
            if (cltype->ctclass == CSYUH) {
                egcmemmove(kmark, jmark + 1, (UWORD) jylen - 2);
                kmark += jylen - 2;
            }
            else {
                if ((*jmark < (UBYTE)0x30) || ((UBYTE)0x39 < *jmark)) {
                    kmark += (UBYTE) cvtatoj((UBYTE) 1, jmark, jylen, kmark);
                } else {
                    found = FALSE;
                }
            }
        }
        kmark += (UBYTE) cvtatoj((UBYTE) 2, jmark + jylen,
                                  cllen - jylen - aylen, kmark);
        if (YTypeChk(clkana) == EXTRA_YOMI) {
            wordid->wirecno |= ALPHID;
        }
    }
    else {
        tankj = FALSE;
        if (*clkana < 0x30 || 0x39 < *clkana) {
            UBYTE   movel = 0;
            if (hkana) {
                if (clfrmq) {
                    wordid->wisegno = SETWIDSEG(clftbl[0].ctsegno);
                    wordid->wiseqno = SETWIDSEQ(clftbl[0].ctsegno, 63);
                    wordid->wirecno = (clftbl[0].ctmrecn)
                        ? clftbl[0].ctmrecn : clftbl[0].cturecn + MAXPLE;
                } 
                else {
                    wordid->wisegno = SETWIDSEG(0);
                    wordid->wiseqno = SETWIDSEQ(0, 63);
                    wordid->wirecno = 0;
                }
                kmark += (UBYTE) cvtatoj((UBYTE) 2, clkana, cllen, kmark);
                found = TRUE;
                hkana = FALSE;
            }
            else if (kkana) {
                wordid->wisegno = SETWIDSEG(0);
                wordid->wiseqno = SETWIDSEQ(0, 63);
                wordid->wirecno = 0;
                kmark += (UBYTE) cvtatoj((UBYTE) 1, clkana, cllen, kmark);
                found = TRUE;
                kkana = FALSE;
            }
            else if (khankaku || hhankaku) {
                wordid->wisegno = SETWIDSEG(0);
                wordid->wiseqno = SETWIDSEQ(0, 63);
                wordid->wirecno = PLBLANK;
                while (YTypeChk(clkana + movel) == EXTRA_YOMI
                        && movel < cllen)
                    movel++;
                egcmemmove(kmark, clkana, movel);
                kmark += movel;
                kmark += (UBYTE) cvtatoj((khankaku)? (UBYTE)2 : (UBYTE)1,
                                          (UBYTE*)(clkana + movel),
                                          (UWORD)(cllen - movel), kmark);
                found = TRUE;
                if (khankaku)
                    khankaku = FALSE;
                else
                    hhankaku = FALSE;
            }
            if (YTypeChk(clkana) == EXTRA_YOMI) {
                wordid->wirecno |= ALPHID;
            }
        }
    }
    *tankan = tankj;
    *clen = (UBYTE) (kmark - clause);
    return ((found) ? FALSE : TRUE);
}
/*PAGE*/

/********************************************************************/
/* newjtree                                                         */
/*                                                                  */
/* function                                                         */
/*   "newjtree" create root node table of clause analysis tree.     */
/* return                                                           */
/*   pointer to root node table                                     */
/********************************************************************/
NONREC
PTNODTBL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

newjtree(stpt)
    UBYTE           stpt;              /* start point of jiritsu-go */
{
/*--- static constant ---*/
    static UBYTE    hbittbl[9][4] =
    {
        {0x00, 0x01, 0x00, 0x00},      /* CHNON */
        {0x40, 0x00, 0x00, 0x00},      /* CHNORMAL */
        {0x00, 0x01, 0x00, 0x00},      /* CHKINSOK */
        {0x00, 0x10, 0x00, 0x00},      /* CHNUM */
        {0x40, 0x00, 0x00, 0x00},      /* CHALPHA */
        {0x08, 0x00, 0x00, 0x00},      /* CHSUB */
        {0x00, 0x01, 0x00, 0x00},      /* CHSEP */
        {0x00, 0x01, 0x00, 0x00},      /* CHJIS */
        {0x40, 0x00, 0x00, 0x00},      /* CHEOT */
    };

    static UBYTE    smask[3][4] =
    {
        {0x00, 0x00, 0x63, 0xff},
        {0x40, 0x00, 0x28, 0x00},
        {0x01, 0x10, 0x00, 0x00}
    };

/*--- automatic variables ---*/
    PTNODTBL        jtree;
    register UBYTE *yp;
    UBYTE          *hp;
    register UBYTE  fchar;
    UBYTE           stlen;
    UBYTE          *clyend;
    register WORD   i;

/*--- begin of newjtree ---*/
    wdyptr = yp = ystart + stpt;
    stlen = (UBYTE) (yend - yp);
    wdhead = 0;

    if ((jtree = newtreept(sizeof(NODETBL))) != (PTNODTBL)0){
        jtree->nbrnode = 0;
        fchar = *(chrtbl + (*yp));
        if (fchar != CHKINSOK) {
            if (fchar == CHNORMAL) {
                wdclas = CSYUB;
                if (!(wdwrdq = findpwrd(yp, stlen, wdwtbl, (UBYTE) 0))
                        || wdwrdq == 0xFF)
                    wdwrdq = 0;
            }
            else {
                wdwrdq = 1;
                hp = *(hbittbl + fchar);
                if (fchar == CHEOT) {
                    wdclas = CSYUH;
                    yp++;
                    if (*yp == 0x83) {
                        hp = hbittbl[CHNORMAL];
                    }
                    else {
                        hp = hbittbl[(int) (*(chrtbl + (*yp)))];
                    }
                    while ((*yp != CEOT) && (yp < yend))
                        yp++;
                    yp++;
                }
                else if (fchar == CHSUB) {
                    wdclas = CSYUF;
                    if (*yp == 0x22) {
                        if (dqtff ^= 0xff) {
                            hp = hbittbl[CHSEP];
                            wdclas = CSYUE;
                        }
                    }
                    if (*yp == 0x27) {
                        if (sqtff ^= 0xff) {
                            hp = hbittbl[CHSEP];
                            wdclas = CSYUE;
                        }
                    }
                    yp++;
                }
                else if (fchar == CHJIS) {
                    wdclas = CSYUJ;
                    clyend =
                        (yend <= (wdyptr + MAXCL)) ? yend : wdyptr + MAXCL;
                    while ((*(chrtbl + (*yp)) == CHJIS) && (yp < clyend))
                        yp += 2;
                }
                else {
                    clyend =
                        (yend <= (wdyptr + MAXCL))? yend:(wdyptr + MAXCL);
                    while ((*(chrtbl + (*yp)) == fchar) && (yp < clyend))
                        yp++;
                    if (fchar == CHSEP) {
                        wdclas = CSYUE;
                    }
                    else {
                        wdclas = CSYUF;
                    }
                }
                wdwtbl[0].jrwordl = (UBYTE) (yp - wdyptr);
                egcmemmove(wdwtbl[0].jrmhbit, hp, 4);
                memset4(wdwtbl[0].jruhbit, (UBYTE) 0x00);
            }
            memset4(wdmask, (UBYTE) 0xff);
            makenode(jtree);
            yp = wdyptr;
            if ((stlen > 1) && (*yp == 0xb5)) {
                wdhead = 1;
            }
            else if ((stlen > 2) && (*yp == 0xba) && (*(yp + 1) == 0xde)) {
                wdhead = 2;
            }
            else if ((stlen > 3) && (*yp == 0xc0)
                     && (*(yp + 1) == 0xde) && (*(yp + 2) == 0xb2)) {
                wdhead = 3;
            }
            if (wdhead) {
                egcmemmove(wdmask, *(smask + wdhead - 1), 4);
                wdyptr = (yp += wdhead);
                stlen -= wdhead;
                fchar = *(chrtbl + (*yp));
                if (fchar == CHNORMAL) {
                    wdclas = CSYUB;
                    if (!(wdwrdq = findpwrd(yp, stlen, wdwtbl, (UBYTE) 0))
                            || wdwrdq == 0xFF)
                        wdwrdq = 0;
                }
                else {
                    wdwrdq = 0;
                }
                makenode(jtree);
            }
        }
        if (!jtree->nbrnode && stlen != 1) {
            UBYTE   c = *yp;
            UWORD   yType = YTypeChk(yp);
            if (yType == EXTRA_YOMI) {
                i = 0;
                while ((YTypeChk(yp) == EXTRA_YOMI)
                    && stlen > 0 
                    && i < 16
                    && ((0x30 <= c && c <= 0x39)?
                    (0x30 <= *yp && *yp <= 0x39):(*yp < 0x30 || 0x39 < *yp))){
                    yp++; stlen--; i++;
                }
                wdwrdq = 1;
                wdclas = CSYUF;
                hp = hbittbl[CHNUM];
                wdwtbl[0].jrwordl = (UBYTE)(yp - wdyptr);
                egcmemmove(wdwtbl[0].jrmhbit, hp, 4);
                memset4(wdwtbl[0].jruhbit, (UBYTE) 0x00);
                memset4(wdmask, (UBYTE) 0xff);
                makenode(jtree);
            }
        }
    }
    if (! jtree->nbrnode) {
        jtree = (PTNODTBL)0;
    }
    return (jtree);
}
/*PAGE*/
/**********************************************************************/
/* makeftree                                                          */
/*                                                                    */
/* function                                                           */
/*   "makeftree" create fuzoku-go,setsubi-go and josuu-shi node table */
/* return                                                             */
/*   none                                                             */
/**********************************************************************/
RECURSIVE
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makeftree(tree, trp, level)
    PTNODTBL        tree;              /* parent node table */
    UBYTE           trp;               /* start point of parent word */
    UBYTE           level;             /* depth of recursion */
{
/*--- static variables ---*/
    static UBYTE    ha[4] = {0x00, 0x20, 0x00, 0x00};
    static UBYTE    hb[4] = {0x01, 0x20, 0x00, 0x00};

/*--- automatic variables ---*/
    FWORDREC        fwdtbl[MAXFWORD];
    register PTNODTBL ftree;
    register PTNODE node;
    PTNODE          nd;
    PTFWORD         frp;
    REG             i;
    UBYTE           tflag;
    UBYTE           nbrfwrd;
    UBYTE           stlen;
    UBYTE           stpt;
    UBYTE           fpoint;
    UBYTE           nodeno;

/*--- begin of makeftree ---*/
    node = tree->nodes;
    ftree = 0;
    fpoint = 0;
    nbrfwrd = 0;
    for (nodeno = tree->nbrnode; nodeno--;) {
        node->ndchain = 0;
        wdyptr = ystart
            + (stpt = trp + node->ndhdlen
               + node->ndlength + node->ndgobil);
        stlen = (UBYTE) (yend - wdyptr);
        if ((stlen)
            && (node->ndrow != 0)   /* setsuzoku-shi */
            &&(node->ndrow != 160)  /* fukushi       */
            &&(node->ndrow != 179)  /* settou        */
            &&(ftree = newtreept(sizeof(FNODETBL)))) {
            ftree->nbrnode = 0;
            if ((level == 1) && ((node->ndrow == 178)
                                 || (node->ndrow == 182)) && (currow != 255)) {
                wdwrdq = findpwrd(wdyptr, stlen, wdwtbl, (UBYTE) 0);
                if (wdwrdq && wdwrdq != 0xFF) {
                    egcmemmove(wdmask, (node->ndrow == 182) ? ha : hb, 4);
                    wdclas = CSYUB;
                    wdhead = 0;
                    makenode(ftree);
                }
            }
            if ((level <= 2) && (node->ndrow >= 170) && (currow != 255)) {
                if (wdwrdq = findswrd(wdyptr, stlen, node->ndrow, wdwtbl)) {
                    memset4(wdmask, (UBYTE) 0xff);
                    wdclas = CSYUC;
                    wdhead = 0;
                    makenode(ftree);
                }
            }
            if (fpoint != stpt) {
                nbrfwrd = findfwrd(wdyptr, stlen, fwdtbl);
                fpoint = stpt;
            }
            if (nbrfwrd) {
                nd = ftree->nodes + ftree->nbrnode;
                frp = fwdtbl;
                for (i = nbrfwrd; i--;) {
                    if ((clconnect(node->ndrow, frp->frcol))
                        && (tflag = clend(frp->frrow, wdyptr + frp->frwordl))
                        && (ftree->nbrnode < MAXFNODE)) {
                        nd->ndclass = CSYUG;
                        nd->ndhdlen = 0;
                        nd->ndlength = frp->frwordl;
                        nd->ndgobil = 0;
                        nd->ndrow = frp->frrow;
                        nd->ndhinshi = 32;
                        nd->ndterm = tflag - (UBYTE)1;
                        nd->ndchain = 0;
                        nd++;
                        (ftree->nbrnode)++;
                    }
                    frp++;
                }
            }
            if ((ftree->nbrnode) && (level < MAXLEVEL)) {
                node->ndchain = (UBYTE *) ftree;
                makeftree(ftree, stpt, (UBYTE)(level + 1));
            }
        }
        node++;
    }
}
/*PAGE*/
/**********************************************************************/
/* wakachi                                                            */
/*                                                                    */
/* function                                                           */
/*   "wakachi" is to be called in case no jiritsu-go found. "wakachi" */
/*   will locate the end of kana string and make dummy word node.     */
/* return                                                             */
/*   pointer to the node table created.                               */
/**********************************************************************/
NONREC
PTNODTBL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

wakachi()
{
/*--- automatic variables ---*/
    PTNODTBL        wtree;
    register PTNODE node;
    register UBYTE *p;
    UBYTE          *clyend;
    UBYTE           rq;
    PLREC           tmptbl[MAXWDTBL];
    UBYTE           ctype;
    UBYTE           c;

/*--- begin of wakachi ---*/
    wtree = newtreept(sizeof(WNODETBL));
    wtree->nbrnode = 1;
    node = wtree->nodes;
    p = ystart;
    if (*p == 0xA6) {
        p++;
    }
    else {
        if (yend <= (ystart + MAXCL)) {
            clyend = yend;
        }
        else {
            clyend = ystart + MAXCL;
            if ((*clyend == 0xDE) || (*clyend == 0xDF)) {
                clyend--;
            }
        }
        ctype = *(chrtbl + (*p));
        while((p < clyend)
            && ((c = *(chrtbl + *p)) != CHNORMAL)
            && (c != CHEOT)
            && (c != CHSEP)
            && ((!(rq = findpwrd(p, (UBYTE)(clyend - p), tmptbl, (UBYTE)0)))
            && rq != 0xFF)) {
            p++;
        }
        if (p == ystart) {
            p++;
            while ((p < clyend)
                && ((c = *(chrtbl + *p)) != CHNORMAL)
                && (c != CHEOT)
                && (c != CHSEP)) {
                p++;
            }
        }
    }
    node->nddicid = 0;
/*@@@ Next 1 line modefied at 01/09/1992 H.Yanata,
      as clear wakachi_go                       @@@*/
/*  node->ndclass = CSYUF; */
    node->ndclass = CSYUW;
/*@@@                                           @@@*/
    node->ndhdlen = 0;
    node->ndlength = (UBYTE) (p - ystart);
    node->ndgobil = 0;
    node->ndhinshi = HCFUTUU;
    node->ndrow = 172;
    node->ndterm = 3;
    node->ndsegno = dbsegn;
    node->ndchain = 0;
    return (wtree);
}
/*PAGE*/
/********************************************************************/
/* makenode                                                         */
/*                                                                  */
/* function                                                         */
/*   "makenode" makes word nodes from jiritsu-go and setsubi-go     */
/*   search result stored in "wdwtbl".                              */
/* return                                                           */
/*   none                                                           */
/********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makenode(tree)
    PTNODTBL        tree;              /* pointer to node table */
{
/*--- automatic variables ---*/
    UBYTE           clrtbl[MAXGWORD];
    UBYTE           lentbl[MAXGWORD];
    register PPR    wrdp;
    REG             hi;
    REG             i;
    REG             nbrfound;
    UBYTE           mhbits, uhbits;
    UBYTE          *mhbit, *uhbit;
    register UBYTE  hcode;
    UBYTE           stlen;
    UBYTE           row;

/*--- begin of makenode ---*/
    npt = tree->nodes + (nct = tree->nbrnode);
    node.ndclass = wdclas;
    node.ndhdlen = wdhead;
    node.ndchain = 0;
    stlen = (UBYTE) (yend - wdyptr);
    wrdp = wdwtbl;
    while (wdwrdq--) {
        node.ndlength = wrdp->jrwordl;
        node.ndsegno = wrdp->jrsegno;
        node.nddicid = wrdp->jrdicid;
        mhbit = wrdp->jrmhbit;
        uhbit = wrdp->jruhbit;
        for (hi = 0; hi < 4; hi++) {
            mhbits = *mhbit++ & *(wdmask + hi);
            uhbits = *uhbit++ & *(wdmask + hi);
            hcode = (UBYTE) (hi << 3);
            for (; mhbits || uhbits; hcode++, mhbits <<= 1, uhbits <<= 1) {
                if ((mhbits & 0x80) || (uhbits & 0x80)) {
                    node.ndmrecn = wrdp->jrmrecn;
                    node.ndurecn = wrdp->jrurecn;
                    node.ndhinshi = hcode;
                    if (hcode == HCKAHEN) {
                        nbrfound = findkwrd(wdyptr, wrdp->jrwordl, clrtbl);
                        for (i = 0; i < nbrfound; i++) {
                            node.ndrow = *(clrtbl + i);
                            node.ndgobil = 0;
                            setnode();
                        }
                    }
                    else if (hcode >= HCKEIYOU) {
                        if (wrdp->jrwordl == 1 && stlen == 1
                            && (yend - ystart == 1)) {
                            node.ndrow = hcode + (UBYTE)171;
                            node.ndgobil = 0;
                            setnode();
                        }
                        else {
                            nbrfound = findgwrd(hcode, wdyptr + wrdp->jrwordl,
                                (UBYTE)(stlen - wrdp->jrwordl), clrtbl,lentbl);
                            for (i = 0; i < nbrfound; i++) {
                                row = 0;
                                switch (wdhead) {
                                case 0:
                                    row = *(clrtbl + i);
                                    break;
                                case 1:
                                    if ((hcode == HCKEIYOU)
                                        || (hcode == HCKEIDOU)) {
                                        row = *(clrtbl + i);
                                    }
                                    else if (*(clrtbl + i) == 171) {
                                        row = 155;
                                    }
                                    break;
                                case 2:
                                    if (hcode == HCKEIDOU) {
                                        row = *(clrtbl + i);
                                    }
                                    else if ((wrdp->jrwordl >= 3)
                                             && (*(clrtbl + i) == 129)) {
                                        row = 170;
                                        break;
                                    }
                                }
                                if (row) {
                                    node.ndrow = row;
                                    node.ndgobil = *(lentbl + i);
                                    setnode();
                                }
                            }
                        }
                    }
                    else {
                        switch (hcode) {
                        case 0:
                            continue;
                        case HCBUNSETSU:
                            row = 0;
                            break;
                        case HCTANKAN:
                            row = 0;
                            break;
                        case HCSETUZOKU:
                            row = 0;
                            break;
                        case HCRENTAI:
                            row = 159;
                            break;
                        case HCFUKU:
                            row = 160;
                            break;
                        default:
                            row = hcode + (UBYTE)171;
                        }
                        if (wdhead == 2) {
                            row = (UBYTE)175;
                        }
                        node.ndrow = row;
                        node.ndgobil = 0;
                        setnode();
                    }
                }
            }
        }
        if ((currow == 255) && (wdhead == 0) && (stlen == wrdp->jrwordl)) {
            node.ndhinshi = 0;
            node.ndrow = 0;
            node.ndgobil = 0;
            node.ndmrecn = wrdp->jrmrecn;
            node.ndurecn = 0;
            setnode();
        }
        wrdp++;
    }
    tree->nbrnode = nct;
}
/*PAGE*/
/*-------------------------------------------------------------------*/
/*    setnode                                                        */
/*-------------------------------------------------------------------*/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

setnode()
{
/*--- automatic variables ---*/
    register UBYTE  tflag;

/*--- begin of setnode ---*/
    if ((nct < MAXJNODE)
        && (tflag = clend(node.ndrow,
                          wdyptr + node.ndlength + node.ndgobil))) {
        node.ndterm = tflag - (UBYTE)1;
        egcmemmove((UBYTE *) npt, (UBYTE *) & node, sizeof(NODEREC));
        npt++;
        nct++;
    }
}
/*PAGE*/
/**********************************************************************/
/* makeptbl                                                           */
/*                                                                    */
/* function                                                           */
/*   "makeptbl" will parse 1st clause anlyze tree and create clause   */
/*   type records and store them in "clstbl".                         */
/* return                                                             */
/*   TRUE : clause create  FALSE : not clause                         */
/**********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makeptbl(jtree)
    PTNODTBL        jtree;             /* pointer to 1st clause analysis tree */
{
/*--- automatic variables ---*/
    register PTNODE jnode;
    register UBYTE  jnodeno;
    UBYTE           jlength;

/*--- begin of makeptbl ---*/
    clausq = 0;
    xclausq = 0;
    jnode = jtree->nodes;
    for (jnodeno = 0; jnodeno < jtree->nbrnode; jnodeno++) {
        xflag = (nodecnt(currow, jnode) || (jsclen)) ? FALSE : TRUE;
        jwnd = jnode;
        wpath[0] = jnodeno;
        setuji = 0;
        jlength = jnode->ndhdlen + jnode->ndlength + jnode->ndgobil;
        if (jnode->ndchain) {
            makefclause((PTNODTBL) jnode->ndchain, (UBYTE) 0, jlength);
        }
        setuji = 0;
        if (jnode->ndterm) {
            setclause(jnode, (UBYTE) 0, jlength);
        }
        jnode++;
    }
    return ((clausq > xclausq) ? TRUE : FALSE);
}
/*PAGE*/
/*-------------------------------------------------------------------*/
/*  makefclause                                                      */
/*-------------------------------------------------------------------*/
RECURSIVE
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

makefclause(ftree, level, phlength)
    PTNODTBL        ftree;
    UBYTE           level;
    UBYTE           phlength;
{
/*--- automatic variables ---*/
    register PTNODE fnode;
    register UBYTE  fnodeno;
    UBYTE           cllen;

/*--- begin of makefclause ---*/
    level++;
    fnode = ftree->nodes;
    for (fnodeno = 0; fnodeno < ftree->nbrnode; fnodeno++) {
        *(wpath + level) = fnodeno;
        if (level == 1) {
            if (fnode->ndclass == CSYUC) {
                setuji = fnode->ndsegno + 1;
            }
            else if (fnode->ndclass == CSYUB) {
                setuji = 1;
            }
            else {
                setuji = 0;
            }
        }
        cllen = phlength + fnode->ndlength + fnode->ndgobil;
        if (fnode->ndchain) {
            makefclause((PTNODTBL) fnode->ndchain, level, cllen);
        }
        if (fnode->ndterm) {
            setclause(fnode, level, cllen);
        }
        fnode++;
    }
}
/*PAGE*/
/*-------------------------------------------------------------------*/
/*  setclause                                                        */
/*-------------------------------------------------------------------*/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

setclause(node, level, length)
    PTNODE          node;
    UBYTE           level;
    UBYTE           length;
{
/*--- automatic variavles ---*/
    register PTCLAUS clause;

/*--- begin of setclause ---*/
    clause = clstbl + clausq;
    while (clause > clstbl) {
        if ((clause - 1)->clendpt < length) {
            egcmemmove((UBYTE *)clause, (UBYTE*)(clause - 1),
                        sizeof(CLAUSREC));
            clause--;
        }
        else {
            break;
        }
    }
    if (xflag) {
        clause->clstat = STATEX;
    }
    else if (node->ndterm == 3) {
        clause->clstat = STATET;
    }
    else if (*(rowtbl + node->ndrow) == RTEND) {
        clause->clstat = STATES;
    }
    else {
        clause->clstat = STATEP;
    }
    clause->clprtylen = length;
    clause->clprtygrm = 0;
    clause->clprtysel = 0;
    clause->clendpt = length;
    clause->clsetsuji = setuji;
    clause->clrow = node->ndrow;
    clause->cllevel = level;
    clause->cljwnd = jwnd;
    clause->clchain = 0;
    egcmemmove(clause->clpath, wpath, (UWORD) level + 1);

    if (clausq < MAXCLAUS) {
        clausq++;
        if (xflag)
            xclausq++;
    }
}
/*PAGE*/
/**********************************************************************/
/* priority                                                           */
/*                                                                    */
/* function                                                           */
/*  "priority" will estimate the priority of a clause by calculating  */
/*  the max two clause length and considering grammertical factor.    */
/* return                                                             */
/*   none                                                             */
/**********************************************************************/
NONREC
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

priority(cl)
    PTCLAUS         cl;                /* pointer to clause record */
{
/*--- automatic variavles --- */
    register PTNODE node;
    register PTCLAUS clause;
    UBYTE          *p;
    register UBYTE  grprty;
    UBYTE           nodeno;
    UBYTE           sjwlen;
    UBYTE           maxsclen;
    UBYTE           maxscgrm;
    UBYTE           maxsjlen;
    UBYTE           d;

/*--- begin of priority ---*/
    node = (clause = cl)->cljwnd;
    if (currow >= 171) {
        if (node->ndhinshi >= HCFUKU) {
            clause->clprtygrm = 1;
        }
        else {
            d = distbl[(int) (currow - 171)][(int) (node->ndhinshi - 1)];
            if (d >= 15) {
                clause->clprtygrm = 4;
            }
            else if (d >= 13) {
                clause->clprtygrm = 3;
            }
            else {
                clause->clprtygrm = 1;
            }
        }
    }
    else {
        clause->clprtygrm = 3;
    }
    if (((node->ndhinshi == HCJINMEI)
         || (node->ndhinshi == HCCHIMEI)
         || (node->ndhinshi == HCMEI))
        && ((currow)
            || (clause->clstat != STATET))) {
        clause->clprtygrm--;
    }
    if (clause->clstat == STATET) {
        if (((currow == 12)
             || (currow == 17)
             || (currow == 104)
             || (currow == 106)
             || (currow == 108)
             || (currow == 160))
            && (clause->clrow >= 171)) {
            clause->clprtygrm = 0;
        }
    }
    else if (clause->clstat == STATES) {
        clause->clprtygrm += 13;
    }
    else if (clause->clchain) {
        maxsclen = 1;
        maxscgrm = 0;
        maxsjlen = 0;
        if (node->ndhinshi == HCKEIDOU) {
            clause->clprtygrm -= 1;
        }
        node = (clause->clchain)->nodes;
        for (nodeno = (clause->clchain)->nbrnode; nodeno--;) {
            if (nodecnt(clause->clrow, node)) {
                sjwlen = node->ndhdlen + node->ndlength + node->ndgobil;
                if ((node->ndterm)
                    && ((*(rowtbl + node->ndrow) != RTEND)
                        || (node->ndterm == 3))
                    && (node->ndrow != 166)) {
                    sclen = sjwlen;
                }
                else {
                    sclen = 0;
                }
                if (node->ndchain) {
                    getplength((PTNODTBL) node->ndchain, sjwlen);
                }
                if (sclen >= maxsclen) {
                    if (node->ndhinshi == HCSETUZOKU) {
                        grprty = (UBYTE)15 - clause->cllevel;
                    }
                    else {
                        switch (*(rowtbl + clause->clrow)) {
                        case RTJTG:
                            if (node->ndhinshi < HCFUKU) {
                                grprty =
                                    distbl[(int) (clause->clrow - 171)]
                                    [(int) (node->ndhinshi - 1)]
                                    - clause->cllevel;
                            }
                            else {
                                grprty = (UBYTE)2;
                            }
                            break;
                        case RTJRY:
                            grprty =
                                (node->ndhinshi >= HC1DAN)?
                                (UBYTE)14 : (UBYTE)12;
                            break;
                        case RTFRY:
                            grprty = (UBYTE)12 - clause->cllevel;
                            break;
                        case RTHPR:
                            grprty = (UBYTE)15 - clause->cllevel;
                            break;
                        case RTRPR:
                            grprty = (UBYTE)2;
                            break;
                        default:
                            grprty = (UBYTE)14 - clause->cllevel;
                        }
                    }
                    if (node->ndhdlen) {
                        grprty--;
                    }
                    if (sclen > maxsclen) {
                        maxsclen = sclen;
                        maxscgrm = grprty;
                        maxsjlen = sjwlen;
                    }
                    else {
                        if (grprty > maxscgrm)
                            maxscgrm = grprty;
                        if (sjwlen > maxsjlen)
                            maxsjlen = sjwlen;
                    }
                }
            }
            node++;
        }
        clause->clprtylen += maxsclen;
        clause->clprtygrm += maxscgrm;
        p = ystart + clause->clendpt;
        while (maxsjlen--) {
            if ((*p >= 0xB1) && (*p <= 0xDD))
                clause->clprtysel++;
            p++;
        }
    }
}
/*PAGE*/
/*--------------------------------------------------------------------*/
/* getplength   (get 2nd clause length)                               */
/*--------------------------------------------------------------------*/
RECURSIVE
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

getplength(tree, blength)
    PTNODTBL        tree;
    UBYTE           blength;
{
/*--- automatic variables ---*/
    register PTNODE node;
    register REG    j;
    UBYTE           flength;

/*--- begin of getplength --- */
    node = tree->nodes;
    for (j = tree->nbrnode; j--;) {
        flength = blength + node->ndlength + node->ndgobil;
        if ((flength > sclen)
            && (node->ndterm)
            && ((*(rowtbl + node->ndrow) != RTEND)
                || (node->ndterm == 3))) {
            sclen = flength;
        }
        if (node->ndchain) {
            getplength((PTNODTBL) node->ndchain, flength);
        }
        node++;
    }
}
/*PAGE*/
/*********************************************************************/
/* selectclause                                                      */
/*                                                                   */
/* function                                                          */
/*   select the clasue type record which have the highest priority   */
/* return                                                            */
/*   pointer to the clause type record selected                      */
/*********************************************************************/
NONREC
PTCLAUS
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

selectclause()
{
/*--- automatic variables ---*/
    register PTNODE node;
    register PTCLAUS clause;
    PTCLAUS         sclaus = (PTCLAUS)0;
    register UBYTE  i;
    UBYTE           j;
    ULONG           maxprty = 0;
    ULONG           prty;
    UWORD           recno;
    UWORD           sltval;
    UBYTE           maxplen = 0;
    UBYTE           wrdl;
    UBYTE           row;
    PTNODTBL        clchain;
    BOOL            first;
    UBYTE           kanji[MAXJKANJI];
    UBYTE           kanjil;
    UBYTE           hbitout[4];
    UBYTE           ndhinshi;
    UBYTE          *sltptr;
    UWORD           sltseg;
#ifdef    PMSLT
    PTR_UDICT       pdarry = dicprty[0].pds;
#endif

/*--- begin of selectclause ---*/
    clause = clstbl;
    first = TRUE;
    for (i = 0; i < jsclausq; i++) {
        node = clause->cljwnd;
        if (i < clausq) {
            if (i == 0 && clause->clstat == STATEX) {
                sclaus = clause;
                break;
            }
            if (clause->clstat != STATEX) {
                if (first) {
                    first = FALSE;
                    sclaus = clause;
                }
                if ((clause->clprtylen > maxplen)
                    || ((clause->clprtylen == maxplen)
                        &&
                      ((sclaus->clchain) || (!clause->clchain) || (cmode)))) {
                    wrdl = ((node->ndhdlen) ? (UBYTE) 3 : (UBYTE) 0)
                        + node->ndlength + node->ndgobil;
                    prty = ((ULONG) (clause->clprtylen) << 11)
                        + ((ULONG) (clause->clprtygrm) << 6)
                        + ((ULONG) wrdl << 3)
                        + ((ULONG) clause->clprtysel);
                    if ((prty > maxprty) && ((!jsclen)
                                             || ((jsclen == clause->clendpt)
                        && (jsjlen == node->ndlength) && (!node->ndhdlen)))) {
                        sclaus = clause;
                        maxplen = clause->clprtylen;
                        maxprty = prty;
                    }
                }
            }
        }
        clause++;
    }

    row = sclaus->clrow;
    clchain = sclaus->clchain;
    node = sclaus->cljwnd;
    if ((sclaus->clsetsuji) && (!jsflag) && (!jsclen)) {
        jsclen = sclaus->clendpt
            - (node->ndhdlen + node->ndlength + node->ndgobil);
        jsjlen = (((PTNODTBL) (node->ndchain))->nodes
                  + (*(sclaus->clpath + 1)))->ndlength;
        sclaus->clendpt -= jsclen;
        sclaus->cllevel = 0;
        jsrow = sclaus->clrow;
        jsflag = 1;
        jshinshi = (sclaus->clsetsuji > 1) ? (UBYTE) HCSETSUBI :
            (((PTNODTBL) (node->ndchain))->nodes
             + (*(sclaus->clpath + 1)))->ndhinshi;
        jsmrecn = (((PTNODTBL) (node->ndchain))->nodes
                   + (*(sclaus->clpath + 1)))->ndmrecn;
    }

    for (j = 0; j < maxslt; j++) {
        clause = clstbl;
        for (i = 0; i < jsclausq; i++, clause++) {
            if ((clause->clendpt != sclaus->clendpt) || (clause->clsetsuji))
                continue;
            node = clause->cljwnd;
            if (node->ndclass != CSYUB) {
                sltseg = dbsegn;
            }
            else {
                sltseg = node->ndsegno;
            }
            if (node->nddicid != 0) {
                continue;
            }
#ifdef    PMSLT
            sltptr = GetSltp(pdarry, (sltseg - 1) * (maxslt * 2));
#else
            sltptr = dicprty[0].pds->hbuhdr.udsltp +
                ((UBYTE) (sltseg - 1) * (maxslt * 2));
#endif
            sltptr += j * 2;
            sltval = (UWORD) getint(sltptr);
            if (sltval == 0x0000)
                continue;
            recno = (node->ndmrecn) ? node->ndmrecn : MAXPLE + node->ndurecn;
            if ((sltval & 0x01FF) != recno)
                continue;
            if ((sclaus >= clause) &&
                (sclaus->clprtygrm >= clause->clprtygrm)) {
                sclaus = clause;
                sclaus->clrow = row;
                sclaus->clchain = clchain;
                goto lbreak;
            }
            else {
                UBYTE   dummy;
                readslt(0, node->ndsegno, node->ndmrecn, node->ndurecn);
                readdwrd(0, node->ndlength, ystart, &kanjil, kanji, hbitout,
                        &dummy);
                ndhinshi = node->ndhinshi;
                if (hbitout[ndhinshi >> 3] & (0x80 >> (ndhinshi & 0x07))) {
                    sclaus = clause;
                    sclaus->clrow = row;
                    sclaus->clchain = clchain;
                    goto lbreak;
                }
            }
        }
    }

lbreak:
    if (jsclen && !jsflag)
        sclaus->clrow = jsrow;
    return (sclaus);
}
/*PAGE*/
/********************************************************************/
/* cvtclause                                                        */
/*                                                                  */
/* function                                                         */
/*   create kanji string of clause from the given clause type record*/
/*   and clause analysis tree.                                      */
/* return                                                           */
/*   TRUE  -  kanji output buffer overflow                          */
/*   FALSE -  ok                                                    */
/********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

cvtclause(clause, jtree)
    PTCLAUS         clause;            /* pointer to clause record */
    PTNODTBL        jtree;             /* pointer to clause analysis tree */
{
/*--- automatic variables ---*/
    register PTNODE node;
    register PTNODE nd;
    PTNODTBL        tree;
    PTCLAUS         cl;
    UBYTE           mhbit[4], uhbit[4], hbitout[4];
    UBYTE          *hp, *yp, *kp, *kq, *mp, *p;
    UWORD           mrecn, urecn;
    register UBYTE  i;
    BOOL            overfl;
    UBYTE           kjl;
    UBYTE           level;
    UBYTE           prtygrm;
    UBYTE           yl;
    UBYTE           wc;
    UWORD           st = 0;
    UBYTE           hdlen = 0xFF;
    UBYTE           hinshi;
    BOOL            sltflag1, sltflag2;

/*--- begin of cvtclause ---*/
    hp = kstart;
    mp = ystart;
    wc = 0;
    tree = jtree;
    overfl = FALSE;
    for (level = 0; level <= clause->cllevel; level++) {
        node = tree->nodes + (*(clause->clpath + level));
        kq = kp = (yp = hp + 3)
            + (yl = node->ndhdlen + node->ndlength + node->ndgobil);
        if (kp + MAXJKANJI > kend) {
            overfl = TRUE;
            break;
        }
        egcmemmove(yp, mp, (UWORD) yl);
        if (level == 0) {
            nd = tree->nodes;
            for (i = 0; i < tree->nbrnode; i ++) {
                if ((nd->ndhdlen == 1) || (nd->ndhdlen == 2)) {
                    hdlen = nd->ndhdlen;
                    break;
                }
                nd++;
            }
            if (i == tree->nbrnode) {
                nd = tree->nodes;
                for (i = 0; i < tree->nbrnode; i ++) {
                    if ((nd->ndlength == hdlen) && (nd->ndhdlen == 0)
                        && (nd->ndhinshi == HCSETTOU)) {
                        readslt(nd->nddicid, nd->ndsegno,
                                nd->ndmrecn, nd->ndurecn);
                        while (hbitout[1] & 0x80 || st == 512) {
                            UBYTE   dummy;
                            st = readdwrd(nd->nddicid, 2, mp, 
                                &kjl, kp, hbitout, &dummy);
                        }
                        dbacod[(int) hdlen][0] = *kp;
                        dbacod[(int) hdlen][1] = *(kp + 1);
                        break;
                    }
                    nd++;
                }
            }
            if (node->ndhdlen) {
                *(kp++) = dbacod[(int) (node->ndhdlen)][0];
                *(kp++) = dbacod[(int) (node->ndhdlen)][1];
                mp += node->ndhdlen;
            }
        }
        if (node->ndclass == CSYUB) {
            memset4(mhbit, (UBYTE) 0x00);
            memset4(uhbit, (UBYTE) 0x00);
            if (mrecn = node->ndmrecn) {
                *(mhbit + ((node->ndhinshi) >> 3)) =
                    (UBYTE) ((0x80) >> ((node->ndhinshi) & 0x07));
            }
            if (urecn = node->ndurecn) {
                *(uhbit + ((node->ndhinshi) >> 3)) =
                    (UBYTE) ((0x80) >> ((node->ndhinshi) & 0x07));
            }
            sltflag1 = lookslt(node->nddicid, node->ndsegno, mrecn, urecn);
            sltflag2 = FALSE;
            if (level == 0) {
                UBYTE   dummy;
                readslt(node->nddicid, node->ndsegno,
                        node->ndmrecn, node->ndurecn);
                /*@@@
                readdwrd(node->nddicid, node->ndlength, mp, &kjl, kp, hbitout);
                @@@*/
                readdwrd(node->nddicid, node->ndlength, mp, &kjl, kp, hbitout,
                         &dummy);
                prtygrm = (clause->clprtygrm)? 
                    (UBYTE)(clause->clprtygrm - 1) : (UBYTE)0;
                if ((node->ndlength == clause->clendpt)
                    && (kjl == 2) && (sltflag1)) {
                    sltflag2 = TRUE;
                }
                else {
                    cl = clstbl;
                    for (i = 0; i < jsclausq; i++) {
                        if (clause->clendpt <= cl->clendpt) {
                            if ((clause->clendpt == cl->clendpt)
                                && (clause->clsetsuji == cl->clsetsuji)) {
                                nd = cl->cljwnd;
                                hinshi = nd->ndhinshi;
                                if ((node->ndlength == nd->ndlength)
                                    && (node->ndhdlen == nd->ndhdlen)) {
                                    if ((hbitout[hinshi >> 3] &
                                         (0x80 >> (hinshi & 0x07)))
                                        && (sltflag1)) {
                                        sltflag2 = TRUE;
                                    }
                                    if ((i < clausq)
                                        && (clause->clstat == cl->clstat)
                                        && (prtygrm <= cl->clprtygrm)
                                        && (clause->clstat != STATEX)) {
                                        if (mrecn) {
                                            *(mhbit + (hinshi >> 3)) |=
                                                (0x80) >> (hinshi & 0x07);
                                        }
                                        if (urecn) {
                                            *(uhbit + (hinshi >> 3)) |=
                                                (0x80) >> (hinshi & 0x07);
                                        }
                                    }
                                }
                            }
                            cl++;
                        }
                        else {
                            break;
                        }
                    }
                }
            }
            if (jsclen && !jsflag) {
                jsclen = 0;
                if (jshinshi == HCSETSUBI) {
                    node->ndclass = CSYUC;
                }
                if ((jsjlen == node->ndlength) && (!sltflag2)) {
                    if (jshinshi == HCSETSUBI) {
                        kp += readswrd(jsmrecn, kp);
                        goto setsubi;
                    }
                    memset4(mhbit, (UBYTE) 0x00);
                    mhbit[(int)(jshinshi >> 3)] |= (0x80 >> (jshinshi & 0x07));
                    memset4(uhbit, (UBYTE) 0x00);
                    uhbit[(int)(jshinshi >> 3)] |= (0x80 >> (jshinshi & 0x07));
                }
            }
            if (!sltflag2) {
                readslt(node->nddicid, node->ndsegno,
                        node->ndmrecn, node->ndurecn);
                while (1) {
                    UBYTE   dummy;
                    p = ((st = readdwrd(node->nddicid, node->ndlength, mp,
                        &kjl, kp, hbitout, &dummy)) < 256) ? mhbit : uhbit;
                    if (((*p & *hbitout)
                         || (*(p + 1) & *(hbitout + 1))
                         || (*(p + 2) & *(hbitout + 2))
                         || (*(p + 3) & *(hbitout + 3)))
                        || st == 512) {
                        break;
                    }
                }
            }
            kp += kjl;
        }
        else if (node->ndclass == CSYUC) {
            kp += readswrd(node->ndmrecn, kp);
        }
        else if ((node->ndclass == CSYUE)
                 || (node->ndclass == CSYUF)
                 || (node->ndclass == CSYUG)
/*@@@ Next 1 kine added at 01/09/1992 by H.Yanata,
      as enable conversion process              @@@*/
                || (node->ndclass == CSYUW)
/*@@@                                           @@@*/
                 ) {
            if (node->ndclass == CSYUF) {
                node->ndclass = CSYUB;
            }
            if (kp + (yl << 1) > kend) {
                overfl = TRUE;
                break;
            }
            else {
                kp += (UBYTE) cvtatoj((UBYTE) 2, mp, node->ndlength, kp);
            }
        }
setsubi:
        mp += node->ndlength;
        if (node->ndgobil) {
            kp += (UBYTE) cvtatoj((UBYTE) 2, mp, node->ndgobil, kp);
            mp += node->ndgobil;
        }
        *hp = node->ndclass;
        *(hp + 1) = yl;
        *(hp + 2) = (UBYTE) (kp - kq);
        hp = kp;
        wc++;
        tree = (PTNODTBL) node->ndchain;
    }                               /* end of for */

#ifdef  BHREAD
    prvkcnv = (UBYTE)(kp - kstart);
    prvycnv = (UBYTE)(mp - ystart);
    prvWc = wc;
#endif

    if (!overfl) {
        kstart = hp;
        wordct += wc;
    }

    return (overfl);
}
/*PAGE*/
/**********************************************************************/
/* nodecnt                                                            */
/*                                                                    */
/* function                                                           */
/*  "nodecnt" will check whether the node pointed by "node" can be    */
/*  connected to the clause which ends with connect matrix row number */
/*  specified by "row"                                                */
/* return                                                             */
/*   TRUE  -  yes.                                                    */
/*   FALSE -  no.                                                     */
/**********************************************************************/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

nodecnt(row, node)
    UBYTE           row;               /* connect matrix row number */
    PTNODE          node;              /* pointer to word node */
{
/*--- automatic variavles ---*/
    register REG    clausrow;
    register REG    nodhinshi;

/*--- begin of nodecnt ---*/
    clausrow = row;
    nodhinshi = node->ndhdlen ? HCSETTOU : node->ndhinshi;
    if (((clausrow == 179)
         && (nodhinshi > HCDAIMEI))
        || ((clausrow <= 170)
            && (nodhinshi == HCSETSUBI))
        || ((clausrow != 178)
            && (clausrow != 182)
            && (clausrow != 255)
            && (nodhinshi == HCJOSUU))) {
        return (FALSE);
    }
    else {
        return (TRUE);
    }
}
/*PAGE*/
/**********************************************************************/
/* clend                                                              */
/*                                                                    */
/* function                                                           */
/*   "clend" check whether the given word could be at end of clause   */
/* return                                                             */
/*   0  -  word has no sence                                          */
/*   1  -  word can not be at end of clause                           */
/*   2  -  word can be at end of clause                               */
/*   4  -  word can be at end of clause and word is at end of sentence*/
/**********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

clend(row, wordend)
    UBYTE           row;               /* connect matrix row number */
    UBYTE          *wordend;           /* pointer to end of word */
{
/*--- automatic variavles ---*/
    register UBYTE  rval;

/*--- begin of clend ---*/
    rval = (terminate(row)) ? (UBYTE)1 : (UBYTE)0;
    if ((wordend < yend)
        && (*(chrtbl + (*wordend)) != CHSEP)) {
        if (*(chrtbl + (*wordend)) == CHKINSOK) {
            rval = 1;
        }
        else {
            rval += 1;
        }
    }
    else {
        rval <<= 2;
    }
    return (rval);
}
