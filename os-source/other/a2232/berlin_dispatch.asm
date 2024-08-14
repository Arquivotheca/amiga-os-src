**
**	$Id: berlin_dispatch.asm,v 1.4 91/08/10 22:25:31 bryce Exp Locker: bryce $
**
**	Multiserial specific init, expunge, open, close and interrupts
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION main,CODE

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "libraries/configregs.i"
	INCLUDE "libraries/configvars.i"
	INCLUDE "hardware/intbits.i"
	INCLUDE "intuition/preferences.i"
	INCLUDE "devices/serial.i"
	LIST

	INCLUDE "macros.i"
	INCLUDE "device_base.i"
	INCLUDE "berlin.i"

;---------- Exports ---------------------------------------------------------

	XDEF	_MD_ColdInit
	XDEF	_MD_Expunge
	XDEF	_MD_Open
	XDEF	_MD_Close

;---------- Imports ---------------------------------------------------------

	XREF	PORTS_IRQ
	XREF	server_object
	XREF	InterruptNameText
	XREF	do_IntuiName


*****i* multidev/MD_ColdInit ************************************************
*
*   SYNOPSIS
*	_MD_ColdInit(DeviceBase,ExecBase);
*		     A5 	A6
*
*   FUNCTION
*	Called once when the device is fired up.  Ramlib takes care of
*	single-threading.  This function must:
*		1> Find the hardware, indicate ownership
*		2> RESET it
*		3> Prepare a unitdata structure and return
*
*   INPUTS
*	A6 - ExecBase
*	A5 - DeviceBase
*	A2 - scratch
*	D2 - scratch
*
*   RESULT
*	D0-IO_ERROR return code.
*
*****************************************************************************
*
*	A2 - scratch

_MD_ColdInit:

;----------------------------------------------------------------------------
;---- Open expansion.library, set md_ExpansionBase
;----------------------------------------------------------------------------
		lea.l	expansionName(pc),a1
		moveq.l #LIBRARY_MINIMUM,d0
		JSRLIB	OpenLibrary
		move.l	d0,md_ExpansionBase(a5)
		beq	ci_NoLibrary

		*
		*   Default rate at which the card interrupts the Amiga.
		*
		move.b	#MAGIC_INTERRUPT_FREQUENCY,md_TestIntFreq(a5)


;----------------------------------------------------------------------------
;---- Find all boards, test and download 6502 code, record information	  ---
;----------------------------------------------------------------------------
		bsr	FindAndConfigure


