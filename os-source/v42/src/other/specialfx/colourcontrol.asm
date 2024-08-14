********************************************************************************
*
*	$Id: ColourControl.asm,v 39.8 93/09/23 17:27:53 spence Exp $
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

	xdef	AllocColours
	xdef	CountColours
	xdef	InstallColours
	xdef	AnimateColours
	xdef	RebuildColours
	xref	DoUCopStuff

	xref	_LVOGetRGB32
	xref	_LVOGetRGB4
	xref	_LVOMakeVPort
	xref	_LVOCWait
	xref	_LVOCMove
	xref	_LVOCBump
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

AllocColours:
	; Allocate the RAM for the ColorControl structures.
	moveq	#CCPrivate_SIZEOF,d0
	mulu	d1,d0			; enough for all the cc_ structures.
	add.l	#FxHandle_SIZEOF,d0	; plus the FXHandle
	move.l	d1,d2
	asl.w	#2,d1
	add.w	d1,d0			; plus the pointers to the cc_structures
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
	move.l	#(SFX_ColorControl&~$80000000),fxh_Type(a1)
	move.l	d2,fxh_Pad(a1)		; fxh_Pad and fxh_Num
	move.l	d3,fxh_AllocSize(a1)
	move.w	d2,d3
	asl.w	#2,d3
	add.l	d3,a0
	move.l	a0,fxh_First(a1)
	move.l	d0,fxh_BackPointer(a1)
	lea	FxHandle_SIZEOF(a1),a1
	move.l	a1,d0			; return the start of the array list
	bra.s	Alloc_CC_loop.
Alloc_CC_loop:
	move.l	a0,(a1)+
	lea	CCPrivate_SIZEOF(a0),a0
Alloc_CC_loop.:
	dbra.s	d2,Alloc_CC_loop
	rts

FAIL_NOMEM:
	move.l	#SFX_ERR_NoMem,(a2)
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; Count the number of copper instructions needed for the ColorRange
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> StackStuff
	; a6 -> UtilityBase

CountColours:
	movem.l	d2-d7/a2-a6,-(sp)
	moveq	#0,d0			; d0 will count the WAITs
	move.l	d0,d1
	move.w	fxh_Num(a0),d7
	beq.s	CountColours.		; early sanity check
	move.w	d7,d1			; d1 will count the colours
	move.l	fxh_First(a0),a1
	subq.w	#1,d7
	move.w	cc_Line(a1),d6
	addq.w	#1,d6
count_colours_loop:
	move.w	cc_Line(a1),d3
	cmp.w	d3,d6
	beq.s	chk_restore
	addq.w	#1,d0			; one more WAIT
	move.w	d3,d6
chk_restore:
	btst.b	#CCB_RESTORE,cc_Flags+1(a1)
	beq.s	count_colours_loop.
	addq.w	#1,d1
	addq.w	#1,d0			; possibly one more WAIT
count_colours_loop.:
	lea	CCPrivate_SIZEOF(a1),a1
	dbra.s	d7,count_colours_loop
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a4)
	beq.s	1$
	; for AA, need 2 colour instructions, and 2 bplcon3 instructions per line
	asl.l	#2,d1
1$:
	add.l	d0,d1			; add the WAITs
	addq.w	#2,d1			; terminator

count_get_ucop:
	bsr	DoUCopStuff
CountColours.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

