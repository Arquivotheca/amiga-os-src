*******************************************************************************
*
*	$Id: calcivg.asm,v 39.24 93/08/11 15:28:53 spence Exp $
*
*******************************************************************************

	SECTION	graphics

V1_3	EQU	1		; for copper.i

	include "/copper.i"
	include "/displayinfo.i"
	include "/gfxbase.i"
	include "/view.i"
	include "/displayinfo_internal.i"
	include	"/macros.i"
	include "exec/memory.i"
	include "exec/macros.i"

	xref	_LVOGfxLookUp
	xref	_LVOGetVPModeID
	xref	_LVOGetDisplayInfoData
	xref	_LVOAllocMem
	xref	_LVOFreeMem
	xref	_new_mode
	xref	_CleanMode
	xref	_GfxBase
	xdef	_CalcIVG
	xdef	_CalcFMode

COMPATIBILITY_KLUDGE	EQU	1

*DODEBUG	EQU	1
*GBDEBUG	EQU	1

	IFD	DODEBUG
D	MACRO
	IFD	GBDEBUG
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	ENDC
	print	\1
dbg\@
	ENDM	; D	MACRO
DR	MACRO
	IFD	GBDEBUG
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	ENDC
	dbug	\1
dbg\@
	ENDM	; DR	MACRO
DRW	MACRO
	IFD	GBDEBUG
	tst.b	gb_Debug(a6)
	beq.s	dbg\@
	ENDC
	dbugw	\1
dbg\@
	ENDM	; DRW	MACRO

	ELSE

D	MACRO
	ENDM
DR	MACRO
	ENDM
DRW	MACRO
	ENDM

	ENDC	; IFD	DODEBUG

*****i* graphics.library/CalcFMode ********************************************
*
*   NAME
*	CalcFMode -- Calculate the max FMode value to use in this ViewPort (V39)
*
*   SYNOPSIS
*	fmode = CalcFMode(ViewPort)
*	 d0.w              a0
*
*	WORD CalcFMode(struct ViewPort *);
*
*   FUNCTION
*	To calculate the maximum FMode (or Bandwidth) that this ViewPort can
*	use, considering the machine's memory type, the ViewPort mode, the
*	number of bitplanes used, and the starting addresses of all the
*	bitplanes.
*
*   INPUTS
*	ViewPort - pointer to this ViewPort
*
*   RESULT
*	fmode - fmode value that this ViewPort should use to maximise
*	        Bandwidth usage, or -1 if the requested mode is not possible
*	        with the machine's memory type or could not find the BitMap.
*
*******************************************************************************

; WORD __asm CalcFMode(register __a0 struct ViewPort *vp);

PHIRES_2x	equ	4
PSHIRES_2x	equ	2
PSHIRES_4x	equ	4

CalcFMErr:
	moveq	#-1,d0
	bra	CalcFM.
 
_CalcFMode:
	movem.l	a2-a3/d2-d6,-(sp)
	move.l	a0,d0
	beq.s	CalcFMErr
	move.l	a0,-(sp)
	jsr	_new_mode
	move.l	d0,d2
	move.l	(sp)+,a0
	move.l	vp_RasInfo(a0),d0
	beq.s	CalcFMErr
	move.l	d0,a1
	move.l	ri_BitMap(a1),d0
	beq.s	CalcFMErr
	move.l	d0,a2
	moveq	#0,d5
	btst	#10,d2	; dualpf?
	beq.s	cfm_nodpf
	move.l	ri_Next(a1),d5
	beq.s	cfm_nodpf
	move.l	d5,a3
	move.l	ri_BitMap(a3),d5
	beq.s	cfm_nodpf
	move.l	d5,a3
	moveq	#0,d5
	move.b	bm_Depth(a3),d5

* a0 -> ViewPort
* a1 -> RasInfo
* a2 -> BitMap
* a3 -> second BitMap
* d2.l = ViewPort mode
* d5.b = flag for dualpf (if not 0, then = Depth of second playfield)

* Find the minimum bandwidth needed for this mode.
* Use d3 for the shift value, d4 for BitMap depth.

cfm_nodpf:
	move.b	bm_Depth(a2),d4
* If DualPlayfield, sum the depths.
	add.b	d5,d4
cfm_sglpfd:
	moveq	#0,d3
	move.l	d2,d0
	and.l	#V_HIRES,d0
	beq.s	cfm_nothires
	cmp.b	#PHIRES_2x,d4
	bls.s	cfm_haveshift
	addq.w	#1,d3
	bra.s	cfm_haveshift

