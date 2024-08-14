*
	TTL "loadseg - asm version by kodiak, mods by REJ"

*	 Kludged slightly to work for ROMLoad by Bryce Nesbitt
*	 Last update: Friday 01-Sep-89 12:40:33
*

	INCDIR	"include:+g:lc/asm_include"

	INCLUDE "exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"

	SECTION loadseg,CODE

ABSEXECBASE	EQU	4

	XDEF	_PrivateLoadSeg
	XDEF	lssHunkReloc32
	XDEF	refill

	XREF	_LVOUnLoadSeg
	XREF	_LVOAllocMem
	XREF	_LVOAllocAbs
	XREF	_LVOClose
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem
	XREF	_LVOOpen
	XREF	_LVOOpenLibrary
	XREF	_LVORead

MAXNAMELEN	EQU	128

HUNK_HEADER	EQU	1011

BUFFERSIZE	EQU	2048
;
;	overlay offsets in first hunk
;

NXTHUNK EQU	0	; BPTR to next hunk
OVBRA	EQU	4	; BRA.L NextModule - would be 0 except for hunk ptr
OVID	EQU	8	; should be $ABCD
STREAM	EQU	12
OVTAB	EQU	16
HTAB	EQU	20
GLBVEC	EQU	24

;------ LoadSeg ------------------------------------------------------
;
;   NAME
;	LoadSeg - load a file into memory and relocate it
;
;   SYNOPSIS
;	success = LoadSeg(allocFunc, freeFunc, table,
;	d0		  a0	     a1        a2
;			  readFunc, readHandle, dataEnviron)
;			  a3	    d1		a6
;
;	actual = readFunc(readHandle, buffer, length)
;	d0		  d1	      a0      d0
;
;	memory = allocFunc(size, flags)
;	d0		   d0	 d1
;
;	freeFunc(memory, size)
;		 a1	 d0
;
;   EXCEPTIONS
;	This LoadSeg fails if resident or overlay hunks exist.
;
;---------------------------------------------------------------------
lsv_DataEnviron EQU	0		; function argument passed in a6
lsv_ReadFunc	EQU	lsv_DataEnviron-4
lsv_Table	EQU	lsv_ReadFunc-4	; BPTR!!!!!!!
lsv_FreeFunc	EQU	lsv_Table-4
lsv_AllocFunc	EQU	lsv_FreeFunc-4
lsv_ReadHandle	EQU	lsv_AllocFunc-4
lsv_Segment	EQU	lsv_ReadHandle-4
lsv_FirstHunk	EQU	lsv_Segment-4
lsv_Buffer	EQU	lsv_FirstHunk-4
lsv_BuffPtr	EQU	lsv_Buffer-4
lsv_BytesLeft	EQU	lsv_BuffPtr-4
lsv_DOSBase	EQU	lsv_BytesLeft-4
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
		movem.l d2-d7/a2-a5,-(a7)
		link	a6,#lsv_LINKSIZE
		move.l	a6,a5

		movem.l d1/a0/a1/a2/a3,lsv_ReadHandle(a5)
		moveq	#0,d6
		move.l	d6,lsv_Segment(a5)
		move.l	d6,lsv_BytesLeft(a5)
		move.l	a2,a4
		add.l	a4,a4
		add.l	a4,a4			; convert to APTR!!!!

	    ;-- open dos library
		moveq	#0,d2
		lea	DLName(pc),a1
		moveq	#0,d0
		move.l	ABSEXECBASE,a6
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,lsv_DOSBase(a5)

	    ;-- allocate read buffer

		move.l	#BUFFERSIZE,d0
		moveq	#0,d1
		move.l	ABSEXECBASE,a6
		jsr	_LVOAllocMem(a6)
		tst.l	d0
		beq	lsFail
		move.l	d0,lsv_Buffer(a5)
		move.l	d0,lsv_BuffPtr(a5)

	    ;-- ensure this is a valid load file
		bsr	GetLong
		cmp.l	#HUNK_HEADER,d0
		bne	lsFail

	    ;-- handle resident library header by failing if it exists
		bsr	GetLong
		tst.l	d0
		bne	lsFail

	    ;-- handle hunks
		bsr	GetLong 	; get table size
		move.l	d0,d7		; save! needed for clearing
		bsr	GetLong 	; get first hunk slot
		move.l	d0,d5		; tnum
		move.l	d0,lsv_FirstHunk(a5)    ;for overlays
		bsr	GetLong 	; get last hunk slot
		move.l	d0,d6		; tmax

		;-- is this an overlay??  if so, don't allocate
		move.l	a4,d1		; no tst.l a4
		bne.s	lsAllocHunks

		;-- allocate the hunk table via GetVector (ala BCPL)
		move.l	d7,d0		; table size (longwords)
