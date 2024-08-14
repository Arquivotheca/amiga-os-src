**
**	$Id: scroll.asm,v 36.45 92/03/23 12:45:13 darren Exp $
**
**      scroll display and character map
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE "debug.i"

**	Exports

	XDEF	PackMap
	XDEF	UnpackMap
	XDEF	InsDelChar
	XDEF	InsDelLine
	XDEF	ScrollYDisplay
	XDEF	ResetBuffer

**	Imports

	XLVO	ScrollRaster		; Graphics
	XLVO	SetBPen			;

	XREF	ClearRaster		; clear.asm
	XREF	RestoreRP


	XREF	CursDisable
	XREF	CursEnable
	XREF	CursUpdate

	XREF	_packSorted
	XREF	_unpackSortedLine


**	Assumptions

	IFNE	cm_BufferYL-cm_BufferXL-2
	FAIL	"cm_BufferXL not before cm_BufferYL: recode"
	ENDC

*------ PackMapLine --------------------------------------------------
*
*   NAME
*	PackMapLine
*
*   SYNOPSIS
*	lineWrap = PackMapLine(consoleUnit, displayLine)
*	D0                     A2           D2
*
*   RESULTS
*	lineWrap - non-zero if lineWrap occurred
*

;
;   d2	displayLine, then cm_BufferWidth
;   d3	buffer X position
;   d4	buffer Y position
;   d5	decrementing display line width
;   d6	IMPLICITNL flag
;
;   a0	incrementing character display line
;   a1	incrementing attribute display line
;   a2	console unit
;   a3	incrementing buffer line array
;   a4	incrementing character buffer
;   a5	incrementing attribute buffer
;
PackMapLine:
		movem.l	d2-d6/a3-a5,-(a7)

		;--	find width of display line
		move.w	cu_CM+cm_DisplayWidth(a2),d5
		cmp.w	cu_DisplayYL(a2),d2
		blt.s	pmlFindDisplay
		bgt.s	pmlPackEmptyLine
		move.w	cu_DisplayXL(a2),d5
		bra.s	pmlFindDisplay
pmlPackEmptyLine:
		moveq	#0,d5
		bra.s	pmlEmptyLine

		;--	find start of display line to pack from
pmlFindDisplay:
		lsl.w	#2,d2
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d2.w),a0
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1
pmlEmptyLine:
		move.w	cu_CM+cm_BufferWidth(a2),d2

		;--	    get start of buffer
		movem.w	cu_CM+cm_BufferXL(a2),d3/d4
		move.w	d4,d0
		lsl.w	#2,d0
		move.l	cu_CM+cm_AttrBufLines(a2),a3
		add.w	d0,a3
		move.l	(a3)+,a5

		;--	    check if display line is valid
		tst.w	d5
		beq.s	pmlCheckEmpty		; no display

		;--	    check for IMPLICITNL
		move.w	(a1),d6
		and.w	#CMAF_IMPLICITNL,d6
		bne.s	pmlPackALine

		;--	    check for empty buffer
pmlCheckEmpty
		tst.w	d3
		bne.s	pmlNewBLine
		tst.w	d4
		beq.s	pmlPackALine

		;--	    start new explicit line
pmlNewBLine:
		;--		clear rest of this line in buffer
		add.w	d3,a5
		add.l	a5,a5

		sub.w	d2,d3
		neg.w	d3
		bra.s	pmlnblClearDBF
pmlnblClearLoop:
		clr.w	(a5)+
pmlnblClearDBF:
		dbf	d3,pmlnblClearLoop

		;--		reset line pointers
		addq.w	#1,d4
		cmp.w	cu_CM+cm_BufferHeight(a2),d4
		blt.s	pmlnblNextLine

		bsr.s	scrollBufferUp1		; preserves a0/a1
		subq.l	#4,a3
		subq.w	#1,d4

pmlnblNextLine:
		move.l	(a3)+,a5
		moveq	#0,d3

pmlPackALine:
		add.w	d3,a5
		move.l	a5,a4
		add.l	cu_CM+cm_AttrToChar(a2),a4
		add.l	a5,a5

		bra.s	pmlDBF

		;--	pack
pmlLoop:
		;--	    check for buffer wrap
		cmp.w	d2,d3
		blt.s	pmlCopy

		;--		wrap destination
		addq.w	#1,d4		; bump buffer Y
		;--		check for full buffer
		cmp.w	cu_CM+cm_BufferHeight(a2),d4
		blt.s	pmlWrapNextLine

		bsr.s	scrollBufferUp1		; preserves a0/a1
		subq.l	#4,a3
		subq.w	#1,d4

