

	section	strings.lib

	xdef	_strcat
	xdef	strcat

	xref	strcpy

_strcat:
	movem.l	4(sp),a0/a1	; to, from

strcat:
	tst.b	(a0)+
	bne.s	strcat

	subq.l	#1,a0
	bra.s	strcpy
	end
