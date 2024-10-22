*******************************************************************************
*
* $Id: sad.asm,v 39.11 93/02/26 14:00:58 mks Exp $
*
* $Log:	sad.asm,v $
* Revision 39.11  93/02/26  14:00:58  mks
* Fixed SAD and added documentation about the "broken" SAD
* 
* Revision 39.6  92/07/13  17:04:18  mks
* Forgot to remove one line...
*
* Revision 39.5  92/07/13  17:00:26  mks
* Now always has the NMI pointing to RTE until someone wants to
* move it to SAD...
*
* Revision 39.4  92/06/08  16:14:13  mks
* Added conditional code for the A2000 class builds to make the NMI
* interrupt be as quick of a NOP as possible...
*
* Revision 39.3  92/06/07  16:03:01  mks
* Conditional code to remove NMI-SAD from the REKICK builds
*
* Revision 39.2  92/04/13  11:30:29  mks
* Removed the last of the hang-on table
*
* Revision 39.1  92/04/09  03:18:26  mks
* Changed method of timeout and saved some ROM space...
*
* Revision 39.0  92/04/06  07:20:30  mks
* First, mainly untested release.
*
*******************************************************************************
*
******* SAD/--Overview-- ******************************************************
*
*	Simple Amiga Debugging Kernel, known as "SAD"
*	It is in EXEC starting in V39
*
*
*		-- General description --
*
* The Simple Amiga Debugging Kernel (SAD) is a set of very simple control
* routines stored in the Kickstart ROM that would let debuggers control the
* Amiga's development enviroment from the outside.  These tools would make
* it possible to do remote machine development/debugging via just the
* on-board serial port.
*
* This set of control routines is very simple and yet completely flexible,
* thus making it possible to control the whole machine.
*
*
*		-- Technical Issues --
*
* SAD will make use of the motherboard serial port that exists in all
* Amiga systems.  The connection via the serial port lets the system be
* able to execute SAD without needing any of the system software up and
* running. (SAD will play with the serial port directly)
*
* With some minor changes to the Amiga hardware, an NMI-like line could
* be hooked up to a pin on the serial port.  This would let external
* control of the machine and would let the external controller stop the
* machine no matter what state it is in.  (NMI is that way)
*
* In order to function correctly, SAD requires the some of the EXEC
* CPU control functions work and that ExecBase be valid.  Beyond that,
* SAD does not require the OS to be running.
*
*
*		-- Command Overview --
*
* The basic commands needed to operate SAD are as follows:
*
* Read and Write memory as byte, word, and longword.
* Get the register frame address (contains all registers)
* JSR to Address
* Return to system operation  (return from interrupt)
*
* These basic routines will let the system do whatever is needed.
* Since the JSR to address and memory read/write routines can be used
* to download small sections of code that could be used to do more
* complex things, this basic command set is thus flexible enough
* to even replace itself.
*
* Caches will automatically be flushed as needed after each write.
* (A call to CacheClearU() will be made after the write and before
* the command done sequence)
*
*
*		-- Technical Command Descriptions --
*
* Since the communications with SAD is via a serial port, data formats
* have been defined for minimum overhead while still giving reasonable data
* reliability.  SAD will use the serial port at default 9600 baud but the
* external tools can change the serial port's data rate if it wishes.  It
* would need to make sure that it will be able to reconnect.  SAD sets
* the baud rate to 9600 each time it is entered.  However, while within
* SAD, a simple command to write a WORD to the SERPER register would
* change the baud rate.  This will remain in effect until you exit and
* re-enter SAD or until you change the register again.  (This can be usefull
* if you need to transfer a large amount of data)
*
* All commands have a basic format that they will follow.  All commands have
* both an ACK and a completion message.
*
* Basic command format is:
*
* SENDER:	$AF <command byte> [<data bytes as needed by command>]
*
* Receive:
* Command ACK:  $00 <command byte>
*
* Command Done: $1F <command byte> [<data if needed>]
*
* Waiting: $53 $41 $44 $BF
*
* Waiting when called from Debug():	$53 $41 $44 $3F
*
* Waiting when in dead-end crash:	$53 $41 $44 $21
*
* The data sequence will be that SAD will emit a $BF and then wait for a
* command. If no command is received within <2> seconds, it will emit $BF
* again and loop back.  (This is the "heart beat" of SAD)  When called from
* Debug() and not the NMI hook, SAD will use $3F as the "heart beat"
*
* If SAD does not get a responce after <10> heartbeats, it will return to
* the system.  (Execute an RTS or RTE as needed)  This is to prevent a full
* hang.  The debugger at the other end can keep SAD happy by sending a
* NO-OP command.
*
* All I/O in SAD times out.  During the transmition of a command, if
* more than 2 seconds pass between bytes of data SAD will time out
* and return to the prompt.  This is mainly to help make sure that
* SAD can never get into an i-loop situation.
*
*
*		-- Data Structure Issues --
*
* While executing in SAD, you may have full access to machine from the CPU
* standpoint.  However, this could also be a problem.  It is important to
* understand that when entered via NMI that many system lists may be in
* unstable state.  (NMI can happen in the middle of the AllocMem routine
* or task switch, etc)
*
* Also, since you are doing debugging, it is up to you to determin what
* operations can be done and what can not be done.  A good example is
* that if you want to write a WORD or LONG that the address will need to
* be even on 68000 processors.  Also, if you read or write memory that does
* not exist, you may get a bus error.  Following system structures may
* require that you check the pointers at each step.
*
* When entered via Debug(), you are now running as a "task" so you will
* be able to assume some things about system structures.  This means that
* you are in supervisor state and that you can assume that the
* system is at least not between states.  However, remember that since
* you are debugging the system, some bad code could cause data structures
* to be invalid.  Again, standard debugging issues are in play.  SAD just
* gives you the hooks to do whatever you need.
*
* Note:  When SAD prompts with $BF you will be in full disable/forbid
* state.  When $3F prompting, SAD will only do a Forbid().  It is possible
* for you to then disable interrupts as needed.  This is done such that it
* is possible to "run" the system from SAD when called with Debug().
*
*
*		-- Data Frames and the Registers --
*
* SAD generates a special data frame that can be used to read what
* registers contain and to change the contents of the registers.
* See the entry for GET_CONTEXT_FRAME for more details
*
* -----------------------------------------------------------------------------
*
* BUGS
*	In V39 EXEC, the WRITE_BYTE command was not connected and this
*	caused all of the command numbers to be off-by-one.  For example,
*	the READ_WORD command is listed as command $05 but in V39 is $04.
*	However, the ACK of the commands are still correct.
*
*	Also, in V39 EXEC, the READ_WORD command would return the wrong
*	data.
*
*	To determin if you are in V39 or V40 SAD, you can issue a simple
*	SAD command at the start of the session.  By sending a READ_WORD
*	command, you may either get a READ_WORD (V40) or a READ_LONG (V39)
*	ACK'ed back.  So the data stream for a safe test would be:
*
*	Send: $AF $05 $00 $F8 $00 $00               ; Read start of ROM...
*	Recv: $00 $05 ....   You have V40 SAD
*	Recv: $00 $06 ....   You have V39 SAD
*
*	Note that you should be ready to read either 2 or 4 bytes of
*	result depending on the ACK sent by the system.
*
*******************************************************************************
*

	include	"assembly.i"
	include	"types.i"
	include	"execbase.i"
	include	"calls.i"

	include	"hardware/intbits.i"

	XREF	_intreq
	XREF	_serper
	XREF	_serdat
	XREF	_serdatr
	XREF	_intenar
	XREF	_intena

	XREF	CrashReset

	XREF	_LVOAllocVec
	XREF	_LVOFreeVec
	XREF	_LVOCacheClearU
	XREF	_LVODebug
	XREF	_LVOSetFunction
	XREF	_LVOSupervisor
	XREF	_LVOColdReboot
