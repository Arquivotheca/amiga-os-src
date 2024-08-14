	FAR	DATA
	TTL	'SYNTH.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

           SECTION      speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                        ;
*                 MAIN MODULE 'SYNTH'                    ;
*                                                        ;
* SYNTHESIZES VOCAL WAVEFORM IN REAL TIME FROM DATA THAT ;
*       IS CONTAINED IN THE COEFFICIENT BUFFER.          ;
*                                                        ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
*           REGISTER USAGE
*
* D0 HALFPITCH,PITCH    D4 FLAGS,MICFRAME
* D1 AMP1               D5 FRIC INDEX
* D2 AMP2               D6 SCRATCH
* D3 HALFAMP3,AMP3      D7 OUTPUT
*
* A0 OUTPUT BUFFER++    A4 MULT
* A1 F1REC              A5 VARIABLES
* A2 F2REC              A6 COEF. BUFFER
* A3 F3REC
*
* COEF. BUFFER = F1,F2,F3,AMP1,AMP2,AMP3,FF,F0...
*

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

_AbsExecBase	EQU	4
io_Size		EQU	IO_SIZE

        INCLUDE    'gloequs.i'
	INCLUDE	   'devices/audio.i'
	INCLUDE    'narrator.i'
	INCLUDE    'private.i'

        EXTERN_SYS AbortIO
        EXTERN_SYS DoIO
        EXTERN_SYS AllocMem
        EXTERN_SYS GetMsg
        EXTERN_SYS Disable
        EXTERN_SYS Enable
        EXTERN_SYS WaitIO
        EXTERN_SYS ReplyMsg

	EXTERN_DATA _intena

        XDEF       SYNTH,DMANodeName
        XREF       MULT,FRICtable,F1table,F2table,F3table
	XREF	   SYNDBG_AllocBuffer,SYNDBG_SaveOutput


UnitFlags       EQU     F0START
COEFWORDBIT	EQU	$01		;Word sync bit in coef bfr (see phonet)
COEFSYLBIT	EQU	$02		;Syl sync bit in coef bfr
SYNCFLAGS	EQU	$06		;Mast for identifying word and syl sync

BUFFLEN		EQU	OUTBUFF2-OUTBUFF1


* Bits in the hi word of d4
NASAL		equ	20
ASPIR		equ	21
FRIC		equ	22
VOICED		equ	23
RECEND		equ	28

*	Offsets into a frame of coef -- n(a6)
F1		equ	0
F2		equ	1
F3		equ	2
AMP1		equ	3
AMP2		equ	4
AMP3		equ	5
FLAGS		equ	6
F0		equ	7

SYNTH:

	MOVE.L	_AbsExecBase,A6

*	;------	Cache the address of IO_FLAGS
	MOVE.L	USERIORB(A5),A2
	MOVE.L	IO_DEVICE(A2),A4
	LEA	ND_UNIT+UNIT_FLAGS(A4),A3
	MOVE.L	A3,UnitFlags(A5)


*	;------	Check to see if we've been aborted
*	;	CAUTION....A2, not A1 -> IORB.  Also,
*	;	other regs may not be correctly setup.
*	;	Check carefully if this code is put
*	;	back into action.
*	BTST	#IOB_ABORT,IO_FLAGS(A2)
*	BNE	SYABORTED


        MOVE.L    #3579000,D0      ;COLOR CLOCK
        DIVU      SampFreq(A5),D0  ;DIVIDE BY SAMPLING FREQ
        MOVE.W    D0,AudPer(A5)    ;SAVE AS PERIOD

	BSET	#IOB_FIRST,IO_FLAGS(A2)		;First time bit

*	;------	Stop all audio DMA channels.  This code was
*	;	originally in MultChan (qv) but was moved here
*	;	because on overflows the processing loop does
*	;	not contain the allocation and freeing of audio
*	;	channels.  This should cure the loss of stereo
*	;	sync when overflow occurs.
	LEA	ND_IORB3(A4),A1	
	MOVE.B	#ADIOF_NOWAIT+IOF_QUICK,IO_FLAGS(A1)
	MOVE.L	NDI_CHMASKS(A2),ioa_Data(A1)
	MOVE.W	NDI_NUMMASKS(A2),ioa_Length+2(A1)
	MOVE.W	#CMD_STOP,IO_COMMAND(A1)
	CALLSYS	DoIO

