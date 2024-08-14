********************************************************************************
*
*	$Id: InstallFX.asm,v 39.10 93/09/14 18:51:50 spence Exp $
*
********************************************************************************

	opt	p=68020
	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'graphics/view.i'
	include	'graphics/copper.i'
	include	'graphics/gfxbase.i'
	include	'utility/tagitem.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	_LVOInstallFXA
	xdef	DoUCopStuff
	xref	_LVONextTagItem
	xref	_LVOFindTagItem
	xref	_LVOAllocMem
	xref	_LVOFreeMem
	xref	_LVOAttemptSemaphore
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOPermit
	xref	_LVOFreeCopList
	xref	_LVOUCopperListInit

******* specialfx.library/InstallFXA *******************************************
*
*   NAME
*	InstallFXA -- Installs the initial settings of the Effect
*	InstallFX -- varargs stub for InstallFXA()
*
*   SYNOPSIS
*	DisplayHandle = InstallFXA(View, ViewPort, TagItems)
*	d0                         a0    a1        a2
*
*	APTR InstallFXA(struct View *, struct ViewPort *,
*	                APTR *, struct TagItem *);
*
*	DisplayHandle = InstallFX(View, ViewPort, Tag1, ...)
*
*	APTR InstallFX(struct View *, struct ViewPort *, ULONG, ...);
*
*   FUNCTION
*	To install the intial settings of the list of special effects in the
*	ViewPort.
*
*   INPUTS
*	View          - The View of the display
*	ViewPort      - The ViewPort of the display
*	TagItems      - A pointer to an array of SFX_ tags.
*
*   TAGS
*	SFX_InstallEffect - ti_Data is an FXHandle obtained from AllocFX()
*
*	SFX_InstallErrorCode - ti_Data points to a ULONG that will contain
*	                       an error code if DisplayHandle is NULL.
*
*   RESULT
*	DisplayHandle - a DisplayHandle if succesful, else NULL, in which case
*	                the ULONG pointed to by SFX_InstallErrorCode
*	                (if used) will contain an error number.
*	                DisplayHandle will be passed to AnimateFX() for
*	                animating the display.
*
*   NOTES
*	After calling InstallFXA(), you should remake the display, either
*	with intuition's MakeScreen()/RethinkDisplay() if using intuition
*	screens, or with graphics' MakeVPort()/MrgCop()/LoadView() if using
*	a custom View and ViewPort.
*
*	This function will not work with a UserCopperList. Either use
*	a UserCopperList for your own tricks, or use specialfx.library, but
*	do not mix-and-match.
*
*   SEE ALSO
*	<libraries/specialfx.h> AllocFX() AnimateFX() RemoveFX()
*
********************************************************************************

_LVOInstallFXA:
	movem.l	d2-d7/a2-a6,-(sp)
	; Save some info on the stack
	sub.l	#FRAME_SIZE,sp
	move.l	sp,a5
	clr.l	stk_ErrorCode(a5)
	move.l	a6,stk_SpecialFXBase(a5)
	move.l	a2,stk_TagPtr(a5)
	movem.l	a0-a1,stk_View(a5)	; save View and ViewPort

	; see if another task is animating
	move.l	a6,a4
	move.l	sfxb_AnimationSemaphore(a4),a0
	move.l	sfxb_ExecBase(a6),a6
	jsr	_LVOAttemptSemaphore(a6)
	move.l	a4,a6
	tst.l	d0
	beq	lock_error

	; ensure the ViewPort is not remade during the operation
	move.l	a6,a4
	move.l	sfxb_GfxBase(a6),a0
	move.l	gb_ActiViewCprSemaphore(a0),a0
	move.l	sfxb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	a4,a6

	; **********************************************************************
	;
	; Each SpecialEffect needs its own UserCopperList, so iterate
	; through the taglist and allocate a UCopList for each tag. If
	; any allocation fails, then they must all be freed.
	;
	; **********************************************************************
