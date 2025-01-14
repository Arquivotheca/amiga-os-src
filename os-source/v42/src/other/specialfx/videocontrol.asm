********************************************************************************
*
*	$Id: VideoControl.asm,v 39.7 93/09/23 18:24:26 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'graphics/view.i'
	include	'graphics/copper.i'
	include	'graphics/gfxbase.i'
	include	'graphics/videocontrol.i'
	include	'hardware/custom.i'
	include	'utility/tagitem.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	AllocVideoControl
	xdef	CountVideoControl
	xdef	InstallVideoControl
	xdef	AnimateVideoControl
	xdef	RebuildVideoControl
	xref	DoUCopStuff

	xref	_LVOCWait
	xref	_LVOCMove
	xref	_LVOCBump
	xref	_LVOMakeVPort
	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_LVOAllocMem
	xref	_LVONextTagItem

BPLCON3_EXTBLKENA	EQU	(1<<0)
BPLCON3_BORDERSPRITE	EQU	(1<<1)
BPLCON3_BORDERNOTRANS	EQU	(1<<4)
BPLCON3_BORDERBLANK	EQU	(1<<5)
BPLCON2_ZDBPEN	EQU	(1<<11)
BPLCON2_ZDCTEN	EQU	(1<<10)
BPLCON2_KILLEHB	EQU	(1<<9)
BPLCON0_ECSENA	EQU	(1<<0)

BPLCON4_SPEVEN_MASK	EQU	$00f0
BPLCON4_SPODD_MASK	EQU	$000f
BPLCON3_PF2OF_MASK	EQU	$1c00
BPLCON3_SPRITERESN_MASK	EQU	$00c0
BPLCON2_ZDPLANE_MASK	EQU	$7000
BPLCON2_PF2PRI_MASK	EQU	$0038
BPLCON2_PF1PRI_MASK	EQU	$0007
BPLCON0_DEPTH_MASK	EQU	$7010
BPLCON0_RESN_MASK	EQU	$8040

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

AllocVideoControl:
	; Allocate the RAM for the FineVideoControl structures.
	moveq	#fvcp_SIZEOF,d0
	mulu	d1,d0			; enough for all the fvc_ structures.
	add.l	#fvch_SIZEOF,d0		; plus the special FXHandle
	move.l	d1,d2
	asl.w	#2,d1
	add.w	d1,d0			; plus the pointers to the fvc_structures
					; = size of the allocation
	move.l	d0,d3
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	move.l	sfxb_ExecBase(a5),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq.s	FAIL_NOMEM
	move.l	d0,a0			; a0 -> start of the array list
	move.l	d0,a1
	lea	fvch_SIZEOF(a0),a0
	move.l	#(SFX_FineVideoControl&~$80000000),fxh_Type(a1)
	move.l	d2,fxh_Pad(a1)		; fxh_Pad and fxh_Num
	move.l	d3,fxh_AllocSize(a1)
	move.w	d2,d3
	asl.w	#2,d3
	add.l	d3,a0
	move.l	a0,fxh_First(a1)
	move.l	d0,fvch_BackPointer(a1)
	lea	fvch_SIZEOF(a1),a1
	move.l	a1,d0			; return the start of the array list
	bra.s	Alloc_FVC_loop.
Alloc_FVC_loop:
	move.l	a0,(a1)+
	lea	fvcp_SIZEOF(a0),a0
Alloc_FVC_loop.:
	dbra.s	d2,Alloc_FVC_loop
	rts

FAIL_NOMEM:
	move.l	#SFX_ERR_NoMem,(a2)
	rts