* }
********************************************************************************
* {	
	; **********************************************************************
	;
	; This code will build the UserCopperList to display the colours of
	; this set.
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> Stack frame
	; a6 -> UtilityBase

InstallColours:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a4,a6
	move.l	a0,a4
	move.l	stk_DispHandle(a5),a3
	move.l	a4,(a3)+		; store the FxHandle in the AnimHandle
	move.l	a3,stk_DispHandle(a5)
	move.l	fxh_UCopList(a4),a2	; use a2 -> UCopList
	move.w	fxh_Num(a4),d4
	subq.w	#1,d4
	bcs	DisplayColours.
	move.l	fxh_First(a4),a4
	moveq	#0,d7
	move.w	cc_Line(a4),d7	; d7 will store the current WAIT value
	addq.w	#1,d7			; (prime for 1st time through)
loop_array1:
	swap	d4
	moveq	#0,d5			; d5 = -1 to restore
loop_array:
	btst.b	#CCB_RESTORE,cc_Flags+1(a4)
	beq.s	1$
	move.l	stk_ViewPort(a5),a0
	move.l	vp_ColorMap(a0),a0
	move.l	cc_Pen(a4),d0
	; For V39 or greater, use GetRGB32(). For pre V39, use GetRGB4().
	cmp.w	#39,LIB_VERSION(a6)
	bge.s	39$
	jsr	_LVOGetRGB4(a6)
	; first convert the Blue in bits 0-3
	move.b	d0,d1
	and.b	#$f,d1
	move.b	d1,d5
	asl.b	#4,d1
	or.b	d5,d1
	move.b	d1,Blue32_orig(a4)
	move.b	d1,Blue32_orig+1(a4)
	move.b	d1,Blue32_orig+2(a4)
	move.b	d1,Blue32_orig+3(a4)
	; now Green...
	move.b	d0,d1
	and.b	#$f0,d1
	move.b	d1,d5
	lsr.b	#4,d5
	or.b	d5,d1
	move.b	d1,Green32_orig(a4)
	move.b	d1,Green32_orig+1(a4)
	move.b	d1,Green32_orig+2(a4)
	move.b	d1,Green32_orig+3(a4)
	; and Red.
	move.w	d0,d1
	lsr.w	#4,d1
	and.b	#$f0,d1
	move.b	d1,d5
	lsr.b	#4,d5
	or.b	d5,d1
	move.b	d1,Red32_orig(a4)
	move.b	d1,Red32_orig+1(a4)
	move.b	d1,Red32_orig+2(a4)
	move.b	d1,Red32_orig+3(a4)
	bra.s	2$
39$:
	moveq	#1,d1
	lea	Red32_orig(a4),a1
	jsr	_LVOGetRGB32(a6)
2$:
	moveq	#-1,d5
	clr.w	d5			; d5 = 0xffff0000 to show repeat this entry,
					; d5 = 0x00008000 to show use the cached values

1$:
	; build the private copper list cache
	move.l	#COLOUR0,a3
	move.w	#(BPLCON3&$1fe),d6
	swap	d6
	move.w	cc_Pen+2(a4),d6
	move.w	d6,d4
	and.w	#31,d6
	add.w	d6,d6
	add.w	d6,a3			; a3 = colour register address
	move.w	d4,d6			; calculate the colour bank number
	and.w	#~31,d6
	asl.w	#8,d6
	or.w	stk_BPLCON3(a5),d6	; store together
	move.l	d6,con3_hi(a4)
	or.w	#$0200,d6
	move.l	d6,con3_lo(a4)
	and.l	#$ffff,d6

build_colours:
	move.w	cc_Line(a4),d0
	cmp.w	d0,d7
	beq.s	same_line
	move.w	d0,d7			; the line number
restore_colour:
	bset.l	#31,d7
same_line:
	moveq	#-2,d0
	swap	d0
	move.w	d7,d0			; build the WAIT
	asl.w	#8,d0
	or.w	#1,d0
	swap	d0
	move.l	d0,Wait(a4)
100$:
	; build the colour registers
	move.b	cc_Blue(a4),d0
	move.b	d0,d2
	swap	d2
	move.b	d0,d2
	move.w	cc_Red(a4),d0
	move.b	cc_Green(a4),d1
	tst.w	d5
	bpl.s	10$
	move.b	Blue32_orig(a4),d0
	move.b	d0,d2
	swap	d2
	move.b	d0,d2
	move.w	Red32_orig(a4),d0
	move.b	Green32_orig(a4),d1
10$:
	moveq	#0,d3
	move.w	d0,d3
	and.w	#$0f00,d3
	move.b	d1,d3
	asl.b	#4,d3
	and.b	#$f,d2
	or.b	d2,d3
	swap	d3
	move.w	d0,d3
	lsr.w	#4,d3
	move.b	d1,d3
	and.b	#$f0,d3
	swap	d2
	lsr.b	#4,d2
	or.b	d2,d3
	move.w	d3,d0
	swap	d0
	move.w	a3,d0
	and.w	#$01fe,d0
	swap	d0
	move.w	d0,d2
	tst.w	d5
	bmi.s	101$
	move.l	d0,pen_hi(a4)
101$:
	move.l	d3,d0
	move.w	a3,d0
	and.w	#$01fe,d0
	swap	d0
	move.w	d0,d1
	tst.w	d5
	bmi.s	102$
	move.l	d0,pen_lo(a4)
102$:
	; now build the copper list
	tst.l	d7			; do we need the WAIT?
	bpl.s	3$
	move.l	a2,a1
	moveq	#0,d0
	move.w	d7,d0
	move.l	d1,-(sp)
	moveq	#0,d1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.l	(sp)+,d1
	bclr.l	#31,d7
3$:
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	4$
	and.w	#~$0200,d6		; Hi bits
	CMOVE	#BPLCON3,d6
4$:

	CMOVE	a3,d2			; Hi Colours
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	2$
	or.w	#$0200,d6		; Lo bits
	CMOVE	#BPLCON3,d6
	CMOVE	a3,d1			; Lo colours
2$:

	; done that one. Do we need to restore the pen?
	tst.l	d5
	bpl.s	next_colour
	move.l	#$8000,d5
	addq.w	#1,d7
	bra	restore_colour
next_colour:
	lea	CCPrivate_SIZEOF(a4),a4
	swap	d4
	dbra	d4,loop_array1

end_colours:
	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)		; CEND
	move.l	a2,a1
	jsr	_LVOCBump(a6)