*
*******************************************************************************
*
* The SAD prompt longwords...
*
SADPROMPT_NMI:	equ	$534144BF
SADPROMPT:	equ	$5341443F
SADPROMPT_DEAD:	equ	$53414421
*
*******************************************************************************
*
* Stack frame structure...
*
 STRUCTURE	SAD_FRAME,0
	; The first three are READ-ONLY...  Mainly used to make it
	; easier to understand what is going on in the system.
	ULONG	SAD_VBR		; Current VBR (always 0 on 68000 CPUs)
	ULONG	SAD_AttnFlags	; ULONG copy of the flags (UPPER WORD==0)
	ULONG	SAD_ExecBase	; ExecBase
	; These fields are the user registers...  The registers are
	; restored from these fields on exit from SAD...
	; Note that USP is only valid if SR was *NOT* supervisor...
	ULONG	SAD_USP		; User stack pointer
	ULONG	SAD_D0		; User register d0
	ULONG	SAD_D1		; User register d1
	ULONG	SAD_D2		; User register d2
	ULONG	SAD_D3		; User register d3
	ULONG	SAD_D4		; User register d4
	ULONG	SAD_D5		; User register d5
	ULONG	SAD_D6		; User register d6
	ULONG	SAD_D7		; User register d7
	ULONG	SAD_A0		; User register a0
	ULONG	SAD_A1		; User register a1
	ULONG	SAD_A2		; User register a2
	ULONG	SAD_A3		; User register a3
	ULONG	SAD_A4		; User register a4
	ULONG	SAD_A5		; User register a5
	ULONG	SAD_A6		; User register a6
	; This is for SAD internal use...  It is the prompt that
	; SAD is using...  Changing this will have no effect on SAD.
	ULONG	SAD_PROMPT	; SAD Prompt Longword...  (internal use)
	; From here on down is the standard exception frame
	; The first two entries (SR and PC) are standard on all 680x0 CPUs
	UWORD	SAD_SR		; Status register (part of exception frame)
	ULONG	SAD_PC		; Return address (part of exception frame)
 LABEL		SAD_FRAME_SIZE
*
*******************************************************************************
*
* Low level routines that everyone (ok, all of SAD) needs...
*
*******************************************************************************
*
BAUD_9600	equ	372	; NTSC...  Very close to PAL speeds...
*				; Based on	(3579545/9600)-1
*				; PAL would be:	(3546895/9600)-1
*
RawIOInit:	xdef	RawIOInit
		move.w	#BAUD_9600,_serper	; Initializes serial speed
		rts
*
*******************************************************************************
*
* Internal use (LVO)
*
* Check if there was a character in the serial hardware.  Get it if
* there, return -1 if not.
*
RawMayGetChar:	xdef	RawMayGetChar
		moveq	#-1,d0
		move.w	_serdatr,d1
		btst	#14,d1
		beq.s	mg_exit
		move.w	#INTF_RBF,_intreq
		and.l	#$7f,d1
		move.l	d1,d0
mg_exit:	rts
*
*******************************************************************************
*
* Internal use (LVO)
*
* Get a character from the serial port hardware
* (7-bit ASCII)
*
RawGetChar:	xdef	RawGetChar
		bsr.s	RawMayGetChar
		tst.l	d0
		bmi.s	RawGetChar
		rts
*
*******************************************************************************
*
* Internal use (LVO)
*
* Put a character (ASCII) out the serial hardware...
* This will also handle ctrl-S and when in ctrl-S can take a
* DEL key to go into Debug...
*
RawPutChar:	xdef	RawPutChar
		tst.b	d0
		beq.s	pc_exit
