/*
 * (c) Copyright Weitek Corp 1990, 1991, All rights reserved
 *
 *	$Header: igc_macros.h,v 10.6 91/08/13 09:31:35 release Exp $
 */

#define IGM_HB_SHIFT 15

#define IGM_NO_SWAP  0
#define IGM_B_SWAP   1
#define IGM_H_SWAP   2
#define IGM_HB_SWAP  3
#define ENDIAN_MACHINE IGM_NO_SWAP

/* ops for rops */
#define OP_MEM8_SCR	1
#define OP_SCR_SCR	2
#define OP_SCR_MEM	4
#define OP_MEM1_MEM	8

/******************************************************************************
 *
 *			HIGHER LEVEL
 *
*****************************************************************************/

/* Interrupt Enable/Dissable Macros */
/* These are the write enable masks for the interrupt registers */
#define IGM_IDLE	0x01
#define IGM_PICKED	0x04
#define IGM_VBLANKED	0x10
#define IGM_MASTER	0x40
#define IGM_WR_ENABLE	((IGM_IDLE|IGM_PICKED|IGM_VBLANKED|IGM_MASTER) << 1)

/* The interrupt field can be combined write enable macros.
 * For example IGM_ENABLE(igc_p, IGM_VBLANKED | IGM_MASTER) enables the
 * vertical blanking interrupt as well as master enable.
 */
#define IGM_INT_ENABLE(video_p, interrupt) \
	IGC_WRITE_INTERRUPT_EN(video_p, \
	((interrupt) | ((interrupt) <<1) & IGM_WR_ENABLE))
#define IGM_INT_DISABLE(video_p, interrupt) \
	IGC_WRITE_INTERRUPT_EN(video_p, \
	(((interrupt) <<1) & IGM_WR_ENABLE))
#define IGM_INT_CHECK(video_p, interrupt) \
	((IGC_READ_INTERRUPT(video_p))& (interrupt))
#define IGM_INT_CLEAR(video_p, interrupt) \
	IGC_WRITE_INTERRUPT(video_p, \
	(((interrupt) <<1) & IGM_WR_ENABLE ))

/* draw a rectangle */
#define IGM_RECTANGLE(igc_p, x_min, y_min, x_max, y_max, status) \
	IGC_WRITE_2D_META_COORD (igc_p, IGM_ABS, IGM_XY, IGM_RECT, \
			IGM_PACK (x_min, y_min)); \
	IGC_WRITE_2D_META_COORD (igc_p, IGM_ABS, IGM_XY, IGM_RECT, \
			IGM_PACK (x_max, y_max)); \
	do status = (int) IGC_DRAW_QUAD (igc_p); \
	    while (status & IGM_STATUS_QB_BUSY_MASK);

/* draw a line */
#define IGM_LINE_DRAW(igc_p, x_0, y_0, x_1, y_1, status) \
	IGC_WRITE_2D_META_COORD (igc_p, IGM_ABS, IGM_XY, IGM_LINE, \
			IGM_PACK (x_0, y_0)); \
	IGC_WRITE_2D_META_COORD (igc_p, IGM_ABS, IGM_XY, IGM_LINE, \
			IGM_PACK (x_1, y_1)); \
	do status = (int) IGC_DRAW_QUAD (igc_p); \
	    while (status & IGM_STATUS_QB_BUSY_MASK);

/* perform a rop on a single pixel */
#define IGM_ROP_PIXEL(igc_p, pixel_x, pixel_y, status) \
	IGC_WRITE_2D_META_COORD (igc_p, IGM_ABS, IGM_XY, IGM_POINT, \
			IGM_PACK(pixel_x, pixel_y) ); \
	do status = (int) IGC_DRAW_QUAD (igc_p); \
	    while (status & IGM_STATUS_QB_BUSY_MASK);

/* send a clip window to the chip */
/* NOTE (CAUTION): the clip window is not clipped to screen coordinates */
#define IGM_CLIP_WINDOW(igc_p, x_min, y_min, x_max, y_max) \
	IGC_WRITE_REG (igc_p, IGM_W_MIN_XY, IGM_PACK(x_min,y_min) ); \
	IGC_WRITE_REG (igc_p, IGM_W_MAX_XY, IGM_PACK(x_max,y_max) );

/* get plane mask from chip */
#define IGM_GET_PLANE_MASK(igc_p) (pixel) IGC_READ_REG (igc_p, IGM_PLANE_MASK);

/* send plane mask to chip */
#define IGM_PUT_PLANE_MASK(igc_p, plane_mask) \
	IGC_WRITE_REG (igc_p, IGM_PLANE_MASK, (ULONG) plane_mask);

