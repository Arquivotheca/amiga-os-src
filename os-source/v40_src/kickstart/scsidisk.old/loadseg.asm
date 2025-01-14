		SECTION driver,CODE

;==============================================================================
; Generic LoadSeg used for loading filesystems and relocating them.  Original
; by Bob "Kodiak" Burns, modifications and speedups by Randal Jesup.  More mods
; by me (Steve Beats) to make it work in this drivers environment.
;==============================================================================

		NOLIST
		INCLUDE	"modifiers.i"
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"libraries/dos.i"
		INCLUDE "libraries/dosextens.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

ABSEXECBASE	EQU	4
	
		XDEF	LoadSeg

		XREF	_LVOUnLoadSeg
		XREF	_LVOAllocMem,_LVOFreeMem
		XREF	_LVOOpenLibrary,_LVOCloseLibrary

MAXNAMELEN	EQU	128

HUNK_HEADER	EQU	1011

BUFFERSIZE	EQU	2048
;
;	overlay offsets in first hunk
;

NXTHUNK EQU	0	; BPTR to next hunk
OVBRA	EQU	4	; BRA.L	NextModule - would be 0 except for hunk ptr
OVID	EQU	8	; should be $ABCD
STREAM	EQU	12
OVTAB	EQU	16
HTAB	EQU	20
GLBVEC	EQU	24

;------	LoadSeg ------------------------------------------------------
;
;   NAME
;	LoadSeg - load a file into memory and relocate it
;
;   SYNOPSIS
;	success = LoadSeg(allocFunc, freeFunc, table,
;	d0                a0         a1        a2
;			  readFunc, readHandle, dataEnviron)
;			  a3        d1          a6
;
;	actual = readFunc(readHandle, buffer, length)
;       d0                d1          a0      d0
;
;	memory = allocFunc(size, flags)
;	d0                 d0    d1
;
;	freeFunc(memory, size)
;	         a1      d0
;
;   EXCEPTIONS
;	This LoadSeg fails if resident or overlay hunks exist.
;
;---------------------------------------------------------------------
lsv_DataEnviron	EQU	0		; function argument passed in a6
lsv_ReadFunc	EQU	lsv_DataEnviron-4
lsv_Table	EQU	lsv_ReadFunc-4	; BPTR!!!!!!!
lsv_FreeFunc	EQU	lsv_Table-4
lsv_AllocFunc	EQU	lsv_FreeFunc-4
lsv_ReadHandle	EQU	lsv_AllocFunc-4
lsv_Segment	EQU	lsv_ReadHandle-4
lsv_FirstHunk	EQU	lsv_Segment-4
lsv_DOSBase	EQU	lsv_FirstHunk-4
lsv_Name	EQU	lsv_DOSBase-MAXNAMELEN
lsv_LINKSIZE	EQU	lsv_Name-4

;
;   d2	various temporaries
;   d3	hunk relocation offset value
;   d4	byte limit of current hunk
;   d5	first hunk slot
;   d6	last hunk slot
;   d7	random temps
;   a2	current hunk address
;   a3	segment tail
;   a4	hunk table
;   a5	lsv structure
;   a6	scratch data area
;
LoadSeg:
		movem.l	d2-d7/a2-a5,-(a7)
		link	a6,#lsv_LINKSIZE
		move.l	a6,a5

		movem.l	d1/a0/a1/a2/a3,lsv_ReadHandle(a5)
		moveq	#0,d6
		move.l	d6,lsv_Segment(a5)
		move.l	a2,a4
		add.l	a4,a4
		add.l	a4,a4			; convert to APTR!!!!

	    ;-- open dos library
		moveq	#0,d2
		lea	DLName(pc),a1
		moveq	#0,d0
		move.l	ABSEXECBASE,a6
;		jsr	_LVOOpenLibrary(a6)
;		move.l	d0,lsv_DOSBase(a5)

	    ;-- ensure this is a valid load file
		bsr	GetLong
		cmp.l	#HUNK_HEADER,d0
		bne	lsFail

	    ;-- handle resident library header by failing if it exists
		bsr	GetLong
		tst.l	d0
		bne	lsFail

	    ;-- handle hunks
		bsr	GetLong		; get table size
		move.l	d0,d7		; save! needed for clearing
		bsr	GetLong		; get first hunk slot
		move.l	d0,d5		; tnum
		move.l	d0,lsv_FirstHunk(a5)	;for overlays
		bsr	GetLong		; get last hunk slot
		move.l	d0,d6		; tmax

		;-- is this an overlay??  if so, don't allocate
		move.l	a4,d1		; no tst.l a4
		bne.s	lsAllocHunks

		;-- allocate the hunk table via GetVector (ala BCPL)
		move.l	d7,d0		; table size (longwords)
		bsr	GetVector	; always adds a longword to request!!!
		move.l	a0,a4

		;-- allocate the hunks themselves
