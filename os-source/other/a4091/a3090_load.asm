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
		INCLUDE "internal/librarytags.i"

		INCLUDE	"modifiers.i"
		INCLUDE	"scsidisk_rev.i"
		INCLUDE	"printf_reloc.mac"
		LIST

;		DEBUGENABLE

		XREF	_AbsExecBase,_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOTaggedOpenLibrary
		XREF	_LVOAddTask,_LVOAlert,_LVOAddDevice
		XREF	_LVOAllocMem,_LVOFreeMem,_LVOForbid,_LVOPermit

		XREF	_LVOAllocConfigDev,_LVOFindResident
		XREF	_LVOGetCurrentBinding
		XREF	_LVOFindTask,_LVOWait,_LVOSetSignal

; These are Dave Haynie's Z-3 A3090 autoconfig registers for the rom.
;
;	A3090 ROM Header
;
;	- 9/28/92 DBH : Original Version
;
;	This is the AUTOCONFIG ROM information for the A3090.
;	All this must be kept at the start of any A3090 ROM,
;	since its necessary to configure the A3090 board.  At
;	some point, presumably autoboot codes and the device
;	driver go here as well.
;
; A3090 Configuration Table
;
; Register	Setting
;    00		10000000 	Zorro III, Nolink, NoROM*, NoChain, 16MB
;    04		01010100	Product $54
;    08		00110000	I/O, Shutup, Extended, Logical=Physical
;    10		00000010	Manufacturer's code ($0202)
;    14		00000010
;    18-24	........	Serial number (version)
;    28		00000010	ROM Vector HI,LO ($0200)*
;    2C		00000000
;  Others	00000000	up to 3c/13c
;
;* The autoboot ROM is left out here for now, since I don't have any
;  ROM code to put in.  To get autoboot, put the autoboot rom stuff at
;  "codestart" and set bit 4 in register 0.
;
;  This table shows the actual ROM bit encodings, including inversions,
;  for each location of the ROM.  Note of course its only the most 
;  significant nybble we're concerned with, and that resisters are split
;  between REGNO and REGNO+$100 for 32-bit autoconfig units.
;
;
;		 High Nybble
; Register	  N	N+$100
;    00		1000	 0000		; 1001 0000	ROM enabled
;    04		1010	 1011
;    08		1100	 1111
;    0C		1111	 1111
;    10		1111	 1101
;    14		1111	 1101
;    18		1111	 1111		; version hi 
;    1C		1111	 1111		; version low
;    20		1111	 1111		; revision hi
;    24		1111	 1111		; revision low
;    28		1111	 1101		; rom vector high
;    2C		1111	 1111		; rom vector low
;    30		1111	 1111
;    ...
;    FC		1111	 1111

	org	$00000000
	
	; The "even" half of the autoconfig registers go here

	dc.b	$9f,$af,$cf,$ff		; 00-0c		ROM enabled
	dc.b	$ff,$ff,~(VERSION>>12),~(VERSION>>4), 	; 10-1c
	dc.b	~(REVISION>>12),~(REVISION>>4),$ff,$ff  ; 20-2c
	dc.b	$ff,$ff,$ff,$ff		; 30-3c
	dc.b	$ff,$ff,$ff,$ff		; 40-4c
	dc.b	$ff,$ff,$ff,$ff		; 50-5c
	dc.b	$ff,$ff,$ff,$ff		; 60-6c
	dc.b	$ff,$ff,$ff,$ff		; 70-7c
	dc.b	$ff,$ff,$ff,$ff		; 80-8c
	dc.b	$ff,$ff,$ff,$ff		; 90-9c
	dc.b	$ff,$ff,$ff,$ff		; a0-ac
	dc.b	$ff,$ff,$ff,$ff		; b0-bc
	dc.b	$ff,$ff,$ff,$ff		; c0-cc
	dc.b	$ff,$ff,$ff,$ff		; d0-dc
	dc.b	$ff,$ff,$ff,$ff		; e0-ec
	dc.b	$ff,$ff,$ff,$ff		; f0-fc

	org	$00000040

	; The "odd" half of the autoconfig registers go here (this is
	; $00000100 in byte-addressing, but the ROM is word addressed).

	dc.b	$0f,$bf,$ff,$ff		; 100-10c
	dc.b	$df,$df,~(VERSION>>8),~(VERSION>>0),   ; 110-11c
	dc.b	~(REVISION>>8),~(REVISION>>0),$df,$ff  ; 120-12c
	dc.b	$ff,$ff,$ff,$ff		; 130-13c
	dc.b	$ff,$ff,$ff,$ff		; 140-14c
	dc.b	$ff,$ff,$ff,$ff		; 150-15c
	dc.b	$ff,$ff,$ff,$ff		; 160-16c
	dc.b	$ff,$ff,$ff,$ff		; 170-17c
	dc.b	$ff,$ff,$ff,$ff		; 180-18c
	dc.b	$ff,$ff,$ff,$ff		; 190-19c
	dc.b	$ff,$ff,$ff,$ff		; 1a0-1ac
	dc.b	$ff,$ff,$ff,$ff		; 1b0-1bc
	dc.b	$ff,$ff,$ff,$ff		; 1c0-1cc
	dc.b	$ff,$ff,$ff,$ff		; 1d0-1dc
	dc.b	$ff,$ff,$ff,$ff		; 1e0-1ec
	dc.b	$ff,$ff,$ff,$ff		; 1f0-1fc

