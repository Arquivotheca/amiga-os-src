**********************************************************************
*
*	$Id: a2024driver.asm,v 39.20 93/04/28 16:16:01 spence Exp $
*
******************************************************************************

	incdir	"include:"
V1_3	SET	1
	include	'exec/types.i'
	include 'exec/memory.i'
	include	'exec/ables.i'
	include	'hardware/custom.i'
	include	'hardware/intbits.i'
	include "V39:src/kickstart/graphics/gfxbase.i"
	include "V39:src/kickstart/graphics/copper.i"
	include "V39:src/kickstart/graphics/displayinfo.i"
	include "V39:src/kickstart/graphics/displayinfo_internal.i"
	include "V39:src/kickstart/graphics/vp_internal.i"
	include "V39:src/kickstart/graphics/view.i"
	include	"v39:src/kickstart/graphics/sprite.i"
	include "A2024vb.i"

	xdef _A2024MoveSprite
	xdef _A2024ChangeSprite
	xdef _A2024ScrollVP
	xdef _A2024PokeColours
	xdef _A2024BuildColours
	xdef _A2024BuildNmlCopList
	xdef _A2024vbasm
	xdef _A2024vbname
	xdef _A2024MrgCop
	xdef _A2024LoadView
	xdef _A2024KillView
	xdef _GfxBaseMrgCop
	xdef _GfxBaseLoadView
	xdef _FirstAddress
	xdef _LastAddress
	xdef _VBlankStruct

	xref	_custom
	xref	_LVOCalcIVG
	xref	_LVOMrgCop
	xref	_LVOLoadView

VPOSR	equ	$dff004
VPOSW	equ	$dff02a

debug_on	equ	0

print	macro	string
	ifne	debug_on
	movem.l	a0/a1/d0/d1,-(a7)
	lea	mystring\@(pc),a0
	bsr	kps
	movem.l	(a7)+,a0/a1/d0/d1
	bra.s	skip\@
mystring\@:
	dc.b	\1,0
	cnop	0,2
skip\@:
	endc
	endm

dbugl	macro	expr
	ifne	debug_on
	movem.l	a0/a1/d0/d1,-(a7)
	move.l	\1,d0
	bsr	debug_long
	movem.l	(a7)+,a0/a1/d0/d1
	endc
	endm

dbugw	macro	expr
	ifne	debug_on
	movem.l	a0/a1/d0/d1,-(a7)
	move.w	\1,d0
	bsr	debug_word
	movem.l	(a7)+,a0/a1/d0/d1
	endc
	endm

_LVOAllocMem	equ	-198
_LVOFreeMem	equ	-210
_LVOObtainSemaphore	equ	-564
_LVOReleaseSemaphore	equ	-570
_LVOAddIntServer	equ	-168
_LVORemIntServer	equ	-174

CWAIT	MACRO	; CopIns, VPos, HPos
	move.w	#COPPER_WAIT,ci_OpCode(\1)
	move.w	\2,ci_VWaitPos(\1)
	move.w	\3,ci_HWaitPos(\1)
	ENDM

CMOVE	MACRO	; CopIns, DestAdd, DestData
	move.w	#COPPER_MOVE,ci_OpCode(\1)
	move.w	\2,ci_DestAddr(\1)
	move.w	\3,ci_DestData(\1)
	ENDM

CBUMP	MACRO	; srcCopList, srcCopIns, rsltCopList
	move.l	\1,a0
	add.l	#ci_SIZEOF,\2
	move.l	\2,a1
	jsr	cbump(pc)
	move.l	d0,\2		; returned CopIns pointer
	move.l	a0,\3		; resulting CopList pointer
	ENDM

CEND	MACRO	; CopIns
	CWAIT	\1,10000,255
	ENDM

HFIX	MACRO	; bd, value
	move.l	\1,-(sp)
	move.l	bd_mspc(\1),\1
	mulu	ms_ratioh(\1),\2
	asl.w	#4,\2
	move.l	(sp)+,\1
	ENDM

VFIX	MACRO	; bd, value
	move.l	\1,-(sp)
	move.l	bd_mspc(\1),\1
	mulu	ms_ratiov(\1),\2
	asl.w	#4,\2
	move.l	(sp)+,\1
	ENDM


H_OFFSET	equ	$20

_FirstAddress:


*******************************************************************************

* All the _A2024...() functions take the following parameters:
*	struct View *
*	struct ViewPort *vp
*	struct ViewPortExtra *vpe
* on the stack.

_A2024BuildColours:
	movem.l	a2-a6/d2-d7,-(sp)
	print	"A2024BuildCoolours "
	lea	hedley_hint_delayed(pc),a2
	move.w	#6,(a2)
	movem.l	4+44(sp),a2-a4
	move.l	a6,d3		; save GfxBase

; a2->View
; a3->ViewPort
; a4->ViewPortExtra

	move.l	vpe_DriverData(a4),a5
	move.l	vp_ColorMap(a3),a6

; a5->BuildData
; a6->ColorMap

	moveq	#32,d2		; cmcount
	move.l	a6,d0
	beq.s	101$
	move.w	cm_Count(a6),d2

; d2 = cmcount

101$:
	move.w	d2,d0
	asl.w	#1,d0
	add.w	d2,d0
	asl.w	#1,d0
	add.w	#DSPINS_COUNTAA,d0
	move.l	vp_DspIns(a3),a0
	jsr	_copinit(pc)
	move.l	d0,a0
	move.l	d0,vp_DspIns(a3)
	beq	no_cl
	tst.w	cl_MaxCount(a0)
	beq	no_cl

	move.l	cl_CopIns(a0),a6

; a6 -> struct CopIns *c

	move.l	#$dff000,a4	; custom
	move.w	v_DyOffset(a2),d0
	add.w	vp_DyOffset(a3),d0
	neg.w	d0
	add.w	#21,d0
	move.l	d3,a2		; GfxBase