cfm_nothires:
	move.w	d2,d0
	and.w	#V_SUPERHIRES,d0
	beq.s	cfm_haveshift		; must be LORES
	cmp.b	#PSHIRES_2x,d4
	bls.s	cfm_haveshift
	addq.w	#1,d3
	cmp.b	#PSHIRES_4x,d4
	bls.s	cfm_haveshift
	addq.w	#1,d3

cfm_haveshift:
* Can the machine handle the bandwidth needed?
* If shift = 0, then only need 1x, so all's OK
* If shift = 1, then need 2x, so GB->MemType must = at least 1
* If shift = 2, then need 4x, so GB->MemType must = 3

	move.b	gb_MemType(a6),d2
	tst.b	d3
	beq.s	cfm_havebw
	cmp.w	#2,d3
	beq.s	cfm_need4x
	tst.b	d2
	beq	CalcFMErr		; Too bad. Have 1x, needed 2x
	bra.s	cfm_havebw
cfm_need4x:
	cmp.w	#BANDWIDTH_4X,d2
	blt	CalcFMErr

cfm_havebw:
	addq.w	#1,d3
	moveq	#-1,d0
	asl.w	d3,d0			; d0 has a mask for checking bitplane
	not.w	d0			; pointer values.

* Now, OR all the bitplane pointers together, and mask them for boundary checking.
* Also, check that the bytes-per-row fields are also aligned correctly.
* reuse d4 and a1

cfm_checkplanes:
	move.w	bm_BytesPerRow(a2),d6
	moveq	#0,d1
	lea	bm_Planes(a2),a1
	moveq	#0,d4
	move.b	bm_Depth(a2),d4
	D	"cfm_checkplanes bpr, depth "
	DRW	d6
	DRW	d4
	D	"Planes are: "
	bra.s	cfm_ckpl1.
cfm_ckpl1:
	DR	(a1)
	or.l	(a1)+,d1
	DR	d1
cfm_ckpl1.:
	dbra	d4,cfm_ckpl1
	tst.b	d5
	beq.s	cfm_noordpf

	or.w	bm_BytesPerRow(a3),d6
	D	"2nd playfield, bpr, depth "
	DRW	d6
	DRW	d5
	lea	bm_Planes(a3),a1
	bra.s	cfm_ckpl2.
cfm_ckpl2:
	DR	(a1)
	or.l	(a1)+,d1
	DR	d1
cfm_ckpl2.:
	dbra	d5,cfm_ckpl2

cfm_noordpf:
	D	"done. bplptrs, bprs "
	DR	d1
	DRW	d6
	D	"Mask "
	DRW	d0
	or.w	d6,d1			; check the bytes-per-row at the same time
	move.w	d1,d4
	and.w	d0,d1
	bne	CalcFMErr		; Too Bad. The bitplanes are misaligned.

* Now, we know the machine can provide the minimum bandwidth needed, but can we
* get any better?

cfm_getbest:
	cmp.w	#3,d3
	beq.s	CalcedFM		; max reached.
	asl.w	#1,d0
	or.w	#1,d0
	move.w	d4,d1
	and.w	d0,d1
	bne.s	CalcedFM		; can't do any better
	addq.w	#1,d3
	bra.s	cfm_getbest

CalcedFM:
* d3 has the solution.
* If d3 = 1, return 0 (1x)
* If d3 = 2, return 0, 1 or 2 depending on Chip type
* If d3 = 3, return 0 - 3 depending on Chip type

	subq.w	#1,d3
	beq.s	CalcedFM.
	move.b	gb_MemType(a6),d0
	cmp.w	#2,d3
	bne.s	1$
2$:
	move.b	d0,d3
	bra.s	CalcedFM.
1$:
	cmp.b	#BANDWIDTH_4X,d0
	bne.s	2$
	moveq	#BANDWIDTH_2XNML,d3

CalcedFM.:
	and.l	#$ff,d3
	move.l	d3,d0

CalcFM.:
	D	"fmode = "
	DR	d0
	movem.l	(sp)+,a2-a3/d2-d6
	rts

