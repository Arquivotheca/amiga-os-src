
	section strings.lib

	xdef	_TailPath
	xdef	TailPath

	xref	rindex

_TailPath:
	move.l	4(sp),a0

TailPath:
	move.l	a2,-(sp)

	move.l	a0,a2

	moveq.l	#'/',d0
	bsr	rindex
	bne.s	TailPath_bump

	move.l	a2,a0
	moveq.l	#':',d0
	bsr	rindex
	bne.s	TailPath_bump

	move.l	a2,d0

TailPath_end:
	move.l	(SP)+,a2
	rts

TailPath_bump:
	;------ we found a ':' or a '/'.  Go to the next character
	addq.l	#1,d0
	bra.s	TailPath_end

	end
