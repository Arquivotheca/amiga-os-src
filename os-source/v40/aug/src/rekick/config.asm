*******************************************************************************
*
* $Id: config.asm,v 39.2 92/06/08 14:58:19 mks Exp $
*
* $Log:	config.asm,v $
* Revision 39.2  92/06/08  14:58:19  mks
* Removed bogus autodoc entry
* 
* Revision 39.1  92/06/07  15:16:05  mks
* Initial release
*
*
*******************************************************************************
*
* ReKick - a really evil hack that is used to load a new kickstart into
* memory and reboot the system into it.  ReKick requires that memory
* be available at $200000 for at least 512K.  (Any less and it will not
* be able to load the kickstart)  It also requires that some memory be
* available as MEMF_CHIP (or MEMF_LOCAL, but that does not exist on
* pre-V37 systems...)  Also, due to the voodoo needed to get this to work,
* it must be run very early in the startup-sequence (first command?)
* and it will only work with kickstarts that were designed for ReKick.
*
*******************************************************************************
*
* Stripped from the pages of expansion/config.asm
*
*******************************************************************************
*
	NOLIST
	include	"exec/types.i"
	include	"exec/nodes.i"
	include	"exec/lists.i"
	include	"exec/libraries.i"
	include	"exec/ables.i"
	include	"exec/memory.i"
	include	"exec/execbase.i"
	include	"exec/macros.i"
	include "exec/interrupts.i"
	include "exec/semaphores.i"

	include	"libraries/configregs.i"
	include	"libraries/configvars.i"

LIBRARIES_EXPANSIONBASE_I	SET	1

TOTALSLOTS	EQU	256

  STRUCTURE	ExpansionInt,0
	UWORD	ei_IntMask	; mask for this list
	UWORD	ei_ArrayMax	; current max valid index
	UWORD	ei_ArraySize	; allocated size
	LABEL	ei_Array	; actual data is after this
	LABEL	ExpansionInt_SIZEOF

  STRUCTURE	BootNode,LN_SIZE
	UWORD	bn_Flags
	CPTR	bn_DeviceNode
	LABEL	BootNode_SIZEOF

**
** ALL PRIVATE EXCEPT FOR eb_Mountlist!!!
**
  STRUCTURE	ExpansionBase,LIB_SIZE
	UBYTE	eb_Flags
	UBYTE	eb_pad0		; longword align
	ULONG	eb_Obsolete1
	UWORD	eb_Z3_ConfigAddr
	UWORD	eb_pad1
	STRUCT	eb_CurrentBinding,CurrentBinding_SIZEOF
	STRUCT	eb_BoardList,LH_SIZE
	STRUCT	eb_MountList,LH_SIZE	; contains struct BootNode entries
	STRUCT	eb_AllocTable,TOTALSLOTS
	STRUCT	eb_BindSemaphore,SS_SIZE
	LABEL	ExpansionBase_SIZEOF


; error codes
EE_OK		EQU 0
EE_LASTBOARD	EQU 40	; could not shut him up
EE_NOEXPANSION	EQU 41	; not enough expansion mem; board shut up
EE_NOMEMORY	EQU 42	; not enough normal memory
EE_NOBOARD	EQU 43	; no board at that address
EE_BADMEM	EQU 44	; tried to add a bad memory card

; Flags
	BITDEF	EB,CLOGGED,0	; someone could not be shutup
	BITDEF	EB,SHORTMEM,1	; ran out of expansion mem
	BITDEF	EB,BADMEM,2	; tried to add a bad memory card
	BITDEF	EB,DOSFLAG,3	; reserved for use by AmigaDOS
	BITDEF	EB,KICKBACK33,4	; reserved for use by AmigaDOS
	BITDEF	EB,KICKBACK36,5	; reserved for use by AmigaDOS
	BITDEF	EB,SILENTSTART,6 ; See public file for def.

;-----------------internal information-----------------------------------------
BusErrorVector			EQU $08
AdddressErrorVector		EQU $0C
IllegalInstructionVector	EQU $10
PrivTrapVector			EQU $20
Line1010Vector			EQU $28
Line1111Vector			EQU $2C


