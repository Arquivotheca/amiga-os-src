/*
 * $Id: egced.h,v 3.4.1.1 91/06/25 21:07:18 katogi GM Locker: katogi $
 */
/*
*
*     egced.h  :   header for sequencial dictionary edit tool
*
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by        T.Koide   1992.09.04
*
*/

#if !defined    __EGCED__
#define     __EGCED__

#include    <stdio.h>

#ifndef        UNIX
#include    <dos.h>
#endif

/*--------------------------------------------------------------*/
/*    defines                                                   */
/*--------------------------------------------------------------*/
typedef        int                BOOL;
typedef        char               BYTE;
typedef        unsigned char      UBYTE;
typedef        short              WORD;
typedef        unsigned short     UWORD;
typedef        long               LONG;
typedef        unsigned long      ULONG;
#define        REG                register int
#define        VOID               void
#define        TRUE               (1 == 1)
#define        FALSE               !TRUE
#ifndef    UNIX
typedef struct _CRect {
    BYTE    top, left, bottom, right;
} CRect, *CRectPtr;
#endif

#ifndef        UNIX
/*--------------------------------------------------------------*/
/*    defines VRAM                                              */
/*--------------------------------------------------------------*/
#define       _VSEG_ADDR       0xA0000000L       // VRAM segment for text
#define       _ASEG_ADDR       0xA2000000L       // VRAM segment for attr
#define        WGDC            while(! (inp(0xa0) & 0x4))

/*--------------------------------------------------------------*/
/*    defines color code & attribute                            */
/*--------------------------------------------------------------*/
#define        EGLWHITE        0xE1
#define        EGLGREEN        0x81
#define        EGLRED          0x41
#define        EGLBLUE         0x21
#define        EGLBLACK        0x01
#define        EGLGRAY         EGLWHITE|EGLBLACK
#define        EGLYELLOW       EGLRED|EGLGREEN
#define        EGLMAGENTA      EGLRED|EGLBLUE
#define        EGLCYAN         EGLBLUE|EGLGREEN
#define        EGLREVERSE      0x05
#endif		   /*	not	UNIX	*/

/*--------------------------------------------------------------*/
/*    enumerations data masking type                            */
/*--------------------------------------------------------------*/
#define        NORMAL            0
#define        MEISI1            1
#define        JINMEI1           2
#define        CHIMEI1           3
#define        MEISI2            4
#define        JINMEI2           5
#define        CHIMEI2           6
#define        SUUSI             7
#define        SETTOU            8
#define        SETUBI            9
#define        JYOSUU            10
#define        FUKUSI            11
#define        SETUZOKU          12
#define        RENTAI            13
#define        KEIYOU            14
#define        KEIDOU            15
#define        SAHEN             16
#define        ZAHEN             17
#define        ICHIDAN           18
#define        KAGYO             19
#define        GAGYO             20
#define        SAGYO             21
#define        TAGYO             22
#define        NAGYO             23
#define        BAGYO             24
#define        MAGYO             25
#define        RAGYO             26
#define        WAGYO             27
#define        BUNFU             28

/*--------------------------------------------------------------*/
/*    defines macro                                             */
/*--------------------------------------------------------------*/
#define        CHKMASK(mode, rec)    ((mode) & (rec))
#ifdef	UNIX
#define        iskana(ch)        (0xa6<=(ch) && (ch)<=0xfd)? TRUE : FALSE
#define        iskanji(c1,c2)    (0x81<=(c1) && (c1)<=0xea\
                            && ((c1==0x81)? ((0x40<=c2)? TRUE:FALSE) : TRUE)\
                            && ((c1==0xea)? ((c2<=0x9e)? TRUE:FALSE) : TRUE))?\
                            TRUE : FALSE
#endif
/*-----------------------------------------------------------------*/
/*		charactor type											   */
/*-----------------------------------------------------------------*/
#define     CHNON        0             /* no convert charactor  */
#define     CHNORMAL     1             /* normal kana moji      */
#define     CHKINSOK     2             /* special kana moji     */
#define     CHNUM        3             /* numerals              */
#define     CHALPHA      4             /* alphabets             */
#define     CHSUB        5             /* taigen kigou or quote */
#define     CHSEP        6             /* kugiri kigou          */
#define     CHJIS        7             /* kanji or gaiji        */
#define     CHEOT        8             /* shift code            */

/*------------------------------------------------------------------------*/
/*		yomi type														  */
/*------------------------------------------------------------------------*/
#define     NORMAL_YOMI     0x0001     /* '' - ''                         */
#define     EXTRA_YOMI      0x0010     /* alphabets, numerals, and ,marks   */
#define     ILLEGAL_YOMI    0x0100     /* convert triger                    */
#define     NEUTRAL_YOMI    0x1000     /* kinsoku kana moji                 */