*
		move.w	d0,-(sp)
		cmp.b	#10,d0
		bne.s	pc_norm
		moveq	#13,d0
		bsr.s	putc
pc_norm:	move.w	(sp)+,d0
putc:
pc_wait:	nop
		move.w	_serdatr,d1
		btst	#13,d1
		beq.s	pc_wait
*
		and.w	#$ff,d0
		or.w	#$100,d0
		move.w	d0,_serdat
*
		bsr.s	RawMayGetChar
*
pc_hold:	cmp.b	#('S'-'A'+1),d0
		bne.s	pc_debug
		bsr.s	RawGetChar
		bra.s	pc_hold
*
pc_debug:	cmp.b	#$7f,d0 	; del
		bne.s	pc_exit
		moveq.l	#0,d0		; (Passes in 0)
		JMPSELF	Debug		; Do debug...
*
pc_exit:	rts
*
*******************************************************************************
*
* This routine will send a long out the serial port.  The long in d0 will
* be sent MSW/LSW form (each word being sent MSB/LSB)
* d0 will *not* be trashed.
* d1 will be trashed.
*
RAWSendLong:	swap	d0		; Get upper word
		bsr.s	RAWSendWord	; Send that word
		swap	d0		; Get lower word again
;		fall into the SendWord routine...
*
*******************************************************************************
*
* This routine will send a word out the serial port.  The word in d0 will
* be sent MSB/LSB form.  d0 will *not* be trashed.
* d1 will be trashed...
*
RAWSendWord:	ror.w	#8,d0		; Get upper byte...
		bsr.s	RAWSendByte	; Send it...
		rol.w	#8,d0		; Get lower byte back...
;		fall into the SendByte routine...
*
*******************************************************************************
*
* Send a byte.  The byte will be in d0.  d0 will not be trashed.
* d1 will be trashed...
*
RAWSendByte:	nop			; Need this to slow things down a bit
		move.w	_serdatr,d1	; Get status of serial port
		btst	#13,d1		; Is it ready for me?
		beq.s	RAWSendByte	; If not, loop for more...
*
		move.w	#$100,d1	; Set stop bit (8N1)
		move.b	d0,d1		; Get data...
		move.w	d1,_serdat	; Send it
		rts
*
*******************************************************************************
*
* Read a byte from the serial port.  If there is no data for more than
* 1 second, it will time out and return -1 in d0.  Flags will also be
* set on exit...  d1 will contain the byte on valid return.
*
READ_TIMEOUT	equ	$91FFF			; Loop count for ~2 seconds...
RAWReadByte:	move.l	#READ_TIMEOUT,d1	; Read timeout count...
		bra.s	ReadByte_Loop		; Go into the read loop...
*
ReadByte_Swap:	swap	d1			; Swap word back down...
ReadByte_Loop:	move.w	_serdatr,d0		; Get data...
		btst	#14,d0			; Check it...
		bne.s	ReadByte_Data		; Got it!
		bchg.b	#1,$bfe001		; Toggle LED and use up time...
		dbra.s	d1,ReadByte_Loop	; Do this for a while...
		swap	d1			; Swap in high word...
		dbra.s	d1,ReadByte_Swap	; And some more...
		moveq.l	#-1,d0			; Set up d0
		rts
ReadByte_Data:	and.l	#$FF,d0			; Mask unused bits
		move.w	#INTF_RBF,_intreq	; Clear interrupt...
		move.l	d0,d1			; Return in d1 too...
		rts
*
*******************************************************************************
*
* Read a word from the serial port.  d0 will contain the last byte
* read or -1 if timeout.  d1 will contain the word...
* CC's will be set based on last byte read...
* Trashes a0...
*
RAWReadWord:	bsr.s	RAWReadByte	; Get a byte
		bmi.s	ReadWord_Exit	; If timeout, exit...
		asl.w	#8,d1		; Shift to upper word...
		move.l	d1,a0		; Save here
		bsr.s	RAWReadByte	; Get next byte
		add.l	a0,d1		; Put into d1
ReadWord_Exit:	tst.l	d0		; Set CC again...
		rts
*
*******************************************************************************
*
* Read a long from the serial port.  d0 will contain the last byte
* read or -1 if timeout.  d1 will contain the long...
* CC's will be set based on last byte read...
* Trashes a1 and a0...
*
RAWReadLong:	bsr.s	RAWReadWord	; Get a word
		bmi.s	ReadLong_Exit	; If timeout, exit...
		swap	d1		; Word into highword...
		move.l	d1,a1		; Save it off...
		bsr.s	RAWReadWord	; Get next word...
		add.l	a1,d1		; Add in the other half
ReadLong_Exit:	tst.l	d0		; Set CC again...
		rts
*
*******************************************************************************
*
* This routine will initialize those things that SAD will need...
*
WackInitCode:	xdef	WackInitCode
		lea	SAD_Entry(pc),a0	; Get debugger entry...
		move.l	a0,DebugEntry(a6)	; Place this into execbase...
		rts