* }
********************************************************************************
* {
	; ***********************************************************************
	;
	; Count the number of copper instructions needed for the FineVideoControl
	;
	; ***********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> StackStuff
	; a6 -> UtilityBase

	; For each VideoControl case, we need the following copperlist:
	; WAIT (fvc_Line, 0)
	; MOVE BPLCON0,xxx
	; MOVE BPLCON2,xxx
	; MOVE BPLCON3,xxx
	; MOVE BPLCON4,xxx
	; If fvc_Count != -1 or (== 1 and this is not the last in the list),
	; then we also need:
	; WAIT (fvc_Line + fvc_Count, 0)
	; MOVE BPLCON0,xxx
	; MOVE BPLCON2,xxx
	; MOVE BPLCON3,xxx
	; MOVE BPLCON4,xxx

FVC_COPINS	EQU	5

CountVideoControl:
	movem.l	d3-d4,-(sp)
	moveq	#0,d0
	move.l	d0,d1			; d1 counts the copper instructions
	move.w	fxh_Num(a0),d0
	beq.s	CountVideoControl.	; early sanity check
	subq.w	#1,d0
	move.l	fxh_First(a0),a1
count_fvc_loop:
	addq.w	#FVC_COPINS,d1		; # of instructions per FineVideoControl
	cmp.w	#-1,fvc_Count(a1)
	beq.s	1$
	cmp.w	#1,fvc_Count(a1)
	bne.s	2$
	tst.w	d0
	bne.s	1$
2$:
	addq.w	#FVC_COPINS,d1		; to restore
1$:
	lea	fvcp_SIZEOF(a1),a1
	dbra.s	d0,count_fvc_loop
	addq.w	#2,d1			; terminator
	jsr	DoUCopStuff(pc)

CountVideoControl.:
	movem.l	(sp)+,d3-d4
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; This code will build the UserCopperList for the FineVideoControl
	; settings.
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> Stack frame
	; a6 -> UtilityBase

InstallVideoControl:
	movem.l	d2-d7/a2-a6,-(sp)

	move.l	stk_DispHandle(a5),a3
	move.l	a0,(a3)+		; store the FxHandle in the AnimHandle
	move.l	a3,stk_DispHandle(a5)

	moveq	#0,d6			; use d6 as CON0:CON2
	move.l	d6,d7			; use d7 as CON3:CON4
					; (and use d4 for cm_Bp_0_Base:cm_Bp_1_Base)

	; fill the special fvch_ part of the FXHandle with the original BPLCONx
	; values.

	move.l	stk_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	1$
	move.l	d0,a2
	tst.b	cm_Type(a2)
	beq.s	1$
	move.l	cm_Bp_0_base(a2),d4
1$:
	move.l	vp_DspIns(a1),d0
	beq.s	got_con
	tst.w	fvch_Valid(a0)
	bmi.s	got_con
	move.l	d0,a1
	move.w	cl_Count(a1),d5
	move.l	cl_CopIns(a1),d0
	beq.s	got_con
	move.l	d0,a1

	; Under 'A' do something different
	btst.b	#GFXB_HR_DENISE,gb_ChipRevBits0(a4)
	beq	get_a_bplcon.

	bra.s	get_bplcon.
get_bplcon:
	move.w	(a1)+,d0		; opcode
	move.l	(a1)+,d1		; address and data
	tst.b	d0
	bne.s	get_bplcon.		; not a MOVE
	move.w	d1,d0
	swap	d1
	cmp.w	#(BPLCON0&$fff),d1
	bne.s	not_con0
	swap	d6
	move.w	d0,d6
	swap	d6
	bra.s	get_bplcon.
not_con0:
	cmp.w	#(BPLCON2&$fff),d1
	bne.s	not_con2
	move.w	d0,d6
	bra.s	get_bplcon.
not_con2:
	cmp.w	#(BPLCON4&$fff),d1
	bne.s	get_bplcon.
	move.w	d0,d7
get_bplcon.:
	dbra.s	d5,get_bplcon
done_con_search:
	move.l	d6,fvch_BPLCON0(a0)	; CON0 and CON2
	swap	d7
	move.w	stk_BPLCON3(a5),d7	; we already have this!
	swap	d7
	move.l	d7,fvch_BPLCON3(a0)	; CON3 and CON4
	move.w	#-1,fvch_Valid(a0)
got_con:
	; walk through the TagList of each FineVideoControl structure to
	; build the UCopList

	move.w	fxh_Num(a0),d5
	beq	InstallVideoControl.	; Duh - pretty stoopid
	subq.w	#1,d5
	move.l	a0,a4
	move.l	fxh_UCopList(a4),a2
	move.l	fxh_First(a4),a3

*	{
next_fvc:
	move.l	fvch_BPLCON0(a4),d6	; get these out of the cache
	move.l	fvch_BPLCON3(a4),d7	; if fvch_Valid is set.

	move.l	a5,-(sp)
	move.l	stk_ViewPort(a5),a5
	bsr	CalcBPLCONs
	move.l	(sp)+,a5

	; Now,	d6 = BPLCON0:BPLCON2
	;	d7 = BPLCON3:BPLCON4
	;	a2 -> UCopList
	;	a3 -> the FineVideoControl structure
	;	a4 -> FxHandle
	;	a5 -> stack stuff
	; so build the UCopList

	move.l	a6,-(sp)
	move.l	stk_SpecialFXBase(a5),a6
	move.l	sfxb_GfxBase(a6),a6
	move.w	fvc_Line(a3),d0
	moveq	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	swap	d6
	CMOVE	#BPLCON0,d6
	swap	d6
	CMOVE	#BPLCON2,d6
	swap	d7
	CMOVE	#BPLCON3,d7
	swap	d7
	CMOVE	#BPLCON4,d7
	move.w	fvc_Count(a3),d0
	cmp.w	#-1,d0
	beq	next_fvc.
	cmp.w	#1,d0
	bne.s	fvc_restore
	tst.w	d5		; if Count == 1 and this is not the last in the
				; list, don't restore in the next line if the
				; next FineVideoControl is the next line.
	beq.s	fvc_restore
	move.w	(fvcp_SIZEOF+fvc_Line)(a3),d1	; next line
	subq.w	#1,d1
	cmp.w	fvc_Line(a3),d1
	beq	next_fvc.
fvc_restore:
	add.w	fvc_Line(a3),d0
	moveq	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	moveq	#0,d1	
	move.w	fvch_BPLCON0(a4),d1
	CMOVE	#BPLCON0,d1
	move.w	fvch_BPLCON2(a4),d1
	CMOVE	#BPLCON2,d1
	move.w	fvch_BPLCON3(a4),d1
	CMOVE	#BPLCON3,d1
	move.w	fvch_BPLCON4(a4),d1
	CMOVE	#BPLCON4,d1
next_fvc.:
	move.l	(sp)+,a6
	lea	fvcp_SIZEOF(a3),a3
	dbra.s	d5,next_fvc

	move.l	a6,-(sp)
	move.l	stk_SpecialFXBase(a5),a6
	move.l	sfxb_GfxBase(a6),a6
	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)		; CEND
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.l	(sp)+,a6

*	}

