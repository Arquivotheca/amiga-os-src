/************************************************************************/
/*                                                                      */
/*      EGCRCVT : EGConvert Romaji ConVerT                              */
/*                                                                      */
/*              Designed    by  T.Nosaka        06/02/1988              */
/*              Coded       by  T.Nosaka        06/02/1988              */
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
#include "egcdef.h"
#include "egckcnv.h"
#include "egcrtbl.h"
#include "egcproto.h"
#ifdef  UNIX
#include "egccrg.h"
#endif

#define  MAXROMA        3

/*PAGE*/
/********************************************************************/
/*   romaji : romaji convertion                                     */
/*      This routine is called by setacode.                         */
/*              Function value is                                   */
/*           0  no conversion                                       */
/*           1  all conversion                                      */
/*           2  partial conversion add beep                         */
/*           3  partial conversion                                  */
/********************************************************************/
NONREC
UBYTE romaji(cmode, src, srcs, srce, dst, dstl, totl)
    UBYTE           cmode;             /* 0:hankaku 1:katakana 2:hirakana */
    UBYTE          *src;               /* source string */
    UWORD           srcs;              /* source string length */
    UWORD           srce;              /* source string length */
    UBYTE          *dst;               /* destination string */
    UWORD          *dstl;              /* destination string length */
    UWORD          *totl;              /* total length */
{
    UBYTE          *p;
    UBYTE           cvtstr[MAXROMA];
    UBYTE           dummystr[4];
    UWORD           dummylen;
    UBYTE           st;
    UWORD           srcl;              /* source string length */
    UWORD           sdstl;
    UWORD           ncvtl;
    UWORD           i;
    UWORD           flag;

    p = src + srcs;
    srcl = srce - srcs;
    i = 0;
    *totl = *dstl = 0;
    flag = 0;

    while (srcl > 0) {
        if ((((*p) >= JV1FR) && ((*p) <= JV1TO))
            || (((*p) >= JV2FR) && ((*p) <= JV2TO))) {
            if (srcl == 1) {
                return (0);
            }
            else {
                dummylen = (UWORD) jistoasc(p, p + 2, dummystr);
                if (dummylen == 1) {
                    *(cvtstr + i) = *dummystr;
                    i++;
                }
                else {
                    flag = 1;
                    break;
                }
            }
            p += 2;
            srcl -= 2;
        }
        else {
            *(cvtstr + i) = *p;
            i++;
            p++;
            srcl--;
        }
        if (i >= 3) {
            st = romcvt(cmode, cvtstr, 0, i, dst + *dstl, &sdstl, totl, &ncvtl);
            *totl += *dstl;
            *dstl += sdstl;
            egcmemmove(cvtstr, cvtstr + i - ncvtl, ncvtl);
            i = ncvtl;
        }
    }                               /* end of loop */
    if (i != 0) {
        st = romcvt(cmode, cvtstr, 0, i, dst + *dstl, &sdstl, totl, &ncvtl);
        *totl += *dstl;
        *dstl += sdstl;
    }
    if (flag == 1) {
        egcmemmove(dst + *totl, p, 2);
        *totl += 2;
        *dstl = *totl;
        srcl -= 2;
        st = 1;
        p += 2;
        if (srcl > 0) {
            egcmemmove(dst + *totl, p, srcl);
            st = 2;
            *totl += srcl;
        }
    }
    if (*dstl == 0) {
        st = 0;
    }
    return (st);
}
/*PAGE*/
/*-----------------------------------------------------------*/
/*   romcvt : romaji convertion                              */
/*-----------------------------------------------------------*/
#define  CVTLMT    9                   /* convert MAX (3 * 3) */
#define  SBIT   0x01                   /* seach flag */
#define  YBIT   0x02                   /* (ry/sh/ts etc) flag */
#define  MBIT   0x04                   /* cmode flag */
#define  TBIT   0x08                   /* mode 0 (hankaku kana) flag */

