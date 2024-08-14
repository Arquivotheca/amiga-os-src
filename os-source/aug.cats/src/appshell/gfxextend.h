#ifndef GRAPHICS_GFXEXTEND_H
#define GRAPHICS_GFXEXTEND_H
/*
** $Filename: gfxextend.h $
** $Release: 1.3 $
**
** Copyright (c) 1989 Commodore-Amiga, Inc.
**
** Executables based on this information may be used in software
** for Commodore Amiga computers.  All other rights reserved.
** This information is provided "as is"; no warranties are made.  All
** use is at your own risk. No liability or responsibility is assumed.
**
*/

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_GELS_H
#include <graphics/gels.h>
#endif

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

/* these data structures are used by the functions in animtools.c to
** allow for an easier interface to the animation system.
*/

/* data structure to hold information for a new vsprite.
** note that:
**     NEWVSPRITE myNVS;
** is equivalent to:
**     struct newVSprite myNVS;
*/
typedef struct newVSprite
{
    WORD *nvs_Image;		/* image data for the vsprite */
    WORD *nvs_ColorSet;		/* color array for the vsprite */
    SHORT nvs_WordWidth;	/* width in words */
    SHORT nvs_LineHeight;	/* height in lines */
    SHORT nvs_ImageDepth;	/* depth of the image */
    SHORT nvs_X;		/* initial x position */
    SHORT nvs_Y;		/* initial y position */
    SHORT nvs_Flags;		/* vsprite flags */
    USHORT nvs_HitMask;		/* Hit mask. */
    USHORT nvs_MeMask;		/* Me mask. */
} NEWVSPRITE;

/* data structure to hold information for a new bob.
** note that:
**     NEWBOB myNBob;
** is equivalent to:
**     struct newBob myNBob;
*/
typedef struct newBob
{
    WORD *nb_Image;		/* image data for the bob */
    SHORT nb_WordWidth;		/* width in words */
    SHORT nb_LineHeight;	/* height in lines */
    SHORT nb_ImageDepth;	/* depth of the image */
    SHORT nb_PlanePick;		/* planes that get image data */
    SHORT nb_PlaneOnOff;	/* unused planes to turn on */
    SHORT nb_BFlags;		/* bob flags */
    SHORT nb_DBuf;		/* 1=double buf, 0=not */
    SHORT nb_RasDepth;		/* depth of the raster */
    SHORT nb_X;			/* initial x position */
    SHORT nb_Y;			/* initial y position */
    USHORT nb_HitMask;		/* Hit mask. */
    USHORT nb_MeMask;		/* Me mask. */
} NEWBOB;

/* data structure to hold information for a new bob constructed from an image.
** note that:
**     NEWIMAGEBOB myNIBob;
** is equivalent to:
**     struct newImageBob myNIBob;
*/
typedef struct newImageBob
{
    struct Image *nib_Image;	/* image for the bob */
    SHORT nib_BFlags;		/* bob flags */
    SHORT nib_DBuf;		/* 1=double buf, 0=not */
    SHORT nib_RasDepth;		/* depth of the raster */
    SHORT nib_X;		/* initial x position */
    SHORT nib_Y;		/* initial y position */
    USHORT nib_HitMask;		/* Hit mask. */
    USHORT nib_MeMask;		/* Me mask. */
} NEWIMAGEBOB;

/* data structure to hold information for a new animation component.
** note that:
**     NEWANIMCOMP myNAC;
** is equivalent to:
**     struct newAnimComp myNAC;
*/
typedef struct newAnimComp
{
    WORD (*nac_Routine) ();	/* routine called when Comp is displayed. */
    SHORT nac_Xt;		/* initial delta offset position. */
    SHORT nac_Yt;		/* initial delta offset position. */
    SHORT nac_Time;		/* Initial Timer value. */
    SHORT nac_CFlags;		/* Flags for the Component. */
} NEWANIMCOMP;

