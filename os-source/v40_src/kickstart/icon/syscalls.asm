*************************************************************************
*
* $Id: syscalls.asm,v 38.1 91/06/24 19:02:18 mks Exp $
*
* $Log:	syscalls.asm,v $
* Revision 38.1  91/06/24  19:02:18  mks
* Changed to V38 source tree - Trimmed Log
* 
*************************************************************************

	SECTION	section

;****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'libraries/dos.i'
	INCLUDE 'libraries/dosextens.i'

	INCLUDE	'internal.i'
	INCLUDE	'asmsupp.i'


;****** Imported Names ***********************************************

*------ Offsets ------------------------------------------------------

	XLIB	FindTask

;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	_setResult2

*------ Data ---------------------------------------------------------

;****** Local Definitions ********************************************


_setResult2:	;(newresult)

	move.l	a6,-(sp)		; Save workbench base...
	sub.l	a1,a1			; Clear a1...
	move.l	il_SysBase(a6),a6	; Get execbase...
	jsr	_LVOFindTask(a6)	; Get the current task...
	move.l	(sp)+,a6		; Restore workbench base...
	move.l	d0,a1			; Get task pointer...
	move.l	4(sp),pr_Result2(a1)	; Set result...
	rts

	END
