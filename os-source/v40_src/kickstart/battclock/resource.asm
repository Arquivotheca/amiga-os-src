*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: resource.asm,v 39.3 92/04/20 10:24:54 mks Exp $
*
* $Log:	resource.asm,v $
* Revision 39.3  92/04/20  10:24:54  mks
* Changed to use TaggedOpenLibrary()
* 
* Revision 39.2  92/02/19  09:40:14  mks
* Added empty 12/24 hour LVOs so that CDTV software will not crash
*
* Revision 39.1  92/01/20  13:16:34  mks
* Changed the priority to be above graphics and to not try to set
* the system time.
*
*************************************************************************

	SECTION	BattClock

	INCLUDE	'internal.i'
	INCLUDE	'exec/ables.i'
	INCLUDE	'hardware/custom.i'
	INCLUDE	'internal/librarytags.i'

*------	Imported Functions ------------------------------------------

	XSYS	InitSemaphore
	XSYS	ObtainSemaphore
	XSYS	ReleaseSemaphore
	XSYS	OpenDevice
	XSYS	DoIO
	XSYS	CloseDevice
	XSYS	MakeLibrary
	XSYS	AddResource
	XSYS	TaggedOpenLibrary
	XSYS	CloseLibrary
	XSYS	Amiga2Date
	XSYS	CheckDate

*------	Exported ----------------------------------------------------

	INT_ABLES

	XREF	_custom

*--------------------------------------------------------------------

	moveq.l	#-1,d0
	rts

initDDescrip:		     	;STRUCTURE RT,0
	dc.w	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	dc.l	initDDescrip	; APTR  RT_MATCHTAG
	dc.l	BattClockEnd	; APTR  RT_ENDSKIP
	dc.b	RTF_COLDSTART	; UBYTE RT_FLAGS
	dc.b	VERSION		; UBYTE RT_VERSION
	dc.b	NT_RESOURCE	; UBYTE RT_TYPE
	dc.b	70		; BYTE  RT_PRI
	dc.l	BattClockName	; APTR  RT_NAME
	dc.l	VERSTRING	; APTR  RT_IDSTRING
	dc.l	InitCode	; APTR  RT_INIT
				; LABEL RT_SIZE



BattClockName:
		BATTCLOCKNAME

VERSTRING:	VSTRING

VERNUM:	EQU	VERSION
REVNUM:	EQU	REVISION

	ds.w	0


*------ Functions Offsets -------------------------------------

V_DEF	MACRO
	dc.W	\1\2+(*-initFunc\2)
	ENDM

initFuncOki:
	dc.w	-1
	V_DEF	ResetBattClock,Oki
	V_DEF	ReadBattClock,Oki
	V_DEF	WriteBattClock,Oki
	V_DEF	Zero,Oki
	V_DEF	Zero,Oki
	V_DEF	Zero,Oki	; Same LVO as the 12/24 hour mode in CDTV
	dc.w	-1

initFuncRicoh:
	dc.w	-1
	V_DEF	ResetBattClock,Ricoh
	V_DEF	ReadBattClock,Ricoh
	V_DEF	WriteBattClock,Ricoh
	V_DEF	ReadBattMem,Ricoh
	V_DEF	WriteBattMem,Ricoh
	V_DEF	Zero,Ricoh	; Same LVO as the 12/24 hour mode in CDTV
	dc.w	-1


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
	movem.l	a2/a3/a4/a6,-(sp)
	move.l	#0,a2		; for no resource case

	moveq.l	#OLTAG_UTILITY,d0
	CALLSYS	TaggedOpenLibrary
	tst.l	d0
	beq.s	InitEnd
	move.l	d0,a3		; utilityLib base

	move.l	#CLOCKBASE,a4	; now find the clock
	move.l	a4,a0
	bsr	GetClockType
	tst.l	d0		; is one there?
	beq.s	InitEnd		; no, no resource
	cmp.l	#OKI_CLOCK,d0	; is it an Oki?
	bne.s	NotOki		; no
; Oki
	lea	initFuncOki(pc),a0	; point to correct set of offsets
	bsr.s	MakeResource	; go make and add the resource
	move.l	a4,a0		; where was the clock?
	bsr	ReadOki		; read the time
	bra.s	InitCommon
NotOki
	cmp.l	#OKI_RESET,d0	; is it a trashed Oki?
	bne.s	NotOkiR		; no