*
******* exec.library/Debug ****************************************************
*
*   NAME
*	Debug -- run the system debugger
*
*   SYNOPSIS
*	Debug(flags)
*	      D0
*
*	void Debug(ULONG);
*
*   FUNCTION
*	This function calls the system debugger.  By default this debugger
*	is "SAD" in >= V39 and "ROM-WACK" in < V39.  Other debuggers are
*	encouraged to take over this entry point (via SetFunction()) so
*	that when an application calls Debug(), the alternative debugger
*	will get control.  Currently a zero is passed to allow future
*	expansion.
*
*   NOTE
*	The Debug() call may be made when the system is in a questionable
*	state; if you have a SetFunction() patch, make few assumptions, be
*	prepared for Supervisor mode, and be aware of differences in the
*	Motorola stack frames on the 68000,'10,'20,'30,'40 (etc.)
*
*   BUGS
*	In ROMWack, calling this function in SUPERVISOR state would have
*	caused the a5 register to be trashed and the user stack pointer to
*	be trashed.  As of V39 (and the instroduction of SAD) this is no
*	longer the case.  However, calling this function in Supervisor
*	state is a bit "tricky" at best...
*
*	Note that due to a bug, pre-V40 SAD had the command
*	codes wrong.  See the SAD autodoc for more details.
*
*   SEE ALSO
*	SetFunction()
*	your favorite debugger's manual...
*	the SAD autodocs...
*	the ROM-WACK chapter of the ROM Kernel Manual... (pre-V39)
*
*******************************************************************************
*
* We are going to go down!  WackCrash is a dead-end call to SAD.
* When SAD exits it will just reboot the machine (hopefully ;^)
*
WackCrash:	xdef	WackCrash		; For exec to call in bad state
		clr.w	-(sp)		; Extra fram for 68010 and up...
		pea	CrashReset	; RTE will return to here...
		move.w	#$2700,-(sp)	; SR will still be supervisor state...
*
SAD_Entry:	; Strange entry point from execbase... (compatibility)
		; This assumes that RTE is used to exit...
		move.l	#SADPROMPT_DEAD,-(sp)
		cmp.l	#SADPROMPT_DEAD,(sp)	; Did the stack work?
		beq.s	Stack_OK	; Build rest of the frame...
		bra	Dead_Stack	; If not working, we are lost...
*
*******************************************************************************
*
* This is the debug entry point.  It will make sure that all is well...
*
Debug:		xdef	Debug			; For the LVO...
SAD_Debug:	move.l	a5,-(sp)		; Save this...
		lea	SAD_Debug1(pc),a5	; Place to continue
		JSRSELF	Supervisor		; Get into supervisor mode...
*
* We will only get here if we were in supervisor state when we called
* the Supervisor function.  If we were supervisor already, we just RTE again
* and set up a fake frame...  The only trick is doing this without
* trashing any registers and setting up the right frame...
*
		btst.b	#AFB_68010,AttnFlags+1(a6)	; Check for 68010
		bne.s	SAD_Frame010		; If 68010 or better...
*
* Ok, now for the frame for the 68000
*
		move.l	(sp)+,a5		; Restore A5
		move.w	sr,-(sp)		; Fake frame...
		bra.s	SAD_UserTrace		; Get into the common entry...
*
* This does the fake frame for 68010 or better CPUs
*
SAD_Frame010:	move.l	(sp),a5			; Get a5 back...
		move.l	a5,-(sp)		; Store another one...
		move.l	8(sp),a5		; Get return address...
		move.l	a5,6(sp)		; Store into RTE frame...
		move.l	(sp)+,a5		; Get a5 back...
		move.w	sr,(sp)			; Store SR...
		move.w	#$20,6(sp)		; Format 0, offset $20
		bra.s	SAD_UserTrace		; Get into the common entry...
*
SAD_SuperDebug:	rte				; Return to original supervisor
*
* At this point we are sure that we are supervisor state...
*
SAD_Debug1:	btst.b	#13-8,(sp)		; Check if we were supervisor
		bne.s	SAD_SuperDebug		; Supervisor state!  Watch out!
*
* User mode call to Debug()...  We will adjust the stack as needed...
*
SAD_userMode:	move.l	usp,a5			; Get user stack pointer...
		addq.l	#4,a5			; Get past the return address
		move.l	(a5)+,-(sp)		; Transfer saved a5...
		move.l	(a5)+,2+4(sp)		; Transfer old return address
		move.l	a5,usp			; Set up user stack pointer
		move.l	(sp)+,a5		; Get a5 back...
		; Fall into the common non-NMI entry: SAD_UserTrace
*
*******************************************************************************
*
* This entry point is only accessed when a trace happens in USER mode...
*
SAD_UserTrace:	move.l	#SADPROMPT,-(sp)
		cmp.l	#SADPROMPT,(sp)	; Did the stack work?
		beq.s	Stack_OK	; Build rest of the frame...
		bra.s	Dead_Stack
*
*******************************************************************************
*
SysIntr7:	xdef	SysIntr7	; For the world to see
*
* Darn hardware bugs!!!
*
* Well, it seems that the A2000 and A500 (and A3000!) systems have this
* problem with getting NMI interrupts when they should not.  (Randomly)
* Usually happens when a level 6 or 5 interrupt is generated.
* Since this only seems to happen on A2000 and A500 systems, (and A3000)
* we will only have those kickstarts have an EMPTY RTE at
* that location.  This should help performance and remove the possible
* problems with NMIs on those systems that have the problem.
* To reconnect SAD to the NMI, you would look to see if the NMI
* vector points at a RTE instruction and if it does, you would add
* 2 to the vector.  (Thus pointing it to SAD_NMI)
*
		rte		; We will do a quick RTE
*
* This is the default SAD-NMI entry point.  It should either be at SysIntr7
* or 2 bytes past that point...
*
SAD_NMI:	; This is the NMI entry for SAD...
		move.l	#SADPROMPT_NMI,-(sp)
		cmp.l	#SADPROMPT_NMI,(sp)	; Is the stack working?
		bne.s	Dead_Stack	; If the stack seems to work...
