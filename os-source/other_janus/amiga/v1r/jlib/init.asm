  
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
	IFGE	INFOLEVEL-1
	move.l	d2,-(a7)
	INFOMSG 1,<'sidecar @ 0x%06lx'> 
	addq.l	#4,a7
	ENDC
		bsr	GetPrefs		; get configuration byte

	INFOMSG 1,<'release 8088 interrupt'>
		;------ set up the various base pointers
		move.l	d2,a0
		add.l	#ParameterOffset,a0
		move.l	d2,a1
		add.l	#IoRegOffset+IoAccessOffset,a1
;==============================================================================
; Set up offset to the keyboard data register depending on which machine we
; are running in.  If the PCConfiguration register can be written and read
; then we are running on a PC emulator board in the A2000.  If not, then we
; are running on an Amiga 1000 with a Sidecar.  ja_KeyboardRegisterOffset
; will contain the signed word offset from IOBase when we are done.
;==============================================================================
		move.b	#$ff,jio_Control(a1)	; ****FRANKS FIX****
		move.w	d0,-(sp)	save config value and assume sidecar
		move.w	#jio_1000KeyboardData,ja_KeyboardRegisterOffset(a2)
		move.b	#$7f,jio_PCconfiguration(a1)
		move.b	jio_PCconfiguration(a1),d0
		andi.b	#$7f,d0
		cmpi.b	#$7f,d0
		bne.s	WasA1000		it's a sidecar
		move.b	#$00,jio_PCconfiguration(a1)
		move.b	jio_PCconfiguration(a1),d0
		andi.b	#$7f,d0
		cmpi.b	#$00,d0
		bne.s	WasA1000		it's a sidecar
		move.w	#jio_2000KeyboardData,ja_KeyboardRegisterOffset(a2)

  		;------ set PC configuration (D0 set from GetPrefs)
		;------ won't hurt to do this in a sidecar anyway
WasA1000	move.w	(sp)+,d0		get back config byte
		move.b  d0,jio_PCconfiguration(a1)
;==============================================================================
; now we have to set up the offset to the pc boot code so we can tell the PC
; where it is.  This value is set by the pcprefs program and can be any of
; A000, D000 or E000.  If the value is D000 and we are running an AT then this
; should be set to $D400 and the boot code loaded $4000 bytes further into the
; buffer memory.  We read the value back in case this is a Sidecar in which
; case the mem config will have been set by dip switches.
;==============================================================================
		move.b	jio_PCconfiguration(a1),d0
		rol.b	#3,d0			get mem bits to bits 0 and 1
		andi.w	#3,d0
		asl.w	#1,d0			make index for getting offset
		move.w	MemOffs(pc,d0.w),ja_ATOffset(a2)
		bra.s	Check4AT		check for special d000 offset

MemOffs		DC.W	0,$a000,$d000,$e000

Check4AT	btst.b	#7,jio_PCconfiguration(a1)	is this an AT
		bne.s	NotAnAT			no, config bit is high
		cmpi.w	#$d000,ja_ATOffset(a2)		yes, check for d000
		bne.s	NotAnAT				nope, mem is at A000
		move.w	#$d400,ja_ATOffset(a2)		AT can't see d000

		;------ clear 8088 interrupts
NotAnAT		move.b	#$ff&~JCNTRLF_CLRPCINT,jio_Control(a1)
		;------ wait for the 8088 to indicate memory is being refreshed
		;------ This must be performed within refresh decay time from
		;------ the write to the lock function being performed in the  
		;------ loop
		move.w	#1000,d0
10$		move.b	#$ff,jb_Lock(a0)    ; initiate refresh
		dbra	d0,10$

		move.b	#$ff,jb_Lock(a0)    ; write the data
		move.l	#1000000,d0	    ; timeout loop
	INFOMSG 90,<'config/8088 release reset'>

checkRefresh:
		tas	jb_Lock(a0)
		beq.s	refreshGoing
		move.b	#$ff,jb_8088Go(a0)  ; hold 8088 from going
		tst.b	jio_ReleasePcReset(a1)
		subq.l	#1,d0
		ble	errInit2	    ; it's taken too long!
		bra.s	checkRefresh

refreshGoing:
	INFOMSG	90,<'OK, AT/XT has woken up at last!'>
		;------ set up the various base pointers
ReallyGoing	move.l	d2,ja_ExpanBase(a2)
		beq	errInit2	    ; board not found
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

MustBeClear	clr.l	(a3)		; check refresh is really going
		tst.l	(a3)		; before clearing memory
		bne.s	MustBeClear

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

; need to set up the segment for the boot file so PC knows where it is
		lea.l	jb_BufferMem(a3),a0
		move.w	ja_ATOffset(a2),jmh_8088Segment(a0)

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

	PUTMSG	1,<'errInit1: called'>

		;------ free up library node
		move.l	a2,a1
		CLEAR	d0
		move.w	LIB_NEGSIZE(a2),d0
		sub.w	d0,a1
		add.w	LIB_POSSIZE(a2),d0