; trashed Oki
	lea	initFuncOki(pc),a0	; point to correct set of offsets
	bsr.s	MakeResource	; go make and add the resource
	move.l	a4,a0		; where was the clock?
	bsr	ResetOki	; reset the clock chip
	moveq.l	#0,d0
	bra.s	InitCommon
NotOkiR
	cmp.l	#RICOH_CLOCK,d0	; is it a Ricoh?
	bne.s	NotRicoh	; no
; Ricoh
	lea	initFuncRicoh(pc),a0	; point to function offsets
	bsr.s	MakeResource	; go make add the resource
	move.l	a4,a0		; clock address
	bsr	ReadRicoh	; read the time
	bra.s	InitCommon
NotRicoh
	cmp.l	#RICOH_RESET,d0	; is it a trashed Ricoh?
	bne.s	InitEnd		; no
; trashed Ricoh
	lea	initFuncRicoh(pc),a0	; point to function offsets
	bsr.s	MakeResource	; go make add the resource
	move.l	a4,a0		; clock address
	bsr	ResetRicoh	; reset the clock chip
	moveq.l	#0,d0

InitCommon

InitEnd:
	move.l	a2,d0		; battclockbase or NULL
	movem.l	(sp)+,a2/a3/a4/a6
	rts


*****i* battclock.resource/internal/MakeResource *********************
*
*   NAME
*	MakeResource -- Create, initialize, and add battclock.resource
*	    to the system list.
*
*   SYNOPSIS
*	BTC = MakeResource(ClockBase, UtilBase, FuncTable)
*	A2                 A4         A3        A0
*
*	Struct BattClockResource * MakeResource( APTR, struct Library *, APTR );
*
*   FUNCTION
*	This routine creates and initializes the battclock.resource
*	    structure.
*
*   INPUTS
*	ClockBase -- Pointer to the base address of the clock chip.
*	UtilBase -- Pointer to utility.library.
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
*	A3 -- utility.library pointer
*	A2 -- battclock.resource pointer
*

MakeResource:
	lea	initStruct,a1	; initialize the library node
	suba.l	a2,a2
	move.l	#BTC_SIZE,d0
	CALLSYS	MakeLibrary	; make us a playpen
	move.l	d0,a2		; save the base
	tst.l	d0		; was there a hitch?
	beq.s	MakeLibFailed	; damn

	move.l	#MAGIC,BTC_Magic(a2)	; for BattMem
	move.l	a6,BTC_Exec(a2)		; set our data
	move.l	a3,BTC_UtilLib(a2)
	move.l	a4,BTC_ClockBase(a2)
	lea	BTC_Semaphore(a2),a0	; to prevent conflicts
	CALLSYS	InitSemaphore
	move.l	a2,a1
	CALLSYS	AddResource	; yea! we made it
	move.l	a2,a6		; a6 now has BattClockBase
	rts

MakeLibFailed:
	move.l	a3,a1		; be nice
	CALLSYS	CloseLibrary
	move.l	(sp)+,d0	; dump return address
	bra	InitEnd		; bye


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
ZeroOki:
ZeroRicoh:
	moveq.l	#0,d0
	rts


*****i* battclock.resource/internal/GetClockType *********************
*
*   NAME
*	GetClockType -- Determine what kind of clock chip is in the
*	    system.
*
*   SYNOPSIS
*	Type = GetCLockType( ClockBase )
*	D0                   A0
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
; first check if there are custom chip registers
; where the clock should be
	DISABLE	a1
	move.l	a0,-(sp)
	lea	_custom,a1
	move.w	intenar(a1),d1
	lea	intena(a1),a1
	lea	intenar(a0),a0
	move.w	(a0),d0
	cmp.w	d0,d1
	bne.s	NotChip
	tst.w	d1
	beq.s	NoInts
	move.w	d1,(a1)
	tst.w	(a0)
	bne.s	NotChip
NoInts
	move.w	#$aaaa,(a1)
	move.w	(a0),d0
	cmp.w	#$2aaa,d0
	bne.s	NotChip
	move.w	d0,(a1)
	tst.w	(a0)
	bne.s	NotChip
	move.w	#$9555,(a1)
	move.w	(a0),d0
	cmp.w	#$1555,d0
	bne.s	NotChip
	move.w	d0,(a1)
	tst.w	(a0)
	bne.s	NotChip
