	FAR	DATA
	TTL	'DEVICE.ASM'
*
*      BUG!!! supposedly AddTask can fail under 1.4, we don't check for this!
*
*
*	Note 1 -- added 8/11/89 -- Clear read iorb io_error field
*	Note 2 -- added 8/28/89 -- Reads now get queued
*	Note 3 -- added 8/28/89 -- Write code no longer checks for queued reads

*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


               SECTION speech
_AbsExecBase	EQU	4


* ***** Included Files ***********************************************


	INCLUDE 'assembly.i'
 	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'hardware/custom.i'
	INCLUDE 'hardware/dmabits.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'

io_Size	EQU	IO_SIZE
        INCLUDE	'featequ.i'
        INCLUDE	'gloequs.i'
	INCLUDE	'devices/audio.i'
	INCLUDE	'narrator.i'
	INCLUDE	'private.i'
        INCLUDE	'narrator_rev.i'


* ****** Imported Globals *********************************************




* ****** Imported Functions *******************************************

	EXTERN_SYS SendIO
	EXTERN_SYS DoIO
	EXTERN_SYS AddDevice
	EXTERN_SYS RemDevice
	EXTERN_SYS AddTask
	EXTERN_SYS RemTask
	EXTERN_SYS AddHead
	EXTERN_SYS AddTail
	EXTERN_SYS AllocMem
	EXTERN_SYS AllocEntry
	EXTERN_SYS AllocSignal
*	EXTERN_SYS AllocAudChan
*	EXTERN_SYS StartAudDMA
*	EXTERN_SYS StopAudDMA
*	EXTERN_SYS FreeAudChan
	EXTERN_SYS Disable
	EXTERN_SYS Enable
	EXTERN_SYS FreeMem
	EXTERN_SYS FreeEntry
	EXTERN_SYS MakeLibrary
*	EXTERN_SYS PutFmt
	EXTERN_SYS PutMsg
	EXTERN_SYS ReplyMsg
	EXTERN_SYS Signal
	EXTERN_SYS Wait
	EXTERN_SYS OpenLibrary
	EXTERN_SYS CloseLibrary
	EXTERN_SYS OpenDevice
	EXTERN_SYS CloseDevice
	EXTERN_SYS GetMsg
	EXTERN_SYS FindTask

        EXTERN_DATA _intena

* ****** Exported Functions *******************************************

*	XDEF	_Init
*	XDEF	_Expunge
	XDEF	BeginIO


* ***** Local Definitions **********************************************



	XDEF	Open
	XDEF	Close
 	XDEF	PerformIO
	XDEF	Null
	XDEF	TermIO

	XREF	SDTaskStart
*	XREF	INITIAL,endMarker
 	XREF	NarratorName,NarratorIdStr
	XREF	Narrator,DEVDBG_StoreParms


******** ROMTAG stuff	*********************************************

	MOVEQ	  #-1,D0			;Protection against bad call
	RTS


*	;------ ROMTAG structure

RomTagDescr:
	DC.W	RTC_MATCHWORD			;Magic
	DC.L	RomTagDescr			;Back pointer
	DC.L	EndNarrCode			;End of code
	DC.B	0				;No auto-init
	DC.B	VERSION
	DC.B	NT_DEVICE
	DC.B	0				;Priority
	DC.L	NarrName
	DC.L	NarratorIdStr
	DC.L	Init


NarrName 	DC.B	'narrator.device',0
		DS.W	0


******** Device/Speech/Initialize **********************************
*
*   NAME
*	Initialize -- initialize the device from nothing
*
*
*   SYNOPSIS
*	Error = Initialize(), SysLib
*	D0		  	A6
*
*
*   FUNCTION
*	Initialize the speech driver at system startup.  This
*	routine is called from OpenDevice if the code is not RAM
*	resident.  THIS ENTRY POINT MUST BE THE FIRST ADDRESS OF
*	THE DRIVER CODE.
*
*
*   INPUTS
*	A6 - Pointer to SysLib
*
*
*   RESULTS
*	0 if init failed, else -1
*	otherwise.
*
*
*   SEE ALSO
*
*
* *********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*


Init:
		MOVEM.L A2-A6/D3,-(SP)
		MOVE.L	A0,D3				;Cache the segment list

		MOVE.L	_AbsExecBase,A6


Init_OpenOK:
*		;------ Call the library initialization routine
		LEA	devFuncInit(PC),A0
		LEA	devStructInit(PC),A1
		SUB.L   A2,A2
		MOVE.L	#ND_SIZE,D0
		CALLSYS MakeLibrary
		TST.L	D0
		BEQ	Init_Err


Init_Success:
*		;------ Managed to get this far, everything else should be 
		;	simple.
		MOVE.L	D0,A2			;Copy device pointer
		MOVE.L  A2,A4			;Store pointer to my device node
		MOVE.L  A6,ND_SYSLIB(A2)	;Store pointer to system library
		MOVE.L	D3,ND_SEGLIST(A2)	;Store segment list pointer


*		;------ Initialize the task msgport list
		LEA	ND_UNIT+MP_MSGLIST(A2),A0
		NEWLIST A0


*		;------	Initialize the pseudo-unit counter
		MOVE.L	#0,ND_PSEUDO(A2)