*	;------	Setup DMA IORBs
	MOVE.L	ND_AUDDMAIORB(A4),A1
	LEA	DMANodeName,A2
	MOVE.L	A2,IO+LN_NAME(A1)
	MOVE.L	ND_MSGPORT(A4),A3
	MOVE.L	A3,IO+MN_REPLYPORT(A1)
	MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
	MOVE.W	#BUFFLEN,ioa_Length+2(A1)
	MOVEQ	#0,D0
	MOVE.B	Volume(A5),D0
	MOVE.W	D0,ioa_Volume(A1)
	MOVE.W	#1,ioa_Cycles(A1)
	MOVE.W	AudPer(A5),ioa_Period(A1)
	LEA	OUTBUFF1(A5),A2
	MOVE.L	A2,ioa_Data(A1)
	MOVE.B	#ADIOF_PERVOL,IO_FLAGS(A1)

	MOVE.L	A1,A0
	LEA	ioa_Size(A1),A2
	MOVE.L	USERIORB(A5),A3
	MOVE.B	NDI_NUMCHAN(A3),D0
	ADD.W	D0,D0
	SUBQ.W	#2,D0					;Adjust for DBF

SYN_IORBLoop:
	MOVE.W	#(ioa_Size/2)-1,D1			;Copy word at a time
	MOVE.L	A0,A1					;UNNECESSARY

SYN_CopyLoop:
	MOVE.W	(A1)+,(A2)+
	DBF	D1,SYN_CopyLoop

	DBF	D0,SYN_IORBLoop


*	;------	Set address of buffer in 2nd set
*	;	of IORBs
	MOVE.L	ND_AUDDMAIORB(A4),A1		;Point to 2nd set
	MOVEQ	#0,D0
	MOVE.B	NDI_NUMCHAN(A3),D0
	MOVE.W	D0,D1				;Save for DBF
	MULU	#ioa_Size,D0
	ADD.L	D0,A1
	LEA	ioa_Data(A1),A1			;Where to put bfr addr

	LEA	OUTBUFF2(A5),A2			;Buffer address
	SUBQ.W	#1,D1				;Number of IORBs

SYN_ALoop:
	MOVE.L	A2,(A1)
	LEA	ioa_Size(A1),A1			;Next IORB
	DBF	D1,SYN_ALoop
	

*	;------	Set the channel mask in the IORB

	MOVE.L	ND_AUDDMAIORB(A4),A1
	LEA	IO_UNIT+3(A1),A1
	MOVEQ	#1,D2

SYN_CM:
	MOVEQ	#3,D0
	MOVE.B	NDI_CHANMASK(A3),D1

SYN_CLoop:
	BTST	D0,D1
	BEQ.S	SYN_CDBF
	CLR.B	(A1)
	BSET	D0,(A1)
	LEA	ioa_Size(A1),A1
SYN_CDBF:
	DBF	D0,SYN_CLoop

	DBF	D2,SYN_CM

*	Here starts SYNTH

	 MOVE.L	  MOTHPTR(A5),CURMOUTH(A5)	;Current mouth shape bfr pos

	 LEA	  MULT,A4	  		;Address of multiplication table
         MOVE.L   COEFPTR(A5),A6  		;Point to coef buffer
 	 CLR.B	  SYNC(A5)			;Clear word/syl sync indicator

         MOVE.L   A5,-(SP)
         MOVEQ    #VARLEN-1,D0    		;CLEAR SYNTH VARIABLES
SYNINIT  CLR.W    (A5)+
         DBF      D0,SYNINIT
         MOVE.L   (SP)+,A5

*
* COMPUTE SAMPLES/FRAME
*
	MOVE.W	  SampFreq(A5),D1  		;NORMALLY 22200
        MULU      #75,D1           		;TRACK RATE (150 wpm is nominal)
        DIVU      RATE(A5),D1
        AND.L     #$FFFF,D1
        DIVU      #60,D1
        LSR.W     #1,D1           		;SAMPERFRAME(A5) IS IN WORDS