lsAllocHunks:
		move.w	d5,d2		; for j = hnum to hmax do  (not tsize!)
		lsl.w	#2,d5
		lea	0(a4,d5.w),a2
		lea	lsv_Segment(a5),a3
lsAllocHunk:
		;-- for each entry in this file
		cmp.w	d6,d2
		bgt.s	lsLoadHunks
		;-- allocate space for the hunk and cache pointer in the table	
		bsr	GetLong		; get size needed
		bsr	GetVector	; get memory
		move.l	a0,d0		; convert
		lsr.l	#2,d0		;   to BPTR
		move.l	d0,(a2)+	; cache pointer	(BPTR!!!!)
		move.l	d0,(a3)		;   and link to end of segment
		move.l	a0,a3		; new tail
		clr.l	(a3)		; make sure list is null-terminated
		addq.w	#1,d2		; count up hunk
		bra.s	lsAllocHunk

lsLoadHunks:
		;-- clear the hunk table from hmax+1 to tsize
		;-- overlays don't work unless this is done!
		moveq	#0,d0
clear_loop:	cmp.w	d7,d2		; d7 still has tsize
		bgt.s	DoneClear	; if tsize == hmax, none get cleared
		move.l	d0,(a2)+	; a2 is still correct from above
		addq.w	#1,d2
		bra.s	clear_loop

DoneClear:
		;-- read in the load image
		moveq	#-1,d4			; byte limit: between hunks
lsLoadHunk:
		;-- perform GetLong-like function, but EOF is OK here
		subq.l	#4,a7
		move.l	a7,a0
		moveq	#4,d0
		move.l	lsv_ReadHandle(a5),d1
		movem.l	lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		tst.l	d0			; check for EOF
		beq	lsFreeHunk
		cmp.l	#4,d0
		bne	lsFail
		;-- switch on hunk type
		move.l	(a7)+,d0
		andi.l	#$3fffffff,d0
		sub.l	#1000,d0
		bmi	lsFail
		cmp.l	#HUNKSWITCHLIMIT,d0
		bge	lsFail
		add.w	d0,d0
		move.w	lsSwitch(pc,d0.w),d0
		jsr	lsSwitch(pc,d0.w)
		bra.s	lsLoadHunk

lsSwitch:
		dc.w	lssHunkName-lsSwitch
		dc.w	lssHunkCode-lsSwitch
		dc.w	lssHunkData-lsSwitch
		dc.w	lssHunkBSS-lsSwitch
		dc.w	lssHunkReloc32-lsSwitch
		dc.w	lssHunkReloc16-lsSwitch
		dc.w	lssHunkReloc8-lsSwitch
		dc.w	lssHunkExt-lsSwitch
		dc.w	lssHunkSymbol-lsSwitch
		dc.w	lssHunkDebug-lsSwitch
		dc.w	lssHunkEnd-lsSwitch
		dc.w	lssHunkHeader-lsSwitch
		dc.w	lssHunkCont-lsSwitch
		dc.w	lssHunkOverlay-lsSwitch
		dc.w	lssHunkBreak-lsSwitch

HUNKSWITCHLIMIT	EQU	(*-lsSwitch)/2

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkName:
		bsr	ReadName		; read name
		rts				; and ignore

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkCode:
lssHunkData:
		;-- get hunk size
		bsr	GetLong
		;-- get new byte limit
		lsl.l	#2,d0
		move.l	d0,d4
*		subq.l	#4,d4
		;-- get hunk base
		move.l	0(a4,d5.w),a2		; get table entry
		add.l	a2,a2
		add.l	a2,a2			; convert to APTR
		addq.l	#4,a2			; skip next segment pointer
		addq.l	#4,d5			; bump hunk number
		;-- load in the code
		move.l	d0,d2
		beq.s	lsshdReturn
		move.l	a2,a0
		move.l	lsv_ReadHandle(a5),d1
		movem.l	lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	d2,d0
		bne	lssFail
lsshdReturn:
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkBSS:
		;-- get hunk size
		bsr	GetLong
		;-- get new byte limit
		move.l	d0,d4
		lsl.l	#2,d4