; expansion control??? FIX?

	; This start of the SCSI boot ROM 
	; appears as $200(board) to system
;	org	$00000080
codestart:

;==============================================================================
; This is the ROM diagnostic table that's used to call us at config time.
; Since this is not an expansion device, we're not going to get called at
; autoconfig time to relocate the RomTag and DiagTable.  However, we are
; going to need the contents of the DiagTable to boot the device properly.
; There will be no need for relocation of the DiagTable anyway because we're
; in wordwide ROMs, so the offsets will be correct.  The RomTag will already
; be relocated when it's loaded into the ROMs.
;==============================================================================
; Added conditional - really a590/a2091
; a3090 roms are byte-wide Z-III
;
DiagTable	DC.B	DAC_NIBBLEWIDE+DAC_CONFIGTIME	rom type and call time
		DC.B	0
		DC.W	DiagEnd-DiagTable	how much to copy
		DC.W	RelocRT-DiagTable	what's called at config time
;	DC.W	0	what's called at config time
		DC.W	BootMe-DiagTable	DOS boot code
		DC.W	BootName-DiagTable	What's this for ?
		DC.W	0
		DC.W	0

;==============================================================================
; This code is called at boot time and is expected to call DOS's init routine.
; We've allready created boot nodes, so DOS will call us normally for booting.
;==============================================================================
BootMe		lea.l	DosName(pc),a1		find DOS's RomTag
		jsr	_LVOFindResident(a6)
		tst.l	d0			did we find it
		beq.s	BootedMe		nope, bad boot
		movea.l	d0,a0
		movea.l	RT_INIT(a0),a0		call init vector from RomTag
		jsr	(a0)			DOS may boot off our device
BootedMe	rts

BootName	DC.B	'romboot.device',0
DosName		DC.B	'dos.library',0
		CNOP	0,2

