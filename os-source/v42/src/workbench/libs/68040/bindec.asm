*
* $Id: bindec.asm,v 0.2 91/07/03 10:25:20 mks Exp $
*
* $Log:	bindec.asm,v $
* Revision 0.2  91/07/03  10:25:20  mks
* First pass at doing branch optimizations
* 
* Revision 0.1  91/06/26  09:01:50  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	bindec.sa 3.4 1/3/91
*
*	bindec
*
*	Description:
*		Converts an input in extended precision format
*		to bcd format.
*
*	Input:
*		a0 points to the input extended precision value
*		value in memory; d0 contains the k-factor sign-extended
*		to 32-bits.  The input may be either normalized,
*		unnormalized, or denormalized.
*
*	Output:	result in the FP_SCR1 space on the stack.
*
*	Saves and Modifies: D2-D7,A2,FP2
*
*	Algorithm:
*
*	A1.	Set RM and size ext;  Set SIGMA = sign of input.
*		The k-factor is saved for use in d7. Clear the
*		BINDEC_FLG for separating normalized/denormalized
*		input.  If input is unnormalized or denormalized,
*		normalize it.
*
*	A2.	Set X = abs(input).
*
*	A3.	Compute ILOG.
*		ILOG is the log base 10 of the input value.  It is
*		approximated by adding e + 0.f when the original
*		value is viewed as 2^^e * 1.f in extended precision.
*		This value is stored in d6.
*
*	A4.	Clr INEX bit.
*		The operation in A3 above may have set INEX2.
*
*	A5.	Set ICTR = 0;
*		ICTR is a flag used in A13.  It must be set before the
*		loop entry A6.
*
*	A6.	Calculate LEN.
*		LEN is the number of digits to be displayed.  The
*		k-factor can dictate either the total number of digits,
*		if it is a positive number, or the number of digits
*		after the decimal point which are to be included as
*		significant.  See the 68882 manual for examples.
*		If LEN is computed to be greater than 17, set OPERR in
*		USER_FPSR.  LEN is stored in d4.
*
*	A7.	Calculate SCALE.
*		SCALE is equal to 10^ISCALE, where ISCALE is the number
*		of decimal places needed to insure LEN integer digits
*		in the output before conversion to bcd. LAMBDA is the
*		sign of ISCALE, used in A9. Fp1 contains
*		10^^(abs(ISCALE)) using a rounding mode which is a
*		function of the original rounding mode and the signs
*		of ISCALE and X.  A table is given in the code.
*
*	A8.	Clr INEX; Force RZ.
*		The operation in A3 above may have set INEX2.
*		RZ mode is forced for the scaling operation to insure
*		only one rounding error.  The grs bits are collected in
*		the INEX flag for use in A10.
*
*	A9.	Scale X -> Y.
*		The mantissa is scaled to the desired number of
*		significant digits.  The excess digits are collected
*		in INEX2.
*
*	A10.	Or in INEX.
*		If INEX is set, round error occured.  This is
*		compensated for by 'or-ing' in the INEX2 flag to
*		the lsb of Y.
*
*	A11.	Restore original FPCR; set size ext.
*		Perform FINT operation in the user's rounding mode.
*		Keep the size to extended.
*
*	A12.	Calculate YINT = FINT(Y) according to user's rounding
*		mode.  The FPSP routine sintd0 is used.  The output
*		is in fp0.
*
*	A13.	Check for LEN digits.
*		If the int operation results in more than LEN digits,
*		or less than LEN -1 digits, adjust ILOG and repeat from
*		A6.  This test occurs only on the first pass.  If the
*		result is exactly 10^LEN, decrement ILOG and divide
*		the mantissa by 10.
*
*	A14.	Convert the mantissa to bcd.
*		The binstr routine is used to convert the LEN digit
*		mantissa to bcd in memory.  The input to binstr is
*		to be a fraction; i.e. (mantissa)/10^LEN and adjusted
*		such that the decimal point is to the left of bit 63.
*		The bcd digits are stored in the correct position in
*		the final string area in memory.
*
*	A15.	Convert the exponent to bcd.
*		As in A14 above, the exp is converted to bcd and the
*		digits are stored in the final string.
*		Test the length of the final exponent string.  If the
*		length is 4, set operr.
*
*	A16.	Write sign bits to final string.
*
*	Implementation Notes:
*
*	The registers are used as follows:
*
*		d0: scratch; LEN input to binstr
*		d1: scratch
*		d2: upper 32-bits of mantissa for binstr
*		d3: scratch;lower 32-bits of mantissa for binstr
*		d4: LEN
*      		d5: LAMBDA/ICTR
*		d6: ILOG
*		d7: k-factor
*		a0: ptr for original operand/final result
*		a1: scratch pointer
*		a2: pointer to FP_X; abs(original value) in ext
*		fp0: scratch
*		fp1: scratch
*		fp2: scratch
*		F_SCR1:
*		F_SCR2:
*		L_SCR1:
*		L_SCR2:

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

