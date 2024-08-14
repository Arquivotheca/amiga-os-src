#ifndef LIBRARIES_SPECIALFX_H
#define LIBRARIES_SPECIALFX_H
/*
**	$Id: specialfx.h,v 39.17 93/09/14 18:54:15 spence Exp $
**
**	Special Effects header file
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

    #ifndef EXEC_TYPES_H
    #include <exec/types.h>
    #endif

    #ifndef GRAPHICS_VIEW_H
    #include <graphics/view.h>
    #endif


/* Tags for InstallFXA() */
#define SFX_InstallEffect 0x80001000	/* ti_Data points to an FXHandle
					 * obtained from AllocFX().
					 */
#define SFX_InstallErrorCode 0x80001001	/* ti_Data is a pointer to a ULONG
					 * which will be filled with an error
					 * code from InstallFXA().
					 */
#define SFX_DoubleBuffer 0x80001002	/* ti_Data is TRUE to enable
					 * copperlist double buffering.
					 * default == FALSE.
					 */

/* Tags for the various SpecialFX features. (V40) */

#define SFX_ColorControl 0x80000001	/* ti_Data points to a
					 * struct ColorControl.
					 */
#define SFX_LineControl	0x80000002	/* ti_Data points to a
					 * struct LineControl.
					 */
#define SFX_IntControl 0x80000003	/* ti_Data points to a
					 * struct InterruptControl.
					 */
#define SFX_FineVideoControl 0x80000004	/* ti_Data points to a
					 * struct FineVideoControl.
					 */
#define SFX_SpriteControl 0x80000005	/* ti_Data points to a
					 * struct SpriteControl.
					 */


/* ColorControl - fill this structure for SFX_ColorControl, to modify the
 * RGB value of any pen at any line in the ViewPort.
 */

struct ColorControl
{
    LONG cc_Pen;		/* Pen number to change */
    WORD cc_Horizontal;		/* X Wait position - ignored for V40 */
    WORD cc_Line;		/* Y Wait position */
    ULONG cc_Red;		/* 32 bit red value */
    ULONG cc_Green;		/* 32 bit green value */
    ULONG cc_Blue;		/* 32 bit blue value */
    UWORD cc_Flags;
    UWORD cc_Pad;
};

#define CCB_MODIFY 0		/* When used with AnimateFXA(), this colour
				 * entry is modified to the RGB value.
				 */
#define CCF_MODIFY (1 << CCB_MODIFY)
#define CCB_RESTORE 1		/* if set, then restore the pen number to its
				 * original value. This needs to be set only
				 * once for the pen number, and is used by
				 * InstallFXA()
				 */
#define CCF_RESTORE (1 << CCB_RESTORE)


/* LineControl - fill this structure for the SFX_LineControl, to scroll
 * individual, or groups of lines in a ViewPort independantly from the rest, and
 * to show parts of the offscreen bitmap in different parts of the ViewPort.
 */

struct LineControl
{
    struct RasInfo *lc_RasInfo;	/* Pointer to an initialised RasInfo structure.
    				 * Set the RxOffset for the scroll value (+ve
    				 * is to the left, -ve to the right) of this area,
    				 * and the RyOffset to the line number in the
    				 * ViewPort's BitMap to start the effect with.
    				 *
    				 * If lc_RasInfo->Next is not NULL, then
    				 * lc_RasInfo will control the even bitplanes, and
    				 * lc_RasInfo->Next the odd bitplanes.
    				 *
    				 * Note that the RasInfo->BitMap->Planes are ignored.
    				 * All operations are on the ViewPort's RasInfo(s)
    				 * BitMap(s) Plane(s).
    				 *
    				 */
    UWORD lc_Line;		/* Start the effect at this line */
    UWORD lc_Count;		/* Effect is over this many lines */
    UWORD lc_Flags;		/* See below */
    WORD lc_SkipRateEven;	/* Number of lines to skip after displaying */
    WORD lc_SkipRateOdd;	/* a line. Default = 1.*/ 
    UWORD lc_Pad;
};

/* A note about the lc_SkipRate... paramters.
 * The first line to be displayed at line lc_Line of the display is line
 * lc_RasInfo->RyOffset of the ViewPort->RasInfo->BitMap.
 * The next line to be displayed at line (lc_Line + 1) is
 * (lc_RasInfo->RyOffset + lc_SkipRateEven).
 * So, if lc_SkipRateEven =
 * 1, contiguous lines are displayed (this is the default).
 * 2, every other line is displayed.
 * 0, the first line is repeated, giving a 'flow' effect.
 * -1, the image is displayed upside down.
 */

#define LCB_MODIFY 0		/* Set for the changes to be made by
				 * AnimateFX().
				 */
#define LCF_MODIFY (1 << LCB_MODIFY)


/* InterruptControl - fill this structure for the SFX_IntControl to cause
 * interrupts at the specified line in a ViewPort.
 */

struct InterruptControl
{
    UWORD inc_Line;		/* Cause the interrupt at the start of this
    				 * line.
    				 */
    UWORD inc_Flags;		/* See below */
};

#define INCB_SET 0		/* When used with AnimateFXA(), the interrupt
				 * at this line is turned on.
				 */
#define INCF_SET (1 << INCB_SET)
#define INCB_RESET 1		/* When used with AnimateFXA(), the interrupt
				 * at this line is turned off.
				 */
