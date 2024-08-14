/*****************************************************************************
    ILBM.h
    
    Mitchell/Ware Systems           Version 1.00            26-Feb-89
    
    InterLeaved BitMap structures.
*****************************************************************************/

#define ROWBYTES(w)    (((w >> 4) + ((w & 0x0F) ? 1 : 0)) << 1)

/*  BMHD property
*/
#define BMHD    "BMHD"

typedef UBYTE Masking;
typedef UBYTE Compression;

typedef enum eMasking { mskNone, mskHasMask, mskHasTransparentColor, mskLasso } eMasking;
typedef enum eCompression { cmpNone, cmpByteRun1 } eCompression;

typedef struct BitMapHeader {
    UWORD w, h;
    UWORD x, y;
    UBYTE nPlanes;
    Masking masking;
    Compression compression;
    UBYTE pad1;
    UWORD transparentColor;
    UBYTE xAspect, yAspect;
    WORD pageWidth, pageHeight;
    } BitMapHeader;

/* CMAP
*/
#define CMAP    "CMAP"

typedef struct ColorRegister {
    UBYTE red, green, blue;
    } ColorRegister;

typedef ColorRegister *CMap;

typedef struct Color4Bits { /* AMIGA color format */
    unsigned pad1 :4, red :4, green :4, blue :4;
    } Color4Bits;

typedef UWORD Color4;

/* GRAB - hotspot of image
*/
#define GRAB    "GRAB"

typedef struct Point2D {
    WORD x, y;
    } Point2D;

/* DEST - scatter information
*/
#define DEST "DEST"

typedef struct DestMerge {
    UBYTE depth;
    UBYTE pad1;
    UWORD planePick;
    UWORD planeOnOff;
    UWORD planeMask;
    } DestMerge;


/* SPRT - sprite
*/
#define SPRT    "SPRT"

typedef UWORD SpritePrecedence;


/* CAMG - ViewPort mode
*/
#define CAMG    "CAMG"

typedef LONG VPMode;


/* CRNG - Color register range info
*/
#define CRNG    "CRNG"

typedef struct CRange {
    WORD pad1;
    WORD rate;
    WORD active;
    UBYTE low, high;
    } CRange;


/* CCRT - Color cycling range  & timing
*/
#define CCRT    "CCRT"

typedef struct CycleInfo    {
    WORD direction;
    UBYTE start, end;
    LONG seconds;
    LONG microseconds;
    WORD pad;
    } CycleInfo;

/* BODY - the image
*/
#define BODY    "BODY"
#
/*****************************************************************************

    miniTerm Macros

*****************************************************************************/

#define miniStamp       0xc0
#define miniInvert      0x30
#define miniCookie      0x60
#define miniSieve       0x80

