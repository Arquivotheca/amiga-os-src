*************************************************************************
*                                                                       *
* Copyright (C) 1985-1991 Commodore Amiga Inc.  All rights reserved.    *
*                                                                       *
*************************************************************************

*************************************************************************
*
* $Id: config.asm,v 39.9 93/03/09 14:16:59 mks Exp $
*
* $Log:	config.asm,v $
* Revision 39.9  93/03/09  14:16:59  mks
* Added extra bus flush to make sure that memory tests work
* even on the different buffers on the A4000T
* 
* Revision 39.8  93/02/09  13:54:38  mks
* Fixed CPU and motherboard RAM test code to work
* with more that 128Meg and to work with mirroring
* and to work as documented (Bad Bryce!!!!)
*
* Revision 39.7  92/11/17  16:31:59  mks
* Added the code needed for A1200 32-bit CPU memory addon...
*
* Revision 39.6  92/06/07  12:23:05  mks
* Moved the FreeVec() back up...  (Where is should be...)
*
* Revision 39.5  92/06/06  20:56:59  mks
* Minor changes to the ReKick code... In hopes of fixing the problem
* with Graphics...
*
* Revision 39.4  92/06/06  18:38:43  mks
* Added the ReKick conditional assembly code and removed the KickIt code
*
* Revision 39.3  92/04/28  13:08:25  mks
* Fixed silly typo in the hardware remapping ram sizing code
*
* Revision 39.2  92/04/27  09:54:47  mks
* Added code to notice ROM remapping with hardware...
*
* Revision 39.1  92/03/12  11:22:55  mks
* Fixed branches that could be short to save ROM space
*
* Revision 39.0  91/10/28  16:25:28  mks
* First release of native build V39 expansion code
*
*************************************************************************

	XDEF	initRoutine
	XDEF	ConfigBoard
	XDEF	ConfigChain
	XDEF	DoDiagList
	XDEF	ExpansionReserved26


	NOLIST
	include	"exec/types.i"
	include	"exec/nodes.i"
	include	"exec/lists.i"
	include	"exec/libraries.i"
	include	"exec/ables.i"
	include	"exec/memory.i"
	include	"exec/execbase.i"

	include	"configregs.i"
	include	"configvars.i"
	include	"private_base.i"
	include	"internal/a3000_hardware.i"

	include "asmsupp.i"
	include "messages.i"

	include "romconstants.i" ;Start, size and type of Kickstart ROM
	LIST


	XLIB	FreeExpansionMem
	XLIB	AllocBoardMem
	XLIB	AllocExpansionMem
	XLIB	WriteExpansionByte
	XLIB	WriteExpansionWord
	XLIB	ReadExpansionRom
	XLIB	AllocConfigDev
	XLIB	AddConfigDev
	XLIB	FreeConfigDev
	XLIB	ConfigBoard
	XLIB	ConfigChain

	XSEM	InitSemaphore

	XLIB	Debug
	XLIB	AddMemList
	XLIB	AddHead
	XLIB	AllocMem
	XLIB	Supervisor
	XLIB	MakeLibrary
	XLIB	OldOpenLibrary
	XLIB	CloseLibrary
	XLIB	AddLibrary
	XLIB	FreeMem
	XLIB	FreeVec
	XLIB	CopyMem

	XREF	SlotsNeeded
	XREF	ExpansionName

	XREF	funcTable
	XREF	dataTable

	TASK_ABLES
	INT_ABLES


	IFNE	E_SLOTSHIFT-16
	FAIL	"Search for E_SLOTSHIFT to find ==16 assumptions"
	ENDC

	IFNE	A3000_ROMS
WHERE_TO_LOOK	EQU	EZ3_EXPANSIONBASE
	ENDC
	IFEQ	A3000_ROMS
WHERE_TO_LOOK	EQU	E_EXPANSIONBASE
	ENDC



;
; void initRoutine( base:d0, seglist:a0 )
;
initRoutine:	movem.l	d2/d3/a2/a3/a5/a6,-(sp)
		lea.l	funcTable(pc),a0
		lea.l	dataTable(pc),a1
		suba.l	a2,a2
		move.l	#ExpansionBase_SIZEOF,d0
		;[D1=trash]
		CALLSYS	MakeLibrary
		tst.l	d0
		bne.s	ei_OK
		movem.l	(sp)+,d2/d3/a2/a3/a5/a6
		rts


ei_OK:
		IFGE	INFOLEVEL-20
		move.l	d0,-(sp)
		INFOMSG	10,<'init: base 0x%lx'>
		addq.l	#4,sp
		ENDC



		move.l	d0,a1		;expansion library base
		move.l	d0,a5		;expansion library base
		CALLSYS	AddLibrary
		;[A5-expansion, A6-exec]


		;
		;	Init library base
		;
		move.w	#EZ3_CONFIGAREA/E_SLOTSIZE,eb_Z3_ConfigAddr(a5)
		;
		lea	eb_BindSemaphore(a5),a0
		CALLSEM	InitSemaphore
		lea	eb_BoardList(a5),a0
		NEWLIST	a0
		lea	eb_MountList(a5),a0
		NEWLIST	a0
		;[A5-expansion, A6-exec]

*******************************************************************************
*
* ReKick KLUDGE! - When ReKick'ed, first pass of the configuration already
* happened.  That is, the eb_BoardList has been generated already.  Plus,
* there is this magic code down in CHIP memory that needs to be FreeVec()ed.
* This is all done here...
*
	IFD	REKICK
*
		movem.l	a2-a4,-(sp)		; Save these...
		move.l	$00280000-4,a4		; Get old ExpansionBase
		lea	eb_BoardList(a4),a2	; Old list
		lea	eb_BoardList(a5),a3	; New List
