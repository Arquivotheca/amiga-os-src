	OPTIMON

;---------------------------------------------------------------------------

	XDEF	_ConvSigned
	XDEF	_ConvUnsigned
	XDEF	_ConvHex
	XDEF	_ConvHexUpper

;---------------------------------------------------------------------------

*****i* locale.library/ConvSigned ********************************************
*
*   NAME
*	ConvSigned -- format a long signed binary to decimal ASCII.
*
*   SYNOPSIS
*	len = ConvSigned(number,buffer);
*		         D0     A0
*
*	ULONG ConvSigned(LONG,STRPTR);
*
******************************************************************************

*****i* locale.library/ConvUnsigned ******************************************
*
*   NAME
*	ConvUnsigned -- format a long signed binary to decimal ASCII.
*
*   SYNOPSIS
*	len = ConvUnsigned(number,buffer);
*		           D0     A0
*
*	ULONG ConvUnsigned(ULONG,STRPTR);
*
******************************************************************************

_ConvSigned:	movem.l	d2-d4,-(sp)
		moveq	#0,d4
		tst.l	d0		; Check the input number
		bpl.s	ConvStart	; If not negative, skip this
		move.b	#'-',(a0)+	; Output the sign
		moveq	#1,d4
		neg.l	d0		; Swap to positive value...
		bra.s	ConvStart

_ConvUnsigned:	movem.l	d2-d4,-(sp)
		moveq.l	#0,d4

ConvStart:      moveq.l	#'0',d3		; Assume we need a 0...
		lea	ConvTable(pc),a1; Get table...
1$:		move.l	(a1)+,d1	; Get current field
		beq.s	ConvDone	; We be done with format...
		moveq.l	#'0'-1,d2	; Start at '0'-1...
2$:		addq.l	#1,d2		; Bump up one...
		sub.l	d1,d0		; Drop the digit...
		bcc.s	2$		; Keep doin it until too far...
		add.l	d1,d0		; Add it back in...
		cmp.l	d3,d2		; Check for leading 0...
		beq.s	1$		; Continue if it is one...
		moveq.l	#0,d3		; Clear leading 0 flag...
		addq.l	#1,d4
		move.b	d2,(a0)+	; Save the character
		bra.s	1$		; Do next digit...

ConvDone:	moveq.l	#'0',d3		; Get the byte to add...
		add.b	d3,d0		; Add it to the last digit...
		move.b	d0,(a0)+	; Save it...
		addq	#1,d4
		move.l	d4,d0
		clr.b	(a0)
		movem.l	(sp)+,d2-d4
		rts			; we are done.

ConvTable:
		dc.l	1000000000	; 1,000,000,000
		dc.l	100000000	;   100,000,000
		dc.l	10000000	;    10,000,000
		dc.l	1000000		;     1,000,000
		dc.l	100000		;       100,000
		dc.l	10000		;	 10,000
		dc.l	1000		;	  1,000
		dc.l	100		;           100
		dc.l	10		;            10
		dc.l	0		; End of table / unit digit default

*****i* locale.library/ConvHex ***********************************************
*
*   NAME
*	ConvHex -- Format a long signed binary to hex ASCII
*
*   SYNOPSIS
*	len = ConvHex(number,buffer,asLong);
*		      D0     A0     D3
*
*	ULONG ConvHex(ULONG,STRPTR,BOOL);
*
******************************************************************************

_ConvHex:
		tst.l	d0
		bne.s	CXStart
		moveq.l	#'0',d0		; Get the byte to add...
		move.b	d0,(a0)+	; Same it...
		clr.b	(a0)
		moveq	#1,d0
		rts			; we are done.

CXStart:        movem.l	d2-d5,-(sp)
		moveq	#0,d1
		moveq	#0,d5
		tst.b	d3
		bne.s	CXLong
		moveq	#3,d2
		swap	d0
		bra.s	CX16sLoop
CXLong:
		moveq	#7,d2
CX16sLoop:
		rol.l	#4,d0
		move.b	d0,d4
		and.b	#$0F,d4
		bne.s	CXYesOut
		tst.w	d1
		beq.s	CX16sDbf
CXYesOut:
		moveq	#-1,d1
		cmp.b	#9,d4
		bhi.s	CXLetter
		add.b	#'0',d4
		bra.s	CXMoveOut
CXLetter:
		add.b	#('a'-10),d4
CXMoveOut:
		addq.l	#1,d5
		move.b	d4,(a0)+

CX16sDbf:
		dbf	d2,CX16sLoop
		move.l	d5,d0
		movem.l	(sp)+,d2-d5
		clr.b	(a0)
		rts

*****i* locale.library/ConvHexUpper ***********************************************
*
*   NAME
*	ConvHexUpper -- Format a long signed binary to hex ASCII
*
*   SYNOPSIS
*	len = ConvHexUpper(number,buffer,asLong);
*		           D0     A0     D3
*
*	ULONG ConvHexUpper(ULONG,STRPTR,BOOL);
*
******************************************************************************

_ConvHexUpper:  tst.l	d0
		bne.s	CXUStart
		moveq.l	#'0',d0		; Get the byte to add...
		move.b	d0,(a0)+	; Same it...
		clr.b	(a0)
		moveq	#1,d0
		rts			; we are done.

CXUStart:       movem.l	d2-d5,-(sp)
		moveq	#0,d1
		moveq	#0,d5
		tst.b	d3
		bne.s	CXULong
		moveq	#3,d2
		swap	d0
		bra.s	CXU16sLoop
CXULong:
		moveq	#7,d2
CXU16sLoop:
		rol.l	#4,d0
		move.b	d0,d4
		and.b	#$0F,d4
		bne.s	CXUYesOut
		tst.w	d1
		beq.s	CXU16sDbf
CXUYesOut:
		moveq	#-1,d1
		cmp.b	#9,d4
		bhi.s	CXULetter
		add.b	#'0',d4
		bra.s	CXUMoveOut
CXULetter:
		add.b	#('A'-10),d4
CXUMoveOut:
		addq.l	#1,d5
		move.b	d4,(a0)+

CXU16sDbf:
		dbf	d2,CXU16sLoop
		move.l	d5,d0
		movem.l	(sp)+,d2-d5
		clr.b	(a0)
		rts

;---------------------------------------------------------------------------

	END
