*******************************************************************************
*
*	$Id: downellipse.asm,v 39.0 91/08/21 17:17:05 chrisg Exp $
*
*******************************************************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'graphics/gfx.i'
	INCLUDE 'graphics/rastport.i'
	INCLUDE 'graphics/clip.i'
	INCLUDE 'graphics/gels.i'
	INCLUDE 'hardware/blit.i'
	INCLUDE	'/macros.i'
	include '/a/submacs.i'

	section graphics

BIG_BLITS	SET	1

	IFD	BIG_BLITS

*	64 bit arithmetic for big ellipses
*	can be optimised quite a bit later

TST3	macro	* first
	ADD3	ZERO,\1,SCRATCH
	endm

SUB3	macro	* first, second, destination 

	movem.l d0-d2,-(sp)

	move.l	-\1_OFF_HI(a5),d2
	move.l	-\2_OFF_HI(a5),d0

	move.l	-\2_OFF_LO(a5),d1
	sub.l	-\1_OFF_LO(a5),d1
	subx.l	d2,d0

	movem.l d0-d1,-\3_OFF_HI(a5) * does not affect ccr

	movem.l (sp)+,d0-d2 

	endm

ADD3	macro	* first, second, destination 

	movem.l d0-d2,-(sp)

	move.l	-\1_OFF_HI(a5),d2
	move.l	-\2_OFF_HI(a5),d0

	move.l	-\2_OFF_LO(a5),d1
	add.l	-\1_OFF_LO(a5),d1
	addx.l	d2,d0

	movem.l d0-d1,-\3_OFF_HI(a5) * does not affect ccr

	movem.l (sp)+,d0-d2
	endm

MUL3	macro	* multiplier, multiplicand, destination 

	movem.l d0-d1,-(sp)

	move.w	-\2_OFF_LO(a5),d1
	mulu	2-\1_OFF_LO(a5),d1	

	move.w	2-\2_OFF_LO(a5),d0
	mulu	2-\1_OFF_LO(a5),d0	

	move.w	d0,2-\3_OFF_LO(a5)
	clr.w	d0
	swap	d0
	add.l	d0,d1
	move.w	d1,-\3_OFF_LO(a5)
	clr.w	d0
	swap	d1
	ext.l	d1
	move.l	d1,-\3_OFF_HI(a5)

	movem.l (sp)+,d0-d1
	endm

DOJSR	macro
	JSR	(a3)
	endm

X_OFF	EQU	1*4
Y_OFF	EQU	X_OFF+1*4
T1_OFF_LO	EQU	Y_OFF+1*4
T1_OFF_HI	EQU	T1_OFF_LO+1*4
T2_OFF_LO	EQU	T1_OFF_HI+1*4
T2_OFF_HI	EQU	T2_OFF_LO+1*4
T3_OFF_LO	EQU	T2_OFF_HI+1*4
T3_OFF_HI	EQU	T3_OFF_LO+1*4
T4_OFF_LO	EQU	T3_OFF_HI+1*4
T4_OFF_HI	EQU	T4_OFF_LO+1*4
T5_OFF_LO	EQU	T4_OFF_HI+1*4
T5_OFF_HI	EQU	T5_OFF_LO+1*4
T6_OFF_LO	EQU	T5_OFF_HI+1*4
T6_OFF_HI	EQU	T6_OFF_LO+1*4
T7_OFF_LO	EQU	T6_OFF_HI+1*4
T7_OFF_HI	EQU	T7_OFF_LO+1*4
T8_OFF_LO	EQU	T7_OFF_HI+1*4
T8_OFF_HI	EQU	T8_OFF_LO+1*4
T9_OFF_LO	EQU	T8_OFF_HI+1*4
T9_OFF_HI	EQU	T9_OFF_LO+1*4
D1_OFF_LO	EQU	T9_OFF_HI+1*4
D1_OFF_HI	EQU	D1_OFF_LO+1*4
D2_OFF_LO	EQU	D1_OFF_HI+1*4
D2_OFF_HI	EQU	D2_OFF_LO+1*4
ZERO_OFF_LO	EQU	D2_OFF_HI+1*4	; for big blits
ZERO_OFF_HI	EQU	ZERO_OFF_LO+1*4 ; for big blits
SCRATCH_OFF_LO	EQU	ZERO_OFF_HI+1*4	
SCRATCH_OFF_HI	EQU	SCRATCH_OFF_LO+1*4
PIXX_R_OFF	EQU	SCRATCH_OFF_HI+1*2
PIXX_L_OFF	EQU	PIXX_R_OFF+1*2
PIXY_B_OFF	EQU	PIXX_L_OFF+1*2
PIXY_T_OFF	EQU	PIXY_B_OFF+1*2
ROUNDX_OFF	EQU	PIXY_T_OFF+1*4
ROUNDY_OFF	EQU	ROUNDX_OFF+1*4
ROUNDCX_OFF	EQU	ROUNDY_OFF+1*4
ROUNDCY_OFF	EQU	ROUNDCX_OFF+1*4
LINEDONE_OFF	EQU	ROUNDCY_OFF+1*1
LINEFLAG_OFF	EQU	LINEDONE_OFF+1*1
WORKSPACE	EQU	LINEFLAG_OFF

