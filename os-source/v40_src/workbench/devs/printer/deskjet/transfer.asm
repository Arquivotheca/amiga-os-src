**********************************************************************
*
* Transfer routine for HP_500C and HP_Deskjet
* Color portion based on Howtek_Pixelmaster
* B&W portion based on HP_DeskJet
*
* C. Scheppner 12/91
* Threshold based color gamma correction (CAS) (and commented out black stuff)
* Hardcoded table for grayscale brightness correction (CAS)
* Threshold of 1 means NO CORRECTION
*
* Based on David Berezowski - April/88
*
**********************************************************************

	INCLUDE "exec/types.i"

	INCLUDE	"intuition/intuition.i"
	INCLUDE "devices/printer.i"
	INCLUDE "devices/prtbase.i"
	INCLUDE "devices/prtgfx.i"

	XREF	_PD
	XREF	_LVODebug
	XREF	_AbsExecBase

	XDEF	_Transfer


MYDEBUG	SET	0

	IFGT	MYDEBUG

	XREF	KPrintF


* DBUG macro for format string and five variables
*	 preserves all registers   
*        outputs to serial port   link with amiga.lib,debug.lib
* Usage: pass name of format string,value,value,value,value,value
*        values may be register name, variablename(PC), or #value
*        

DBUG	MACRO	* passed name of format string, with args on stack
	MOVEM.L	d0-d7/a0-a6,-(sp)
	MOVE.L  \6,-(sp)
	MOVE.L  \5,-(sp)
	MOVE.L  \4,-(sp)
	MOVE.L  \3,-(sp)
	MOVE.L  \2,-(sp)
	LEA.L	\1(PC),a0
	LEA.L   (sp),a1
	JSR	KPrintF
	ADDA.L	#20,sp
	MOVEM.L	(sp)+,d0-d7/a0-a6	
	ENDM
	ENDC

	IFEQ	MYDEBUG
DBUG	MACRO
* disabled debug macro
	ENDM
	ENDC


_Transfer:
; Transfer(PInfo, y, ptr, colors)
; struct PrtInfo *PInfo		4-7
; UWORD y;			8-11
; UBYTE *ptr;			12-15
; UWORD *colors;		16-19
;

* Warning - if you change this line, you must change
* both the exit: and ncexit: lines
	movem.l	d2-d7/a2-a6,-(sp)	;save regs used

	movea.l	_PD,a3			;a3 = ptr to PD
	cmpi.w	#SHADE_COLOR,pd_Preferences+pf_PrintShade(a3)	;color dump?
	bne	not_color		;no

	movea.l	48(sp),a0		;a0 = PInfo
	move.l	52(sp),d0		;d0 = y
	movea.l	56(sp),a1		;a1 = ptr
	movea.l	60(sp),a2		;a2 = colors

color:
; a0 - PInfo
; a1 - ptr
; a2 - colors
; d0 - y

	movea.l	a1,a4
	adda.w	(a2)+,a4		;a4 = ptr + colors[0] (cptr)
	movea.l	a1,a5
	adda.w	(a2)+,a5		;a5 = ptr + colors[1] (mptr)
	movea.l	a1,a6
	adda.w	(a2)+,a6		;a6 = ptr + colors[2] (yptr)

;	move.l	a6,-(sp)
;	move.l	_AbsExecBase,a6
;	jsr	_LVODebug(a6)
;	move.l	(sp)+,a6

	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_width(a0),width	;# of pixels to do
	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	lsl.w	#2,d2			;d2 = (y & 3) << 2
	movea.l	pi_dmatrix(a0),a3	;a3 = dmatrix
	adda.l	d2,a3			;a3 = dmatrix + ((y & 3) << 2)
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

	moveq	#0,d1
	movea.l	_PD,a1			;a1 = ptr to PD
	move.w	pd_Preferences+pf_PrintThreshold(a1),d1	; threshold (gamma)

	DBUG	THRES,d1,d0,#0,#0,#0

	lsl.l	#4,d1	
	lea.l	gammas(PC),a1
	adda.l	d1,a1

	DBUG	GAMMA,0(a1),4(a1),8(a1),12(a1),#0

	moveq	#0,d1
	moveq	#0,d4
	moveq	#0,d5
	moveq	#0,d6