*	lsl.w	  #2,d1				;slow down frame rate for test
  	MOVE.W	  D1,SAMPERFRAME(A5)          

	JSR	  SYNDBG_AllocBuffer		;Allocate buffer for synth output

        MOVE.L    COEFPTR(A5),A6  		;RESTORE POINTER TO COEF BUFFER 

	MOVEQ     #0,D0           		;CLEAR EVAHBODY
        MOVE.L    D0,D1
        MOVE.L    D0,D2
        MOVE.L    D0,D3
        MOVE.L    D0,D4
        MOVE.L    D0,D5
        MOVE.L    D0,D6
        MOVE.L    D0,D7


*
* Initialize some variables
*
	move.w	#BUFFLEN/2,SAMPCNT(a5)	;init sample count
	lea	OUTBUFF1(a5),a0		;use first buffer
	clr.w	BUFFLAG(a5)		;   and don't switch yet
	move.w	SAMPERFRAME(a5),d4
	move.w	#127,RECLEN(a5)
	move.w	#511,FRICLEN(a5)
	move.w	#1,FRINC(A5)    	;initialize fric recording direction +
	move.w	#1,UPDATE(a5)		;update to 1st frame
	lea	F1table,a1		;so 1st frame makes sense
	lea	F2table,a2
	lea	F3table,a3
	lea	MULT,a4


*
* Branch to frame update for 1st frame
*
	bra	syn4a

*
* Synthesis routines
*
	include 'syn.i'


*	;------ New pitch pulse code.

NewPulse	TST.W	UPDATE(A5)		;Update frame?
		BEQ	FUbypass		;  No, skip frame update

*
* Frame update
*
nxtframe:
	addq.l	#8,a6		;point to next frame

syn4a:
	MOVE.B  FLAGS(A6),D7	;Added for word/syllable sync	    * 4/12/89
	AND.B	#SYNCFLAGS,D7	;Isolate word/syllable sync flags   * 4/12/89
	OR.B	D7,SYNC(A5)	;Store in globals area		    * 4/12/89
	ADDQ.L	#1,CURMOUTH(A5)	;Update mouth shape buffer position * 4/12/89
	
	moveq	#0,d7
	move.b	(a6),d7		;get f1 index
	bmi	SYNEXIT		;branch if end of coef
	subq.w	#1,UPDATE(a5)	;check to see if we skip a frame
	bne	nxtframe	;branch if we do
	swap	d4		;get the flags from FLAGS(a6)
	clr.w	d4
	move.b	FLAGS(a6),d4
	swap	d4
* F1 update	
FU1	lsl.w	#7,d7		;mult index by 128
	move.l	#F1table,d6	;get F1 base address
	add.l	d6,d7		;point to specific record
	move.l	d7,F1REC(a5)	;save it
* F2 update
	moveq	#0,d7
	move.b	F2(a6),d7
	btst	#FRIC,d4	;fricative?
	bne.s	FU2		;yes, branch
	lsl.w	#7,d7		;no, compute F2 address
	move.l	#F2table,d6	;get F2 base address
	bra.s	FU3
FU2	lsl.w	#6,d7		;CONVERT has shifted 3 already (total 9)
	move.l	#FRICtable,d6	;get FRIC base address
FU3	add.l	d6,d7		;point to specific record
	move.l	d7,F2REC(a5)	;save it
* F3 update
	moveq	#0,d7
	move.b	F3(a6),d7
	btst	#ASPIR,d4	;aspirant?
	bne.s	FU4		;yes, branch
	lsl.w	#7,d7		;no, compute F3 address
	move.l	#F3table,d6	;get F3 base address
	bra.s	FU5
FU4	lsl.w	#6,d7		;CONVERT has shifted 3 already (total 9)
	move.l	#FRICtable,d6	;get FRIC (asp) base address
FU5	add.l	d6,d7		;point to specific record
	move.l	d7,F3REC(a5)