******* graphics.library/CalcIVG **********************************************
*
*   NAME
*	CalcIVG -- Calculate the number of blank lines above a ViewPort (V39)
*
*   SYNOPSIS
*	count = CalcIVG(View, ViewPort)
*	 d0.w           a0    a1
*
*	UWORD CalcIVG(struct View *, struct ViewPort *);
*
*   FUNCTION
*	To calculate the maximum number of blank lines above a viewport needed to
*	load all the copper instructions, after accounting for the viewport
*	bandwidth and size.
*
*   INPUTS
*	View       - pointer to the View
*	ViewPort   - pointer to the ViewPort you are interested in.
*
*   RESULT
*	count      - the number of ViewPort resolution scan lines needed to
*	             execute all the copper instructions for ViewPort,
*	             or 0 if any error. 
*
*   NOTES
*	The number of copper instructions comes from the vp->vp_DspIns list.
*	Although there may be other copper instructions in the final list (from
*	UCopIns, SprIns and ClrIns) they are currently ignored for this
*	function. This also means that if the ViewPort has never been made
*	(for example, the ViewPort of an intuition screen was opened behind)
*	then vp->vp_DspIns is NULL.
*
*	Although CalcIVG() returns the true number of lines needed by the
*	copper, intuition still maintains an inter-screen gap of 3 non-laced
*	lines (6 interlaced). Therefore, for intuition screens use:
*	MAX(CalcIVG(v, vp), (islaced ? 6 : 3))
*
*
*   SEE ALSO
*	GfxNew()  VideoControl()  graphics/view.h
*
*******************************************************************************
*
* How to calculate the Inter-ViewPort Gap:
*
* The copper can only use the DMA bus every other ColourClock.
* MOVE and SKIP instructions need 2 fetches, ie 4 colourclocks.
* WAIT needs 2 fetches (4 colourclocks) and another 2 colourclocks to wake up.
*
* Therefore, the total number of ColourClocks needed is:
*
* ( (Total number of copper instructions * 4) + 
*   (Total number of WAIT instructions * 2) )
*
* The number of copper instructions that can be executed per scanline depend on
* 1) The total number of ColourClocks on the line
* 2) The width of the display window if the display window is open.
* 3) The number of bitplanes turned on in the IVG.
* 4) The bandwidth (FMode) we are using.
* 5) The display mode (LORES, HIRES, SUPERHIRES)
*
* The total number of colourclocks per line is in MonitorInfo->TotalColorClocks
*
* (FOR OLD IMPLEMENTATION, NOW REMOVED BUT UNDER RCS:
* If the display window is left open, use the look-up tables to read the number
* of copper cycles for the relevant BitPlaneUsage and Bandwidth. Calculate the
* number of copper cycles available before the window opens
* from (View->DxOffset + ViewPort->DxOffset), and the number available after the
* window closes (View->DxOffset + ViewPort->DxOffset + ViewPort->DWidth).
* Subtract that total from the TotalColorClocks.)
*
* Therefore, the total number of lines needed = 
*
* 	 Total number of ColourClocks needed
* 	-------------------------------------
* 	Total number of ColourClocks per line
*
* (The total number of CopperInstructions per line is the total number of
* ColourClocks per line, divided by 2).
*
*******************************************************************************

; UWORD __asm CalcIVG(register __a0 struct View *v, register __a1 struct ViewPort *vp);

_CalcIVG:
	movem.l	a2-a4/d2/d4-d7,-(sp)

* d6.w	= fmode		(default = 1x bandwidth)
* d4.w	= TotalColorClocks per blank line
* d3.w	= TotalColorClocks per line with bpu
* d2.l	= vpmodeid
* d7.(31-16)	= PixelSpeed
* a2	-> View
* a3	-> ViewPort
* a4	-> MonitorInfo

	moveq	#0,d7
	move.w	d7,d6
	move.w	d7,d4
	move.l	a0,a2		; safe keeping
	move.l	a1,a3

	sub.l	#mtr_SIZEOF,sp
	move.l	sp,a4

* Now look for a ViewPortExtra
	move.l	a3,a0
	jsr	_LVOGetVPModeID(a6)
	cmp.l	#INVALID_ID,d0
	bne.s	toad
	move.l	a3,-(sp)
	jsr	_new_mode
	move.l	d0,(sp)
	jsr	_CleanMode
	addq.l	#4,sp
toad:
	move.l	d0,d2
	move.l	vp_ColorMap(a3),d0
	beq.s	not_in_diw
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	not_in_diw
	move.l	cm_CoerceDisplayInfo(a0),d0
	bne.s	calc_coerced
	move.l	cm_NormalDisplayInfo(a0),d0
	beq.s	calc_nocoerce
