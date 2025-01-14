*******************************************************************************
*
* $Id: rekick.asm,v 39.4 92/06/11 10:39:09 mks Exp $
*
* $Log:	rekick.asm,v $
* Revision 39.4  92/06/11  10:39:09  mks
* Fixed bug in that D2 was not NULL...
* 
* Revision 39.3  92/06/08  18:39:53  mks
* Added command line file name option and ability to run from Workbench
*
* Revision 39.2  92/06/08  14:59:02  mks
* Added autodoc and changed the ReKick text a bit...
*
* Revision 39.1  92/06/07  15:16:26  mks
* Initial release
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
* Method of operation:  (slimy, sick, ugly, etc.)
*
* In order to work, Rekick contains some code that basically does the
* very early bootup work of Exec and Expansion.  (RTF_SINGLETASK)
* After this is completed, control is passed to the loaded kickstart
* which has been specially modified to work with ReKick.
*
* The kickstart itself will contain at the start of it a branch to the
* code that continues after ReKick finished its work.  Note that due to
* the way ReKick must work, there is no way to make it deal with standard
* kickstart files.  (Sorry, but this is major voodoo)
*
* -- Mike ("my brain hurts due to ReKick") Sinz
*
******* ReKick ****************************************************************
*
*   NAME
*	ReKick
*
*   SYNOPSIS
*	ReKick - A tool to run special test Kickstarts on A2000/A500 machines
*
*   FUNCTION
*	ReKick is a tool for A2000 and A500 systems with at least 1Meg of
*	AutoConfig RAM at $200000.  It, along with the special kickstart
*	files designed for ReKick, will let developers and beta testers of
*	our new kickstarts boot into the kickstart.
*
*	ReKick does *NOT* require that the autoconfig memory be the first
*	board in the system and thus will work with systems that have
*	A2091 with RAM or A590 with RAM.  (Since the drive part configured
*	before the RAM did...)  Also, ReKick will fully configure the boards
*	using the new kickstart.  This means that there are no limitations
*	to the type of boards that the system should correctly understand
*	with ReKick vs ROM kickstarts.
*
*	ReKick works by first checking to see if the system has memory
*	to support it.  It will then check for a file in DEVS:Kickstart.
*	This file must be of the correct format and it must pass certain
*	consistancy checks.  After the file has been loaded and checked,
*	ReKick will print a notice and give you 3 seconds to abort the
*	reboot.  If you do not reboot, you will be booted into the normal
*	boot process of the new, beta kickstart.
*
*	Since ReKick will quietly fail if there is no DEVS:Kickstart or
*	if you have already been ReKick'ed, it is very easy to just have
*	ReKick as the first line of the startup-sequence.
*
*	ReKick takes a single command line parameter that is parsed as
*	the filename of the kickstart.  If no command line parameter is
*	given, ReKick will use DEVS:Kickstart as the file name.
*
*	ReKick can also be started from Workbench.  When started from
*	Workbench, it will take any command line parameters and it will
*	not give you a chance to abort it.
*
*   INPUTS
*	A valid kickstart file in DEVS:Kickstart or in the file name
*	given at the command line.
*
*   RESULTS
*	A system running the beta Kickstart
*
*   NOTE
*	Due to the fact that you are not really in ROM with the kickstart,
*	and since this trick does not use the MMU (it works on 68000 machines)
*	the ROM is *not* write protected.  Writes to ROM may well crash the
*	system.
*
*	Due to the methods required for BootMenu to operate, BootMenu may
*	not be possible with ReKick'ed machines.
*
*	Note that the command line parameter is not parsed but is assumed
*	to be a complete file name.  This means that if you wish to have
*	the file "test disk:kickstart file" used you would use:
*
*	      ReKick test disk:kickstart file
*
*	No quotes are required since it only takes the one argument.
*
*******************************************************************************
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"hardware/cia.i"
	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/dmabits.i"
	INCLUDE	"hardware/intbits.i"
	INCLUDE	"dos/dos.i"
	INCLUDE	"dos/dosextens.i"

	INCLUDE	"rekick_rev.i"
*
	XREF	_intena
	XREF	_ciaaddra
	XREF	_custom
	XREF	_color
	XREF	EndOfMem
	XREF	AllocAbsInternal
	XREF	MagicConfig