EXPANSION_RAM_PRIORITY	EQU 0		;Zorro II expansion memory
KICKIT_RAM_PRIORITY	EQU 5		;Kickit leftover
Z3_RAM_PRIORITY		EQU 20		;Zorro II
LOW_RAM_PRIORITY	EQU 30		;Good motherboard stuff
HIGH_RAM_PRIORITY	EQU 40		;Better CPU stuff

;------------------------------------------------------------------------------

	LIST

*
*******************************************************************************
*
* Since this code does all sorts of magic including a RESET instruction,
* we need to make sure we are in memory that will not go away.  This
* turns out to be CHIP memory on all current systems.  (MEMF_LOCAL under
* V37 and up, but that can not be counted on...)
*
	SECTION	ReKick,CODE,CHIP
WHERE_TO_LOOK	EQU	E_EXPANSIONBASE

TempEBase:	ds.b	ExpansionBase_SIZEOF

**
* Stripped out code from Expansion to get this special magic working...
**
MagicConfig:	xdef	MagicConfig
		move.l	a6,-(sp)
		lea	TempEBase(pc),a6	; Simple/silly...
		;
		lea	eb_BoardList(a6),a0
		NEWLIST	a0

		;------ free the 8 meg space
		move.l	#$80,d1
		moveq.l	#$20,d0
		bsr	FreeExpansionMem

		;------ free the 1/2meg expansion space
		moveq.l	#$07,d1	;7 slots of 64K each
		move.l	#$e9,d0
		bsr	FreeExpansionMem

		;------ now find the devices
		lea.l	WHERE_TO_LOOK,A0
		bsr	ConfigChain

*
* Wow!, we are now configed and the $200000 memory should be available
* Now, place a pointer to our ExpansionBase in a special spot in
* that space such that the ROM can find it...
*
		move.l	a6,$280000-4	; Last longword of the kickstart...
		move.l	(sp)+,a6
*
		rts

* expansion.library/ConfigBoard ***********************
*
*   NAME
*	ConfigBoard - configure a board
*
*   SYNOPSIS
*	error = ConfigBoard( board, configDev )
*	D0                   A0     A1
*
*   FUNCTION
*	This routine configures an expansion board.  The board
*	will generally live at E_EXPANSIONBASE, but the base is
*	passed as a parameter to allow future compatibility.
*	The configDev parameter must be a valid configDev that
*	has already had ReadExpansionRom() called on it.
*
*	ConfigBoard will allocate expansion memory and place
*	the board at its new address.  It will update configDev
*	accordingly.  If there is not enough expansion memory
*	for this board then an error will be returned.
*
*   INPUTS
*	board - the current address that the expansion board is
*		responding.
*	configDev - an initialized ConfigDev structure, returned
*		by AllocConfigDev.
*
*   RESULTS
*	error - non-zero if there was a problem configuring this board
*		(Can return EE_OK or EE_NOEXPANSION)
*
*   SEE ALSO
*	FreeConfigDev
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	For V36, this function was split into two parts.  First the
*	system configures all boards (using chip memory).  Later when
*	multitasking is up, the system adds memory and calls autoboot
*	drivers.
*
*   REGISTER USAGE
*
*/

ConfigBoard:	movem.l	d2/a2/a3,-(sp)
		move.l	a0,a2	; base address for board
		move.l	a1,a3	; configdev structure


		;------ don't go on if we are clogged
		;------ (someone could not be shut up)
		;	:TODO: Don't clog ZIII because of ZII
		moveq	#EE_NOEXPANSION,d0
		btst	#EBB_CLOGGED,eb_Flags(a6)
		bne.s	ConfigBoard_End

		move.b	cd_Rom+er_Type(a3),d0
		bsr	SlotsNeeded
		move.w	d0,cd_SlotSize(a3)

		;[D0=EE_NOEXPANSION]
		;------ check type of board
		;	:TODO: Support Zorro III board at Zorro II address
		;	:TODO: Consider adding rejection of boards not of
		;	       type II or III.  1.3 had no such protection.
		moveq	#$FFFFFF00+ERT_TYPEMASK,d1
		and.b	cd_Rom+er_Type(a3),d1

		;------ get the mem mask field of the type byte
		move.b	cd_Rom+er_Type(a3),d0	;pass entire byte
		bsr	AllocBoardMem
		tst.l	d0
		bpl.s	ConfigBoard_RecordData



