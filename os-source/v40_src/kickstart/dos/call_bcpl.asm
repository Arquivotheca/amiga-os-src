*
* bcall.asm
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "exec/funcdef.i"
        INCLUDE "exec/exec_lib.i"
        INCLUDE "libraries/dos_lib.i"

callsys macro
        CALLLIB _LVO\1
        endm
        
SYSBASE	EQU	4

	XDEF	_call_bcpl_fptr
	XDEF	@call_bcpl_fptr

	XREF	SAVE
	XREF	RET

SysBase	EQU	$4

*
* Call bcpl function pointer from C.  Counts on ONLY being called by start()
* in cli right after started by ACTIV.
*
* Take d1 = parameter for function (CPTR)
*      d2 = bcpl function pointer (CPTR)
*
_call_bcpl_fptr:
@call_bcpl_fptr:
	movem.l d2-d7/a2-a6,-(sp)

	sub.l	a0,a0			; Z register
	move.l	SysBase,a3
	move.l	ThisTask(a3),a3
	move.l	TC_SPLOWER(a3),a1	; get base of BCPL stack ptr
	move.l	pr_GlobVec(a3),a2	; get global vector
	lea	SAVE,a5			; save routine
	lea	RET,a6			; return routine
	moveq	#32,d0
	add.l	d0,a1			; stack increment of ACTIV
	asr.l	#2,d1			; function wants BPTR
	move.l	d1,(a1)			; set up BCPL arguements
	move.l	d2,a4			; function pointer to call

	; I assume stack increment should be 32!!!
	jsr	(a5)

	move.l	d1,d0			; return value

	movem.l (sp)+,d2-d7/a2-a6
	rts

	END
