/************************************************************************/
/*                                                                      */
/*      EGCLDMLS : EGConvert low-level data manupulation language       */
/*                  < code and romaji convert >                         */
/*                                                                      */
/*              Designed    by  T.Nosaka        11/07/1988              */
/*              Coded       by  T.Nosaka        11/07/1988              */
/*                                                                      */
/*      (C) Copyright 1988-1991 ERGOSOFT Corp.                          */
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
/*      1  egccodeinit(code)                                            */
/*      2  egcrmcvt(src,srcl,dst,dtl,totl,cmode)                        */
/*      3  egcjacvt(src,srcl,dst,dstl)                                  */
/*      4  egcajcvt(src,srcl,dst,dstl,cmode)                            */
/*      5  egcsjcvt(src,srcl,dst,dstl)                                  */
/*      6  egcjscvt(src,srcl,dst,dstl)                                  */
/*      7  egcsecvt(src,srcl,dst,dstl)                                  */
/*      8  egcescvt(src,srcl,dst,dstl)                                  */
/*      9  egciecvt(src,srcl,dst,dstl)                                  */
/*     10  egceicvt(src,srcl,dst,dstl)                                  */
/*                                                                      */
/*  <private>                                                           */
/*      1  nullcvt()                                                    */
/*                                                                      */
/************************************************************************/
#ifdef    UNIX
#include <stdio.h>
#endif
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egcldml.h"
#include "egcproto.h"

#ifdef UNIX
WORD            egccode = CVT_SJIS;
PTRWFUNC        inputcvt;
PTRWFUNC        outputcvt;
UBYTE           inputbuf[BUFSIZ * 3];
UBYTE           outputbuf[BUFSIZ * 3];
UWORD           inputlen;
UWORD           outputlen;

/*-----------------------------------------------------------*/
/*   egccodeinit : convert target code init                  */
/*-----------------------------------------------------------*/
/*
** convert target code define
*/

NONREC
WORD egccodeinit(code)
    WORD            code;              /* target code */
{

    switch (code) {
    case CVT_SJIS:
        egccode = code;
    case CVT_JIS:
        egccode = code;
    case CVT_EUC:
        egccode = code;
    default:
        return ((WORD) ERCIN01);
        break;
    }
    return ((WORD) NORMAL);
}

/*-----------------------------------------------------------*/
/*   ecodeset : convert target code set                      */
/*-----------------------------------------------------------*/
NONREC
WORD ecodeset()
{
    switch (egccode) {
    case CVT_SJIS:
        inputcvt = nullcvt;
        outputcvt = nullcvt;
        break;
    case CVT_JIS:
        inputcvt = egcjscvt;
        outputcvt = egcsjcvt;
        break;
    case CVT_EUC:
        inputcvt = egcescvt;
        outputcvt = egcsecvt;
        break;
    default:
        inputcvt = nullcvt;
        outputcvt = nullcvt;
        break;
    }
    return ((WORD) NORMAL);
}
#endif

#ifndef DOS_EGBRIDGE
/*-----------------------------------------------------------*/
/*   egcrmcvt : romaji convertion                            */
/*-----------------------------------------------------------*/
NONREC
WORD egcrmcvt(src, srcl, dst, dstl, totl, cmode)
    UBYTE          *src;               /* source string */
    UWORD           srcl;              /* source string length */
    UBYTE          *dst;               /* destination string */
    UWORD          *dstl;              /* destination string length */
    UWORD          *totl;              /* total length */
    UBYTE           cmode;             /* 0:hankaku 1:katakana 2:hirakana */