;--------------- there was not enough Zorro II memory for you
ConfigBoard_TryShutup:
ConfigBoard_NoExpansionMem:
		btst	#ERFB_NOSHUTUP,cd_Rom+er_Flags(a3)
		beq.s	ConfigBoard_CanShutup

		;------ this board can't be shut up: it will live at the
		;------ config address.  This code used to be confused.
		;------ only a 64K board can be left at the config address;
		;------ a larger board will overlap other boards!
		;
		bset	#EBB_CLOGGED,eb_Flags(a6)

		cmp.w	#1,cd_SlotSize(a3)
		bne.s	ConfigBoard_ReturnError

		move.l	a2,d0			;configuration address
		moveq	#E_SLOTSHIFT,D1
		lsr.l	d1,d0			;turn into a slot number
		bra.s	ConfigBoard_RecordData


		;------ not enough expansion mem so shut him up
ConfigBoard_CanShutup:
		;---!!! shouldn't shut up if bootable device? */
		bset	#EBB_SHORTMEM,eb_Flags(a6)
		moveq	#0,d1
		moveq	#ec_Shutup+ExpansionRom_SIZEOF,d0
		move.l	a2,a0
		bsr	WriteExpansionByte

ConfigBoard_ReturnError:
		or.b	#CDF_BADMEMORY!CDF_SHUTUP,cd_Flags(a3)
		moveq	#EE_NOEXPANSION,d0
		bra.s	ConfigBoard_End
;---------------

*******************************************************************************


		;------ enter the data into the configdev structures
ConfigBoard_RecordData:
		;------ there was enough mem
		move.l	d0,d2			;return from AllocBoardMem()
		move.w	d0,cd_SlotAddr(a3)	;return from AllocBoardMem()
		move.w	d0,cd_BoardAddr(a3)			;word-to-long
		move.w	cd_SlotSize(a3),cd_BoardSize(a3)	;word-to-long

		;------ put the board at its new address
		move.l	d2,d1
		moveq	#ec_BaseAddress+ExpansionRom_SIZEOF,d0
		move.l	a2,a0
		bsr	WriteExpansionByte

ConfigBoard_Ok:
		moveq	#EE_OK,d0
		bset	#CDB_CONFIGME,cd_Flags(a3)	;"needs driver"

ConfigBoard_End:
		movem.l	(sp)+,d2/a2/a3
		rts




* expansion.library/ConfigChain ***********************
*
*   NAME
*	ConfigChain - configure the whole damn system
*
*   SYNOPSIS
*	error = ConfigChain( baseAddr )
*	D0                   A0
*
*   FUNCTION
*	This is the big one!  This routine will take a base address
*	(generally E_EXPANSIONBASE) and configure all the devices that
*	live there.  This routine will call all the other routines
*	that might need to be called.  All boards that are found will
*	be linked into the configuration list.
*
*   INPUTS
*	baseAddr - the base address to start looking for boards.
*		Zorro II is a subset of Zorro III; if a Zorro III
*		address is passed in, this code searches for both.
*
*   RESULTS
*	error - non-zero if something went wrong.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	FreeConfigDev
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	a2 - configuration address (Traditionally $00E80000)
*	a3 - current configdev
*	a4 - Alternate configuration address (Traditionally $FF000000)
*
*/
		IFNE	EZ3_EXPANSIONBASE-$ff000000
		FAIL	!! Recode transparent translation value !!
		ENDC


ConfigChain:	movem.l	a2/a3/a4,-(sp)

		move.l	a0,a2		; board
		move.l	a0,a4
		cmpa.l	#EZ3_EXPANSIONBASE,a0
		bne.s	cc_ZorroII
		move.l	#E_EXPANSIONBASE,a4
cc_ZorroII:



