/************************************************************************/
/*                                                                      */
/*    EDEF    : ERGOSOFT Standard Definitions Header File               */
/*                                                                      */
/*            Designed   by      I.Iwata     04/14/1987                 */
/*            Coded      by      I.Iwata     04/14/1987                 */
/*                                                                      */
/*     (C) Copyright 1987-1991 ERGOSOFT Corp.                           */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef        __EDEF__
#define        __EDEF__

/*
*** Data Type Definitions
*/

#define VOID    void
typedef int     BOOL;
typedef char    BYTE;
typedef unsigned char UBYTE;
typedef struct {
    BYTE            hbyte;
    BYTE            lbyte;
}               DBYTE;

typedef short   WORD;
typedef unsigned short UWORD;

typedef long    LONG;
typedef unsigned long ULONG;

typedef int      INT;
typedef unsigned int UINT;

typedef UBYTE  *Ptr;
typedef Ptr    *Handle;

/*
*** Boolean Value
*/

#ifndef TRUE
#define TRUE    (1 == 1)
#endif

#ifndef FALSE
#define FALSE   !TRUE
#endif

#ifndef NORMAL
#define NORMAL  FALSE
#endif

#ifndef ERROR
#define ERROR   TRUE
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef NIL
#define NIL     0
#endif

#ifndef REG
#define REG         int
#endif

#endif    /* __EDEF__ */
