/************************************************************************/
/*                                                                      */
/*      EGCKCNV : EGConvert Kana to kanji convert header file           */
/*                                                                      */
/*              Designed    by  I.Iwata         01/01/1985              */
/*              Coded       by  I.Iwata         01/01/1985              */
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
/************************************************************************/
#include "egcldml.h"
/************************************************************************/
/*          Constant  Common  Definition                                */
/************************************************************************/
#ifndef    __EGCKCNV__
#define    __EGCKCNV__
/*
**  system parameter
*/
#define     MAXRECORD     80           /* record                            */
#define     MAXKCNV      127           /* max analysis len                  */
#define     MAXTAIL       12           /* max okuri-gana length             */
#define     MAXCL         24           /* max clause length                 */
#define     MAXCLAUSE    255           /* max clause quantity               */
#define     MAXOUTBUF   3072           /* output buffer size                */
#define     MAXSLTVAL     64           /* self learning maximum             */
#ifdef  UNIX
#define     HEAPSIZE   (2048 * 2)      /* work area size                    */
#else
#define     HEAPSIZE    2048           /* work area size                    */
#endif
#define     PLOBLKL     4096           /* PLO size                          */
#define     DSOBLKL      128           /* DSO segment size                  */
#define     PLALIN        16           /* PL alingnment                     */
#define     DSOEXT        16           /* DSO max extention                 */
#define     SLTDFL         4           /* SLT element NO min value default  */
#define     GETIDXSIZE(g)  (sizeof(GIEREC) * (g + 1)) /* global index size  */
#define     MD_STR_OFFSET  8           /* main-dict structure ver offset    */
#define     UD_STR_OFFSET 54           /* user-dict structure ver offset    */
#define     MD_VER_OFFSET 11           /* main-dict management ver offset   */
#define     UD_VER_OFFSET 57           /* user-dict management ver offset   */
#define     EGCSYSVERNO 40000          /* egconvert system version NO       */
/*
**  system constant
*/
#define     PLBLKL      1024           /* max PL segment size               */
#define     DSBLKL      2048           /* DS segment size                   */
#define     UDBLKL       128           /* user dict segment size            */
#define     MAXCLAUS      16           /* clause table size                 */
#define     MAXPTYPE       8           /* clause type table size            */
#define     MAXJNODE      24           /* jiritsu-go node table size        */
#define     MAXFNODE       8           /* fuzoku-go node table size         */
#define     MAXLEVEL      12           /* number of words per clause        */
#define     MAXDWORD      12           /* jiritsu-go search table size      */
#define     MAXFWORD      12           /* fuzoku-go search table size       */
#define     MAXGWORD       8           /* gobi search table size            */
#define     MAXCODE       62           /* decode table size                 */
#define     MAXDSEG      512           /* max segment number of dict        */
#define     MAXPLE       384           /* max PL element per segment        */
#define     MAXPLOE      128           /* max PLO element per segment       */
#define     MAXWDTBL    (MAXDWORD * 2) /* max word table size               */
#define     MAXPLOF       16           /* max PLO format size               */
#define     MAXPLOL (MAXPLOE * MAXPLOF + 2) /* max PLO length per segment   */
#define     PLBLANK   0x0400           /* phonetic label blank number       */
#define     DSBLANK       63           /* description blank number          */
#define     ALPHID    0x0800           /* alphabet constat                  */
#define     DEFNUM         7           /* default number format             */
/*
** char definition (shift JIS) 
*/
/*  row  */
#ifdef  EGCVT
#define  JV1FR   0x81                  /* sjis lv1 start */
#define  JV1TO   0x9f                  /* sjis lv1 end   */
#define  JV2FR   0xe0                  /* sjis lv2 start */
#define  JV2TO   0xfc                  /* sjis lv2 end   */
/*  colum  */
#define  JH1FR   0x40                  /* sjis lv1 start */
#define  JH1TO   0x7e                  /* sjis lv1 end   */
#define  JH2FR   0x80                  /* sjis lv2 start */
#define  JH2TO   0x9e                  /* sjis lv2 end   */
#define  JH3FR   0x9f                  /* sjis lv3 start */
#define  JH3TO   0xfc                  /* sjis lv3 end   */
#endif

/*
**  charactor type
*/
#define     CHNON        0             /* no convert charactor  */
#define     CHNORMAL     1             /* normal kana moji      */
#define     CHKINSOK     2             /* special kana moji     */
#define     CHNUM        3             /* numerals              */
#define     CHALPHA      4             /* alphabets             */
#define     CHSUB        5             /* taigen kigou or quote */
#define     CHSEP        6             /* kugiri kigou          */
#define     CHJIS        7             /* kanji or gaiji        */
#define     CHEOT        8             /* shift code            */

