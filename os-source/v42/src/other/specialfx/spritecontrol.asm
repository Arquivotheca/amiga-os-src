********************************************************************************
*
*	$Id: SpriteControl.asm,v 39.2 93/09/13 19:09:56 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'graphics/view.i'
	include	'graphics/copper.i'
	include	'graphics/gfxbase.i'
	include	'graphics/sprite.i'
	include	'hardware/custom.i'

	include	"SpecialFXBase.i"
	include	"SpecialFX.i"
	include	"SpecialFX_internal.i"

	xdef	AllocSpriteControl
	xdef	CountSpriteControl
	xdef	InstallSpriteControl
	xdef	AnimateSpriteControl
	xref	DoUCopStuff
	xref	new_mode

	xref	_LVOCWait
	xref	_LVOCMove
	xref	_LVOCBump
	xref	_LVOMoveSprite
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

AllocSpriteControl:
	; Allocate the RAM for the SpriteControl structures.
	moveq	#scp_SIZEOF,d0
	mulu	d1,d0			; enough for all the fvc_ structures.
	add.l	#sch_SIZEOF,d0		; plus the special FXHandle
	move.l	d1,d2
	asl.w	#2,d1
	add.w	d1,d0			; plus the pointers to the spc_structures
					; = size of the allocation
	move.l	d0,d3
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	move.l	sfxb_ExecBase(a5),a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq.s	FAIL_NOMEM
	move.l	d0,a0			; a0 -> start of the array list
	move.l	d0,a1
	lea	sch_SIZEOF(a0),a0
	move.l	#(SFX_SpriteControl&~$80000000),fxh_Type(a1)
	move.l	d2,fxh_Pad(a1)		; fxh_Pad and fxh_Num
	move.l	d3,fxh_AllocSize(a1)
	move.w	d2,d3
	asl.w	#2,d3
	add.l	d3,a0
	move.l	a0,fxh_First(a1)
	lea	sch_BackPointer(a1),a1
	move.l	d0,(a1)+		; fxh_BackPointer
	move.l	a1,d0			; return the start of the array list
	bra.s	Alloc_SPC_loop.
Alloc_SPC_loop:
	move.l	a0,(a1)+
	lea	scp_SIZEOF(a0),a0
Alloc_SPC_loop.:
	dbra.s	d2,Alloc_SPC_loop
	rts

FAIL_NOMEM:
	move.l	#SFX_ERR_NoMem,(a2)
	rts