pmlWrapNextLine:
		move.l	(a3)+,a5
		moveq	#0,d3		; zero buffer X
		move.l	a5,a4
		add.l	cu_CM+cm_AttrToChar(a2),a4
		add.l	a5,a5
		move.w	#CMAF_IMPLICITNL,d6	; set lineWrap


		;--	    copy character and attribute
pmlCopy:
		move.b	(a0)+,d0	; character
		move.w	(a1)+,d1	; attribute
		bpl.s	pmlDone		;   not RENDERED

		addq.w	#1,d3
		move.b	d0,(a4)+	; copy this character
		and.w	#~(CMAF_IMPLICITNL!CMAF_HIGHLIGHT!CMAF_SELECTED),d1
		or.w	d6,d1
		move.w	d1,(a5)+	;   and attribute
		move.w	#CMAF_IMPLICITNL,d6	; set lineWrap

pmlDBF:
		dbf	d5,pmlLoop

pmlDone:
		movem.w	d3/d4,cu_CM+cm_BufferXL(a2)
		movem.l	(a7)+,d2-d6/a3-a5
		rts


;------ scrollBufferUp1 ----------------------------------------------
;
;   a2	console unit
;
;	preserves a0/a1
;
scrollBufferUp1:
		movem.l	a0-a1,-(a7)
		move.l	cu_CM+cm_AttrBufLines(a2),a0
		lea	4(a0),a1
		move.l	(a0),d0
		move.w	cu_CM+cm_BufferHeight(a2),d1
		subq.w	#2,d1
		bmi.s	sbu1Done
sbu1Loop:
		move.l	(a1)+,(a0)+
		dbf	d1,sbu1Loop
		move.l	d0,(a0)+
sbu1Done:
		movem.l	(a7)+,a0-a1
		rts


;------	sortMap ------------------------------------------------------
;
;   d2	map width
;   d3	map current limit (XxxxxxYL)
;   d4	slot 0 offset (address/2)
;   d5	== BufferLines for Buf, == -1 for Disp
;   a3	AttrXxxxLines array base
;
sortMap:
		;-- sort lines in buffer
;   d6	Work (free) memory index
;   d7	Target (next) slot index
		;-- get working store in space between buffer and display
		move.w	d5,d6		; initial free index
		move.w	d3,d7		; decrementing target from limit

		;-- move this target slot to associated memory location
smSortSlotLoop:
		;--	check if target memory already empty
		cmp.w	d7,d6
		beq.s	smTargetReady

		;--	see who is in target memory
		move.w	d7,d1		; construct target memory address/2
		mulu	d2,d1		;
		add.l	d4,d1		;
		move.w	d3,d0		; limit interesting tenants to valid
		move.l	a3,a0
		;--	    find matching entry
smFindTenantLoop:
		cmp.l	(a0)+,d1
		dbeq	d0,smFindTenantLoop
		bne.s	smTargetReady	; tenant is inconsequential

		sub.w	d3,d0		; recover tenant's index
		neg.w	d0		;

		cmp.w	d7,d0		; see if target memory already correct
		beq.s	smSortSlotDBF

		;--	save memory contents in work (free) memory
		move.w	d6,d0		; construct work memory address
		muls	d2,d0		;
		add.l	d4,d0		;
		move.l	d0,-(a0)	; patch slot pointer
		;--	    set source and destination, attr and char
		move.l	d1,a0
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1
		move.l	d0,a4
		move.l	a4,a5
		add.l	cu_CM+cm_AttrToChar(a2),a4
		add.l	a5,a5
		move.w	d2,d0
		bra.s	smCopyWorkDBF
		;--	    copy tenant in target memory to work memory
smCopyWorkLoop:
		move.b	(a0)+,(a4)+
		move.w	(a1)+,(a5)+
smCopyWorkDBF:
		dbf	d0,smCopyWorkLoop


		;--	copy target to target memory
