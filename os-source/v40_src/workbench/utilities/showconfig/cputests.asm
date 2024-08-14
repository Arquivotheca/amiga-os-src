
*	ULONG GetCPUType(void)
*
*		Returns a number, representing the type of CPU in the
*		system: 68000L, 68010L, 68020L, 68030L, or 68040L.
*
*	ULONG GetFPUType(void)
*
*		Returns a number, representing the type of FPU in the
*		system: 0L (no FPU), 68881L, or 68882L.
*
*	ULONG GetMMUType(void)
*
*		Returns a number, representing the type of MMU in the
*		system: 0L (no MMU), 68851L, 68030L, 68040L, or 0xFFFFFFFFL
*		(FPU responding to MMU address!).
*
* modified for 68040, CAS 08/91

;======================================================================
;
;	SetCPU V1.5
;	by Dave Haynie
;
;	68030 Assembly Function Module
;
;	This module contains functions that access functions in the 68020,
;	68030, and 68851 chips, and ID all of these, plus the 68881/68882
;	FPU chips, magic reset stuff, and exception handler.
;
;======================================================================

;======================================================================
;
;	Macros & constants used herein...
;
;======================================================================

	section code

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


ATNFLGS		EQU	$129

LVOSupervisor	EQU	-30
LVOSuperState	EQU	-150
LVOFindTask	EQU	-294
LVOAllocTrap	EQU	-342
LVOFreeTrap	EQU	-348

TAGTC		EQU	28
TAGCRP		EQU	32
TAGBOOT		EQU	62

ANYCREG		EQU	$00dff010

;======================================================================
;
;	Need just a little more stuff
;
;======================================================================

	NOLIST
	include "exec/execbase.i"
	include "exec/tasks.i"
	LIST

*	machine mc68020
*		mc68881


;**********************************************************************
;
;	This section contains functions that identify and operate on CPU
;	things.
;
;**********************************************************************

	XDEF	@GetCPUType	; ID the CPU
	XDEF	@GetFPUType	; ID the FPU
	XDEF	@GetMMUType	; ID the MMU

;======================================================================
;
;	This function returns the type of the CPU in the system as a
;	longword: 68000, 68010, 68020, or 68030.  The testing must be done
;	in reverse order, in that any higher CPU also has the bits set for
;	a lower CPU.  Also, since 1.3 doesn't recognize the 68030, if I
;	find the 68020 bit set, I always check for the presence of a
;	68030.
;
;	This routine should be the first test routine called under 1.2
;	and 1.3.
;
;	ULONG GetCPUType(void);
;
;======================================================================

@GetCPUType:
	movem.l	a4/a5/a6,-(sp)		; Save this register
	move.l	4,a6			; Get ExecBase
	btst.b	#AFB_68040,ATNFLGS(a6)	; Does the OS think an '040 is here?
	beq	30$			; no
	move.l	#68040,d0		; yes
	movem.l	(sp)+,a4/a5/a6
	rts
30$
	btst.b	#AFB_68030,ATNFLGS(a6)	; Does the OS think an '030 is here?
	beq	0$			; nope
	move.l	#68030,d0		; Sure does...
	movem.l	(sp)+,a4/a5/a6
	rts
0$
	btst.b	#AFB_68020,ATNFLGS(a6)	; Maybe a 68020
	bne	2$
	btst.b	#AFB_68010,ATNFLGS(a6)	; Maybe a 68010?
	bne	1$
	move.l	#68000,d0		; Just a measley '000
	movem.l	(sp)+,a4/a5/a6
	rts
1$
	move.l	#68010,d0		; Yup, we're an '010
	movem.l	(sp)+,a4/a5/a6
	rts
2$
	move.l	#68020,d0		; Assume we're an '020
	lea	3$,a5			; Get the start of the supervisor code
	CALLSYS	Supervisor
	movem.l	(sp)+,a4/a5/a6
	rts