* }
********************************************************************************
* {
	; ***********************************************************************
	;
	; Count the number of copper instructions needed for the SpriteControl
	;
	; ***********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> StackStuff
	; a6 -> UtilityBase

	; For each SpriteControl line range, we need the following copper
	; instructions.
	; WAIT (spc_Line, 0)
	; MOVE SPRxPOS,xxx
	; MOVE SPRxCTL,xxx
	; MOVE SPRnPTH,xxx
	; MOVE SPRnPTL,xxx

SPC_COPINS	EQU	5

CountSpriteControl:
	move.w	fxh_Num(a0),d0
	beq.s	CountSpriteControl.	; early sanity check
	IFEQ	SPC_COPINS-5
	move.w	d0,d1
	asl.w	#2,d1
	add.w	d0,d1
	ELSE
	FAIL
	ENDC
	addq.w	#2,d1			; terminator
	jsr	DoUCopStuff(pc)
CountSpriteControl.:
	rts

* }
********************************************************************************
* {
	; **********************************************************************
	;
	; This code will build the UserCopperList for the SpriteControl
	;
	; **********************************************************************

	; enters with:
	; a0 -> FxHandle
	; a4 -> GfxBase
	; a5 -> Stack frame
	; a6 -> UtilityBase

InstallSpriteControl:
	movem.l	d2-d7/a2-a6,-(sp)

	move.l	stk_DispHandle(a5),a3
	move.l	a0,(a3)+		; store the FxHandle in the AnimHandle
	move.l	a3,stk_DispHandle(a5)

	move.w	fxh_Num(a0),d5
	beq	InstallSpriteControl.	; Duh - pretty stoopid
	subq.w	#1,d5
	move.l	a4,a6
	move.l	a0,a4
	move.l	fxh_UCopList(a4),a2
	move.l	fxh_First(a4),a3
	move.l	#SPR0PTH,d2

	; calculate how far we need to shift the ss_x value for 35ns resolution
	move.l	stk_ViewPort(a5),a0
	jsr	new_mode(pc)
	moveq	#1,d1
	tst.w	d0			; HIRES?
	bmi.s	1$
	moveq	#0,d1
	btst.b	#5,d0			; SHIRES?
	bne.s	1$
	moveq	#2,d1			; LORES
1$:
	move.w	d1,sch_To35ns(a4)
	
next_sc:
	move.w	spc_Line(a3),d0
	moveq	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	; Let MoveSprite() calculate the POS and CTL values.
	; However, doing so will cause the sprite pointer in copinit
	; to be updated as well, which we don't want to do as that could
	; leave a double image if AnimateFX() later changes the sprite
	; number. So, store the current value of the pointers in the copinit
	; list, and poke them back after the MoveSprite() call.

	move.l	spc_ExtSprite(a3),a1
	moveq	#0,d0
	move.w	ss_num(a1),d0
	asl.w	#3,d0
	move.l	gb_copinit(a6),a0
	lea	copinit_sprstrtup(a0,d0.w),a0
	move.l	a0,-(sp)
	move.l	(a0)+,d3
	move.l	(a0)+,d4

	move.l	stk_ViewPort(a5),a0
	move.l	d0,d1
	move.w	ss_x(a1),d0
	move.w	ss_y(a1),d1
	move.w	d0,scp_Orig_x(a3)
	move.w	d1,scp_Orig_y(a3)
	jsr	_LVOMoveSprite(a6)
	move.l	(sp)+,a0
	move.l	d3,(a0)+
	move.l	d4,(a0)+

	move.l	spc_ExtSprite(a3),a0
	move.l	ss_posctldata(a0),a1
	move.l	a1,d4
	move.w	(a1),scp_OrigPos(a3)
	move.w	gb_SpriteFMode(a6),d0		; How do you lock this?
	beq.s	is_1x
	cmp.w	#$0c,d0
	beq.s	is_4x
is_2x:
	moveq	#8,d6
	move.w	4(a1),scp_OrigCtl(a3)
	bra.s	spc_MOVE
is_1x:
	moveq	#4,d6
	move.w	2(a1),scp_OrigCtl(a3)
	bra.s	spc_MOVE
is_4x:
	moveq	#16,d6
	move.w	8(a1),scp_OrigCtl(a3)
spc_MOVE:
	; calculate the original x position in 35ns units
	move.w	d6,sch_PtrOffset(a4)
	move.w	scp_OrigPos(a3),d0
	asl.w	#3,d0
	move.w	scp_OrigCtl(a3),d1
	btst.b	#0,d1		; SH2
	beq.s	1$
	or.b	#4,d0
1$:
	lsr.w	#3,d1
	and.w	#3,d1
	or.w	d1,d0
	and.w	#$3ff,d0
	move.w	d0,scp_OrigX35ns(a3)
	; and the original y line value
	moveq	#0,d0
	move.b	scp_OrigPos(a3),d0	; SV7 - 0
	move.w	scp_OrigCtl(a3),d1
	btst.b	#2,d1			; SV8
	beq.s	2$
	or.w	#$100,d0
2$:
	btst.b	#6,d1
	beq.s	3$
	or.w	#$200,d0
3$:
	move.w	d0,scp_OrigYLine(a3)
	
	moveq	#0,d0
	move.w	ss_num(a0),d0
	asl.w	#2,d0
	move.l	d0,d3
	add.l	d2,d0
	add.w	#SPRPOSOFF,d0
	add.w	d3,d0
	move.w	scp_OrigPos(a3),d1
	CMOVE	d0,d1			; SPRxPOS
	and.w	#$fff,d0
	move.w	d0,scp_CopIns(a3)
	addq.w	#2,d0
	move.w	d1,scp_CopData(a3)
	move.w	scp_OrigCtl(a3),d1
	CMOVE	d0,d1			; SPRxCTL
	move.l	d2,d0
	add.w	d3,d0
	add.l	d6,d4			; skip the POS/CTL data in the address
	swap	d4
	CMOVE	d0,d4			; SPRxPTH
	addq.w	#2,d0
	swap	d4
	CMOVE	d0,d4			; SPRxPTL
	lea	scp_SIZEOF(a3),a3
	dbra.s	d5,next_sc

	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)		; CEND
	move.l	a2,a1
	jsr	_LVOCBump(a6)

