*
	TTL "loadseg - asm version by kodiak, many mods by REJ"
*
*	for CAPE:

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "dos/dosextens.i"
	INCLUDE "dos/doshunks.i"

	SECTION loadseg,CODE

ABSEXECBASE	EQU	4
	
	XDEF	@loadseg
	XDEF	@loadsegbstr
	XDEF	@InternalLoadSeg
	XDEF	@unloadseg
	XDEF	@InternalUnLoadSeg

	XREF	_LVOUnLoadSeg
	XREF	_LVOAllocMem
	XREF	_LVOClose
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem
	XREF	_LVOOpen
	XREF	_LVOOpenLibrary
	XREF	_LVORead
	XREF	@ClearICache

BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

KLIB	MACRO	;\1 - symbolname
	XREF	@\1
	ENDM

	KLIB	dosbase
	BLIB	findstream
	BLIB	endstream
	BLIB	read
	KLIB	setresult2

MAXNAMELEN	EQU	128

;
; Important tuning parameter!!!! Must be multiple of 4!
;
BUFFERSIZE	EQU	1024
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

;------	@InternalLoadSeg----------------------------------------------
;
;   NAME
;	@InternalLoadSeg - load a file into memory and relocate it
;
;   SYNOPSIS
;	seglist = @InternalLoadSeg(readHandle, table, functable), DOSBase
;	d0                         d0          a0     a1           a6
;
;
;	FuncTable[0] ->  Actual = ReadFunc(readhandle,buffer,length),DOSBase
;		         D0                D1         A0     D0      A6
;	FuncTable[1] ->  Memory = AllocFunc(size,flags), Execbase
;		         D0                 D0   D1      a6
;	FuncTable[2] ->  FreeFunc(memory,size), Execbase
;		                  A1     D0     a6
;
;
;   EXCEPTIONS
;	This LoadSeg fails if resident or overlay hunks exist.
;
;---------------------------------------------------------------------

lsv_FreeFunc	EQU	0		; don't change position!
lsv_AllocFunc	EQU	lsv_FreeFunc-4	; don't change position!
lsv_ReadFunc	EQU	lsv_AllocFunc-4	; don't change position!
lsv_ReadHandle	EQU	lsv_ReadFunc-4
lsv_Segment	EQU	lsv_ReadHandle-4
lsv_FirstHunk	EQU	lsv_Segment-4
lsv_Buffer	EQU	lsv_FirstHunk-4
lsv_FarJump	EQU	lsv_Buffer-4	; stack position for lsFail
lsv_BuffPtr	EQU	lsv_FarJump-4
lsv_LongsLeft	EQU	lsv_BuffPtr-4
lsv_Table	EQU	lsv_LongsLeft-4
lsv_Name	EQU	lsv_Table-MAXNAMELEN
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
;   a6	DOSBase
;
@InternalLoadSeg:
		movem.l	d2-d7/a2-a6,-(a7)
		link	a5,#lsv_LINKSIZE

		move.l	d0,lsv_ReadHandle(a5)
		movem.l	(a1),d1/d2/d3		; copy the function list
		movem.l	d1/d2/d3,lsv_ReadFunc(a5) ; TRICKY!
		move.l	a7,lsv_FarJump(a5)	; stack ptr for lsFail
		moveq	#0,d6
		move.l	d6,lsv_Segment(a5)
		move.l	d6,lsv_LongsLeft(a5)
		move.l	d6,lsv_Buffer(a5)	; in case we have an error
		move.l	d6,d4			; error if exit before HUNK_END 
		move.l	a0,a4
		add.l	a4,a4
		add.l	a4,a4			; convert BPTR to APTR!!!!
		move.l	a4,lsv_Table(a5)	; so we can check later

	    ;-- allocate read buffer

		move.l	#BUFFERSIZE,d0
		moveq	#0,d1
		move.l	a6,a3			; preserve a6
		move.l	ABSEXECBASE,a6
		jsr	_LVOAllocMem(a6)
		move.l	a3,a6
		tst.l	d0
		beq	lsFail
		move.l	d0,lsv_Buffer(a5)
		move.l	d0,lsv_BuffPtr(a5)

	    ;-- ensure this is a valid load file
		bsr	GetLong
		cmp.l	#HUNK_HEADER,d0
		bne	lsFailHeader

	    ;-- handle resident library header by failing if it exists
		bsr	GetLong		; if it set cc's, we could nuke the tst
		tst.l	d0		; # of resident libraries
		bne	lsFailHunk

	    ;-- handle hunks
		bsr	GetLong		; get table size
		move.l	d0,d7		; save! needed for clearing
		bsr	GetLong		; get first hunk slot
		move.l	d0,d5		; tnum (hnum)
		move.l	d0,lsv_FirstHunk(a5)	;for overlays
		bsr	GetLong		; get last hunk slot
		move.l	d0,d6		; tmax (hmax)

		;-- is this an overlay??  if so, don't allocate
		move.l	a4,d1		; no tst.l a4
		bne.s	lsAllocHunks

		;-- allocate the hunk table via GetVector (ala BCPL)
		move.l	d7,d0		; table size (longwords)
		bsr	GetVector	; getvector adds two longwords!!!
		move.l	a0,a4

		;-- allocate the hunks themselves