BINDEC    IDNT    2,1 Motorola 040 Floating Point Software Package

	include	"fpsp.i"

	section	8

* Constants in extended precision
LOG2 	dc.l	$3FFD0000,$9A209A84,$FBCFF798,$00000000
LOG2UP1	dc.l	$3FFD0000,$9A209A84,$FBCFF799,$00000000

* Constants in single precision
FONE 	dc.l	$3F800000,$00000000,$00000000,$00000000
FTWO	dc.l	$40000000,$00000000,$00000000,$00000000
FTEN 	dc.l	$41200000,$00000000,$00000000,$00000000
F4933	dc.l	$459A2800,$00000000,$00000000,$00000000

RBDTBL 	dc.b	0,0,0,0
	dc.b	3,3,2,2
	dc.b	3,2,2,3
	dc.b	2,3,3,2

	xref	binstr
	xref	sintdo
	xref	PTENRN,PTENRM,PTENRP

	xdef	bindec
	xdef	sc_mul
bindec:
	movem.l	d2-d7/a2,-(a7)
	fmovem.x fp0-fp2,-(a7)

* A1. Set RM and size ext. Set SIGMA = sign input;
*     The k-factor is saved for use in d7.  Clear BINDEC_FLG for
*     separating  normalized/denormalized input.  If the input
*     is a denormalized number, set the BINDEC_FLG memory word
*     to signal denorm.  If the input is unnormalized, normalize
*     the input and test for denormalized result.
*
	fmove.l	#rm_mode,FPCR	;set RM and ext
	move.l	(a0),L_SCR2(a6)	;save exponent for sign check
	move.l	d0,d7		;move k-factor to d7
	clr.b	BINDEC_FLG(a6)	;clr norm/denorm flag
	move.w	STAG(a6),d0	;get stag
	andi.w	#$e000,d0	;isolate stag bits
	beq.s	A2_str		;if zero, input is norm
*
* Normalize the denorm
*
un_de_norm:
	move.w	(a0),d0
	andi.w	#$7fff,d0	;strip sign of normalized exp
	move.l	4(a0),d1
	move.l	8(a0),d2
norm_loop:
	sub.w	#1,d0
	lsl.l	#1,d2
	roxl.l	#1,d1
	tst.l	d1
	bge.b	norm_loop
*
* Test if the normalized input is denormalized
*
	tst.w	d0
	bgt.b	pos_exp		;if greater than zero, it is a norm
	st	BINDEC_FLG(a6)	;set flag for denorm
pos_exp:
	andi.w	#$7fff,d0	;strip sign of normalized exp
	move.w	d0,(a0)
	move.l	d1,4(a0)
	move.l	d2,8(a0)

* A2. Set X = abs(input).
*
A2_str:
	move.l	(a0),FP_SCR2(a6) ; move input to work space
	move.l	4(a0),FP_SCR2+4(a6) ; move input to work space
	move.l	8(a0),FP_SCR2+8(a6) ; move input to work space
	andi.l	#$7fffffff,FP_SCR2(a6) ;create abs(X)

* A3. Compute ILOG.
*     ILOG is the log base 10 of the input value.  It is approx-
*     imated by adding e + 0.f when the original value is viewed
*     as 2^^e * 1.f in extended precision.  This value is stored
*     in d6.
*
* Register usage:
*	Input/Output
*	d0: k-factor/exponent
*	d2: x/x
*	d3: x/x
*	d4: x/x
*	d5: x/x
*	d6: x/ILOG
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/final result
*	a1: x/x
*	a2: x/x
*	fp0: x/float(ILOG)
*	fp1: x/x
*	fp2: x/x
*	F_SCR1:x/x
*	F_SCR2:Abs(X)/Abs(X) with $3fff exponent
*	L_SCR1:x/x
*	L_SCR2:first word of X packed/Unchanged

	tst.b	BINDEC_FLG(a6)	;check for denorm
	beq.b	A3_cont		;if clr, continue with norm
	move.l	#-4933,d6	;force ILOG = -4933
	bra.b	A4_str
A3_cont:
	move.w	FP_SCR2(a6),d0	;move exp to d0
	move.w	#$3fff,FP_SCR2(a6) ;replace exponent with 0x3fff
	fmove.x	FP_SCR2(a6),fp0	;now fp0 has 1.f
	sub.w	#$3fff,d0	;strip off bias
	fadd.w	d0,fp0		;add in exp
	fsub.s	FONE,fp0	;subtract off 1.0
	fbge.w	pos_res		;if pos, branch
	fmul.x	LOG2UP1,fp0	;if neg, mul by LOG2UP1
	fmove.l	fp0,d6		;put ILOG in d6 as a lword
	bra.b	A4_str		;go move out ILOG
pos_res:
	fmul.x	LOG2,fp0	;if pos, mul by LOG2
	fmove.l	fp0,d6		;put ILOG in d6 as a lword


