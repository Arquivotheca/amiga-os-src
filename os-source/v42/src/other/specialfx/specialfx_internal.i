********************************************************************************
*
*	$Id: specialfx_internal.i,v 39.13 93/09/17 18:08:32 spence Exp $
*
********************************************************************************

	IFND	EXEC_TYPES_I
	include	'exec/types.i'
	ENDC
	IFND	EXEC_LISTS_I
	include	'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	include	'exec/nodes.i'
	ENDC
	IFND	SPECIALFX_SPECIALFX_I
	include	"SpecialFX.i"
	ENDC

ToFXHandle	equ	-4		; backpointer to the FXHandle
	
* The handle for the Special Effects:
    STRUCTURE FXHandle,0
	ULONG	fxh_Type		; will be SFX_...
	UWORD	fxh_Pad
	UWORD	fxh_Num
	ULONG	fxh_AllocSize
	ULONG	fxh_First
	ULONG	fxh_UCopList
	ULONG	fxh_Offset		; offset from the copperlist caches to the
					; first copper instruction of this effect.
	ULONG	fxh_BackPointer		; points to fxh_Type
    LABEL FxHandle_SIZEOF

* The handle for the Animation, created with DisplayFX()
    STRUCTURE AnimHandle,0
	ULONG	ah_View
	ULONG	ah_ViewPort
	ULONG	ah_HandleSize	; size of this handle
	ULONG	ah_HandleCount	; number of FXHandles in the list
	APTR	ah_Copper1L	; point to the two copperlists
	APTR	ah_Copper1S	; to use in doublebuffering the animation
	APTR	ah_Copper2L	; (Short and Long)
	APTR	ah_Copper2S
	ULONG	ah_Copper2LSize	; Size of the memory allocation for Copper2L
	ULONG	ah_Copper2SSize	; Size of the memory allocation for Copper2S
	UWORD	ah_CopperToUse	; 0 or 1.
	UWORD	ah_Flags
	; here is a list of the handles.
    LABEL AnimHandle_SIZEOF

    BITDEF	AH,CALCOFFSET,0	; set this to calculate the fxh_Offset in each
    				; fxh in the list.
    BITDEF	AH,NODBUFF,1	; set this to disable copperlist doublebuffering

* This is the data used internally in cc_Private:

    STRUCTURE   CCPrivate,cc_SIZEOF
	ULONG	Wait
	ULONG	con3_hi
	ULONG	pen_hi
	ULONG	con3_lo
	ULONG	pen_lo
	ULONG	Red32_orig
	ULONG	Green32_orig
	ULONG	Blue32_orig
	ULONG	Spare
    LABEL	CCPrivate_SIZEOF

* The LineControl handle has this data after the FXHandle.
    STRUCTURE LCHPrivate,fxh_BackPointer
	UWORD	lch_Shift	; 2 log lch_OffsetCount
	UWORD	lch_OffsetCount
	UWORD	lch_DxOffset
	UWORD	lch_Mask	; mask for lch_OffsetCount
	UWORD	lch_ZeroModOdd
	UWORD	lch_ZeroModEven
    LABEL	lch_SIZEOF	; backpointer is after the last lco_ entry

* This is a structure of the data stored for LineControl. There will be 64 of
* these on a AA system, and stored after the LCHPrivate structure.
* (The true number will be (16 << *(GfxBase->bwshifts[GfxBase->MemType])))

    STRUCTURE	lcOffsetTable,0
	UWORD	lco_DDFSTRT
	UWORD	lco_BPLCON1
	WORD	lco_BPLMOD1	; difference of the MOD values at this offset
	WORD	lco_BPLMOD2	; from the origin
	UWORD	lco_FMODE
	UWORD	lco_DDFSTOP
	WORD	lco_ModDiff	; ((16 * Bandwidth) / 8) = (Bandwidth * 2)
				; ie value to add to modulos for each whole
				; fetch cycle shifted. Store the value as
				; 2 log (bw * 2) so we can shift instead of
				; multiplying.
	UWORD	lco_CompensateEven	; compensation for not optimizing DDFSTRT, eg
	UWORD	lco_CompensateOdd	; changing DDFSTRT by 8 for 4 cycle Hires 1x.
	UWORD	lco_Pad
    LABEL	lco_SIZEOF

* The LineControl pointers point to this data:
    STRUCTURE	LCPrivate,lc_SIZEOF
	UWORD	lcp_Type
	UWORD	lcp_CopCount		; number of copper instructions
	STRUCT	lcp_AnimCache,(14*2)	; cache for AnimateFX(). Stores all
					; the data for the MOVE instructions.
    LABEL	lcp_SIZEOF

* These are the types, held in sclp_Type. They show whether the previous and next
* ScrollLines structure in the list are 1, 2 or >=3 lines away.
* They are not flags, but are added together to provide an offset into a LUT.
NEXT_LC_1	equ	0
NEXT_LC_2	equ	2
NEXT_LC_3G	equ	4
PREV_LC_1	equ	0
PREV_LC_2	equ	6