* Get & justify amplitudes
	clr.w	d1		;Clr lower word of D1 (preserves F1 delay elem)
	moveq	#0,d2
	clr.w	d3
	move.b	AMP1(a6),d1
	move.b	AMP2(a6),d2
	move.b	AMP3(a6),d3
	move.w	d2,d7		;save amp2 in scratch
	swap	d2
	move.w	d7,d2		;duplicate amp2 in d2 upper word
	btst	#FRIC,d4	;check for VOICED & FRIC
	beq.s	FU7		;branch if not a fricative
	btst	#VOICED,d4
	beq.s	FU7		;branch if not voiced
	lsr.w	#1,d2		;voiced fric -6db for half a pitch period
FU7	lsl.w	#5,d1		;justify amplitudes
	lsl.l	#5,d2		; for
	lsl.w	#5,d3		; multiplication table
	move.b	F0(a6),PITCH+1(a5) ;get and save pitch period
* End frame update

FUbypass:
	move.l	F1REC(a5),a1	;point back to start
	move.l	F2REC(a5),a2	;     of appropriate
	move.l	F3REC(a5),a3	;         recordings
	move.w	#127,RECLEN(a5)	;reset record length
	bclr	#RECEND,d4	;clear record end flag bit
	move.w	PITCH(a5),d0	;restore pitchcount
	move.w	d0,d7		;pitch to scratch
	lsr.w	#1,d7		;1/2 pitch period
	swap	d0
	move.w	d7,d0		;1/2 pitch period to upper word
	swap	d0
	swap	d2		;position -6db fric amplitude


* Synthesis steering logic
	MOVE.L	D4,D7		;Copy flags to D7
	SWAP	D7		;Get flags in lower word
	AND.W	#$00F0,D7	;Isolate VFAN flags
	LSR.W	#2,D7		;Shift to form index into branch table
				;    index = VFAN  (voiced, fric, asp, nasal)
	ADDQ.W	#2,D7		;**KLUDGE**  MANX 3.6b barfs on "JMP 2(PC,D7)"
	JMP	0(PC,D7.W)	;Jump to the correct table entry

	BRA	SIL		;0000
	BRA	SIL		;0001	   N
	BRA	AH		;0010	  A
	BRA	SIL		;0011	  AN
	BRA	FF		;0100	 F
	BRA	SIL		;0101 	 F N
	BRA	FFAH		;0110	 FA
	BRA	SIL		;0111	 FAN
	BRA	F123		;1000	V
	BRA	AN		;1001	V  N
	BRA	F1AH		;1010	V A
	BRA	SIL		;1011	V AN
	BRA	F1FF		;1100	VF
	BRA	F1FF		;1101	VF N
	BRA	SIL		;1110	VFA
	BRA	SIL		;1111	VFAN

* End steering logic

         DC.B     'COPYRIGHT 1990 MARK BARTON & JOSEPH KATZ'
	 CNOP     0,2

SYNEXIT:
	MOVE.L	_AbsExecBase,A6
	MOVE.L	USERIORB(A5),A0
	MOVE.L	IO_DEVICE(A0),A3
	MOVE.L	ND_AUDDMAIORB(A3),A1
	MOVEQ	#0,D3
	MOVE.B	NDI_NUMCHAN(A0),D3
	MOVE.L	D3,D0
	MULU	#ioa_Size,D0
	TST.W	BUFFLAG(A5)
	BNE.S	SYNEX2
	ADD.L	D0,A1

SYNEX2	SUBQ.W	#1,D3

	MOVE.L	_AbsExecBase,A6
SYNEXL	MOVE.L	A1,-(SP)
	CALLSYS	WaitIO
	MOVE.L	(SP)+,A1
	LEA	ioa_Size(A1),A1
	DBF	D3,SYNEXL

	MOVEQ	#0,D3
	BRA.S	SYN_Return

SYN_AbortIt:
	MOVEQ	  #IOERR_ABORTED,D3

SYN_Return:
	RTS


*************************************************************************
*									*
* 			Output subroutine				*
*									*
*    This subroutine is called from the various systhesis routines.	*
*    It calls subroutines to handle read and write requests.		*
*									*
*    On return:								*
*									*
*    D7 = 0 ==> No error						*
*         1 ==> User abort of write request				*
*									*
*************************************************************************

