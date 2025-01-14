*******************************************************************************
*
*	$Id: utility.asm,v 42.0 93/06/16 11:18:22 chrisg Exp $
*
*******************************************************************************

;---------------------------------------------------------------------------
; utility routines for database stuff
;---------------------------------------------------------------------------

; ULONG __asm copy_dbstuff(register __a0 UBYTE **src, register __a1 ULONG *size, register __a2 UBYTE **dst, 
; register __a3 ULONG *bytes, register __d0 ULONG raw);	/* __asm */

; c code was:
;    ULONG result = 0;
;
;   while((raw--) && ((*size)--) && ((*bytes)--) && (++result))
;   {
;	*(*dst)++ = *(*src)++;
;   }
;
;   return( result );

; this routine does the following:
; copy min(raw,*size,*bytes) from *dst to *src
; return the number of bytes copied
; subtract the number of bytes copied from *size and *bytes
; update the pointers

_copy_dbstuff::
; get # bytes to copy in d0
	movem.l	a2-a5,-(a7)
	cmp.l	(a3),d0		; compare against *bytes
	bls.s	1$
	move.l	(a3),d0
1$:	cmp.l	(a1),d0		; compare against *size
	bls.s	2$
	move.l	(a1),d0
2$:	move.l	(a0),a4
	move.l	(a2),a5
	move.l	d0,d1
	lsr.w	#1,d1
	bcc.s	end_of_copy
	move.b	(a4)+,(a5)+
	bra.s	end_of_copy
copy_loop:
	move.b	(a4)+,(a5)+
	move.b	(a4)+,(a5)+
end_of_copy:
	dbra	d1,copy_loop
	move.l	a4,(a0)
	move.l	a5,(a2)
	sub.l	d0,(a3)
	sub.l	d0,(a1)
	movem.l	(a7)+,a2-a5
	rts