3$
	movec	cacr,d1			; Get the cache register
	move.l	d1,a4			; Save it for a minute
	bset.l	#CIB_BURST,d1		; Set the inst burst bit
	bclr.l	#CIB_ENABLE,d1		; Clear the inst cache bit
	movec	d1,cacr			; Try to set the CACR
	movec	cacr,d1
	btst.l	#CIB_BURST,d1		; Do we have a set burst bit?
	beq	4$
	move.l	#68030,d0		; It's a 68030
	bset.b	#AFB_68030,ATNFLGS(a6)
4$
	move.l	a4,d1			; Restore the original CACR
	movec	d1,cacr
	rte


;======================================================================
;
;	This function returns 0L if the system contains no MMU,
;	68851L if the system does contain an 68851, 68030L if the
;	system contains a 68030, or 68040L if a 68040.
;
;	This routine seems to lock up on at least some CSA 68020
;	boards, though it runs just fine on those from Ronin and
;	Commodore, as well as all 68030 boards it's been tested on.
;
;	ULONG GetMMUType(void)
;
;======================================================================

@GetMMUType:
	movem.l	a3/a4/a5/a6,-(sp)		; Save this stuff
	move.l	4,a6			; Get ExecBase
	move.l	#0,a1
	CALLSYS	FindTask		; Call FindTask(0L)
	move.l	d0,a3

	move.l	TC_TRAPCODE(a3),a4	; Change the exception vector
	move.l	#2$,TC_TRAPCODE(a3)

	move.l	#-1,d0			; Try to detect undecode FPU
	subq.l	#4,sp			; Let's try an MMU instruction
	dc.w	$f017			; Slimey PMOVE tc,(sp)
	dc.w	$4200
	cmpi	#0,d0			; Any MMU here?
	beq	1$
	cmpi	#-1,d0			; Hardware bugs?
	beq	1$
	btst.b	#AFB_68040,ATNFLGS(a6)	; Does the OS think an '040 is here?
	beq	31$			; no
	move.l	#68040,d0		; yes
	beq	1$
31$
	btst.b	#AFB_68030,ATNFLGS(a6)	; Does the OS think an '030 is here?
	beq	1$			; no
	move.l	#68030,d0		; yes
1$
	addq.l	#4,sp			; Return that local
	move.l	a4,TC_TRAPCODE(a3)	; Reset exception stuff
	movem.l	(sp)+,a3/a4/a5/a6	; and return the registers
	rts

	; This is the exception code.  No matter what machine we're on,
	; we get an exception.  If the MMU's in place, we should get a
	; privilige violation; if not, an F-Line emulation exception.
2$
	move.l	(sp)+,d0		; Get Amiga supplied exception #
	cmpi	#11,d0			; Is it an F-Line?
	beq	3$			; If so, go to the fail routine
	move.l	#68851,d0		; We have MMU
	addq.l	#4,2(sp)		; Skip the MMU instruction
	rte
3$
	moveq.l	#0,d0			; It dinna woik,
	addq.l	#4,2(sp)		; Skip the MMU instruction
	rte



;======================================================================
;
;	This function returns the type of the FPU in the system as a
;	longword: 0 (no FPU), 68881, or 68882.
;
;	ULONG GetFPUType(void);
;
;======================================================================

@GetFPUType:
	movem.l	a5/a6,-(sp)		; Save this register
	move.l	4,a6			; Get ExecBase
	btst.b	#AFB_68881,ATNFLGS(a6)	; Does the OS think an FPU is here?
	bne.s	1$
	moveq.l	#0,d0			; No FPU here, dude
	movem.l	(sp)+,a5/a6		; Give back the register
	rts
1$:
	btst.b	#AFB_68882,ATNFLGS(a6)	; Does the OS think a 68882 is here?
	bne.s	2$
	move.l	#68881,d0
	movem.l	(sp)+,a5/a6
	rts
2$:
	btst.b	#AFB_FPU40,ATNFLGS(a6)	; Does the OS think a 68040 FPU is here?
	bne.s	3$
	move.l	#68882,d0
	movem.l	(sp)+,a5/a6
	rts
3$:
	move.l	#68040,d0
	movem.l	(sp)+,a5/a6
	rts