/* data structure to hold information for a new animation sequence.
** note that:
**     NEWANIMSEQ myNAS;
** is equivalent to:
**     struct newAnimSeq myNAS;
*/
typedef struct newAnimSeq
{
    struct AnimOb *nas_HeadOb;	/* common Head of Object. */
    WORD *nas_Images;		/* array of Comp image data */
    SHORT *nas_Xt;		/* arrays of initial offsets. */
    SHORT *nas_Yt;		/* arrays of initial offsets. */
    SHORT *nas_Times;		/* array of Initial Timer value. */
     WORD (**nas_Routines) ();	/* Array of fns called when comp drawn */
    SHORT nas_CFlags;		/* Flags for the Component. */
    SHORT nas_Count;		/* Num Comps in seq (= arrays size) */
    SHORT nas_SingleImage;	/* one (or count) images. */
} NEWANIMSEQ;

/* Data structure to hold text rendering information
 */
struct RenderInfo
{
    SHORT BackPen;		/* Pen for rendering background */
    SHORT Highlight;		/* Pen for highlighting */
    SHORT Shadow;		/* Pen for shadows */
    SHORT TextPen;		/* Pen for rendering text */

    SHORT MenuText;		/* Pen for rendering menu text */
    SHORT MenuBack;		/* Pen for rendering menu backdrop */

    struct TextAttr *TextAttr;	/* font attribute */
    struct TextFont *Font;	/* font to use */

    SHORT TopBorder;		/* Where the top of the window will be */
    SHORT ScrType;		/* Screen Type (0=LORES,1=HIRES) */
};

#define	SIZE_RI	sizeof(struct RenderInfo)

#define AREA_SIZE 200

struct DynamicImage
{
    struct Image di_image;	/* Image structure to draw/display */
    struct BitMap di_bmap;	/* Image's BitMap */
    struct RastPort di_rport;	/* Image's RastPort */
    ULONG di_size;		/* Size of ImageData */

    struct TmpRas di_tmpras;	/* Flood Fill TmpRas */
    UBYTE *di_workspace;	/* Flood Fill workspace */

    struct AreaInfo di_area;	/* Area Fill AreaInfo */
    WORD di_array[AREA_SIZE];	/* Area Fill workspace */

    struct Layer_Info *di_li;	/* Layer_Info for layer manipulations */
    struct Layer *di_layer;	/* Layer structure */
    ULONG Flags;		/* Application Flags -- see defines */
};

#define	SIZE_DI	sizeof(struct DynamicImage)

/* Application Flags.
 * Used to indicate what portions of the DynamicImage need
 * to be initialized.
 */
#define DI_FILL		(1L<<0)
#define	DI_LAYER	(1L<<1)

/* OR this into your GadgetID to get an autoknob, then call InitSlider
 * after each ModifyProp (InitSlider will automatically free the
 * old slider).
 */
#define	AUTO3DKNOB 0x8000

struct Slider3D
{
    struct RastPort s_rp;	/* RastPort for rendering 3D knob */
    struct BitMap s_bm;		/* BitMap for rendering 3D knob */
    USHORT * s_idata;		/* Data for 3D knob */
    SHORT s_size;		/* Size of data for 3D knob */
    SHORT s_val;		/* Value of slider */
    SHORT s_bv1[10];		/* 3D border vectors */
    SHORT s_bv2[10];
    struct Border s_b1;		/* 3D border structures */
    struct Border s_b;
};

struct Button3D
{
    SHORT bb_v1[40];		/* 3D border vectors */
    SHORT bb_v2[40];
    struct Border bb_o1;	/* 3D border structures */
    struct Border bb_o;
    struct Border bb_i1;
    struct Border bb_i;
};

#define	EXTEND_ID 621213L

