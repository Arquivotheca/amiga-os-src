  
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
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/memory.i"
	INCLUDE "libraries/configvars.i"
	INCLUDE "libraries/dos.i"

	INCLUDE "hardware/intbits.i"

	INCLUDE "janus.i"
	INCLUDE "janusreg.i"
	INCLUDE "janusvar.i"
	INCLUDE "services.i"
	LIST

	INCLUDE "asmsupp.i"

	XDEF	InitTable
	XDEF	Init

	XLIB	OpenLibrary
	XLIB	CloseLibrary
	XLIB	FreeMem
	XLIB	AddIntServer

	XLIB	Open
	XLIB	Read
	XLIB	Close

_LVOGetCurrentBinding	EQU -138	    ; !!! I hate doing this

	XLIB	SetJanusHandler
	XLIB	SetJanusEnable

	XREF	_AbsExecBase

	XREF	Open
	XREF	Close
	XREF	Expunge
	XREF	Null
	XREF	subsysName
	XREF	JanusIntCode
	XREF	AmigaRW


InitTable:
		dc.l	JanusAmiga_SIZEOF+4*MAXHANDLER
		dc.l	funcTable
		dc.l	0
		dc.l	Init


Init:		; ( d0: lib node, a0: seg list )
		movem.l d2-d5/a2-a3/a6,-(sp)

	PUTMSG	1,<'%s/Init: called'>

		;------ do some low level initialization
		move.l	d0,a2
		move.l	a0,ja_SegList(a2)

		lea	subsysName(pc),a0
		move.l	a0,LN_NAME(a2)
		move.l	a0,ja_IntServer+LN_NAME(a2)
		move.l	a0,ja_ReadHandler+LN_NAME(a2)

		move.l	a6,ja_ExecBase(a2)

		;------ open the DOS
		lea	dosLibName(pc),a1
		moveq	#0,d0
		CALLSYS OpenLibrary
		move.l	d0,ja_DOSBase(a2)
		beq	errInit1

		;------ go out and find the board.
		lea	elName(pc),a1
		moveq	#0,d0
		CALLSYS OpenLibrary	

		tst.l	d0
		beq	errInit2

		move.l	d0,a6
		link	a5,#-CurrentBinding_SIZEOF
		move.l	a7,a0
		CALLSYS GetCurrentBinding

		;------ only pay attention to the first board
		move.l	cb_ConfigDev(a7),a0
		bclr	#CDB_CONFIGME,cd_Flags(a0)
		move.l	a2,cd_Driver(a0)
		move.l	cd_BoardAddr(a0),d2
		
		unlk	a5

		;------ done with the expansion.library
		move.l	a6,a1
		move.l	_AbsExecBase,a6
		CALLSYS CloseLibrary

gotBaseAddress:
	IFGE	INFOLEVEL-40
	move.l	d2,-(a7)
	INFOMSG 1,<'sidecar @ 0x%06lx'> 
	addq.l	#4,a7
	ENDC

	INFOMSG 90,<'release 8088 interrupt'>
		;------ set up the various base pointers
		move.l	d2,a0
		add.l	#ParameterOffset,a0
		move.l	d2,a1
		add.l	#IoRegOffset+IoAccessOffset,a1

		;------ clear 8088 interrupts
		move.b	#$ff&~JCNTRLF_CLRPCINT,jio_Control(a1)

		;------ wait for the 8088 to indicate memory is being refreshed
		;------ This must be performed within refresh decay time from
		;------ the write to the lock function being performed in the  
		;------ loop
		move.b	#$ff,jb_Lock(a0)    ; initiate refresh
		move.b	#$ff,jb_Lock(a0)    ; write the data
		move.l	#1000000,d0	    ; timeout loop

