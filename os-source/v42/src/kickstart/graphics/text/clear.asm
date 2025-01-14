**
**	$Id: clear.asm,v 42.1 93/07/20 13:41:35 chrisg Exp $
**
**      clear functions for ABasic
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"

	INCLUDE		"/gfx.i"
	include		'/bitmap_internal.i'
	INCLUDE		"/rastport.i"
	include		'/macros.i'

	INCLUDE		"text.i"

	INCLUDE		"macros.i"


*------ Imported Functions -------------------------------------------

	XLVO	ClearEOL		; Graphics
	XLVO	SetAPen			;
	XLVO	RectFill		;


*------ Exported Functions -------------------------------------------

	XDEF		_ClearEOL
	XDEF		_ClearScreen


******* graphics.library/ClearEOL ************************************
*
*   NAME
*	ClearEOL -- Clear from current position to end of line.
*
*   SYNOPSIS
*	ClearEOL(rp)
*	         A1
*
*	void ClearEOL(struct RastPort *);
*
*   FUNCTION
*	Clear a rectangular swath from the current position to the
*	right edge of the rastPort.  The height of the swath is taken
*	from that of the current text font, and the vertical
*	positioning of the swath is adjusted by the text baseline,
*	such that text output at this position would lie wholly on
*	this newly cleared area.
*	Clearing consists of setting the color of the swath to zero,
*	or, if the DrawMode is 2, to the BgPen.
*
*   INPUTS
*	rp - pointer to RastPort structure
*
*   RESULT
*
*   NOTES
*	o   This function may use the blitter.
*
*   SEE ALSO
*	Text()  ClearScreen()  SetRast()
*	graphics/text.h  graphics/rastport.h
*
**********************************************************************
_ClearEOL:
		MOVEM.L D2-D3,-(A7)
		MOVE.W	rp_cp_x(A1),D0
		EXT.L	D0
		MOVE.W	rp_cp_y(A1),D1
		SUB.W	rp_TxBaseline(A1),D1
		EXT.L	D1
		MOVEQ	#0,D2
		MOVE.L	rp_BitMap(A1),A0
		MOVE.W	bm_BytesPerRow(A0),D2
		cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
		bne.s	no_interleaved
		btst	#IBMB_INTERLEAVED,bm_Flags(a0)
		beq.s	no_interleaved
		move.l	bm_Planes+4(a0),d2
		sub.l	bm_Planes(a0),d2
no_interleaved:
		LSL.W	#3,D2
		SUBQ.W	#1,D2
		MOVE.L	D1,D3
		MOVE.L	rp_Font(A1),A0
		ADD.W	tf_YSize(A0),D3
		SUBQ.W	#1,D3
		BSR.s	clearRaster
		MOVEM.L (A7)+,D2-D3
		RTS


******* graphics.library/ClearScreen *********************************
*
*   NAME
*	ClearScreen -- Clear from current position to end of RastPort.
*
*   SYNOPSIS
*	ClearScreen(rp)
*	            A1
*
*	void ClearScreen(struct RastPort *);
*
*   FUNCTION
*	Clear a rectangular swath from the current position to the
*	right edge of the rastPort with ClearEOL, then clear the rest
*	of the screen from just beneath the swath to the bottom of
*	the rastPort.
*	Clearing consists of setting the color of the swath to zero,
*	or, if the DrawMode is 2, to the BgPen.
*
*   INPUTS
*	rp - pointer to RastPort structure
*
*   NOTES
*	o   This function may use the blitter.
*
*   SEE ALSO
*	ClearEOL()  Text()  SetRast()
*	graphics/text.h  graphics/rastport.h
*
**********************************************************************
_ClearScreen:
		MOVEM.L D2-D3,-(A7)
		MOVE.L	A1,-(A7)
		CALLLVO ClearEOL
		MOVE.L	(A7)+,A1
		MOVEQ	#0,D0
		MOVE.W	rp_cp_y(A1),D1
		SUB.W	rp_TxBaseline(A1),D1
		MOVE.L	rp_Font(A1),A0
		ADD.W	tf_YSize(A0),D1
		EXT.L	D1
		MOVEQ	#0,D2
		MOVE.L	rp_BitMap(A1),A0
		MOVE.W	bm_BytesPerRow(A0),D2
		cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
		bne.s	no_interleaved1
		btst	#IBMB_INTERLEAVED,bm_Flags(a0)
		bne.s	no_interleaved1
		move.l	bm_Planes+4(a0),d2
		sub.l	bm_Planes(a0),d2
no_interleaved1:
		LSL.W	#3,D2
		SUBQ.W	#1,D2
		MOVEQ	#0,D3
		MOVE.W	bm_Rows(A0),D3
		SUBQ.W	#1,D3
		CMP.L	D1,D3
		BLT.S	csClipped
		BSR.S	clearRaster
csClipped:
		MOVEM.L (A7)+,D2-D3
		RTS


*------ clearRaster --------------------------------------------------
*
*   NAME
*	clearRaster - clear a rectangle of the raster port
*
*   SYNOPSIS
*	clearRaster(RastPort, xl, yl, xu, yu)
*		    A1	      D0  D1  D2  D3
*
*   FUNCTION
*	This function sets a rectangle in the RastPort to the
*	background color for DrawMode 1, or zero otherwise
*	rp - pointer to RastPort structure
*
**********************************************************************
clearRaster:
		MOVEM.L D4-D6/A2,-(A7)
		MOVE.L	A1,A2		    ; save RastPort
		MOVE.B	rp_FgPen(A1),D4	    ; save foreground pen
		MOVE.L	rp_AreaPtrn(A1),D5  ; save areafill pattern
		MOVE.L	D0,D6		    ; save xl
		MOVEQ	#0,D0
		CMP.B	#RP_JAM2,rp_DrawMode(A1)
		BNE.S	isClear
		MOVE.B	rp_BgPen(A1),D0
isClear:
		MOVE.L	D1,-(A7)
		CALLLVO SetAPen
		MOVE.L	(A7)+,D1
		MOVE.W	D6,D0		    ; xl
		MOVE.L	A2,A1
		CALLLVO RectFill

		MOVE.L	A2,A1
		MOVE.L	D5,rp_AreaPtrn(A1)
		MOVE.B	D4,D0
		CALLLVO SetAPen

		MOVEM.L (A7)+,D4-D6/A2
		RTS

		END