*
*******************************************************************************
*
* Since this code does all sorts of magic including a RESET instruction,
* we need to make sure we are in memory that will not go away.  This
* turns out to be CHIP memory on all current systems.  (MEMF_LOCAL under
* V37 and up, but that can not be counted on...)
*
	SECTION	ReKick,CODE,CHIP
*
*******************************************************************************
*
FILE_ID		equ	$414D4947	; 'AMIG'
KICK_ID		equ	$0000FFFF	; ROM ID...
KICK_SIZE	equ	$00080000	; 512K
KICK_START	equ	$00200000	; Check for it...
KICK_KEY	equ	$DEADFEED	; Key for first X-OR...
START_EXEC	equ	$00200002	; Address of jump instruction...
*
*******************************************************************************
*
* Now for the startup code.  It will read the file and check if it is the
* right type and then get the version/revision from it...
*
* Register usage:
*
*	d7	The file handle of the kickstart file....
*	d6	Output file handle
*	a6	ExecBase
*	a5	DOSBase
*	a4	Loaded version
*	a3	Pointer at file name...
*
ReKickStart:	move.l	4,a6		; Get ExecBase...
		lea	kickName(pc),a3	; Get default name of file to load...
		move.l	ThisTask(a6),a4	; Get our process pointer...
		tst.l	pr_CLI(a4)	; Are we a CLI?
		beq.s	ReKick_WB	; If NULL, we are a WB run program...
		clr.l	-(sp)		; No workbench startup message...
*
* Now handle the arguments...
*
spaceSkip:	cmp.b	#' ',(a0)+	; Check for spaces
		beq.s	spaceSkip	; Continue...
		subq.l	#1,a0		; Set to the first character
		move.l	a0,a1		; Store it in a1...
findEnd:	move.b	(a0)+,d0	; Get next byte...
		beq.s	foundEnd	; If NULL, we found the end...
		cmp.b	#10,d0		; If not LF, we continue...
		bne.s	findEnd		; Loop...
stripEnd:	clr.b	-1(a0)		; Clear the LF to a NULL...
foundEnd:	subq.l	#1,a0		; Back up 1...
		cmp.l	a0,a1		; Are we the same?
		beq.s	CheckMem	; If the same, no argument...
		cmp.b	#' ',-1(a0)	; Is the previous byte a space?
		beq.s	stripEnd	; Stip it if it is...
		move.l	a1,a3		; We have an argument so, use it as...
		bra.s	CheckMem	; ...the kickstart file name...
*
* Handle Workbench Startup...
*
ReKick_WB:	lea	pr_MsgPort(a4),a4	; Get message port...
		move.l	a4,a0		; We first wait for the WB Startup MSG
		JSRLIB	WaitPort	; (it should be here very quickly)
		move.l	a4,a0		; Now we get the message...
		JSRLIB	GetMsg		; Get it...
		move.l	d0,-(sp)	; Now, store it on the stack...
*
* First, check if we have memory at the place where we
* want to load the kickstart.  If not, we exit since either
* the kickstart was loaded or we don't have the memory to load
* it...
*
CheckMem:	move.l	#KICK_START+KICK_SIZE-4,a1	; Get pointer to end
		JSRLIB	TypeOfMem	; Check the memory
		tst.l	d0		; Check for valid value...
		beq.s	exit		; If NULL, we have no memory...
*
* Ok, so we have the memory, now check for doing the kick...
*
		lea	dosName(pc),a1	; Get name of DOS.library
		moveq.l	#0,d0		; Any version...
		JSRLIB	OpenLibrary	; Open it...
		tst.l	d0		; Did it work?
		bne.s	dosOpen		; If so, it is open...
exit:		move.l	(sp)+,d0	; Get startup-message
		beq.s	cli_Exit	; If none, we exit CLI...
		move.l	d0,a1		; Get ready to reply the message...
		JMPLIB	ReplyMsg	; Reply and exit (simple)
cli_Exit:	rts			; Exit (quietly...)
**
dosOpen:	move.l	d0,a5		; a5 will store DOSBase...
		move.l	a3,d1		; Get file name read for DOS Open()
		move.l	#MODE_OLDFILE,d2 ;Only if it exists...
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Open		; Open the file...
		exg	a5,a6		; Restore DOS/Exec bases...
		move.l	d0,d7		; Store file handle
		bne.s	gotFile		; If we got the file, continue...
exit1:		move.l	a5,a1		; Get dos.library
		JSRLIB	CloseLibrary	; Close it...
		bra.s	exit		; final exit...
