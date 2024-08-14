;
;   handler.asm -- PC: DOS Handler
;
	SECTION JFHS

StartJFHS	DC.L	(EndJFHS-StartJFHS)/4
StartCode:

INFOLEVEL	EQU 0

	INCLUDE "exec/types.i"
	INCLUDE "exec/ports.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "../janus/janus.i"
	INCLUDE "../janus/janusreg.i"
	INCLUDE "../janus/janusvar.i"
	INCLUDE "../janus/i86block.i"
	INCLUDE "../janus/services.i"
	INCLUDE "../janus/setupsig.i"

	INCLUDE "diagmsg.i"
	INCLUDE "macros.i"

	XREF	_AbsExecBase

	XLVO	AllocMem
	XLVO	AllocSignal
	XLVO	CloseLibrary
	XLVO	FindTask
	XLVO	FreeMem
	XLVO	GetMsg
	XLVO	OpenLibrary
	XLVO	PutMsg
	XLVO	Wait

	XLVO	AllocJanusMem
	XLVO	CleanupJanusSig
	XLVO	FreeJanusMem
	XLVO	JBCopy
	XLVO	SendJanusInt
	XLVO	SetupJanusSig

    STRUCTURE	LOCAL_VARS,0
	APTR	l_DevList
	APTR	l_JanusBase
	APTR	l_PCCallSig
	APTR	l_PCCallData
	APTR	l_Buffer
	UWORD	l_BuffSeg
	UWORD	l_BuffOffset
	LABEL	L

XFBUFFERSIZE	EQU 512

    STRUCTURE	LocalFileHandle,0
	APTR	f_FileName
	UWORD	f_FNSize
	UWORD	f_PCHandle
	LABEL	FileHandle_SIZEOF