lsAllocHunks:
		;-- cannot assume hmax < tsize!!!!!
		move.w	d5,d2		; for j = hnum to tsize-1 do (not hmax!)
		lsl.w	#2,d5
		lea	0(a4,d5.w),a2	; hunk table pointer [hnum]
		lea	lsv_Segment(a5),a3	; head of BPTR list of hunks
lsAllocHunk:
		;-- for each entry in this file
		cmp.w	d7,d2		; compare with tsize (use >=)
		bgt.s	lsLoadHunks	; terminate loop

		;-- is this <= hmax?
		cmp.w	d6,d2		; if >hmax set table entry to 0
		bgt.s	lsExtraHunk	; no

		;-- allocate space for the hunk and cache pointer in the table	
		;-- GetVector exits us if there's a failure
		bsr	GetLong		; get size needed
 		bsr	GetVector	; get memory (may read lw of flags)
		move.l	a0,d0		; convert
		lsr.l	#2,d0		;   to BPTR (MUST use LSR!!!!)
		move.l	d0,(a2)+	; cache pointer	in table (BPTR!!!!)
		move.l	d0,(a3)		;   and link to end of segment
		move.l	a0,a3		; new tail
		clr.l	(a3)		; make sure list is null-terminated

table_loop:	addq.w	#1,d2		; count up hunk
		bra.s	lsAllocHunk

lsExtraHunk:	clr.l	(a2)+		; null out table entry
		bra.s	table_loop



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
		move.l	a7,a0			; space for read
		moveq	#1,d0			; read 4 bytes
		bsr	BufRead
		tst.l	d0			; check for EOF
		beq	lsFreeHunk
		subq.l	#1,d0			; did we get 4 bytes? (1 long)
		bne	lsFailHunk

		;-- switch on hunk type - must leave (type-1000)*2 in D1!
		move.l	(a7)+,d1
		andi.l	#$3fffffff,d1		; mask off alloc bits
		sub.l	#1000,d1
		bmi	lsFailHunk			; <1000 not allowed
		cmp.l	#HUNKSWITCHLIMIT,d1	; > max not allowed
		blt.s	10$			; ok
		btst.l	#HUNKB_ADVISORY,d1	; do we care that we don't
						; understand it?
		beq	lsFailHunk
		; we don't understand it, handle it like a debug since it's
		; not required we understand it.
		moveq	#HUNK_DEBUG-1000,d1

10$		add.w	d1,d1			; word table
		move.w	lsSwitch(pc,d1.w),d0
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
		; oops, this should be DREL32 (1015).  reloc32short is 1020.
		dc.w	lssHunkReloc32Short-lsSwitch
		dc.w	lssHunkDRel16-lsSwitch
		dc.w	lssHunkDRel8-lsSwitch
		dc.w	lssHunkLib-lsSwitch
		dc.w	lssHunkIndex-lsSwitch
		dc.w	lssHunkReloc32Short-lsSwitch	; 1020
		dc.w	lssHunkRelReloc32-lsSwitch