NONREC
UBYTE romcvt(cmode, sbufptr, rmjstart, crsrend, rstr, cnvnbr, totalnbr, ncvtl)
    UBYTE           cmode;             /* 0:hankaku(kana) 1:zenkaku(kana)
                                        * 2:zenkaku(hira) */
    UBYTE          *sbufptr;           /* ->screen buffer */
    UWORD           rmjstart;          /* offset to romaji start point */
    UWORD           crsrend;           /* offset to cursor end */
    UBYTE          *rstr;              /* romaji string area(16 bytes) */
    UWORD          *cnvnbr;            /* conversion number (bytes) */
    UWORD          *totalnbr;          /* total number (bytes) */
    UWORD          *ncvtl;             /* no convertion number of cursor */
/* f/c = 0:no convertion 1:all convertion 2:partial convertion and beep */
/*                    3:partial convertion      */
{
/* local variable */
    UBYTE           charnbr;           /* ascii string length */
    UBYTE           charcode;          /* temporary ascii code area */
    UBYTE          *ascstr;            /* ascii string area */
    UBYTE           tempstr[CVTLMT];   /* temporary string area(9 bytes) */
    UBYTE           pval;              /* previous value */
    UBYTE           status;            /* function value */
    UBYTE           dstchr;            /* temporary area */
    UBYTE           rflag;             /* set of flag bit */
    UWORD           cnbr;              /* convert number */
    UWORD           i, j, k;           /* counter */
    UWORD           xx, x, y;          /* table counter */
    UWORD           n;                 /* jis code */

    *ncvtl = 0;
    *cnvnbr = *totalnbr = 0;
    j = k = 0;
    rflag = pval = 0;
    ascstr = sbufptr + rmjstart;
    charnbr = (UBYTE) (crsrend - rmjstart);
    for (i = 0; i < charnbr; i++) {
        dstchr = ascstr[i];
        if ((dstchr >= 0x61) && (dstchr <= 0x7A))
            dstchr -= 0x20;         /* cvt large moji */
        if (((pval == 0x4E) || ((pval == 0x4D) && (dstchr != 0x4D)))
            && (dstchr != 0x59))
            tempstr[j - 1] = 0xDD;  /* case of 0xDD */
        else {
            if ((dstchr >= 0x41) && (dstchr <= 0x5A) && (pval == dstchr))
                tempstr[j - 1] = 0xAF;  /* case of 0xAF */
        }
        if ((dstchr < 0x41) || (dstchr > 0x5A)) {       /* case of not A - Z */
            charcode = ascstr[i];
            if (charcode == 0x5B)
                charcode = 0xA2;    /* kagikakko */
            else {
                if (charcode == 0x5D)
                    charcode = 0xA3;/* kagikakko */
            }
            if (charcode != 0)
                tempstr[j++] = charcode;
            pval = 0;
        }
        else {
            x = xx = 0;
            rflag &= ~(MBIT | TBIT | SBIT);     /* MBIT & TBIT & SBIT off */
            if ((i + 1) != charnbr)
                rflag &= ~(YBIT);   /* YBIT off */
            while ((!(rflag & SBIT)) && (!(rflag & TBIT))) {
                while ((cvtchr[x][0] != 0xFF)
                       || (cvtchr[x][1] != dstchr)
                       || (!(rflag & MBIT))) {  /* first moji search */
                    if (cvtchr[x][0] == 0xFF) {
                        xx++;
                        if (cvtchr[x][1] == cmode)
                            rflag |= MBIT;      /* MBIT on */
                        if (cvtchr[x][1] == 0)
                            rflag |= TBIT;      /* TBIT on */
                    }
                    x++;
                }
                x++;
                xx++;
                while ((cvtchr[x][0] != 0xFF)
                       && (!(rflag & SBIT))) {  /* second moji and third moji
                                                 * search */
                    y = 0;
                    rflag |= SBIT;  /* SBIT on */
                    while ((y < 2) && (rflag & SBIT) && (cvtchr[x][y] != 0)) {
                        charcode = ascstr[i + y + 1];
                        if ((charcode >= 0x61) && (charcode <= 0x7A))
                            charcode -= 0x20;   /* cvt large moji */
                        if ((charcode != cvtchr[x][y])
                            || ((i + y + 1) == charnbr))
                            rflag &= ~(SBIT);   /* SBIT off */
                        else {
                            if (y == 0)
                                rflag |= YBIT;  /* YBIT on */
                        }
                        y++;
                    }
                    x++;
                }
            }
            if (rflag & SBIT) {     /* case of cvt */
                rflag &= ~(YBIT);   /* YBIT off */
                i += y;
                x -= xx;
                x--;
                y = 0;
                while ((y < 3) && (cvtroma[x][y] != 0)) {       /* cvt move */
                    if ((cvtroma[x][y] >= 0x80) && (cvtroma[x][y] <= 0x8F)) {
                        tempstr[j++] = 0xFF;
                    }
                    tempstr[j++] = cvtroma[x][y];
                    y++;
                }
                pval = 0;
            }
            else {                  /* case of not cvt */
                tempstr[j++] = ascstr[i];
                pval = dstchr;
            }
        }
    }                               /* end of for loop */
    if (rflag & YBIT) {
        j--;
        (*ncvtl)++;
    }
    if (pval != 0) {
        j--;
        (*ncvtl)++;
    }
    cnbr = j;                       /* set conversion number */
    status = 1;                     /* default all conversion */

    if (pval != 0) {
        if (rflag & YBIT)
            j++;
        j++;
        status = 2;                 /* partial conversion */
    }
    if (cmode == 0) {               /* output ascii code */
        for (i = 0; i < j; i++)
            *rstr++ = tempstr[i];
        *cnvnbr = cnbr;
        *totalnbr = j;
        if ((cnbr > 0) && (status == 2) &&
            ((tempstr[i - 2] == 0xAF) || (tempstr[i - 2] == 0xDD)))
            status = 3;
    }
    else {                          /* output zenkaku(hira/kana) code */
        for (i = 0; i < cnbr; i++) {
            if ((tempstr[i] == 0xFF)
                && (tempstr[i + 1] > 0x7F) && (tempstr[i + 1] < 0x89)) {
                /* xwa    wi    we   xka   xke  */
                /* 0x80  0x82  0x83  0x87  0x88 */
                *cnvnbr += cvtatoj(cmode, &tempstr[k], (UBYTE) (i - k),
                                   rstr + (*cnvnbr));
                n = tempstr[++i] + 0x830E;
                if (cmode == 2)
                    n -= 0xA2;
                *(rstr + (*cnvnbr)++) = (UBYTE) (n >> 8);
                *(rstr + (*cnvnbr)++) = (UBYTE) n;      /* under bit */
                k = i + 1;
            }
        }
        *cnvnbr += cvtatoj(cmode, &tempstr[k], (UBYTE) (i - k),
                           rstr + (*cnvnbr));
        *totalnbr = *cnvnbr;
        if (status == 2) {          /* partial conversion */
            *totalnbr = cvtatoj(cmode, &tempstr[cnbr], (UBYTE) (j - cnbr),
                                rstr + (*cnvnbr));
            *totalnbr += *cnvnbr;
            if (cnbr > 0) {
                if (((*rstr == 0x82) &&
                /* zenkaku TSU(hira)       zenkaku N(hira) */
                     (*(rstr + 1) == 0xC1 || *(rstr + 1) == 0xF1))
                    || ((*rstr == 0x83) &&
                /* zenkaku TSU(kana)       zenkaku N(kana) */
                        (*(rstr + 1) == 0x62 || *(rstr + 1) == 0x93)))
                    status = 3;
            }
        }
    }                               /* end of zenkaku */
    return (status);
}