;------ The start of the file handler code ---------------------------
;
;   SYNOPSYS
;	result = JFileHandler(dosPacket);
;	d1		      d1
;
;   FUNCTION
;	Each open remote file on the PC has an associated invocation
;	of this task.
;
;   INPUTS
;	dosPacket -- a struct DosPacket with the following elements:
;	    dp_Arg1	string of the form "PC:PCFileName", where
;			PCFileName is to be passed to the PC.
;	    dp_Arg2	struct JMStartup *
;	    dp_Arg3	struct DeviceList * (same as struct JMDev *)
;
;---------------------------------------------------------------------
;
;   BCPL REGISTERS
;	d0	work reg - hold the stack increment in a procedure
;		call
;	d1	the first argument and result register
;	d2-d4	the second to fourth argument registers
;	d5-d7	general work registers
;	a0	hold the constant 0 (to allow n(Z,Di.L) addresses)
;	a1	BCPL P pointer (MC address of first arg)
;	a2	BCPL G pointer (MC address of global 0)
;	a3	work reg - hold the return address in the entry and
;		return sequences
;	a4	base reg - hold the MC address of entry to the
;		current procedure. It is necessary for position
;		independent addressing of data in program code.  Note
;		that instruction and data space MUST be the same.
;	a5	MC address of the save S/R:
;			MOVEA.L (SP)+,a3
;			MOVEM.L a1/a3/a4,-12(a1,D0.L)
;			ADDA.L	D0,a1
;			MOVEM.L D1-D4,(a1)
;			JMP	(a4)
;	a6	MC address of the return code:
;			MOVEM.L -12(a1),a1/a3
;			MOVE.L	-4(a1),a4
;			JMP	(a3)
;	a7	the system stack pointer
;
;---------------------------------------------------------------------
;
;   TASK REGISTERS
;	a2	current packet
;	a3	temporary around subroutine calls
;	a4	process/task
;	a5	local variables base
;
		;------ preserve everything for use by the
		;------ return code below

		movem.l d0-d7/a0-a4/a6,-(a7)
		link	a5,#-L			local variables on stack
		suba.l	a1,a1			and find this process
		move.l	_AbsExecBase,a6
		CALLLVO FindTask
		move.l	d0,a4			should always find it !!

	IFGE	INFOLEVEL-50
	move.l	d0,-(a7)
	INFOMSG 1,<'JFHand: JF_Task = $%lx'>
	addq.l	#4,a7
	ENDC

		lsl.l	#2,d1			convert BPTR to APTR
		movea.l	d1,a2

	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	move.l	dp_Type(a2),-(a7)
	INFOMSG 1,<'StartPkt: type %ld, arg1 $%lx, arg2 $%lx, arg3 $%lx'>
	add.w	#16,a7
	ENDC

		;------ initialize the local variables
		lea.l	jlName(pc),a1		get janus.library
		moveq.l	#0,d0			any version I suppose
		CALLLVO OpenLibrary
		move.l	d0,l_JanusBase-L(a5)	and stash it
		beq	start1Fail		uh oh! no library

		;---------- l_PCCallSig
		moveq	#-1,d0			no need to test this return
		CALLLVO AllocSignal		SetUpJanusSig does it

		move.l	d0,d1
		moveq	#JSERV_PCCALL,d0	interrupt we need
		moveq	#Syscall86_SIZEOF,d2	memory area and type
		move.l	#MEMF_PARAMETER+MEM_WORDACCESS,d3
		move.l	l_JanusBase-L(a5),a6
		CALLLVO SetupJanusSig
		move.l	d0,l_PCCallSig-L(a5)	save pointer to server
		beq	start2Fail		if there was one of course

	IFGE	INFOLEVEL-70
	move.l	d0,-(a7)
	INFOMSG 1,<'JFHand: PCCallSig = $%lx'>
	addq.l	#4,a7
	ENDC
  
		;---------- l_PCCallData
		move.l	d0,a0			stash parameter pointer
		move.l	ss_ParamPtr(a0),l_PCCallData-L(a5)

		;---------- l_Buffer
		move.l	#XFBUFFERSIZE,d0	allocate transfer buffer
		move.l	#MEMF_BUFFER+MEM_BYTEACCESS,d1
		CALLLVO AllocJanusMem
		move.l	d0,l_Buffer-L(a5)	and stash with local vars
		beq	start3Fail

	IFGE	INFOLEVEL-70
	move.l	d0,-(a7)
	INFOMSG 1,<'JFHand: Transfer Buffer = $%lx'>
	addq.l	#4,a7
	ENDC

; calculate buffer offset and segment for 8088 addressing

		;---------- l_BuffXxxx
		movea.l	ja_ParamMem(a6),a0
		adda.l	#WordAccessOffset,a0
		sub.l	jb_BufferMem+jmh_68000Base(a0),d0
		move.w	d0,l_BuffOffset-L(a5)
		move.w	jb_BufferMem+jmh_8088Segment(a0),l_BuffSeg-L(a5)

	IFGE	INFOLEVEL-50
	move.w	d0,-(a7)
	move.w	l_BuffSeg-L(a5),-(a7)
	INFOMSG 1,<'JFHand: BuffSeg = $%x, BuffOffset = $%x'>
	addq.l	#4,a7
	ENDC

		;------ initialize the DeviceList
		move.l	dp_Arg3(a2),a0		BPTR in startup packet
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	a0,l_DevList-L(a5)	stash device list ptr
		lea.l	pr_MsgPort(a4),a1	fill in our message port
		move.l	a1,dl_Task(a0)		so DOS signals us...
;!!!no		move.l	#DLT_VOLUME,dl_Type(a0)	...when device is called
;!!!no		move.l	#ID_DOS_DISK,dl_DiskType(a0)
		moveq.l #TRUE,d0		return OK packet
		bsr	ReturnPkt

		;------
		;------ handle new packets
		;------
packetLoop:
		move.l	pr_PktWait(a4),d0	check for alternate wait
		beq.s	stdWait			this always happens (poof)
		movea.l	d0,a0
		jsr	(a0)			call alternate wait fncn
		bra.s	getPkt
