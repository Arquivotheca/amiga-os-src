*
* C initial startup procedure under AmigaDOS
* 
* Use the following command line to make c.o
* asm -iINCLUDE: ac.a
*
* Use the following command line to make cres.o
* asm -dRESIDENT -iINCLUDE: -ocres.o ac.a
*

	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"workbench/startup.i"

MEMFLAGS	EQU	MEMF_CLEAR+MEMF_PUBLIC
AbsExecBase	EQU	4

; some usefull macros:

CALLSYS	MACRO
	XREF	_LVO\1
	CALLLIB	_LVO\1
	ENDM
	
	XDEF	_XCEXIT		; exit(code) is standard way to leave C.
	XDEF	_@XCEXIT
	
	XREF	_LinkerDB	; linker defined base value
	XREF	__BSSBAS	; linker defined base of BSS
	XREF	__BSSLEN	; linker defined length of BSS
	IFD	RESIDENT
		XREF	_RESLEN
		XREF	_RESBASE
		XREF	_NEWDATAL
		XREF	__stack
	ENDC
	
	section text,code

	XREF	__xmain			; Name of C program to start with.
	XREF	_MemCleanup		; Free all allocated memory
	XREF	___fpinit		; initialize floating point
	XREF	___fpterm		; terminate floating point

start:
	movem.l	d1-d6/a0-a6,-(a7)

	move.l	a0,a2			; save command pointer
	move.l	d0,d2			; and command length
	lea	_LinkerDB,a4		; load base register
	move.l	AbsExecBase.W,a6

	IFND	RESIDENT
		lea	__BSSBAS,a3		; get base of BSS
		moveq	#0,d1
		move.l	#__BSSLEN,d0		; get length of BSS in longwords
		bra.s	clr_lp			; and clear for length given
clr_bss		move.l	d1,(a3)+
clr_lp		dbf	d0,clr_bss
		move.l	a7,__StackPtr(A4)	; Save stack ptr
		move.l	a6,_SysBase(A4)
	ENDC
	

	IFD	RESIDENT
		movem.l	d2,-(a7)
		movem.l	a0-a2,-(a7)

;------		get the size of the stack, if CLI use cli_DefaultStack
;------					   if WB use a7 - TC_SPLOWER
		move.l	ThisTask(a6),A3
		move.l	pr_CLI(A3),d1
		beq.s	fromwb
		lsl.l	#2,d1
		move.l	d1,a0
		move.l	cli_DefaultStack(a0),d1
		lsl.l	#2,d1			; # longwords -> # bytes
		bra.s	dostack

fromwb
		move.l	a7,d1
		sub.l	TC_SPLOWER(a3),d1
dostack
		moveq	#0,d2			; use d2 as flag for newstack or not
		move.l	#_RESLEN,d0
		cmp.l	__stack(a4),d1		; This a4 is in the original
						; set of data
		bcc.s	nochange	
		move.l	__stack(a4),d1
		add.l	d1,d0			; increase size of mem for new stack
		moveq	#1,d2			; set flag
		
nochange
		move.l	d1,a3			; save stacksize to set up stack checking
		move.l	#MEMFLAGS,d1
		CALLSYS	AllocMem
		tst.l	d0
		bne.s	ok1
		movem.l	(a7)+,d2/a0-a2		; undo stack
		move.l	ThisTask(a6),A3
		move.l	pr_CLI(A3),d1
		bne.s	notfromwb

		lea	pr_MsgPort(A3),a0	; our process base
		CALLSYS	WaitPort
		lea	pr_MsgPort(A3),a0	; our process base
		CALLSYS	GetMsg
		move.l	d0,a2
		CALLSYS	Forbid
		move.l	a2,a1
		CALLSYS	ReplyMsg

notfromwb
		move.l	#127,d0
		movem.l	(a7)+,d1-d6/a0-a6
		rts

ok1		move.l	d0,a0
		move.l	d0,a2

	;a2 now has difference
		move.l	d0,a1
		move.l	#_NEWDATAL,d0
		sub.l	#_RESBASE,a4
	;copy data over
