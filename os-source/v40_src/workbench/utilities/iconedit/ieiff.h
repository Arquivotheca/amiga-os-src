#ifndef IEIFF_H
#define IEIFF_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>

#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>

#include "iemain.h"

#if 0

/*****************************************************************************/

#define	MAXCOLORS 32

/* Work structure */
struct ILBM
{
    ULONG           ir_ModeID;		/* Display mode id */
    UWORD           ir_Width;		/* Width of image */
    UWORD           ir_Height;		/* Height of image */
    UWORD           ir_Depth;		/* Depth of image */
    struct BitMap   ir_BMap;		/* Bitmap */
    struct RastPort ir_RPort;		/* RastPort */
    WORD            ir_CRegs[MAXCOLORS];	/* Color table used by LoadRGB4() */
    WORD            ir_NumColors;		/* Number of colors in color table */
};
typedef struct ILBM * ILBMPtr;


#define	BPR(w)		((w) + 15 >> 4 << 1)	/* Bytes per row */

/*****************************************************************************/

ILBMPtr ReadILBM(BPTR drawer, STRPTR name, struct TagItem *attrs);
VOID FreeILBM(ILBMPtr ilbm);
ILBMPtr GetILBM(struct IFFHandle *iff);
BOOL PutILBM(struct IFFHandle *iff, ILBMPtr ilbm);
BOOL WriteILBM(BPTR drawer, STRPTR name, ILBMPtr ir, BOOL icon);
BOOL GetBMHD(struct IFFHandle *iff, struct BitMapHeader *bmhd);
BOOL PutBMHD(struct IFFHandle *iff, struct BitMapHeader *bmhd, ILBMPtr ir);
WORD GetColors(struct IFFHandle *iff, struct ColorRegister *cmap);
BOOL PutColors(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct ColorRegister *cmap);
VOID GetHotSpot(struct IFFHandle *iff, struct Point2D *grab, WORD, WORD);
BOOL PutHotSpot(struct IFFHandle *iff, struct Point2D *grab);
BOOL GetBody(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bm);
BOOL GetLine(struct IFFHandle *iff, UBYTE *buf, WORD wide, WORD deep, UBYTE cmptype);
BOOL PutBody(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bitmap);
VOID FillInILBM(WindowInfoPtr wi, DynamicImagePtr di, ILBMPtr ir);

#endif

/*****************************************************************************/

APTR newdtobject (STRPTR name, Tag tag1,...);
ULONG getdtattrs (Object * o, ULONG data,...);
ULONG setdtattrs (Object * o, ULONG data,...);

/*****************************************************************************/


#endif /*IEIFF_H */
