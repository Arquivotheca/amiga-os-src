;*********************************************************************
;
; djmount.asm -- mount command for jdisk device
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
;*********************************************************************

	INCLUDE "assembly.i"

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/ports.i" 
	INCLUDE "exec/io.i"
	INCLUDE "exec/memory.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE	"janus/janusvar.i"
	INCLUDE "janus/i86block.i"
	INCLUDE "janus/janus.i"
	INCLUDE "janus/setupsig.i"
	INCLUDE "janus/services.i"
	LIST

	INCLUDE "djmount.i" 
	INCLUDE "asmsupp.i"

	XLVO	AllocMem
	XLVO	FreeMem
	XLVO	AllocSignal
	XLVO	CloseDevice
	XLVO	CloseLibrary
	XLVO	Forbid
	XLVO	FreeSignal
	XLVO	OpenDevice
	XLVO	OpenLibrary
	XLVO	Permit
	XLVO	Wait

	XLVO	DeviceProc
	XLVO	Open
	XLVO	Close
	XLVO	Delay

	XLVO	AllocJanusMem
	XLVO	CleanupJanusSig
	XLVO	FreeJanusMem
	XLVO	JanusMemToOffset
	XLVO	SendJanusInt
	XLVO	SetupJanusSig

	XREF	_SysBase
	XREF	_DOSBase

	XDEF	_main

BOOT_TIMEOUT	EQU	40	; seconds to wait for PC handler booted

;---------------------------------------------------------------------
	SECTION section,data
Info		DS.L	1
_JanusBase	DS.L	1

JH0		EQU	$034a4830	    ; BCPL string "JH0"
;---------------------------------------------------------------------
	SECTION section,code

_main:
		movem.l d2-d7/a2-a6,-(a7)

		;------ check that JH0 does not already exist
	IFGE	INFOLEVEL-50
	INFOMSG	50,<'-----------------------------------------'>
	INFOMSG	50,<'DJMount: Checking for JH0:'>
	ENDC
		move.l	_DOSBase,a0	    ; get DOSBase
		move.l	dl_Root(a0),a0	    ; get RootNode
		move.l	rn_Info(a0),a0	    ; get BPTR Info
		add.l	a0,a0		    ; adjust to APTR
		add.l	a0,a0		    ;
		move.l	a0,Info 	    ; save Info structure location
		move.l	di_DevInfo(a0),d0   ; get BPTR DevInfo
loopDevInfos:
		beq.s	endOfDevInfos
		lsl.l	#2,d0		    ; adjust to APTR   
		move.l	d0,a0
		move.l	jmd_Name(a0),a1     ; get BPTR to name
		add.l	a1,a1		    ; adjust to APTR
		add.l	a1,a1		    ;
		cmp.l	#JH0,(a1)	    ; check if "JH0"
		beq.s	endOfDevInfos
		move.l	(a0),d0 	    ; get BPTR jmd_Next
		bra.s	loopDevInfos

endOfDevInfos:
		;------ if d0 is zero here, JH0 was not found
		tst.l	d0
		bne	errMain1

		;------ get janus.library		 
		lea	jlName(pc),a1
		moveq	#0,d0
		move.l	_SysBase,a6
		CALLLVO OpenLibrary
		move.l	d0,_JanusBase	
		beq	errMain1	    ; library not found
	IFGE	INFOLEVEL-50
	move.l	d0,-(a7)
	INFOMSG 50,<'DJMount: JanusBase=0x%lx'>
	addq.l	#4,a7
	ENDC

;============== before starting up, wait for the PC's handler to be running
		movea.l	_JanusBase,A0		; wait for $42
		movea.l	ja_ParamMem(A0),A2
		moveq.l	#BOOT_TIMEOUT,d4	; only wait for a while
		movea.l	_DOSBase,a6
PCReadyLoop	cmpi.b	#$42,jb_ParamMem+jmh_pad0(A2)
		beq.s	PCGoing			; yes, PC up and running
		moveq.l	#50,d1			; delay 1 second
		CALLLVO	Delay
		dbra	d4,PCReadyLoop		; and try again maybe
;==== PC didn't boot it's handler within timeout time so error exit

		movea.l	_JanusBase,a1
		movea.l	_SysBase,a6
		CALLLVO	CloseLibrary
		bra	errMain1

		;------ setup signal handshake for this initialization code
