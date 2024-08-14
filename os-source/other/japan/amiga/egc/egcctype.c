/************************************************************************/
/*                                                                      */
/*      EGCCTYPE : EGConvert Chractor and string TYPE check             */
/*                                                                      */
/*              Designed    by  I.Iwata         01/01/1985 (1,2,3)      */
/*              Coded       by  I.Iwata         01/01/1985 (1,2,3)      */
/*              Designed    by  H.Yanata        01/01/1992 (4,5,6)      */
/*              Coded       by  H.Yanata        01/01/1992 (4,5,6)      */
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
/*      1  ctrlcheck(strp,slen)                                         */
/*      2  strcheck(strp,slen)                                          */
/*      3  codecheck(type,scode)                                        */
/*      4  NtoKCheck(src,srcl,numl)                                     */
/*      5  YStrChk(yomip,yomil)                                         */
/*      6  YTypeChk(yomip)                                              */
/*      7  IsIncluding(yomip,yomil,type)                                */
/*      8  IsStrAlpha(yomip, yomil)                                     */
/*      9  IsStrKKana(yomip, yomil)                                     */
/*                                                                      */
/************************************************************************/
#include    "edef.h"
#include    "egcdef.h"
#include    "egcproto.h"
#include    "egckcnv.h"

#define    HANKANA     0x03                 /* hankaku kana */
#define    HKANA       0x02                 /* hirakana     */
#define    KKANA       0x01                 /* katakana     */
#define    HANKAKU     0x00                 /* hankaku      */

/*
*** external data
*/
extern UBYTE    chrtbl[256];

#ifndef UNIX
/*--------------------------------------------------------------*/
/*      ctrlchk : control code check                            */
/*--------------------------------------------------------------*/
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
ctrlchk(strp, slen)
    UBYTE          *strp;              /* ->target string                */
    UWORD           slen;              /* string length                  */
{
    UBYTE           hflag;             /* TRUE:not control FALSE:control */
    UWORD           i;                 /* counter                        */
    UBYTE           tcode;             /* temp code                      */
    hflag = TRUE;
    for (i = 0; i < slen; i++) {
        tcode = *strp++;
        if (((tcode >= JV1FR) && (tcode <= JV1TO))
            || ((tcode >= JV2FR) && (tcode <= JV2TO))) {
            strp++;
            i++;
        }
        else if (tcode < 0x20) {
            hflag = FALSE;
            break;
        }
    }
    return (hflag);
}
#endif                                 /* Not UNIX */
/*------------------------------------------------------------------*/
/*  strcheck : yomi string check                                    */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
strcheck(src, srcl)
    UBYTE*  src;
    UWORD   srcl;
{
    UBYTE           dst[80];
    UBYTE*          p = dst;
    UWORD           ascl = 0;
    UBYTE           fchar;
    BOOL            st = NORMAL, loop = TRUE;

    ascl = jistoasc(src, src + srcl, dst);
    do {
        switch (fchar = *(chrtbl + *p++)) {
            case    CHJIS:
            case    CHNON:
            case    CHNUM:
            case    CHALPHA:
            case    CHSEP:
            case    CHEOT:
                st = ERROR;
                loop = FALSE;
                break;
        }
    } while((--ascl > 0) && loop);
    return  st;
}
/*------------------------------------------------------------------*/
/*  NtoKCheck : number to kansuji check                             */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
NtoKCheck(src, srcl, numl)
    UBYTE*    src;
    UWORD     srcl;
    UWORD*    numl;
{
    register UWORD  i = 0;
     BOOL           ntokflag = TRUE;

    *numl = 0;
    while (i < srcl) {
        if ((*(src + i) == 0x82)
            && ((*(src + i + 1) >= 0x4f) && (*(src + i + 1) <= 0x58))) {
            (*numl) += 2; i += 2;
        } else if ((0x30 <= *(src + i)) && (0x39 >= *(src + i))) {
            (*numl) ++;   i ++;
        } else {
            break;
        }
    }
    return    (*numl != (UWORD)0)?    TRUE : FALSE;
}

/*----------------------------------------------------------------*/
/*      codecheck : check char type                               */
/*          f/v   = HANKAKU(0) KKANA(1) HKANA(2)                  */
/*----------------------------------------------------------------*/
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
codecheck(type, scode)
    UBYTE           type;              /* previous type                      */
    UBYTE          *scode;             /* asciicode or highbyte of sjiscode  */
{
    UBYTE           st;                /* function value to be returned      */
    st = (UBYTE) HANKAKU;
    if (((*scode >= JV1FR) && (*scode <= JV1TO)) ||
        ((*scode >= JV2FR) && (*scode <= JV2TO))) {
        if (((*scode == 0x83) && (*(scode + 1) >= 0x40) &&
             (*(scode + 1) <= 0x96)) ||
            ((*scode == 0x81) && (*(scode + 1) == 0x5b) &&
             (type == (UBYTE) KKANA)) ||
             ((*scode == 0x81) && (*(scode + 1) == 0x67)))
            st = (UBYTE) KKANA;     /* katakana then 1              */
        else
            st = (UBYTE) HKANA;     /* hirakana(incl. kigou) then 2 */
        return (st);
    }
    else
        return (st);                /* hankaku then 0               */
}
/*PAGE*/

