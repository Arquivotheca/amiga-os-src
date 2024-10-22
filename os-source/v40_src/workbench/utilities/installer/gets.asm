*  Fgets (handle,buffer,count) - stops at NEWLINE, ignores RETURN

		section	gets.asm,code
		include 'lat_macros.i'
		xref	_DOSBase

NEWLINE		equ		$0a
RETURN		equ		$0d

		DECLAREL Fgets

		movem.l	a2-a3/d2-d5,-(sp)
		move.l	4+24(sp),a3
		move.l	8+24(sp),a2
		move.l	12+24(sp),d5
merge
		moveq	#0,d4
more
		move.l	a3,d1
		move.l	a2,d2
		moveq	#1,d3
		move.l	a6,-(sp)
		CallDos	Read
		move.l	(sp)+,a6
		tst.l	d0
		bmi.s	error			; error condition
		beq.s	eof				; read zero, end of file

		cmp.b	#NEWLINE,(a2)
		beq.s	done			; a NEWLINE means we're done
		cmp.b	#RETURN,(a2)
		beq.s	more			; ignore RETURN
		addq.w	#1,a2

		addq.l	#1,d4
		cmp.l	d5,d4
		blt.s	more
done
		clr.b	(a2)
		move.l	d4,d0
		movem.l	(sp)+,a2-a3/d2-d5
		rts

eof		tst.l	d4
		bne.s	done
		moveq	#-1,d4
		bra.s	done
error
		moveq	#-2,d4
		bra.s	done

*	Gets(buffer,count)

		DECLAREL	Gets

		move.l	a6,-(sp)
		CallDos	Input
		move.l	(sp)+,a6
		beq.s	nyet

		movem.l	a2-a3/d2-d5,-(sp)
		move.l	d0,a3
		move.l	4+24(sp),a2
		move.l	8+24(sp),d5
		bra.s	merge
nyet
		moveq	#-1,d0
		rts

		end
