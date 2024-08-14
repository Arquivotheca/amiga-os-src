*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.  *
*									*
*************************************************************************


*************************************************************************
*
* asmsupp.asm
*
* Source Control
* ------ -------
*
* $Header: resource.asm,v 36.7 90/02/28 11:39:03 rsbx Exp $
*
* $Locker:  $
*
*
*************************************************************************

	SECTION BattClock

	INCLUDE 'internal.i'

*------ Imported Functions ------------------------------------------

	XSYS	InitSemaphore
	XSYS	ObtainSemaphore
	XSYS	ReleaseSemaphore
	XSYS	OpenDevice
	XSYS	DoIO
	XSYS	CloseDevice
	XSYS	MakeLibrary
	XSYS	AddResource
	XSYS	OpenLibrary
	XSYS	CloseLibrary
	XSYS	Amiga2Date
	XSYS	CheckDate

*------ Exported ----------------------------------------------------

	XREF	EndCode

*--------------------------------------------------------------------

	moveq.l #-1,d0
	rts

initDDescrip:			;STRUCTURE RT,0
	dc.w	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	dc.l	initDDescrip	; APTR	RT_MATCHTAG
	dc.l	EndCode 	; APTR	RT_ENDSKIP
	dc.b	RTF_COLDSTART	; UBYTE RT_FLAGS
	dc.b	VERSION 	; UBYTE RT_VERSION
	dc.b	NT_DEVICE	; UBYTE RT_TYPE
	dc.b	45		; BYTE	RT_PRI
	dc.l	BattClockName	; APTR	RT_NAME
	dc.l	VERSTRING	; APTR	RT_IDSTRING
	dc.l	InitCode	; APTR	RT_INIT
				; LABEL RT_SIZE



BattClockName:
		BATTCLOCKNAME

timerName:
		TIMERNAME

VERSTRING:
		VSTRING

VERNUM: EQU	VERSION
REVNUM: EQU	REVISION

	ds.w	0


*------ Functions Offsets -------------------------------------

initFuncOki:
	dc.l	ResetBattClockOki
	dc.l	ReadBattClockOki
	dc.l	WriteBattClockOki
	dc.l	Zero
	dc.l	Zero
	dc.l	-1

initFuncRicoh:
	dc.l	ResetBattClockRicoh
	dc.l	ReadBattClockRicoh
	dc.l	WriteBattClockRicoh
	dc.l	Zero
	dc.l	Zero
	dc.l	-1


*------ Initializaton Table -----------------------------------

initStruct:
	INITBYTE	LN_TYPE,NT_RESOURCE
	INITLONG	LN_NAME,BattClockName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERNUM
	INITWORD	LIB_REVISION,REVNUM
	INITLONG	LIB_IDSTRING,VERSTRING

	DC.L		0


RicohBuffSize	equ	14	; must be even
RicohMemSize	equ	13
RicohBitSize	equ	96	; 12*8



*****i* battclock.resource/internal/InitCode *************************
*
*   NAME
*	InitCode -- Initialize the resource from nothing.
*
*   SYNOPSIS
*	InitCode()
*
*	void InitCode( );
*
*   FUNCTION
*	This routine initializes the battclock.resource from nothing.
*
*   INPUTS
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*	A4 -- ClockBase pointer
*	A3 -- utility.library pointer
*	A2 -- battclock.resource pointer
*

InitCode:
	movem.l a2/a3/a4/a6,-(sp)
	move.l	#0,a2		; for no resource case

	move.l	#CLOCKBASE,a4	; now find the clock
	move.l	a4,a0
	bsr	GetClockType
	tst.l	d0		; is one there?
	beq	InitEnd 	; no, no resource
	cmp.l	#OKI_CLOCK,d0	; is it an Oki?
	bne	NotOki		; no
; Oki
	lea	initFuncOki,a0	; point to correct set of offsets
	bsr	MakeResource	; go make and add the resource
	move.l	a4,a0		; where was the clock?
	bsr	ReadOki 	; read the time
	bra	InitCommon
NotOki
	cmp.l	#OKI_RESET,d0	; is it a trashed Oki?
	bne	NotOkiR 	; no
; trashed Oki
	lea	initFuncOki,a0	; point to correct set of offsets
	bsr	MakeResource	; go make and add the resource
	move.l	a4,a0		; where was the clock?
	bsr	ResetOki	; reset the clock chip
	moveq.l #0,d0
	bra	InitCommon
NotOkiR
	cmp.l	#RICOH_CLOCK,d0 ; is it a Ricoh?
	bne	NotRicoh	; no
; Ricoh
	lea	initFuncRicoh,a0	; point to function offsets
	bsr	MakeResource	; go make add the resource
	move.l	a4,a0		; clock address
	bsr	ReadRicoh	; read the time
	bra	InitCommon
NotRicoh
	cmp.l	#RICOH_RESET,d0 ; is it a trashed Ricoh?
	bne	InitEnd 	; no
; trashed Ricoh
	lea	initFuncRicoh,a0	; point to function offsets
	bsr	MakeResource	; go make add the resource
	move.l	a4,a0		; clock address
	bsr	ResetRicoh	; reset the clock chip
	moveq.l #0,d0

InitCommon
	bsr	SetSysTime	; set the system time

InitEnd:
	move.l	a2,d0		; battclockbase or NULL
	movem.l (sp)+,a2/a3/a4/a6
	rts


