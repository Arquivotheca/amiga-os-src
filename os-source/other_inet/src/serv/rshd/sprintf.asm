# -----------------------------------------------------------------------
# 
#
# $Locker:  $
#
# $Id: sprintf.asm,v 1.0 92/08/07 16:32:36 bj Exp $
#
# $Revision: 1.0 $
#
# $Log:	sprintf.asm,v $
* Revision 1.0  92/08/07  16:32:36  bj
* Initial revision
* 
#
# $Header: AS225:src/serv/rshd/RCS/sprintf.asm,v 1.0 92/08/07 16:32:36 bj Exp $
#
#------------------------------------------------------------------------
#

;/* example:
; *
; * sprintf(temp,"got rexxmsg[%s]\n",(char *)rxmsg->rm_Args[0]) ;
; */
 

;/* =====================================
; * sprintf  asm glue to call Exec RawDoFmt
; * ========================================
; */
 
;/*
;* sprintf - uses RawDoFmt.  Can't be in C  !
;*/

	
	XDEF _sprintf

RawDoFmt EQU	-522
SysBase	 EQU	4

_sprintf:
		movem.l	a2/a3/a6,-(a7)
		move.l	20(a7),a0
		lea	24(a7),a1
		lea	myputstr(pc),a2
		move.l	16(a7),a3
		move.l	SysBase,a6
		jsr	RawDoFmt(a6)
		movem.l	(a7)+,a2/a3/a6
		rts

myputstr:
		move.b	d0,(a3)+
		rts

; end
	
