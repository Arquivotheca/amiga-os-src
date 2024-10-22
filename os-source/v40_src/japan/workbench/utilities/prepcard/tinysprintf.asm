
		XDEF	_SPrintF
		XDEF	_SPrintC
		XREF	_LVORawDoFmt

_SPrintF:
		movem.l	a2/a3/a6,-(sp)
		move.l	4*4(sp),a3	;output str ptr
		move.l	5*4(sp),a0	;fmt str ptr
		move.l	6*4(sp),a1	;data
		lea	stuffChar(pc),a2
		move.l	4,a6
		jsr	_LVORawDoFmt(a6)

		movem.l	(sp)+,a2/a3/a6
		rts

stuffChar:
		move.b	d0,(a3)+
		rts

**
** SPrintC counts total length, but does not actually store any data ... used so we
** can dynamically allocate buffer to hold a list of output lines
**

_SPrintC:
		movem.l	a2/a3/a6,-(sp)
		move.l	4*4(sp),a3	;output str ptr
		move.l	5*4(sp),a0	;fmt str ptr
		move.l	6*4(sp),a1	;data
		moveq	#4,d0
		move.l	d0,(a3)		;total length min size is 4 (enough for a NULL,
					;and safety pad)

		lea	countChar(pc),a2
		move.l	4,a6
		jsr	_LVORawDoFmt(a6)

		
		movem.l	(sp)+,a2/a3/a6
		rts
countChar:
		addq.l	#1,(a3)		;add to total length
		rts

		END