*		;------	Create msgport for audio commands
		MOVE.L	#MP_SIZE,D0				;Get mem
		MOVE.L	#MEMF_PUBLIC+MEMF_CLEAR,D1
		CALLSYS	AllocMem
		TST.L	D0
		BEQ	Init_Err

Init_MP2:	MOVE.L	D0,A3
		MOVE.L	A3,ND_CMSGPORT(A2)
		MOVE.B	#NT_MSGPORT,MP+LN_TYPE(A3)		;Initialize port
		LEA	cportname,A1
		MOVE.L	A1,MP+LN_NAME(A3)
		MOVEQ	#-1,D0
		CALLSYS	AllocSignal
		MOVE.B	D0,MP_SIGBIT(A3)
		SUB.L	A1,A1
		CALLSYS	FindTask
		MOVE.L	D0,MP_SIGTASK(A3)
		LEA	MP_MSGLIST(A3),A0
		NEWLIST	A0


*		;------ Create task stack
		LEA	ND_TCB(A2),A1		;Get TCB pointer
		LEA	ND_STACK(A2),A0		;Get top of stack
		MOVE.L	A0,TC_SPLOWER(A1)	;Store in TCB
		LEA	ND_STACKSIZE(A0),A0	;Compute top of stack
		MOVE.L	A0,TC_SPUPPER(A1)	;Store in TCB
		MOVE.L	A2,-(A0)		;Device node
		MOVE.L	A0,TC_SPREG(A1)		;Current stack pointer


*		;------ Add task to system (A1 already points to TCB)
		LEA	SDTaskStart,A2		;Startup task
		SUB.L	A3,A3			;Finalizer routine
		CALLSYS	AddTask
	

*		;------ Add the device
		MOVE.L	A4,A1
		CALLSYS AddDevice


*		;------ Everything worked, return non-zero
		MOVEQ   #-1,D0

Init_End:
		MOVEM.L (SP)+,A2-A6/D3
		RTS

Init_Err:
		MOVEQ	#0,D0
		BRA.S	Init_End



******* Device/Speech/Open ****************************************
*
*   NAME
*	OpenDevice - open the narrator device.
*
*   SYNOPSIS
*	error = OpenDevie("narrator.device", 0, IORequest, 0);
*
*   FUNCTION
*	The OpenDevice routine grants access to the narrator device.
*	OpenDevice checks the unit number, and if non-zero, returns
*	an error (ND_UnitErr).  If this is the first time the driver
*	has been opened, OpenDevice will attempt to open the audio
*	device and allocate the driver's static buffers.  If either
*	of these operations fail, an error is returned (see the .hi
*	files for possible error return codes).  Next, OpenDevice
*	(done for all opens, not just the first one) initializes the
*	user's IORequest block (IORB).  Default values for sex, rate,
*	pitch, pitch mode, sampling frequency, and mouths are set in
*	the appropriate fields of the IORB.  Note that if users wish
*	to use non-default values for these parms, the values must
*	be set after the open is done.  OpenDevice then assigns a
*	pseudo-unit number to the IORB for use in synchronizing read
*	and write requests.  See the read command for more details.
*	Finally, OpenDevice stores the device node pointer in the
*	IORB and clears the delayed expunge bit.
*
*   INPUTS
*	deviceName - must be "narrator.device"
*	unitNumber - must be 0
*	IORequest  - the user's IORB (need not be initialized)
*	flags	   - not used
*
*   RESULTS
*	IORB fields set:
*	   rate	    - 150 words/minute
*	   pitch    - 110 Hz
*	   mode	    - Natural
*	   sex      - Male
*	   mouths   - Off
*	   sampfreq - 22200
*	   volume   - 64 (max)
*	and if NEWIORB:
*	   articulate  - 100
*	   centralize  - 0
*	   centralphon - 23  /AX/
*	
*	error - same as io_Error field of IORB
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*


Open:		
		MOVEM.L	 A2/A3/A6,-(SP)


*		;------ Check the unit number, only 0 is valid
		TST.L	D0
		BEQ.S	Open_UnitOK
		MOVEQ   #ND_UnitErr,D0
		BRA	Open_Err

Open_UnitOK:
		MOVE.L	A1,A2				;Copy IORB pointer
                TST.W   LIB_OPENCNT(A6)		        ;Driver already open?
		BNE.S   Is_Open				;Yes


*               ;------ First time driver has been opened.

*		;	Open the audio device and allocate fixed buffers.
		LEA	AudioLib,A0			;Open the audio device
		LEA	ND_IORB3(A6),A1
		CLR.W   ioa_Length+2(A1)		;!!!sam 8/28/85!!!
		MOVEQ	#0,D0
		MOVEQ	#0,D1
		LINKSYS	OpenDevice
		TST.L	D0
		BEQ.S	Open_GotAudio
		MOVE.B	#ND_NoAudLib,D0
		BRA	Open_Err


*		;	Setup audio control IORB
Open_GotAudio:
		LEA	ND_IORB3(A6),A1
		MOVE.B	#NT_MESSAGE,MN+LN_TYPE(A1)		;Set type
		LEA	cportname,A0				;Set name
		MOVE.L	A0,MN+LN_NAME(A1)
		MOVE.L	ND_CMSGPORT(A6),ND_IORB3+MN_REPLYPORT(A6)