* A4. Clr INEX bit.
*     The operation in A3 above may have set INEX2.

A4_str:
	fmove.l	#0,FPSR		;zero all of fpsr - nothing needed


* A5. Set ICTR = 0;
*     ICTR is a flag used in A13.  It must be set before the
*     loop entry A6. The lower word of d5 is used for ICTR.

	clr.w	d5		;clear ICTR


* A6. Calculate LEN.
*     LEN is the number of digits to be displayed.  The k-factor
*     can dictate either the total number of digits, if it is
*     a positive number, or the number of digits after the
*     original decimal point which are to be included as
*     significant.  See the 68882 manual for examples.
*     If LEN is computed to be greater than 17, set OPERR in
*     USER_FPSR.  LEN is stored in d4.
*
* Register usage:
*	Input/Output
*	d0: exponent/Unchanged
*	d2: x/x/scratch
*	d3: x/x
*	d4: exc picture/LEN
*	d5: ICTR/Unchanged
*	d6: ILOG/Unchanged
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/final result
*	a1: x/x
*	a2: x/x
*	fp0: float(ILOG)/Unchanged
*	fp1: x/x
*	fp2: x/x
*	F_SCR1:x/x
*	F_SCR2:Abs(X) with $3fff exponent/Unchanged
*	L_SCR1:x/x
*	L_SCR2:first word of X packed/Unchanged

A6_str:
	tst.l	d7		;branch on sign of k
	ble.b	k_neg		;if k <= 0, LEN = ILOG + 1 - k
	move.l	d7,d4		;if k > 0, LEN = k
	bra.b	len_ck		;skip to LEN check
k_neg:
	move.l	d6,d4		;first load ILOG to d4
	sub.l	d7,d4		;subtract off k
	addq.l	#1,d4		;add in the 1
len_ck:
	tst.l	d4		;LEN check: branch on sign of LEN
	ble.b	LEN_ng		;if neg, set LEN = 1
	cmp.l	#17,d4		;test if LEN > 17
	ble.b	A7_str		;if not, forget it
	move.l	#17,d4		;set max LEN = 17
	tst.l	d7		;if negative, never set OPERR
	ble.b	A7_str		;if positive, continue
	or.l	#opaop_mask,USER_FPSR(a6) ;set OPERR & AIOP in USER_FPSR
	bra.b	A7_str		;finished here
LEN_ng:
	moveq.l	#1,d4		;min LEN is 1


* A7. Calculate SCALE.
*     SCALE is equal to 10^ISCALE, where ISCALE is the number
*     of decimal places needed to insure LEN integer digits
*     in the output before conversion to bcd. LAMBDA is the sign
*     of ISCALE, used in A9.  Fp1 contains 10^^(abs(ISCALE)) using
*     the rounding mode as given in the following table (see
*     Coonen, p. 7.23 as ref.; however, the SCALE variable is
*     of opposite sign in bindec.sa from Coonen).
*
*	Initial					USE
*	FPCR[6:5]	LAMBDA	SIGN(X)		FPCR[6:5]
*	----------------------------------------------
*	 RN	00	   0	   0		00/0	RN
*	 RN	00	   0	   1		00/0	RN
*	 RN	00	   1	   0		00/0	RN
*	 RN	00	   1	   1		00/0	RN
*	 RZ	01	   0	   0		11/3	RP
*	 RZ	01	   0	   1		11/3	RP
*	 RZ	01	   1	   0		10/2	RM
*	 RZ	01	   1	   1		10/2	RM
*	 RM	10	   0	   0		11/3	RP
*	 RM	10	   0	   1		10/2	RM
*	 RM	10	   1	   0		10/2	RM
*	 RM	10	   1	   1		11/3	RP
*	 RP	11	   0	   0		10/2	RM
*	 RP	11	   0	   1		11/3	RP
*	 RP	11	   1	   0		11/3	RP
*	 RP	11	   1	   1		10/2	RM
*
* Register usage:
*	Input/Output
*	d0: exponent/scratch - final is 0
*	d2: x/0 or 24 for A9
*	d3: x/scratch - offset ptr into PTENRM array
*	d4: LEN/Unchanged
*	d5: 0/ICTR:LAMBDA
*	d6: ILOG/ILOG or k if ((k<=0)&(ILOG<k))
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/final result
*	a1: x/ptr to PTENRM array
*	a2: x/x
*	fp0: float(ILOG)/Unchanged
*	fp1: x/10^ISCALE
*	fp2: x/x
*	F_SCR1:x/x
*	F_SCR2:Abs(X) with $3fff exponent/Unchanged
*	L_SCR1:x/x
*	L_SCR2:first word of X packed/Unchanged

A7_str:
	tst.l	d7		;test sign of k
	bgt.b	k_pos		;if pos and > 0, skip this
	cmp.l	d6,d7		;test k - ILOG
	blt.b	k_pos		;if ILOG >= k, skip this
	move.l	d7,d6		;if ((k<0) & (ILOG < k)) ILOG = k