InstallVideoControl.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; Under 'A', there is no bplcon3 or bplcon4 to look for. Also, there are three
; writes to bplcon0; the first sets the depth to 0, the second sets the depth,
; and the third sets the depth to 0 again - remember under 'A' there is no fine
; control on DIWSTRT and DIWSTOP so this is how 'A' makes the interscreen gaps.
; Because of this, we cannot use the last BPLCON0 found as we do under ECS and AA.

get_a_bplcon:
	move.w	(a1)+,d0		; opcode
	move.l	(a1)+,d1		; address and data
	tst.b	d0
	bne.s	get_a_bplcon.		; not a MOVE
	move.w	d1,d0
	swap	d1
	cmp.w	#(BPLCON0&$fff),d1
	bne.s	not_a_con0
	swap	d6
	and.w	$7000,d6
	or.w	d0,d6
	swap	d6
	bra.s	get_a_bplcon.
not_a_con0:
	cmp.w	#(BPLCON2&$fff),d1
	bne.s	get_a_bplcon.
	move.w	d0,d6
get_a_bplcon.:
	dbra.s	d5,get_a_bplcon
	bra	done_con_search

* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         AnimateVideoControl
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

AnimateVideoControl:
	movem.l	d7/a2/a5-a6,-(sp)
	move.l	fxh_First(a3),a4
	exg	a3,a4
	move.l	ah_ViewPort(a5),a2
	move.w	ah_CopperToUse(a5),d1
	asl.w	#3,d1
	move.l	a5,a1
	move.l	a2,a5			; need a5 for CalcBPLCONs()
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne.s	no_hw_fvc_update	; ViewPort is hidden, so don't update the h/w copperlist

	move.l	vp_ColorMap(a2),d0
	beq.s	3$
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	3$
	move.l	cm_Bp_0_base(a0),d4
3$:
	move.l	sfxb_UtilityBase(a6),a6

	movem.l	d1/a1-a3,-(sp)
	move.l	ah_Copper1S(a1,d1.w),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a4),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_change_fvc
	movem.l	(sp),d1/a1-a3
1$:
	move.l	ah_Copper1L(a1,d1),d0
	beq.s	2$
	add.l	fxh_Offset(a4),d0
	move.l	d0,a2
	bsr.s	do_change_fvc