*		;	Allocate the fixed buffers
		MOVE.L	#FIXBFRSZ,D0			;Size of buffer
		MOVE.L  #MEMF_CLEAR+MEMF_CHIP,D1	;Get chip memory
		LINKSYS AllocMem			;Get buffer
		TST.L   D0				;Allocate successful?
		BNE.S   Open_GotBuffer			;Yes, branch
                MOVE.B  #ND_NoMem,D0			;No, return an error
		BRA     Open_Err

Open_GotBuffer	MOVE.L	D0,ND_GLOBALS(A6)


*		;------ Increment open count and set default speaking parms.
		;	Test to see if this is a new style IORB, and if so,
		;	set additional defaults.  To determine old vs new
		;	IORB, first test length.  Old IORB is 70 bytes long.
		;	Since someone may append their own data to the IORB
		;	(and if they do, may they die horribly), I also test
		;	the NDF_NEWIORB bit in the flags field.  Damn I'm nice.
Is_Open 	ADDQ.W	#1,LIB_OPENCNT(A6)
*		DISABLE A0
		ADDQ.L	#1,ND_PSEUDO(A6)
		MOVE.L	ND_PSEUDO(A6),IO_UNIT(A2)
*		ENABLE	A0
		MOVE.W	#DEFSEX,NDI_SEX(A2)		;Male speaker
		MOVE.W	#DEFRATE,NDI_RATE(A2)		;Male rate
		MOVE.W	#DEFPITCH,NDI_PITCH(A2)		;Male pitch
		MOVE.W	#DEFMODE,NDI_MODE(A2)		;Natural F0 mode
		CLR.B	NDI_MOUTHS(A2)			;No mouths for now
		MOVE.W	#DEFFREQ,NDI_SAMPFREQ(A2)	;Normal samp frequency
		MOVE.W	#DEFVOL,NDI_VOLUME(A2)		;Normal speaking volume

		CMP.W	#70,MN_LENGTH(A2)		;IORB length = 70 bytes?
		BEQ.S	OldIORB				; Yes, must be old IORB
		BTST	#NDB_NEWIORB,NDI_FLAGS(A2)	; No, is NEWIORB bit set
		BEQ.S	OldIORB				;  No, must be old IORB
							;  Yes, must be new IORB
		MOVE.B	#DEFF0ENTHUS,NDI_F0ENTHUSIASM(A2)  ;Default enthus
		CLR.B	NDI_F0PERTURB(A2)		;Default F0 perturbation
		CLR.B	NDI_F1ADJ(A2)			;Clear biases
		CLR.B	NDI_F2ADJ(A2)
		CLR.B	NDI_F3ADJ(A2)
		CLR.B	NDI_A1ADJ(A2)
		CLR.B	NDI_A2ADJ(A2)
		CLR.B	NDI_A3ADJ(A2)
		MOVE.B	#DEFARTIC,NDI_ARTICULATE(A2)	;Normal articulation
		MOVE.B	#DEFCENTRAL,NDI_CENTRALIZE(A2)	;No centralization
		CLR.L	NDI_CENTPHON(A2)		;Centralize to vapor
		CLR.B	NDI_AVBIAS(A2)			;No extra AV bias
		CLR.B	NDI_AFBIAS(A2)			;No extra AF bias
		MOVE.B	#DEFPRIORITY,NDI_PRIORITY(A2)	;Default priority
		JSR	DEVDBG_StoreParms		;Store pointer to Parms
		BRA.S	NewIORB				;Do not clear flags

OldIORB		CLR.B	NDI_FLAGS(A2)			;Clear flags
NewIORB		CLR.B	IO_ERROR(A2)			;Set noErr
		MOVE.L	A6,IO_DEVICE(A2)		;Save device node ptr


*		;------	New Opens clear the Expunge bit
		LEA	ND_UNIT(A6),A3			;Unit pointer
		BCLR	#UNITB_EXPUNGE,UNIT_FLAGS(A3)	;Clear Expunge bit


Open_End	MOVEM.L	(SP)+,A2/A3/A6
		RTS

Open_Err:
		MOVE.B	D0,IO_ERROR(A2)			;Return error
		MOVE.L	#-1,IO_DEVICE(A2)		;Invalidate devNode ptr
		BRA.S	Open_End




******* Device/Speech/Read **********************************
*
*   NAME
*	Read - Return the next different mouth shape from an 
*       associated write
*
*   SYNOPSIS
*	Standard device command.  See DoIO/SendIO.
*
*   FUNCTION
*	The read command of the narrator device returns mouth
*	shapes to the user.  The shape returned is guaranteed
*	to be differnt from the previously returned shape 
*	(allowing updating to be done only when something has
*	changed).  Each read request is associated with a 
*	write request by the pseudo-unit number assigned by
*	the OpenDevice call.  Since the first structure in
*	the read-mouth IORB is a narrator (write) IORB, this
*	association is easily made by copying the narrator 
*	IORB into the narrate_rb field fo the read IORB.
*	See the .hi files.  If there is no write in progress
*	or in the device input queue with the same pseudo-
*	unit number as the read request, the read will be
*	returned to the user with an error.  This is also
*	how the user knows that the write request has 
*	finished and that s/he should not issue any more
*	reads.  Note that in this case the mouth shapes may
*	not be different from previously returned values.
*
*   INPUTS
*	IORB with the narrator_rb structure copied from the
*	associated write request execpt for:
*	   io_Message - message port for read request
*	   io_Command - CMD_READ
*	   io_Error   - 0
*	   width      - 0
*	   height     - 0
*
*   RESULTS
*	IORB fields set:
*	   width  - mouth width in millimeters/3.67
*		    (division done for scaling)
*	   height - mouth height in millimeters
*	   shape  - compressed form of mouth shapes
*	  	    (internal use only)
*
*   SEE ALSO
*	Write command.
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*