ConfigChain_Loop:
		BTST	#EBB_CLOGGED,eb_Flags(a6)
		bne.s	ConfigChain_Exit

		bsr.s	AllocConfigDev
		move.l	d0,a3
		move.l	a3,a1
		move.l	a2,a0			;Zorro II/III
		bsr.s	ReadExpansionRom
		move.l	a2,a0			;Zorro II/III
		tst.l	d0
		beq.s	ConfigChain_GotBoard

		move.l	a3,a1
		move.l	a4,a0			;Zorro II
		bsr.s	ReadExpansionRom
		move.l	a4,a0			;Zorro II
		tst.l	d0
		bne.s	ConfigChain_FreeCD	;end of chain...


ConfigChain_GotBoard:
		move.l	a3,a1
		bsr	ConfigBoard
		move.l	a3,a0
		bsr	AddConfigDev
		BRA.S	ConfigChain_Loop

		;------ there is no board there
ConfigChain_FreeCD:
		move.l	a3,a0
		bsr.s	FreeConfigDev
ConfigChain_Exit:
		CLEAR	d0
ConfigChain_End:
		movem.l	(sp)+,a2/a3/a4
		rts

ConfigChain_NoMem:
		moveq	#EE_NOMEMORY,d0
		bra.s	ConfigChain_End
*
*******************************************************************************
*
* Stripped from the pages of expansion/utils.asm
*
*******************************************************************************
*
* These are the stripped down functions that will be needed
*
FreeExpansionMem:
		move.l	d0,a0
		add.l	d1,a0
FreeExpansionMem_DoIt:
		lea	eb_AllocTable(a6,d0.l),a0
		moveq	#1,d0
		bra.s	FreeExpansionMem_Entry
FreeExpansionMem_Loop:
		move.b	d0,(a0)+
FreeExpansionMem_Entry
		dbra	d1,FreeExpansionMem_Loop
FreeExpansionMem_End:
		rts
*
*******************************************************************************
*
AllocConfigDev:	moveq	#ConfigDev_SIZEOF,D0
		bsr	Allocate	; Get the memory...
		move.l	d0,a0
		move.l	#ConfigDev_SIZEOF-1,d1
clear_loop:	clr.b	(a0)+		; Slow, but who cares?
		dbra	d1,clear_loop
		rts
*
*******************************************************************************
*
FreeConfigDev:	move.l	a0,a1
		moveq	#ConfigDev_SIZEOF,D0
		bsr	Deallocate	; Free it
		rts
*
*******************************************************************************
*
ReadExpansionRom:
		movem.l	d2/a2/a3,-(sp)

		move.l	a0,a2		; board
		move.l	a1,a3		; config dev structure

		;------ room for second rom structure
		lea	-ExpansionRom_SIZEOF(sp),sp

		;------ read in the rom image
		lea	cd_Rom(a3),a1
		bsr.s	readrom

		;------ some sanity checks
		;move.b	cd_Rom+er_Flags(a3),d0	;Zorro III defines these
		;and.b	#$3f,d0			;Zorro III defines these
		;bne.s	ReadExpansionRom_Err	;Zorro III defines these

		tst.b	cd_Rom+er_Reserved03(a3)
		bne.s	ReadExpansionRom_Err

		move.w	cd_Rom+er_Manufacturer(a3),d0
		beq.s	ReadExpansionRom_Err
		cmp.w	#-1,d0
		beq.s	ReadExpansionRom_Err

		;------ make sure it reads the same
		moveq	#10,d2

ReadExpansionRom_CheckLoop:
		move.l	a2,a0
		move.l	sp,a1
		bsr.s	readrom

		lea	cd_Rom(a3),a0
		move.l	sp,a1
		moveq	#ExpansionRom_SIZEOF,d0
		bsr.s	bcmp
		tst.l	d0
		bne.s	ReadExpansionRom_Err

		dbra	d2,ReadExpansionRom_CheckLoop

		;------ we made it through the tests */
		CLEAR	d0

ReadExpansionRom_End:

		;------ clean up temp structure
		lea	ExpansionRom_SIZEOF(sp),sp

		movem.l	(sp)+,d2/a2/a3
		rts

ReadExpansionRom_Err:
		moveq	#EE_NOBOARD,d0
		bra.s	ReadExpansionRom_End


**************************

