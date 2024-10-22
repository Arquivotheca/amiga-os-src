kprintf	MACRO
	IFD DEBUG
		movem.l	d0-d1/a0-a1,-(a7)
	IFGE	NARG-9
		move.l	\9,-(a7)		; stack arg8
	ENDC
	IFGE	NARG-8
		move.l	\8,-(a7)		; stack arg7
	ENDC
	IFGE	NARG-7
		move.l	\7,-(a7)		; stack arg6
	ENDC
	IFGE	NARG-6
		move.l	\6,-(a7)		; stack arg5
	ENDC
	IFGE	NARG-5
		move.l	\5,-(a7)		; stack arg4
	ENDC
	IFGE	NARG-4
		move.l	\4,-(a7)		; stack arg3
	ENDC
	IFGE	NARG-3
		move.l	\3,-(a7)		; stack arg2
	ENDC
	IFGE	NARG-2
		move.l	\2,-(a7)		; stack arg1
	ENDC
	IFGE	NARG-1
STKUSE	SET	NARG<<2				; stack space used
	IFND	_kprintf
	XREF	_kprintf
	ENDC
		pea.l	str\@(pc)		; push string address
		jsr	_kprintf		; call kprintf function
		lea.l	STKUSE(a7),a7		; scrap stuff on stack
		movem.l	(a7)+,d0-d1/a0-a1
		bra.s	skp\@
str\@		dc.b	\1,0			; the actual string
		ds.w	0
skp\@:
	ENDC
	ENDC
	ENDM
