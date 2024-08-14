/*
 * (c) Copyright Weitek Corp 1990, 1991, All rights reserved
 *
 *	$Header: igc_gtk.h,v 13.0 91/09/30 16:51:54 release Exp $
 */

/*
	IGC Toolkit Include file
	This is to be used by the graphics toolkit,
	regardless of whether the chip or the predictor or the
	emulator is being used
*/

#include <sys/types.h>

/* define common types */
typedef unsigned char pixel;
/*typedef unsigned long ULONG;*/

/* define common structures */

	/* struct pxy { short x; short y;}; */
	struct point_2d { int x; int y; };
	struct size_2d { int w; int h; };
#ifdef IGCFORX11
	struct rect_igc { int x_min; int y_min; int x_max; int y_max; };
#else
	struct rect { int x_min; int y_min; int x_max; int y_max; };
#endif

#define IGM_PACK(a,b)	(((ULONG)(a)<<16) + ((ULONG)(b) & 0xffff))

#define IGM_PIXEL_STATUS_OKAY(status) (!(status & (IGM_STATUS_ERROR_MASK | \
						  IGM_STATUS_PIXEL_SOFT_MASK) ) )
#define IGM_BLIT_STATUS_OKAY(status) (!(status & (IGM_STATUS_ERROR_MASK | \
						  IGM_STATUS_BLIT_SOFT_MASK) ) )
#define IGM_QUAD_STATUS_OKAY(status) (!(status & (IGM_STATUS_ERROR_MASK | \
						  IGM_STATUS_QUAD_SOFT_MASK) ) )

/* STATUS */
typedef unsigned long STATUS;

/* define status parameters */
#define IGM_STATUS_BUSY_MASK		0x40000000	/* chip is busy */
#define IGM_STATUS_QB_BUSY_MASK		0x80000000	/* quad/blit busy */
#define IGM_STATUS_PICKED_MASK		0x00000080	/* pick detected */
#define IGM_STATUS_PIXEL_SOFT_MASK	0x00000040	/* pixel needs software */
#define IGM_STATUS_BLIT_SOFT_MASK	0x00000020	/* blit needs software */
#define IGM_STATUS_QUAD_SOFT_MASK	0x00000010	/* quad needs software */
#define IGM_STATUS_QUAD_CONCAVE_MASK	0x00000008	/* quad is concave */
#define IGM_STATUS_QUAD_HIDDEN_MASK	0x00000004	/* quad is hidden */
#define IGM_STATUS_QUAD_VISIBLE_MASK	0x00000002	/* quad is visible */
#define IGM_STATUS_QUAD_INTERS_MASK	0x00000001	/* quad intersects window */


/* define miscellaneous parameters */
#define IGM_VALID_SCREEN_W_MASK   0xfffff01f	/* mask of valid screen
						   width bits */
/* RASTER */
/* define raster values */
#define IGM_RASTER_IGNORE_PATTERN 0x00000000	/* ignore pattern */
#define IGM_RASTER_USE_PATTERN	  0x00020000	/* use pattern */

#define IGM_RASTER_X11_MODE	  0x00000000	/* draw X11 mode */
#define IGM_RASTER_OVERSIZE_MODE  0x00010000	/* draw oversized mode */

/* define raster masks */
#define IGM_RASTER_USE_PATTERN_MASK	0x00020000	/* use pattern mask */
#define IGM_RASTER_FILL_MODE_MASK       0x00010000	/* draw fill mode mask */

#define IGM_RASTER_MINTERMS_MASK	0x0000ffff	/* minterms mask */

/* merge non-minterms values from raster with new minterms value */
#define IGM_MERGE_MINTERMS_RASTER(minterms,raster) \
	( (raster&(-1-IGM_RASTER_MINTERMS_MASK)) \
		| (minterms&IGM_RASTER_MINTERMS_MASK) );

/* SYSCONFIG */
/* define sysconfig masks */
/* Define these fields a little more generally (TLT). */
/* first, lay out the bit fields and widths and masks */
#define IGM_MASK(w,s)			((((unsigned long)1 << (w)) - 1) << (s))
#define IGM_SYSBIT_VERSION		0
#define IGM_SYSFLD_VERSION		3
#define IGM_SYSCONFIG_VERSION_MASK       (IGM_MASK(IGM_SYSFLD_VERSION,IGM_SYSBIT_VERSION))

#define IGM_SYSBIT_TESTMODE		(IGM_SYSBIT_VERSION + IGM_SYSFLD_VERSION)
#define IGM_SYSFLD_TESTMODE		6
#define IGM_SYSCONFIG_TESTMODE_MASK     (IGM_MASK(IGM_SYSFLD_TESTMODE,IGM_SYSBIT_TESTMODE))