Read:
		MOVEM.L	A3/A6,-(SP)
		MOVE.L	IO_DEVICE(A1),A3
		MOVE.L	_AbsExecBase,A6

*		;------	Clear IOB_QUICK.  This will cause DoIO to wait
		BCLR	#IOB_QUICK,IO_FLAGS(A1)
		CLR.B	IO_ERROR(A1)				;Note 1

		
*		;------	If no write request is in progress, and the unit
*		;	is not stopped, return an error.
		LEA	ND_UNIT(A3),A0
		BTST	#UNITB_STOP,UNIT_FLAGS(A0)
		BNE.S	Read_AddTail
		MOVE.L	ND_USERIORB(A3),D0
		BNE.S	Read_withWrite
		MOVE.B	#ND_NoWrite,IO_ERROR(A1)
		BSR	TermIO
		BRA	Read_Rtn


*		;------	A write is in progress, is it the one associated with
*		;	this read?  If not, add the read request to the end of
*		;       the input queue.
Read_withWrite:
		MOVE.L	D0,A0	
		MOVE.L	IO_UNIT(A0),D0
		CMP.L	IO_UNIT(A1),D0
		BEQ.S	Read_sameUnit
Read_AddTail	LEA	ND_UNIT+MP_MSGLIST(A3),A0
		DISABLE
		ADDTAIL
		ENABLE
		BRA.S	Read_Rtn


*		;------	The read and write requests match.  If the mouths have 
*		;	changed, return the new shapes.  If not, add to head
*		;	of input queue.  If the unit is stopped, add the read
*		;	request to the head of the input queue.
Read_sameUnit:
		LEA	ND_UNIT(A3),A0
		BTST	#UNITB_STOP,UNIT_FLAGS(A0)
		BNE.S	Read_Stopped

		TST.B	NDI_MOUTHS(A1)		;If not mouth read, its word
		BEQ.S	Read_Stopped		;   or syllable sync, queue it

		MOVE.B	MRB_SHAPE(A1),D0
		MOVE.L	ND_GLOBALS(A3),A0
		CMP.B	CURMOUTH(A0),D0		;wrong!! curmouth should not
						;be in globals, but in iorb
		BNE.S	Read_NewMouth

Read_Stopped	LEA	ND_UNIT+MP_MSGLIST(A3),A0
		DISABLE
		ADDHEAD
		ENABLE
		BRA.S	Read_Rtn


*		;------	Unpack and return new mouth values 
Read_NewMouth:
		MOVE.B	CURMOUTH(A0),D0
		MOVE.B	D0,D1
		MOVE.B	D0,MRB_SHAPE(A1)
		AND.B	#$0F,D0
		MOVE.B	D0,MRB_WIDTH(A1)
		LSR.B	#4,D1
		MOVE.B	D1,MRB_HEIGHT(A1)
		CLR.B	IO_ERROR(A1)
		BSR	TermIO
		
Read_Rtn	MOVEM.L	(SP)+,A3/A6
		RTS



******* Device/Speech/Write ****************************************
*
*   NAME
*	Write - Send speech request to the narrator device
*
*   SYNOPSIS
*	Standard device command.  See DoIO/SendIO.
*
*   FUNCTION
*	Performs the speech request.  If there is an
*	associated read request on the device input queue,
*	write will remove it and return an initial mouth
*	shape to the user.  Note that if you are going
*	to be doing reads, the mouths parameter must be
*	set to 1.
*
*   INPUTS
*	Narrator IORB
*	   ch_masks - array of audio channel selection masks
*		      (see audio device documentation for
*		       description of this field)
*	   nm_masks - number of audio channel selection masks
*	   mouths   - 0 if no mouths are desired
*		      1 if mouths are to be read
*	   rate     - speaking rate
*	   pitch    - pitch
*	   mode     - pitch mode
*	              0 if natural mode
*		      1 if robotic mode
*	   sex      - 0 if male
*	  	    - 1 if female
*	   io_Message - message port
*	   io_Command - CMD_WRITE
*	   io_Data    - input string
*	   io_Length  - length of input string
*	   
*   RESULTS
*	The function sets the io_Error field of the IORB.  The
*	io_Actual field is set to the length of the input string
*	that was actually processed.  If the return code indicates
*	a phoneme error (ND_PhonErr), io_Actual is the position in
*	the input string where the error occured.
*
*   SEE ALSO
*	Read command.
*	Audio device documentation.
*
*********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*

Write:		JMP	Narrator		;Does not return  	Note 3


		MOVE.L	A2,-(SP)


*		;------	Is a read request for this write request is on the queue
Write_StartLoop:
		DISABLE	A0
		MOVE.L	IO_UNIT(A1),D0
		LEA	ND_UNIT+MP_MSGLIST(A3),A0
		MOVE.L	(A0),D1
