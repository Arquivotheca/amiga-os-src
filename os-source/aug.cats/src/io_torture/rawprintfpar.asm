*************************************************************************
*
* PARALLEL VERSION
*
* _RawPrintf.asm  - Self-contained printf clone.  Formatted strings
*		    are sent directly out the parallel port.
*		    
*		    This function may be called at any time, including
*		    interrupts.
*		    Warning: This function supports 16 bit integers.
*		    Take care!
*
*	Bryce Nesbitt, 02-24-89
*
*************************************************************************

		XREF    DPutFmt
		XDEF	_RawPrintf

* Called with format string and parameters on stack
_RawPrintf:
		movem.l a0/a1,-(sp)
		move.l	4*3(SP),A0      ;grab format string
		lea.l	4*4(SP),A1      ;grab stack address of parameters
		movem.l A6/D0/D1,-(SP)
		move.l	4,a6
        	JSR     DPutFmt
		movem.l (SP)+,D0/D1/A6
		movem.l (sp)+,a0/a1
		rts

		END