readrom:	; ( board:a0, ExpansionRom: a1 )
		movem.l	d2/a2/a3,-(sp)
		move.l	a0,a2
		move.l	a1,a3


		;------ read the first byte
		CLEAR	d0
		bsr.s	ReadExpansionByte
		move.b	d0,(a3)+

		;------ read all the other bytes (they are inverted)
		MOVEQ	#1,d2

readrom_loop:
		move.l	d2,d0
		move.l	a2,a0
		bsr.s	ReadExpansionByte
		not.b	d0
		move.b	d0,(a3)+

		addq.w	#1,d2
		cmp.w	#ExpansionRom_SIZEOF,d2
		bls.s	readrom_loop

		movem.l	(sp)+,d2/a2/a3
		rts


*************************

bcmp:	; ( a: a0, b: a1, len : d0 )

		;------ special case for length of zero
		tst.l	d0
		beq.s	bcmp_end

		;------ adjust loop count
		subq.l	#1,d0

bcmp_loop:

		cmpm.b	(a0)+,(a1)+
		dbne	d0,bcmp_loop
		beq.s	bcmp_success

		blo.s	bcmp_lo
		moveq	#1,d0
		bra.s	bcmp_end
bcmp_lo:
		moveq	#-1,d0
		bra.s	bcmp_end

bcmp_success:
		CLEAR	d0

bcmp_end:
		rts


************************************

bcopy_loop:
		move.b	(a0)+,(a1)+
bcopy:	; ( a: a0, b: a1, len : d0 )
		dbra	d0,bcopy_loop

		rts
*
*******************************************************************************
*
ReadExpansionByte:
		lsl.w	#2,d0		;times 4
		lea.l	0(a0,d0.w),a0
		move.b	$002(a0),d1	;Zorro II
reb_Common:	lsr.b	#4,d1
		CLEAR	d0
		move.b	(a0),d0
		and.b	#$f0,d0
		or.b	d1,d0
		rts
*
*******************************************************************************
*
SlotsNeeded:	moveq	#ERT_MEMMASK,d1
		and.l	d1,d0
		lsl.w	#1,d0
		move.b	boardMapTable(pc,d0.w),d0
		rts
*
*******************************************************************************
*
boardMapTable:	; size,offset
	DC.B	128,32
	DC.B	1,0
	DC.B	2,0
	DC.B	4,0
	DC.B	8,0
	DC.B	16,0
	DC.B	32,0
	DC.B	64,32

AllocBoardMem:
		and.w	#ERT_MEMMASK,d0

		lsl.w	#1,d0		;:OPTIMIZE:
		lea	boardMapTable(pc,d0.w),a0

		CLEAR	d0
		CLEAR	d1

		move.b	(a0)+,d0	; size
		move.b	(a0)+,d1	; offset

AllocExpansionMem:

		movem.l	d2/d3,-(sp)
		move.l	d0,d2
		move.l	d1,d3

		;------ first try from the higher expansion area
		cmp.w	#7,d2
		bhi.s	AllocExpansionMem_8meg

		move.l	#$f0,d1
		move.l	#$e8,d0
		bsr.s	AllocExpansionSlot

		;------ quit if that one succeeded
		tst.l	d0
		bpl.s	AllocExpansionMem_End

		;------ try to allocate from the big space
AllocExpansionMem_8meg:
		move.l	#TOTALSLOTS,d1
		move.l	d3,d0
		bsr.s	AllocExpansionSlot

AllocExpansionMem_End:
		movem.l	(sp)+,d2/d3
		rts
*
*******************************************************************************
*
AllocExpansionSlot:	; ( first:d0, last:d1, numslots:d2 )

		movem.l	a2/a3,-(sp)

		lea.l	eb_AllocTable(a6),a0
		lea.l	0(a0,d0.l),a2
		lea.l	0(a0,d1.l),a3
		bra.s	AllocExpansionSlot_MainLoop

AllocExpansionSlot_Increment:
		;------ add the requested size to the current pointer
		add.l	d2,a2

		;------ main loop: try until we run out of expansion space
AllocExpansionSlot_MainLoop:
		cmp.l	a2,a3
		bls.s	AllocExpansionSlot_Fail

		;------ see if we fit in this range
		move.l	a2,a1
		move.l	d2,d1
		subq.l	#1,d1