*****i* battclock.resource/internal/MakeResource *********************
*
*   NAME
*	MakeResource -- Create, initialize, and add battclock.resource
*	    to the system list.
*
*   SYNOPSIS
*	BTC = MakeResource(ClockBase, FuncTable)
*	A2		   A4	      A0
*
*	Struct BattClockResource * MakeResource( APTR, struct Library *, APTR );
*
*   FUNCTION
*	This routine creates and initializes the battclock.resource
*	    structure.
*
*   INPUTS
*	ClockBase -- Pointer to the base address of the clock chip.
*	FuncTable -- Pointer to the function pointer table for
*	    MakeLibrary()
*
*   RESULTS
*	BTC -- Pointer to BattClockResource structure.
*
*   NOTES
*	Battclock.resource properly initialized with resource pointer
*	    returned in A2 and A6. If any error occures, then the
*	    resource will not be created and A2 will be NULL and A6
*	    unchanged.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*	A4 -- ClockBase pointer
*	A2 -- battclock.resource pointer
*

MakeResource:
	lea	initStruct,a1	; initialize the library node
	suba.l	a2,a2
	move.l	#BTC_SIZE,d0
	CALLSYS MakeLibrary	; make us a playpen
	move.l	d0,a2		; save the base
	tst.l	d0		; was there a hitch?
	beq.s	MakeLibFailed	; damn

	move.l	#MAGIC,BTC_Magic(a2)    ; for BattMem
	move.l	a6,BTC_Exec(a2)         ; set our data
	move.l	a4,BTC_ClockBase(a2)
	lea	BTC_Semaphore(a2),a0    ; to prevent conflicts
	CALLSYS InitSemaphore
	move.l	a2,a1
	CALLSYS AddResource	; yea! we made it
	move.l	a2,a6		; a6 now has BattClockBase
	rts

MakeLibFailed:
	move.l	(sp)+,d0        ; dump return address
	bra	InitEnd 	; bye


*****i* battclock.resource/internal/SetSysTime ***********************
*
*   NAME
*	SetSysTime -- Set the system time from the time read from the
*	    clock chip.
*
*   SYNOPSIS
*	SetSysTime( AmigaTime )
*		    D0
*
*	void SetSysTime( ULONG );
*
*   FUNCTION
*	This routine takes a longword and send the timer.device a
*	    TR_SETSYSTIME command with the longword as the seconds
*	    field of the timerequest timeval.
*
*   INPUTS
*	AmigaTime -- Longword containing the number of seconds from
*	01-Jan-1978 to the current time.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*	A3 -- timerequest
*

SetSysTime:
	movem.l d2/a3/a6,-(sp)
	move.l	BTC_Exec(a6),a6         ; find exec
	move.l	d0,d2			; save the time

	lea	-IOTV_SIZE(sp),sp
	movea.l sp,a3			; IORequest

	lea	timerName,a0
	movea.l a3,a1			; this IORequest
	moveq.l #0,d0			; unit doesn't matter
	moveq.l #0,d1			; no special flags
	CALLSYS OpenDevice
	tst.l	d0			; any errors ?
	bne.s	SSTCleanup		; yep

	movea.l a3,a1
	move.w	#TR_SETSYSTIME,IO_COMMAND(a1)   ; this command

	move.l	d2,IOTV_TIME+TV_SECS(a1)        ; systime
	move.l	#0,IOTV_TIME+TV_MICRO(a1)

	CALLSYS DoIO			; doit

	movea.l a3,a1
	CALLSYS CloseDevice		; close the device

SSTCleanup:
	lea.l	IOTV_SIZE(sp),sp        ; clean up stack
	movem.l (sp)+,d2/a3/a6
	rts


*****i* battclock.resource/internal/Zero *****************************
*
*   NAME
*	Zero -- Returns 0 in register D0.
*
*   SYNOPSIS
*	Result = Zero( )
*
*	ULONG Zero( );
*
*   FUNCTION
*	Returns 0.
*
*   INPUTS
*
*   RESULTS
*	Result -- Will be 0.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

Zero:
	moveq.l #0,d0
	rts


*****i* battclock.resource/internal/GetClockType *********************
*
*   NAME
*	GetClockType -- Determine what kind of clock chip is in the
*	    system.
*
*   SYNOPSIS
*	Type = GetCLockType( ClockBase )
*	D0		     A0
*
*	ULONG GetClockType( APTR );
*
*   FUNCTION
*	This routine attempts to determine if a clock chip exists at
*	    ClockBase and what kind it is.
*
*   INPUTS
*	ClockBase -- Pointer to the base address of the clock chip.
*
*   RESULTS
*	Type -- The type of clock chip found or 0.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

GetClockType:
	moveq.l #$f,d1		; data mask

; on a non-trashed Oki, regf will be 4
; on a Ricoh, regf is always read as 0
	move.b	Regf(a0),d0
	and.b	d1,d0
	cmp.b	#4,d0
	beq	Oki

; its either a trashed Oki, a Ricoh or it isn't
	clr.b	Regf(a0)        ; reset test modes
	clr.b	Rege(a0)
	move.b	#9,Regd(a0)     ; try for Ricoh page 1

; on a Ricoh, page 1 regc will always read as 0
	move.b	#5,Regc(a0)
	move.b	Regc(a0),d0
	and.b	d1,d0
	beq	TestRicoh

; its either a trashed oki or not there
TestOki
	move.b	#4,Regd(a0)     ; reset oki
	move.b	#7,Regf(a0)
	move.b	#4,Regf(a0)
	clr.b	Rege(a0)

	move.b	#1,Regf(a0)     ; set 24hr mode
	move.b	#5,Regf(a0)
	move.b	#4,Regf(a0)

; if its an Oki, regf will read as 4
	move.b	Regf(a0),d0
	and.b	d1,d0
	cmp.b	#4,d0
	beq	TrashedOki
NoClock
	moveq.l #NO_CLOCK,d0	; hope its a A500
	rts
TrashedOki
	moveq.l #OKI_RESET,d0	; we found a trashed Oki
	rts
Oki
	moveq.l #OKI_CLOCK,d0	; we found an Oki
	rts

; its either a Ricoh or it isn't there
; if its a Ricoh, regd will be 9
TestRicoh
	move.b	Regd(a0),d0
	and.b	d1,d0
	cmp.b	#9,d0
	bne	NoClock
