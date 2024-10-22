********************************************************************************
*
*	$Id: InterruptControl.asm,v 39.8 93/09/13 19:09:37 spence Exp $
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
	include	'hardware/intbits.i'
	include	'utility/tagitem.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	AllocIntControl
	xdef	CountIntControl
	xdef	InstallIntControl
	xdef	AnimateIntControl
	xdef	RemoveIntControl
	xdef	_LVOFindVP
	xref	DoUCopStuff
	xref	new_mode

	xref	_LVOCWait
	xref	_LVOCMove
	xref	_LVOCBump
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOAllocMem

********************************************************************************
* {

	; *********************************************************************
	;
	; Allocate the FXHandle
	;
	; *********************************************************************

	; Enters with:
	; d1 = Number of control structures to allocate
	; a2 -> Error variable
	; a5 = a6 -> SFXBase
	; Returns
	; d0 -> FXHandle

AllocIntControl:
	; Allocate the RAM for the ColorControl structures.
	moveq	#inc_SIZEOF,d0
	mulu	d1,d0			; enough for all the inc_ structures.
	add.l	#FxHandle_SIZEOF,d0	; plus the FXHandle
	move.l	d1,d2
	asl.w	#2,d1
	add.w	d1,d0			; plus the pointers to the inc_structures
					; = size of the allocation
	move.l	d0,d3
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	move.l	sfxb_ExecBase(a5),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq.s	FAIL_NOMEM
	move.l	d0,a0			; a0 -> start of the array list
	move.l	d0,a1
	lea	FxHandle_SIZEOF(a0),a0
	move.l	#(SFX_IntControl&~$80000000),fxh_Type(a1)
	move.l	d2,fxh_Pad(a1)		; fxh_Pad and fxh_Num
	move.l	d3,fxh_AllocSize(a1)
	move.w	d2,d3
	asl.w	#2,d3
	add.l	d3,a0
	move.l	a0,fxh_First(a1)
	move.l	d0,fxh_BackPointer(a1)
	lea	FxHandle_SIZEOF(a1),a1
	move.l	a1,d0			; return the start of the array list
	bra.s	Alloc_INC_loop.
Alloc_INC_loop:
	move.l	a0,(a1)+
	lea	inc_SIZEOF(a0),a0
Alloc_INC_loop.:
	dbra.s	d2,Alloc_INC_loop
	rts

FAIL_NOMEM:
	move.l	#SFX_ERR_NoMem,(a2)
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; Count the number of copper instructions needed for the IntControl
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> StackStuff
	; a6 -> UtilityBase

	; For each IntControl case, we need the following copperlist:
	; WAIT (inc_Line, 0)
	; MOVE INTREQW,xxx	; trigger the interrupt

CountIntControl:
	movem.l	d2-d7/a2-a6,-(sp)
	moveq	#0,d0			; d0 will count the WAITs
	move.l	d0,d1
	move.w	fxh_Num(a0),d1
	beq.s	CountIntControl.	; early sanity check
	asl.l	#1,d1			; 2 instrunctions per IntControl
	addq.w	#2,d1			; terminator
	jsr	DoUCopStuff(pc)

CountIntControl.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

* }
********************************************************************************
* {	
	; **********************************************************************
	;
	; This code will build the UserCopperList to trigger the interrupts.
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> Stack frame
	; a6 -> UtilityBase

InstallIntControl:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a4,a6
	move.l	a0,a4
	move.l	stk_DispHandle(a5),a3
	move.l	a4,(a3)+		; store the FxHandle in the AnimHandle
	move.l	a3,stk_DispHandle(a5)

	; build the UCopList for the interrupts.

	move.l	fxh_UCopList(a4),a2	; use a2 -> UCopList
	move.w	fxh_Num(a4),d4
	move.l	fxh_First(a4),a4
	lea	-inc_SIZEOF(a4),a4
	bra.s	next_intcont
do_next_intcont:
	moveq	#0,d7
	move.w	inc_Line(a4),d0
	moveq	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.w	#(INTF_SETCLR|INTF_COPER),d1
	CMOVE	#INTREQ,d1
next_intcont:
	lea	inc_SIZEOF(a4),a4
	dbra.s	d4,do_next_intcont

	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)		; CEND
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	; now store this ViewPort in our ViewPort list in SpecialFXBase,
	; and enable the interrupts.

	; first, get the Semaphore
	move.l	stk_SpecialFXBase(a5),a3
	move.l	sfxb_ExecBase(a3),a6
	move.l	sfxb_VPListSemaphore(a3),a0
	jsr	_LVOObtainSemaphore(a6)
	lea	sfxb_VPList(a3),a0
search_vplist:
	move.w	vpl_Count(a0),d0
	cmp.w	#MAX_VPLIST,d0
	beq.s	nxt_vplist_chk
	addq.w	#1,d0
	move.w	d0,vpl_Count(a0)
	lea	vpl_ViewPorts(a0),a1
10$:
	tst.l	(a1)+
	bne.s	10$
	move.l	stk_ViewPort(a5),-4(a1)

