/************************************************************************/
/*                                                                      */
/*      EGCUDEP : EGConvert System defination depend header file        */
/*                                                                      */
/*              Designed    by  Y.Katogi        06/18/1990              */
/*              Coded       by  Y.Katogi        06/18/1990              */
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
#ifndef        __EGCUDEP__
#define        __EGCUDEP__

#include    "egcdef.h"

#ifdef UNIX
#define     MAXAPENDIC     3           /* max appending dictionary */
#define     MAXOPENDIC    10           /* max opening dictionary */
#endif

#ifdef MAC
#define     MAXAPENDIC     2           /* max appending dictionary */
#define     MAXOPENDIC     2           /* max opening dictionary */
#endif

#ifdef DOS_EGC
#define     MAXAPENDIC     2           /* max appending dictionary */
#define     MAXOPENDIC     2           /* max opening dictionary */
#endif

#define     MAXPATH       64           /* Dictionary path length */
#define     EGCONVVERLEN   3           /* egconvert version length */
#define     EGCSYSVERLEN   3           /* system version length */

#define     EXPID          1           /* expert dictionary ID number */

/*
*** Dictionary unique number
*/
#define     MAINDIC     0x0001
#define     USERDIC     0x0010
#define     EXPTDIC     0x0100

/*
*** Dictionary active flag constant
*/
#define     ACTIVE      FALSE
#define     SUSPEND     TRUE

#ifdef  NDG
typedef int     TIME;
#else
typedef long    TIME;
#endif

/*
** global index element structure
*/
typedef struct ie {
    UBYTE           ieyomi[4];         /* yomi */
    UBYTE           ieplp[2];          /* pointer to PL segment */
    UBYTE           ieplop[2];         /* pointer to PLO segment */
}               GIEREC, *PIE;

/*
** user dictionary header structure
*/
typedef struct hd {
    UBYTE           udidxs;            /* GI start segment          */
    UBYTE           udidxl;            /* GI length                 */
    UBYTE           udslts;            /* SLT start segment         */
    UBYTE           udsltl;            /* SLT length                */
    UBYTE           udplos;            /* PLO start segment         */
    UBYTE           udplol;            /* PLO length                */
    UBYTE           uddsos;            /* DSO start segment         */
    UBYTE           udsegq;            /* segment quantity          */
    UBYTE           udexts[2];         /* next free segment         */
    UBYTE           uduidx[44];        /* upper index               */
    UBYTE           udstrver[3];       /* version of structure      */
    UBYTE           uddicver[3];       /* version of user dict      */
    UBYTE           udusrver[2];       /* version of vender         */
    UBYTE           ueidxs[2];         /* extend GI start segment   */
    UBYTE           ueidxl[2];         /* extend GI length          */
    UBYTE           ueslts[2];         /* extend SLT start segment  */
    UBYTE           uesltl[2];         /* extend SLT length         */
    UBYTE           ueplos[2];         /* extend PLO start segment  */
    UBYTE           ueplol[2];         /* extend PLO length         */
    UBYTE           uedsos[2];         /* extend DSO start segment  */
    UBYTE           uesegq[2];         /* extend segment            */
    UBYTE           udmaxslt[2];       /* Max SLT value             */
    UBYTE           udmaxdso;          /* maximum DSO block         */
    UBYTE           busyNum;           /* busy number format        */
    UBYTE           cvtHch;            /* hankaku conversion flag   */
    UBYTE           cvtHal;            /* hankaku conversion flag   */
    /* end of header */
    PIE             udidxp;            /* pointer to GI buffer      */
    UBYTE          *udsltp;            /* pointer to SLT buffer     */
    UBYTE          *udplop;            /* pointer to PLO buffer     */
    UBYTE          *uddsop;            /* pointer to DSO buffer     */
}               UDDH, *PUD;

/*
** dictionary status
*/
typedef struct udict_info {
#ifndef MAC
    EGCFILE         fp;                /* User dictionary discripter*/
    UBYTE           path[MAXPATH];     /* User dictionary pathname  */
#else
    WORD         	fp;                /* User dictionary discripter*/
    Str63           path;              /* User dictionary pathname  */
#endif
    UBYTE           strver[EGCSYSVERLEN];
    UBYTE           dicver[EGCSYSVERLEN];
    UDDH            hbuhdr;            /* user dictionary header handle */
    UWORD           hbsegq;            /* segment quantity          */
#if  (defined(UNIX) || defined(MAC))
    UBYTE          *plp;               /* PL pointer                */
#endif
#ifdef  PMPLO
    LONG            plosof;            /* previous PLO start offsef */
    LONG            ploeof;            /* previous PLO end offsef   */
#endif
#ifndef MAC
    UBYTE*          bfrp;              /* buffering pointer         */
    LONG            bfrsof;            /* previous start offset     */
    LONG            bfreof;            /* previous end offset       */
    WORD            bfrid;             /* previous dictionary id    */
    UBYTE           bfrmod;            /* previous buffering mode   */
#endif
#ifdef	MAC
    UBYTE          *iobp;              /* I/O buffer pointer */
#endif
    LONG            ploff;             /* PL offsets                */
    LONG            dsoff;             /* DS offsets                */
    LONG            gtoff;             /* GT offsets                */
#ifdef UNIX
    BYTE            dictflag;          /* User dictionary flags     */
    TIME            last;              /* Last access time          */
    struct udict_info *prev;
    struct udict_info *next;
    unsigned long   inode;             /* User dictionary inode     */
#endif
    UBYTE           reconv;            /* Reconvert flag            */
    UBYTE           suspended;         /* suspended flag            */
}               UDICT_INFO, *PTR_UDICT;

typedef UDICT_INFO MDICT_INFO;
typedef PTR_UDICT PTR_MDICT;

/*
 * Expert Dictionary Information
 */

typedef struct _DICIDTBL {             /* dictionary infomations table */
    WORD            idno;              /* dictionary ID number */
    PTR_MDICT       pds;               /* pointer to infomations table */
}               DICIDTBL;

extern DICIDTBL dicstat[MAXOPENDIC];
extern DICIDTBL dicprty[MAXAPENDIC];

#endif /* __EGCUDEP__ */
