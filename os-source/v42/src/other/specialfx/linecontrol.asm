********************************************************************************
*
*	$Id: LineControl.asm,v 39.10 93/09/21 16:47:40 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'graphics/view.i'
	include	'graphics/copper.i'
	include	'graphics/gfxbase.i'
	include	'hardware/custom.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	AllocLineControl
	xdef	CountLineControl
	xdef	InstallLineControl
	xdef	AnimateLineControl
	xdef	RebuildLineControl
	xref	DoUCopStuff
	xref	new_mode

	xref	_LVOCWait
	xref	_LVOCMove
	xref	_LVOCBump
	xref	_LVOMakeVPort
	xref	_LVOScrollVPort
	xref	_LVOFreeVPortCopLists
	xref	_LVOAllocMem
	xref	_LVOForbid
	xref	_LVOPermit

ON	equ	0
OFF	equ	1
DODEBUGTABLES	equ	OFF
DODEBUGGOT	equ	OFF
DODEBUGMODS	equ	OFF
DODEBUGTYPE	equ	OFF
DODEBUGWAIT	equ	OFF

	IFEQ	DODEBUGTABLES
DEBUGTABLES	MACRO
	print	"Scroll "
	dbugw	ri_RxOffset(a3)
	print	"	DDFSTRT "
	dbugw	lco_DDFSTRT(a4)
	print	"	DDFSTOP "
	dbugw	lco_DDFSTOP(a4)
	print	"	CON1 "
	dbugw	lco_BPLCON1(a4)
	print	"	MOD1 "
	dbugw	lco_BPLMOD1(a4)
	print	"	MOD2 "
	dbugw	lco_BPLMOD2(a4)
	print	"	FMODE "
	dbugw	lco_FMODE(a4)
	print	"	MDiff "
	dbugw	lco_ModDiff(a4)
	print	"	CompEven "
	dbugw	lco_CompensateEven(a4)
	print	"	CompOdd "
	dbugw	lco_CompensateOdd(a4)
	print	"	BPL1PTR "
	dbug	stk_ZeroBPL1PTR(a5)
	print	"	BPL2PTR "
	dbug	stk_ZeroBPL2PTR(a5)
	print	"	ZeroModOdd "
	dbugw	stk_ZeroModOdd(a5)
	print	"	ZeroModEven "
	dbugw	stk_ZeroModEven(a5)
	print	10
	ENDM
	ELSE
DEBUGTABLES	MACRO
	ENDM
	ENDC

	IFEQ	DODEBUGGOT
DEBUGGOT	MACRO
	print	"Return d4 = "
	dbug	d4
	print	" ("
	swap	d4
	moveq	#0,d0
	move.w	d4,d0
	sub.w	#lch_SIZEOF,d0
	divu	#lco_SIZEOF,d0
	dbugw	d0
	print	") ("
	swap	d4
	moveq	#0,d0
	move.w	d4,d0
	sub.w	#lch_SIZEOF,d0
	divu	#lco_SIZEOF,d0
	dbugw	d0
	print	") WholeOdd = "
	dbugw	stk_WholeOdd(a5)
	print	" WholeEven = "
	dbugw	stk_WholeEven(a5)
	print	10
	ENDM
	ELSE
DEBUGGOT	MACRO
	ENDM
	ENDC

	IFEQ	DODEBUGMODS
DEBUGMODS	MACRO	;\1 = string, \2 = string
	print	\1
	dbugw	d1
	print	" "
	dbugw	d6
	print	\2
	ENDM
	ELSE
DEBUGMODS	MACRO
	ENDM
	ENDC

	IFEQ	DODEBUGTYPE
DEBUGTYPE	MACRO
	dbugw	d5
	print	10
	ENDM
	ELSE
DEBUGTYPE	MACRO
	ENDM
	ENDC

	IFEQ	DODEBUGWAIT
DEBUGWAIT	MACRO
	print	"WAIT "
	dbugw	d0
	print	10
	ENDM
	ELSE
DEBUGWAIT	MACRO
	ENDM
	ENDC

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
	; a3 -> ViewPort
	; a5 = a6 -> SFXBase
	; Returns
	; d0 -> FXHandle

AllocLineControl:
	; Calculate the size of the lcOffsetTables. Number needed =
	; (16 << *(GfxBase->bwshifts[GfxBase->MemType]))
	move.l	d1,d2
	move.l	a3,a0
	jsr	new_mode(pc)
	move.l	d2,d1
	move.l	d0,d6			; d6 has the mode bits
	moveq	#16,d2			; assume a 16 count
	moveq	#0,d0
	move.l	sfxb_GfxBase(a6),a0
	move.l	gb_bwshifts(a0),a1
	move.b	gb_MemType(a0),d0
	move.b	0(a1,d0.w),d0
	asl.l	d0,d2
alloc_have_count:
	move.w	d2,d4
	swap	d4
	move.w	d0,d4			; lch_OffsetCount
	addq.w	#4,d4			; 2 log lch_OffsetCount
	swap	d4
	IFEQ	(lco_SIZEOF-20)		; if the lco_SIZEOF == 20 bytes
	move.w	d2,d3
	asl.w	#4,d2			; * 16
	asl.w	#2,d3			; * 4
	add.w	d3,d2			; * 18
	ELSE
	FAIL
	ENDC
	move.l	d2,d5
	; Allocate the RAM for the LineControl structures.
	moveq	#lcp_SIZEOF,d0
	mulu	d1,d0			; enough for all the lc_ structures.
	add.l	#lch_SIZEOF,d0		; plus the special FXHandle
	add.l	d2,d0
	move.l	d1,d2
	asl.w	#2,d1
	add.w	d1,d0			; plus the pointers to the lc_structures
	addq.l	#4,d0			; plus BackPointer
					; = size of the allocation
	move.l	d0,d3
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	move.l	sfxb_ExecBase(a5),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq.s	FAIL_NOMEM
	move.l	d0,a0			; a0 -> start of the array list
	move.l	d0,a1
	lea	(lch_SIZEOF+4)(a0,d5.l),a0
	move.l	#(SFX_LineControl&~$80000000),fxh_Type(a1)
	move.l	d2,fxh_Pad(a1)		; fxh_Pad and fxh_Num
	move.l	d3,fxh_AllocSize(a1)
	move.w	d2,d3
	asl.w	#2,d3
	add.l	d3,a0
	move.l	a0,fxh_First(a1)
	move.l	d4,lch_Shift(a1)	; lch_Shift and lch_OffsetCount
	; the mask is just lch_OffsetCount - 1.
	subq.w	#1,d4
	move.w	d4,lch_Mask(a1)
	lea	lch_SIZEOF(a1,d5.l),a1
	move.l	d0,(a1)+		; lch_BackPointer
	move.l	a1,d0			; return the start of the array list
	bra.s	Alloc_LC_loop.
Alloc_LC_loop:
	move.l	a0,(a1)+
	move.l	#$10001,lc_SkipRateEven(a0)	; set Even and Odd SkipRates to 1.
	lea	lcp_SIZEOF(a0),a0
Alloc_LC_loop.:
	dbra.s	d2,Alloc_LC_loop
	rts

FAIL_NOMEM:
	move.l	#SFX_ERR_NoMem,(a2)
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; Count the number of copper instructions needed for the LineControl
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> StackStuff
	; a6 -> UtilityBase

	;
	; For each LineControl case, we need the following copperlist:
	; WAIT (lc_Line-1, 0)		; LINE 1
	; MOVE BPLMOD1,xxx	; set start of next line correctly
	; MOVE BPLMOD2,xxx
	; WAIT (lc_Line, 0)		; LINE 2
	; MOVE BPLMOD1,xxx	; align next sl_YCount lines up
	; MOVE BPLMOD2,xxx
	; MOVE DDFSTRT,xxx
	; MOVE BPLCON1,xxx
	; MOVE FMODE,xxx	; (only on AA)
	; WAIT (lc_Line+lc_Count-1, 0)	; LINE 3
	; MOVE BPLMOD1,xxx	; set start of next line correctly
	; MOVE BPLMOD2,xxx
	; WAIT (lc_Line+lc_Count, 0)	; LINE 4
	; MOVE BPLMOD1,xxx	; align next sl_YCount lines up
	; MOVE BPLMOD2,xxx
	; MOVE DDFSTRT,xxx
	; MOVE BPLCON1,xxx
	; MOVE FMODE,xxx	; (only on AA)
	;
	; There are times when some of these LINEs are not needed. These
	; are defined in the following LUT:

LINE_1_INS	equ	3
LINE_2AA_INS	equ	6
LINE_2ECS_INS	equ	5
LINE_3_INS	equ	3
LINE_4AA_INS	equ	6
LINE_4ECS_INS	equ	5

; The tables take on the form:
; Previous Scroll 1 line away:  Next 1 line away, Next 2 lines away, Next 3> lines away
; Previous Scroll 2 lines away: Next 1 line away, Next 2 lines away, Next 3> lines away

; If only scrolling 1 line, the copper instructions are:
AAScroll1Line:
	dc.w	LINE_2AA_INS,(LINE_2AA_INS+LINE_4AA_INS),(LINE_2AA_INS+LINE_4AA_INS)
	dc.w	(LINE_1_INS+LINE_2AA_INS),(LINE_1_INS+LINE_2AA_INS+LINE_4AA_INS),(LINE_1_INS+LINE_2AA_INS+LINE_4AA_INS)
ScrollLineTableSize	equ	*-AAScroll1Line
; If scrolling 2 or more lines:
AAScroll2Lines:
	dc.w	(LINE_2AA_INS+LINE_3_INS),(LINE_2AA_INS+LINE_3_INS+LINE_4AA_INS),(LINE_2AA_INS+LINE_3_INS+LINE_4AA_INS)
	dc.w	(LINE_1_INS+LINE_2AA_INS+LINE_3_INS),(LINE_1_INS+LINE_2AA_INS+LINE_3_INS+LINE_4AA_INS),(LINE_1_INS+LINE_2AA_INS+LINE_3_INS+LINE_4AA_INS)

ECSScroll1Line:
	dc.w	LINE_2ECS_INS,(LINE_2ECS_INS+LINE_4ECS_INS),(LINE_2ECS_INS+LINE_4ECS_INS)
	dc.w	(LINE_1_INS+LINE_2ECS_INS),(LINE_1_INS+LINE_2ECS_INS+LINE_4ECS_INS),(LINE_1_INS+LINE_2ECS_INS+LINE_4ECS_INS)