* {

	moveq	#0,d5			; d5 will count the number of tag items
	move.l	d5,d7			; bit 31 is set to disable copper dbuffering
	move.l	sfxb_GfxBase(a6),a4
	move.l	sfxb_UtilityBase(a6),a6
	move.l	vp_UCopIns(a1),d2	; store the original UCopIns pointer
	lea	stk_TagPtr(a5),a3
calc_loop:
	move.l	a3,a0
	jsr	_LVONextTagItem(a6)
	tst.l	d0
	beq.s	tags_done
	move.l	d0,a0
	move.l	ti_Tag(a0),d0
	and.l	#~$80001000,d0
	bne.s	1$
	; must be SFX_InstallEffect
	move.l	ti_Data(a0),d0
	beq.s	calc_loop
	move.l	d0,a0			; a0 -> FxHandle from AllocFX(). The 
					; FxHandle structure is *before* this
					; pointer
	move.l	ToFXHandle(a0),a0	; BackPointer
	move.l	fxh_Type(a0),d0
	and.l	#~$80000000,d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	stk_SpecialFXBase(a5),a1
	move.l	sfxb_CountVectors(a1),a1
	move.l	-4(a1,d0),a1
	jsr	(a1)
	addq.l	#1,d5
	bra.s	calc_loop
1$:
	subq.w	#1,d0
	bne.s	install_dbuff
	; must be SFX_InstallErrorCode
	move.l	ti_Data(a0),stk_ErrorCode(a5)	; for error passing
	bra.s	calc_loop
install_dbuff:
	; is SFX_DoubleBuffer
	move.l	ti_Data(a0),d7
	beq.s	calc_loop		; if FALSE, set d7 to 0,
	moveq	#-1,d7			; else set d7 to -1
	bra.s	calc_loop

* 	{
tags_done:
	move.l	a2,stk_TagPtr(a5)	; restore

	; allocate the RAM for the AnimHandle
	moveq	#AnimHandle_SIZEOF,d0
	move.l	d5,a2			; safe keeping
	asl.l	#2,d5
	add.l	d5,d0			; 4 bytes per SFX handle
	move.l	d0,d5
	move.l	#(MEMF_PUBLIC|MEMF_CLEAR),d1
	move.l	gb_ExecBase(a4),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq	FAILURE_0
	move.l	d0,a3
	add.l	#AnimHandle_SIZEOF,d0
	move.l	d0,stk_DispHandle(a5)	; Points to the list of FXHandles
	beq	FAILURE_0
	move.l	stk_View(a5),ah_View(a3)
	move.l	stk_ViewPort(a5),ah_ViewPort(a3)
	move.l	d5,ah_HandleSize(a3)
	move.l	a2,ah_HandleCount(a3)
	tst.l	d7
	bmi.s	1$
	or.w	#AHF_NODBUFF,ah_Flags(a3)
1$:

	; checkpoint
	; a3 -> AnimHandle
	; a4 -> GfxBase
	; a5 -> stack frame
	; a6 -> ExecBase

	; Look through the DspIns for the last bplcon3 value.
	moveq	#0,d2			; store bplcon3 here
	move.l	stk_ViewPort(a5),a0
	move.l	vp_DspIns(a0),d0
	beq.s	got_con3
	move.l	d0,a0
	move.w	cl_Count(a0),d7
	move.l	cl_CopIns(a0),d0
	beq.s	got_con3
	move.l	d0,a0
	move.w	#(BPLCON3&$fff),d3
	bra.s	get_bplcon3.
get_bplcon3:
	move.w	(a0)+,d0		; opcode
	move.l	(a0)+,d1		; address and data
	cmp.b	#0,d0
	bne.s	get_bplcon3.
	swap	d1
	cmp.w	d3,d1
	bne.s	get_bplcon3.		; not a MOVE
	swap	d1
	move.w	d1,d2			; store the bplcon3
get_bplcon3.:
	dbra.s	d7,get_bplcon3
	and.w	#%0001110111111111,d2	; save the bits of bplcon3 we don't want to touch
got_con3:
	move.w	d2,stk_BPLCON3(a5)

	; now walk through the taglist again, and build the copperlist
	move.l	gb_UtilBase(a4),a6
	lea	stk_TagPtr(a5),a0
	movem.l	a0/a6,-(sp)
build_loop:
	movem.l	(sp),a0/a6
	jsr	_LVONextTagItem(a6)
	tst.l	d0
	beq.s	tags_done2
	move.l	d0,a0
	move.l	ti_Tag(a0),d0
	and.l	#~$80001000,d0
	bne.s	build_loop
	; must be SFX_InstallEffect
	move.l	ti_Data(a0),d0
	beq	calc_loop
	move.l	d0,a0			; a0 -> FxHandle from AllocFX(). The 
					; FxHandle structure is *before* this
					; pointer
	move.l	ToFXHandle(a0),a0	; BackPointer
	move.l	fxh_Type(a0),d0
	and.l	#~$80000000,d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	stk_SpecialFXBase(a5),a1
	move.l	sfxb_InstallVectors(a1),a1
	move.l	-4(a1,d0),a1
	jsr	(a1)
	bra.s	build_loop

tags_done2:
*	}

	; now store this AnimHandle in our list.
	; a3 has the AnimHandle.

	; first, get the Semaphore
	move.l	stk_SpecialFXBase(a5),a4
	move.l	sfxb_ExecBase(a4),a6
	move.l	sfxb_AHListSemaphore(a4),a0
	jsr	_LVOObtainSemaphore(a6)
	lea	sfxb_AHList(a4),a0
search_ahlist:
	move.w	ahl_Count(a0),d0
	cmp.w	#MAX_AHLIST,d0
	beq.s	nxt_ahlist_chk
	addq.w	#1,d0
	move.w	d0,ahl_Count(a0)
	lea	ahl_AnimHandles(a0),a1
10$:
	tst.l	(a1)+
	bne.s	10$
	move.l	a3,-4(a1)

	move.l	sfxb_AHListSemaphore(a4),a0
	jsr	_LVOReleaseSemaphore(a6)

	moveq	#0,d0
	addq.l	#8,sp			; a0/a6
_InstallFXA.:
	; release the semaphore
	move.l	d0,d2
	move.l	stk_SpecialFXBase(a5),a6
	move.l	sfxb_GfxBase(a6),a0
	move.l	gb_ActiViewCprSemaphore(a0),a0
	move.l	sfxb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
ret_err:
	move.l	stk_ErrorCode(a5),d0
	beq.s	1$
	move.l	d0,a0
	move.l	d2,(a0)			; store the error code
1$:

	move.l	a3,d0			; return the AnimHandle
clean_up:
	add.l	#FRAME_SIZE,sp
	movem.l	(sp)+,a2-a6/d2-d7
	rts

nxt_ahlist_chk:
	move.l	MLN_SUCC(a0),a0
	tst.l	MLN_SUCC(a0)	; end of the list
	bne.s	search_ahlist
new_ahlist:
	moveq	#ahl_SIZEOF,d0
	move.l	#(MEMF_CLEAR+MEMF_PUBLIC),d1	
	move.l	sfxb_ExecBase(a4),a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,d2
	beq.s	NO_AHLIST_ERR
	move.l	d0,a1
	lea	sfxb_AHListHead(a4),a0
	ADDHEAD	a0,a1
	move.l	d2,a0
	bra	search_ahlist

NO_AHLIST_ERR:
	; Low on memory, so we need to undo all the UCopList stuff
	; NYI!!
	bra.s	_InstallFXA.

; If the AnimationSemaphore is already locked, return an error SFX_ERR_Animating.
lock_error:
	; Look for a SFX_InstallErrorCode
	move.l	sfxb_UtilityBase(a6),a6
	move.l	#SFX_InstallErrorCode,d0
	move.l	a2,a0
	jsr	_LVOFindTagItem(a6)
	tst.l	d0
	beq.s	clean_up
	move.l	d0,a0
	move.l	ti_Data(a0),stk_ErrorCode(a5)
	move.l	#SFX_ERR_Animating,d2
	sub.l	a3,a3
	bra.s	ret_err
* }