/* f/c = 0:no convertion 1:all convertion 2:partial convertion and beep */
/*                                        3:partial convertion          */
{
#ifdef UNIX
    WORD            st;

    if (srcl > (UWORD) BUFSIZ)
        return (EGCRM_NOT);
    if (inputcvt(src, srcl, inputbuf, &inputlen) != NORMAL)
        return ((WORD) EGCRM_NOT);
    st = (WORD) romaji(cmode, inputbuf, (UWORD) 0, inputlen,
                       outputbuf, &outputlen, totl);
    if (outputcvt(outputbuf, outputlen, inputbuf, &inputlen) != NORMAL)
        return ((WORD) EGCRM_NOT);
    if (inputlen > (UWORD) BUFSIZ)
        return ((WORD) EGCRM_NOT);
    (VOID) nullcvt(inputbuf, inputlen, dst, dstl);
    if ((*totl - outputlen) != (UWORD) 0) {
        if (outputcvt(outputbuf + outputlen, *totl - outputlen,
                      inputbuf, &inputlen) != NORMAL) {
            return ((WORD) EGCRM_NOT);
        }
        if ((inputlen + *dstl) > (UWORD) BUFSIZ)
            return ((WORD) EGCRM_NOT);
        (void) nullcvt(inputbuf, inputlen, dst + *dstl, totl);
        *totl += *dstl;

    }
    else {
        *totl = *dstl;
    }
    switch (st) {
    case 0:
        st = EGCRM_NOT;
        break;
    case 1:
        st = EGCRM_ALL;
        break;
    case 2:
        st = EGCRM_PCE;
        break;
    case 3:
        st = EGCRM_PCN;
        break;
    default:
        st = EGCRM_NOT;
        break;
    }
    return (st);
#else
    if (*src == 0xFF && srcl == 1)
        return (0);
    return (romaji(cmode, src, (UWORD) 0, srcl, dst, dstl, totl));
#endif
}
#endif
/* PAGE */
/*-----------------------------------------------------------*/
/*   egcjacvt : jis to ascii convertion                      */
/*-----------------------------------------------------------*/
NONREC
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

egcjacvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination string length */
/*    f/c  = 0:normal, !=0 error                 */
{
#ifdef UNIX
    *dstl = (UWORD) 0;
    if (srcl == (UWORD) 0)
        return ((WORD) ERJAC03);    /* jis length error */

    if (srcl > (UWORD) BUFSIZ)
        return ((WORD) ERJAC03);

    if (inputcvt(src, srcl, inputbuf, &inputlen) != NORMAL) {
        return ((WORD) ERJAC01);
    }

    outputlen = jistoasc(inputbuf, inputbuf + inputlen, outputbuf);

    if (outputlen == (UWORD) 0)
        return ((WORD) ERJAC01);    /* no convertion error */

    if (outputcvt(outputbuf, outputlen, inputbuf, &inputlen) != NORMAL)
        return ((WORD) ERJAC01);

    if (inputlen > (UWORD) BUFSIZ)
        return ((WORD) ERJAC10);

    (void) nullcvt(inputbuf, inputlen, dst, dstl);
#else
    *dstl = jistoasc(src, src + srcl, dst);
    if (*dstl == 0)
        return (ERJAC01);
#endif
    return (NORMAL);
}

/*-----------------------------------------------------------*/
/*   egcajcvt : asc to jis   convertion                      */
/*-----------------------------------------------------------*/
NONREC
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

egcajcvt(src, srcl, dst, dstl, cmode)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination string length */
    UBYTE           cmode;             /* 1:katakana 2:hirakana     */
/*    f/c  = 0:normal, !=0 error                 */
{
#ifdef UNIX
    *dstl = (UWORD) 0;
    if ((srcl == (UWORD) 0) || (srcl > (UWORD) BUFSIZ))
        return ((WORD) ERAJC03);    /* ascii length error */
    if (inputcvt(src, srcl, inputbuf, &inputlen) != NORMAL) {
        return ((WORD) ERAJC01);
    }
    outputlen = cvtatoj(cmode, inputbuf, inputlen, outputbuf);
    if (outputlen == (UWORD) 0)
        return ((WORD) ERAJC01);    /* no convertion error */
    if (outputcvt(outputbuf, outputlen, inputbuf, &inputlen) != NORMAL) {
        return ((WORD) ERAJC01);
    }
    if (inputlen > (UWORD) BUFSIZ)
        return ((WORD) ERAJC10);
    (void) nullcvt(inputbuf, inputlen, dst, dstl);
#else
    *dstl = cvtatoj(cmode, src, srcl, dst);
    if (*dstl == 0)
        return (ERAJC01);
#endif
    return (NORMAL);
}
/* PAGE */
#ifndef DOS_EGBRIDGE
/*-----------------------------------------------------------*/
/*   egcsjcvt : shift-jis to jis(2020-)                      */
/*-----------------------------------------------------------*/
NONREC
WORD egcsjcvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
 /* f/c = 0:normal, !=0 error */
{
    WORD            norm;
    UWORD           xi;
    UWORD           scode, jcode;
    register UWORD  i;

    *dstl = (UWORD) 0;

    if (!(xi = srcl >> 1))
        return ((WORD) ERSJC03);    /* s-jis length error */

    norm = (WORD) NORMAL;
    i = (UWORD) 0;
    while ((i < xi) && (norm == (WORD) NORMAL)) {
        scode = (*(src + i * 2) << 8) + (*(src + i * 2 + 1));
        jcode = sjcnvtr(scode);
        if (jcode) {
            *(dst + i * 2) = (UBYTE) ((jcode >> 8) + 0x20);
            *(dst + i * 2 + 1) = (UBYTE) (jcode + 0x20);
        }
        else
            norm = (WORD) ERSJC02;  /* s-jis string error */
        i++;
    }
    *dstl = srcl;
    return (norm);
}