DisplayColours.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         AnimateColours
	;
	; Change the system copperlist, and optionally the intermediate
	; copperlist.
	; 
	; Takes - a3 -> FXHandle
	;	  a5 -> AnimHandle
	;	  a6 -> SFXBase
	;
	; **********************************************************************

AnimateColours:
	movem.l	d7/a2/a6,-(sp)
	move.l	sfxb_GfxBase(a6),a6
	move.l	fxh_First(a3),a4
	move.l	ah_ViewPort(a5),a2
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne.s	no_hw_colour_update	; ViewPort is hidden, so don't update the h/w copperlist

	movem.l	a2/a4,-(sp)
	move.w	ah_CopperToUse(a5),d0
	asl.w	#3,d0
	move.l	ah_Copper1S(a5,d0.w),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a3),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the colour list.
	moveq	#-1,d2			; flag to disable updating the cache
	bsr.s	do_change_colours
	movem.l	(sp),a2/a4
1$:
	move.w	ah_CopperToUse(a5),d0
	asl.w	#3,d0
	move.l	ah_Copper1L(a5,d0.w),d0	; get the right copperlist to use.
	beq.s	2$
	add.l	fxh_Offset(a3),d0
	move.l	d0,a2
	moveq	#0,d2
	bsr.s	do_change_colours
2$:
	movem.l	(sp)+,a2/a4
no_hw_colour_update:
	movem.l	(sp)+,d7/a2/a6
	bra	colour_change_interm

do_change_colours:
	move.w	fxh_Num(a3),d2
	; skip over the WAIT (there may be more than 1 if this is line 256).
2$:
	btst.b	#0,1(a2)
	beq.s	1$
	addq.l	#4,a2
	bra.s	2$