smTargetReady:
		move.w	d7,d0
		move.w	d7,d1
		lsl.w	#2,d0
		move.l	0(a3,d0.w),a0
		mulu	d2,d1
		add.l	d4,d1

		cmp.l	a0,d1		; check if target already correct
		beq.s	smSortSlotDBF

		;--	    get next work (free) slot index
		move.l	a0,d6
		sub.l	d4,d6
		divs	d2,d6
		;--	    fix up target slot
		move.l	d1,0(a3,d0.w)
		;--	    set source and destination, attr and char
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1
		move.l	d1,a4
		move.l	d1,a5
		add.l	cu_CM+cm_AttrToChar(a2),a4
		add.l	a5,a5
		move.w	d2,d0
		bra.s	smCopyDBF
		;--	    copy target to target memory
smCopyLoop:
		move.b	(a0)+,(a4)+
		move.w	(a1)+,(a5)+
smCopyDBF:
		dbf	d0,smCopyLoop


smSortSlotDBF:
		dbf	d7,smSortSlotLoop
		rts


*------ ResetBuffer --------------------------------------------------
*
*   NAME
*       ResetBuffer
*
*   SYNOPSIS
*       void ResetBuffer(consoleUnit)
*                        A2
*
*   NOTES
*       This routine is to be called when explicitly clearing
*       BufferXL/YL - e.g., on a FormFeed which clears the display
*       map, and the Buffer map.
*
*       Its been determined that the existing buffer may need to
*       be resorted before clearing BufferXL/YL, which is what this
*       does.
*
*   ASSUMPTIONS
*       The caller has checked to make sure that the console unit
*       has a character map.
*
ResetBuffer:     
		movem.l	d0-d7/a0-a5,-(a7)
		bsr.s	SortBuffer
		clr.l	cu_CM+cm_BufferXL(a2)		; and YL
		movem.l	(a7)+,d0-d7/a0-a5
emptybuffer:
		rts

*------ SortBuffer --------------------------------------------------
*
*   NAME
*       SorttBuffer
*
*   SYNOPSIS
*       void SortBuffer(consoleUnit)
*                        A2
*
*   NOTES
*       Sorts off-screen buffer.
*
*   ASSUMPTIONS
*       The caller has checked to make sure that the console unit
*       has a character map.
*
** Used by PackMap() below and ResetBuffer()

SortBuffer:
		move.w	cu_CM+cm_BufferWidth(a2),d2
		move.l	cu_CM+cm_BufferStart(a2),d4
		move.w	cu_CM+cm_BufferHeight(a2),d5
		move.l	cu_CM+cm_AttrBufLines(a2),a3
		move.l	cu_CM+cm_BufferXL(a2),d3	; test both, get Y
		bne	sortMap				; if already empty
							; nothing to sort
		rts

*------ PackMap ------------------------------------------------------
*
*   NAME
*	PackMap
*
*   SYNOPSIS
*	PackMap(consoleUnit)
*	        A2
*
PackMap:
		movem.l	d2-d7/a3-a5,-(a7)

;   d2	map width
;   d3	map current limit (XxxxxxYL)
;   d4	slot 0 offset (address/2)
;   d5	== BufferHeight for Buf, == -1 for Disp
;   a3	AttrXxxxLines array base
		;-- sort lines in buffer
		bsr.s	SortBuffer

		;-- sort lines in display
pmSortDisplay:
		move.w	cu_CM+cm_DisplayWidth(a2),d2
		move.l	cu_DisplayXL(a2),d3		; test both, get Y
		beq	pmDone
		move.l	cu_CM+cm_DisplayStart(a2),d4
		moveq	#-1,d5
		move.l	cu_CM+cm_AttrDispLines(a2),a3
		bsr	sortMap


		moveq	#0,d0
		move.b	cu_BgColor(a2),d0
		lsl.w	#CMAS_BGPEN,d0
		or.w	#CMAF_RENDERED,d0
		move.l	d0,-(a7)
		move.w	cu_YCP(a2),d0
		move.l	d0,-(a7)
		move.w	cu_XCP(a2),d0
		move.l	d0,-(a7)
		move.w	cu_DisplayYL(a2),d0
		move.l	d0,-(a7)
		move.w	cu_DisplayXL(a2),d0
		move.l	d0,-(a7)
		pea	cu_CM(a2)
		bsr	_packSorted
		lea	24(a7),a7

		;	    (packSorted clears selection as side effect)
		bclr	#CUB_SELECTED,cu_Flags(a2)
		beq.s	pmScrollBuffer

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)
		clr.l	cd_SelectedUnit(a6)
		bclr	#CUB_CURSSELECT,cu_CursorFlags(a2)

		;--	    scroll buffer
