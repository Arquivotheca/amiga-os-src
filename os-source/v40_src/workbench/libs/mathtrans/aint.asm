

*          ***********************************************************
*		   **														**
*          **  Math (RAM) Link Library Assembly Language Interface  ** 
*          **                                           		    ** 
*          ***********************************************************


***********************************************************************
**                                                                    *
**   Copyright 1984, Amiga Computer Inc.   All rights reserved.       *
**   No part of this program may be reproduced, transmitted,          *
**   transcribed, stored in retrieval system, or translated into      *
**   any language or computer language, in any form or by any         *
**   means, electronic, mechanical, magnetic, optical, chemical,      *
**   manual or otherwise, without the prior written permission of     *
**   Amiga Computer Incorporated, 3350 Scott Blvd, Bld #7,            *
**   Santa Clara, CA 95051                                            *
**                                                                    *
***********************************************************************

* 15:34 03-Jun-85 LAH Broke up math library to create ROM, RAM and Link.
* 15:50 14-Apr-85 KBS Converted to V25 standard compatible library.
* 09:35 28-Jan-85 MJS moved float/string conversion to V2xstr.asm
* 14:41  1-Jan-85 MJS added float/string conversion
* 16:00 12-Dec-84 MJS first version
* 10:56 30-Dec-84 MJS version for RAM or ROM math library


		xref	_MathBase
		xref	FFPCPYRTB					* Keep Motorola happy

*		xref	FFPFPI,FFPIFP				* Basic math functions
*		xref	FFPCMP,FFPTST,FFPABS,FFPNEG
*	 	xref	FFPADD,FFPSUB,FFPMUL,FFPDIV	* Note: Replaced with "DUP" entries below

		xref	FFPASIN,FFPACOS,FFPATAN		* Transcendental math functions
		xref	FFPSIN,FFPCOS
		xref	FFPTAN,FFPSINCS,FFPSINH
		xref	FFPCOSH,FFPTANH,FFPEXP
		xref	FFPLOG,FFPLOG10,FFPPWR
		xref	FFPTIEEE,FFPFIEEE


		xdef	FFPFPI,FFPIFP
		xdef	FFPCMP,FFPTST,FFPABS,FFPNEG
		xdef	FFPADD,FFPSUB,FFPMUL,FFPDIV

*------ Basic Math Function Assembly Language Entry Points  --------------------------
 
*		xdef	MFSPFix						* (float)D0 to (int)D0
*		xdef	MFSPFlt						* (int)D0 to (float)D0
*		xdef	MFSPCmp						* cc: (float)D1 - (float)D0
*		xdef	MFSPTst						* cc: (float)D1 - (float)0.00
*		xdef	MFSPAbs						* absolute value of (float)D0
*		xdef	MFSPNeg						* negate sign of (float)D0
*		xdef	MFSPAdd						* (float)D1 + (float)D0
*		xdef	MFSPSub						* (float)D1 - (float)D0
*		xdef	MFSPMul						* (float)D1 X (float)D0
*		xdef	MFSPDiv						* (float)D1 / (float)D0

*------ Transcendental Math Function Assembly Language Entry Points --------------------------

		xdef	MFSPAsin					* Arcsine of (float)D0
		xdef	MFSPAcos					* Arccosine of (float)D0
		xdef	MFSPAtan					* Arctangent of (float)D0
		xdef	MFSPSin						* Sine of (float)D0
		xdef	MFSPCos						* Cosine of (float)D0
		xdef	MFSPTan						* Tangent of (float)D0
		xdef	MFSPSincos					* Sine and cosine of (float)D0
		xdef	MFSPSinh					* Hyperbolic sine of (float)D0
		xdef	MFSPCosh					* Hyperbolic cosine of (float)D0
		xdef	MFSPTanh					* Hyperbolic tangent of (float)D0
		xdef	MFSPExp						* Exponential (base e) of (float)D0
		xdef	MFSPLog						* Natural logarithm of (float)D0
		xdef	MFSPLog10					* Base 10 logarithm of (float)D0
		xdef	MFSPPow						* (float)D0 ** (float)D1
		xdef	MFSPTieee					* Convert (float)D0 to IEEE SP standard
		xdef	MFSPFieee					* Convert (IEEE SP standard) to float
 