;:bryce
;; bsr	   GetVector	   ; always adds a longword to request!!!
  bsr	  GetVectorA	   ; always adds a longword to request!!!
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
		bsr	GetLong 	; get size needed
		bsr	GetVector	; get memory
		move.l	a0,d0		; convert
		lsr.l	#2,d0		;   to BPTR
		move.l	d0,(a2)+        ; cache pointer (BPTR!!!!)
		move.l	d0,(a3)         ;   and link to end of segment
		move.l	a0,a3		; new tail
		clr.l	(a3)            ; make sure list is null-terminated
		addq.w	#1,d2		; count up hunk
		bra.s	lsAllocHunk

lsLoadHunks:
		;-- clear the hunk table from hmax+1 to tsize
		;-- overlays don't work unless this is done!
		moveq	#0,d0
clear_loop:	cmp.w	d7,d2		; d7 still has tsize
		bgt.s	DoneClear	; if tsize == hmax, none get cleared
		move.l	d0,(a2)+        ; a2 is still correct from above
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
		movem.l lsv_ReadFunc(a5),a1/a6
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

HUNKSWITCHLIMIT EQU	(*-lsSwitch)/2

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
		move.l	0(a4,d5.w),a2           ; get table entry
		add.l	a2,a2
		add.l	a2,a2			; convert to APTR
;:bryce
;		addq.l	#4,a2			; skip next segment pointer
		addq.l	#4,d5			; bump hunk number
		;-- load in the code
		move.l	d0,d2
		beq.s	lsshdReturn
		move.l	a2,a0
		move.l	lsv_ReadHandle(a5),d1
		movem.l lsv_ReadFunc(a5),a1/a6
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
		move.l	0(a4,d5.w),a2           ; get table entry
		add.l	a2,a2
		add.l	a2,a2			; convert to APTR
;:bryce
;		addq.l	#4,a2			; skip next segment pointer
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
lssHunkOverlay:
		tst.l	lsv_Table(a5)
		bne	lsFail		; fail if overlay or no hunks
		bsr	GetLong
		move.l	d0,d2		; save
		bsr	GetVector	; get memory for overlay table
		move.l	a0,d0		; save in d0 (i need a0) CAREFUL!
		move.l	lsv_FirstHunk(a5),d1
		move.l	0(a4,d1.w),a0   ;addr of first hunk from table
		add.l	a0,a0
		add.l	a0,a0		; convert to APTR
		move.l	lsv_ReadHandle(a5),STREAM(a0)
		move.l	d0,OVTAB(a0)    ; APTR, not BPTR!!!!
		move.l	a4,d1
		lsr.l	#2,d1
		move.l	d1,HTAB(a0)     ; BPTR, not APTR!!!!
		move.l	lsv_DOSBase(a5),a1      ; assume public globvec
		move.l	dl_GV(a1),GLBVEC(a0)    ; APTR! to glovec #0
		lsl.l	#2,d2		; convert to bytes
		addq.l	#4,d2		; because BCPL uses s+1 (globveced size)
		move.l	d0,a0		; ov area (getvec()ed)
		move.l	d2,d0		; size for read
		move.l	lsv_ReadHandle(a5),d1
		movem.l lsv_ReadFunc(a5),a1/a6
		jsr	(a1)            ; read overlay table
		cmp.l	d2,d0
		bne	lssFail
		; fall thru to lssHunkBreak...
; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkBreak:
		addq.l	#4,a7		; pop return address
		neg.l	lsv_Segment(a5) ; make sure we return -seg!!!!
					; PrivateLoadSeg depends on this!!!!
		bra	lsCheckOK	; and clean up
					; DO NOT FREE HUNK TABLE!

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
;
; Don't keep mem around, read directly out of buffer.  Note that buffer is
; always in longwords, and that refill resets a3/d7 (and sets d7 already
; decremented so we can avoid doing it again).	refill hits d0,d1,d7/a0,a1,a3,a6
; Don't try a buffsize > 32K, it won't like it.  As usual, it dies on hunk
; numbers >32K.
;
lssHunkReloc32:
		move.l	lsv_BuffPtr(a5),a3      ; pointer to words
		move.l	lsv_BytesLeft(a5),d7    ; bytes left in buffer