HUNKSWITCHLIMIT	EQU	(*-lsSwitch)/2

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkName:
		bra	ReadName		; read name
						; and ignore (was bsr/rts)

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkCode:
lssHunkData:
lssHunkBSS:
		;-- all of these MUST leave hunk address in a2, size in d4!!!!
		;-- Save hunk (type-1000)*2.  Switch code guarantees it's in D1!
		move.l	d1,d7

		;-- Note: hunk size in header may be larger than here!
		;-- get hunk size
		bsr	GetLong
		;-- get new byte limit for relocations (global)
		move.l	d0,d4			; longs
		lsl.l	#2,d4			; bytes
		;-- get hunk base
		move.l	0(a4,d5.w),d3		; get table entry
		lsl.l	#2,d3
		addq.l	#4,d3
		move.l	d3,a2			; save in a2!
		addq.l	#4,d5			; bump hunk number

		;-- now switch on (type-1000)*2
		subq.w	#(HUNK_BSS-1000)*2,d7
		beq.s	handle_BSS

		;-- load in the code/data
		move.l	d0,d2			; longs to read
		beq.s	lsshdReturn
		move.l	a2,a0			; into hunk
		bsr	BufRead
		cmp.l	d2,d0			; did we get them all?
		bne	lssFail

		;-- now clear the rest of the hunk if not filled
		move.l	-8(a2),d0		; size in bytes of mem block
						; (from allocvec)
		lsl.l	#2,d2			; d2 into bytes
		add.l	d2,d3			; point past end of block
		sub.l	d2,d0			; the number of bytes read
		subq.l	#8,d0			; size and next hunk ptr
		lsr.l	#2,d0			; back to longs
		bne.s	handle_BSS		; clear if there's any left.
lsshdReturn:
		rts

handle_BSS:
		;-- clear the bss area (d0 is size in longs, d3 ptr to mem)
		;-- handle BSS >64K longs now
		move.l	d3,a0			; hunk pointer
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
lssHunkOverlay:
		tst.l	lsv_Table(A5)	; were we passed a table?
		bne	lsFailHunk		; fail if overlay or no hunks
		bsr	GetLong		; overlay table size
	        move.l	d0,d2		; save
		bsr	GetVector	; get memory for overlay table
		move.l	a0,d0		; save in d0 (i need a0) CAREFUL!
		move.l	lsv_FirstHunk(a5),d1
		move.l	0(a4,d1.w),a0	;addr of first hunk from table
		add.l	a0,a0
		add.l	a0,a0		; convert to APTR
		move.l	lsv_ReadHandle(a5),STREAM(a0)
		move.l	d0,OVTAB(a0)	; APTR, not BPTR!!!!
		move.l	a4,d1
		lsr.l	#2,d1
		move.l	d1,HTAB(a0)	; BPTR, not APTR!!!!
		move.l	dl_GV(a6),GLBVEC(a0)	; APTR! to globvec #0
					; note: d2 is in longs!
		addq.l	#1,d2		; because BCPL uses s+1 (globveced size)
		move.l	d0,a0		; ov area (getvec()ed)
		move.l	d2,d0		; size for read (longs)
		bsr	BufRead
		cmp.l	d2,d0		; check for success
		bne	lssFail
		; fall thru to lssHunkBreak...
; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkBreak:
		addq.l	#4,a7		; pop return address
		neg.l	lsv_Segment(a5)	; make sure we return -seg!!!!
					; PrivateLoadSeg depends on this!!!!
		bra	lsCheckOK	; and clean up
					; DO NOT FREE HUNK TABLE!
				
;****************************************************************************
;
; Don't keep mem around, read directly out of buffer.  Note that buffer is
; always in longwords, and that refill resets a3/d7 (and sets d7 already
; decremented so we can avoid doing it again).  refill hits d0,d1,d7/a0,a1,a3,a6
; Don't try a buffsize > 32K, it won't like it.  As usual, it dies on hunk
; numbers >32K.
;
; Reloc32Short is 16-bit offsets from 16-bit hunk numbers.  Note that Reloc32
; breaks for hunks > 32K anyways, and that 99% of relocs are <= 64K.  The 16-bit
; offsets are unsigned.  The number of entries is also 16 bits unsigned.
; NOTE: if the end of a Reloc32Short would put the next hunk on a word boundary,
; then a WORD 0 shall be inserted at the end (to force longword alignment).
;
; We share the routine, and tell what to do by saving d5, and putting a 1 or 0
; into it.
;
; Reloc32 never worked for more than 64K relocations anyways, either.
;
; RelReloc32 does a pc-relative relocation (subtracts the address of the
; destination of the reloc).
;

lssHunkRelReloc32:
		move.l	d5,-(a7)		; save d5
		moveq	#2,d5			; use long reads, rel reloc
		bra.s	Reloc32Common
lssHunkReloc32Short:
		move.l	d5,-(a7)		; save d5
		moveq	#1,d5			; use word reads
		bra.s	Reloc32Common