#define IGM_SYSBIT_PIXEL_BUF_WRITE	(IGM_SYSBIT_TESTMODE + IGM_SYSFLD_TESTMODE)
#define IGM_SYSFLD_PIXEL_BUF_WRITE	1
#define IGM_SYSCONFIG_PIX_BUF_WRITE_MASK (IGM_MASK(IGM_SYSFLD_PIXEL_BUF_WRITE,IGM_SYSBIT_PIXEL_BUF_WRITE))

#define IGM_SYSBIT_PIXEL_BUF_READ	(IGM_SYSBIT_PIXEL_BUF_WRITE + IGM_SYSFLD_PIXEL_BUF_WRITE)
#define IGM_SYSFLD_PIXEL_BUF_READ	1
#define IGM_SYSCONFIG_PIX_BUF_READ_MASK  (IGM_MASK(IGM_SYSFLD_PIXEL_BUF_READ,IGM_SYSBIT_PIXEL_BUF_READ))

#define IGM_SYSBIT_SWAP_BITS		(IGM_SYSBIT_PIXEL_BUF_READ + IGM_SYSFLD_PIXEL_BUF_READ)
#define IGM_SYSFLD_SWAP_BITS		1
#define IGM_SYSCONFIG_PIX_SWAP_BITS_MASK (IGM_MASK(IGM_SYSFLD_SWAP_BITS,IGM_SYSBIT_SWAP_BITS))
#define IGM_SYSCONFIG_PIX_SWAP_BITS	 IGM_SYSCONFIG_PIX_SWAP_BITS_MASK /* 1 bit field */
#define IGM_SYSCONFIG_PIX_NO_SWAP_BITS	 0L

#define IGM_SYSBIT_SWAP_BYTE		(IGM_SYSBIT_SWAP_BITS + IGM_SYSFLD_SWAP_BITS)
#define IGM_SYSFLD_SWAP_BYTE		1
#define IGM_SYSCONFIG_PIX_SWAP_BYTE_MASK (IGM_MASK(IGM_SYSFLD_SWAP_BYTE,IGM_SYSBIT_SWAP_BYTE))
#define IGM_SYSCONFIG_PIX_SWAP_BYTE	 IGM_SYSCONFIG_PIX_SWAP_BYTE_MASK /* 1 bit field */
#define IGM_SYSCONFIG_PIX_NO_SWAP_BYTE	 0L

#define IGM_SYSBIT_SWAP_HALF		(IGM_SYSBIT_SWAP_BYTE + IGM_SYSFLD_SWAP_BYTE)
#define IGM_SYSFLD_SWAP_HALF		1
#define IGM_SYSCONFIG_PIX_SWAP_HALF_MASK (IGM_MASK(IGM_SYSFLD_SWAP_HALF,IGM_SYSBIT_SWAP_HALF))
#define IGM_SYSCONFIG_PIX_SWAP_HALF	 IGM_SYSCONFIG_PIX_SWAP_HALF_MASK  /* 1 bit field */
#define IGM_SYSCONFIG_PIX_NO_SWAP_HALF	 0L

#define IGM_SYSBIT_SHIFT0		(IGM_SYSBIT_SWAP_HALF + IGM_SYSFLD_SWAP_HALF)
#define IGM_SYSFLD_SHIFT0		3
#define IGM_SYSCONFIG_SHIFT0_MASK	(IGM_MASK(IGM_SYSFLD_SHIFT0,IGM_SYSBIT_SHIFT0))
#define IGM_SYSCONFIG_SHIFT0_SHIFT	IGM_SYSBIT_SHIFT0

#define IGM_SYSBIT_SHIFT1		(IGM_SYSBIT_SHIFT0 + IGM_SYSFLD_SHIFT0)
#define IGM_SYSFLD_SHIFT1		3
#define IGM_SYSCONFIG_SHIFT1_MASK	(IGM_MASK(IGM_SYSFLD_SHIFT1,IGM_SYSBIT_SHIFT1))
#define IGM_SYSCONFIG_SHIFT1_SHIFT	IGM_SYSBIT_SHIFT1

#define IGM_SYSBIT_SHIFT2		(IGM_SYSBIT_SHIFT1 + IGM_SYSFLD_SHIFT1)
#define IGM_SYSFLD_SHIFT2		3
#define IGM_SYSCONFIG_SHIFT2_MASK	(IGM_MASK(IGM_SYSFLD_SHIFT2,IGM_SYSBIT_SHIFT2))
#define IGM_SYSCONFIG_SHIFT2_SHIFT	IGM_SYSBIT_SHIFT2

