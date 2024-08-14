*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: resource.asm,v 39.1 92/01/20 13:20:02 mks Exp $
*
* $Log:	resource.asm,v $
* Revision 39.1  92/01/20  13:20:02  mks
* Moved the priority to above graphics
* 
* Revision 39.0  92/01/14  13:42:53  mks
* Initial work on taking BattMem native for V39
*
*************************************************************************

	SECTION	BattMem

	INCLUDE	'internal.i'

*------	Imported Functions ------------------------------------------

	XSYS	InitSemaphore
	XSYS	ObtainSemaphore
	XSYS	ReleaseSemaphore
	XSYS	MakeLibrary
	XSYS	AddResource
	XSYS	OpenResource
	XSYS	ReadBattClockMem
	XSYS	WriteBattClockMem

*--------------------------------------------------------------------

initDDescrip:		     	;STRUCTURE RT,0
	dc.w	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	dc.l	initDDescrip	; APTR  RT_MATCHTAG
	dc.l	BattMemEnd	; APTR  RT_ENDSKIP
	dc.b	RTF_COLDSTART	; UBYTE RT_FLAGS
	dc.b	VERSION		; UBYTE RT_VERSION
	dc.b	NT_RESOURCE	; UBYTE RT_TYPE
	dc.b	69		; BYTE  RT_PRI
	dc.l	BattMemName	; APTR  RT_NAME
	dc.l	VERSTRING	; APTR  RT_IDSTRING
	dc.l	InitCode	; APTR  RT_INIT
				; LABEL RT_SIZE



BattMemName:
		BATTMEMNAME

BattClockName:
		BATTCLOCKNAME

VERSTRING:
		VSTRING

VERNUM:	EQU	VERSION
REVNUM:	EQU	REVISION

	ds.w	0


*------ Functions Offsets -------------------------------------
V_DEF	MACRO
	dc.W	\1+(*-initFunc)
	ENDM

initFunc:
	dc.w	-1
	V_DEF	ObtainBattSemaphore
	V_DEF	ReleaseBattSemaphore
	V_DEF	ReadBattMem
	V_DEF	WriteBattMem
	dc.w	-1


*------ Initializaton Table -----------------------------------

initStruct:
	INITBYTE	LN_TYPE,NT_RESOURCE
	INITLONG	LN_NAME,BattMemName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERNUM
	INITWORD	LIB_REVISION,REVNUM
	INITLONG	LIB_IDSTRING,VERSTRING

	DC.L		0


*****i* battmem.resource/internal/InitCode ***************************
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
*	This routine initializes the battmem.resource from nothing.
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
*	A3 -- battclock.resource pointer
*	A2 -- battmem.resource pointer
*

InitCode:
	movem.l	d2/a2/a3/a6,-(sp)
	move.l	#0,a2		; for no resource case

	lea	BattClockName,a1	; find the the clock
	CALLSYS	OpenResource
	tst.l	d0
	beq.s	InitEnd
	move.l	d0,a3		; battclockLib base

	lea	initFunc,a0	; point to correct set of offsets
	lea	initStruct,a1	; initialize the library node
	suba.l	a2,a2
	move.l	#BTM_SIZE,d0
	CALLSYS	MakeLibrary	; make us a playpen
	move.l	d0,a2		; save the base
	tst.l	d0		; was there a hitch?
	beq.s	InitEnd		; damn

	move.l	#0,BTM_Magic(a2)	; for BattMem
	move.l	a6,BTM_Exec(a2)		; set our data
	move.l	a3,BTM_BattClock(a2)
	lea	BTM_Semaphore(a2),a0	; to prevent conflicts
	CALLSYS	InitSemaphore
	move.l	a2,a1
	CALLSYS	AddResource	; yea! we made it
	move.l	a2,a6		; a6 now has BattClockBase

	moveq.l	#0,d1
	moveq.l	#1,d2
	bsr.s	ReadBattMem	; do a read so to set checksum

InitEnd:
	move.l	a2,d0		; battclockbase or NULL
	movem.l	(sp)+,d2/a2/a3/a6
	rts


******* battmem.resource/ObtainBattSemaphore *************************
*
*   NAME
*	ObtainBattSemaphore -- Obtain access to nonvolatile ram. (V36)
*
*   SYNOPSIS
*	ObtainBattSemaphore( )
*
*	void ObtainBattSemaphore( void );
*
*   FUNCTION
*	Aquires exclusive access to the system nonvolatile ram.
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

ObtainBattSemaphore:
	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	rts


******* battmem.resource/ReleaseBattSemaphore ************************
*
*   NAME
*	ReleaseBattSemaphore -- Allow nonvolatile ram to others.  (V36)
*
*   SYNOPSIS
*	ReleaseBattSemaphore( )
*
*	void ReleaseBattSemaphore( void );
*
*   FUNCTION
*	Relinquish exclusive access to the system nonvolatile ram.
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

