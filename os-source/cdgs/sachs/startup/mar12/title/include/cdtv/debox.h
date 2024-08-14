/* :ts=8
*
*   debox.h - Headers for dbox
*
* William A. Ware					A515
* Name changes to flags.				AA01
* Version 2						C128
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY			    *
*   Copyright (C) 1992, Silent Software Incorporated.			    *
*   All Rights Reserved.						    *
****************************************************************************/

#ifndef CDTV_DEBOX_H
#define CDTV_DEBOX_H

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

/*
 * DeBox file types.
 */
#define DEBOXTYPE_UNKNOWN	0	/*  Unspecified data.		   */
#define DEBOXTYPE_PIC		1	/*  Picture.			   */

/*
 * Error codes.
 */
#define DEBOXERR_HEADER		-1	/*  Header is invalid.		   */
#define DEBOXERR_DATA		-2	/*  Data has an error in it.	   */
#define DEBOXERR_TYPE		-3	/*  Data is the wrong type of file.
					    ie DEBOXTYPE_UNKNOWN instead of 
					    DEBOXTYPE_PIC.		   */
#define DEBOXERR_MEMORY		-4	/*  Not enough memory to decompress
					    file.			   */

#define COMPHEADER_SIZE		(sizeof(struct CompHeader))

struct CompHeader
{
	UBYTE	ci_Check;	/*  Checksum for the header.		*/
	UBYTE	ci_Version;	/*  Version for the compression.	*/
	UBYTE	ci_pad;		/*  Reserved for future use.		*/
	UBYTE	ci_Type;	/*  Type of data, e.g. DEBOXTYPE_PIC,
				    DEBOXTYPE_UNKNOWN, et al.		*/
	ULONG	ci_DataInfo;	/*  Info on data -- for library's use.	*/
	LONG	ci_Size;	/*  Size of uncompressed data.		*/
	LONG	ci_CSize;	/*  Size of compressed data.		*/
};


/*
 * Notes on BMInfo:
 *
 * bmi_TotalSize:
 *	DecompBMInfo() allocates everything together so there are private
 * parts after this structure.  This is the total size of the entire
 * structure, public and private.
 *
 * bmi_TransparentColor:
 *	If BMIF_TRANSPARENT_COLOR is set in bmi_Flags, then this field can be
 * used to make a mask out of the decompressed bit map.
 */
struct BMInfo
{
	ULONG		bmi_TotalSize;	/*  See notes above.		   */
	UWORD		*bmi_ColorMap;	/*  Pointer to an Amiga colormap.  */
	struct RangeInfo	*bmi_RangeInfo; /* Pointer to RangeInfos.  */
	UWORD		bmi_Width,	/*  Dimensions of the picture.	   */
			bmi_Height;	
	UBYTE		bmi_Depth;	/*  Number of bitplanes.	   */
	UBYTE		bmi_Flags;	/*  See below.			   */
	UWORD		bmi_Modes;	/*  View modes (HIRES, LACE, etc.) */
		/* 
		 * V2
		 *  Being a UBYTE bmi_NumColors could not go pass 255.
		 *  colors.  Seeing no way around this, bmi_ColorCount has
		 *  been added to take its place.  bmi_NumColors will still
		 *  be maintained but it the colors are greater than 255,
		 *  bmi_NumColors will equal 255 while bmi_ColorCount will
		 *  have the actual number of colors.
		 */
	UBYTE		bmi_NumColors;
	UBYTE		bmi_NumRanges;	/*  Number of RangeInfos.	   */
	UBYTE		bmi_TransparentColor;	/*  See notes above.	   */
	UBYTE		bmi_pad1;	/*  Reserved for future use.	   */
	UWORD		bmi_pad2;	/*  Reserved for future use.	   */
		/*
		 * V2 
		 *  Number of colors in colormap. (see bmi_NumColors)
		 *  Also note that V2+ of DecompBMInfo() will fill this in
		 *  with bmi_NumColors if it is zero.
		 */
	UWORD		bmi_ColorCount;	/*  V2 Number of colors in colormap*/
};

/*
 * BMInfo Flags.
 */
#define BMIF_HAS_COLORS		1	/*  BMInfo has a colormap.	   */
#define BMIF_HAS_MASK		(1<<1)	/*  Bitmap has mask plane.	   */
#define BMIF_HAS_RANGES		(1<<2)	/*  BMInfo has RangeInfos.	   */
#define BMIF_TRANSPARENT_COLOR	(1<<3)	/*  See notes above.		   */

/*
 * Private 
 */
#define BMIF_HAS_EXTCOLORS	(1<<4)	/*  V2 Extended colors are present.*/
#define BMIF_HAS_BMIEXT		(1<<5)	/*  V2 Has a BMIExtension. */

/* V2
 * SPECIAL data for GetSpecialData() function.
 */
	/* V2
	 * BMISPECIAL_MODEID:
	 *	GetSpecialData() will return a ULONG with the bmi's modeid as
	 *	for use under 2.0 operating system.
	 *
	 *      example:
	 *	    ULONG modeid;
	 *	    modeid = (ULONG)GetSpecialData( BMISPECIAL_MODEID, bmi );
	 */
#define BMISPECIAL_MODEID	0x5211120d

	/* V2
	 * BMISPEICAL_RGB32
	 *	Raw 32 bit Cmap data (see LoadRGB32) is availible. GetSpecial()
	 *	will return NULL if this data is not found.
	 *
	 *	It is suggested that the GetBMInfoRGB32() be used instead since it
	 *	will automaticlly detect and use this data or if not availible 
	 *	use the compressed color data contained in the BMInfo structure.
	 */
#define BMISPECIAL_RGB32	0xd011ff0d

/*
 * Notes on RangeInfo:
 *
 * rgi_Size:
 *	Contains the total size of the RangeInfo structure.  If there is an
 * array of RangeInfos, this value may be used as an offset from the current
 * RangeInfo to the next one in the array.
 *
 * rgi_CArray[]:
 *	It contains color table indicies.  It specifies the order in which
 * colors should be rotated through the colormap.  For example, if rgi_CArray
 * were to contain 1,2,4, you would cycle register 4 into register 2,
 * register 2 into register 1, and register 1 into register 4.
 */
struct RangeInfo
{
	UWORD	rgi_Size;	/*  See notes above.			   */
	UBYTE	rgi_Low,	/*  Start the cycle at rgi_CArray[rgi_Low] */
		rgi_High;	/*  and end at rgi_CArray[rgi_High]	   */
	BYTE	rgi_Dir;	/*  Direction -- -1:Left, 0:Off, 1: Right  */
	UBYTE	rgi_Flags;	/*  Flags (none defined as of yet).	   */
	WORD	rgi_Seconds;	/*  Time between each cycle.		   */
	LONG	rgi_MicroSeconds;
	WORD	rgi_SecondsLeft;/*  Countdown variables.		   */
	LONG	rgi_MicroLeft;	/*  Both modified by CycleRanges().	   */
	ULONG	rgi_reserved;
	UBYTE	rgi_CArray[32];	/*  See notes above.			   */
};

#define NEXT_RANGEINFO(ri)  ((struct RangeInfo *)(&((UBYTE *)ri)[ri->rgi_Size]))


struct SuperView
{
	struct View	sv_View;
	struct ViewPort	sv_ViewPort;
	struct RasInfo	sv_RasInfo;
	LONG		sv_Flags;
};

/*
 * SuperView Flags.
 */
#define SVF_ALLOCATED	1	/* This structure created by CreateView(). */


#endif