RP_OFF	EQU	2*4	; rp
CX_OFF	EQU	3*4	; cx
CY_OFF	EQU	4*4	; cy
A_OFF	EQU	5*4	; a
B_OFF	EQU	6*4	; b

HI_BYTE_OFF	EQU	0
LO_BYTE_OFF	EQU	3
HI_WORD_OFF	EQU	0
LO_WORD_OFF	EQU	2

	XREF	_LVOWritePixel
	xref	waitblitdone

	XDEF	_draw_ellipse

_draw_ellipse:

	LINK	A5,#-WORKSPACE

	CLR.B	-LINEDONE_OFF(A5)	* line_done = FALSE

	MOVEM.L	D2-D7/A2/a3/a4,-(SP)	* save registers on stack
	move.l	2+_LVOWritePixel(a6),a3	* just a short time, really
	if (a1).l=#0	* null layer ptr?
	if #NANBC+NABC+ANBC+ABNC=rp_minterms(a1).b  * xor mode?
	btst #0,rp_Mask(a1)   * this bitplane
	if <>
	move.l	rp_BitMap(a1),a4 * cache BitMap ptr here
	if #1=bm_Depth(a4).b * want only one bitplane
		lea	fast_xor_pixel(PC),a3
		WAITBLITDONE
	endif
	endif
	endif
	endif

	MOVE.L	RP_OFF(A5),A2	* rastport pointer in a2
	MOVE.W	A_OFF+LO_WORD_OFF(A5),D2	* x = a in d2
	EXT.L	D2
	CLR.L	D3		* y = 0 in d3

	MOVE.W	D2,D0		* copy of a in d0
	MOVE.W	D2,D1		* copy of a in d1
	MULS	D0,D1		* (a*a) in d1
	MOVE.L	D1,-T1_OFF_LO(A5)	* t1 = (a*a)
	CLR.L	-T1_OFF_HI(A5)
	ADD.L	D1,D1		* 2*t1
	MOVE.L	D1,-T2_OFF_LO(A5)	* t2 = 2*t1
	CLR.L	-T2_OFF_HI(A5)
	ADD.L	D1,D1		* 2*t2
	MOVE.L	D1,-T3_OFF_LO(A5)	* t3 = 2*t2
	CLR.L	-T3_OFF_HI(A5)

	MOVE.W	B_OFF+LO_WORD_OFF(A5),D1	* b in d1
	MULS	B_OFF+LO_WORD_OFF(A5),D1	* (b*b) in d1

	MOVE.L	D1,-T4_OFF_LO(A5)	* t4 = (b*b)
	CLR.L	-T4_OFF_HI(A5)
	ADD.L	D1,D1		* 2*t4
	MOVE.L	D1,-T5_OFF_LO(A5)	* t5 = 2*t4
	CLR.L	-T5_OFF_HI(A5)