lssHunkReloc32:
		move.l	d5,-(a7)		; save d5
		moveq	#0,d5			; use long reads
Reloc32Common:
		move.l	lsv_BuffPtr(a5),a3	; pointer to words
		move.l	lsv_LongsLeft(a5),d7
		lsl.l	#2,d7			; bytes left in buffer

reloc32loop:	bsr.s	getreloc		; get number of relocs for this
		move.l	d1,d2			;  hunk.
		beq.s	lsshrReturn		; done if size == 0

		bsr.s	getreloc		; now get the hunk #
		cmp.w	d6,d1			; hunk out of range
		bgt.s	ErrorResetBuf

		; while limited to <32K hunks, don't force it to less than 8K.
		; if 8K max hunks is ok, use lsl.w
		; if more than 32K hunks are needed, use cmp.l above

		lsl.l	#2,d1			; longword offset of hunk num
		move.l	0(a4,d1.l),d3		; hunk table entry
		lsl.l	#2,d3			; convert to APTR
		addq.l	#4,d3			; skip next hunk ptr

						; longwords (not less than 1)
		subq.l	#1,d2			; set up index (0 - x-1)

relocloop:	bsr.s	getreloc		; returns reloc in d1
		bmi.s	ErrorResetBuf		; offset < 0 == err
		cmp.l	d4,d1			; offset > size == err
		bgt.s	ErrorResetBuf

		cmp.w	#2,d5			; is this a PC-relative reloc?
		bne.s	1$
		lea	0(a2,d1.l),a0		; address of reference
		move.l	(a0),d0			; no sub.l a0,(a0)
		sub.l	a0,d0
		move.l	d0,(a0)

1$		add.l	d3,0(a2,d1.l)		; add base to offset

		dbf	d2,relocloop
		bra.s	reloc32loop		; next hunk #

ErrorResetBuf:
		moveq	#1,d2			; flag for error
lsshrReturn:
		tst.l	d5			; force longword alignment at
		beq.s	aligned			; end of Reloc32Short!
		btst	#1,d7			; is d7 a multiple of 2 not 4?
		beq.s	aligned
		bsr.s	getreloc		; if odd word, read a word

aligned:	move.l	(a7)+,d5		; restore d5
		move.l	a3,lsv_BuffPtr(a5)	; restore pointers to structure
		ext.l	d7			; paranoia
		lsr.l	#2,d7			; back to longs
		move.l	d7,lsv_LongsLeft(a5)
		tst.w	d2
		bne.s	lssFail
		rts

;-------- getreloc -------------------------------------------------------
;
; Subroutine used to get either a word or long (depends on d5) from buffer.
; assumes a3->pointer to buffer, d7->bytes left in buffer, result (long) in d1.
; d5 = 1 -> UWORD, d5 = 0|2 ->LONG
; Must leave conditional bits set.
; See above warning about what registers refill hits!
;
;-------------------------------------------------------------------------

getreloc:
		tst.l	d5
		bne.s	short_reloc
		subq.w	#4,d7			; now get a lword of reloc info
		bpl.s	gotw3
		bsr.s	refill			; refill buffer
gotw3:		move.l	(a3)+,d1		; this was a GetLong
		rts				; two exits!
short_reloc:
		moveq	#0,d1			; so d1 will be an unsigned word
		subq.w	#2,d7			; now get a short word of reloc
		bpl.s	gotw4
		bsr.s	refill			; refill buffer
gotw4:		move.w	(a3)+,d1		; get one word (unsigned) reloc
		tst.l	d1			; UNSIGNED!!
		rts

;------ refill ---------------------------------------------------------
;
;	refill (buffptr, bufflen, globals),
;		a3	 d7.w	  a5	   
;
;   on entry, d7.w MUST == -4 or -2
;
;   Refill the buffer normally maintained by BufRead
;   resets a3, d7
;   d7 is set to bytes in buffer + original value
;   if d7 == -2, there are no bytes left (word reads)
;   DO NOT MIX WORD READS WITH LONG READS WITHOUT ALIGNING AFTER WORDS!
;
;   must preserve most regs, destroys d0/d1/a0/a1, modifies a3/d7
;---------------------------------------------------------------------
refill:		movem.l	d2/d3,-(a7)	; save d2/d3
		move.l	a3,a0		; save current a3
		move.l	lsv_Buffer(a5),d2
		move.l	d2,a3		; update buffptr in reg a3
		move.l	#BUFFERSIZE,d3
		movem.l	lsv_ReadHandle(a5),d1/a1
		jsr	(a1)
		movem.l	(a7)+,d2/d3	; restore
		tst.l	d0		; must be >= 0 and fit in word
		bmi.s	ReadError
		add.w	d0,d7		; since d7 was -2/4, d7 = bufsize-2/4
		rts