/*
**  yomi type
*/
#define     NORMAL_YOMI     0x0001     /* '' - ''                         */
#define     EXTRA_YOMI      0x0010     /* alphabets, numerals, and ,marks   */
#define     ILLEGAL_YOMI    0x0100     /* convert triger                    */
#define     NEUTRAL_YOMI    0x1000     /* kinsoku kana moji                 */

/*
**   type of connect matrix row
*/
#define     RTJTG    1                 /* jiritsu-go taigen   */
#define     RTJRY    2                 /* jiritsu-go renyou   */
#define     RTFRY    3                 /* fuzoku-go renyou    */
#define     RTHPR    4                 /* high priority       */
#define     RTRPR    5                 /* row priority        */
#define     RTEND    6                 /* syuuryou,meirei     */

/*
**  hinsi code
*/
#define     HCFUTUU      1             /* futuu meisi           */
#define     HCJINMEI     2             /* jinmei meisi(sei)     */
#define     HCCHIMEI     3             /* timei meisi           */
#define     HCDAIMEI     4             /* dai meishi            */
#define     HCMEI        5             /* jinmei meishi(mei)    */
#define     HCKENKU      6             /* ken-mei ku-mei        */
#define     HCSUU        7             /* suusi                 */
#define     HCSETTOU     8             /* settouji              */
#define     HCSETSUBI    9             /* setsubiji             */
#define     HCJOSUU     10             /* josuuji               */
#define     HCNUM       11             /* numeral               */
#define     HCBUNSETSU  12             /* bunsetsu learn        */
#define     HCTANKAN    13             /* tankanji learn        */
#define     HCFUKU      14             /* fukusi                */
#define     HCSETUZOKU  15             /* setuzokusi            */
#define     HCRENTAI    16             /* rentaisi              */
#define     HCKEIYOU    17             /* keiyousi              */
#define     HCKEIDOU    18             /* keiyou dousi          */
#define     HCKAHEN     19             /* bunnrui fukanou       */
#define     HCSAHEN     20             /* sahne dousi           */
#define     HCZAHEN     21             /* zahen dousi           */
#define     HC1DAN      22             /* 1dan dousi            */
#define     HC5DANK     23             /* 5dan ka-gyou dousi    */
#define     HC5DANG     24             /* 5dan ga-gyou dousi    */
#define     HC5DANS     25             /* 5dan sa-gyou dousi    */
#define     HC5DANT     26             /* 5dan ta-gyou dousi    */
#define     HC5DANN     27             /* 5dan na-gyou dousi    */
#define     HC5DANB     28             /* 5dan ba-gyou dousi    */
#define     HC5DANM     29             /* 5dan ma-gyou dousi    */
#define     HC5DANR     30             /* 5dan ra-gyou dousi    */
#define     HC5DANW     31             /* 5dan wa-gyou dousi    */
/*PAGE*/
/*
**  clause status code
*/
#define     STATES      0              /* syuryou-kei or meirei-kei */
#define     STATEP      1              /* end of clase              */
#define     STATET      2              /* at end of text            */
#define     STATEX      0xFF           /* not hit clause            */

/*
** word type
*/
#ifndef CEOT
#define     CEOT        0X04           /* EOT code          */
#endif
#define     CSYUB       0X42           /* jiritsu-go    'B' */
#define     CSYUC       0X43           /* setsubi-go    'C' */
#define     CSYUE       0X45           /* kigou         'E' */
#define     CSYUF       0X46           /* jiritsu-go    'F' */
#define     CSYUG       0X47           /* fuzoku-go     'G' */
#define     CSYUH       0X48           /* muhenkan      'H' */
#define     CSYUJ       0X4a           /* kanji gaiji   'J' */
#define     ELMAGE      0xFD           /* CL learn          */
/*@@@ Next constant added at 01/09/1992 by H.Yanata,
      as enable conversion clause               @@@*/
#define     CSYUW       0X57           /* wakachi_go    'W' */
/*@@@*/