;============================================================================
;---- Allocate memory for all units on a single board
;----
;---- A single array is created for all units on all cards.  The size of
;---- this array is (((mdu_SIZE * NumACIAs) * boards)
;============================================================================
;   A2-CD
;   D0-Running AllocMem counter
;

		;Count number of A2232 boards installed
		move.l	md_OurConfigDev(a5),a2
		move.l	a2,d0
		beq	ci_Zombie		    ;No A2232 installed

		moveq	#4,d0			    ;Running AllocMem total+4
		move.w	#mdu_SIZE,md_UnitSize(a5)
ci_more:	addq.l	#NumACIAs,md_NumUnits(a5)
		add.l	#NumACIAs*mdu_SIZE,d0
		move.l	cd_NextCD(a2),a2
		move.l	a2,d1
		bne.s	ci_more 		    ;loop

		move.l	d0,a2			    ;size
		move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1
		JSRLIB	AllocMem
		PRINTF	600,<'------------Unit memory at %lx------------'>,d0
		tst.l	d0
		beq	ci_NoMemory
		move.l	d0,a0
		move.l	a2,(a0)+                    ;size +4
		move.l	a0,md_UnitArray(a5)


;----------------------------------------------------------------------------
;---- Loop to initialize static information for all unit structures
;----------------------------------------------------------------------------
;   a0-memory
;   a1-offset into A2232
;   a2-ConfigDev
;   d0-loop counter
;   d1-acia number
		move.l	md_UnitArray(a5),a0
		move.l	md_OurConfigDev(a5),a2

		;[a0=Current Memory Index]
ci_Next:
		;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		;Allocate one permanent interrupt server per board
		move.l	a0,-(sp)
		  moveq   #IS_SIZE,d0
		  move.l  #MEMF_CLEAR!MEMF_PUBLIC,d1
		  JSRLIB  AllocMem
		move.l	(sp)+,a0
		move.l	d0,cd_IntServer(a2) ;init'ed to 0 by find loop
		beq.s	ci_NoMemory
		move.l	d0,a1
	       ;move.b	#NT_INTERRUPT,LN_TYPE(a1)   ;flag for cd_IntServer
		move.b	#MAGIC_INTERRUPT_PRIORITY,LN_PRI(a1)
		move.l	#PORTS_IRQ,IS_CODE(a1)
		move.l	a0,IS_DATA(a1)  ;Start of Unit structures for board
		move.l	#InterruptNameText,LN_NAME(a1)
		;<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


		;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		move.l	cd_BoardAddr(a2),a1
		add.w	#global_acia_start,a1	;Calculate first ControlArea
		moveq	#NumACIAs-1,d0
		moveq	#0,d1
ci_InitUnitLp:
		move.l	a6,mdu_SysBase(a0)
		move.l	cd_BoardAddr(a2),mdu_BoardBase(a0)
		move.l	a2,mdu_ConfigDev(a0)
		move.b	d1,mdu_ACIANumber(a0)   ;ACIA associated with unit
		move.l	a1,mdu_ControlArea(a0)  ;Control area for this ACIA
		addq.b	#1,d1
		add.w	#acia_struct_SIZEOF,a1	;increment A2232 offset
		add.w	#mdu_SIZE,a0		;increment memory pointer
		dbra	d0,ci_InitUnitLp
		;<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		move.l	cd_NextCD(a2),a2
		move.l	a2,d0
		bne.s	ci_Next


ci_Zombie:	move.l	a5,d0
		rts

;----------------------------------------------------------------------------
;---- Bad Exit
;----------------------------------------------------------------------------
ci_NoLibrary:
ci_NoMemory:	moveq	#0,d0	;:TODO: On no memory case, check cleanup
		rts


expansionName:	DC.B	'expansion.library',0
		CNOP	0,4




;============================================================================
;----- Find all unclaimed a2232 boards.  Set CDB_CONFIGME, md_OurConfigDev.
;============================================================================
;
;   INPUTS
;	A6 - ExecBase
;	A5 - Devicebase
;
;   USAGE
;	A6 - ExpansionBase
;	A2 - Current ConfigDev
;	A3 - Singly linked list of ConfigDev structures
;

FindAndConfigure:
		PUSHM	a2/a3/a6
		move.l	md_ExpansionBase(a5),a6 ;[A6=ExpansionBase]
		suba.l	a2,a2			;No previous ConfigDev
		lea.l	md_OurConfigDev(a5),a3  ;Stuff next CD pointer here.

;>>>>> Obtain exclusive rights to configure boards >>>>>>>>>>>>>>>>>>>>>>>>>
		JSRLIB	ObtainConfigBinding

fac_FindNext:	move.l	a2,a0
		move.l	#EXPANSION_MANUFACTURER_ID,d0
		moveq.l #EXPANSION_PRODUCT_ID,d1
		JSRLIB	FindConfigDev
		tst.l	d0
		beq.s	fac_EndLoop
		move.l	d0,a2

		PRINTF	100,<'ConfigDev=$%lx, BoardAddr=$%lx'>,d0,cd_BoardAddr(a2)
		bclr.b	#CDB_CONFIGME,cd_Flags(a2)  ;Is this board in use?
		beq	fac_FindNext		    ;Yes...

		;[D0=ConfigDev]
		bsr.s	TestAndDownLoad 	;Process & download from D0
		tst.l	d0
		beq	fac_FindNext		;Board was bad...
		;Note: CDB_CONFIGME bit left cleared on bad board.
		;Note: Should return -1 (ioerr_selftest)

		clr.l	cd_IntServer(a2)        ;paranoia
		move.l	a2,(a3)                 ;Chain CD's via this field
		lea.l	cd_NextCD(a2),a3        ;next place to stuff value
		clr.l	(a3)                    ;paranoia
		bra	fac_FindNext

fac_EndLoop:	JSRLIB	ReleaseConfigBinding
;<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


		POPM
		rts


;----------------------------------------------------------------------------
;---- Test the card, download the driver, start the 6502 and verify. --------
;----------------------------------------------------------------------------
;
;   INPUTS
;	D0 - ConfigDev of an A2232 card
;
;   RESULTS
;	D0 - ConfigDev, or NULL if board failed.
;
;   USAGE
;	A2 - board address
;

TestAndDownLoad:
		PUSHM	a2
		move.l	d0,a2
		move.l	cd_BoardAddr(a2),a2


	;---- stop 6502 before download -----
		move.l	#global_long_stop_6502,d0
		tst.w	0(a2,d0.l)

	;---- test & clear 6502 memory (STATIC RAM RULES)
		;>>>>>>>>>>>>>>>>>>>>>>>
		;---- loop to fill memory with address pattern
		move.l	a2,a0
		move.w	#(global_memory_size/4)-1,d1
tad_Pattern:	move.l	a0,(a0)+
		dbra	d1,tad_Pattern
		bsr.s	tad_FlushCache

		;---- check for pattern, test memory, test again, clear
		move.l	a2,a0
		move.w	#(global_memory_size/4)-1,d1
tad_Test:	cmpa.l	(a0),a0         ;Unique address test
		bne.s	tad_BadMemory

		move.l	a0,d0		;Bit function test
		neg.l	(a0)
		neg.l	d0
		cmp.l	(a0),d0
		bne.s	tad_BadMemory

		move.l	#$5555,d0	;Bit short test (0->1 and 1->0)
		move.l	d0,(a0)
		cmp.l	(a0),d0
		bne.s	tad_BadMemory

		clr.l	(a0)
		tst.l	(a0)+
		dbne	d1,tad_Test
		;<<<<<<<<<<<<<<<<<<<<<<<
		beq.s	tad_GoodMemory


tad_BadMemory:	POPM	NOBUMP
		moveq	#0,d0
		rts		;(exit)


;---- memory is good, continue ----------------------------------------------
tad_GoodMemory: bsr.s	tad_FlushCache
		moveq	#0,d1		;load
		bsr.s	tad_DownLoad
		moveq	#1,d1		;verify
		bsr.s	tad_DownLoad
		bne.s	tad_BadMemory
		bsr.s	tad_FlushCache

	;---- start 6502
		move.l	#global_long_start_6502,d0
		tst.w	0(a2,d0.l)

	;---- Wait for 6502 to initialize
	;:TODO: Timeout/EasyRequest (6502 processor failed to start/bad mem.)
tad_BusyWait:	PRINTF	100,<'tad_BusyWait'>
		tst.b	global_startupflag(a2)
		beq.s	tad_BusyWait

		move.l	a2,d0
		POPM
		rts		;(exit)


;
;	Flush the instruction & data caches.  NOTE WELL: The 68030
;	data cache has a "bug".  On longword aligned longword writes
;	the 68030 allocates a valid date cache entry EVEN IF THE HARDWARE
;	asserts cache inhibit.
;
;	Only the MMU's CI bit can prevent this operation.  Since we can't
;	depend on the likes of Enforcer in the system, we flush the cache.
;
tad_FlushCache:	move.l	a6,-(sp)
		move.l	ABSEXECBASE,a6
		cmp.w	#CACHE_ROM_VERSION,LIB_VERSION(a6)
		blo.s	tad_TooOld
		JSRLIB	CacheClearU
tad_TooOld:	move.l	(sp)+,a6
		rts



;============================================================================
;----- Either download or check the 6502 code.	We do a download pass,	-----
;----- then a verify pass						-----
;============================================================================
;
;   INPUTS
;	D1 - 0 = download  1 = check
;	A2 - BoardBase
;
;   RETURN
;	Z=1 - ok
;	Z=0 - bad
;
tad_DownLoad:	PRINTF	600,<'DownLoading Code'>
		lea.l	server_object,a0
tad_NextLine:	move.b	(a0)+,d0            ;grab offset
		asl.w	#8,d0		    ;xx00
		move.b	(a0)+,d0            ;xxyy
		tst.w	d0
		beq.s	tad_EndObject
		lea.l	0(a2,d0.w),a1       ;dest address
		moveq	#0,d0		    ;clear high count...
		move.b	(a0)+,d0            ;...grab number of bytes-1
		PRINTF	998,<'Loading %ld+1 bytes from %lx to %lx'>,d0,a0,a1

		;load or verify, then back to tad_NextLine
		tst.l	d1
		bne.s	tad_Verify
tad_InnerCopy:	move.b	(a0)+,(a1)+         ;load
		dbra	d0,tad_InnerCopy
		bra.s	tad_NextLine
tad_Verify:	cmp.b	(a0)+,(a1)+         ;compare
		dbne	d0,tad_Verify
		beq.s	tad_NextLine

tad_EndObject:	rts			    ;(exit) Z is return value



*****i* multidev/MD_Expunge *************************************************
*
*   SYNOPSIS
*	void _MD_Expuge(DeviceBase,ExecBase);
*			a5	   a6
*
*   FUNCTION
*	Called by the memory allocator, under Forbid(), when the device
*	is expunged.
*
*****************************************************************************

_MD_Expunge:
		PUSHM	a2



;>>>>>>>>>>>>>>> loop through all boards freeing memory, etc. >>>>>>>>>>>>>>>

		move.l	md_OurConfigDev(a5),a2
		bra.s	mde_enter

mde_freeboard:	;
		;   Set "please config me" bit.
		;
		bset.b	#CDB_CONFIGME,cd_Flags(a2)  ;Release ownership
		bne.s	mde_next		    ;if we did not own...

		;
		;   Free permanent interrupt structure (one per board)
		;
		move.l	cd_IntServer(a2),d0
		beq.s	mde_next
		clr.l	cd_IntServer(a2)
		move.l	d0,a1
		moveq	#IS_SIZE,d0
		JSRLIB	FreeMem

mde_next:	move.l	cd_NextCD(a2),a2
mde_enter:	move.l	a2,d0
		bne.s	mde_freeboard

;<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




		;---- Free Unit memory
		move.l	md_UnitArray(a5),a1
		move.l	a1,d0
		beq.s	mde_nofreemem
		move.l	-(a1),d0    ;Size of allocation stored before ptr.
		JSRLIB	FreeMem
mde_nofreemem:

		;---- Close expansion.library
		move.l	md_ExpansionBase(a5),d0
		beq.s	mde_noexp
		move.l	d0,a1
		JSRLIB	CloseLibrary
mde_noexp:


		POPM
		rts




*****i* multidev/MD_Open ****************************************************
*
*   SYNOPSIS
*	_MD_Open()
*
*   FUNCTION
*	Berlin board specific open functions.  See list inline with code.
*
*   REGISTERS
*	[A1-IORequest D1-flags A5-Device A6-SysBase d0=IO_ERROR]
*
*****************************************************************************
*
*	a3 - IO_UNIT


		;[a1-IORequest d1-flags a5-Device a6-SysBase d0=IO_ERROR]
_MD_Open:	PUSHM	a3
		move.l	IO_UNIT(a1),a3



;-----Debugging-----
		PRINTF	499,<'_MD_Open IO_UNIT %lx'>,a3
		PRINTF	499,<' BoardBase $%lx'>,mdu_BoardBase(a3)
		PRINTF	499,<' ControlArea $%lx'>,mdu_ControlArea(a3)
		PRINTF	500,<' UseCount $%lx'>,mdu_UseCount(a3)



;-----Deal with SHARED bit-----
		moveq	#SerErr_UnitBusy,d0		;prepare error code
		btst.b	#SERB_SHARED,IO_SERFLAGS(a1)
		beq.s	mdo_Exclusive


		;-----shared case-----
		cmp.w	#-1,mdu_UseCount(a3)
		beq	mdo_exit	    ;someone has exclusive access...
		addq.w	#1,mdu_UseCount(a3)
		cmp.w	#1,mdu_UseCount(a3)
		beq.s	mdo_firsttime
		;-----shared re-open does less initialization-----
		;[a1-IORequest(preserved) a6-ExecBase]
		bsr	mdo_FillDefaults
		bra	mdo_ok


		;-----exclusive case-----
mdo_Exclusive:	tst.w	mdu_UseCount(a3)
		bne	mdo_exit
		move.w	#-1,mdu_UseCount(a3)    ;Set count to -1
mdo_firsttime:



;-----fill in default settings from preferences-----
		;[a1-IORequest(preserved) a6-ExecBase]
		bsr	mdo_FillDefaults

;-----Prepare lists-----
		lea.l	mdu_xmit_LIST(a3),a0
		NEWLIST a0
		lea.l	mdu_rcvr_LIST(a3),a0
		NEWLIST a0
		lea.l	mdu_SS(a3),a0
		move.l	a1,-(sp)
		JSRLIB	InitSemaphore
		move.l	(sp)+,a1


;-----Copy over acia_struct info that we keep cached locally-----
		move.l	mdu_ControlArea(a3),a0
		BUSYWAIT acia_command_flag(a0)  ;ensure 6502 is clear FIRST
		;   Given a near-infinite speed CPU, it is possible that
		;   a command was left over from a previous close.  The
		;   protection is free, even if unused.

		PRINTF	601,<'SWIZZLE 1'>
		SWIZZLE acia_xmit_head(a0)
		move.w	d0,mdu_xmit_head(a3)
		move.b	acia_xmit_buffer_min_page(a0),mdu_xmit_min(a3)
		move.b	acia_xmit_buffer_max_page(a0),mdu_xmit_max(a3)
		move.w	mdu_xmit_max(a3),d0
		sub.w	mdu_xmit_min(a3),d0
		move.w	d0,mdu_xmit_size(a3)    ;size of xmit buffer

		;[mdu_ControlArea(a3)]
		PRINTF	600,<'SWIZZLE 2'>
		SWIZZLE acia_rcvr_head(a0)  ;flush read buffer on first open
		move.w	d0,mdu_rcvr_tail(a3)
		move.b	acia_rcvr_buffer_min_page(a0),mdu_rcvr_min(a3)
		move.b	acia_rcvr_buffer_max_page(a0),mdu_rcvr_max(a3)

		moveq	#0,d0
		move.w	mdu_rcvr_max(a3),d0
		sub.w	mdu_rcvr_min(a3),d0
		move.w	d0,mdu_rcvr_size(a3)    ;size of buffer
		lsr.w	#1,d0
		move.l	d0,mdu_rcvr_thresh(a3)  ;half size of buffer (LONG!)


;-----Set up how often we will be bothered by the 6502----
		move.l	mdu_BoardBase(a3),a0
		move.b	md_TestIntFreq(a5),global_interrupt_stash(a0)
		move.b	md_TestIntFreq(a5),global_interrupt_freq(a0)


;-----Create interrupt server, if none exists-----
		;[a5-Device a6-SysBase]
		move.l	a1,-(sp)
		move.l	a3,a0			;Unit structure
		bsr.s	CreateIntServer
		move.l	(sp)+,a1
		;[A1-IORequest]

;-----Tell 6502 the channel is enabled------
		move.l	mdu_BoardBase(a3),a0
		move.b	mdu_ACIANumber(a3),d0
		bset.b	d0,global_channels(a0)


;-----set initial serial parameters-----
		;[A1-IORequest]
		move.w	#SDCMD_SETPARAMS,IO_COMMAND(a1)
		JSRLIB	DoIO
		;[D0-error number  A1-garbage]

mdo_ok: 	moveq	#0,d0		;Ignore error from SDCMD_SETPARAMS
mdo_exit:	POPM			;Error number in d0
		rts



;============================================================================
;----- Set up interrupt server.  One server per board			-----
;============================================================================
		;[a0-Unit a5-Device a6-SysBase]
CreateIntServer:

		;
		;   Check if AddIntServer() has been done
		;
		move.l	mdu_ConfigDev(a0),a1
		move.l	cd_IntServer(a1),a1
		tst.b	LN_TYPE(a1)
		bne.s	cis_AlreadyIn

		;
		;   Add it.
		;
		move.b	#NT_INTERRUPT,LN_TYPE(a1)   ;Set cd_IntServer flag
		moveq	#INTB_PORTS,d0
		PRINTF	500,<'AddIntServer: node=%lx code=%lx'>,a1,IS_CODE(a1)
		JSRLIB	AddIntServer	;A1,D0
		PRINTF	980,<'After AddIntServer'>

cis_AlreadyIn:	rts



;============================================================================
;----- Fill in an IORequest with the scoop from preferences		-----
;----- This code copied from serial.device (many errors fixed)          -----
;============================================================================
;   a2-copy of IORequest

		;[a1-IORequest(preserved) a6-ExecBase]
mdo_FillDefaults:
		PUSHM	a1/a2

		;--clear out IORequest to prevent sticky bits from
		;--surviving CMD_FLUSH, or a miscoded Open().
		;--Only IO_SERFLAGS should not be totally cleared.
		lea.l	IO_CTLCHAR(a1),a0
		moveq	#((IO_SERFLAGS-IO_CTLCHAR)/2)-1,d0
mod_ClearRequestLoop:
		clr.w	(a0)+
		dbra	d0,mod_ClearRequestLoop
		and.b	#SERF_SHARED,IO_SERFLAGS(a1)
		clr.w	IO_STATUS(a1)

		lea.l	-pf_SIZEOF(sp),sp           ;link pf_SIZEOF bytes
		moveq	#33,d0
		lea.l	do_IntuiName(pc),a1
		JSRLIB	OpenLibrary
		tst.l	d0
		beq.s	do_NoIntui
		move.l	sp,a0
		move.l	d0,a6
		move.l	#pf_SIZEOF,D0
		JSRLIB	GetPrefs
		move.l	sp,a0
		move.l	a2,a1		    ;copy of IORequest
		;[a1-IORequest a0-prefs buffer]
		bsr.s	mdo_FillPrefs	    ;a1-IORequest a0-Prefs
		move.l	a6,a1
		move.l	ABSEXECBASE,a6
		JSRLIB	CloseLibrary
do_NoIntui:	lea.l	pf_SIZEOF(sp),sp    ;unlink pf_SIZEOF bytes

		POPM
		rts


;============================================================================
;----- Given a buffer and IORequest, fill in preferences info		-----
;============================================================================


;New definitions from V1.4 time
;SUBF_8192	 EQU 8192
;SUBF_16384	 EQU 16384
;SUBF_32768	 EQU 32768
;SUBF_65536	 EQU 65536
;SPARITY_MARK	EQU 3
;SPARITY_SPACE	EQU 4



		;[a1-IORequest a0-prefs buffer]
mdo_FillPrefs:

;--IO_CTLCHAR--
;--IO_BRKTIME--
;--IO_TERMARRAY--
;--IO_STATUS--
;--IO_RBUFLEN--
		move.l	#SER_DEFAULT_CTLCHAR,IO_CTLCHAR(a1)
		move.l	#SER_DEFAULT_BRKTIME,IO_BRKTIME(a1)
		;clr.l	 IO_TERMARRAY(a1)
		;clr.w	 IO_STATUS(a1)
		;clr.l	 IO_RBUFLEN(a1)

;--IO_BAUD--
		move.l	a3,-(sp)
		lea.l	sBaudTable(pc),a3
		move.w	pf_BaudRate(a0),d0
		asl.w	#2,d0	    ;Index into longword table
		move.l	0(a3,d0.w),IO_BAUD(a1)
		PRINTF	401,<'IO_BAUD set to %ld. '>,IO_BAUD(a1)
		move.l	(sp)+,a3

;--IO_WRITELEN--
;--IO_READLEN--
		moveq	#8,d1
		move.b	pf_SerRWBits(a0),d0
		and.b	#$f0,d0
		sub.b	d0,d1		;8-number = write bits
		move.b	d1,IO_WRITELEN(a1)
		PRINTF	401,<'IO_WRITELEN %lx. '>,d1

		moveq	#8,d1
		move.b	pf_SerRWBits(a0),d0
		lsr.b	#4,d0
		sub.b	d0,d1		;8-number = read bits
		move.b	d1,IO_READLEN(a1)
		PRINTF	400,<'IO_READLEN %lx.'>,d1

;--IO_STOPBITS--
		move.b	#1,IO_STOPBITS(a1)
		btst.b	#4,pf_SerStopBuf(a0)
		beq.s	sDoPrefs20
		addq.b	#1,IO_STOPBITS(a1)  ;Bump to 2
sDoPrefs20:

		moveq	#0,d0
		move.b	IO_STOPBITS(a1),d0
		PRINTF	401,<'IO_STOPBITS set to %ld. '>,d0


;--IO_EXTFLAGS--
;--IO_SERFLAGS--
		;clr.l	 IO_EXTFLAGS(a1) ;prefs can't set MARK/SPACE parity

		;--Parity--
		MOVE.B	pf_SerParShk(a0),d0
		LSR.B	#4,d0
		BEQ.S	sDoPrefsNoParity
		  BSET.B  #SERB_PARTY_ON,IO_SERFLAGS(a1)
		  CMP.B   #SPARITY_ODD,d0
		  BNE.S   sDoPrefsNODD
		  BSET.B  #SERB_PARTY_ODD,IO_SERFLAGS(a1)
sDoPrefsNODD:
		  CMP.B   #SPARITY_EVEN,d0
		  BNE.S   sDoPrefsNEVEN
		  BSET.B  #SERB_PARTY_ODD,IO_SERFLAGS(a1)
sDoPrefsNEVEN:
		  CMP.B   #SPARITY_MARK,d0
		  BNE.S   sDoPrefsNMARK
		  BSET.B  #SEXTB_MSPON,IO_EXTFLAGS+3(a1)
		  BSET.B  #SEXTB_MARK,IO_EXTFLAGS+3(a1)
sDoPrefsNMARK:
		  CMP.B   #SPARITY_SPACE,d0
		  BNE.S   sDoPrefsNSPACE
		  BSET.B  #SEXTB_MSPON,IO_EXTFLAGS+3(a1)
		  BCLR.B  #SEXTB_MARK,IO_EXTFLAGS+3(a1)
sDoPrefsNSPACE:
sDoPrefsNoParity:

		;--Handshake--
		;0- XON 1-RTS 2-NONE
		MOVE.B	pf_SerParShk(a0),d0
		AND.B	#$0F,d0
		BEQ.S	sDoPrefs35			    ;case 0
		  BSET.B  #SERB_XDISABLED,IO_SERFLAGS(a1)
		  CMP.B   #SHSHAKE_RTS,d1
		  BNE.S   sDoPrefs35			    ;case 2
		  BSET.B  #SERB_7WIRE,IO_SERFLAGS(a1)       ;case 1
sDoPrefs35:

		moveq	#0,d0
		move.b	IO_SERFLAGS(a1),d0
		PRINTF	400,<'IO_SERFLAGS set to %lx.'>,d0

		rts


sBaudTable:	DC.L 110    ;0
		DC.L 300    ;1
		DC.L 1200   ;2
		DC.L 2400   ;3
		DC.L 4800   ;4
		DC.L 9600   ;5
		DC.L 19200  ;6
		DC.L -1     ;7-MIDI is invalid
		DC.L 115200 ;8
;----------------------------------------------------------------------------




*****i* multidev ************************************************************
*
*  NAME
*	_MD_Close
*
*  FUNCTION
*
*
*****************************************************************************


		;[a5-DeviceBase  a6-SysBase  a1-IORequest]
_MD_Close:
		PUSHM	a2/a3
		PRINTF	600,<'_MD_Close'>


		;
		;   Check open count. -1 means exclusive access
		;   If last one out from the unit, drop DTR
		;
		move.l	IO_UNIT(a1),a3
		cmp.w	#-1,mdu_UseCount(a3)
		bne.s	mdc_notx	    ; not exclusive...
		move.w	#1,mdu_UseCount(a3)
mdc_notx:	subq.w	#1,mdu_UseCount(a3)
		bne.s	mdc_u_notlast

		    ;
		    ;	Close the specific channel we were using
		    ;
		    move.l  mdu_ControlArea(a3),a2
		    BUSYWAIT acia_command_flag(a2)  ;ensure 6502 is clear
		    move.b  #$a2,acia_command_flag(a2)  ;close,drop dtr
		    ;
		    ;	Note that the 6502 does above operation in parallel
		    ;

		   ;move.l  mdu_BoardBase(a3),a0
		   ;move.b  mdu_ACIANumber(a3),d0
		   ;bclr.b  d0,global_channels(a0)  ;Turn off channel :TODO:


mdc_u_notlast	;
		;   If last one out from device, turn off the lights
		;
		cmp.w	#1,LIB_OPENCNT(a5)      ;last one out??
		bne.s	mdc_LeaveInts
		    move.l  mdu_BoardBase(a3),a0
		    clr.b   global_interrupt_freq(a0)

		    ;
		    ;	Remove interrupt server (unless it is already gone)
		    ;
		    move.l  mdu_ConfigDev(a3),a1
		    move.l  cd_IntServer(a1),a1
		    tst.b   LN_TYPE(a1)     ;Check cd_IntServer flag
		    beq.s   mdc_NoReRem
		    clr.b   LN_TYPE(a1)
		    moveq   #INTB_PORTS,d0
		    PRINTF  100,<'RemIntServer $%lx'>,a1
		    JSRLIB  RemIntServer
mdc_NoReRem:
mdc_LeaveInts:	;
		;
		;


		POPM
		rts

		END
