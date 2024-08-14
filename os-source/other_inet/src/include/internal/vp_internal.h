/******************************************************************************
*
*	$Id: vp_internal.h,v 39.18 92/08/06 11:38:39 spence Exp $
*
******************************************************************************/

#ifndef	GRAPHICS_VPINTERNAL_H
#define	GRAPHICS_VPINTERNAL_H

/* Arguments all driver routines take: */
#define DRIVER_PARAMS struct View *v, struct ViewPort *vp, struct ViewPortExtra *vpe

/* The BuildData structures */

struct BuildData
{
	struct	Rectangle cliposcan;
	struct  ProgInfo *pinfo;
	struct	MonitorSpec *mspc;
	struct	CopIns *c;
	struct  CopIns *firstwait;
	LONG	Offset;		/* in bitmap */
	LONG	Offset2;	/* in 2nd bitmap */
	UWORD	DiwStrtX;
	UWORD	DiwStopX;
	UWORD	DiwStrtY;
	UWORD	DiwStopY;
	UWORD	DDFStrt;
	UWORD	DDFStop;
	UWORD	Modulo;
	UWORD	FnCount;
	UWORD	bplcon0;
	UWORD	bplcon1;
	UWORD	bplcon2;	/* for Playfield Prioritys & for genloc stuff */
	UWORD	bplcon3;	/* for genloc stuff */
	UWORD	bplcon4;	/* colour table offsets */
	UWORD   Modulo2;
	UWORD   FMode;
	UWORD 	FudgedFMode;	/* for those oscan fudges */
	UWORD	Index;		/* index into the LUTs */
	UWORD	Cycle;		/* cycle type */
	UWORD	FirstFetch;	/* first fetch needed */
	BOOL	LHSFudge;	/* shows we need to fudge the LHS */
	BOOL	RHSFudge;
	UBYTE	Scroll;		/* for bplcon1 */
	UBYTE	Flags;
	UBYTE	Scroll2;
	UBYTE	ToViewY;
	UWORD	RGADiwStopYL;	/* DiwStopY written for Long Frame */
	UWORD	RGADiwStopYS;	/* DiwStopY written for Short Frame */
};

/* Flags for the BD structures */

#define BD_FUDGEDFMODE  1
#define BD_IS_DPF	2
#define	BD_IS_LACE	4
#define BD_IS_SDBL	8
#define BD_IS_A2024	16
#define BD_VIDEOOSCAN	32

#define DSPINS_COUNTAA	256
#define DSPINS_COUNTA	64

#define MVP_NO_CM 10

#define CYCLE_2	0
#define CYCLE_4 1
#define CYCLE_8 2
#define CYCLE_16 3
#define CYCLE_32 4

struct ProgInfo
{
	UWORD	bplcon0;	/* minimum needed for this mode */
	UWORD	bplcon2;	/* has KILLEHB flag, PF2PRI, and SOGEN */
	UBYTE	ToViewX;	/* convert from VP horizontal resoliution to View */
	UBYTE	pad;
	UWORD	Flags;		/* see below */
	UWORD	MakeItType;
	UWORD	ScrollVPCount;
	UWORD	DDFSTRTMask;	/* for 1x, 2x or 4x */
	UWORD	DDFSTOPMask;
	UBYTE	ToDIWResn;	/* convert from the stored DisplayWindow resolution to hardware resolution */
	UBYTE	Offset;		/* for calculating offsets */
};

#define PROGINFO_NATIVE		0x0001
#define PROGINFO_VARBEAM	0x0002
#define PROGINFO_SHIFT3		0x0010
#define PROGINFO_SHIFT5		0x0020
#define PROGINFO_SCANDBL	0x0040
#define PROGINFO_HAM		0x0080

struct VecTable
{
	APTR	MoveSprite;
	APTR	ChangeSprite;
	APTR	ScrollVP;
	APTR	BuildVP;
	APTR	PokeColors;
	/* extendable */
};

#define GOOD_CLIPSTUFF 1
#define BAD_CLIPSTUFF 0

/* Chip Bugs. These bits are set in GfxBase->Bugs by cpwrup.c. They should
 * be cleared (when appropriate) by a SetPatch.
 */

#define BUG_ALICE_LHS	0x01
#define BUG_ALICE_RHS	0x02
#define BUG_VBLANK	0x04
#define BUG_H0		0x08
#define CHANGE		0x10	/* not really a bug, but write vbstrt etc in vblank */ 
#define BUG_ALICE_LHS_PROG	0x20

/* For FMode register */

#define FMB_BSCAN2	14
#define FMB_SSCAN2	15
#define FMF_BSCAN2	(1 << FMB_BSCAN2)
#define FMF_SSCAN2	(1 << FMB_SSCAN2)

#endif