/*-----------------------------------------------------------*/
/*   egcjscvt : jis(2020-) to shift-jis                      */
/*-----------------------------------------------------------*/
NONREC
WORD egcjscvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
 /* f/c = 0:normal, !=0 error */
{
    WORD            norm;
    UWORD           xi;
    UWORD           scode;
    UWORD           jcode;
    register UWORD  i;

    *dstl = (UWORD) 0;

    if (!(xi = srcl >> 1))
        return ((WORD) ERJSC03);    /* jis length error */

    norm = (WORD) NORMAL;
    i = (UWORD) 0;
    while ((i < xi) && (!norm)) {
        jcode = (((*(src + i * 2)) - 0x20) << 8) + ((*(src + i * 2 + 1)) - 0x20);
        scode = jscnvtr(jcode);
        if (scode) {
            *(dst + i * 2) = (UBYTE) (scode >> 8);
            *(dst + i * 2 + 1) = (UBYTE) (scode);
        }
        else
            norm = (WORD) ERJSC02;  /* jis string error */

        i++;
    }
    *dstl = srcl;
    return (norm);
}
#endif
#ifdef UNIX
/*-----------------------------------------------------------*/
/*   egcsecvt : SHIFT JIS to EUC Kanji                       */
/*-----------------------------------------------------------*/
NONREC
WORD egcsecvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
/*    f/c  = NORMAL -> normal, != NORMAL -> error    */
{
    register int    ch1, ch2;
    register int    ph, pl;
    register int    i;
    register int    j;

    i = 0;
    j = 0;
    *dstl = (UWORD) 0;
    if (srcl == (UWORD) 0)
        return ((WORD) ERSEC03);
    while (i < (int) srcl) {
        ch1 = src[i];
        /* Ascii */
        if (ch1 <= 0x80
            || ch1 >= 0xf0) {
            dst[j++] = ch1;
            i++;
        }
        else if (ch1 >= 0xa0 && ch1 < 0xe0) {
            dst[j++] = 0x8e;
            dst[j++] = ch1;
            i++;
        }
        else {
            /* Kanji */
            ch2 = src[i + 1];
            if (ch1 < 0xe0) {
                ph = (ch1 << 1) - 0xe1;
            }
            else {
                ph = (ch1 << 1) - 0x161;
            }
            if (ch2 >= 0x9f) {
                ++ph;
                pl = ch2 - 0x7E;
            }
            else {
                pl = ch2 - 0x1f - (ch2 >= 0x80);
            }
            ph += 0x80;
            pl += 0x80;
            dst[j++] = ph;
            dst[j++] = pl;
            i += 2;
        }
    }
    *dstl = (UWORD) j;
    return ((WORD) NORMAL);
}

/*-----------------------------------------------------------*/
/*   egcescvt : EUC Kanji to SHIFT JIS                       */
/*-----------------------------------------------------------*/
NONREC
WORD egcescvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
/*    f/c  = NORMAL -> normal, != NORMAL -> error    */
{
    unsigned char   ch, cl;
    unsigned char   jh, jl;
    register int    i;
    register int    j;

    i = 0;
    j = 0;
    *dstl = (UWORD) 0;
    if (srcl == (UWORD) 0)
        return ((WORD) ERESC03);

    while (i < (int) srcl) {
        ch = src[i];

        /* Ascii */
        if (ch == 0x8e) {
            if (++i < (int) srcl) {
                ch = src[i];
                dst[j++] = ch;
                i++;
            }
        }
        else if (ch <= 0x80) {
            dst[j++] = ch;
            i++;
        }
        else {
            jh = ch - 0x80;
            jl = src[i + 1] - 0x80;

            ch = ((jh + 1) / 2) + 0x70;
            cl = jl + 0x1f;
            cl += (jh % 2) ? 0 : 0x5e;
            cl += (cl > 0x7e) ? 1 : 0;
            ch += (ch > 0x9e) ? 0x40 : 0;
            dst[j++] = ch;
            dst[j++] = cl;
            i += 2;
        }
    }
    *dstl = (UWORD) j;
    return ((WORD) NORMAL);
}

