* TackOn(oldpath,newname,maxlen) tack newname onto oldpath for maxlen characters.

			section	tackon.asm,code

			xdef		_TackOn,TackOn

_TackOn:	move.l		4(sp),a0			; old path
			move.l		8(sp),a1			; new path
			move.l		12(sp),d0			; max length

TackOn:		move.l		a2,-(sp)			; save a reg
			tst.w		d0
			bmi.s		NoEnd				; length is negative, invalid
			beq.s		NoEnd				; length is zero, invalid
			move.l		a0,a2				; save string pointer
			tst.b		(a0)				; if any characters in old path
			beq.s		CopyName			; old path is null string. just copy

FindEnd:	move.b		(a0)+,d1			; get last character.
			subq.w		#1,d0				; subtract 1 from length.
			beq.s		NoEnd				; string has no terminating null.
			tst.b		(a0)				; test if this character is end of string.
			bne.s		FindEnd				; if not, check next

			move.l		a0,a2				; save end of string.

			cmp.b		#'/',d1				; if last character was a slash
			beq.s		CopyName			; or
			cmp.b		#':',d1				; a colon
			beq.s		CopyName			; and
			tst.b		(a1)				; if newname is non-null
			beq.s		CopyEnd				; then
			move.b		#'/',(a0)+			; add a slash
			subq.w		#1,d0				; subtract 1 from length.
			beq.s		NoRoom				; string has no terminating null.
			
CopyName:	move.b		(a1)+,(a0)+			; move bytes
			beq.s		CopyEnd
			sub.w		#1,d0				; subtract 1 from length
			bne.s		CopyName			; if room left continue, else abort.

NoRoom:		clr.b		(a2)				; terminate the old string.
NoEnd:		moveq		#0,d0				; set return = FAILURE
			move.l		(sp)+,a2			; restore reg
			rts								; and return

CopyEnd:	moveq		#-1,d0				; set return = SUCCESS
			move.l		(sp)+,a2			; restore reg
			rts								; and return

			end