2$:
	movem.l	(sp)+,d1/a1-a3
no_hw_fvc_update:
	movem.l	(sp)+,d7/a2/a5-a6
	bra.s	fvc_change_interm

do_change_fvc:
	moveq	#-2,d3			; FFFF FFFE terminator
	move.w	fxh_Num(a4),d2
	bra.s	fvc_change_loop
	; skip over the WAIT (there may be more than 1 if this is line 256).
look_for_move:
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	change_fvc
	cmp.l	(a2),d3			; end of the copperlist?
	beq.s	change_fvc.
not_ours:
	addq.l	#4,a2			; it was a WAIT, so skip it.
	bra.s	look_for_move
change_fvc:
	cmp.w	#(BPLCON0&$0fff),d0	
	bne.s	not_ours
	btst.b	#FVCB_MODIFY,fvc_Flags+1(a3)
	beq.s	no_fvc_change
	; do the change:
	; call CalcBPLCONs for the values we need, and just poke them in.

	bsr	CalcBPLCONs

	move.w	d6,6(a2)		; BPLCON2
	swap	d6
	move.w	d6,2(a2)		; BPLCON0
	move.w	d7,14(a2)		; BPLCON4
	swap	d7
	move.w	d7,10(a2)		; BPLCON3
	move.l	d6,fvcp_BPLCON2(a3)	; cache these for the intermediate lists
	move.l	d7,fvcp_BPLCON4(a3)

no_fvc_change:
	lea	16(a2),a2		; on to the next instruction
	lea	fvcp_SIZEOF(a3),a3	; and the next fvc structure
fvc_change_loop:
	move.l	fvch_BPLCON0(a4),d6	; get these out of the cache
	move.l	fvch_BPLCON3(a4),d7
	dbra.s	d2,look_for_move
change_fvc.:
	rts

fvc_change_interm:
	; now update the intermediate copperlists.
	move.l	ah_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	fvc_load
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	fvc_load
	btst.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	bne.s	AnimateVideoControl.
fvc_load:
	move.w	fxh_Num(a4),d2
	move.l	fxh_UCopList(a4),a0	; look for this UCopList
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
	btst.b	#FVCB_MODIFY,fvc_Flags+1(a3)
	beq.s	9$
	move.w	fvcp_BPLCON0(a3),ci_DestData(a0)
	move.w	fvcp_BPLCON2(a3),(ci_SIZEOF+ci_DestData)(a0)
	move.w	fvcp_BPLCON3(a3),((ci_SIZEOF*2)+ci_DestData)(a0)
	move.w	fvcp_BPLCON4(a3),((ci_SIZEOF*3)+ci_DestData)(a0)
9$:
	lea	fvcp_SIZEOF(a3),a3
	lea	(ci_SIZEOF*4)(a0),a0
7$:
	dbra.s	d2,8$
	bra.s	AnimateVideoControl.

6$:
	lea	ci_SIZEOF(a0),a0
	bra.s	8$

AnimateVideoControl.:
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         RebuildVideoControl
	;
	; Change the system copperlist and intermediate copperlist for a
	; mode coercion
	; 
	; Takes - a0 -> FXHandle
	;	  a2 -> SFXBase
	;	  a4 -> AnimHandle
	;	  d5.w = -1 if the View mode or current monitor changed.
	;
	; **********************************************************************

BPLCON0_WANT_MASK	EQU	$8045

RebuildVideoControl:
	tst.w	d5
	beq	rebuild_fvc.	; no mode change, so no rebuild

	; We need to change the EXTBLNKEN bit in BPLCON3, and
	; the ECSENA, LACE and resolution bits in BPLCON0

	movem.l	d2-d7/a2/a3/a5,-(sp)
	move.l	ah_ViewPort(a4),a1
	; if the ViewPort is hidden, we need to call MakeVPort(), as intuition
	; does not remake the ViewPort until it becomes visible.
	btst.b	#5,vp_Modes(a1)		; V_VP_HIDE
	beq.s	no_mvp
	movem.l	a0/a1/a6,-(sp)
	move.l	ah_View(a4),a0
	move.l	sfxb_GfxBase(a2),a6
	jsr	_LVOMakeVPort(a6)
	movem.l	(sp)+,a0/a1/a6
