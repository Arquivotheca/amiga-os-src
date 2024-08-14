**
**      $Filename potgo.asm $
**      $Release: 1.4 $
**      $Revision: 36.8 $
**      $Date: 91/02/12 11:28:01 $
**
**      potgo resource module
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
**	$Id: potgo.asm,v 36.8 91/02/12 11:28:01 darren Exp $

	SECTION	potgo,code

**	Included Files

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/ables.i"

	INCLUDE		"potgo_rev.i"


**	Local Macros

XLVO	MACRO
	XREF	_LVO\1
	ENDM

**	Imported

	XREF	_intena
	XREF	_potgo


 STRUCTURE	PotgoResource,LIB_SIZE
    APTR	pr_ExecBase		; SysBase
    UWORD	pr_Allocated		; the owned bits of the reg
    UWORD	pr_HiByte		; current register high byte
    LABEL	pr_SIZEOF

**********************************************************************

residentPR:
		dc.w	RTC_MATCHWORD
		dc.l	residentPR
		dc.l	prEndModule
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_RESOURCE
		dc.b	100
		dc.l	prName
		dc.l	prID
		dc.l	prInitTable

prName:
		dc.b	'potgo.resource',0

prID:
		VSTRING


prInitTable:
		dc.l	pr_SIZEOF
		dc.l	prFuncInit
		dc.l	prStructInit
		dc.l	prInit



FUNCDEF	MACRO
		dc.w    pr\1-prFuncInit
	ENDM

prFuncInit:
		dc.w	-1
	INCLUDE	"potgo_lib.i"
		dc.w	-1

prStructInit:
*	;------ Initialize the library
		INITBYTE    LN_TYPE,NT_RESOURCE
		INITLONG    LN_NAME,prName
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		dc.w	    0


;------ potgo.resource/Init ------------------------------------------
;
;
prInit:
		move.l	d0,a0
		move.l	a6,pr_ExecBase(a0)
		rts


******* potgo.resource/AllocPotBits **********************************
*
*   NAME
*	AllocPotBits -- Allocate bits in the potgo register.
*
*   SYNOPSIS
*	allocated = AllocPotBits(bits)
*	D0                       D0
*
*	UWORD AllocPotBits( UWORD );
*
*   FUNCTION
*	The AllocPotBits routine allocates bits in the hardware potgo
*	register that the application wishes to manipulate via
*	WritePotgo.  The request may be for more than one bit.  A
*	user trying to allocate bits may find that they are
*	unavailable because they are already allocated, or because
*	the start bit itself (bit 0) has been allocated, or if
*	requesting the start bit, because input bits have been
*	allocated.  A user can block itself from allocation: i.e. 
*	it should FreePotgoBits the bits it has and re-AllocPotBits if
*	it is trying to change an allocation involving the start bit.
*
*   INPUTS
*	bits - a description of the hardware bits that the application
*		wishes to manipulate, loosely based on the register
*		description itself:
*	    START (bit 0) - set if you wish to use start (i.e. start
*		    the proportional controller counters) with the
*		    input ports you allocate (below).  You must
*		    allocate all the DATxx ports you want to apply
*		    START to in this same call, with the OUTxx bit
*		    clear.
*	    DATLX (bit 8) - set if you wish to use the port associated
*		    with the left (0) controller, pin 5.
*	    OUTLX (bit 9) - set if you promise to use the LX port in
*		    output mode only.  The port is not set to output
*		    for you at this time -- this bit set indicates
*		    that you don't mind if STARTs are initiated at any
*		    time by others, since ports that are enabled for
*		    output are unaffected by START.
*	    DATLY (bit 10) - as DATLX but for the left (0) controller,
*		    pin 9.
*	    OUTLY (bit 11) - as OUTLX but for LY.
*	    DATRX (bit 12) - the right (1) controller, pin 5.
*	    OUTRX (bit 13) - OUT for RX.
*	    DATRY (bit 14) - the right (1) controller, pin 9.
*	    OUTRY (bit 15) - OUT for RY.
*
*   RESULTS
*	allocated - the START and DATxx bits of those requested that
*		were granted.  The OUTxx bits are don't cares.
*
**********************************************************************
prAllocPotBits:
		move.l	d2,-(a7)
		move.l	pr_ExecBase(a6),a0
		DISABLE	a0,NOFETCH
		move.w	pr_Allocated(a6),d2
		move.w	d2,d1
		and.w	#$5500,d1
		lsl.w	#1,d1
		or.w	d2,d1
		not.w	d1
		and.w	d1,d0	; eliminate allocated DATxx & START
		btst	#0,d0	; trying to allocate START?
		beq.s	startOK	;   no
		move.w	d2,d1	; find if there are any input ports
		lsr.w	#1,d1	;   already allocated...
		not.w	d1	;
		and.w	#$5500,d1
		and.w	d2,d1	; if zero then no inputs allocated
		beq.s	startOK	;
		bclr	#0,d0	; START allocation fails

