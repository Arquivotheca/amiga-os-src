*******************************************************************************
*
*	$Id: coppermover.asm,v 42.0 93/06/16 11:17:08 chrisg Exp $
*
*******************************************************************************

	section graphics

V1_3	EQU	1		; for copper.i

	include 'exec/types.i'
	include	'exec/alerts.i'
	include "/copper.i"
	include "/view.i"
	include "/gfxbase.i"
	include "/monitor.i"
	include "/display.i"
	include	"/macros.i"
	include	'hardware/custom.i'

	xdef _coppermover

	STRUCTURE copstuff,0
		APTR	cs_first
		APTR	cs_copptr
		APTR	cs_copIptr
		UWORD	cs_maxins
		UWORD	cs_beamx
		UWORD	cs_beamy
		UWORD	cs_flags
		UWORD	cs_cnt
		UWORD	cs_icnt
		UWORD	cs_DyOffset
		UWORD	cs_moreflags
		APTR	cs_fcopptr
		APTR	cs_cop2ptr
		APTR	cs_cop3ptr
		APTR	cs_cop4ptr
		APTR	cs_cop5ptr
	        UWORD	cs_cnt2
	        UWORD	cs_cnt3
	        UWORD	cs_cnt4
	        UWORD	cs_cnt5
		LONG cs_ratioh
		UWORD cs_colorclocks
		SHORT cs_ibeamx
		SHORT cs_ibeamy
		ULONG	cs_topline
		UWORD	cs_thisbeamy
		UWORD	cs_origbeamy
		UWORD	cs_totalrows
	LABEL	cs_SIZEOF

nCPR_NT_LOF	equ 15
nCPR_NT_SHT	equ 14

nCPR_NT_2	equ 13
nCPR_NT_3	equ 12
nCPR_NT_4	equ 11
nCPR_NT_5	equ 10

*	ensure that ci_Opcode is offset 0
	ifne	ci_OpCode
		fail
	endc

*DODEBUG	EQU	1

	IFD	DODEBUG
D	MACRO
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	print	\1
dbg\@
	ENDM
DR	MACRO
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	dbug	\1
dbg\@
	ENDM
DRW	MACRO
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	dbugw	\1
dbg\@
	ENDM

	ELSE

D	MACRO
	ENDM
DR	MACRO
	ENDM
DRW	MACRO
	ENDM

	ENDC