/* calculate pixel address */
#define IGM_CALC_PIXEL_ADDRESS(fb_p, x, y, screen_width) \
		((fb_p) + ((screen_width)*(y))+(x))

/******************************************************************************
 *
 *			2D REGISTER ACCESS
 *
 *****************************************************************************/
#define IGC_WRITE_2D_META_COORD(igc_p,rel_mode,coord,poly,data)\
IGC_WRITE_2D_META_COORD_(igc_p,rel_mode,coord,poly,data,ENDIAN_MACHINE)

#define IGC_WRITE_2D_COORD_REG(igc_p,rel_mode,coord,reg,data)\
IGC_WRITE_2D_COORD_REG_(igc_p,rel_mode,coord,reg,data,ENDIAN_MACHINE)

#define IGC_READ_2D_COORD_REG(igc_p,coord,reg)\
IGC_READ_2D_COORD_REG_(igc_p,coord,reg,ENDIAN_MACHINE)


/******************************************************************************
 *
 *			MISC REGISTER ACCESS
 *
 *****************************************************************************/
#define IGC_READ_REG(igc_p, reg)\
	IGC_READ_REG_ (igc_p, reg, ENDIAN_MACHINE)

#define IGC_WRITE_REG(igc_p, reg, data)\
	IGC_WRITE_REG_ (igc_p, reg, data, ENDIAN_MACHINE)

#define IGC_READ_INTERRUPT(video_p)\
	IGC_READ_INTERRUPT_ (video_p, ENDIAN_MACHINE)

#define IGC_WRITE_INTERRUPT(video_p, interrupt)\
	IGC_WRITE_INTERRUPT_ (video_p, interrupt, ENDIAN_MACHINE)

#define IGC_READ_INTERRUPT_EN(video_p)\
	IGC_READ_INTERRUPT_EN_ (video_p, ENDIAN_MACHINE)

#define IGC_WRITE_INTERRUPT_EN(video_p, interrupt)\
	IGC_WRITE_INTERRUPT_EN_ (video_p, interrupt, ENDIAN_MACHINE)

#define IGC_READ_SYS_CNTRL(video_p, reg)				\
	IGC_READ_SYS_CNTRL_(video_p, reg, ENDIAN_MACHINE)

#define IGC_WRITE_SYS_CNTRL(video_p, reg, data)			\
	IGC_WRITE_SYS_CNTRL_(video_p, reg, data, ENDIAN_MACHINE)


/******************************************************************************
 *
 *			OPS 
 *
 *****************************************************************************/
#define IGC_DRAW_QUAD(igc_p)\
IGC_DRAW_QUAD_ (igc_p, ENDIAN_MACHINE)

#define IGC_DO_BLIT(igc_p)\
IGC_DO_BLIT_ (igc_p, ENDIAN_MACHINE)

#define IGC_READ_PIXEL8(igc_p)\
igc_read_pixel8_ (igc_p, ENDIAN_MACHINE)

#define IGC_WRITE_PIXEL8(igc_p, data)\
IGC_WRITE_PIXEL8_ (igc_p, data, ENDIAN_MACHINE)

#define IGC_WRITE_PIXEL1(igc_p, count, data)\
IGC_WRITE_PIXEL1_ (igc_p, count, data, ENDIAN_MACHINE)

#define IGC_NEXT_PIXELS(igc_p, width)\
IGC_NEXT_PIXELS_ (igc_p, width, ENDIAN_MACHINE)


/******************************************************************************
 *
 *			FRAME BUFFER ACCESS
 *
 *****************************************************************************/
#define IGM_CALC_PIXEL4_ADDRESS(fb_p, x, y, screen_width)\
(ULONG *)IGM_CALC_PIXEL_ADDRESS(fb_p, x, y, screen_width)

#define IGC_READ_PIXEL4(fbp4_p)\
IGC_READ_PIXEL4_(fbp4_p)

#define IGC_WRITE_PIXEL4(fbp4_p, data)\
IGC_WRITE_PIXEL4_(fbp4_p, data)


/******************************************************************************
 *
 *			VIDEO ACCESS
 *
 *****************************************************************************/
#define IGC_READ_VIDEO(video_p, control)\
IGC_READ_VIDEO_ (video_p, control, ENDIAN_MACHINE)

#define IGC_WRITE_VIDEO(video_p, control, data)\
IGC_WRITE_VIDEO_ (video_p, control, data, ENDIAN_MACHINE)

#define IGC_READ_RAMDAC(video_p, control)\
IGC_READ_RAMDAC_ (video_p, control, ENDIAN_MACHINE)