new_SwapLists:	move.l	(a2),a0         	;LH_HEAD
		move.l	a0,(a3)         	;LH_HEAD
		move.l	a3,LN_PRED(a0)
		move.l	LH_TAILPRED(a2),a0
		move.l	a0,LH_TAILPRED(a3)
		addq.l	#4,a3
		move.l	a3,(a0)         	;LN_SUCC
*
* Now, copy the eb_AllocTable
*
		move.b	eb_Flags(a4),eb_Flags(a5)	; Copy flags...
		lea	eb_AllocTable(a4),a0	; Old table
		lea	eb_AllocTable(a5),a1	; New table
		move.l	#TOTALSLOTS-1,d0	; Number to copy...
rekick_Copy:	move.b	(a0)+,(a1)+		; Copy byte...
		dbra.s	d0,rekick_Copy		; Copy it up...
*
* Now, FreeVec() the ReKick code...
*
		move.l	$00280000-8,a1	; Get memory address
		CALLSYS	FreeVec		; Free that memory... ;^)
*
		movem.l	(sp)+,a2-a4		; Restore...
*
*******************************************************************************
*
* Standard AutoConfig/non-ReKick code...
*
	ELSE
		;
		;	Prepare to config Zorro III cards
		;	(Make Z3 configuration area read/writeable)
		;
		IFNE	A3000_SUPERKICKSTART
		move.l	#$ff008507,d0	;Cache-inhibited 16MB at $FF000000
		move.l	#$403F8107,d1	;Read/write, cached, 2GB at $40000000
		bsr	SetTT
		ENDC




		exg.l	a5,a6		;swap exec and expansion
		;[A5-exec, A6-expansion]
		;------ free the 8 meg space
		move.l	#$80,d1
		moveq.l	#$20,d0
		CALLSYS FreeExpansionMem

		;------ free the 1/2meg expansion space
		moveq.l	#$07,d1	;7 slots of 64K each
		move.l	#$e9,d0
		CALLSYS FreeExpansionMem

		;------ now find the devices
		lea.l	WHERE_TO_LOOK,A0
		CALLSYS	ConfigChain

		exg.l	a5,a6		;swap exec and expansion
		;[A5-expansion, A6-exec]
		;
		;	After configuring, translate the Zorro III memory,
		;	and set a tricky speed-up for fast memory.
		;
		IFNE	A3000_SUPERKICKSTART
		move.l	#$403F8107,d0	;Read/write, cached, 2GB at $40000000
		move.l	#$04038207,d1	;Read-only, cached, 64MB at $04000000
		bsr	SetTT
		ENDC
*
	ENDC
*
*******************************************************************************

*******************************************************************************
*
* If ReKick hack, skip this part...
*
	IFND	REKICK
*
*******************************************************************************
*
*	After configuring memory, but before adding it to the memlists,
*	search for the old ExecBase.  We'll probably find it in one of
*	the expansion cards.
*
*	The previous contents of location 4 are stashed in ChkBase(a6)
*
*	If we find an ExecBase, call any ColdCapture, then copy the KickMem
*	and Capture pointers.
*
*******************************************************************************

		move.l	ChkBase(a6),d1	;Former contents of location 4
		IFGE	INFOLEVEL-10
		  move.l  d1,-(sp)
		  INFOMSG 10,<'Checking for ExecBase at $%lx'>
		  move.l  (sp)+,d1	; test CC's
		ENDC
		beq.s	ex_badbase	* nothing to check for...  .s
		move.l	d1,a1 		; (already validated as not odd)
		add.l	ChkBase(a1),d1
		not.l	d1
		bne.s	ex_badbase	* magic cookie tastes bad...

		;------ verify critical checksum:
		;[D1 - NULL]
		lea	SoftVer(a1),a0
		moveq	#(ChkSum-SoftVer)/2,d0
ex_chkchk:	add.w	(a0)+,d1
		dbra	d0,ex_chkchk
		not.w	d1
		bne.s	ex_badbase

		;------ process to cold capture vector
		move.l	ColdCapture(a1),d0
		IFGE	INFOLEVEL-4
		  move.l  d0,-(sp)
		  INFOMSG 4,<'Old ExecBase! ColdCapture %lx'>
		  move.l  (sp)+,d0	; test CC's
		ENDC
		beq.s	ex_nocapture

		movem.l	a1/a5/a6,-(sp)
		move.l	d0,a0			;function pointer
		move.l	a1,a6			;give 'em old ExecBase
		lea	ex_returncap(pc),a5
		clr.l	ColdCapture(a6)		; prevent infinite reboot loop
		jmp	(a0)
ex_returncap:   INFOMSG 4,<'ColdCapture returned'>
		movem.l	(sp)+,a1/a5/a6

ex_nocapture:	movem.l	KickMemPtr(a1),d0/d1/d2
		movem.l	d0/d1/d2,KickMemPtr(a6)
		movem.l	ColdCapture(a1),d0/d1/d2
		movem.l	d0/d1/d2,ColdCapture(a6)
*
* Copy over alert timout...
*
		move.l	LastAlert+3*4(a1),LastAlert+3*4(a6)
ex_badbase:
*
	ENDC
*
*******************************************************************************



		IFNE	A3000_ROMS
		;[A5-expansion, A6-exec]