DisplayIntControl.:
	move.l	sfxb_VPListSemaphore(a3),a0
	jsr	_LVOReleaseSemaphore(a6)
	movem.l	(sp)+,d2-d7/a2-a6
	rts

nxt_vplist_chk:
	move.l	MLN_SUCC(a0),a0
	tst.l	MLN_SUCC(a0)	; end of the list
	bne.s	search_vplist
new_vplist:
	moveq	#vpl_SIZEOF,d0
	move.l	#(MEMF_CLEAR+MEMF_PUBLIC),d1	
	move.l	sfxb_ExecBase(a3),a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,d2
	beq.s	NO_VPLIST_ERR
	move.l	d0,a1
	lea	sfxb_VPListHead(a3),a0
	ADDHEAD	a0,a1
	move.l	d2,a0
	bra	search_vplist

NO_VPLIST_ERR:
	; Low on memory, so we need to undo all the UCopList stuff
	; NYI!!
	bra.s	DisplayIntControl.

* }
*******************************************************************************
* {
	; **********************************************************************
	;
	;                         AnimateIntControl
	;
	; Change the system copperlist, and optionally the intermediate
	; copperlist.
	; 
	; Takes - a3 -> FXHandle
	;	  a5 -> AnimHandle
	;	  a6 -> SFXBase
	;
	; For speed optimisation, can trash
	; a0-a1/a3-a4/d0-d6
	;
	; **********************************************************************

AnimateIntControl:
	movem.l	d7/a2,-(sp)
	move.l	fxh_First(a3),a4
	move.l	ah_ViewPort(a5),a2
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne.s	no_hw_intc_update	; ViewPort is hidden, so don't update the h/w copperlist

	movem.l	a2/a4,-(sp)
	move.l	ah_Copper1S(a5),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a3),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_change_intc
	movem.l	(sp),a2/a4
1$:
	move.l	ah_Copper2S(a5),d0	; Int control is quick enough for us
					; to update both copies of the copperlist
					; (which we need to do anyway as we are
					; toggling a bit).
	beq.s	3$			; no copper list?
	add.l	fxh_Offset(a3),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_change_intc
	movem.l	(sp),a2/a4
3$:
	move.l	ah_Copper1L(a5),d0	; get the right copperlist to use.
	beq.s	2$
	add.l	fxh_Offset(a3),d0
	move.l	d0,a2
	bsr.s	do_change_intc
2$:
	movem.l	(sp),a2/a4
	move.l	ah_Copper2L(a5),d0	; get the right copperlist to use.
	beq.s	4$
	add.l	fxh_Offset(a3),d0
	move.l	d0,a2
	bsr.s	do_change_intc
4$:
	movem.l	(sp)+,a2/a4
no_hw_intc_update:
	movem.l	(sp)+,d7/a2
	bra.s	intc_change_interm

do_change_intc:
	moveq	#-2,d4			; FFFF FFFE terminator
	move.w	fxh_Num(a3),d2
	bra.s	int_change_loop
	; skip over the WAIT (there may be more than 1 if this is line 256).
look_for_move:
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	change_intc
	cmp.l	(a2),d4			; end of the copperlist?
	beq.s	change_intc.
not_ours:
	addq.l	#4,a2			; it was a WAIT, so skip it.
	bra.s	look_for_move
change_intc:
	cmp.w	#(INTREQ&$0fff),d0
	bne.s	not_ours
	btst.b	#INCB_SET,inc_Flags+1(a4)
	beq.s	no_int_set
	or.w	#INTF_SETCLR,2(a2)
	bra.s	no_int_change
no_int_set:
	btst.b	#INCB_RESET,inc_Flags+1(a4)
	beq.s	no_int_change
	bclr.b	#(INTB_SETCLR-8),2(a2)
no_int_change:
	addq.l	#4,a2			; on to the next instruction
	lea	inc_SIZEOF(a4),a4	; and the next ic structure
int_change_loop:
	dbra.s	d2,look_for_move
change_intc.:
	rts

intc_change_interm:
	; now update the intermediate copperlists.
	move.l	ah_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	intc_load
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	intc_load
	btst.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	bne.s	AnimateIntControl.
intc_load:
	move.w	fxh_Num(a3),d2
	move.l	fxh_UCopList(a3),a0	; look for this UCopList
	move.l	vp_UCopIns(a1),d0
find_ucoplist:
	cmp.l	a0,d0
	beq.s	found_ucoplist
	move.l	d0,a1
	move.l	ucl_Next(a1),d0
	bra.s	find_ucoplist
found_ucoplist:
	move.l	ucl_FirstCopList(a0),a0
	move.l	cl_CopIns(a0),a0
	bra.s	7$
8$:
	btst.b	#0,ci_OpCode+1(a0)
	bne.s	6$			; a WAIT
	btst.b	#INCB_SET,inc_Flags+1(a4)
	beq.s	10$
	or.w	#INTF_SETCLR,ci_DestData(a0)
	bra.s	9$
10$:
	btst.b	#INCB_RESET,inc_Flags+1(a4)
	beq.s	6$
	bclr.b	#(INTB_SETCLR-8),ci_DestData(a0)