pmScrollBuffer:
		move.w	cu_CM+cm_BufferYL(a2),d0
		move.w	cu_CM+cm_BufferLines(a2),d1
		sub.w	d1,d0
		blt.s	pmDone

		;--	    scroll up buffer
		addq.w	#1,d0
		sub.w	d0,cu_CM+cm_BufferYL(a2)
		add.w	d0,d1
		move.w	cu_CM+cm_BufferWidth(a2),d2
		mulu	d2,d0
		mulu	d2,d1
		move.l	cu_CM+cm_BufferStart(a2),a0
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1

;-- old scrollMap in sorted environment
;
;	d0	character increment to scroll (+: up,  -: down)
;	d1	scroll field character size
;

		cmp.l	d0,d1		; scroll increment vs. field size
		ble.s	pmUpClrAll	; clear everything (?) ok, but I fail
					; to see how we ever fail this test.
					; Looks like hold-over code; I'll leave
					; it alone for now. -darren-
					;
					; we get here when the packed map
					; overflows into our work area
					; (there is an extra 50% of buffer
					; allocated at map alloc time)
					;
					; scroll up packed buffer, and
					; throw away what we can't use
				
		lea	0(a0,d0.l),a3	; get source as per increment
		add.l	d0,d0		;
		lea	0(a1,d0.l),a4	;
		lsr.l	#1,d0		;
		sub.l	d0,d1		; get characters to copy

		move.l	d0,d2
		swap	d2		; for double DBF loop

		move.l	d1,d3
		swap	d3		; for double DBF loop

		bra.s	pmUpCopyDBF
pmUpCopyLoop:
		move.b	(a3)+,(a0)+	; copy character
		move.w	(a4)+,(a1)+	; copy attribute
pmUpCopyDBF:
		dbf	d1,pmUpCopyLoop
		dbf	d3,pmUpCopyLoop
		bra.s	pmUpClrDBF
pmUpClrLoop:
		clr.w	(a1)+		; empty attribute
					; clear rest of attribute buffer
pmUpClrDBF:
		dbf	d0,pmUpClrLoop
		dbf	d2,pmUpClrLoop
		bra.s	pmDone

pmUpClrAll:
		move.l	d1,d0		; clear entire scroll field

		move.l	d0,d2
		swap	d2		; for double DBF loop

		bra.s	pmUpClrDBF

pmDone:
		movem.l	(a7)+,d2-d7/a3-a5
		rts



*------ UnpackMap ----------------------------------------------------
*
*   NAME
*	UnpackMap
*
*   SYNOPSIS
*	UnpackMap(consoleUnit)
*	          A2
*
UnpackMap:
		movem.l	d2-d3,-(a7)
		;-- set Display variables
		movem.w	cu_XMax(a2),d0/d1		; and cu_YMax
		addq.w	#1,d0
		addq.w	#1,d1
		movem.w	d0/d1,cu_CM+cm_DisplayWidth(a2)	; and cm_DisplayHeight
		;-- find display memory store
		move.l	d0,d2				; save DisplayWidth
		mulu	d1,d0
		sub.l	cu_CM+cm_AllocSize(a2),d0
		neg.l	d0				; first CharDisp index
		add.l	cu_CM+cm_AllocBuffer(a2),d0	; first CharDisp byte
		sub.l	cu_CM+cm_AttrToChar(a2),d0	; first AttrDisp byte/2
		move.l	d0,d3				; save it
		move.l	d0,cu_CM+cm_DisplayStart(a2)	;
		;--	fill AttrDispLines
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		bra.s	umAttrDispLinesDBF
umAttrDispLinesLoop:
		move.l	d0,(a0)+
		add.l	d2,d0
umAttrDispLinesDBF:
		dbf	d1,umAttrDispLinesLoop

		sub.l	cu_CM+cm_BufferStart(a2),d3
		divu	cu_CM+cm_BufferWidth(a2),d3
		subq	#1,d3			; spare used by PackMap sort
		cmp.w	cu_CM+cm_BufferLines(a2),d3
		ble.s	umBufferHeightOK
		move.w	cu_CM+cm_BufferLines(a2),d3