no_mvp:
	move.l	vp_DspIns(a1),d0
	beq	RebuildVideoControl.
	move.l	d0,a1
	move.l	cl_CopIns(a1),d0
	beq	RebuildVideoControl.
	move.l	d0,a1
	; look for a BPLCON3
	move.w	cl_Count(a2),d2
	move.l	#(BPLCON3&$fff),d3
	move.l	#(BPLCON0&$fff),d4
	moveq	#0,d5			; store the new CON3:CON0 in d5
	lea	-ci_SIZEOF(a1),a1
	bra.s	find_con0.

	; assume the order of copper instructions is BPLCON0 followed by BPLCON3
find_con0:
	move.b	ci_OpCode+1(a1),d0
	bne.s	find_con0.		; not a MOVE
	move.l	ci_DestAddr(a1),d0	; and ci_DestData
	swap	d0
	and.w	#$fff,d0
	cmp.w	d0,d4
	beq.s	found_con0
find_con0.
	lea	ci_SIZEOF(a1),a1
	dbra.s	d2,find_con0
	bra.s	found_cons		; no con0??
found_con0:
	swap	d0
	move.w	d0,d5	
	and.w	#BPLCON0_WANT_MASK,d5
	bra.s	find_con3.

find_con3:
	move.b	ci_OpCode+1(a1),d0
	bne.s	find_con3.
	move.l	ci_DestAddr(a1),d0
	swap	d0
	and.w	#$fff,d0
	cmp.w	d0,d3
	beq.s	found_con3
find_con3.:
	lea	ci_SIZEOF(a1),a1
	dbra.s	d2,find_con3
	bra.s	found_cons		; no con3 on 'A' machines
found_con3:
	swap	d5
	swap	d0
	move.w	d0,d5
	and.w	#BPLCON3_EXTBLKENA,d5
	swap	d5

found_cons:
	move.w	fvch_BPLCON0(a0),d0
	and.w	#$7fbf,d0		; remove the Resolution bits
	or.w	d5,d0
	move.w	d0,fvch_BPLCON0(a0)	
	move.w	fvch_BPLCON3(a0),d0
	swap	d5
	or.w	d5,d0
	move.w	d0,fvch_BPLCON3(a0)
	swap	d5

	move.l	fxh_First(a0),a3
	movem.l	a0/a3,-(sp)
	move.l	ah_Copper1S(a4),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a0),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_rebuild_fvc
	movem.l	(sp),a0/a3
1$:
	tst.l	ah_Copper2SSize(a4)
	beq.s	10$
	move.l	ah_Copper2S(a4),d0	; get the right copperlist to use.
	beq.s	10$			; no copper list?
	add.l	fxh_Offset(a0),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_rebuild_fvc
	movem.l	(sp),a0/a3
10$:
	tst.l	ah_Copper2LSize(a4)
	beq.s	2$
	move.l	ah_Copper2L(a4),d0
	beq.s	2$
	add.l	fxh_Offset(a0),d0
	move.l	d0,a2
	bsr.s	do_rebuild_fvc
	movem.l	(sp),a0/a3
2$:
	move.l	ah_Copper1L(a4),d0
	beq.s	20$
	add.l	fxh_Offset(a0),d0
	move.l	d0,a2
	bsr.s	do_rebuild_fvc
20$:
	movem.l	(sp)+,a0/a3
	bra	rebuild_interm

do_rebuild_fvc:
	moveq	#-2,d3			; FFFF FFFE terminator
	move.w	fxh_Num(a0),d2
	bra.s	fvc_rebuild_loop
	; skip over the WAIT (there may be more than 1 if this is line 256).
look_for_move1:
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	rebuild_fvc
	cmp.l	(a2),d3			; end of the copperlist?
	beq.s	rebuild_fvc.
not_ours1:
	addq.l	#4,a2			; it was a WAIT, so skip it.
	bra.s	look_for_move1
rebuild_fvc:
	cmp.w	#(BPLCON0&$0fff),d0	
	bne.s	not_ours1

	; do the change:

	move.w	2(a2),d0		; BPLCON0
	and.w	#$7fbf,d0
	or.w	d5,d0			; OR in the new LACE and resn bits
	move.w	d0,2(a2)
	move.w	10(a2),d0		; BPLCON3
	swap	d5
	or.w	d5,d0
	move.w	d0,10(a2)
	swap	d5

	; if the fvc_Count is not -1, then there will be another set of
	; CONs that reset the register back to it's original state.
	cmp.w	#-1,fvc_Count(a3)
	beq.s	no_fvc_rebuild
	lea	16(a2),a2		; on to the next instruction
