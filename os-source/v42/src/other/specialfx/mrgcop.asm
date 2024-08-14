********************************************************************************
*
*	$Id: MrgCop.asm,v 39.2 93/09/21 16:54:15 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/lists.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'graphics/gfxbase.i'

	include	"SpecialFXBase.i"
	include	"specialfx.i"
	include	"specialfx_internal.i"

	xdef	SfxBase
	xdef	SFXMrgCop
	xref	_LVOAllocMem
	xref	_LVOFreeMem
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOCopyMemQuick
	xref	_LVOWaitTOF
	xref	_intena

* SFXBase will contain a pointer to the SfxBase. This is filled by the library
* Init routine, and is valid until the library is expunged, at which point this
* pointer is no longer used anyway!
*
* WARNING: This will not work if SpecialFX.library ever ends up in ROM!

SfxBase:	dc.l	0

ON	equ	0
OFF	equ	1
DODEBUGCOP	equ	OFF

	IFEQ	DODEBUGCOP
DEBUGCOP	MACRO
	print	"Copper1L = "
	dbug	ah_Copper1L(a5)
	print	"Copper1S = "
	dbug	ah_Copper1S(a5)
	print	"Copper2L = "
	dbug	ah_Copper2L(a5)
	print	"Copper2S = "
	dbug	ah_Copper2S(a5)
	print	10
	ENDM
	ELSE
DEBUGCOP	MACRO
	ENDM
	ENDC


*
* SFXMrgCop() calls the graphics.library MrgCop() LVO, and then
* duplicates the copperlist just built. It then walks through the list of
* known AnimHandles, and checks them against this View. If they are the
* same, then the AnimHandles' copper pointers are updated.
*
* The copy of the old copperlist is then Freed.
*

SFXMrgCop:
	move.l	SfxBase(pc),a0
	move.l	sfxb_OldMrgCop(a0),a0
	move.l	a1,-(sp)	; save the View
	jsr	(a0)		; call the old MrgCop. d0 = return value
	move.l	(sp)+,a1

	movem.l	d0/d3-d4/a2-a6,-(sp)
	sub.l	a4,a4		; a4 -> first AnimHandle found that matches this View
	move.l	SfxBase(pc),a2
	move.l	a1,d3

	; get the AnimHandle semaphore
	move.l	sfxb_AHListSemaphore(a2),a0
	move.l	sfxb_ExecBase(a2),a6
	jsr	_LVOObtainSemaphore(a6)

	; walk through the AnimHandle list, looking for an AnimHandle with
	; the same View. If we find one, copy the copperlist(s).
	lea	sfxb_AHList(a2),a0
search_ahlist:
	lea	ahl_AnimHandles(a0),a3
	move.w	ahl_Count(a0),d4
	bra.s	search_ahlist.
search_ahlist_next:
	move.l	(a3)+,d0
	beq.s	search_ahlist.
	move.l	d0,a1		; the AnimHandle
	cmp.l	ah_View(a1),d3	; the same View?
	beq.s	found_ah
search_ahlist.:
	dbra.s	d4,search_ahlist_next
	; wasn't in that list, so check the next.
	move.l	MLN_SUCC(a0),a0
	TSTLIST	a0
	bne.s	search_ahlist
	; is empty
	bra	SFXMrgCop.

found_ah:
	; we have found a matching AnimHandle.
	move.l	a0,-(sp)
	move.l	d3,a1		; get the View
	move.l	-4(a3),a5
	move.l	a4,d0		; is it the first AnimHandle match?
	bne.s	no_duping
	btst.b	#AHB_NODBUFF,ah_Flags+1(a5)	; do we want dbuffering?
	bne.s	no_duping
	move.l	a5,a4
	move.l	v_LOFCprList(a1),d0
	beq.s	no_LOF_dup
	move.l	d0,a0
	move.l	a0,-(sp)
	moveq	#0,d0
	move.w	crl_MaxCount(a0),d0
	asl.w	#2,d0			; (* 4)
	move.l	d0,sfxb_CopperLSize(a2)
	move.l	#(MEMF_CHIP|MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	(sp)+,a0
	move.l	d0,sfxb_CopperLCache(a2)	; (protected by the Semaphore)
	beq	ERR_NO_LOF_MEM
	move.l	d0,a1		; dest
	moveq	#0,d0
	move.w	crl_MaxCount(a0),d0	; size
	asl.w	#2,d0			; (* 4)
	move.l	crl_start(a0),a0	; source
	jsr	_LVOCopyMemQuick(a6)	; duplicate.