; a0 - sxptr
; a1 - ptr to gammas
; a2 - ColorInt ptr
; a3 - dmatrix ptr
; a4 - cptr
; a5 - mptr
; a6 - yptr
; d0 - byte to set (x >> 3)
; d1 - Black
; d2 - x
; d3 - dvalue (dmatrix[x & 3])
; d4 - Cyan
; d5 - Magenta
; d6 - Yellow
; d7 - bit to clear

cwidth_loop:

	move.b	PCMCYAN(a2),d4		;d4 = Cyan
	move.b	PCMMAGENTA(a2),d5	;d5 = Magenta
	move.b	PCMYELLOW(a2),d6	;d6 = Yellow

	DBUG	BYMC1,d2,d1,d6,d5,d4

* try at gamma/linear correction

	move.b  32(a1,d4.b),d4		;d4 = Cyan
	move.b  0(a1,d5.b),d5		;d5 = Magenta
	move.b  0(a1,d6.b),d6		;d6 = Yellow

	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,sx		;# of times to use this pixel

csx_loop:
	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to clear
	move.b	d2,d7
	not.b	d7			;compute bit to clear

	moveq.l	#3,d3
	and.w	d2,d3			;d3 = x & 3
	move.b	0(a3,d3.w),d3		;d3 = dmatrix[x & 3]

	DBUG	BYMC2,d3,d1,d6,d5,d4

cyan:
	cmp.b	d3,d4			;render cyan pixel?
	ble.s	magenta			;no.
	bset.b	d7,0(a4,d0.w)		;clear cyan pixel

magenta:
	cmp.b	d3,d5			;render magenta pixel?
	ble.s	yellow			;no.
	bset.b	d7,0(a5,d0.w)		;clear magenta pixel

yellow:
	cmp.b	d3,d6			;render yellow pixel?
	ble.s	csx_end			;no, skip to next pixel.
	bset.b	d7,0(a6,d0.w)		;clear yellow pixel

csx_end
	addq.w	#1,d2			;x++
	subq.w	#1,sx			;sx--
	bne.s	csx_loop
	subq.w	#1,width		;width--
	bne	cwidth_loop

exit:
	movem.l	(sp)+,d2-d7/a2-a6	;restore regs all used
	moveq.l	#0,d0			;flag all ok
	rts				;goodbye

**********************************************************************
*
* B&W and GreyScale Transfer routine for HP_500C and HP_Deskjet
*
* Based on David Berezowski - Mar/88
*
**********************************************************************

not_color:
; Transfer(PInfo, y, ptr, [colors])
; struct PrtInfo *PInfo		4-7
; UWORD y;			8-11
; UBYTE *ptr;			12-15
;

* Register saves were done at the _Transfer entry
*	movem.l	d2-d7/a2-a6,-(sp)	;save regs used

	movea.l	48(sp),a0		;a0 = PInfo
	move.l	52(sp),d0		;d0 = y
	movea.l	56(sp),a1		;a1 = ptr

	move.w	pi_width(a0),d1		;d1 = width
	subq.w	#1,d1			;adjust for dbra

	move.w	pi_threshold(a0),d3	;d3 = threshold, thresholding?
	beq.s	grey_scale		;no, grey-scale

threshold:
; a0 - PInfo
; a1 - ptr
; d0 - y
; d1 - width
; d3 - threshold

	eori.b	#15,d3			;d3 = dvalue
	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr (NOT USED)
; d0 - byte to set (x >> 3)
; d1 - width
; d2 - x
; d3 - dvalue
; d4 - Black
; d5 - sx
; d6 - bit to set

twidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel (sx)

	cmp.b	d3,d4			;render this pixel?
	ble.s	tsx_end			;no, skip to next pixel.
	subq.w	#1,d5			;adjust for dbra

tsx_render:				;yes, render this pixel sx times
	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.w	d2,d6
	not.w	d6			;compute bit to set
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ x

	addq.w	#1,d2			;x++
	dbra	d5,tsx_render		;sx--
	dbra	d1,twidth_loop		;width--
	bra	ncexit			;all done
	
tsx_end:
	add.w	d5,d2			;x += sx
	dbra	d1,twidth_loop		;width--
	bra	ncexit