AllocExpansionSlot_CheckLoop:
		tst.b	(a1)+		; if zero then slot is allocated
		dbeq	d1,AllocExpansionSlot_CheckLoop
		beq.s	AllocExpansionSlot_Increment

		;------ success! mark the slots as taken
		CLEAR	d0
		move.l	d2,d1
		move.l	a2,a1
		bra.s	AllocExpansionSlot_TLoopEntry

AllocExpansionSlot_TakenLoop:
		move.b	d0,(a1)+
AllocExpansionSlot_TLoopEntry:
		dbra	d1,AllocExpansionSlot_TakenLoop

		;------ return the slot number
		sub.l	a0,a2
		move.l	a2,d0
		bra.s	AllocExpansionSlot_End


AllocExpansionSlot_Fail:
		moveq	#-1,d0
AllocExpansionSlot_End:
		movem.l	(sp)+,a2/a3
		rts
*
*******************************************************************************
*
WriteExpansionByte:
		lsl.w	#2,d0		;times 4
		lea.l	0(a0,d0.w),a0

		move.b	d1,d0		;D1=high bits
		lsl.b	#4,d0		;D0=low bits

		move.b	d0,$002(a0)
web_Common:	move.b	d1,(a0)		;write entire byte to addr
		CLEAR	d0
		rts
*
*******************************************************************************
*
AddConfigDev:
		move.l	a0,a1
		lea	eb_BoardList(a6),a0
*
Enqueue:	MOVE.B  LN_PRI(A1),D1
		MOVE.L  (A0),D0
en_next:	MOVE.L  D0,A0
		MOVE.L  (A0),D0             ; next node
		BEQ.S   en_end
		CMP.B   LN_PRI(A0),D1
		BLE.S   en_next		; if we cannot insert yet
en_end:		MOVE.L  LN_PRED(A0),D0
		MOVE.L  A1,LN_PRED(A0)
		MOVE.L  A0,(A1)
		MOVE.L  D0,LN_PRED(A1)
		MOVE.L  D0,A0
		MOVE.L  A1,(A0)
*
		rts
*
*******************************************************************************
*
*    REGISTER USAGE
*	A0 -- mem header
*	A1 -- mem being freed
*	A2 -- ptr to current mem chunk being looked at (one after A1 eventually)
*

Deallocate:
	lea     $0400,a0		; Hard coded memory block...
	TST.L   D0
	BEQ.S   de_rts

	MOVEM.L D3/A2,-(SP)


	;------ round the base of the block down.
	MOVE.L  A1,D1			* check validity of the
	MOVEQ   #~MEM_BLOCKMASK,D3
	AND.L   D3,D1			*   freed region address
	EXG     D1,A1			* and size up
	SUB.L   A1,D1
	ADD.L   D1,D0

	;------ round size up to even block address:
	ADDQ.L  #MEM_BLOCKMASK,D0	* bias
	AND.L   D3,D0			* round
	BEQ.S   de_exit

	LEA	MH_FIRST(A0),A2		* pred of first node (head)
	MOVE.L  (A2),D3			* test for null list
	BEQ.S   at_head



	;------ scan for position in free list:
de_chunkloop:
	CMP.L   D3,A1			* went past returned region?
	BLS.S   de_inlist		* add region to free list

	;------ advance to the next node
	MOVE.L  D3,A2
	MOVE.L  (A2),D3			* get next node address
	BNE.S   de_chunkloop		* not end of list
	BRA.S	de_head_check



de_inlist:
	BEQ.S	de_freetwice		* already in the list

de_head_check:
	;------ check for first node on the list
	MOVEQ  #MH_FIRST,D1
	ADD.L  A0,D1
	CMP.L  A2,D1
	BEQ.S  at_head

	;------ join pred?
	MOVE.L  MC_BYTES(A2),D3
	ADD.L   A2,D3
	CMP.L   A1,D3			* curr = pred.size + pred ?
	BEQ.S   pred_join

	;------ don't join with pred:
at_head:
	MOVE.L  (A2),(A1)		* pred.succ -> curr.succ
	MOVE.L  A1,(A2)			* curr -> pred.succ
	MOVE.L  D0,MC_BYTES(A1)		* size -> curr.size
	BRA.S   tail_check

	;------ join with pred:
pred_join:
	ADD.L   D0,MC_BYTES(A2)		* size + pred.size -> pred.size
	MOVE.L  A2,A1			* pred -> curr

tail_check:
	;------ at tail?
	TST.L   (A1)
	BEQ.S   de_done

	;------ join succ?
	MOVE.L  MC_BYTES(A1),D3
	ADD.L   A1,D3
	CMP.L   (A1),D3			* pred = curr.size + curr ?
	BNE.S   de_done

	;------ join with succ:
	MOVE.L  (A1),A2
	MOVE.L  (A2),(A1)		* curr.succ.succ -> curr.succ
	MOVE.L  MC_BYTES(A2),D3
	ADD.L   D3,MC_BYTES(A1)		* curr.size+succ.size->curr.size

de_done:
	ADD.L   D0,MH_FREE(A0)
de_exit:
	MOVEM.L (SP)+,D3/A2
de_rts:	RTS

de_freetwice:
	;------ we tried to free a piece of memory that was already
	;------ in the free list.
	MOVEM.L	(SP)+,D3/A2
	RTS
*
*****************************************************************************
*
Allocate:
*	    ------- quick check for space:
	    lea     $0400,a0		; Hard coded memory block...
	    CMP.L   MH_FREE(A0),D0
	    BHI.S   am_quickfail	* exit if not enough total space...
	    TST.L   D0
	    BEQ.S   am_rts		* idiot case; asking for zero bytes...


InternalAllocate:
	    MOVE.L  A2,-(SP)
*	    ------- round size up to even block address:
	    ADDQ.L  #MEM_BLOCKMASK,D0	* bias
	    AND.W   #~MEM_BLOCKMASK,D0	* and round

	    LEA	    MH_FIRST(A0),A2	* pred of first node (same as head)



*	    -------
*	    ------- Chunk loop is in two halves, alternating registers
*	    ------- (we must always have pointers to the current and
*	    -------- previous chunks.  This double-loop saves bookkeeping)
*	    -------
am_chunkloop:
	    MOVE.L  (A2),A1
	    MOVE.L  A1,D1		* test for zero
	    BEQ.S   am_none		* end of list
	    CMP.L   MC_BYTES(A1),D0
	    BLS.S   am_found		* A2=prev A1=current

	    MOVE.L  (A1),A2
	    MOVE.L  A2,D1		* test for zero
	    BEQ.S   am_none		* end of list
	    CMP.L   MC_BYTES(A2),D0
	    BHI.S   am_chunkloop	* A1=prev A2=current

*	    ------- exchange so both paths look the same
	    EXG.L   A1,A2		* cc's unaffected



*	    ------- A0=memheader A1=current chunk A2=previous chunk A3-split
am_found:   BEQ.S   no_split

*	    ------- split the region:
	    MOVE.L  A3,-(SP)
	     LEA.L  0(A1,D0.L),A3	* newly created chunk's address
	     MOVE.L A3,(A2)		* make previous chunk point to new
	     MOVE.L (A1),(A3)+		* new MC_NEXT  = current MC_NEXT
	     MOVE.L MC_BYTES(A1),D1
	     SUB.L  D0,D1		* remaining size
	     MOVE.L D1,(A3)		* new MC_BYTES = size of new chunk
	     SUB.L  D0,MH_FREE(A0)	* adjust memory header
	    MOVE.L  (SP)+,A3
	    MOVE.L  A1,D0		* set result
	    MOVE.L  (SP)+,A2
	    RTS				* exit.  cc's


*	    ------- exact fit.  just cut out this area.
no_split:   MOVE.L  (A1),(A2)		* make previous chunk point to next
	    SUB.L   D0,MH_FREE(A0)	* adjust memory header
	    MOVE.L  (SP)+,A2
	    MOVE.L  A1,D0		* set result
	    RTS				* exit.  cc's


am_none:    MOVE.L  (SP)+,A2
am_quickfail:
	    MOVEQ   #0,D0
am_rts:	    RTS				* exit.  cc's

*
*******************************************************************************
*
	END
