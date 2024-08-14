#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdmsc : miscelaneous subroutines for HDML                    */
/*                                                                      */
/*              Designed    by  I.Iwata         07/09/1986              */
/*              Coded       by  I.Iwata         07/09/1986              */
/*              Modified    by  K.Nakamura      10/18/1988              */
/*                                                                      */
/*      (C) Copyright 1985-88 ERGOSOFT Corp.                            */
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
/*      1        egcinst(iop,chp,cn,editpt)                             */
/*      2        egcdelt(iop,cn,editpt)                                 */
/*      3  UWORD egcpos(dir,str,iop)                                    */
/*      4  UWORD egcsel(cb)                                             */
/*      5  UWORD egcsbl(cb)                                             */
/*      6  UWORD egcoel(cb)                                             */
/*      7  UWORD egcobl(cb)                                             */
/*                                                                      */
/************************************************************************/

#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egberr.h"

#define    egccep(P)  P->acbebp+(P->acbcpn-1)*3
/*PAGE*/

/*-------------------------------------------------------------------*/
/*  4   egcsel : screen buffer length upto current element           */
/*-------------------------------------------------------------------*/
UWORD 
egcsel(cb)
    ACB             cb;                /* -> communication block       */
{
    register UWORD *p;                 /* -> element buffers            */
    register UWORD  len;               /* element length                */
    UWORD           i;                 /* element number                */

    len = 0;
    for (p = cb->acbebp + 1, i = 1; i < cb->acbcpn; p += 3, ++i) {
        len += *p;
    }
    return (len);
}

/*-------------------------------------------------------------------*/
/*  5   egcsbl : screen buffer length                                */
/*-------------------------------------------------------------------*/
UWORD 
egcsbl(cb)
    ACB             cb;                /* communication block          */
{
    register UWORD *p;                 /* -> element buffers            */
    register UWORD  len;               /* element length                */
    UWORD           i;                 /* element number                */

    len = 0;
    for (p = cb->acbebp + 1, i = 1; i <= cb->acbtpn; p += 3, ++i) {
        len += *p;
    }
    return (len);
}

/*-------------------------------------------------------------------*/
/*  6   egcoel : original buffer length upto current element         */
/*-------------------------------------------------------------------*/
UWORD 
egcoel(cb)
    ACB             cb;                /* ->communication block        */
{
    register UWORD *p;                 /* -> element buffers            */
    register UWORD  len;               /* element length                */
    UWORD           i;                 /* element number                */

    len = 0;
    for (p = cb->acbebp + 2, i = 1; i < cb->acbcpn; p += 3, ++i) {
        len += *p;
    }
    return (len);
}

/*-------------------------------------------------------------------*/
/*  7   egcobl : original buffer length                              */
/*-------------------------------------------------------------------*/
UWORD 
egcobl(cb)
    ACB             cb;                /* ->communication block        */
{
    register UWORD *p;                 /* -> element buffers            */
    register UWORD  len;               /* element length                */
    UWORD           i;                 /* element number                */

    len = 0;
    for (p = cb->acbebp + 2, i = 1; i <= cb->acbtpn; p += 3, ++i) {
        len += *p;
    }
    return (len);
}

/*-------------------------------------------------------------------*/
/*  1   egcint : insert char/string to screen buffer                 */
/*-------------------------------------------------------------------*/
egcinst(iop, chp, cn, cb)
    UWORD           iop;               /* insert point                         */
    UBYTE          *chp;               /* -> insert char                       */
    UWORD           cn;                /* insert length                        */
    ACB             cb;                /* -> communication block               */
{
    register UBYTE *p;                 /* insert point                         */
    register UWORD *cep;               /* -> element buffer of current element */
    UWORD           cel;               /* length of from current to end        */
    WORD            st;

    cep = egccep(cb) + 1;
    p = cb->acbsbp + egcsel(cb) + iop;
    if (cb->acbcpn == cb->acbtpn && *cep == iop) {
        *cep = iop + cn;
    }
    else {
        cel = egcsbl(cb) - egcsel(cb);
        if (cel > iop)
            egcmemmove(p + cn, p, cel - iop);
        *cep += cn;
    }
    if (cn > 0)
        egcmemmove(p, chp, cn);
}

/*-------------------------------------------------------------------*/
/*  2   egcdelt : delete char/string to editor                       */
/*-------------------------------------------------------------------*/
egcdelt(iop, cn, cb)
    UWORD           iop;               /* delete position from head of current
                                        * element */
    UWORD           cn;                /* delete length                                */
    ACB             cb;                /* -> communication block                       */
{
    register UBYTE *p;                 /* delete point                        */
    register UWORD *cep;               /* -> element buffer of current element   */
    UWORD           cel;               /* length of from current to end           */
    WORD            st;

    cep = egccep(cb) + 1;
    p = cb->acbsbp + egcsel(cb) + iop;
    if (cb->acbcpn == cb->acbtpn && *cep <= iop + cn) {
        *cep = iop;
    }
    else {
        cel = egcsbl(cb) - egcsel(cb);
        if (cel > iop + cn)
            egcmemmove(p, p + cn, cel - iop - cn);
        *cep -= cn;
    }
}

/*-------------------------------------------------------------------*/
/*  3   egcpos : positioning char boundary                           */
/*-------------------------------------------------------------------*/
UWORD 
egcpos(dir, str, iop)
    UBYTE           dir;               /* direction            */
    UBYTE          *str;               /* char string ptr      */
    UWORD           iop;               /* target               */
{
    register UWORD  i;                 /* counter for work  */
    register UBYTE  wchr;              /* char work             */
    UWORD           bp, ap;            /* before after      */

    i = bp = ap = 0;
    while (i < iop) {
        bp = i;
        wchr = *(str + (i++));
        if (((wchr >= JV1FR) && (wchr <= JV1TO))
            || ((wchr >= JV2FR) && (wchr <= JV2TO)))
            i++;
        ap = i;
    }
    if (dir == DRLEFT)
        i = bp;
    else
        i = ap;
    return (i);
}
/*PAGE*/