*	moveq	#(4-$1b),d0
* HARDCODE THE DISPLAYWINDOW Y VALUES
	move.w	#$2c,bd_DiwStrtY(a5)
	move.w	#$f4,bd_DiwStopY(a5)
	and.w	#$ff,bd_RGADiwStopYL(a5)
	and.w	#$ff,bd_RGADiwStopYS(a5)
	btst.b	#PALn,gb_DisplayFlags+1(a2)
	bne.s	2$
	or.w	#$f400,bd_RGADiwStopYL(a5)
	or.w	#$f400,bd_RGADiwStopYS(a5)
	bra.s	1$
2$:
	addq.w	#8,d0
	move.w	#$12c,bd_DiwStopY(a5)
	or.w	#$2c00,bd_RGADiwStopYL(a5)
	or.w	#$2c00,bd_RGADiwStopYS(a5)
1$:
	move.w	d0,d4		; save ypos in d4
	sub.w	#9,d0
	CWAIT	a6,d0,#0
	CBUMP	a0,a6,a0

	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a2)
	beq.s	goat
	CMOVE	a6,#fmode,#0
	CBUMP	a0,a6,a0

goat:
	CMOVE	a6,#bplcon1,#$0
	CBUMP	a0,a6,a0
	CMOVE	a6,#color,#$f00
	CBUMP	a0,a6,a0

	move.l	gb_a2024_sync_raster(a2),d0
	move.l	d0,d1
	swap	d1
	CMOVE	a6,#bplpt,d1
	move.w	d0,-(sp)
	CBUMP	a0,a6,a0
	move.w	(sp)+,d0
	CMOVE	a6,#bplpt+2,d0
	CBUMP	a0,a6,a0

	moveq	#$15,d0
	btst.b	#PALn,gb_DisplayFlags+1(a2)
	beq.s	2$
	add.w	#8,d0
2$:
	asl.w	#8,d0
	move.b	#$61,d0
	CMOVE	a6,#diwstrt,d0
	move.w	d0,-(sp)
	CBUMP	a0,a6,a0
	move.w	(sp)+,d0

	move.b	#$d1,d0
	btst.b	#GFXB_HR_DENISE,gb_ChipRevBits0(a2)
	beq.s	3$
	add.w	#$100,d0
3$:
	CMOVE	a6,#diwstop,d0
	CBUMP	a0,a6,a0

	btst.b	#GFXB_HR_DENISE,gb_ChipRevBits0(a2)
	beq.s	4$
	CMOVE	a6,#diwhigh,#$2000
	CBUMP	a0,a6,a0

4$:
	CMOVE	a6,#ddfstrt,#$40
	CBUMP	a0,a6,a0
	CMOVE	a6,#ddfstop,#$d0
	CBUMP	a0,a6,a0
	CMOVE	a6,#bplcon0,#$9200		; hires, 1bitplane.
	CBUMP	a0,a6,a0

	move.l	a0,-(sp)
	move.l	a3,a0			; a0 was cl
	jsr	new_mode(pc)
	move.l	(sp)+,a0
	move.l	d0,d5

	moveq	#1,d1
	cmp.l	#A2024TENHERTZ_KEY,d5
	bne.s	5$
	or.w	#$800,d1
5$:
	CMOVE	a6,#color+2,d1
	CBUMP	a0,a6,a0

	CWAIT	a6,d4,#($5f+H_OFFSET)
	CBUMP	a0,a6,a0

	moveq	#8,d3
	cmp.l	#A2024TENHERTZ_KEY,d5
	bne.s	6$
	moveq	#12,d3
6$:
	moveq	#0,d7
10$:
	moveq	#1,d6		; tcolor
	btst.b	#1,d7
	beq.s	7$
	or.w	#$f00,d6
7$:
	btst.b	#2,d7
	beq.s	8$
	or.w	#$0f0,d6
8$:
	btst.b	#3,d7
	beq.s	9$
	or.w	#$008,d6
9$:
	CMOVE	a6,#color+2,d6
	lea	field_pick(pc),a1
	move.w	0(a1,d7.w),d0
	or.w	d0,ci_OpCode(a6)
	CBUMP	a0,a6,a0
	addq.w	#2,d7
	cmp.w	d7,d3
	bne.s	10$

	CWAIT	a6,d4,#($7f+H_OFFSET)
	CBUMP	a0,a6,a0

	CMOVE	a6,#color+2,#$8f0	; should really check real depth
	CBUMP	a0,a6,a0

	CWAIT	a6,d4,#($9f+H_OFFSET)
	CBUMP	a0,a6,a0

	move.w	#$fff,d1
	move.l	vp_RasInfo(a3),a1
	move.l	ri_BitMap(a1),a1
	move.w	bm_BytesPerRow(a1),d0
	cmp.w	#126,d0
	ble.s	11$
	move.w	#$ff1,d1
11$:
	cmp.b	#1,bm_Depth(a1)
	bgt.s	12$
	and.w	#$f0ff,d1	; make this 1 bitplane
12$:
	CMOVE	a6,#color+2,d1
	CBUMP	a0,a6,a0

	addq.w	#1,d4
	CWAIT	a6,d4,#0
	CBUMP	a0,a6,a0

	CMOVE	a6,#color,#0
	CBUMP	a0,a6,a0

	CMOVE	a6,#bplcon0,#0
	CBUMP	a0,a6,a0

	moveq	#-2,d1
	sub.w	vp_DyOffset(a3),d1
	IFEQ	1
	moveq	#-2,d0
	VFIX	a5,d0
	moveq	#1,d1
	VFIX	a5,d1
	asl.w	#1,d1
	sub.w	d0,d1
	HFIX	a5,d1
	ENDC

	clr.l	bd_firstwait(a5)
	CWAIT	a6,d1,#0
	CBUMP	a0,a6,a0

* That's the Hedley control bits done. Now, let's go for the real colours.

* First of all, the FMode.
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a2)
	beq.s	nofmode
	move.w	bd_FudgedFMode(a5),d0
	or.w	gb_SpriteFMode(a2),d0
	CMOVE	a6,#fmode,d0
	CBUMP	a0,a6,a0
	CMOVE	a6,#bplcon3,#$c00
	CBUMP	a0,a6,a0