*
*------ execute single operand library function  (must be jumped to)
*
*  at entry :   D0 has first argument,
*		A0 	function vector offset,
*		TOS	return address.
*
*  at exit  :	D0 has result,
*		condition codes set by function subroutine,
*		registers D3, D4, D5, D6, D7 preserved.
*		A0 used for function call.
*		function subroutine must preserve other registers.
*
jmp_so:
		movem.l d3-d7,-(sp)					* save registers
		move.l	d0,d7						* get first argument
		moveq	#0,d1						* preset dummy part
		jsr		(a0)						* execute function subroutine
		movem.l d7,-(sp)					* DON'T TOUCH CONDITION CODES
		movem.l (sp)+,d0/d3-d7				* put result, restore registers
		rts

*
*------ execute double operand library function  (must be jumped to)
*
*  at entry :   D0 has first argument, D1 has second argument.
*		A0 	function vector offset,
*		TOS	return address,
*
*  at exit  :	D0 has result,
*		condition codes set by function subroutine,
*		registers D3, D4, D5, D6, D7 preserved.
*		A0 used for function call and return.
*		function subroutine must preserve other registers.
*
jmp_do:
		movem.l d3-d7,-(sp)	* stack registers before return.
		move.l	d1,d6		* get second argument
		move.l	d0,d7		* get first argument
		moveq	#0,d1		* preset dummy part
		jsr		(a0)		* execute function subroutine
		movem.l d7,-(sp)	* DON'T TOUCH CONDITION CODES
		movem.l (sp)+,d0/d3-d7	* put result, restore registers
		rts

*********************************************************************************
*********************************************************************************
***																			  ***
***  RAM LIBRARY ENTRY POINTS (ASSEMBLY LANGUAGE) FOR BASIC MATH FUNCTIONS	  ***
***																			  ***
*********************************************************************************
*********************************************************************************

MFSPFix:
		lea		FFPFPI,a0					* (float)D0 to (int)D0
		bra.s	jmp_so

MFSPFlt:
		lea		FFPIFP,a0					* (int)D0 to (float)D0
		bra.s	jmp_so

MFSPCmp:
		lea		FFPCMP,a0					* cc: (float)D1 - (float)D0
		bra.s	jmp_do

MFSPTst:
		lea		FFPTST,a0					* cc: (float)D1 - (float)0.00
		bra.s	jmp_so

MFSPAbs:
		lea		FFPABS,a0					* absolute value of (float)D0
		bra.s	jmp_so

MFSPNeg:
		lea		FFPNEG,a0					* negate sign of (float)D0
		bra.s	jmp_so

MFSPAdd:
		lea		FFPADD,a0					* (float)D1 + (float)D0
		bra.s	jmp_do

MFSPSub:
		lea		FFPSUB,a0					* (float)D1 - (float)D0
		bra.s	jmp_do

MFSPMul:
		lea		FFPMUL,a0					* (float)D1 X (float)D0
		bra.s	jmp_do

MFSPDiv:
		lea		FFPDIV,a0					* (float)D1 / (float)D0
		bra.s	jmp_do

*********************************************************************************
*********************************************************************************
***																			  ***
***  RAM LIB ENTRY PTS (ASSEMBLY LANGUAGE) FOR TRANSCENDENTAL MATH FUNCTIONS  ***
***																			  ***
*********************************************************************************
*********************************************************************************

MFSPAsin:
		lea		FFPASIN,a0					* Arcsine of (float)D0
		bra		jmp_so

MFSPAcos:
		lea		FFPACOS,a0					* Arccosine of (float)D0
		bra		jmp_so

MFSPAtan:
		lea		FFPATAN,a0					* Arctangent of (float)D0
		bra		jmp_so

MFSPSin:
		lea		FFPSIN,a0					* Sine of (float)D0
		bra		jmp_so

MFSPCos:
		lea		FFPCOS,a0					* Cosine of (float)D0
		bra		jmp_so

MFSPTan:
		lea		FFPTAN,a0					* Tangent of (float)D0
		bra		jmp_so