waitForIt:
		move.l	#SIGF_DOS,d0		wait for port signal
		move.l	_AbsExecBase,a6
	INFOMSG 20,<'JFHand: Wait(NextPacket)'>
		CALLLVO Wait
	INFOMSG 20,<'JFHand: Awake(NextPacket)'>
stdWait:
		lea.l	pr_MsgPort(a4),a0	check if message there
		CALLLVO GetMsg
		tst.l	d0
		beq.s	waitForIt		nope, nothing for us
getPkt:
		movea.l	d0,a0			message received
		movea.l	LN_NAME(a0),a2		get packet from msg

		;------ dispatch interesting packets
		move.l	dp_Type(a2),d0

	IFGE	INFOLEVEL-5
	move.l	d0,-(a7)
	INFOMSG 1,<'JFHand: Packet Type = %ld'>
	addq.l	#4,a7
	ENDC

		swap	d0
		tst.w	d0
		bne	boringPacket
		swap	d0

		cmpi.w	#ACTION_READ,d0
		beq	readPacket
		cmpi.w	#ACTION_WRITE,d0
		beq	writePacket
		cmpi.w	#ACTION_SEEK,d0
		beq	seekPacket
		cmpi.w	#ACTION_FINDINPUT,d0
		beq	oopenPacket
		cmpi.w	#ACTION_FINDOUTPUT,d0
		beq	nopenPacket
		cmpi.w	#ACTION_END,d0
		beq	closePacket
		cmpi.w	#ACTION_DIE,d0
		beq	diePacket
	INFOMSG	5,<'JFHand: Not a packet we recognise!!'>
		bra	boringPacket


		;------ the file handler has decided to die, let
		;------ the BCPL interface make it go away
jfhDie:
		move.l	l_DevList-L(a5),a0
		clr.l	dl_Task(a0)
		     
		;------ free buffer memory
		move.l	#XFBUFFERSIZE,d0
		move.l	l_Buffer-L(a5),a1
		move.l	l_JanusBase-L(a5),a6
		CALLLVO FreeJanusMem

jfhCleanup:
		;------ cleanup PCCallSig stuff
		move.l	l_PCCallSig-L(a5),a0
		move.l	l_JanusBase-L(a5),a6
		CALLLVO CleanupJanusSig

jfhClose:
		;------ close the janus library
		move.l	l_JanusBase-L(a5),a1
		move.l	_AbsExecBase,a6
		CALLLVO CloseLibrary

jfhEnd:
		unlk	a5
		movem.l (a7)+,d0-d7/a0-a4/a6
		jmp	(a6)			; return

start1Fail:
	INFOMSG 20,<'JFHand: start1Fail'>
		moveq	#FALSE,d0
		bsr	ReturnPkt
		bra.s	jfhEnd

start2Fail:
	INFOMSG 20,<'JFHand: start2Fail'>
		moveq	#FALSE,d0
		bsr	ReturnPkt
		bra.s	jfhClose

start3Fail:
	INFOMSG 20,<'JFHand: start3Fail'>
		moveq	#FALSE,d0
		bsr	ReturnPkt
		bra	jfhCleanup

jlName: 	dc.b	'janus.library',0
		CNOP	0,2

;------ Packet Cases -------------------------------------------------

;---------- readPacket ----------
readPacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'readPacket: handle = $%lx, buffer = $%lx, len = %ld'>
	add.w	#12,a7
	ENDC
		move.l	dp_Arg2(a2),a3		data buffer
		move.l	dp_Arg3(a2),d3		data length
		moveq	#0,d4			actual length

rLoop:
		move.l	#XFBUFFERSIZE,d2
		cmp.l	d2,d3
		bgt.s	rLenClip
		move.l	d3,d2
