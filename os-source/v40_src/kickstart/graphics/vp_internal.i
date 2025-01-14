******************************************************************************
*
*	$Id: vp_internal.i,v 39.15 93/04/15 16:44:43 chrisg Exp $
*
******************************************************************************

	IFND	GRAPHICS_VPINTERNAL_I
GRAPHICS_VPINTERNAL_I	SET	1

	IFND	EXEC_TYPES_I
	include	'exec/types.i'
	ENDC

	IFND	GRAPHICS_GFX_I
	include 'graphics/gfx.i'
	ENDC

* Arguments all driver routines take

* The BuildData structures

    STRUCTURE	BuildData,0
	STRUCT	bd_cliposcan,ra_SIZEOF
	APTR	bd_pinfo
	APTR	bd_mspc
	APTR	bd_c
	APTR	bd_firstwait
	LONG	bd_Offset
	LONG	bd_Offset2
	UWORD	bd_DiwStrtX
	UWORD	bd_DiwStopX
	UWORD	bd_DiwStrtY
	UWORD	bd_DiwStopY
	UWORD	bd_DDFStrt
	UWORD	bd_DDFStop
	UWORD	bd_Modulo
	UWORD	bd_FnCount
	UWORD	bd_bplcon0
	UWORD	bd_bplcon1
	UWORD	bd_bplcon2
	UWORD	bd_bplcon3
	UWORD	bd_bplcon4
	UWORD	bd_Modulo2
	UWORD	bd_FMode
	UWORD	bd_FudgedFMode
	UWORD	bd_Index
	UWORD	bd_Cycle
	UWORD	bd_FirstFetch
	BOOL	bd_LHSFudge
	BOOL	bd_RHSFudge
	UBYTE	bd_Scroll
	UBYTE	bd_Flags
	UBYTE	bd_Scroll2
	UBYTE	bd_ToViewY
	UWORD	bd_RGADiwStopYL
	UWORD	bd_RGADiwStopYS
    LABEL bd_SIZEOF

BD_FUDGEDFMODE	EQU	1
BD_IS_DPF	EQU	2
BD_IS_LACE	EQU	4
BD_IS_SDBL	EQU	8
BD_IS_A2024	EQU	16
BD_VIDEOOSCAN	EQU	32

DSPINS_COUNTA	EQU	80
DSPINS_COUNTAA	EQU	(DSPINS_COUNTA+32)

MVP_NO_CM	EQU	10


    STRUCTURE ProgInfo,0
	UWORD	pgi_bplcon0
	UWORD	pgi_bplcon2
	UBYTE	pgi_ToViewX
	UBYTE	pgi_pad
	UWORD	pgi_Flags
	UWORD	pgi_MakeItType
	UWORD	pgi_ScrollVPCount
	UWORD	pgi_DDFSTRTMask
	UWORD	pgi_DDFSTOPMask
	UBYTE	pgi_ToDIWResn
	UBYTE	pgi_Offset
    LABEL pgi_SIZEOF

PROGINFO_NATIVE		equ	$0001
PROGINFO_VARBEAM	equ	$0002
PROGINFO_SHIFT3		equ	$0010
PROGINFO_SHIFT5		equ	$0020
PROGINFO_SCANDBL	equ	$0040
PROGINFO_HAM		equ	$0080

    STRUCTURE VecTable,0
	APTR	vt_MoveSprite
	APTR	vt_ChangeSprite
	APTR	vt_ScrollVP
	APTR	vt_BuildVP
	APTR	vt_PokeColors
    LABEL vt_SIZEOF

    BITDEF	BM,BUG_ALICE_LHS,0
    BITDEF	BM,BUG_ALICE_RHS,1
    BITDEF	BM,BUG_VBLANK,2
    BITDEF	BM,BUG_H0,3
    BITDEF	BM,CHANGE,4
    BITDEF	BM,BUG_ALICE_LHS_PROG,5

    BITDEF	FM,BSCAN2,14
    BITDEF	FM,SSCAN2,15

	ENDC
