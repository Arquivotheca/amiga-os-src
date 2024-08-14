# -----------------------------------------------------------------------
# mysprintf.asm
#
# $Locker:  $
#
# $Id: mysprintf.asm,v 38.0 93/04/13 11:34:24 bj Exp $
#
# $Revision: 38.0 $
#
# $Log:	mysprintf.asm,v $
* Revision 38.0  93/04/13  11:34:24  bj
* v38 rlogin version. This version returns a pointer to
* the filled buffer. See example in source code.
* 
#
# $Header: AS225:src/c/rlogin/RCS/mysprintf.asm,v 38.0 93/04/13 11:34:24 bj Exp $
#
#------------------------------------------------------------------------
#
;/* example:
; *
; * mysprintf(temp,"got rexxmsg[%s]\n",(char *)rxmsg->rm_Args[0]) ;
; *
; * this version allows this :
; *
; *    char *foo = mysprintf(buffer,"format",vars) ;
; *
; */
 

;/* =====================================
; * mysprintf  asm glue to call Exec RawDoFmt
; * ========================================
; */
 
;/*
;* mysprintf - uses RawDoFmt.  Can't be in C  !
;*/

	
	XDEF _mysprintf

RawDoFmt EQU	-522
SysBase	 EQU	4

_mysprintf:
		movem.l	a2/a3/a6,-(a7)
		move.l	20(a7),a0
		lea	24(a7),a1
		lea	myputstr(pc),a2
		move.l	16(a7),a3
		move.l	16(a7),-(A7)
		move.l	SysBase,a6
		jsr	RawDoFmt(a6)
		move.l  (A7)+,d0
		movem.l	(a7)+,a2/a3/a6
		rts

myputstr:
		move.b	d0,(a3)+
		rts

; end
	
