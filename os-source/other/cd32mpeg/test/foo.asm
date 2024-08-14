

	XDEF	_WCP
	XREF	_LVOWriteChunkyPixels
	XREF	_kprintf

_WCP:	movem.l	d0-d7/a0-a6,-(a7)

        movem.l	d0-d7/a0-a6,-(a7)
        pea	msg1(pc)
	jsr	_kprintf
	addq.l	#4,a7
	movem.l	(a7)+,d0-d7/a0-a6

	jsr	_LVOWriteChunkyPixels(a6)

	movem.l	d0-d7/a0-a6,-(a7)
	pea	msg1(pc)
	jsr	_kprintf
	lea.l	16*4(a7),a7

	movem.l	(a7)+,d0-d7/a0-a6
	rts

msg1	dc.b	'D: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx',10
	dc.b	'A: %08lx %08lx %08lx %08lx %08lx %08lx %08lx ',10,0