umBufferHeightOK:
		move.w	d3,cu_CM+cm_BufferHeight(a2)

		;--	test for empty buffer
		tst.l	cu_CM+cm_BufferXL(a2)	; and BufferYL
		beq.s	umEmpty

		move.w	cu_CM+cm_DisplayHeight(a2),d2
		clr.w	cu_DisplayXL(a2)
		move.w	d2,d0
		subq.w	#1,d0
		move.w	d0,cu_DisplayYL(a2)
		moveq	#0,d3
		move.w	#CMAF_CURSOR,-(a7)	; cursorMask argument
		pea	cu_YCP(a2)
		pea	cu_XCP(a2)
		pea	8(a7)
		pea	cu_DisplayYL(a2)
		pea	cu_DisplayXL(a2)
		bra.s	umDBF

		;--	unpack each line
umLoop:
		move.l	d2,-(a7)
		pea	cu_CM(a2)
		bsr	_unpackSortedLine
		addq.l	#8,a7
		add.w	d0,d3
umDBF:
		dbf	d2,umLoop

		lea	22(a7),a7

		;--	handle partial unpacking
		move.w	d3,d0
		beq.s	umCursUpdate

		moveq	#0,d1
		bsr.s	scrollDisp

		sub.w	d3,cu_YCP(a2)
		sub.w	d3,cu_DisplayYL(a2)
umCursUpdate:
		bsr	CursUpdate

		movem.l	(a7)+,d2-d3
		rts

umEmpty:
		clr.l	cu_XCP(a2)		; and YCP
		clr.l	cu_DisplayXL(a2)	; and DisplayYL
		bra.s	umCursUpdate


;------ scrollDisp ----------------------------------------------------
;
;   d0	scroll amount (+ up, - down)
;   d1	scroll origin
;   a2	console unit
;

;   d0	source offset
;   d1	destination offset
;   d2	offset limit
;   d3	decrementing scroll height
;   d4	cache
;   d5  shift increment
;   a0	shifted cm_AttrDispLines origin
;   a1	address temporary
;   a4	address of slot now in cache

scrollDisp:
		movem.l	d2-d5/a4,-(a7)
		move.w	cu_CM+cm_DisplayHeight(a2),d3
		sub.w	d1,d3		; scroll height
		move.w	d3,d2
		lsl.w	#2,d2		; offset limit
		lsl.w	#2,d1
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		add.w	d1,a0		; shifted cm_AttrDispLines origin

		lsl.w	#2,d0		; first source offset
		move.w	d0,d5		;   and shift increment
		moveq	#0,d1		; first destination offset
		move.l	a0,a4
		move.l	(a4),d4		; cache first destination
		tst.w	d0
		bra.s	sdBound
sdLoop:
		lea	0(a0,d0.w),a1		; address of source
		cmp.l	a1,a4			; check if source is in cache
		bne.s	sdStandard

		;--	end of cycle, bump to next cycle
		move.l	d4,0(a0,d1.w)		; copy cache to dest
		addq.w	#4,d0			; bump to next cycle
		cmp.w	d2,d0			; limit source
		blt.s	sdNextCycleDest		;
		sub.w	d2,d0			;   adjust to within limit
sdNextCycleDest:
		lea	0(a0,d0.w),a4
		move.l	(a4),d4			; cache destination
		bra.s	sdNextSource

		;--	standard cycle
sdStandard:
		move.l	(a1),0(a0,d1.w)		; copy source to dest
sdNextSource:
		move.w	d0,d1			; old source is new dest
		add.w	d5,d0			; bump source by increment
sdBound:
		bpl.s	sdBoundUpper
		add.w	d2,d0
		bra.s	sdDBF
sdBoundUpper:
		cmp.w	d2,d0			; limit source
		blt.s	sdDBF			;
		sub.w	d2,d0			;   adjust to within limit
sdDBF:
		dbf	d3,sdLoop

sdDone:
		movem.l	(a7)+,d2-d5/a4
		rts


*------ ScrollYDisplay -----------------------------------------------
*
*   NAME
*	ScrollYDisplay - convert character units to raster units and scroll
*
*   SYNOPSIS
*	ScrollYDisplay(consoleUnit, increment)
*	               a2           d0
*
*---------------------------------------------------------------------
ScrollYDisplay:
		movem.l	d2-d5/a4-a5,-(a7)
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	sydSR

		move.w	d0,d5		; save increment
		;-- find scroll direction
		bpl.s	sydUp
		beq.s	sydDone

		;--	scroll down
		;--	    scroll off bottom of display
		moveq	#0,d1
		bsr	insDelLine
		bra.s	sydDone

		;--	scroll up
sydUp:
		;--	    gobble top into buffer
		move.w	d5,d3		; for <increment> lines
		moveq	#0,d2		; from top of display