checkRefresh:
		tas	jb_Lock(a0)
		beq.s	refreshGoing
	INFOMSG 90,<'config/8088 release reset'>
		move.b	#$ff,jb_8088Go(a0)  ; hold 8088 from going
		tst.b	jio_ReleasePcReset(a1)
		subq.l	#1,d0
		ble	errInit2	    ; it's taken too long!
		bra.s	checkRefresh

		clr.b	jr_Reset(a2)

refreshGoing:
		move.l	d2,ja_ExpanBase(a2)
		beq	errInit2	    ; board not found

		;------ set up the various base pointers
		move.l	d2,a3
		add.l	#ParameterOffset,a3
		move.l	a3,ja_ParamMem(a2)

		move.l	d2,a0
		add.l	#IoRegOffset+IoAccessOffset,a0
		move.l	a0,ja_IoBase(a2)

		;------ a bit of magic -- our library is sized to have
		;------ the interrupt tables at the end
		lea	JanusAmiga_SIZEOF(a2),a0
		move.l	a0,ja_IntHandlers(a2)

		;------ clear out parameter and buffer memory
		CLEAR	d1

		move.l	a3,a0
		move.w	#(ParameterSize>>2)-1,d0
1$
		move.l	d1,(a0)+
		dbra	d0,1$

		move.l	ja_ExpanBase(a2),a0
		move.w	#(BufferSize>>2)-1,d0
2$
		move.l	d1,(a0)+
		dbra	d0,2$

		;------ now set up the parameter memory
		add.l	#WordAccessOffset,a3
		move.w	#JanusBase_SIZEOF,jb_Interrupts(a3)
		move.w	#JanusBase_SIZEOF+2*MAXHANDLER,jb_Parameters(a3)
		move.w	#MAXHANDLER,jb_NumInterrupts(a3)

		;------ initialize the parameter area and the softint array
		moveq	#-1,d1
		move.w	#MAXHANDLER-1,d0
		lea	JanusBase_SIZEOF(a3),a0
3$
		move.l	d1,(a0)+
		dbra	d0,3$

		;------ load in the 8088 boot data file, if it exists
		move.l	a2,a6
		bsr	ReBoot8088

		;------ set up the memory list heads
		move.l	ja_ExpanBase(a2),a0
		lea	jb_BufferMem(a3),a1
		;	D0 contains boot size from above
		move.w	#BufferSize-1,d1
		bsr	Init_MemInit

		move.l	a3,a0
		sub.l	#WordAccessOffset,a0
		lea	jb_ParamMem(a3),a1
		move.w	#JanusBase_SIZEOF+4*MAXHANDLER,d0
		move.w	#ParameterSize-1,d1
		bsr	Init_MemInit

		;------ set up the interrupt handlers
		move.b	#$bf,ja_SpurriousMask(a2)   ; mask out LPT1 & COM2
		lea	ja_IntServer(a2),a1
		move.l	a2,IS_DATA(a1)
		move.l	#JanusIntCode,IS_CODE(a1)
	move.b	#10,LN_PRI(a1)		; !!! testing only
		moveq	#INTB_PORTS,d0
		move.l	ja_ExecBase(a2),a6
		CALLSYS AddIntServer

		;------ initialize amiga read/write 
		lea	ja_ReadHandler(a2),a1
		move.l	a2,IS_DATA(a1)
		move.l	#AmigaRW,IS_CODE(a1)
		moveq	#JSERV_READAMIGA,d0
		exg	a2,a6
		CALLSYS SetJanusHandler

		moveq	#JSERV_READAMIGA,d0
		moveq	#1,d1
		CALLSYS SetJanusEnable
		exg	a2,a6


		;------ and let the pc run!
		move.l	ja_IoBase(a2),a0
		sub.l	#WordAccessOffset,a3
		UNLOCK	jb_8088Go(a3)
		UNLOCK	jb_Lock(a3)

		;------ enable interrupts from the pc
		move.b	#$ff&~JCNTRLF_ENABLEINT,jio_Control(a0)

		;------ set up return code
		move.l	a2,d0
endInit: 

	PUTMSG	1,<'%s/Init: ending'>

		movem.l (sp)+,d2-d5/a2-a3/a6
		rts

