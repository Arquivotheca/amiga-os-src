/*
 * $Id: egcpart.h,v 3.4.1.1 91/07/08 14:14:34 katogi GM Locker: katogi $
 */
/************************************************************************/
/*                                                                      */
/*      EGCPART : EGConvert   utility parts header                      */
/*                                                                      */
/*              Designed    by  Y.Katogi        08/27/1990              */
/*              Coded       by  Y.Katogi        08/27/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                         --- NOTE ---                                 */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef        __EGCPART__
#define        __EGCPART__

typedef union _part {
    UBYTE           recval;            /* sequencial file record value */
    REG             st;
}               EGCPVAL;

typedef struct egcp {
    UBYTE           dirs[32];
    UBYTE           lfile[32];
    UBYTE           ufile[32];
    EGCPVAL         egcpval;
    UBYTE         **hext;
}               EGCPART, *PEGCPART;

#define         DIRECTORY     TRUE
#define         NODIRECTORY   FALSE
#define         SEQVER_1      (UBYTE)32
#define         SEQVER_2      (UBYTE)48
#define         SEQVER_3      (UBYTE)80

/*
*** Exchange type
*/
#define         STOM        1
#define         STOL        2
#define         MTOS        3
#define         MTOL        4
#define         LTOS        5
#define         LTOM        6

#endif    /* __EGCPART__ */
