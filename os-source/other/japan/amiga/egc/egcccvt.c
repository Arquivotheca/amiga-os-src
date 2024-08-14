/************************************************************************/
/*                                                                      */
/*      EGCCCVT : EGConvert Code ConVerT                                */
/*                                                                      */
/*         Designed     by   M.Nishibayasi   10/20/1985                 */
/*         Coded        by   M.Nishibayasi   10/20/1985                 */
/*                                                                      */
/*       (C) Copyright 1985-90 ERGOSOFT Corp.                           */
/*       All Rights Reserved                                            */
/*                                                                      */
/*                            --- NOTE ---                              */
/*                                                                      */
/*       THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A    */
/*       TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES      */
/*       WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.    */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include "edef.h"
#include "egcenv.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcproto.h"

/* numeral to kansuji convert    */
#define  ERR     TRUE
#define  CONV    FALSE

/*****************   table of Katakana   *************************/

UWORD           cvkana[192] =
{
/*    CHAR  $20 TO $2F   */
    0X8140, 0X8149, 0X8168, 0X8194, 0X8190, 0X8193, 0X8195, 0X8166,
    0X8169, 0X816a, 0X8196, 0X817B, 0X8143, 0X817C, 0X8144, 0X815E,
/*    CHAR  $30 TO $3F   */
    0X824F, 0X8250, 0X8251, 0X8252, 0X8253, 0X8254, 0X8255, 0X8256,
    0X8257, 0X8258, 0X8146, 0X8147, 0X8183, 0X8181, 0X8184, 0X8148,
/*    CHAR  $40 TO $4F   */
    0X8197, 0X8260, 0X8261, 0X8262, 0X8263, 0X8264, 0X8265, 0X8266,
    0X8267, 0X8268, 0X8269, 0X826A, 0X826B, 0X826C, 0X826D, 0X826E,
/*    CHAR  $50 TO $5F   */
    0X826F, 0X8270, 0X8271, 0X8272, 0X8273, 0X8274, 0X8275, 0X8276,
    0X8277, 0X8278, 0X8279, 0X816D, 0X818F, 0X816E, 0X814F, 0X8151,
/*    CHAR  $60 TO $6F   */
    0X814D, 0X8281, 0X8282, 0X8283, 0X8284, 0X8285, 0X8286, 0X8287,
    0X8288, 0X8289, 0X828A, 0X828B, 0X828C, 0X828D, 0X828E, 0X828F,
/*    CHAR  $70 TO $7F  */
    0X8290, 0X8291, 0X8292, 0X8293, 0X8294, 0X8295, 0X8296, 0X8297,
    0X8298, 0X8299, 0X829A, 0X816F, 0X8162, 0X8170, 0X8160, 0X8140,
/*    CHAR  $80 TO $9F  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*    CHAR  $A0 TO $AF  */
    0X8140, 0X8142, 0X8175, 0X8176, 0X8141, 0X8145, 0X8392, 0X8340,
    0X8342, 0X8344, 0X8346, 0X8348, 0X8383, 0X8385, 0X8387, 0X8362,
/*    CHAR  $B0 TO $BF  */
    0X815B, 0X8341, 0X8343, 0X8345, 0X8347, 0X8349, 0X834A, 0X834C,
    0X834E, 0X8350, 0X8352, 0X8354, 0X8356, 0X8358, 0X835A, 0X835C,
/*    CHAR  $C0 TO $CF  */
    0X835E, 0X8360, 0X8363, 0X8365, 0X8367, 0X8369, 0X836A, 0X836B,
    0X836C, 0X836D, 0X836E, 0X8371, 0X8374, 0X8377, 0X837A, 0X837D,
/*    CHAR  $D0 TO $DF  */
    0X837E, 0X8380, 0X8381, 0X8382, 0X8384, 0X8386, 0X8388, 0X8389,
    0X838A, 0X838B, 0X838C, 0X838D, 0X838F, 0X8393, 0X814A, 0X814B
};

/*****************  TABLE OF HIRAGANA **********************/