1$:
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	change_ecs_colours
	; now we know we are changing a AA copperlist

	moveq	#-2,d4			; FFFF FFFE terminator
	moveq	#12,d6			; to add to the copper pointer for each colour change
	bra.s	no_colour_change2
change_colours:
	btst.b	#CCB_MODIFY,cc_Flags+1(a4)
	beq.s	no_colour_change1
	move.l	con3_hi(a4),d5
	move.w	pen_hi(a4),d7

2$:	; find a MOVE BPLCON3
	move.l	(a2)+,d3
	cmp.l	d3,d4
	beq.s	done_colour_anim		; end of copperlist
	cmp.l	d3,d5
	bne.s	2$
	; check it's the right one
	move.w	(a2),d3
	cmp.w	d3,d7
	bne.s	2$

3$:
	; calculate the Hi colour bits
	move.w	cc_Red(a4),d0
	lsr.w	#4,d0
	move.b	cc_Blue(a4),d1
	lsr.b	#4,d1
	move.b	cc_Green(a4),d0
	bfins	d1,d0{28:4}
	move.w	d0,2(a2)		; into the copperlist
	tst.l	d2
	bmi.s	4$			; don't update the cache for the SHF
	move.w	d0,pen_hi+2(a4)		; into the cache

4$:
	; calculate the Lo colour bits
	move.w	cc_Red(a4),d0
	and.w	#$f00,d0
	move.b	cc_Green(a4),d0
	asl.b	#4,d0
	move.b	cc_Blue(a4),d1
	bfins	d1,d0{28:4}
	move.w	d0,10(a2)
	tst.l	d2
	bmi.s	no_colour_change
	move.w	d0,pen_lo+2(a4)

no_colour_change:
	add.l	d6,a2
	lea	CCPrivate_SIZEOF(a4),a4
no_colour_change2:
	dbra.s	d2,change_colours
done_colour_anim:
	rts
no_colour_change1:
	addq.l	#4,a2
	bra.s	no_colour_change

change_ecs_colours:
	moveq	#-2,d4			; FFFF FFFE terminator
	bra.s	no_ecs_colour_change1
ecs_change_colours:
	cmp.l	#-1,cc_Pen(a4)
	beq.s	done_colour_anim
	move.w	pen_hi(a4),d7
2$:
	move.l	(a2)+,d3
	cmp.l	d3,d4
	beq.s	done_colour_anim		; end of copperlist
	swap	d3
	cmp.w	d3,d7
	bne.s	2$

	btst.b	#CCB_MODIFY,cc_Flags+1(a4)
	beq.s	no_ecs_colour_change
	; calculate the Hi colour bits
	move.w	cc_Red(a4),d0
	lsr.w	#4,d0
	move.b	cc_Green(a4),d0
	and.b	#$f0,d0
	move.b	cc_Blue(a4),d1
	lsr.b	#4,d1
	or.b	d1,d0
	move.w	d0,-2(a2)		; into the copperlist
	tst.l	d2
	bmi.s	no_ecs_colour_change
	move.w	d0,pen_hi+2(a4)		; into the cache
no_ecs_colour_change:
	lea	CCPrivate_SIZEOF(a4),a4
no_ecs_colour_change1:
	dbra.s	d2,ecs_change_colours
	bra.s	done_colour_anim

	; ******************************************************************
	;
	; This code will update the intermediate copper list in
	; vp->ClrIns
	;
	; ******************************************************************

colour_change_interm:
	; now update the intermediate copperlists
	move.l	ah_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	colour_load
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	colour_load
	btst.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	bne.s	AnimateColours.
colour_load:
	move.w	fxh_Num(a3),d2
	move.l	fxh_UCopList(a3),a0	; look for this UCopList
	move.l	ucl_FirstCopList(a0),a0
	move.l	cl_CopIns(a0),a0
	move.l	sfxb_GfxBase(a6),a1
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a1)
	bne.s	10$
	bra.s	update_ecs_copins