rLenClip:
		move.w	d2,d0
		move.w	#$3f00,d1
	INFOMSG	50,<'readPacket: calling TransferData'>
	IFGE	INFOLEVEL-50
	move.l	d0,-(sp)
	INFOMSG	50,<'     length requested = %ld'>
	addq.l	#4,sp
	ENDC
		bsr	TransferData
		tst.l	d1		    check for error
		bne	rwError
	INFOMSG	50,<'readPacket: no error from TransferData'>
	IFGE	INFOLEVEL-50
	move.l	d0,-(sp)
	INFOMSG	50,<'     length returned = %ld'>
	addq.l	#4,sp
	ENDC
		move.l	d0,d5		    cache length
		move.l	l_Buffer-L(a5),a0   get source 
		move.l	a3,a1		    and destination
		move.l	l_JanusBase-L(a5),a6
	IFGE	INFOLEVEL-50
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	move.l	d0,-(sp)
	INFOMSG	50,<'readPacket: JBCopy len=%ld source=$%lx dest=$%lx'>
	adda.w	#12,sp
	ENDC
		CALLLVO JBCopy		    copy buffer to user data
	INFOMSG	50,<'readPacket: back from JBCopy'>
		add.l	d5,d4		    update actual
		cmp.w	d2,d5		    check for truncation
		bne.s	rwDone
		add.l	d2,a3		    increment user data pointer
		sub.l	d2,d3		    decrement remaining bytes
		bgt	rLoop

rwDone:
		move.l	d4,d0
rwEnd:
		bra	endPacket

rwError:
		moveq	#-1,d0
		bra.s	rwEnd


;---------- writePacket ----------
writePacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'writePacket: handle = $%lx, buffer = $%lx, len = %ld'>
	add.w	#12,a7
	ENDC
		move.l	dp_Arg2(a2),a3		data buffer
		move.l	dp_Arg3(a2),d3		data length
		moveq	#0,d4			actual length

wLoop:
		move.l	#XFBUFFERSIZE,d2
		cmp.l	d2,d3
		bgt.s	wLenClip
		move.l	d3,d2
wLenClip:
		move.w	d2,d0
		move.l	a3,a0		    	get source
		move.l	l_Buffer-L(a5),a1   	and destination
		move.l	l_JanusBase-L(a5),a6
	INFOMSG	50,<'writePacket: calling JBCopy'>
		CALLLVO JBCopy		    ; copy user data to buffer
	INFOMSG	50,<'writePacket: back from JBCopy'>

		move.w	d2,d0
		move.w	#$4000,d1
	INFOMSG	50,<'writePacket: calling TransferData'>
		bsr	TransferData
		tst.l	d1		    ; check for error
		bne	rwError
	INFOMSG	50,<'writePacket: no error from TransferData'>

		add.l	d0,d4		    ; update actual
		cmp.w	d2,d0		    ; check for truncation
		bne	rwDone
		add.l	d2,a3		    ; increment user data pointer
		sub.l	d2,d3		    ; decrement remaining bytes
		bgt	rLoop
		bra	rwDone


;---------- seekPacket ----------
seekPacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'Seek    handle 0x%04lx, position 0x%lx, mode %ld'>
	add.w	#12,a7
	ENDC
		move.l	dp_Arg1(a2),a1		PC file handle
		move.l	dp_Arg2(a2),d0		position
		move.l	dp_Arg3(a2),d1		seek mode
		move.l	l_PCCallData-L(a5),a0
		addq.w	#1,d1
		blt.s	badSMode
		cmp.w	#2,d1
		bgt.s	badSMode
		or.w	#$4200,d1
		move.w	d1,s86_AX(a0)
		move.w	f_PCHandle(a1),s86_BX(a0)
		move.w	d0,s86_DX(a0)
		swap	d0
		move.w	d0,s86_CX(a0)
		bsr	Do86Function
		bsr	ReturnPkt
		bra	packetLoop

badSMode:
		move.l	#ERROR_SEEK_ERROR,d1
		bra	badPacket