**
gotFile:	move.l	d7,d1		; Get file handle...
		lea	fileID(pc),a0	; Get buffer...
		move.l	a0,d2		; (DOS uses d2)
		move.l	#108,d3		; 108 bytes...  (1 long word)
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Read		; Read the file ID
		exg	a5,a6		; Swap DOS/Exec bases...
		sub.l	d3,d0		; Check if it worked...
		beq.s	readID		; If 108 bytes, ID read OK
exit2:		move.l	d7,d1		; Get file handle
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Close		; Close the file
		exg	a5,a6		; Swap DOS/Exec bases...
		bra.s	exit1		; continue the exit path...
**
readID:		move.l	fileID(pc),d0	; Get the file's ID
		cmp.l	#FILE_ID,d0	; It must be of this file type
		bne.s	exit2		; If not, just exit...
*
* Ok, so we have a file that looks good and is a different version.
* Now let us try to load it...
*
		move.l	#KICK_SIZE,d0	; Size of memory needed...
		move.l	#MEMF_PUBLIC,d1	; PUBLIC memory...
		JSRLIB	AllocMem	; Allocate the memory...
		move.l	d0,a4		; Save the allocation...
		tst.l	d0		; Check it...
		beq.s	exit2		; No memory, so exit...
*
		move.l	d7,d1		; Get file handle
		move.l	a4,d2		; Get buffer...
		move.l	#KICK_SIZE,d3	; Get kickstart size
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Read		; Read some of the file...
		exg	a5,a6		; Swap DOS/Exec bases...
		cmp.l	#KICK_SIZE,d0	; Did it read right?
		beq.s	kickRead	; If so, continue...
exit3:		move.l	a4,a1		; Get memory block...
		move.l	#KICK_SIZE,d0	; Get size
		JSRLIB	FreeMem		; Free the memory
		bra.s	exit2		; Do the rest of the exit...
*
* We now decode the kickstart.  The reason we have the kickstart
* encoded is such that it is a bit harder to hack at...  (Not much, but some)
*
kickRead:	move.l	a4,a1		; Now, decode the kickstart...
		move.l	#(KICK_SIZE/16)-1,d2	; Number of quadlong words...
		move.l	#KICK_KEY,d1	; Starting key...
kick_Decode:	move.l	(a1),d0		; Get original value...
		eor.l	d0,d1		; X-OR it with key...
		move.l	d1,(a1)+	; Save fixed value out...
		move.l	(a1),d1		; Get original value...
		eor.l	d1,d0		; X-OR it with key...
		move.l	d0,(a1)+	; Save fixed value out...
		move.l	(a1),d0		; Get original value...
		eor.l	d0,d1		; X-OR it with key...
		move.l	d1,(a1)+	; Save fixed value out...
		move.l	(a1),d1		; Get original value...
		eor.l	d1,d0		; X-OR it with key...
		move.l	d0,(a1)+	; Save fixed value out...
		dbra.s	d2,kick_Decode	; Keep decoding it...
*
* Now, check to see if it is a valid kickstart...
*
		move.l	a4,a0		; Get kickstart again...
		moveq.l	#0,d1		; Used for addx
		moveq.l	#0,d0		; Checksum...
		move.l	#(KICK_SIZE/16)-1,d2	; Number of quadlong words...
kick_sum:	add.l	(a0)+,d0	; Add next long word...
		addx.l	d1,d0		; Add in the overflow...
		add.l	(a0)+,d0	; Add next long word...
		addx.l	d1,d0		; Add in the overflow...
		add.l	(a0)+,d0	; Add next long word...
		addx.l	d1,d0		; Add in the overflow...
		add.l	(a0)+,d0	; Add next long word...
		addx.l	d1,d0		; Add in the overflow...
		dbra.s	d2,kick_sum	; Do the whole thing...
		addq.l	#1,d0		; Did it sum?
		bne.s	exit3		; If not, we exit...
