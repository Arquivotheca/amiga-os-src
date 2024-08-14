/************************************************************************/
/*                                                                      */
/*      EGCLDML : EGConvert Low-level Data Manupulation Language        */
/*                                                                      */
/*              Designed    by  Y.Katogi        10/12/1990              */
/*              Coded       by  Y.Katogi        10/12/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef        __EGCLDML__
#define        __EGCLDML__

#define    MAXJYOMI      12           /* max jiritsugo yomi */
#define    MAXJKANJI     64           /* max jiritsugo kanji */
#define    CVT_SJIS      1
#define    CVT_JIS       2
#define    CVT_EUC       3
#define    DICNAMELEN    64

#define    _MAX_KSTR    1024
#define    _MAX_KSEP    (256 * 3)
#define    _MAX_CLSEP   256
#define    _MAX_WID		256

/*
** jiritsu-go record identification
*/
typedef struct wi {
    UBYTE           wisegno;           /* segment number */
    UBYTE           wiseqno;           /* kanji number */
    UWORD           wirecno;           /* yomi number */
}               WORDID, *PTWORDID;

typedef WORD (*PTRWFUNC)();

#endif    /* __EGCLDML__ */