;==============================================================================
; This code is actually called at config time and we use it to resolve all of
; the absolute references in the RomTag so that our init routine can be found
; at "binddrivers" time.  On entry to the routine, useful registers are:-
;
** These are the calling conventions for the diagnostic callback
** (da_DiagPoint).
**
** A7 -- points to at least 2K of stack
** A6 -- ExecBase
** A5 -- ExpansionBase
** A3 -- your board's ConfigDev structure
** A2 -- Base of diag/init area that was copied
** A0 -- Base of your board
**
** Your board must return a value in D0.  If this value is NULL, then
** the diag/init area that was copied in will be returned to the free
** memory pool.
; a0 = BoardAddress (For A590/2091 the offset to the ROM code is $2000)
; a2 = pointer to a memory copy of DiagTable and this code up to EndCode
; a3 = pointer to the configdev struct for this board (can stash stuff there)
;==============================================================================
RelocRT		movem.l	d2-d3/a2-a4,-(sp)	stash our registers
	printf <'In RelocRT, relocating board @ $%lx'>,a0
		move.l	a0,a4			save board addr

		lea.l	InitDescriptor(pc),a0	point to our RomTag
		lea.l	DiagEnd(pc),a1
		move.l	a0,RT_MATCHTAG(a0)	RomTag must point to itself
		move.l	a1,RT_ENDSKIP(a0)	just skip over ram version
		lea	DeviceName(pc),a1	set pointers to strings
		move.l	a1,RT_NAME(a0)
		lea	IDString(pc),a1
		move.l	a1,RT_IDSTRING(a0)
		; RT_INIT from the seglist...

		; load seglist from the rom into ram
		; remember that each byte is spaced 4 bytes apart
		pea	4			temporary RH to read real size
		pea	A3090_DRIVER_OFFSET(a4)
		move.l	sp,d1			RH (temp)
		subq.w	#4,sp
		move.l	sp,a0			buffer (temp)
		moveq	#4,d0			read 4 bytes
		bsr	Z3Read			read it, ignore errors
		move.l	(sp)+,d0		4 bytes of length of seglist
		addq.w	#8,sp			drop temp RH

		move.l	d0,-(sp)		      size (bytes) of seglist
		pea	A3090_DRIVER_OFFSET+4*4(a4)   address of first byte
	IFD DEBUG_CODE
	move.l	(sp),a0
	printf <'seglist starts at $%lx, length %ld'>,a0,d0
	ENDC
		lea.l	_LVOAllocMem(a6),a0	allocfunc
		lea.l	_LVOFreeMem(a6),a1	freefunc
		suba.l	a2,a2			no overlay table
		lea.l	Z3Read(pc),a3		readfunc
		move.l	sp,d1			read handle
		bsr	LoadSeg			call loadseg ^
		addq.w	#8,sp			drop readhandle

	printf <'Loaded seglist $%lx'>,d0
		; have seglist, point romtag init at it
		lsl.l	#2,d0			BPTR->CPTR
		beq.s	10$			load failure - d0 == 0
		move.l	d0,a1
		addq.w	#4,a1			point at first instruction
		lea.l	InitDescriptor(pc),a0	point to our RomTag
		move.l	a1,RT_INIT(a0)		init point is start of seglist
		
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
; read from Z3 "byte"-wide ROM.  Uses readhandle with 0(rh) = pointer,
; 4(rh) = number of bytes left to EOF.  Inputs: d1 - rh, a0 - buf, d0 - size
; returns number of bytes xferred or 0 for EOF.
;
; Because of a bug in expansion with Z3, byte-wide is really 2-nibble-wide,
; one in each word of the longword.
; NOTE: this is 1 nibble per _word_, not longword!
;==============================================================================
Z3Read:		movem.l	d2-d4/a2,-(sp)
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
		beq.s	Z3ReadDone
		tst.l	d0			out of bytes, return EOF
		beq.s	Z3ReadEOF
		move.b	(a1),d4			DDDDxxxx
		and.b	#$f0,d4			DDDD0000
		move.b	2(a1),d1		DDDDxxxx
		lsr.w	#4,d1			upper byte of d1=0 - 0000DDDD
		or.b	d4,d1			DDDDDDDD
		move.b	d1,(a0)+
		addq.w	#4,a1			1 byte per longword
		subq.l	#1,d0
		subq.l	#1,d3
		bra.s	1$

Z3ReadDone	bsr	Z3SaveRH		save modified readhandle
		move.l	d2,d0			we read all that was asked
;	IFD DEBUG_CODE
;	move.l (sp)+,a0
;	printf <'read $%lx bytes to $%lx, first = $%08lx'>,d0,a0,(a0)
;	ENDC
		movem.l	(sp)+,d2-d4/a2		original count
		rts

Z3ReadEOF	bsr.s	Z3SaveRH		save modified readhandle
		move.l	d2,d0			original count
		sub.l	d3,d0			 - number not read = # read
;	IFD DEBUG_CODE
;	addq.w	#4,sp
;	printf <'EOF, read $%lx bytes'>,d0
;	ENDC
		movem.l	(sp)+,d2-d4/a2		original count
		rts

Z3SaveRH ;printf <'Next read from $%lx, %ld bytes left'>,a1,d0
		move.l	a1,(a2)			save modified readhandle
		move.l	d0,4(a2)
		rts

;==============================================================================
; This is just treated as a normal RomTag and allows us to init the fake
; ConfigDev structure for autobooting.  Our priority should be > RomBoot
;==============================================================================
InitDescriptor	DC.W	RTC_MATCHWORD		so we can be found
		DC.L	InitDescriptor		firewall consistency check
		DC.L	0			how to skip over this code
		DC.B	RTW_COLDSTART		flags
		DC.B	VERSION			version number
		DC.B	NT_DEVICE		node type when we are linked
		DC.B	HD_PRI			priority to run this task at
		DC.L	DeviceName		root name of this device
		DC.L	IDString		descriptive ID string
		DC.L	-1			code to call for initialisation


	INCLUDE	"loadseg.asm"

ExpanName	DC.B	'expansion.library',0

IDString	DC.B	VSTRING
DeviceName	DC.B	'ncr.device',0
		CNOP	0,2

DiagEnd:	ds.b	0
		;config code will copy from the beginning to here

		END
