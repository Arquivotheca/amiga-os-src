*******************************************************************************
*
* 68040 support library for 68881/68882 FPU emulation
*
* $Id: library.asm,v 1.25 93/01/18 17:55:09 mks Exp $
*
* $Log:	library.asm,v $
* Revision 1.25  93/01/18  17:55:09  mks
* Now does the ReadFromRAM DMA flag
* 
* Revision 1.24  93/01/18  10:18:44  mks
* Added new code to deal with the CachePreDMA/PostDMA
* situation such that non-cache-line-aligned transfers will
* only turn off copyback in the pages it is in.
*
* Revision 1.23  92/08/26  10:59:27  mks
* Added some needed PFLUSHA commands
*
* Revision 1.22  92/08/26  09:24:54  mks
* Some debugging cleanup and some fixes to the cache settings...
*
* Revision 1.21  92/08/25  19:27:56  mks
* First cut at the MMU-based 68040.library.  Only needs to
* do some 68EC040 testing yet...
*
* Revision 1.20  92/08/13  10:03:15  mks
* Conditional code for the ARP fix...
*
* Revision 1.19  92/08/10  17:58:22  mks
* Added special hack to deal with arp.library crash in copyback mode
*
* Revision 1.18  92/06/10  12:24:48  mks
* Added code to deal with 68040 copyback caches and DMA...
*
* Revision 1.17  91/10/11  07:59:01  mks
* Added EC040 support to the library
*
* Revision 1.16  91/08/20  15:02:19  mks
* Changed a conditional assembly flag to be set for non-3640 support
*
* Revision 1.15  91/07/23  22:40:07  mks
* Cleaned up for release
*
* Revision 1.14  91/07/17  13:01:26  mks
* Added the blank LVO for Switch030 but did not do anything with it.
*
* Revision 1.13  91/07/17  12:41:54  mks
* Cleaned up 68030 reset code.  Now if I could only get it to
* be able to get back out of reset!
*
* Revision 1.12  91/07/11  18:19:34  mks
* Added the beginnings of the 68030 boot support.  It works
* but you can't come back...
*
* Revision 1.11  91/07/10  17:42:25  mks
* Moved entry_open closer to the other entry points.
*
* Revision 1.10  91/07/09  11:00:19  mks
* Added the needed check for V37 exec (minimum version that
* supports the 68040)
*
* Revision 1.9  91/07/03  11:58:59  mks
* Made a branch word sized...
* Moved where the vectors are...
*
* Revision 1.8  91/07/02  13:28:06  mks
* Added code to move the VBR into FAST RAM up in the library data area
*
* Revision 1.7  91/06/26  20:01:25  mks
* Added conditional assembly for the SIMPLE_040 case which does
* not do any FPU or External Cache setting and thus will work with
* any 68040 hardware.
*
* Revision 1.6  91/06/24  16:27:16  mks
* Changed to use the 68040 cpusha instruction directly...
*
* Revision 1.5  91/06/24  07:42:02  mks
* Added / changed code to support both the old hardware (never released)
* and the new fixed hardware.
* Added the extra CacheControl() call to make sure that all of the
* caches have been flushed and reset before they are turned on for
* the first time.
* Now, once again, turns on CopyBack and External caches.
*
* Revision 1.4  91/06/13  17:25:03  mks
* New cache code built now and the caching is working
* Removed (for now) the installation of the FPU code since
* we have not yet gotten the final version.
*
* Revision 1.3  91/05/22  17:07:43  mks
* Changed to use the assembly_options.i include file
*
* Revision 1.2  91/05/22  11:44:34  mks
* Installed the Amiga specific mem_write routine and removed mem_read
* since it is not needed on the Amiga version
*
* Revision 1.1  91/05/21  16:20:19  mks
* Initial revision
*
*
*******************************************************************************
*
* Options for HX68 to turn on 68040 MODE...
*
	INCLUDE	'assembly_options.i'
*
*******************************************************************************
*
* Set this to 1 if you want debugging output to the serial port...
DEBUGGING	SET	0
*
* Set this to 1 if you want the FPU code added...
FPUCODE		SET	1
*
* Set this to 1 if you want the patch for arp.library added.  This
* patch hurts system performance, specifically:  REXX and SHELL
ARP_FIX		SET	0
*

;	Included Files

	INCLUDE 'exec/types.i'
	INCLUDE	'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/io.i'
	INCLUDE	'exec/execbase.i'

	INCLUDE	'exec/libraries.i'
	INCLUDE	'exec/devices.i'

	INCLUDE	'devices/input.i'

	INCLUDE	'hardware/cia.i'

	INCLUDE	'68040_rev.i'

;	Imported Names

*------ Tables -------------------------------------------------------

	XREF	EndCode
*
* Stuff we will patch
*
	XREF	_LVOCacheClearU
	XREF	_LVOCacheClearE
	XREF	_LVOCachePreDMA
	XREF	_LVOCachePostDMA
	XREF	_LVOCacheControl
	XREF	_LVOAddDevice
	XREF	_LVOAddLibrary
	XREF	_LVOCloseLibrary
	XREF	_LVOAddResource
	XREF	_LVOAddTask
	XREF	_LVOAddIntServer
	XREF	_LVOSetIntVector
	XREF	_AbsExecBase
*
*******************************************************************************
*
CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		jsr	_LVO\1(a6)
		ENDM
*
*******************************************************************************
*
JMPSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		jmp	_LVO\1(a6)
		ENDM
*
*******************************************************************************
*
* This is the MMU frame I use...
*
 STRUCTURE	MMU_Frame,0
	ULONG	mmu_CRP		; URP for 68040
	ULONG	mmu_CRP_1	; SRP for 68040
	ULONG	mmu_TC
	;
	APTR	mmu_LevelA
	APTR	mmu_LevelB
	APTR	mmu_LevelC
	;
	ULONG	mmu_Indirect
	APTR	mmu_Magic
	;
	ULONG	mmu_InvalidA	; The invalid value at LevelA  (68040 only)
	ULONG	mmu_InvalidB	; The invalid value at LevelB  (68040 only)
	ULONG	mmu_InvalidC	; The invalid value at LevelC  (68040 only)
	;
	STRUCT	mmu_NestCounts,MLH_SIZE	; Nest Count Table List
	;
	LABEL	MMU_Frame_SIZE
*
 STRUCTURE	Nest_Counts,0
	STRUCT	nc_Node,MLN_SIZE
	ULONG	nc_Low
	ULONG	nc_High
	LABEL	nc_Count
*
*******************************************************************************
*
* Just a block to prevent illegal entry...
*
		moveq	#0,d0
		rts
*
*******************************************************************************
*
* This is space for the exception vectors...
*
	IFNE	FPUCODE