Output	MOVE.W    #BUFFLEN/2,SAMPCNT(A5)  	;Restore sample count
	SUB.L     #BUFFLEN,A0		  	;Pt back to beginning of buffer
	MOVEM.L   A0-A6/D0-D5,-(SP)		;Save some registers
	MOVE.L	  USERIORB(A5),A3		;Get user's IORB
	MOVE.L	  IO_DEVICE(A3),A4		;Get device pointer
	MOVE.L	  _AbsExecBase,A6		;Setup exec base
	BSR	  DoRead			;Handle user read requests
	BSR	  SYN_Write			;Send buffer to audio device
	MOVEM.L	  (SP)+,A0-A6/D0-D5		;Restore registers
	MOVE.L	  A3,D7				;Save A3
	MOVE.L	  USERIORB(A5),A3		;Get user IORB
	BTST	  #IOB_ABORT,IO_FLAGS(A3)	;Abort request?
	BEQ.S	  OutNoAb			;  No, continue
	MOVEQ	  #1,D7				;  Yes, set cc and return
	BRA.S	  OutRTS				

OutNoAb	JSR	  SYNDBG_SaveOutput		;Save the synth output in buffer

	MOVE.L	  D7,A3				;Restore A3
	LEA       OUTBUFF1(A5),A0       	;Initialize to buffer 1
	NOT.W     BUFFLAG(A5)           	;Invert buffer flag
        BEQ.S     OutCCOK			;If 0, use buffer 1
	LEA       OUTBUFF2(A5),A0       	;Otherwise, use buffer 2
OutCCOK	MOVEQ	  #0,D7				;Set cc to indicate no error
OutRTS	RTS


*************************************************************************
*									*
* 			DoRead subroutine				*
*									*
*************************************************************************

DoRead	MOVEQ	#0,D0				;D0=sync flags returned to user

	TST.B	Mouth(A5)			;Do we want mouths?
	BEQ.S	ReadWordSync			;   No, try word sync
	MOVEQ	#1,D0				;   Yes, Bit 1 ==> mouth change
						;     Note: also must test if
						;           mouth has actually
						;           changed.  Done below
ReadWordSync:
	BTST	#NDB_WORDSYNC,NDI_FLAGS(A3)	;Do we want word sync?
	BEQ.S	ReadSylSync			;   No, try syllable sync
	BTST	#COEFWORDBIT,SYNC(A5)		;   Yes, do we have it?
	BEQ.S	ReadSylSync			;      No, try syllable sync
	OR.B	#$02,D0				;      Yes, Bit 2 ==> word sync 

ReadSylSync:
	BTST	#NDB_SYLSYNC,NDI_FLAGS(A3)	;Do we want syllable sync?
	BEQ.S	ReadAnything			;   No, anthing to do?
	BTST	#COEFSYLBIT,SYNC(A5)		;   Yes, do we have it?
	BEQ.S	ReadAnything			;      No, anything to do?
	OR.B	#$04,D0				;      Yes, Bit 3 ==> syl sync 

ReadAnything:
	TST.B	D0				;Anything to do?
	BNE.S	ReadCheckQueue			;   Yes, go to it
	RTS					;   No, just return


*	;------	Is there a read on the read queue for this write?
ReadCheckQueue:
	DISABLE
	LEA	ND_UNIT+MP_MSGLIST(A4),A0
	CMP.L	LH_TAILPRED(A0),A0
	BEQ.S	ReadReturn			;Input queue empty, just return
	MOVE.L	(A0),A1				;Look at first entry on list
	CMP.W	#CMD_READ,IO_COMMAND(A1)	;Is it a read?
	BNE.S	ReadReturn			;No, just return
	MOVE.L	IO_UNIT(A1),D1			;Does the read match the 
	CMP.L	IO_UNIT(A3),D1			;          current write?
	BNE.S	ReadReturn			;No, just return