8$:
	btst.b	#0,ci_OpCode+1(a0)
	bne.s	5$			; a WAIT
	btst.b	#CCB_MODIFY,cc_Flags+1(a4)
	beq.s	9$
	lea	ci_SIZEOF(a0),a0	; skip the bplcon3
	move.l	pen_hi(a4),ci_DestAddr(a0)
	lea	ci_SIZEOF*2(a0),a0	; skip the PenHi and Con3
	move.l	pen_lo(a4),ci_DestAddr(a0)
6$:
	lea	ci_SIZEOF(a0),a0	; skip the PenLo
7$:
	lea	CCPrivate_SIZEOF(a4),a4
10$:
	dbra.s	d2,8$
	bra.s	AnimateColours.

5$:
	lea	ci_SIZEOF(a0),a0
	bra.s	8$
9$:
	lea	ci_SIZEOF*4(a0),a0
	bra.s	7$

do_ecs_copins:
	btst.b	#0,ci_OpCode+1(a0)
	bne.s	ecs_wait			; a WAIT
	btst.b	#CCB_MODIFY,cc_Flags+1(a4)
	beq.s	70$
	move.l	pen_hi(a4),ci_DestAddr(a0)
70$:
	lea	CCPrivate_SIZEOF(a4),a4
	lea	ci_SIZEOF(a0),a0
update_ecs_copins:
	dbra.s	d2,do_ecs_copins

AnimateColours.:
	rts

ecs_wait:
	lea	ci_SIZEOF(a0),a0
	bra.s	do_ecs_copins

* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         RebuildColours
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

RebuildColours:
	tst.w	d5
	beq	done_colour_rebuild	; no mode change, so no rebuild

	; All we need to change is the EXTBLNKEN bit in the BPLCON3 copper
	; instructions and caches.

	movem.l	d2-d7/a2/a3/a5,-(sp)
	move.l	sfxb_GfxBase(a2),a5
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a5)
	beq	RebuildColours.		; nothing to do on ECS or A machines.

	move.l	ah_ViewPort(a4),a1
	; if the ViewPort is hidden, we need to call MakeVPort(), as intuition
	; does not remake the ViewPort until it becomes visible.
	btst.b	#5,vp_Modes(a1)		; V_VP_HIDE
	beq.s	no_mvp
	movem.l	a0/a1/a6,-(sp)
	move.l	ah_View(a4),a0
	move.l	a5,a6
	jsr	_LVOMakeVPort(a6)
	movem.l	(sp)+,a0/a1/a6
no_mvp:
	move.l	vp_DspIns(a1),d0
	beq	RebuildColours.
	move.l	d0,a1
	move.w	cl_Count(a1),d2
	move.l	cl_CopIns(a1),d0
	beq	RebuildColours.
	move.l	d0,a1
	; look for a BPLCON3
	move.l	#(BPLCON3&$fff),d3
	lea	-ci_SIZEOF(a1),a1
	bra.s	find_con3.
find_con3:
	move.b	ci_OpCode+1(a1),d0
	bne.s	find_con3.		; not a MOVE
	move.l	ci_DestAddr(a1),d0	; and ci_DestData
	swap	d0
	and.w	#$fff,d0
	cmp.w	d0,d3
	beq.s	found_con3
find_con3.:
	lea	ci_SIZEOF(a1),a1
	dbra.s	d2,find_con3
	moveq	#0,d0			; no con3??

found_con3:
	swap	d0
	move.w	d0,d3
	and.w	#1,d3			; keep the EXTBLNKEN bit
	
	; change the copperlists and update the cache too.
	move.l	ah_ViewPort(a4),a2
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne.s	no_hw_colour_rebuild	; ViewPort is hidden, so don't update the h/w copperlist
	move.l	fxh_First(a0),a3

	movem.l	a2/a3,-(sp)
	move.l	ah_Copper1S(a4),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a0),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the colour list.
	moveq	#-1,d2			; flag to disable updating the cache
	bsr.s	do_rebuild_colours
	movem.l	(sp),a2/a3
