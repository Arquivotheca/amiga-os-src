*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ListMembers rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_ListMembers
_ListMembers:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-132(a6)
		move.l	(a7)+,a6
		rts
	END