*
*   KillF - Zap all ROMTags found in $F00000 memory
*
START	EQU $F00000
END	EQU $F80000-6	;No ROMTags allowed at very end of memory
ILLEGAL EQU $4AFC	;ROMTag identifier


		lea.l	START,a0
		move.w	#ILLEGAL,d0
		move.l	#(END-START)/2,d1


;----------------------------------------------------------------------------
		moveq	#0,d7	    ;flag "no tags killed yet"
kf_loop 	cmp.w	(a0)+,d0
		bne.s	kf_nomatch  ;No magic cookie...
		move.l	a0,a1
		subq.l	#2,a1
		cmpa.l	(a0),a1
		bne.s	kf_nomatch  ;No back pointer...

		;---- found a valid ROMTag!, zap the back pointer! ----
		clr.l	(a0)        ;Could also be neg.l (a0)
		moveq	#5,d7	    ;flag "killed at least one tag"
		tst.l	(a0)
		bne.s	PrintError

kf_nomatch:	subq.l	#1,d1
		bne.s	kf_loop
;----------------------------------------------------------------------------


		move.l	d7,d0	    ;0=no tags killed  5=tags killed
		beq.s	kf_nokill

		;--- If we killed any tags, also kill Exec reboot
		lea.l	START,a0
		clr.l	(a0)
		tst.l	(a0)
		bne.s	PrintError

kf_nokill	rts



;----------------------------------------------------------------------------
PLHJSR		MACRO
		XREF	_LVO\1
		JSR	_LVO\1(A6)
		ENDM
PrintError:	move.l	4,a6
		lea	er_DOSName(pc),a1
		PLHJSR	OldOpenLibrary
		move.l	d0,a6
		PLHJSR	Output
		move.l	d0,d1
		beq.s	er_output
		move.l	#er_Error,d2
		moveq	#er_ErrorE-er_Error,d3
		PLHJSR	Write		;skip error check
		move.l	a6,a1
		move.l	4,a6
		PLHJSR	CloseLibrary
er_output:	moveq	#20,d0		;RETURN_FAIL
		rts

er_DOSName	dc.b	'dos.library',0
er_Error	dc.b	'Error writing to $F00000 memory!',10,0
er_ErrorE

		dc.b	'Version 2.0c'
		CNOP	0,4
