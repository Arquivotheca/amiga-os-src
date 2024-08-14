**
**      $Id: clear.asm,v 36.30 92/05/19 10:29:37 darren Exp $
**
**      clear areas of the console
**
**      (C) Copyright 1985,1989,1991,1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"debug.i"

**	Exports

	XDEF		CDClear
	XDEF		CDCBClear

	XDEF		ClearEOL
	XDEF		ClearEOD
	XDEF		ClearDisplay
	XDEF		ClearScreen
	XDEF		ClearRaster
	XDEF		RestoreRP


**	Imports

	XLVO	SetAPen			; Graphics
	XLVO	RectFill		;
	XLVO	SetDrMd			;

	XREF	ResetBuffer		; scroll

	XREF	sgrNewMask		; write

**	Assumptions

	IFNE	CUFB_FIXEDBG-7
	FAIL	"CUFB_FIXEDBG not most significant bit"
	ENDC
	IFNE	cu_ReadLastOut-cu_ReadLastIn-2
	FAIL	"cu_ReadLastOut does not follow cu_ReadLastIn"
	ENDC
	IFNE	cu_YCP-cu_XCP-2
	FAIL	"cu_YCP does not follow cu_XCP"
	ENDC
	IFNE	cu_DisplayYL-cu_DisplayXL-2
	FAIL	"cu_DisplayYL does not follow cu_DisplayXL"
	ENDC
	IFNE	RP_JAM2-1
	FAIL	"RP_JAM2 not bit 0, recode"
	ENDC
	IFNE	RP_INVERSVID-4
	FAIL	"RP_INVERSVID not bit 2, recode"
	ENDC


******* console.device/CMD_CLEAR *************************************
*
*    NAME
*	CMD_CLEAR -- Clear console input buffer.
*
*    FUNCTION
*	Remove from the console input buffer any reports waiting to
*	satisfy read requests.
*
*    IO REQUEST INPUT
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_CLEAR
*	io_Flags	IOB_QUICK set if quick I/O is possible, else 0
*
*    SEE ALSO
*	exec/io.h, devices/console.h
*
**********************************************************************

CDCBClear	EQU	-1

CDClear:
		move.l	IO_UNIT(a1),a0
		clr.l	cu_ReadLastIn(a0)	; and cu_ReadLastOut
		rts


*------	ClearEOL -----------------------------------------------------
*
*   NAME
*	ClearEOL - clear from current position to end of line
*
*---------------------------------------------------------------------
ClearEOL:
		movem.l d2-d3,-(a7)

		;-- find clearing area
		movem.w	cu_XCP(a2),d0/d1		; and cu_YCP
		move.w	cu_XMax(a2),d2			; right bound
		addq.w	#1,d2				;

		;--	check for new display.L
		cmp.w	cu_DisplayYL(a2),d1
		bne.s	ceolHaveEOD
		cmp.w	cu_DisplayXL(a2),d0
		bge.s	ceolDone
		move.w	cu_DisplayXL(a2),d2	; right bound
		move.w	d0,cu_DisplayXL(a2)	; new Display.L

ceolHaveEOD:
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	ceolCR

		;-- clear buffer area
		lsl.w	#2,d1
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d1.w),a0

		add.w	d0,a0
		add.l	a0,a0

		sub.w	d0,d2
		bra.s	ceolCBDBF
ceolCBLoop:
		clr.w	(a0)+
ceolCBDBF:
		dbf	d2,ceolCBLoop

		;-- clear raster area
ceolCR:
		bsr	clearEOL
ceolDone:
		movem.l	(a7)+,d2-d3
		rts


*------	ClearEOD -----------------------------------------------------
*
*   NAME
*	ClearEOD -- clear from current position to end of RastPort
*
*---------------------------------------------------------------------
ClearEOD:
		movem.l d2-d3,-(a7)

		tst.l	cu_XCP(a2)		; and cu_YCP
		beq.s	cdEntry

		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	ceodCR

		;-- check for new display.L
		movem.w	cu_XCP(a2),d0/d1	; and cu_YCP
		movem.w	cu_DisplayXL(a2),d2/d3	; and cm_DisplayYL

		;--	check for anything to clear
		cmp.w	d3,d1
		bgt.s	ceodDone
		bne.s	ceodNewEOD

		cmp.w	d2,d0
		bge.s	ceodDone

ceodNewEOD:
		movem.w	d0/d1,cu_DisplayXL(a2)	; and cm_DisplayYL

		;-- clear raster area
