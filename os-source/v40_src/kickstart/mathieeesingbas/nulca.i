
NULCA1	macro
\@restart	move.w	\1,command(a0)
\@comeagain	move.w	(a0),d7
		btst	#12,d7
		beq.s	\@morework
;		bit 12=1
		btst	#11,d7
		beq.s	\@exit_and_eval
\@morework
		tst.w	d7
		bmi.s	\@maybedone
		cmp.w	#$4900,d7
		beq.s	\@comeagain
		cmp.w	#$900,d7
		ble.s	\@comeagain
		bsr	exception
		bra.s	\@restart
\@maybedone
		cmp.w	#$8900,d7
		beq.s	\@comeagain
		cmp.w	#$C900,d7
		beq.s	\@comeagain
\@exit_and_eval:
	endm

;NULCA1	macro
;\@restart	move.w	\1,command(a0)
;\@w78	move.w	(a0),d7
;	cmp.w	#$8900,d7
;	beq	\@w78
;;	check for pending exceptions
;	tst.w	d7	; can use a different bcc later
;;		do a gross check inline
;	bmi.s	\@w79
;		bsr	exception
;;		test code
;		bra	\@restart
;
;\@w79:
;	cmp.w	#$C900,d7
;	beq	\@w78
;	endm

*	NULL Release Macro
NULREL	macro
\@nr	tst.w	(a0)
	bmi	\@nr
	endm