nofmode:
	move.l	a0,a2
	lea	dflt_clrs(pc),a0
	move.l	vp_ColorMap(a3),a1
	move.l	a1,d0
	beq.s	100$
	move.l	cm_ColorTable(a1),a0
100$:
	sub.l	#64,sp
	move.l	sp,a1
	jsr	convert_to_hedley(pc)
	moveq	#31,d7
	move.l	#color,d2
	move.l	sp,a4
	
loadhcolours:
	CMOVE	a6,d2,(a4)+
	CBUMP	a2,a6,a2
	addq.w	#2,d2
	dbra.s	d7,loadhcolours
	add.l	#64,sp
	moveq	#0,d0

	move.l	a6,bd_c(a5)
byebye1:
	print	"leaving A2024BuildColours "
	dbugw	bd_DiwStrtY(a5)
	dbugw	bd_DiwStopY(a5)
	dbugw	bd_RGADiwStopYL(a5)
	dbugw	bd_RGADiwStopYS(a5)
	movem.l	(sp)+,a2-a6/d2-d7
	rts

no_cl:
	moveq	#MVP_NO_DSPINS,d0
	bra.s	byebye1


_A2024ScrollVP:
_A2024BuildNmlCopList:
	moveq	#0,d0
	rts



******************************************************************************

* struct CopList * __regargs copinit(struct CopList *cl, int num)
*                d0                                  a0      d0
_copinit:
	movem.l	d6-d7/a2/a6,-(sp)
	move.l	d0,d6
	move.l	a0,d7		; test cl, and set flag
	bne.s	cophavehead
	moveq	#cl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,d7		; test memory, and set flag.
	beq.s	copinit.
	move.l	d0,a0
cophavehead:
	move.l	a0,a2
	tst.l	cl_CopIns(a2)
	bne.s	havecopins
	moveq	#ci_SIZEOF,d0
	muls	d6,d0
	moveq	#0,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	bne.s	copgotcopins
	tst.l	d7
	beq.s	1$
	move.l	a2,a1
	moveq	#cl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	moveq	#0,d7
	bra.s	copinit.
1$:
	move.w	d0,cl_MaxCount(a2)
	move.l	d0,cl_CopIns(a2)
	move.l	d0,cl_CopPtr(a2)
	move.w	d0,cl_Count(a2)
	move.l	a2,d7
	bra.s	copinit.
copgotcopins:
	move.l	d0,cl_CopIns(a2)
	move.w	d6,cl_MaxCount(a2)
havecopins:
	move.l	cl_CopIns(a2),cl_CopPtr(a2)
	move.w	#0,cl_Count(a2)
	move.l	a2,d7
copinit.:
	move.l	d7,d0
	movem.l	(sp)+,d6-d7/a2/a6
	rts

;struct CopIns *cbump(struct CopList *cl, struct CopIns *c)
;               d0                    a0                  a1
; also returns struct CopList *cl in a0

cbump:
	movem.l	a2-a3/a6,-(sp)
	move.l	a0,a2
	move.l	a1,a3
	move.l	a1,d0
	add.w	#1,cl_Count(a2)
	move.w	cl_MaxCount(a2),d1
	cmp.w	cl_Count(a2),d1
	bgt	cbump.
	tst.l	cl_Next(a2)
	bne.s	havenextbuff
	moveq	#cl_SIZEOF,d0
	move.l	#MEMF_PUBLIC,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,cl_Next(a2)
	bne.s	cat
dog:
	move.l	a3,d0
	sub.w	#1,cl_Count(a2)
	move.l	a2,a0
	bra.s	cbump.
cat:
	move.l	#ci_SIZEOF,d0
	asl.w	#4,d0
	move.l	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	bne.s	2$
	move.l	cl_Next(a2),a1
	moveq	#cl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	move.l	#0,cl_Next(a2)
	bra.s	dog
2$:
	move.l	cl_Next(a2),a0	
	move.l	d0,cl_CopIns(a0)
	move.l	#16,cl_MaxCount(a0)
	move.l	#0,cl_Next(a0)
havenextbuff:
	move.l	cl_Next(a2),a2
	move.l	cl_CopIns(a2),a1
	sub.l	#ci_SIZEOF,a3
	move.w	ci_OpCode(a3),ci_OpCode(a1)
	move.w	ci_nxtlist(a3),ci_nxtlist(a1)
	move.w	#CPRNXTBUF,ci_OpCode(a3)
	move.l	a2,ci_nxtlist(a3)
	move.w	#1,cl_Count(a2)
	add.w	#ci_SIZEOF,a1
	move.l	a1,d0
	move.l	a2,a0

cbump.:
	movem.l	(sp)+,a2/a3/a6
	rts

;ULONG new_mode(struct ViewPort *vp)
;       d0                       a0
new_mode:
	movem.l	a2/a3,-(sp)
	moveq	#0,d0
	move.w	vp_Modes(a0),d0
	move.l	vp_ColorMap(a0),d1
	beq.s	1$
	move.l	d1,a2
	tst.b	cm_Type(a2)
	beq.s	1$
	move.l	cm_CoerceDisplayInfo(a2),d1
	bne.s	2$
	move.l	cm_NormalDisplayInfo(a2),d1
	beq.s	1$
2$:
	move.l	d1,a3
	move.l	rec_Control(a3),d0
	bra.s	new_mode.

1$:
	move.l	d0,d1
	and.l	#V_EXTENDED_MODE,d1
	beq.s	3$
	move.w	vp_Modes(a0),d1
	and.w	#V_DUALPF,d1
	beq.s	4$
	move.l	#$8c44,d1
	bra.s	5$
4$:
	move.l	#$8c04,d1
	bra.s	5$
3$:
	move.w	vp_Modes(a0),d1
	and.w	#V_SUPERHIRES,d1
	beq.s	new_mode.
	move.l	#(~V_HIRES),d1
5$:
	and.l	d1,d0

new_mode.
	movem.l	(sp)+,a2/a3
	rts