/*--------------------------------------------------------------*/
/*    defines value                                             */
/*--------------------------------------------------------------*/
#define        YOMI            1        /* yomi code                    */
#define        HINSHI          2        /* hinshi code                    */
#define        KANJI           3        /* kanji code                    */
#define        SUUJI           10       /* suuji code (hinshi edit)    */
#define        BUNSETU         11       /* bunsetu code (hinshi edit)    */
#define        MISIYOU         12       /* misiyou code (hinshi edit)    */
#define        DICSIZ32        32       /* size of record (32 byte)    */
#define        DICSIZ48        48       /* size of record (48 byte)    */
#define		   DICSIZ80		   80		/* size of record (80 byte)    */
#define        YOMISIZ         12       /* maximum yomi length            */
#define        HINSIZ          4        /* hinshi length                */
#define        KNJSIZ32        16       /* maximum kanji length (32 byte) */
#define        KNJSIZ48        30       /* maximum kanji length (48 byte) */
#define		   KNJSIZ80		   64       /* maximum kanji length (80 byte) */
#define        ADDMAX          500      /* maximum number of adding    */
#define        NO_TARGET       -1       /* err code (search)            */
#define        CSR_LEFT        0        /* cursor move range of display   */
#define        CSR_RIGHT       1        /* cursor move range of display   */
#define        CSR_TOP         0        /* cursor move range of display   */
#define        CSR_BOTTOM      13       /* cursor move range of display   */
#define        HINSHI_LEFT     0        /* cursor move range of hinshi edit*/
#define        HINSHI_TOP      0        /* cursor move range of hinshi edit*/
#define        HINSHI_RIGHT    0        /* cursor move range of hinshi edit */
#define        HINSHI_BOTTOM   31       /* cursor move range of hinshi edit */
#define        DISPMAX         14       /* maximum number of data on display*/
#define        PAGEMAX         50       /* maximum number of data on page    */
#define        INDEX_MIN       0        /* minimum index number        */
#define        NO_CSR_MOVE     (UBYTE)0 /* display mode                */
#define        ILLEGAL         0        /* illegal key type            */
#define        ABORT           0        /* abort command                */
#define        CSR_MOVE        1        /* key type(cursor move)        */
#define        COMMAND         2        /* key type(command mode)        */
#define        QUIT            3        /* command mode                */
#define        EDIT            4        /* command mode                */
#define        DELETE          0        /* data moving mode            */
#define        ENTRY           1        /* data moving mode            */
#define        YES             1        /* support flag (do it)        */
#define        NO              0        /* support flag (stop)            */
#define        MASK            0x18     /* data masking code(CTRL-X)    */
#define        STOP_PROCESS    0        /* print & quit mode (stop)    */
#define        SELECT_ALL      1        /* print mode (all)            */
#define        SELECT_RANGE    2        /* print mode (range)            */
#define        QUIT_AFTER_SAVE 1        /* quit mode (after save)        */
#define        QUIT_IGNORE     2        /* quit mode (ignore)            */
#define        EXIT_SUCCESS    0        /* exit code (normal)            */
#define        EXIT_ERROR      1        /* exit code (error)            */
#define        DISPHIN         0x1b     /* hinshi vertical display        */
#define        PAGE            0x0c     /* page feed code                */
#ifdef    UNIX
#define        DELIMITER       '/'
#else
#define        DELIMITER       '\\'
#define        Int23Nbr        0x23
#endif

/*--------------------------------------------------------------*/
/*    defines data control block                                */
/*--------------------------------------------------------------*/
typedef struct{
    UBYTE    ubYomi[12];                 /*    yomi data buffer            */
    UBYTE    ubHinshi[4];                /*    hinshi data buffer          */
    UBYTE    ubKanji[64];                /*    kanji data buffer           */
}     sREC48;                            /*    data structure (32 & 48 & 80) */

#ifndef        UNIX
union REGS    iRegs, oRegs;              /*    MS-DOS system call          */
#endif

typedef struct {
    FILE        *fp;                     /*    read & write file pointer    */
    UBYTE        ubFile[80];             /*    current file name            */
    sREC48     **sRecBuf;                /*    data buffer                  */
    WORD         wIndexMax;              /*    maximam index number         */
    WORD         wIndexNum;              /*    current index number         */
    WORD         wDispNum;               /*    top number on display        */
	WORD		*IndexNum;
	WORD		 IndexMax;
    int          iCsrX, iCsrY;           /*    point of cursor              */
    int          iDicLen;                /*    data length                  */
    int          iKnjLen;                /*    kanji length                 */
    int          iReadOnly;              /*    read only flag               */
    int          VertHin;                /*    display hinshi vertical flag */
    BYTE         ubMask[2];              /*    data masking flag            */
}    sDATASET;

