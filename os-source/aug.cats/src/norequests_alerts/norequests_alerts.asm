*
* norequests_alerts.asm - kills all requesters (cancel) and alerts (reboot)
*	systemwide and non-reversible
*
* blink from norequests_alerts.o to norequests_alerts library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/macros.i"

	XREF	_LVOEasyRequestArgs
	XREF	_LVOAutoRequest
	XREF	_LVORequest
	XREF	_LVODisplayAlert
	XREF	_LVOAlert

startme:

	movem.l	d1-d7/a0-a6,-(sp)

	move.l	4,a6			;exec
	lea	iname(pc),a1		;intuition.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq	badopen
	move.l	d0,a5			;IntuitionBase (a5)

	move.l	#copyend-copystart,d0
	move.l	#0,d1
	JSRLIB	AllocMem
	move.l	d0,d2			;dest for code (d2)
	beq.s	outahere

	lea.l	copystart,a0		;source
	movea.l	d0,a1			;dest
	move.l	#copyend-copystart,d0	;size
	JSRLIB	CopyMem			;copy code to alloc'd mem

	move.l	d2,d3			;start of cancel code
	add.l	#reboot-copystart,d3	;start of reboot code

	movea.l	a6,a1			;library to setfunction (exec)
	movea.l #_LVOAlert,a0 ;offset
	move.l	d3,d0			;new code
	JSRLIB	SetFunction

	movea.l	a5,a1			;library to setfunction (intuition)
	movea.l #_LVODisplayAlert,a0	;offset
	move.l	d3,d0			;new code
	JSRLIB	SetFunction

	movea.l	a5,a1			;library to setfunction (intuition)
	movea.l #_LVORequest,a0		;offset
	move.l	d2,d0			;new code
	JSRLIB	SetFunction

	movea.l	a5,a1			;library to setfunction (intuition)
	movea.l #_LVOAutoRequest,a0	;offset
	move.l	d2,d0			;new code
	JSRLIB	SetFunction

	cmpi.w	#36,LIB_VERSION(a5)
	blt.s	not36
	movea.l	a5,a1			;library to setfunction (intuition)
	movea.l #_LVOEasyRequestArgs,a0 ;offset
	move.l	d2,d0			;new code
	JSRLIB	SetFunction
not36

outahere:
	movea.l	a5,a1			;IntuitionBase
	move.l	4,a6
	JSRLIB	CloseLibrary
badopen:
	moveq	#00,d0			;return OK
	movem.l	(sp)+,d1-d7/a0-a6
	rts

*
* Code to be copied to allocated RAM
*
copystart:

cancel:
	moveq.l	#0,d0
	rts
cancele:

reboot:
		XDEF	_ColdReboot
		XREF	_LVOSupervisor

ABSEXECBASE	    EQU 4	    ;Pointer to the Exec library base
MAGIC_ROMEND	    EQU $01000000   ;End of Kickstart ROM
MAGIC_SIZEOFFSET    EQU -$14	    ;Offset from end of ROM to Kickstart size
V36_EXEC	    EQU 36	    ;Exec with the ColdReboot() function
TEMP_ColdReboot     EQU -726	    ;Offset of the V36 ColdReboot function


_ColdReboot:	move.l	ABSEXECBASE,a6
		cmp.w	#V36_EXEC,LIB_VERSION(a6)
		blt.s	old_exec
		jmp	TEMP_ColdReboot(a6)     ;Let Exec do it


;---- manually reset the Amiga ---------------------------------------------
old_exec:	lea.l	GoAway(pc),a5
		jsr	_LVOSupervisor(a6)


;-------------- MagicResetCode ---------DO NOT CHANGE-----------------------
		CNOP	0,4			;IMPORTANT! Longword align!
GoAway: 	lea.l	MAGIC_ROMEND,a0 	;(end of ROM)
		sub.l	MAGIC_SIZEOFFSET(a0),a0 ;(end of ROM)-(ROM size)=PC
		move.l	4(a0),a0                ;Get Initial Program Counter
		subq.l	#2,a0			;now points to second RESET
		reset				;first RESET instruction
		jmp	(a0)                    ;CPU Prefetch executes this
		;NOTE: the RESET and JMP instructions must share a longword!
;---------------------------------------DO NOT CHANGE-----------------------

reboote:
copyend:

	CNOP 0,2

verstag	DC.B	'$VER: norequests_alerts 37.1 (6.5.92)',0

iname	DC.B	'intuition.library',0

	END