*******************************************************************************
*
*	Size and add A3000 motherboard & 32 bit expansion space memory.
*
*	The A3000's bus is somewhat "sticky"; the last value written may
*	appear back in a read.  To clear the bus a random read of location
*	$20 is done before checking readback.
*
*	ROM space optimization is possible; code written under pressure.
*
*******************************************************************************
;
;	Size A3000 memory.  We must account for:
;		1> Bus timeout mode.
;		2> 512K or so of write-protected Kickstart image in the middle.
;		3> 512K or so of MAPROM Kickstart image in the middle...
;		4> non-memory reading back the last thing written.
;
;	There are 32*16MB blocks of expansion memory space (512MB).  We size
;	in 1MB increments.
;
;	   ...
;	$10000000____________________________________  (or higher)
;			32-Bit Memory expansion space
;			(CPU/"local" slot)
;	$08000000____________________________________
;		 -------------kickstart--------------
;	$07800000------------------------------------
;			A3000 motherboard memory
;				space
;	$01000000____________________________________
;	   ...
RAM_MIDDLE		EQU $08000000
RAM_MMU_SIZE		EQU 512*1024	;Size of SuperKickstart

RAM_STEPSIZE		EQU 1024*1024	;Test in 1MB chunks


;	Going down!
		move.l	#RAM_MIDDLE,a0			;A0=Working address
		move.l	a0,d0				;D0=Start
		move.l	#RAM_STEPSIZE,d2		;D2=Stepsize
		sub.l	a2,a2				;Fake mirror address
		move.l	(a2),d3				;Mirror value
tlm_MunchMore:	suba.l	d2,a0
		bsr	Test_A3K			;A0 preserved, CC's
		beq.s	tlm_MunchMore

;	Calculate Addmem Size
		adda.l	d2,a0			;A0=base (base=working+step)
		sub.l	a0,d0			;D0=size (size=start-base)
		beq.s	tlm_NoMem


;
; New magic for sizing the memory (just in case of Superkickstart or
; the new hardware remapping...)
;
		clr.l	-(sp)		; Clear the adjustment flag...

		IFNE	A3000_SUPERKICKSTART
;
;	Specialized Kludge.  Some section of our fast RAM might have been
;	write-protected by the SuperKickstart scheme.  The following code
;	skips over that area, if present.  A more general purpose solution
;	would have been nice...
;
;	Either takes no action, or subtracts RAM_MMU_SIZE from D0.
;
			subq.l	#4,sp
			btst.b	#AFB_68040,AttnFlags+1(a6)
			bne.s	mmu_Too_New

			dc.w	$f017,$4200	;pmove	TC,(sp)
			tst.b	(sp)		;Test E bit of TC (bit 7)
			beq.s	mmu_Disabled

			;Test User Program Space,(a0),7-levels
			dc.w	$f010,$9c12	;ptestw	#2,(a0),#7
			dc.w	$f017,$6200	;pmove	MMUSR,(sp)
			btst.b	#10-8,(sp)	;(I)nvalid Bit
			bne.s	mmu_NoTranslation
			btst.b	#11-8,1(sp)	;(W)rite Protected Bit
			bne.s	mmu_NotProtected
			move.l	#RAM_MMU_SIZE,4(sp)	; Set size adjustment
mmu_Too_New:
mmu_Disabled:
mmu_NoTranslation:
mmu_NotProtected:
			addq.l	#4,sp
			;44 bytes
		ENDC

;
; Now, check to see if we have to adjust for the hardware remapping...
;
		IFNE	A3000_SUPERKICKSTART	; Only needed for A3000
		tst.l	(sp)			; Already remapped?
		bne.s	map_Already		; If non-zero, already mapped
		ENDC
;
;
RAM_MAP_LOC		EQU $07F80000	; First location mapped
ROM_MAP_LOC		EQU $00F80000	; First ROM location mapped
;
		move.l	RAM_MAP_LOC,d2		; Get what is there...
		cmp.l	ROM_MAP_LOC,d2		; Is it the same?
		bne.s	map_Not			; If not, no mapping...
		move.l	d0,RAM_MAP_LOC		; Try changing it...
		cmp.l	ROM_MAP_LOC,d0		; Did it match?
		bne.s	map_Not2		; Second test did not work?
;
; We are mapped...
;
		move.l	#RAM_MMU_SIZE,(sp)	; Set it up...
;
map_Not2:	move.l	d2,RAM_MAP_LOC		; Restore RAM...
map_Not:
map_Already:	sub.l	(sp)+,d0		; Subtract mapping value...
		moveq	#LOW_RAM_PRIORITY,d2			;D2=pri
		bsr	Add_A3K
tlm_NoMem:
;	End of down!



;	Going up!
		move.l	#RAM_MIDDLE,a0			;A0=Working address
		move.l	a0,d0				;D0=Start
		move.l	#RAM_STEPSIZE,d2		;D2=Stepsize
		move.l	a0,a2				;Mirror address...
		move.l	(a2),d3				;Mirror value...
		bra.s	tum_Enter
tum_MunchMore:	add.l	d2,a0
tum_Enter	bsr	Test_A3K			;A0 preserved, CC's
		beq.s	tum_MunchMore

		exg	d0,a0			;A0=start D0=Working address
		sub.l	a0,d0			;D0=size (size=working-base)
		beq.s	tum_NoMem

		moveq	#HIGH_RAM_PRIORITY,d2			;D2=pri
		bsr	Add_A3K
tum_NoMem:
;	End of up!


;
;	Enable Bus Timeout.  Our choices are:
;		0 - DSACK after 9 uSecs (default)
;		1 - BERR  after 250 mSecs
;	Some hardware chokes on 9 uSec timeout (too fast).
;
		move.b	#$80,A3000_BusTimeoutMode	;Set BERR (250 mSecs)

		;[A5-expansion, A6-exec]
		ENDC		;A3000_ROMS
******************************************************************************


		IFD MACHINE_A1200
		;[A5-expansion, A6-exec]
