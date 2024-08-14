*----------------------------------------------------------------
*
*  CIA Resource Data Definition
*
*----------------------------------------------------------------


; DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER
;
;	The CiaBase definition was mistakenly published pre 1.3
;
; DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER



;	IFNE	(VERSION-36)
; STRUCTURE	CIAR,LIB_SIZE
;	APTR	CR_HWADDR	; This must be first!
;	; from here on, everything is private
;	STRUCT	CR_INTNODE,IS_SIZE
;	; now longword aligned
;	STRUCT	CR_IVTA,IV_SIZE
;	STRUCT	CR_IVTB,IV_SIZE
;	STRUCT	CR_IVALRM,IV_SIZE
;	STRUCT	CR_IVSP,IV_SIZE
;	STRUCT	CR_IVFLG,IV_SIZE
;	APTR	CR_EXECBASE
;	UWORD	CR_IntMask
;	UBYTE	CR_IEnable
;	UBYTE	CR_IActive
;	UBYTE	CR_IProcessing
;	LABEL	CR_SIZE
;
;	ENDC



;	IFEQ	(VERSION-36)
 STRUCTURE	CIAR,LIB_SIZE
	APTR	CR_HWADDR
	UWORD	CR_IntMask
	UBYTE	CR_IEnable
	UBYTE	CR_IActive
	STRUCT	CR_INTNODE,IS_SIZE
	STRUCT	CR_IVTA,IV_SIZE
	STRUCT	CR_IVTB,IV_SIZE
	STRUCT	CR_IVALRM,IV_SIZE
	STRUCT	CR_IVSP,IV_SIZE
	STRUCT	CR_IVFLG,IV_SIZE
	APTR	CR_EXECBASE

;-- New
	APTR	CR_SharedData		;ptr to shared global data
	UBYTE	CR_IProcessing
	UBYTE	CR_Allocated		; track allocated bits
	UBYTE	CR_TimerAlloc		; track which timer.device using
	UBYTE	CR_TimerINT		; track timer interrupts (CIAA only)

	LABEL	CR_SIZE

;	ENDC

;-- Shared data - used by the jumpy timer.device

 STRUCTURE	CIAShared,0

	APTR	cs_CIAB		;ptr to CIAB base
	APTR	cs_CIAA		;ptr to CIAA base

	;***WARNING
	;these are filled in via MOVEM for speed - don't assume too
	;much about the order.  Three (3) long words for timer CIA
	;ptr, CIA bit number, and index number which is used as
	;a comparison value (e.g., if the next free count-down timer
	;is a better choice for timer.device than what timer.device
	;is currently using).

	STRUCT	cs_TimerUsage,12

	;***NOTE***
	;these are filled in by the timer.device at init time before
	;there is any possible contention over who gets which interrupt
	;vectors - TimerBase & TimerCode MUST be in the order presented
	;here (use MOVEM - order like an Interrupt structure)

	APTR	cs_TimerCall	;call back hook - call timer swap cia code
	APTR	cs_TimerBase	;stashed copy of timer base
	APTR	cs_TimerCode	;interrupt code to call in timer.

	LABEL	cs_SIZEOF

PREFSCOUNT	EQU	3	;4-1 for loop, and index #
TIMERINT	EQU	0	;bit number

