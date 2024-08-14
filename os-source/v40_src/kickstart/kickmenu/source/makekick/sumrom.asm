*
*   Assembly language support for kickit.
*
*   Monday 31-Jul-89 03:44:54  Bryce Nesbitt
*   Monday 16-Jan-90	       Bryce Nesbitt
*
*
	SECTION assembly,CODE

	XDEF	_Install_Cold	;Set a ColdCapture for Exec to notice
	XDEF	_CopyStuffAndBoot
	XDEF	_SumROM 	;Checksum N bytes of ROM, return result
	XDEF	_ColdReboot	;Reboot the machine on any CPU type
;;;	XDEF	_memcopyl	;Copy longs, including overlapping cases

	XREF	_LVOSupervisor


	INCLUDE "exec/types.i"
	INCLUDE "exec/execbase.i"

ABSEXECBASE	EQU 4
FrigidMagic	EQU $10c



*****************************************************************************
*
*   SYNOPSIS
*	Install_Cold(function)
*
*	void (*)() Install_Cold( void (*)() );
*
*   FUNCTION
*	Install a pointer in Exec's "ColdCapture" vector.  Returns the
*	old pointer.  The ExecBase checksum will be recalucated.
*	The function will be called very early in the boot.  The pointer
*	must be in chip or other "local" memory.
*
*	Call while under Disable() or Forbid().
*
*****************************************************************************

_Install_Cold:			    ;(stolen from ramkick)
	move.l	a6,-(sp)

*	initialize coolcapture vector

	MOVE.L	ABSEXECBASE,A6
	MOVE.L	ColdCapture(A6),A1
	MOVE.L	4+4(sp),ColdCapture(A6)
	MOVE.L	4+8(sp),FrigidMagic

*   recalculate the execbase checksum

	MOVEQ	#0,D1
	LEA	SoftVer(A6),A0
	MOVE.W	#(MaxExtMem-SoftVer)/2,D0

1$:	ADD.W	(A0)+,D1
	DBF	D0,1$
	NOT.W	D1
	MOVE.W	D1,ChkSum(A6)

	move.l	a1,d0		;Return old pointer
	move.l	(sp)+,a6
	rts


*****************************************************************************
*
*   SYNOPSIS
*	result = SumROM(base,size);
*
*	ULONG SumROM(ULONG *,ULONG);
*
*   FUNCTION
*	Checksum a Kickstart ROM.  NULL means the checksum was good.
*
*****************************************************************************
_SumROM:
	move.l	4(sp),a0    ;ARG0-base
	move.l	8(sp),d1    ;ARG1-size
	lsr.l	#2,d1	    ;divide by four
	subq.l	#1,d1	    ;subtract one
;----------------------------------------------------------------------------
	moveq	#0,d0	    ;Zero out stored checksum

cs_chksum:
	add.l	(a0)+,d0        ;12 (nom)
	bcc.s	cs_nooverflow	;12
	addq.l	#1,d0
cs_nooverflow:
	nop			;4
	dbra	d1,cs_chksum	;10
	sub.l	#$10000,d1
	bpl.s	cs_chksum
	addq.l	#1,d0
	rts



****************************************************************************
*
*   Copy the Kickstart then reboot the machine without using the stack.
*
*****************************************************************************
VirtualTC	dc.l	0   ;Memory location for _silly_ PMOVE instruction


_CopyStuffAndBoot:
		move.w	#$5555,$2fe
		move.l	1*4(a7),a1      ;destination
		move.l	2*4(a7),a0      ;source
		move.l	3*4(a7),d0      ;count

		move.l	4,a6			;Get a pointer to ExecBase
		lea.l	StuffAndBoot(pc),a5     ;Location of code to RUN
		jsr	_LVOSupervisor(a6)      ;RUN code in Supervisor mode


StuffAndBoot:
		;[A0-source]
		;[A1-destination]
		;[D0-count]

;
;   Turn off MMU
;


;    add a check for capable processor
;
;		 lea.l	 VirtualTC(pc),a4
;		 dc.w	 $f014			 ; PMOVE TC,(a4)
;		 dc.w	 $4200
;		 and.l	 #$7fffffff,(a4)
;		 dc.w	 $f014			 ; PMOVE (a4),TC
;		 dc.w	 $4000
;		 move.l  #$00000808,d4		 ; Diable + flush caches
;		 DC.W	 $4E7B,$4002		 ; MOVEC D1,CACR



****************************************************************************
;
; Copy memory in blocks of longs.  Overlapping copies are supported.
; Length must be a multiple of sizeof(long). Zero count is supported.
;
;	 memcopyl(to,from,size);
;   void memcopyl(long *,long *,long)
;
;_memcopyl:
;;;		move.l	1*4(a7),a1      ;destination
;;;		move.l	2*4(a7),a0      ;source
;;;		move.l	3*4(a7),d0      ;count
		move.l	d0,d1
		lsr.l	#2,d0		;divide by four
		beq.s	exit
		subq.l	#1,d0		;adjust for DBRA
		cmpa.l	a0,a1		;Compare A1 to A0
		bhi.s	decending	;destination is higher...

accending:	move.l	d0,d1
		swap	d1
copyloop1:	move.l	(a0)+,(a1)+
		dbra	d0,copyloop1
		dbra	d1,copyloop1
		;;;rts			   ;exit _memcopyl
		bra.s	exit

decending:	add.l	d1,a0		    ;add true count to source
		add.l	d1,a1		    ;add true count to destination
		move.l	d0,d1		    ;spl
		swap	d1		    ;	it count
copyloop2:	move.l	-(a0),-(a1)
		dbra	d0,copyloop2
		dbra	d1,copyloop2

exit:		;;;;rts 		    ;exit _memcopyl

		bra.s	MagicResetCode	    ;Branch to aligned function
		nop

****************************************************************************
*
*   NAME
*	ColdReboot - reboot the Amiga
*
*   SYNOPSIS
*	ColdReboot()
*
*	void ColdReboot(void);
*
*   FUNCTION
*	Reboot the machine.  All external memory and peripherals will be
*	RESET, and the machine will start its power up diagnostics.
*
*	The MagicResetCode must be used exactly as specified here.
*	The MagicResetCode must be longword aligned.  Failure to
*	duplicate the code EXACTLY will result in improper operation
*	under certain system configurations.
*
*   RESULT
*	This function never returns.
*
****************************************************************************



_ColdReboot:

		move.l	4,a6			;Get a pointer to ExecBase
		lea.l	MagicResetCode(pc),a5   ;Location of code to RUN
		jsr	_LVOSupervisor(a6)      ;RUN code in Supervisor mode

;-------------- MagicResetCode ---------DO NOT CHANGE-----------------------

		CNOP	0,4	;IMPORTANT!  Longword align!  Do not change!
MagicResetCode:
		lea.l	2,a0	;Point to JMP instruction at start of ROM
		RESET		;all RAM goes away now!
		jmp	(a0)    ;Rely on prefetch to execute this instruction

;---------------------------------------DO NOT CHANGE-----------------------


		END