no_LOF_dup:
	move.l	d3,a1		; get the View pointer	
	move.l	v_SHFCprList(a1),d0
	beq.s	no_SHF_dup
	move.l	d0,a0
	move.l	a0,-(sp)
	moveq	#0,d0
	move.w	crl_MaxCount(a0),d0
	asl.w	#2,d0			; (* 4)
	move.l	d0,sfxb_CopperSSize(a2)
	move.l	#(MEMF_CHIP|MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	(sp)+,a0
	move.l	d0,sfxb_CopperSCache(a2)
	beq	ERR_NO_SHF_MEM
	move.l	d0,a1		; dest
	moveq	#0,d0
	move.w	crl_MaxCount(a0),d0	; size
	asl.w	#2,d0			; (* 4)
	move.l	crl_start(a0),a0	; source
	jsr	_LVOCopyMemQuick(a6)	; duplicate.
no_SHF_dup:
no_duping:
	; put the Copper pointers in the AnimHandle, but
	; first FreeMem() the previous copies.
	; We need to ensure that we are not Freeing the copperlists as they
	; are being viewed!
	move.l	a2,-(sp)
	move.l	sfxb_GfxBase(a2),a2
	DISABLE
	move.l	d3,a0			; Get the View
	move.l	v_LOFCprList(a0),d0
	beq.s	1$
	move.l	d0,a1
	move.l	crl_start(a1),gb_LOFlist(a2)
1$:
	btst.b	#2,gb_Modes+1(a2)	; are we laced?
	beq.s	2$
	move.l	v_SHFCprList(a0),d0
	beq.s	2$
	move.l	d0,a1
	move.l	crl_start(a1),gb_SHFlist(a2)
2$:
	ENABLE
	exg	a2,a6			; exchg GfxBase<->ExecBase
	jsr	_LVOWaitTOF(a6)		; make sure the original copperlists
					; are being used.
	jsr	_LVOWaitTOF(a6)
	move.l	a2,a6			; restore ExecBase in a6
	move.l	(sp)+,a2		; and restore SfxBase in a2

	move.l	ah_Copper2L(a5),d0
	beq.s	no_cop2L_free
	move.l	d0,a1
	move.l	ah_Copper2LSize(a5),d0
	beq.s	no_cop2L_free
	jsr	_LVOFreeMem(a6)
no_cop2L_free:
	move.l	ah_Copper2S(a5),d0
	beq.s	no_cop2S_free
	move.l	d0,a1
	move.l	ah_Copper2SSize(a5),d0
	beq.s	no_cop2S_free
	jsr	_LVOFreeMem(a6)
no_cop2S_free:
	move.l	d3,a1			; get the View back.
	move.l	v_LOFCprList(a1),d0
	beq.s	no_LOF_view
	move.l	d0,a0
	move.l	crl_start(a0),d0
no_LOF_view:
	move.l	d0,ah_Copper1L(a5)
	btst.b	#AHB_NODBUFF,ah_Flags+1(a5)
	beq.s	1$
	move.l	d0,ah_Copper2L(a5)	; use the same copperlist
	clr.l	ah_Copper2LSize(a5)	; but don't free it next time.
	bra.s	2$
1$:
	move.l	sfxb_CopperLCache(a2),ah_Copper2L(a5)
	move.l	sfxb_CopperLSize(a2),ah_Copper2LSize(a5)
2$:
	clr.l	sfxb_CopperLCache(a2)
	move.l	v_SHFCprList(a1),d0
	beq.s	no_SHF_view
	move.l	d0,a0
	move.l	crl_start(a0),d0
no_SHF_view:
	move.l	d0,ah_Copper1S(a5)
	btst.b	#AHB_NODBUFF,ah_Flags+1(a5)
	beq.s	1$
	move.l	d0,ah_Copper2S(a5)	; use the same copperlist
	clr.l	ah_Copper2SSize(a5)	; but don't free it next time.
	bra.s	2$
1$:
	move.l	sfxb_CopperSCache(a2),ah_Copper2S(a5)
	move.l	sfxb_CopperSSize(a2),ah_Copper2SSize(a5)
2$:
	clr.l	sfxb_CopperSCache(a2)
	clr.w	ah_CopperToUse(a5)
	DEBUGCOP
	or.w	#AHF_CALCOFFSET,ah_Flags(a5)
	move.l	(sp)+,a0
	bra	search_ahlist.		; go back and look for another AnimHandle.

SFXMrgCop.:
	; release the Semaphore
	move.l	sfxb_AHListSemaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)
	movem.l	(sp)+,d0/d3-d4/a2-a6

	rts

* error routines don't do anything useful yet!!
* When they do, AnimateFXA() will need to check for NULL copperlists.
ERR_NO_SHF_MEM:
	move.l	sfxb_CopperLCache(a2),d0
	beq.s	ERR_NO_LOF_MEM
	move.l	d0,a1
	move.l	d3,a0		; the View
	move.l	v_LOFCprList(a0),a0
	moveq	#0,d0
	move.w	crl_MaxCount(a0),d0
	jsr	_LVOFreeMem(a6)
ERR_NO_LOF_MEM:
	move.l	(sp)+,a0	; pop the next entry in the list off the stack
	bra.s	SFXMrgCop.