*	bart - 11.10.88 support for "big blits"
*	bart - 11.10.88 assumes that (0 <= a <= 32768) && (0 <= b <= 32768)
	
	CLR.L	-ZERO_OFF_LO(a5)	* for multiple 
	CLR.L	-ZERO_OFF_HI(a5) 	* precision comparisons

	SWAP	D0
	CLR.W	D0
	SWAP	D0
	MOVE.L	D0,-T9_OFF_LO(A5) 	* copy of a in T9
	CLR.L	-T9_OFF_HI(A5)

	MUL3	T9,T5,T7

	MOVE.L	-T7_OFF_LO(A5),D0	* bogus debug compatibility
	MOVE.L	-T5_OFF_LO(A5),D1	* restore t5 in D1

	ADD3	T5,T5,T6
	ADD3	T7,T7,T8

	CLR.L	-T9_OFF_LO(A5)	* t9 = 0
	CLR.L	-T9_OFF_HI(A5)	* t9 = 0

	SUB3	T7,T2,D1
	ADD3	T4,D1,D1

	SUB3	T8,T5,D2
	ADD3	T1,D2,D2		* sets condition codes

is_d2_lst:

	BGE	end_is_d2_lst		* check condition codes

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D0	* cx

	MOVE.W	D0,D6		* copy of cx in d6
	ADD.W	D2,D0		* pixel_x_right = cx + x
	MOVE.W	D0,-PIXX_R_OFF(A5)	* save pixel_x_right

	SUB.W	D2,D6		* pixel_x_left  = cx - x

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1	* cy

	MOVE.W	D1,D7		* copy of cy in d7
	ADD.W	D3,D1		* pixel_y_bottom = cy + y
	MOVE.W	D1,-PIXY_B_OFF(A5)	* save pixel_y_bottom

	SUB.W	D3,D7		* pixel_x_top	= cy - y

	BTST.B	 #ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BEQ.S	no_d2_onedot

	CMP.W	D0,D6		* pixel_x_left = pixel_x_right ?
	BEQ.S	end_d2_onedot	* nothing to do

	BSR	do_onedot	* do common code

	BRA.S	end_d2_onedot

no_d2_onedot:

	BSR	do_main		* do common code

end_d2_onedot:

	ADDQ.L	#1,D3		* y += 1

	ADD3	T3,T9,T9		
	TST3	D1		

	BGE	is_d1_gth

	ADD3	T9,D1,D1	
	ADD3	T2,D1,D1	
	ADD3	T9,D2,D2	

	BRA	end_d1_gth	

is_d1_gth:

	SUBQ.L	#1,D2		* x -= 1

	SUB3	T6,T8,T8		
	ADD3	T9,D1,D1	
	ADD3	T2,D1,D1	
	SUB3	T8,D1,D1	
	ADD3	T9,D2,D2	
	ADD3	T5,D2,D2	
	SUB3	T8,D2,D2	

end_d1_gth:

	BRA	is_d2_lst

end_is_d2_lst:

is_x_ge:

	TST.L	D2		* x >= 0 ?
	BLT	finish

	CLR.B	-LINEFLAG_OFF(A5)	* line_flag = FALSE

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D0	* cx

	MOVE.W	D0,D6		* copy of cx in d6
	ADD.W	D2,D0		* pixel_x_right = cx + x
	MOVE.W	D0,-PIXX_R_OFF(A5)	* save pixel_x_right

	SUB.W	D2,D6		* pixel_x_left  = cx - x

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1	* cy

	MOVE.W	D1,D7		* copy of cy in d7
	ADD.W	D3,D1		* pixel_y_bottom = cy + y
	MOVE.W	D1,-PIXY_B_OFF(A5)	* save pixel_y_bottom

	SUB.W	D3,D7		* pixel_x_top	= cy - y

	BTST.B	 #ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BEQ.S	no_d1_onedot

	TST.B	-LINEDONE_OFF(A5)	* !linedone ?
	BNE.S	end_d1_onedot

	CMP.W	D0,D6		* pixel_x_left = pixel_x_right ?
	BEQ.S	end_d1_onedot	* nothing to do

	ST	-LINEFLAG_OFF(A5)	* line_flag = TRUE;

	BSR	do_onedot	* do common code
	
	BRA.S	end_d1_onedot