UWORD           cvhira[64] =
{
/* CHAR $A0 TO $AF */
    0X8140, 0X8142, 0X8175, 0X8176, 0X8141, 0X8145, 0X82F0, 0X829F,
    0X82A1, 0X82A3, 0X82A5, 0X82A7, 0X82E1, 0X82E3, 0X82E5, 0X82C1,
/* CHAR $B0 TO $BF */
    0X815B, 0X82A0, 0X82A2, 0X82A4, 0X82A6, 0X82A8, 0X82A9, 0X82AB,
    0X82AD, 0X82AF, 0X82B1, 0X82B3, 0X82B5, 0X82B7, 0X82B9, 0X82BB,
/* CHAR $C0 TO $CF */
    0X82BD, 0X82BF, 0X82C2, 0X82C4, 0X82C6, 0X82C8, 0X82C9, 0X82CA,
    0X82CB, 0X82CC, 0X82CD, 0X82D0, 0X82D3, 0X82D6, 0X82D9, 0X82DC,
/* CHAR $D0 TO $DF */
    0X82DD, 0X82DE, 0X82DF, 0X82E0, 0X82E2, 0X82E4, 0X82E6, 0X82E7,
    0X82E8, 0X82E9, 0X82EA, 0X82EB, 0X82ED, 0X82F1, 0X814A, 0X814B
};