ECSScroll2Lines:
	dc.w	(LINE_2ECS_INS+LINE_3_INS),(LINE_2ECS_INS+LINE_3_INS+LINE_4ECS_INS),(LINE_2ECS_INS+LINE_3_INS+LINE_4ECS_INS)
	dc.w	(LINE_1_INS+LINE_2ECS_INS+LINE_3_INS),(LINE_1_INS+LINE_2ECS_INS+LINE_3_INS+LINE_4ECS_INS),(LINE_1_INS+LINE_2ECS_INS+LINE_3_INS+LINE_4ECS_INS)

CountLineControl:
	movem.l	d2-d7/a2-a6,-(sp)
	moveq	#0,d1			; d1 will count the instructions
	move.w	fxh_Num(a0),d7
	beq	CountLineControl.
	moveq	#0,d4			; use d4 to divide by 2 for lace
	move.l	stk_ViewPort(a5),a1
	btst.b	#2,vp_Modes+1(a1)	; laced?
	beq.s	4$
	moveq	#1,d4
4$:
	move.l	fxh_First(a0),a1	; First LineControl structure
	subq.w	#1,d7
	move.w	lc_Line(a1),d6
	movem.l	d5/a3,-(sp)	
	lea	AAScroll1Line(pc),a3
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a4)
	bne.s	1$
	lea	ECSScroll1Line(pc),a3
1$:
	; What type is the first ScrollLine?
	; We know it is >1 line from the previous ScrollLine.
	; If there is another in the list, how far is it from there?
	moveq	#0,d5
	move.l	d5,d3
	tst.w	d7
	beq.s	20$
	move.w	lcp_SIZEOF+lc_Line(a1),d0
	sub.w	d6,d0
	sub.w	lc_Count(a1),d0
	asr.w	d4,d0
	beq.s	2$
	moveq	#NEXT_LC_2,d5
	subq.w	#1,d0
	beq.s	2$
20$:
	moveq	#NEXT_LC_3G,d5
2$:
	addq.w	#PREV_LC_2,d5
	move.w	d5,lcp_Type(a1)

	move.w	lc_Count(a1),d0
	asr.w	d4,d0
	cmp.w	#1,d0
	beq.s	3$
	add.w	#ScrollLineTableSize,d5
3$:
	move.w	0(a3,d5.w),d5		; This many copper instructions for the first LineControl.
	add.w	d5,d1
	asl.w	#2,d5			; 4 bytes per copper instruction
	move.w	d5,lcp_CopCount(a1)
	tst.w	d7
	beq.s	lc_counted		; no more to do.

	bra.s	count_lc_loop.
count_lc_loop:
	move.w	d6,d3
	add.w	lc_Count(a1),d3	; d3 is where the last LineControl finished
	lea	lcp_SIZEOF(a1),a1
	move.w	lc_Line(a1),d6
	; What type is this one? How far is it from the last? How far is it to the next?
	moveq	#0,d5
	move.w	d6,d0
	sub.w	d3,d0			; how far from the previous?
	asr.w	d4,d0
	beq.s	1$
	moveq	#PREV_LC_2,d5
1$:
	tst.w	d7
	bne.s	3$
	addq.w	#NEXT_LC_3G,d5
	bra.s	2$
3$:
	move.w	d6,d0
	add.w	lc_Count(a1),d0
	move.w	lcp_SIZEOF+lc_Line(a1),d2
	sub.w	d0,d2
	asr.w	d4,d2
	beq.s	2$
	addq.w	#2,d5
	subq.w	#1,d2
	beq.s	2$			; next is 2 away
	addq.w	#2,d5			; next is 3> away
2$:
	move.w	d5,lcp_Type(a1)
	cmp.w	#1,lc_Count(a1)
	beq.s	do_lc_count
	add.w	#ScrollLineTableSize,d5
do_lc_count:
	move.w	0(a3,d5.w),d5		; get the count
	add.w	d5,d1			; add it to the total
	asl.w	#2,d5			; 4 bytes per copper instruction
	move.w	d5,lcp_CopCount(a1)	; and store this count
count_lc_loop.
	dbra.s	d7,count_lc_loop

lc_counted:
	movem.l	(sp)+,d5/a3
	addq.w	#2,d1			; terminator
	bsr	DoUCopStuff(pc)

CountLineControl.
	movem.l	(sp)+,d2-d7/a2-a6
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; This code will build the UserCopperList for the LineControl
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> Stack frame
	; a6 -> UtilityBase

InstallLineControl:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a4,a6
	move.l	a0,a4			; store in a4 for safer keeping

	move.l	stk_DispHandle(a5),a3
	move.l	a0,(a3)+		; store the FxHandle in the AnimHandle
	move.l	a3,stk_DispHandle(a5)

	tst.w	fxh_Num(a0)
	beq	InstallLineControl.	; early sanity check

	; **********************************************************************
	;
	; This code calculates the deltas for DDFSTRT, BPLCON1, BPLMODx and FMODE
	; for 64 different positions, by faking out a ViewPort, and calling
	; ScrollVPort(). Then for each position, look through the vp->DspIns, and
	; store the results.
	;
	; **********************************************************************

	sub.l	#(vp_SIZEOF+ri_SIZEOF),sp
	move.l	sp,a2			; a2 -> dummy ViewPort
	lea	vp_SIZEOF(a2),a1	; a1 -> RasInfo
	move.l	stk_ViewPort(a5),a3
	moveq	#0,d0
	move.l	d0,vp_Next(a2)
	move.l	vp_ColorMap(a3),vp_ColorMap(a2)
	move.l	d0,vp_DspIns(a2)
	move.l	d0,vp_SprIns(a2)
	move.l	d0,vp_ClrIns(a2)
	move.l	d0,vp_UCopIns(a2)
	move.l	vp_DWidth(a3),vp_DWidth(a2)	; DWidth and DHeight
	move.l	vp_DxOffset(a3),vp_DxOffset(a2)	; DxOffset and DyOffset
	move.w	vp_DxOffset(a3),lch_DxOffset(a4)	; store the orignal DxOffset
	move.l	vp_Modes(a3),vp_Modes(a2)	; Modes, SpritePri and ExtendedModes
	bclr.b	#5,vp_Modes(a2)			; reset VP_HIDE
	move.l	a1,vp_RasInfo(a2)
	move.l	vp_RasInfo(a3),a3
	move.l	d0,ri_Next(a1)			; No DPF for now.
	move.l	ri_BitMap(a3),ri_BitMap(a1)
	move.l	d0,ri_RxOffset(a1)		; RxOffset and RyOffset
	move.l	a1,a3

	; a2 -> dummy ViewPort
	; a3 -> dummy RasInfo

	; Make the ViewPort

	FORBID	a1
	move.l	stk_View(a5),a0
	move.l	a2,a1
	move.l	v_ViewPort(a0),-(sp)
	move.l	a1,v_ViewPort(a0)	; fake this ViewPort into the View
	jsr	_LVOMakeVPort(a6)
	move.l	stk_View(a5),a0
	move.l	(sp)+,v_ViewPort(a0)
	PERMIT	save

	move.l	a4,-(sp)
	move.w	lch_OffsetCount(a4),d7
	subq.w	#1,d7			; for dbra
	lea	lch_SIZEOF(a4),a4	; point to the first lcOffsetTable
	move.l	vp_ColorMap(a2),d0
	beq.s	1$
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	1$
	move.b	cm_AuxFlags(a0),d2
	bclr.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)	; not exactly friendly, as the ColorMap is not a copy
1$:
	move.l	d2,-(sp)
	moveq	#0,d6			; d6 will store the BPLxMODS of the first position.
	moveq	#0,d5			; to show first time through.
	bra.s	search_ci

svp_loop:
	addq.w	#1,ri_RxOffset(a3)
	move.l	a2,a0
	jsr	_LVOScrollVPort(a6)

	; look for  FMode, Modulos, BplCon1, and DDFStrt
	; to store in the lcOffsetTable holdings.
search_ci:
	move.l	vp_DspIns(a2),d0
*	beq	FAILURE_3
	move.l	d0,a0
	move.l	cl_CopIns(a0),d0
*	beq	FAILURE_3
	move.l	d0,a0
lc_curr_loop:
	lea	ci_SIZEOF(a0),a0	; skip the first WAIT
	btst.b	#0,ci_OpCode+1(a0)	; last WAIT?
	bne	searched_ci.
	move.w	ci_DestAddr(a0),d0
	move.w	ci_DestData(a0),d1
	and.w	#$fff,d0
	cmp.w	#(BPLCON0&$fff),d0
	beq.s	lc_curr_con0
	cmp.w	#(BPLCON1&$fff),d0
	beq.s	lc_curr_con1
	cmp.w	#(DDFSTRT&$fff),d0
	beq.s	lc_curr_ddfstrt
	cmp.w	#(DDFSTOP&$fff),d0
	beq.s	lc_curr_ddfstop
	cmp.w	#(FMODE&$fff),d0
	beq.s	lc_curr_fmode
	cmp.w	#(BPL2PTH&$fff),d0
	beq	lc_curr_ptr2
	cmp.w	#(BPL1PTH&$fff),d0
	beq.s	lc_curr_ptr1
	cmp.w	#(BPLMOD1&$fff),d0
	beq.s	lc_curr_MOD1
	cmp.w	#(BPLMOD2&$fff),d0
	bne.s	lc_curr_loop
lc_curr_MOD2:
	tst.l	d5
	bne.s	1$
	move.w	d1,d6
	move.w	d1,stk_ZeroModEven(a5)
	moveq	#1,d5		; assume we always write BPLMOD1 then BPLMOD2 in the copper lists
1$:
	sub.w	d6,d1
	move.w	d1,lco_BPLMOD2(a4)
	bra.s	lc_curr_loop
lc_curr_MOD1:
	swap	d6
	tst.l	d5
	bne.s	1$
	move.w	d1,d6
	move.w	d1,stk_ZeroModOdd(a5)
1$:
	sub.w	d6,d1
	move.w	d1,lco_BPLMOD1(a4)
	swap	d6
	bra.s	lc_curr_loop
lc_curr_con0:
	move.w	d1,stk_BPLCON0(a5)
	bra	lc_curr_loop
lc_curr_con1:
	move.w	d1,lco_BPLCON1(a4)
	bra	lc_curr_loop
lc_curr_ddfstrt:
	move.w	d1,lco_DDFSTRT(a4)
	bra	lc_curr_loop