no_d1_onedot:

	ST	-LINEFLAG_OFF(A5)	* line_flag = TRUE;

	BSR	do_main		* do common code

end_d1_onedot:

	TST.B	-LINEFLAG_OFF(A5)	* line_flag ?
	BEQ.S	skip_set

	ST	-LINEDONE_OFF(A5)	* line_done = TRUE;

skip_set:

	SUBQ.L	#1,D2		* x -= 1

	SUB3	T6,T8,T8	
	TST3	D2		
	BGE	is_d2_gth 	

	ADDQ.L	#1,D3		* y += 1
	CLR.B	-LINEDONE_OFF(A5)	* line_done = FALSE

	ADD3	T3,T9,T9		
	ADD3	T5,D2,D2	
	ADD3	T9,D2,D2	
	SUB3	T8,D2,D2	

	BRA.S	end_d2_gth

is_d2_gth:

	ADD3	T5,D2,D2	
	SUB3	T8,D2,D2	

end_d2_gth

	BRA	is_x_ge

finish:

	BTST.B	 #ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BNE.S	return

	TST.B	-LINEDONE_OFF(A5)	* line_done ?
	BEQ.S	no_increment

	ADDQ.L	#1,D3		* y += 1

no_increment:

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D6	* pixel_x_right = cx in d6
	MOVE.W	B_OFF+LO_WORD_OFF(A5),D5	* b in d5

while_yle:

	CMP.W	D5,D3		* y <= b ?	
	BGT.S	return

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1	* cy

	MOVE.W	D1,D7		* copy of cy in d7
	SUB.W	D3,D1		* pixel_x_top	= cy - y
	ADD.W	D3,D7		* pixel_y_bottom = cy + y

	CMP.W	D7,D1		* pixel_y_bottom > pixel_y_top ?
	BGE.S	no_top

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_right in d0
	DOJSR

no_top:

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_right in d0
	MOVE.W	D7,D1		* pixel_y_bottom in d1
	DOJSR

	ADDQ.L	#1,D3		* y += 1
	BRA.S	while_yle

return:

	MOVEM.L	(SP)+,D2-D7/A2/a3/a4	* restore registers
	UNLK	A5

	RTS

do_onedot:

	MOVEA.L	A2,A1		* copy of rp in a1
	DOJSR

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_left in d0
	MOVE.W	-PIXY_B_OFF(A5),D1	* pixel_y_bottom in d1
	DOJSR
	
	CMP.W	-PIXY_B_OFF(A5),D7	* pixel_y_bottom > pixel_y_top ?
	BGE.S	end_do_onedot

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	-PIXX_R_OFF(A5),D0	* pixel_x_right in d0
	MOVE.W	D7,D1		* pixel_y_top in d1
	DOJSR
	
	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_left in d0
	MOVE.W	D7,D1		* pixel_y_top in d1
	DOJSR

end_do_onedot:

	RTS

do_main:

	MOVEA.L	A2,A1		* copy of rp in a1
	DOJSR

	CMP.W	-PIXX_R_OFF(A5),D6	* pixel_x_left < pixel_x_right ?
	BGE.S	do_skip

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_left in d0
	MOVE.W	-PIXY_B_OFF(A5),D1	* pixel_y_bottom in d1
	DOJSR