ReadError:	addq.w	#8,a7		; drop return addr to getreloc and
					; it's return addr to HunkReloc32
		bra.s	ErrorResetBuf	; set error, restore d5, free up stuff.

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkSymbol:
		bsr	ReadName		; flush symbol name
		tst.l	d0			; check symbol length
		beq.s	lsshsDone		; end of symbol hunk if zero
		bsr	GetLong			; flush symbol value
		bra.s	lssHunkSymbol		; get next one
lsshsDone:
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
;  Might be faster to Seek() past debug, since we know the length - FIX!
;
lssHunkDebug:
		bsr	GetLong		; size of debug hunk
		move.w	d0,d2		; lower 16 bits in d2
		swap	d0		; handle big debugs (not likely, but)
		move.w  d0,d7		; so GetLong doesn't clobber it
		bra.s	lsshdDBF
lsshdLoop:
		bsr.s	GetLong		; SLOW! FIX!
lsshdDBF:
		dbf	d2,lsshdLoop	; low word
		dbf	d7,lsshdLoop	; high word
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkEnd:
		moveq	#-1,d4		; flag end in limit
		rts			; (it's OK to see EOF now)

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssFail:
lssHunkDRel16:
lssHunkDRel8:
lssHunkLib:
lssHunkIndex:
lssHunkReloc16:
lssHunkReloc8:
lssHunkExt:
lssHunkHeader:
lssHunkCont:
		moveq	#0,d4		; flag error
		addq.l	#4,a7		; pop switch return address
					; fall thru to lsFreeHunk
; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

; lsFail comes here

lsFreeHunk:
		move.l	a4,d0
		beq.s	lsCheckOK

		;-- free the temporary hunk table via FreeVec
		move.l	a4,a0
		bsr	FreeVec

		;-- check for error
lsCheckOK:
		cmp.l	#-1,d4			; means last hunk was HUNK_END
		beq.s	lsOK

		;-- failure - unload the segment
		move.l	lsv_Segment(a5),d1
		move.l	lsv_FreeFunc(a5),a1
		bsr	@InternalUnLoadSeg	; a6 = DOSBase
		moveq	#0,d7
		bra.s	lsReturn

lsOK:
		;-- return list head BPTR as result
		move.l	lsv_Segment(a5),d7

		;-- set result2 to 0 for the hell of it
		moveq	#0,d0
		bsr	@setresult2

lsReturn:
		move.l	ABSEXECBASE,a6
		move.l	lsv_Buffer(a5),d0
		beq.s	lsCloseLib
		move.l	d0,a1			; Free the read buffer
		move.l	#BUFFERSIZE,d0
		jsr	_LVOFreeMem(a6)
lsCloseLib:
		;-- clear the Icache, since we've been modifying code
		;-- ignore a0, since we're telling to clear everything
		moveq	#-1,d0			; clear all
		bsr	@ClearICache

		;
		move.l	d7,d0			; seglist for return
		unlk	a5
		movem.l	(a7)+,d2-d7/a2-a6
		rts

;
; General failure routine.  Stack is set to lsv_FarJump after calling
;
lsFailHeader:	;-- Set a default error value for normal error conditions
		move.l	#ERROR_OBJECT_WRONG_TYPE,d0
		bsr	@setresult2
		bra.s	lsFail
lsFailHunk:
		move.l	#ERROR_BAD_HUNK,d0
		bsr	@setresult2
lsFail:
		move.l	lsv_FarJump(a5),a7	; fix stack
		moveq	#0,d4
		bra.s	lsFreeHunk

; - - -	GetLong - - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   RESULT
;   d0	next longword in stream
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GetLong:
		subq.l	#4,a7		; space on stack for the long
		move.l	a7,a0
		moveq	#1,d0		; read 1 longword
		bsr	BufRead
		subq.l	#1,d0		; should return 1
		bne.s	lsFail		; fixes stack
		move.l	(a7)+,d0	; return value
		rts


; - - -	ReadName  - - - - - - - - - - - - - - - - - - - - - - - - - -
;
;   INPUT
;   a5	lsv structure
;
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ReadName:
		bsr.s	GetLong
		tst.l	d0
		beq.s	rnReturn
		cmp.l	#MAXNAMELEN/4,d0	; longs!
		bge.s	lsFailHunk			; longer than maximum?
		lea	lsv_Name(a5),a0		; buffer space
		move.l	d0,-(a7)		; longs
		bsr	BufRead
		cmp.l	(a7)+,d0		; check for error!
		bne.s	lsFailHunk		; fixes stack
rnReturn:
		rts


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
		movem.l	d0/d2/a6,-(a7)	; used to save 4 less as size
		rol.l	#3,d1		; move to right bits and mask
		moveq	#MEMF_FAST!MEMF_CHIP,d2
		and.l	d2,d1
*
* to support extensions to this, we will use code 11 (memf_fast|memf_chip)
* to indicate we should read the next long and use it for the bits.
*
		cmp.l	d2,d1
		bne.s	normal_alloc

* must read longword of flags for allocmem/whatever.  Can't use GetLong
* because of failure modes.
* Bit 30 is reserved for future expansion.  For now, merely clear it.

		subq.l	#4,a7
		move.l	a7,a0
		moveq	#1,d0		; read 1 longword
		bsr	BufRead
		move.l	(a7)+,d1	; value read (flags)
		bclr	#30,d1
		subq.l	#1,d0
		bne.s	gFail		; error reading!
		move.l	(a7),d0		; get size back (TRICKY!)

normal_alloc:
		or.w	#MEMF_PUBLIC,d1 ; MEMF_PUBLIC is in low 16 bits
		move.l	ABSEXECBASE,a6
		move.l	lsv_AllocFunc(a5),a1
		jsr	(a1)
		tst.l	d0
		beq.s	gFail

		move.l	d0,a0
		move.l	(a7)+,(a0)+	; size -> (a0); a0 = a0+4
		movem.l	(a7)+,d2/a6	; careful! depends on movem order!
		rts
gFail:
		movem.l	(a7)+,d0/d2/a6	; we only really care about a6
		bra	lsFail		; fixes stack


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
		move.l	a6,-(a7)
		move.l	ABSEXECBASE,a6
		move.l	lsv_FreeFunc(a5),a0
		jsr	(a0)
		move.l	(a7)+,a6
exitfree:
		rts

;------ @unloadseg ---------------------------------------------------
;
;	@unloadseg(segment)
;	           d1     
;		  BPTR
;
;---------------------------------------------------------------------
@unloadseg:
_unloadseg:
	XDEF	_unloadseg
		move.l	ABSEXECBASE,a0
		lea	_LVOFreeMem(a0),a1
		; fall through to InternalUnLoadSeg

;------ @InternalUnLoadSeg--------------------------------------------
;
;	@InternalUnLoadSeg(segment,freefunc)
;	                   d1      a1      
;
;	FreeFunc(memory,size), Execbase
;	         A1     D0     a6
;   EXCEPTIONS
;
;   REGISTER USAGE
;   a2 - segment
;   a3 - temp
;   a4 - freefunc - called with execbase in a6!
;
;---------------------------------------------------------------------
@InternalUnLoadSeg:
		asl.l	#2,d1			; BPTR
		bne.s	us_ok
		move	d1,d0			; error return
		rts
us_ok:
		movem.l	d2/a2/a3/a4/a6,-(a7)

		move.l	d1,a2
		move.l  a1,a4			; save freefunc

		cmp.l	#$ABCD,OVID(a2)		; magic word
		bne.s	no_overlay

		bsr	@dosbase		; get dosbase
		move.l	d0,a6
		move.l	GLBVEC(a2),d0
		cmp.l	dl_GV(a6),d0		; double check
		bne.s	no_overlay

		; we have an overlay
		move.l	STREAM(a2),d1		; close the input stream
		bsr	@endstream		; Close() - a6 has dosbase
					; handle errors?  we can't get them

		; free the hunk table
		move.l	ABSEXECBASE,a6		; need execbase for free
		move.l	HTAB(a2),a1		; BPTR!
		move.l	a1,d0
		beq.s	10$
		add.l	a1,a1
		add.l	a1,a1
		move.l	-(a1),d0		; get size we alloced!
		jsr	(a4)			; free

10$:		; free the overlay table
		move.l	OVTAB(a2),a1		; APTR
		move.l	a1,d0
		beq.s	20$
		move.l	-(a1),d0		; get size we alloced!
		jsr	(a4)			; free
20$
		; fall thru and free hunks
;
; We don't handle resident libraries
;

no_overlay:
		move.l	ABSEXECBASE,a6		; need execbase in a6
		move.l	a2,d2
		beq.s	us_done

us_loop:	move.l	a2,a1			; free current hunk
		move.l	(a1),a2			; get next hunk first
		add.l	a2,a2
		add.l	a2,a2			; BPTR!
		move.l	-(a1),d0		; get size we alloced!
		jsr	(a4)			; normally FreeMem()
		move.l	a2,d0			; are we done?
		bne.s	us_loop

us_done:	
		movem.l	(a7)+,d2/a2/a3/a4/a6
		moveq	#-1,d0			; ok return
		rts

		
;-------BufRead -----------------------------------------------------
;
;	actual = BufRead(buffer, length), frame,
;       d0               a0      d0       a5    
;
;	frame has offsets lsv_Buffer, lsv_BuffPtr, and lsv_LongsLeft used here
;	also lsv_ReadHandle and lsv_ReadFunc
;
;	Maximum 128K buffer!
;
;	WARNING: d0 is in LONGS!
;---------------------------------------------------------------------
BufRead:
		move.l	lsv_BuffPtr(a5),a1	; needed for both paths
		move.l	lsv_LongsLeft(a5),d1
		cmp.l	d0,d1			; d1-d0
		bmi.s	not_enough		; d0 > bytes left?
		move.l	d0,d1			; we have enough already
		bra.s	brloop			; leave d0 as return value
brxfer:		move.l	(a1)+,(a0)+		; could use CopyMemQuick!!!
brloop:		dbra	d1,brxfer		; buffer max 128K!
		move.l	a1,lsv_BuffPtr(a5)	; store back pointer to next
		sub.l	d0,lsv_LongsLeft(a5)	; word avail and correct count
		rts

		; d1 has longs left, a1 has buffptr
not_enough:	movem.l	d2/d3/d4/d7/a2,-(a7)	; move what we have...
		sub.l	d1,d0			; keep count...
		move.l	d1,d7			; save longs read so far
		bra.s	brloop2			; if 0, xfer nothing
brxfer2:	move.l	(a1)+,(a0)+
brloop2:	dbra	d1,brxfer2		; d0=longs left after buffer

		; now finish the rest of the read...
		; d0 has longs needed
		move.l	d0,d4			; length for read (longs)

		; do we need more than half a buffer?
		cmp.l	#BUFFERSIZE/8,d4	; d4-buffersize/8
		bpl.s	read_lots

		; we need to read less than 1/2 buffer - read into buffer
		; and fill request from buffer
		; d7 has longs read so far
		move.l	a0,a2			; save destination ptr
		bsr.s	fill_buffer		; fill the buffer
						; returns longs read in d0

		; if we hit eof before getting enough, read what we got
		cmp.l	d4,d0
		bmi.s	hit_eof			; read what longs we have
		move.l	d4,d0			; don't read more than we need
hit_eof:	
		; fill_buffer goes to lsFail on a real error

		; d0 has # of longs to read (<= # of longs in buffer)
		move.l	a2,a0			; destination
		bsr.s	BufRead			; RECURSIVE (but only once!)
						; should never get to
						; not_enough!
		add.l	d7,d0			; longs read first time + second
		bra.s	br_exit			; exit
		
read_lots:	; need to read more than 1/2 a buffer - direct to destination
		; read into dest directly, then refill buffer
		move.l	d4,d3			; longs to read
		lsl.l	#2,d3			; bytes
		move.l	a0,d2			; buffer (destination)
		movem.l	lsv_ReadHandle(a5),d1/a1
		jsr	(a1)			; ReadFunc

		;-- we purposely use asr to shift AND check sign at once
		asr.l	#2,d0			; change into longs read (safe)
		bmi	lsFail			; didn't get all or error
						; note - EOF is OK
		;-- if bytes is 3, and we needed 4, it will return 0 (meaning
		;-- there was an error).  The ASR does this.
		add.l	d0,d7			; return number of longs read

		; BufRead finished, now refill my buffer
		; works out ok, since we almost always want a few longs
		; after each large read.
		bsr.s	fill_buffer
		move.l	d7,d0			; longs read

br_exit:	movem.l	(a7)+,d2/d3/d4/d7/a2
		rts
;
; refill buffer - reads in a buffer, resets pointers in globals
; whomps d0-d3, a0-a1.  returns longs read in D0
;
fill_buffer:	move.l	lsv_Buffer(a5),d2
		move.l	d2,lsv_BuffPtr(a5)	; reset current buffer ptr
		
		move.l	#BUFFERSIZE,d3
		movem.l	lsv_ReadHandle(a5),d1/a1
		jsr	(a1)			; ReadFunc

		; we use asr on purpose, since it will set the cc
		; change to longwords, panic on error.
		asr.l	#2,d0			
		bmi	lsFail			; EOF is OK.
		move.l	d0,lsv_LongsLeft(a5)	; we may not get all we wanted
		rts


;****** _LoadSeg ***********************************************
;
;	seglist = @loadseg (fileName,table,filehandle),
;	   d0		      d1      d2      d3
;	  BPTR		     CPTR    BPTR    BPTR
;
;	success = @InternalLoadSeg(readHandle, table, functable), DOSBase
;	d0                         d0          a0     a1    	    A6
;
;
;	FuncTable[0] ->  Actual = ReadFunc(readhandle,buffer,length),DOSBase
;		         D0                D1         A0     D0      A6
;	FuncTable[1] ->  Memory = AllocFunc(size,flags), Execbase
;		         D0                 D0   D1      a6
;	FuncTable[2] ->  FreeFunc(memory,size), Execbase
;		                  A1     D0     a6
;
;	if seg < 0, then seg = -seg, don't close FH (overlay load)
;
;*********************************************************************
READFUNC	EQU	0
ALLOCFUNC	EQU	4
FREEFUNC	EQU	8

@loadsegbstr:
_loadsegbstr:
	XDEF	_loadsegbstr
		; string is BSTR, may be in ROM or more likely resident
		moveq	#0,d0
		asl.l	#2,d1			; BPTR->cptr
		beq.s	lsb_exit		; null string
		move.l	d1,a0
		move.b	(a0),d0
		addq.w	#2,d0			; 1 byte for null, 1 for round
		andi.b  #$FE,d0			; make even
		sub.w	d0,a7			; get space from stack
		move.l	a7,a1
		move.l	a7,d1			; loadseg wants cptr to str
		move.w	d0,-(a7)		; save size to restore stack

		move.b	(a0)+,d0		; get size again
		bra.s	2$
1$		move.b	(a0)+,(a1)+		; copy bstr onto stack
2$		dbra	d0,1$
		clr.b	(a1)			; null-terminate

		bsr.s	@loadseg
		add.w	(sp)+,sp		; fix stack - CAREFUL!
lsb_exit:	rts

@loadseg:
_loadseg:
	XDEF	_loadseg
		movem.l	d2/d7/a6,-(a7)

		; set up the function list - FIX! use hooks!
		move.l	ABSEXECBASE,a6
		pea	_LVOFreeMem(a6)		; push in reverse order!
		pea	_LVOAllocMem(a6)
		pea	@read			; Read()

		move.l  d1,d7			; save name ptr
		bsr	@dosbase		; get dosbase in D0
		move.l	d0,a6

		; test for overlay
		move.l	d7,d1			; filename == NULL means overlay
		beq.s	overlay

		move.l	#MODE_OLDFILE,d2	; open the file to load
		bsr	@findstream		; Open() the file
		move.l	d0,d2			; save fh for close
		beq.s	plsDone
		sub.l	a0,a0			; table = NULL

loadit:		move.l	a7,a1			; function table pointer
		bsr	@InternalLoadSeg
		tst.l	d0
		bmi.s	invertseg		; if overlay negate & dont close

		move.l	d2,d1			; fh
		move.l	d0,d2			; return value (save)
		bsr	@endstream		; Close the file (a6-dosbase)
plsDone:
		move.l	d2,d0
;
; the distributed version of ovs.asm assumes returns in D1!!!!!
;
ls_exit:	move.l	d0,d1			; GODDAMN stinkin' BCPL!!!!
		lea.l	12(a7),a7		; drop function table
		movem.l	(a7)+,d2/d7/a6
		rts

; handle overlays
overlay:	move.l	d2,a0			; table
		move.l	d3,d0			; filehandle
		bra.s loadit

invertseg:	neg.l	d0			; LoadSeg returned < 0
						; means overlay, don't close
		bra.s	ls_exit			; the file handle

	END
