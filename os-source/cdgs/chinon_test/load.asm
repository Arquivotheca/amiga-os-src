		SECTION driver,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/nodes.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/resident.i"
		INCLUDE	"exec/alerts.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/execbase.i"
		INCLUDE	"libraries/configvars.i"
		INCLUDE	"libraries/expansion.i"
		INCLUDE	"devices/scsidisk.i"

		INCLUDE	"printf_reloc.mac"
		LIST

		INCLUDE	"load_rev.i"

;		DEBUGENABLE

		XREF	_AbsExecBase,_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOTaggedOpenLibrary
		XREF	_LVOAddTask,_LVOAlert,_LVOAddDevice
		XREF	_LVOAllocMem,_LVOFreeMem,_LVOForbid,_LVOPermit


;==============================================================================
; This is just treated as a normal RomTag and allows us to init the fake
; ConfigDev structure for autobooting.  Our priority should be > RomBoot
;==============================================================================
InitDescriptor	DC.W	RTC_MATCHWORD		so we can be found
		DC.L	InitDescriptor		firewall consistency check
		DC.L	DiagEnd			how to skip over this code
		DC.B	RTW_COLDSTART		flags
		DC.B	VERSION			version number
		DC.B	NT_LIBRARY		node type when we are linked
		DC.B	-49			priority to run this task at
		DC.L	Name			root name of this device
		DC.L	IDString		descriptive ID string
		DC.L	Init			code to call for initialisation


Name		dc.b	'chinon_loader',0
IDString	dc.b	VSTRING
		ds.w	0
args		dc.b	'test',$0a,0
		ds.w	0


;==============================================================================
; This code is actually called at config time and we use it to resolve all of
; the absolute references in the RomTag so that our init routine can be found
; at "binddrivers" time.  On entry to the routine, useful registers are:-
;
** A7 -- points to at least 2K of stack
** A6 -- ExecBase
**
;==============================================================================
Init		movem.l	d2-d3/a2-a4,-(sp)	stash our registers
	printf <'in chinon_loader'>

		pea	TestCodeEnd-TestCode      size (bytes) of seglist
		pea	TestCode(pc)		  address of first byte

		lea.l	_LVOAllocMem(a6),a0	allocfunc
		lea.l	_LVOFreeMem(a6),a1	freefunc
		suba.l	a2,a2			no overlay table
		lea.l	MyRead(pc),a3		readfunc
		move.l	sp,d1			read handle
		bsr	LoadSeg			call loadseg ^
		addq.w	#8,sp			drop readhandle

	printf <'Loaded seglist $%lx'>,d0
		; have seglist, point romtag init at it
		lsl.l	#2,d0			BPTR->CPTR
		beq.s	10$			load failure - d0 == 0
		move.l	d0,a1
		addq.w	#4,a1			point at first instruction
		; call it
		move.l	#5,d0
		lea	args(pc),a0		"test", no args
		movem.l	d2-d7/a2-a6,-(sp)
		jsr	(a1)			call __main
		movem.l	(sp)+,d2-d7/a2-a6

		moveq.l	#1,d0			indicate success
10$		movem.l	(sp)+,d2-d3/a2-a4	restore our registers
		rts

		XREF _LVORawPutChar
		XREF _LVORawDoFmt
kprintf:	; ( format, {values} )
		movem.l a2/a6,-(sp)

		move.l	3*4(sp),a0       ;Get the FormatString pointer
		lea.l	4*4(sp),a1       ;Get the pointer to the DataStream
		lea.l	stuffChar(pc),a2
		move.l	4,a6
		jsr	_LVORawDoFmt(a6)

		movem.l (sp)+,a2/a6
		rts

	;------ PutChProc function used by RawDoFmt -----------
stuffChar:
		tst.b	d0
		beq.s	10$
		move.l	a6,-(sp)
		move.l	4,a6
		jsr _LVORawPutChar(a6)
		move.l	(sp)+,a6
10$		rts

;==============================================================================
; read from ROM.  Uses readhandle with 0(rh) = pointer,
; 4(rh) = number of bytes left to EOF.  Inputs: d1 - rh, a0 - buf, d0 - size
; returns number of bytes xferred or 0 for EOF.
;==============================================================================
MyRead:		movem.l	d2-d4/a2,-(sp)
;	IFD DEBUG_CODE
;	move.l a0,-(sp)
;	ENDC
		move.l	d0,d3			save size and
		move.l	d0,d2			original size
		move.l	d1,a2			read handle
		move.l	0(a2),a1		current pointer for read
		move.l	4(a2),d0		number of bytes left to read
		moveq	#0,d1
1$		tst.l	d3			we're done
		beq.s	ReadDone
		tst.l	d0			out of bytes, return EOF
		beq.s	ReadEOF
		move.b	(a1)+,(a0)+
		subq.l	#1,d0
		subq.l	#1,d3
		bra.s	1$

ReadDone	bsr.s	SaveRH		save modified readhandle
		move.l	d2,d0			we read all that was asked
;	IFD DEBUG_CODE
;	move.l (sp)+,a0
;	printf <'read $%lx bytes to $%lx, first = $%08lx'>,d0,a0,(a0)
;	ENDC
		movem.l	(sp)+,d2-d4/a2		original count
		rts

ReadEOF		bsr.s	SaveRH		save modified readhandle
		move.l	d2,d0			original count
		sub.l	d3,d0			 - number not read = # read
;	IFD DEBUG_CODE
;	addq.w	#4,sp
;	printf <'EOF, read $%lx bytes'>,d0
;	ENDC
		movem.l	(sp)+,d2-d4/a2		original count
		rts

SaveRH ;printf <'Next read from $%lx, %ld bytes left'>,a1,d0
		move.l	a1,(a2)			save modified readhandle
		move.l	d0,4(a2)
		rts

	INCLUDE	"loadseg.asm"

; this is the chinon test code
TestCode:
	INCLUDE "test_code.asm"
TestCodeEnd:

DiagEnd:	ds.b	0
		;config code will copy from the beginning to here

		END
