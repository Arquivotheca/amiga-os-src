******************************************************************************
* This file contains Postscript Math Operators implemented in 680x0 Assembler
*
* started 02/MAY/91, written by L. Vanhelsuwe			(C) Commodore-Amiga
******************************************************************************

**!! WARNING: This file assumes pso_SIZEOF is 8 (using magic number instead of label)
**!! WARNING: This file uses 68020 instructions

	include	"include:exec/types.i"
	include	errors.i
	include	objects.i
	include	stack.i
	include	context.i
	include	macros.i

	SECTION	text,CODE	; same section as C code to prevent binary fragmentation

	XDEF	_ps_add
	XDEF	_ps_sub
	XDEF	_ps_mul

	XDEF	_clear_AEXC
	XDEF	_check_INEX
	XDEF	_set_round_mode

	XDEF	_sincosr

	XDEF	_TransformPoint,_DTransformPoint
	XDEF	_ArrayTransformPoint,_ArrayDTransformPoint
	
FPU_UNFL	equ	11		;FPU SR CC bit #
FPU_OVFL	equ	12		;FPU SR CC bit #

*----------------------------------------------------------------------------
* error = _ps_add(DPSContext);
*  D0				4(SP)
*----------------------------------------------------------------------------

_ps_add		move.l	4(sp),a1			;-> Context pointer

			NEED_ARGS	a1,2			;need 2 numeric arguments

			OPERSP	a1,a0				;get pointer to TOS of Operand stack

			moveq	#TYPE_MASK,d0		;get argument types and do type checking
			moveq	#TYPE_MASK,d1
			and.b	pso_type(a0),d0		;TYPE_INT & TYPE_REAL are 1 and 2 resp.
			and.b	pso_type+8(a0),d1
			subq.b	#1,d0				;1 or 2 becomes 0 or 1
			subq.b	#1,d1

			lsr.b	#1,d0				;TOS operand is num when no 1bits left
			bne		typecheck_err
			bcs		arg1_is_real		;if C=1 operand was TYPE_REAL

			lsr.b	#1,d1				;NOS operand is num when no 1bits left
			bne		typecheck_err
			bcs.s	arg2_is_real
;-----------
do_int_add	move.l	pso_intval(a0),d0	;SIGNED add integers together
			add.l	pso_intval+8(a0),d0
			bvs		int_undef_res_err		;if no overflow occured,
			move.l	d0,pso_intval+8(a0)	;store result and exit
			bra		pop_arg2			;else, do add in floating point space
;-----------
int_undef_res_err
			fmove.l	pso_intval(a0),fp0	;get both args as integers !
			fadd.l	pso_intval+8(a0),fp0
			bra.s	store_real
;-----------
arg1_is_real lsr.b	#1,d1				;NOS operand is num when no 1bits left
			bne		typecheck_err
			bcs		add_2reals
			
			fmove.s	pso_realval(a0),fp0		;ARG1 is REAL
			fadd.l	pso_realval+8(a0),fp0	;ARG2 is INT
			bra.s	store_real
;-----------
arg2_is_real fmove.l pso_realval(a0),fp0	;ARG1 is INT
			fadd.s pso_realval+8(a0),fp0	;ARG2 is REAL
			bra.s	store_real
;-----------
add_2reals	fmove.s	pso_realval(a0),fp0		;both args are REAL
			fadd.s	pso_realval+8(a0),fp0	;do the add in EXTENDED prec.

store_real	fmove.s	fp0,d1					;try to jam extended prec in single

			fmove.l	fpsr,d0					;see if FPU add overflowed....
			bftst	d0,{19:2}				;test underfl & overfl bits
			bne		undef_res_err

			move.l	d1,pso_realval+8(a0)	;no, store correct result
			move.b	#TYPE_REAL,pso_type+8(a0)	;promote int to real.
;-----------
pop_arg2	POPOPER	a1						;discard old ARG2
			moveq	#ERR_OK,d0
			rts
;-------------------------------------------
typecheck_err
			moveq	#ERR_typecheck,d0
			rts
undef_res_err
			moveq	#ERR_undefinedresult,d0
			rts
ostack_uflow
			moveq	#ERR_stackunderflow,d0
			rts
*----------------------------------------------------------------------------
* error = ps_sub(DPSContext);
*----------------------------------------------------------------------------

_ps_sub		move.l	4(sp),a1			;-> Context pointer
			NEED_ARGS	a1,2			;need 2 numeric arguments
			OPERSP	a1,a0				;get pointer to TOS of Operand stack

			moveq	#TYPE_MASK,d0		;get argument types and do type checking
			moveq	#TYPE_MASK,d1
			and.b	pso_type+8(a0),d0	;TYPE_INT & TYPE_REAL are 1 and 2 resp.
			and.b	pso_type(a0),d1
			subq.b	#1,d0				;1 or 2 becomes 0 or 1
			subq.b	#1,d1

			lsr.b	#1,d0				;TOS operand is num when no 1bits left
			bne.s	typecheck_err
			bcs.s	real_arg1			;if C=1 operand was TYPE_REAL

			lsr.b	#1,d1				;NOS operand is num when no 1bits left
			bne.s	typecheck_err
			bcs.s	real_arg2
