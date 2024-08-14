/************************************************************************/
/*                                                                      */
/*      egcvt.h : definition of standard for HDML                       */
/*                                                                      */
/*                      Version 1.1                                     */
/*                                                                      */
/*              Designed    by  I.Iwata         07/07/1986              */
/*              Coded       by  I.Iwata         07/07/1986              */
/*              Modified    by  I.Iwata         02/10/1987              */
/*                                                                      */
/*      (C) Copyright 1985-87 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/

/*      Communication Block structure                            */

typedef struct tagCB {
    UBYTE    acbcmd;     /* HDML command code                    */
    UBYTE    acbdir;     /* HDML sub command code                */
    UWORD    acbrsp;     /* HDML errror code                     */

    UBYTE    acbsmd;     /* system mode                          */
    UBYTE    acbimd;     /* input mode                           */

    UWORD    acbtpn;     /* total phrase number                  */
    UWORD    acbcpn;     /* current phrase number                */
    UWORD    acbcrs;     /* cursor start position                */
    UWORD    acbcre;     /* cursor end position                  */
    UWORD    acbrmj;     /* romaji start position                */
    UWORD    acbswd;     /* screen width                         */
    UWORD    acbtpg;     /* total page number                    */
    UWORD    acbcpg;     /* current page number                  */
    UWORD    acbspg;     /* homonym page size                    */
    UWORD    acbchm;     /* current homonym number               */
    UWORD    acbhyl;     /* homonym yomi length                  */
    UWORD    acbmik;     /* max  input key                       */

    UWORD   *acbebp;     /* pointer to element buffer            */
    UBYTE   *acbsbp;     /* pointer to screen buffer             */
    UBYTE   *acbobp;     /* pointer to origenal buffer           */
    UBYTE   *acbhpp;     /* pointer to homonym page buffer       */
    UBYTE   *acbhep;     /* pointer to homonym element buffer    */
    UBYTE   *acbhbp;     /* pointer to homonym buffer            */
    WORDID  *acbwid;     /* pointer to homonym word id. buffer   */
    UBYTE   *acbhym;     /* pointer to homonym yomi buffer       */
}   CB,*ACB;

/*   acbsmd  definition  */                               
#define     CSMCVT      0x80    /* 1:convert 0:hommony  */
#define     CSMCEL      0x40    /* 1:convert 0:no-conv. */
#define     CSMAGE      0x20    /* age flag             */

/*   acbcmd  definition  */                               
#define     CMOVE       1       /* cursor move          */
#define     RVCVT       2       /* recverse convert     */
#define     TXCVT       3       /* kanatokanji convert  */
#define     CHCVT       4       /* change phrase        */
#define     PHFIX       5       /* phrase fix           */
#define     AGEHOM      6       /* age homonym          */
#define     CLEAR       7       /* clear control block  */
#define     CDCVT       8       /* code convert         */
#define     CANHOM      9       /* cancel homonym       */
#define     NEXTHOM     10      /* next homonym         */ 
#define     PREVHOM     11      /* previous homonym     */
#define     CLLEARN     12      /* cl learn             */

/*   acbimd  definition  */                               
#define     IMDJCD      0x80    /* JIS code     */
#define     IMDRMJ      0x40    /* romaji in    */
#define     IMDZEN      0x20    /* zen kaku     */
#define     IMDHRC      0x10    /* hira kana    */

/*   acbdir definition  */
#define     DRLEFT      0       /* left         */
#define     DRRIGHT     1       /* right        */
#define     DRDOWN      2       /* down         */
#define     DRUP        3       /* up           */
#define     DRHOME      4       /* home         */
#define     DRLAST      5       /* last         */

/*   acbrsp definition  */
#ifndef NORMAL
#define     NORMAL      0       /* normal return */
#endif
#ifndef ERROR
#define     ERROR       1       /* error  retrun */
#endif

/*   constant definition */
#define MAXCLN  256     /* max clauses                  */
#define MAXCHN  1024    /* max chracters                */
#define MAXHML  1024    /* max homonym buffer length    */
#define MAXHOM  256     /* max homonyms                 */
#define MAXHMP  64      /* max homonym pages            */
#define PAGEHOMS 9      /* max page homonyms            */

#ifdef  HASPROTO
#include "hdmproto.h"
#endif

