*
* $Id: x_operr.asm,v 0.4 91/07/03 10:27:17 mks Exp $
*
* $Log:	x_operr.asm,v $
* Revision 0.4  91/07/03  10:27:17  mks
* First pass at doing branch optimizations
* 
* Revision 0.3  91/07/02  15:53:00  mks
* Did optimizations for unified memory since the Amiga system
* runs as unified address space.
*
* Revision 0.2  91/06/26  13:26:49  mks
* Removed calls to "check_force" as per MOTO FAX of 6/14/91
*
* Revision 0.1  91/06/26  08:58:55  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	x_operr.sa 3.4 4/26/91
*
*	fpsp_operr --- FPSP handler for operand error exception
*
*	See 68040 User's Manual pp. 9-44f
*
* Note 1: For trap disabled 040 does the following:
* If the dest is a fp reg, then an extended precision non_signaling
* NAN is stored in the dest reg.  If the dest format is b, w, or l and
* the source op is a NAN, then garbage is stored as the result (actually
* the upper 32 bits of the mantissa are sent to the integer unit). If
* the dest format is integer (b, w, l) and the operr is caused by
* integer overflow, or the source op is inf, then the result stored is
* garbage.
* There are three cases in which operr is incorrectly signaled on the
* 040.  This occurs for move_out of format b, w, or l for the largest
* negative integer (-2^7 for b, -2^15 for w, -2^31 for l).
*
*	  On opclass = 011 fmove.(b,w,l) that causes a conversion
*	  overflow -> OPERR, the exponent in wbte (and fpte) is:
*		byte    56 - (62 - exp)
*		word    48 - (62 - exp)
*		long    32 - (62 - exp)
*
*			where exp = (true exp) - 1
*
*  So, wbtemp and fptemp will contain the following on erroneoulsy
*	  signalled operr:
*			fpts = 1
*			fpte = $4000  (15 bit externally)
*		byte	fptm = $ffffffff ffffff80
*		word	fptm = $ffffffff ffff8000
*		long	fptm = $ffffffff 80000000
*
* Note 2: For trap enabled 040 does the following:
* If the inst is move_out, then same as Note 1.
* If the inst is not move_out, the dest is not modified.
* The exceptional operand is not defined for integer overflow
* during a move_out.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

X_OPERR	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

	xref	mem_write
	xref	real_operr
	xref	real_inex
	xref	get_fline
	xref	fpsp_done
	xref	reg_dest

	xdef	fpsp_operr
fpsp_operr:
*
	link		a6,#-LOCAL_SIZE
	fsave		-(a7)
	movem.l		d0-d1/a0-a1,USER_DA(a6)
	fmovem.x	fp0-fp3,USER_FP0(a6)
	fmovem.l	fpcr/fpsr/fpiar,USER_FPCR(a6)

*
* Check if this is an opclass 3 instruction.
*  If so, fall through, else branch to operr_end
*
	btst.b	#TFLAG,T_BYTE(a6)
	beq.b	operr_end

*
* If the destination size is B,W,or L, the operr must be
* handled here.
*
	move.l	CMDREG1B(a6),d0
	bfextu	d0{3:3},d0	;0=long, 4=word, 6=byte
	cmpi.b	#0,d0		;determine size; check long
	beq.s	operr_long
	cmpi.b	#4,d0		;check word
	beq.w	operr_word
	cmpi.b	#6,d0		;check byte
	beq.w	operr_byte

*
* The size is not B,W,or L, so the operr is handled by the
* kernel handler.  Set the operr bits and clean up, leaving
* only the integer exception frame on the stack, and the
* fpu in the original exceptional state.
*
operr_end:
	bset.b		#operr_bit,FPSR_EXCEPT(a6)
	bset.b		#aiop_bit,FPSR_AEXCEPT(a6)

	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_operr

operr_long:
	moveq.l	#4,d1		;write size to d1
	move.b	STAG(a6),d0	;test stag for nan
	andi.b	#$e0,d0		;clr all but tag
	cmpi.b	#$60,d0		;check for nan
	beq	operr_nan
	cmpi.l	#$80000000,FPTEMP_LO(a6) ;test if ls lword is special
	bne.b	chklerr		;if not equal, check for incorrect operr
	bsr	check_upper	;check if exp and ms mant are special
	tst.l	d0
	bne.b	chklerr		;if d0 is true, check for incorrect operr
	move.l	#$80000000,d0	;store special case result
	bsr	operr_store
	bra.w	not_enabled	;clean and exit