/*
** dictionary reference number (old version)
*/
#define     FREFW       1              /* kanji dict    */
#define     FREFL       2              /* yomi dict     */
#define     FREFG       3              /* grammer dict  */
/*
** grammer table header
*/
#define     FHCONECT     0             /* address of connect matrix         */
#define     FHTERM       1             /* address of clasue end table       */
#define     FHKEIYOU     2             /* address of keiyoushi gobi table   */
#define     FHKEIDOU     3             /* address of keiyou-doushi gobi table*/
#define     FHKAHEN      4             /* address of row table              */
#define     FHSAHEN      5             /* address of sahen-doushi gobi table */
#define     FHZAHEN      6             /* address of zahen-doushi gobi table */
#define     FH1DAN       7             /* address of 1dan-doushi gobi table  */
#define     FH5DANK      8             /* address of ka-5dandoushi gobi table*/
#define     FH5DANG      9             /* address of ga-5dandoushi gobi table*/
#define     FH5DANS     10             /* address of sa-5dandoushi gobi table*/
#define     FH5DANT     11             /* address of ta-5dandoushi gobi table*/
#define     FH5DANN     12             /* address of na-5dandoushi gobi table*/
#define     FH5DANB     13             /* address of ba-5dandoushi gobi table*/
#define     FH5DANM     14             /* address of ma-5dandoushi gobi table*/
#define     FH5DANR     15             /* address of ra-5dandoushi gobi table*/
#define     FH5DANW     16             /* address of wa-5dandoushi gobi table*/
#define     FHSETSUBI   17             /* address of setsubi-go table */
#define     FHFUZOKU    18             /* address of fuzoku-go table */
#define     FHLAST      19             /* total size of grammer dict */
#define     FHNUM       20             /* number of entry of header */
#define     FHSIZE      80
/*
** setsubi-go type
*/
#define     STJINMEI    1              /* jinmei setsuzoku */
#define     STCHIMEI    2              /* chimei setsuzoku */
#define     STFUKU      3              /* fukushi-ka */
#define     STSUU       4              /* josuu sahen-ka */
#define     STSAHEN     5              /* sahen-ka */
#define     STKEIDOU    6              /* keiyou-doushi-ka */
#define     STSETTOU    7              /* settou-ka */

/*******************************************************************/
/*       Macros                                                    */
/*******************************************************************/
/*
** 4 bytes memory set
*/
#define memset4(P,V)    { *((UBYTE *)(P))     \
                        = *((UBYTE *)(P) + 1) \
                        = *((UBYTE *)(P) + 2) \
                        = *((UBYTE *)(P) + 3) \
                        = V;\
                        }
/*
** wordid get & set
*/
#define GETWIDSEQ(q)    (UBYTE)((q) & 0x7F)
#define GETWIDSEG(g, q) ((q) & 0x80)? \
                        (UWORD)(0x0100 | (UWORD)(g)) : (UWORD)(g)
#define GETWIDREC(r)    (UWORD) ((r) & 0x01FF)
#define GETWIDDID(d)    (UBYTE) ((d) >> 12)
#define SETWIDSEQ(g, q) (UBYTE) ((q) | (((UWORD) (g) > 0x00FF) << 7))
#define SETWIDSEG(g)    (UBYTE) (g)
#define GETWIDVAL(g,q,r) (  ((ULONG)(GETWIDSEG(g, q)) << 22) \
                          | ((ULONG)(GETWIDREC(r)) << 11)    \
                          | ((ULONG)(GETWIDSEQ(q)))         )

/*PAGE*/
/************************************************************************/
/*          Structure  Common  Definition                               */
/************************************************************************/
/*
** word node 
*/
typedef struct nd {
    WORD            nddicid;           /* dictionary ID number */
    UBYTE           ndclass;           /* word type */
    UBYTE           ndhdlen;           /* settou-go length */
    UBYTE           ndlength;          /* word length */
    UBYTE           ndgobil;           /* gobi length */
    UBYTE           ndrow;             /* connect matrix row */
    UBYTE           ndhinshi;          /* hinshi code */
    UBYTE           ndterm;            /* bunnsetsu end flag */
    UWORD           ndsegno;           /* dict segment number */
    UWORD           ndmrecn;           /* main dict record number */
    UWORD           ndurecn;           /* user dict record number */
    UBYTE          *ndchain;           /* pointer to children */
}               NODEREC, *PTNODE;

/*
** node table
*/
typedef struct nt {
    UBYTE           nbrnode;           /* root node quautity */
    NODEREC         nodes[MAXJNODE];   /* root node table */
}               NODETBL, *PTNODTBL;

typedef struct fnt {
    UBYTE           fnbrnode;          /* fuzoku-go node quautity */
    NODEREC         fnodes[MAXFNODE];  /* fuzoku-go node table */
}               FNODETBL;

typedef struct wnt {
    UBYTE           wnbrnode;
    NODEREC         wnodes[1];
}               WNODETBL;