*
* Ok, so the kickstart looks good and seems to have loaded just fine,
* so now let us get this show on the road...
*
* First, we tell the user what is about to happen...
*
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Output		; Get output file handle
		exg	a5,a6		; Swap DOS/Exec bases...
		move.l	d0,d6		; Check if we got one...
		beq.s	doBoot		; If not, do the boot.
		lea	message(pc),a0	; Get message pointer
		move.l	d6,d1		; Get file handle
		move.l	a0,d2		; Get message
		move.l	#message_len,d3	; Length of message
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Write		; Write the message
		move.l	#50*3,d1	; Get delay time...  (3 seconds)
		JSRLIB	Delay		; Wait a bit...
		lea	lf(pc),a0	; Get pointer to LF
		move.l	d6,d1		; Get file handle
		move.l	a0,d2		; Get buffer
		moveq.l	#1,d3		; Get length
		JSRLIB	Write		; Write the linefeed
		exg	a5,a6		; Swap DOS/Exec bases...
		moveq.l	#0,d0		; Clear d0
		moveq.l	#0,d1		; Clear d1 (no signals...)
		JSRLIB	SetSignal	; Check signal...
		btst.l	#SIGBREAKB_CTRL_C,d0	; Check for CTRL-C
		beq.s	doBoot		; If no CTRL-C, boot...
exit4:		lea	break(pc),a0	; Get break string
		move.l	d6,d1		; Get file handle
		move.l	a0,d2		; Get break string
		move.l	#break_len,d3	; Get break string length
		exg	a5,a6		; Swap DOS/Exec bases...
		JSRLIB	Write		; Write the string
		exg	a5,a6		; Swap DOS/Exec bases...
		bra	exit3		; Continue with the exit...
*
* Ok, so we are now going to do the reboot...  I guess things get
* interesting now...  At this point, no more calls to DOS or anything
* else will happen.  We will full Disable() and copy the code into the
* memory as needed and then go and do the magic to make it work...
*
doBoot:		lea	superBoot(pc),a5	; Point at supervisor code
		JSRLIB	Supervisor		; Do the right thing...
superBoot:	bchg.b	#1,_ciaaddra		; Flash power light off...
		moveq.l	#0,d1			; Clear d1...
		move.w	#$7FFF,d0		; Stop all interrupts
		lea	_custom,a3		; Point at hardware...
		move.w	d0,intena(a3)		; (hardware poke...)
		move.w	d0,dmacon(a3)		; Clear DMA too...
		move.w	d1,bpldat(a3)		; No bitplane data
		move.w	d1,color(a3)		; Black screen...
*
* Make sure VBR is back at 0
*
		btst.b	#AFB_68010,AttnFlags+1(a6)	; Check for 68010
		beq.s	skipVBR			; If no VBR, we have no work
		OPT	p=68010			; Turn on 68010 assembly mode
		sub.l	a0,a0			; Clear a0
		movec.l	a0,vbr			; Set VBR to 0...
		btst.b	#AFB_68020,AttnFlags+1(a6)	; Check for >68010
		beq.s	skipVBR
		OPT	p=68020			; Turn on 68020 assembly mode
		move.l	#CACRF_FreezeI!CACRF_ClearI!CACRF_FreezeD!CACRF_ClearD,d1
		movec.l	d1,CACR			; Turn off caches...
skipVBR:	OPT	p=68000			; Turn on 68000 assembly mode
*
* Copy the kickstart...
*
		move.l	#(KICK_SIZE/16)-1,d2	; Number of quadlong words...
		move.l	#KICK_START,a0		; destination of kickstart...
copyLoop:	move.l	(a4)+,(a0)+		; Copy long word 1
		move.l	(a4)+,(a0)+		; Copy long word 2
		move.l	(a4)+,(a0)+		; Copy long word 3
		move.l	(a4)+,(a0)+		; Copy long word 4
		dbra.s	d2,copyLoop		; Continue...
*
* Now, the kickstart is copied, we need to start the system again...
*
*******************************************************************************
*
* Some last moment heroics to get an ExecBase-like structure in MEMF_LOCAL
* memory.  Well, to do this in all systems, it needs to be MEMF_CHIP which
* is this code itself...
* Anyway, we will now build an execbase that is good enough to give us
* the ColdCapture vector as working...
*
		lea	FakeExecBase(pc),a4	; Get fake execbase
		lea	RebootContinue(pc),a0	; Get ColdCapture code
		move.l	a0,ColdCapture(a4)	; Store in my fake ExecBase
		move.l	a4,d0			; Get execbase number...
		not.l	d0			; Negative (for magic cookie)
		move.l	d0,ChkBase(a4)		; Make cookie taste good...
*
		moveq.l	#0,d1			; Get ready for exec Checksum
		lea	SoftVer(a4),a0		; Address of checksum area
		move.w	#(ChkSum-2-SoftVer)/2,d0