#define INCF_RESET (1 << INCB_RESET)
/* NB - setting both INCF_SET and INCF_RESET is undefined. */


/* FineVideoControl - fill this structure for the SFX_FineVideoControl to
 * operate graphics.library's VideoControl() features on a line-by-line basis
 * rather than a ViewPort basis.
 */
struct FineVideoControl
{
    struct TagItem *fvc_TagList;
    				/* A pointer to a taglist of VideoControl()
    				 * tags. Only a subset of VC() tags will be
    				 * supported, listed below.
    				 */
    UWORD fvc_Line;		/* line number the VideoControl() should
				 * start on.
				 */
    WORD fvc_Count;		/* VideoControl() lasts for this many lines,
    				 * and then the previous settings are restored.
    				 * If == -1, then maintain this to the end of
    				 * the ViewPort.
    				 */
    UWORD fvc_Flags;		/* See below */
    UWORD fvc_Pad;
};

#define FVCB_MODIFY 0		/* When used with AnimateFXA(), the video
				 * features are replaced for this range of
				 * lines with the new features of the
				 * fvc_TagList.
				 */
#define FVCF_MODIFY (1 << FVCB_MODIFY)

/* Here is a list of VideoControl tags supported as of V40:
 *
 * VTAG_CHROMAKEY_CLR		
 * VTAG_CHROMAKEY_SET		
 * VTAG_BITPLANEKEY_CLR		
 * VTAG_BITPLANEKEY_SET		
 * VTAG_BORDERBLANK_CLR		
 * VTAG_BORDERBLANK_SET		
 * VTAG_BORDERNOTRANS_CLR		
 * VTAG_BORDERNOTRANS_SET		
 * VTAG_CHROMA_PEN_CLR		
 * VTAG_CHROMA_PEN_SET		
 * VTAG_CHROMA_PLANE_SET		
 * VTAG_PF1_BASE_SET		
 * VTAG_PF2_BASE_SET		
 * VTAG_SPEVEN_BASE_SET		
 * VTAG_SPODD_BASE_SET		
 * VTAG_BORDERSPRITE_SET		
 * VTAG_BORDERSPRITE_CLR		
 * VTAG_SPRITERESN_SET		
 * VTAG_PF1_TO_SPRITEPRI_SET	
 * VTAG_PF2_TO_SPRITEPRI_SET	
 *
 * Also, the following tags are recognised by SpecialFX.library, although they
 * are not recognised by graphics.library's VideoControl():
 *
 * VTAG_SFX_DEPTH_SET
 * VTAG_SFX_HAM_SET
 * VTAG_SFX_EHB_SET
 * VTAG_SFX_NORM_SET
 * VTAG_SFX_MAKEDPF
 * VTAG_SFX_PF1PRI
 * VTAG_SFX_PF2PRI
 */

#define VTAG_SFX_DEPTH_SET	0xc0000000	/* Set the depth of the
						 * display.
						 */
#define VTAG_SFX_HAM_SET	0xc0000001	/* enable HAM - only useful if
						 * the depth is 6 or 8 bitplanes
						 */
#define VTAG_SFX_EHB_SET	0xc0000002	/* enable EHB - only works if 
						 * the depth is 6 bitplanes.
						 */
#define VTAG_SFX_NORM_SET	0xc0000003	/* clear HAM, EHB and DPF bits. */
#define VTAG_SFX_DPF_SET	0xc0000004	/* enable DualPlayfield */
#define VTAG_SFX_PF1PRI		0xc0000005	/* give Playfield 1 priority
						 * (default)
						 */
#define VTAG_SFX_PF2PRI		0xc0000006	/* give Playfield 2 priority */


/* SpriteControl - fill this structure for the SFX_SpriteControl. This allows
 * you to move and change sprites in a certain range of lines of the 
 * ViewPort, and to reuse the same sprite in a different ViewPort range.
 */
struct SpriteControl
{
    struct ExtSprite *spc_ExtSprite;
    				/* Pointer to an initialised
				 * ExtSprite structure. The
				 * sc_ExtSprite.es_SimpleSprite
				 * structure should be initialised as
				 * normal for the graphics.library
				 * sprite functions.
				 */
    struct TagItem *spc_TagList;
				/* Pointer to a TagList of ChangeExtSpriteA()
				 * tags. As of V40, ChangeExtSpriteA() has no
				 * tags defined, so this should be NULL. There
				 * is no guarantee that SpriteControl will
				 * support all or any of the future
				 * ChangeExtSpriteA() tags.
				 */
    UWORD spc_Line;		/* First line of the range in which the
				 * sprite is to be used.
				 */
    UWORD spc_Flags;		/* See below */
};

#define SPCB_MODIFY 0		/* When used with AnimateFX(), replace
				 * the sprite image with that in
				 * sc_ExtSprite.
				 */
#define SPCF_MODIFY (1 << SPCB_MODIFY)

/********************************************************************************/

#define SFX_ERR_NoMem 1		/* Not enough memory to complete the operation. */
#define SFX_ERR_MakeVP 2	/* MakeVPort failed */
#define SFX_ERR_Animating 3	/* Another application is already animating */
#define SFX_ERR_NotSupported 4	/* One of the effects in the TagList is not
				 * supported by this ViewPort.
				 */ 


#define SPECIALFXNAME	"specialfx.library"

#endif /* LIBRARIES_SPECIALFX_H */