;swizzle(UWORD c)
;              d0
swizzle:
	movem.l	d2-d4,-(sp)
	move.w	d0,d2
	lsr.w	#8,d2
	and.w	#$f,d2		; r
	move.w	d0,d3
	lsr.w	#4,d3
	and.w	#$f,d3		; g
	move.w	d0,d4
	and.w	#$f,d4		; b

	mulu	#77,d2
	mulu	#150,d3
	mulu	#29,d4
	
	move.l	d2,d1
	add.l	d3,d1
	add.l	d4,d1
	add.l	#128,d1
	asr.l	#8,d1
	asr.l	#2,d1

	move.w	d1,d0
	and.w	#1,d0
	and.w	#2,d1
	asl.w	#1,d1
	or.w	d1,d0
	movem.l	(sp)+,d2-d4
	rts

;convert_to_hedley(WORD *input,WORD *output)
;                        a0          a1
convert_to_hedley:
	movem.l	a2/a3/d2-d6,-(sp)
	move.l	a0,a2
	move.l	a1,a3
	moveq	#0,d2		; k
	moveq	#0,d3		; i
loop_i:
	moveq	#0,d4		; j
loop_j:
	move.w	d3,d1
	and.w	#1,d1
	asl.w	#1,d1
	move.w	d1,d5
	move.w	d4,d6
	and.w	#1,d6
	or.w	d6,d5		; even_c
	asl.w	#1,d5
	move.w	0(a2,d5.w),d0
	bsr.s	swizzle(pc)
	move.w	d0,d6
	asl.w	#1,d6		; c

	move.w	d4,d1
	asr.w	#1,d1
	move.w	d3,d5
	and.w	#2,d5
	or.w	d1,d5		; odd_c
	asl.w	#1,d5
	move.w	0(a2,d5.w),d0
	bsr	swizzle(pc)
	or.w	d0,d6

	move.w	d6,d0
	bsr.s	to_digital(pc)
	move.w	d2,d1
	asl.w	#1,d1
	move.w	d0,0(a3,d1.w)

	move.w	d4,d0
	add.w	#16,d0
	asl.w	#1,d0
	move.w	0(a2,d0.w),d0
	bsr	swizzle(pc)
	asl.w	#1,d0
	move.w	d0,d6		; c

	move.w	d3,d0
	add.w	#16,d0
	asl.w	#1,d0
	move.w	0(a2,d0.w),d0
	bsr	swizzle(pc)
	or.w	d6,d0

	bsr.s	to_digital(pc)
	move.w	d2,d1
	add.w	#16,d1
	asl.w	#1,d1
	move.w	d0,0(a3,d1.w)
	addq.w	#1,d2

	addq.w	#1,d4
	cmp.w	#4,d4
	blt.s	loop_j

	addq.w	#1,d3
	cmp.w	#4,d3
	blt	loop_i

	movem.l	(sp)+,a2/a3/d2-d6
	rts

;UWORD to_digital(UWORD c)
;      d0               d0
to_digital:
	moveq	#0,d1
	btst.b	#0,d0
	beq.s	1$
	moveq	#1,d1
1$:
	btst.b	#1,d0
	beq.s	2$
	or.w	#8,d1
2$:
	btst.b	#2,d0
	beq.s	3$
	or.w	#$80,d1
3$:
	btst.b	#3,d0
	beq.s	4$
	or.w	#$800,d1
4$:
	move.w	d1,d0
	rts

hedley_hint_delayed:	dc.w	0
hedley_hint:	dc.w	0
hedley_sprite_panel:
	dc.w	0

; A2024vbasm -
; enters with:
; a1 -> vbdata
;
; return with Z flag set

_A2024vbasm:
	move.l	(a1),a0		; a0 -> GfxBase->hedley_count
	tst.w	gb_hedley_flags-gb_hedley_count(a0)		; test hedley_flags
	bmi.s	doA2024vbasm
A2024vbasm.1:
	print	"No A2024vbasm "
A2024vbasm.:
	move.l	#$dff000,a0	; need this if higher priority than gfxlib vbasm
	moveq	#0,d0
	rts
doA2024vbasm:
	move.l	gb_ActiViewCprSemaphore-gb_hedley_count(a0),a5
	cmp.w	#-1,SS_QUEUECOUNT(a5)			; locked?
	bne.s	A2024vbasm.1	; don't touch it!
; safe to call.

; ensure a long frame after switching from an interlaced screen
	xref	_intena
	ENABLE	a5
	move.w	VPOSR,d0
	or.w	#$8000,d0
	move.w	d0,VPOSW
	DISABLE a5,NOFETCH

; now, poke sprite posctldata
	move.w	(a0),d0		; frame # * 8
	move.w	hedley_hint(pc),d1
	beq.s	nohint
	move.w	hedley_sprite_panel(pc),d0
	bra.s	yeshint
nohint:
	lea	hedley_hint_delayed(pc),a5
	subq	#1,(a5)
	bpl.s	yeshint
	clr.w	(a5)
yeshint:
	add.w	d0,d0
	add.w	d0,d0		; cvt to posctl index
	lea	hposctl(pc),a5
	add.w	d0,a5
	lsr.w	#2,d0
	lea	_hsprites(pc),a6	; sprite pointers
	movem.l	a0/d0,-(a7)
	moveq	#7,d0
pokesp:	move.l	(a6)+,a0		; spr ptr
	move.l	(a5)+,(a0)
	dbra	d0,pokesp
	movem.l	(a7)+,a0/d0

	move.l	poke(a1),a5	; start poking here.
	lea	colours(a1),a6	; colours to poke
	add.w	d0,a6
	move.w	#$4000,$dff09a	; disable interrupts


	move.w	(a6)+,(a5)	; poke that colour
	addq.w	#8,a5
	move.w	(a6)+,(a5)	; poke that colour
	addq.w	#8,a5
	move.w	(a6)+,(a5)	; poke that colour
	addq.w	#8,a5
	move.w	(a6),(a5)	; poke that colour

* Now update the value in diwstrty to 0x2c if in the lower half of the screen