; check the 24hr bit
	move.b	Rega(a0),d0
	btst	#0,d0
	bne	Ricoh
	moveq.l #RICOH_RESET,d0
	rts
Ricoh
	move.b	#9,Regd(a0)
	moveq.l #RICOH_CLOCK,d0
	rts


******* battclock.resource/ResetBattClock() **************************
*
*   NAME
*	ResetBattClock -- Reset the clock chip.  (V36)
*
*   SYNOPSIS
*	ResetBattClock( )
*
*	void ResetBattClock( );
*
*   FUNCTION
*	This routine does whatever is neeeded to put the clock chip
*	    into a working and usable state and also sets the date on
*	    the clock chip to 01-Jan-1978.
*
*   INPUTS
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ResetBattClockOki:
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address

	bsr	ResetOki		; make it right

	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	rts


ResetBattClockRicoh:
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address

	bsr	ResetRicoh		; make it right

	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	rts


******* battclock.resource/ReadBattClock() ***************************
*
*   NAME
*	ReadBattClock -- Read time from clock chip.  (V36)
*
*   SYNOPSIS
*	AmigaTime = ReadBattClock( )
*
*	ULONG ReadBattClock( );
*	D0
*
*   FUNCTION
*	This routine reads the time from the clock chip and returns
*	    it as the number of seconds from 01-jan-1978.
*
*   INPUTS
*
*   RESULTS
*	AmigaTime -- The number of seconds from 01-Jan-1978 that the
*	    clock chip thinks it is.
*
*   NOTES
*	If the clock chip returns an invalid date, the clock chip is
*	    reset and 0 is returned.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ReadBattClockOki:
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address

	bsr	ReadOki 		; get the time

	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	move.l	(sp)+,d0
	rts


ReadBattClockRicoh:
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address

	bsr	ReadRicoh		; get the time

	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	move.l	(sp)+,d0
	rts


******* battclock.resource/WriteBattClock() **************************
*
*   NAME
*	WriteBattClock -- Set the time on the clock chip.  (V36)
*
*   SYNOPSIS
*	WriteBattClock( AmigaTime )
*			D0
*
*	void WriteBattClock( ULONG );
*
*   FUNCTION
*	This routine writes the time given in AmigaTime to the clock
*	    chip.
*
*   INPUTS
*	AmigaTime -- The number of seconds from 01-Jan-1978 to the
*	    time that should be written to the clock chip.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

WriteBattClockOki:
	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address
	move.l	(sp)+,d0

	bsr	WriteOki		; put the time

	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	rts


WriteBattClockRicoh:
	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0    ; grab the semaphore
	LINKSYS ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0    ; get the clock address
	move.l	(sp)+,d0

	bsr	WriteRicoh		; put the time

	lea	BTC_Semaphore(a6),a0    ; release the semaphore
	LINKSYS ReleaseSemaphore
	rts


*****i* battclock.resource/internal/WriteRawClock ********************
*
*   NAME
*	WriteRawClock -- Write precomputed data to the clock chip.
*
*   SYNOPSIS
*	WriteRawClock( ClockBase, Data )
*		       A0	  A1
*
*	void WriteRawClock( APTR, struct ClockTemp * );
*
*   FUNCTION
*	Writes precomputed and formatted data to the clock chip.
*
*   INPUTS
*	ClockBase -- Pointer to the clock chip base address.
*	Data -- Pointer to the data to be written to the clock chip.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

WriteRawClock:
	move.b	(a1)+,Reg0(a0)  ;sec    ;sec
	move.b	(a1)+,Reg1(a0)
	move.b	(a1)+,Reg2(a0)  ;min    ;min
	move.b	(a1)+,Reg3(a0)
	move.b	(a1)+,Reg4(a0)  ;hour   ;hour
	move.b	(a1)+,Reg5(a0)
	move.b	(a1)+,Reg6(a0)  ;mday   ;wday
	move.b	(a1)+,Reg7(a0)          ;mday
	move.b	(a1)+,Reg8(a0)  ;month
	move.b	(a1)+,Reg9(a0)          ;month
	move.b	(a1)+,Rega(a0)  ;year
	move.b	(a1)+,Regb(a0)          ;year
	move.b	(a1)+,Regc(a0)  ;wday
	rts