/*-----------------------------------------------------------*/
/*   egciecvt : IKIS to EUC Kanji                            */
/*-----------------------------------------------------------*/
NONREC
WORD egciecvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
/*    f/c  = NORMAL -> normal, != NORMAL -> error    */
{
    WORD            norm;
    register UWORD  i;
    UWORD           status;
    UWORD           dest_length;
    UBYTE           ch;

    *dstl = (UWORD) 0;

    /* length error check : please define appropriate error code */
    if (srcl == 0)
        return ((WORD) ERIEC03);

    norm = (WORD) NORMAL;

    status = (UWORD) 0;
    dest_length = (UWORD) 0;
    for (i = srcl; i < srcl; i++) {
        ch = *(src++);
        if (status == (UWORD) 0) {
            if ((ch >= 0240) && (ch <= 0376)) {
                status = (UWORD) 1;
            }
            if (ch == (UWORD) 0216) {
                ch = (UWORD) 0250;
            }
        }
        else {                      /* if ( status == ( UWORD )1 ) */
            if (!(*src & 0200)) {
                /* error ? : please define error code */
                /* break ? */
                *dstl = (UWORD) 0;
                return ((WORD) ERIEC01);
            }
            status = (UWORD) 0;
        }
        *(dst++) = ch;
        dest_length++;
    }
    *dstl = dest_length;
    return (norm);
}

/*-----------------------------------------------------------*/
/*   egceicvt : EUC Kanji to IKIS                            */
/*-----------------------------------------------------------*/

#define IKIS_NORMAL     0
#define IKIS_FIRST      1
#define IKIS_SS2        2
#define IKIS_SS3_I      3
#define IKIS_SS3_II     4

NONREC
WORD egceicvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
/*    f/c  = NORMAL -> normal, != NORMAL -> error    */
{
    WORD            norm;
    register UWORD  i;
    UWORD           state;
    UWORD           dest_length;
    UBYTE           ch;

    *dstl = (UWORD) 0;
    /* length error check : please define appropriate error code */
    if (srcl == 0) {
        return ((WORD) EREIC03);
    }
    norm = (WORD) NORMAL;
    dest_length = (UWORD) 0;
    state = (UWORD) IKIS_NORMAL;
    for (i = srcl; i < srcl; i++) {
        ch = *(src++);
        switch (state) {
        case IKIS_NORMAL:
            if (!(ch & 0200)) {
                /* CODE SET 0 */
            }
            else if (ch == 0216) {
                ch = 0250;
                state = (UWORD) IKIS_SS2;
            }
            else if (ch == 0217) {
                state = (UWORD) IKIS_SS3_I;
            }
            else if ((ch >= 0200) && (ch <= 0240)) {
                /* C1 area code */
                /*
                 * NOTE. ISSS2 and ISSS3 choud be previously checked
                 */
                /*
                 * becouse they are include in C1 area
                 */
            }
            else {                  /* EUC Kanji */
                state = IKIS_FIRST;
            }
            break;
        case IKIS_FIRST:
        case IKIS_SS2:
            state = IKIS_NORMAL;
            break;
        case IKIS_SS3_I:
            state = IKIS_SS3_II;
            break;
        case IKIS_SS3_II:
            state = NORMAL;
            break;
        }
        *(dst++) = ch;
        dest_length++;
    }
    *dstl = dest_length;
    return (norm);
}

/*-----------------------------------------------------------*/
/*   nullcvt : null convert                                  */
/*-----------------------------------------------------------*/
NONREC
WORD nullcvt(src, srcl, dst, dstl)
    UBYTE          *src;               /* source string             */
    UWORD           srcl;              /* source string length      */
    UBYTE          *dst;               /* destination string        */
    UWORD          *dstl;              /* destination length        */
/*    f/c  = NORMAL -> normal, != NORMAL -> error    */
{
    egcmemmove(dst, src, srcl);
    *dstl = srcl;
    return ((WORD) NORMAL);
}
#endif