*
* Ok, so the stack tested ok...  Lets build the frame...
*
Stack_OK:	movem.l	d0-d7/a0-a6,-(sp)	; Save all the registers...
		move.l	usp,a6			;
		move.l	a6,-(sp)		; User stack pointer
		move.l	ABSEXECBASE,a6		; Get execbase...
		move.l	a6,d0			; Is is OK?
		beq.s	Bad_ExecBase
		btst	#0,d0			; Is it odd?
		bne.s	Bad_ExecBase
		add.l	ChkBase(a6),d0		; Does it checksum?
		addq.l	#1,d0			; (This should make it 0)
		bne.s	Bad_ExecBase		; Execbase is trash if not...
*
		moveq.l	#0,d6			; Clear d6...
		move.w	AttnFlags(a6),d6	; Get flags...
		moveq.l	#0,d0			; Assume VBR=0
		btst	#AFB_68010,d6		; Do we have a VBR?
		beq.s	SAD_NoVBR		; If not, skip...
*
		opt	p=68010			; Assemble for 68010
		movec	vbr,d0			; Get VBR
		opt	p=68000			; Back to normal...
*
SAD_NoVBR:	move.l	d0,a4			; Save VBR...
		movem.l	d0/d6/a6,-(sp)		; VBR/AttnFlags/ExecBase
*
* Ok, so we are set up, ready to go...
*
		move.l	sp,a5			; Get context frame...
		move.l	SAD_PROMPT(a5),d7	; Get prompt...
*
* Save the state of the interrupt enables for the serial port
*
		move.w	#(INTF_RBF!INTF_TBE),d0	; Get the bit mask...
		move.w	_intenar,d5		; And in the bits...
		and.w	d0,d5			; Mask of what we will need
		move.w	d0,_intena		; Clear interrupt enables...
		bsr	RawIOInit		; Set the speed...
*
* Do main command loop...
*
		bsr.s	SAD_Main		; Do main loop...
*
* Restore serial port interrupt settings...
*
		bset	#INTB_SETCLR,d5		; Set the old intena settings
		move.w	d5,_intena		; Set them up...
*
* Returning to system...
*
		movem.l	(sp)+,d0/d6/a6		; VBR/AttnFlags/ExecBase
		move.l	(sp)+,a6		; Get user stack pointer
		move.l	a6,usp			; Set it...
		movem.l	(sp)+,d0-d7/a0-a6	; Restore registers
		addq.l	#4,sp			; Unstack the prompt
		rte
*
*******************************************************************************
*
Dead_Stack:	; Without a stack, I don't know what to do...  I guess a
		; reboot would be good here...  (Fall into it)
*
Bad_ExecBase:	; Execbase does not look good, what are we to do?
		; I guess we should just crash the OS at this point!
		bra	CrashReset	; Just blow us away!
*
*******************************************************************************
*
* Registers as used by the following routines:
*
* D0 - Scrap
* D1 - Scrap
* D2 - Scrap  (local)
* D3 -
* D4 -
* D5 - cache of old serial interrupt settings...
* D6 - Processor flags  (From AttnFlags)
* D7 - The SAD prompt.  If this is NULL, SAD will exit...
*
* A0 - Scrap
* A1 - Scrap
* A2 - Scrap  (local)
* A3 -
* A4 - VBR (0 on 68000, VBR on 680x0 where x>0)
* A5 - Context pointer
* A6 - ExecBase
* A7 - Stack pointer
*
*******************************************************************************
*
* Main loop for SAD...  It will remain in this loop until timeout or D7
* becomes NULL...
*
SAD_Main:	tst.l	d7		; Check for NULL prompt...
		beq.s	SAD_Main_Exit	; Exit if NULL prompt...
		moveq.l	#9,d2		; timeout...
SAD_Wait1:	move.l	d7,d0		; Get prompt...
		bsr	RAWSendLong	; Put up the prompt...
SAD_Wait2:	bsr	RAWReadByte	; Get a byte...
		bpl.s	SAD_Command	; Got a byte?
		dbra.s	d2,SAD_Wait1	; Try until timeout...
SAD_Main_Exit:	rts			; Exit...
*
SAD_Command:	cmp.b	#27,d1		; Check for <ESC>
		beq.s	SAD_Main_Exit	; If <ESC> we exit...
		cmp.b	#$AF,d1		; Check for $AF byte...
		bne.s	SAD_Wait2	; If not $AF, continue waiting...
		bsr	RAWReadByte	; Get command...
		bmi.s	SAD_Wait2	; No command following $AF!
		moveq.l	#9,d2		; Restart the counter...
		subq.b	#1,d1		; Test the command...
		bmi.s	SAD_Wait2	; Loop back and wait if 0 (NO-OP)
		lea	Commands(pc),a0	; Get command table...
*
* Find the command...
*
SAD_Find:	move.l	(a0)+,d0	; Get command
		beq.s	SAD_Main	; Do nothing (invalid command)
		dbra.s	d1,SAD_Find	; Keep looking...
		pea	SAD_Main(pc)	; Put return address onto stack
		move.l	d0,a0		; Get command address
		jmp	(a0)		; Do the command (returns to SAD_Main)
*
* Command Table...
*
Commands:	dc.l	Command01
		dc.l	Command02
		dc.l	Command03
		dc.l	Command04
		dc.l	Command05
		dc.l	Command06
		dc.l	Command07
		dc.l	Command08
		dc.l	Command09
		dc.l	Command0A
		dc.l	Command0B
		dc.l	Command0C
		dc.l	Command0D
		dc.l	Command0E
		dc.l	Command0F
		dc.l	Command10
		dc.l	0