reloc32loop:	subq.w	#4,d7
		bpl.s	gotw1
		bsr	refill			; refill buffer
gotw1:		move.l	(a3)+,d2                ; this was a GetLong
		beq.s	lsshrReturn		; done if size == 0

		subq.w	#4,d7			; now get the hunk #
		bpl.s	gotw2
		bsr	refill			; refill buffer
gotw2:		move.l	(a3)+,d0                ; this was a GetLong
		cmp.w	d6,d0			; hunk out of range
		bgt.s	ErrorResetBuf

		lsl.w	#2,d0			; longword offset
		move.l	0(a4,d0.w),d3           ; hunk table entry
		lsl.l	#2,d3			; convert to APTR
;:bryce
;		addq.l	#4,d3			; skip next hunk ptr

						; longwords (not less than 1)
		subq.l	#1,d2			; set up index (0 - x-1)

relocloop:
		subq.w	#4,d7			; now get a word of reloc info
		bpl.s	gotw3
		bsr	refill			; refill buffer
gotw3:		move.l	(a3)+,d1                ; this was a GetLong
		bmi.s	ErrorResetBuf		; offset < 0 == err
		cmp.l	d4,d1			; offset > size == err
		bgt.s	ErrorResetBuf

		add.l	d3,0(a2,d1.l)           ; add base to offset

		dbf	d2,relocloop
		bra	reloc32loop		; next hunk #

ErrorResetBuf:
		moveq	#1,d2			; flag for error
lsshrReturn:
		move.l	a3,lsv_BuffPtr(a5)
		ext.l	d7			; paranoia
		move.l	d7,lsv_BytesLeft(a5)
		tst.w	d2
		bne.s	lssFail
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkSymbol:
		bsr	ReadName		; flush symbol name
		tst.l	d0			; check symbol length
		beq.s	lsshsDone		; end of symbol hunk if zero
		bsr	GetLong 		; flush symbol value
		bra.s	lssHunkSymbol
lsshsDone:
		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
;  WOULD be MUCH faster to Seek() past debug, since we know the length
;
lssHunkDebug:
		bsr	GetLong
		move.l	d0,d2		;-Bryce

		bra.s	lsshdDBF