grey_scale:
; a0 - PInfo
; a1 - ptr
; d0 - y
; d1 - width

	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	lsl.w	#2,d2			;d2 = (y & 3) << 2
	movea.l	pi_dmatrix(a0),a3	;a3 = dmatrix
	adda.l	d2,a3			;a3 = dmatrix + ((y & 3) << 2)
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)


* Note - commeted out code used variable grayscale correction
* Current code (which follows it) uses one hardcoded grayscale correction
* unless threshold == 1, in which case NO correction is applied.
* Additional commented out code follows the register comments

*	moveq	#0,d7
*	movea.l	_PD,a4			;a4 = ptr to PD
*	move.w	pd_Preferences+pf_PrintThreshold(a4),d7	; threshold (gamma)
*	DBUG	THRES,d7,d0,#0,#0,#0
*	lsl.l	#4,d7	
*	lea.l	gammas(PC),a4
*	adda.l	d7,a4

	moveq	#0,d7
	movea.l	_PD,a4			;a4 = ptr to PD
	move.w	pd_Preferences+pf_PrintThreshold(a4),d7	; threshold
	lea.l	greysame(PC),a4		;a4 = no correction grey table
	cmpi.w	#1,d7			;if thres==1, no correction
	beq.s	donecorrect
	lea.l	greycorrect(PC),a4	;a4 = brightness correction grey table
donecorrect
	DBUG	THRES,d7,d0,#0,#0,#0
	DBUG	GAMMA,0(a4),4(a4),8(a4),12(a4),#0


; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr
; a4 - gammas
; d0 - byte to set (x >> 3)
; d1 - width
; d2 - x
; d3 - dvalue (dmatrix[x & 3])
; d4 - Black
; d5 - sx
; d6 - bit to set

gwidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black

	DBUG	GRAY1,d2,d4,#0,#0,#0

* gamma correct

* This was correct for using the threshold based color correction tables
*	move.b  16(a4,d4.b),d4		;gamma correct

* This is correct for single graytab correction table (preserves blacks)
	move.b  0(a4,d4.b),d4		;gamma correct

	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel (sx)
	subq.w	#1,d5			;adjust for dbra

gsx_loop:
	moveq.l	#3,d3
	and.w	d2,d3			;d3 = x & 3	
	move.b	0(a3,d3.w),d3		;d3 = dmatrix[x & 3]

	DBUG	GRAY2,d3,d4,#0,#0,#0

	cmp.b	d3,d4			;render this pixel?
	ble.s	gsx_end			;no, skip to next pixel.

	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.w	d2,d6
	not.w	d6			;compute bit to set
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ x

gsx_end
	addq.w	#1,d2			;x++
	dbra	d5,gsx_loop		;sx--
	dbra	d1,gwidth_loop		;width--

ncexit:
	movem.l	(sp)+,d2-d7/a2-a6	;restore regs used
	moveq.l	#0,d0			;flag all ok
	rts				;goodbye


**** DATA
sx	dc.w	0
width	dc.w	0


* Lightening for grayscale printouts
greysame:
* no correction if threshold = 1
	DC.B 	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15	
greycorrect:
* hard-coded correction otherwise
	DC.B    0,1,1,2,3,3,4,5,5,6,7,8,9,11,13,15