;	btst.b	#3,d0		; frame value == 8, 24 etc = a bottom frame
;	beq.s	intopframe
;	addq.w	#8,a5
;	addq.w	#6,a5		; the vertical wait
;	dbugw	(a5)
;	move.b	#$2a,(a5)
;	move.l	todiwstrt(a1),a5
;	dbugw	(a5)
;	move.b	#$2c,(a5)
intopframe:
	move.w	4(a0),d1	; depth from gbase->hedley_tmp
	beq.s	inc_counter
	move.l	tobplptrs(a1),a5 ; a5 now points to the bitplaneptrs.
	lea	offset(a1),a6
	move.w	d0,d1
	asl.w	#1,d1
	add.w	d1,a6		; a6 now points to the bitplane pointers offsets

	move.l	(a6)+,d1	; get the 1st plane pointer
	move.w	d1,4(a5)
	swap	d1
	move.w	d1,(a5)
	addq.w	#8,a5
	move.l	(a6)+,d1	; get the 2nd plane pointer
	move.w	d1,4(a5)
	swap	d1
	move.w	d1,(a5)

	move.w	4(a0),d1	; depth from gbase->hedley_tmp
	cmp.w	#1,d1
	beq.s	1$		; only 1 bitplane deep

	addq.w	#8,a5
	move.l	(a6)+,d1	; get the 3rd plane pointer
	move.w	d1,4(a5)
	swap	d1
	move.w	d1,(a5)
	addq.w	#8,a5
	move.l	(a6),d1		; get the 4th plane pointer
	move.w	d1,4(a5)
	swap	d1
	move.w	d1,(a5)
1$:

inc_counter:
	move.w	#$c000,$dff09a	; enable
	move.w	hedley_hint(pc),d1
	beq.s	2$
	lea	hedley_hint(pc),a5
	clr.w	(a5)
	bra		A2024vbasm.
2$:	addq.w	#8,d0		; increment the frame counter
	cmp.w	maxcount(a1),d0
	blt.s	1$
	moveq	#0,d0
1$:
	move.w	d0,(a0)		; poke back into GfxBase
	bra	A2024vbasm.

field_pick:
	dc.w	$bc00,$7c00,$dc00,$ec00,$f400,$f800

dflt_clrs:
	dc.w		$0000
	dc.w		$0f00
	dc.w		$00f0
	dc.w		$0ff0
	dc.w		$000f
	dc.w		$0f0f
	dc.w		$00ff
	dc.w		$0fff
	dc.w		$0620
	dc.w		$0e50
	dc.w		$09f1
	dc.w		$0eb0
	dc.w		$055f
	dc.w		$092f
	dc.w		$00f8
	dc.w		$0ccc
	dc.w		$0000
	dc.w		$0111
	dc.w		$0222
	dc.w		$0333
	dc.w		$0444
	dc.w		$0555
	dc.w		$0666
	dc.w		$0777
	dc.w		$0888
	dc.w		$0999
	dc.w		$0aaa
	dc.w		$0bbb
	dc.w		$0ccc
	dc.w		$0ddd
	dc.w		$0eee
	dc.w		$0fff

_A2024vbname:
	dc.b	"A2024VBServer",0,0
	cnop	0,2

* Takes a struct View *v in a1,
* GfxBase in a6.

COPSTARTOFFSET	equ	$2e
*TODIWSTRT	equ	45*4
TOBPLTRS	equ	53*4		;7*4

dpl_table:
	dc.w	$000
	dc.w	$0f0
	dc.w	$ff0

VBAdded:
	dc.l	0
LastViewMerged:
	dc.l	0

_A2024MrgCop:
	; View is in a1
	;

	move.l	a6,-(sp)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	(sp)+,a6

	; call the ROM graphics
	move.l	_GfxBaseMrgCop(pc),a0
	move.l	a1,-(sp)
	jsr	(a0)
	lea	LastViewMerged(pc),a0
	move.l	(sp)+,d1		; this View

	; if this View is the same as the last A2024 view, then reload the
	; the hedley_flags. These are reset by MrgCop(), and if the previous
	; View was a custom non-A2024 view, then the hedley_flags value will be
	; lost.

	cmp.l	(a0),d1
	beq.s	2$

	; this is a new A2024 view.
	move.l	d1,(a0)

2$:
	move.l	a6,-(sp)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
	move.l	(sp)+,a6

	rts


_A2024KillView:
* mspec in a0.
	print	"kill view "
*	bclr.b	#7,gb_hedley_flags(a6)	; turn off VBlank for now.
	move.l	a6,-(sp)
	move.l	_VBlankStruct(pc),a1
	moveq	#INTB_VERTB,d0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVORemIntServer(a6)
	move.l	(sp)+,a6
	lea	VBAdded(pc),a0
	clr.w	(a0)
	rts

_A2024LoadView:
	print	"load view "
	bclr.b	#7,gb_hedley_flags(a6)	; turn off VBlank for now.
	move.l	a6,-(sp)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	(sp)+,a6

	lea	LastViewMerged(pc),a0
	cmp.l	(a0),a1
	bne	noa2024lv
	move.w	gb_hedley_flags(a6),d1
	beq	noa2024lv

	move.l	gb_hedley+28(a6),a0	; vbdata is in gfxbase->hedley[7]

* Calculate the colours that are going to be poked into the copperlists.

	movem.l	a2-a3/d2-d5,-(sp)
	moveq	#4,d4			; assume we have a 15Hz mode. Count in d4
	lea	frames4(pc),a3
	moveq	#$001,d0		; 1st colour change in d0
	btst.b	#5,d1
	beq.s	was15hz
	moveq	#6,d4
	lea	frames6(pc),a3
	move.w	#$f01,d0
was15hz:
	move.w	#$ff0,d1

	move.l	a3,-(sp)
	lea	dpl_table(pc),a3
	swap	d4
	move.w	gb_hedley_tmp(a6),d4	; depth
	move.w	d4,d2
	asl.w	#1,d2
	move.w	0(a3,d2.w),d2		; 4th colour in d2
	move.l	(sp)+,a3
	move.w	d4,-(sp)		; save the depth
	swap	d4
	or.w	#$1,d2			; assume 1024 wide
	btst.b	#GFXB_HR_AGNUS,gb_ChipRevBits0(a6)
	bne.s	3$
	or.w	#$f,d2			; make it 1008 wide