;---------- oopenPacket ----------
oopenPacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'OldOpen arg1 0x%lx, arg2 0x%lx, arg3 0x%lx'>
	add.w	#12,a7
	ENDC
		move.w	#$3d00,d0	open existing file
		bra	OpenFile


;---------- nopenPacket ----------
nopenPacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'NewOpen arg1 0x%lx, arg2 0x%lx, arg3 0x%lx'>
	add.w	#12,a7
	ENDC
		move.w	#$3c00,d0	open a brand new file
		bra	OpenFile


;---------- closePacket ----------
closePacket:
	IFGE	INFOLEVEL-5
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	INFOMSG 1,<'Close   arg1 0x%lx, arg2 0x%lx, arg3 0x%lx'>
	add.w	#12,a7
	ENDC
		move.l	l_PCCallData-L(a5),a0
		move.w	#$3e00,s86_AX(a0)	fill AH w/ byte in d0
		move.l	dp_Arg1(a2),a1		file handle
		move.w	f_PCHandle(a1),s86_BX(a0)
		bsr	Do86Function
		movem.l d0-d1,-(a7)		save return codes
		move.l	dp_Arg1(a2),a1		file handle

	IFGE	INFOLEVEL-5
	move.w	f_FNSize(a1),-(a7)
	move.l	f_FileName(a1),-(a7)
	move.l	f_FileName(a1),-(a7)
	INFOMSG 1,<'Close   name "%s" @ 0x%lx, len %ld'>
	add.w	#10,a7
	ENDC

		moveq	#0,d0
		move.w	f_FNSize(a1),d0
		move.l	f_FileName(a1),a1
		move.l	l_JanusBase-L(a5),a6
		CALLLVO FreeJanusMem

		moveq	#FileHandle_SIZEOF,d0
		move.l	dp_Arg1(a2),a1
		move.l	_AbsExecBase,a6
		CALLLVO FreeMem

		movem.l (a7)+,d0-d1
		bsr	ReturnPkt
		bra	packetLoop
		

;---------- diePacket ----------
diePacket:
		moveq	#TRUE,d0
		bsr	ReturnPkt
		bra	jfhDie

;---------- boringPacket ----------
boringPacket:
	IFGE	INFOLEVEL-50
	move.l	dp_Arg3(a2),-(a7)
	move.l	dp_Arg2(a2),-(a7)
	move.l	dp_Arg1(a2),-(a7)
	move.l	dp_Type(a2),-(a7)
	INFOMSG 1,<'boring type %ld: arg1 0x%lx, arg2 0x%lx, arg3 0x%lx'>
	add.w	#16,a7
	ENDC
		move.l	#ERROR_ACTION_NOT_KNOWN,d1
badPacket:
		moveq	#0,d0
endPacket:
		bsr	ReturnPkt
		bra	packetLoop