calc_coerced:
	move.l	d0,a1		; DisplayInfoRecord
	move.w	rec_MajorKey(a1),d2
	swap	d2
	move.w	rec_MinorKey(a1),d2	; the coerced ModeID
calc_nocoerce:
	D	"ModeID = "
	DR	d2
	move.l	cm_vpe(a0),d0
	beq.s	not_in_diw
	move.l	d0,a1	

	move.l	a1,-(sp)
	move.l	a3,a0
	bsr	_CalcFMode
	move.l	(sp)+,a1
	D	"FMode = "
	DRW	d0
	tst.w	d0
	bmi.s	CalcIVGErr
	move.w	d0,d6
not_in_diw:
	bsr.s	calc_cc_perblankline

have_cc_per_line:

* OK - now how many copper instructions do we have for this ViewPort?
* Only look at DspIns. SprIns, ClrIns and UCopIns, may have copper 
* instructions before the ViewPort opens, but these are dynamic copper
* instructions. 

	moveq	#0,d0
	move.l	vp_DspIns(a3),d7
	beq.s	_CalcIVG.1
2$:
	move.l	d7,a0
	add.w	cl_Count(a0),d0
	move.l	cl_Next(a0),d7
	bne.s	2$
	sub.w	cl_SLRepeat(a0),d0	; subtract the number of instructions
					; repeated for Short and Long Frames.
* total CopInstructions is in d0.w
	D	"CopInstructions = "
	DR	d0
3$:
	asl.l	#1,d0		; total number of CopperCycles needed (without extras for WAIT)
	divu	d4,d0
	D	"After Division d0.l = "
	DR	d0
bollox:
* if there is a remainder, round the result up to the next whole value
	cmp.l	#$ffff,d0
	ble.s	no_rounding
	addq.w	#1,d0
	bra.s	chk_kludge
no_rounding:
	or.w	#EXACT_LINE,cl_PrivateFlags(a0)	; show an exact whole line
						; (cl_PrivateFlags is only cleared in copinit())
chk_kludge:
	IFNE	COMPATIBILITY_KLUDGE
	; Under V37, there was always at least 2 lines in the IVG for ECS.
	btst.b	#GFXB_AA_ALICE,gb_ChipRevBits0(a6)
	bne.s	chk_lace
	cmp.w	#2,d0
	bge.s	chk_lace
	moveq	#2,d0
	ENDC

chk_lace:
* double the result if the mode is laced.
	btst.b	#0,dis_PropertyFlags+3(a4)	; DIPF_IS_LACE
	beq.s	wanker
	D	"laced "
	asl.w	#1,d0
	bra.s	_CalcIVG.

* Halve the result if scan doubled

wanker:
	btst.b	#1,dis_PropertyFlags+1(a4)	; DIPF_IS_SCANDBL
	beq.s	_CalcIVG.
	D	"Scan Doubled "
	move.w	d0,d1
	and.w	#1,d1
	add.w	d1,d1
	or.w	d1,cl_PrivateFlags(a0)		; HALF_LINE
	addq.w	#1,d0
	asr.w	#1,d0

_CalcIVG.:
	move.l	d0,d7
_CalcIVG.1:
	add.l	#mtr_SIZEOF,sp
	moveq	#0,d0
	move.w	d7,d0

_CalcIVGrts:
	D	"CalcIVG returns "
	DRW	d0
	movem.l	(sp)+,a2-a4/d2/d4-d7
	rts

CalcIVGErr:
	moveq	#0,d7
	bra.s	_CalcIVG.1

calc_cc_perblankline:
	move.l	a4,a1		; buffer
	sub.l	a0,a0		; handle
	moveq	#mtr_SIZEOF,d0	; for a MonitorInfo
	move.l	#DTAG_MNTR,d1	; tagID
	jsr	_LVOGetDisplayInfoData(a6)	; (d2 has modeid)
	move.w	mtr_TotalColorClocks(a4),d4
*	addq.w	#1,d4
	asr.w	#1,d4		; The number of CopperCycles available is half the ColorClocks
	D	"Copper Cycles = "
	DRW	d4
	move.l	a4,a1		; buffer
	sub.l	a0,a0		; handle
	moveq	#dis_PixelSpeed+2,d0
	move.l	#DTAG_DISP,d1	; tagID
	jsr	_LVOGetDisplayInfoData(a6)	; (d2 has modeid)
	
cc_pbl.:
	rts

	end