lc_curr_ddfstop:
	move.w	d1,lco_DDFSTOP(a4)
	tst.l	d5
	bne	lc_curr_loop
	move.w	d1,stk_ZeroDDFSTOP(a5)
	bra	lc_curr_loop
lc_curr_fmode:
	move.w	d1,lco_FMODE(a4)
	bra	lc_curr_loop
lc_curr_ptr1:
	move.w	d1,d2			; store the bplptr1 in d2
	swap	d2
	lea	ci_SIZEOF(a0),a0	; assume low part of the addess is the next instruction
	move.w	ci_DestData(a0),d2
	cmp.l	#1,d5			; have we just found the first BPLMOD2?
	bne	lc_curr_loop
	move.l	d2,stk_ZeroBPL1PTR(a5)
	moveq	#2,d5
	bra	lc_curr_loop
lc_curr_ptr2:
	move.w	d1,d4			; store the bplptr2 in d4
	swap	d4
	lea	ci_SIZEOF(a0),a0	; assume low part of the addess is the next instruction
	move.w	ci_DestData(a0),d4
	cmp.l	#2,d5
	bne	lc_curr_loop
	move.l	d4,stk_ZeroBPL2PTR(a5)
	move.l	#3,d5
	bra	lc_curr_loop

searched_ci.:
	move.w	lco_FMODE(a4),d1
	moveq	#0,d0
	move.l	a6,-(sp)
	move.l	gb_bwshifts(a6),a6
	and.w	#3,d1
	move.b	0(a6,d1.w),d0	; fmode shift value
	; there may be an extra offset to consider if this position is a different
	; fmode than the original, in which case the ddfstop may be different. We
	; therefore have to add the extra line length as
	; (((lco_DDFSTOP - original DDFSTOP) / cycles) * words per cycle)
	move.w	lco_DDFSTOP(a4),d3
	sub.w	stk_ZeroDDFSTOP(a5),d3
*	beq.s	lc_curr_fmode_2
	tst.w	d1
	beq.s	lc_curr_fmode_1x
	cmp.w	#BANDWIDTH_4X,d1
	beq.s	lc_curr_fmode_4x
lc_curr_fmode_2x:
	moveq	#2,d1		; assume 8 cycle. Can only be 4 cycle if SHires
	btst.b	#6,stk_BPLCON0+1(a5)
	beq.s	1$
	moveq	#1,d1		; is indeed a 4 cycle
1$:
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
*	bra.s	lc_curr_fmode_1
	bra.s	compensate
lc_curr_fmode_1x:
	moveq	#2,d1		; assume 8 cycle. Can be 4 cycle if Hires, 2 cycle is SHires
	tst.w	stk_BPLCON0(a5)
	bmi.s	2$		; Hires
	btst.b	#6,stk_BPLCON0+1(a5)
	beq.s	1$
	moveq	#0,d1		; a 2 cycle for SHires
	bra.s	1$
2$:
	moveq	#1,d1		; a 4 cycle for Hires
1$:
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
compensate:
	sub.l	stk_ZeroBPL1PTR(a5),d2
	sub.w	d3,d2
	move.w	d2,lco_CompensateEven(a4)
	sub.l	stk_ZeroBPL2PTR(a5),d4
	sub.w	d3,d4
	move.w	d4,lco_CompensateOdd(a4)
	bra.s	lc_curr_fmode_1
lc_curr_fmode_4x:
	moveq	#2,d1		; must be an 8 cycle
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
lc_curr_fmode_1:
	add.w	d3,lco_BPLMOD1(a4)
	add.w	d3,lco_BPLMOD2(a4)

lc_curr_fmode_2:
	addq.w	#1,d0		; 0, 1 or 2 becomes 1, 2 or 3, which is how much 
				; we shift when dealing with whole jumps.
	move.w	d0,lco_ModDiff(a4)
	move.l	(sp)+,a6
	DEBUGTABLES
	lea	lco_SIZEOF(a4),a4
	dbra.s	d7,svp_loop

	move.l	(sp)+,d2	; cm_AuxFlags
	move.l	(sp)+,a4	; FxHandle
	move.l	vp_ColorMap(a2),d0
	beq.s	1$
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	1$
	move.b	d2,cm_AuxFlags(a0)
1$:
	move.l	a2,a0		; free up the memory taken by the MakeVPort().
	jsr	_LVOFreeVPortCopLists(a6)
	add.l	#(vp_SIZEOF+ri_SIZEOF),sp

	; ************************************
	;
	; checkpoint
	; a4 -> struct FxHandle
	; a5 -> Stack stuff
	; a6 -> GfxBase
	;
	; Now build the copper list!
	;
	; ************************************

	moveq	#0,d7
	move.w	fxh_Num(a4),d7
	subq.w	#1,d7
	; if laced, use the upper word of d7 to show this.
	move.l	stk_ViewPort(a5),a0
	jsr	new_mode(pc)
	and.b	#V_LACE,d0
	beq.s	2$
	or.l	#$10000,d7
2$:
	move.l	fxh_UCopList(a4),a2
	move.l	fxh_First(a4),a3	; first lc_ structure
	moveq	#0,d6			; BPLMOD1:BPLMOD2 differences from the origin

build_lc_loop:
	swap	d7
	; d4 will hold the offset from the FxHandle of the Offset table for the
	; odd:even bitplanes
	jsr	GetOffsetTable

	; check if there is a difference between the DDFSTRT values of the
	; even and odd offsets.
	moveq	#0,d1
	move.l	d4,d3
	swap	d3
	move.w	lco_DDFSTRT(a4,d4.w),d0
	cmp.w	lco_DDFSTRT(a4,d3.w),d0
	beq.s	bld_no_DPFDiff
	blt.s	bld_fudge_even
	; the odd DDFSTRT is the smaller, so store that and calculate the extra
	; offset for Odd modulos.
	move.w	lco_DDFSTRT(a4,d3.w),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	bra.s	bld_no_DPFDiff