* The FineVideoControl handle has this data after the FXHandle
    STRUCTURE FVCHPrivate,fxh_BackPointer
	UWORD	fvch_BPLCON0
	UWORD	fvch_BPLCON2
	UWORD	fvch_BPLCON3
	UWORD	fvch_BPLCON4
	ULONG	fvch_Valid		; = -1 if the BPLCONx values are valid
	ULONG	fvch_BackPointer	; points to fxh_Type
    LABEL	fvch_SIZEOF

* This is the internal FineVideoControl structure atually used
    STRUCTURE FVCPrivate,fvc_SIZEOF
	UWORD	fvcp_BPLCON2		; value of CON2 after AnimateFX()
	UWORD	fvcp_BPLCON0		; value of CON0 after AnimateFX()
	UWORD	fvcp_BPLCON4		; value of CON4 after AnimateFX()
	UWORD	fvcp_BPLCON3		; value of CON3 after AnimateFX()
     LABEL	fvcp_SIZEOF

* The SpriteControl handle has this data after the FXHandle
    STRUCTURE SCHPrivate,fxh_BackPointer
	UWORD	sch_To35ns		; shift the ss_x value by this for
					; 35ns pixel resolution.
	UWORD	sch_PtrOffset		; add this to the PTR values
	ULONG	sch_BackPointer		; points to fxh_Type
    LABEL	sch_SIZEOF

* This is the internal SpriteControl structure actually used.
    STRUCTURE	scPrivate,spc_SIZEOF
	UWORD	scp_OrigPos		; original Pos data
	UWORD	scp_OrigCtl		; original Ctl data
	UWORD	scp_Orig_x		; original ss_x
	UWORD	scp_Orig_y		; original ss_y
	UWORD	scp_OrigX35ns		; original x position in 35ns resolution
	UWORD	scp_OrigYLine		; original y line value
	UWORD	scp_CopIns		; copperinstruction last used
					; by this SpriteControl effect
	UWORD	scp_CopData		; And the value of the copperins
	STRUCT	scp_CopCache,12		; cache SPRxCTL, PTH and PTL
    LABEL	scp_SIZEOF


; Data held on the stack in InstallFX()
    STRUCTURE	StackStuff,0
	ULONG	stk_SpecialFXBase
	ULONG	stk_TagPtr
	ULONG	stk_View
	ULONG	stk_ViewPort
	ULONG	stk_DispHandle
	APTR	stk_ErrorCode
	UWORD	stk_BPLCON3
	UWORD	stk_WholeOdd
	UWORD	stk_WholeEven
	UWORD	stk_ZeroModOdd
	UWORD	stk_ZeroModEven
	UWORD	stk_BPLCON0
	UWORD	stk_ZeroDDFSTOP
	UWORD	stk_DPFDDFSTRT
	UWORD	stk_OddOffset
	UWORD	stk_EvenOffset
	ULONG	stk_ZeroBPL1PTR
	ULONG	stk_ZeroBPL2PTR
	APTR	stk_OrigHandle
    LABEL	FRAME_SIZE

dbug	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.l \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	8(a7),a7
	bra.s	b\@
a\@: dc.b	'%lx ',0,0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

dbugw	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.w \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	6(a7),a7
	bra.s	b\@
a\@: dc.b	'%x ',0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

print	macro	string
	xref	KPutStr
	movem.l	a0/a1/d0/d1,-(a7)
	lea	mystring\@(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0/a1/d0/d1
	bra.s	skip\@
mystring\@:
	dc.b	\1,0
	cnop	0,2
skip\@:
	endm

COLOUR0	equ	$dff180
BPLMOD1	equ	$dff108
BPLMOD2	equ	$dff10a
BPLCON0	equ	$dff100
BPLCON1	equ	$dff102
BPLCON2	equ	$dff104
BPLCON3	equ	$dff106
BPLCON4	equ	$dff10c
DDFSTRT equ	$dff092
DDFSTOP	equ	$dff094
FMODE	equ	$dff1fc
BPL1PTH	equ	$dff0e0
BPL2PTH	equ	$dff0e4
INTREQ	equ	$dff09c
INTENA	equ	$dff09a
INTENAR	equ	$dff01c
SPR0PTH	equ	$dff120
SPR0POS equ	$dff140
SPRPOSOFF	equ	(SPR0POS-SPR0PTH)

CMOVE	MACRO	;reg,value
	movem.l	d0/d1,-(sp)
	move.l	a2,a1			; a2 -> UCopList
	move.l	\1,d0
	move.l	\2,d1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	movem.l	(sp)+,d0/d1
	ENDM

ABSW	MACRO	; reg
	tst.w	\1
	bpl.s	a\@
	neg.w	\1
a\@:
	ENDM

ABSL	MACRO	; reg
	tst.l	\1
	bpl.s	b\@
	neg.l	\1
b\@:
	ENDM