*
******* SAD/NOP ***************************************************************
*
* NO-OP - Do nothing other than tell SAD you are still there...
*
* Command:	$AF $00
* Data:		<none>
*
* This just tells SAD you are still there.  Required so that timeouts do
* not exit SAD while you are not doing anything.
*
* This command will *NOT* be ACK'ed.  It will just cause the timeout to
* be restarted.
*
*
*******************************************************************************
*
* Very simple command...  Does nothing, no code, etc..  ;^)
*
******* SAD/WRITE_BYTE ********************************************************
*
* WRITE BYTE - Write the given data to the address given	(V40 SAD)
*
* Command:	$AF $01
* Data:		$ww $xx $yy $zz $qq
*
* Write the byte <$qq> to address <$wwxxyyzz>
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $01
* Command DONE:	$1F $01
*
* BUGS
*	This command does not exists in pre-V40 EXEC.
*	This command can be emulated with the WRITE_ARRAY command with
*	a length of 1.
*
*******************************************************************************
*
Command01:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		bsr	RAWReadByte	; Get the data
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save the data
		moveq.l	#$0001,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.b	d2,(a2)		; Do the write...
		bra.s	Command_Cache	; Do command DONE (with cache clear)
*
******* SAD/WRITE_WORD ********************************************************
*
* WRITE WORD - Write the given data to the address given
*
* Command:	$AF $02
* Data:		$ww $xx $yy $zz $qq $rr
*
* Write the word <$qqrr> to address <$wwxxyyzz>
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $02
* Command DONE:	$1F $02
*
*
*******************************************************************************
*
Command02:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		bsr	RAWReadWord	; Get the data
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save the data
		moveq.l	#$0002,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.w	d2,(a2)		; Do the write...
		bra.s	Command_Cache	; Do command DONE (with cache clear)
*
******* SAD/WRITE_LONG ********************************************************
*
* WRITE LONG - Write the given data to the address given
*
* Command:	$AF $03
* Data:		$ww $xx $yy $zz $qq $rr $ss $tt
*
* Write the long <$qqrrsstt> to address <$wwxxyyzz>
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $03
* Command DONE:	$1F $03
*
*
*******************************************************************************
*
Command03:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		bsr	RAWReadLong	; Get the data
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save the data
		moveq.l	#$0003,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.l	d2,(a2)		; Do the write...
		bra.s	Command_Cache	; Do command DONE (with cache clear)
*
******* SAD/READ_BYTE *********************************************************
*
* READ BYTE - Read a byte from the given address
*
* Command:	$AF $04
* Data:		$ww $xx $yy $zz
*
* Read a byte from address <$wwxxyyzz>  Returns <$qq> as result
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $04
* Command DONE:	$1F $04 $qq
*
*
*******************************************************************************
*
Command04:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		moveq.l	#$0004,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.b	(a2),d2		; Get answer...
		bsr.s	Command_Done	; Start the answer...
		move.l	d2,d0		; get answer
		bra	RAWSendByte	; Send the answer and RTS...
*
******* SAD/READ_WORD *********************************************************
*
* READ WORD - Read a word from the given address		(V40 SAD)
*
* Command:	$AF $05
* Data:		$ww $xx $yy $zz
*
* Read a word from address <$wwxxyyzz>  Returns <$qqrr> as result
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $05
* Command DONE:	$1F $05 $qq $rr
*
* BUGS
*	This command does not return correct values in pre-V40 EXEC.
*
*******************************************************************************
*
Command05:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		moveq.l	#$0005,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.w	(a2),d2		; Get answer...
		bsr.s	Command_Done	; Start the answer...
		move.l	d2,d0		; get answer
		bra	RAWSendWord	; Send the answer and RTS...
*
*******************************************************************************
*
* These are here to make short branches work...
*
Command_Exit:	rts
Command_Cache:	move.l	d0,d2		; Save command number...
		JSRSELF	CacheClearU	; Clear caches
		move.l	d2,d0		; Get command number back
*
Command_Done:	or.w	#$1F00,d0	; Get $1F $?? for command DONE
		bra	RAWSendWord	; Send it and rts...
*
*
******* SAD/READ_LONG *********************************************************
*
* READ LONG - Read a long from the given address
*
* Command:	$AF $06
* Data:		$ww $xx $yy $zz
*
* Read a long from address <$wwxxyyzz>  Returns <$qqrrsstt> as result
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $06
* Command DONE:	$1F $06 $qq $rr $ss $tt
*
*
*******************************************************************************
*
Command06:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		moveq.l	#$0006,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		move.l	(a2),d2		; Get answer...
		bsr.s	Command_Done	; Start the answer...
		move.l	d2,d0		; get answer
		bra	RAWSendLong	; Send the answer and RTS...
*
******* SAD/CALL_ADDRESS ******************************************************
*
* CALL ADDRESS - JSR to the given address.
*
* Command:	$AF $07
* Data:		$ww $xx $yy $zz
*
* Call the following address as a subroutine.  No registers will be
* set up but the context frame will exist.  Standard calling
* conventions apply (d0/d1/a0/a1 are available, rest must be saved)
* The command will be ACK'ed when received.
* Command ACK:	$00 $07
* Command DONE:	$1F $07
*
*
*******************************************************************************
*
Command07:	bsr	RAWReadLong	; Get the address
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,a2		; Get address...
		moveq.l	#$0007,d0	; Get ACK...
		bsr	RAWSendWord	; Send ACK...
		jsr	(a2)		; Call routine...
		moveq.l	#$0007,d0	; Get command...
		bra.s	Command_Done	; Exit...
*
******* SAD/RETURN_TO_SYSTEM **************************************************
*
* RETURN TO SYSTEM
*
* Command:	$AF $08
* Data:		$00 $00 $00 $00
*
* This command will return <exit> from SAD back to whatever started it.
* The 4 $00 are required as a "safty" to this command.  The command
* will be ACK'ed only as it will have lost control of the system.
*
* Command ACK:	$00 $08
*
*
*******************************************************************************
*
Command08:	bsr	RAWReadLong	; Get the 4 NULLs...
		bmi.s	Command_Exit	; If error, exit...
		tst.l	d1		; Check if NULL
		bne.s	Command_Exit	; If not NULL, exit...
		moveq.l	#0,d7		; Clear prompt...  (causes exit)
		moveq.l	#$0008,d0	; Get command ACK...
		bra	RAWSendWord	; Send and exit...