Chip
	moveq.l	#1,d0
	bra.s	FixChip
NotChip
	moveq.l	#0,d0
FixChip
	move.w	#$7fff,(a1)
	ori.w	#$8000,d1
	move.w	d1,(a1)
	move.l	(sp)+,a0
	ENABLE	a1
	tst.l	d0
	bne.s	NoClock
; it doesn't look like custom chip registers, so
; go looking for a clock we know about

	moveq.l	#$f,d1		; data mask

; on a non-trashed Oki, regf will be 4
; on a Ricoh, regf is always read as 0
	move.b	Regf(a0),d0
	and.b	d1,d0
	cmp.b	#4,d0
	beq.s	Oki

; its either a trashed Oki, a Ricoh or it isn't
	clr.b	Regf(a0)	; reset test modes
	clr.b	Rege(a0)
	move.b	#9,Regd(a0)	; try for Ricoh page 1

; on a Ricoh, page 1 regc will always read as 0
	move.b	#5,Regc(a0)
	move.b	Regc(a0),d0
	and.b	d1,d0
	beq.s	TestRicoh

; its either a trashed oki or not there
TestOki
	move.b	#4,Regd(a0)	; reset oki
	move.b	#7,Regf(a0)
	move.b	#4,Regf(a0)
	clr.b	Rege(a0)

	move.b	#1,Regf(a0)	; set 24hr mode
	move.b	#5,Regf(a0)
	move.b	#4,Regf(a0)

; if its an Oki, regf will read as 4
	move.b	Regf(a0),d0
	and.b	d1,d0
	cmp.b	#4,d0
	beq.s	TrashedOki
NoClock
	moveq.l	#NO_CLOCK,d0	; hope its a A500
	rts
TrashedOki
	moveq.l	#OKI_RESET,d0	; we found a trashed Oki
	rts
Oki
	moveq.l	#OKI_CLOCK,d0	; we found an Oki
	rts

; its either a Ricoh or it isn't there
; if its a Ricoh, regd will be 9
TestRicoh
	move.b	Regd(a0),d0
	and.b	d1,d0
	cmp.b	#9,d0
	bne.s	NoClock
; check the 24hr bit
	move.b	Rega(a0),d0
	btst	#0,d0
	bne.s	Ricoh
	moveq.l	#RICOH_RESET,d0
	rts
Ricoh
	move.b	#9,Regd(a0)
	moveq.l	#RICOH_CLOCK,d0
	rts


******* battclock.resource/ResetBattClock ****************************
*
*   NAME
*	ResetBattClock -- Reset the clock chip.  (V36)
*
*   SYNOPSIS
*	ResetBattClock( )
*
*	void ResetBattClock( void );
*
*   FUNCTION
*	This routine does whatever is neeeded to put the clock chip
*	into a working and usable state and also sets the date on the
*	clock chip to 01-Jan-1978.
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
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address

	bsr	ResetOki		; make it right

	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	rts


ResetBattClockRicoh:
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address

	bsr	ResetRicoh		; make it right

	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	rts


******* battclock.resource/ReadBattClock *****************************
*
*   NAME
*	ReadBattClock -- Read time from clock chip.  (V36)
*
*   SYNOPSIS
*	AmigaTime = ReadBattClock( )
*
*	ULONG ReadBattClock( void );
*	D0
*
*   FUNCTION
*	This routine reads the time from the clock chip and returns it
*	as the number of seconds from 01-jan-1978.
*
*   INPUTS
*
*   RESULTS
*	AmigaTime       The number of seconds from 01-Jan-1978 that
*	                    the clock chip thinks it is.
*
*   NOTES
*	If the clock chip returns an invalid date, the clock chip is
*	reset and 0 is returned.
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
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address

	bsr	ReadOki			; get the time

	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	move.l	(sp)+,d0
	rts


ReadBattClockRicoh:
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address

	bsr	ReadRicoh		; get the time

	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	move.l	(sp)+,d0
	rts


******* battclock.resource/WriteBattClock ****************************
*
*   NAME
*	WriteBattClock -- Set the time on the clock chip.  (V36)
*
*   SYNOPSIS
*	WriteBattClock( AmigaTime )
*	                D0
*
*	void WriteBattClock( ULONG );
*
*   FUNCTION
*	This routine writes the time given in AmigaTime to the clock
*	chip.
*
*   INPUTS
*	AmigaTime       The number of seconds from 01-Jan-1978 to the
*	                    time that should be written to the clock
*	                    chip.
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
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address
	move.l	(sp)+,d0

	bsr	WriteOki		; put the time

	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	rts