Write_Loop:	
		MOVE.L	D1,A2
		MOVE.L	(A2),D1
		BEQ.S	Write_NoRead
		CMP.W	#CMD_READ,IO_COMMAND(A2)
		BNE.S	Write_Loop
		CMP.L	IO_UNIT(A2),D0
		BNE.S	Write_Loop


*		;------	Found a read request on the queue.  Start it going.
Write_FoundRead:
		MOVE.L	A1,-(SP)		;Save write IORB
		MOVE.L	A2,A1			;Copy read IORB ptr
		REMOVE				;Delink read IORB
		ENABLE  A0
		MOVE.L	A2,A1			;Move read IORB ptr 
		MOVE.B	#0,IO_ERROR(A1)		;No error
		BSR	TermIO			;Reply to user
		MOVE.L	(SP)+,A1		;Restore write IORB
		BRA.S	Write_StartLoop		;Look for more reads



*		;------ Startup the write request
Write_NoRead:
		ENABLE	A0
		MOVE.L	(SP)+,A2
		JMP	Narrator		;!!!! DOES NOT RETURN !!!!!
		


******* Device/Speech/Close ***************************************
*
*   NAME
*	CloseDevice - terminates access to the narrator device
*
*   SYNOPSIS
*	CloseDevice(IORequest)
*
*   FUNCTION
*	Close invalidates the IO_UNIT and IO_DEVICE fields in the
*	IORB, preventing subsequent IO until another OpenDevice.
*	CloseDevice also reduces the open count.  If the count 
*	goes to 0 and the expunge bit is set, the device is 
*	expunged.  If the open count goes to zero and the delayed
*	expunge bit is not set, CloseDevice sets the expunge bit.
*
*   INPUTS
*	IORequest block
*
*   RESULTS
*	IORequest block with unit and device pointers invalidated.
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTAION
*
*


Close:
	MOVEM.L	A2/A3/D2/D3,-(SP)
	LEA	ND_UNIT(A6),A3		;Get unit pointer


*	;------	Remove all IO requests with same pseudo-unit
	MOVE.L	IO_UNIT(A1),D2			;Get pseudo unit number
*	DISABLE	A0
	LEA	MP_MSGLIST+LH_HEAD(A3),A2	;List head
	MOVE.L	(A2),D3				;Pointer to first node

CloseLoop:
	MOVE.L	D3,A2				;Abort each IO request
	MOVE.L	(A2),D3
	BEQ.S	CloseDone
	MOVE.L	A2,A1
	CMP.L	IO_UNIT(A1),D2
	BNE.S	CloseLoop
	BSR	AbortIO
	BRA.S	CloseLoop

CloseDone:
*	ENABLE	A0


*	;------	Reduce device open count.  If last close, jump to Expunge
	MOVEQ	#0,D0				;RC if not last close
*	DISABLE	A0
	SUBQ.W  #1,LIB_OPENCNT(A6)
	BNE.S	Close_NotLast

*	;------	Release fixed buffers.		;!!!sam 11/20/85!!!
	MOVE.L	A6,A2				;!!!sam 11/20/85!!!
	MOVE.L	_AbsExecBase,A6			;!!!sam 11/20/85!!!
	MOVE.L  ND_GLOBALS(A2),A1		;!!!sam 11/20/85!!!
        MOVE.L  #FIXBFRSZ,D0			;!!!sam 11/20/85!!!
	CALLSYS	FreeMem				;!!!sam 11/20/85!!!

*	;------	Close audio device		;!!!sam 11/20/85!!!
	LEA	ND_IORB3(A2),A1			;!!!sam 11/20/85!!!
	CALLSYS	CloseDevice			;!!!sam 11/20/85!!!
	MOVE.L	A2,A6				;!!!sam 11/20/85!!!

	BTST	#UNITB_EXPUNGE,UNIT_FLAGS(A3)
	BEQ.S	Close_NotLast
	MOVE.L	A1,-(SP)			;******* c test only
	BSR	Expunge				;Returns seglist ptr in D0
	MOVE.L	(SP)+,A1			;******* c test only

Close_NotLast:
*	ENABLE	A0
	MOVEM.L	(SP)+,A2/A3/D2/D3
	RTS



* ****** Device/Speech/PerformIO ***********************************
*
*   NAME
*	PerformIO -- do an actual IO request
*
*   SYNOPSIS
*	PerformIO( iORequest ), DevPtr, SysLib
*		   A1	
*
*   FUNCTION
*	PerformIO is called by the device task when a new IO
*	request comes in.  Handles write requests only.  This
*	is a leftover from the earlier days of exec.
*
*
*   INPUTS
*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
* *********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*	
*

PerformIO:	
	MOVE.W		IO_COMMAND(A1),D0	;GET COMMAND
	BLE		NoIO			;INVALID
	CMP.W		#CMD_FLUSH,D0
	BGT		NoIO			;INVALID
	LSL.W		#2,D0			;CONVERT TO LONG OFFSET
	LEA		CommandTbl,A0
	MOVE.L		0(A0,D0.W),A0
	JSR		(A0)

Perform_End:
	RTS