;------ TransferData -------------------------------------------------
;
;   INPUT
;	d0	transfer length  
;	d1	transfer function
;
;   RETURNS
;	d0	actual length, valid if no error
;	d1	error	     
;
TransferData:
		move.l	l_PCCallData-L(a5),a0
		move.w	d1,s86_AX(a0)			; function 
		move.l	dp_Arg1(a2),a1			; JF file handle
		move.w	f_PCHandle(a1),s86_BX(a0)	; PC file handle
		move.w	d0,s86_CX(a0)			; length
		move.w	l_BuffSeg-L(a5),s86_DS(a0)	; transfer buffer
		move.w	l_BuffOffset-L(a5),s86_DX(a0)	;
		move.w	#$21,s86_INT(a0)	; function request

	IFGE	INFOLEVEL-5
	INFOMSG	5,<'====Start of transferData===='>
	move.w	s86_DX(a0),-(sp)
	move.w	s86_CX(a0),-(sp)
	move.w	s86_BX(a0),-(sp)
	move.w	s86_AX(a0),-(sp)
	move.w	s86_PSW(a0),-(sp)
	move.w	s86_INT(a0),-(sp)
	INFOMSG	5,<'INT=%x, PSW=%04x, AX=%x, BX=%x, CX=%x, DX=%x'>
	adda.w	#12,sp
	move.w	s86_BP(a0),-(sp)
	move.w	s86_ES(a0),-(sp)
	move.w	s86_DI(a0),-(sp)
	move.w	s86_DS(a0),-(sp)
	move.w	s86_SI(a0),-(sp)
	INFOMSG	5,<'SI=%x, DS=%x, DI=%x, ES=%x, BP=%x'>
	adda.w	#10,sp
	ENDC

		moveq	#JSERV_PCCALL,d0
		move.l	l_JanusBase-L(a5),a6
		CALLLVO SendJanusInt

		move.l	l_PCCallSig-L(a5),a0
		move.l	ss_SigMask(a0),d0
		move.l	_AbsExecBase,a6
	INFOMSG 20,<'Wait  (TransferData)'>
		CALLLVO Wait
	INFOMSG 20,<'Awake (TransferData)'>

		move.l	l_PCCallData-L(a5),a0

	IFGE	INFOLEVEL-5
	INFOMSG	5,<'====PC returned the following===='>
	move.w	s86_DX(a0),-(sp)
	move.w	s86_CX(a0),-(sp)
	move.w	s86_BX(a0),-(sp)
	move.w	s86_AX(a0),-(sp)
	move.w	s86_PSW(a0),-(sp)
	move.w	s86_INT(a0),-(sp)
	INFOMSG	5,<'INT=%x, PSW=%04x, AX=%x, BX=%x, CX=%x, DX=%x'>
	adda.w	#12,sp
	move.w	s86_BP(a0),-(sp)
	move.w	s86_ES(a0),-(sp)
	move.w	s86_DI(a0),-(sp)
	move.w	s86_DS(a0),-(sp)
	move.w	s86_SI(a0),-(sp)
	INFOMSG	5,<'SI=%x, DS=%x, DI=%x, ES=%x, BP=%x'>
	adda.w	#10,sp
	ENDC

		moveq	#0,d1
		move.w	s86_PSW(a0),d0
		btst	#0,d0		    check carry flag
		beq.s	xfNoError

		move.w	s86_AX(a0),d1	    get error
		add.l	#100000,d1
		rts

xfNoError:
		moveq	#0,d0
		move.w	s86_AX(a0),d0	    get length
		rts


;------ OpenFile -----------------------------------------------------
;
;	d0	PC-DOS function to use (in high byte)	    
;
OpenFile:
		move.l	a3,-(a7)

		move.l	l_PCCallData-L(a5),a0
		move.b	#$02,d0 	; compatability, read/write (for OPEN)
		move.w	d0,s86_AX(a0)	; fill AH w/ byte in d0
		clr.w	s86_CX(a0)	; normal attributes (for CREATE)
		move.w	#$21,s86_INT(a0)    ; function request

		;------ get file handle memory
		moveq	#FileHandle_SIZEOF,d0
		moveq	#0,d1
		move.l	_AbsExecBase,a6
		CALLLVO AllocMem
		tst.l	d0
		beq	open1Fail

		move.l	d0,a3
		;------ cache the file handle
		move.l	dp_Arg1(a2),a0
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	d0,fh_Arg1(a0)

	IFGE	INFOLEVEL-50
	move.l	d0,-(a7)
	INFOMSG 1,<'Handle 0x%lx'>
	addq.l	#4,a7
	ENDC

		;------ get the file name
		move.l	dp_Arg3(a2),a0
		bsr	GetName

		move.l	d0,f_FileName(a3)
		beq	open2Fail
		move.w	d1,f_FNSize(a3)

		bsr	GetSegAddress

		move.l	l_PCCallData-L(a5),a0
		move.w	d0,s86_DS(a0)
		swap	d0
		move.w	d0,s86_DX(a0)

		bsr	Do86Function
		tst.l	d0
		beq.s	open3Fail 
				     
		move.w	s86_AX(a0),f_PCHandle(a3)

	IFGE	INFOLEVEL-50
	move.w	s86_AX(a0),-(a7)
	INFOMSG 1,<'PC Handle 0x%x'>
	addq.l	#2,a7
	ENDC