********************************************************************************
********************************************************************************
* {
* DoUCopStuff - allocates a UCopList, and links it in with the rest.
*
* Takes - d1 = copper instructions
*	  d2 -> Original UCopIns in the ViewPort in case of error.
*	  a0 -> FXHandle
*	  a4 -> GfxBase
*	  a5 -> Stk frame
*	  a6 -> UtilityBase
*
* Trashes d3,d4
*

DoUCopStuff:
	; now allocate a UserCopperlist
	move.l	d1,d3
	move.l	a0,d4
	moveq	#ucl_SIZEOF,d0
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	exg	a4,a6
	move.l	a6,-(sp)
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOAllocMem(a6)
	move.l	(sp)+,a6
	tst.l	d0
	beq.s	FAILURE_1
	move.l	d0,a0
	move.l	d4,a1
	move.l	d0,fxh_UCopList(a1)
	move.l	d0,d4
	move.l	d3,d0
	jsr	_LVOUCopperListInit(a6)
	exg	a4,a6
	tst.l	d0
	beq.s	FAILURE_2
	; link this UCopList into the list in the ViewPort
	move.l	stk_ViewPort(a5),a0
	move.l	d4,a1
	move.l	vp_UCopIns(a0),ucl_Next(a1)
	move.l	a1,vp_UCopIns(a0)
	rts

* }
********************************************************************************


FAILURE_2:
	; free the UCopList
	move.l	stk_SpecialFXBase(a5),a6
	move.l	sfxb_ExecBase(a6),a6
	move.l	a2,a1
	moveq	#ucl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
FAILURE_1:
	; free all the other UCopLists. d2 -> original UCopList, so we know where
	; to stop.
	addq.l	#4,sp			; remove the rts from DoUCopstuff()
	move.l	stk_SpecialFXBase(a5),a6
	move.l	sfxb_ExecBase(a6),a4
	move.l	sfxb_GfxBase(a6),a6
	move.l	stk_ViewPort(a5),a2
	move.l	vp_UCopIns(a2),d3
1$:
	cmp.l	d3,d2
	beq.s	FAILURE_0
	move.l	d3,a0
	move.l	ucl_FirstCopList(a0),a0
	jsr	_LVOFreeCopList(a6)
	exg	a4,a6
	move.l	d3,a1
	move.l	ucl_Next(a1),d3
	moveq	#ucl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	exg	a4,a6
	bra.s	1$

FAILURE_0:
	; not enough memory
	moveq	#SFX_ERR_NoMem,d0
	bra	_InstallFXA.

	end