;-----------
do_int_sub	move.l	pso_intval+8(a0),d0	;SIGNED subtract integers together
			sub.l	pso_intval(a0),d0
			bvs		intsub_undef		;if no overflow occured,
			move.l	d0,pso_intval+8(a0)	;store result and exit
			bra		pop_arg2			;else, do add in floating point space
;-----------
intsub_undef
			fmove.l	pso_intval+8(a0),fp0	;get both args as integers !
			fsub.l	pso_intval(a0),fp0
			bra		store_real
;-----------
real_arg1	lsr.b	#1,d1					;NOS operand is num when no 1bits left
			bne		typecheck_err
			bcs.s	sub_2reals
;-----------
			fmove.s	pso_realval+8(a0),fp0	;ARG1 is REAL
			fsub.l	pso_realval(a0),fp0		;ARG2 is INT
			bra		store_real
;-----------
real_arg2	fmove.l pso_realval+8(a0),fp0	;ARG1 is INT
			fsub.s pso_realval(a0),fp0		;ARG2 is REAL
			bra		store_real
;-----------
sub_2reals	fmove.s	pso_realval+8(a0),fp0	;both args are REAL
			fsub.s	pso_realval(a0),fp0		;do the sub in EXTENDED prec.
			bra		store_real

*----------------------------------------------------------------------------
* error = ps_mul(DPSContext);
*----------------------------------------------------------------------------
_ps_mul		move.l	4(sp),a1			;-> Context pointer
			NEED_ARGS	a1,2			;need 2 numeric arguments
			OPERSP	a1,a0				;get pointer to TOS of Operand stack

			moveq	#TYPE_MASK,d0		;get argument types and do type checking
			moveq	#TYPE_MASK,d1
			and.b	pso_type+8(a0),d0	;TYPE_INT & TYPE_REAL are 1 and 2 resp.
			and.b	pso_type(a0),d1
			subq.b	#1,d0				;1 or 2 becomes 0 or 1
			subq.b	#1,d1

			lsr.b	#1,d0				;TOS operand is num when no 1bits left
			bne		typecheck_err
			bcs.s	breal_arg1			;if C=1 operand was TYPE_REAL

			lsr.b	#1,d1				;NOS operand is num when no 1bits left
			bne		typecheck_err
			bcs.s	breal_arg2
;-----------
do_int_mul	move.l	pso_intval+8(a0),d0	;SIGNED subtract integers together
			muls.l	pso_intval(a0),d0		;68020+ INSTRUCTION **!!
			bvs		intmul_undef		;if no overflow occured,
			move.l	d0,pso_intval+8(a0)	;store result and exit
			bra		pop_arg2			;else, do add in floating point space
;-----------
intmul_undef
			fmove.l	pso_intval+8(a0),fp0	;get both args as integers !
			fmul.l	pso_intval(a0),fp0
			bra		store_real
;-----------
breal_arg1	lsr.b	#1,d1					;NOS operand is num when no 1bits left
			bne		typecheck_err
			bcs.s	mul_2reals
;-----------
			fmove.s	pso_realval+8(a0),fp0	;ARG1 is REAL
			fmul.l	pso_realval(a0),fp0		;ARG2 is INT
			bra		store_real
;-----------
breal_arg2	fmove.l pso_realval+8(a0),fp0	;ARG1 is INT
			fmul.s	pso_realval(a0),fp0		;ARG2 is REAL
			bra		store_real
;-----------
mul_2reals	fmove.s	pso_realval+8(a0),fp0	;both args are REAL
			fmul.s	pso_realval(a0),fp0		;do the sub in EXTENDED prec.
			bra		store_real
*----------------------------------------------------------------------------
* Clear/Check all FPU error codes.
* clear_AEXC should be called before executing a long sequence of FPU calculations
* after which check_INEX can be called to determine success or failure of the
* entire calculation.
*----------------------------------------------------------------------------
_clear_AEXC	moveq	#0,d0
			fmove.l	d0,fpsr					;clear all status bits
			rts

_check_INEX	moveq	#0,d0					;assume FALSE
			fmove.l	fpsr,d1
			bftst	d1,{19:2}				;test underfl & overfl bits
			beq		clean_calc
			moveq	#-1,d0
clean_calc	rts

; we want (int)value to come out the same as floor(value), no rounding up
_set_round_mode:
			moveq.l	#%10000,d0				round towards 0
			fmove.l	d0,fpcr
			rts

*----------------------------------------------------------------------------