*	;------	If the mouth shape has changed send msg to user.  This works
	;       even if we only wanted word/syllable syncs because the "mouth
	;       has changed" bit in the SYNC field will not be set.  If the
	;       mouth shape has not changed, turn that bit off, and if no
	;       other bit in the SYNC field is set, return without sending
	;       anything to the user.  To allow for backward compatability
	;	between new and old IORBs, we add the length of the write IORB,
	;	the offsets (defined in private.i) of the mouth and sync
	;	fields, and the start of the read IORB to get the correct
	;	read IORB fields.
	MOVE.W	MN_LENGTH(A3),D2		;Get length of write IORB
	MOVE.L	CURMOUTH(A5),A2			;Get current mouth bfr pos
	MOVE.B	(A2),D1				;Get current mouth shape
	CMP.B	MRB_SHAPE-MRB_WIDTH(A1,D2),D1	;Has mouth shape changed?
	BNE.S	ReadSend			;Yes
	AND.B	#$FE,D0				;No, reset mouth changed bit
	BEQ.S	ReadReturn			;If no other flag, just return


*	;------ Set the SYNC field in the user's read IORB to indicate which
	;       condition(s) we are responding to, remove the read request
	;	from the queue, and reply to the user.
ReadSend:
	MOVE.B	D0,MRB_SYNC-MRB_WIDTH(A1,D2)	;Store sync flags in SYNC byte
	MOVE.B	D1,MRB_SHAPE-MRB_WIDTH(A1,D2)	;Store in IORB (internal use)
	MOVE.B	D1,D0
	AND.B	#$0F,D0				;Compute mouth width
	MOVE.B	D0,MRB_WIDTH-MRB_WIDTH(A1,D2)
	LSR.B	#4,D1
	MOVE.B	D1,MRB_HEIGHT-MRB_WIDTH(A1,D2)	;Compute mouth height
	CLR.B	IO_ERROR(A1)
	REMHEADQ A0,A1,A2			;Remove IORB from queue
	ENABLE
	CALLSYS	ReplyMsg			;Reply to it
	CLR.B	  SYNC(A5)			;Clear word/syl sync indicator
	RTS

ReadReturn:
	ENABLE
	RTS



*************************************************************************
*									*
* 			SYN_Write subroutine				*
*									*
*************************************************************************

SYN_Write:
	MOVEQ	#0,D2
	MOVE.B	NDI_NUMCHAN(A3),D2
	MOVE.L	D2,D7
	MULU	#ioa_Size,D7

	TST.W	BUFFLAG(A5)
	BNE.S	SYN_Buff2
	MOVE.L	ND_AUDDMAIORB(A4),A1
	MOVE.L	A1,A2
	ADD.L	D7,A2
	BRA.S	SYN_SetLoop

SYN_Buff2:
	MOVE.L	ND_AUDDMAIORB(A4),A2
	MOVE.L	A2,A1
	ADD.L	D7,A1

SYN_SetLoop:
	MOVE.W	D2,D7
	SUBQ.W	#1,D7

SYN_OutLoop:
	BSET	#IOB_QUICK,IO_FLAGS(A1)
	MOVE.L  A1,-(SP)
	BEGINIO
	MOVE.L  (SP)+,A1
	LEA	ioa_Size(A1),A1
	DBF	D7,SYN_OutLoop

	BCLR	#IOB_FIRST,IO_FLAGS(A3)
	BEQ.S	SYN_Wait
	LEA	ND_IORB3(A4),A1
	MOVE.W	#CMD_START,IO_COMMAND(A1)
	CALLSYS	DoIO
	BRA.S	SYN_WReturn

SYN_Wait:
	MOVE.W	D2,D7
	SUBQ.W	#1,D7
	MOVE.L	A2,A1

SYN_WLoop:
	MOVE.L	A1,-(SP)
	CALLSYS	WaitIO
	MOVE.L	(SP)+,A1
	TST.B	IO_ERROR(A1)
	BNE.S	SYN_WDie
	LEA	ioa_Size(A1),A1
	DBF	D7,SYN_WLoop


SYN_WReturn:
	RTS


SYN_WDie:
	BSET	#IOB_ABORT,IO_FLAGS(A3)
	BRA.S	SYN_WReturn


DMANodeName:
	DC.B	'Speech IORB',0
	CNOP	0,2

	END 