*		subq.l	#4,d4
		;-- get hunk base
		move.l	0(a4,d5.w),a2		; get table entry
		add.l	a2,a2
		add.l	a2,a2			; convert to APTR
		addq.l	#4,a2			; skip next segment pointer
		addq.l	#4,d5			; bump hunk number
		;-- clear the bss area
		move.l	a2,a0
		moveq	#0,d1
		move.w	d0,d2			; low word
		swap	d0			; high word
		bra.s	lsshbssDBF
lsshbssLoop:
		move.l	d1,(a0)+
lsshbssDBF:
		dbf	d2,lsshbssLoop
		dbf	d0,lsshbssLoop
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkBreak:
		addq.l	#4,a7		; pop return address
		neg.l	lsv_Segment(a5)	; make sure we return -seg!!!!
					; PrivateLoadSeg depends on this!!!!
		bra	lsCheckOK	; and clean up
					; DO NOT FREE HUNK TABLE!
				
				
; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
;
; try to keep one block of mem, don't alloc/free/alloc/free/...
; actually, the mem block is the largest of those seen so far
;
lssHunkReloc32:
		sub.l	a3,a3			; never allocated before
reloc32loop:	bsr	GetLong
		lsl.l	#2,d0			; bytes
		move.l	d0,d2			; save
		beq.s	lsshrReturn		; good exit

		bsr	GetLong			; hunk number
		cmp.l	d6,d0
		bgt.s	ErrFreeReloc		; out of range

		lsl.w	#2,d0			; longword offset
		move.l	0(a4,d0.w),d3		; get table entry
		lsl.l	#2,d3			; convert to APTR
		addq.l	#4,d3			; skip next segment pointer

		move.l	a3,d7			; ever allocated one?
		beq.s	doalloc
		cmp.l	-4(a3),d2		; have one, is it big enough?
		ble.s	noalloc			; yup!

		move.l	-(a3),d0		; free the vector
		addq.l	#4,d0
		move.l	a3,a1
		move.l	ABSEXECBASE,a6
		jsr	_LVOFreeMem(a6)

doalloc:	moveq	#0,d1			; allocate area for reloc info
		move.l	d2,d0			; size of table (bytes)
		move.l	d0,d7			; save temp
		addq.l	#4,d0			; for size
		move.l	ABSEXECBASE,a6
		jsr	_LVOAllocMem(a6)	; vector
		tst.l	d0
		beq.s	lssFail			; not FreeReloc!
		move.l	d0,a3
		move.l	d7,(a3)+		; store size of vector

noalloc:	move.l	d2,d0			; now read in the reloc info
		move.l	a3,a0			; dest
		move.l	lsv_ReadHandle(a5),d1
		movem.l	lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	d0,d2			; did we get it all?
		bne.s	ErrFreeReloc

		move.l	d2,d0			; bytes
		lsr.l	#2,d0			; longwords (not less than 1)
		subq.l	#1,d0			; set up index (0 - x-1)
		move.l	a3,a6			; temp - unused in loop
						; used as index+disp
relocloop:	move.l	(a6)+,d7		; offset
		bmi.s	ErrFreeReloc		; < 0 == error
		cmp.l	d4,d7
		bgt.s	ErrFreeReloc		; > max == error
		
		add.l	d3,0(a2,d7.l)		; add base to offset

		dbf	d0,relocloop
		bra	reloc32loop		; next hunk

ErrFreeReloc:
		moveq	#1,d7			; flag for error
		bra.s	lsshrFree

lsshrReturn:
		moveq	#0,d7			; flag for ok
lsshrFree:
		move.l	a3,d0			; did we allocate?
		beq.s	realreturn
		move.l	-(a3),d0		; free the vector
		addq.l	#4,d0
		move.l	a3,a1
		move.l	ABSEXECBASE,a6
		jsr	_LVOFreeMem(a6)
		tst.l	d7
		bne.s	lssFail			; !0 == failure
realreturn:	rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkSymbol:
		bsr.s	ReadName		; flush symbol name
		tst.l	d0			; check symbol length
		beq.s	lsshsDone		; end of symbol hunk if zero
		bsr.s	GetLong			; flush symbol value
		bra.s	lssHunkSymbol
lsshsDone:
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
;  Might be faster to Seek() past debug, since we know the length
;
lssHunkDebug:
		bsr.s	GetLong
		move.w	d0,d2
		swap	d0		; handle big debugs (not likely, but)
		bra.s	lsshdDBF
lsshdLoop:
		bsr.s	GetLong