/*------------------------------------------------------------------*/
/*  YStrChk : yomi string check                                     */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
YStrChk(yomip, yomil)
    UBYTE*  yomip;
    UWORD   yomil;
{
    register UWORD  i;
    register UWORD  ctype;

    for (i = 0; i < yomil; i ++, yomip ++) {
        ctype = YTypeChk(yomip);
        if ((ctype == ILLEGAL_YOMI) ||
            (ctype == NEUTRAL_YOMI && i == 0))
            return  FALSE;
    }
    return  TRUE;
}

/*------------------------------------------------------------------*/
/*  YTypeChk : Yomi Type Check                                      */
/*------------------------------------------------------------------*/
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
YTypeChk(yomip)
    UBYTE*    yomip;
{
    register UBYTE  ch = chrtbl[*yomip];
	UWORD           yType = NORMAL_YOMI;

	if (ch == CHNORMAL) {
        if (*yomip < 0x7f || *yomip == 0xa5)
            yType = EXTRA_YOMI;
        else
            yType = (UBYTE)NORMAL_YOMI;
    } else if (ch == CHSUB || ch == CHKINSOK) {
        yType = NEUTRAL_YOMI;
    } else {
        yType = ILLEGAL_YOMI;
    }
    return  yType;
}

/*------------------------------------------------------------------*/
/*  IsIncluding : Is string including ?                             */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
IsIncluding(yomip, yomil, type)
    UBYTE*  yomip;
    UWORD   yomil;
    UBYTE   type;
{
    UBYTE           ctype = HKANA, ptype = HKANA;
    BOOL            st = FALSE;
    register UWORD  i;
    for (i = 0; i < yomil; i++) {
        if (type == HANKANA) {
            if ((ptype = codecheck(ctype, yomip + i)) == HANKAKU
                && (0xB1 <= *(yomip + i) && *(yomip + i) <= 0xDD)) {
                st = TRUE;
                break;
            }
        } else {
            if ((ptype = codecheck(ctype, yomip + i)) == type) {
                st = TRUE;
                break;
            }
            i++;
        }
        ctype = ptype;
    }
    return  st;
}

/*------------------------------------------------------------------*/
/*  IsStrAlpha : Is string alphabet ?                               */
/*------------------------------------------------------------------*/
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
IsStrAlpha(src, srcl)
    UBYTE*  src;
    UWORD   srcl;
{
    register UWORD  i;
    register UBYTE* p = src;
    extern UBYTE jta[94];
	register UBYTE	c;

    for (i = 0; i < srcl; i++, p++) {
        if (! ((*p >= 0x21 && *p <= 0x2F) || (*p >= 0x40 && *p <= 0x7E))) {
            if ((*p == 0x82 && ((c = *(p + 1)) >= 0x60 && c <= 0x9A)) ||
            	(*p == 0x81 && ((c = *(p + 1)) >= 0x43 && c <= 0x9E) &&
                    jta[c - ((c > 0x7E)? 0X41 : 0X40)] &&
                    (c != 0X75 && c != 0X76) &&
                    (c != 0X69 && c != 0X6A) &&
                    (c != 0X6F && c != 0X70))) {
                    i++; p++;
            } else
                break;
        }
    }
    return  i;
}

/*------------------------------------------------------------------*/
/*  IsStrKKana : Is string kata-kana ?                              */
/*------------------------------------------------------------------*/
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
IsStrKKana(src, srcl)
    UBYTE*  src;
    UWORD   srcl;
{
    UBYTE           ctype = HKANA, ptype = HKANA;
    register UWORD  i;

    for (i = 0; i < srcl; i += 2) {
        if ((ptype = codecheck(ctype, src + i)) != KKANA) {
            break;
        }
        ctype = ptype;
    }
    return  i;
}
/*------------------------------------------------------------------*/
/*  IsStrSep : Is string separate simbol ?                          */
/*------------------------------------------------------------------*/
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
IsStrSep(src, srcl)
    UBYTE*  src;
    UWORD   srcl;
{
    register UWORD  i;
    register UBYTE  c;
    for (i = 0; i < srcl; i += 2) {
        if (*(src + i) == 0x81
            && ((c = *(src + i + 1)) == 0x41
                                || c == 0x42
                                || c == 0x43
                                || c == 0x44))
            return  TRUE;
    }
    return  FALSE;
}