k_pos:
	move.l	d6,d0		;calc ILOG + 1 - LEN in d0
	addq.l	#1,d0		;add the 1
	sub.l	d4,d0		;sub off LEN
	swap	d5		;use upper word of d5 for LAMBDA
	clr.w	d5		;set it zero initially
	clr.w	d2		;set up d2 for very small case
	tst.l	d0		;test sign of ISCALE
	bge.b	iscale		;if pos, skip next inst
	addq.w	#1,d5		;if neg, set LAMBDA true
	cmp.l	#$ffffecd4,d0	;test iscale <= -4908
	bgt.b	no_inf		;if false, skip rest
	addi.l	#24,d0		;add in 24 to iscale
	move.l	#24,d2		;put 24 in d2 for A9
no_inf:
	neg.l	d0		;and take abs of ISCALE
iscale:
	fmove.s	FONE,fp1	;init fp1 to 1
	bfextu	USER_FPCR(a6){26:2},d1 ;get initial rmode bits
	lsl.w	#1,d1		;put them in bits 2:1
	add.w	d5,d1		;add in LAMBDA
	lsl.w	#1,d1		;put them in bits 3:1
	tst.l	L_SCR2(a6)	;test sign of original x
	bge.b	x_pos		;if pos, don't set bit 0
	addq.l	#1,d1		;if neg, set bit 0
x_pos:
	lea.l	RBDTBL,a2	;load rbdtbl base
	move.b	(a2,d1),d3	;load d3 with new rmode
	lsl.l	#4,d3		;put bits in proper position
	fmove.l	d3,fpcr		;load bits into fpu
	lsr.l	#4,d3		;put bits in proper position
	tst.b	d3		;decode new rmode for pten table
	bne.b	not_rn		;if zero, it is RN
	lea.l	PTENRN,a1	;load a1 with RN table base
	bra.b	rmode		;exit decode
not_rn:
	lsr.b	#1,d3		;get lsb in carry
	bcc.b	not_rp		;if carry clear, it is RM
	lea.l	PTENRP,a1	;load a1 with RP table base
	bra.b	rmode		;exit decode
not_rp:
	lea.l	PTENRM,a1	;load a1 with RM table base
rmode:
	clr.l	d3		;clr table index
e_loop:
	lsr.l	#1,d0		;shift next bit into carry
	bcc.b	e_next		;if zero, skip the mul
	fmul.x	(a1,d3),fp1	;mul by 10**(d3_bit_no)
e_next:
	add.l	#12,d3		;inc d3 to next pwrten table entry
	tst.l	d0		;test if ISCALE is zero
	bne.b	e_loop		;if not, loop


* A8. Clr INEX; Force RZ.
*     The operation in A3 above may have set INEX2.
*     RZ mode is forced for the scaling operation to insure
*     only one rounding error.  The grs bits are collected in
*     the INEX flag for use in A10.
*
* Register usage:
*	Input/Output

	fmove.l	#0,FPSR		;clr INEX
	fmove.l	#rz_mode,FPCR	;set RZ rounding mode


* A9. Scale X -> Y.
*     The mantissa is scaled to the desired number of significant
*     digits.  The excess digits are collected in INEX2. If mul,
*     Check d2 for excess 10 exponential value.  If not zero,
*     the iscale value would have caused the pwrten calculation
*     to overflow.  Only a negative iscale can cause this, so
*     multiply by 10^(d2), which is now only allowed to be 24,
*     with a multiply by 10^8 and 10^16, which is exact since
*     10^24 is exact.  If the input was denormalized, we must
*     create a busy stack frame with the mul command and the
*     two operands, and allow the fpu to complete the multiply.
*
* Register usage:
*	Input/Output
*	d0: FPCR with RZ mode/Unchanged
*	d2: 0 or 24/unchanged
*	d3: x/x
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA
*	d6: ILOG/Unchanged
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/final result
*	a1: ptr to PTENRM array/Unchanged
*	a2: x/x
*	fp0: float(ILOG)/X adjusted for SCALE (Y)
*	fp1: 10^ISCALE/Unchanged
*	fp2: x/x
*	F_SCR1:x/x
*	F_SCR2:Abs(X) with $3fff exponent/Unchanged
*	L_SCR1:x/x
*	L_SCR2:first word of X packed/Unchanged

A9_str:
	fmove.x	(a0),fp0	;load X from memory
	fabs.x	fp0		;use abs(X)
	tst.w	d5		;LAMBDA is in lower word of d5
	bne.b	sc_mul		;if neg (LAMBDA = 1), scale by mul
	fdiv.x	fp1,fp0		;calculate X / SCALE -> Y to fp0
	bra.b	A10_st		;branch to A10