UBYTE           DHHira[88] =
{
/* 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,  */
    /* $829F to $82BF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    /* $82C0 to $82DF */
    0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0,
    /* $82E0 to $82F1 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

UBYTE           DHKana[88] =
{
/* 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,  */
    /* $8340 to $835F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    /* $8360 to $837F */
    1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0,
    /* $8380 to $8396 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*****************   table of JIS to ASCII ***********************/
UBYTE           jta[94] =
{
/*    CHAR  .1 - .94     */
/*               1    2    3    4    5    6    7    8    9   10  */
     /* 1-10 */ 0x20, 0xa4, 0xa1, 0x2c, 0x2e, 0xa5, 0x3a, 0x3b, 0x3f, 0x21,
     /* 11-20 */ 0xde, 0xdf, 0x00, 0x60, 0x00, 0x5e, 0x00, 0x5f, 0x00, 0x00,
     /* 21-30 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00,
     /* 31-40 */ 0x2f, 0x00, 0x7e, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x27, 0x00,
     /* 41-50 */ 0x22, 0x28, 0x29, 0x00, 0x00, 0x5b, 0x5d, 0x7b, 0x7d, 0x3c,
     /* 51-60 */ 0x3e, 0x00, 0x00, 0xa2, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x2b,
     /* 61-70 */ 0x2d, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x3c, 0x3e, 0x00, 0x00,
     /* 71-80 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x24,
     /* 81-90 */ 0x00, 0x00, 0x25, 0x23, 0x26, 0x2a, 0x40, 0x00, 0x00, 0x00,
     /* 91-94 */ 0x00, 0x00, 0x00, 0x00
};

UBYTE           jtax[188] =
{
/*    CHAR    .4/5  .1 - .94     */
/*            .1xx  .for dakuten */
/*            .2xx  .for handakuten*/
/*                    1         2         3         4         5  */
     /* 1- 5 */ 0x00, 0xa7, 0x00, 0xb1, 0x00, 0xa8, 0x00, 0xb2, 0x00, 0xa9,
     /* 6-10 */ 0x00, 0xb3, 0x00, 0xaa, 0x00, 0xb4, 0x00, 0xab, 0x00, 0xb5,
     /* 11-15 */ 0x00, 0xb6, 0x01, 0xb6, 0x00, 0xb7, 0x01, 0xb7, 0x00, 0xb8,
     /* 16-20 */ 0x01, 0xb8, 0x00, 0xb9, 0x01, 0xb9, 0x00, 0xba, 0x01, 0xba,
     /* 21-25 */ 0x00, 0xbb, 0x01, 0xbb, 0x00, 0xbc, 0x01, 0xbc, 0x00, 0xbd,
     /* 26-30 */ 0x01, 0xbd, 0x00, 0xbe, 0x01, 0xbe, 0x00, 0xbf, 0x01, 0xbf,
     /* 31-35 */ 0x00, 0xc0, 0x01, 0xc0, 0x00, 0xc1, 0x01, 0xc1, 0x00, 0xaf,
     /* 36-40 */ 0x00, 0xc2, 0x01, 0xc2, 0x00, 0xc3, 0x01, 0xc3, 0x00, 0xc4,
     /* 41-45 */ 0x01, 0xc4, 0x00, 0xc5, 0x00, 0xc6, 0x00, 0xc7, 0x00, 0xc8,
     /* 46-50 */ 0x00, 0xc9, 0x00, 0xca, 0x01, 0xca, 0x02, 0xca, 0x00, 0xcb,
     /* 51-55 */ 0x01, 0xcb, 0x02, 0xcb, 0x00, 0xcc, 0x01, 0xcc, 0x02, 0xcc,
     /* 56-60 */ 0x00, 0xcd, 0x01, 0xcd, 0x02, 0xcd, 0x00, 0xce, 0x01, 0xce,
     /* 61-65 */ 0x02, 0xce, 0x00, 0xcf, 0x00, 0xd0, 0x00, 0xd1, 0x00, 0xd2,
     /* 66-70 */ 0x00, 0xd3, 0x00, 0xac, 0x00, 0xd4, 0x00, 0xad, 0x00, 0xd5,
     /* 71-75 */ 0x00, 0xae, 0x00, 0xd6, 0x00, 0xd7, 0x00, 0xd8, 0x00, 0xd9,
     /* 76-80 */ 0x00, 0xda, 0x00, 0xdb, 0x00, 0xdc, 0x00, 0xdc, 0x00, 0xb2,
     /* 81-85 */ 0x00, 0xb4, 0x00, 0xa6, 0x00, 0xdd, 0x01, 0xb3, 0x00, 0xb6,
     /* 86-90 */ 0x00, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     /* 91-94 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*-------------------------------------------------------------------*/
/*   cvtatoj                                                         */
/*-------------------------------------------------------------------*/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

cvtatoj(mode, ascs, ascl, jiss)
    UBYTE           mode;
    UBYTE          *ascs;
    UWORD           ascl;
    UBYTE          *jiss;
{
/*--- automatic variables ---*/
    UBYTE           dhok;
    register UWORD  n;
    register UBYTE *pd;
    UBYTE          *asce;

/*--- begin of cvtatoj ---*/
    pd = jiss;
    asce = ascs + ascl;
    while (ascs < asce) {
        if (((*ascs >= JV1FR) && (*ascs <= JV1TO)) ||
            ((*ascs >= JV2FR) && (*ascs <= JV2TO))) {
            *(pd++) = *(ascs++);
            *(pd++) = *(ascs++);
        }
        else {
            n = asctojis(mode, *(ascs++));
            dhok = (UBYTE) FALSE;
            if (pd > jiss)
                if (n == 0x814A)
                    dhok = dhcheck(pd - 2, (UBYTE) TRUE);
                else if (n == 0x814B)
                    dhok = dhcheck(pd - 2, (UBYTE) FALSE);
            if (!dhok) {
                if (n > (UWORD) 255) {
                    *(pd++) = (UBYTE) (n >> 8);
                    *(pd++) = (UBYTE) (n & 0x00ff);
                }
                else {
                    *(pd++) = (UBYTE) (n & 0x00ff);
                }
            }
        }
    }                               /* end while */
    return ((UWORD) (pd - jiss));

    /*--- end of cvtatoj ---*/
}
/*********************************************************************/
/**                                                                 **/
/**  asctojis : convert ascii-code to jis-code                       **/
/**                                                                 **/
/*********************************************************************/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

asctojis(emode, scode)
    UBYTE           emode, scode;
{
    UWORD           wcode, icode;

    wcode = scode;
    if (((wcode >= 0x20) && (wcode < 0x7f)) ||
        ((wcode >= 0xa0) && (wcode < 0xe0))) {
        if ((emode == (UBYTE) 2) && (wcode >= 0xa0) && (wcode < 0xe0)) {
            icode = cvhira[wcode - 0xa0];
        }
        else {
            icode = cvkana[wcode - 0x20];
        }
    }
    else {
        icode = wcode;
    }
    return (icode);
}

/*********************************************************************/
/**                                                                 **/
/**  dhcheck : check dakuten/handakuten                             **/
/**                                                                 **/
/*********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

dhcheck(p, s)                    /* m */
    UBYTE          *p;
    UBYTE           s;
{

    UWORD           n, m;
    UBYTE           d, dhf;

    n = *(p++);
    m = (n << 8) + *p;

    if ((m == 0X8345) && (s == (UBYTE) TRUE)) { /* )^        */
        (*p) += (0x94 - 0x45);
        return ((UBYTE) TRUE);
    }
    else if ((m >= 0X829F) && (m < 0X82F2))     /* hiragana? */
        dhf = DHHira[m - 0X829F];
    else if ((m >= 0X8340) && (m < 0X8397))     /* katakana? */
        dhf = DHKana[m - 0X8340];
    else
        dhf = 0;

    switch (dhf) {
    case 0:
        d = (UBYTE) FALSE;
        break;
    case 1:
        if (s) {
            (*p)++;
            d = (UBYTE) TRUE;
        }
        else {
            d = (UBYTE) FALSE;
        } break;
    case 2:
        if (s) {
            (*p)++;
            d = (UBYTE) TRUE;
        }
        else {
            (*p) += 2;
            d = (UBYTE) TRUE;
        } break;
    default:
        d = (UBYTE) FALSE;
        break;
    }
    return (d);
}

/*********************************************************************/
/**                                                                 **/
/**  jistoasc : convert jis-code to ascii-code                      **/
/**                                                                 **/
/*********************************************************************/
NONREC
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

jistoasc(jiss, jise, ascs)       /* m */
    UBYTE          *jiss;              /* -> jis-strings (start) */
    UBYTE          *jise;              /* -> jis-strings (end)   */
    UBYTE          *ascs;              /* -> ascii-strings       */
 /* F/Val = ascii-string length */
{
    UBYTE          *jptr, *aptr;       /* pointer for  JIS-strng ASCII-strng */
    UBYTE           attr;              /* pointer for  JIS-strng ASCII-strng */
    UBYTE           acode;             /* pointer for  JIS-strng ASCII-strng */
    UWORD           i;                 /* UBYTE  i;             counter */
    UWORD           scode;             /* shift jis code */

    jptr = jiss;                    /* jis start address */
    aptr = ascs;                    /* ascii start address */
    i = (UWORD) 0;
    while (jptr < jise) {           /* repeat until end of text */
        if (((*jptr >= JV1FR) && (*jptr <= JV1TO)) ||
            ((*jptr >= JV2FR) && (*jptr <= JV2TO))) {
            scode = (*jptr << 8) + *(jptr + 1);
            acode = jacnvtr(scode, &attr);
            if (acode != (UBYTE) 0) {
                *(aptr++) = acode;
                i++;
                if (attr != (UBYTE) 0) {
                    /*@@@ Modified 06/12/1991 H.YAnata @@@*/
                    /**(aptr++) = attr - (UBYTE) 1 + 0xde;*/
                    *(aptr++) = attr - (UBYTE)1 + (UBYTE)0xde;
                    i++;
                }
                jptr += 2;
            }
            else {
                *(aptr++) = *(jptr++);
                i++;
                *(aptr++) = *(jptr++);
                i++;
            }
        }
        else {
            *(aptr++) = *(jptr++);
            i++;
        }
    }
    return (i);
}                                      /* end of jistoasc */

/*-------------------------------------------------------------------*/
/*   jacnvtr                                                         */
/*-------------------------------------------------------------------*/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

jacnvtr(scode, attrp)
    UWORD           scode;             /* shift jis code */
    UBYTE          *attrp;             /* attribute 0:normal 1:dakuten
                                        * 2:handakuten */
{
    UWORD           jcode;             /* jis kuten code working */
    UBYTE           r, c;              /* row, colum             */

    *attrp = (UBYTE) 0;
    if (scode <= 0xff)
        return ((UBYTE) scode);
    jcode = sjcnvtr(scode);
    r = (UBYTE) (jcode >> 8);
    c = (UBYTE) (jcode & 0x00ff);
    if (r == (UBYTE) 1)
        return (jta[c - 1]);
    else if (r == (UBYTE) 3) {
        /* numeric           upper-char        lower-char */
        if (((c >= 16) && (c <= 25)) || ((c >= 33) && (c <= 58))
            || ((c >= 65) && (c <= 90)))
            return ((UBYTE) (c + 0x20));
    }
    else if ((r == (UBYTE) 4) || (r == (UBYTE) 5)) {
        *attrp = jtax[(c - 1) * 2];
        return (jtax[(c - 1) * 2 + 1]);
    }
    else
        return ((UBYTE) 0);
    return ((UBYTE) 0);
}

/*********************************************************************/
/**  sjcnvtr : shift-jis  to jis(t-jis code-conversion              **/
/*********************************************************************/
NONREC
UWORD

sjcnvtr(scode)
    UWORD           scode;
{
    UBYTE           ih, il;            /* high byte & low byte */

    ih = (UBYTE) (scode >> 8);
    il = (UBYTE) (scode & 0x00ff);
    if ((ih >= JV1FR) && (ih <= JV1TO))
        ih -= JV1FR;
    else if ((ih >= JV2FR) && (ih <= JV2TO))
        ih = (ih - (UBYTE)JV2FR) + (UBYTE) 31;
    else
        return ((UWORD) 0);
    ih <<= 1;
    if ((il >= JH1FR) && (il <= JH1TO))
        il -= JH1FR;
    else if ((il >= JH2FR) && (il <= JH2TO))
        il = (il - (UBYTE)JH2FR) + (UBYTE)63;
    else if ((il >= JH3FR) && (il <= JH3TO)) {
        il -= JH3FR;
        ih++;
    }
    else
        return ((UWORD) 0);

    return ((UWORD) (((ih + 1) << 8) + il + 1));
}

/*********************************************************************/
/**  jscnvtr : jis(kuten) to shift-jis code-conversion              **/
/*********************************************************************/
NONREC
UWORD

jscnvtr(jcode)
    UWORD           jcode;
{
    UBYTE           ir, ic;

    ir = (UBYTE) (jcode >> 8);
    ic = (UBYTE) (jcode & 0x00ff);
    if ((ir < (UBYTE) 1) || (ir > (UBYTE) 94))
        return ((UWORD) 0);
    if ((ic < (UBYTE) 1) || (ic > (UBYTE) 94))
        return ((UWORD) 0);
    ir--;
    ic--;
    if (ir & 0x01)
        ic += 0x9f;
    else {
        if (ic > (UBYTE) 62)
            ic += 1;
        ic += 0x40;
    }
    ir >>= 1;
    if (ir >= (UBYTE) 31)
        ir = ir - (UBYTE)31 + (UBYTE)0xe0;
    else
        ir += 0x81;
    return ((UWORD) ((ir << 8) + ic));
}

/*********************************************************************/
/**     numeral to kansuji convert                                  **/
/**                                                                 **/
/**          UBYTE numtokansu(mode,src,scl,dst,dsl)                 **/
/**                                                                 **/
/*********************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

numtokansu(mode, src, scl, dst, dsl)
    UBYTE           mode;              /* kanji style */
    UBYTE          *src;               /* source pointer */
    UWORD           scl;               /* source length */
    UBYTE          *dst;               /* destination pointer */
    UWORD          *dsl;               /* destination length */
{                                      /* program start */
    UBYTE           astr[16];          /* ascii string area (16 bytes) */
    UWORD           nset[4];           /* set of numbers */
    UBYTE           i, j;              /* counter */
    UBYTE           cnbr;              /* ascii number */
    UBYTE           val[4];            /* handling number */
    UWORD           ascl;
    UWORD           pset, nx;          /* temporary numbers */
    UWORD           n;                 /* jis code */
    UBYTE          *darea;             /* destination pointer */

    if ((scl > (UWORD) 32) || (scl == (UWORD) 0))
        return ((UBYTE) ERR);
    for (i = (UBYTE) 0; i < (UBYTE) 4; i++)
        nset[i] = (UWORD) 0;
    cnbr = (UBYTE) jistoasc(src, src + scl, astr);
    if (cnbr > (UBYTE) 16)
        return ((UBYTE) ERR);
    for (j = (UBYTE) 0; j < (UBYTE) (3 - (cnbr - 1) / 4); j++);
    for (i = (UBYTE) 0; i < cnbr; i++) {
        if ((astr[i] < '0') || (astr[i] > '9'))
            return ((UBYTE) ERR);
        nset[j] = nset[j] * 10 + astr[i] - '0';
        if ((cnbr - i) % 4 == 1)
            j++;
    }
    darea = dst;
    if (mode == (UBYTE) 0) {
        egcmemmove(dst, astr, (UWORD) cnbr);
        darea += cnbr;
    }
    else if ((mode == (UBYTE) 1) ||
             (mode == (UBYTE) 2)) { /* case of normal kanji */
        for (i = (UBYTE) 0; i < (UBYTE) 4; i++) {
            if (nset[i] != (UWORD) 0) {
                pset = nset[i];
                for (j = (UBYTE) 0; j < (UBYTE) 4; j++) {
                    nx = pset / 10;
                    val[3 - j] = (UBYTE) (pset - nx * 10);
                    pset = nx;
                }
                for (j = (UBYTE) 0; j < (UBYTE) 4; j++) {
                    if (val[j] != (UBYTE) 0) {
                        if ((val[j] != (UBYTE) 1) || (j == (UBYTE) 3)) {
                            kansuji(mode, val[j], darea);
                            darea += 2;
                        }
                        if (j < (UBYTE) 3) {
                            kansuji(mode, (UBYTE) 12 - j, darea);
                            darea += 2;
                        }
                    }
                }
                if (i < (UBYTE) 3) {
                    kansuji((UBYTE) 3, (UBYTE) 3 - i, darea);
                    darea += 2;
                }
            }
        }
        if ((nset[0] | nset[1] | nset[2] | nset[3]) == (UWORD) 0) {
            kansuji(mode, (UBYTE) 0, darea);
            darea += 2;
        }
    }                               /* end of case 1,2 */
    else if (mode == (UBYTE) 3) {
        for (i = (UBYTE) 0; i < cnbr; i++) {
            kansuji((UBYTE)1, (astr[i] - (UBYTE)'0'), darea);
            darea += 2;
        }
    }                               /* end of case 3 */
    else if ((mode == (UBYTE) 4) ||
             (mode == (UBYTE) 5) ||
             (mode == (UBYTE) 6)) {
        for (i = (UBYTE) 0; astr[i] == '0'; i++);
        while (i < cnbr) {
            n = 0x824f + astr[i] - '0';
            *(darea++) = (UBYTE) (n >> 8);
            *(darea++) = (UBYTE) (n & 0x00ff);
            if (((cnbr - i) % 3 == (UBYTE) 1) &&
                (i != cnbr - (UBYTE) 1)) {      /* add ',' */
                if ((mode == (UBYTE) 4) || (mode == (UBYTE) 6)) {
                    *(darea++) = 0x81;
                    *(darea++) = 0x43;
                }
                else {
                    *(darea++) = 0x2c;
                }
            }
            i++;
        }
        if ((nset[0] | nset[1] | nset[2] | nset[3]) == (UWORD) 0) {
            *(darea++) = 0x82;
            *(darea++) = 0x4f;
        }
    }
    else if (mode == (UBYTE) 7) {
        for (i = (UBYTE) 0; i < cnbr; i++) {
            n = 0x824f + astr[i] - '0';
            *(darea++) = (UBYTE) (n >> 8);
            *(darea++) = (UBYTE) (n & 0x00ff);
        }
    }                               /* end of case 7 */
    *dsl = darea - dst;
    if (mode == (UBYTE) 6) {
        ascl = jistoasc(darea - *dsl, darea, darea - *dsl);
        *dsl = ascl;
    }
    else
        return ((UBYTE) ERR);
    return ((UBYTE) CONV);
}                                      /* end of program */

/*****************************************************************/
/*                                                               */
/* UBYTE kansuji(cmode,number,jisarea)                           */
/*   number-character ==> kanji code                             */
/*                                                               */
/*****************************************************************/
NONREC
UBYTE
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

kansuji(cmode, number, jisarea)
    UBYTE           cmode;             /* conversion mode */
    UBYTE           number;            /* number code */
    UBYTE          *jisarea;           /* jis-code area */
{
    UBYTE           xcode;

    static UWORD    kanji[20] =
    {
        /* 0      1      2      3      4      5      6      7   */
        0x815a, 0x88ea, 0x93f1, 0x8e4f, 0x8e6c, 0x8cdc, 0x985a, 0x8eb5,
        /* 8      9     10     100   1000                        */
        0x94aa, 0x8be3, 0x8f5c, 0x9553, 0x90e7,
        /* 1      2      3      10    10^4   10^8  10^12         */
        0x88eb, 0x93f3, 0x8e51, 0x8f45, 0x969c, 0x89ad, 0x929b
    };


    if (cmode == (UBYTE) 1)
        xcode = number;             /* normal style */
    else if (cmode == (UBYTE) 3)
        xcode = number + (UBYTE) 16;/* 10^4, 10^8, 10^12 */
    else if (cmode == (UBYTE) 2) {  /* old style */
        xcode = number;
        if ((number >= (UBYTE) 1) && (number <= (UBYTE) 3))
            xcode += 12;
        else if (number == (UBYTE) 10)
            xcode = (UBYTE) 16;
    }

    else
        return ((UBYTE) 0xff);

    if (xcode < (UBYTE) 20) {
        *(jisarea++) = (UBYTE) (kanji[xcode] >> 8);
        *(jisarea++) = (UBYTE) (kanji[xcode] & 0x00ff);
        return ((UBYTE) 0x00);
    }
    else
        return ((UBYTE) 0xff);
}                                      /* end of kansuji */
