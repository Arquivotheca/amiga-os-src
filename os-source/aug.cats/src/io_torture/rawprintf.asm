*************************************************************************
*
* _RawPrintf.asm  - Self-contained printf clone.  Formatted strings
*		    are sent directly out the serial port.  Xon/Xoff
*		    handshake is supported.
*		    This function may be called at any time, including
*		    interrupts.
*		    Warning: This function supports 16 bit integers.
*		    Take care!
*
*	Bryce Nesbitt, 02-24-89
*
*************************************************************************

		XREF	_LVORawDoFmt
		XDEF	_RawPrintf

_RawPrintf:
		movem.l a0/a1,-(sp)
		move.l	4*3(SP),A0      ;grab format string
		lea.l	4*4(SP),A1      ;grab stack address of parameters
		movem.l A2/A3/A6/D0/D1,-(SP)
		move.l	4,a6
		lea.l	PSCODE(pc),a2
		suba.l	a3,a3
		jsr	_LVORawDoFmt(A6)
		movem.l (SP)+,D0/D1/A2/A3/A6
		movem.l (sp)+,a0/a1
		rts


PSCODE: 	move.w	$DFF018,d1
		btst	#13,d1
		beq.s	PSCODE
		and.b	#$7f,d1
		cmp.b	#$13,d1 ;Check for Xoff
		beq.s	PSCODE
		and.w	#$ff,d0
		or.w	#$100,d0
		move.w	d0,$DFF030
		rts

		END