*----------------------------------------------------------------------------
; calculates sin*r cos*r
*----------------------------------------------------------------------------
_sincosr
			fsincos.d 4(sp),fp0:fp1		; fp0 - cos, fp1 - sin
			fmove.d 20(sp),fp2			; fp2 - radius
			move.l 12(sp),a0			; &c variable
			move.l 16(sp),a1			; &s variable
			moveq #0,d0
			fmul.d 20(sp),fp0
			fmul.d 20(sp),fp1
			fmove.d fp0,(a0)			; *c = fp0*radius
			fmove.d fp1,(a1)			; *s = fp1*radius
			rts

*----------------------------------------------------------------------------
; void TransformPoint( &matrix,x,y,&resultx,&resulty )
*----------------------------------------------------------------------------
_TransformPoint:
			fmovem.x	fp2/fp3,-(sp)	args now at 28(sp)
			movea.l		28(sp),a0		source matrix
			fmove.s		32(sp),fp0		x co-ordinate
			fmove.s		36(sp),fp1		y co-ordinate

			fmove.s		(a0),fp2		fp2=m[0]*x+m[2]*y+m[4]
			fmul.x		fp0,fp2
			fmove.s		8(a0),fp3
			fmul.x		fp1,fp3
			fadd.x		fp3,fp2
			fmove.s		16(a0),fp3
			fadd.x		fp3,fp2
			movea.l		40(sp),a1		get result address
			fmove.s		fp2,(a1)		store it

			fmove.s		4(a0),fp2		fp2=m[1]*x+m[3]*y+m[5]
			fmul.x		fp0,fp2
			fmove.s		12(a0),fp3
			fmul.x		fp1,fp3
			fadd.x		fp3,fp2
			fmove.s		20(a0),fp3
			fadd.x		fp3,fp2
			movea.l		44(sp),a1		get result address
			fmove.s		fp2,(a1)		store it

			fmovem.x	(sp)+,fp2/fp3
			rts


*----------------------------------------------------------------------------
; void DTransformPoint( &matrix,x,y,&resultx,&resulty )
*----------------------------------------------------------------------------
_DTransformPoint:
			fmovem.x	fp2/fp3,-(sp)	args now at 28(sp)
			movea.l		28(sp),a0		source matrix
			fmove.s		32(sp),fp0		x co-ordinate
			fmove.s		36(sp),fp1		y co-ordinate

			fmove.s		(a0),fp2		fp2=m[0]*x+m[2]*y
			fmul.x		fp0,fp2
			fmove.s		8(a0),fp3
			fmul.x		fp1,fp3
			fadd.x		fp3,fp2
			movea.l		40(sp),a1		get result address
			fmove.s		fp2,(a1)		store it

			fmove.s		4(a0),fp2		fp2=m[1]*x+m[3]*y
			fmul.x		fp0,fp2
			fmove.s		12(a0),fp3
			fmul.x		fp1,fp3
			fadd.x		fp3,fp2
			movea.l		44(sp),a1		get result address
			fmove.s		fp2,(a1)		store it

			fmovem.x	(sp)+,fp2/fp3
			rts


*----------------------------------------------------------------------------
; void TransformPointArray( &matrix,&co-ordinate)
*----------------------------------------------------------------------------
_ArrayTransformPoint:
			movem.l		4(sp),a0/a1		a0=matrix, a1=points
			movem.l		(a1),d0/d1		d0=x, d1=y

			fmove.s		d0,fp0			x co-ordinate
			fmove.s		d1,fp1			y co-ordinate
			fmul.s		(a0),fp0		x'=m[0]*x+m[2]*y+m[4]
			fmul.s		8(a0),fp1
			fadd.x		fp1,fp0
			fadd.s		16(a0),fp0
			fmove.s		fp0,(a1)		stash x result

			fmove.s		d0,fp0			x co-ordinate
			fmove.s		d1,fp1			y co-ordinate
			fmul.s		4(a0),fp0		y'=m[1]*x+m[3]*y
			fmul.s		12(a0),fp1
			fadd.x		fp1,fp0
			fadd.s		20(a0),fp0
			fmove.s		fp0,4(a1)		stash y result

			rts

*----------------------------------------------------------------------------
; void DTransformPointArray( &matrix,&co-ordinate)
*----------------------------------------------------------------------------
_ArrayDTransformPoint:
			movem.l		4(sp),a0/a1		a0=matrix, a1=points
			movem.l		(a1),d0/d1		d0=x, d1=y

			fmove.s		d0,fp0			x co-ordinate
			fmove.s		d1,fp1			y co-ordinate
			fmul.s		(a0),fp0		x'=m[0]*x+m[2]*y+m[4]
			fmul.s		8(a0),fp1
			fadd.x		fp1,fp0
			fmove.s		fp0,(a1)		stash x result

			fmove.s		d0,fp0			x co-ordinate
			fmove.s		d1,fp1			y co-ordinate
			fmul.s		4(a0),fp0		y'=m[1]*x+m[3]*y
			fmul.s		12(a0),fp1
			fadd.x		fp1,fp0
			fmove.s		fp0,4(a1)		stash y result

			rts


			END