_coppermover:
* coppermover(copstuff,c)
*	needs copstuff (co
	movem.l	4(sp),a0/a1	* get copstuff,c

	move.w	(a1),d1		* ci_OpCode(a1)
	D	"coppermover, c = "
	DR	a1
	D	"instruction = "
	DRW	d1
	move.w	d1,d0
	and.w	#3,d0
*	assume COPPER_MOVE is a NULL
	if =
* some commonly used constants
		movem.l	d2/d3/d4/a2/a3,-(sp)
		move.l	#$1feffff,d2
		moveq	#3,d3
		moveq	#nCPR_NT_SHT,d4

		movem.l	cs_copptr(a0),a2/a3	* and copIptr
		addq.l	#2,a1			* preoffset
		if #0=a3.w
			repeat
*				get and convert move instruction
				move.l (a1)+,d0		* ci_DestAddr(a1)
				and.l	d2,d0

*				which list to put it in?
				btst	d4,d1
				if =
					move.l	d0,(a2)+
				endif

				move.w	(a1)+,d1	* ci_OpCode(a1)
				move.w	d1,d0
				and.w	d3,d0
			until <>
		else
			btst.b	#1,cs_moreflags+1(a0)
			if =
				repeat
*					get and convert move instruction
					move.l (a1)+,d0		* ci_DestAddr(a1)
					and.l	d2,d0
	
*					which list to put it in?
					btst	d4,d1
					if =
						move.l	d0,(a2)+
					endif
					btst	#nCPR_NT_LOF,d1
					if = 
						move.l	d0,(a3)+
					endif
	
					move.w	(a1)+,d1	* ci_OpCode(a1)
					move.w	d1,d0
					and.w	d3,d0
				until <>
			else
				repeat
*					get and convert move instruction
					move.l (a1)+,d0		* ci_DestAddr(a1)
					and.l	d2,d0
	
*					which list to put it in?
					btst	d4,d1
					if =
						move.l	d0,(a2)+
					endif
					btst	#nCPR_NT_LOF,d1
					if = 
						move.l	d0,(a3)+
					endif
	
					move.w	(a1)+,d1	* ci_OpCode(a1)
					move.w	d1,d0
					and.w	d3,d0
				until <>
			endif
		endif
		subq.l	#2,a1			* move back again
		movem.l	a2/a3,cs_copptr(a0)	* put updated values back
		movem.l	(sp)+,d2/d4/d3/a2/a3
	endif
	move.l	a1,d0			* return value
	rts

*----------------------------------------------------------------------------*
*
* The following is copied from do_wait.asm, so that only one copy of
* the assembler structure copstuff needs to be defined.
*
* spence Wed Oct  3 10:14:52 1990
*
*----------------------------------------------------------------------------*

*******************************************************************************
*
*   A downcoded version of the do_wait function in /c/copper.c
*
*   All variables are converted to registers wherever possible, rather than
*   use the stack.
*
*   a0 -> cs   CopStuff              d0:32 = scratch
*   a1 -> c    CopIns                d1:32 = scratch
*   a2 -> cl   CopList               d2:16 = ov_flow
*   a3 -> mspc MonitorSpec           d3:16 = v_waitpos_rel
*   a4 -> bmx                        d4:16 = tot_rows and v_waitpos_abs
*   a5 -> bmy                        d5:16 = min_ytop
*   a6 -> GfxBase                    d6:16 = h_waitpos
*   a7 -> stack                      d7:16 = v_waitpos
*
*******************************************************************************

COPPERVPOS_OVERFLOW equ 1
COPPERIVPOS_OVERFLOW equ	8
STRADDLES_256	equ	VPXB_STRADDLES_256
STRADDLES_512	equ	VPXB_STRADDLES_512
WHOLE_LINE equ	64

DC_HWAIT equ 1
*DONT_USE_DOWAIT equ	1

	IFND	DC_HWAIT
	xref	_get_hwait_hack
	ENDC
	xref	_new_mode
	xref	_custom

	IFD		DC_HWAIT
	xdef	_get_hwait_hack
	ENDC
	IFND	DONT_USE_DOWAIT
	xdef	_do_wait

MAKE_DY_POS	MACRO
	ENDM

CLIP_VPOS MACRO
	tst.w	\1
	bpl	\@cvp
	moveq	#0,\1
\@cvp:
	ENDM

_do_wait:
	movem.l	a2-a6/d2-d7,-(sp)
	movem.l	4+4*11(sp),a0-a3	; cs, c, cl, mspc
	moveq.l	#0,d2			; ov_flow  (upper word is skip value)
					; skip == FALSE, top word d2 is positive

	D	10
	D	"_do_wait() *** cs, c, cl, mspc, vp = "
	DR	a0
	DR	a1
	DR	a2
	DR	a3
	DR	cl__ViewPort(a2)
	move.w	cs_beamy(a0),cs_origbeamy(a0)
	move.w	ms_total_rows(a3),d4	; tot_rows
	subq.w	#1,d4
	move.w	d4,cs_totalrows(a0)
	addq.w	#1,d4

*		a3 can now be used as scratch.

	move.w	ci_VWaitPos(a1),d7	; v_waitpos
	move.l	cl__ViewPort(a2),a4
	move.w	vp_DyOffset(a4),d0
	MAKE_DY_POS	d0
	add.w	d0,d7

	movem.l	a0-a1,-(sp)
	move.l	a4,-(sp)
	jsr	_new_mode
	addq.l	#4,sp
	movem.l	(sp)+,a0-a1
	D	"modes in d0 = "
	DRW	d0
	btst.b	#2,d0		; LACE?
	if <>
		addq.w	#1,d7
		asr.w	#1,d7
	endif
	btst.b	#3,d0		; DOUBLESCAN
	if <>
		add.w	d7,d7
		btst.b	#1,cl_Flags+1(a2)	; HALF_LINE ?
		beq.s	100$
		addq.w	#1,d7
100$:
	endif
	add.w	cs_DyOffset(a0),d7
	D	"ci_VWaitVPos "
	DRW	ci_VWaitPos(a1)
	D	"cs_DyOffset = "
	DRW	cs_DyOffset(a0)
	D	"vp->DyOffset = "
	DRW	vp_DyOffset(a4)
	D	"clipping d7 = "
	DRW	d7
	CLIP_VPOS d7

	move.w	gb_system_bplcon0(a6),d0
	andi.w	#(GENLOCK_AUDIO+GENLOCK_VIDEO),d0
	if <>
		subq.w	#1,d4
	endif
	if	d7.w >= d4.w
		move.w	d4,d7
		subq.w	#1,d7
		cmp.w	#255,d7
		bge.s	fuckit
		D	"Whoops! d7 < 255. d7 == "
		DRW	d7
		move.w	#255,d7		; terminate the copperlist for
					; monitors with less than 255 total_rows
fuckit:
	endif

HIGHESTLINE equ	$500

	moveq	#0,d6
	move.l	cs_topline(a0),d6
	addq.l	#1,d6			; is it -1?
	bne.s	nottopline
	moveq	#0,d6
	move.w	d7,d6
	asl.l	#8,d6
	cmp.l	#HIGHESTLINE,d6
	bhi.s	use_d6
	move.l	#HIGHESTLINE,cs_topline(a0)
	move.w	#(HIGHESTLINE>>8),d7
	D	"use HIGHESTLINE "
	bra.s	nottopline
use_d6:
	move.l	d6,cs_topline(a0)
nottopline:

* d4:16 can now be used for v_waitpos_abs

	move.w	ci_HWaitPos(a1),d6	; h_waitpos
	move.w	d7,d4		; v_waitpos_abs
	move.w	d7,d3		; v_waitpos_rel

	move.b	gb_ChipRevBits0(a6),d0
	and.b	#GFXF_HR_DENISE,d0
	bne.s	cow
	btst.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
	bne.s	cow
	btst.b	#nCPR_NT_SHT-8,ci_OpCode(a1)
	bne.s	cow
	cmp.w	cs_ibeamy(a0),d4
	bne.s	6$
	cmp.w	cs_ibeamx(a0),d6
	bne.s	6$
	swap 	d2
	move.w	#-1,d2 		; skip == TRUE, upper word of d2 is negative
	swap 	d2
	bra.s	cow
6$	move.w	cs_moreflags(a0),d1
	and.w	#COPPERVPOS_OVERFLOW,d1
	beq.s	cow
	cmpi.w	#255,cs_ibeamy(a0)
	bgt.s	cow
	cmpi.w	#255,d7
	ble.s	cow

	bset.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
	movem.l	a0-a3,-(sp)
	D	"call self for short frame "
	jsr	_do_wait		; call self for short frame
	movem.l	(sp)+,a0-a3
	bclr.b	#nCPR_NT_LOF-8,ci_OpCode(a1)

	bset.b	#nCPR_NT_SHT-8,ci_OpCode(a1)
	movem.l	a0-a3,-(sp)
	D	"call self for long frame "
	jsr	_do_wait		; call self for long frame
	movem.l	(sp)+,a0-a3
	bclr.b	#nCPR_NT_SHT-8,ci_OpCode(a1)

	bra   	do_wait_bye

cow:
	D	"cow "
 	lea	cs_beamx(a0),a4	; bmx
	lea	cs_beamy(a0),a5	; bmy

	btst.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
	if <>
		D	"ibeam "
		lea		cs_ibeamx(a0),a4	; bmx
		lea		cs_ibeamy(a0),a5	; bmy
	endif

	D	"Check line 255. d7 = "
	DRW	d7
	if #255 < d7.w	.extend
		* If the previous ViewPort's IVG straddled line 256, then
		* don't bother with the rest of this shit. Only do this for
		* AA machines though, as on ECS, the forced IVG of 3 lines is
		* more than needed, so the extra WAIT is necessary.
		btst.b	#STRADDLES_256,cs_moreflags+1(a0)
		beq.s	no_straddles
		D	"straddles "
		move.b	gb_ChipRevBits0(a6),d0
		and.b	#GFXF_AA_LISA,d0
		beq.s	no_straddles
		* save ourselves some grief later
		* Shit. I *really* hate this code.
		move.w	d7,d0
		sub.w	#256,d0
		cmp.w	(a5),d0
		bne.s	2$
		add.w	#256,(a5)
2$:
		* If d7 >512 and the previous ViewPort straddled line 512
		* we don't want an extra WAIT(255, xx)
		* This only applies to the DblPAL monitor, which is the only
		* one with >512 lines!
		cmp.w	#512,d7
		blt.s	1$
		btst.b	#STRADDLES_512,cs_moreflags+1(a0)	
		beq.s	1$
		move.w	#2,d2
		sub.w	#256,d3
		bra	dont_do_fw
1$:
		move.w	#1,d2
		bra	dont_do_fw
no_straddles:
		move.b	gb_ChipRevBits0(a6),d0
		and.b	#GFXF_HR_DENISE,d0
		beq.s	4$
		move.w	(a5),d0
		if #255 <> d0.w
4$:			move.w	#255,d7
			if d7.w <> d6.w
				D	"d7 <> d6 "
				DRW	d7
				DRW	d6
				move.w	#1,d2
				move.w	cs_moreflags(a0),d1
				btst.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
				bne.s	1$
				and.w	#COPPERVPOS_OVERFLOW,d1
				bra.s	2$
1$:				and.w	#COPPERIVPOS_OVERFLOW,d1
2$:				beq.s	3$
				move.w	(a5),d0
				and.w	#$FF00,d0
				sub.w	d0,d3
				bra	vpos_overflow
3$:
				D	"wanker "
				move.w	d4,cs_thisbeamy(a0)
				IFND	DC_HWAIT
				move.l	a1,-(sp)
				move.l	a0,-(sp)
				jsr		_get_hwait_hack
				move.l	(sp)+,a0
				move.l	(sp)+,a1
				ENDC
				IFD	DC_HWAIT
				jsr		_get_hwait_hack
				ENDC
				move.w	d0,d6
			endif
		else
			if #255 <> d6.w
				D	"bollox "
				move.w	#1,d2
				bra	vpos_overflow
			else
				D	"tosspot "
				move.w	#255,d7
			endif
		endif
	endif

finish_wait:
	D	"finish_wait. (a4) (a5) d6 d7 d2.l "
	DRW	(a4)
	DRW	(a5)
	DRW	d6
	DRW	d7
	DR	d2
	cmp.w	#-1,d6		; error from get_hwait_hack()
	beq	dont_do_fw
	cmp.w	(a4),d6
	bne.s	do_fw
	cmp.w	(a5),d7
	beq	dont_do_fw
do_fw:
	D	"do_fw "
	tst.w	d2
	beq.s	do_fw1
	move.w	(a5),d0
	cmp.w	#255,d0
	bne.s	do_fw1
	cmp.w	(a4),d6
	ble.s	dont_do_fw1
do_fw1:
	D	"do_fw1"
	IFD		COPPERDEBUG
	subq.w	#1,cs_cnt(a0)
	bmi		AlertItL
	ENDC
	move.w	d6,d0
	and.w	#$FE,d0
	move.w	d7,d1
	asl.w	#8,d1
	or.w	d0,d1
	or.w	#COPPER_WAIT,d1
	swap	d1
	move.w	#$FFFE,d1	; d1.l = temp
	btst.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
	if = .extend
		move.l	cs_copptr(a0),a3
		move.l	d1,(a3)+
		move.l	a3,cs_copptr(a0)
		D	"do_fw1 d4 d6 = "
		DRW	d4
		DRW	d6
		move.w	d4,(a5)
		move.w	d6,(a4)
		btst.b	#nCPR_NT_SHT-8,ci_OpCode(a1)
		if =
			D	"ibeam "
			move.w	d4,cs_ibeamy(a0)
			move.w	d6,cs_ibeamx(a0)
		endif
	endif
	
	move.w	cs_flags(a0),d0
	and.w	#INTERLACE,d0
	beq.s	dont_do_fw
do_fw2:
	D	"do_fw2 "
	IFD		COPPERDEBUG
	subq.w	#1,cs_icnt(a0)
	bmi		AlertItS
	ENDC
	btst.b	#nCPR_NT_SHT-8,ci_OpCode(a1)
	if =
		D	"d2 = "
		DR	d2
		tst.l	d2	; test UPPER word of d2 -- skip if negative
		blt.s	1$
		move.l	cs_copIptr(a0),a3
		move.l	d1,(a3)+
		move.l	a3,cs_copIptr(a0)
1$		move.w	d4,(a5)
		move.w	d6,(a4)
	endif
dont_do_fw1:
	D	"dont_do_fw1 "
	cmpi.w	#1,d2
	bne.s	1$
	cmpi.w	#255,(a5)
	bne.s	1$
	move.w	#2,d2
1$ 	move.w	d4,(a5)
dont_do_fw:
	D	"dont_do_fw. d2 d3 d4 = "
	DRW	d2
	DRW	d3
	DRW	d4
	tst.w	d2
	if <>
		sub.w	#256,d3
vpos_overflow:
		btst.b	#nCPR_NT_LOF-8,ci_OpCode(a1)
		if =
			D	"frog "
			or.w	#COPPERVPOS_OVERFLOW,cs_moreflags(a0)
			btst.b	#nCPR_NT_SHT-8,ci_OpCode(a1)
			if =
				D	"goat "
				or.w	#COPPERIVPOS_OVERFLOW,cs_moreflags(a0)
			endif
		else
			D	"donkey "
			or.w	#COPPERIVPOS_OVERFLOW,cs_moreflags(a0)
		endif
		move.w	d3,d7

		move.w	d4,cs_thisbeamy(a0)
		if #256 > d7.w
			D	"#256 > d7.w "
			cmpi.w 	#2,d2
			bne.s	1$
			cmp.w	(a5),d4
			bne.s	1$
			cmp.w	cs_totalrows(a0),d4
			beq.s	1$		; we have already clipped to total_rows
			IFND	DC_HWAIT
			move.l	a1,-(sp)
			move.l	a0,-(sp)
			jsr		_get_hwait_hack
			move.l	(sp)+,a0
			move.l	(sp)+,a1			
			ENDC
			IFD	DC_HWAIT
			jsr		_get_hwait_hack
			ENDC
			move.w	d0,d6
			cmp.w	ci_HWaitPos(a1),d6
			blt.s	1$
			add.w	#256,d3
			move.w	#255,d7
			move.w	#3,d2
			bra.s	2$
1$ 			move.w	ci_HWaitPos(a1),d6
			move.w	#0,d2
2$
		else
			IFND	DC_HWAIT
			move.l	a1,-(sp)
			move.l	a0,-(sp)
			jsr		_get_hwait_hack
			move.l	(sp)+,a0
			move.l	(sp)+,a1			
			ENDC
			IFD	DC_HWAIT
			jsr		_get_hwait_hack
			ENDC
			move.w	d0,d6
			move.w	#255,d7
		endif
		bra	finish_wait
	endif

do_wait_bye:
	movem.l	(sp)+,a2-a6/d2-d7
	rts

	IFD		COPPERDEBUG
_LVOAlert equ	-108

AlertItL:
	ALERT	AN_LongFrame
	bra	do_wait_bye

AlertItS:
	ALERT	AN_ShortFrame
	bra	do_wait_bye
	ENDC

	ENDC

*******************************************************************************
*
*   A downcoded version of the get_hwait_hack function in /c/copper.c
*
*   All variables are converted to registers wherever possible, rather than
*   use the stack.
*
*   a0 -> cs   CopStuff              d0:32 = scratch
*   a1 -> c    CopIns                d1:32 = scratch
*   a2 -> cl   CopList               d2:32 = scratch
*   a3 -> variables on stack         d3 = first
*   a4 -> state jump table           d4 = second
*   a5 -> hackptr                    d5 = maxcount
*   a6 -> GfxBase                    d6 = target
*   a7 -> stack                      d7 = state
*
*   Enters with a0/a1/a6 set up.
*   do_wait() also sets up d3.w and d4.w. d4 is the YWwaitPos of this ViewPort,
*   and d3 keeps track of the number of consecutive WAITs. The true Y position
*   of this wait is ((d4.w - d3.w) + 0xff).
*
*   Returns value in d0:16, == -1 if the WAIT on line 255 should be skipped.
*
*******************************************************************************


	IFD		DC_HWAIT
	STRUCTURE hwaitstuff,0
		UWORD	hw_dafstrt
		UWORD	hw_dafstop
		UWORD	hw_diwhigh
		UWORD	hw_diwstop
		UWORD	hw_diwstrt
		UWORD	hw_bplcon0
		UWORD	hw_bovp
		UWORD	hw_tovp
		UWORD	hw_hmax
		UWORD	hw_oldbplcon0
		UWORD	hw_granularity
		UWORD	hw_granularity1
		UWORD	hw_fmode
		UWORD	hw_YWait
	LABEL	hwait_sizeof

STATE0	equ	0
STATE1	equ	(STATE0+4)
STATE2	equ	(STATE1+4)
STATE3	equ	(STATE2+4)
STATE4	equ	(STATE3+4)
STATE5	equ	(STATE4+4)
STATE6	equ	(STATE5+4)
STATE7	equ	(STATE6+4)

HWAIT	equ	222
GRANULARITY equ	8
FETCH_MASK equ	$FFFC

	xref	lmult
	xref	_CalcFMode

HFIX	MACRO
	move.l	a0,-(sp)	; if HFIX is used more than once, save a0 elsewhere for speed
	move.l	cs_ratioh(a0),d1
	jsr	lmult		; d0.l = (d1.l * d0.l)
	asr.l	#4,d0
	move.l	(sp)+,a0
	ENDM

* {
_get_hwait_hack:
	IFD	DONT_USE_DOWAIT
	movem.l	4(sp),a0/a1		; from C source
	ENDC
	movem.l	a2-a6/d2-d7,-(sp)
	sub.l	#hwait_sizeof,sp
	move.l	sp,a3

	move.w	#GRANULARITY,hw_granularity(a3)
	move.w	#GRANULARITY-1,hw_granularity1(a3)
	moveq	#0,d0
	move.l	d0,(a3)		; dafstrt, dafstop
	move.l	d0,hw_diwhigh(a3)	; and diwstop
	move.l	d0,hw_diwstrt(a3)	; and bplcon0
	move.l	d0,hw_bovp(a3)		; and tovp
	move.l	d0,hw_hmax(a3)		; and oldbplcon0
	move.l	d0,hw_fmode(a3)		; and hw_VYPosFlag
	move.w	d0,hw_YWait(a3)

	move.w	#$ff00,d0
	and.w	d0,d3
	and.w	d0,d4
	sub.w	d3,d4			; which block number?
	add.w	#$ff,d4
	move.w	d4,hw_YWait(a3)		; the true y WAIT value 
	move.w	cs_thisbeamy(a0),d7
	move.w	cs_origbeamy(a0),d5
	and.w	d0,d7
	and.w	d0,d5
	sub.w	d5,d7
	cmp.w	#$200,d7
	bge.s	ret_hwait		; this WAIT should be max-X as another WAIT
					; is immediately following (eg line 512).

	moveq	#STATE1,d7		; starting state
	lea	state_table(pc),a4	; jump table
	move.l	cs_copptr(a0),d5
	move.l	d5,a5			; hackptr
	sub.l	cs_fcopptr(a0),d5
	asr.l	#2,d5			; d5:16 = maxcount

	move.b	gb_ChipRevBits0(a6),d0
	and.b	#(GFXF_HR_DENISE|GFXF_AA_LISA),d0
	if =			; Old A Chips.
		move.l	#_custom+bplcon0,d6
		moveq	#STATE6,d7
	else
		btst.b	#GFXB_AA_LISA,d0
		if = 		; ECS chips
			move.l	#_custom+diwhigh,d6
		else
			move.l	#_custom+fmode,d6
			moveq	#STATE7,d7
		endif
	endif
	and.l	#$1FE,d6		; d6:16 = target
	D	"First state = "
	DR	d7
	D	"First target = "
	DR	d6

while_loop:
	tst.w	d5
	beq.s		end_while
	subq.w	#1,d5
	move.w	-(a5),d4		; second
	move.w	-(a5),d3		; first
	if d3.w = d6.w
		jmp	0(a4,d7.w)	; jump into the appropriate switch
	endif
	bra		while_loop

end_while:
ret_hwait:
	moveq	#0,d0
	move.w	cs_colorclocks(a0),d0
	D	"ret_hwait d0 = "
	DRW	d0
	subq.w	#4,d0
	and.w	#$fffe,d0

hwait_rts:
	D	"hwait_rts, d0 = "
	DR	d0

	add.w	#hwait_sizeof,sp
	movem.l	(sp)+,a2-a6/d2-d7
	rts


state0:
*		found diwstrt
	D	"state0 hw_bplcon0 = "
	DRW	hw_bplcon0(a3)
	D	"hw_fmode = "
	DRW	hw_fmode(a3)
	D	"hw_dafstop "
	DRW	hw_dafstop(a3)

	move.w	d4,hw_diwstrt(a3)

* calculate the bottom line of the previous ViewPort
	move.w	hw_diwstop(a3),d1
	move.w	d1,d0
	lsr.w	#8,d0
*	move.b	gb_ChipRevBits0(a6),d2
*	and.b	#GFXF_HR_DENISE,d2
*	beq.s	20$
	move.w	hw_diwhigh(a3),d1
	and.w	#$700,d1
*	bra.s	30$
*20$:	not.w	d1
*	and.w	#$8000,d1
*	lsr.w	#7,d1
30$:	add.w	d1,d0
	move.w	d0,hw_bovp(a3)
	move.b	gb_ChipRevBits0(a6),d2
	and.b	#GFXF_HR_DENISE,d2
	bne.s	20$
	move.w	cs_thisbeamy(a0),hw_bovp(a3)	; 'A' bottom of viewport

* calculate the top of the previous ViewPort
20$:
	move.w	d4,d0
	lsr.w	#8,d0
	tst.b	d2
	beq.s	40$
	move.w	hw_diwhigh(a3),d1
	and.w	#7,d1
	lsl.w	#8,d1
	add.w	d1,d0
40$:	move.w	d0,hw_tovp(a3)

*******************************************************************************
*
* We need to find the penultimate colour clock that is available to the copper
* on the line.
*
* definitions:
*    UNSATURATED:    hwait = ((total_colorclocks - 1) & 0xfffe)
*    FULLYSATURATED: hwait = ddfstrt
*
* if ((cs_tovp is on a 256 line boundary and cs_tovp > YWait) ||
*     (cs_bovp <= YWait))
* {
*    if (previous CopList had WHOLE_LINES set (if all instructions fitted on a
*        whole number of lines)
*        return 0
*    else
*        UNSATURATED;
* }
* else
* {
*    // find the penultimate colour clock available to the copper
*    lastcc = UNSATURATED;
*    if (((lastcc & 0x7) >= 2) || //not an even multiple of 8 colour clocks
*        ((ddfstop + granularity) < lastcc)) // display does not go to the rhs
*    {
*        hwait = (lastcc - 2);
*    }
*    else
*    {
*        cycles = (2, 4, 8, 16 or 32 depending on mode);
*        if (bpu == cycles)
*        {
*            FULLYSATURATED
*        }
*        else
*        {
*            // the last fetch cycle is at most 8 cycles long.
*            cycles = (2, 4 or 8 depending on the mode of the last fetch)
* 
*            // (* = available for copper)
*            // 2  1                      2cycle
*            // *  
*            //
*            // 4  2  3  1                4 cycle
*            // *     *
*            //
*            // 8  4  6  2  7  3  5  1    8 cycle
*            // *     *     *     *
*
*            if (bpu <= ((cycles / 2) + 1))
*            {
*                hwait = (lastcc - 2);
*            }
*            else
*            {
*                if (cycles == 4)
*                {
*                    hwait = (lastcc - 4); // bpu == 3 in a 4 cycle mode.
*                }
*                else
*                {
*                    // cycles == 8
*                    if (bpu == 6)
*                    {
*                        hwait = (lastcc - 4)
*                    }
*                    else
*                    {
*                        // must be 7 cycles
*                        hwait = (lastcc - 8);
*                    }
*                }
*            }
*        }
*    }
* }
*
*******************************************************************************

	moveq	#0,d0
	move.w	cs_colorclocks(a0),d0
	subq.w	#1,d0
	and.w	#$fffe,d0		; UNSATURATED

	move.w	cs_origbeamy(a0),d1	; special case: a usercopperlist may
	cmp.b	#$ff,d1			; have started and ended on line 255.
	beq.s	4$
	move.w	hw_tovp(a3),d1
	tst.b	d1			; on a 256 line boundary?
	bne.s	3$
	btst.b	#6,cs_moreflags+1(a0)	; EXACT_LINE?
	beq.s	5$
	moveq	#0,d0
	bra.s	hwait_rts
5$:
	cmp.w	hw_YWait(a3),d1
	bgt.s	hwait_rts

3$:
	move.w	hw_bovp(a3),d1
	cmp.w	hw_YWait(a3),d1
	ble	hwait_rts

4$:
	move.w	d0,d1
	and.w	#$7,d1
	cmp.w	#2,d1
	bge.s	1$
	move.w	hw_dafstop(a3),d1
	add.w	hw_granularity(a3),d1
	cmp.w	d0,d1
	bge.s	2$
1$:
	subq.w	#2,d0
	bra	hwait_rts
2$:
	; calculate the cycle.
	moveq	#2,d1
	move.w	hw_bplcon0(a3),d2
	btst.b	#6,d2		; superhires?
	bne.s	mulfmode
	moveq	#4,d1
	tst.w	d2		; hires?
	bmi.s	mulfmode
	moveq	#8,d1		; lores!
mulfmode:
	move.w	hw_fmode(a3),d3
	beq.s	havecycles	; 1x
	cmp.w	#BANDWIDTH_4X,d3
	bne.s	cyc_2x
	add.w	d1,d1		; 4x
cyc_2x:
	add.w	d1,d1
havecycles:
	moveq	#8,d4		; bpu
	btst.b	#4,d2		; is it 8 bitplanes?
	bne.s	havebpu		; yep
	lsr.w	#8,d2
	lsr.w	#4,d2
	and.w	#$7,d2
	move.w	d2,d4
havebpu:
	cmp.b	#$ff,cs_origbeamy+1(a0)	; WAIT on line 255?
	bne.s	6$

	move.l	d0,-(sp)

	; d1 = cycle, d4 = bpu
	; At this point, we need to calculate the number of cycles available
	; to the copper in the middle of the line, in case there are more
	; copper instructions in the line than there are copper cycles.
	;
	; TotalCopperCycle = (Cycles outside displaywindow +
	;                     Cycles inside displaywindow).
	; CCOutside = (((ddfstrt - 2) / 2) + (((totclks & 0xfff8) - ddfstop) / 2))
	; offset = 0; Acycles = MIN(cycles, 8);
	; if (cycles == 16) offset = 4;
	; if (cycles == 32) offset = 12;
	; CCInside = (((bpu <= (Acycles / 2)) ? ((Acycles / 2) + offset)
	;                                   : ((Acycles - bpu) + offset))
	;             * (ddfstop - ddfstrt))
	; Therefore, copperinstructions per line = (TotalCopperCycles / 2)
	;
	; The copper instrutcions in this CopList is cl->cl_Count
	; SHIT!

	move.w	hw_dafstrt(a3),d7
	subq.w	#2,d7
	move.w	cs_colorclocks(a0),d6
	and.w	#$fff8,d6
	sub.w	hw_dafstop(a3),d6
	subq.w	#8,d6			; last fetch is an 8 cycle
	sub.w	d6,d7
	asr.w	#1,d7			; CCOutside = d7

	moveq	#0,d5			; offset = d5
	move.w	d1,d2
	cmp.w	#8,d2
	ble.s	7$
	moveq	#8,d2			; Acycles = d2
	moveq	#4,d5			; assume 16 cycle
	cmp.w	#16,d1
	beq.s	7$
	moveq	#12,d5			; nope, 32 cycles
7$:
	move.w	d2,d3
	asr.w	#1,d3			; (Acycles / 2) = d3
	cmp.w	d4,d3
	bgt.s	8$
	sub.w	d4,d2
	move.w	d2,d3
8$:
	add.w	d3,d5
	move.w	hw_dafstop(a3),d6
	sub.w	hw_dafstrt(a3),d6
	mulu	d6,d5			; d5 = CCInside
	divu	d1,d5			; CCInside per cycle
	add.w	d5,d7			; d7 = copper cycles for the line.
	asr.w	#1,d7			; d7 = copper instructions for the line.

	; count the instructions to the previous WAIT
	move.l	cs_copptr(a0),a4
	moveq	#-1,d2
	move.l	#$10001,d5
	move.l	#$10000,d3
9$:
	addq.w	#1,d2
	move.l	-(a4),d6
	and.l	d5,d6
	cmp.l	d3,d6
	bne.s	9$
	move.l	(sp)+,d0
	cmp.w	d2,d7
	bgt.s	6$			; if less copins on the line than cycles?
	moveq	#-1,d0			; signal an error, 
	bra	hwait_rts

6$:
	cmp.w	d4,d1		; bpu == cycles?
	bne.s	1$
	move.w	hw_dafstrt(a3),d0	; FULLYSATURATED
	bra	hwait_rts
1$:
	; how many cycles in the last fetch cycle?
	cmp.w	#8,d1
	ble.s	2$
	moveq	#8,d1
2$:
	move.w	d1,d2
	asr.w	#1,d2
	addq.w	#1,d2
	cmp.w	d2,d4		; bpu <= ((cycles / 2) + 1)?
	bgt.s	3$
	subq.w	#2,d0
	bra	hwait_rts
3$:
	cmp.w	#4,d1
	bne.s	4$
	sub.w	d1,d0
4$:
	cmp.w	#6,d4
	beq.s	5$
	subq.w	#4,d0		; must be 7 bitplanes
5$:
	subq.w	#4,d0
	bra	hwait_rts


state1:
*		found diwhigh
	D	"state1 "
	DRW	d4
	move.w	d4,hw_diwhigh(a3)
	move.l	#_custom+ddfstop,d6
	and.l	#$1FE,d6
	moveq	#STATE2,d7
	bra	while_loop

state2:
*		found dafstop
	D	"state2 "
	DRW	d4
	move.w	d4,hw_dafstop(a3)
	move.l	#_custom+ddfstrt,d6
	and.l	#$1FE,d6
	moveq	#STATE3,d7
	bra	while_loop

state3:
*		found dafstart
	D	"state3 "
	DRW	d4
	move.w	d4,hw_dafstrt(a3)
	move.l	#_custom+diwstop,d6
	and.l	#$1FE,d6
	moveq	#STATE4,d7
	bra	while_loop

state4:
*		found diwstop
	D	"state4 "
	DRW	d4
	move.w	d4,hw_diwstop(a3)
	move.l	#_custom+bplcon0,d6
	and.l	#$1FE,d6
	moveq	#STATE5,d7
	bra	while_loop

state5:
*		found bplcon0
	D	"state5 "
	DRW	d4
	move.b	gb_ChipRevBits0(a6),d0
	and.b	#GFXF_HR_DENISE,d0
	if =
		move.w	hw_oldbplcon0(a3),hw_bplcon0(a3)
	else
		move.w	d4,hw_bplcon0(a3)
	endif

	moveq	#GRANULARITY,d1			; assume this will be the Granularity.
	move.w	d1,hw_granularity(a3)
	subq.w	#1,d1
	move.w	d1,hw_granularity1(a3)

	move.l	#_custom+diwstrt,d6
	and.l	#$1FE,d6
	moveq	#STATE0,d7
	bra	while_loop

state6:
*		found 'old denise' bplcon0
	D	"state6 "
	DRW	d4
	move.w	d4,hw_oldbplcon0(a3)
	move.l	#_custom+ddfstop,d6
	and.l	#$1FE,d6
	moveq	#STATE2,d7
	bra	while_loop

state7:
*		found fmode
	D	"state7 "
	DRW	d4
	and.w	#$3,d4
	move.w	d4,hw_fmode(a3)
	move.l	#_custom+diwhigh,d6
	and.l	#$1FE,d6
	moveq	#STATE1,d7
	bra	while_loop

state_table:
	jmp	state0
	jmp	state1
	jmp	state2
	jmp	state3
	jmp	state4
	jmp	state5
	jmp	state6
	jmp	state7

	ENDC
* }

	end