do_skip:

	CMP.W	-PIXY_B_OFF(A5),D7	* pixel_y_bottom > pixel_y_top ?
	BGE.S	end_do_main

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	-PIXX_R_OFF(A5),D0	* pixel_x_right in d0
	MOVE.W	D7,D1		* pixel_y_top in d1
	DOJSR

	CMP.W	-PIXX_R_OFF(A5),D6	* pixel_x_left < pixel_x_right ?
	BGE.S	end_do_main

	MOVEA.L	A2,A1		* copy of rp in a1
	MOVE.W	D6,D0		* pixel_x_left in d0
	MOVE.W	D7,D1		* pixel_y_top in d1
	DOJSR

end_do_main

	RTS
		
	ENDC

*	keep old code around for immediate comparison

	IFND	BIG_BLITS

DOJSR	macro
*	JSR		_LVOWritePixel(A6)		* call graphics pixel routine
	JSR	(a3)
	endm

X_OFF	EQU	1*4
Y_OFF	EQU	X_OFF+1*4
T1_OFF	EQU	Y_OFF+1*4
T2_OFF	EQU	T1_OFF+1*4
T3_OFF	EQU	T2_OFF+1*4
T4_OFF	EQU	T3_OFF+1*4
T5_OFF	EQU	T4_OFF+1*4
T6_OFF	EQU	T5_OFF+1*4
T7_OFF	EQU	T6_OFF+1*4
T8_OFF	EQU	T7_OFF+1*4
T9_OFF	EQU	T8_OFF+1*4
D1_OFF	EQU	T9_OFF+1*4
D2_OFF	EQU	D1_OFF+1*4
PIXX_R_OFF	EQU	D2_OFF+1*2
PIXX_L_OFF	EQU	PIXX_R_OFF+1*2
PIXY_B_OFF	EQU	PIXX_L_OFF+1*2
PIXY_T_OFF	EQU	PIXY_B_OFF+1*2
ROUNDX_OFF	EQU	PIXY_T_OFF+1*4
ROUNDY_OFF	EQU	ROUNDX_OFF+1*4
ROUNDCX_OFF	EQU	ROUNDY_OFF+1*4
ROUNDCY_OFF	EQU	ROUNDCX_OFF+1*4
LINEDONE_OFF	EQU	ROUNDCY_OFF+1*1
LINEFLAG_OFF	EQU	LINEDONE_OFF+1*1
WORKSPACE	EQU	LINEFLAG_OFF

RP_OFF	EQU	2*4		; rp
CX_OFF	EQU	3*4		; cx
CY_OFF	EQU	4*4		; cy
A_OFF	EQU	5*4		; a
B_OFF	EQU	6*4		; b

HI_BYTE_OFF	EQU	0
LO_BYTE_OFF	EQU	3
HI_WORD_OFF	EQU	0
LO_WORD_OFF	EQU	2

	XREF	_LVOWritePixel
	xref	waitblitdone

	XDEF	_draw_ellipse

_draw_ellipse:

	LINK	A5,#-WORKSPACE

	CLR.B	-LINEDONE_OFF(A5)			* line_done = FALSE

	MOVEM.L	D2-D7/A2/a3/a4,-(SP)		* save registers on stack
	move.l	2+_LVOWritePixel(a6),a3		* just a short time, really
	if (a1).l=#0	* null layer ptr?
		if #NANBC+NABC+ANBC+ABNC=rp_minterms(a1).b  * xor mode?
			btst #0,rp_Mask(a1)   * this bitplane
			if <>
				move.l	rp_BitMap(a1),a4 * cache BitMap ptr here
				if #1=bm_Depth(a4).b * want only one bitplane
					lea	fast_xor_pixel(PC),a3
					WAITBLITDONE
				endif
			endif
		endif
	endif

	MOVE.L	RP_OFF(A5),A2			* rastport pointer in a2
	MOVE.W	A_OFF+LO_WORD_OFF(A5),D2	* x = a in d2
	EXT.L	D2
	CLR.L	D3							* y = 0 in d3

	MOVE.W	D2,D0					* copy of a in d0
	MOVE.W	D2,D1					* copy of a in d1
	MULS	D0,D1					* (a*a) in d1
	MOVE.L	D1,-T1_OFF(A5)			* t1 = (a*a)
	ADD.L	D1,D1					* 2*t1
	MOVE.L	D1,-T2_OFF(A5)			* t2 = 2*t1
	ADD.L	D1,D1					* 2*t2
	MOVE.L	D1,-T3_OFF(A5)			* t3 = 2*t2

	MOVE.W	B_OFF+LO_WORD_OFF(A5),D1	* b in d1
	MULS	B_OFF+LO_WORD_OFF(A5),D1	* (b*b) in d1

	MOVE.L	D1,-T4_OFF(A5)			* t4 = (b*b)
	ADD.L	D1,D1					* 2*t4
	MOVE.L	D1,-T5_OFF(A5)			* t5 = 2*t4