9$:
	lea	inc_SIZEOF(a4),a4
	lea	ci_SIZEOF(a0),a0
7$:
	dbra.s	d2,8$
	bra.s	AnimateIntControl.

6$:
	lea	ci_SIZEOF(a0),a0
	bra.s	8$

AnimateIntControl.:
	rts

* }
*******************************************************************************
* {
	; **********************************************************************
	;
	; This code will decrease the IntCount, and reset the interrupts if
	; the count == 0.
	;
	; **********************************************************************

	; enters with:
	; a0 -> ViewPort
	; a5 -> FxHandle
	; a6 -> SpecialFXBase

RemoveIntControl:
	movem.l	a2/a3,-(sp)
	move.l	a6,a2
	move.l	a0,a3
	move.l	sfxb_VPListSemaphore(a2),a0
	move.l	sfxb_ExecBase(a2),a6
	jsr	_LVOObtainSemaphore(a6)

	; look through the list for this ViewPort, and remove it.
	lea	sfxb_VPList(a2),a0
search_vplist_r:
	move.w	vpl_Count(a0),d0
	subq.w	#1,d0
	lea	vpl_ViewPorts(a0),a1
11$:
	cmp.l	(a1)+,a3	; is this the ViewPort?
	beq.s	10$
	dbra.s	d0,11$
	; wasn't in that list, so check the next list.
	move.l	MLN_SUCC(a0),a0	; no need to error check, as we
				; know it's in here somewhere
	bra.s	search_vplist_r

10$:
	clr.l	-4(a1)
	sub.w	#1,vpl_Count(a0)

RemoveIntControl.:
	move.l	sfxb_VPListSemaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)
	move.l	a2,a6
	movem.l	(sp)+,a2/a3
	rts

* }
********************************************************************************
* {

******* specialfx.library/FindVP ***********************************************
*
*   NAME
*	FindVP -- Finds the ViewPort that an interrupt occurred in.
*
*   SYNOPSIS
*	ViewPort = (View, LineNum)
*	d0          a0    a1
*
*	struct ViewPort *FindVP(struct View *, UWORD *);
*
*   FUNCTION
*	To find the ViewPort that an interrupt from InterruptControl
*	occurred in.
*
*   INPUTS
*	View - The view this all happens in
*	LineNum - Pointer to a UWORD containing the line the 
*	          interrupt occurred on
*
*   RESULT
*	ViewPort - The ViewPort the interrupt occurred in, or NULL if error.
*	LineNum - replaced with the Line number in the ViewPort that the
*	          interrupt occurred on.
*
*   NOTES
*	This function is safe to be called in an interrupt handler. In fact,
*	if you have more than one ViewPort using InterruptControl features, you
*	should call this function to determine which ViewPort the interrupt
*	occurred in.
*
*	You should call graphics.library/VBeamPos() as soon as your interrupt
*	server is called if you are using this function to ensure the LineNum
*	passed to FindVP() is correct.
*
*   SEE ALSO
*	<libraries/specialfx.h> AnimateFX() RemoveFX()
*	graphics.library/VBeamPos()
*
********************************************************************************

_LVOFindVP:
	movem.l	d2-d5/a2-a4,-(sp)
	move.w	d0,d4
	move.w	v_DyOffset(a0),d1
	lea	sfxb_VPList(a6),a3
listloop:
	lea	vpl_ViewPorts(a3),a4
	move.w	vpl_Count(a3),d3
	bra.s	FindVPLoop.
FindVPLoop:
	move.l	(a4)+,d2
	beq.s	FindVPLoop
	move.l	d2,a2
	move.w	d1,d2
	add.w	vp_DyOffset(a2),d2
	move.l	a2,a0
	jsr	new_mode(pc)	; (does not corrupt a0 or a1)
	btst.b	#3,d0		; V_DOUBLESCAN?
	beq.s	1$
	add.w	vp_DyOffset(a2),d2
1$:
	move.w	d2,d5
	cmp.w	d2,d4
	beq.s	found		; int occurred on first line of VP
	blt.s	FindVPLoop.	; This VP is lower than the interrupt.
	add.w	vp_DHeight(a2),d2
	btst.b	#3,d0		; V_DOUBLESCAN?
	beq.s	2$
	add.w	vp_DHeight(a2),d2
2$:
	cmp.w	d2,d4
	ble.s	found		; int occurred within the VP.
FindVPLoop.:
	dbra.s	d3,FindVPLoop
	move.l	MLN_SUCC(a3),a3
	tst.l	MLN_SUCC(a3)	; at the end of the list?
	bne.s	listloop
	moveq	#0,d0
	bra.s	FindVP.

found:
	sub.w	d5,d4		; this is the line number in the ViewPort the
				; interrupt occurred on.
	btst.b	#3,d0		; V_DOUBLESCAN
	beq.s	1$
	asr.w	#1,d4
1$:
	move.w	d4,(a1)
	move.l	a2,d0
FindVP.:
	movem.l	(sp)+,d2-d5/a2-a4
	rts
* }