*
******* SAD/GET_CONTEXT_FRAME *************************************************
*
* GET CONTEXT FRAME
*
* Command:	$AF $09
* Data:		<none>
*
* This command will return a pointer to the saved context.  This will be
* a pointer to all of the registers that were saved on the stack along
* with some other details.  Returns frame address <$wwxxyyzz>
*
* The pointer returned is to the following structure:
*
* STRUCTURE	SAD_FRAME,0
*	; The first three are READ-ONLY...  Mainly used to make it
*	; easier to understand what is going on in the system.
*	ULONG	SAD_VBR		; Current VBR (always 0 on 68000 CPUs)
*	ULONG	SAD_AttnFlags	; ULONG copy of the flags (UPPER WORD==0)
*	ULONG	SAD_ExecBase	; ExecBase
*	; These fields are the user registers...  The registers are
*	; restored from these fields on exit from SAD...
*	; Note that USP is only valid if SR was *NOT* supervisor...
*	ULONG	SAD_USP		; User stack pointer
*	ULONG	SAD_D0		; User register d0
*	ULONG	SAD_D1		; User register d1
*	ULONG	SAD_D2		; User register d2
*	ULONG	SAD_D3		; User register d3
*	ULONG	SAD_D4		; User register d4
*	ULONG	SAD_D5		; User register d5
*	ULONG	SAD_D6		; User register d6
*	ULONG	SAD_D7		; User register d7
*	ULONG	SAD_A0		; User register a0
*	ULONG	SAD_A1		; User register a1
*	ULONG	SAD_A2		; User register a2
*	ULONG	SAD_A3		; User register a3
*	ULONG	SAD_A4		; User register a4
*	ULONG	SAD_A5		; User register a5
*	ULONG	SAD_A6		; User register a6
*	; This is for SAD internal use...  It is the prompt that
*	; SAD is using...  Changing this will have no effect on SAD.
*	ULONG	SAD_PROMPT	; SAD Prompt Longword...  (internal use)
*	; From here on down is the standard exception frame
*	; The first two entries (SR and PC) are standard on all 680x0 CPUs
*	UWORD	SAD_SR		; Status register (part of exception frame)
*	ULONG	SAD_PC		; Return address (part of exception frame)
*
*
* Command ACK:	$00 $09
* Command DONE:	$1F $09 $ww $xx $yy $zz
*
*
*******************************************************************************
*
Command09:	moveq.l	#$0009,d0	; Get ACK...
		bsr	RAWSendWord	; Send ACK
		bsr.s	Command_Done	; Send done...
		move.l	a5,d0		; Get address
		bra	RAWSendLong	; Send address and return...
*
******* SAD/ALLOCATE_MEMORY ***************************************************
*
* ALLOCATE MEMORY
*
* Command:	$AF $0A
* Data:		$qq $rr $ss $tt $hh $ii $jj $kk
*
* Allocate a chunk of memory that is <$qqrrsstt> bytes in size.  Note
* that this call is only safe when SAD is in $3F prompting mode (called
* from Debug()) and even then may be unsafe if the system is in bad shape.
* (You are debugging after all)  The returned address will be available to
* you until you release it.  (It is obtained via a call to AllocVec())  The
* type of memory allocated is <$hhiijjkk>.  Note that the allocation may
* fail. In that case, the address returned will be $00000000.
*
* Command ACK:	$00 $0A
* Command DONE:	$1F $0A $ww $xx $yy $zz
*
*
*******************************************************************************
*
Command0A:	bsr	RAWReadLong	; Get the size...
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save size...
		bsr	RAWReadLong	; Get type...
		bmi.s	Command_Exit	; If error, exit...
		move.l	d1,-(sp)	; Save for a moment...
		moveq.l	#$000A,d0	; Get ACK ready
		bsr	RAWSendWord	; Send the ACK...
		move.l	(sp)+,d1	; Get d1...
		move.l	d2,d0		; Get size...
		JSRSELF	AllocVec	; Allocate...
		move.l	d0,d2		; Save address...
		moveq.l	#$000A,d0	; Get command...
		bsr.s	Command_Done	; Send the beginning of the done
		move.l	d2,d0		; Get address...
		bra	RAWSendLong	; Send the address and exit...
*
******* SAD/FREE_MEMORY *******************************************************
*
* FREE MEMORY
*
* Command:	$AF $0B
* Data:		$ww $xx $yy $zz
*
* Free the memory allocated with the ALLOCATE MEMORY command.  This command
* has the same restrictions as ALLOCATE MEMORY.  Memory is released by
* calling FreeVec() on the address <$wwxxyyzz>
*
* Command ACK:	$00 $0B
* Command DONE:	$1F $0B
*
*
*******************************************************************************
*
Command0B:	bsr	RAWReadLong	; Get the size...
		bmi	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save it for a moment...
		moveq.l	#$000B,d0	; Get command ACK...
		bsr	RAWSendWord	; Send the ack...
		move.l	d2,a1		; Get address...
		JSRSELF	FreeVec		; Free it up...
		moveq.l	#$000B,d0	; Set up command
		bra	Command_Done	; Return command done...