se_setchk:	add.w	(a0)+,d1		; Do the magic...
		dbf	d0,se_setchk		; ...so ColdCapture will work
		not.w	d1
		move.w	d1,ChkSum(a4)
*
* Now, just cruft it into ABSEXECBASE... (Would not work if enforcer was on)
*
		move.l	a4,4			; Put it into ABSEXECBASE!
		cnop	0,4			; Long-align now...
		lea	$01000000,a0		; End of ROM
		sub.l	-$14(a0),a0		; The PC address...
		move.l	4(a0),a0		; Get initial address
		subq.l	#2,a0			; Back up to the second RESET
		reset				; First RESET
		jmp	(a0)			; Jump to second reset...
*
*******************************************************************************
*
* If all goes well, we will continue here after the ROM noticed the EXECBASE
* and the ColdCapture we put there.  We clear it away, so as not to cause
* future problems...
*
RebootContinue:	moveq.l	#0,d2			; Clear d2...
		move.l	d2,4			; Clear location 4 (to forget)
*
* Now, hit the hardware again, just to make sure all is ok...
*
		move.w	#$0111,_color		; Black screen...
*
* Get size of CHIP memory...
*
		move.l	d2,a3			; Clear a3
		move.l	d2,a0			; Clear a0
		clr.l	(a3)			; Clear location 0
		move.l	#$0F2D4B689,d1		; Get non-NULL value...
		bra.s	enter_here		; Skip into loop...
top_loop:	move.l	d0,(a3)			; Restore...
enter_here:	lea.l	$4000(a3),a3		; Next 16K
		cmp.l	#$200000,a3		; Are we done?
		beq.s	endofmem		; If so, exit...
		move.l	(a3),d0			; Get old value...
		move.l	d1,(a3)			; Put in new value
		nop				; pipeline flush
		cmp.l	(a0),d1			; Shadow?
		beq.s	endofmem		; If so, exit
		cmp.l	(a3),d1			; Is it really there...
		beq.s	top_loop		; Check for more...
endofmem:	move.l	d0,(a3)			; Restore last one...
*
* Now make the CHIP memory list
*
		move.w	#$0400,a0		; Low memory bound...
		lea	message(pc),a1		; Memory name...
		move.l	a3,d0			; End of memory...
		sub.l	a0,d0			; End-start = size
		move.l	#MEMF_24BITDMA!MEMF_LOCAL!MEMF_CHIP!MEMF_PUBLIC,d1
		moveq.l	#-10,d2			; Priority...
*
*******************************************************************************
*
* From memory.asm  (AddMemListInternal)
*
;------ clear out links
	clr.l	(A0)
	clr.l	LN_PRED(A0)

;------ set up node
	move.b	#NT_MEMORY,LN_TYPE(a0)
	move.b	d2,LN_PRI(a0)
	move.l  a1,LN_NAME(a0)

;------ set up memheader
	move.w	d1,MH_ATTRIBUTES(a0)

;------ a0 is the base of the mem header, a1 is the
;------ base of the first mem chunk.  The memheader is carved from
;------ the start of the new memory area.
	lea	MH_SIZE(a0),a1

;------ make sure free block is long aligned
	move.l	a1,d1
	addq.l	#MEM_BLOCKMASK,d1
	and.w	#~MEM_BLOCKMASK,d1
	exg     d1,a1			; a1 is now rounded up

;------ adjust size by the amount we rounded up
	sub.l	a1,d1			; a1 <= d1, so 0 >= d1 > -7
	add.l   d1,d0

;------ and that the size is long aligned
	and.w	#~MEM_BLOCKMASK,d0
	sub.l	#MH_SIZE,d0

;------ finish with the mem headers
	move.l	a1,MH_FIRST(a0)
	move.l	a1,MH_LOWER(a0)
	move.l	a1,d1
	add.l	d0,d1
	move.l	d1,MH_UPPER(a0)
	move.l	d0,MH_FREE(a0)

;------ set up the first mem chunk
	clr.l	(a1)			;MC_NEXT
	move.l	d0,MC_BYTES(a1)
*
*******************************************************************************
*
		moveq.l	#0,d2			; Clear d2...
