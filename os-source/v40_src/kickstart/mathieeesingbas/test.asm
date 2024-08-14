;
; * Simple test of methieeesingbas.library
; * We use the fact that enforcer will do the output for us...
;
	xref	_LVOOpenLibrary
	xref	_LVOCloseLibrary
	xref	_LVOIEEESPFloor


	move.l	4,a6		; Get execbase
	lea	libname(pc),a1
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,d6		; Math library
	exg.l	d6,a6		; Now point at math...
;
	move.l	#$4A000000,d2	; Start with this...
	moveq.l	#5,d3		; 5 times...
loop:	move.l	d2,d0		; Get into place
	jsr	_LVOIEEESPFloor(a6)	; Do it
	move.l	0,d1		; Enforcer hit...
	add.l	#$00800000,d2	; To next value...
	dbra	d3,loop
;
	exg	d6,a6		; Get execbase back...
	move.l	d6,a1		; Library
	jsr	_LVOCloseLibrary(a6)	;close it
	moveq.l	#0,d0
	rts

libname:	dc.b	'mathieeesingbas.library',0