InstallSpriteControl.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
* }
********************************************************************************
* {
	; **********************************************************************
	;
	;                         AnimateSpriteControl
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

AnimateSpriteControl:
	movem.l	d7/a2,-(sp)
	move.l	fxh_First(a3),a4
	move.l	ah_ViewPort(a5),a2
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne.s	no_hw_spc_update		; ViewPort is hidden, so don't update the h/w copperlist
	move.l	#SPR0PTH,d1
	moveq	#0,d0

	movem.l	a2/a4,-(sp)
	move.w	ah_CopperToUse(a5),d0
	asl.w	#3,d0
	move.l	ah_Copper1S(a5,d0.w),d0	; get the right copperlist to use.
	beq.s	1$			; no copper list?
	add.l	fxh_Offset(a3),d0	; jump to the first instruction
	move.l	d0,a2			; here is the first WAIT in the interrupt list.
	moveq	#-1,d2			; flag to disable updating the cache
	bsr.s	do_change_sc
	movem.l	(sp),a2/a4
1$:
	move.w	ah_CopperToUse(a5),d0
	asl.w	#3,d0
	move.l	ah_Copper1L(a5,d0.w),d0	; get the right copperlist to use.
	beq.s	2$
	add.l	fxh_Offset(a3),d0
	move.l	d0,a2
	moveq	#0,d2
	bsr.s	do_change_sc
2$:
	movem.l	(sp)+,a2/a4
no_hw_spc_update:
	movem.l	(sp)+,d7/a2
	bra	spc_change_interm

do_change_sc:
	moveq	#-2,d4			; FFFF FFFE terminator
	move.w	fxh_Num(a3),d2
	move.w	sch_To35ns(a3),d7
	bra	spc_change_loop
	; skip over the WAIT (there may be more than 1 if this is line 256).
look_for_move:
	move.w	(a2),d0
	btst.b	#0,d0			; is it a MOVE?
	beq.s	change_sc
	cmp.l	(a2),d4			; end of the copperlist?
	beq	change_sc.
not_ours:
	addq.l	#4,a2			; it was a WAIT, so skip it.
	bra.s	look_for_move
change_sc:
	; is this a sprite instruction?
	cmp.w	#(SPR0POS&$fff),d0
	blt.s	not_ours
	cmp.w	#(COLOUR0&$fff),d0
	bge.s	not_ours
	btst.b	#SPCB_MODIFY,spc_Flags+1(a4)
	beq	no_spc_change

	; calculate the new pos/ctl values
	move.l	spc_ExtSprite(a4),a0
	; the new X value is (((ss_x - scp_Orig_x) << sch_To35ns) + scp_Orig35ns)
	move.w	ss_x(a0),d0
	sub.w	scp_Orig_x(a4),d0
	asl.w	d7,d0
	add.w	scp_OrigX35ns(a4),d0
	; calculate the POS value
	move.w	d0,d6
	lsr.w	#3,d6		; SH10 - SH3 in the low byte
	and.w	#$00ff,d6
	; the new Y value is ((ss_y - scp_Orig_y) + scp_OrigYLine)
	move.w	ss_y(a0),d3
	sub.w	scp_Orig_y(a4),d3
	add.w	scp_OrigYLine(a4),d3
	move.w	d3,-(sp)
	rol.w	#8,d3
	move.w	d3,d5
	and.w	#$ff00,d5
	or.w	d5,d6		; SV7 - SV0 in the upper byte
	move.w	d6,2(a2)	; into the copperlist
	; calculate the CTL value
	move.w	scp_OrigCtl(a4),d6
	and.w	#$0080,d6	; preserve the ATTACH bit
	btst.b	#2,d0		; SH2 set?
	beq.s	1$
	or.b	#1,d6
1$:
	and.w	#3,d0		; SH1 and SH0
	asl.b	#3,d0
	or.b	d0,d6
	btst.b	#0,d3		; SV8 set?
	beq.s	2$
	or.b	#4,d6
2$:
	btst.b	#1,d3		; SV9 set?
	beq.s	3$
	or.b	#$40,d6
3$:
*	move.w	ss_y(a0),d3
	move.w	(sp)+,d3
	add.w	ss_height(a0),d3
	rol.w	#8,d3		; EV7 - EV0 in the upper byte
	move.w	d3,d0
	and.w	#$ff00,d0
	or.w	d0,d6
	btst.b	#0,d3		; EV8 set?
	beq.s	4$
	or.b	#2,d6
4$:
	btst.b	#1,d3		; EV9 set?
	beq.s	5$
	or.b	#$20,d6
5$:
	move.w	d6,6(a2)	; into the copperlist
	; put in the sprite pointers
	moveq	#0,d6
	move.w	sch_PtrOffset(a3),d6
	add.l	ss_posctldata(a0),d6
	move.w	d6,14(a2)	; low word
	swap	d6
	move.w	d6,10(a2)	; high word

	moveq	#0,d0
	move.w	ss_num(a0),d0
	asl.w	#2,d0
	move.w	d0,d3
	add.l	d1,d0		; the SPRxPTH RGA address
	and.w	#$fff,d0
	move.w	d0,8(a2)	; high address
	addq.w	#2,d0
	move.w	d0,12(a2)	; low address
	add.w	#(SPRPOSOFF-2),d0
	add.w	d3,d0		; SPRxPOS RGA address
	move.w	d0,(a2)
	addq.w	#2,d0		; SPRxCTL RGA address
	move.w	d0,4(a2)

no_change_snum:
	tst.l	d2
	bmi.s	no_spc_change	; don't change the caches in the short frame
	lea	scp_CopIns(a4),a0
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)
	bra.s	goat
