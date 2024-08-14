/************************************************************************/
/*                                                                      */
/*     EGCMIOS : EGConvert Memory menager I/O System                    */
/*                                                                      */
/*         Designed    by    I.Iwata       01/01/1985                   */
/*         Coded       by    I.Iwata       01/01/1985                   */
/*         Modified    by    H.Yanata      01/01/1992                   */
/*                                                                      */
/*     (C) Copyright 1985-1991 ERGOSOFT Corp.                           */
/*     All Rights Reserved                                              */
/*                                                                      */
/*                      --- NOTE ---                                    */
/*                                                                      */
/*     THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A      */
/*     TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES        */
/*     WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.      */
/*                                                                      */
/************************************************************************/
#ifdef UNIX
#include <stdio.h>
#endif
#include "edef.h"
#include "egcenv.h"
#include "egcdef.h"
#include "egcproto.h"

#ifdef DOS_EGC
#define  MAXUSR       20
#define  LCELL        128

/*****************************************************************************
*   Memory allocate size about 80000 words dictionary
*  (Self learning : 16, Additionaly words : 4000)
******************************************************************************
*   Dictionary structure     4
*   Expert dictionary       11 (jinmei-chimei dictionary)
*   Heap                    16
*   Grammatical table       50
*   Grobal index            15
*   SLT table                1:(Off memory) 340:(On memory)
*   BFR block               16
*   PLO block               17:(Off memory) 128:(On memory)
*   DSO block               16
*****************************************************************************/

#ifdef   EXP
#if     (defined(PMSLT) && defined(PMPLO))
#define     MAXIDX    150
#elif   (defined(PMSLT) && !defined(PMPLO))
#define     MAXIDX    330
#elif   (!defined(PMSLT) && defined(PMPLO))
#define     MAXIDX    476
#else
#define     MAXIDX    512       /* maximum allocate size */
#endif
#else
#if     (defined(PMSLT) && defined(PMPLO))
#define     MAXIDX    140
#elif   (defined(PMSLT) && !defined(PMPLO))
#define     MAXIDX    251
#elif   (!defined(PMSLT) && defined(PMPLO))
#define     MAXIDX    465
#else
#define     MAXIDX    512
#endif
#endif

typedef struct _usl {
    UWORD           urlen;             /* user request length   */
    UBYTE          *urptr;             /* user request pointer  */
}               ULIST;

ULIST           uq[MAXUSR];            /* user queue */
UBYTE           mi[MAXIDX];            /* free space table */
UBYTE           ta[(UWORD)MAXIDX * LCELL];    /* space pool */
#endif  /* DOS_EGC */
/*PAGE*/

#ifdef  DOS_EGC
/*****************************************************/
/* initmem : initialize memory                       */
/*****************************************************/
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif

initmem()
{
    register UWORD  i;

    for (i = 0; i < (UWORD) MAXUSR; i++) {
        uq[i].urlen = (UWORD) 0;
        uq[i].urptr = NULL;
    }
    for (i = 0; i < (UWORD) MAXIDX; i++)
        mi[i] = (UBYTE) 0;
    for (i = 0; i < (UWORD) MAXIDX * LCELL; i++)
        ta[i] = (UBYTE) 0;
}
#endif  /* DOS_EGC */
/*PAGE*/

/********************************************************************/
/* egcnewptr : allocate memory                                      */
/********************************************************************/
NONREC
VOID*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
egcnewptr(size)
    int   size;
{
#ifdef UNIX
    return ((VOID*)calloc(size, sizeof(UBYTE)));
#endif
#ifdef MAC
    return ((VOID*)malloc(size));
#endif

#ifdef DOS_EGC
    UWORD           ncell;
    UWORD           ui, ii, i, ni;

    if (size <= (UWORD) 0)
        return ((UBYTE *) 0);
    ncell = (size - 1) / LCELL + (UWORD) 1;
    i = ni = (UWORD) 0;
    ii = 0xFFFF;
    while ((i < (UWORD) MAXIDX) && (ni < ncell)) {
        if (mi[i] != (UBYTE) 0) {
            ii = 0xFFFF;
            ni = (UWORD) 0;
        }
        else {
            if (ii == 0xFFFF)
                ii = i;
            ni++;
        }
        i++;
    }
    if ((ni < ncell) || (ii == 0xFFFF)) {
        return ((UBYTE *) 0);
    }
    ui = (UWORD) 0;
    while (ui < (UWORD) MAXUSR) {
        if (uq[ui].urlen == (UWORD) 0)
            break;
        ui++;
    }
    if (ui >= (UWORD) MAXUSR)
        return ((UBYTE *) 0);
    for (ni = (UWORD) 0; ni < ncell; ni++)
        mi[ii + ni] = 0xff;
    uq[ui].urlen = size;
    uq[ui].urptr = (UBYTE *) (ta + ii * LCELL);

    return (uq[ui].urptr);
#endif
}
/*PAGE*/