openEnd:
		move.l	(a7)+,a3
		bsr	ReturnPkt
		bra	packetLoop

open3Fail:
	INFOMSG 20,<'open3Fail'>
		moveq	#0,d0
		move.w	f_FNSize(a3),d0
		move.l	f_FileName(a3),a1
		move.l	l_JanusBase-L(a5),a6
		CALLLVO FreeJanusMem

open2Fail:
	INFOMSG 20,<'open2Fail'>
		moveq	#FileHandle_SIZEOF,d0
		move.l	a3,a1
		move.l	_AbsExecBase,a6
		CALLLVO FreeMem

open1Fail:
	INFOMSGo :� ���   �� g:� �pU� X0`b�� f<�    X0f:� ��U� X0` ����� X0fT� X0:� ��`:� ��`  Ѕ�   f:� ��U� X0`:� ��T� X0J�gf�����l:� ���   `�   o:� ���   J�l D��   r	�29 ��H���:�`J�o �   r	�29 ��H���:� <��  ��  ���6 J���f@3� X 0F@3� X$ . �� @�� ��(P . �R�� @�� ��&P��d:�`�`>3� X(0F@3� X, . �� @�� �n(P . �R�� @�� �n&P��d:�`�R��� Ѕ�   ,  Ѕ�   . �� 0.��H�】� X00.��H�】� X4` �4 . �". ���   gJy X8fJ� X0g:� �lJ� X4g:� �n-M��:� �~ . ��:�:� �� .��`NqL�8���N^NuNV  ������H�8�.. ,. B���*n J� o �* . ��l*. �   f  . �=@���    f  ��   f  �n ��mH0.��H��S�$ �  �o:� �� r�:�0:�` �   o:� ��0:�`09 ��H���:�-M��n ��m:� ��n ��m:� �z .����:�T�0.��H��   g:� ��` Zn ��mF0.��H�S�$ �  �o:� �� r�:�0:�` �   o:� ��0:�`09 ��H���:�-M�� . �". ���   g  � ��( ��o:� ��X� X0`:� ��T� X0��g�� g:� �pU� X0U� X0J�gf�����l:� ���   `�   o:� ���   J�l D��   r	�29 ��H���:�`J�o �   r	�29 ��H���:� . �� @�� �D(P . �R�� @�� �D&P��d:�`�n ��m:� �z .����:�T��    ` �=| �� . �". ���   g  � ��( ��o:� ��X� X0`:� ��T� X0��g�� g:� �pU� X0U� X0J�gf�����l:� ���   `�   o:� ���   J�l D��   r	�29 ��H���:�`J�o �   r	�29 ��H���:� <��  ��  ��"���6 J���f@3� X(0F@3� X, . �� @�� ��(P . �R�� @�� ��&P��d:�`�`>3� X 0F@3� X$ . �� @�� �(P . �R�� @�� �&P��d:�`�R��� ��S��   R�,  ��S��   R�. �� 0.��H��ѹ X00.��H��ѹ X4` �$ . �". ���   gJ� X0g:� �lJ� X4g:� �n-M��:� �~ . ��:�:� �� .��`NqL�8���N^Nu  NV��.   gr�î `R-n ��A� -h ��-P��A� -h ��-P��Hn ,Hn��N� ;�POHn  Hn N� ;�POHn ,Hn N� ;�PO n  P� �XgD/. 4/. 0/. ,/. (/. $/.  /. /. /. /. /. /.  n  P h N�O� 0` � n   P� �Xf n  �    gp�` hJ� o ^J� o VB���B���B���J� ,gz n , P� �Xgl n ,/( /. /. N� �tO� -@��B���-n����/. 4/. 0/. ,Hx /. /. /.��/.��/  n , P PN�O� $A���-h  4-h  0-P , n �    o^/( /. /. N� �tO� -@��B���-n����/. (/. $/.  /. /. /.��/.��/ a |O�  A���-h  (-h  $-P   n /( /. /. N� �tO� -@��B���-n����/. /. /. Hx /. /. /.��/.��/  n  P PN�O� $/. 4/. 0/. , . r�/ /. /. /.��/.��/.�� n�� P PN�O� $/. (/. $/.  Hx /. /. /.��/.��/.�� n�� P PN�O� $/. (/. $/.  Hx /. /. /. /. /.  n  P PN�O� $/.��/.��/.��Hx /. /. /. /. /.  n  P PN�O� $J���g/.��N� ��XOJ���g/.��N� ��XOJ���g/.��N� ��XOp N^NuNV��H�0�Hn N� ��XO(@ n  h *Hn N� ��XO-@�� n  h ,Hn N� �0XO( p��". ��op��` . =@��J� o  �*L-n����T���=n ��0.��Sn��J@o> n��0�  ���> 0.��H�& S�mJGlr��`B�G`�ݮ��0.��H�"����`�0.��H��� �����    op` . =@��x ` �tL�0���N^Nu   � �
 � � � �  �" �$ �* �, �0 �2 �6 �8 �@ �B �DB]�UFA:�FA�]FA:�0F@�A:�F]�]�UFA:��]FA�]T�FA�]:�0F@�A:��]:� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� ��Be�eFA:�FA�eFA;0%F@�A:�Fe�e�eFA:��eFA�eT�FA�e;0%F@�A:��e; � �
 � � �& �0 �2 �6 �@ �D �N �N �T �^ �h �l �n�]�C0�A�@:�FA�B�]FA�C0�B�A:��B0�@�A:��]�C�]�B0�A�@:��B�]�C0�@�@:�FA�C�]�C0�B�A:��C0�@�A:��C�]�] �� �� �� �� �� �� �� �� �� �� �� �� �� � � � ��]�E0�A�@:�FA�D�]FA�E0�D�A:��D0�@�A:��]�E�]�D0�A�@:��D�]�E0�@�@:�FA�E�]�E0�D�A:��E0�@�A:��E�]�] �\ �^ �h �n �z �� �� �� �� �� �� �� �� �� �� �� ���e�C0%�A�@:�FA�B�eFA�C0%�B�A:��B0%�@�A:��e�C�e�B0%�A�@:��B�e�C0%�@�@:�FA�C�e�C0%�B�A:��C0%�@�A:��C�e�e � � � � �$ �. �0 �4 �> �B �L �L �R �\ �f �j �l�e�E0%�A�@:�FA�D�eFA�E0%�D�A:��D0%�@�A:��e�E�e�D0%�A�@:��D�e�E0%�@�@:�FA�E�e�E0%�D�A:��E0%�@�A:��E�e�e����T�@  A  Q��HQ��D+$"$*�*�",<    | r�;$2$:�:�220%0<<  :�:��A���NuU�HA  NV�xH�<�B��� n p �    W�D =@��X�-h ��-P�� n -h ��.   fp`B�-@�� . �-@��g-@   . �r��-@  n��0(    g n �� )�H�H�-@ Jn��g*J�  g
 . �-@ . r��� @�� $ -P m � . �� .  . �� ,  R�r��b ( R���b Jn��g J���g . ����d^ . ����dT n�� "( Ү ��Ш "( Ү �Ёv���*@ ( Ю v��"<  � �& .a0)
		bra.s	rpRts
		
;------ data shared by all invocations ------
openCount	DC.L	0			;!!!

;------ look like a BCPL module ------
		CNOP	0,4
		DC.L	0
		DC.L	1			; index of start entry
		DC.L	StartCode-StartJFHS	; offset of start entry
		DC.L	0			; no globals referenced
EndJFHS:

	END