* ****** Device/Speech/Expunge ***********************************
*
*   NAME
*	Expunge - Removes device and memory from system
*
*
*   SYNOPSIS
*	Expunge(devNode)
*		   A6
*
*
*   FUNCTION
*	Expunge is called by RemDevice, not by the user. 
*	If the open count is non-zero, Expunge sets the delayed
*	expunge bit in the unit flags byte and returns.  If
*	the open count is zero, Expunge will remove all memory
*	allocated for the device and remove the device from
*	the system list.
*
*
*   INPUTS
*
*
*   RESULTS
*	Retuns 0 if open count is not 0, returns seglist pointer
*	if the expunge happens.
*
*
*   SEE ALSO
*
* *********************************************************************
*
*   REGISTER USAGE
*
*  
*   IMPLEMENTATION
*
   
Expunge:
	MOVEM.L	A3/A6/D3,-(SP)
	MOVE.L	A6,A3
	MOVE.L	_AbsExecBase,A6



	LEA	ND_UNIT(A3),A0				;Unit pointer

*	;------ Last close?  If so, release fixed buffers
	TST.W	LIB_OPENCNT(A3)
	BEQ.S	ExDev
	BSET	#UNITB_EXPUNGE,UNIT_FLAGS(A0)		;Set delayed expunge bit
	MOVEQ	#0,D0					;Return code	
	BRA	ExReturn

ExDev:
*	DISABLE
*	;------	Release fixed buffers.
*	MOVE.L  ND_GLOBALS(A3),A1			;!!!sam 11/20/85!!!
*	MOVE.L  #FIXBFRSZ,D0				;!!!sam 11/20/85!!!
*	CALLSYS	FreeMem					;!!!sam 11/20/85!!!


*	;------	Close audio device
*	LEA	ND_IORB3(A3),A1				;!!!sam 11/20/85!!!
*	CALLSYS	CloseDevice				;!!!sam 11/20/85!!!


*	;------	Remove my task
	LEA	ND_TCB(A3),A1
	CALLSYS	RemTask


*	;------	Remove the device
	MOVE.L	A3,A1
	REMOVE


*	;------	Remove audio control msgport
	MOVE.L	ND_CMSGPORT(A3),A1
	MOVE.L	#MP_SIZE,D0
	CALLSYS	FreeMem


*	;------	Remove device node
	MOVE.L	ND_SEGLIST(A3),D3
	MOVE.L	A3,A1
	MOVEQ	#0,D0			*!!!sam 8/31/85!!!
	MOVEQ	#0,D1			*!!!sam 8/31/85!!!
	MOVE.W	LIB_NEGSIZE(A3),D0	*!!!sam 8/31/85!!!
	SUB.L	D0,A1			*!!!sam 8/31/85!!!
	MOVE.W	LIB_POSSIZE(A3),D1	*!!!sam 8/31/85!!!
	ADD.L	D1,D0			*!!!sam 8/31/85!!!
*	MOVE.L	#ND_SIZE,D0		*!!!sam 8/31/85!!!
	CALLSYS	FreeMem
	MOVE.L	D3,D0
*	ENABLE

ExReturn:	
	MOVEM.L	(SP)+,A3/A6/D3
	RTS



* ****** Device/Speech/BeginIO ***********************************
*
*   NAME
*	BeginIO -- do an actual IO request
*
*
*   SYNOPSIS
*	BeginIO( iORequest ), devNode
*		   A1		A6
*
*
*   FUNCTION
*	BeginIO receives driver requests from the user and
*	either processes them immediately, or queues them
*	for later.  Currently only writes are queued.
*
*
*   INPUTS
*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
*
* *********************************************************************
*
*   REGISTER USAGE
*
*  
*   IMPLEMENTATION
*

BeginIO:
	LEA	ND_UNIT(A6),A0			;Compute msgport pointer
	MOVE.B	#NT_MESSAGE,LN_TYPE(A1)		;Set type
	BSET	#IOB_ACTIVE,IO_FLAGS(A1)	;Set IORB active bit

	BCLR	#IOB_QUICK,IO_FLAGS(A1)

	CMP.W	#CMD_STOP,IO_COMMAND(A1)
	BEQ	StopIO
	CMP.W	#CMD_START,IO_COMMAND(A1)
	BEQ.S	StartIO
	CMP.W	#CMD_FLUSH,IO_COMMAND(A1)
	BEQ	FlushIO
	CMP.W	#CMD_RESET,IO_COMMAND(A1)
	BEQ	ResetIO
	CMP.W	#CMD_READ,IO_COMMAND(A1)
*	BEQ	Read						;	Note 2

	BTST	#UNITB_SHUT,UNIT_FLAGS(A0)
	BEQ	BeginOK
	MOVE.B	#IOERR_ABORTED,IO_ERROR(A1)
	MOVEM.L	A3/A6,-(SP)
	MOVE.L	A6,A3
	MOVE.L	_AbsExecBase,A6
	BSR	TermIO
	MOVEM.L	(SP)+,A3/A6
	RTS

BeginOK:
	LINKSYS PutMsg

	RTS	



******* Device/Speech/StartIO StopIO ****************************
*
*   NAME
*	StopIO  - Stops the device.
*	StartIO - Restarts the device after StopIO
*
*   SYNOPSIS
*	Standard device commands.  See DoIO/SendIO
*
*   FUNCTION
*	StopIO halts the currently active speech (if any) and
*	prevents any queued requests from starting. 
*
*	StartIO restarts the currently active speech (if any)
*	and allows queued request to start.
*
*   INPUTS
*	io_Command = CMD_STOP or CMD_START
*
*   RESULTS
*
*   SEE ALSO
*
*
*
*
**********************************************************************
*
*   REGISTER USAGE
*
*  
*   IMPLEMENTATION
*