*
*	CHECK FOR INCORRECTLY GENERATED OPERR EXCEPTION HERE
*
chklerr:
	move.w	FPTEMP_EX(a6),d0
	and.w	#$7FFF,d0	;ignore sign bit
	cmp.w	#$3FFE,d0	;this is the only possible exponent value
	bne.b	chklerr2
fixlong:
	move.l	FPTEMP_LO(a6),d0
	bsr	operr_store
	bra.w	not_enabled
chklerr2:
	move.w	FPTEMP_EX(a6),d0
	and.w	#$7FFF,d0	;ignore sign bit
	cmp.w	#$4000,d0
	bhs.w	store_max	;exponent out of range

	move.l	FPTEMP_LO(a6),d0
	and.l	#$7FFF0000,d0	;look for all 1's on bits 30-16
	cmp.l	#$7FFF0000,d0
	beq.b	fixlong

	tst.l	FPTEMP_LO(a6)
	bpl.b	chklepos
	cmp.l	#$FFFFFFFF,FPTEMP_HI(a6)
	beq.b	fixlong
	bra.w	store_max
chklepos:
	tst.l	FPTEMP_HI(a6)
	beq.b	fixlong
	bra.w	store_max

operr_word:
	moveq.l	#2,d1		;write size to d1
	move.b	STAG(a6),d0	;test stag for nan
	andi.b	#$e0,d0		;clr all but tag
	cmpi.b	#$60,d0		;check for nan
	beq.w	operr_nan
	cmpi.l	#$ffff8000,FPTEMP_LO(a6) ;test if ls lword is special
	bne.b	chkwerr		;if not equal, check for incorrect operr
	bsr	check_upper	;check if exp and ms mant are special
	tst.l	d0
	bne.b	chkwerr		;if d0 is true, check for incorrect operr
	move.l	#$80000000,d0	;store special case result
	bsr	operr_store
	bra.w	not_enabled	;clean and exit
*
*	CHECK FOR INCORRECTLY GENERATED OPERR EXCEPTION HERE
*
chkwerr:
	move.w	FPTEMP_EX(a6),d0
	and.w	#$7FFF,d0	;ignore sign bit
	cmp.w	#$3FFE,d0	;this is the only possible exponent value
	bne.b	store_max
	move.l	FPTEMP_LO(a6),d0
	swap	d0
	bsr	operr_store
	bra.w	not_enabled

operr_byte:
	moveq.l	#1,d1		;write size to d1
	move.b	STAG(a6),d0	;test stag for nan
	andi.b	#$e0,d0		;clr all but tag
	cmpi.b	#$60,d0		;check for nan
	beq.b	operr_nan
	cmpi.l	#$ffffff80,FPTEMP_LO(a6) ;test if ls lword is special
	bne.b	chkberr		;if not equal, check for incorrect operr
	bsr	check_upper	;check if exp and ms mant are special
	tst.l	d0
	bne.b	chkberr		;if d0 is true, check for incorrect operr
	move.l	#$80000000,d0	;store special case result
	bsr.s	operr_store
	bra.w	not_enabled	;clean and exit
*
*	CHECK FOR INCORRECTLY GENERATED OPERR EXCEPTION HERE
*
chkberr:
	move.w	FPTEMP_EX(a6),d0
	and.w	#$7FFF,d0	;ignore sign bit
	cmp.w	#$3FFE,d0	;this is the only possible exponent value
	bne.b	store_max
	move.l	FPTEMP_LO(a6),d0
	asl.l	#8,d0
	swap	d0
	bsr.s	operr_store
	bra.w	not_enabled

*
* This operr condition is not of the special case.  Set operr
* and aiop and write the portion of the nan to memory for the
* given size.
*
operr_nan:
	or.l	#opaop_mask,USER_FPSR(a6) ;set operr & aiop

	move.l	ETEMP_HI(a6),d0	;output will be from upper 32 bits
	bsr.s	operr_store
	bra	end_operr
