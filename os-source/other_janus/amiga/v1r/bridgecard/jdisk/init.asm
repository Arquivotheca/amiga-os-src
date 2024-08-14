;********************************************************************
;
; init.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


	INCLUDE "assembly.i"

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/initializers.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/memory.i"
	INCLUDE "janus/i86block.i"
	INCLUDE "janus/janus.i"
	INCLUDE "janus/setupsig.i"
	INCLUDE "janus/services.i"
	LIST

	INCLUDE "jddata.i" 
	INCLUDE "asmsupp.i"

	XDEF	InitTable
		
	XLVO	AllocSignal
	XLVO	CloseLibrary
	XLVO	Disable
	XLVO	Enable
	XLVO	FreeMem
	XLVO	FreeSignal
	XLVO	OpenLibrary 
	XLVO	Wait

	XLVO	AllocJanusMem
	XLVO	CleanupJanusSig
	XLVO	FreeJanusMem
	XLVO	GetParamOffset
	XLVO	JanusMemToOffset
	XLVO	SendJanusInt
	XLVO	SetJanusHandler
	XLVO	SetJanusEnable
	XLVO	SetParamOffset
	XLVO	SetupJanusSig


	XREF	JDOpen
	XREF	JDClose
	XREF	JDExpunge
	XREF	JDNull
	XREF	JDBeginIO
	XREF	JDAbortIO

	XREF	JDiskName
	XREF	VERNUM
	XREF	REVNUM


InitTable:
		dc.l	JDiskDevice_SIZEOF
		dc.l	funcTable
		dc.l	initStructData
		dc.l	init

funcTable:
		dc.l	JDOpen
		dc.l	JDClose
		dc.l	JDExpunge
		dc.l	JDNull
		dc.l	JDBeginIO
		dc.l	JDAbortIO   

		dc.l	-1


init:		; ( d0: lib node, a0: seg list )
		movem.l d2-d4/a2-a6,-(sp)

	PUTMSG	30,<'%s/Init: called'>

		;------ do some low level initialization
		move.l	d0,a5
		move.l	a0,jd_Segment(a5)

		move.l	a6,jd_ExecBase(a5)

		lea	JDiskName(pc),a0
		move.l	a0,LN_NAME(a5)
		move.l	a0,jd_CmdTerm+LN_NAME(a5)

		lea	jd_CmdQueue(a5),a0
		NEWLIST a0
		
		;------ get janus.library		 
		lea	jlName(pc),a1
		moveq	#0,d0
		CALLLVO OpenLibrary
		move.l	d0,jd_JanusBase(a5)
		beq	initErr1	    ; library not found

		;------ setup signal handshake for this initialization code
		moveq	#-1,D0
		CALLLVO AllocSignal
		move.l	d0,d1
		move.l	d0,d4		    ; save signal to free later
		moveq	#JSERV_HARDDISK,D0
		moveq	#AmigaDskReq_SIZEOF,D2
		move.l	#MEMF_PARAMETER!MEM_WORDACCESS,d3

		move.l	jd_JanusBase(a5),a6
		CALLLVO SetupJanusSig
		tst.l	d0
		beq	initErr2	  
		move.l	d0,a3		    ; save signal structure

		;------ inquire the hard disk
		move.l	ss_ParamPtr(a3),a4
		move.w	#ADR_FNCTN_INIT,adr_Fnctn(a4)
		moveq	#JSERV_HARDDISK,D0
		CALLLVO SendJanusInt
       
		move.l	ss_SigMask(a3),d0
		move.l	jd_ExecBase(a5),a6
		CALLLVO Wait		     

	IFGE	INFOLEVEL-20
	move.w	adr_Err(a4),-(a7)
	PUTMSG	1,<'%s/adr_Err: %d'>
	addq.l	#2,a7
	ENDC
		tst.w	adr_Err(a4)
		bne	initErr4

		;------ now inquire for each partition on the hard disk
	IFGE	INFOLEVEL-20
	move.w	adr_Part(a4),-(a7)
	PUTMSG	1,<'%s/adr_Part: %d'>
	addq.l	#2,a7
	ENDC
		move.w	adr_Part(a4),d2     ; number of partitions
		subq.w	#1,d2		    ; change to 0 origin
		blt	initErr4
		moveq	#0,d3		    ; largest partition
		
		moveq	#DskPartition_SIZEOF,d0
		move.l	#MEMF_BUFFER!MEM_WORDACCESS,d1
		move.l	jd_JanusBase(a5),a6
		CALLLVO AllocJanusMem
		tst.l	d0
		beq	initErr4
		move.l	d0,a2
		CALLLVO JanusMemToOffset
		move.w	d0,adr_BufferAddr(a4)
		move.w	#ADR_FNCTN_INFO,adr_Fnctn(A4)

		;------ loop for each partition