******************************************************************************
*
* On the A1200, we check to see if the processor can address the 32-bit
* memory at the CPU slot addresses.  On the 24-bit address machines the
* addresses will map back down into CHIP RAM.  So, we need to check for
* the memory with mirroring test at location 0 for the first address.
* (We assume that if the first address worked that the card is designed
* correctly and we will continue to work at the other addresses...)
*
******************************************************************************

RAM_MIDDLE		EQU $08000000

RAM_STEPSIZE		EQU 1024*1024	;Test in 1MB chunks

		sub.l	a2,a2		; Point at location 0...
		move.l	#RAM_MIDDLE,a0	; Point at start of our magic address
		move.l	(a2),d2		; Get location 0...
		not.l	(a0)		; Invert CPU RAM
		move.l	(a2),d1		; Get new location 0...
		move.l	d2,(a2)		; Restore location 0 (maybe)
		cmp.l	d2,d1		; Check if it changed...
		bne.s	no_cpu_mem	; If it did we have 24-bit wrap...
*
* Now test the memory since there is some...
*
		move.l	a0,d0			; Store the start...
		move.l	a0,a2			; Get Start...
		move.l	(a2),d2			; Get start value...

cpu_mem_test:	move.l	4(a0),d3	; Store here for a bit...
		move.l	(a0),d1
		not.l	(a0)
		not.l	d1
		nop			;ensure serialization on 040
		cmp.l	a0,a2		; Check for mirror?
		beq.s	skip_mirror1	; If same address, skip test...
		move.l	d2,(a2)		; Restore first location...
skip_mirror1:	move.l	d3,4(a0)	; Flush bus...
		cmp.l	(a0),d1
		bne.s	cpu_mem_end
		not.l	d1
		not.l	(a0)
		nop			;ensure serialization on 040
		cmp.l	a0,a2		; Check for mirror?
		beq.s	skip_mirror2	; If same address, skip test...
		move.l	d2,(a2)		; Restore first location...
skip_mirror2:	move.l	d3,4(a0)	l Flush bus...
		cmp.l	(a0),d1
		bne.s	cpu_mem_end	;if no more...
*
		add.l	#RAM_STEPSIZE,a0	; Next block...
		bra.s	cpu_mem_test	; ...keep looking...
*
cpu_mem_end:	exg	d0,a0		; A0=start, d0=working address...
		sub.l	a0,d0		; d0=size
		beq.s	no_cpu_mem	; If size is 0, skip this...
*
								;A0=base
		lea.l	memname(pc),a1				;A1=name
								;D0=size
		move.l	#MEMF_PUBLIC!MEMF_FAST!MEMF_LOCAL,d1	;D1=flags
		moveq	#HIGH_RAM_PRIORITY,d2			;D2=pri
		CALLSYS	AddMemList	;size, attributes, pri, base, name
					; d0   d1          d2   a0    a1
no_cpu_mem:
		ENDC		; MACHINE_A1200
******************************************************************************

******************************************************************************
*
*	Merge & add memory to the system memory list.  The merge must
*	be done without touching the destination memory; this prevents
*	trashing any reboot recoverable memory and/or ram disks.
*
*		while (FindLowest())	/* Find lowest non-processed memory */
*			{
*			while (FindLowest())
*				{
*				if ==	extend current chunk
*				else	AddMemList	break
*				}
*			}
*
******************************************************************************
*		a2-Low end of board
*		a3-Top end of board

am_Top:		bsr.s	FindLowest		;Lowest unprocessed board
		move.l	a0,d1
		beq.s	am_End			;No more boards....
		move.l	d0,a2			;Base of board
		move.l	d0,a3			;
am_contig:	add.l	cd_BoardSize(a0),a3	;Running total of end
		bset.b	#CDB_PROCESSED,cd_Flags(a0)
		bsr.s	FindLowest
		cmp.l	a3,d0
		beq.s	am_contig	;Contiguous chunk...

		;
		;	Autoconfig memory is assumed to be DMAable.
		;	If the high byte is clear, it is MEMF_24BITDMA.
		;
		;	:TODO:Better 24bit dma definition.  1.3 compat
		;
		moveq	#MEMF_PUBLIC!MEMF_FAST,d1	; attributes
		moveq   #Z3_RAM_PRIORITY,d2		; pri
		move.l	a2,d0				; test address
		rol.l	#8,d0
		tst.b	d0
		bne.s	am_24bit
		bset.l	#MEMB_24BITDMA,d1		; attributes
		moveq   #EXPANSION_RAM_PRIORITY,d2	; pri
am_24bit:	move.l	a2,a0				; start of board
		move.l	a3,d0				; end of board
		sub.l	a2,d0				; end-start=length
		lea.l   memname(pc),a1			; name
		IFGE	INFOLEVEL-100
			move.l	d0,-(sp)
			move.l	a0,-(sp)
			INFOMSG	20,<'AddMemList: base $%lx length $%lx'>
			addq.l	#8,sp
		ENDC

*******************************************************************************
*
* ReKick KLUDGE - If memory is at $200000, we will change it to start at
* $280000 and drop the size by $80000
*
	IFD	REKICK
*
		cmp.l	#$200000,a0	; Are we it?
		bne.s	rekick_Skip	; If not, skip...
		add.l	#$080000,a0	; Add 512K to start
		sub.l	#$080000,d0	; Subtract 512K from size...
rekick_Skip:
*
	ENDC
*
*******************************************************************************

		CALLSYS	AddMemList	;size, attributes, pri, base, name
					; d0   d1          d2   a0    a1
		bra.s	am_Top

*******************************************************************************
am_End:
		;------ set up the return value
		move.l	a5,d0		;expansion library base

		movem.l	(sp)+,d2/d3/a2/a3/a5/a6
		rts