3$:
	tst.b	gb_hedley_flags+1(a6)
	bmi.s	1$			; don't reset the counter if this is the
					; same ViewPort as last time
	move.w	#0,gb_hedley_count(a6)
1$:
	move.w	d4,d5
	asl.w	#3,d5
	move.w	d5,maxcount(a0)		; counters are multiplied by 8 for VBasm optimisations.

	lea	colours(a0),a2
*	bclr.b	#7,gb_hedley_flags(a6)	; turn off VBlank for now.
	bra.s	calccolours.

calccolours:
	move.w	d0,(a2)+		; F6-4, INTERLACE, 0, 1
	move.w	d4,d5
	add.w	d5,d5
	move.w	0(a3,d5.w),(a2)+	; FN0, FN1, FN2, EXPAND
	move.w	d1,(a2)+		; ENP0, ENP1, WPB, 0
	move.w	d2,(a2)+		; DPL0, DPL1, LESS16, 1
calccolours.
	dbra.s	d4,calccolours

* now, where is the first copper instruction that will be poked,
* and how many copper instructions are there between the last colour that gets
* poked and the first bitplane pointer instruction?

	move.l	v_LOFCprList(a1),a3
	move.l	crl_start(a3),a3
	add.w	#COPSTARTOFFSET,a3
	move.b	gb_ChipRevBits0(a6),d4
	btst.b	#GFXB_AA_LISA,d4
	beq.s	1$
	addq.w	#4,a3			; AA coplists also poke the fmode.
1$:
	btst.b	#GFXB_HR_DENISE,d4
	bne.s	2$
	subq.w	#4,a3			; A coplists don't have the diwhigh.
2$:
	move.l	a3,poke(a0)

	IFEQ	1
* See to the displaywindow vertical position.

	add.w	#TODIWSTRT,a3
	move.l	a3,todiwstrt(a0)
	move.w	(a3),d0
;	print	'diwstrt = '
;	dbugw	d0
	lsr.w	#8,d0
	sub.w	#$2c,d0
	move.w	d0,dy(a0)
	ENDC

* Now the bitplane pointers

	add.l	#TOBPLTRS,a3
	btst.b	#GFXB_AA_LISA,d4
	beq.s	3$
	addq.l	#8,a3			; extra bplcon3 and fmode
3$:
	btst.b	#GFXB_HR_DENISE,d4
	bne.s	10$
	subq.l	#4,a3			; no bplcon3
10$:
	move.l	a3,tobplptrs(a0)

* Now calculate the bitplanepointers for each frame

	move.l	gb_hedley+24(a6),a2	; struct BitMap * of the front ViewPort is in gfxbase->hedley[6]
	moveq	#0,d0
	move.w	bm_BytesPerRow(a2),d0
	lea	bm_Planes(a2),a2
	move.l	#400,d1
	btst.b	#PALn,gb_DisplayFlags+1(a6)
	beq.s	4$
	move.l	#512,d1
4$:
	mulu	d0,d1

	moveq	#512/8,d2
	btst.b	#5,gb_hedley_flags+1(a6)
	beq.s	5$
	moveq	#336/8,d2

5$:
	IFEQ	1
* Bring into bitplane alignment, using
*	offset &= (0xfffffffe << GBASE->bwshifts[CalcFMode(vp)]);
* There is only one visible ViewPort in the View, so find that.

	moveq	#-2,d5
	movem.l	a0/a1,-(sp)
	move.l	a1,a0			; The View
	move.l	v_ViewPort(a1),a1
8$:
	btst.b	#5,vp_Modes(a1)		; VP_HIDE bit
	beq.s	7$
	move.l	vp_Next(a1),a1
	bra.s	8$			; should never hit a NULL pointer
7$:
	move.l	vp_ColorMap(a1),d3
	beq.s	9$
	move.l	d3,a1
	move.l	cm_vpe(a1),d3
	beq.s	9$
	move.l	d3,a1
	move.w	vpe_FMode(a1),d3
	and.w	#$f,d3
	move.l	gb_bwshifts(a6),a0
	move.b	0(a0,d3.w),d3
	asl.l	d3,d5
9$:
	and.l	d5,d2
	movem.l	(sp)+,a0/a1
	sub.l	d1,d2
	and.l	d5,d1
	ENDC

	sub.l	d1,d2

* calculate the offset from the actual bitplanes to the bplptr values
* (ie how much has the bitplane been scrolled?)

	move.w	(a3),d3
	swap	d3
	move.w	4(a3),d3		; d3 has the address of the 1st bitplane
					; in the copperlist
	move.l	(a2),d5
	sub.l	d3,d5			; this is the scrolled size
	move.w	(sp)+,d4		; get the depth
	lea	offset(a0),a3
	bra.s	calcplanes

calcplanesloop:
	move.l	(a2),d3			; next plane
	sub.l	d5,d3
	move.l	d3,(a3)			; 1st frame
	add.l	d1,d3
	move.l	d3,16(a3)		; 2nd frame
	add.l	d2,d3
	move.l	d3,32(a3)		; 3rd frame
	add.l	d1,d3
	move.l	d3,48(a3)		; 4th frame
	add.l	d2,d3
	move.l	d3,64(a3)		; 5th frame
	add.l	d1,d3
	move.l	d3,80(a3)		; 6th frame
	addq.l	#4,a3			; next plane storage
	move.l	(a2)+,d3
	sub.l	d5,d3
	add.l	d0,d3			; next plane is offset from this one by a scanline
	move.l	d3,(a3)			; 1st frame
	add.l	d1,d3
	move.l	d3,16(a3)		; 2nd frame
	add.l	d2,d3
	move.l	d3,32(a3)		; 3rd frame
	add.l	d1,d3
	move.l	d3,48(a3)		; 4th frame
	add.l	d2,d3
	move.l	d3,64(a3)		; 5th frame
	add.l	d1,d3
	move.l	d3,80(a3)		; 6th frame
	addq.l	#4,a3