1$:
	tst.l	ah_Copper2SSize(a4)
	beq.s	10$
	move.l	ah_Copper2S(a4),d0	; get the right copperlist to use.
	beq.s	10$			; no copper list?
	add.l	fxh_Offset(a0),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the colour list.
	moveq	#-1,d2			; flag to disable updating the cache
	bsr.s	do_rebuild_colours
	movem.l	(sp),a2/a3
10$:
	tst.l	ah_Copper2LSize(a4)
	beq.s	2$
	move.l	ah_Copper2L(a4),d0	; get the right copperlist to use.
	beq.s	2$
	add.l	fxh_Offset(a0),d0
	move.l	d0,a2
	moveq	#-1,d2
	bsr.s	do_rebuild_colours
	movem.l	(sp),a2/a3
2$:
	move.l	ah_Copper1L(a4),d0	; get the right copperlist to use.
	beq.s	20$
	add.l	fxh_Offset(a0),d0
	move.l	d0,a2
	moveq	#0,d2
	bsr.s	do_rebuild_colours
20$:
	movem.l	(sp)+,a2/a3
no_hw_colour_rebuild:
	bra.s	colour_rebuild_interm

do_rebuild_colours:
	move.w	fxh_Num(a0),d2
	; skip over the WAIT (there may be more than 1 if this is line 256).
2$:
	btst.b	#0,1(a2)
	beq.s	1$
	addq.l	#4,a2
	bra.s	2$
1$:
	moveq	#-2,d4			; FFFF FFFE terminator
	moveq	#12,d6			; to add to the copper pointer for each colour change
	bra.s	no_colour_rebuild2
rebuild_colours:
	move.l	con3_hi(a3),d5
	move.w	pen_hi(a3),d7

2$:	; find a MOVE BPLCON3
	move.l	(a2)+,d0
	cmp.l	d0,d4
	beq.s	done_colour_rebuild	; end of copperlist
	cmp.l	d0,d5
	bne.s	2$
	; check it's the right one
	move.w	(a2),d1
	cmp.w	d1,d7
	bne.s	2$

	and.w	#$fffe,d0		; mask out the EXTBLNKEN
	or.w	d3,d0			; and OR in the new value
	move.w	d0,-2(a2)		; into the copperlist
	tst.l	d2
	bmi.s	1$
	move.w	d0,con3_hi+2(a3)	; and in to the cache
1$:
	move.l	con3_lo(a3),d0
	and.w	#$fffe,d0
	or.w	d3,d0
	move.w	d0,6(a2)
	tst.l	d2
	bmi.s	3$
	move.w	d0,con3_lo+2(a3)
3$:
	add.l	d6,a2
	lea	CCPrivate_SIZEOF(a3),a3
no_colour_rebuild2:
	dbra.s	d2,rebuild_colours
done_colour_rebuild:
	rts

colour_rebuild_interm:
	; now update the intermediate copperlists
	move.l	fxh_UCopList(a0),a0	; look for this UCopList
	move.l	ucl_FirstCopList(a0),a0
	move.w	cl_Count(a0),d2
	move.l	cl_CopIns(a0),a0
	move.l	#(BPLCON3&$fff),d4
	bra.s	10$

8$:
	btst.b	#0,ci_OpCode+1(a0)
	bne.s	10$			; a WAIT
	move.w	ci_DestAddr(a0),d0	; is is a BPLCON3?
	and.w	#$fff,d0
	cmp.w	d0,d4
	bne.s	10$
	move.w	ci_DestData(a0),d0
	and.w	#$fffe,d0
	or.w	d3,d0
	move.w	d0,ci_DestData(a0)
10$:
	lea	ci_SIZEOF(a0),a0
	dbra.s	d2,8$

RebuildColours.:
	movem.l	(sp)+,d2-d7/a2/a3/a5
	rts

* }
