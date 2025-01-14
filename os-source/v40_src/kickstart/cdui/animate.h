#ifndef GRAPHICS_ANIMATE_H
#define GRAPHICS_ANIMATE_H
/*
**	$Id: animate.h,v 40.1 93/04/15 18:50:09 vertex Exp Locker: jerryh $
**
**	copperlist animation control
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/* Tags for the various AnimationControl() features. (V40) */

#define ACTAG_ColorRange 	0x80000001	/* ti_Data points to a
						 * struct ColorRange.
						 */
#define ACTAG_ScrollLines	0x80000002	/* ti_Data points to a
						 * struct ScrollLines.
						 */
#define ACTAG_RepeatLines	0x80000003	/* ti_Data points to a
						 * struct RepeatLines.
						 */



/* ColorRange - fill this structure for ACTAG_ColorRange.
 * Pass in ti_Data a pointer to an array of ColorRange structures. This array
 * can be used to change a number of pens on a line. Each line can
 * have an array of pens to change. The array is terminated when the cor_Pen at
 * the start of the line == -1
 *
 * Example:
 * struct ColorRange cr[] =
 * {
 *	{1, 0, 10, -1, 0, 0, NULL}, {2, 0, 0, 0, -1, 0, NULL},
 *	{1, 0, 11, 0x88888888, 0, 0, CORF_RESTORE, NULL}, {2, 0, 0, 0, 0x88888888, 0, 0, NULL},
 *      {-1, NULL},
 * };
 * This would change pen 1 on line 10 to red, pen 2 on line 10 to green,
 * pen 1 on line 11 to dark red, pen 2 on line 11 to dark green. At line 12, pen 1
 * is restored to its original value.
 */

struct ColorRange
{
    LONG cor_Pen;		/* Pen number to change */
    WORD cor_WAIT_X;		/* X Wait position - ignored for V40 */
    WORD cor_WAIT_Y;		/* Y Wait position */
    ULONG cor_Red;		/* 32 bit red value */
    ULONG cor_Green;		/* 32 bit green value */
    ULONG cor_Blue;		/* 32 bit blue value */
    ULONG cor_Flags;
    UBYTE cor_Private[36];	/* private */
};

#define CORB_ANIMATE 0		/* set to modify an existing ColorRange. If
				 * this bit is set in the first ColorRange in
				 * the array, then the hardware copperlist is
				 * modified with these new colours.
				 * THIS BIT SHOULD ONLY BE SET AFTER THE
				 * COLOUR RANGE HAS BEEN BUILT.
				 */
#define CORF_ANIMATE (1 << CORB_ANIMATE)
#define CORB_MODIFY 1		/* When used with CORB_ANIMATE, this colour
				 * entry is modified to the RGB value.
				 */
#define CORF_MODIFY (1 << CORB_MODIFY)
#define CORB_RESTORE 2		/* if set, then restore the pen number to its
				 * original value. This needs to be set only once
				 * for the pen number.
				 */
#define CORF_RESTORE (1 << CORB_RESTORE)

/* ScrollLines - fill this structure for ACTAG_ScrollLines. It allows a group
 * of lines in a ViewPort to be scrolled separately from the remainder.
 */

struct ScrollLines
{
    ULONG sl_Mask;		/* Mask of the bitplanes to scroll. */
    UWORD sl_YOffset;		/* First line in this ViewPort to start
				 * scrolling.
				 */
    UWORD sl_YCount;		/* Number of lines to scroll */
    WORD sl_XOffset;		/* Value by which to scroll the lines. -ve is to
    				 * the left, +ve to the right.
    				 */
    UWORD sl_ScrollRate;	/* Move the lines by sl_XOffset every
    				 * sl_ScrollRate times ChangeVPBitMap() changes
    				 * the BitMap (which is usually every time, but
    				 * can be changed with ACTAG_FrameRate).
    				 * If NULL, then the scroll is done only by
    				 * AnimationControl().
    				 */
};

/* RepeatLines - fill this structure for ACTAG_RepeatLines. It allows a single
 * line of a ViewPort to be repeated over a range of other lines.
 */

struct RepeatLines
{
    ULONG lr_Mask;		/* Mask of the bitplanes to repeat */
    UWORD lr_YOffset;		/* first line in the ViewPort to start repeating*/
    UWORD lr_YCount;		/* Repeat for this many lines */
};

#define AC_ERR_NoMem 1		/* not enough memory to complete the operation. */

#endif /* GRAPHICS_ANIMATE_H */