no_spc_change:
	lea	16(a2),a2		; on to the next instruction
goat:
	lea	scp_SIZEOF(a4),a4	; and the next sc structure
spc_change_loop:
	dbra.s	d2,look_for_move
change_sc.:
	rts

spc_change_interm:
	; now update the intermediate copperlists.
	move.l	ah_ViewPort(a5),a1
	move.l	vp_ColorMap(a1),d0
	beq.s	spc_load
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	spc_load
	btst.b	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	bne.s	AnimateSpriteControl.
spc_load:
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
	btst.b	#SPCB_MODIFY,spc_Flags+1(a4)
	beq.s	9$
	move.l	scp_CopIns(a4),ci_DestAddr(a0)	; SPRxPOS
	move.l	scp_CopCache(a4),(ci_SIZEOF+ci_DestAddr)(a0)	; SPRxCTL
	move.l	(scp_CopCache+4)(a4),((ci_SIZEOF*2)+ci_DestAddr)(a0)	; SPRxPTH
	move.l	(scp_CopCache+8)(a4),((ci_SIZEOF*3)+ci_DestAddr)(a0)	; SPRxPTL
9$:
	lea	scp_SIZEOF(a4),a4
	lea	(ci_SIZEOF*SPC_COPINS)(a0),a0	; skip copper instructions + next WAIT
7$:
	dbra.s	d2,8$
	bra.s	AnimateSpriteControl.

6$:
	lea	ci_SIZEOF(a0),a0
	bra.s	8$

AnimateSpriteControl.:
	rts

* }
*******************************************************************************