#define IGM_SYSBIT_PLLBACKUP		(IGM_SYSBIT_SHIFT2 + IGM_SYSFLD_SHIFT2)
#define IGM_SYSFLD_PLLBACKUP		1
#define IGM_SYSCONFIG_PLLBACKUP_MASK	(IGM_MASK(IGM_SYSFLD_PLLBACKUP,IGM_SYSBIT_PLLBACKUP))
#define IGM_SYSCONFIG_PLLBACKUP_SHIFT	IGM_SYSBIT_PLLBACKUP

#define IGM_SYSBIT_DRIVELOAD2		(IGM_SYSBIT_PLLBACKUP + IGM_SYSFLD_PLLBACKUP)
#define IGM_SYSFLD_DRIVELOAD2		1
#define IGM_SYSCONFIG_DRIVELOAD2_MASK	(IGM_MASK(IGM_SYSFLD_DRIVELOAD2,IGM_SYSBIT_DRIVELOAD2))
#define IGM_SYSCONFIG_DRIVELOAD2_SHIFT	IGM_SYSBIT_DRIVELOAD2

#define IGM_SYSBIT_CASWT2		(IGM_SYSBIT_PLLBACKUP + IGM_SYSFLD_PLLBACKUP)
#define IGM_SYSFLD_CASWT2		1
#define IGM_SYSCONFIG_CASWT2_MASK	(IGM_MASK(IGM_SYSFLD_CASWT2,IGM_SYSBIT_CASWT2))
#define IGM_SYSCONFIG_CASWT2_SHIFT	IGM_SYSBIT_CASWT2

#define IGM_SYSBIT_DUMMY

/* define pattern register count (# of registers in pattern array) */
#define IGM_PATTERN_REG_COUNT		8

/* DRAW_MODE */
/* define draw_mode masks */
#define IGM_DRAW_MODE_PICK_MASK		 0x000000c0	/* for pick mode */
#define IGM_DRAW_MODE_DEST_BUF_MASK	 0x00000030	/* for dest. buffer */
#define IGM_DRAW_MODE_SUPPRESS_ODD_MASK	 0x0000000c	/* for supp. odd lines */
#define IGM_DRAW_MODE_SUPPRESS_EVEN_MASK 0x00000003	/* for supp. even lines */

/* define draw_mode values */
#define IGM_DRAW_MODE_PICK_DISABLE	 0x00000080	/* for pick mode */
#define IGM_DRAW_MODE_PICK_ENABLE	 0x000000c0	/* for pick mode */
#define IGM_DRAW_MODE_DEST_BUF0		 0x00000020	/* for dest. buffer 0 */
#define IGM_DRAW_MODE_DEST_BUF1		 0x00000030	/* for dest. buffer 1 */
#define IGM_DRAW_MODE_NO_SUPPRESS_ODD	 0x00000008	/* not supp. odd lines */
#define IGM_DRAW_MODE_SUPPRESS_ODD	 0x0000000c	/* for supp. odd lines */
#define IGM_DRAW_MODE_NO_SUPPRESS_EVEN	 0x00000002	/* not supp. even lines */
#define IGM_DRAW_MODE_SUPPRESS_EVEN	 0x00000003	/* for supp. even lines */

/* to specify video register addresses */
#define IGM_VIDEO_HRZC		 1
#define IGM_VIDEO_HRZT		 2
#define IGM_VIDEO_HRZSR		 3
#define IGM_VIDEO_HRZBR		 4
#define IGM_VIDEO_HRZBF		 5
#define IGM_VIDEO_PREHRZC	 6
#define IGM_VIDEO_VRTC		 7
#define IGM_VIDEO_VRTT		 8
#define IGM_VIDEO_VRTSR		 9
#define IGM_VIDEO_VRTBR		10
#define IGM_VIDEO_VRTBF		11
#define IGM_VIDEO_PREVRTC	12
#define IGM_VIDEO_SRADDR	13
#define IGM_VIDEO_SRTCTL	14
#define IGM_VIDEO_SRADDR_INC	15

/* to specify debugging levels */
#define WTK_D_NORMAL	10
#define WTK_D_COARSE	20
#define WTK_D_MEDIUM	30
#define WTK_D_FINE	40
 
/*      for debugging   */
#define WTK_DEBUG_MODE(mode)\
(debug_mode >= mode)
 
extern int debug_mode;

/*
 * (c) Copyright Weitek Corp 1990, 1991, All rights reserved
 *
 *	$Header: igc_gtk.h,v 13.0 91/09/30 16:51:54 release Exp $
 */