1$
		move.l	#DskPartition_SIZEOF,adr_Count(a4)
		move.w	d2,adr_Part(a4)
		moveq	#JSERV_HARDDISK,d0
		CALLLVO SendJanusInt

		move.l	ss_SigMask(a3),d0
		move.l	jd_ExecBase(a5),a6
		CALLLVO Wait		     

		tst.w	adr_Err(a4)
		bne	initErr5
		
		cmp.w	dp_NumSecs(a2),d3
		bge.s	2$
		move.w	dp_NumSecs(a2),d3
2$
		move.l	jd_JanusBase(a5),a6
		dbf	d2,1$

	IFGE	INFOLEVEL-20
	move.w	d3,-(a7)
	PUTMSG	1,<'%s/max sectors: %d'>
	addq.l	#2,a7
	ENDC

		;------ free the stuff used here that won't be needed anymore
		move.l	a2,a1
		moveq	#DskPartition_SIZEOF,d0
		CALLLVO FreeJanusMem

		move.l	a3,a0
		CALLLVO CleanupJanusSig

		move.l	jd_ExecBase(a5),a6
		move.l	d4,d0
		CALLLVO FreeSignal

		;------ allocate the parameter, buffer, & interrupt to be used
		moveq	#AmigaDskReq_SIZEOF,d0
		move.l	#MEMF_PARAMETER!MEM_WORDACCESS,d1
		move.l	jd_JanusBase(a5),a6
		CALLLVO AllocJanusMem
		move.l	d0,jd_AmigaDskReq(a5)
		beq	initErr2
		move.l	d0,a2

		move.w	d3,d0
		mulu	#512,d0 	    ; 512 bytes / sector
		move.l	d0,jd_TBSize(a5)
		move.l	#MEMF_BUFFER!MEM_BYTEACCESS,d1
		CALLLVO AllocJanusMem
		move.l	d0,jd_TrackBuffer(a5)
		beq	initErr6

		CALLLVO JanusMemToOffset
		move.w	d0,adr_BufferAddr(a2)

		;------ 
		move.l	jd_ExecBase(a5),a6
		CALLLVO Disable

		move.l	#JSERV_HARDDISK,d0
		move.l	jd_JanusBase(a5),a6
		CALLLVO GetParamOffset
		cmp.w	#$ffff,d0
		bne.s	initErr7

		move.l	jd_AmigaDskReq(a5),d0
		CALLLVO JanusMemToOffset
		move.l	d0,d1
		moveq	#JSERV_HARDDISK,d0
		CALLLVO SetParamOffset

		moveq	#JSERV_HARDDISK,d0
		lea	jd_CmdTerm(a5),a1
		CALLLVO SetJanusHandler

		moveq	#JSERV_HARDDISK,d0
		moveq	#1,d1
		CALLLVO SetJanusEnable

		move.l	jd_ExecBase(a5),a6
		CALLLVO Enable
		;------

		;------ set up return code
		move.l	a5,d0
initEnd:

	PUTMSG	30,<'%s/Init: ending'>

		movem.l (sp)+,d2-d4/a2-a6
		rts


initErr7:
	PUTMSG	1,<'%s/initErr7: called'>
		move.l	jd_ExecBase(a5),a6
		CALLLVO Enable
		move.l	jd_TBSize(a5),d0
		move.l	jd_TrackBuffer(a5),a1
		move.l	jd_JanusBase(a5),a6
		CALLLVO FreeJanusMem
initErr6:
	PUTMSG	1,<'%s/initErr6: called'>
		moveq	#AmigaDskReq_SIZEOF,d0
		move.l	jd_AmigaDskReq(a5),a1
		CALLLVO FreeJanusMem
		bra	initErr2

initErr5:
	PUTMSG	1,<'%s/initErr5: called'>
		move.l	a2,a1
		moveq	#DskPartition_SIZEOF,d0
		CALLLVO FreeJanusMem
initErr4:
	PUTMSG	1,<'%s/initErr4: called'>
		move.l	a3,a0
		move.l	jd_JanusBase(a5),a6
		CALLLVO CleanupJanusSig
initErr3:
	PUTMSG	1,<'%s/initErr3: called'>
		move.l	jd_ExecBase(a5),a6
		move.l	d4,d0
		bmi.s	initErr2 
		CALLLVO FreeSignal
initErr2:
	PUTMSG	1,<'%s/initErr2: called'>
		move.l	jd_JanusBase(a5),a1
		CALLLVO CloseLibrary
initErr1:

	PUTMSG	1,<'%s/InitErr1: called'>

		;------ free up library node
		move.l	a5,a1
		CLEAR	d0
		move.w	LIB_NEGSIZE(a5),d0
		sub.w	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		CALLLVO FreeMem

		moveq	#0,d0
		bra	initEnd
				  

;----------------------------------------------------------------------
initStructData:
		;------ initialize the device library structure
		INITBYTE	LN_TYPE,NT_DEVICE
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERNUM
		INITWORD	LIB_REVISION,REVNUM

		DC.W	0

jlName: 	JANUSNAME

	END