NUM_VECS	set	256	; 256 vectors...
		cnop	0,4	; Make sure we are long-word alligned...
NewVectors:	ds.l	NUM_VECS
	ENDC
*
*******************************************************************************
*
initLDescrip:				; STRUCTURE RT,0
		dc.w	RTC_MATCHWORD	; UWORD RT_MATCHWORD
		dc.l	initLDescrip	; APTR  RT_MATCHTAG
		dc.l	EndCode		; APTR  RT_ENDSKIP
		dc.b	RTF_AUTOINIT	; UBYTE RT_FLAGS
		dc.b	VERSION		; UBYTE RT_VERSION
		dc.b	NT_LIBRARY	; UBYTE RT_TYPE
		dc.b	-126		; BYTE  RT_PRI
		dc.l	subsysName	; APTR  RT_NAME
		dc.l	VERSTR		; APTR  RT_IDSTRING
		dc.l	inittable	; APTR  RT_INIT
					; LABEL RT_SIZE
*
subsysName:	dc.b	'68040.library',0
VERSTR:		VSTRING
InputName:	dc.b	'input.device',0	; For patching input.device
		CNOP	0,2
*
inittable:	dc.l LIB_SIZE
		dc.l vectors
		dc.l 0
		dc.l initFunc
*
*******************************************************************************
*
* Define the vectors as needed...
*
V_DEF		MACRO
		DC.W	entry_\1-vectors
		ENDM

vectors:
		DC.W	-1
		V_DEF	open
		V_DEF	close
		V_DEF	expunge
		V_DEF	reserved
		V_DEF	stub1
		V_DEF	stub2
		DC.W	-1
*
*******************************************************************************
*
* Internal definitions...
*
		BITDEF	CACR,040_ICache,15
		BITDEF	CACR,040_DCache,31
*
*******************************************************************************
*
initFunc:	move.l	d0,a1				; Pointer to 68040 base
		move.w	#REVISION,LIB_REVISION(a1)	; Set revision...
*
* Check that we are at least V37 of exec...
*
		cmpi.w	#37,LIB_VERSION(a6)	; Check if exec is >= V37
		bcs.s	ClearOut		; If less than V37, too old...
*
* Now, we check if we have a 68040	(execbase is in a6...)
*
		btst.b	#AFB_68040,AttnFlags+1(a6)	; Check for 68040
		bne.s	Yes_68040
*
* No 68040, so remove the library...
*
ClearOut:	movem.l	a5/a6,-(sp)		; save regs used
		;
		; calculate mem used and then free it
		;
		move.l	a1,a5			; base in a5
		moveq	#0,d0			; Clear size...
		move.w	LIB_NEGSIZE(a5),d0	; Get negative size
		sub.l	d0,a1			; Adjust start memory pointer
		add.w	LIB_POSSIZE(a5),d0	; Add in positive size
		CALLSYS	FreeMem			; free our memory
		movem.l	(sp)+,a5/a6		; restore regs used
		;
		; Fall into the RTS below...
		;
*
* Both expunge and close are NOPs...  reserved is currently always NULL...
*
entry_reserved:
entry_expunge:
entry_close:	moveq.l	#0,d0			; return 0
just_rts:	rts
;
entry_open:	move.l	a6,d0			; Return library base
		rts
*
*******************************************************************************
*
* The following is an empty stub...
*
entry_stub1:
entry_stub2:	; not done yet...
		moveq.l	#0,d0			; Did not work
		rts				; Return  :-)
*
*******************************************************************************
*
* Now, we install ourselves into the system...  Set open count to 1...
*
Yes_68040:	move.w	#1,LIB_OPENCNT(a1)	; Set to 1...
		move.l	a1,-(sp)
		move.l	a5,-(sp)		; Save these
*
* We can not have interruptions while doing these patches...
*
		PRINTF	<'Turning off caches and tasks...'>
		CALLSYS	Forbid			; No task switches here...
*
		moveq.l	#0,d0			; Now, turn off the other
		moveq.l	#-1,d1			; ...caches via CacheControl
		CALLSYS	CacheControl		; ...so that we can setfunction
		move.l	d0,-(sp)		; Save old settings...