#define IGC_WRITE_RAMDAC(video_p, control, data)\
IGC_WRITE_RAMDAC_ (video_p, control, data, ENDIAN_MACHINE)


/****************************************************************************
 * 
 *  			CHIP DEFINITIONS
 *
 ****************************************************************************
 */

/* all accesses (igc and video registers) have hbb in the same address bits */
#define IGM_INSTR_SHIFT_HBB   (16-2)

/* to specify polygon type */
#define IGM_POINT 0
#define IGM_LINE  1
#define IGM_TRI   2
#define IGM_QUAD  3
#define IGM_RECT  4

/* to specify 2D coordinate register */
#define IGM_X0  0
#define IGM_Y0  0
#define IGM_X1  1
#define IGM_Y1  1
#define IGM_X2  2
#define IGM_Y2  2
#define IGM_X3  3
#define IGM_Y3  3

/* to specify what the input coordinates are relative to */
#define IGM_ABS 0
#define IGM_VTX 1
#define IGM_WIN_REG 1
#define IGM_WIN_META 0

/* to specify which coordinate (XY means and X and Y packed) */
#define IGM_X   1
#define IGM_Y   2
#define IGM_XY  3

/* register numbers */
#define IGM_STATUS       0

#define IGM_MISC_PE_OFF 0x180
#define IGM_OOR		(IGM_MISC_PE_OFF >> 2) + 1
#define IGM_CINDEX	(IGM_MISC_PE_OFF >> 2) + 3
#define IGM_W_OFF	(IGM_MISC_PE_OFF >> 2) + 4
#define IGM_PE_W_MIN	(IGM_MISC_PE_OFF >> 2) + 5
#define IGM_PE_W_MAX	(IGM_MISC_PE_OFF >> 2) + 6
#define IGM_TREG	(IGM_MISC_PE_OFF >> 2) + 7
#define IGM_XCLIP	(IGM_MISC_PE_OFF >> 2) + 8
#define IGM_YCLIP	(IGM_MISC_PE_OFF >> 2) + 9
#define IGM_XEDGE_LT	(IGM_MISC_PE_OFF >> 2) + 10
#define IGM_XEDGE_GT	(IGM_MISC_PE_OFF >> 2) + 11
#define IGM_YEDGE_LT	(IGM_MISC_PE_OFF >> 2) + 12
#define IGM_YEDGE_GT	(IGM_MISC_PE_OFF >> 2) + 13

#define IGM_MISC_PIX_OFF 0x200
#define IGM_FGROUND	(IGM_MISC_PIX_OFF >> 2) + 0
#define IGM_BGROUND	(IGM_MISC_PIX_OFF >> 2) + 1
#define IGM_PLANE_MASK	(IGM_MISC_PIX_OFF >> 2) + 2
#define IGM_DRAW_MODE	(IGM_MISC_PIX_OFF >> 2) + 3
#define IGM_PAT_ORIGINX	(IGM_MISC_PIX_OFF >> 2) + 4
#define IGM_PAT_ORIGINY	(IGM_MISC_PIX_OFF >> 2) + 5
#define IGM_RASTER	(IGM_MISC_PIX_OFF >> 2) + 6
#define IGM_PIXEL8_REG	(IGM_MISC_PIX_OFF >> 2) + 7
#define IGM_W_MIN_XY	(IGM_MISC_PIX_OFF >> 2) + 8
#define IGM_W_MAX_XY	(IGM_MISC_PIX_OFF >> 2) + 9
#define IGM_PATTERN	(IGM_MISC_PIX_OFF >> 2) + 0x20

#define IGM_PATTERN0     IGM_PATTERN + 0
#define IGM_PATTERN1     IGM_PATTERN + 1
#define IGM_PATTERN2     IGM_PATTERN + 2
#define IGM_PATTERN3     IGM_PATTERN + 3
#define IGM_PATTERN4     IGM_PATTERN + 4
#define IGM_PATTERN5     IGM_PATTERN + 5
#define IGM_PATTERN6     IGM_PATTERN + 6
#define IGM_PATTERN7     IGM_PATTERN + 7

/****************************************************************************
 *
 *	  		2D Register Access
 *
 ***************************************************************************/

/* Shift right to compensate for addressing left shift by the compiler */
#define IGM_INSTR_META_COORD (0x001200>>2)

/* Subtract 2 from shift to compensate for addressing left shift by
 * the compiler */
#define IGM_INSTR_SHIFT_REG   ( 6-2)  
#define IGM_INSTR_SHIFT_REL   ( 5-2)
#define IGM_INSTR_SHIFT_YX    ( 3-2)