look_for_move2:
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	rebuild_fvc2
	cmp.l	(a2),d3			; end of the copperlist?
	beq.s	rebuild_fvc.
not_ours2:
	addq.l	#4,a2
	bra.s	look_for_move2
rebuild_fvc2:
	cmp.w	#(BPLCON0&$0fff),d0
	bne.s	not_ours2

	; do the change:

	move.w	2(a2),d0		; BPLCON0
	and.w	#$7fbf,d0
	or.w	d5,d0
	move.w	d0,2(a2)
	move.w	10(a2),d0		; BPLCON3
	swap	d5
	or.w	d5,d0
	move.w	d0,10(a2)
	swap	d5
	
no_fvc_rebuild:
	lea	fvcp_SIZEOF(a3),a3	; and the next fvc structure
	lea	16(a2),a2		; on to the next instruction
fvc_rebuild_loop:
	dbra.s	d2,look_for_move1
rebuild_fvc.:
	rts

rebuild_interm:
	move.l	fxh_UCopList(a0),a5
	move.l	ucl_FirstCopList(a5),a5
	move.l	cl_CopIns(a5),a5
	move.w	fxh_Num(a0),d2
	bra.s	rebuild_interm_loop.
rebuild_interm_loop:
	move.w	(ci_SIZEOF+ci_DestData)(a5),d0	; BPLCON0
	and.w	#$7fbf,d0
	or.w	d5,d0			; OR in the new LACE and resn bits
	move.w	d0,(ci_SIZEOF+ci_DestData)(a5)
	move.w	d0,fvcp_BPLCON0(a3)	; and update the cache
	move.w	((ci_SIZEOF*3)+ci_DestData)(a5),d0	; BPLCON3
	swap	d5
	or.w	d5,d0
	move.w	d0,((ci_SIZEOF*3)+ci_DestData)(a5)
	swap	d5
	lea	(ci_SIZEOF*5)(a5),a5
	move.w	d0,fvcp_BPLCON3(a3)

	; if the fvc_Count is not -1, then there will be another set of
	; CONs that reset the register back to it's original state.
	cmp.w	#-1,fvc_Count(a3)
	beq.s	1$
	move.w	(ci_SIZEOF+ci_DestData)(a5),d0	; BPLCON0
	and.w	#$7fbf,d0
	or.w	d5,d0			; OR in the new LACE and resn bits
	move.w	d0,(ci_SIZEOF+ci_DestData)(a5)
	move.w	((ci_SIZEOF*3)+ci_DestData)(a5),d0	; BPLCON3
	swap	d5
	or.w	d5,d0
	move.w	d0,((ci_SIZEOF*3)+ci_DestData)(a5)
	swap	d5
	lea	(ci_SIZEOF*5)(a5),a5
1$:
	lea	fvcp_SIZEOF(a3),a3
rebuild_interm_loop.:
	dbra.s	d2,rebuild_interm_loop

RebuildVideoControl.:
	movem.l	(sp)+,d2-d7/a2/a3/a5
	rts

rebuild_hidden:
	clr.w	fvch_Valid(a0)
	beq.s	RebuildVideoControl.