/********************************************************************/
/* egcfreeptr : deallocate memory                                   */
/********************************************************************/
NONREC
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
egcfreeptr(ptr)
    UBYTE          *ptr;
{
#ifdef UNIX
    (VOID) free((char *) ptr);
    ptr = NULL;
    return ((WORD) 0);
#endif
#ifdef MAC
    (VOID) free((char *) ptr);
    ptr = NULL;
    return ((WORD) 0);
#endif

#ifdef DOS_EGC
    UWORD           ncell;
    UWORD           ui, ii, ni;

    ui = (UWORD) 0;
    while (ui < (UWORD) MAXUSR) {
        if ((uq[ui].urlen != (UWORD) 0) && (uq[ui].urptr == ptr))
            break;
        ui++;
    }
    if (ui >= (UWORD) MAXUSR)
        return ((WORD) - 1);
    ncell = (uq[ui].urlen - (UWORD) 1) / LCELL + (UWORD) 1;
    ii = (UWORD) (ptr - ta) / LCELL;
    for (ni = (UWORD) 0; ni < ncell; ni++)
        mi[ii + ni] = (UBYTE) 0;
    uq[ui].urlen = (UWORD) 0;
    uq[ui].urptr = NULL;
    return ((WORD) 0);
#endif
}
/*PAGE*/
/*-------------------------------------------------------------------*/
/*      egcmemmove                                                   */
/*      copies n characters from s2 to s1. returns s1.               */
/*-------------------------------------------------------------------*/
NONREC
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
egcmemmove(dest, src, cnt)
    UBYTE          *dest;
    UBYTE          *src;
    UWORD           cnt;
{
    UBYTE          *p, *q;

    if (cnt) {
        if (src < dest) {
            p = src + cnt - 1;
            q = dest + cnt - 1;
            while (cnt--)
                *(q--) = *(p--);
        }
        else {
            p = src;
            q = dest;
            while (cnt--)
                *(q++) = *(p++);
        }
    }
    return (dest);
}

/*-------------------------------------------------------------------*/
/*      sets the first n characters in s to c                        */
/*-------------------------------------------------------------------*/
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
egcmemset(s, c, n)
    UBYTE          *s, c;
    UWORD           n;
{
    register UBYTE *p;
    register UWORD  i;

    p = s;
    for (i = n; i--; *(p++) = c);

}
/*PAGE*/

/*--------------------------------------------------------------------*/
/*      string compare up to n characters                             */
/*        (remove the "unsigned" for signed collating set)            */
/*--------------------------------------------------------------------*/
WORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
memncmp(s, t, n)
    UBYTE   *s, *t;
    UWORD   n;
{
    register UBYTE *p, *q;
    int             i;

    p = s;
    q = t;

    for (i = n; i-- && (*p == *q); q++)
        if (!*p++)
            return ((WORD) 0);
    if (i < 0)
        return ((WORD) 0);
    if (*p < *q)
        return ((WORD) - 1);
    return ((WORD) 1);
}
/*--------------------------------------------------------------------*/
/*      setint                                                        */
/*--------------------------------------------------------------------*/
VOID
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
setint(p, n)
    UBYTE*  p;
    UWORD   n;
{
    p[0] = (UBYTE) (n >> 8);
    p[1] = (UBYTE) (n & 0x00FF);
}
/*--------------------------------------------------------------------*/
/*      getint                                                        */
/*--------------------------------------------------------------------*/
UWORD
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
getint(p)
    UBYTE*  p;
{
    return (((UWORD) (*p) << 8) + (UWORD) (*(p + 1)));
}