*
* We now have a real memory list, so we can allocate from it...
*
alloc_start:	lea	alloc_start(pc),a1	; Get start
		moveq.l	#$FFFFFFF8,d1		; Get mask...
		move.l	a1,d0			; Get into d0
		and.l	d0,d1			; Mask it...
		move.l	d1,a1			; Store it in a1...
		lea	EndOfMem(pc),a0		; End of memory...
		neg.l	d1			; Make d0 negative
		add.l	a0,d1			; Now we have the size in d1
		moveq.l	#7,d0			; Get 7...
		add.l	d0,d1			; Add 7...
		not.l	d0			; Invert bits...
		and.l	d0,d1			; Mask to multiple of 8...
		movem.l	d1/a1,-(sp)		; Save it on the stack...
		move.w	#$0400,a0		; Get memory header...
*
* AllocAbsInternal:
*
	MOVEM.L D2-D4/A2-A3,-(SP)
	MOVE.L	D1,D0

	MOVE.L  A1,A3		;startwant
	MOVE.L  A1,D2
	ADD.L   D0,D2		;loc+size=endwant

	LEA	MH_FIRST(A0),A2	;prevchunk

	;------ scan list until chunk+chunklen >= endwant
aa1:
	MOVE.L  (A2),D3
	MOVE.L  D3,A1
	MOVE.L  MC_BYTES(A1),D4
	ADD.L   D3,D4
	CMP.L   D2,D4
	BCC.S   aa_found	;BHS
	MOVE.L  A1,A2	;------ bump next pointer
	BRA.S   aa1

	;------ Found it!
aa_found:
	;------ we found the right region.  update the free mem count
	SUB.L   D0,MH_FREE(A0)

	;------ see if we need to split the end of the chunk
	SUB.L   D2,D4		;endcurrent-endwant
	BNE.S   aa_splitsucc

	;------ the ends are the same.  Get the next ptr.
	MOVE.L  (A1),A0
	BRA.S   aa_checkhead

aa_splitsucc:
	;------ get the address of the new node
	LEA     0(A3,D0.L),A0	;just past end of new block
	MOVE.L  (A1),(A0)	;(startcurrent),(newblock)
	MOVE.L  A0,(A1)		;newblock,(startcurrent)
	MOVE.L  D4,MC_BYTES(A0)	;difference is newsize

aa_checkhead:
	CMP.L   A3,D3		;startwant vs. startcurrent
	BEQ.S   aa_removenode

	SUB.L   A3,D3
	NEG.L   D3
	MOVE.L  D3,MC_BYTES(A1)
	BRA.S   aa_done

aa_removenode:
	;------ update the previous pointer
	MOVE.L  A0,(A2)

aa_done:
	MOVE.L  A3,D0
aa_end:
	MOVEM.L (SP)+,D2-D4/A2-A3
*
*******************************************************************************
*
* At this point we are now in the memory list so things should not trash us.
* We now will do some magic to config the boards as needed...
*
		bsr	MagicConfig		; Do the magic config...
*
*******************************************************************************
*
* Now, plug in the memory address for later FreeVec calls...
*
		movem.l	(sp)+,d1/a1		; Get allocation address/size
		move.l	d1,(a1)+		; Store size (for FreeVec())
		move.l	a1,$280000-8		; Second to last long (magic)
		jmp	START_EXEC		; Start it up...
*
*******************************************************************************
*
* The special fake ExecBase goes here...
*
FakeExecBase:	ds.b	LIB_SIZE
fake_SoftVer:	ds.w	1	; kickstart release number (obs.)
		ds.w	1	;LowMemChkSum	; checksum of 68000 trap vectors
fake_ChkBase:	ds.l	1	;ChkBase 	; system base pointer complement
fake_Cold:	dc.l	1	;ColdCapture	; coldstart soft capture vector
		ds.l	9	; Rest of FakeExecBase
fake_ChkSum:	ds.w	1	;ChkSum		; for all of the above (minus 2)
*
*******************************************************************************
*
* Some strings go here...
*
fileID:		ds.l	27		; We read header into here...
dosName:	dc.b	'dos.library',0
kickName:	dc.b	'DEVS:Kickstart',0
message:	VSTRING
		dc.b	'ReKick loaded and validated the BETA kickstart.',10
		dc.b	'ReKick will now reboot in 3 seconds.',10
		dc.b	'Press ctrl-C to abort.',10
break:		dc.b	'*** Break',10
lf:		dc.b	10
		VERSTAG
*
message_len	equ	break-message
break_len	equ	lf-break
*
*******************************************************************************
*
		END