lsshdDBF:
		dbf	d2,lsshdLoop
		dbf	d0,lsshdLoop
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkEnd:
		moveq	#-1,d4		; flag end in limit
		rts			; (it's OK to see EOF now)

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssFail:
lssHunkOverlay:
lssHunkReloc16:
lssHunkReloc8:
lssHunkExt:
lssHunkHeader:
lssHunkCont:
		moveq	#0,d4		; flag error
		addq.l	#4,a7		; pop switch return address
					; fall thru to lsFreeHunk
; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

lsFreeHunk:
		move.l	a4,d0
		beq.s	lsCheckOK

		;-- free the temporary hunk table via FreeVec
		move.l	a4,a0
		bsr	FreeVec

		;-- check for error
lsCheckOK:
		cmp.l	#-1,d4
		beq.s	lsOK

		;-- unload the segment
		move.l	lsv_Segment(a5),d1
		move.l	lsv_FreeFunc(a5),a1
		move.l	lsv_DataEnviron(a5),a6
		bsr	UnLoadSeg
		moveq	#0,d7
		bra.s	lsReturn

lsOK:
		;-- return list head BPTR as result
		move.l	lsv_Segment(a5),d7

lsReturn:
lsCloseLib:
;		move.l	lsv_DOSBase(a5),a1	; close dosbase
;		move.l	ABSEXECBASE,a6
;		jsr	_LVOCloseLibrary(a6)

		move.l	d7,d0			; seglist for return
		move.l	a5,a6
		unlk	a6
		movem.l	(a7)+,d2-d7/a2-a5
		rts

lsFail:
		moveq	#0,d4
		bra.s	lsFreeHunk

; - - -	GetLong - - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   RESULT
;   d0	next longword in stream
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GetLong:
		subq.l	#4,a7
		move.l	a7,a0
		moveq	#4,d0
		move.l	lsv_ReadHandle(a5),d1
		movem.l	lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	#4,d0
		bne.s	gFail
		move.l	(a7)+,d0
		rts


; - - -	ReadName  - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   INPUT
;   a5	lsv structure
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ReadName:
		bsr.s	GetLong
		lsl.l	#2,d0
		beq.s	rnReturn
		cmp.l	#MAXNAMELEN,d0
		bge.s	rnFail
		lea	lsv_Name(a5),a0
		move.l	d0,-(a7)
		move.l	lsv_ReadHandle(a5),d1
		movem.l	lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	(a7)+,d0
		bne.s	rnFail
rnReturn:
		rts
rnFail:
		addq.l	#4,a7
		bra.s	lsFail


; - - -	GetVector - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   INPUT
;   d0	vector size (without size prefix), in longwords, with memory
;	flags CHIP and FAST in bits 30 and 31
;
;   RESULT
;   a0	vector
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GetVector:
		move.l	d0,d1
		lsl.l	#2,d0		; also drops off CHIP/FAST bits
		addq.l	#8,d0
		move.l	d0,-(a7)	; used to save 4 less as size
		rol.l	#3,d1
		and.l	#6,d1		; mask to MEMF_FAST+MEMF_CHIP
		or.l	#MEMF_PUBLIC,d1
		move.l	lsv_AllocFunc(a5),a1
		move.l	lsv_DataEnviron(a5),a6
		jsr	(a1)
		tst.l	d0
		bne.s	gvCacheSize
gFail:
		addq.l	#8,a7		; drop return addr, temp
		bra	lsFail
gvCacheSize:
		move.l	d0,a0
		move.l	(a7)+,(a0)+	; size -> (a0); a0 = a0+4
		rts


; - - -	FreeVector - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   INPUT
;   a0	vector
;
;   NOTES
;	destroys d0,d1,a0,a1
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FreeVec:	move.l	a0,d0
		beq.s	exitfree
		move.l	a0,a1
		move.l	-(a1),d0	; size
		move.l	lsv_FreeFunc(a5),a0
		move.l	lsv_DataEnviron(a5),a6	; usually DOSBase
		jsr	(a0)
exitfree:
		rts

;------ UnLoadSeg ----------------------------------------------------
;
;	UnLoadSeg(segment, freeFunc, dataEnviron)
;	          d1       a1        a6
;
;	freeFunc(memory, size)
;	         a1      d0
;
;   EXCEPTIONS
;
;---------------------------------------------------------------------
UnLoadSeg:
		; don't dare do my own unloadseg, unless I want to handle
		; overlays
		rts

;		move.l	lsv_DOSBase(a5),a6
;		jmp	_LVOUnLoadSeg(a6)

DLName		DC.B	'dos.library',0
		CNOP	0,2
	
		END
