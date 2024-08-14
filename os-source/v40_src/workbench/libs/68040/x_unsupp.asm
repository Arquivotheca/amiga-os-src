*
* $Id: x_unsupp.asm,v 0.3 91/07/03 10:27:39 mks Exp $
*
* $Log:	x_unsupp.asm,v $
* Revision 0.3  91/07/03  10:27:39  mks
* First pass at doing branch optimizations
* 
* Revision 0.2  91/06/26  13:27:18  mks
* Removed calls to "check_force" as per MOTO FAX of 6/14/91
*
* Revision 0.1  91/06/26  08:58:29  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	x_unsupp.sa 3.2 4/26/91
*
*	fpsp_unsupp --- FPSP handler for unsupported data type exception
*
* Trap vector #55	(See table 8-1 Mc68030 User's manual).
* Invoked when the user program encounters a data format (packed) that
* hardware does not support or a data type (denormalized numbers or un-
* normalized numbers).
* Normalizes denorms and unnorms, unpacks packed numbers then stores
* them back into the machine to let the 040 finish the operation.
*
* Unsupp calls two routines:
* 	1. get_op -  gets the operand(s)
* 	2. res_func - restore the function back into the 040 or
* 			if fmove.p fpm,<ea> then pack source (fpm)
* 			and store in users memory <ea>.
*
*  Input: Long fsave stack frame
*
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

X_UNSUPP	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

	xref	get_op
	xref	res_func
	xref	gen_except
	xref	fpsp_fmt_error

	xdef	fpsp_unsupp
fpsp_unsupp:
*
	link		a6,#-LOCAL_SIZE
	fsave		-(a7)
	movem.l		d0-d1/a0-a1,USER_DA(a6)
	fmovem.x	fp0-fp3,USER_FP0(a6)
	fmovem.l	fpcr/fpsr/fpiar,USER_FPCR(a6)


	move.b		(a7),VER_TMP(a6) ;save version number
	move.b		(a7),d0		;test for valid version num
	andi.b		#$f0,d0		;test for $4x
	cmpi.b		#VER_4,d0	;must be $4x or exit
	bne.l		fpsp_fmt_error

	fmove.l		#0,FPSR		;clear all user status bits
	fmove.l		#0,FPCR		;clear all user control bits
*
*	The following lines are used to ensure that the FPSR
*	exception byte and condition codes are clear before proceeding,
*	except in the case of fmove, which leaves the cc's intact.
*
unsupp_con:
	move.l		USER_FPSR(a6),d1
	btst		#5,CMDREG1B(a6)	;looking for fmove out
	bne.s		fmove_con
	and.l		#$FF00FF,d1	;clear all but aexcs and qbyte
	bra.b		end_fix
fmove_con:
	and.l		#$0FFF40FF,d1	;clear all but cc's, snan bit, aexcs, and qbyte
end_fix:
	move.l		d1,USER_FPSR(a6)

	st		UFLG_TMP(a6)	;set flag for unsupp data

	bsr		get_op		;everything okay, go get operand(s)
	bsr		res_func	;fix up stack frame so can restore it
	clr.l		-(a7)
	move.b		VER_TMP(a6),(a7) ;move idle fmt word to top of stack
	bra		gen_except
*
	end