MFSPSincos:
		lea		FFPSINCS,a0					* Sine and cosine of (float)D0
		movem.l d3-d7,-(sp)	* stack registers before return.
		move.l	d0,d7		* get first argument
		move.l	d1,-(sp)	* Save cosine storage address
		moveq	#0,d1		* preset dummy part
		jsr		(a0)		* execute function subroutine
		move.l	(sp)+,a0	* A0 =  address for cosine storage
		move.l	d7,(a0)		* Save cosine result
		move.l	d6,d0		* Functional result (sine) into D0
		movem.l (sp)+,d3-d7	* restore registers
		rts

MFSPSinh:
		lea		FFPSINH,a0					* Hyperbolic sine of (float)D0
		bra		jmp_so

MFSPCosh:
		lea		FFPCOSH,a0					* Hyperbolic cosine of (float)D0
		bra		jmp_so

MFSPTanh:
		lea		FFPTANH,a0					* Hyperbolic tangent of (float)D0
		bra		jmp_so

MFSPExp:
		lea		FFPEXP,a0					* Exponential (base e) of (float)D0
		bra		jmp_so

MFSPLog:
		lea		FFPLOG,a0					* Natural logarithm of (float)D0
		bra		jmp_so

MFSPLog10:
		lea		FFPLOG10,a0					* Base 10 logarithm of (float)D0
		bra		jmp_so

MFSPPow:
		lea		FFPPWR,a0					* (float)D0 ** (float)D1
		bra		jmp_do

MFSPTieee:
		lea		FFPTIEEE,a0					* Convert (float)D0 to IEEE SP standard
		bra		jmp_so

MFSPFieee:
		lea		FFPFIEEE,a0					* Convert (IEEE SP standard) to float
		bra		jmp_so



*********************************************************************************
*********************************************************************************
***																			  ***
***  THE FOLLOWING SUBSTITUTE ENTRY POINTS ARE FOR CODE THAT HAS BEEN		  ***
***  REFERENCED FROM THE TRANSCENDENTAL MATH LIBRARY BACK TO THE BASIC MATH   ***
***  LIBRARY WHICH IS LOCATED IN ROM.                                         ***
***																			  ***
*********************************************************************************
*********************************************************************************

		xref	_LVOSPFix,_LVOSPFlt,_LVOSPCmp,_LVOSPTst,_LVOSPAbs
		xref	_LVOSPNeg,_LVOSPAdd,_LVOSPSub,_LVOSPMul,_LVOSPDiv



FFPFPI:
		lea		_LVOSPFix,a0
		bra.s	leave1

FFPIFP:
		lea		_LVOSPFlt,a0
		bra.s	leave1

FFPCMP:
		lea		_LVOSPCmp,a0
		bra.s	leave2

FFPTST:
		lea		_LVOSPTst,a0
		bra.s	leave2

FFPABS:
		lea		_LVOSPAbs,a0
		bra.s	leave1

FFPNEG:
		lea		_LVOSPNeg,a0
		bra.s	leave1

FFPADD:
		lea		_LVOSPAdd,a0
		bra.s	leave2

FFPSUB:
		lea		_LVOSPSub,a0
		bra.s	leave2

FFPMUL:
		lea		_LVOSPMul,a0
		bra.s	leave2

FFPDIV:
		lea		_LVOSPDiv,a0




leave2:
		movem.l	d0-d1/a0,-(sp)				* Save registers clobbered in call
		add.l	_MathBase,a0				* Add vector address to function offset
		move.l	d6,d1						* Transfer 2nd argument to D1
		move.l	d7,d0						* Transfer 1st argument to D0
		jsr		(a0)						* Go do the FFP add through interface
		movem.l	d0,-(sp)					* Transfer FFP result to D7 (where expected)
		movem.l	(sp)+,d7					* DON'T TOUCH CCR
		movem.l	(sp)+,d0-d1/a0				* Restore registers clobbered in call
		rts									* Exit to caller


leave1:
		movem.l	d0/a0,-(sp)					* Save registers clobbered in call
		add.l	_MathBase,a0				* Add vector address to function offset
		move.l	d7,d0						* Transfer argument to D0
		jsr		(a0)						* Go do the FFP add through interface
		move.l	d0,d7						* Transfer FFP result to D7 (where expected)
		movem.l	(sp)+,d0/a0					* Restore registers clobbered in call
		rts									* Exit to caller


		end