calcplanes:
	dbra.s	d4,calcplanesloop

; now, set the fucking sprite pointers to point at the stupid hedley sprites in copinit
	lea	_hsprites(pc),a2
	move.l	gb_copinit(a6),a3
2$:	move.w	(a3),d2
	cmp.w	#sprpt,d2
	beq.s	11$
	lea	4(a3),a3
	bra.s	2$
11$:
	moveq	#7,d2
1$:	move.l	(a2)+,d3
	move.w	d3,6(a3)
	swap	d3
	move.w	d3,2(a3)
	lea	8(a3),a3
	dbra	d2,1$

	bset.b	#7,gb_hedley_flags(a6)	; signal for vblank to work
	move.w	VBAdded(pc),d0
	bne.s	added_vb
	print	"add int server "
	movem.l	a1/a6,-(sp)
	move.l	_VBlankStruct(pc),a1
	moveq	#INTB_VERTB,d0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOAddIntServer(a6)
	movem.l	(sp)+,a1/a6
	lea	VBAdded(pc),a0
	move.w	#-1,(a0)

added_vb:
	movem.l	(sp)+,a2-a3/d2-d5
	bra.s	calllvo

noa2024lv:
	; at this point, a LoadView() was called on this view after another A2024
	; view had been made and loaded. Consequently, the data in GfxBase is old.
	; eg - from a A2024 WB, run a program that opens a custom A2024 View, and
	; then restores the original GfxBase view with a LoadView(). The data in
	; GfxBase is from the MrgCop() of the custom View.
	;
	; Therefore, we need to MrgCop() this View, and recursively LoadView() this
	; View.
	;
	; a1 -> this View
	; a6 -> GfxBase
	; The ActiViewCprSemaphore is locked by this task, but we are calling
	; MrgCop() and LoadView() on this context anyway, so no deadlocking.
	move.l	a1,a2
	jsr	_LVOMrgCop(a6)
	move.l	a2,a1
	jsr	_LVOLoadView(a6)
*	bclr.b	#7,gb_hedley_flags(a6)
calllvo:
	move.l	_GfxBaseLoadView(pc),a0
	move.l	a1,-(sp)
	jsr	(a0)
	move.l	(sp)+,a1

	move.l	a6,-(sp)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
	move.l	(sp)+,a6

*	bset.b	#7,gb_hedley_flags(a6)	; signal for vblank to work

	rts

	ifne	debug_on

output_byte_hex
	move.l	d1,-(a7)
	lsr	#4,d1
	and	#$f,d1
	cmp.b	#10,d1
	blt.s	1$
	add.b	#'A'-'0'-10,d1
1$:	add.b	#'0',d1
	bsr	out_ser_d1
	move.l	(a7),d1
	and	#$f,d1
	cmp.b	#10,d1
	blt.s	2$
	add.b	#'A'-'0'-10,d1
2$:	add.b	#'0',d1
	bsr	out_ser_d1
	move.l	(a7)+,d1
	rts


out_ser_d1	move.w	d0,-(a7)
x1:	move.w	_custom+serdatr,d0
	btst	#13,d0
	beq	x1
	and	#$ff,d1
	bset	#8,d1
	move	d1,_custom+serdat
	move	(a7)+,d0
	rts

kps:
	move.b	(a0)+,d1
	beq.s	done1
	bsr	out_ser_d1
	bra.s	kps
done1	rts

debug_byte
	move.l	d1,-(a7)
	move	d0,d1
	bsr	output_byte_hex
	move.b	#' ',d1
	bsr	out_ser_d1
	move.l	(a7)+,d1
	rts


debug_word
	move.l	d1,-(a7)
	move	d0,d1
	lsr	#8,d1
	bsr	output_byte_hex
	move	d0,d1
	bsr	output_byte_hex
	move.b	#' ',d1
	bsr	out_ser_d1
	move.l	(a7)+,d1
	rts

debug_long
	movem.l	d0/d1,-(a7)
	swap	d0
	move	d0,d1
	lsr	#8,d1
	bsr	output_byte_hex
	move	d0,d1
	bsr	output_byte_hex
	movem.l	(a7),d0/d1
	move	d0,d1
	lsr	#8,d1
	bsr	output_byte_hex
	move	d0,d1
	bsr	output_byte_hex
	move.b	#' ',d1
	bsr	out_ser_d1
	movem.l	(a7)+,d0/d1
	rts
	
	endc

_A2024PokeColours:
; a2=viewport
	movem.l	a2-a6/d2-d7,-(a7)
	print	"pokecolours "
	sub	#64,a7		; a7=work space for color conversion
	move.l	vp_ColorMap(a2),a0
	move.w	cm_Count(a0),d7
	move.l	cm_ColorTable(a0),a0
	move.l	a7,a1
	bsr	convert_to_hedley
	move.l	a6,a5
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	vp_DspIns(a2),d0
	beq	no_poke
	move.l	d0,a3		; a3=dspins=CopList ptr
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne	no_poke
	move.l	cl_CopLStart(a3),a3	; a3=cop ptr
; now, search for bplcon0 load before the colors are loaded
getbp0:
	cmp.w	#bplcon0,(a3)
	lea	4(a3),a3
	bne.s	getbp0
2$:	cmp.w	#bplcon0,(a3)
	lea	4(a3),a3
	bne.s	2$
3$:	cmp.w	#color,(a3)
	beq.s	4$
	lea	4(a3),a3
	bra.s	3$
; now, we are pointing (hopefully!) at a color 0 load!
4$:	move.w	(a3),d0			; d0=color reg num
	cmp.w	#color,d0
	blt.s	no_poke
	cmp.w	#color+32,d0
	bhs.s	no_poke
	and	#63,d0
	move.w	0(a7,d0.w),2(a3)
	lea	4(a3),a3
	bra.s	4$