WriteBattClockRicoh:
	move.l	d0,-(sp)
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address
	move.l	(sp)+,d0

	bsr	WriteRicoh		; put the time

	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	rts


*****i* battclock.resource/internal/WriteRawClock ********************
*
*   NAME
*	WriteRawClock -- Write precomputed data to the clock chip.
*
*   SYNOPSIS
*	WriteRawClock( ClockBase, Data )
*	               A0         A1
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
	move.b	(a1)+,Reg0(a0)	;sec	;sec
	move.b	(a1)+,Reg1(a0)
	move.b	(a1)+,Reg2(a0)	;min	;min
	move.b	(a1)+,Reg3(a0)
	move.b	(a1)+,Reg4(a0)	;hour	;hour
	move.b	(a1)+,Reg5(a0)
	move.b	(a1)+,Reg6(a0)	;mday	;wday
	move.b	(a1)+,Reg7(a0)		;mday
	move.b	(a1)+,Reg8(a0)	;month
	move.b	(a1)+,Reg9(a0)		;month
	move.b	(a1)+,Rega(a0)	;year
	move.b	(a1)+,Regb(a0)		;year
	move.b	(a1)+,Regc(a0)	;wday
	rts


*****i* battclock.resource/internal/ReadRawClock *********************
*
*   NAME
*	ReadRawClock -- Read data from the clock chip.
*
*   SYNOPSIS
*	ReadRawClock( ClockBase, Data )
*	              A0         A1
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
	moveq.l	#$f,d0
	move.b	Reg0(a0),(a1)	;sec	;sec
	and.b	d0,(a1)+
	move.b	Reg1(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg2(a0),(a1)	;min	;min
	and.b	d0,(a1)+
	move.b	Reg3(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg4(a0),(a1)	;hour	;hour
	and.b	d0,(a1)+
	move.b	Reg5(a0),(a1)
	and.b	d0,(a1)+
	move.b	Reg6(a0),(a1)	;mday	;wday
	and.b	d0,(a1)+
	move.b	Reg7(a0),(a1)		;mday
	and.b	d0,(a1)+
	move.b	Reg8(a0),(a1)	;month
	and.b	d0,(a1)+
	move.b	Reg9(a0),(a1)		;month
	and.b	d0,(a1)+
	move.b	Rega(a0),(a1)	;year
	and.b	d0,(a1)+
	move.b	Regb(a0),(a1)		;year
	and.b	d0,(a1)+
	move.b	Regc(a0),(a1)	;wday
	and.b	d0,(a1)+
	rts


*****i* battclock.resource/internal/Bin2Bcd **************************
*
*   NAME
*	Bin2Bcd -- Convert a binary number to 2 BCD digits.
*
*   SYNOPSIS
*	Bin2Bcd( Num, Ten, Where )
*	         D0   D1   A2
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
*	D0             D1   A2
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
	moveq.l	#0,d0
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
*	            A0
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
	moveq.l	#1,d0		; for convenience
	move.l	#400,d1		; timeout
SOHls1
	move.b	d0,Regd(a0)	; set holdbit
	btst.b	d0,Regd(a0)	; still busy?
	beq.s	HaveOkiHold	; no, got it
	clr.b	Regd(a0)	; reset holdbit
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
	move.b	d0,Regd(a0)	; set holdbit
	btst.b	d0,Regd(a0)	; still busy?
	beq.s	HaveOkiHold	; no, got it
	clr.b	Regd(a0)	; reset holdbit
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
*	          D0         A0
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
	lea	-ClockTempSize-CD_SIZE(sp),sp	; get some temp areas
	move.l	sp,a2			; clocktemp struct
	move.l	a0,-(sp)		; save clockbase
	lea	ClockTempSize(a2),a0	; ClockData struct
	LINKLIB	_LVOAmiga2Date,BTC_UtilLib(a6)	; fill in ClockData from AmigaTime
	lea	ClockTempSize(a2),a1	; ClockData struct (same one)
	move.l	(sp)+,a0		; clockbase

	; convert ClockData to clocktemp (correctly formatted)
	moveq.l	#10,d1			; for convenience

	moveq.l	#0,d0
	move.w	sec(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	min(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	hour(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	mday(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	month(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	year(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+
	clr.w	d0
	swap.w	d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	moveq.l	#0,d0
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
	clr.b	Regd(a0)	; Release Oki hold

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
*	                     A0
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
	movem.l	d2/a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp	; get some temp
	move.l	sp,a1			; clocktemp struct
	lea	ClockTempSize(a1),a2	; ClockData struct

	bsr	SetOkiHold	; stop ticking
	bsr	ReadRawClock	; get the data
	exg.l	a2,a1		; ClockData to a1
	clr.b	Regd(a0)	; Release Oki hold

	; convert clocktemp to ClockData
	move.l	sp,a2
	moveq.l	#10,d1

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

	moveq.l	#0,d0
	move.b	(a2)+,d0
	move.w	d0,wday(a1)

	lea	ClockTempSize(sp),sp	; give back some temp
	move.l	a0,a2		; save clockbase
	move.l	a1,a0		; clockdata
	LINKLIB	_LVOCheckDate,BTC_UtilLib(a6)	; convert & check date
	move.l	a2,a0		; clockdata
	tst.l	d0		; good date?
	bne.s	ReadCommon	; yes

	bsr.s	ResetOki	; bad date
	moveq.l	#0,d0


ReadCommon:
	lea	CD_SIZE(sp),sp	; return temp
	movem.l	(sp)+,d2/a2
	rts


*****i* battclock.resource/internal/ResetOki *************************
*
*   NAME
*	ResetOki -- Reset the Oki clock chip.
*
*   SYNOPSIS
*	ResetOki( ClockBase )
*	          A0
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

	moveq.l	#0,d0
	bsr	WriteOki	; put the date
	rts


*****i* battclock.resource/internal/ResetRicoh ***********************
*
*   NAME
*	ResetRicoh -- Reset the Ricoh clock chip.
*
*   SYNOPSIS
*	ResetRicoh( ClockBase )
*	            A0
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

	moveq.l	#0,d0
	bsr.s	WriteRicoh	; put the date
	rts


*****i* battclock.resource/internal/WriteRicoh ***********************
*
*   NAME
*	WriteOki -- Set the time on an Ricoh clock chip.
*
*   SYNOPSIS
*	WriteRicoh( AmigaTime, ClockBase )
*	            D0         A0
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
	lea	-ClockTempSize-CD_SIZE(sp),sp	; need some temp
	move.l	sp,a2			; clockTemp
	move.l	a0,-(sp)		; save clockbase
	lea	ClockTempSize(a2),a0	; ClockData struct
	LINKLIB	_LVOAmiga2Date,BTC_UtilLib(a6)	; fill in ClockData struct
	lea	ClockTempSize(a2),a1	; ClockData struct (again)
	move.l	(sp)+,a0	; ClockBase

	; convert ClockData to ClockTemp (formatted correctly)
	moveq.l	#10,d1

	moveq.l	#0,d0
	move.w	sec(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	min(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	hour(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	wday(a1),d0
	divu.w	d1,d0
	swap.w	d0
	move.b	d0,(a2)+

	moveq.l	#0,d0
	move.w	mday(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
	move.w	month(a1),d0
	bsr	Bin2Bcd

	moveq.l	#0,d0
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
	clr.b	Regd(a0)	; set hold
	exg.l	a2,a1
	bsr	WriteRawClock	; write the data to the chip
	exg.l	a2,a1
	move.b	#1,Regd(a0)
	move.b	#1,Rega(a0)	; set 24hr mode
	move.b	1+year(a1),Regb(a0)	; set the leapyear counter
	move.b	#3,Regf(a0)	; clear intermediate counts
	clr.b	Regf(a0)
	move.b	#9,Regd(a0)	; release hold

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
*	                       A0
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
	movem.l	d2/a2,-(sp)
	lea	-ClockTempSize-CD_SIZE(sp),sp	; need temp
	move.l	sp,a1			; clocktemp struct
	lea	ClockTempSize(a1),a2	; ClockData struct

	clr.b	Regd(a0)	; set hold
	bsr	ReadRawClock	; get the raw clock data
	exg.l	a2,a1
	move.b	#9,Regd(a0)	; release hold

	move.l	sp,a2		; ClockTemp

	; convert ClockTemp into ClockData
	moveq.l	#10,d1

	bsr	Bcd2Bin
	move.w	d0,sec(a1)

	bsr	Bcd2Bin
	move.w	d0,min(a1)

	bsr	Bcd2Bin
	move.w	d0,hour(a1)

	moveq.l	#0,d0
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

	lea	ClockTempSize(sp),sp	; return some temp
	move.l	a0,a2		; clockbase
	move.l	a1,a0		; ClockData struct
	LINKLIB	_LVOCheckDate,BTC_UtilLib(a6)	; is the date good?
	move.l	a2,a0		; clockbase
	tst.l	d0		; good date?
	bne	ReadCommon	; yes

	bsr	ResetRicoh	; BZZZZZZZT, no reset the chip
	moveq.l	#0,d0
	bra	ReadCommon


*****i* battclock.resource/internal/ReadBattClockMem *****************
*
*   NAME
*	ReadBattCLockMem -- Read a bitstring from nonvolatile ram.
*
*   SYNOPSIS
*	Data = ReadBattClockMem( Offset, Len )
*	D0                       D1      D2
*
*	ULONG ReadBattCLockMem( ULONG, ULONG );
*
*   FUNCTION
*	Read a bitstring from nonvolatile ram.
*
*   INPUTS
*	Offset -- Bit offset of first bit to read.
*	Len -- Length of bitstring to read.
*
*   RESULTS
*	Data -- Bitstring read, right justified.
*
*   NOTES
*	This operation will "clip" the write to the memory available
*	    in the clock chip and return 0s for bits outside memory
*	    available.
*	Lengths greater than 32 are set to 32.
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

ReadBattMemRicoh:
	movem.l	d2-d4,-(sp)
	move.l	d1,d4
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address
	moveq.l	#0,d0
	move.l	d0,-(sp)
	lea	-RicohBuffSize(sp),sp
	move.l	sp,a1
	bsr	GetMemRicoh
	move.l	d4,d1
	moveq.l	#0,d0			; set the 4 bytes after the legal
	move.b	d0,RicohMemSize-1(a1)	;  memory to zero (makes my life
	move.b	d0,RicohMemSize+0(a1)	;  easier).
	move.b	d0,RicohMemSize+1(a1)
	move.b	d0,RicohMemSize+2(a1)

; at this point a1 -> Ricoh buffer (validated)
	tst.l	d2
	beq.s	RBMRexit
	cmp.l	#32,d2
	bls.s	RBMRLenOk1
	moveq.l	#32,d2
RBMRLenOk1
	moveq.l	#0,d0
	move.l	#RicohBitSize,d3	; left = 96
	sub.l	d1,d3		; left = 96 - offset
	bls.s	RBMRexit	; if (left <= 0) goto WBMRexit

; at this point offset and len are valid
	moveq.l	#-1,d3
	lsr.l	d2,d3
	not.l	d3		; data mask (left justified)

	move.l	d1,d4
	asr.l	#3,d1
	add.l	d1,a1
	and.l	#7,d4
	beq.s	RBMRNoRot1
	ror.l	d4,d3
RBMRNoRot1

	moveq.l	#3,d1
RBMRReadLoop
	rol.l	#8,d0
	move.b	(a1)+,d0
	dbf	d1,RBMRReadLoop

	and.l	d3,d0
	tst.l	d4
	beq.s	RBMRNoRot2
	rol.l	d4,d0
RBMRNoRot2

; at this point the read data is left justified
	rol.l	d2,d0		; data now right justified

RBMRexit
	move.l	d0,d3
	lea	RicohBuffSize(sp),sp
	move.l	(sp)+,d0
	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	move.l	d3,d0
	movem.l	(sp)+,d2-d4
	rts


*****i* battclock.resource/internal/WriteBattClockMem ****************
*
*   NAME
*	WriteBattCLockMem -- Write a bitstring to nonvolatile ram.
*
*   SYNOPSIS
*	WriteBattClockMem( Data, Offset, Len )
*	                   D0    D1      D2
*
*	void WriteBattCLockMem( ULONG, ULONG, ULONG );
*
*   FUNCTION
*	Write a bitstring to the nonvolatile ram.
*
*   INPUTS
*	Data -- Bitstring to write, right justified.
*	Offset -- Bit offset of first bit to write.
*	Len -- Length of bitstring to write.
*
*   RESULTS
*
*   NOTES
*	This operation will "clip" the write to the memory available
*	    in the clock chip.
*	Lengths greater than 32 are set to 32.
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

WriteBattMemRicoh:
	movem.l	d2-d4,-(sp)
	move.l	d0,d3
	move.l	d1,d4
	lea	BTC_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	move.l	BTC_ClockBase(a6),a0	; get the clock address
	move.l	d0,-(sp)		; safty buffer
	lea	-RicohBuffSize(sp),sp
	move.l	sp,a1
	bsr.s	GetMemRicoh
	move.l	d3,d0
	move.l	d4,d1

; at this point a1 -> Ricoh buffer (validated)
	move.l	a1,-(sp)

	cmp.l	#32,d2
	bls.s	WBMRLenOk1
	moveq.l	#32,d2
WBMRLenOk1
	move.l	#RicohBitSize,d3	; left = 96
	sub.l	d1,d3		; left = 96 - offset
	bls.s	WBMRexit	; if (left <= 0) goto WBMRexit
	move.l	d2,d4		; rot = len
	sub.l	d3,d4		; rot = len - left
	bls.s	WBMRLenOk2		; if (rot <= 0) goto LenOk
	move.l	d3,d2		; len = left
	asr.l	d4,d0		; d0 >>= rot
WBMRLenOk2
	tst.l	d2
	beq.s	WBMRexit	; ignore 0 length requests

; at this point d0, offset, len are all valid

	moveq.l	#-1,d3
	asl.l	d2,d3
	not.l	d3
	and.l	d3,d0
	not.l	d3		; d3 masks out the area to write

	ror.l	d2,d0		; left justify data
	ror.l	d2,d3		; left justify mask

	move.l	d1,d4
	asr.l	#3,d1
	add.l	d1,a1		; adj data ptr to 1st modified byte
	move.l	d4,d1
	and.l	#7,d1
	beq.s	WBMRNoRot
	ror.l	d1,d0		; adjust the data & mask
	ror.l	d1,d3
WBMRNoRot

; this loop will write to 4 bytes of memory, but will not modify
;  memory past the end of the buffer (if all goes as planed)
	moveq.l	#3,d1		; 4 times through loop
WBMRWriteLoop
	rol.l	#8,d0
	rol.l	#8,d3
	and.b	d3,(a1)
	or.b	d0,(a1)+
	dbf	d1,WBMRWriteLoop

; the buffer is ready, write it
	move.l	(sp)+,a1
	bsr	PutMemRicoh

WBMRexit
	lea	RicohBuffSize(sp),sp
	move.l	(sp)+,d0
	lea	BTC_Semaphore(a6),a0	; release the semaphore
	LINKSYS	ReleaseSemaphore
	movem.l	(sp)+,d2-d4
	rts





*****i* battclock.resource/internal/GetMemRicoh **********************
*
*   NAME
*	GetMemRicoh -- Get and verify the memory from the Ricoh chip.
*
*   SYNOPSIS
*	GetMemRicoh( ClockBase, Buffer )
*	             A0         A1
*
*	void GetMemRicoh( APTR, APTR );
*
*   FUNCTION
*	Reads the memory from the Ricoh clock chip into the buffer
*	    and checks the CRC of the data. If the CRCs conflict, then
*	    zero the buffer in memory and on the chip.
*
*   INPUTS
*	ClockBase -- Pointer to the base address of the Ricoh chip.
*	Butter -- Pointer to a buffer to hold the data from the chip.
*
*   RESULTS
*
*   NOTES
*	Registers A0 and A1 are preserved.
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

GetMemRicoh
	move.l	a1,-(sp)
	bsr.s	RawGetMemRicoh
	move.l	(sp)+,a1
	movem.l	a0/a1,-(sp)
	move.l	a1,a0
	moveq.l	#-1,d0
	moveq.l	#RicohMemSize-1,d1
	bsr	CalcCRC8
	movem.l	(sp)+,a0/a1
	cmp.b	RicohMemSize-1(a1),d0
	beq.s	GMRok

	moveq.l	#RicohMemSize-1,d0
	move.l	a1,d1
GMRcl
	clr.b	(a1)+
	dbf	d0,GMRcl
	move.l	d1,a1
	bsr.s	PutMemRicoh
GMRok
	rts


; a0 = ClockBase
; a1 = buffer    (t)

RawGetMemRicoh:
	movem.l	d2/a2,-(sp)
	moveq.l	#$f,d1
	moveq.l	#RicohMemSize-1,d2
	move.l	a0,a2
	addq.l	#3,a2
RGMRls
	move.b	#$a,Regd(a0)
	move.b	(a2),d0
	and.b	d1,d0
	asl.b	#4,d0
	move.b	d0,(a1)
	move.b	#$b,Regd(a0)
	move.b	(a2),d0
	and.b	d1,d0
	or.b	d0,(a1)+
	addq.l	#4,a2
	dbf	d2,RGMRls

	movem.l	(sp)+,d2/a2
	move.b	#9,Regd(a0)
	rts


*****i* battclock.resource/internal/PutMemRicoh **********************
*
*   NAME
*	PutMemRicoh -- Write buffer to the Ricoh chip memory.
*
*   SYNOPSIS
*	PutMemRicoh( ClockBase, Buffer )
*	             A0         A1
*
*	void PutMemRicoh( APTR, APTR );
*
*   FUNCTION
*	Writes the buffer to the memory on the Ricoh clock chip after
*	    calculating the new CRC.
*
*   INPUTS
*	ClockBase -- Pointer to the base address of the Ricoh chip.
*	Butter -- Pointer to the buffer with the data to write.
*
*   RESULTS
*
*   NOTES
*	Registers A0 and A1 are preserved.
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

PutMemRicoh
	movem.l	a0/a1,-(sp)
	moveq.l	#-1,d0
	move.l	a1,a0
	moveq.l	#RicohMemSize-1,d1
	bsr.s	CalcCRC8
	movem.l	(sp)+,a0/a1
	move.b	d0,RicohMemSize-1(a1)
	move.l	a1,-(sp)
	bsr.s	RawPutMemRicoh
	move.l	(sp)+,a1
	rts


; a0 = ClockBase
; a1 = buffer    (t)

RawPutMemRicoh:
	move.l	a2,-(sp)
	moveq.l	#RicohMemSize-1,d1
	move.l	a0,a2
	addq.l	#3,a2
RPMRls
	move.b	(a1)+,d0
	move.b	#$b,Regd(a0)
	move.b	d0,(a2)
	asr.b	#4,d0
	move.b	#$a,Regd(a0)
	move.b	d0,(a2)
	addq.l	#4,a2
	dbf	d1,RPMRls

	move.l	(sp)+,a2
	move.b	#9,Regd(a0)
	rts


*****i* battclock.resource/internal/CalcCRC8 *************************
*
*   NAME
*	CalcCRC8 -- Calculates an 8 bit CRC value.
*
*   SYNOPSIS
*	Crc = CalcCRC8( InitialCrc, Data, Len )
*	D0              D0          A0    D1
*
*	BYTE CalcCRC8( BYTE, APTR, ULONG );
*
*   FUNCTION
*	Calculates an 8 bit CRC value for an area of memory.
*
*   INPUTS
*	InitialCrc -- CRC value to start with.
*	Data -- Pointer to the data to have the CRC computed for.
*	Len -- Number of bytes of date to uses in computations.
*
*   RESULTS
*	Crc -- The CRC value computed from the InitialCrc and the data.
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

CalcCRC8:
	move.l	d2,-(sp)
	lea	CRCTable,a1
	move.l	d1,d2
	bra.s	CC8le
CC8ls
	move.b	(a0),d1
	bsr.s	DivNibble
	move.b	(a0)+,d1
	asl.b	#4,d1
	bsr.s	DivNibble
CC8le	dbf	d2,CC8ls

	move.l	(sp)+,d2
	rts


; Updates the CRC based on 1 nibble of data (hi nibble of lo byte)
; a1 -> CRC table
; d0 -> CRC
; d1 -> data (t)

DivNibble:
	and.b	#$f0,d1
	eor.b	d0,d1
	ror.b	#4,d1
	move.b	d1,d0
	and.b	#$f0,d0
	and.w	#$f,d1
	move.b	0(a1,d1.w),d1
	eor.b	d1,d0
	rts

; Polynomial => X^8 + X^6 + X^4 + X^2 + X + 1
CRCTable:
	dc.b	$00,$57,$ae,$f9
	dc.b	$0b,$5c,$a5,$f2
	dc.b	$16,$41,$b8,$7f
	dc.b	$1d,$4a,$b3,$e4


BattClockEnd:	; For the endskip
	end