*****i* battclock.resource/internal/ReadRawClock *********************
*
*   NAME
*	ReadRawClock -- Read data from the clock chip.
*
*   SYNOPSIS
*	ReadRawClock( ClockBase, Data )
*		      A0	 A1
*
*	void ReadRawClock( APTR, struct ClockTemp * );
*
*   FUNCTION
*	Reads data from the clock chip.
*
*   INPUTS
*	ClockBase -- Pointer to the clock chip base address.
*	Data -- Pointer to where the data read from the clock chip
*	    should be written.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ReadRawClock:
	moveq.l #$f,d0
	move.b	Reg0(a0),(a1)   ;sec    ;sec
	and.b	d0,(a1)+
	move.b	Reg1(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg2(a0),(a1)   ;min    ;min
	and.b	d0,(a1)+
	move.b	Reg3(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg4(a0),(a1)   ;hour   ;hour
	and.b	d0,(a1)+
	move.b	Reg5(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg6(a0),(a1)   ;mday   ;wday
	and.b	d0,(a1)+
	move.b	Reg7(a0),(a1)           ;mday
	and.b	d0,(a1)+
	move.b	Reg8(a0),(a1)   ;month
	and.b	d0,(a1)+
	move.b	Reg9(a0),(a1)           ;month
	and.b	d0,(a1)+
	move.b	Rega(a0),(a1)   ;year
	and.b	d0,(a1)+
	move.b	Regb(a0),(a1)           ;year
	and.b	d0,(a1)+
	move.b	Regc(a0),(a1)   ;wday
	and.b	d0,(a1)+
	rts


*****i* battclock.resource/internal/Bin2Bcd **************************
*
*   NAME
*	Bin2Bcd -- Convert a binary number to 2 BCD digits.
*
*   SYNOPSIS
*	Bin2Bcd( Num, Ten, Where )
*		 D0   D1   A2
*
*	void Bin2Bcd( ULONG, UWORD, APTR );
*
*   FUNCTION
*	Converts a binary number to 2 BCD digits and stores them is
*	    separate bytes.
*
*   INPUTS
*	Num -- Binary number to convert.
*	Ten -- 10.
*	Where -- Location to write the 2 BCD digits.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

Bin2Bcd:
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+
	swap.w	d0
	move.b	d0,(a2)+
	rts


*****i* battclock.resource/internal/Bcd2Bin **************************
*
*   NAME
*	Bcd2Bin -- Convert a BCD number to binary.
*
*   SYNOPSIS
*	Num = Bcd2Bin( Ten, Where )
*	D0	       D1   A2
*
*	ULONG Bcd2Bin( UWORD, APTR );
*
*   FUNCTION
*	Converts a BCD number stored as 2 BCD digits in separate byte
*	    to binary.
*
*   INPUTS
*	Ten -- 10.
*	Where -- Location of the 2 BCD digits.
*
*   RESULTS
*	Num -- Binary result.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

Bcd2Bin:
	moveq.l #0,d0
	move.b	(a2)+,d2
	move.b	(a2)+,d0
	mulu.w	d1,d0
	add.b	d2,d0
	rts


*****i* battclock.resource/internal/SetOkiHold ***********************
*
*   NAME
*	SetOkiHold -- Get the Oki clock chip to stop ticking.
*
*   SYNOPSIS
*	SetOkiHold( ClockBase )
*		    A0
*
*	void SetOkiHold( APTR );
*
*   FUNCTION
*	Sets the hold bit on an Oki clock chip so that the clock data
*	    can br read or written.
*
*   INPUTS
*	ClockBase -- Base address of Oki clock chip.
*
*   RESULTS
*
*   NOTES
*	If the hold cannot be achieved in 400 tries, the chip is reset
*	    and another attempt is made. This may trash the time!
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

SetOkiHold:
	moveq.l #1,d0		; for convenience
	move.l	#400,d1 	; timeout
SOHls1
	move.b	d0,Regd(a0)     ; set holdbit
	btst.b	d0,Regd(a0)     ; still busy?
	beq.s	HaveOkiHold	; no, got it
	clr.b	Regd(a0)        ; reset holdbit
	dbf	d1,SOHls1	; loop

	; if we get here then there is a problem getting the hold
	;  so we try reseting the chip (maybe trashing the time!)
	move.b	#4,Regd(a0)
	move.b	#7,Regf(a0)
	move.b	#4,Regf(a0)
	clr.b	Rege(a0)

	move.b	d0,Regf(a0)
	move.b	#5,Regf(a0)
	move.b	#4,Regf(a0)
	; and try for the hold again

	move.l	#400,d1
SOHls2
	move.b	d0,Regd(a0)     ; set holdbit
	btst.b	d0,Regd(a0)     ; still busy?
	beq.s	HaveOkiHold	; no, got it
	clr.b	Regd(a0)        ; reset holdbit
	dbf	d1,SOHls2	; loop
	; if we fall out here, give up, hopefully the user will notice
	; that the time is wrong

HaveOkiHold
	rts


*****i* battclock.resource/internal/WriteOki *************************
*
*   NAME
*	WriteOki -- Set the time on an Oki clock chip.
*
*   SYNOPSIS
*	WriteOki( AmigaTime, ClockBase )
*		  D0	     A0
*
*	void WriteOki( ULONG, APTR );
*
*   FUNCTION
*	Sets the time on an Oki clock chip to that given in AmigaTime.
*
*   INPUTS
*	AmigaTime -- The time to write to the clock chip given as the
*	    number of seconds from 01-Jan-1978.
*	ClockBase -- Base address of Oki clock chip.
*
*   RESULTS
*
*   NOTES
*	The Oki clock chip only stores the last 2 digits to the year,
*	    so valid dates are 01-Jan-1978 to 31-Dec-2077.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

WriteOki:
	move.l	a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp   ; get some temp areas
	move.l	sp,a2			; clocktemp struct
	move.l	a0,-(sp)                ; save clockbase
	lea	ClockTempSize(a2),a0    ; ClockData struct
	bsr	Amiga2Date		; fill in ClockData from AmigaTime
	lea	ClockTempSize(a2),a1    ; ClockData struct (same one)
	move.l	(sp)+,a0                ; clockbase

	; convert ClockData to clocktemp (correctly formatted)
	moveq.l #10,d1			; for convenience

	moveq.l #0,d0
	move.w	sec(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	min(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	hour(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	mday(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	month(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	year(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+
	clr.w	d0
	swap.w	d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	moveq.l #0,d0
	move.w	wday(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	move.l	sp,a2		; clocktemp (again)

	; what the hey, reset the thing
	move.b	#4,Regd(a0)
	move.b	#7,Regf(a0)
	move.b	#4,Regf(a0)
	clr.b	Rege(a0)

	move.b	#1,Regf(a0)
	move.b	#5,Regf(a0)
	move.b	#4,Regf(a0)

	bsr	SetOkiHold	; hold it
	exg.l	a2,a1
	bsr	WriteRawClock	; write the data
	exg.l	a2,a1
	clr.b	Regd(a0)        ; Release Oki hold

	lea	CD_SIZE+ClockTempSize(sp),sp
	move.l	(sp)+,a2
	rts

*****i* battclock.resource/internal/ReadOki **************************
*
*   NAME
*	ReadOki -- Get the time from an Oki clock chip.
*
*   SYNOPSIS
*	AmigaTime = ReadOki( ClockBase )
*			     A0
*
*	ULONG ReadOki( APTR );
*
*   FUNCTION
*	Returns the time from an Oki clock chip.
*
*   INPUTS
*	ClockBase -- Base address of Oki clock chip.
*
*   RESULTS
*	AmigaTime -- The time from the clock chip given as the
*	    number of seconds from 01-Jan-1978.
*
*   NOTES
*	The Oki clock chip only stores the last 2 digits to the year,
*	    so valid dates are 01-Jan-1978 to 31-Dec-2077.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ReadOki:
	movem.l d2/a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp   ; get some temp
	move.l	sp,a1			; clocktemp struct
	lea	ClockTempSize(a2),a2    ; ClockData struct

	bsr	SetOkiHold	; stop ticking
	bsr	ReadRawClock	; get the data
	exg.l	a2,a1		; ClockData to a1
	clr.b	Regd(a0)        ; Release Oki hold

	; convert clocktemp to ClockData
	move.l	sp,a2
	moveq.l #10,d1

	bsr	Bcd2Bin
	move.w	d0,sec(a1)

	bsr	Bcd2Bin
	move.w	d0,min(a1)

	bsr	Bcd2Bin
	move.w	d0,hour(a1)

	bsr	Bcd2Bin
	move.w	d0,mday(a1)

	bsr	Bcd2Bin
	move.w	d0,month(a1)

	bsr	Bcd2Bin
	add.w	#1900,d0
	cmp.w	#1978,d0
	bcc.s	OkiNow
	add.w	#100,d0
OkiNow
	move.w	d0,year(a1)

	moveq.l #0,d0
	move.b	(a2)+,d0
	move.w	d0,wday(a1)

	lea	ClockTempSize(sp),sp    ; give back some temp
	move.l	a0,a2		; save clockbase
	move.l	a1,a0		; clockdata
	bsr	CheckDate	; convert & check date
	move.l	a2,a0		; clockdata
	tst.l	d0		; good date?
	bne	ReadCommon	; yes

	bsr	ResetOki	; bad date
	moveq.l #0,d0


ReadCommon:
	lea	CD_SIZE(sp),sp  ; return temp
	movem.l (sp)+,d2/a2
	rts


*****i* battclock.resource/internal/ResetOki *************************
*
*   NAME
*	ResetOki -- Reset the Oki clock chip.
*
*   SYNOPSIS
*	ResetOki( ClockBase )
*		  A0
*
*	void ResetOki( APTR );
*
*   FUNCTION
*	Resets the Oki clock chip to a usable state and sets the date
*	    on the chip to 01-Jan-1978.
*
*   INPUTS
*	ClockBase -- Base address of Oki clock chip.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ResetOki:
	move.b	#4,Regd(a0)
	move.b	#7,Regf(a0)
	move.b	#4,Regf(a0)
	clr.b	Rege(a0)

	move.b	#1,Regf(a0)
	move.b	#5,Regf(a0)
	move.b	#4,Regf(a0)

	moveq.l #0,d0
	bsr	WriteOki	; put the date
	rts


*****i* battclock.resource/internal/ResetRicoh ***********************
*
*   NAME
*	ResetRicoh -- Reset the Ricoh clock chip.
*
*   SYNOPSIS
*	ResetRicoh( ClockBase )
*		    A0
*
*	void ResetRicoh( APTR );
*
*   FUNCTION
*	Resets the Ricoh clock chip to a usable state and sets the date
*	    on the chip to 01-Jan-1978.
*
*   INPUTS
*	ClockBase -- Base address of Ricoh clock chip.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ResetRicoh
	clr.b	Rege(a0)
	move.b	#1,Regd(a0)
	move.b	#3,Regf(a0)
	clr.b	Regf(a0)
	move.b	#1,Rega(a0)
	move.b	#9,Regd(a0)

	moveq.l #0,d0
	bsr	WriteRicoh	; put the date
	rts


*****i* battclock.resource/internal/WriteRicoh ***********************
*
*   NAME
*	WriteOki -- Set the time on an Ricoh clock chip.
*
*   SYNOPSIS
*	WriteRicoh( AmigaTime, ClockBase )
*		    D0	       A0
*
*	void WriteRicoh( ULONG, APTR );
*
*   FUNCTION
*	Sets the time on an Ricoh clock chip to that given in AmigaTime.
*
*   INPUTS
*	AmigaTime -- The time to write to the clock chip given as the
*	    number of seconds from 01-Jan-1978.
*	ClockBase -- Base address of Ricoh clock chip.
*
*   RESULTS
*
*   NOTES
*	The Ricoh clock chip only stores the last 2 digits to the year,
*	    so valid dates are 01-Jan-1978 to 31-Dec-2077.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

WriteRicoh:
	move.l	a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp   ; need some temp
	move.l	sp,a2			; clockTemp
	move.l	a0,-(sp)                ; save clockbase
	lea	ClockTempSize(a2),a0    ; ClockData struct
	bsr	Amiga2Date		; fill in ClockData struct
	lea	ClockTempSize(a2),a1    ; ClockData struct (again)
	move.l	(sp)+,a0        ; ClockBase

	; convert ClockData to ClockTemp (formatted correctly)
	moveq.l #10,d1

	moveq.l #0,d0
	move.w	sec(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	min(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	hour(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	wday(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	moveq.l #0,d0
	move.w	mday(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	month(a1),d0
	bsr	Bin2Bcd

	moveq.l #0,d0
	move.w	year(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+
	clr.w	d0
	swap.w	d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	move.l	sp,a2		; ClockTemp

	clr.b	Rege(a0)
	clr.b	Regd(a0)        ; set hold
	exg.l	a2,a1
	bsr	WriteRawClock	; write the data to the chip
	exg.l	a2,a1
	move.b	#1,Regd(a0)
	move.b	#1,Rega(a0)     ; set 24hr mode
	move.b	1+year(a1),Regb(a0)     ; set the leapyear counter
	move.b	#3,Regf(a0)     ; clear intermediate counts
	clr.b	Regf(a0)
	move.b	#9,Regd(a0)     ; release hold

	lea	CD_SIZE+ClockTempSize(sp),sp
	move.l	(sp)+,a2
	rts


*****i* battclock.resource/internal/ReadRicoh ************************
*
*   NAME
*	ReadRicoh -- Get the time from an Ricoh clock chip.
*
*   SYNOPSIS
*	AmigaTime = ReadRicoh( ClockBase )
*			       A0
*
*	ULONG ReadRicoh( APTR );
*
*   FUNCTION
*	Returns the time from an Ricoh clock chip.
*
*   INPUTS
*	ClockBase -- Base address of Ricoh clock chip.
*
*   RESULTS
*	AmigaTime -- The time from the clock chip given as the
*	    number of seconds from 01-Jan-1978.
*
*   NOTES
*	The Ricoh clock chip only stores the last 2 digits to the year,
*	    so valid dates are 01-Jan-1978 to 31-Dec-2077.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

ReadRicoh:
	movem.l d2/a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp   ; need temp
	move.l	sp,a1			; clocktemp struct
	lea	ClockTempSize(a2),a2    ; ClockData struct

	clr.b	Regd(a0)        ; set hold
	bsr	ReadRawClock	; get the raw clock data
	exg.l	a2,a1
	move.b	#9,Regd(a0)     ; release hold

	move.l	sp,a2		; ClockTemp

	; convert ClockTemp into ClockData
	moveq.l #10,d1

	bsr	Bcd2Bin
	move.w	d0,sec(a1)

	bsr	Bcd2Bin
	move.w	d0,min(a1)

	bsr	Bcd2Bin
	move.w	d0,hour(a1)

	moveq.l #0,d0
	move.b	(a2)+,d0
	move.w	d0,wday(a1)

	bsr	Bcd2Bin
	move.w	d0,mday(a1)

	bsr	Bcd2Bin
	move.w	d0,month(a1)

	bsr	Bcd2Bin
	add.w	#1900,d0
	cmp.w	#1978,d0
	bcc.s	RicohNow
	add.w	#100,d0
RicohNow
	move.w	d0,year(a1)

	lea	ClockTempSize(sp),sp    ; return some temp
	move.l	a0,a2		; clockbase
	move.l	a1,a0		; ClockData struct
	bsr	CheckDate	; is the date good?
	move.l	a2,a0		; clockbase
	tst.l	d0		; good date?
	bne	ReadCommon	; yes

	bsr	ResetRicoh	; BZZZZZZZT, no reset the chip
	moveq.l #0,d0
	bra	ReadCommon


;
;	struct ClockData
;		{
;		UWORD	sec;
;		UWORD	min;
;		UWORD	hour;
;		UWORD	mday;
;		UWORD	month;
;		UWORD	year;
;		UWORD	wday;
;		};
;
;
;	/*
;	 * an interesting and useful macro I designed (the 'function' form
;	 * is much simpler, but for constants everthing is resolved at compile
;	 * time).
;	 */
;	#define CALC_DATE(YEAR,MONTH,DAY) \
;	  ( \
;	  ((YEAR)-1+((MONTH)+9)/12)*365L \
;	  +((YEAR)-1+((MONTH)+9)/12)/4 \
;	  -((YEAR)-1+((MONTH)+9)/12)/100 \
;	  +((YEAR)-1+((MONTH)+9)/12)/400 \
;	  +((((MONTH)+9)%12)*306+5)/10 \
;	  +(DAY) \
;	  -1 \
;	 )
;

*****i* utility.library/CalcDate() ***********************************
*
*   NAME
*	CalcDate -- Returns a relative day number based on date.  (V36)
*
*   SYNOPSIS
*	Index = CalcDate( Date )
*	D0		  A0
*
*	ULONG CalcDate( struct ClockData * );
*
*   FUNCTION
*	Calculates the relative day number of the date specified relative
*	    to a date a long time ago.
*
*   INPUTS
*	Date -- pointer to a ClockData structure.
*
*   RESULTS
*	Index -- The number of days since a date a long time ago.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	ULONG __asm CalcDate(register __d0 ULONG year, register __d1 ULONG month, register __d2 ULONG day)
;		{
;		month += 9;
;		year = year-1 + (month/12);
;		month %= 12;
;		month = month*306 +5;
;
;		return (year*365) + (year/4) - (year/100) + (year/400)
;			+ (month/10) + day -1;
;		}

CalcDate:
	MOVE.L	D2,-(A7)
	MOVE.L	D0,D2		; d2=year
	MOVEQ	#09,D0
	ADD.L	D1,D0		; d0=month+9
	MOVEQ	#12,D1
	divul.l d1,d1:d0	; d1=(month+9)%12
	ADD.L	D0,D2		; d2=year+(month+9)/12
	MOVE.L	#306,D0
	mulu.l	d1,d0		; d0=((month+9)%12)*306
	ADDQ.L	#5,D0		; d0=((month+9)%12)*306+5
	MOVEQ	#10,D1
	divul.l d1,d1:d0	; d0=(((month+9)%12)*306+5)/10
	MOVE.L	D0,-(SP)
	SUBQ.L	#1,D2		; d2=year+(month+9)/12 -1
	MOVE.L	D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ	#100,D1
	LSL.L	#2,D1
	divul.l d1,d1:d0	; d0=(year+(month+9)/12-1)/400
	MOVE.L	D0,-(SP)
	MOVE.L	D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ	#100,D1
	divul.l d1,d1:d0	; d0=(year+(month+9)/12-1)/100
	MOVE.L	D0,-(SP)
	MOVE.L	D2,D1		; d1=year+(month+9)/12-1
	LSR.L	#2,D2		; d2=(year+(month+9)/12-1)/4
	MOVE.L	#365,D0
	mulu.l	d1,d0		; d0=(year+(month+9)/12-1)*365
	ADD.L	D2,D0		; d0=d0 + (year+(month+9)/12-1)/4
	SUB.L	(SP)+,D0        ; d0=d0 - (year+(month+9)/12-1)/100
	ADD.L	(SP)+,D0        ; d0=d0 + (year+(month+9)/12-1)/400
	ADD.L	(SP)+,D0        ; d0=d0 + (((month+9)%12)*306+5)/10
	MOVE.L	(A7)+,D2
	ADD.L	D2,D0		; d0=d0 + day
	SUBQ.L	#1,D0		; d0=d0 -1
	RTS


******* utility.library/Amiga2Date() *********************************
*
*   NAME
*	Amiga2Date -- Calculate the date from a timestamp.  (V36)
*
*   SYNOPSIS
*	Amiga2Date( AmigaTime, Date )
*		    D0	       A0
*
*	void Amiga2Date( ULONG, struct ClockData * );
*
*   FUNCTION
*	Fills in a ClockData structure with the date and time calculated
*	    from a ULONG containing the number of seconds from 01-Jan-1978
*	    to the date.
*
*   INPUTS
*	AmigaTime -- The number of seconds from 01-Jan-1978.
*
*   RESULTS
*	Date -- Filled in with the date/time specified by AmigaTime.
*
*   NOTES
*
*   SEE ALSO
*	CheckDate(), Date2Amiga()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	void __asm Amiga2Date(register __d0 ULONG amiga, register __a0 struct ClockData *cd)
;	  {
;	   LONG year;
;	   LONG month;
;	   LONG day;
;	   LONG hour;
;	   LONG minute;
;	   LONG second;
;	   LONG absday;
;	   LONG work;
;
;
;	   second  = amiga % 60;
;	   minute  = amiga / 60;
;	   hour    = minute / 60;
;	   minute %= 60;
;	   hour   %= 24;
;
;	   absday  = amiga / (60 * 60 * 24);
;	   absday += CALC_DATE(1978,1,1);      /* add the Amiga date offset */
;
;	   cd->wday  = (absday+3)%7
;
;	   /*
;	    * the following date logic took a great deal of time to develop and prove
;	    * so please give credit if you use it. That also includes the macro
;	    * in AmigArc.h.
;	    * NOTE: the following were split into separate lines for aestetic(sp?)
;	    * reasons as well as to prevent the (sometimes very stupid!) 'C'
;	    * optimizer from changing the math (implicit floor function is very
;	    * important!).
;	    */
;
;	   /* find year */
;	   work = absday;
;	   work -= (absday + 1) / 146097;   /* subtract quadcentury leap days */
;	   work += work / 36524;	    /* add century leap days */
;	   work -= (work + 1) / 1461;       /* subtract all leap days */
;	   year = work / 365;
;
;	   /* find day of year */
;	   work = absday;
;	   work -= year * 365;
;	   work -= year / 4;
;	   work += year / 100;
;	   work -= year / 400;
;
;	   /* find month */
;	   month = work / 153;
;	   month *= 5;
;	   month += 10 * (work % 153) / 305;
;
;	   /* find day */
;	   day = 1 + work;
;	   day -= (long)((month * 306 + 5) / 10);
;
;	   /* adjust to Gregorian calendar */
;	   month += 2;
;	   year += month / 12;
;	   month %= 12;
;	   month++;
;
;	   cd->sec   = second;
;	   cd->min   = minute;
;	   cd->hour  = hour;
;	   cd->day   = day;
;	   cd->month = month;
;	   cd->year  = year;
;	  }

Amiga2Date:
	MOVEM.L D2-D3,-(A7)
	MOVEQ	#60,D1
	divul.l d1,d1:d0
	MOVE.W	D1,sec(A0)      ; d1=amiga%60
	MOVEQ	#60,D1	; d0=amiga/60
	divul.l d1,d1:d0
	MOVE.W	D1,min(A0)      ; d1=(amiga/60)%60
	MOVEQ	#24,D1	; d0=amiga/3600
	divul.l d1,d1:d0
	MOVE.W	D1,hour(A0)     ; d1=(amiga/3600)%24
	ADDI.L	#722390,D0	; d0=absday
	MOVE.L	D0,-(SP)
	MOVE.L	D0,D2
	ADDQ.L	#3,D0		; offset so sunday = 0
	MOVEQ.L #7,d1
	divul.l d1,d1:d0	; d1=dayofweek
	MOVE.W	d1,wday(A0)
	MOVE.L	D2,D0
	ADDQ.L	#1,D0
	MOVE.L	#146097,D1
	divul.l d1,d1:d0	; d0=(absday+1)/146097
	SUB.L	D0,D2		; work=absday - (absday+1)/146097
	MOVE.L	D2,D0
	MOVE.L	#36524,D1
	divul.l d1,d1:d0
	ADD.L	D0,D2		; work += work/36524
	MOVE.L	D2,D0
	ADDQ.L	#1,D0
	MOVE.L	#1461,D1
	divul.l d1,d1:d0
	SUB.L	D0,D2		; work -= (work+1)/1461
	MOVE.L	D2,D0
	MOVE.L	#365,D1
	divul.l d1,d1:d0

	MOVE.L	(SP)+,D2        ; work = absday
	MOVE.L	D0,-(SP)        ; save year
	MOVE.L	D0,D3
	MOVEQ	#100,D1
	LSL.L	#2,D1
	divul.l d1,d1:d0	; d0=year/400
	SUB.L	D0,D2		; work-=year/400
	MOVE.L	D3,D0
	MOVEQ	#100,D1
	divul.l d1,d1:d0	; d0=year/100
	ADD.L	D0,D2		; work+=year/100
	MOVE.L	D3,D0
	LSR.L	#2,D0
	SUB.L	D0,D2		; work-=year/4
	MOVE.L	D3,D0
	MOVE.L	#365,D1
	mulu.l	d1,d1:d0	; d0=year*365
	SUB.L	D0,D2		; d2=work-=year*365

	MOVE.L	D2,D0
	MOVEQ	#102,D1
	NOT.B	D1
	divul.l d1,d1:d0	; d1=work%153
	MOVEQ	#10,D0
	mulu.l	d1,d1:d0	; d0=10*(work%153)
	MOVE.L	#305,D1
	divul.l d1,d1:d0	; d0=10*(work%153)/305
	MOVE.L	D0,D3
	MOVE.L	D2,D0
	MOVEQ	#102,D1
	NOT.B	D1
	divul.l d1,d1:d0	; d0=work/153
	MOVE.L	D0,D1
	LSL.L	#2,D1
	ADD.L	D0,D1		; d1=5*(work/153)
	ADD.L	D1,D3		; d2==work; d3==month

	MOVE.L	#306,D0
	MOVE.L	D3,D1
	mulu.l	d1,d1:d0	; d0=month*306
	ADDQ.L	#5,D0		; d0=month*306+5
	MOVEQ	#10,D1
	divul.l d1,d1:d0	; d0=(month*306+5)/10
	ADDQ.L	#1,D2
	SUB.L	D0,D2		; d2=day=1+work-(month*306+5)/10
	MOVE.W	D2,mday(A0)
	MOVE.L	D3,D0
	ADDQ.L	#2,D0
	MOVEQ	#12,D1
	divul.l d1,d1:d0	; d0=(month+2)/12
	ADDQ.L	#1,D1		; d1=(month+2)%12 +1
	MOVE.W	D1,month(A0)
	ADD.L	(SP)+,D0        ; year+=(month+2)/12
	MOVE.W	D0,year(A0)
	MOVEM.L (A7)+,D2-D3
	RTS


******* utility.library/Date2Amiga() *********************************
*
*   NAME
*	Date2Amiga -- Calculate seconds from 01-Jan-1978.  (V36)
*
*   SYNOPSIS
*	AmigaTime = Date2Amiga( Date )
*	D0			A0
*
*	ULONG Date2Amiga( struct ClockData * );
*
*   FUNCTION
*	Calculates the number of seconds from 01-Jan-1978 to the date
*	    specified in the ClockData structure.
*
*   INPUTS
*	Date -- Pointer to a ClockData structure containing the date
*	    of interest.
*
*   RESULTS
*	AmigaTime -- The number of seconds from 01-Jan-1978 to the date
*	    specified in Date.
*
*   NOTES
*	This function does no sanity checking of the data in Date.
*
*   SEE ALSO
*	Amiga2Date(), CheckDate()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	ULONG __asm Date2Amiga(register __a0 struct ClockData *cd)
;	  {
;	   return (((CalcDate(cd->year,cd->month,cd->day) - CALC_DATE(1978,1,1))
;		 * 24 + cd->hour) * 60 + cd->min) * 60 + cd->sec;
;	  }

Date2Amiga:
	MOVEM.L D2-D3,-(A7)
	MOVEQ	#00,D0
	MOVE.W	year(A0),D0
	MOVEQ	#00,D1
	MOVE.W	month(A0),D1
	MOVEQ	#00,D2
	MOVE.W	mday(A0),D2
	BSR	CalcDate
	MOVEQ	#00,D1
	MOVE.W	sec(A0),D1
	MOVEQ	#00,D2
	MOVE.W	min(A0),D2
	MOVEQ	#00,D3
	MOVE.W	hour(A0),D3
	SUBI.L	#722390,D0
	MOVE.L	D1,-(A7)
	MOVEQ	#24,D1
	mulu.l	d1,d0
	ADD.L	D3,D0
	MOVEQ	#60,D1
	mulu.l	d1,d0
	ADD.L	D2,D0
	MOVEQ	#60,D1
	mulu.l	d1,d0
	MOVE.L	(A7)+,D1
	ADD.L	D1,D0
	MOVEM.L (A7)+,D2-D3
	RTS


******* utility.library/CheckDate() **********************************
*
*   NAME
*	CheckDate -- Checks if a ClockData struct contains a leagal date.
*
*   SYNOPSIS
*	AmigaTime = CheckDate( Date )
*	D0		       a0
*
*	ULONG CheckDate( struct ClockData * );
*
*   FUNCTION
*	Determines if the Date is a legal date and returns the number
*	    of seconds to Date from 01-Jan-1978 if it is.
*
*   INPUTS
*	Date -- pointer to a ClockData structure.
*
*   RESULTS
*	AmigaTime -- 0 if Date invalid; otherwise, the number of seconds
*	    to Date from 01-Jan-1978.
*
*   NOTES
*
*   SEE ALSO
*	Amiga2Date(), Date2Amiga()
*
*   BUGS
*	The wday field of the ClockData structure is not checked.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

CheckDate:
	lea	-CD_SIZE(sp),sp
	move.l	a0,-(sp)        ; save it
	bsr	Date2Amiga	; get AmigaTime
	move.l	(sp)+,a1        ; ClockData
	move.l	sp,a0		; temp ClockData
	movem.l d0/a1,-(sp)
	bsr	Amiga2Date	; convert back

CDCommon:
	movem.l (sp)+,d0/a1
	move.l	sp,a0

; a1 -> ClockData struct passed in
; a0 -> ClockData struct created by converting the passed CD to AmigaTime
;	then converting back to CD
; d0 AmigaTime from passed ClockData struct

; if they are both the same, then the date/time is good

	move.l	#CD_SIZE-3,d1	; check the date (but not wday field)
CDls
	cmpm.b	(a0)+,(a1)+
	bne.s	BadDate 	; BZZZZZZZZT, you loose!
	dbf	d1,CDls
CDe
	lea	CD_SIZE(sp),sp
	rts

BadDate
	moveq.l #0,d0
	bra.s	CDe


w	 END