StartIO:
	BCLR	#UNITB_STOP,UNIT_FLAGS(A0)
	MOVE.L	A1,-(SP)
	LEA	ND_IORB3(A6),A1
	MOVE.W	#CMD_START,IO_COMMAND(A1)
	BSET	#IOB_QUICK,IO_FLAGS(A1)
	LINKSYS	DoIO
	LEA	ND_TCB(A6),A1
	MOVE.L	ND_SIGNALS(A6),D0
	LINKSYS	Signal
	MOVE.L	(SP)+,A1
	BRA.S	SReturn


StopIO:
	BSET	#UNITB_STOP,UNIT_FLAGS(A0)	;Bit used to stop new requests
	MOVE.L	A1,-(SP)	
	LEA	ND_IORB3(A6),A1			;Get command IORB
	MOVE.W	#CMD_STOP,IO_COMMAND(A1)	;Set STOP
 	BSET	#IOB_QUICK,IO_FLAGS(A1)		;Set quick bit
	LINKSYS	DoIO
	MOVE.L	(SP)+,A1
	
SReturn:
	CLR.B	IO_ERROR(A1)		;Set no error
	MOVEM.L	A3/A6,-(SP)
	MOVE.L  A6,A3
	MOVE.L	_AbsExecBase,A6 
	BSR	TermIO
	MOVEM.L	(SP)+,A3/A6		;Fixup and leave	
	RTS




******* Device/Speech/ResetIO ***********************************
*
*   NAME
*	Reset - Reset the device to a known state
*
*   SYNOPSIS
*	Standard device command.  See DoIO/SendIO.
*
*   FUNCTION
*	Resets the device as though it has just be initialized.
*	Aborts all read/write requests whether active of enqueued.
*	Restarts device if it has been stopped.
*
*   INPUTS
*	io_Command = CMD_RESET
*
*   RESULTS
*
*   SEE ALSO
*
*
*
*
**********************************************************************
*
*   REGISTER USAGE
*
*  
*   IMPLEMENTATION
*

ResetIO:

	MOVEM.L	A2/A3/A4/A6/D3,-(SP)
	MOVE.L	A0,A3
	MOVE.L	A1,A4

*	;------	Shut driver
	BSET	#UNITB_SHUT,UNIT_FLAGS(A3)	;Mark unit as shut down

	BSR	FlushAll

*	;------	Make sure the device is started.  Also does TermIO.
	MOVE.L	A4,A1				;IORB
	MOVE.L	A3,A0				;Unit
	MOVE.L	IO_DEVICE(A1),A6		;devNode
	BSR	StartIO

*	;------ Re-open driver to input
	BCLR	#UNITB_SHUT,UNIT_FLAGS(A3)

	MOVEM.L	(SP)+,A2/A3/A4/A6/D3		;Fixup and leave	
	RTS



******* Device/Speech/FlushIO ***********************************
*
*   NAME
*	Flush - Aborts all inprogress and queued requests
*
*   SYNOPSIS
*	Standard device command.  See DoIO/SendIO
*
*   FUNCTION
*	Aborts all inprogress and queued speech requests.
*
*   INPUTS
*	io_Command - CMD_FLUSH
*
*   RESULTS
*
*   SEE ALSO
*
*
*
*
**********************************************************************
*
*   REGISTER USAGE
*
*  
*   IMPLEMENTATION
*

FlushIO:

	MOVEM.L	A2/A3/A4/A6/D3,-(SP)
	MOVE.L	A0,A3
	MOVE.L	A1,A4

*	;------	Shut driver
	BSET	#UNITB_SHUT,UNIT_FLAGS(A3)	;Mark unit as shut down

	BSR	FlushAll

*	;------ Re-open driver to input
	BCLR	#UNITB_SHUT,UNIT_FLAGS(A3)

	
	MOVE.L	A4,A1
	CLR.B	IO_ERROR(A1)		;Set no error
	MOVE.L  IO_DEVICE(A1),A3
	MOVE.L	_AbsExecBase,A6 
	BSR	TermIO
	MOVEM.L	(SP)+,A2/A3/A4/A6/D3		;Fixup and leave	
	RTS

FlushAll:
*	;------	Abort anything in progress
	MOVE.L	ND_USERIORB(A6),D0		;Get in progress IORB
	BEQ.S	FlushQ				;Nothing in progress
	MOVE.L	D0,A1				;Kill in progress request
	BSR	AbortIO


*	;------	Clear out the msgport queue
FlushQ:
	DISABLE	A0
	LEA	MP_MSGLIST+LH_HEAD(A3),A2	;List head
	MOVE.L	(A2),D3				;Pointer to first node

FlushLoop:
	MOVE.L	D3,A2				;Abort each IO request
	MOVE.L	(A2),D3
	BEQ.S	FDone
	MOVE.L	A2,A1
	BSR	AbortIO
	BRA.S	FlushLoop

FDone	ENABLE	A0
	RTS

	

