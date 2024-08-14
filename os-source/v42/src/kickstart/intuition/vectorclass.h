

#ifndef INTUITION_VECTORCLASS_H
#define	INTUITION_VECTORCLASS_H TRUE

/*** vectorclass.h *********************************************************
 *
 *  Vector image class interface
 *
 *  $Id: vectorclass.h,v 40.0 94/02/15 17:51:07 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1990, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 **************************************************************************/

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef INTUITION_IMAGECLASS_H
#include <intuition/imageclass.h>
#endif

#define	VECTORCLASS "vectoriclass"

/* see imageclass.h for additional tags */

struct Vector
{
    UBYTE v_Flags;		/* Flags (see below) */
    UBYTE v_States;		/* Image states that this vector applies to */
    WORD v_DataOffset;		/* Short pointer to data */
};

/* values of v_Flags */
#define VF_PENMASK	0x0F	/* Pen number goes into bottom nybble */

#define	VF_TEXT		(TEXTPEN)	/* 0x02 */
#define	VF_SHINE	(SHINEPEN)	/* 0x03 */
#define	VF_SHADOW	(SHADOWPEN)	/* 0x04 */
#define	VF_FILL		(FILLPEN)	/* 0x05 */
#define	VF_FILLTEXT	(FILLTEXTPEN)	/* 0x06 */
#define	VF_BACKG	(BACKGROUNDPEN)	/* 0x07 */

#define	VF_BDETAIL	(BARDETAILPEN)	/* 0x09 */
#define	VF_BBLOCK	(BARBLOCKPEN)	/* 0x0A */

#define	VF_MONO		0x10	/* Vector is used for monochrome rendering */
#define	VF_COLOR	0x20	/* Vector is used for color rendering */

#define VF_LINED	0x00	/* NB THIS VALUE IS ZERO */
#define	VF_FILLED	0x40	/* filled shape, not outline */
#define VF_RECTANGLE	0x00	/* NB THIS VALUE IS ZERO */
#define VF_POLYGON	0x80	/* polygon, not rectangle */

#define VF_LINERECT	(VF_LINED | VF_RECTANGLE)
#define VF_FILLRECT	(VF_FILLED | VF_RECTANGLE)
#define VF_LINEPOLY	(VF_LINED | VF_POLYGON)
#define VF_FILLPOLY	(VF_FILLED | VF_POLYGON)


/* v_States can take the following possible values:
 * (NB: These are a collapsed subset of the IDS_ states)
 */

#define	VSB_NORMAL	0			/* IDS_NORMAL */
#define	VSB_SELECTED	1			/* IDS_SELECTED */
#define	VSB_INANORMAL	2			/* IDS_INACTIVENORMAL */

#define	VS_NORMAL	(1L << VSB_NORMAL)
#define	VS_SELECTED	(1L << VSB_SELECTED)
#define	VS_INANORMAL	(1L << VSB_INANORMAL)

/* Images for Window borders have this particular combination: */
#define	VS_WBORDERIMG	(VS_NORMAL | VS_SELECTED | VS_INANORMAL)

#define VS_NUMSTATES	(3)


/* A VectorInfo structure contains all the information about a vector
 * object, including dimensions, flags, border treatment, and a pointer
 * to the actual vectors to use when creating its imagery.
 */
struct VectorInfo
{
    BYTE vi_Width[3];		/*  Med, low, hi */
    BYTE vi_Height[3];		/*  Med, low, hi */

    struct Vector *vi_VList;
    BYTE vi_DesignWidth;
    BYTE vi_DesignHeight;
    BYTE vi_VCount;
    UBYTE vi_States;		/* States we want generated */
    UBYTE vi_Border;		/* Special border treatment for this guy */
    UBYTE vi_Flags;		/* Flags: see below */
};

/* VectorInfo shares vi_States values with the Vector v_States */

/* VectorInfo vi_Flags: */

#define VIF_WMULTIPLY		0x0F	/* Multiplier for width (VIF_REFFONT) */
#define VIF_REFFONT		0x10	/* Allows reference font to be spec'd */
#define VIF_SPECIFYWIDTH	0x20	/* IA_Width accepted */
#define VIF_SPECIFYHEIGHT	0x40	/* IA_Height accepted */


/* VectorInfo vi_Border treatments: */

#define VIB_HORIZ	0x01	/* horizontal border */
#define	VIB_HLEFT	0x02	/* horizontal border, left side */
#define	VIB_HRIGHT	0x04	/* horizontal border, right side */
#define	VIB_VERT	0x08	/* vertical border */
#define VIB_BRCORN	0x10	/* bottom-right corner */

#define	VIB_3D		0x20	/* 3D border treatment */
#define VIB_THICK3D	0x40	/* 3D border with double-thick sides */
#define VIB_INMENU	0x80	/* background is a menu pane */


#endif /* INTUITION_VECTORCLASS_H */
