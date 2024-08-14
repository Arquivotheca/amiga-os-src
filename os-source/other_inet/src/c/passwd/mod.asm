; mod.asm    bj   9-15-91
;
;  casm -a $*.asm -cqvy -o $*.o


; uses utility library UDivMod32() function to get the MODulus
; this function assumes that UtilityBase truly points to a valid
; utility.library base. 
;
;  result = mod( ULONG dividend, ULONG divisor, struct Library *UtilityBase ) ;
;   d0                 d0             d1                    a0
;

	xdef _mod

_mod:

	movem.l d6-d7/a2-a3/a6,-(a7)
	move.l $0018(a7),d0
	move.l $001C(a7),d1
	move.l $0020(a7),a6
	jsr -156(a6)
	move.l d1,d0
	movem.l   (a7)+,d6-d7/a2-a3/a6
	rts