*	bart - 06.09.86 prevent overflow if b > 255
*	bart - 06.09.86 assumes that (0 <= a <= 1024) && (0 <= b <= 1024)

	MULU	D1,D0					* a*(low word of)t5
	MOVE.L	D0,-T9_OFF(A5)			* temporary storage

	CLR.W	D1
	SWAP 	D1						* (high word of)t5
	MOVE.W	D2,D0					* copy of a in d0

	MULU	D1,D0					* a*(high word of)t5
	SWAP 	D0						* (low word of)(a*(high word of)t5)
	CLR.W	D0						* (high word of)(a*(high word of)t5) == 0
	ADD.L	-T9_OFF(A5),D0			* result

	MOVE.L	D0,-T7_OFF(A5)			* t7 = a*t5

	MOVE.L	-T5_OFF(A5),D1			* restore t5 in D1

*	end bart - 06.09.86

	ADD.L	D1,D1					* 2*t5
	MOVE.L	D1,-T6_OFF(A5)			* t6 = 2*t5
	
	LSL.L	#1,D0					* 2*t7
	MOVE.L	D0,-T8_OFF(A5)			* t8 = 2*t7

	CLR.L	-T9_OFF(A5)				* t9 = 0

	LSR.L	#1,D1					* "t5" = (t6/2)
	SUB.L	D0,D1					* t5 - t8
	ADD.L	-T1_OFF(A5),D1			* t1 - t8 + t5
	MOVE.L	D1,D5					* variable "d2" in register D5

	MOVE.L	-T2_OFF(A5),D0			* t2
	SUB.L	-T7_OFF(A5),D0			* t2 - t7
	ADD.L	-T4_OFF(A5),D0			* t2 - t7 + t4
	MOVE.L	D0,D4					* variable "d1" in register D4	

is_d2_lst:

	TST.L	D5
	BGE		end_is_d2_lst	

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D0		* cx

	MOVE.W	D0,D6					* copy of cx in d6
	ADD.W	D2,D0					* pixel_x_right = cx + x
	MOVE.W	D0,-PIXX_R_OFF(A5)		* save pixel_x_right

	SUB.W	D2,D6					* pixel_x_left  = cx - x

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1		* cy

	MOVE.W	D1,D7					* copy of cy in d7
	ADD.W	D3,D1					* pixel_y_bottom = cy + y
	MOVE.W	D1,-PIXY_B_OFF(A5)		* save pixel_y_bottom

	SUB.W	D3,D7					* pixel_x_top    = cy - y

	BTST.B 	#ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BEQ.S	no_d2_onedot

	CMP.W	D0,D6					* pixel_x_left = pixel_x_right ?
	BEQ.S	end_d2_onedot			* nothing to do

	BSR		do_onedot				* do common code

	BRA.S	end_d2_onedot

no_d2_onedot:

	BSR		do_main					* do common code