ceodCR:
		bsr.s	clearEOL

		move.w	cu_XROrigin(a2),d0
		move.w	cu_YCP(a2),d1
		addq.w	#1,d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		bsr.s	clearEODEntry
ceodDone:
		movem.l	(a7)+,d2-d3
		rts


*------	ClearScreen ------------------------------------------------------
*
*   NAME
*	ClearScreen -- clear raster display
*
*---------------------------------------------------------------------
ClearScreen:
		movem.l	d2-d3,-(a7)

		movem.l	d0-d1,-(a7)
		moveq	#RP_JAM2,d0
		lea	cd_RastPort(a6),a1
		LINKGFX SetDrMd
		movem.l	(a7)+,d0-d1
		bra.s	cdCR


*------	ClearDisplay -----------------------------------------------------
*
*   NAME
*	ClearDisplay -- clear entire display
*
*---------------------------------------------------------------------
ClearDisplay:
		movem.l d2-d3,-(a7)
cdEntry:
		;-- show cursor is not rendered
		clr.l	cu_CursorPattern(a2)
		;-- clear display and off screen buffers
		clr.l	cu_DisplayXL(a2)	; and cu_DisplayYL

		bsr	ResetBuffer	
**		clr.l	cu_CM+cm_BufferXL(a2)	; and cm_BufferYL

		;--FIX!! confusing background & cell char
		;--they are different, so I dont mix them
		;--like the original code below


*		tst	cu_FixedFlags(a2)	; btst #CUFB_FIXEDBG,
*		bmi.s	cdCR
*		move.b	cu_BgPen(a2),cu_BgColor(a2)
*

		;-- reset optimal scrolling mask having cleared the screen

		move.b	#1,cu_ScrollMask(a2)
		bsr	sgrNewMask

cdCR:
		;-- delay border/edge fill, and cache last draw mode

		move.b	cd_RastPort+rp_DrawMode(a6),cu_ClearDrMd(a2)

		bset	#CUB_BORDERFILL,cu_States(a2)

		move.w	cu_XROrigin(a2),d0
		move.w	cu_YROrigin(a2),d1

		bsr.s	clearEODEntry

		movem.l	(a7)+,d2-d3
		rts

;---------------------------------------------------------------------
clearEOL:
		move.w	cu_XCP(a2),d0
		mulu	cu_XRSize(a2),d0
		add.w	cu_XROrigin(a2),d0
		move.w	cu_YCP(a2),d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		move.l	d1,d3
		add.w	cu_YRSize(a2),d3
		subq.w	#1,d3
		bra.s	clearEOLEntry


clearEODEntry:
		move.w	cu_YRExtant(a2),d3
clearEOLEntry:
		move.w	cu_XRExtant(a2),d2
; drop		bsr.s	ClearRaster
; thru		rts


*------ ClearRaster --------------------------------------------------
*
*   NAME
*	ClearRaster - clear a rectangle of the raster port
*
*   SYNOPSIS
*	error = ClearRaster(xl, yl, xu, yu)
*			    d0  d1  d2  d3
*
*   FUNCTION
*	This function sets a rectangle in the RastPort to the
*	background color for DrawMode 1, or zero otherwise
*
*---------------------------------------------------------------------
ClearRaster:
		;-- ensure that the result is a positive rectangle
		cmp.w	d0,d2
		blt.s	noClearing
		cmp.w	d1,d3
		blt.s	noClearing
		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne.s	noClearing

		movem.w	d0/d1,-(a7)		; save rectangle origin
		move.b	cu_BgColor(a2),d0
		lea	cd_RastPort(a6),a1
		LINKGFX SetAPen
		clr.l	cd_RastPort+rp_AreaPtrn(a6)

		movem.w	(a7)+,d0/d1

		lea	cd_RastPort(a6),a1

		; Fix bug in conceal mode, and avoid cursor
		; droppings.  Conceal mode used to set rp_Mask
		; to 0, so clears/scrolls didn't work at all -
		; real bad since the screen is not synced with
		; the character map.

		move.b	#$FF,rp_Mask(a1)

		LINKGFX RectFill

		;-- FALL THROUGH

;------ Restore shared RastPort drawing values ---------------------------------------
;
;   INPUT
;	a2	unit
;	a6	device
;
RestoreRP:
		move.l	cu_Mask(a2),cd_RastPort+rp_Mask(a6)
		move.l	cu_Minterms(a2),cd_RastPort+rp_minterms(a6)
		move.l	cu_Minterms+4(a2),cd_RastPort+rp_minterms+4(a6)
		move.b	cu_DrawMode(a2),cd_RastPort+rp_DrawMode(a6)
noClearing:
		rts

	END
