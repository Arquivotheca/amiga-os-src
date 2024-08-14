	SECTION	text
	XREF	_PrefsBase
	XDEF	_GetPrefsDrawer
	XDEF	_GetPref
	XDEF	_SetPref
_GetPrefsDrawer:	movem.l	A4,safekeep
	move.l	_PrefsBase,a4
	move.l 4(sp),D0
	move.l 8(sp),D1
	jsr	-30(a4)
	movem.l safekeep,A4
	rts
_GetPref:	movem.l	A4,safekeep
	move.l	_PrefsBase,a4
	move.l 4(sp),D0
	move.l 8(sp),D1
	move.l 12(sp),A0
	jsr	-36(a4)
	movem.l safekeep,A4
	rts
_SetPref:	movem.l	A4,safekeep
	move.l	_PrefsBase,a4
	move.l 4(sp),D0
	move.l 8(sp),D1
	move.l 12(sp),A0
	move.l 16(sp),A1
	jsr	-42(a4)
	movem.l safekeep,A4
	rts
safekeep	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	END
