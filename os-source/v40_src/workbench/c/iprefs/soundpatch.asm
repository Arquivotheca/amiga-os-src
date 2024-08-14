	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "dos/dos.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVOSignal
	XREF	_SysBase

	XREF	_iprefsTask
	XREF	_doFlash
	XREF	_oldDisplayBeep

;---------------------------------------------------------------------------

	XDEF	@DisplayBeepPatch

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

@DisplayBeepPatch:
	movem.l	a0/a6,-(sp)		; save screen pointer
	move.l	_SysBase,a6		; load SysBase
	move.l	_iprefsTask,a1		; find IPrefs task
	move.l	#SIGBREAKF_CTRL_F,d0	; signal to send to IPrefs
	CALL	Signal			; signal IPrefs
	tst.w	_doFlash		; should we flash?
	movem.l	(sp)+,a0/a6		; restore registers
	bne.s	DoFlash			; we should flash, go to it
	rts				; bye

DoFlash:
	move.l	_oldDisplayBeep,a1	; load address of old DisplayBeep code
	jmp	(a1)			; call old DisplayBeep

;---------------------------------------------------------------------------

	END
