#ifndef SKETCHPAD_H
#define SKETCHPAD_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

#include "dynamicimages.h"


/*****************************************************************************/


struct NewSketchPad
{
    struct Window *Window;	/* Parent Window */
    VOID *Visual;		/* Visual Information */
    UBYTE MagX;			/* Magnification Factor 1-16 */
    UBYTE MagY;
    SHORT LeftEdge;		/* Gadget Placement */
    SHORT TopEdge;
    SHORT Width;		/* SketchPad size */
    SHORT Height;
    SHORT Depth;
    ULONG Flags;
    UBYTE APen;			/* Beginning Foreground Pen */
    UBYTE BPen;			/* Beginning Background Pen */
    UBYTE Draw_Mode;		/* Beginning Draw Mode */
};
typedef struct NewSketchPad * NewSketchPadPtr;

struct SketchPad
{
    struct Window *Window;	/* Destination Window */
    VOID *Visual;		/* Visual Information */
    UBYTE MagX;			/* Horizontal Magnification */
    UBYTE MagY;			/* Vertical Magnification */
    SHORT LeftEdge;		/* Dest. Left Edge */
    SHORT TopEdge;		/* Dest. Top Edge */
    SHORT Width;		/* Original Width */
    SHORT Height;		/* Original Height */
    SHORT Depth;		/* Depth */
    ULONG Flags;		/* SketchPad Flags */
    UBYTE APen;			/* ForeGround Pen */
    UBYTE BPen;			/* BackGround Pen */
    UBYTE OPen;			/* Outline Pen */
    UBYTE Draw_Mode;		/* Drawing Mode */
    struct RastPort *Drp;	/* Dest. RastPort (Window->RPort) */
    struct RastPort Wrp;	/* Working RastPort */
    struct BitMap Wbm;		/* Working BitMap */
    BOOL Grid;			/* Use Grid? */
    struct Gadget Gad;		/* Sketch Gadget */

    /* The Repeat Image is the life-size image
     * (the non-magnified version).
     */
    struct DynamicImage *Adi;	/* Current Repeat Image */
    SHORT RLeftEdge;		/* Repeat LeftEdge */
    SHORT RTopEdge;		/* Repeat TopEdge */

    struct DynamicImage Undo;	/* Undo Area */
    struct DynamicImage Brush;	/* Brushes to BOB */
    WORD cursor[10];		/* Cursor Imagery */
    UBYTE Flood_Mode;		/* Flood-Fill mode */
    USHORT *AreaPtrn;		/* Area Fill Pattern */
    USHORT AreaPtSz;		/* Size of Area Fill Pattern */
    USHORT LinePtrn;		/* Line Pattern */
    struct Gadget *Active_Tool;	/* For mutual exclusion */

    struct DynamicImage Backup;	/* Image for cursor drawing */
};
typedef struct SketchPad * SketchPadPtr;

extern USHORT SPDither[];
extern USHORT SPVerBar[];

/* REQUIRED Button IDs, when setting up SketchPad accessories */
#define SPSKETCHPAD_ID	3000
#define SPCLEAR_ID	3100
#define SPUNDO_ID	3200
#define SPTOOLS_ID	3300
#define SPSCROLL_ID	3400
#define	SPREPEAT_ID	3500
#define SPPALETTE_ID	3600

/* SketchPad Application Flags */
#define SP_GRID		0x00000001
#define	SP_TOOLS	0x00000002

/* IDCMP's need for the sketch pad */
#define SKETCHPAD (GADGETUP|GADGETDOWN|INTUITICKS|MOUSEMOVE|MOUSEBUTTONS)

#define	SPSetAPen(a,b)	(a->APen = b)
#define	SPSetBPen(a,b)	(a->BPen = b)
#define	SPSetDrMd(a,b)	(a->Draw_Mode = b)
#define	SPSetFlood(a,b) (a->Flood_Mode = b)
#define SPSetAfPt(a,b,c) {a->AreaPtrn = ((USHORT *)b); a->AreaPtSz = c; }


/*****************************************************************************/


SketchPadPtr OpenSketchPad(NewSketchPadPtr nsp);
VOID CloseSketchPad(SketchPadPtr sp);

VOID SPSetRepeat(SketchPadPtr sp, DynamicImagePtr di, SHORT, SHORT);
VOID SPDraw(SketchPadPtr sp, struct IntuiMessage * rmsg);
VOID SPRefresh(SketchPadPtr sp);
VOID SPClear(SketchPadPtr sp);
VOID SPUndo(SketchPadPtr sp);
VOID SPSaveToUndo(SketchPadPtr sp);


/*****************************************************************************/


#endif	/* SKETCHPAD_H */