/*--------------------------------------------------------------*/
/*    function prototype                                        */
/*--------------------------------------------------------------*/
int            main(
#ifdef    COMPLETE
int, char **
#endif
);
VOID        vfCpyrt(
#ifdef    COMPLETE
VOID
#endif
);
BOOL        bfMemInit(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfSetMonitor(
#ifdef    COMPLETE
VOID
#endif
);
BOOL        bfGetIndex(
#ifdef    COMPLETE
sDATASET *
#endif
);
BOOL        bfReadRec(
#ifdef    COMPLETE
sDATASET *
#endif
);
WORD        wfIndexSearch(
#ifdef    COMPLETE
const   sDATASET *, UBYTE *
#endif
);
BOOL        bfWriteRec(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfMakeBox(
#ifdef    COMPLETE
int, int, int, int
#endif
);
VOID        vfMakeRvsBox(
#ifdef    COMPLETE
int, int, int, int, int
#endif
);
VOID        vfRemoveBox(
#ifdef    COMPLETE
VOID
#endif
);
VOID        vfCsrMove(
#ifdef    COMPLETE
sDATASET *, UBYTE
#endif
);
VOID        vfDisplay(
#ifdef    COMPLETE
sDATASET *, UBYTE
#endif
);
VOID        vfBoxDisplay(
#ifdef    COMPLETE
const   sDATASET *
#endif
);
VOID        vfDispHin(
#ifdef    COMPLETE
const sDATASET *
#endif
);
BOOL        bfFileInit(
#ifdef    COMPLETE
int, char **, sDATASET *
#endif
);
VOID        vfKey(
#ifdef    COMPLETE
UBYTE *
#endif
);
VOID        vfGetKey(
#ifdef    COMPLETE
UBYTE *
#endif
);
int            ifSelect(
#ifdef    COMPLETE
sDATASET *, UBYTE
#endif
);
VOID        vfFind(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfDelete(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfGetKey(
#ifdef    COMPLETE
UBYTE *
#endif
);
VOID        vfDataMove(
#ifdef    COMPLETE
sDATASET *, int
#endif
);
VOID        vfEntry(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfHinshiEdit(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfModify(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfSort(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfChange(
#ifdef    COMPLETE
sDATASET *
#endif
);
int            ifQuit(
#ifdef    COMPLETE
sDATASET *
#endif
);
int            ifSprt(
#ifdef    COMPLETE
VOID
#endif
);
VOID        vfPrintCtrl(
#ifdef    COMPLETE
sDATASET *
#endif
);
VOID        vfPrintMrg(
#ifdef    COMPLETE
sDATASET *, WORD, WORD
#endif
);
VOID        vfPrintScript(
#ifdef    COMPLETE
sREC48 *, UBYTE *, WORD
#endif
);
VOID        vfPageHead(
#ifdef    COMPLETE
sDATASET *, int
#endif
);
VOID        vfPageFeed(
#ifdef    COMPLETE
VOID
#endif
);
VOID        vfInitHin(
#ifdef    COMPLETE
UBYTE *
#endif
);
VOID        vfChkHin(
#ifdef    COMPLETE
sREC48 *, UBYTE *
#endif
);
VOID        vfAddHinType(
#ifdef    COMPLETE
UBYTE *, UBYTE, UBYTE, UBYTE, UBYTE, UBYTE
#endif
);
VOID        vfInsert(
#ifdef    COMPLETE
sDATASET *
#endif
);
WORD        wfHomonym(
#ifdef    COMPLETE
sDATASET *, WORD *, WORD *
#endif
);
BOOL		YStrChk(
#ifdef	  COMPLETE
UBYTE *, UWORD
#endif
);
UWORD		YTypeChk(
#ifdef	  COMPLETE
UBYTE *
#endif
);
BOOL        bfStrChk(
#ifdef    COMPLETE
UBYTE *, int, int
#endif
);
VOID        vfBufClr(
#ifdef    COMPLETE
UBYTE *, int
#endif
);
VOID        vfShowMenu(
#ifdef    COMPLETE
VOID
#endif
);
VOID        vfGetString(
#ifdef    COMPLETE
UBYTE*, int
#endif
);
VOID        vfDispReadOnly(
#ifdef    COMPLETE
VOID
#endif
);
UBYTE        ubDataMask(
#ifdef    COMPLETE
BYTE *
#endif
);
VOID        vfGetIndex(
#ifdef   COMPLETE
sDATASET *
#endif
);
VOID        vfDispVertHin(
#ifdef    COMPLETE
const   sDATASET *
#endif
);
WORD        egcsqchk(
#ifdef    COMPLETE
UBYTE    *
#endif
);
WORD        wfMemCmp(
#ifdef    COMPLETE
UBYTE*, UBYTE*, WORD
#endif
);

#ifndef        UNIX
VOID        pokew(UWORD, UWORD, UWORD);
UWORD        peek(UWORD, UWORD);
VOID        SetCRect(CRect *, WORD, WORD, WORD, WORD);
VOID        ClearText(CRect *, UWORD);
VOID        SetBakColor(WORD);
VOID        DrawText(WORD, WORD, UBYTE *, WORD, UWORD);
VOID        DrawChr(WORD, WORD, UWORD, UWORD);
VOID        DrawKnj(WORD, WORD, UWORD, UWORD);
VOID        ScrollCRect(CRect *, WORD);
VOID(interrupt    far *A_INT23)();
VOID        vfAutoScrl(sDATASET *, UBYTE);
UBYTE        inkey(VOID);
VOID        SetStopKey(VOID);
VOID        RcvStopKey(VOID);
int            waitgch(VOID);
int            nonwaitgch(VOID);
#endif        /*    not UNIX    */

#endif    /*ifndef __EGCED__*/

