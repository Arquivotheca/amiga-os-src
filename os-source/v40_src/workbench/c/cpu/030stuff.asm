CALLSYS macro   *
	jsr     LVO\1(A6)
	endm

CIB_ENABLE	EQU	0
CIB_FREEZE	EQU	1
CIB_ENTRY	EQU	2
CIB_CLEAR	EQU	3
CIB_BURST	EQU	4

CDB_ENABLE	EQU	8
CDB_FREEZE	EQU	9
CDB_ENTRY	EQU	10
CDB_CLEAR	EQU	11
CDB_BURST	EQU	12
CDB_WALLOC	EQU	13

AFB_68030	EQU	2
AFB_68040	EQU	3

ATNFLGS		EQU	$129

LVOSupervisor	EQU	-30
LVOFindTask	EQU	-294
LVOAllocTrap	EQU	-342
LVOFreeTrap	EQU	-348

NOLIST
	include "exec/execbase.i"
	include "exec/tasks.i"
LIST

	section text,code

	XDEF	_GetMMUType	; Returns the type of MMU
	XDEF	_GetCRP		; Gets MMU CRP register
	XDEF	_SetCRP		; Sets MMU CRP register
	XDEF	_SetSRP		; Sets MMU SRP register
	XDEF	_GetTC		; Gets MMU TC register
	XDEF	_SetTC		; Sets MMU TC register
	XDEF	_GetVBR		; Gets the VBR register
	XDEF	_SetVBR		; Sets the VBR register


;**********************************************************************
;
;	This section contains functions that identify and operate on
;	MMU things.  Unfortunately, there aren't any MMU op-codes in
;	the Manx assembler yet, so I have to fudge them here.
;
;**********************************************************************


;======================================================================
;
;	This function returns 0L if the system contains no MMU,
;	68851L if the system does contain an 68851, or 68030L if the
;	system contains a 68030.
;
;	This routine seems to lock up on at least some CSA 68020
;	boards, though it runs just fine on those from Ronin and
;	Commodore, as well as all 68030 boards it's been tested on.
;
;	ULONG GetMMUType()
;
;
;	NOTE
;		If a >=68040 is installed, GetMMUType now returns zero.
;		The rest of CPU assumes a 68851/68030 type MMU.
;				-Bryce
;======================================================================

_GetMMUType:
	movem.l	a3-a6,-(sp)		; Save this stuff
	move.l	4,a6			; Get ExecBase
	moveq	#0,d0			; D0=Return Code
	btst.b	#AFB_68040,ATNFLGS(a6)	; Do we have a 68040?
	bne.s	4$			; Branch if so
	move.l	#68030,d0		; D0=Return Code
	btst.b	#AFB_68030,ATNFLGS(a6)	; Do we have a 68030?
	bne.s	4$			; Branch if so

	suba.l	a1,a1
	CALLSYS	FindTask		; Call FindTask(0L)
	move.l	d0,a3
	move.l	TC_TRAPCODE(a3),a4	; Change the exception vector
	move.l	#2$,TC_TRAPCODE(a3)
	subq.l	#4,sp			; Let's try an MMU instruction
	dc.w	$f017			; Slimey PMOVE tc,(sp)
	dc.w	$4200
1$
	addq.l	#4,sp			; Return that local
	move.l	a4,TC_TRAPCODE(a3)	; Reset exception stuff
4$
	movem.l	(sp)+,a3-a6		; and return the registers
	rts

	; This is the exception code.  No matter what machine we're on,
	; we get an exception.  If the MMU's in place, we should get a
	; privilige violation; if not, an F-Line emulation exception.
2$
	move.l	(sp)+,d0		; Get Amiga supplied exception #
	cmpi	#11,d0			; Is it an F-Line?
	beq.s	3$			; If so, go to the fail routine
	move.l	#68851,d0		; We have MMU
	addq.l	#4,2(sp)		; Skip the MMU instruction
	rte
3$
	moveq.l	#0,d0			; It dinna woik,
	addq.l	#4,2(sp)		; Skip the MMU instruction
	rte


;======================================================================
;
;	This function returns the MMU CRP register.  It assumes a 68020
;	system with MMU, or a 68030 based system (eg, test for MMU before
;	you call this, or you wind up in The Guru Zone).  Note that the
;	CRP register is two longwords long.
;
;	void GetCRP(ULONG *)
;
;======================================================================

_GetCRP:
	move.l	4(sp),a0		; Pointer to the CRP storage area
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	lea	2$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	move.l	(sp)+,a5
	rts
2$
	pmove	crp,(a0)
	rte


;======================================================================
;
;	This function sets the MMU CRP register.  It assumes a 68020
;	system with MMU, or a 68030 based system (eg, test for MMU before
;	you call this, or you wind up in The Guru Zone).  Note that the
;	CRP register is two longwords long.
;
;	void SetCRP(ULONG *)
;
;======================================================================

_SetCRP:
	move.l	4(sp),a0		; Pointer to the CRP storage area
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	lea	2$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	move.l	(sp)+,a5		; Give back registers
	rts
2$
	pmove	(a0),crp
	rte


;======================================================================
;
;	This function returns the MMU TC register.  It assumes a 68020
;	system with MMU, or a 68030 based system (eg, test for MMU before
;	you call this, or you wind up in The Guru Zone).
;
;	ULONG GetTC()
;
;======================================================================

_GetTC:
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	subq.l	#4,sp			; Make a place to dump TC
	move.l	sp,a0
	lea	2$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	move.l	(sp),d0			; Here's the result
	addq.l	#4,sp
	move.l	(sp)+,a5
	rts
2$
	pmove	tc,(a0)
	rte


;======================================================================
;
;	This function sets the MMU TC register.  It assumes a 68020
;	system with MMU, or a 68030 based system (eg, test for MMU before
;	you call this, or you wind up in The Guru Zone).
;
;	void SetTC(ULONG)
;
;======================================================================

_SetTC:
	lea	4(sp),a0		; Get address of our new TC value
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	lea	2$,a5			; Get the start of the supervisor code
 	CALLSYS	Supervisor
	move.l	(sp)+,a5
	rts
2$
	pmove	(a0),tc
	rte


;======================================================================
;
;	This function returns the VBR register.  It assumes a 68020 or
;	or 68030 based system.
;
;	ULONG GetVBR()
;
;======================================================================

_GetVBR:
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	subq.l	#4,sp			; Make a place to dump VBR
	move.l	sp,a0
	lea	2$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	move.l	(sp),d0			; Here's the result
	addq.l	#4,sp
	move.l	(sp)+,a5
	rts
2$
	movec	vbr,a1
	move.l	a1,(a0)
	rte


;======================================================================
;
;	This function sets the VBR register.  It assumes a 68020 or
;	68030 based system.
;
;	void SetVBR(ULONG)
;
;======================================================================

_SetVBR:
	lea	4(sp),a0		; Get address of our new VBR value
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	lea	2$,a5			; Get the start of the supervisor code
 	CALLSYS	Supervisor
	move.l	(sp)+,a5
	rts
2$
	move.l	(a0),a0
	movec	a0,vbr
	rte


;======================================================================
;
;	This function sets the MMU SRP register. It operates identically
;	to the SetSRP() function.
;
;	void SetSRP(ULONG *)
;
;======================================================================

_SetSRP:
	move.l	4(sp),a0		; Pointer to the CRP storage area
	move.l	4,a6			; Get ExecBase
	move.l	a5,-(sp)
	lea	2$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	move.l	(sp)+,a5		; Give back registers
	rts
2$
	pmove	(a0),srp
	rte

	END
