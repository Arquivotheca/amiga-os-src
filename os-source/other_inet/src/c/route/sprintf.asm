;/* example:
; *
; * sprintf(temp,"got rexxmsg[%s]\n",(char *)rxmsg->rm_Args[0]) ;
; */
 

;/* =====================================
; * mysprintf  asm glue to call Exec RawDoFmt
; * ========================================
; */
 
;/*
;* _sprintf - uses RawDoFmt.  Can't be in C  !
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
	