end_d2_onedot:

	ADDQ.L	#1,D3					* y += 1
	MOVE.L	-T9_OFF(A5),D0			* t9 in d0
	ADD.L	-T3_OFF(A5),D0			* t9 + t3 in d0
	MOVE.L	D0,-T9_OFF(A5)			* update t9 += t3

	TST.L	D4						* test variable "d1" < 0 ?
	BGE.S	is_d1_gth

	ADD.L	D0,D4					* "d1" += t9
	ADD.L	-T2_OFF(A5),D4			* "d1" += t9 + t2
	ADD.L	D0,D5					* "d2" += t9

	BRA.S	end_d1_gth

is_d1_gth:

	SUBQ.L	#1,D2					* x -= 1
	MOVE.L	-T8_OFF(A5),D1			* t8 in d1
	SUB.L	-T6_OFF(A5),D1			* t8 - t6 in d1
	MOVE.L	D1,-T8_OFF(A5)			* update t8 -= t6

	ADD.L	D0,D4					* "d1" += t9
	ADD.L	-T2_OFF(A5),D4			* "d1" += t9 + t2
	SUB.L	D1,D4					* "d1" += t9 + t2 - t8
	
	ADD.L	D0,D5					* "d2" += t9
	ADD.L	-T5_OFF(A5),D5			* "d1" += t9 + t5
	SUB.L	D1,D5					* "d1" += t9 + t5 - t8

end_d1_gth:

	BRA		is_d2_lst

end_is_d2_lst:

is_x_ge:

	TST.L	D2						* x >= 0 ?
	BLT		finish

	CLR.B	-LINEFLAG_OFF(A5)		* line_flag = FALSE

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D0		* cx

	MOVE.W	D0,D6					* copy of cx in d6
	ADD.W	D2,D0					* pixel_x_right = cx + x
	MOVE.W	D0,-PIXX_R_OFF(A5)		* save pixel_x_right

	SUB.W	D2,D6					* pixel_x_left  = cx - x

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1		* cy

	MOVE.W	D1,D7					* copy of cy in d7
	ADD.W	D3,D1					* pixel_y_bottom = cy + y
	MOVE.W	D1,-PIXY_B_OFF(A5)		* save pixel_y_bottom

	SUB.W	D3,D7					* pixel_x_top    = cy - y

	BTST.B 	#ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BEQ.S	no_d1_onedot

	TST.B	-LINEDONE_OFF(A5)		* !linedone ?
	BNE.S	end_d1_onedot

	CMP.W	D0,D6					* pixel_x_left = pixel_x_right ?
	BEQ.S	end_d1_onedot			* nothing to do

	ST		-LINEFLAG_OFF(A5)	* line_flag = TRUE;

	BSR		do_onedot				* do common code
	
	BRA.S	end_d1_onedot

no_d1_onedot:

	ST		-LINEFLAG_OFF(A5)	* line_flag = TRUE;

	BSR		do_main					* do common code

end_d1_onedot:

	TST.B	-LINEFLAG_OFF(A5)		* line_flag ?
	BEQ.S	skip_set

	ST		-LINEDONE_OFF(A5)	* line_done = TRUE;

skip_set:

	SUBQ.L	#1,D2					* x -= 1

	MOVE.L	-T8_OFF(A5),D0			* t8 in d0
	SUB.L	-T6_OFF(A5),D0			* t8 - t6 in d0
	MOVE.L	D0,-T8_OFF(A5)			* update t8 -= t6

	TST.L	D5						* test variable "d2" < 0 ?
	BGE.S	is_d2_gth

	ADDQ.L	#1,D3					* y += 1
	CLR.B	-LINEDONE_OFF(A5)		* line_done = FALSE

	MOVE.L	-T9_OFF(A5),D1			* t9 in d1
	ADD.L	-T3_OFF(A5),D1			* t9 + t3 in d1
	MOVE.L	D1,-T9_OFF(A5)			* update t9 += t3
	SUB.L	D0,D1					* t9 - t8
	ADD.L	-T5_OFF(A5),D1			* t5 + t9 - t8
	ADD.L	D1,D5					* set variable "d2" += t5 + t9 - t8

	BRA.S	end_d2_gth