*
* Store_max loads the max pos or negative for the size, sets
* the operr and aiop bits, and clears inex and ainex, incorrectly
* set by the 040.
*
store_max:
	or.l	#opaop_mask,USER_FPSR(a6) ;set operr & aiop
	bclr.b	#inex2_bit,FPSR_EXCEPT(a6)
	bclr.b	#ainex_bit,FPSR_AEXCEPT(a6)
	fmove.l	#0,FPSR

	tst.w	FPTEMP_EX(a6)	;check sign
	blt.b	load_neg
	move.l	#$7fffffff,d0
	bsr.s	operr_store
	bra.s	end_operr
load_neg:
	move.l	#$80000000,d0
	bsr.s	operr_store
	bra.s	end_operr

*
* This routine stores the data in d0, for the given size in d1,
* to memory or data register as required.  A read of the fline
* is required to determine the destination.
*
operr_store:
	move.l	d0,L_SCR1(a6)	;move write data to L_SCR1

 ifne COPY_IN_OUT
	move.l	USER_FPIAR(a6),a0	;opcode address
	move.w	(a0),d0			; Get opcode into low word...
 else
	move.l	d1,-(a7)	;save register size
	bsr	get_fline	;fline returned in d0
	move.l	(a7)+,d1
 endc

	bftst	d0{26:3}		;if mode is zero, dest is Dn
	bne.b	dest_mem
*
* Destination is Dn.  Get register number from d0. Data is on
* the stack at (a7). D1 has size: 1=byte,2=word,4=long/single
*
	andi.l	#7,d0		;isolate register number
	cmpi.l	#4,d1
	beq.b	op_long		;the most frequent case
	cmpi.l	#2,d1
	bne.b	op_con
	or.l	#8,d0
	bra.b	op_con
op_long:
	or.l	#$10,d0
op_con:
	move.l	d0,d1		;format size:reg for reg_dest
	bra	reg_dest	;call to reg_dest returns to caller
*				;of operr_store
*
* Destination is memory.  Get <ea> from integer exception frame
* and call mem_write.
*
dest_mem:
	lea.l	L_SCR1(a6),a0	;put ptr to write data in a0
	move.l	EXC_EA(a6),a1	;put user destination address in a1
	move.l	d1,d0		;put size in d0
	bra	mem_write	; Tail recursion...
*
* Check the exponent for $c000 and the upper 32 bits of the
* mantissa for $ffffffff.  If both are true, return d0 clr
* and store the lower n bits of the least lword of FPTEMP
* to d0 for write out.  If not, it is a real operr, and set d0.
*
check_upper:
	cmpi.l	#$ffffffff,FPTEMP_HI(a6) ;check if first byte is all 1's
	bne.b	true_operr	;if not all 1's then was true operr
	cmpi.w	#$c000,FPTEMP_EX(a6) ;check if incorrectly signalled
	beq.b	not_true_operr	;branch if not true operr
	cmpi.w	#$bfff,FPTEMP_EX(a6) ;check if incorrectly signalled
	beq.b	not_true_operr	;branch if not true operr
true_operr:
	move.l	#1,d0		;signal real operr
	rts
not_true_operr:
	clr.l	d0		;signal no real operr
	rts

*
* End_operr tests for operr enabled.  If not, it cleans up the stack
* and does an rte.  If enabled, it cleans up the stack and branches
* to the kernel operr handler with only the integer exception
* frame on the stack and the fpu in the original exceptional state
* with correct data written to the destination.
*
end_operr:
	btst.b		#operr_bit,FPCR_ENABLE(a6)
	beq.b		not_enabled
enabled:
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_operr

not_enabled:
*
* It is possible to have either inex2 or inex1 exceptions with the
* operr.  If the inex enable bit is set in the FPCR, and either
* inex2 or inex1 occured, we must clean up and branch to the
* real inex handler.
*
ck_inex:
	move.b	FPCR_ENABLE(a6),d0
	and.b	FPSR_EXCEPT(a6),d0
	andi.b	#$3,d0
	beq.s	operr_exit
*
* Inexact enabled and reported, and we must take an inexact exception.
*
take_inex:
	move.b		#INEX_VEC,EXC_VEC+1(a6)
	move.l		USER_FPSR(a6),FPSR_SHADOW(a6)
	or.l		#sx_mask,E_BYTE(a6)
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_inex
*
* Since operr is only an E1 exception, there is no need to frestore
* any state back to the fpu.
*
operr_exit:
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	unlk		a6
	bra		fpsp_done

	end