******* Device/Speech/AbortIO ****************************************
*
*   NAME
*	AbortIO - Abort an IO request
*
*   SYNOPSIS
*	AbortIO(IORequest)
*
*   FUNCTION
*	Aborts a speech IO request.  The request may be in the queue
*       or currently active.
*
*   INPUTS
*	IORB of request to abort.
*
*   RESULTS
*	io_Error field of IORB set to IOERR_ABORTED
*
*   SEE ALSO
*
*
**********************************************************************
*
*

AbortIO:
	MOVEM.L	A3/A6,-(SP)

	MOVE.L	A6,A3
	MOVE.L	_AbsExecBase,A6
	DISABLE

	LEA	ND_UNIT(A3),A0			;Get msgport pointer
	LEA	MP_MSGLIST+LH_HEAD(A0),A0	;Pointer to first node


*	;------ Are we in progress?  If so, set a bit for Synth to look at.

	CMP.L	ND_USERIORB(A3),A1		;Currently running in Synth?
	BNE.S	AbloopQ				;No, check queue
	BSET	#IOB_ABORT,IO_FLAGS(A1)		;Yes, set the abort bit
	BRA.S	AbNotFound			;Fixup and leave


*	;------	Not in progress, are we still on the queue?

AbloopQ:
	MOVE.L	(A0),A0				;Get first node
Abloop	MOVE.L	(A0),D0				;Get node successor
	BEQ.S	AbNotFound			;Not found
	CMP.L	A0,A1				;Node to Abort?
	BEQ.S	AbFound				;Yes
	MOVE.L	D0,A0				;No, keep looking
	BRA.S	Abloop

AbFound	MOVE.L	A1,D0				;Remove node from queue
	REMOVE
	MOVE.L	D0,A1
	MOVE.B	#IOERR_ABORTED,IO_ERROR(A1)	;Set return code

	ENABLE
	BSR	TermIO
	BRA.S	AbReturn

AbNotFound:
	ENABLE

AbReturn:
	MOVEM.L	(SP)+,A3/A6
	RTS

		

* ****** Device/Speech/Null ****************************************
*
*   NAME
*	Null - 
*
*   SYNOPSIS
*	Zero = Null(), DevNode
*	D0	       A6
*
*   FUNCTION
*	Be a constant source of zero's for unimplemented routines.
*
*   RESULTS
*	Zero -- Always return 0 in D0
*
*
*   SEE ALSO
*	SLNullFunc
*
*
* *********************************************************************

ExtFunc:
Null:
		MOVEQ	#0,D0
		RTS
NoIO:
		MOVE.B	#ND_Unimpl,IO_ERROR(A1)
		BSR	TermIO
		RTS

UpdateIO:
ClearIO:
		CLR.B	IO_ERROR(A1)
		BSR	TermIO
		RTS




* ****** Device/TermIO ************************************************
*
*   NAME
*	TermIO - Return an IO request to the user.
*
*
*   SYNOPSIS
*	TermIO( iORequest ), devLib, SysLib
*		A1	     A3        A6     
*
*
*   FUNCTION
*	TermIO does all the necessary cleanup after an I/O
*	Request block has been completed.  It will mark it
*	as done, send it back to the originating task.
*
*
*   INPUTS
*	iORequest -- a pointer to the I/O Request Block
*
* *********************************************************************
*
*   NOTE
*	This routine would normally be found in the device support
*	library.  Currently it does not have a permanent place to
*	live, and is part of each driver's address space.  This
*	will be fixed as we get the device support library nailed
*	down.
*


TermIO:
		LEA	ND_UNIT(A3),A0

*		;------ Clear unit active flag
		BCLR	#UNITB_ACTIVE,UNIT_FLAGS(A0)

*		;------ Clear all flag bits
		MOVE.B	#0,IO_FLAGS(A1)

*		;------ and send it back
		CALLSYS ReplyMsg


Term_End:
		RTS



		XDEF devStructInit
devStructInit:



*		;------ Initialize the device
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,NarratorName
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_OPENCNT,0
		INITBYTE	ND_UNIT+MP+LN_TYPE,NT_MSGPORT
		INITLONG	ND_UNIT+MP+LN_NAME,NarratorName

 		INITBYTE	ND_UNIT+UNIT_FLAGS,0


		INITBYTE	ND_TCB+LN_TYPE,NT_TASK
		INITLONG	ND_TCB+LN_NAME,NarratorName
		INITBYTE	ND_TCB+LN_PRI,0

		
		DC.L		0


*		;------ Device function table

		XDEF devFuncInit
devFuncInit:
		DC.L	Open		; - 6
		DC.L	Close		; - C
		DC.L	Expunge		; -12 
		DC.L	ExtFunc		; -18
		DC.L	BeginIO		; -1E
		DC.L	AbortIO		; -24
		DC.L	-1		; END OF TABLE MARKER

devFuncEnd:

devFuncSize	EQU	(devFuncEnd-devFuncInit)


*		;------ Device command jump table

CommandTbl	DC.L	NoIO		;invalid
		DC.L	ResetIO		;reset
		DC.L	Read		;read
		DC.L	Write		;write
		DC.L	UpdateIO	;update
		DC.L	ClearIO		;clear
		DC.L	StopIO		;stop
		DC.L	StartIO		;start
		DC.L	FlushIO		;flush
		DC.L	NoIO		;finish


cportname	DC.B	'command port',0
		CNOP	0,2


*		;------ Sam's audio library
AudioLib	STRING	'audio.device'


EndNarrCode:	END



