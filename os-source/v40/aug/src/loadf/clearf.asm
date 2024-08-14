
;
; Clear then test $F00000 board.
;	Quite sloppy, magic, mode.
;


;	move.l	#%101101110111101111101111110111,D0
;	bsr	clearto
;	bsr	checkfrom
;	bne	fail

	move.l	#$55555555,D0
	bsr	clearto
	bsr	checkfrom
	bne	fail

	bsr	addrfill
	bsr	addrtest
	bne	fail

	move.l	#$aaaaaaaa,D0
	bsr	clearto
	bsr	checkfrom
	bne	fail

;	move.l	#$4E704E70,D0
;	bsr	clearto
;	bsr	checkfrom
;	bne	fail

	move.l	#0,D0
	bsr	clearto
	bsr	checkfrom
	bne	fail

	move.l	#-1,D0
	bsr	clearto
	bsr	checkfrom
	bne	fail

	moveq	#0,d0
	bsr	PrintOK
	moveq	#0,d0		;RETURN_OK
	rts

fail:	bsr	PRINTLONGHEX
	moveq	#20,d0		;RETURN_FAIL
	rts



;
;PRINTLONGHEX(value)
;	      d0
;
PLHJSR		MACRO
		XREF	_LVO\1
		JSR	_LVO\1(A6)
		ENDM
PRINTLONGHEX	movem.l d0-d3/a0-a1/a6,-(a7)
		subq.l	#5,a7	;Take 10 bytes from stack
		subq.l	#5,a7	;(it's smaller...)
		move.l	a7,a0
		moveq	#7,d2

1$		rol.l	#4,d0
		move.l	d0,d1
		andi.w	#$f,d1
		move.b	er_hextab2(pc,d1.w),(a0)+
		dbra	d2,1$

		move.b	#10,(a0)
		move.l	4,a6
		lea	er_DOSName(pc),a1
		PLHJSR	OldOpenLibrary	;V1.0 compatible :-)
		tst.l	d0		;grrr...
		beq.s	er_lib
		move.l	d0,a6
		PLHJSR	Output
		move.l	d0,d4		;[D4-Output handle]
		beq.s	er_output
		move.l	d4,d1
		move.l	a7,d2
		moveq	#9,d3
		PLHJSR	Write		;skip error check
		move.l	d4,d1
		move.l	#er_Error,d2
		moveq	#er_ErrorE-er_Error,d3
		PLHJSR	Write		;skip error check
		move.l	a6,a1
		move.l	4,a6
		PLHJSR	CloseLibrary
		addq.l	#5,a7
		addq.l	#5,a7
er_output
er_lib		movem.l (a7)+,d0-d3/a0-a1/a6
		rts

er_hextab2	dc.b	'0123456789ABCDEF'
er_DOSName	dc.b	'dos.library',0
er_Error	dc.b	'Error writing to $F00000 memory!',10,0
er_ErrorE


PrintOK:	move.l	4,a6
		lea	er_DOSName(pc),a1
		PLHJSR	OldOpenLibrary
		move.l	d0,a6
		PLHJSR	Output
		move.l	d0,d1
		beq.s	er_output2
		move.l	#er_OK,d2
		moveq	#er_OKe-er_OK,d3
		PLHJSR	Write	    ;skip error check
		move.l	a6,a1
		move.l	4,a6
		PLHJSR	CloseLibrary
er_output2:	rts

er_OK		dc.b	'$F00000 memory test & clear passed.',10,0
er_OKe
		CNOP	0,4



clearto:
	move.l	#($80000/(4*8))-1,d1
	lea.l	$F00000,a0
loop:	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	dbra	d1,loop
	sub.l	#$10000,d1
	bpl	loop
	rts


checkfrom:
	move.l	#($80000/4)-1,d1
	lea.l	$F00000,a0
loop1:	cmp.l	(a0)+,d0
	dbne	d1,loop1
	bne.s	exitfail
	sub.l	#$10000,d1
	bpl	loop1
	moveq	#0,d0
	rts
exitfail
	move.l	a0,d0
	rts


;----fill with unique address-----
addrfill:
	move.l	#($80000/4)-1,d1
	lea.l	$F00000,a0
af_loop move.l	a0,(a0)+
	dbra	d1,af_loop
	sub.l	#$10000,d1
	bpl	af_loop
	rts


addrtest:
	move.l	#($80000/4)-1,d1
	lea.l	$F00000,a0
at_loop cmp.l	(a0),a0         ;cmpa.l (a0)+,a0 won't work
	addq.l	#4,a0
	dbne	d1,at_loop
	bne.s	exitfail2
	sub.l	#$10000,d1
	bpl	at_loop
	moveq	#0,d0
	rts
exitfail2
	move.l	a0,d0
	rts


	dc.b	'Version 2.0c'
	CNOP	0,4

	END