/*
** clause type record
*/
typedef struct cl {
    UBYTE           clstat;            /* clause status */
    UBYTE           clendpt;           /* clause length */
    UBYTE           clprtylen;         /* max two clause length */
    UBYTE           clprtygrm;         /* grammatical priority */
    UBYTE           clprtysel;         /* max 2nd clause jirits-go length */
    UWORD           clsetsuji;         /* settou & setsubi type */
    UBYTE           clrow;             /* connect matrix row */
    UBYTE           cllevel;           /* word quantity */
    PTNODE          cljwnd;            /* pointer to root node */
    PTNODTBL        clchain;           /* chain to 2nd clause */
    UBYTE           clpath[MAXLEVEL];  /* path in analysis tree */
}               CLAUSREC, *PTCLAUS;

/*
** clause format record
*/
typedef struct ct {
    WORD            ctdicid;           /* dic id */
    UBYTE           ctclass;           /* jiritsu-go type */
    UBYTE           ctalen;            /* settou-go length */
    UBYTE           ctjlen;            /* jiritsu-go length */
    UWORD           ctsegno;           /* jiritsu-go dict segment */
    UWORD           ctmrecn;           /* main dict record */
    UWORD           cturecn;           /* user dict record */
    UBYTE           ctmhbit[4];        /* main dict hinshi */
    UBYTE           ctuhbit[4];        /* user dict hinshi */
}               CLTYPE, *PTCLTYPE;

/*
** jiritsu-go setsubi-go word table
*/
typedef struct jr {
    WORD            jrdicid;           /* dictionary id number */
    UBYTE           jrwordl;           /* word length */
    UWORD           jrsegno;           /* segment number */
    UWORD           jrmrecn;           /* main dict record number */
    UWORD           jrurecn;           /* user dict record number */
    UBYTE           jrmhbit[4];        /* main dict hinshi */
    UBYTE           jruhbit[4];        /* user dict hinshi */
}               PLREC, *PPR;
/*PAGE*/
/*
** fuzoku-go search table
*/
typedef struct fr {
    UBYTE           frwordl;           /* word length */
    UBYTE           frcol;             /* connect matrix colum */
    UBYTE           frrow;             /* connect matrix row */
}               FWORDREC, *PTFWORD;


/*
** unload file format
*/
typedef struct ud {
    UBYTE           yomi[MAXJYOMI];    /* yomi */
    UBYTE           hinshi[4];         /* hinshi */
    UBYTE           kanji[MAXJKANJI];  /* kanji */
}               RECULD, *PUR;

/*
** main dictionary header structure
*/
typedef struct mh {
    UBYTE           mhsegq;            /* segment quantity LOW byte */
    UBYTE           mhsegqh;           /* segment quantity HI byte */
    UBYTE           mhploff[2];        /* PL offset */
    UBYTE           mhdsoff[2];        /* DS offset */
    UBYTE           mhgtoff[2];        /* GT offset */
    UBYTE           mhstrver[3];       /* version of structure */
    UBYTE           mhdicver[3];       /* version of main dict */
    UBYTE           mhusrver[2];       /* version of vender */
}               MDHD, *PMH;

/*
** hinshi-bunnrui-fukanou-go elemnt (dict99)
*/
typedef struct kr {
    UBYTE           krylen;            /* yomi length */
    UBYTE           kryomi[7];         /* yomi */
    UBYTE           krdummy;           /* not used */
    UBYTE           krrow;             /* connect matrix row */
}               KSREC, *PTKS;

/*
** gobi element (dict99)
*/
typedef struct gr {
    UBYTE           grylen;            /* yomi length */
    UBYTE           gryomi[3];         /* yomi */
    UBYTE           grdummy;           /* not used */
    UBYTE           grrow;             /* connect matrix row */
}               GOBIREC, *PTGOBI;

/*
** setsubi-go element (dict99)
*/
typedef struct sr {
    UBYTE           srylen;            /* yomi length */
    UBYTE           sryomi[7];         /* yomi */
    UBYTE           srkcode1[2];       /* 1st kanji code */
    UBYTE           srkcode2[2];       /* 2nd kanji code */
    UBYTE           srklen;            /* kanji length */
    UBYTE           srflag;            /* type */
}               SJREC, *PTSJ;

/*
** slt elememnt
*/
typedef struct _SLT {
    UWORD           RecNo;
    UWORD           SegNo;
    UBYTE           SeqNo;
    UBYTE           FmtNo;
}               DBSLTBL, *PDSLT;

#ifdef  DOS_EGC
#ifdef  PMSLT

typedef struct  _sltb {
    UWORD           seg;
    UWORD           busy;
    UBYTE           buf[MAXSLTVAL * 2];
}               SLTTBL;
#endif  /* PMSLT */
#endif  /* DOS_EGC */

#endif    /* __EGCKCNV__ */
