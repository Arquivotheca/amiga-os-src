/********************************************************************************
**   sdraw2.h
**
**      Purpose: CDTV screen saver header file
**
**      Date: Mar 4, 1992
**      Copyright (C) 1992, 1993 Commodore Amiga Inc.  All rights reserved.
**
********************************************************************************/
#define HALF(a)      ((*(a)+(a)[1])>>1)
#define FIX(x)       (((LONG)(x)) << 7)
#define FIXH(x)      (((LONG)((*(x)+(x)[1])>>1)) << 7)

#define SDRAW_INIT      0
#define SDRAW_CLEAR     1

#define WIDTH           (320)
#define HEIGHT          (210)
#define DEPTH           (1)     /* Screen Depth */

/*
 * Inputhandler data structure
 */
struct ihd {
        struct Task     *sigtask;
        ULONG           blanksig, unblanksig;
        ULONG           lastsec;        /* Time of last interesting event */
        ULONG           savedelay;      /*  # of seconds until blank.     */
        ULONG           cursec;         /* line time counter */
};


#define MAXLINES     (20)
#define MAXPOINTS    (20)
#define ANIMINTERVAL (60)       /* Change animation every 60 seconds */

struct OldInfo          /* Referenced in splines.a */
{
    LONG oldx;
    LONG oldy;
    LONG oldwidth;
    LONG oldptr;
    WORD oldcol;
    struct GfxBase *gfxbase;
};

struct SGlobal {
    struct RastPort rastPort;
    struct BitMap bitMap;
    struct RasInfo rasInfo;

    struct box {
        WORD x[MAXPOINTS];
        WORD y[MAXPOINTS];
    } store[MAXLINES];

    WORD  maxpoints;
    struct box *ptr;
    struct box *eptr;
    WORD numlines;

    WORD mdelta;
    WORD maxlines;

    WORD closed;
    unsigned char realfunc[14];
    char namefunc[20];


    LONG seed;
    WORD dx[MAXPOINTS],dy[MAXPOINTS];   /* Difference X,Y       */
    WORD ox[MAXPOINTS],oy[MAXPOINTS];   /* Old X,Y              */
    WORD nx[MAXPOINTS],ny[MAXPOINTS];   /* New X,Y              */

    WORD dr,dg,db;              /* Difference R,G,B     */
    WORD or,og,ob;              /* Old R,G,B            */
    WORD nr,ng,nb;              /* New R,G,B            */
    ULONG anim_secs,lastanim_secs;
    LONG oldx;          /* splines wants a pointer to this ... */
    LONG oldy;
    LONG oldwidth;
    LONG oldptr;
    WORD oldcol;
    struct GfxBase *gfxbase;
};

struct SaverGlobals {
        struct Interrupt        handlerstuff;
        struct ihd              idata;
        struct PlayerPrefsBase  *PPBase;
        struct IOStdReq         *idreq;
        struct BitMap           *bm;
        struct View             *view;
        struct View             **altlv;
        UWORD                   *oldcmap, *cmap, *black, *rcolors;
        WORD                    fade, fadecnt;
        WORD                    fadetimer, fadeadd;
        short                   minx, maxx, miny, maxy;
        BYTE                    sprites, blanksigbit, unblanksigbit;
        int                     SaverType;
        struct SGlobal          *g2;
};