sc_mul:
	tst.b	BINDEC_FLG(a6)	;check for denorm
	beq.b	A9_norm		;if norm, continue with mul
	fmovem.x fp1,-(a7)	;load ETEMP with 10^ISCALE
	move.l	8(a0),-(a7)	;load FPTEMP with input arg
	move.l	4(a0),-(a7)
	move.l	(a0),-(a7)
	move.l	#18,d3		;load count for busy stack
A9_loop:
	clr.l	-(a7)		;clear lword on stack
	dbf.w	d3,A9_loop
	move.b	VER_TMP(a6),(a7) ;write current version number
	move.b	#BUSY_SIZE-4,1(a7) ;write current busy size
	move.b	#$10,$44(a7)	;set fcefpte[15] bit
	move.w	#$0023,$40(a7)	;load cmdreg1b with mul command
	move.b	#$fe,$8(a7)	;load all 1s to cu savepc
	frestore (a7)+		;restore frame to fpu for completion
	fmul.x	36(a1),fp0	;multiply fp0 by 10^8
	fmul.x	48(a1),fp0	;multiply fp0 by 10^16
	bra.b	A10_st
A9_norm:
	tst.w	d2		;test for small exp case
	beq.b	A9_con		;if zero, continue as normal
	fmul.x	36(a1),fp0	;multiply fp0 by 10^8
	fmul.x	48(a1),fp0	;multiply fp0 by 10^16
A9_con:
	fmul.x	fp1,fp0		;calculate X * SCALE -> Y to fp0


* A10. Or in INEX.
*      If INEX is set, round error occured.  This is compensated
*      for by 'or-ing' in the INEX2 flag to the lsb of Y.
*
* Register usage:
*	Input/Output
*	d0: FPCR with RZ mode/FPSR with INEX2 isolated
*	d2: x/x
*	d3: x/x
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA
*	d6: ILOG/Unchanged
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/final result
*	a1: ptr to PTENxx array/Unchanged
*	a2: x/ptr to FP_SCR2(a6)
*	fp0: Y/Y with lsb adjusted
*	fp1: 10^ISCALE/Unchanged
*	fp2: x/x

A10_st:
	fmove.l	FPSR,d0		;get FPSR
	fmove.x	fp0,FP_SCR2(a6)	;move Y to memory
	lea.l	FP_SCR2(a6),a2	;load a2 with ptr to FP_SCR2
	btst.l	#9,d0		;check if INEX2 set
	beq.b	A11_st		;if clear, skip rest
	ori.l	#1,8(a2)	;or in 1 to lsb of mantissa
	fmove.x	FP_SCR2(a6),fp0	;write adjusted Y back to fpu


* A11. Restore original FPCR; set size ext.
*      Perform FINT operation in the user's rounding mode.  Keep
*      the size to extended.  The sintdo entry point in the sint
*      routine expects the FPCR value to be in USER_FPCR for
*      mode and precision.  The original FPCR is saved in L_SCR1.

A11_st:
	move.l	USER_FPCR(a6),L_SCR1(a6) ;save it for later
	andi.l	#$00000030,USER_FPCR(a6) ;set size to ext,
*					;block exceptions


* A12. Calculate YINT = FINT(Y) according to user's rounding mode.
*      The FPSP routine sintd0 is used.  The output is in fp0.
*
* Register usage:
*	Input/Output
*	d0: FPSR with AINEX cleared/FPCR with size set to ext
*	d2: x/x/scratch
*	d3: x/x
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA/Unchanged
*	d6: ILOG/Unchanged
*	d7: k-factor/Unchanged
*	a0: ptr for original operand/src ptr for sintdo
*	a1: ptr to PTENxx array/Unchanged
*	a2: ptr to FP_SCR2(a6)/Unchanged
*	a6: temp pointer to FP_SCR2(a6) - orig value saved and restored
*	fp0: Y/YINT
*	fp1: 10^ISCALE/Unchanged
*	fp2: x/x
*	F_SCR1:x/x
*	F_SCR2:Y adjusted for inex/Y with original exponent
*	L_SCR1:x/original USER_FPCR
*	L_SCR2:first word of X packed/Unchanged

A12_st:
	movem.l	d0-d1/a0-a1,-(a7)	;save regs used by sintd0
	move.l	L_SCR1(a6),-(a7)
	move.l	L_SCR2(a6),-(a7)
	lea.l	FP_SCR2(a6),a0		;a0 is ptr to F_SCR2(a6)
	fmove.x	fp0,(a0)		;move Y to memory at FP_SCR2(a6)
	tst.l	L_SCR2(a6)		;test sign of original operand
	bge.b	do_fint			;if pos, use Y
	or.l	#$80000000,(a0)		;if neg, use -Y
do_fint:
	move.l	USER_FPSR(a6),-(a7)
	bsr	sintdo			;sint routine returns int in fp0
	move.b	(a7),USER_FPSR(a6)
	add.l	#4,a7
	move.l	(a7)+,L_SCR2(a6)
	move.l	(a7)+,L_SCR1(a6)
	movem.l	(a7)+,d0-d1/a0-a1	;restore regs used by sint
	move.l	L_SCR2(a6),FP_SCR2(a6)	;restore original exponent
	move.l	L_SCR1(a6),USER_FPCR(a6) ;restore user's FPCR