is_d2_gth:

	MOVE.L	-T5_OFF(A5),D1			* t5 in d1
	SUB.L	D0,D1					* t5 - t8
	ADD.L	D1,D5					* set variable "d2" += t5 - t8

end_d2_gth

	BRA		is_x_ge

finish:

	BTST.B 	#ONE_DOTn,rp_Flags+1(A2)	* rp->Flags & ONE_DOT ?
	BNE.S	return

	TST.B	-LINEDONE_OFF(A5)		* line_done ?
	BEQ.S	no_increment

	ADDQ.L	#1,D3					* y += 1

no_increment:

	MOVE.W	CX_OFF+LO_WORD_OFF(A5),D6		* pixel_x_right = cx in d6
	MOVE.W	B_OFF+LO_WORD_OFF(A5),D5		* b in d5

while_yle:

	CMP.W	D5,D3					* y <= b ?		
	BGT.S	return

	MOVE.W	CY_OFF+LO_WORD_OFF(A5),D1		* cy

	MOVE.W	D1,D7					* copy of cy in d7
	SUB.W	D3,D1					* pixel_x_top    = cy - y
	ADD.W	D3,D7					* pixel_y_bottom = cy + y

	CMP.W	D7,D1					* pixel_y_bottom > pixel_y_top ?
	BGE.S	no_top

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_right in d0
	DOJSR

no_top:

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_right in d0
	MOVE.W	D7,D1					* pixel_y_bottom in d1
	DOJSR

	ADDQ.L	#1,D3					* y += 1
	BRA.S	while_yle

return:

	MOVEM.L	(SP)+,D2-D7/A2/a3/a4		* restore registers
	UNLK	A5

	RTS

do_onedot:

	MOVEA.L	A2,A1					* copy of rp in a1
	DOJSR

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_left in d0
	MOVE.W	-PIXY_B_OFF(A5),D1		* pixel_y_bottom in d1
	DOJSR
	
	CMP.W	-PIXY_B_OFF(A5),D7		* pixel_y_bottom > pixel_y_top ?
	BGE.S	end_do_onedot

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	-PIXX_R_OFF(A5),D0		* pixel_x_right in d0
	MOVE.W	D7,D1					* pixel_y_top in d1
	DOJSR
	
	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_left in d0
	MOVE.W	D7,D1					* pixel_y_top in d1
	DOJSR

end_do_onedot:

	RTS

do_main:

	MOVEA.L	A2,A1					* copy of rp in a1
	DOJSR

	CMP.W	-PIXX_R_OFF(A5),D6		* pixel_x_left < pixel_x_right ?
	BGE.S	do_skip

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_left in d0
	MOVE.W	-PIXY_B_OFF(A5),D1		* pixel_y_bottom in d1
	DOJSR

do_skip:

	CMP.W	-PIXY_B_OFF(A5),D7		* pixel_y_bottom > pixel_y_top ?
	BGE.S	end_do_main

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	-PIXX_R_OFF(A5),D0			* pixel_x_right in d0
	MOVE.W	D7,D1					* pixel_y_top in d1
	DOJSR

	CMP.W	-PIXX_R_OFF(A5),D6		* pixel_x_left < pixel_x_right ?
	BGE.S	end_do_main

	MOVEA.L	A2,A1					* copy of rp in a1
	MOVE.W	D6,D0					* pixel_x_left in d0
	MOVE.W	D7,D1					* pixel_y_top in d1
	DOJSR

end_do_main

	RTS

	ENDC

fast_xor_pixel:
* a4 = bitmap ptr
	muls	(a4),d1
	move.l	bm_Planes(a4),a0		* get the only ptr
	add.l	d1,a0			* pnt to right row
	move.w	d0,d1
	asr.w	#3,d1
	add.w	d1,a0			* pnt to right byte
	not.b	d0			* 0->7 7->0  mod 8
	bchg	d0,(a0)			* complement the bit
	rts

	END