*
* First, the patches to EXEC cache functions:
* These must be done with all caches off...
*
	;
	; CachePreDMA
	;
	; This adds the special kludge to make 68040 and DMA devices
	; work with CopyBack modes turned on...
	;
		PRINTF	<'Installing CachePreDMA() patch'>
		lea	NewCachePreDMA(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOCachePreDMA,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
	;
	; CachePostDMA
	;
	; This adds the special kludge to make 68040 and DMA devices
	; work with CopyBack modes turned on...
	;
		PRINTF	<'Installing CachePostDMA() patch'>
		lea	NewCachePostDMA(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOCachePostDMA,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
	;
	; CacheControl
	;
	; This adds the 3640 external cache control bit to the
	; CacheControl code.  It also fixes the return values from
	; CacheControl to correctly return the BURST ENABLE bit
	; if the cache bit is on.  (68040 bursts all caches)
	; Since this is a complete replacement, we are done.
	;
		PRINTF	<'Installing CacheControl() patch'>
		lea	NewCacheControl(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOCacheControl,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
	;
	; AddLibrary
	;
	; This fixes programs/libraries that do not use
	; MakeLibrary() to generate the library structure.
	;
		PRINTF	<'Installing AddLibrary() patch'>
		lea	NewAddLibrary(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOAddLibrary,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldAddLibrary(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	IFNE	ARP_FIX
	;
	; CloseLibrary
	;
	; This fixes arp.library on 68040 machines since it
	; places some code onto the stack and then runs it...
	;
		PRINTF	<'Installing CloseLibrary() patch'>
		lea	NewCloseLibrary(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOCloseLibrary,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldCloseLibrary(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	ENDC
	;
	; AddDevice
	;
	; This fixes programs/libraries that do not use
	; MakeLibrary() to generate the library structure.
	;
		PRINTF	<'Installing AddDevice() patch'>
		lea	NewAddDevice(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOAddDevice,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldAddDevice(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	; AddResource
	;
	; This fixes programs/libraries that do not use
	; CacheClearU() after generating the resource.
	;
		PRINTF	<'Installing AddResource() patch'>
		lea	NewAddResource(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOAddResource,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldAddResource(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	; AddTask
	;
	; This fixes programs that install the code into memory
	; without flushing the caches.  This happens to also fix
	; the most common problem like this:  Fake seglist generation
	; for calls to CreateProc()  (A trick needed in pre-2.0 days)
	;
		PRINTF	<'Installing AddTask() patch'>
		lea	NewAddTask(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOAddTask,a0		; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldAddTask(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	; AddIntServer
	;
	; Once again, people had generated code that was then
	; installed as a server for the interrupts.  This should
	; be a very minor hit since very few call AddIntServer()
	;
		PRINTF	<'Installing AddIntServer() patch'>
		lea	NewAddIntServer(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOAddIntServer,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldAddIntServer(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
	;
	; SetIntVector
	;
	; Same issues as AddIntServer above...
	;
		PRINTF	<'Installing SetIntVector() patch'>
		lea	NewSetIntVector(pc),a0
		move.l	a0,d0			; Get pointer to new routine
		move.l	a6,a1			; Get library to be patched
		move.w	#_LVOSetIntVector,a0	; Get LVO offset...
		CALLSYS	SetFunction		; Install new code...
		lea	OldSetIntVector(pc),a0	; Get storage slot...
		move.l	d0,(a0)			; Save old code address...
*
* Now, patch input.device so that a IND_ADDHANDLER will flush
* the caches.  (Arg!  But this is a big payoff)
*
		PRINTF	<'Installing input.device/IND_ADDHANDLER patch'>
	; First, we need to find input.device on the list
		lea	DeviceList(a6),a0	; Get list structure
		lea	InputName(pc),a1	; Get input.device string
		CALLSYS	FindName		; Find it on the list
		move.l	d0,a1			; This is the device we patch
		tst.l 	d0			; Check if NULL
		beq.s	NoINDPatch		; If NULL, no Patch...
	;
	; We patch BeginIO in input.device to check for ADDHANDLER
	; as the command.  Since many tools copy up code for use as
	; input handlers and just ADDHANDLER them, this will fix
	; all of those caching issues.
	;
		lea	NewBeginIO(pc),a0	; Get new code
		move.l	a0,d0			; address for SetPatch...
		move.w	#DEV_BEGINIO,a0		; LVO offset for BeginIO...
		CALLSYS	SetFunction		; Install it...
		lea	OldBeginIO(pc),a0	; Save old code address
		move.l	d0,(a0)			; ...for the patch.
NoINDPatch:
		PRINTF	<'Installed all patches.'>

*
* We need to install ourselves in the interrupt vectors.
*
	IFNE	FPUCODE
*
* Check if we have a 68040 FPU
*
		btst.b	#AFB_FPU40,AttnFlags+1(a6)
		beq.s	NoFPU40			; If not FPU, skip FPU code...
*
* Install the FPU emulation code since we have a 68040 FPU
*
		lea	start_fpsp(pc),a5	; Get address
		CALLSYS	Supervisor		; Call it in super state
*
* Ok, we are now "IN" and need to set up the flags such that we look
* like a 68881/2 FPU...	(a6 is still execbase...)
*
		bset.b	#AFB_68881,AttnFlags+1(a6)	; Set the 68881
		bset.b	#AFB_68882,AttnFlags+1(a6)	;     and 68882 flags
NoFPU40:
	ENDC
*
* Ok, now set DTT1 for CopyBack mode and other supervisor type things...
*
		lea	TestMMU(pc),a5		; Get address...
		CALLSYS	Supervisor		; Do it in supervisor mode...
		PRINTF	<'MMU Test Done.  ROM=%08lx'>,d0
		tst.l	d0			; Check ROM address...
		beq.s	NoMMU			; If none, no MMU...
		xref	_BuildMMU		; The MMU table builder...
		jsr	_BuildMMU		; Build the MMU with d0 ROM
		PRINTF	<'MMU Frame=%08lx'>,d0
		tst.l	d0			; Check MMU Frame...
		beq.s	NoMMU			; If no Frame, no MMU...
		lea	OnMMU(pc),a5		; Get address...
		CALLSYS	Supervisor		; Turn on the MMU...
		PRINTF	<'MMU Turned on'>
NoMMU:
*
* Now, turn on caches again...
*
		move.l	(sp)+,d0		; Get old settings
		moveq.l	#-1,d1			; (All bits...)
		PRINTF	<'Turning on caches:  %08lx'>,d0
		CALLSYS	CacheControl		; Turn back on...
*
* Let the system multi-task again...
*
		PRINTF	<'Turning tasks back on...'>
		CALLSYS	Permit			; Let's go!
*
* Ok, we are done and installed.  Put together the correct return results
*
		move.l	(sp)+,a5		; Restore
		move.l	(sp)+,d0		; Get library base...
		rts
*
******************************************************************************
*
* This code will do the following:  It will set up DTT1 to be the right
* cache mode (CopyBack) and it will check for the MMU.  If there is an
* MMU, it will return (in D0) the address of the ROM.  If there is no MMU,
* it will return NULL in D0
*
TestMMU:	movec.l	DTT1,d1		; Get DTT1
		and.b	#$9F,d1		; Mask off cache modes....
		or.b	#$20,d1		; Set it to COPYBACK...
		movec.l	d1,DTT1		; Save new DTT1
*
* Now for the MMU...
*
		moveq.l	#0,d0		; Assume we did not work...
		or.w	#$0700,sr	; Full DISABLE now...
		movec.l	tc,d1		; Get TC...
		tst.w	d1		; Check if on...
		bmi.s	tmmu_On		; If on, test for ROM...
		bset.l	#15,d1		; Turn on MMU...
		movec.l	d1,tc		; Try to turn it on...
		movec.l	tc,d1		; Get it back...
		movec.l	d0,tc		; Turn it off again...
		tst.w	d1		; Did it turn on?
		bpl.s	tmmu_Done	; If not, we have no MMU
*
* Ok, so we have an MMU, now test for the ROM address...
*
tmmu_On:	move.l	#$00F80000,d0	; We have
		moveq.l	#1,d1		; USER space
		movec.l	d1,dfc		; ...set DFC
		movec.l	d1,sfc		; ...set SFC
		move.l	d0,a0		; Get ROM address...
		ptestr	(a0)		; Check if we can read...
		movec.l	mmusr,d1	; Get status of PTEST
		btst.l	#1,d1		; Was it an ITT hit?
		bne.s	tmmu_Done	; If so, we exit...
		and.w	#$F000,d1	; Mask lower words...
		move.l	d1,d0		; ROM address...
tmmu_Done:	rte			; Return...
*
******************************************************************************
*
* This routine turns on the MMU with the MMU Frame given...
*
OnMMU:		lea	MMUFrame(pc),a0		; Get MMU Frame storage...
		move.l	d0,(a0)			; Store it...
		move.l	d0,a0			; Get MMU frame...
		or.w	#$0700,sr		; Full disable...
		pflusha				; Flush the whole ATC
		move.l	(a0)+,d0		; Get URP...
		movec.l	d0,urp			; User root pointer...
		move.l	(a0)+,d0		; Get SRP...
		movec.l	d0,srp			; Supervisor root pointer
		move.l	(a0)+,d0		; Get TC
		movec.l	d0,tc			; Set Translation Control
		pflusha				; Once more, flush the ATC
		moveq.l	#0,d0			; NULL ITTx and DTTx...
		movec.l	d0,itt0			; Turn it off...
		movec.l	d0,itt1			; Turn it off...
		movec.l	d0,dtt0			; Turn it off...
		movec.l	d0,dtt1			; Turn it off...
		rte
*
******************************************************************************
*
* This is the MMU frame.  NULL on systems without MMU setup.
*
MMUFrame:	dc.l	0	; MMU frame...
*
******************************************************************************
*
* AddLibrary patch code
*
OldAddLibrary:	dc.l	0			; Storage for old
NewAddLibrary:	move.l	OldAddLibrary(pc),-(sp)	; Set so RTS to old code
		move.l	a1,-(sp)		; Only A1 is needed...
		CALLSYS	CacheClearU		; Clear the caches
		move.l	(sp)+,a1		; Restore
		rts
*
******************************************************************************
*
	IFNE	ARP_FIX
*
* CloseLibrary patch code
*
OldCloseLibrary:
		dc.l	0			; Storage for old
NewCloseLibrary:
		move.l	OldCloseLibrary(pc),-(sp)	; Set so RTS to old
		move.l	a1,-(sp)		; Only A1 is needed...
		CALLSYS	CacheClearU		; Clear the caches
		move.l	(sp)+,a1		; Restore
		rts
*
	ENDC
*
******************************************************************************
*
* AddDevice patch code
*
OldAddDevice:	dc.l	0			; Storage for old
NewAddDevice:	move.l	OldAddDevice(pc),-(sp)	; Set so RTS to old code
		move.l	a1,-(sp)		; Only A1 is needed...
		CALLSYS	CacheClearU		; Clear the caches
		move.l	(sp)+,a1		; Restore
		rts
*
******************************************************************************
*
* AddResource patch code
*
OldAddResource:	dc.l	0			; Storage for old
NewAddResource:	move.l	OldAddResource(pc),-(sp) ;Set so RTS to old code
		move.l	a1,-(sp)		; Only A1 is needed...
		CALLSYS	CacheClearU		; Clear the caches
		move.l	(sp)+,a1		; Restore
		rts
*
******************************************************************************
*
* AddTask patch code
*
OldAddTask:	dc.l	0			; Storage for old
NewAddTask:	move.l	OldAddTask(pc),-(sp)	; Set so RTS to old code
		move.l	a1,-(sp)		; Only A1 is needed...
		CALLSYS	CacheClearU		; Clear the caches
		move.l	(sp)+,a1		; Restore a1
		rts
*
******************************************************************************
*
* AddIntServer patch code
*
OldAddIntServer:
		dc.l	0			; Storage for old
NewAddIntServer:
		move.l	OldAddIntServer(pc),-(sp)	; Set so RTS to old code
		movem.l	d0/a1,-(sp)		; Only D0/A1 are needed...
		CALLSYS	CacheClearU		; Clear the caches
		movem.l	(sp)+,d0/a1		; Restore a1
		rts
*
******************************************************************************
*
* SetIntVector patch code
*
OldSetIntVector:
		dc.l	0			; Storage for old
NewSetIntVector:
		move.l	OldSetIntVector(pc),-(sp)	; Set so RTS to old code
		movem.l	d0/a1,-(sp)		; Only D0/A1 are needed...
		CALLSYS	CacheClearU		; Clear the caches
		movem.l	(sp)+,d0/a1		; Restore a1
		rts
*
******************************************************************************
*
* input.device BeginIO patch code to trap/flush on IND_ADDHANDLER
*
OldBeginIO:	dc.l	0			; Storage for old
NewBeginIO:	move.l	OldBeginIO(pc),-(sp)	; Set so RTS to old code
		; Now, check if it is IND_ADDHANDLER
		cmp.w	#IND_ADDHANDLER,IO_COMMAND(a1)
		bne.s	Not_ADDHANDLER		; If not ADDHANDLER, skip...
		movem.l	a1/a6,-(sp)		; save these
		move.l	_AbsExecBase,a6		; Get EXECBASE
		CALLSYS	CacheClearU		; Clear the caches
		movem.l	(sp)+,a1/a6		; Restore...
Not_ADDHANDLER:	rts
*
******* exec.library/CachePostDMA ********************************************
*
*   NAME
*	CachePostDMA - Take actions after to hardware DMA  (V37)
*
*   SYNOPSIS
*	CachePostDMA(vaddress,&length,flags)
*	             a0       a1      d0
*
*	CachePostDMA(APTR,LONG *,ULONG);
*
*   FUNCTION
*	Take all appropriate steps after Direct Memory Access (DMA).  This
*	function is primarily intended for writers of DMA device drivers.  The
*	action will depend on the CPU type installed, caching modes, and the
*	state of any Memory Management Unit (MMU) activity.
*
*	As implemented
*		68000 - Do nothing
*		68010 - Do nothing
*		68020 - Do nothing
*		68030 - Flush the data cache
*		68040 - Flush matching areas of the data cache
*		????? - External cache boards, Virtual Memory Systems, or
*			future hardware may patch this vector to best emulate
*			the intended behavior.
*			With a Bus-Snooping CPU, this function my end up
*			doing nothing.
*
*   INPUTS
*	address	- Same as initially passed to CachePreDMA
*	length	- Same as initially passed to CachePreDMA
*	flags	- Values:
*			DMA_NoModify - If the area was not modified (and
*			thus there is no reason to flush the cache) set
*			this bit.
*
*   SEE ALSO
*	exec/execbase.i, CachePreDMA, CacheClearU, CacheClearE
*
******************************************************************************
*
* Replace CachePostDMA to handle the 68040 CopyBack vs DMA problem...
*
* This is a real nasty problem:  We have to watch out for DMA to memory
* while the CPU is accessing memory within the same cache line.
* This all mixes in with the CacheControl function since what we
* will do is to have PreDMA turn off CopyBack mode and PostDMA
* turn it back on...  (only if needed as CacheControl() may have
* been called too...  arg!!!
*
NewCachePostDMA:
		btst.l	#DMAB_ReadFromRAM,d0	; Check if READ DMA
		bne.s	dma_Caches		; If so, skip...
		move.l	a0,d1			; Get address...
		or.l	(a1),d1			; or in length...
		and.b	#$0F,d1			; Check for non-aligned...
		beq.s	dma_Caches		; Don't count if aligned...
*
* Now, we check if we can do the MMU trick...
*
		move.l	MMUFrame(pc),d1		; Get MMU frame
		bne.s	On_MMU_Way		; Do it the MMU way...
*
		lea	Nest_Count(pc),a1	; We trash a1...
		subq.l	#1,(a1)			; Subtract the nest count...
		bra.s	dma_Caches		; Do the DMA work...
*
* Ok, so we have an MMU and need to deal with turning on the pages
*
On_MMU_Way:	move.l	a0,-(sp)		; (result, fake)
		move.l	a4,-(sp)		; Save a4
		lea	On_MMU_Page(pc),a4	; Address of Cache ON code
		bra.s	MMU_Way			; Do the common code...
*
*****o* exec.library/CachePreDMA **********************************************
*
*   NAME
*	CachePreDMA - Take actions prior to hardware DMA  (V37)
*
*   SYNOPSIS
*	paddress = CachePreDMA(vaddress,&length,flags)
*	d0                     a0       a1      d0
*
*	APTR CachePreDMA(APTR,LONG *,ULONG);
*
*
*   INPUTS
*	address	- Base address to start the action.
*	length	- Pointer to a longword with a length.
*	flags	- Values:
*			DMA_Continue - Indicates this call is to complete
*			a prior request that was broken up.
*
*   RESULTS
*	paddress- Physical address that coresponds to the input virtual
*		  address.
*	&length	- This length value will be updated to reflect the contiguous
*		  length of physical memory present at paddress.  This may
*		  be smaller than the requested length.  To get the mapping
*		  for the next chunk of memory, call the function again with
*		  a new address, length, and the DMA_Continue flag.
*
******************************************************************************
*
* Replace CachePreDMA to handle the 68040 CopyBack vs DMA problem...
*
NewCachePreDMA:
		btst.l	#DMAB_Continue,d0	; Check if we are continue mode
		bne.s	ncp_Continue		; Skip the Continue case...
		btst.l	#DMAB_ReadFromRAM,d0	; Check if READ DMA
		bne.s	ncp_Continue		; Skip if read...
		move.l	a0,d1			; Get address...
		or.l	(a1),d1			; or in length...
		and.b	#$0F,d1			; Check of non-alignment
		beq.s	ncp_Continue		; Don't count if aligned
*
* Now, we check if we can do the MMU trick...
*
		move.l	MMUFrame(pc),d1		; Get MMU frame...
		bne.s	Off_MMU_Way		; If so, do MMU way...
*
		lea	Nest_Count(pc),a1	; Get a1...
		addq.l	#1,(a1)			; Nest this...
ncp_Continue:	move.l	a0,d0			; Get result...
dma_Caches:	move.l	d0,-(sp)		; Save result...
ncp_DoWork:	moveq.l	#0,d0			; Clear bits
		moveq.l	#0,d1			; Clear mask
		bsr.s	NewCacheControl		; Do the cache setting/clear
		move.l	(sp)+,d0		; Restore d0
		rts				; Return...
*
* Ok, so we have an MMU and need to deal with turning off the pages
* given...
*
Off_MMU_Way:	move.l	a0,-(sp)		; Save result
		move.l	a4,-(sp)		; Save a4
		lea	Off_MMU_Page(pc),a4	; Address of Cache OFF code
*
MMU_Way:	move.l	a5,-(sp)		; Save a5
		lea	Do_MMU_Way(pc),a5	; Get addres of code
		CALLSYS	Supervisor		; Do it...
		move.l	(sp)+,a5		; Restore a5
		move.l	(sp)+,a4		; Restore a4
		bra.s	ncp_DoWork		; Return with result on stack
*
*****o* 68040.library/CacheControl *******************************************
*
*   NAME
*	CacheControl - Instruction & data cache control
*
*   SYNOPSIS
*	oldBits = CacheControl(cacheBits,cacheMask)
*	D0                     D0        D1
*
******************************************************************************
*
* This new cache control completely replaces the ROM version.
* There is no reason to support the other chips here and we need to
* support the 3640 external cache...
*
NewCacheControl:	movem.l	d2/d3,-(sp)		; Save...
			and.l	d1,d0		; Destroy irrelevant bits
			not.l	d1		; Change mask to preserve bits
			move.l	a5,a1		; Save a5...
			lea.l	ncc_Sup(pc),a5	; Code that runs in supervisor
			CALLSYS	Supervisor	; Do it...
			move.l	d3,d0		; Set return value...
			movem.l	(sp)+,d2/d3	; Restore...
			rts			; Done...
*
* Some storage for these features...
*
		cnop	0,4			; Long align them...
Base_Cache:	dc.l	0			; Base cache settings...
Nest_Count:	dc.l	0			; Nest count of the cache...
*
*	d0-mask	d1-bits	d2-scratch d3-result
*	a1-Saved a5...
*
ncc_Sup:	or.w	#$0700,SR	;DISABLE
		movec	CACR,d2		; Get cache control register
		and.l	#CACRF_040_ICache!CACRF_040_DCache,d2 ;!BIT ASUMPTIONS!
*				;10987654321098765432109876543210
*				;D000000000000000I000000000000000
		swap	d2	;I000000000000000D000000000000000 CACRF_040
		ror.w	#8,d2	;I00000000000000000000000D0000000 CACRF_040
		rol.l	#1,d2	;00000000000000000000000D0000000I CACRF_040
*
* Add in the "ghost" cache setting...
*
		or.l	Base_Cache(pc),d2	; Base cache mode...
*
* Now, set the burst modes too...  (040 always bursts the cache)
*
		move.l	d2,d3		; Move it over...
		rol.l	#4,d3		; Shift cache info into burst info
		or.l	d3,d2		; Store with the burst bits as needed
*
* Mirror the Data Cache into the CopyBack bit...
*
		btst.l	#CACRB_EnableD,d2
		beq.s	ncc_NoCB	; If no data cache, no copyback...
		bset.l	#CACRB_CopyBack,d2
ncc_NoCB:	move.l	d2,d3		; Set result: old cache settings
*
* Now, mask out what we want to change and change it...
*
		and.l	d1,d2		; Mask out what we want to change...
		or.l	d0,d2		; Change those...
*
* Now store the "asked for" new setting in Base_Cache...
*
		move.l	#CACRF_EnableD,d0	; Get data cache...
		and.l	d2,d0			; Mask it...
		move.l	d0,Base_Cache-ncc_Sup(a5)	; Store it...
*
* Now, check if data cache should be off due to DMA...
*
		tst.l	Nest_Count(pc)		; Check for PreDMA nest
		beq.s	ncc_Normal		; If not, we just do it...
		bclr.l	#CACRB_EnableD,d2	; If set, we don't do DCache
ncc_Normal:
*
* Now, take the 68030 settings and go back to 68040 settings...
*
*				;10987654321098765432109876543210
*				;XXXXXXXXXXXXXXXXXXXXXXXDXXXXXXXI
		ror.l	#1,d2	;IXXXXXXXXXXXXXXXXXXXXXXXDXXXXXXX CACRF_040
		rol.w	#8,d2	;IXXXXXXXXXXXXXXXDXXXXXXXXXXXXXXX CACRF_040
		swap	d2	;DXXXXXXXXXXXXXXXIXXXXXXXXXXXXXXX CACRF_040
		and.l	#CACRF_040_ICache!CACRF_040_DCache,d2 ;!BIT ASUMPTIONS!
*
* All we need to do is play with the internal cache settings...
*
ncc_NoECache:	nop			;68040 BUG KLUDGE. Mask 14D43B
		cpusha	BC		; Push data and instruction cache...
		nop			;68040 BUG KLUDGE. Mask 14D43B
		movec	d2,CACR		; Set the new cache control reg bits
		nop			;68040 BUG KLUDGE. Mask 14D43B
		move.l	a1,a5		; Restore a5...
		rte			;rte restores SR
*
******************************************************************************
*
* The magic for MMU based Pre/PostDMA calls...
*
* This routine is the general page manager.  It will deal with the
* start and end pages as needed.
* Input:	a4 - Routine to manipulate the page
*		d1 - MMU Frame
*		a0 - Start address
*		*a1- Size
*		a5 - Scrap...
*		d0 - SCrap...
*		a6 - ExecBase
*
Do_MMU_Way:	move.l	d1,a5			; Get MMU Frame into a5...
		move.l	a0,d0			; Get start address...
		move.l	d0,-(sp)		; Save start address...
		add.l	(a1),d0			; Calculate end address...
		bsr.s	Do_MMU_d0		; d0 is address; do it...
		move.l	(sp)+,d0		; Get start again...
		bsr.s	Do_MMU_d0		; d0 is address; do it...
		rte				; We be done...
*
* Ok, so now we are called as follows:
*
*	a6 - ExecBase
*	a5 - MMU Frame pointer
*	a4 - Routine to manipulate the page
*	d0 - Address which needs protection
*	d1 - Scrap
*	a0 - Scrap
*	a1 - Scrap
*
*	a0/a1/d0/d1 may all be trashed :-)
*
Do_MMU_d0:	moveq.l	#$0F,d1			; Mask...
		and.l	d0,d1			; Check for cache line address
		beq.s	Do_MMU_RTS		; If on line address, no-op.
*
		move.l	d0,-(sp)		; Save address...
		bfextu	d0{1:19},d0		; Get page number
		move.l	mmu_NestCounts(a5),d1	; Point at list head
dmd_Loop:	move.l	d1,a0			; Get into address register
		move.l	(a0),d1			; Get Next pointer
		beq.s	dmd_NoFind		; Did not find it...
		cmp.l	nc_Low(a0),d0		; Are we above low?
		bcs.s	dmd_Loop		; Not this one...
		cmp.l	nc_High(a0),d0		; Are we below limit?
		bhi.s	dmd_Loop		; Not this one...
		sub.l	nc_Low(a0),d0		; Subtract low...
		lea 	nc_Count(a0),a1		; Point at start of space
		add.l	d0,a1			; Adjust for page offset
		add.l	d0,a1			; (*2 since they are words)
		move.l	(sp)+,d0		; Restore address...
*
		movec.l	urp,a0			; Get ROOT pointer...
		bfextu	d0{0:7},d1		; Get the root index...
		asl.l	#2,d1			; *4
		add.l	d1,a0			; Add to root pointer...
		move.l	(a0),d1			; Get page entry
		and.l	#$FFFFFE00,d1		; Mask into the page table
		move.l	d1,a0			; Store pointer...
		bfextu	d0{7:7},d1		; Get the pointer index...
		asl.l	#2,d1			; *4
		add.l	d1,a0			; Add to table pointer...
		move.l	(a0),d1			; Get page entry...
		and.l	#$FFFFFF00,d1		; Mask to the pointer...
		move.l	d1,a0			; Put into address register...
		bfextu	d0{14:6},d1		; Get index into page table
		asl.l	#2,d1			; *4
		add.l	d1,a0			; a0 now points at the page...
		move.l	(a0),d1			; Get page entry...
		btst.l	#0,d1			; Check if bit 0 is set...
		bne.s	dmd_skip		; If set, we are valid...
		bclr.l	#1,d1			; Check if indirect...
		beq.s	dmd_skip		; If not indirect, A0 is valid
		move.l	d1,a0			; a0 is now the page entry...
dmd_skip:	jmp	(a4)			; Ok, so now do the page work
*
dmd_NoFind:	move.l	(sp)+,d0		; Restore d0...
Do_MMU_RTS:	rts				; Done...
*
* At this point we are being called as follows:
*	a0 - Points to the page entry in the MMU table for the address
*	a1 - Points at the WORD size nest count for this page in the MMU
*	d0 - Scrap
*	d1 - Scrap
*	STACK - Ready to RTS...
*
Off_MMU_Page:	move.w	(a1),d0			; Get the count...
		addq.w	#1,(a1)			; Bump the count...
		tst.w	d0			; Are we 0?
		bne.s	Do_MMU_RTS		; If not, we already are nested
		addq.l	#3,a0			; Point at last byte of long
		cpusha	dc			; Push the data cache before ATC
		pflusha				; Flush the ATC...
		bclr.b	#5,(a0)			; Clear the copyback bit...
		cpushl	dc,(a0)			; Push the cache...
		rts
*
* This routine is called just like Off_MMU_Page is...
*
On_MMU_Page:	subq.w	#1,(a1)			; Drop count...
		move.w	(a1),d0			; Get count...
		bne.s	Do_MMU_RTS		; If not 0, still nested...
		addq.l	#3,a0			; Point at last byte of long
		pflusha				; Flush the ATC...
		bset.b	#5,(a0)			; Set the copyback bit...
		cpushl	dc,(a0)			; Push the cache...
		rts
*
*******************************************************************************
*
	IFNE	FPUCODE
*
*	skeleton.sa 3.2 12/10/90
*
*	Each entry point for exception 'xxxx' begins with a 'jmp fpsp_xxxx'.
*	Put any target system specific handling that must be done immediately
*	before the jump instruction.  If there no handling necessary, then
*	the 'fpsp_xxxx' handler entry point should be placed in the exception
*	table so that the 'jmp' can be eliminated. If the FPSP determines that the
*	exception is one that must be reported then there will be a
*	return from the package by a 'jmp real_xxxx'.  At that point
*	the machine state will be identical to the state before
*	the FPSP was entered.  In particular, whatever condition
*	that caused the exception will still be pending when the FPSP
*	package returns.  Thus, there will be system specific code
*	to handle the exception.
*
*	If the exception was completely handled by the package, then
*	the return will be via a 'jmp fpsp_done'.  Unless there is
*	OS specific work to be done (such as handling a context switch or
*	interrupt) the user program can be resumed via 'rte'.
*
*	In the following skeleton code, some typical 'real_xxxx' handling
*	code is shown.  This code may need to be moved to an appropriate
*	place in the target system, or rewritten.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SKELETON	IDNT    2,1 Motorola 040 Floating Point Software Package

	section 8
*
*******************************************************************************
*
	include	"fpsp.i"

	xref	b1238_fix
	xref	fpsp_bsun
	xdef	real_bsun
	xref	fpsp_operr
	xdef	real_operr
	xref	fpsp_snan
	xdef	real_snan
	xref	fpsp_unfl
	xdef	real_unfl
	xref	fpsp_ovfl
	xdef	real_ovfl
	xdef	real_inex
	xdef	inex
	xdef	dz
	xdef	real_dz
*
*	Divide by Zero exception
*
*	All dz exceptions are 'real', hence no fpsp_dz entry point.
*
dz:
real_dz:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E1,E_BYTE(a6)
	frestore	(sp)+
	unlk		a6
	rte
*
*	Inexact exception
*
*	All inexact exceptions are real, but the 'real' handler
*	will probably want to clear the pending exception.
*	The provided code will clear the E3 exception (if pending),
*	otherwise clear the E1 exception.  The frestore is not really
*	necessary for E1 exceptions.
*
* Code following the 'inex' label is to handle bug #1232.  In this
* bug, if an E1 snan, ovfl, or unfl occured, and the process was
* swapped out before taking the exception, the exception taken on
* return was inex, rather than the correct exception.  The snan, ovfl,
* and unfl exception to be taken must not have been enabled.  The
* fix is to check for E1, and the existence of one of snan, ovfl,
* or unfl bits set in the fpsr.  If any of these are set, branch
* to the appropriate  handler for the exception in the fpsr.  Note
* that this fix is only for d43b parts, and is skipped if the
* version number is not $40.
*
*
inex:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	cmpi.b		#VER_40,(sp)		;test version number
	bne.b		not_fmt40
	fmove.l		fpsr,-(sp)
	btst.b		#E1,E_BYTE(a6)		;test for E1 set
	beq.b		not_b1232
	btst.b		#snan_bit,2(sp) ;test for snan
	beq.s		inex_ckofl
	add.l		#4,sp
	frestore	(sp)+
	unlk		a6
	jmp		fpsp_snan
inex_ckofl:
	btst.b		#ovfl_bit,2(sp) ;test for ovfl
	beq.s		inex_ckufl
	add.l		#4,sp
	frestore	(sp)+
	unlk		a6
	jmp		fpsp_ovfl
inex_ckufl:
	btst.b		#unfl_bit,2(sp) ;test for unfl
	beq.s		not_b1232
	add.l		#4,sp
	frestore	(sp)+
	unlk		a6
	jmp		fpsp_unfl

*
* We do not have the bug 1232 case.  Clean up the stack and call
* real_inex.
*
not_b1232:
	add.l		#4,sp
	frestore	(sp)+
	unlk		a6

real_inex:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
not_fmt40:
	bclr.b		#E3,E_BYTE(a6)		;clear and test E3 flag
	beq.b		inex_cke1
*
* Clear dirty bit on dest resister in the frame before branching
* to b1238_fix.
*
	movem.l		d0/d1,USER_DA(a6)
	bfextu		CMDREG1B(a6){6:3},d0		;get dest reg no
	bclr.b		d0,FPR_DIRTY_BITS(a6)	;clr dest dirty bit
	bsr		b1238_fix		;test for bug1238 case
	movem.l		USER_DA(a6),d0/d1
	bra.b		inex_done
inex_cke1:
	bclr.b		#E1,E_BYTE(a6)
inex_done:
	frestore	(sp)+
	unlk		a6
	rte

*
*	Overflow exception
*
real_ovfl:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E3,E_BYTE(a6)		;clear and test E3 flag
	bne.b		ovfl_done
	bclr.b		#E1,E_BYTE(a6)
ovfl_done:
	frestore	(sp)+
	unlk		a6
	rte

*
*	Underflow exception
*
real_unfl:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E3,E_BYTE(a6)		;clear and test E3 flag
	bne.b		unfl_done
	bclr.b		#E1,E_BYTE(a6)
unfl_done:
	frestore	(sp)+
	unlk		a6
	rte

*
*	Signalling NAN exception
*
real_snan:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E1,E_BYTE(a6)	;snan is always an E1 exception
	frestore	(sp)+
	unlk		a6
	rte

*
*	Operand Error exception
*
real_operr:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E1,E_BYTE(a6)	;operr is always an E1 exception
	frestore	(sp)+
	unlk		a6
	rte

*
*	BSUN exception
*
*	This sample handler simply clears the nan bit in the FPSR.
*
real_bsun:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E1,E_BYTE(a6)	;bsun is always an E1 exception
	fmove.l		FPSR,-(sp)
	bclr.b		#nan_bit,(sp)
	fmove.l		(sp)+,FPSR
	frestore	(sp)+
	unlk		a6
	rte

*
*	F-line exception
*
*	A 'real' F-line exception is one that the FPSP isn't supposed to
*	handle. E.g. an instruction with a co-processor ID that is not 1.
*
*
		xref		fpsp_fline
		xdef		real_fline
old_fline:	dc.l		0
*
* We should call the original F-Line code...
*
real_fline:	move.l		old_fline(pc),-(sp)
		PRINTF		<'We have a real F-Line exception'>
		rts

*
*	Unsupported data type exception
*
	xref	fpsp_unsupp
	xdef	real_unsupp
real_unsupp:
	link		a6,#-LOCAL_SIZE
	fsave		-(sp)
	bclr.b		#E1,E_BYTE(a6)	;unsupp is always an E1 exception
	frestore	(sp)+
	unlk		a6
	rte

*
*	Trace exception
*
	xdef	real_trace
real_trace:
	rte
*
*	fpsp_fmt_error --- exit point for frame format error
*
*	The fpu stack frame does not match the frames existing
*	or planned at the time of this writing.  The fpsp is
*	unable to handle frame sizes not in the following
*	version:size pairs:
*
*	{4060, 4160} - busy frame
*	{4028, 4130} - unimp frame
*	{4000, 4100} - idle frame
*
*	This entry point simply holds an f-line illegal value.
*	Replace this with a call to your kernel panic code or
*	code to handle future revisions of the fpu.
*
	xdef	fpsp_fmt_error
fpsp_fmt_error:	move.l		old_fline(pc),-(sp)
		PRINTF		<'We have a format error!'>
		rts
*
*	fpsp_done --- FPSP exit point
*
*	The exception has been handled by the package and we are ready
*	to return to user mode, but there may be OS specific code
*	to execute before we do.  If there is, do it now.
*
*
	xdef	fpsp_done
fpsp_done:
	rte

*
*	mem_write --- write to user or supervisor address space
*
* Writes to memory while in supervisor mode.  copyout accomplishes
* this via a 'moves' instruction.  copyout is a UNIX SVR3 (and later) function.
* If you don't have copyout, use the local copy of the function below.
*
*	a0 - supervisor source address
*	a1 - user destination address
*	d0 - number of bytes to write (maximum count is 12)
*
* The supervisor source address is guaranteed to point into the supervisor
* stack.  The result is that a UNIX
* process is allowed to sleep as a consequence of a page fault during
* copyout.  The probability of a page fault is exceedingly small because
* the 68040 always reads the destination address and thus the page
* faults should have already been handled.
*
* If the EXC_SR shows that the exception was from supervisor space,
* then just do a dumb (and slow) memory move.  In a UNIX environment
* there shouldn't be any supervisor mode floating point exceptions.
*
*
*	mem_read --- read from user or supervisor address space
*
* Reads from memory while in supervisor mode.  copyin accomplishes
* this via a 'moves' instruction.  copyin is a UNIX SVR3 (and later) function.
* If you don't have copyin, use the local copy of the function below.
*
* The FPSP calls mem_read to read the original F-line instruction in order
* to extract the data register number when the 'Dn' addressing mode is
* used.
*
*Input:
*	a0 - user source address
*	a1 - supervisor destination address
*	d0 - number of bytes to read (maximum count is 12)
*
* Like mem_write, mem_read always reads with a supervisor
* destination address on the supervisor stack.  Also like mem_write,
* the EXC_SR is checked and a simple memory copy is done if reading
* from supervisor space is indicated.
*
*******
* NOTE: For the Amiga system, we have a unified memory space and thus
* there is no need for copyin/copyout functions.  So, all that is needed
* is a simple memory move of the number of bytes asked for.  This is done
* here with the simple dbra loop.  It should make this much faster than
* the UNIX style code...
*
		xdef	mem_write
		xdef	mem_read
mem_rw:		move.b	(a0)+,(a1)+	; Transfer the byte
mem_read:	; Both routines are exactly the same...
mem_write:	dbra	d0,mem_rw	; Loop for the number of bytes
		rts
*
*******************************************************************************
*
* Install ourselves...  (Cribbed from fpsp.asm)
*
trace_vec	equ	$24
fline_vec	equ	$2c

BSUN_VEC	equ	$c0
INEX2_VEC	equ	$c4
DZ_VEC		equ	$c8
UNFL_VEC	equ	$cc
OPERR_VEC	equ	$d0
OVFL_VEC	equ	$d4
SNAN_VEC	equ	$d8
UNSUP_VEC	equ	$dc


start_fpsp:	or.w	#$0700,SR		; DISABLE (restored via RTE)
*
		movec.l	VBR,a1
		lea	NewVectors(pc),a0	; Get pointer to new
*
		PRINTF	<'Old VBR=%lx   New VBR=%lx',10>,a1,a0
*
		move.w	#NUM_VECS-1,d0		; Number to transfer...
copy_vbr:	move.l	(a1)+,(a0)+
		dbra	d0,copy_vbr		; Copy the old table
		lea	NewVectors(pc),a0	; Get pointer again...
		movec.l	a0,VBR			; Set the new VBR...
*
		PRINTF	<'Old FLINE_VEC: %8lx'>,fline_vec(a0)
		PRINTF	<'Old INEX2_VEC: %8lx'>,INEX2_VEC(a0)
		PRINTF	<'Old DZ_VEC:    %8lx'>,DZ_VEC(a0)
		PRINTF	<'Old BSUN_VEC:  %8lx'>,BSUN_VEC(a0)
		PRINTF	<'Old UNFL_VEC:  %8lx'>,UNFL_VEC(a0)
		PRINTF	<'Old OPERR_VEC: %8lx'>,OPERR_VEC(a0)
		PRINTF	<'Old OVFL_VEC:  %8lx'>,OVFL_VEC(a0)
		PRINTF	<'Old SNAN_VEC:  %8lx'>,SNAN_VEC(a0)
		PRINTF	<'Old UNSUP_VEC: %8lx'>,UNSUP_VEC(a0)
*
* Set up the new F-Line exception and make sure it still can call the old one
*
		move.l	fline_vec(a0),old_fline
		move.l	#fpsp_fline,fline_vec(a0)
*
* The following are 100% local
*
		move.l	#inex,INEX2_VEC(a0)
		move.l	#dz,DZ_VEC(a0)
*
* These are fpsp based...
*
		move.l	#fpsp_bsun,BSUN_VEC(a0)
		move.l	#fpsp_unfl,UNFL_VEC(a0)
		move.l	#fpsp_operr,OPERR_VEC(a0)
		move.l	#fpsp_ovfl,OVFL_VEC(a0)
		move.l	#fpsp_snan,SNAN_VEC(a0)
		move.l	#fpsp_unsupp,UNSUP_VEC(a0)
*
		PRINTF	<'New FLINE_VEC: %8lx'>,fline_vec(a0)
		PRINTF	<'New INEX2_VEC: %8lx'>,INEX2_VEC(a0)
		PRINTF	<'New DZ_VEC:    %8lx'>,DZ_VEC(a0)
		PRINTF	<'New BSUN_VEC:  %8lx'>,BSUN_VEC(a0)
		PRINTF	<'New UNFL_VEC:  %8lx'>,UNFL_VEC(a0)
		PRINTF	<'New OPERR_VEC: %8lx'>,OPERR_VEC(a0)
		PRINTF	<'New OVFL_VEC:  %8lx'>,OVFL_VEC(a0)
		PRINTF	<'New SNAN_VEC:  %8lx'>,SNAN_VEC(a0)
		PRINTF	<'New UNSUP_VEC: %8lx'>,UNSUP_VEC(a0)
*
		PRINTF	<'Installed 68040 FPU vectors  (V%ld.%ld)'>,#VERSION,#REVISION
*
* Now, push back the data cache to make sure we are installed...
*
		cpusha	DC	; Push back the data cache...
*
		rte	; Return from the Supervisor function...
*
	ENDC
*******************************************************************************
*
		END
