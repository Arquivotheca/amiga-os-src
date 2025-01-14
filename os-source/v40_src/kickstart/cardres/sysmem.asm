**
**	$Id: sysmem.asm,v 1.3 92/11/30 10:31:39 darren Exp $
**
**	Credit card resource (System ram related code)
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	INCLUDE	"carddata.i"
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"

	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/lists.i"

	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/ables.i"

	INCLUDE	"tuples.i"
	INCLUDE	"gayle.i"
	INCLUDE	"debug.i"


** Imports

	XREF	OwnCard			;resource.asm
	XREF	ReleaseCard
	XREF	BeginCardAccess
	XREF	CardAccessSpeed

	XREF	CopyTuple		;tuples.asm
	XREF	DeviceTuple
	XREF	IfAmigaXIP

	XREF	resname

** Exports

	XDEF	AddSysMem

** Equates

TUPLE_STORE	EQU	TP_Device_MINSIZEOF+8

TUPLE_MAX_SIZE	EQU	258		;max size of tuple body +1

	CNOP	0,4

*****i* card.resource/AddSysMem ****************************************
*
*   NAME
*	AddSysMem -- Try to add a credit-card as system memory
*   
*   SYNOPSIS
*	void AddSysMem( resource, execbase )
*			   a5		a6
*
*   FUNCTION
*	Examine credit-card, and attempt to add it as system memory
*	if the card meets these requirements:
*
*	o There is a card in the slot (machine will reboot if card is
*	  removed while being examined, or if its retained for use
*	  as system ram, but this is an early init ROM module, so a
*	  reboot is not all that painful).
*
*	o Its NOT an Amiga execute-in-place card.  This check is made
*	  because you could have a card of type SRAM/DRAM, but not
*	  formatted with execute-in-place code (at least during testing
*	  of your execute-in-place code.  In the real world I expect
*	  most execute-in-place software will exist on ROM, EPROM,
*	  or FLASH-ROM.
*
*	o Its NOT formatted (there is NO CISTPL_FORMAT tuple).  If it
*	  has a CISTPL_FORMAT tuple, assume its a disk - we don't want
*	  to destroy it by adding it as system ram (the user might well
*	  think they are booting off of it).
*
*	o There is a CISTPL_DEVICE tuple (which describes the size,
*	  speed, etc., of common memory).
*
*	o The Device ID type field in the CISTPL_DEVICE tuple describes
*	  a card of DTYPE_SRAM, or DTYPE_DRAM.
*
*	o The card is at least 250ns speed, or better.
*
*	o The card is writeable (not to be confused with write-enabled)
*	  Because of the complexities involved in determining how
*	  the write-protect switch should be interpreted, we do
*	  a writeable check.
*
*	o Alternatively the card may NOT have a CISTPL_DEVICE tuple,
*	  in which case we will make an attempt at sizing the card.
*	  This case assumes an SRAM, or DRAM card which has lost its
*	  CISTPL_DEVICE tuple (such as a dead battery), or may even
*	  completely lack the ability to store tuples (e.g., no
*	  battery, and no rom for attribute memory).  As an interesting
*	  side-effect its possible to use factory fresh SRAM cards
*	  as system memory (such as a Fujitsu, or Panasonic card which
*	  does not have any default CIS on the cards).
*
*	o The card must have at least 8K of free space (even thats
*	  pointless) else it won't be used as system memory.
*
*   RETURNS
*	None because the function will either release, or retain
*	ownership of the credit-card.
*
***********************************************************************

AddSysMem:	


	PRINTF	DBG_ENTRY,<'AddSysMem($%lx)'>,A2

		movem.l	d2-d3/a2-a3/a6,-(sp)

		move.l	a5,a6

		lea	crb_MyHandle(a6),a2
		sub.l	#TUPLE_STORE,sp
		movea.l	sp,a3

	; Attempt to own credit-card -- to early for SetPatch, so don't
	; bother going through the vectors.  Card is owned in RESET|IMMEDIATE
	; modes


		move.l	a2,a1
		bsr	OwnCard

		tst.l	d0
		BNE_S	exitaddsysmem

	PRINTF	DBG_FLOW,<'AddSysMem owns credit card'>

		move.l	a2,a1
		BSRSELF	BeginCardAccess		;turn light ON

	; determine if this is an Amiga execute-in-place card

		BSRSELF	IfAmigaXIP
		tst.l	d0
		BNE_S	freecard

	; determine if there is a CISTPL_FORMAT tuple.
	; if so, assume its a disk, not to be used as system ram


		move.l	a3,a0
		move.l	a2,a1
		moveq	#CISTPL_FORMAT,d1

		moveq	#00,d0			; no body

		bsr	CopyTuple
		tst.l	d0
		BNE_S	freecard

	PRINTF	DBG_FLOW,<'Card does not have CISTPL_FORMAT tuple'>

	; determine if there is a CISTPL_DEVICE tuple

		move.l	a3,a0
		move.l	a2,a1
		moveq	#CISTPL_DEVICE,d1
		moveq	#(TP_Device_MINSIZEOF-TUPLE_SIZEOF),d0
		bsr	CopyTuple

	; no CISTPL_DEVICE tuple?  Try to size as a memory card then

		tst.l	d0
		beq	sizecard

		move.l	a3,a0
		lea	crb_DeviceTData(a6),a1

		bsr	DeviceTuple
		tst.l	d0
		beq.s	freecard

	; is this type SRAM, or DRAM?

		lea	crb_DeviceTData(a6),a1

		cmp.b	#DTYPE_SRAM,dtd_DTtype(a1)
		BEQ_S	isSRAM

		cmp.b	#DTYPE_DRAM,dtd_DTtype(a1)
		BEQ_S	isDRAM

	PRINTF	DBG_FLOW,<'Card is neither SRAM, or DRAM'>

freecard:

	PRINTF	DBG_FLOW,<'releasing credit card'>

		movea.l	a2,a1
		moveq	#CARDF_REMOVEHANDLE,d0
		bsr	ReleaseCard
exitaddsysmem:
		add.l	#TUPLE_STORE,sp

		movem.l	(sp)+,d2-d3/a2-a3/a6

	PRINTF	DBG_EXIT,<'Exit AddSysMem()'>


		rts

	;--
	;  If we are here, it means we have a SRAM, or DRAM card - do we
	;  have one that is fast enough though?
	;--

isSRAM:
	PRINTF	DBG_FLOW,<'Card is SRAM'>
isDRAM:

		move.l	dtd_DTspeed(a1),d1
		BEQ_S	freecard		;undefined speed

		cmp.l	#250,d1
		BHI_S	freecard		;too slow

	PRINTF	DBG_FLOW,<'Card is 250ns or less'>

	; set memory access speed based on CIS

		move.l	d0,d2

		move.l	a2,a1			;handle
		move.l	d1,d0			;speed

		bsr	CardAccessSpeed

		move.l	d2,d0			;ignore return


	; Have tuples, first available mem location used will be
	; common memory + 512, skipping past what we hope is
	; the end of the tuple chain.  Also protect first 512
	; bytes of RAM cards from inadvertent write of bogus
	; DEVICE tuple
		
addtomem:

		move.l	#512,d3			;skip this much for tuples
		move.l	#COMMON_MEMORY+512,a0

	; don't bother if the card is too small

		cmp.l	#DSIZE_8K,d0
		bcs	freecard

		sub.l	d3,d0			;leaves amount unused

	; do not try to add more than 4 megs

		move.l	#COMMON_MEM_SIZE,d1
		cmp.l	d1,d0
		bcs.s	membounds

		move.l	d1,d0
		sub.l	d3,d0

membounds:
	; check if card is "writeable"

		move.l	#$AAAA5555,d2
		
	; is already this pattern?  If so, invert test pattern

		cmp.l	(a0),d2
		bne.s	notmypattern

		not.l	d2
notmypattern:
		move.l	d2,(a0)
		cmp.l	(a0),d2
		BNE_S	freecard
		
	PRINTF	DBG_FLOW,<'Card is writeable'>
	PRINTF	DBG_FLOW,<'Final calced size is $%lx'>,D0

		moveq	#MEMF_FAST!MEMF_PUBLIC,d1

		moveq	#-5,d2			;SLOWFAST_MEM

		lea	resname(pc),a1

	PRINTF	DBG_OSCALL,<'AddMemList($%lx,$%lx,$%lx,$%lx,$%lx)'>,d0,d1,d2,a0,a1

		move.l	crb_ExecLib(a6),a6
		JSRLIB	AddMemList

		BRA_S	exitaddsysmem

	;--
	;  If we are here, it means we couldn't find a CISTPL_DEVICE
	;  tuple, so try to size as memory card.
	;
	;  This code is here because I'm anticipating LOW COST DRAM
	;  cards which do not have any attribute memory, or DEVICE
	;  tuple.  Typically this would not be the case, and we would
	;  never need to use this code at all (its at best slow, and
	;  potentially prone to cause random problems if the card
	;  is read/write sensitive (such as an IO card without
	;  a CISTPL_DEVICE tuple - something that should never be).  Of
	;  interest this code makes it possible to use uninitialized
	;  SRAM's too, though its much more efficient to write a CIS
	;  before using such cards.
	;
	;  The memory test is Destructive, though note this
	;  memory is added too late to be used as ROM-TAG memory.
	;
	;  The memory test checks for memory wrap.
	;
	;  RETURN values are setup for AddMemList() above.
	;
	;	A0 == Pointer to base of memory to use
	;	D0 == Size of memory (rounded up to nearest long word)
	;
	;--


sizecard:
	PRINTF	DBG_FLOW,<'Try to size credit card'>

	IFNE	DISABLE_DCACHE

		moveq	#00,d0			;data cache off
		bsr	cacheset
		move.l	d0,-(sp)

	ENDC

		move.l	#$AAAA5555,d1		;pattern

		movea.l	#COMMON_MEMORY,a0
		movea.l	a0,a1			;start
		move.l	a1,d0
		add.l	#COMMON_MEM_SIZE,d0	;limit

	; when sizing, we have to check for memory wrap too!

		clr.l	(a1)			;begin

		tst.w	2(a1)			;read word
		bne.s	cardsized
		tst.l	(a1)			;same?
		bne.s	cardsized

dosizing:
		adda.w	#256,a0			;do page at a time

	; check for exceeding 4 MEG max space (assume previous
	; page verified as usable)

		cmp.l	a0,d0
		bls.s	cardsized

		cmp.l	(a0),d1			;is already pattern?
		bne.s	notpattern

		not.l	d1			;reverse pattern
notpattern:
		move.l	d1,(a0)			;write pattern

		tst.l	(a1)			;check for mem wrap
		bne.s	memwrap

		cmp.l	(a0),d1			;test for pattern match
		beq.s	dosizing


memwrap:
	PRINTF	DBG_HARDWARE,<'memend $%lx memread $%lx'>,(a1),(a0)

cardsized:
		move.l	a0,d0
		sub.l	a1,d0			;sized

	PRINTF	DBG_FLOW,<'Calced Memory Size = $%lx'>,D0

	IFNE	DISABLE_DCACHE

		move.l	d0,d1
		move.l	(sp)+,d0
		bsr.s	cacheset
		move.l	d1,d0
	ENDC

		bra	addtomem


	IFNE	DISABLE_DCACHE

cacheset:
		movem.l	d1-d2/a0-a1/a6,-(sp)
		move.l	#CACRF_EnableD,d2
		move.l	d2,d1
		move.l	crb_ExecLib(a6),a6

	PRINTF	DBG_OSCALL,<'CacheControl($%lx,$%lx)'>,D0,D1

		JSRLIB	CacheControl

	PRINTF	DBG_OSCALL,<'$%lx=CacheControl()'>,D0

		and.l	d2,d0

		movem.l	(sp)+,d1-d2/a0-a1/a6
		rts

	ENDC


		END