PCGoing		movea.l	_SysBase,a6
		moveq	#-1,D0
		CALLLVO AllocSignal
	IFGE	INFOLEVEL-50
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'DJMount: Signal allocated for int # 9= $%lx'>
	ADDQ.L	#4,SP
	ENDC
		move.l	d0,d1
		move.l	d0,d4		    ; save signal to free later
		moveq	#JSERV_HARDDISK,D0
		moveq	#AmigaDskReq_SIZEOF,D2
		move.l	#MEMF_PARAMETER!MEM_WORDACCESS,d3

		move.l	_JanusBase,a6
		CALLLVO SetupJanusSig
	IFGE	INFOLEVEL-50
	move.l	d0,-(a7)
	INFOMSG 50,<'DJMount: SetupJanusSig=0x%lx'>
	addq.l	#4,a7
	ENDC
		tst.l	d0
		beq	errMain2	  
		move.l	d0,a3		    ; save signal structure

		;------ inquire the hard disk
		move.l	ss_ParamPtr(a3),a4
		move.w	#ADR_FNCTN_INIT,adr_Fnctn(a4)
		moveq	#JSERV_HARDDISK,D0
		CALLLVO SendJanusInt
		move.l	ss_SigMask(a3),d0
		move.l	_SysBase,a6
		CALLLVO Wait		     

		;------ now inquire for each partition on the hard disk
		move.w	adr_Part(a4),d2     ; number of partitions
		beq	errMain3
		
		moveq	#DskPartition_SIZEOF,d0
		move.l	#MEMF_BUFFER!MEM_WORDACCESS,d1
		move.l	_JanusBase,a6
		CALLLVO AllocJanusMem
		tst.l	d0
		beq	errMain3
		move.l	d0,a5
		CALLLVO JanusMemToOffset
		move.w	d0,adr_BufferAddr(a4)
		move.w	#ADR_FNCTN_INFO,adr_Fnctn(A4)
		moveq	#0,d3		    ; current partition

		;------ loop for each partition
partitionLoop:
	INFOMSG	50,<'DJMount: partitionLoop:'>
		move.l	#DskPartition_SIZEOF,adr_Count(a4)
		move.w	d3,adr_Part(a4)
		moveq	#JSERV_HARDDISK,d0
		CALLLVO SendJanusInt
		move.l	ss_SigMask(a3),d0
		move.l	_SysBase,a6
		CALLLVO Wait		     

	IFGE	INFOLEVEL-50
	move.l	adr_Count(a4),-(a7)
	move.l	d3,-(a7)
	INFOMSG 50,<'DJMount: partition #:%ld; adr_Count:%ld (=12?)'>
	addq	#8,a7
	ENDC
		cmp.l	#DskPartition_SIZEOF,adr_Count(a4)
		blt	errMain4

		;---------- build a device node for this partition
		move.l	#jma_SIZEOF,d0
		move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
		CALLLVO AllocMem
	move.l	d0,-(a7)
	INFOMSG 50,<'DJMount: device node at $%lx'>
	addq.l	#4,a7
		tst.l	d0
		beq	errMain4
		move.l	d0,a2

		;---------- fill the device node
		move.l	#$400,jma_jmd+jmd_StackSize(a2)
		move.l	#10,jma_jmd+jmd_Priority(a2)
		lea	jma_jms(a2),a0
		move.l	a0,d0
		lsr.l	#2,d0
		move.l	d0,jma_jmd+jmd_Startup(a2)
		move.l	_DOSBase,a6
		move.l	#df0Name,d1
		CALLLVO DeviceProc
		tst.l	d0
		beq	errMain4
		move.l	d0,a0
		move.l	pr_SegList-pr_MsgPort(a0),a0
		add.l	a0,a0
		add.l	a0,a0
		move.l	12(a0),jma_jmd+jmd_SegList(a2)

		;---------- fill the environment vector
		move.l	#(jme_SIZEOF/4)-1,jma_jme+jme_StructSize(a2)
		move.l	#1,jma_jme+jme_NSectsPerBlk(a2)
		move.l	#128,jma_jme+jme_BlockSize(a2)
		move.l	#5,jma_jme+jme_NBuffers(a2)	 
		moveq	#0,d0
		move.w	dp_NumHeads(a5),d0
	move.l	d0,-(a7)
	INFOMSG 5,<'    NSurfaces: %ld'>
	addq.l	#4,a7
		move.l	d0,jma_jme+jme_NSurfaces(a2)
		move.w	dp_NumSecs(a5),d0
	move.l	d0,-(a7)
	INFOMSG 5,<'    NSectors: %ld'>
	addq.l	#4,a7
		move.l	d0,jma_jme+jme_NBlksPerTrack(a2)
	IFGE	INFOLEVEL-5
	clr.l	d0
	move.w	dp_EndCyl(a5),d0
	move.l	d0,-(sp)
	INFOMSG	5,<'    EndCyl:  %ld'>
	addq.l	#4,sp
	move;********************************************************************
