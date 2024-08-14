*
* $Id: coolfill.asm,v 39.0 92/01/07 13:57:38 mks Exp $
*
* $Log:	coolfill.asm,v $
* Revision 39.0  92/01/07  13:57:38  mks
* This is the first version for Workbench of the really cool backfill hook.
* 
*******************************************************************************
*
* The following INCLUDE files are needed to make this program assemble.
* They come with the Amiga Macro-Assembler Package.
*
	NOLIST					; No need to list these
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"graphics/clip.i"

	INCLUDE	"workbenchbase.i"
	LIST					; Ok, lets start the listing
*
*******************************************************************************
*
* This is the only fixed address in the system and it even "floats"...
*
		xref	_AbsExecBase
		xref	_LVOOpenWorkBench
*
*******************************************************************************
*
* Some macros that make calling the system routines easier...
*
CALLSYS		MACRO
		xref	_LVO\1		; Set the external reference
		jsr	_LVO\1(a6)	; Call the system...
		ENDM
*
JUMPSYS		MACRO
		xref	_LVO\1		; Set the external reference
		jmp	_LVO\1(a6)	; Jump to the system...
		ENDM
*
*******************************************************************************
*
* This is the code called by the layer hook...
*
CoolHook:	xdef	CoolHook
		movem.l	d2-d7/a3-a6,-(sp)	; Save these...
		move.l	h_SubEntry(a0),a4	; (WorkbenchBase in A4)
		move.l	h_Data(a0),a5		; Put data into address reg
		move.l	a1,d7			; Save the data packet
		move.l	a5,a0			; Get semaphore pointer...
		move.l	wb_SysBase(a4),a6	; Get EXECBASE
		CALLSYS	ObtainSemaphore		; Get the bitmap semaphore
		move.l	d7,a1			; Get data packet back
*
* Now, we do the rendering...
*
		move.l	(a1)+,a0		; Get the layer...
		move.l	wb_GfxBase(a4),a6	; Point at GfxBase
		move.l	bms_PatternBitMap(a5),d0	; Get PatternBitMap
		beq	SimpleCase		; None?  Simple (0) case
		move.l	d0,a3			; Save PatternBitMap...
*
* Ok, so we may have a problem here...
*
		move.l	a2,-(sp)		; Save
*
* Now, we need to blit this bitmap over to the screen but we need to
* stay within the bounds of the bitmap and repeat as needed
*
		move.l	rp_BitMap(a2),a2	; Get bitmap...
		move.l	(a1)+,d0
		move.l	d0,bms_ToMinX(a5)	; Set up working info
		move.l	d0,bms_FromMinX(a5)
		move.l	(a1)+,d0
		move.l	d0,bms_FromMaxX(a5)
*
* Now adjust the FROM for the window's position...
*
		move.w	lr_MinX(a0),d0		; Get left boarder...
		neg.w	d0			; Make negative...
X_Origin:	add.w	pbm_Width(a3),d0	; Make sure it is positive
		bmi.s	X_Origin		; Keep doing it until positive
		add.w	d0,bms_FromMinX(a5)	; Add it into the MinX
		add.w	d0,bms_FromMaxX(a5)	; ...and MaxX
*
		move.w	lr_MinY(a0),d0		; Get top edge...
		neg.w	d0			; Make it negative...
Y_Origin:	add.w	pbm_Height(a3),d0	; Make sure it is positive
		bmi.s	Y_Origin		; Keep doing it until positive
		add.w	d0,bms_FromMinY(a5)	; Add it into the MinY
		add.w	d0,bms_FromMaxY(a5)	; ...and MaxY
*
* Save off a value we will need
*
		move.w	bms_FromMinX(a5),bms_SaveMinX(a5)
		move.w	bms_FromMaxX(a5),bms_SaveMaxX(a5)
		move.w	bms_ToMinX(a5),bms_SaveMinX1(a5)
*
* Ok, so we know the rectangles we want to blit, now check
* for wrapping of the source...
*
Do_Wrap:	move.w	pbm_Width(a3),d1	; Get width...
XWrap:		cmp.w	bms_FromMinX(a5),d1	; Check it...
		bgt.s	No_XWrap
		sub.w	d1,bms_FromMinX(a5)	; Unwrap it
		sub.w	d1,bms_FromMaxX(a5)
		bra.s	XWrap
No_XWrap:
		move.w	pbm_Height(a3),d1	; Get height...