no_poke:	move.l	gb_ActiViewCprSemaphore(a5),a0
	jsr	_LVOReleaseSemaphore(a6)
	move.l	a5,a6
	add	#64,a7
	movem.l	(a7)+,a2-a6/d2-d7
	rts

	xdef	_hsprites

_hsprites:	ds.l	8	; eight sprite pointers
hposctl:	ds.l	8*8	; position & control words for all (upto 6 frames) of all 8 sprites, poked during vblank



cvtsprite:
; entr a2=sdata d0=num trashes a2/d0
	print	"cvtsprite a2 d0 = "
	dbugl	a2
	dbugw	d0
	cmp.l	#0,a2
	beq.s	cvtsprite.
	add	d0,d0
	add	d0,d0
	move.l	a0,-(a7)
	lea	_hsprites(pc),a0
	move.l	0(a0,d0.w),a0
	moveq	#20,d0
	lea	4(a0),a0
	lea	4(a2),a2
1$:	move.l	(a2)+,(a0)+
	dbra	d0,1$
	move.l	(a7)+,a0
cvtsprite.:
	rts
		
	


ofstable_start	equ	*
ntsc_ofstable_15hz:
	dc.w	128,44
	dc.w	128,44-200
	dc.w	128-256,44
	dc.w	128-256,44-200
	dc.l	0
ntsc_ofstable_10hz:
	dc.w	113,44
	dc.w	113,44-200
	dc.w	113-336,44
	dc.w	113-336,44-200
	dc.w	113-336*2,44
	dc.w	113-336*2,44-200
	dc.l	0
ofstable_size	equ	*-ofstable_start

pal_ofstable_15hz:
	dc.w	128,44
	dc.w	128,44-256
	dc.w	128-256,44
	dc.w	128-256,44-256
	dc.l	0
pal_ofstable_10hz:
	dc.w	113,44
	dc.w	113,44-256
	dc.w	113-336,44
	dc.w	113-336,44-256
	dc.w	113-336*2,44
	dc.w	113-336*2,44-256
	dc.l	0


_A2024ChangeSprite:
	print	"changesprite a1 a2 = "
	dbugl	a1
	dbugl	a2
	move.l  a2,ss_posctldata(a1)
	move.w	ss_num(a1),d0
	bsr	cvtsprite
	movem.w	ss_x(a1),d0/d1
	bra.s	moves1
; fall A2024MoveSprite

_A2024MoveSprite:
;	print	'movesp xy='
;	dbugw	d0
;	dbugw	d1
	print	"MoveSprite a1 posctl(a1) = "
	dbugl	a1
	dbugl	ss_posctldata(a1)
	movem.l	a2/d0,-(a7)
	move.w	ss_num(a1),d0
	move.l	ss_posctldata(a1),a2
	bsr	cvtsprite
	movem.l	(a7)+,a2/d0

moves1:
	movem.l	a5/d2-d7,-(a7)
	movem	d0/d1,ss_x(a1)
	cmp.l	#0,a0	; vp=0?
	beq	no_vp
	movem	vp_DxOffset(a0),d2/d3
	add.w	d2,d0
	add.w	d3,d1
;	add.w	d3,d1

no_vp:	
	move.w	hedley_hint_delayed(pc),d7
	bne.s	nohintsp
	lea	hedley_hint(pc),a0
	st	(a0)
	move.w	#1,hedley_hint_delayed-hedley_hint(a0)
nohintsp:
	moveq	#0,d7
	lea	ntsc_ofstable_15hz(pc),a0		; should be mode sensitive
	btst	#5,gb_hedley_flags+1(a6)
	beq.s	is_15_ms
	lea	ntsc_ofstable_10hz(pc),a0
is_15_ms:
	btst.b	#PALn,gb_DisplayFlags+1(a6)
	beq.s	1$
	add.l	#ofstable_size,a0
1$:
	move.w	ss_height(a1),d3
	move.w	ss_num(a1),d2
	lsl	#2,d2
	lea	hposctl(pc),a1
	add	d2,a1
ms1:	tst.l	(a0)				; end of table?
	beq	no_movesprite
	movem.w	d0-d1,-(a7)
	btst	#5,gb_hedley_flags+1(a6)
	bne.s	is_10_ms
	asr.w	#1,d0				; should only do if 15 hz
is_10_ms:
	asr.w	#1,d1
	add.w	(a0)+,d0
	add.w	(a0)+,d1

	cmp.w	#455+16,d0
	bhs	spclip
	cmp.w	#113-16,d0
	blt.s	spclip

	btst.b	#PALn,gb_DisplayFlags+1(a6)
	beq.s	1$
	cmp.w	#300,d1
	bhs	spclip
	cmp.w	#30,d1
	blt.s	spclip
	bra.s	2$
1$:
	cmp.w	#244,d1
	bhs	spclip
	cmp.w	#22,d1
	blt.s	spclip
2$:
	lea	hedley_sprite_panel(pc),a5
	move.w	d7,(a5)
	move	d1,d6
	move	d0,d4
	moveq	#0,d5				; d5=ctl value
	lsr.w	#1,d4
	bcc.s	ms2
	addq	#1,d5
ms2:	and.w	#$0ff,d4
	lsl.w	#8,d1
	bcc.s	ms3
	bset	#2,d5
ms3:	or.w	d1,d4
	add.w	d3,d6
	lsl.w	#8,d6
	bcc.s	ms4
	bset	#1,d5
ms4:	or.w	d6,d5
	move.w	d4,(a1)
	move.w	d5,2(a1)
ns1:	lea	8*4(a1),a1
	addq	#8,d7
	movem.w	(a7)+,d0-d1		
	bra	ms1
spclip:
	clr.l	(a1)
	bra.s	ns1
no_movesprite:
	movem.l	(a7)+,a5/d2-d7
	rts


_GfxBaseLoadView:
	dc.l	0

_GfxBaseMrgCop:
	dc.l	0

_VBlankStruct:
	dc.l	0

frames6:
	dc.w	$f0f,$00f
frames4:
	dc.w	$ff1,$0f1,$f01,$001

_LastAddress:

	end