startOK:
		btst	#0,d2	; was START already set?
		beq.s	inputOK	;   if not, input allocations are OK
		move.w	d0,d1
		lsr.w	#1,d1	; get output allocations
		and.w	#$5500,d0
		and.w	d1,d0	;   only output allocations are OK
		move.w	d0,d1	;
		lsl.w	#1,d1	;
		or.w	d1,d0	;   (save that they are outputs)
inputOK:
		and.l	#$ff01,d0
		or.w	d0,pr_Allocated(a6)
		ENABLE	a0,NOFETCH
		move.l	(a7)+,d2
		rts


******* potgo.resource/FreePotBits ***********************************
*
*   NAME
*	FreePotBits -- Free allocated bits in the potgo register.
*
*   SYNOPSIS
*	FreePotBits(allocated)
*	            D0
*
*	void FreePotBits( UWORD );
*
*   FUNCTION
*	The FreePotBits routine frees previously allocated bits in the
*	hardware potgo register that the application had allocated via
*	AllocPotBits and no longer wishes to use.  It accepts the
*	return value from AllocPotBits as its argument.
*
**********************************************************************
prFreePotBits:
		and.w	#$5501,d0
		move.w	d0,d1
		lsl.w	#1,d1
		and.w	#$aa00,d1
		or.w	d1,d0
		not.w	d0
		and.w	d0,pr_Allocated(a6)
		rts


******* potgo.resource/WritePotgo ************************************
*
*   NAME
*	WritePotgo -- Write to the hardware potgo register.
*
*   SYNOPSIS
*	WritePotgo(word, mask)
*	           D0    D1
*
*	void WritePotgo( UWORD, UWORD );
*
*   FUNCTION
*	The WritePotgo routine sets and clears bits in the hardware
*	potgo register.  Only those bits specified by the mask are
*	affected -- it is improper to set bits in the mask that you
*	have not successfully allocated.  The bits in the high byte
*	are saved to be maintained when other users write to the
*	potgo register.  The START bit is not saved, it is written
*	only explicitly as the result of a call to this routine with
*	the START bit set: other users will not restart it.
*
*   INPUTS
*	word - the data to write to the hardware potgo register and
*	    save for further use, except the START bit, which is
*	    not saved.
*	mask - those bits in word that are to be written.  Other
*	    bits may have been provided by previous calls to
*	    this routine, and default to zero.
*
**********************************************************************
prWritePotgo:
		and.w	d1,d0
		not.w	d1
		move.l	pr_ExecBase(a6),a0
		DISABLE	a0,NOFETCH
		and.w	d1,pr_HiByte(a6)
		or.w	pr_HiByte(a6),d0
		move.w	d0,_potgo
		clr.b	d0			; do not cache starts
		move.w	d0,pr_HiByte(a6)
		ENABLE	a0,NOFETCH
		rts

prEndModule:

	END