bld_fudge_even:
	move.w	lco_ModDiff(a4,d3.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	swap	d1
bld_no_DPFDiff:
	move.l	d1,stk_OddOffset(a5)	; OddOffset:EvenOffset
	move.w	d0,stk_DPFDDFSTRT(a5)

	move.w	lcp_Type(a3),d5
	DEBUGTYPE
	cmp.w	#PREV_LC_2,d5
	blt	build_line2_chk
	; here the previous section was 2 or more lines away, so we need a
	; LINE 1
	subq.w	#PREV_LC_2,d5
	move.w	lc_Line(a3),d0
	subq.w	#1,d0			; Y value
	sub.w	d7,d0			; (-1 if laced)
	DEBUGWAIT
	moveq	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	lc_RasInfo(a3),a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	1$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
1$:
	move.w	stk_WholeEven(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateEven(a4,d4.w),d1
	sub.w	stk_EvenOffset(a5),d1
	move.w	d1,d6
	move.w	d1,d2
	add.w	stk_ZeroModEven(a5),d1
	move.l	a0,-(sp)
	DEBUGMODS	"L1M2 "," "
	CMOVE	#BPLMOD2,d1
	swap	d6
	move.l	(sp)+,a0
	move.l	ri_Next(a0),d0
	beq.s	bld_no_dpf_line1	; ??? What about SCANDBL ???
	move.l	d0,a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	2$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
2$:
	swap	d4
	move.w	stk_WholeOdd(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateOdd(a4,d4.w),d1
	sub.w	stk_OddOffset(a5),d1
	move.w	d1,d2
	add.w	stk_ZeroModOdd(a5),d1
	swap	d4
bld_no_dpf_line1
	move.w	d2,d6
	DEBUGMODS	"L1M1 "," "
	CMOVE	#BPLMOD1,d1
	swap	d6

build_line2_chk:
	; we always need a LINE2, so do the easy bits first.
	moveq	#0,d1
	move.l	d1,d0
	move.w	lc_Line(a3),d0
	DEBUGWAIT
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.w	stk_DPFDDFSTRT(a5),d1
	CMOVE	#DDFSTRT,d1
	move.l	lc_RasInfo(a3),a0
	tst.l	ri_Next(a0)
	bne.s	bld_dpf_line2
	move.w	lco_BPLCON1(a4,d4.w),d1
	bra.s	bld_con1_line2
bld_dpf_line2:
	move.w	lco_BPLCON1(a4,d4.w),d1
	and.w	#$f0f0,d1		; mask the even pfd scroll
	swap	d4
	move.w	lco_BPLCON1(a4,d4.w),d0
	and.w	#$0f0f,d0		; mask the odd pfd scroll
	or.w	d0,d1
	swap	d4
bld_con1_line2
	CMOVE	#BPLCON1,d1
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	bld_mods_line2
	move.w	lco_FMODE(a4,d4.w),d1
	CMOVE	#FMODE,d1
bld_mods_line2:
	; now the modulos.
	; From our table, we deduce:
	; if (the height of this block == 1)
	; {
	;    if (the next block is 1 line away)
	;        calculate modulos to point to bitplanes of the next block's offset
	;    if (the next block is 2 or more lines away)
	;        calculate modulos to point to restore the bplptrs
	; }
	; else  // height of this block is 2 or more lines
	;    calculate the modulos to point to start of next line in this block
	move.w	lc_Count(a3),d3
	cmp.w	#1,d3
	bne	bld_mods_h2_line2
	tst.w	d5
	beq.s	bld_mods_h1_line2
	; height is 1, and the next block is 2 or more lines away, so
	; calculate the modulos to put the bplptrs back to their orignal setting.
	move.w	lco_BPLMOD2(a4,d4.w),d2
	sub.w	d2,d6
	move.l	stk_ZeroModOdd(a5),d1	; Odd:Even
	sub.w	d6,d1
	sub.w	stk_EvenOffset(a5),d1
	DEBUGMODS	"L2H1N2M2 "," "
	CMOVE	#BPLMOD2,d1
	swap	d6
	swap	d1
	swap	d4
	move.w	lco_BPLMOD1(a4,d4.w),d2
	sub.w	d2,d6
	move.w	stk_ZeroModOdd(a5),d1
	sub.w	d6,d1
	sub.w	stk_OddOffset(a5),d1
	swap	d6
	swap	d4
	DEBUGMODS	"L2H1N2M1 "," "
	CMOVE	#BPLMOD1,d1
	bra	build_line3_chk

bld_mods_h1_line2:
	; height is 1, and the next block is 1 line away, so calculate the modulos
	; to put the bplptrs at the start of the next line.
	;
	; First, simulate the code for case height = 1, next block >= 2 lines away
	move.l	lco_BPLMOD1(a4,d4.w),d1	; MOD1:MOD2
	sub.w	d1,d6
	swap	d6
	swap	d1
	sub.w	d1,d6
	swap	d6

bld_mods_h1_line2_3:
	; Now combine the code that calculates the modulos as if the next
	; block were >2 lines away (line 3 code), and the code that works out
	; the modulos for a new block (line 1 code).
	; The code is simplifed, but basically the line 3 simulation works out
	; to be (ZeroModEven + d6 - ZeroModEven) = d6. This is then subtracted
	; from the line 1 calculation.
	movem.l	a3/d4,-(sp)
	move.l	lc_RasInfo(a3),a0	; get the RasInfo of the current LineControl
	move.l	a0,d3			; and save it.
	move.w	lcp_Type(a3),d5		; Also store its type.
	lea	lcp_SIZEOF(a3),a3
	jsr	GetOffsetTable		; get new d4 value
	move.l	d3,a0

	; check if there is a difference between the DDFSTRT values of the
	; even and odd offsets.
	moveq	#0,d1
	move.l	d4,d3
	swap	d3
	move.w	lco_DDFSTRT(a4,d4.w),d0
	cmp.w	lco_DDFSTRT(a4,d3.w),d0
	beq.s	bld2_no_DPFDiff
	blt.s	bld2_fudge_even
	; the odd DDFSTRT is the smaller, so store that and calculate the extra
	; offset for Odd modulos.
bld2_fudge_odd:
	move.w	d1,d2
	move.w	lco_DDFSTRT(a4,d3.w),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	bra.s	frog
bld2_fudge_even:
	move.w	lco_ModDiff(a4,d3.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	cmp.w	stk_DPFDDFSTRT(a5),d0
	bge.s	1$
	neg.w	d1
1$:
	swap	d1
frog:
	add.w	d1,stk_EvenOffset(a5)
	swap	d1
	add.w	d1,stk_OddOffset(a5)
	bra.s	goat
bld2_no_DPFDiff:
	; is this DPF?
	tst.l	ri_Next(a0)
	beq.s	goat		; no, so don't touch the Offset stuff.
	; is there a difference between the current DDFSTRT and the next DDFSTRT?
	move.w	d0,d1
	move.w	stk_DPFDDFSTRT(a5),d0
	cmp.w	d0,d1
	beq.s	goat
	; yes there is.
	moveq	#1,d3
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d3
	cmp.w	d0,d1
	bge.s	1$
	neg.w	d3
1$:
	; Is the LineControl more than 1 line from the previous?
	cmp.w	#PREV_LC_2,d5
	blt.s	bld2_fudge1
	tst.l	stk_OddOffset(a5)
	beq.s	goat
	swap	d3
	move.w	stk_EvenOffset(a5),d0
	cmp.w	stk_OddOffset(a5),d0
	blt.s	2$
	swap	d3
2$:
	move.l	d3,stk_OddOffset(a5)
	bra.s	goat
bld2_fudge1:
	move.w	stk_EvenOffset(a5),d0
	cmp.w	stk_OddOffset(a5),d0
	beq.s	goat
	blt.s	1$
	sub.w	d3,stk_EvenOffset(a5)
	bra.s	goat
1$:
	sub.w	d3,stk_OddOffset(a5)
	

goat:
	move.l	lc_RasInfo(a3),a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	1$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
1$:
	move.w	stk_WholeEven(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateEven(a4,d4.w),d1
	move.w	d1,d2
	sub.w	stk_EvenOffset(a5),d1
	add.w	stk_ZeroModEven(a5),d1
	sub.w	d6,d1
	move.w	d2,d6
	move.l	a0,-(sp)
	DEBUGMODS	"L2H1N2M2 "," "
	CMOVE	#BPLMOD2,d1
	move.l	(sp)+,a0
	swap	d6
	move.l	ri_Next(a0),d0
	beq.s	bld_mods_h1_line2_nodpf
	move.l	d0,a0
bld_mods_h1_line2_nodpf:
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	2$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
2$:
	swap	d4
	move.w	stk_WholeOdd(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateOdd(a4,d4.w),d1
	move.w	d1,d2
	sub.w	stk_OddOffset(a5),d1
	add.w	stk_ZeroModOdd(a5),d1
	sub.w	d6,d1
	swap	d4
	move.w	d2,d6
	DEBUGMODS	"L2H1N2M1 ",10
	CMOVE	#BPLMOD1,d1
	swap	d6
	movem.l	(sp)+,a3/d4
	bra	build_lc_loop.		; this LineControl is finished.
bld_mods_h2_line2:
	; height is 2 or more, so we just need the normal modulo value
	move.l	stk_ZeroModOdd(a5),d1	; Odd:Even
	sub.w	stk_EvenOffset(a5),d1
	subq.w	#1,d3			; d3 = count
	move.w	lco_BPLMOD2(a4,d4.w),d2
	; check out the repeat stuff.
	cmp.w	#1,lc_SkipRateEven(a3)
	beq.s	no_rpt_even		; there isn't any.
	move.l	lc_RasInfo(a3),a0	; no checking needed at this point.
	move.l	ri_BitMap(a0),d0
	beq.s	no_rpt_even
	move.l	d0,a0
	move.w	bm_BytesPerRow(a0),d0
	; calculate the modulo for the repetition as
	; (-BytesPerRow + (SkipRate + 1))
	muls	lc_SkipRateEven(a3),d0
	sub.w	bm_BytesPerRow(a0),d0
	add.w	d0,d1			; offset the modulo by this much to repeat.
	muls	d3,d0			; but the difference in d6 is the
	sub.w	d0,d6			; bytes per row * rows.
no_rpt_even:
	add.w	d2,d1
*	add.w	lco_CompensateEven(a4,d4.w),d1
	sub.w	d2,d6
	DEBUGMODS	"L2H2N2M2 "," "
	CMOVE	#BPLMOD2,d1
	swap	d1
	sub.w	stk_OddOffset(a5),d1
	swap	d4
	move.w	lco_BPLMOD1(a4,d4.w),d2
	swap	d6
	cmp.w	#1,lc_SkipRateOdd(a3)
	beq.s	no_rpt_odd
	move.l	lc_RasInfo(a3),a0	; no checking needed at this point.
	move.l	ri_Next(a0),d0
	beq.s	1$
	move.l	d0,a0
1$:
	move.l	ri_BitMap(a0),d0
	beq.s	no_rpt_odd
	move.l	d0,a0
	move.w	bm_BytesPerRow(a0),d0
	; calculate the modulo for the repetition as
	; (-BytesPerRow + (SkipRate + 1))
	muls	lc_SkipRateEven(a3),d0
	sub.w	bm_BytesPerRow(a0),d0
	add.w	d0,d1			; offset the modulo by this much to repeat.
	muls	d3,d0			; but the difference in d6 is the
	sub.w	d0,d6			; bytes per row * rows.
no_rpt_odd:
	add.w	d2,d1
*	add.w	lco_CompensateOdd(a4,d4.w),d1
	sub.w	d2,d6
	swap	d6
	DEBUGMODS	"L2H1N2M1 "," "
	CMOVE	#BPLMOD1,d1
	swap	d4

build_line3_chk:
	; Check if we need to build LINE_3
	; we always need LINE_3 if this block is 2 or more lines high.
	move.w	lc_Count(a3),d0
	cmp.w	#2,d0
	blt.s	build_line4_chk
	add.w	lc_Line(a3),d0
	subq.w	#1,d0		; line number
	sub.w	d7,d0		; (-1 if laced)
	moveq	#0,d1
	DEBUGWAIT
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	; now the modulos.	
	; From our table, we deduce:
	; if (the next block is only 1 line away)
	;    calculate the modulos to point to bitplanes of the next block's offset
	; else
	;    calculate the modulos to restore the bitplanes as they were.
	tst.w	d5
	beq	bld_mods_h1_line2_3	; same code
bld_mods_h2_line3:
	; next block is 2 or more lines away.
	move.w	stk_ZeroModEven(a5),d1
	sub.w	stk_EvenOffset(a5),d1
	sub.w	d6,d1
*	add.w	lco_CompensateEven(a4,d4.w),d1
	DEBUGMODS	"L3H2N2M2 "," "
	CMOVE	#BPLMOD2,d1
	swap	d6
	move.w	stk_ZeroModOdd(a5),d1
	sub.w	stk_OddOffset(a5),d1
	swap	d4
	sub.w	d6,d1
*	add.w	lco_CompensateOdd(a4,d4.w),d1
	swap	d4
	DEBUGMODS	"L3H2N2M1 "," "
	CMOVE	#BPLMOD1,d1
	swap	d6

build_line4_chk:
	; check if we need to build LINE_4.
	; We always need LINE_4 if the next block is 2 or more lines away.
	tst.w	d5
	beq	build_lc_loop.
	move.w	lc_Count(a3),d0
	add.w	lc_Line(a3),d0
	moveq	#0,d1
	DEBUGWAIT
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	moveq	#lch_SIZEOF,d1		; use the first Offset table to restore
	move.w	d1,d4
	swap	d4
	move.w	d1,d4
	move.w	lco_DDFSTRT(a4,d4.w),d1
	CMOVE	#DDFSTRT,d1
	move.l	lc_RasInfo(a3),a0
	tst.l	ri_Next(a0)
	bne.s	bld_dpf_line4
	move.w	lco_BPLCON1(a4,d4.w),d1
	bra.s	bld_con1_line4
bld_dpf_line4:
	move.w	lco_BPLCON1(a4,d4.w),d1
	and.w	#$f0f0,d1		; mask the even pfd scroll
	swap	d4
	move.w	lco_BPLCON1(a4,d4.w),d0
	and.w	#$0f0f,d0		; mask the odd pfd scroll
	or.w	d0,d1
bld_con1_line4:
	CMOVE	#BPLCON1,d1
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	bld_mods_line4
	move.w	lco_FMODE(a4,d4.w),d1
	CMOVE	#FMODE,d1
bld_mods_line4:
	cmp.w	#NEXT_LC_3G,d5
	bge.s	bld_mods_line4_n3
	moveq	#0,d6
	bra.s	build_lc_loop.
bld_mods_line4_n3:
	; next is 3 or more lines away, so just restore the modulo values
	move.w	stk_ZeroModEven(a5),d1
	DEBUGMODS	"L4M2 "," "
	CMOVE	#BPLMOD2,d1
	swap	d4
	move.w	stk_ZeroModOdd(a5),d1
	DEBUGMODS	"L4M1 ",10
	CMOVE	#BPLMOD1,d1
	moveq	#0,d6

build_lc_loop.:
	lea	lcp_SIZEOF(a3),a3
	swap	d7			; count back in low word
	dbra	d7,build_lc_loop

	move.l	stk_ZeroModOdd(a5),lch_ZeroModOdd(a4)	; store these for AnimateFX()

	move.l	#10000,d0
	move.l	#255,d1
	DEBUGWAIT
	move.l	a2,a1
	jsr	_LVOCWait(a6)		; CEND
	move.l	a2,a1
	jsr	_LVOCBump(a6)

InstallLineControl.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         AnimateLineControl
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

CHECK	EQU	0
NOCHECK	EQU	1

*********************************
INCCOP	MACRO	; count, newblock
		; increments the coplist by count instructions, and
		; ensures next instruction is a MOVE. If newblock is TRUE, then
		; checks that the next MOVE is to DDFSTRT or BPLMOD2
		;
		; corrupts d0
	tst.l	d5		; don't touch the copper list if we are past
	bmi.s	is_ours\@	; the end.
	IFNE	\2
	add.l	#(\1*4),a2	; increment by count (hx68 optimises this)
	ELSE
	add.l	#((\1-1)*4),a2	; increment by (count - 1)
	move.l	(a2)+,d0	; increment by one more, but
	cmp.l	#-2,d0		; check if this is the terminator.
	beq.s	cop_end\@
	ENDC
incw\@:
	move.w	(a2),d0
	btst.b	#0,d0		; MOVE?
	beq.s	inc\@
	cmp.l	#-2,(a2)	; terminator?
	beq.s	cop_end\@
	addq.l	#4,a2		; skip over the WAIT
	bra.s	incw\@		; and check again.
cop_end\@:
	or.l	#$80000000,d5	; set the upper bit of d5 to disable copper pokes.
	IFEQ	\2
	bra.s	is_ours\@	; stupid assembler barfs if branch size == 0.
	ENDC
inc\@:
	IFEQ	\2		; check the MOVE instruction
	cmp.w	#(DDFSTRT&$0fff),d0	
	bne.s	chk_mod2\@
	cmp.w	#(BPLCON1&$0fff),4(a2)	; if not BPLCON1, then this could be
					; the DDFSTRT of another screen,
	beq.s	is_ours\@
	bra.s	cop_end\@
chk_mod2\@:
	cmp.w	#(BPLMOD2&$0fff),d0
	bne.s	not_ours\@
	cmp.w	#(BPLMOD1&$0fff),4(a2)	; a SpecialFX screen?
	beq.s	is_ours\@
not_ours\@:
	addq.l	#4,a2		; skip this MOVE
	bra.s	incw\@		; and try again.
	ENDC
is_ours\@:
	ENDM
**********************************
POKECOP	MACRO	; offset,inc
		; pokes offset(a2) with d1.w, and updates the cache in a6.
		; Optionally increments the icl pointer.
	tst.l	d5		; don't poke the copperlist if past the
	bmi.s	nopoke\@	; end.
	move.w	d1,\1(a2)
nopoke\@:
	move.w	d1,(a6)+
	ENDM
**********************************
	
AnimateLineControl:
	movem.l	d7/a2/a5-a6,-(sp)
	move.l	fxh_First(a3),a4
	move.l	ah_ViewPort(a5),a2
	move.l	a5,a0
	sub.l	#FRAME_SIZE,sp
	move.l	sp,a5
	move.w	ah_CopperToUse(a0),d1
	asl.w	#3,d1
	move.l	a6,stk_SpecialFXBase(a5)
	move.l	a2,stk_ViewPort(a5)
	move.l	lch_ZeroModOdd(a3),stk_ZeroModOdd(a5)

	move.l	sfxb_GfxBase(a6),a6

	movem.l	d1/a0/a2/a4,-(sp)
	move.l	ah_Copper1S(a0,d1.w),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a3),d0	; jump to the first instruction in our effect
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	bsr.s	do_change_lc
	movem.l	(sp),d1/a0/a2/a4
1$:
	move.l	ah_Copper1L(a0,d1.w),d0
	beq.s	2$
	add.l	fxh_Offset(a3),d0	; jump to the first instruction in our effect
	move.l	d0,a2
	bsr.s	do_change_lc
2$:
	movem.l	(sp)+,d1/a0/a2/a4
	add.l	#FRAME_SIZE,sp
	movem.l	(sp)+,d7/a2/a5-a6
	bra	lc_change_interm


	; ******************************
	;
	; Change the actual copper lists
	;
	; ******************************
	; {

do_change_lc:
	moveq	#0,d5
	move.l	stk_ViewPort(a5),a0
	move.w	vp_Modes(a0),d0
	and.w	#V_VP_HIDE,d0
	beq.s	no_hw_lc_update		; ViewPort is hidden, so don't update the h/w copperlist
	or.l	#$80000000,d5		; Flag to show no copperlist updates
no_hw_lc_update:
	moveq	#-2,d3			; FFFF FFFE terminator
	move.w	fxh_Num(a3),d2
	beq	change_lc.

	; skip over the WAIT (there may be more than 1 if this is line 256).
look_for_move:
	tst.l	d5
	bmi.s	is_ours
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	change_lc
	cmp.l	(a2),d3			; end of the copperlist?
	bne.s	not_ours
	or.l	#$80000000,d5		; show no copperlist update
	bra.s	is_ours			; but do the calculations.
not_ours:
	addq.l	#4,a2			; it was a WAIT, so skip it.
	bra.s	look_for_move
change_lc:
	cmp.w	#(DDFSTRT&$0fff),d0	
	beq.s	is_ours
	cmp.w	#(BPLMOD2&$0fff),d0
	bne.s	not_ours
is_ours:
	; do the change:
	; calculate all the instruction's values.

	; checkpoint
	; a2 -> CopIns
	; a3 -> struct FxHandle
	; a4 -> struct LineControl
	; a5 -> stack stuff
	; a6 -> GfxBase

	moveq	#0,d7
	move.w	fxh_Num(a3),d7
	subq.w	#1,d7
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	anm_ecs
	bset.l	#31,d7			; set bit 31 of d7 to show AA
anm_ecs:
	moveq	#0,d6			; BPLMOD1:BPLMOD2 differences from the origin
	exg	a3,a4
anim_lc_loop:
	move.l	a6,-(sp)
	tst.w	fxh_Pad(a4)		; did we come from RebuildLineControl()?
	bmi.s	no_modify_chk
	btst.b	#LCB_MODIFY,lc_Flags+1(a3)
	beq	no_lc_change
no_modify_chk:
	lea	lcp_AnimCache(a3),a6	; use a6 to point to the AnimCache
	; d4 will hold the offset from the FxHandle of the Offset table for the
	; odd:even bitplanes
	jsr	GetOffsetTable

	; check if there is a difference between the DDFSTRT values of the
	; even and odd offsets.
	moveq	#0,d1
	move.l	d4,d3
	swap	d3
	move.w	lco_DDFSTRT(a4,d4.w),d0
	cmp.w	lco_DDFSTRT(a4,d3.w),d0
	beq.s	anm_no_DPFDiff
	blt.s	anm_fudge_even
	; the odd DDFSTRT is the smaller, so store that and calculate the extra
	; offset for Odd modulos.
	move.w	lco_DDFSTRT(a4,d3.w),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	bra.s	anm_no_DPFDiff
anm_fudge_even:
	move.w	lco_ModDiff(a4,d3.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	swap	d1
anm_no_DPFDiff:
	move.l	d1,stk_OddOffset(a5)	; OddOffset:EvenOffset
	move.w	d0,stk_DPFDDFSTRT(a5)

	move.w	lcp_Type(a3),d5
	DEBUGTYPE
	cmp.w	#PREV_LC_2,d5
	blt	anim_line2_chk
	; here the previous section was 2 or more lines away, so we need a
	; LINE 1
	subq.w	#PREV_LC_2,d5
	move.l	lc_RasInfo(a3),a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	1$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
1$:
	move.w	stk_WholeEven(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateEven(a4,d4.w),d1
	sub.w	stk_EvenOffset(a5),d1
	move.w	d1,d6
	move.w	d1,d2
	add.w	stk_ZeroModEven(a5),d1
	;	CMOVE	#BPLMOD2,d1
	swap	d6
	POKECOP	2
	move.l	ri_Next(a0),d0
	beq.s	anm_no_dpf_line1	; ??? What about SCANDBL ???
	move.l	d0,a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	2$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
2$:
	swap	d4
	move.w	stk_WholeOdd(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateOdd(a4,d4.w),d1
	sub.w	stk_OddOffset(a5),d1
	move.w	d1,d2
	add.w	stk_ZeroModOdd(a5),d1
	swap	d4
anm_no_dpf_line1
	move.w	d2,d6
	;	CMOVE	#BPLMOD1,d1
	swap	d6
	POKECOP	6
	INCCOP	3,CHECK			; increment the copper pointer over LINE1
					; by skipping two MOVE MODs,
					; and the next WAIT

anim_line2_chk:
	; we always need a LINE2, so do the easy bits first.
	move.w	stk_DPFDDFSTRT(a5),d1
	;	CMOVE	#DDFSTRT,d1
	POKECOP	2
	move.l	lc_RasInfo(a3),a0
	tst.l	ri_Next(a0)
	bne.s	anm_dpf_line2
	move.w	lco_BPLCON1(a4,d4.w),d1
	bra.s	anm_con1_line2
anm_dpf_line2:
	move.w	lco_BPLCON1(a4,d4.w),d1
	and.w	#$f0f0,d1		; mask the even pfd scroll
	swap	d4
	move.w	lco_BPLCON1(a4,d4.w),d0
	and.w	#$0f0f,d0		; mask the odd pfd scroll
	or.w	d0,d1
	swap	d4
anm_con1_line2
	;	CMOVE	#BPLCON1,d1
	POKECOP	6
	INCCOP	2,NOCHECK
	tst.l	d7			; AA?
	bpl.s	anm_mods_line2
	move.w	lco_FMODE(a4,d4.w),d1
	;	CMOVE	#FMODE,d1
	POKECOP	2
	INCCOP	1,NOCHECK
anm_mods_line2:
	; now the modulos.
	; From our table, we deduce:
	; if (the height of this block == 1)
	; {
	;    if (the next block is 1 line away)
	;        calculate modulos to point to bitplanes of the next block's offset
	;    if (the next block is 2 or more lines away)
	;        calculate modulos to point to restore the bplptrs
	; }
	; else  // height of this block is 2 or more lines
	;    calculate the modulos to point to start of next line in this block
	move.w	lc_Count(a3),d3
	cmp.w	#1,d3
	bne	anm_mods_h2_line2
	tst.w	d5
	beq	anm_mods_h1_line2
	; height is 1, and the next block is 2 or more lines away, so
	; calculate the modulos to put the bplptrs back to their orignal setting.
	move.w	lco_BPLMOD2(a4,d4.w),d2
	sub.w	d2,d6
	move.l	stk_ZeroModOdd(a5),d1	; Odd:Even
	sub.w	d6,d1
	sub.w	stk_EvenOffset(a5),d1
	;	CMOVE	#BPLMOD2,d1
	swap	d4
	POKECOP	2
	swap	d6
	swap	d1
	move.w	lco_BPLMOD1(a4,d4.w),d2
	sub.w	d2,d6
	move.w	stk_ZeroModOdd(a5),d1
	sub.w	d6,d1
	sub.w	stk_OddOffset(a5),d1
	swap	d6
	swap	d4
	;	CMOVE	#BPLMOD1,d1
	POKECOP	6
	INCCOP	3,CHECK			; skip over the two mods and the next WAIT
	bra	anim_line3_chk
anm_mods_h1_line2:
	; height is 1, and the next block is 1 line away, so calculate the modulos
	; to put the bplptrs at the start of the next line.
	;
	; First, simulate the code for case height = 1, next block >= 2 lines away
	move.l	lco_BPLMOD1(a4,d4.w),d1	; MOD1:MOD2
	sub.w	d1,d6
	swap	d6
	swap	d1
	sub.w	d1,d6
	swap	d6
anm_mods_h1_line2_3:
	; Now combine the code that calculates the modulos as if the next
	; block were >2 lines away (line 3 code), and the code that works out
	; the modulos for a new block (line 1 code).
	; The code is simplifed, but basically the line 3 simulation works out
	; to be (ZeroModEven + d6 - ZeroModEven) = d6. This is then subtracted
	; from the line 1 calculation.
	movem.l	a3/d4,-(sp)
	move.l	lc_RasInfo(a3),d3
	move.w	lcp_Type(a3),d5
	lea	lcp_SIZEOF(a3),a3
	jsr	GetOffsetTable		; get new d4 value
	move.l	d3,a0

	; check if there is a difference between the DDFSTRT values of the
	; even and odd offsets.
	moveq	#0,d1
	move.l	d4,d3
	swap	d3
	move.w	lco_DDFSTRT(a4,d4.w),d0
	cmp.w	lco_DDFSTRT(a4,d3.w),d0
	beq.s	anm2_no_DPFDiff
	blt.s	anm2_fudge_even
	; the odd DDFSTRT is the smaller, so store that and calculate the extra
	; offset for Odd modulos.
anm2_fudge_odd:
	move.w	d1,d2
	move.w	lco_DDFSTRT(a4,d3.w),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	bra.s	afrog
anm2_fudge_even:
	move.w	lco_ModDiff(a4,d3.w),d2
	moveq	#1,d1
	asl.w	d2,d1
	cmp.w	stk_DPFDDFSTRT(a5),d0
	bge.s	1$
	neg.w	d1
1$:
	swap	d1
afrog:
	add.w	d1,stk_EvenOffset(a5)
	swap	d1
	add.w	d1,stk_OddOffset(a5)
	bra.s	agoat
anm2_no_DPFDiff:
	; is this DPF?
	tst.l	ri_Next(a0)
	beq.s	agoat		; no, so don't touch the Offset stuff.
	; is there a difference between the current DDFSTRT and the next DDFSTRT?
	move.w	d0,d1
	move.w	stk_DPFDDFSTRT(a5),d0
	cmp.w	d0,d1
	beq.s	agoat
	; yes there is.
	moveq	#1,d3
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d3
	cmp.w	d0,d1
	bge.s	1$
	neg.w	d3
1$:
	; Is the LineControl more than 1 line from the previous?
	cmp.w	#PREV_LC_2,d5
	blt.s	anm2_fudge1
	tst.l	stk_OddOffset(a5)
	beq.s	agoat
	swap	d3
	move.w	stk_EvenOffset(a5),d0
	cmp.w	stk_OddOffset(a5),d0
	blt.s	2$
	swap	d3
2$:
	move.l	d3,stk_OddOffset(a5)
	bra.s	agoat
anm2_fudge1:
	move.w	stk_EvenOffset(a5),d0
	cmp.w	stk_OddOffset(a5),d0
	beq.s	agoat
	blt.s	1$
	sub.w	d3,stk_EvenOffset(a5)
	bra.s	agoat
1$:
	sub.w	d3,stk_OddOffset(a5)

agoat:
	move.l	lc_RasInfo(a3),a0
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	1$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
1$:
	move.w	stk_WholeEven(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateEven(a4,d4.w),d1
	move.w	d1,d2
	sub.w	stk_EvenOffset(a5),d1
	add.w	stk_ZeroModEven(a5),d1
	sub.w	d6,d1
	move.w	d2,d6
	;	CMOVE	#BPLMOD2,d1
	POKECOP	2
	swap	d6
	move.l	ri_Next(a0),d0
	beq.s	anm_mods_h1_line2_nodpf
	move.l	d0,a0
anm_mods_h1_line2_nodpf:
	move.w	ri_RyOffset(a0),d1
	sub.w	lc_Line(a3),d1		; offset from the display line to the bitmap line wanted
	beq.s	2$
	move.l	ri_BitMap(a0),a1
	muls	bm_BytesPerRow(a1),d1
2$:
	swap	d4
	move.w	stk_WholeOdd(a5),d0
	move.w	lco_ModDiff(a4,d4.w),d2
	asl.w	d2,d0
	sub.w	d0,d1
	add.w	lco_CompensateOdd(a4,d4.w),d1
	move.w	d1,d2
	sub.w	stk_OddOffset(a5),d1
	add.w	stk_ZeroModOdd(a5),d1
	sub.w	d6,d1
	swap	d4
	move.w	d2,d6
	;	CMOVE	#BPLMOD1,d1
	POKECOP	6
	INCCOP	3,CHECK
	swap	d6
	movem.l	(sp)+,a3/d4
	bra	anim_lc_loop.		; this LineControl is finished.
anm_mods_h2_line2:
	; height is 2 or more, so we just need the normal modulo value
	move.l	stk_ZeroModOdd(a5),d1	; Odd:Even
	sub.w	stk_EvenOffset(a5),d1
	subq.w	#1,d3			; d3 = count
	move.w	lco_BPLMOD2(a4,d4.w),d2
	cmp.w	#1,lc_SkipRateEven(a3)
	beq.s	anm_no_rpt_even
	move.l	lc_RasInfo(a3),a0	; no checking needed at this point.
	move.l	ri_BitMap(a0),d0
	beq.s	anm_no_rpt_even
	move.l	d0,a0
	move.w	bm_BytesPerRow(a0),d0
	; calculate the modulo for the repetition as
	; (-BytesPerRow + (SkipRate + 1))
	muls	lc_SkipRateEven(a3),d0
	sub.w	bm_BytesPerRow(a0),d0
	add.w	d0,d1			; offset the modulo by this much to repeat.
	muls	d3,d0			; but the difference in d6 is the
	sub.w	d0,d6			; bytes per row * rows.
anm_no_rpt_even:
	add.w	d2,d1
*	add.w	lco_CompensateEven(a4,d4.w),d1
	sub.w	d2,d6
	;	CMOVE	#BPLMOD2,d1
	swap	d4
	POKECOP	2
	swap	d1
	sub.w	stk_OddOffset(a5),d1
	move.w	lco_BPLMOD1(a4,d4.w),d2
	swap	d6
	cmp.w	#1,lc_SkipRateOdd(a3)
	beq.s	anm_no_rpt_odd
	move.l	lc_RasInfo(a3),a0	; no checking needed at this point.
	move.l	ri_Next(a0),d0
	beq.s	1$
	move.l	d0,a0
1$:
	move.l	ri_BitMap(a0),d0
	beq.s	anm_no_rpt_odd
	move.l	d0,a0
	move.w	bm_BytesPerRow(a0),d0
	muls	lc_SkipRateOdd(a3),d0
	sub.w	bm_BytesPerRow(a0),d0
	add.w	d0,d1			; offset the modulo by this much to repeat.
	muls	d3,d0			; but the difference in d6 is the
	sub.w	d0,d6			; bytes per row * rows.
anm_no_rpt_odd:
	add.w	d2,d1
*	add.w	lco_CompensateOdd(a4,d4.w),d1
	sub.w	d2,d6
	swap	d6
	;	CMOVE	#BPLMOD1,d1
	POKECOP	6
	INCCOP	3,CHECK			; skip two MODs and the next WAIT
	swap	d4

anim_line3_chk:
	; Check if we need to anim LINE_3
	; we always need LINE_3 if this block is 2 or more lines high.
	move.w	lc_Count(a3),d0
	cmp.w	#2,d0
	blt	anim_line4_chk
	; now the modulos.	
	; From our table, we deduce:
	; if (the next block is only 1 line away)
	;    calculate the modulos to point to bitplanes of the next block's offset
	; else
	;    calculate the modulos to restore the bitplanes as they were.
	tst.w	d5
	beq	anm_mods_h1_line2_3	; same code
	; next block is 2 or more lines away.
	move.w	stk_ZeroModEven(a5),d1
	sub.w	stk_EvenOffset(a5),d1
	sub.w	d6,d1
*	add.w	lco_CompensateEven(a4,d4.w),d1
	;	CMOVE	#BPLMOD2,d1
	POKECOP	2
	swap	d6
	move.w	stk_ZeroModOdd(a5),d1
	sub.w	stk_OddOffset(a5),d1
	swap	d4
	sub.w	d6,d1
*	add.w	lco_CompensateOdd(a4,d4.w),d1
	swap	d4
	;	CMOVE	#BPLMOD1,d1
	POKECOP	6
	INCCOP	3,CHECK			; skip two MODs and the next WAIT
	swap	d6

anim_line4_chk:
	; check if we need to anim LINE_4.
	; We always need LINE_4 if the next block is 2 or more lines away.
	tst.w	d5
	beq	anim_lc_loop.
	moveq	#lch_SIZEOF,d1		; use the first Offset table to restore
	move.w	d1,d4
	swap	d4
	move.w	d1,d4
	move.w	lco_DDFSTRT(a4,d4.w),d1
	;	CMOVE	#DDFSTRT,d1
	POKECOP	2
	move.l	lc_RasInfo(a3),a0
	tst.l	ri_Next(a0)
	bne.s	anm_dpf_line4
	move.w	lco_BPLCON1(a4,d4.w),d1
	bra.s	anm_con1_line4
anm_dpf_line4:
	move.w	lco_BPLCON1(a4,d4.w),d1
	move.w	d1,d3
	and.w	#$f0f0,d1		; mask the even pfd scroll
	and.w	#$0f0f,d3		; mask the odd pfd scroll
	or.w	d3,d1
anm_con1_line4:
	;	CMOVE	#BPLCON1,d1
	POKECOP	6
	INCCOP	2,NOCHECK		; increment by the DDFSTRT and BPLCON1
	tst.l	d7			; AA?
	bpl.s	anm_mods_line4
	move.w	lco_FMODE(a4,d4.w),d1
	;	CMOVE	#FMODE,d1
	POKECOP	2
	INCCOP	1,NOCHECK
anm_mods_line4:
	cmp.w	#NEXT_LC_3G,d5
	bge.s	anm_mods_line4_n3
	moveq	#0,d6
	bra.s	anim_lc_loop.
anm_mods_line4_n3:
	; next is 3 or more lines away, so just restore the modulo values
	move.w	stk_ZeroModEven(a5),d1
	;	CMOVE	#BPLMOD2,d1
	POKECOP	2
	move.w	stk_ZeroModOdd(a5),d1
	;	CMOVE	#BPLMOD1,d1
	POKECOP	6
	INCCOP	3,CHECK			; skip two MODs and the next WAIT
	moveq	#0,d6

anim_lc_loop.:
	lea	lcp_SIZEOF(a3),a3
	move.l	(sp)+,a6		; restore
	dbra	d7,anim_lc_loop
	exg	a3,a4
change_lc.:
	rts

no_lc_change:
	add.w	lcp_CopCount(a3),a2	; on to the next instruction
	bra.s	anim_lc_loop.

	; }

	; ***********************************
	;
	; Change the intermediate copperlists.
	;
	; ***********************************

INCIC	EQU	0
NOINC	EQU	1

**********************************
POKEICL	MACRO	offset,inc,check
		; pokes the offset intermediate copperinstruction from
		; a0 with (a1)+.
		; Optionally increments the ci_ pointer, and optionally
		; check for a MOVE.
	move.w	(a1)+,((ci_SIZEOF*\1)+ci_DestData)(a0)
	IFEQ	\2	; INCIC
	lea	(ci_SIZEOF*(\1+1))(a0),a0
	IFEQ	\3	; CHECK
picl_chk\@:
	btst.b	#0,ci_OpCode+1(a0)
	beq.s	picl_move\@
	lea	ci_SIZEOF(a0),a0
	bra.s	picl_chk\@
picl_move\@:
	ENDC
	ENDC
	ENDM
**********************************

lc_change_interm:
	move.l	ah_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	lc_load
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	lc_load
	btst.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	bne	AnimateLineControl.
lc_load:
	moveq	#0,d5			; set bit 31 if AA
	move.l	sfxb_GfxBase(a6),a0
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a0)
	beq.s	1$
	moveq	#-1,d5
1$:
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
	bra	icl_loop.
icl_loop:
	btst.b	#0,ci_OpCode+1(a0)
	bne	icl_WAIT		; a WAIT
	tst.w	fxh_Pad(a4)		; did we come from RebuildLineControl()?
	bmi.s	no_modify_chk2
	btst.b	#LCB_MODIFY,lc_Flags+1(a4)
	beq	icl_skip
no_modify_chk2:
	; update these intermediate copper instructions.
	; we need to follow the same pattern as when we built the list.
	lea	lcp_AnimCache(a4),a1
	move.w	lcp_Type(a4),d5
	cmp.w	#PREV_LC_2,d5
	blt.s	icl_line2_chk
	subq.w	#PREV_LC_2,d5
	POKEICL	0,NOINC,NOCHECK		; CMOVE BPLMOD2
	POKEICL	1,INCIC,CHECK		; CMOVE BPLMOD1
icl_line2_chk:
	POKEICL	0,NOINC,NOCHECK		; CMOVE DDFSTRT
	POKEICL	1,INCIC,NOCHECK		; CMOVE BPLCON1
	tst.l	d5
	bpl.s	icl_mods_line2
	POKEICL	0,INCIC,NOCHECK		; CMOVE FMODE
icl_mods_line2:
	POKEICL	0,NOINC,NOCHECK		; CMOVE	BPLMOD2
	POKEICL	1,INCIC,CHECK		; CMOVE	BPLMOD1
icl_line3_chk:
	move.w	lc_Count(a4),d0
	cmp.w	#2,d0
	blt.s	icl_line4_chk
	POKEICL	0,NOINC,NOCHECK		; CMOVE BPLMOD2
	POKEICL	1,INCIC,CHECK		; CMOVE BPLMOD1
icl_line4_chk:
	tst.w	d5
	beq.s	icl_done1
	POKEICL	0,NOINC,NOCHECK		; CMOVE DDFSTRT
	POKEICL	1,INCIC,NOCHECK		; CMOVE BPLCON1
	tst.l	d5
	bpl.s	icl_mods_line4
	POKEICL	0,INCIC,NOCHECK		; CMOVE FMODE
icl_mods_line4:
	POKEICL	0,NOINC,NOCHECK		; CMOVE	BPLMOD2
	POKEICL	1,INCIC,CHECK		; CMOVE	BPLMOD1
	bra.s	icl_done1

icl_skip:
	move.w	lcp_CopCount(a4),d0	; number of copper instructions to
					; skip (*4)
	IFEQ	(ci_SIZEOF-6)		; if the ci_SIZEOF == 6 bytes
	move.w	d0,d1
	asr.w	#1,d1			; /2
	add.w	d1,d0			; *6
	ELSE
	FAIL
	ENDC
	add.w	d0,a0			; skip this many instructions
icl_done1:
	lea	lcp_SIZEOF(a4),a4
icl_loop.:
	dbra.s	d2,icl_loop
	bra.s	AnimateLineControl.

icl_WAIT:
	lea	ci_SIZEOF(a0),a0
	bra	icl_loop

AnimateLineControl.:
	rts
* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         RebuildLineControl
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

RebuildLineControl:
	tst.w	d5
	bmi.s	2$		; definitely rebuild
	move.l	ah_ViewPort(a4),a1	; only rebuild if the DxOffset
	move.w	vp_DxOffset(a1),d0	; has changed
	cmp.w	lch_DxOffset(a0),d0
	bne.s	2$
	rts

2$:
	; We need to recalculate the LineControlHandle lch_ values, the
	; lcOffsetTable.
	; We then need to rebuild the copperlist using AnimateLineControl,
	; although we need to make sure that all the LineControls are made
	; (not just those with LCB_MODIFY set).

	movem.l	d2-d7/a2-a6,-(sp)
	; Start by rebuilding the lco_ table.
	sub.l	#FRAME_SIZE,sp
	move.l	sp,a5
	move.l	sfxb_GfxBase(a2),a6
	move.l	a2,stk_SpecialFXBase(a5)
	move.l	ah_ViewPort(a4),stk_ViewPort(a5)
	move.l	ah_View(a4),stk_View(a5)
	move.l	a4,-(sp)
	move.l	a0,a4

	sub.l	#(vp_SIZEOF+ri_SIZEOF),sp
	move.l	sp,a2			; a2 -> dummy ViewPort
	lea	vp_SIZEOF(a2),a1	; a1 -> RasInfo
	move.l	stk_ViewPort(a5),a3
	moveq	#0,d0
	move.l	d0,vp_Next(a2)
	move.l	vp_ColorMap(a3),vp_ColorMap(a2)
	move.l	d0,vp_DspIns(a2)
	move.l	d0,vp_SprIns(a2)
	move.l	d0,vp_ClrIns(a2)
	move.l	d0,vp_UCopIns(a2)
	move.l	vp_DWidth(a3),vp_DWidth(a2)	; DWidth and DHeight
	move.l	vp_DxOffset(a3),vp_DxOffset(a2)	; DxOffset and DyOffset
	move.w	vp_DxOffset(a3),lch_DxOffset(a4)	; store the orignal DxOffset
	move.l	vp_Modes(a3),vp_Modes(a2)	; Modes, SpritePri and ExtendedModes
	bclr.b	#5,vp_Modes(a2)			; reset VP_HIDE
	move.l	a1,vp_RasInfo(a2)
	move.l	vp_RasInfo(a3),a3
	move.l	d0,ri_Next(a1)			; No DPF for now.
	move.l	ri_BitMap(a3),ri_BitMap(a1)
	move.l	d0,ri_RxOffset(a1)		; RxOffset and RyOffset
	move.l	a1,a3

	; a2 -> dummy ViewPort
	; a3 -> dummy RasInfo

	; Make the ViewPort

	FORBID	a1
	move.l	stk_View(a5),a0
	move.l	a2,a1
	move.l	v_ViewPort(a0),-(sp)
	move.l	a1,v_ViewPort(a0)	; fake this ViewPort into the View
	jsr	_LVOMakeVPort(a6)
	move.l	stk_View(a5),a0
	move.l	(sp)+,v_ViewPort(a0)
	PERMIT	save

	move.l	a4,-(sp)
	move.w	lch_OffsetCount(a4),d7
	subq.w	#1,d7			; for dbra
	lea	lch_SIZEOF(a4),a4	; point to the first lcOffsetTable
	move.l	vp_ColorMap(a2),d0
	beq.s	1$
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	1$
	move.b	cm_AuxFlags(a0),d2
	bclr.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)	; not exactly friendly, as the ColorMap is not a copy
1$:
	move.l	d2,-(sp)
	moveq	#0,d6			; d6 will store the BPLxMODS of the first position.
	moveq	#0,d5			; to show first time through.
	bra.s	search_ci_r

svp_loop_r:
	addq.w	#1,ri_RxOffset(a3)
	move.l	a2,a0
	jsr	_LVOScrollVPort(a6)

	; look for  FMode, Modulos, BplCon1, and DDFStrt
	; to store in the lcOffsetTable holdings.
search_ci_r:
	move.l	vp_DspIns(a2),d0
*	beq	FAILURE_3
	move.l	d0,a0
	move.l	cl_CopIns(a0),d0
*	beq	FAILURE_3
	move.l	d0,a0
lc_curr_loop_r:
	lea	ci_SIZEOF(a0),a0	; skip the first WAIT
	btst.b	#0,ci_OpCode+1(a0)	; last WAIT?
	bne	searched_ci_r.
	move.w	ci_DestAddr(a0),d0
	move.w	ci_DestData(a0),d1
	and.w	#$fff,d0
	cmp.w	#(BPLCON0&$fff),d0
	beq.s	lc_curr_con0_r
	cmp.w	#(BPLCON1&$fff),d0
	beq.s	lc_curr_con1_r
	cmp.w	#(DDFSTRT&$fff),d0
	beq.s	lc_curr_ddfstrt_r
	cmp.w	#(DDFSTOP&$fff),d0
	beq.s	lc_curr_ddfstop_r
	cmp.w	#(FMODE&$fff),d0
	beq.s	lc_curr_fmode_r
	cmp.w	#(BPL2PTH&$fff),d0
	beq	lc_curr_ptr2_r
	cmp.w	#(BPL1PTH&$fff),d0
	beq.s	lc_curr_ptr1_r
	cmp.w	#(BPLMOD1&$fff),d0
	beq.s	lc_curr_MOD1_r
	cmp.w	#(BPLMOD2&$fff),d0
	bne.s	lc_curr_loop_r
lc_curr_MOD2_r:
	tst.l	d5
	bne.s	1$
	move.w	d1,d6
	move.w	d1,stk_ZeroModEven(a5)
	moveq	#1,d5		; assume we always write BPLMOD1 then BPLMOD2 in the copper lists
1$:
	sub.w	d6,d1
	move.w	d1,lco_BPLMOD2(a4)
	bra.s	lc_curr_loop_r
lc_curr_MOD1_r:
	swap	d6
	tst.l	d5
	bne.s	1$
	move.w	d1,d6
	move.w	d1,stk_ZeroModOdd(a5)
1$:
	sub.w	d6,d1
	move.w	d1,lco_BPLMOD1(a4)
	swap	d6
	bra.s	lc_curr_loop_r
lc_curr_con0_r:
	move.w	d1,stk_BPLCON0(a5)
	bra	lc_curr_loop_r
lc_curr_con1_r:
	move.w	d1,lco_BPLCON1(a4)
	bra	lc_curr_loop_r
lc_curr_ddfstrt_r:
	move.w	d1,lco_DDFSTRT(a4)
	bra	lc_curr_loop_r
lc_curr_ddfstop_r:
	move.w	d1,lco_DDFSTOP(a4)
	tst.l	d5
	bne	lc_curr_loop_r
	move.w	d1,stk_ZeroDDFSTOP(a5)
	bra	lc_curr_loop_r
lc_curr_fmode_r:
	move.w	d1,lco_FMODE(a4)
	bra	lc_curr_loop_r
lc_curr_ptr1_r:
	move.w	d1,d2			; store the bplptr1 in d2
	swap	d2
	lea	ci_SIZEOF(a0),a0	; assume low part of the addess is the next instruction
	move.w	ci_DestData(a0),d2
	cmp.l	#1,d5			; have we just found the first BPLMOD2?
	bne	lc_curr_loop_r
	move.l	d2,stk_ZeroBPL1PTR(a5)
	moveq	#2,d5
	bra	lc_curr_loop_r
lc_curr_ptr2_r:
	move.w	d1,d4			; store the bplptr2 in d4
	swap	d4
	lea	ci_SIZEOF(a0),a0	; assume low part of the addess is the next instruction
	move.w	ci_DestData(a0),d4
	cmp.l	#2,d5
	bne	lc_curr_loop_r
	move.l	d4,stk_ZeroBPL2PTR(a5)
	move.l	#3,d5
	bra	lc_curr_loop_r

searched_ci_r.:
	move.w	lco_FMODE(a4),d1
	moveq	#0,d0
	move.l	a6,-(sp)
	move.l	gb_bwshifts(a6),a6
	and.w	#3,d1
	move.b	0(a6,d1.w),d0	; fmode shift value
	; there may be an extra offset to consider if this position is a different
	; fmode than the original, in which case the ddfstop may be different. We
	; therefore have to add the extra line length as
	; (((lco_DDFSTOP - original DDFSTOP) / cycles) * words per cycle)
	move.w	lco_DDFSTOP(a4),d3
	sub.w	stk_ZeroDDFSTOP(a5),d3
*	beq.s	lc_curr_fmode_2
	tst.w	d1
	beq.s	lc_curr_fmode_1x_r
	cmp.w	#BANDWIDTH_4X,d1
	beq.s	lc_curr_fmode_4x_r
lc_curr_fmode_2x_r:
	moveq	#2,d1		; assume 8 cycle. Can only be 4 cycle if SHires
	btst.b	#6,stk_BPLCON0+1(a5)
	beq.s	1$
	moveq	#1,d1		; is indeed a 4 cycle
1$:
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
*	bra.s	lc_curr_fmode_1
	bra.s	compensate_r
lc_curr_fmode_1x_r:
	moveq	#2,d1		; assume 8 cycle. Can be 4 cycle if Hires, 2 cycle is SHires
	tst.w	stk_BPLCON0(a5)
	bmi.s	2$		; Hires
	btst.b	#6,stk_BPLCON0+1(a5)
	beq.s	1$
	moveq	#0,d1		; a 2 cycle for SHires
	bra.s	1$
2$:
	moveq	#1,d1		; a 4 cycle for Hires
1$:
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
compensate_r:
	sub.l	stk_ZeroBPL1PTR(a5),d2
	sub.w	d3,d2
	move.w	d2,lco_CompensateEven(a4)
	sub.l	stk_ZeroBPL2PTR(a5),d4
	sub.w	d3,d4
	move.w	d4,lco_CompensateOdd(a4)
	bra.s	lc_curr_fmode_1_r
lc_curr_fmode_4x_r:
	moveq	#2,d1		; must be an 8 cycle
	asr.w	d1,d3		; / cycles
	add.w	d3,d3		; * words per cycle (= * 2)
lc_curr_fmode_1_r:
	add.w	d3,lco_BPLMOD1(a4)
	add.w	d3,lco_BPLMOD2(a4)

lc_curr_fmode_2_r:
	addq.w	#1,d0		; 0, 1 or 2 becomes 1, 2 or 3, which is how much 
				; we shift when dealing with whole jumps.
	move.w	d0,lco_ModDiff(a4)
	move.l	(sp)+,a6
	lea	lco_SIZEOF(a4),a4
	dbra.s	d7,svp_loop_r

	move.l	(sp)+,d2	; cm_AuxFlags
	move.l	(sp)+,a4	; FxHandle
	move.l	vp_ColorMap(a2),d0
	beq.s	1$
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	1$
	move.b	d2,cm_AuxFlags(a0)
1$:
	move.l	a2,a0		; free up the memory taken by the MakeVPort().
	jsr	_LVOFreeVPortCopLists(a6)
	add.l	#(vp_SIZEOF+ri_SIZEOF),sp

	move.l	stk_ZeroModOdd(a5),lch_ZeroModOdd(a4)	; store these for AnimateFX()

	;
	; Now rebuild the copperlists. Use the high bit of fxh_Pad to show
	; AnimateLineControl that we want the copperlist rebuilt, and to ignore
	; the LCB_MODIFY flag.
	;
	; Call AnimateLineControl with
	; a3 -> FXHandle
	; a5 -> AnimHandle
	; a6 -> SFXBase

	bset.b	#7,fxh_Pad(a4)
	move.l	a4,a3
	move.l	stk_SpecialFXBase(a5),a6
	move.l	(sp)+,a5	; retrieve the AnimHandle
	add.l	#FRAME_SIZE,sp	; don't need this anymore, and AnimateLineControl
				; allocates another one, so reduce stack usage.
	move.l	a4,-(sp)
	jsr	AnimateLineControl(pc)
	move.l	(sp)+,a4
	bclr.b	#7,fxh_Pad(a4)

RebuildLineControl.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
* }
********************************************************************************
* {
;
; GetOffsetTable() - calculates the offsets from the FxHandle of the 
; Offset table for the odd:even bitplanes.
;
; Takes:
;	a3 -> LineControl structure
;	a4 -> FxHandle
;	a5 -> StkStuff
;
; Returns:
;	d4 = Odd:Even offsets

GetOffsetTable:
	move.l	lc_RasInfo(a3),d0
	beq.s	GOT_no_ri		; what the hell is this?
	move.l	d0,a0
	move.w	ri_RxOffset(a0),d0
	move.w	d0,d4
	IFEQ	(lco_SIZEOF-20)		; if the lco_SIZEOF == 20 bytes
	bpl.s	1$
	add.w	lch_OffsetCount(a4),d0
1$:
	and.w	lch_Mask(a4),d0
	move.w	d0,d1
	asl.w	#2,d1			; * 4
	asl.w	#4,d0			; * 16
	add.w	d1,d0			; * 20
	ELSE
	FAIL				; else fail assembly
	ENDC
	add.w	#lch_SIZEOF,d0		; the offset value from FxHandle for this lco_
	move.w	lco_ModDiff(a4,d0.w),d1	; calculate the Whole number from the ModDiff
	addq.w	#3,d1			; of this lco_
	asr.w	d1,d4
	neg.w	d4
	move.w	d4,stk_WholeEven(a5)
	move.w	d0,d4
	swap	d4
	move.l	ri_Next(a0),d1
	beq.s	GOT_no_dpf

	move.l	d1,a0
	move.w	ri_RxOffset(a0),d0
	move.w	d0,d4
	bpl.s	2$
	add.w	lch_OffsetCount(a4),d0
2$:
	and.w	lch_Mask(a4),d0
	move.w	d0,d1
	asl.w	#2,d1			; * 4
	asl.w	#4,d0			; * 16
	add.w	d1,d0			; * 20
	add.w	#lch_SIZEOF,d0
	move.w	lco_ModDiff(a4,d0.w),d1	; calculate the Whole number from the ModDiff
	addq.w	#3,d1			; of this lco_
	asr.w	d1,d4
	neg.w	d4
	move.w	d4,stk_WholeOdd(a5)
	move.w	d0,d4
	bra.s	GOT.
GOT_no_dpf:
	move.w	d0,d4
	move.w	stk_WholeEven(a5),stk_WholeOdd(a5)
GOT.:
	swap	d4
	DEBUGGOT
	rts

GOT_no_ri:	; boy, is this screwed up!
	addq.l	#4,sp		; remove the return address
	bra	anim_lc_loop.

* }
********************************************************************************