******************************************************************************
*
*	SYNOPSIS
*		FindLowest.  Search the boardlist for the lowest addressed,
*		good, non-processed, memory ConfigDev.
*
*	REGISTERS
*		a6-ExecBase
*		a5-ExpansionBase
*		a0-ConfigDev of the above board, NULL if none.
*		d0-lowest unprocessed board address
*
******************************************************************************

FindLowest:	lea.l	eb_BoardList(a5),a1
		suba.l	a0,a0
		moveq	#-1,d0
fl_Next:	TSTNODE	a1,a1
		beq.s	fl_EndSearch
			btst	#ERTB_MEMLIST,cd_Rom+er_Type(a1)
			beq.s	fl_Next		;Not memory card...
			move.b	cd_Flags(a1),d0
			and.b	#CDF_BADMEMORY!CDF_PROCESSED!CDF_SHUTUP,d0
			bne.s	fl_Next
			cmp.l	cd_BoardAddr(a1),d0
			bls.s	fl_Next
			move.l	cd_BoardAddr(a1),d0	;Set return length
			move.l	a1,a0			;Set return CD
			bra.s	fl_Next
fl_EndSearch:	rts




		IFNE	A3000_ROMS
******************************************************************************
*
*	Support for A3000 sizing functions
*
******************************************************************************
;
;	Test_A3K.  Test memory location in A0.  Return with CC's set.
;	A0 preserved.
;
Test_A3K:	move.l	4(a0),-(sp)	; Store old ...
		move.l	(a0),d1		; Get memory location...
		not.l	(a0)
		not.l	d1
		nop			;ensure serialization on 040
		cmp.l	a0,a2		; Check for mirror?
		beq.s	test_mirror1	; If same address, skip test...
		move.l	d3,(a2)		; Restore mirror...
test_mirror1:	move.l	(sp),4(a0)	; Cause bus to flush
		cmp.l	(a0),d1
		bne.s	test_End
		not.l	(a0)
		not.l	d1
		nop			;ensure serialization on 040
		cmp.l	a0,a2		; Check for mirror?
		beq.s	test_mirror2	; If same address, skip test...
		move.l	d3,(a2)		; Restore first location...
test_mirror2:	move.l	(sp),4(a0)	; Cause bus to flush
		cmp.l	(a0),d1
test_End:	addq.l	#4,sp		; Adjust stack
		rts



Add_A3K:							;A0=base
		lea.l	memname(pc),a1				;A1=name
								;D0=size
		move.l	#MEMF_PUBLIC!MEMF_FAST!MEMF_LOCAL,d1	;D1=flags
								;D2=pri
		JMPSYS	AddMemList	;size, attributes, pri, base, name
					; d0   d1          d2   a0    a1
		ENDC



******* expansion.library/ConfigBoard ***********************
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

	IFGE	INFOLEVEL-50
	movem.l	a0/a1,-(sp)
	INFOMSG	50,<'ConfigBoard: board 0x%lx, cd 0x%lx'>
	addq.l	#8,sp
	ENDC

		;------ don't go on if we are clogged
		;------ (someone could not be shut up)
		;	:TODO: Don't clog ZIII because of ZII
		moveq	#EE_NOEXPANSION,d0
		btst	#EBB_CLOGGED,eb_Flags(a6)
		bne	ConfigBoard_End

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
		cmp.b	#ERT_ZORROII,d1
		bne.s	ConfigBoard_Zorro_III

		;------ get the mem mask field of the type byte
		move.b	cd_Rom+er_Type(a3),d0	;pass entire byte
		CALLSYS	AllocBoardMem
		tst.l	d0
		bpl	ConfigBoard_RecordData



;--------------- there was not enough Zorro II memory for you
ConfigBoard_TryShutup:
ConfigBoard_NoExpansionMem:
		btst	#ERFB_NOSHUTUP,cd_Rom+er_Flags(a3)
		beq.s	ConfigBoard_CanShutup
		INFOMSG	450,<'ConfigBoard: board can not be shutup!'>

		;------ this board can't be shut up: it will live at the
		;------ config address.  This code used to be confused.
		;------ only a 64K board can be left at the config address;
		;------ a larger board will overlap other boards!
		;
		bset	#EBB_CLOGGED,eb_Flags(a6)

		cmp.w	#1,cd_SlotSize(a3)
		bne.s	ConfigBoard_ReturnError

		INFOMSG	450,<'ConfigBoard: 64K board - configuring it'>
		move.l	a2,d0			;configuration address
		moveq	#E_SLOTSHIFT,D1
		lsr.l	d1,d0			;turn into a slot number
		bra	ConfigBoard_RecordData


		;------ not enough expansion mem so shut him up
ConfigBoard_CanShutup:
		INFOMSG	450,<'ConfigBoard: attempting shutup'>
		;---!!! shouldn't shut up if bootable device? */
		bset	#EBB_SHORTMEM,eb_Flags(a6)
		moveq	#0,d1
		moveq	#ec_Shutup+ExpansionRom_SIZEOF,d0
		move.l	a2,a0
		CALLSYS	WriteExpansionByte

ConfigBoard_ReturnError:
		or.b	#CDF_BADMEMORY!CDF_SHUTUP,cd_Flags(a3)
		moveq	#EE_NOEXPANSION,d0
		bra	ConfigBoard_End
;---------------



*******************************************************************************
;
; Slots needed if the extended configuration bit is set.
; (This code never gives a Zorro III card less than 16MB)
;
Z3_Extended:
		dc.w	256	;16  Megabytes
		dc.w	512	;32  Megabytes
		dc.w	1024	;64  Megabytes
		dc.w	2048	;128 Megabytes
		dc.w	4096	;256 Megabytes
		dc.w	8192	;512 Megabytes
		dc.w	16384	;1   Gigabyte
		dc.w	-1	;RESERVED :TODO: reject