* A13. Check for LEN digits.
*      If the int operation results in more than LEN digits,
*      or less than LEN -1 digits, adjust ILOG and repeat from
*      A6.  This test occurs only on the first pass.  If the
*      result is exactly 10^LEN, decrement ILOG and divide
*      the mantissa by 10.  The calculation of 10^LEN cannot
*      be inexact, since all powers of ten upto 10^27 are exact
*      in extended precision, so the use of a previous power-of-ten
*      table will introduce no error.
*
*
* Register usage:
*	Input/Output
*	d0: FPCR with size set to ext/scratch final = 0
*	d2: x/x
*	d3: x/scratch final = x
*	d4: LEN/LEN adjusted
*	d5: ICTR:LAMBDA/LAMBDA:ICTR
*	d6: ILOG/ILOG adjusted
*	d7: k-factor/Unchanged
*	a0: pointer into memory for packed bcd string formation
*	a1: ptr to PTENxx array/Unchanged
*	a2: ptr to FP_SCR2(a6)/Unchanged
*	fp0: int portion of Y/abs(YINT) adjusted
*	fp1: 10^ISCALE/Unchanged
*	fp2: x/10^LEN
*	F_SCR1:x/x
*	F_SCR2:Y with original exponent/Unchanged
*	L_SCR1:original USER_FPCR/Unchanged
*	L_SCR2:first word of X packed/Unchanged

A13_st:
	swap	d5		;put ICTR in lower word of d5
	tst.w	d5		;check if ICTR = 0
	bne.s	not_zr		;if non-zero, go to second test
*
* Compute 10^(LEN-1)
*
	fmove.s	FONE,fp2	;init fp2 to 1.0
	move.l	d4,d0		;put LEN in d0
	subq.l	#1,d0		;d0 = LEN -1
	clr.l	d3		;clr table index
l_loop:
	lsr.l	#1,d0		;shift next bit into carry
	bcc.b	l_next		;if zero, skip the mul
	fmul.x	(a1,d3),fp2	;mul by 10**(d3_bit_no)
l_next:
	add.l	#12,d3		;inc d3 to next pwrten table entry
	tst.l	d0		;test if LEN is zero
	bne.b	l_loop		;if not, loop
*
* 10^LEN-1 is computed for this test and A14.  If the input was
* denormalized, check only the case in which YINT > 10^LEN.
*
	tst.b	BINDEC_FLG(a6)	;check if input was norm
	beq.b	A13_con		;if norm, continue with checking
	fabs.x	fp0		;take abs of YINT
	bra.s	test_2
*
* Compare abs(YINT) to 10^(LEN-1) and 10^LEN
*
A13_con:
	fabs.x	fp0		;take abs of YINT
	fcmp.x	fp2,fp0		;compare abs(YINT) with 10^(LEN-1)
	fbge.w	test_2		;if greater, do next test
	subq.l	#1,d6		;subtract 1 from ILOG
	move.w	#1,d5		;set ICTR
	fmove.l	#rm_mode,FPCR	;set rmode to RM
	fmul.s	FTEN,fp2	;compute 10^LEN
	bra.w	A6_str		;return to A6 and recompute YINT
test_2:
	fmul.s	FTEN,fp2	;compute 10^LEN
	fcmp.x	fp2,fp0		;compare abs(YINT) with 10^LEN
	fblt.w	A14_st		;if less, all is ok, go to A14
	fbgt.w	fix_ex		;if greater, fix and redo
	fdiv.s	FTEN,fp0	;if equal, divide by 10
	addq.l	#1,d6		; and inc ILOG
	bra.b	A14_st		; and continue elsewhere
fix_ex:
	addq.l	#1,d6		;increment ILOG by 1
	move.w	#1,d5		;set ICTR
	fmove.l	#rm_mode,FPCR	;set rmode to RM
	bra.w	A6_str		;return to A6 and recompute YINT
*
* Since ICTR <> 0, we have already been through one adjustment,
* and shouldn't have another; this is to check if abs(YINT) = 10^LEN
* 10^LEN is again computed using whatever table is in a1 since the
* value calculated cannot be inexact.
*
not_zr:
	fmove.s	FONE,fp2	;init fp2 to 1.0
	move.l	d4,d0		;put LEN in d0
	clr.l	d3		;clr table index
z_loop:
	lsr.l	#1,d0		;shift next bit into carry
	bcc.b	z_next		;if zero, skip the mul
	fmul.x	(a1,d3),fp2	;mul by 10**(d3_bit_no)