sydUpLineLoop:

	PRINTF	DBG_FLOW,<'D2=%lx'>,D2

		bsr	PackMapLine
		addq.w	#1,d2
		cmp.w	d2,d3
		bgt.s	sydUpLineLoop

		move.w	d5,d0
sydSR:
		moveq	#0,d1
		bsr	insDelLine

sydDone:
		movem.l	(a7)+,d2-d5/a4-a5
		rts


*------ InsDelChar ---------------------------------------------------
*
*   NAME
*	InsDelChar - insert or delete characters on line
*
*   SYNOPSIS
*	InsDelChar(consoleUnit, increment)
*	           a2           d0
*
*---------------------------------------------------------------------
InsDelChar:
		movem.l	d2-d6/a4-a5,-(a7)

		;-- check for degenerate case
		tst.w	d0
		beq	iclDone

		;-- check whether any chars are displayed here
		move.w	cu_DisplayYL(a2),d3
		move.w	cu_DisplayXL(a2),d2
		bne.s	iclHaveDYL
		move.w	cu_XMax(a2),d2
		addq.w	#1,d2
		subq.w	#1,d3
iclHaveDYL:
		move.w	d2,d6			; cache massaged XL

		movem.w	cu_XCP(a2),d4/d5
		cmp.w	d5,d3

		blt	iclDone			; SIGNED compare!!!
						; e.g., d3 = -1
		bne.s	iclScroll

		;-- this ins/del is on the last display line
		sub.w	d4,d2
		ble	iclDone

		;--     set the new DisplayYL (could be less than
		;--	what it was (e.g., if XL was == 0, YL may be
		;--	on previous line - calculated upon entry of
		;--	subroutine)
		;--
		;--	We have determined that we are inserting, or
		;--	deleting on the last line of the display (last
		;--	valid line in map, but note that we came up
		;--	with a massaged value for DisplaYL above)

		move.w	d3,cu_DisplayYL(a2)

		;-- check whether scrolling or clearing is appropriate
		tst.w	d0			; check increment
		bpl.s	iclDelete


		;--	check if clearing instead of inserting is OK
		sub.w	cu_XMax(a2),d4		; XCP - XMax
		cmp.w	d0,d4
		bgt	iclClear		; clear d2 chars from CP

		;--	update DisplayXL
		move.w	d6,d1			; DisplayXL calculated
		sub.w	d0,d1			; add increment
		cmp.w	cu_XMax(a2),d1
		ble.s	icliSetDisplayXL
		move.w	d6,d1
icliSetDisplayXL:
		move.w	d1,cu_DisplayXL(a2)
		move.w	d1,d4

iclInsertScroll:
		;--	check if character map insertion is needed
		tst.l	cu_CM+cm_AllocSize(a2)
		beq	iclSR

		movem.w	cu_XCP(a2),d2/d3	; and cu_YCP
		lsl.w	#2,d3
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d3.w),a0
		add.w	d4,a0			; new DisplayXL or DisplayWidth
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		move.l	a0,a4
		move.l	a1,a5
		add.l	a1,a1
		add.w	d0,a4
		add.w	d0,a5
		add.l	a5,a5
		sub.w	d2,d4
		add.w	d0,d4
		bra.s	icliShiftDBF
		;--	    shift characters to the right
icliShiftLoop:
		move.b	-(a4),-(a0)
		move.w	-(a5),-(a1)
icliShiftDBF:
		dbf	d4,icliShiftLoop

		move.w	d0,d4
		neg.w	d4
		bra.s	icliFillDBF
		;--	    fill space with blanks
icliFillLoop:
		move.b	#' ',-(a0)
		move.w	#CMAF_RENDERED,-(a1)
icliFillDBF:
		dbf	d4,icliFillLoop
		bra.s	iclSR


iclScroll:
		move.w	cu_CM+cm_DisplayWidth(a2),d4
		tst.w	d0			; check increment
		bmi	iclInsertScroll
		bra.s	iclDeleteScroll


		;--	check if clearing instead of deleting is OK
iclDelete:


		cmp.w	d0,d2
		ble.s	iclClear		; clear d2 chars from CP

		;--	update DisplayXL
		move.w	d6,d4			; DisplayXL (massaged)

		clr.w	cu_DisplayXL(a2)	; 0 minimum

		sub.w	d0,d4
		bmi.s	iclDelMin

		move.w	d4,cu_DisplayXL(a2)