;
; Sub-size (amount of memory actually required by a PIC)
;		cd_BoardSize	- logical size of PIC
;		cd_SlotSize	- physical size of PIC
;
Z3_LogicalSize:
		dc.w 0		;Logical matches physical (not looked up)
		dc.w 0		;Automatically sized by OS
		dc.w 1		;64  Kilobytes
		dc.w 2		;128 Kilobytes
		dc.w 4		;256 Kilobytes
		dc.w 8		;512 Kilobytes
		dc.w 16		;1   Megabyte
		dc.w 32		;2   Megabytes
		dc.w 64		;4   Megabytes
		dc.w 64+32	;6   Megabytes
		dc.w 128	;8   Megabytes
		dc.w 128+32	;10  Megabytes
		dc.w 128+64	;12  Megabytes
		dc.w 128+64+32	;14  Megabytes
		dc.w -1		;Reserved :TODO: reject
		dc.w -1		;Reserved :TODO: reject


ConfigBoard_Zorro_III:
		;
		;	Find the configuration size of the board
		;	(how much hardware space it responds to)
		;
		moveq	#0,d0			;clear upper bits
		btst.b	#ERFB_EXTENDED,cd_Rom+er_Flags(a3)
		beq.s	cb_OldTable		;force 16MB

		moveq	#ERT_MEMMASK,d0
		and.b	cd_Rom+er_Type(a3),d0	;pass entire byte
		add.w	d0,d0			;lsl.w	#1,d0
cb_OldTable:	lea.l	Z3_Extended(pc,d0.w),a0

		;[d0-high word clear]
		move.w	(a0),d0			; size   (Zorro III)
		move.w	d0,cd_SlotSize(a3)
		moveq	#0,d1			; offset (Zorro III)
		CALLSYS	AllocExpansionMem ;:TODO: in a few years, a range check
		move.w	d0,cd_SlotAddr(a3)
		move.w	d0,cd_BoardAddr(a3)

		IFGE	INFOLEVEL-300
		move.l	cd_BoardAddr(a3),-(sp)
		INFOMSG	300,<'Z3: Board address %lx'>
		addq.l	#4,sp
		ENDC

		;
		;	Find the logical size of the board
		;	(how much actual stuff is on the board)
		;
		moveq	#ERT_Z3_SSMASK,d0
		and.b	cd_Rom+er_Flags(a3),d0
		add.w	d0,d0
		beq.s	cd_Physical
		move.w	Z3_LogicalSize(pc,d0.w),d0
		bra.s	cb_Given
cd_Physical:	move.w	cd_SlotSize(a3),d0	;Logical matches physical
cb_Given:	move.w	d0,cd_BoardSize(a3)	;Assume low word is clear


		moveq	#ec_Z3_HighBase+ExpansionRom_SIZEOF,d0
		move.w	cd_SlotAddr(a3),d1
		move.l	a2,a0
		CALLSYS WriteExpansionWord
		bra.s	ConfigBoard_Ok
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
		CALLSYS	WriteExpansionByte




ConfigBoard_Ok:
		;Test & AutoSize board
		bsr.s	DoOptions	; destroys d2/a2

		moveq	#EE_OK,d0
		bset	#CDB_CONFIGME,cd_Flags(a3)	;"needs driver"

ConfigBoard_End:
		IFGE	INFOLEVEL-50
		move.l	cd_BoardSize(a3),-(sp)
		move.l	cd_BoardAddr(a3),-(sp)
		move.l	d0,-(sp)
		INFOMSG	50,<'ConfigBoard return=%ld. Address %lx, Size %lx'>
		addq.l	#8,sp
		addq.l	#4,sp
		ENDC

		movem.l	(sp)+,d2/a2/a3
		rts




****************************************************************************
*
*	Test & Size memory type boards.
*
*	a6-ExpansionBase
*	a2-scratch
*	a3-ConfigDev
*	d2-scratch
*
DoOptions:
		IFGE	INFOLEVEL-100
		move.l	a3,-(sp)
		INFOMSG	100,<'DoOptions: called for cd 0x%lx'>
		addq.l	#4,sp
		ENDC


		;------ test if the memory should be linked into the free list
		btst	#ERTB_MEMLIST,cd_Rom+er_Type(a3)
		beq.s	DoOptions_End

		move.l	cd_BoardAddr(a3),a2
		move.l	cd_BoardSize(a3),d0		; size
		bne.s	DoOptions_Sized

;------------------------------------------------------------------------------
;
;	Board is of unknown size.  We must sniff it.
;
;	This code clears the first longword of the board, then moves up
;	in EZ3_SIZEGRANULARITY chunks.  If longword won't store the magic
;	data, or the first longword reflects the write, the sizing is
;	done.  If we test all the way to the physical size of the board,
;	sizing is done.
;
;	The data bus is confused so that writes don't read back (sometimes
;	the value on the data bus is "sticky" if no device responds).
;
;
;	a6 - ExpansionBase
;
;	a2 - [BoardAddr]
;	a1 - working pointer
;	a0 - limit
;
;	d2 - holds first longword
;	d1 - magic data
;	d0 - scratch
;
;	d0 = cd_BoardSize(a3)
;
		moveq	#0,d0
		move.w	cd_SlotSize(a3),d0
		swap	d0		;E_SLOTSHIFT==16
		move.l	a2,a0
		add.l	d0,a0		;calculated end-of-board

		move.l	(a2),d2		;save first longword
		clr.l	(a2)

		move.l	a2,a1
		move.l	#$0F2D4B689,d1
		bra.s	enter_here