YWrap:		cmp.w	bms_FromMinY(a5),d1	; Check it...
		bgt.s	No_YWrap
		sub.w	d1,bms_FromMinY(a5)	; Unwrap it
		sub.w	d1,bms_FromMaxY(a5)
		bra.s	YWrap
No_YWrap:
*
* Ok, we know that the source is now starting within our bitmap
* so lets generate some of the coordinates:
*
		move.w	bms_ToMinX(a5),d2	; Get destination X
		move.w	bms_ToMinY(a5),d3	; Get destination Y
*
		move.w	bms_FromMaxX(a5),d4	; Get MaxX source
		cmp.w	pbm_Width(a3),d4	; Do we fit?
		blt.s	X_Ok			; If so, skip this
		move.w	pbm_Width(a3),d4	; Get width...
		subq.w	#1,d4			; We are really one less...
*
X_Ok:		move.w	bms_FromMinX(a5),d0	; Get source X
		sub.w	d0,d4
		addq.w	#1,d4			; Size of X blit...
		add.w	d4,bms_FromMinX(a5)	; Adjust X start
		add.w	d4,bms_ToMinX(a5)
*
		move.w	bms_FromMaxY(a5),d5	; Get MaxY source
		cmp.w	pbm_Height(a3),d5	; Check if we fit
		blt.s	Y_Ok			; If so, skip this...
		move.w	pbm_Height(a3),d5	; Get max height
		subq.w	#1,d5
*
Y_Ok:		move.w	bms_FromMinY(a5),d1	; Get source Y
		sub.w	d1,d5
		addq.w	#1,d5			; Size of Y blit...
*
* Ok, so set up the last few values:
*
		move.l	a2,a1			; Destination bitmap
		move.l	pbm_BitMap(a3),a0	; Source bitmap
		move.l	#$C0,d6			; Copy MinTerm
		moveq.l	#-1,d7			; All bit planes...
		CALLSYS	BltBitMap		; Do the blit...
*
* Ok, now check if we are done in X...
*
		move.w	bms_FromMaxX(a5),d0	; Get Max X
		sub.w	bms_FromMinX(a5),d0	; Subtract Min
		bge	Do_Wrap			; If not done, do again...
*
* Restore X...
*
		move.w	bms_SaveMinX(a5),bms_FromMinX(a5)
		move.w	bms_SaveMaxX(a5),bms_FromMaxX(a5)
		move.w	bms_SaveMinX1(a5),bms_ToMinX(a5)
*
* Now, check the Y...
*
		move.w	bms_FromMaxY(a5),d5	; Get MaxY source
		cmp.w	pbm_Height(a3),d5	; Check if we fit
		blt.s	RepBlit_Done		; If so, we are done...
*
* Oh boy, we need to adjust...
*
		move.w	pbm_Height(a3),d5	; Get max height
		subq.w	#1,d5
		sub.w	bms_FromMinY(a5),d5
		addq.w	#1,d5			; Size of Y blit...
		add.w	d5,bms_FromMinY(a5)
		add.w	d5,bms_ToMinY(a5)	; Adjust the Y values...
		bra	Do_Wrap			; and go again...
*
RepBlit_Done:	move.l	(sp)+,a2		; Restore
		bra.s	HookDone
*
* No bitmap, so just do the simple (0) minterm case...
*
SimpleCase:	moveq.l	#0,d2			; Clear d2
		move.w	ra_MinX(a1),d2		; Get X pos
*
		moveq.l	#0,d3
		move.w	ra_MinY(a1),d3		; Get Y pos
*
		moveq.l	#0,d4
		move.w	ra_MaxX(a1),d4
		sub.l	d2,d4
		addq.l	#1,d4			; Get X size
*
		moveq.l	#0,d5
		move.w	ra_MaxY(a1),d5
		sub.l	d3,d5
		addq.l	#1,d5			; Get Y size
*
		move.l	d2,d0			; X Source
		move.l	d3,d1			; Y Source
		moveq.l	#0,d6			; NULL minterm
		moveq.l	#-1,d7			; FF mask
*
		move.l	rp_BitMap(a2),a1	; Get bitmap
		move.l	a1,a0
		CALLSYS	BltBitMap		; Do the backfill-0
*
HookDone:	move.l	a5,a0			; Get semaphore
		move.l	wb_SysBase(a4),a6	; Get ExecBase
		CALLSYS	ReleaseSemaphore	; Release the semaphore
		movem.l	(sp)+,d2-d7/a3-a6	; Restore
		rts
*
*******************************************************************************
*
* "A master's secrets are only as good as the
*  master's ability to explain them to others."  -  Michael Sinz
*
*******************************************************************************
*
		end