ReleaseBattSemaphore:
	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ReleaseSemaphore
	rts


******* battmem.resource/ReadBattMem *********************************
*
*   NAME
*	ReadBattMem -- Read a bitstring from nonvolatile ram.  (V36)
*
*   SYNOPSIS
*	Error = ReadBattMem( Buffer, Offset, Len )
*	D0                   A0      D0      D1
*
*	ULONG ReadBattMem( APTR, ULONG, ULONG );
*
*   FUNCTION
*	Read a bitstring from nonvolatile ram.
*
*   INPUTS
*	Buffer          Where to put the bitstring.
*	Offset          Bit offset of first bit to read.
*	Len             Length of bitstring to read.
*
*   RESULTS
*	Error           Zero if no error.
*
*   NOTES
*	The battery-backed memory is checksummed. If a checksum error
*	is detected, all bits in the battery-backed memory are
*	silently set to zero.
*
*	Bits in the battery-backed memory that do not exist are read
*	as zero.
*
*	Partial byte reads (less than 8 bits) result in the bits read
*	being put in the low-order bits of the destination byte.
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

ReadBattMem:
	movem.l	d0-d1/a0,-(sp)
	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	movem.l	(sp)+,d0-d1/a0

	movem.l	d2-d5/a2,-(sp)
	move.l	a0,a2			; buffer
	move.l	d0,d5			; offset
	move.l	d1,d3
	move.l	d1,d4
	andi.l	#7,d4			; len%8
	lsr.l	#3,d3			; len/8 (byte count)
	beq.s	RBMremainder
	moveq.l	#8,d2
RBMloop
	move.l	d5,d1			; offset
	LINKLIB	_LVOReadBattClockMem,BTM_BattClock(a6)
	move.b	d0,(a2)+
	addq.l	#8,d5			; offset
	subq.l	#1,d3			; len (byte)
	bne.s	RBMloop
RBMremainder
	move.l	d4,d2			; len (bit)
	beq.s	RBMdone
	move.l	d5,d1			; offset
	LINKLIB	_LVOReadBattClockMem,BTM_BattClock(a6)
	move.b	d0,(a2)+
RBMdone
	movem.l	(sp)+,d2-d5/a2

	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ReleaseSemaphore
	moveq.l	#0,d0
	rts


******* battmem.resource/WriteBattMem ********************************
*
*   NAME
*	WriteBattMem -- Write a bitstring to nonvolatile ram.  (V36)
*
*   SYNOPSIS
*	Error = WriteBattMem( Buffer, Offset, Len )
*	D0                    A0      D0      D1
*
*	ULONG WriteBattMem( APTR, ULONG, ULONG );
*
*   FUNCTION
*	Write a bitstring to the nonvolatile ram.
*
*   INPUTS
*	Buffer          Where to get the bitstring.
*	Offset          Bit offset of first bit to write.
*	Len             Length of bitstring to write.
*
*   RESULTS
*	Error           Zero if no error.
*
*   NOTES
*	The battery-backed memory is checksummed. If a checksum error
*	is detected, all bits in the battery-backed memory are
*	silently set to zero.
*
*	Partial byte writes (less than 8 bits) result in the bits
*	written being read from the low-order bits of the source byte.
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

WriteBattMem:
	movem.l	d0-d1/a0,-(sp)
	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ObtainSemaphore
	movem.l	(sp)+,d0-d1/a0

	movem.l	d2-d5/a2,-(sp)
	move.l	a0,a2			; buffer
	move.l	d0,d5			; offset
	move.l	d1,d3
	move.l	d1,d4
	andi.l	#7,d4			; len%8
	lsr.l	#3,d3			; len/8 (byte count)
	beq.s	WBMremainder
	moveq.l	#8,d2
WBMloop
	move.l	d5,d1			; offset
	move.b	(a2)+,d0
	LINKLIB	_LVOWriteBattClockMem,BTM_BattClock(a6)
	addq.l	#8,d5			; offset
	subq.l	#1,d3			; len (byte)
	bne.s	WBMloop
WBMremainder
	move.l	d4,d2			; len (bit)
	beq.s	WBMdone
	move.l	d5,d1			; offset
	move.b	(a2)+,d0
	LINKLIB	_LVOWriteBattClockMem,BTM_BattClock(a6)
WBMdone
	movem.l	(sp)+,d2-d5/a2

	lea	BTM_Semaphore(a6),a0	; grab the semaphore
	LINKSYS	ReleaseSemaphore
	moveq.l	#0,d0
	rts

BattMemEnd:	; For the EndSkip
	end