top_loop:	move.l	d0,(a1)         ;restore location
enter_here:	adda.l	#EZ3_SIZEGRANULARITY,a1
		cmpa.l	a1,a0
		beq.s	endofmem
		move.l	(a1),d0
		move.l	d1,(a1)
		tst.l	(a2)		;confuse data bus (reads zero)
		cmp.l	(a1),d1
		bne.s	endofmem	;no readback
		nop			;cause 68040 to sync pipeline
		tst.l	(a2)		;check first longword for shadow
		beq.s	top_loop	;shadow not detected...

endofmem:	move.l	d0,(a1)         ;restore location
		move.l	a1,d0
		sub.l	a2,d0		;LastAddr-BoardAddr=size
		move.l	d0,cd_BoardSize(a3)

		move.l	d2,(a2)		;restore first longword

		IFGE	INFOLEVEL-150
		move.l	a0,-(sp)
		move.l	d0,-(sp)
		move.l	a2,-(sp)
		INFOMSG 150,<'Autosize Base=$%lx Size=%lx Max=%lx'>
		addq.l	#8,sp
		addq.l	#4,sp
		ENDC
;------------------------------------------------------------------------------



DoOptions_Sized:
		;[D0-BoardSize]
		;[A2-BoardSize]
		;[A3-ConfigDev]
		tst.l	d0				; boardsize
		beq.s	DoOptions_Bad

		;------ _Very_ quick test to ensure memory actually exists
		;	:TODO: AutoSize _all_ boards, compare results
		;	:TODO: Improve memory test.
		move.l	(a2),d2		;save first longword
		clr.l	(a2)
		tst.l	(a2)
		bne.s	DoOptions_Bad
		moveq	#-1,d1
		not.l	(a2)
		cmp.l	(a2),d1
		bne.s	DoOptions_Bad
		move.l	d2,(a2)		;restore first longword
DoOptions_End:	rts


DoOptions_Bad:	bset.b	#EBB_BADMEM,eb_Flags(a6)
		bset.b	#CDB_BADMEMORY,cd_Flags(a3)
		rts
**************************************************************




*****i* internal/DiagList *****************************************************
*
*   FUNCTION
*	For V36 all processing of ConfigDevs has been moved to a seprate pass.
*
*	Before time starts, expansion runs RTF_SINGLETASK.  All boards are
*	configured to their final address, with ConfigDev structures kept in
*	chip memory.  Next all memory is added to the system.
*
*	A separate RTF_COLDSTART romtag starts this code.  After the system
*	is in good shape, It chains to all ConfigDevs in the system, calling
*	diag code.
*
*******************************************************************************
*
* CALLED
*	A6-SysBase
*	A0-A3
*	D0-D1
*
* USED
*	A6-ExpansionBase
*	A5-ExecBase
*	A3-Next
*	A2-DoDiag
*	D1-
*	D0-
*
DoDiagList:	MOVEM.L	D2/A2-A3/A5,-(SP)
		lea.l	ExpansionName(pc),a1
		CALLSYS	OldOpenLibrary
		move.l	d0,a5
		exg.l	a6,a5


		;[A5-exec, A6-expansion]
		;
		;	Call driver code for each card
		;
		lea.l	eb_BoardList(a6),a3

dd_Loop2:	TSTNODE	a3,a3
		beq.s	dd_EndList2
		btst	#ERTB_DIAGVALID,cd_Rom+er_Type(a3)
		beq.s	dd_Loop2	;loop
		move.l	cd_BoardAddr(a3),d0
		beq.s	dd_Loop2	;loop
		move.l	d0,a2
		IFGE	INFOLEVEL-100
		 move.l	a3,-(sp)
		 move.l	a2,-(sp)
		 move.l	a6,-(sp)
		 INFOMSG 100,<'A6=%lx Calling DoDiag for board=%lx cd=%lx'>
		 add.l	#12,sp
		ENDC
		bsr.s	DoDiag		;A2-board address A3-CD A5-exec A6-exp
		bra.s	dd_Loop2	;loop
dd_EndList2:


		exg.l	a6,a5
		move.l	a5,a1
		CALLSYS	CloseLibrary
		MOVEM.L	(SP)+,D2/A2-A3/A5
		rts



**************************************************************
*
*	Call the board's diag vector
*
*	A6-ExpansionBase
*	A5-ExecBase
*	A2 = boardbase	(preserved)
*	A3 = cd		(preserved)
*
DoDiag:		; (boardbase: a2, cd: a3)
		movem.l	d2/d3/a2/a4/a5/a6,-(sp)


		lea	-DiagArea_SIZEOF(sp),sp

		;------ get the diag vector
		CLEAR	d0
		move.w	cd_Rom+er_InitDiagVec(a3),d0
		beq	DoDiag_end

		;------ get first byte of diag area
		;------ (this works for nybble boards because
		;------ the flags are stored in the upper half of the byte)
		lea	0(a2,d0.l),a0
		move.l	a0,d3			; save it for later
		move.b	(a0),d0
		move.b	d0,d1

		;------ see if we need to bother now
		and.b	#DAC_BOOTTIME,d0
		cmp.b	#DAC_CONFIGTIME,d0
		bne.s	DoDiag_end


		;------ we need execbase from now on...
		move.l	ABSEXECBASE,a5
		exg	a5,a6

		;------ copy out the diag area (a0 still has source ptr)
		move.l	sp,a1			; dest pointer
		moveq	#DiagArea_SIZEOF,d0	; size
		and.w	#DAC_BUSWIDTH,d1
		beq.s	DoDiag_nybblecopy
		cmp.w	#DAC_BYTEWIDE,d1
		beq.s	DoDiag_bytewide
		cmp.w	#DAC_WORDWIDE,d1
		bne.s	DoDiag_end

		;------ wordwide copy -- a0 still has the source
		lea	wordToWordCopy(pc),a4
		bra.s	DoDiag_copyDiagArea

