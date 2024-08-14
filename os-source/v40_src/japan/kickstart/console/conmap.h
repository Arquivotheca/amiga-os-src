#ifndef	DEVICES_CONMAP_H
#define	DEVICES_CONMAP_H
/*
**	$Id: conmap.h,v 36.5 90/06/07 11:33:23 kodiak Exp $
**
**	Console device map definition
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

/*----- cm_Attr... entries -----------------------------------------*/
#define	CMAM_FGPEN	0x0007		/* for FgPen */
#define	CMAS_FGPEN	0
#define	CMAM_BGPEN	0x0038		/* for BgPen */
#define	CMAS_BGPEN	3
#define	CMAM_SOFTSTYLE	0x01c0		/* for SetSoftStyle */
#define	CMAS_SOFTSTYLE	6
#define	CMAF_INVERSVID	0x0200		/* RP_INVERSVID set */
#define	CMAB_INVERSVID	9
#define	CMAF_SELECTED	0x0400		/* selection */
#define	CMAB_SELECTED	10
#define	CMAF_HIGHLIGHT	0x0800		/* highlighted part of selection */
#define	CMAB_HIGHLIGHT	11
#define	CMAF_TABBED	0x1000		/* tab immediately preceeded */
#define	CMAB_TABBED	12		/*   this character placement */
#define	CMAF_IMPLICITNL	0x2000		/* CUB_IMPLICITNL set (valid for 1st */
#define	CMAB_IMPLICITNL	12		/*   character in line only) */
#define	CMAF_CURSOR	0x4000		/* cursor cached here during resize */
#define	CMAB_CURSOR	13
#define	CMAF_RENDERED	0x8000		/* this entry and all entries to the */
#define	CMAB_RENDERED	15		/*   left on the line are valid. */
					/*   (must be the sign bit) */

 struct ConsoleMap {
    ULONG   cm_AllocSize;	/* AllocMem size for cm_AllocBuffer */
    ULONG   *cm_AttrBufLines;	/* array with cm_BufferLines elements */
				/*   containing address/2 of off-screen */
				/*   attr lines */
    ULONG   *cm_AttrDispLines;	/* array with cm_DisplayHeight elements */
				/*   containing address/2 of displayed */
				/*   attr lines */
    ULONG   cm_BufferStart;	/* start of memory for buffer: address/2 */
    ULONG   cm_DisplayStart;	/* start of memory for display: address/2 */
    LONG    cm_AttrToChar;	/* delta to apply to attr address/2 to */
				/*   find associated character address */
    UWORD   cm_BufferLines;	/* maximum rows in window */
    UWORD   cm_BufferWidth;	/* number of columns off-screen */
    UWORD   cm_BufferHeight;	/* number of rows off-screen */
    UWORD   cm_DisplayWidth;	/* number of columns in display */
    UWORD   cm_DisplayHeight;	/* number of rows in display */
    UWORD   cm_BufferXL;	/* X append loc in last ....Buffer line */
    UWORD   cm_BufferYL;	/* append ....Buffer line */
};

#define	cm_AllocBuffer	cm_AttrBufLines

#endif	/* DEVICES_CONMAP_H */