errInit3:
	PUTMSG	1,<'%s/errInit3: called'>
		move.l	d4,d1
		CALLSYS Close

errInit2:
	PUTMSG	1,<'%s/errInit2: called'>
		move.l	ja_DOSBase(a2),a1
		CALLSYS CloseLibrary

errInit1:

	PUTMSG	1,<'%s/errInit1: called'>

		;------ free up library node
		move.l	a2,a1
		CLEAR	d0
		move.w	LIB_NEGSIZE(a2),d0
		sub.w	d0,a1
		add.w	LIB_POSSIZE(a2),d0
		LINKEXE FreeMem

		moveq	#0,d0
		bra	endInit 


; initialize a JanusMemHead structure
Init_MemInit	; (a0: byte 68k base, a1: word memhead, d0: first, d1: max)
		move.l	d2,-(sp)
		move.l	a0,jmh_68000Base(a1)	; set base address

		addq.w	#3,d0
		and.w	#$fffc,d0
		move.w	d0,jmh_First(a1)
		move.w	d1,jmh_Max(a1)

		move.w	d1,d2
		sub.w	d0,d2
		move.w	d2,jmh_Free(a1)


		;------ now set up the first mem chunk
		CLEAR	d1
		move.w	d0,d1
		move.w	#-1,jmc_Next(a0,d1.l)	; -1 means no next chunk
		move.w	d2,jmc_Size(a0,d1.l)	; size is # of free bytes -1

		move.l	(sp)+,d2
		rts


FUNCDEF 	MACRO
		XREF	\1
		dc.l	\1
		ENDM

		LIBINIT

funcTable:
		dc.l	Open
		dc.l	Close
		dc.l	Expunge
		dc.l	Null

		INCLUDE "jfuncs.i"

		dc.l	-1

;------ ReBoot8088 ---------------------------------------------------------
;------ load in the 8088 boot data file, if it exists
ReBoot8088:
		movem.l d2-d5/a2/a6,-(a7)
		move.l	a6,a2
		moveq	#0,d5		    ; count of bytes loaded

		lea	janusBootName(pc),a0
		move.l	a0,d1
		move.l	#MODE_OLDFILE,d2
		move.l	ja_DOSBase(a2),a6
		CALLSYS Open
	IFGE	INFOLEVEL-20
	move.l	d5,-(a7)
	PUTMSG	20,<'%s/boot Open: %lx'>
	addq.l	#4,sp
	ENDC
		move.l	d0,d4		    ; save open handle
		beq.s	readNothing 

readLoop:
		move.l	d4,d1		    ; get file handle	 
		move.l	ja_ExpanBase(a2),d2
		add.l	d5,d2		    ;	read buffer
		move.l	#512,d3 	    ;	and length
		CALLSYS Read
		tst.l	d0
		bmi	readError
		beq.s	readEOF
	IFGE	INFOLEVEL-20
	suba.l	a0,a0
	move.l	0(a0,d2.l),-(a7)
	PUTMSG	20,<'%s/boot Read: %08lx'>
	addq.l	#4,sp
	ENDC
		add.l	d0,d5		    ; update buffer position
		bra.s	readLoop
	 
readEOF:
		move.l	d4,d1
		CALLSYS Close

readNothing:
	IFGE	INFOLEVEL-20
	move.l	d5,-(a7)
	PUTMSG	20,<'%s/ReBoot8088: %ld bytes booted'>
	addq.l	#4,sp
	ENDC
		move.l	d5,d0
		      
readEnd:
		movem.l (a7)+,d2-d5/a2/a6
		rts

readError:
		move.l	d4,d1
		CALLSYS Close
		moveq	#0,d0
		bra.s	readEnd

janusBootName:	dc.b	'sys:sidecar/pc.boot',0
elName		dc.b	'expansion.library',0
dosLibName:	DOSNAME

		ds.w	0

	END