DoDiag_bytewide:
		lea	byteToWordCopy(pc),a4
		bra.s	DoDiag_copyDiagArea

DoDiag_nybblecopy:
		lea	nybbleToWordCopy(pc),a4

DoDiag_copyDiagArea:
		jsr	(a4)

		;------ make sure there is a boot vector
		tst.w	da_BootPoint(sp)
		beq.s	DoDiag_end

		;------ allocate memory for the diag area
		CLEAR	d0
		move.w	da_Size(sp),d0	;Bug fixed for V1.3
		CLEAR	d1
		CALLSYS	AllocMem

	IFGE	INFOLEVEL-600
	move.l	d0,-(sp)
	INFOMSG	600,<'expansion: diag area allocated at %lx'>
	addq.l	#4,sp
	ENDC

		tst.l	d0
		beq.s	DoDiag_end

		;------ copy in diag area
		move.l	d0,d2
		move.l	d0,a1
		move.l	d3,a0			; bring back base of diag area
		CLEAR	d0
		move.w	da_Size(sp),d0
		jsr	(a4)

		;------ we'll need to find our copy of DiagArea at AutoBoot time
		move.l	d2,cd_Rom+er_Reserved0c(A3)

		;------ set up the registers and jump to the rom/diagnostic code
		move.l	a2,a0	;[board base]
		move.l	d2,a2	;[pointer to copied diag area]

		CLEAR	d0
		move.w	da_DiagPoint(a2),d0
		beq.s	DoDiag_end

**
** These are the calling conventions for the diagnostic callback
** (da_DiagPoint).
**
** A7 -- points to at least 2K of stack
** A6 -- ExecBase
** A5 -- ExpansionBase
** A3 -- your board's ConfigDev structure
** A2 -- Base of diag/init area that was copied
** A0 -- Base of your board
**
** Your board must return a value in D0.  If this value is NULL, then
** the diag/init area that was copied in will be returned to the free
** memory pool.
**

		jsr	0(a2,d0.l)

		tst.l	d0
		bne.s	DoDiag_end

		;------ free up the expansion area
		move.l	a2,a1
	;	CLEAR	d0	; (already clear: see tst.l above)

		;------ our copy of DiagArea is no longer valid
		move.l	d0,cd_Rom+er_Reserved0c(A3)

		move.w	da_Size(sp),d0
		CALLSYS	FreeMem

DoDiag_end:
		lea	DiagArea_SIZEOF(sp),sp
		movem.l	(sp)+,d2/d3/a2/a4/a5/a6
		rts

******************************************

wordToWordCopy:
		JMPSYS	CopyMem

******************************************

byteToWordCopy_loop:
		move.b	(a0)+,(a1)+
		addq.l	#1,a0
byteToWordCopy:
		dbra	d0,byteToWordCopy_loop
		rts	;BUG fix: This RTS is new for V1.4

******************************************

nybbleToWordCopy:
		move.w	d2,-(sp)
		move.w	#0,d2
		CLEAR	d1
		bra.s	nybbleToWordCopy_entry

nybbleToWordCopy_loop:
		move.b	(a0),d2
		and.b	#$f0,d2
		move.b	2(a0),d1
		lsr.w	#4,d1
		or.b	d2,d1
		move.b	d1,(a1)+
		addq.l	#4,a0
nybbleToWordCopy_entry:
		dbra	d0,nybbleToWordCopy_loop

		move.w	(sp)+,d2
		rts
******************************************



memname:	dc.b	'expansion memory',0
		ds.w	0


*****i* expansion.library/ConfigChain ***********************
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

		CALLSYS	AllocConfigDev
		move.l	d0,a3
		tst.l	d0
		beq.s	ConfigChain_NoMem

		move.l	a3,a1
		move.l	a2,a0			;Zorro II/III
		CALLSYS	ReadExpansionRom
		move.l	a2,a0			;Zorro II/III
		tst.l	d0
		beq.s	ConfigChain_GotBoard

		move.l	a3,a1
		move.l	a4,a0			;Zorro II
		CALLSYS	ReadExpansionRom
		move.l	a4,a0			;Zorro II
		tst.l	d0
		bne.s	ConfigChain_FreeCD	;end of chain...


ConfigChain_GotBoard:
		move.l	a3,a1
		CALLSYS	ConfigBoard
		move.l	a3,a0
		CALLSYS	AddConfigDev
		BRA.S	ConfigChain_Loop




		;------ there is no board there
ConfigChain_FreeCD:
		move.l	a3,a0
		CALLSYS	FreeConfigDev
ConfigChain_Exit:
		CLEAR	d0
ConfigChain_End:
		movem.l	(sp)+,a2/a3/a4
		rts

ConfigChain_NoMem:
		moveq	#EE_NOMEMORY,d0
		bra.s	ConfigChain_End



		IFNE	A3000_SUPERKICKSTART
;
;	SetTT1 (TT1,TT0)
;		d0  d1
;
SetTT:		move.l	ABSEXECBASE,a0
		btst.b	#AFB_68040,AttnFlags+1(a0)
		bne.s	stt_noaction
		move.l	d0,-(sp)
		dc.w	$f017,$0c00		; PMOVE (sp),TT1
		move.l	d1,-(sp)
		beq.s	stt_none
		dc.w	$f017,$0800		; PMOVE (sp),TT0
stt_none:	addq.l	#8,sp
stt_noaction:	rts
		ENDC


ExpansionReserved26:
		moveq	#0,d0
		rts

		END