z_next:
	add.l	#12,d3		;inc d3 to next pwrten table entry
	tst.l	d0		;test if LEN is zero
	bne.b	z_loop		;if not, loop
	fabs.x	fp0		;get abs(YINT)
	fcmp.x	fp2,fp0		;check if abs(YINT) = 10^LEN
	fbne.w	A14_st		;if not, skip this
	fdiv.s	FTEN,fp0	;divide abs(YINT) by 10
	addq.l	#1,d6		;and inc ILOG by 1
	addq.l	#1,d4		; and inc LEN
	fmul.s	FTEN,fp2	; if LEN++, the get 10^^LEN


* A14. Convert the mantissa to bcd.
*      The binstr routine is used to convert the LEN digit
*      mantissa to bcd in memory.  The input to binstr is
*      to be a fraction; i.e. (mantissa)/10^LEN and adjusted
*      such that the decimal point is to the left of bit 63.
*      The bcd digits are stored in the correct position in
*      the final string area in memory.
*
*
* Register usage:
*	Input/Output
*	d0: x/LEN call to binstr - final is 0
*	d1: x/0
*	d2: x/ms 32-bits of mant of abs(YINT)
*	d3: x/ls 32-bits of mant of abs(YINT)
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA/LAMBDA:ICTR
*	d6: ILOG
*	d7: k-factor/Unchanged
*	a0: pointer into memory for packed bcd string formation
*	    /ptr to first mantissa byte in result string
*	a1: ptr to PTENxx array/Unchanged
*	a2: ptr to FP_SCR2(a6)/Unchanged
*	fp0: int portion of Y/abs(YINT) adjusted
*	fp1: 10^ISCALE/Unchanged
*	fp2: 10^LEN/Unchanged
*	F_SCR1:x/Work area for final result
*	F_SCR2:Y with original exponent/Unchanged
*	L_SCR1:original USER_FPCR/Unchanged
*	L_SCR2:first word of X packed/Unchanged

A14_st:
	fmove.l	#rz_mode,FPCR	;force rz for conversion
	fdiv.x	fp2,fp0		;divide abs(YINT) by 10^LEN
	lea.l	FP_SCR1(a6),a0
	fmove.x	fp0,(a0)	;move abs(YINT)/10^LEN to memory
	move.l	4(a0),d2	;move 2nd word of FP_RES to d2
	move.l	8(a0),d3	;move 3rd word of FP_RES to d3
	clr.l	4(a0)		;zero word 2 of FP_RES
	clr.l	8(a0)		;zero word 3 of FP_RES
	move.l	(a0),d0		;move exponent to d0
	swap	d0		;put exponent in lower word
	beq.b	no_sft		;if zero, don't shift
	subi.l	#$3ffd,d0	;sub bias less 2 to make fract
	tst.l	d0		;check if > 1
	bgt.b	no_sft		;if so, don't shift
	neg.l	d0		;make exp positive
m_loop:
	lsr.l	#1,d2		;shift d2:d3 right, add 0s
	roxr.l	#1,d3		;the number of places
	dbf.w	d0,m_loop	;given in d0
no_sft:
	tst.l	d2		;check for mantissa of zero
	bne.b	no_zr		;if not, go on
	tst.l	d3		;continue zero check
	beq.b	zer_m		;if zero, go directly to binstr
no_zr:
	clr.l	d1		;put zero in d1 for addx
	addi.l	#$00000080,d3	;inc at bit 7
	addx.l	d1,d2		;continue inc
	andi.l	#$ffffff80,d3	;strip off lsb not used by 882
zer_m:
	move.l	d4,d0		;put LEN in d0 for binstr call
	addq.l	#3,a0		;a0 points to M16 byte in result
	bsr	binstr		;call binstr to convert mant


* A15. Convert the exponent to bcd.
*      As in A14 above, the exp is converted to bcd and the
*      digits are stored in the final string.
*
*      Digits are stored in L_SCR1(a6) on return from BINDEC as:
*
*  	 32               16 15                0
*	-----------------------------------------
*  	|  0 | e3 | e2 | e1 | e4 |  X |  X |  X |
*	-----------------------------------------
*
* And are moved into their proper places in FP_SCR1.  If digit e4
* is non-zero, OPERR is signaled.  In all cases, all 4 digits are
* written as specified in the 881/882 manual for packed decimal.
*
* Register usage:
*	Input/Output
*	d0: x/LEN call to binstr - final is 0
*	d1: x/scratch (0);shift count for final exponent packing
*	d2: x/ms 32-bits of exp fraction/scratch
*	d3: x/ls 32-bits of exp fraction
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA/LAMBDA:ICTR
*	d6: ILOG
*	d7: k-factor/Unchanged
*	a0: ptr to result string/ptr to L_SCR1(a6)
*	a1: ptr to PTENxx array/Unchanged
*	a2: ptr to FP_SCR2(a6)/Unchanged
*	fp0: abs(YINT) adjusted/float(ILOG)
*	fp1: 10^ISCALE/Unchanged
*	fp2: 10^LEN/Unchanged
*	F_SCR1:Work area for final result/BCD result
*	F_SCR2:Y with original exponent/ILOG/10^4
*	L_SCR1:original USER_FPCR/Exponent digits on return from binstr
*	L_SCR2:first word of X packed/Unchanged