;		LINKEXE FreeMem

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

;------ ReBoot8088 ----------------------------------------------------------
;------ load in the 8088 boot data file, if it exists.  Returns bytes loaded
;----------------------------------------------------------------------------
ReBoot8088:
	INFOMSG	1,<'Trying to boot pc.boot'>
		movem.l d2-d5/a2/a6,-(a7)
		clr.l	d5			; count of bytes loaded
		move.l	a6,a2			; janus library base
		lea	XTBootName(pc),a0	; get filename
		move.l	a0,d1
		clr.w	ja_ATFlag(a6)		; assume a PC

; fix for PC/AT.  Load a different boot file if it's an AT

		movea.l	ja_IoBase(a2),a0	; XT or AT board
		btst.b	#7,jio_PCconfiguration(a0)
		bne.s	LoadXTBoot		; bootfile OK, it's an XT
		move.w	#1,ja_ATFlag(a6)	; it's an AT
		lea.l	ATBootName(pc),a0
		move.l	a0,d1			; do the boot for an AT
		cmpi.w	#$d400,ja_ATOffset(a6)	; maybe we need offset
		bne.s	LoadXTBoot		; nope, mem is at a000
		move.l	#$4000,d5		; AT can't see lower 16K

LoadXTBoot	move.l	#MODE_OLDFILE,d2
		move.l	ja_DOSBase(a2),a6	; pointer to DOS library
		CALLSYS Open
		move.l	d0,d4			; save open handle
		beq.s	readNothing 		; no file to read
	INFOMSG	1,<'File opened OK'>

		move.l	d4,d1			; get file handle	 
		move.l	ja_ExpanBase(a2),d2	; and buffer to read into

; fix for PC/AT.  Load at an extra offset of $4000 for the AT

		tst.l	d5			; if d5 <> $4000
		beq.s	ItsAnXT			; offset OK, it's an XT
		addi.l	#$4000,d2		; else increment buffer ptr

ItsAnXT		move.l	#100000,d3		; allow for maximum length
	IFGE	INFOLEVEL-10
	move.l	d2,-(sp)
	INFOMSG	1,<'pc.boot loaded at %lx'>
	addq.l	#4,sp
	ENDC
		CALLSYS Read
		tst.l	d0			; did we get the file in ?
		bmi.s	readError		; nope, some kind of error
		add.l	d0,d5			; stash number of bytes
readError:
		move.l	d4,d1			; OK, close the file
		CALLSYS Close
readNothing:
		move.l	d5,d0			; return # bytes loaded
	IFGE	INFOLEVEL-1
	move.l	d5,-(sp)
	INFOMSG	1,<'Number of bytes loaded: %ld'>
	addq.l	#4,sp
	move.l	ja_ExpanBase(a2),-(sp)
	INFOMSG	1,<'Bytes loaded to address: $%lx'>
	addq.l	#4,sp
	ENDC
		movem.l (a7)+,d2-d5/a2/a6
		rts

;===========================================================================
; Fetch PC configuration byte from the preferences file in sys:sidecar.
;===========================================================================

GetPrefs	movem.l	d1-d4/a0-a1/a6,-(sp)	can't clobber anything
	INFOMSG	1,<'Getting preferences'>
		move.w	#$fe00,-(sp)		space for prefs data

		movea.l	ja_IoBase(a2),a0	; XT or AT board
		btst.b	#7,jio_PCconfiguration(a0)
		bne.s	XTPrefs			; prefs OK, it's an XT
		move.w	#$de00,(sp)

XTPrefs		movea.l	ja_DOSBase(a2),a6	get DOS library
		lea.l	PrefsName(pc),a0
		move.l	a0,d1
		move.l	#MODE_OLDFILE,d2
		CALLSYS	Open
		move.l	d0,d4			did we get the file ?
		beq.s	UseDefaults		no, use default values
		move.l	sp,d2
		moveq.l	#1,d3			only 1 byte to read
		move.l	d4,d1
		CALLSYS	Read
		move.l	d4,d1			close the file
		CALLSYS	Close
	INFOMSG	1,<'Opened preferences file OK'>
UseDefaults	clr.l	d0
		move.b	(sp),d0			return correct value
	MOVE.L	d0,-(SP)
	INFOMSG	1,<'value stored = %ld'>
	ADDQ.L	#4,sp
		addq.l	#2,sp			scrap one on stack
		movem.l	(sp)+,d1-d4/a0-a1/a6
		rts

XTBootName:	dc.b	'sys:sidecar/pc.boot',0
		CNOP	0,2
ATBootName:	dc.b	'sys:sidecar/at.boot',0
		CNOP	0,2
elName		dc.b	'expansion.library',0
		CNOP	0,2
PrefsName	dc.b	'sys:sidecar/2500prefs',0
		CNOP	0,2
dosLibName:	DOSNAME
		CNOP	0,2
		END