iclDelMin:
		add.w	d0,d4			; negative cond. caught below

iclDeleteScroll:
		;--	check if character map deletion is needed
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	iclSR

		movem.w	cu_XCP(a2),d2/d3	; and cu_YCP
		lsl.w	#2,d3
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d3.w),a0
		add.w	d2,a0			; XCP
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		move.l	a0,a4
		move.l	a1,a5
		add.l	a1,a1
		add.w	d0,a4			; XCP + increment
		add.w	d0,a5
		add.l	a5,a5

		sub.w	d2,d4			; old DisplayXL or DisplayWidth
		sub.w	d0,d4			;   minus XCP minus increment

		tst.w	d4
		bmi.s	icldShiftNone

		bra.s	icldShiftDBF
		;--	    shift characters to the left
icldShiftLoop:
		move.b	(a4)+,(a0)+
		move.w	(a5)+,(a1)+
icldShiftDBF:
		dbf	d4,icldShiftLoop

icldShiftNone:
		move.w	d0,d4			; get size of cleared space
		bra.s	icldClearDBF
		;--	    clear vacated cells
icldClearLoop:
		clr.w	(a1)+
icldClearDBF:
		dbf	d4,icldClearLoop


iclSR:
		bsr	scrollChar

iclDone:
		movem.l	(a7)+,d2-d6/a4-a5
		rts


iclClear:
		move.w	cu_XCP(a2),d0
		mulu	cu_XRSize(a2),d0
		add.w	cu_XROrigin(a2),d0

		move.w	cu_YCP(a2),d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1

		mulu	cu_XRSize(a2),d2
		add.w	d0,d2
		subq.w	#1,d2

		move.w	d1,d3
		add.w	cu_YRSize(a2),d3
		subq.w	#1,d3

		bsr	CursDisable
		bsr	ClearRaster
		bsr	CursEnable

		;--	set the new DisplayXL to the CP on this line
		move.w	cu_XCP(a2),cu_DisplayXL(a2)
		bra.s	iclDone


*------ InsDelLine ---------------------------------------------------
*
*   NAME
*	InsDelLine - insert or delete lines in a display
*
*   SYNOPSIS
*	InsDelLine(consoleUnit, increment)
*	           a2           d0
*	insDelLine(consoleUnit, increment, ycp)
*	           a2           d0         d1
*
*---------------------------------------------------------------------
;
;   d0	increment (positive is delete)
;   d1	number of lines actually displayed after insdel point (inclusive)
;   d2	cu_YCP, insdel point
;   d3	temp, then increment
;   d4	temp
;   d5	temp
;
InsDelLine:
		move.w	cu_YCP(a2),d1

insDelLine:
		movem.l	d2-d5,-(a7)
		;-- check for degenerate case
		tst.w	d0
		beq	idlDone

		move.w	d1,d2

		;-- check whether any lines are displayed here
		move.w	cu_DisplayYL(a2),d1
		tst.w	cu_DisplayXL(a2)
		bne.s	idlHaveDYL
		subq.w	#1,d1
idlHaveDYL:
		sub.w	d2,d1
		blt	idlDone
		addq.w	#1,d1			; displayed lines: 1..n

		;-- check whether scrolling or clearing is appropriate
		tst.w	d0			; check increment
		bpl	idlDelete

		;--	check if clearing instead of inserting is OK
		move.w	d2,d3
		sub.w	cu_YMax(a2),d3		; YCP - YMax
		cmp.w	d0,d3			; -insert ? -remaining_space
		bgt	idlClear		; clear d1 lines at CP

		;--	set the new Display.L
		move.w	cu_DisplayYL(a2),d4
		sub.w	d0,d4			; add increment
		cmp.w	cu_YMax(a2),d4
		ble.s	idliSetDisplayYL
		;--	     bound DisplayYL, find DisplayXL if mapped
		move.w	d4,d3
		move.w	cu_YMax(a2),d4
		move.w	cu_XMax(a2),d5
		addq.w	#1,d5
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	idliSetDisplayXL

		;--	     find last char in line that will be the last line
		sub.w	d4,d3
		neg.w	d3
		add.w	d4,d3
		lsl.w	#2,d3
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d3.w),a0
		add.w	d5,a0
		add.l	a0,a0
idliFindDisplayXLLoop:
		tst.w	-(a0)
		dbmi	d5,idliFindDisplayXLLoop
		bmi.s	idliSetDisplayXL
		moveq	#0,d5