A15_st:
	tst.b	BINDEC_FLG(a6)	;check for denorm
	beq.b	not_denorm
	ftst.x	fp0		;test for zero
	fbeq.w	den_zero	;if zero, use k-factor or 4933
	fmove.l	d6,fp0		;float ILOG
	fabs.x	fp0		;get abs of ILOG
	bra.b	convrt
den_zero:
	tst.l	d7		;check sign of the k-factor
	blt.b	use_ilog	;if negative, use ILOG
	fmove.s	F4933,fp0	;force exponent to 4933
	bra.b	convrt		;do it
use_ilog:
	fmove.l	d6,fp0		;float ILOG
	fabs.x	fp0		;get abs of ILOG
	bra.b	convrt
not_denorm:
	ftst.x	fp0		;test for zero
	fbne.w	not_zero	;if zero, force exponent
	fmove.s	FONE,fp0	;force exponent to 1
	bra.b	convrt		;do it
not_zero:
	fmove.l	d6,fp0		;float ILOG
	fabs.x	fp0		;get abs of ILOG
convrt:
	fdiv.x	24(a1),fp0	;compute ILOG/10^4
	fmove.x	fp0,FP_SCR2(a6)	;store fp0 in memory
	move.l	4(a2),d2	;move word 2 to d2
	move.l	8(a2),d3	;move word 3 to d3
	move.w	(a2),d0		;move exp to d0
	beq.b	x_loop_fin	;if zero, skip the shift
	subi.w	#$3ffd,d0	;subtract off bias
	neg.w	d0		;make exp positive
x_loop:
	lsr.l	#1,d2		;shift d2:d3 right
	roxr.l	#1,d3		;the number of places
	dbf.w	d0,x_loop	;given in d0
x_loop_fin:
	clr.l	d1		;put zero in d1 for addx
	addi.l	#$00000080,d3	;inc at bit 6
	addx.l	d1,d2		;continue inc
	andi.l	#$ffffff80,d3	;strip off lsb not used by 882
	move.l	#4,d0		;put 4 in d0 for binstr call
	lea.l	L_SCR1(a6),a0	;a0 is ptr to L_SCR1 for exp digits
	bsr	binstr		;call binstr to convert exp
	move.l	L_SCR1(a6),d0	;load L_SCR1 lword to d0
	move.l	#12,d1		;use d1 for shift count
	lsr.l	d1,d0		;shift d0 right by 12
	bfins	d0,FP_SCR1(a6){4:12} ;put e3:e2:e1 in FP_SCR1
	lsr.l	d1,d0		;shift d0 right by 12
	bfins	d0,FP_SCR1(a6){16:4} ;put e4 in FP_SCR1
	tst.b	d0		;check if e4 is zero
	beq.b	A16_st		;if zero, skip rest
	or.l	#opaop_mask,USER_FPSR(a6) ;set OPERR & AIOP in USER_FPSR


* A16. Write sign bits to final string.
*	   Sigma is bit 31 of initial value; RHO is bit 31 of d6 (ILOG).
*
* Register usage:
*	Input/Output
*	d0: x/scratch - final is x
*	d2: x/x
*	d3: x/x
*	d4: LEN/Unchanged
*	d5: ICTR:LAMBDA/LAMBDA:ICTR
*	d6: ILOG/ILOG adjusted
*	d7: k-factor/Unchanged
*	a0: ptr to L_SCR1(a6)/Unchanged
*	a1: ptr to PTENxx array/Unchanged
*	a2: ptr to FP_SCR2(a6)/Unchanged
*	fp0: float(ILOG)/Unchanged
*	fp1: 10^ISCALE/Unchanged
*	fp2: 10^LEN/Unchanged
*	F_SCR1:BCD result with correct signs
*	F_SCR2:ILOG/10^4
*	L_SCR1:Exponent digits on return from binstr
*	L_SCR2:first word of X packed/Unchanged

A16_st:
	clr.l	d0		;clr d0 for collection of signs
	andi.b	#$0f,FP_SCR1(a6) ;clear first nibble of FP_SCR1
	tst.l	L_SCR2(a6)	;check sign of original mantissa
	bge.b	mant_p		;if pos, don't set SM
	moveq.l	#2,d0		;move 2 in to d0 for SM
mant_p:
	tst.l	d6		;check sign of ILOG
	bge.b	wr_sgn		;if pos, don't set SE
	addq.l	#1,d0		;set bit 0 in d0 for SE
wr_sgn:
	bfins	d0,FP_SCR1(a6){0:2} ;insert SM and SE into FP_SCR1

* Clean up and restore all registers used.

	fmove.l	#0,FPSR		;clear possible inex2/ainex bits
	fmovem.x (a7)+,fp0-fp2
	movem.l	(a7)+,d2-d7/a2
	rts

	end