*
******* SAD/TURN_ON_SINGLE ****************************************************
*
* TURN ON SINGLE STEPPING
*
* Command:	$AF $0C
* Data:		<none>
*
* This command will turn on SAD single stepping mode.  This means that SAD
* will single step (via 68000 trace mode) the system.  SAD will take over
* the TRACE exception vector during this time.  This command will return
* the contents of the vector such that you can return this value when
* you wish to turn off single stepping mode.  Note that turning on single
* stepping mode while in $BF prompting will cause the step to be taken
* and then SAD will execute in $3F mode (non-NMI)
* The command returns <$wwxxyyzz> which you must use when turning off
* the single-step mode.
*
* Command ACK:	$00 $0C
* Command DONE:	$1F $0C $ww $xx $yy $zz
*
*
*******************************************************************************
*
Command0C:	moveq.l	#$000C,d0	; Get ACK
		bsr	RAWSendWord	; Send the word...
		move.l	SAD_VBR(a5),a2	; Get VBR...
		move.l	36(a2),d2	; Get old trace location...
		lea	SAD_UserTrace(pc),a0	; Get new trace
		move.l	a0,36(a2)	; Set new trace...
		bset.b	#15-8,SAD_SR(a5)	; Set trace bit...
		bsr	Command_Cache	; Send the command done...
		move.l	d2,d0		; Get answer
		bra	RAWSendLong	; Send it and exit...
*
******* SAD/TURN_OFF_SINGLE ***************************************************
*
* TURN OFF SINGLE STEPPING
*
* Command:	$AF $0D
* Data:		$ww $xx $yy $zz
*
* This command will turn off SAD single stepping mode.  You need to pass
* to it the address returned from the call to turn on single stepping mode.
*
* Command ACK:	$00 $0D
* Command DONE:	$1F $0D
*
*
*******************************************************************************
*
Command0D:	bsr	RAWReadLong	; Get the address...
		bmi	Command_Exit	; Exit on error...
		move.l	d1,d2		; Save answer...
		moveq.l	#$000D,d0	; Get ACK
		bsr	RAWSendWord	; Send the word...
		move.l	SAD_VBR(a5),a2	; Get VBR...
		move.l	d2,36(a2)	; Set new trace...
		bclr.b	#15-8,SAD_SR(a5)	; Clear trace bit...
		bsr	Command_Cache	; Send the command done...
		move.l	d2,d0		; Get answer
		bra	RAWSendLong	; Send it and exit...
*
******* SAD/WRITE_ARRAY *******************************************************
*
* WRITE ARRAY - Write a range of bytes
*
* Command:	$AF $0E
* Data:		$ww $xx $yy $zz $qq $rr $ss $tt
*
* Write a range of bytes to address <$wwxxyyzz> for <$qqrrsstt> bytes
* After the computer sends the ACK, you must then send the byte stream...
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $0E
* Command DONE:	$1F $0E
*
*
*******************************************************************************
*
Command0E:	bsr	RAWReadLong	; Get the address
		bmi	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		bsr	RAWReadLong	; Get number of bytes
		bmi	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save number...
		moveq.l	#$000E,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		bra.s	loop_entry	; Enter the loop
swap_again:	swap	d2		; Swap back
read_next:	bsr	RAWReadByte	; Get next byte
		bmi	Command_Exit	; No good?
		move.b	d1,(a2)+	; Write it...
loop_entry:	dbra.s	d2,read_next	; Inner loop (d2 low word)
		swap	d2
		dbra.s	d2,swap_again	; Outer loop (d2 high word)
		moveq.l	#$000E,d0	; Send command done
		bra	Command_Cache	; Send DONE and flush caches and exit
*
******* SAD/READ_ARRAY ********************************************************
*
* READ ARRAY - Read a range of bytes
*
* Command:	$AF $0F
* Data:		$ww $xx $yy $zz $qq $rr $ss $tt
*
* Read a range of bytes from address <$wwxxyyzz> for <$qqrrsstt> bytes
* Will return that number of bytes...
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $0F
* Command DONE:	$1F $0F $uu [$uu ...]
*
*
*******************************************************************************
*
Command0F:	bsr	RAWReadLong	; Get the address
		bmi	Command_Exit	; If error, exit...
		move.l	d1,a2		; Save that...
		bsr	RAWReadLong	; Get number of bytes
		bmi	Command_Exit	; If error, exit...
		move.l	d1,d2		; Save number...
		moveq.l	#$000F,d0	; Now to ACK the command...
		bsr	RAWSendWord	; Send ACK...
		bsr	Command_Done	; Send start of DONE message...
		bra.s	loop_bottom	; Start into the loop...
*
swap_back:	swap	d2		; Swap back into high word...
do_next:	move.b	(a2)+,d0	; Get next byte
		bsr	RAWSendByte	; Send it...
loop_bottom:	dbra.s	d2,do_next	; Inner loop (d2 low word)
		swap	d2
		dbra.s	d2,swap_back	; Outer loop (d2 high word)
		rts			; Done...
*
******* SAD/RESET *************************************************************
*
* RESET - Reset the computer...
*
* Command:	$AF $10
* Data:		$FF $FF $FF $FF
*
* This command will reset the computer.  the $FFFFFFFF value is there
* mainly to prevent false reset.  This command will only be ACK'ed as
* the computer will be reset afterwards...
*
* Command will be ACK'ed when received.
* Command ACK:	$00 $10
*
*
*******************************************************************************
*
Command10:	bsr	RAWReadLong	; Get the address
		bmi	Command_Exit	; If error, exit...
		addq.l	#1,d1		; Check if $FFFFFFFF
		bne	Command_Exit	; If not, we will exit...
		moveq.l	#$0010,d0	; Get command ACK...
		bsr	RAWSendWord	; Send ACK...
*
* Now reboot the machine!
*
		JMPSELF	ColdReboot	; <grin>
*
*******************************************************************************
*
		END