;
; dev.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


	INCLUDE "assembly.i"

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"

	INCLUDE "janus/i86block.i"
	INCLUDE "janus/services.i"
	LIST

	INCLUDE "jddata.i" 
	INCLUDE "asmsupp.i"

	XDEF	JDOpen
	XDEF	JDClose
	XDEF	JDExpunge
	XDEF	JDNull
		
	XLVO	CloseLibrary
	XLVO	FreeMem

	XLVO	FreeJanusMem
	XLVO	SetParamOffset
	XLVO	SetJanusHandler

		    
;------ jdisk.device/Open --------------------------------------------
JDOpen:
	PUTMSG	60,<'%s/jdisk.device -- Open'>
		move.l	d0,IO_UNIT(a1)
		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
		addq.w	#1,LIB_OPENCNT(a6)
		rts

;------ jdisk.device/Close -------------------------------------------
JDClose:
	PUTMSG	60,<'%s/Close: beginning'>
		;------ clear device as a precaution against further use
		moveq	#-1,d0 
		move.l	d0,IO_DEVICE(a1)

		;------ check if device should now be expunged
		subq.w	#1,LIB_OPENCNT(a6)
		bne.s	1$
		btst	#LIBB_DELEXP,LIB_FLAGS(a6)
		beq.s	1$
		jmp	LIB_EXPUNGE(a6) 	; call expunge for device

1$
	PUTMSG	60,<'%s/Close: ending'>
		moveq	#0,d0
		rts

;------ jdisk.device/Expunge -----------------------------------------
JDExpunge:
		tst.w	LIB_OPENCNT(a6)
		beq.s	realExpunge

		bset	#LIBB_DELEXP,LIB_FLAGS(a6)
		moveq	#0,d0
		rts

realExpunge:
		movem.l a2/a6,-(a7)
		move.l	a6,a2		    ; cache device base

		moveq	#JSERV_HARDDISK,d0
		suba.l	a1,a1
		move.l	jd_JanusBase(a2),a6
		CALLLVO SetJanusHandler

		moveq	#JSERV_HARDDISK,d0
		moveq	#-1,d1
		CALLLVO SetParamOffset

		moveq	#AmigaDskReq_SIZEOF,d0
		move.l	jd_AmigaDskReq(a2),a1
		CALLLVO FreeJanusMem

		move.l	jd_TBSize(a2),d0
		move.l	jd_TrackBuffer(a2),a1
		CALLLVO FreeJanusMem

		move.l	jd_ExecBase(a2),a6
		move.l	jd_JanusBase(a2),a1
		CALLLVO CloseLibrary
						
		move.l	jd_Segment(a2),-(a7)

		;------ remove device
		move.l	a2,a1
		REMOVE
    
		;------ free up devicNeedToClose		; nothing to close
		CALLLVO	Close			; thats it, we're done
NoNeedToClose	dbra	d3,DummyLoop		; go back for next
		moveq	#0,d0

endMain:
		movem.l (a7)+,d2-d7/a2-a6
		rts

	  
errMain4:
		move.l	a5,a1
		moveq	#DskPartition_SIZEOF,d0
		move.l	_JanusBase,a6
		CALLLVO FreeJanusMem

errMain3:
		move.l	a3,a0
		move.l	_JanusBase,a6
		CALLLVO CleanupJanusSig
errMain2:
		move.l	_SysBase,a6
		move.l	d4,d0
		bmi.s	1$
		CALLLVO FreeSignal
1$
		move.l	_JanusBase,a1
		CALLLVO CloseLibrary
errMain1:
		moveq	#20,d0
		bra	endMain

jdbName:	dc.b	12,'jdisk.device',0
		CNOP	0,4
df0Name:	dc.b	'DF0:',0
		CNOP	0,4

; allow for 8 units because there can be up to 2 hard disks installed on
; the PC side with a maximum of 4 partitions per disk.

dummyname:	dc.b	'JH0:SMB',0
		dc.b	'JH1:SMB',0
		dc.b	'JH2:SMB',0
		dc.b	'JH3:SMB',0
		dc.b	'JH4:SMB',0
		dc.b	'JH5:SMB',0
		dc.b	'JH6:SMB',0
		dc.b	'JH7:SMB',0
		CNOP	0,2
jlName: 	JANUSNAME
	   
	END