#define IGM_INSTR_SHIFT_VTYPE ( 6-2)
#define IGC_WRITE_2D_META_COORD_(igc_p,rel_mode,coord,poly,data,hbb) \
    (igc_p)[IGM_INSTR_META_COORD 					\
	+ ((rel_mode)<< IGM_INSTR_SHIFT_REL) 			\
	+ ((coord)   << IGM_INSTR_SHIFT_YX) 			\
	+ ((poly)    << IGM_INSTR_SHIFT_VTYPE)			\
	+ ((hbb)     << IGM_INSTR_SHIFT_HBB) 			\
	] = (data)

#define IGM_INSTR_COORD_REG (0x001000>>2)
#define IGC_WRITE_2D_COORD_REG_(igc_p,rel_mode,coord,reg,data,hbb) \
    (igc_p)[IGM_INSTR_COORD_REG 					\
	+ ((rel_mode)<< IGM_INSTR_SHIFT_REL) 			\
	+ ((coord)   << IGM_INSTR_SHIFT_YX) 			\
	+ ((reg)     <<  IGM_INSTR_SHIFT_REG) 			\
	+ ((hbb)     << IGM_INSTR_SHIFT_HBB) 			\
	] = (data)

#define IGC_READ_2D_COORD_REG_(igc_p,coord,reg,hbb) 		\
    (igc_p)[IGM_INSTR_COORD_REG 					\
	+ ((coord)   << IGM_INSTR_SHIFT_YX) 			\
	+ ((reg)     <<  IGM_INSTR_SHIFT_REG) 			\
	+ ((hbb)     << IGM_INSTR_SHIFT_HBB) 			\
	]

/****************************************************************************
 *
 *	  		Misc Register Access
 *
 ***************************************************************************/
/************************* CHANGED EJS 11 JULY 91 ******************* */
/* As far as I can tell the correct shifting has been accomplished in
   the register definitions so the IGM_INSTR_SHIFT_REG shift has been
   removed from the miscellaneous macros */

#define IGM_INSTR_MISC_REG (0x000000>>2)
#define IGC_READ_REG_(igc_p, reg, hbb) 				\
    (igc_p)[IGM_INSTR_MISC_REG					\
	 + (reg)						\
	 + ((hbb) << IGM_INSTR_SHIFT_HBB)			\
	 ]

/*    do {} while ((IGC_READ_REG(igc_p, IGM_STATUS)) 		\
    	& IGM_STATUS_BUSY_MASK);				\*/
#define IGC_WRITE_REG_(igc_p, reg, data, hbb)			\
    (igc_p)[IGM_INSTR_MISC_REG					\
	 + (reg)						\
	 + ((hbb) << IGM_INSTR_SHIFT_HBB)			\
	 ] = (data)

/****************************************************************************
 *
 *	  		Operation commands.
 *
 ***************************************************************************/
#define IGM_INSTR_DO_BLIT	(0x000004>>2)
#define IGC_DO_BLIT_(igc_p, hbb) 				\
    (igc_p)[IGM_INSTR_DO_BLIT					\
	 + ((hbb) << IGM_INSTR_SHIFT_HBB) 			\
	 ]

#define IGM_INSTR_DRAW_QUAD	(0x000008>>2)
#define IGC_DRAW_QUAD_(igc_p, hbb) 				\
    (igc_p)[IGM_INSTR_DRAW_QUAD					\
	 + ((hbb)  << IGM_INSTR_SHIFT_HBB) 			\
	 ]

#define IGM_INSTR_PIXEL8	(0x00000c>>2)
#define IGC_WRITE_PIXEL8_(igc_p, data, hbb) 			\
    (igc_p)[IGM_INSTR_PIXEL8					\
	 + ((hbb)  << IGM_INSTR_SHIFT_HBB) 			\
	 ] = (data)

#define IGM_INSTR_PIXEL1	(0x000080>>2)
#define IGC_WRITE_PIXEL1_(igc_p, pixels, data, hbb) 		\
    (igc_p)[IGM_INSTR_PIXEL1					\
	 + ((hbb)  << IGM_INSTR_SHIFT_HBB) 			\
	 + ((pixels))						\
	 ] = (data)

#define IGM_INSTR_NEXT_PIXELS	(0x000014>>2)
#define IGC_NEXT_PIXELS_(igc_p, width, hbb)			\
    (igc_p)[IGM_INSTR_NEXT_PIXELS				\
	 + ((hbb)  << IGM_INSTR_SHIFT_HBB) 			\
	 ] = (width)

/****************************************************************************
 *
 *	  			Video Access.
 *
 ***************************************************************************/

