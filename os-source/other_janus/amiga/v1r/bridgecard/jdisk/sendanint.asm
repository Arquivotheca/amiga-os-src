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
	INCLUDE "janus/i86block.i"
	INCLUDE "janus/janus.i"
	INCLUDE "janus/setupsig.i"
	INCLUDE "janus/services.i"
	LIST

	INCLUDE "djmount.i" 
	INCLUDE "asmsupp.i"

	XLVO	AllocMem
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

;---------------------------------------------------------------------
	SECTION section,data
JanusBase	DS.L	1
;---------------------------------------------------------------------
	SECTION section,code

_main:
		movem.l d2-d7/a2-a6,-(a7)

	INFOMSG	50,<'-----------------------------------------'>
	INFOMSG	50,<'SendAnInt: Starting up!'>

		;------ get janus.library		 
		lea	jlName(pc),a1
		moveq	#0,d0
		move.l	_SysBase,a6
		CALLLVO OpenLibrary
		move.l	d0,JanusBase	
		beq	errMain1	    ; library not found
	move.l	d0,-(a7)
	INFOMSG 50,<'SendAnInt: JanusBase=0x%lx'>
	addq.l	#4,a7

		;------ setup signal handshake for this initialization code
		moveq	#-1,D0
		CALLLVO AllocSignal
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'SendAnInt: Signal=$%lx'>
	ADDQ.L	#4,SP
		move.l	d0,d1
		move.l	d0,d4		    ; save signal to free later

		MOVEQ.L	#15,D0			; interrupt # 15
		CLR.L	D2			; don't need any memory
		CLR.L	D3
		MOVE.L	JanusBase,A6
		CALLLVO	SetupJanusSig
	INFOMSG	50,<'SendAnInt: Setup the JanusSig'>
		MOVE.L	D0,A3			; save int pointer
		MOVEQ.L	#15,D0
		MOVE.L	JanusBase,A6
		CALLLVO	SendJanusInt
	INFOMSG	50,<'SendAnInt: Interrupt sent to PC'>
;	MOVE.L	_SysBase,A6
;	MOVE.L	ss_SigMask(A3),D0
;	CALLLVO	Wait			; wait for reply
;	INFOMSG	50,<'Back from wait'>

	INFOMSG	50,<'Delaying for a little while (zzzzz)'>
		move.l	_DOSBase,A6
		moveq.l	#100,d1
		CALLLVO	Delay

	INFOMSG	50,<'SendAnInt: Cleaning up now!'>
		move.l	JanusBase,A6
		move.l	a3,a0
		CALLLVO CleanupJanusSig

		move.l	_SysBase,a6
		move.l	d4,d0
		CALLLVO FreeSignal

		move.l	JanusBase,a1
		CALLLVO CloseLibrary
	INFOMSG 50,<'SendAnInt: All done, bye-bye!'>
		moveq	#0,d0

endMain:
		movem.l (a7)+,d2-d7/a2-a6
		rts

errMain1:
		moveq.l	#20,d0
		bra	endMain

jlName: 	JANUSNAME
	   
	END