lsshdLoop:	bsr	GetLong
lsshdDBF:	dbf	d2,lsshdLoop
		sub.l	#$10000,d2	;-Bryce (old code trashed loop counter)
		bpl.s	lsshdLoop

		rts

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssHunkEnd:
		moveq	#-1,d4		; flag end in limit
		rts			; (it's OK to see EOF now)

; . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
lssFail:
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
		bsr	FreeVecA

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
		move.l	lsv_Buffer(a5),d0
		beq.s	lsCloseLib
		move.l	d0,a1			; Free the read buffer
		move.l	#BUFFERSIZE,d0
		move.l	ABSEXECBASE,a6
		jsr	_LVOFreeMem(a6)
lsCloseLib:
		move.l	lsv_DOSBase(a5),a1      ; close dosbase
		move.l	ABSEXECBASE,a6
		jsr	_LVOCloseLibrary(a6)

		move.l	d7,d0			; seglist for return
		move.l	a5,a6
		unlk	a6
		movem.l (a7)+,d2-d7/a2-a5
		rts

lsFail:
		moveq	#0,d4
		bra.s	lsFreeHunk

; - - - GetLong - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
		movem.l lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	#4,d0
		bne.s	gFail
		move.l	(a7)+,d0
		rts


; - - - ReadName  - - - - - - - - - - - - - - - - - - - - - - - - - -
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
		movem.l lsv_ReadFunc(a5),a1/a6
		jsr	(a1)
		cmp.l	(a7)+,d0
		bne.s	rnFail
rnReturn:
		rts
rnFail:
		addq.l	#4,a7
		bra.s	lsFail


; - - - GetVector - - - - - - - - - - - - - - - - - - - - - - - - - -
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
		addq.l	#4,d0	    ;BUG????
;		move.l	d0,-(a7)        ; used to save 4 less as size

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
;		move.l	(a7)+,(a0)+     ; size -> (a0); a0 = a0+4
		rts

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;GetVector that uses AllocMem
GetVectorA:
		move.l	d0,d1
		lsl.l	#2,d0		; also drops off CHIP/FAST bits
		addq.l	#8,d0
		move.l	d0,-(a7)        ; used to save 4 less as size
		rol.l	#3,d1
		and.l	#6,d1		; mask to MEMF_FAST+MEMF_CHIP
		or.l	#MEMF_PUBLIC,d1
		lea.l	AllocMem(pc),a1
		move.l	lsv_DataEnviron(a5),a6
		jsr	(a1)
		tst.l	d0
		bne.s	gvCacheSizeA
		addq.l	#8,a7		; drop return addr, temp
		bra	lsFail
gvCacheSizeA:
		move.l	d0,a0
		move.l	(a7)+,(a0)+     ; size -> (a0); a0 = a0+4
		rts


; - - - FreeVector - - - - - - - - - - - - - - - - - - - - - - - - - -
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
		move.l	-(a1),d0        ; size
		move.l	lsv_FreeFunc(a5),a0
		move.l	lsv_DataEnviron(a5),a6  ; usually DOSBase
		jsr	(a0)
exitfree:
		rts

FreeVecA:	move.l	a0,d0
		beq.s	exitfreeA
		move.l	a0,a1
		move.l	-(a1),d0        ; size
		lea.l	FreeHigh(pc),a0
		move.l	lsv_DataEnviron(a5),a6  ; usually DOSBase
		jsr	(a0)
exitfreeA:
		rts

;------ UnLoadSeg ----------------------------------------------------
;
;	UnLoadSeg(segment, freeFunc, dataEnviron)
;		  d1	   a1	     a6
;
;	freeFunc(memory, size)
;		 a1	 d0
;
;   EXCEPTIONS
;
;---------------------------------------------------------------------
UnLoadSeg:
		; don't dare do my own unloadseg, unless I want to handle
		; overlays
		move.l	lsv_DOSBase(a5),a6
		jmp	_LVOUnLoadSeg(a6)


;-------BuffRead -----------------------------------------------------
;
;	actual = Read(readHandle, buffer, length)
;	d0	      d1	  a0	  d0
;
;	WARNING: assumes d0 is multiple of 4!
;---------------------------------------------------------------------
BufRead:
		move.l	lsv_BuffPtr(a5),a1      ; needed for both paths
		cmp.l	lsv_BytesLeft(a5),d0
		bgt.s	not_enough		; d0 > bytes left?
		move.l	d0,d1			; we have enough already
		asr.l	#2,d1			; we overwrite d1 because
		bra.s	brloop			;  we don't care
brxfer: 	move.l	(a1)+,(a0)+
brloop: 	dbra	d1,brxfer
		move.l	a1,lsv_BuffPtr(a5)      ; store back pointer to next
		sub.l	d0,lsv_BytesLeft(a5)    ; word avail and correct count
		rts

not_enough:	move.l	d1,-(a7)                ; save fh
		move.l	lsv_BytesLeft(a5),d1
		sub.l	d1,d0			; keep count...
		asr.l	#2,d1			; longwords
		bra.s	brloop2 		; if 0, xfer nothing
brxfer2:	move.l	(a1)+,(a0)+
brloop2:	dbra	d1,brxfer2

		; read into dest directly
		move.l	(a7),d1                 ; fh
		bsr	Read
		tst.l	d0
		bmi.s	badread1
		add.l	lsv_BytesLeft(a5),d0    ; make correct (errors foul up)
		move.l	d0,-(a7)

		; refill buffer
		move.l	lsv_Buffer(a5),a0
		move.l	a0,lsv_BuffPtr(a5)
		move.l	4(a7),d1                ; fh
		move.l	#BUFFERSIZE,d0
		bsr	Read
		tst.l	d0
		bmi.s	badread2
		move.l	d0,lsv_BytesLeft(a5)
		move.l	(a7)+,d0                ; bytes read
		addq.l	#4,a7			; drop fh
		rts
badread1:
		subq.l	#4,a7
badread2:
		addq.l	#8,a7			; drop temp, return addr
		bra	lsFail

;------ refill ---------------------------------------------------------
;
;	refill (buffptr, bufflen, globals)
;		a3	 d7.w	  a5
;
;   on entry, d7.w MUST == -4
;
;   Refill the buffer normally maintained by BufRead
;   resets a3, d7
;   d7 is set to bytes in buffer - 4!
;
;   must preserve most regs, destroys d0/d1/a0/a1/a6, mods a3/d7
;---------------------------------------------------------------------
refill: 	movem.l d2/d3,-(a7)     ; save d2/d3
		move.l	lsv_Buffer(a5),d2
		move.l	d2,a3		; update buffptr in reg a3
		move.l	#BUFFERSIZE,d3
		move.l	lsv_ReadHandle(a5),d1
		move.l	lsv_DOSBase(a5),a6
		jsr	_LVORead(a6)
		tst.l	d0
		bmi.s	ReadError
		add.w	d0,d7		; since d7 was -4, d7 = bufsize-4
		movem.l (a7)+,d2/d3
		rts

ReadError:	movem.l (a7)+,d2/d3
		addq.l	#4,a7		; drop return addr
		bra	lsFail

;------ Read ---------------------------------------------------------
;
;	actual = Read(readHandle, buffer, length)
;	d0	      d1	  a0	  d0
;
;---------------------------------------------------------------------
Read:
		movem.l d2/d3,-(a7)
		move.l	a0,d2
		move.l	d0,d3
		jsr	_LVORead(a6)
		movem.l (a7)+,d2/d3
		rts

;------ FreeHigh -----------------------------------------------------
;
;	FreeHigh(memory, size)
;		 a1	 d0
;
;---------------------------------------------------------------------
FreeHigh:
		move.l	a6,-(a7)
		move.l	ABSEXECBASE,a6
		jsr	_LVOFreeMem(a6)
		move.l	(a7)+,a6
		rts

FreeHighInc:
		rts

;------ AllocMem -----------------------------------------------------
;
;	addr = AllocMem(size, where)
;	d0		d0	d1
;
;---------------------------------------------------------------------
AllocMem:
		move.l	a6,-(a7)
		move.l	ABSEXECBASE,a6
		jsr	_LVOAllocMem(a6)
		move.l	(a7)+,a6
		rts

AllocMemInc:
		move.l	LoadAddress,d1
		add.l	d0,LoadAddress
		move.l	d1,d0
		rts
LoadAddress	dc.l	0



;****** PrivateLoadSeg ***********************************************
;	segment = PrivateLoadSeg (fileName)
;	d0			  d1
;
;
; other junk you don't want to know:
;
;	PrivateLoadSeg (0L, table, filehandle)  -- overlay
;
;	segment = LoadSeg(allocFunc, freeFunc, table,
;	d0		  a0	     a1        a2
;			  readFunc, readHandle, dataEnviron)
;			  a3	    d1		a6
;
;	if seg < 0, then seg = -seg, don't close FH (overlay load)
;
;*********************************************************************
_PrivateLoadSeg:
		move.l	4(a7),d1
		move.l	8(a7),LoadAddress

		movem.l d2/a2/a3/a6,-(a7)
		move.l	ABSEXECBASE,a6
		moveq	#0,d2
		lea	DLName(pc),a1
		moveq	#0,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,a6

		move.l	20(a7),d1               ; filename
		beq.s	overlay

		move.l	#MODE_OLDFILE,d2
		jsr	_LVOOpen(a6)
		move.l	d0,d2
		beq.s	plsCloseLib
		move.l	d0,d1			; file handle
		sub.l	a2,a2			; table = NULL

loadit: 	lea	AllocMemInc(pc),a0      ; was AllocRecoverable
		lea	FreeHighInc(pc),a1
		lea	BufRead(pc),a3
		bsr	LoadSeg
		tst.l	d0
		bmi.s	invertseg		; if overlay negate & dont close

		move.l	d2,d1
		move.l	d0,d2
		jsr	_LVOClose(a6)
plsCloseLib:
		move.l	a6,a1
		move.l	ABSEXECBASE,a6
		jsr	_LVOCloseLibrary(a6)
plsDone:
		move.l	d2,d0
		move.l	d0,d1			; GODDAMN stinkin' BCPL!!!!

		move.l	LoadAddress,d0		;-Bryce
		movem.l (a7)+,d2/a2/a3/a6
		rts

overlay:	move.l	24(a7),a2               ; table
		move.l	28(a7),d1               ; filehandle
		bra.s loadit

invertseg:	move.l	d0,d2			; LoadSeg returned < 0
		neg.l	d2			; means overlay, don't close
		bra.s	plsCloseLib		; the file handle

DLName		dc.b	'dos.library',0


	END