idliSetDisplayXL:
		move.w	d5,cu_DisplayXL(a2)

idliSetDisplayYL:
		move.w	d4,cu_DisplayYL(a2)

		;--	check if character map insertion is needed
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	idlSR

		move.w	d0,d3
		move.w	d2,d1
		bsr	scrollDisp

		;--	    fill empty space with empty lines
		move.w	d2,d4
		lsl.w	#2,d4
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		add.w	d4,a0
		move.w	d3,d5
		neg.w	d5
		bra.s	idliFillYDBF

idliFillYLoop:
		move.l	(a0)+,a1
		add.l	a1,a1
		move.w	cu_CM+cm_DisplayWidth(a2),d4
		bra.s	idliFillXDBF

idliFillXLoop:
		clr.w	(a1)+
idliFillXDBF:
		dbf	d4,idliFillXLoop

idliFillYDBF:
		dbf	d5,idliFillYLoop

		bra.s	idlSRD3

idlDelete:
		;--	check if clearing instead of deleting is OK
		cmp.w	d0,d1
		ble.s	idlClear		; clear d1 lines at CP

		;--	set the new Display.L
		sub.w	d0,cu_DisplayYL(a2)

		;--	check if character map deletion is needed
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	idlSR

		move.w	d0,d3
		move.w	d2,d1
		bsr	scrollDisp

idlSRD3:
		move.w	d3,d0

idlSR:
		bsr.s	scrollLine

idlDone:
		movem.l	(a7)+,d2-d5
		rts


idlClear:
		;--	set the new Display.L to the beginning of this line
		clr.w	cu_DisplayXL(a2)
		move.w	d2,cu_DisplayYL(a2)

		;--	clear the rectangle
		mulu	cu_YRSize(a2),d1
		move.w	d1,d3

		move.w	d2,d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		add.w	d1,d3
		subq.w	#1,d3

		move.w	cu_XROrigin(a2),d0
		move.w	cu_XRExtant(a2),d2
		
		bsr	CursDisable
		bsr	ClearRaster
		bsr	CursEnable

		bra.s	idlDone


;------ raster scroll routines ---------------------------------------
;
;   INPUT
;	d0	delta
;	d2	scrollLine YCP
;	a2	unit
;	a6	device
;   MODIFIED
;	d0-d5/a0-a1
;
scrollLine:
		move.w	d2,d3
		mulu	cu_YRSize(a2),d3
		add.w	cu_YROrigin(a2),d3
		bra.s	scrollSRLine
		
scrollChar:
		muls	cu_XRSize(a2),d0
		beq.s	scrollRts
		moveq	#0,d1
		move.w	cu_XCP(a2),d2
		mulu	cu_XRSize(a2),d2
		add.w	cu_XROrigin(a2),d2
		move.w	cu_YCP(a2),d3
		mulu	cu_YRSize(a2),d3
		add.w	cu_YROrigin(a2),d3
		move.l	d3,d5
		add.w	cu_YRSize(a2),d5
		subq.w	#1,d5
		bra.s	scrollSRChar

scrollRts:
		rts

scrollDisplay:
		move.w	cu_YROrigin(a2),d3
		ext.l	d3

scrollSRLine:
		move.w	d0,d1
		beq.s	scrollRts
		moveq	#0,d0
		muls	cu_YRSize(a2),d1
		move.w	cu_XROrigin(a2),d2
		ext.l	d2
		move.w	cu_YRExtant(a2),d5
		ext.l	d5

scrollSRChar:
		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne.s	scrollRts
		move.w	cu_XRExtant(a2),d4
		ext.l	d4
		bsr	CursDisable
		movem.w	d0/d1,-(a7)		; save rectangle origin
		move.b	cu_BgColor(a2),d0
		lea	cd_RastPort(a6),a1
		LINKGFX SetBPen

		movem.w	(a7)+,d0/d1
		lea	cd_RastPort(a6),a1

		; Fix bug in conceal mode, and avoid cursor
		; droppings.  Conceal mode used to set rp_Mask
		; to 0, so clears/scrolls didn't work at all -
		; real bad since the screen is not synced with
		; the character map.
		;
		; Also optimize scrolling

		move.b	cu_ScrollMask(a2),rp_Mask(a1)
		
		LINKGFX	ScrollRaster

		bsr	RestoreRP
		bra	CursEnable


	END