#define IGM_INSTR_SYS_CNTRL (0x000000>>2)
#define IGM_INSTR_SYS_CNTRL_SHIFT_REG (2 - 2)
#define IGM_SYSCONFIG    1
#define IGM_INTERRUPT    2
#define IGM_INTERRUPT_EN 3

#define IGC_READ_SYS_CNTRL_(video_p, reg, hbb)				\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ ((reg)	<< IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)        << IGM_INSTR_SHIFT_HBB) 			\
	]
    
#define IGC_WRITE_SYS_CNTRL_(video_p, reg, data, hbb)			\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ ((reg)	<< IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)        << IGM_INSTR_SHIFT_HBB)				\
	] = (data)

#define IGC_READ_INTERRUPT_(video_p, hbb)				\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ (IGM_INTERRUPT << IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)         << IGM_INSTR_SHIFT_HBB) 			\
	]
    
#define IGC_WRITE_INTERRUPT_(video_p, interrupt, hbb)			\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ (IGM_INTERRUPT << IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)         << IGM_INSTR_SHIFT_HBB)			\
	] = (interrupt)

#define IGC_READ_INTERRUPT_EN_(video_p, hbb)				\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ (IGM_INTERRUPT_EN << IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)         << IGM_INSTR_SHIFT_HBB) 			\
	]
    
#define IGC_WRITE_INTERRUPT_EN_(video_p, interrupt, hbb)		\
    (video_p)[IGM_INSTR_SYS_CNTRL					\
	+ (IGM_INTERRUPT_EN << IGM_INSTR_SYS_CNTRL_SHIFT_REG)		\
	+ ((hbb)         << IGM_INSTR_SHIFT_HBB)			\
	] = (interrupt)
#define IGM_INSTR_VIDEO	(0x000100>>2)
#define IGM_INSTR_VIDEO_SHIFT_REG (2 - 2)

#define IGC_WRITE_VIDEO_(video_p, regno, data, hbb)			\
    (video_p)[IGM_INSTR_VIDEO						\
	+ ((regno) << IGM_INSTR_VIDEO_SHIFT_REG)			\
	+ ((hbb)   << IGM_INSTR_SHIFT_HBB)				\
	] = (data)

#define IGC_READ_VIDEO_(video_p, regno, hbb)				\
    (video_p)[IGM_INSTR_VIDEO						\
	+ ((regno) << IGM_INSTR_VIDEO_SHIFT_REG)			\
	+ ((hbb)   << IGM_INSTR_SHIFT_HBB)				\
	]

#define IGM_INSTR_VRAM (0x000180>>2)
#define IGM_INSTR_VRAM_SHIFT_REG (2 - 2)

#define IGC_WRITE_VRAM_(video_p, regno, data, hbb)			\
    (video_p)[IGM_INSTR_VRAM						\
	+ ((regno) << IGM_INSTR_VRAM_SHIFT_REG)				\
	+ ((hbb)   << IGM_INSTR_SHIFT_HBB)				\
	] = (data)

#define IGC_READ_VRAM_(video_p, regno, hbb)				\
    (video_p)[IGM_INSTR_VRAM						\
	+ ((regno) << IGM_INSTR_VRAM_SHIFT_REG)				\
	+ ((hbb)   << IGM_INSTR_SHIFT_HBB)				\
	]

/*
 * RAMDAC definitions
 */
#define IGM_INSTR_RAMDAC (0x000200>>2)
#define IGM_INSTR_RAMDAC_SHIFT_REG (2 - 2)

#define IGC_READ_RAMDAC_(video_p, control, hbb)				\
	((video_p)[IGM_INSTR_RAMDAC					\
	+ ((control) << IGM_INSTR_RAMDAC_SHIFT_REG)			\
	+ ((hbb)     << IGM_INSTR_SHIFT_HBB)				\
	] )

#define IGC_WRITE_RAMDAC_(video_p, control, data, hbb)			\
    	(video_p)[IGM_INSTR_RAMDAC					\
	+ ((control) << IGM_INSTR_RAMDAC_SHIFT_REG)			\
	+ ((hbb)     << IGM_INSTR_SHIFT_HBB)				\
	] = (data) 

/****************************************************************************
 *
 *	  			Pixel Access.
 *
 ***************************************************************************/
#define IGC_READ_PIXEL4_(pixel4_p) 		*(pixel4_p)
#define IGC_WRITE_PIXEL4_(pixel4_p, data) 	*(pixel4_p) = (data)
#define IGC_READ_PIXEL(pixel_p)			*(pixel_p)
#define IGC_WRITE_PIXEL(pixel_p, data)		*(pixel_p) = (data)