/* Extended Image structure for handling dynamic images */
struct EImage
{
    struct Image *ei_Image;	/* Embedded Image structure(s) */
    ULONG ei_EID;		/* Extended structure ID */
    ULONG ei_ISize;		/* Size of Image structure(s) */
    ULONG ei_DSize;		/* Size of ImageData */
    struct BitMap *ei_BM;	/* Image's BitMap */
    struct RastPort *ei_RP;	/* Image's RastPort */
    struct TmpRas *ei_TmpRas;	/* Flood Fill TmpRas */
    UBYTE *ei_WorkSpace;	/* Flood Fill workspace */
    struct AreaInfo *ei_Area;	/* Area Fill AreaInfo */
    WORD *ei_Array;		/* Area Fill workspace */
    struct Layer_Info *ei_LI;	/* Layer_Info for layer manipulations */
    struct Layer *ei_Layer;	/* Layer structure */
    ULONG ei_Flags;		/* Application Flags -- see defines */
};

#define	SIZE_EI	sizeof(struct EImage)

/* Extended Border structure for handling dynamic borders */
struct EBorder
{
    struct Border *eb_Border;	/* Embedded Border structure(s) */
    ULONG eb_EID;		/* Extended structure ID */
    WORD *eb_BVectors;		/* Border Vectors */
    ULONG eb_BSize;		/* Size of Border */
    ULONG eb_VSize;		/* Size of Vectors */
    WORD eb_Style;		/* Style of Border */
};

#define	SIZE_EB sizeof(struct EBorder)

/* Extended IntuiText structure for handling dynamic text */
struct EIText
{
    struct IntuiText *eit_IntuiText;	/* Embedded IntuiText structure(s) */
    ULONG eit_EID;		/* Extended structure ID */
    UBYTE *eit_Text;		/* Text */
    ULONG eit_ISize;		/* Size of IntuiText */
    ULONG eit_TSize;		/* Size of text */
};

#define	SIZE_EIT sizeof(struct EIText)

struct SliderKnob
{
    struct EImage *sk_Image;	/* Slider Knob */
    struct EBorder *sk_Border;	/* 3D Border */
    WORD sk_MinValue;		/* Minimum Value */
    WORD sk_Value;		/* Value of slider */
    WORD sk_MaxValue;		/* Maximum Value */
};

#define	SIZE_KNOB sizeof(struct SliderKnob)

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
    struct DynamicImage Backup;	/* Cursor image */
    WORD cursor[10];		/* Cursor Imagery */
    UBYTE Flood_Mode;		/* Flood-Fill mode */
    USHORT *AreaPtrn;		/* Area Fill Pattern */
    USHORT AreaPtSz;		/* Size of Area Fill Pattern */
    USHORT LinePtrn;		/* Line Pattern */
    struct Gadget *Active_Tool;	/* For mutual exclusion */
    BOOL acursor;		/* Cursor active? */
};

extern USHORT SPDither[];

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
#define	SP_UNDO		0x00000002
#define	SP_TOOLS	0x00000004
#define	SP_CLEAR	0x00000008
#define	SP_PALETTE	0x00000010

/* IDCMP's need for the sketch pad */
#define SKETCHPAD (GADGETUP|GADGETDOWN|MOUSEMOVE|MOUSEBUTTONS|VANILLAKEY)

#define	SPSetAPen(a,b)	(a->APen = b)
#define	SPSetBPen(a,b)	(a->BPen = b)
#define	SPSetDrMd(a,b)	(a->Draw_Mode = b)
#define	SPSetFlood(a,b) (a->Flood_Mode = b)
#define SPSetAfPt(a,b,c) {a->AreaPtrn = ((USHORT *)b); a->AreaPtSz = c; }

/* common defines */
#define	SIZE_DI	sizeof(struct DynamicImage)
#define	SIZE_GAD sizeof(struct Gadget)
#define	MEMORY	MEMF_PUBLIC|MEMF_CLEAR
#define	ISODD(n) (n % 2)
#define	ISEVEN(n) ((n % 2) ? 0 : 1)

#define	EB_SGL		0
#define	EB_SGL_IN	1
#define	EB_SGL_OUT	2
#define	EB_DBL_IN	3
#define	EB_DBL_OUT	4
#define	EB_CRV_IN	5
#define	EB_CRV_OUT	6
#define	EB_SGL1_IN	7
#define	EB_SGL1_OUT	8

#define	MPUBLIC (MEMF_PUBLIC | MEMF_CLEAR)

#endif