* }
********************************************************************************
* {

* CalcBPLCONs: calculate the values of BPLCON0,2,3,4 based on the original
* values and the taglist specified.
*
* Takes:
*	d4 = cm_Bp_0_Base:cm_Bp_1_Base
*	d6 = BPLCON0:BPLCON2
*	d7 = BPLCON3:BPLCON4
*	a3 -> FineVideoControl structure
*	a5 -> ViewPort
*	a6 -> UtilityBase
*
* Returns:
*	d6,d7 = new BPLCON values for the copperlists.
*
* Only corrupts d0-d1/a0-a1
*

CalcBPLCONs:
	move.l	fvc_TagList(a3),-(sp)
install_fvc_loop:
	move.l	sp,a0
	jsr	_LVONextTagItem(a6)
	tst.l	d0
	beq	CalcBPLCONs.
	move.l	d0,a0
	move.l	ti_Tag(a0),d0
	; now we have the tag, do something with it.
	cmp.l	#VTAG_SFX_DEPTH_SET,d0
	bge.s	do_vtag_sfx
	cmp.l	#VTAG_CHROMA_PLANE_SET,d0
	ble	do_vtag_genloc
	cmp.l	#VTAG_PF2_TO_SPRITEPRI_SET,d0
	bgt	unsupported_vtag
	sub.l	#VTAG_PF1_BASE_SET,d0
	asl.w	#2,d0
	lea	vtag_list2(pc),a1
	move.l	0(a1,d0.w),a1
	jmp	(a1)

do_vtag_sfx:
	and.l	#~VTAG_SFX_DEPTH_SET,d0
	asl.w	#2,d0
	lea	vtag_list3(pc),a1
	move.l	0(a1,d0.w),a1
	jmp	(a1)

do_vtag_depth:
	move.l	ti_Data(a0),d0
	and.l	#~(BPLCON0_DEPTH_MASK<<16),d6	; clear the depth
	cmp.w	#8,d0
	bge.s	8$
	ror.l	#4,d0			; rotate depth to upper bits of the LONG
	or.l	d0,d6
	bra.s	install_fvc_loop
8$:
	or.l	#$00100000,d6		; BPU3
	bra.s	install_fvc_loop
do_vtag_ham:
	or.l	#(V_HAM<<16),d6
	bra.s	install_fvc_loop
do_vtag_ehb:
	and.w	#~BPLCON2_KILLEHB,d6
	bra	install_fvc_loop
do_vtag_norm:
	and.l	#~((V_HAM|V_DUALPF)<<16),d6	; disable ham and dpf in BPLCON0
	or.w	#BPLCON2_KILLEHB,d6	; and enable KILLEHB
	bra	install_fvc_loop
do_vtag_makedpf:
	or.l	#(V_DUALPF<<16),d6
	bra	install_fvc_loop
do_vtag_pf2pri:
	or.w	#V_PFBA,d6
	bra	install_fvc_loop
do_vtag_pf1pri:
	and.w	#~(V_PFBA),d6
	bra	install_fvc_loop

do_vtag_genloc:
	and.l	#~$80000000,d0
	asl.w	#2,d0
	lea	vtag_list1(pc),a1
	move.l	0(a1,d0.w),a1
	jmp	(a1)

do_chroma_key_clr:
	and.w	#~BPLCON2_ZDCTEN,d6
	bra	install_fvc_loop
do_chroma_key_set:
	or.w	#BPLCON2_ZDCTEN,d6
	bra.s	ecs_enable
do_bitplane_key_clr:
	and.w	#~BPLCON2_ZDBPEN,d6
	bra	install_fvc_loop
do_bitplane_key_set:
	or.w	#BPLCON2_ZDBPEN,d6
	bra.s	ecs_enable
do_borderblank_clr:
	and.l	#~(BPLCON3_BORDERBLANK<<16),d7
	bra	install_fvc_loop
do_borderblank_set:
	or.l	#(BPLCON3_BORDERBLANK<<16),d7
	bra.s	ecs_enable
do_bordernotrans_clr:
	and.l	#~(BPLCON3_BORDERNOTRANS<<16),d7
	bra	install_fvc_loop
do_bordernotrans_set:
	or.l	#(BPLCON3_BORDERNOTRANS<<16),d7
	bra.s	ecs_enable
do_chroma_plane_set:
	move.l	ti_Data(a0),d0	
	subq.w	#1,d0
	and.w	#7,d0
	ror.w	#4,d0
	and.w	#~BPLCON2_ZDPLANE_MASK,d6
	or.w	d0,d6
ecs_enable:
	or.l	#(BPLCON0_ECSENA<<16),d6
	bra	install_fvc_loop

do_pf2_base_set:		; the calculations of the pfx_base values
				; are taken from get_bplcon3() and
				; get_bplcon4() of
				; graphics/a/NewColorStuff.asm
	move.l	ti_Data(a0),d0
	move.w	d0,d1
	lsl.w	#8,d0
	and.w	#$ff,d7
	or.w	d0,d7		; the BPLCON4 value
calc_color_bits:
	move.w	d1,d0
	sub.w	d4,d1
	swap	d4
	move.w	d0,d4		; new bp_0_base
	swap	d4
_color_ofs_bits:
; int __asm color_ofs_bits(register __d1 number);
; return d0=the 3 bit value for color offset NN
;	128->7 64->6 32->5 16->4 8->3 4->2 2->1 0->0
; trashes d1,d0
	tst.b	d1
	beq.s	2$
	moveq	#8,d0
1$:
	subq	#1,d0
	add.b	d1,d1
	bcc.s	1$
	bra.s	3$
2$:
	moveq	#0,d0
3$:
	ror.l	#6,d0
	and.l	#~(BPLCON3_PF2OF_MASK<<16),d7	; the BPLCON3 value
	or.l	d0,d7
	bra	install_fvc_loop
do_pf1_base_set:
	move.w	ti_Data+2(a0),d4	; new bp_1_base
	move.l	d4,d1
	swap	d1			; d1.w = bp_0_base
	bra.s	calc_color_bits
do_speven_base_set:
	move.l	ti_Data(a0),d0
	and.w	#BPLCON4_SPEVEN_MASK,d0
	and.w	#~BPLCON4_SPEVEN_MASK,d7
	or.w	d0,d7
	bra	install_fvc_loop
do_spodd_base_set:
	move.l	ti_Data(a0),d0
	lsr.w	#4,d0
	and.w	#BPLCON4_SPODD_MASK,d0
	and.w	#~BPLCON4_SPODD_MASK,d7
	or.w	d0,d7
	bra	install_fvc_loop
do_bordersprite_set:
	or.l	#(BPLCON3_BORDERSPRITE<<16),d7
	bra.s	ecs_enable
do_bordersprite_clr:
	and.l	#~(BPLCON3_BORDERSPRITE<<16),d7
	bra	install_fvc_loop
do_sprite_resn_set:
	moveq	#0,d0
	move.w	ti_Data+2(a0),d0
	bmi.s	sprite_resn_deft	; -1 == default resolution
sprite_resn:
	asl.w	#6,d0
	swap	d0
	and.l	#~(BPLCON3_SPRITERESN_MASK<<16),d7
	or.l	d0,d7
	bra	install_fvc_loop
sprite_resn_deft:
	move.l	vp_ColorMap(a5),d0
	beq	install_fvc_loop
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq	install_fvc_loop
	moveq	#0,d0
	move.b	cm_SpriteResDefault(a0),d0
	bra.s	sprite_resn
do_pf1_to_spritepri_set:
	move.l	ti_Data(a0),d0
	and.w	#~BPLCON2_PF1PRI_MASK,d6
	and.w	#BPLCON2_PF1PRI_MASK,d0
	or.w	d0,d6
	bra	install_fvc_loop
do_pf2_to_spritepri_set:
	move.l	ti_Data(a0),d0
	and.w	#~BPLCON2_PF2PRI_MASK,d6
	asl.w	#3,d0
	and.w	#BPLCON2_PF2PRI_MASK,d0
	or.w	d0,d6
unsupported_vtag:
	bra	install_fvc_loop

CalcBPLCONs.:
	addq.l	#4,sp		; remove the TagList * from the stack
	rts


vtag_list1:
	dc.l	do_chroma_key_clr
	dc.l	do_chroma_key_set
	dc.l	do_bitplane_key_clr
	dc.l	do_bitplane_key_set
	dc.l	do_borderblank_clr
	dc.l	do_borderblank_set
	dc.l	do_bordernotrans_clr
	dc.l	do_bordernotrans_set
	dc.l	unsupported_vtag	; chroma_pen_clr
	dc.l	unsupported_vtag	; chroma_pen_set
	dc.l	do_chroma_plane_set

vtag_list2:
	dc.l	do_pf1_base_set
	dc.l	do_pf2_base_set
	dc.l	do_speven_base_set
	dc.l	do_spodd_base_set
	dc.l	unsupported_vtag	; bordersprite_get
	dc.l	do_bordersprite_set
	dc.l	do_bordersprite_clr
	dc.l	do_sprite_resn_set
	dc.l	unsupported_vtag	; sprite_resn_get
	dc.l	do_pf1_to_spritepri_set
	dc.l	unsupported_vtag	; do_pf1_to_spritepri_get
	dc.l	do_pf2_to_spritepri_set

vtag_list3:
	dc.l	do_vtag_depth
	dc.l	do_vtag_ham
	dc.l	do_vtag_ehb
	dc.l	do_vtag_norm
	dc.l	do_vtag_makedpf
	dc.l	do_vtag_pf1pri
	dc.l	do_vtag_pf2pri

* }
