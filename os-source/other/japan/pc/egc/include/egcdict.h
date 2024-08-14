/************************************************************************/
/*                                                                      */
/*      EGCDICT : EGConvert Dictionary Maintenance Library Header File  */
/*                                                                      */
/*              Designed    by  I.Iwata         02/25/1987              */
/*              Coded       by  I.Iwata         02/25/1987              */
/*                                                                      */
/*      (C) Copyright 1987-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef     __EGCDICT__
#define     __EGCDICT__
#include    <stdio.h>
#include	"EGCdef.h"

/*
*** Dictionary Reading Record Definitions
*/
typedef struct tagDICR {
    UBYTE           smode;             /* search mode 0:sequential 1:direct */
    UBYTE           dmode;             /* dictionary mode 0:main 1:user     */
    UBYTE           yomi[MAXJYOMI];    /* yomi strings                      */
    UBYTE           yomil;             /* yomi length                       */
    UBYTE           hinshi[4];         /* hinshi strings                    */
    UWORD           recq;              /* quantity of PL/PLO record         */
    UBYTE          *plp;               /* PL/PLO record pointer             */
    UBYTE          *dsp;               /* DS/DSO record pointer             */
    UWORD           segno;             /* segment no (GI element number)    */
    UWORD           recno;             /* PLO record number                 */
    UWORD           dbks;              /* tan-kan to be returned next time  */
    UWORD           dbkq;              /* quantity of tan-kanji             */
    UBYTE           dbws;              /* homonym to be returned next time  */
    UBYTE           dbwq;              /* quantity of homonym               */
    UWORD           segq;              /* quantity of segment               */
}               DICR, *DICRP;

/*
*** Dictionary Making Record Definitions
*/
typedef struct tagDICM {
    EGCFILE         lf;                /* label file discripter         */
    EGCFILE         wf;                /* work file discripter          */
    UBYTE           dupll;             /* yomi duplicate length         */
    UBYTE           preyomi[MAXJYOMI]; /* preceding yomi                */
    UBYTE           preyomil;          /* preceding yomi length         */
    UBYTE           prekanji[MAXJKANJI];/* preceding kanji               */
    UBYTE           prekanjil;         /* preceding kanji length        */
    UBYTE           lhinshi[4];        /* label record hinshi           */
    UWORD           segno;             /* segment number                */
    UWORD           lblen;             /* label total length ;paragraph */
    UBYTE          *lblk;              /* --> label block               */
    UWORD           lblkl;             /* label block length            */
    UWORD           lbcnt;             /* label block word count        */
    UBYTE          *lrec;              /* --> label record              */
    UWORD           lrecl;             /* label record length           */
    UWORD           lrcnt;             /* label record word count       */
    UBYTE          *wblk;              /* --> word block                */
    UWORD           wblkl;             /* word block index              */
    UBYTE          *wrec;              /* --> word record               */
    UWORD           wrecl;             /* word record length            */
    UWORD           wconcl;            /* word record conclusive length */
    UWORD           homcnt;            /* homonyms count                */
    UBYTE          *tkbuf;             /* --> tan-kanji buffer          */
    UWORD           tkcnt;             /* tan-kanji count               */
    BOOL            first;             /* first time flag               */
}               DICM, *DICMP;

#endif    /* __EGCDICT__ */
