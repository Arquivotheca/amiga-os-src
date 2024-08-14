******************************************************************************
* This file contains an add function used by the scanner to test integer
* add overflow.
*
* started 22/MAY/91, written by P. Jones			(C) Commodore-Amiga
******************************************************************************

	SECTION text,CODE					;stops binary frags

	XDEF _scan_add						; stack param version
	XDEF @scan_add						; reg param version

	XDEF _scan_addbase					; stack param version
	XDEF @scan_addbase					; reg param version
*-----------------------------------------------------------------------------
* error = _scan_add(int *accum,int value)
*  D0                 4(SP)     8(SP)
*-----------------------------------------------------------------------------

_scan_add	move.l 4(SP),a0
			move.l 8(SP),d0

@scan_add	
			moveq.l #10,d1
			muls.l (a0),d1
			bvs scan_overflow

			add.l d1,d0				; d0 = acumm+value
			bvs scan_overflow

			move.l d0,(a0)			; if no overflow,put result in (accum)
			moveq #0,d0				; error = OK
			rts

scan_overflow
			moveq #1,d0				; error = OVERFLOW
			rts

*-----------------------------------------------------------------------------
* error = _scan_addbase(int *accum,int value,int base)
*  D0                      4(SP)     8(SP)     12(SP)
*-----------------------------------------------------------------------------

_scan_addbase	move.l 4(SP),a0
			move.l 8(SP),d0
			move.l 12(SP),d1

@scan_addbase	
			mulu.l (a0),d1
			bvs scanbase_overflow

			add.l d1,d0				; d0 = acumm+value
			bvs scanbase_overflow

			move.l d0,(a0)			; if no overflow,put result in (accum)
			moveq #0,d0				; error = OK
			rts

scanbase_overflow
			moveq #1,d0				; error = OVERFLOW
			rts
	END