* Lightening for color printouts
* Threshold setting = table used
* (note that CYAN correction always uses 2 tables past selected threshold

* These extra tables are just here for possible filezap backing off...
* extra table - no correction
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

gammas:

* 0 = no correction  (we should never receive this value)
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

* 1 = no correction (Threshold 1 means NO CORRECTION on any color)
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 2 = no correction (but CYAN gets corrected with table 4)
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 3 = no correction (but CYAN gets corrected with table 5)
	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

* 4 = enhanced linear -1 (CYAN uses table 5, etc.)
	DC.B 0,1,2,3,4,4,5,6,7,8,9,10,11,12,13,14
* 5 = linear -1
	DC.B 0,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14

* 6 = enhanced linear -2
	DC.B 0,1,2,2,3,3,4,5,6,7,8,9,10,11,12,13
* 7 = linear -2
	DC.B 0,1,1,2,2,3,4,5,6,7,8,9,10,11,12,13

* 8 = enhanced linear -3
	DC.B 0,1,1,2,2,3,3,4,5,6,7,8,9,10,11,12
* 9 = linear -3
	DC.B 0,1,1,1,2,2,3,4,5,6,7,8,9,10,11,12

* 10= enhanced linear -4
	DC.B 0,1,1,2,2,3,4,5,5,6,6,7,8,9,10,11
* 11= linear -4
	DC.B 0,1,1,1,2,2,3,3,4,5,6,7,8,9,10,11

* 12= enhanced linear -5
	DC.B 0,1,1,2,2,3,3,4,5,5,5,6,7,8,9,10
* 13= linear -5
	DC.B 0,1,1,1,2,2,2,3,3,4,5,6,7,8,9,10

* 14= enhanced linear -6
	DC.B 0,1,1,1,2,2,3,3,3,4,4,5,6,7,8,9
* 15= linear -6
	DC.B 0,1,1,1,1,2,2,2,3,3,4,5,6,7,8,9

* 16= enhanced linear -7 (for CYAN when threshold is 14)
	DC.B 0,1,1,1,2,2,2,3,3,3,4,4,5,6,7,8
* 17= linear -7 (for CYAN when threshold is 15)
	DC.B 0,1,1,1,1,2,2,2,3,3,3,4,5,6,7,8

* 18= enhanced linear -8 (not used)
	DC.B 0,1,1,1,2,2,2,3,3,3,4,4,4,5,6,7


* previous test gammas (gammas generated with HP's example code)
* linear no correction (extra table for backing off correction)
*	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 0 = linear no correction
*	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 1 = gamma 1.1
*	DC.B 0,1,2,3,4,4,5,6,8,9,10,11,12,13,14,15
* 2 = gamma 1.2
*	DC.B 0,1,1,2,3,4,5,6,7,8,9,10,11,13,14,15
* 3 = gamma 1.3
*	DC.B 0,0,1,2,3,4,5,6,7,8,9,10,11,12,14,15
* 4 = gamma 1.4
*	DC.B 0,0,1,2,2,3,4,5,6,7,9,10,11,12,14,15
* 5 = gamma 1.5
*	DC.B 0,0,1,1,2,3,4,5,6,7,8,9,11,12,14,15
* 6 = gamma 1.6
*	DC.B 0,0,1,1,2,3,3,4,5,7,8,9,10,12,13,15
* 7 = gamma 1.7
*	DC.B 0,0,0,1,2,2,3,4,5,6,8,9,10,12,13,15
* 8 = 0 correct
*	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 9 = linear -1
*	DC.B 0,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14
* 10= linear -2
*	DC.B 0,1,1,2,2,3,4,5,6,7,8,9,10,11,12,13
* 11= linear -3
*	DC.B 0,1,1,1,2,2,3,4,5,6,7,8,9,10,11,12
* 12= linear -4
*	DC.B 0,1,1,1,2,2,3,3,4,5,6,7,8,9,10,11
* 13= linear -5
*	DC.B 0,1,1,1,2,2,2,3,3,4,5,6,7,8,9,10
* 14= linear -6
*	DC.B 0,1,1,1,1,2,2,2,3,3,4,5,6,7,8,9
* 15 = 0 correct
*	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
* 0 correct - extra table for if pushed up
*	DC.B 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

* real old test gammas
* 1.5 correct on 16 values
*	DC.B 0,0,1,1,2,3,4,5,6,7,8,9,11,12,14,15 
* linear correction 1 (lightening) ...too dark
*	DC.B 0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
* linear correction 2 (lightening)
*	DC.B 0,0,1,1,2,3,4,5,6,7,8,9,10,11,12,13
* linear correction 3 (lightening)
*	DC.B 0,1,1,1,2,3,4,5,6,7,8,9,10,11,12,13

	IFGT	MYDEBUG
BYMC1	DC.B	'PRE:  x=%ld  B$%lx  Y$%lx  M$%lx  C$%lx',10,0
BYMC2	DC.B	'POST: d3=$%lx  B$%lx  Y$%lx  M$%lx  C$%lx',10,0
GRAY1	DC.B	'PRE:  x=%ld G=$%lx',10,0
GRAY2	DC.B	'POST: d3=%ld G=$%lx',10,0
THRES   DC.B	10,'threshold is %ld, y=%ld',10,0
GAMMA   DC.B	'gammas are $%08lx %08lx %08lx %08lx',10,0

	ENDC 	
END
