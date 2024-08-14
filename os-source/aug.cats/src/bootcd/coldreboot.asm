		INCLUDE "exec/types.i"
		INCLUDE "exec/libraries.i"

		XDEF	_ColdReboot
		XREF	_LVOSupervisor

ABSEXECBASE		EQU 4
MAGIC_ROMEND		EQU $01000000
MAGIC_SIZEOFFSET	EQU -$14
V36_EXEC		EQU 36
TEMP_ColdReboot		EQU -726

_ColdReboot:	move.l	ABSEXECBASE,a6
		cmp.w	#V36_EXEC,LIB_VERSION(a6)
		blt.s	old_exec
		jmp	TEMP_ColdReboot(a6)

old_exec:	lea.l	GoAway(pc),a5
		jsr	_LVOSupervisor(a6)

;		cnop	0,4
GoAway:		lea.l	MAGIC_ROMEND,a0
		sub.l	MAGIC_SIZEOFFSET(a0),a0
		move.l	4(a0),a0
		subq.l	#2,a0
		reset
		jmp	(a0)
