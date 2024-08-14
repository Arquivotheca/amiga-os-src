
;BCMP.ASM
;
;   max compare size = 64k !!
;
;   BCMP(p1,p2,n)   return 1=failed, 0=compare ok

     xdef  _bcmp

_bcmp:
	movem.l 4(A7),A0/A1	;A0 = ptr1, A1 = ptr2
	move.l	12(A7),D0	;# bytes
	moveq.l	#0,D1		;sets Z flag
	bra	dropb
loopb	cmpm.b	(A0)+,(A1)+	;BYTE COMPARE
dropb	dbne.w	D0,loopb	;until count exhausted or compare failed
	bne	false
	moveq.l	#0,D0
	rts			;return TRUE

false	moveq.l	#1,D0
	rts			;return FALSE