cpy		move.l	(a4)+,(a0)+
		subq.l	#1,d0
		bne.s	cpy
	;a4 now points at number of relocs
		move.l	(a4)+,d0
reloc		beq.s	nreloc
		move.l	a1,a0
		add.l	(a4)+,a0		; a0 now has add of reloc
		add.l	(a0),a2
		move.l	a2,(a0)
		move.l	a1,a2			; restore offset
		subq.l	#1,d0
		bra.s	reloc

nreloc		move.l	a1,a4			; set up new base register
		add.l	#_RESBASE,a4

		move.l	#_RESLEN,realdatasize(a4)
		movem.l	(a7)+,a0-a2

		move.l	a6,_SysBase(A4)
		tst.b	d2
		movem.l	(a7)+,d2		; restore d2
		movem.l	a7,__StackPtr(A4)	; Save stack ptr (movem doesn't
						; change flags
		beq.s	nochg2

;------		set up new stack
		move.l	a4,d0
		sub.l	#_RESBASE,d0
		add.l	#_RESLEN,d0
		add.l	__stack(a4),d0		; here a4 will be pointing at the
						; new data, but _stack will be the
						; same if all goes well

		sub.l	#128,d0			; 128 down for good measure
		move.l	d0,a7
		move.l	__stack(a4),d0
		move.l	d0,4(a7)		; fill in size of new stack	
		add.l	d0,realdatasize(a4)	; need to know how much to free later

nochg2
;------		Set _base for stack checking
		move.l	a7,d1
		sub.l	a3,d1			; get top of stack
		add.l	#128,D1			; allow for parms overflow
		move.l	D1,__base(A4)		; save for stack checking

	ENDC

clrwb
	clr.l	_WBenchMsg(A4)

;------	clear any pending signals
	moveq	#0,d0
	move.l	#$00003000,d1
	CALLSYS	SetSignal
	

;------ attempt to open DOS library:
	lea	DOSName(PC),A1
	moveq.l	#0,D0
	CALLSYS	OpenLibrary
	move.l	D0,_DOSBase(A4)
	bne.s	ok2
	moveq.l	#100,d0
	bra.w	exit2

ok2
;------	are we running as a son of Workbench?
	move.l	ThisTask(a6),A3
	move.l	pr_CurrentDir(A3),_curdir(A4)
	tst.l	pr_CLI(A3)
	beq.s	fromWorkbench
	
*=======================================================================
*====== CLI Startup Code ===============================================
*=======================================================================
*
* Entry: D2 = command length
*	A2 = Command pointer

fromCLI:
	IFND	RESIDENT		; we need to set _base if not resident
		move.l	a7,D0		; get top of stack
		sub.l	4(a7),D0	; compute bottom
		add.l	#128,D0		; allow for parms overflow
		move.l	D0,__base(A4)	; save for stack checking
	ENDC
	
;------	find command name:
	move.l	ThisTask(a6),A3
	move.l	pr_CLI(a3),a0
	add.l	a0,a0		; bcpl pointer conversion
	add.l	a0,a0
	move.l	cli_CommandName(a0),a1
	add.l	a1,a1		; bcpl pointer conversion
	add.l	a1,a1

;------	collect parameters:
	move.b	(a1)+,d0
	move.l	a1,__ProgramName(A4)


	move.l	d2,-(A7)		; push command line length
	move.l	a2,-(A7)		; push command line address
	bra.s	main

*=======================================================================
*====== Workbench Startup Code =========================================
*=======================================================================

fromWorkbench:

	ifnd	RESIDENT	; we need to set _base if not resident
		move.l	TC_SPLOWER(a3),__base(A4)	; set base of stack
		moveq	#127,d0
		addq.l	#1,d0		; Efficient way of getting in 128
		add.l	d0,__base(A4)	; allow for parms overflow
	endc

;------	we are now set up. wait for a message from our starter
	lea	pr_MsgPort(A3),a0	; our process base
	CALLSYS	WaitPort
	lea	pr_MsgPort(A3),a0	; our process base
	CALLSYS	GetMsg
	move.l	d0,_WBenchMsg(a4)

	move.l	d0,a2			; get first argument
	move.l	sm_ArgList(a2),d0
	beq.s	do_main
	move.l	_DOSBase(a4),a6
	move.l	d0,a0
	move.l	wa_Lock(a0),d1
	move.l	d1,_curdir(A4)
	CALLSYS	CurrentDir
do_main
	move.l	_WBenchMsg(A4),a0	; get address of workbench message
	move.l	sm_ArgList(a0),a0	; get address of arguments
	move.l	wa_Name(a0),__ProgramName(A4)	; get name of program
	move.l	#0,-(sp)		; command line length
	move.l	#0,-(sp)		; command line address
	
*=======================================================================
*====== common code ====================================================
*=======================================================================

main:	jsr	___fpinit(PC)		; Initialize floating point
	jsr	__xmain(PC)		; call C entrypoint
	moveq.l	#0,d0			; set successful status
	bra.s	exit2

_XCEXIT:
	move.l	4(SP),d0		; extract return code
_@XCEXIT:
exit2
	move.l	d0,-(a7)
	move.l	__ONEXIT(A4),d0		; exit trap function?
	beq.s	exit3
	move.l	d0,a0
	jsr	(a0)
exit3	move.l	__Children(a4),d0	; if we have any children
	beq.s	exit4			;	orphan them
	move.l	d0,a0
	move.l	(a0),a0
	jsr	(a0)
exit4	jsr	_MemCleanup(PC)		; cleanup leftover memory alloc.
	move.l	AbsExecBase.W,a6
	move.l	_DOSBase(A4),a1
	CALLSYS	CloseLibrary		; close Dos library

	jsr	___fpterm(PC)		; clean up any floating point

;------	if we ran from CLI, skip workbench cleanup:
	tst.l	_WBenchMsg(A4)
	beq.s	exitToDOS

;------	return the startup message to our parent
;------	we forbid so workbench can't UnLoadSeg() us
;------	before we are done:
	move.l	AbsExecBase.W,A6
	CALLSYS	Forbid
	move.l	_WBenchMsg(a4),a1
	CALLSYS	ReplyMsg

exitToDOS:
	IFD	RESIDENT
		move.l	realdatasize(a4),d0
		move.l	a4,a1
		sub.l	#_RESBASE,a1
		move.l	AbsExecBase.W,a6
		move.l	(A7)+,d6
		movea.l	__StackPtr(a4),a5
		CALLSYS	FreeMem
		move.l	d6,d0
		movea.l	a5,sp
	ELSE
		move.l	(A7)+,D0
		movea.l	__StackPtr(a4),SP	; restore stack ptr
	ENDC
	
	movem.l	(a7)+,d1-d6/a0-a6
	rts


DOSName	dc.b	'dos.library',0

	section	__MERGED,BSS

	XREF	_DOSBase

	XDEF	_NULL,_SysBase,_WBenchMsg
	XDEF	_curdir,__mbase,__mnext,__msize,__tsize
	XDEF	__oserr,__OSERR,__FPERR,__SIGFPE,__ONERR,__ONEXIT,__ONBREAK
	XDEF	__SIGINT
	XDEF	__ProgramName,__StackPtr,__base
	XDEF	__Children

	IFD	RESIDENT
realdatasize	ds.l	1	; size of memory allocated for data +
				; possible stack
	ENDC
	
_NULL		ds.l	1
__base		ds.l	1	; base of stack
__mbase		ds.l	1	; base of memory pool
__mnext		ds.l	1	; next available memory location
__msize		ds.l	1	; size of memory pool
__tsize		ds.l	1	; total size?
__oserr		equ	*
__OSERR		ds.l	1
__FPERR		ds.l	1
__SIGFPE	ds.l	1
__SIGINT	ds.l	1
__ONERR		ds.l	1
__ONEXIT	ds.l	1
__ONBREAK	ds.l	1
_curdir		ds.l	1
_SysBase	ds.l	1
_WBenchMsg	ds.l	1
__StackPtr	ds.l	1
__ProgramName	ds.l	1
__Children	ds.l	1


		END
