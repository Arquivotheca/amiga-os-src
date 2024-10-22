	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "dos/dos.i"
	INCLUDE "intuition/intuition.i"
	INCLUDE	"intuition/screens.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_SysBase
	XREF	_GfxBase
	XREF	_UtilityBase
	XREF	_LVOObtainSemaphoreShared
	XREF	_LVOReleaseSemaphore
	XREF	_LVOBltBitMap
	XREF	_LVOSignal
	XREF	_LVOGetTagData
	XREF	_LVOStricmp
	XREF	@CouldCloseWB
	XREF	_bl
	XREF	_iprefsTask
	XREF	_oldOpenScreen
	XREF	_oldCloseWorkBench
	XREF	_newScreenBackdrop

	XREF	@RemoveDTBackdrops

;---------------------------------------------------------------------------

	XDEF	@OpenScreenPatch
	XDEF	@CloseWorkBenchPatch
	XDEF	_BackdropRender
	XDEF	@BackdropRender
	XDEF	_WBName

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

   STRUCTURE BackdropLock,SS_SIZE
	APTR	ow_BitMap
	UWORD	ow_Width
	UWORD	ow_Height

;---------------------------------------------------------------------------

_WBName DC.B "Workbench",0

@OpenScreenPatch:
	movem.l	a0/a1/a6,-(sp)		; save these people

	move.l	a1,a0			; load tag list pointer
	move.l	#SA_PubName,d0		; tag to look for
	moveq	#0,d1			; default value if tag not found
	move.l	_UtilityBase,a6
	CALL	GetTagData
	tst.l	d0			; see if we got something
	bne.s	2$			; tag found, could be Workbench

1$:	movem.l	(sp)+,a0/a1/a6		; restore these
	move.l	_oldOpenScreen,-(sp)	; push address of old routine
	rts				; call the real thing

2$:	lea	_WBName(pc),a0		; "Workbench"
	move.l	d0,a1			; name of new screen
	CALL	Stricmp			; are they the same?
	tst.l	d0
	bne.s	1$			; not the right name, bye

	movem.l	(sp)+,a0/a1/a6		; restore these

	move.l	a2,-(sp)
	move.l	_oldOpenScreen,a2	; get address of old routine
	jsr	(a2)			; call real OpenScreenTagList()
	move.l	(sp)+,a2

common_patch:
	move.l	d0,-(sp)			; save return value
	move.l	a6,-(sp)			; save IntuitionBase
	move.l	_SysBase,a6			; load SysBase
	move.l	_iprefsTask,a1			; find IPrefs task
	move.l	#SIGBREAKF_CTRL_E,d0		; signal to send to IPrefs
	CALL	Signal				; signal IPrefs
	move.l	(sp)+,a6			; get IntuitionBase back
	move.l	(sp)+,d0			; get return value back
	rts					; bye!

;---------------------------------------------------------------------------

@CloseWorkBenchPatch:
	bsr	@CouldCloseWB			; see if WB can be closed
	tst.w	d0
	bne.s	1$

	moveq	#0,d0				; set return to FALSE
  	rts					; WB can't be closed, leave

1$:
	bsr	@RemoveDTBackdrops		; remove any datatypes backdrop
	move.l	_oldCloseWorkBench,a0		; get address of old routine
	jsr	(a0)				; do real function
	bra.s	common_patch

;---------------------------------------------------------------------------

@BackdropRender:
_BackdropRender:
		movem.l	d2-d7/a5/a6,-(sp)	; Save
		lea	_bl,a5			; semaphore
		move.l	_SysBase,a6		; Get ExecBase
		move.l	a5,a0			; Set up for Obtain
		CALL	ObtainSemaphoreShared	; Obtain the semaphore

		; Now, we do the rendering...
		addq.l	#4,a1			; Point at rectangle
		move.l	_GfxBase,a6		; Point at GfxBase
		move.l	ow_BitMap(a5),d0	; Get bitmap...
		beq	SimpleCase		; None?  Simple (0) case

		; Now we need to blit this bitmap over to the screen but we
		; need to stay within the bounds of the bitmap and repeat as
		; needed

		move.l	a2,-(sp)		; Save rastport
		move.l	rp_BitMap(a2),a2	; Get bitmap...
		move.l	(a1)+,d0
		move.l	d0,ToMinX(pc)		; Set up working info
		move.l	d0,FromMinX(pc)
		move.l	(a1)+,d1
		move.l	d1,FromMaxX(pc)

		; Save off a value we will need
		move.w	FromMinX(pc),SaveMinX(pc)
		move.w	FromMaxX(pc),SaveMaxX(pc)
		move.w	ToMinX(pc),SaveMinX1(pc)

		; Ok, so we know the rectangles we want to blit, now check
		; for wrapping of the source...

Do_Wrap:	move.w	ow_Width(a5),d1		; Get width...
XWrap:		cmp.w	FromMinX(pc),d1		; Check it...
		bgt.s	No_XWrap
		sub.w	d1,FromMinX(pc)		; Unwrap it
		sub.w	d1,FromMaxX(pc)
		bra.s	XWrap
No_XWrap:
		move.w	ow_Height(a5),d1	; Get height...
YWrap:		cmp.w	FromMinY(pc),d1		; Check it...
		bgt.s	No_YWrap
		sub.w	d1,FromMinY(pc)		; Unwrap it
		sub.w	d1,FromMaxY(pc)
		bra.s	YWrap
No_YWrap:
		; Ok, we know that the source is now starting within our bitmap
		; so lets generate some of the coordinates:

		move.w	ToMinX(pc),d2		; Get destination X
		move.w	ToMinY(pc),d3		; Get destination Y

		move.w	FromMaxX(pc),d4		; Get MaxX source
		cmp.w	ow_Width(a5),d4		; Do we fit?
		blt.s	X_Ok			; If so, skip this
		move.w	ow_Width(a5),d4		; Get width...
		subq.w	#1,d4			; We are really one less...

X_Ok:		move.w	FromMinX(pc),d0		; Get source X
		sub.w	d0,d4
		addq.w	#1,d4			; Size of X blit...
		add.w	d4,FromMinX(pc)		; Adjust X start
		add.w	d4,ToMinX(pc)

		move.w	FromMaxY(pc),d5		; Get MaxY source
		cmp.w	ow_Height(a5),d5	; Check if we fit
		blt.s	Y_Ok			; If so, skip this...
		move.w	ow_Height(a5),d5	; Get max height
		subq.w	#1,d5

Y_Ok:		move.w	FromMinY(pc),d1		; Get source Y
		sub.w	d1,d5
		addq.w	#1,d5			; Size of Y blit...

		; Ok, so set up the last few values:
		move.l	a2,a1			; Destination bitmap
		move.l	ow_BitMap(a5),a0	; Source bitmap
		move.l	#$C0,d6			; Copy MinTerm
		moveq.l	#-1,d7			; All bit planes...
		CALL	BltBitMap		; Do the blit...

		; Ok, now check if we are done in X...
		move.w	FromMaxX(pc),d0		; Get Max X
		sub.w	FromMinX(pc),d0		; Subtract Min
		bge	Do_Wrap			; If not done, do again...

		; Restore X...
		move.w	SaveMinX(pc),FromMinX(pc)
		move.w	SaveMaxX(pc),FromMaxX(pc)
		move.w	SaveMinX1(pc),ToMinX(pc)

		; Now, check the Y...

		move.w	FromMaxY(pc),d5		; Get MaxY source
		cmp.w	ow_Height(a5),d5	; Check if we fit
		blt.s	RepBlit_Done		; If so, we are done...

		; Oh boy, we need to adjust...

		move.w	ow_Height(a5),d5	; Get max height
		subq.w	#1,d5
		sub.w	FromMinY(pc),d5
		addq.w	#1,d5			; Size of Y blit...
		add.w	d5,FromMinY(pc)
		add.w	d5,ToMinY(pc)		; Adjust the Y values...
		bra	Do_Wrap			; and go again...

RepBlit_Done:	move.l	(sp)+,a2		; Restore
		bra.s	HookDone

;---------------------------------------------------------------------------

SimpleCase:
		; No bitmap, so just do the simple (0) minterm case...

		moveq.l	#0,d2			; Clear d2
		move.w	ra_MinX(a1),d2		; Get X pos

		moveq.l	#0,d3
		move.w	ra_MinY(a1),d3		; Get Y pos

		moveq.l	#0,d4
		move.w	ra_MaxX(a1),d4
		sub.l	d2,d4
		addq.l	#1,d4			; Get X size

		moveq.l	#0,d5
		move.w	ra_MaxY(a1),d5
		sub.l	d3,d5
		addq.l	#1,d5			; Get Y size

		move.l	d2,d0			; X Source
		move.l	d3,d1			; Y Source
		moveq.l	#0,d6			; NULL minterm
		moveq.l	#-1,d7			; FF mask

		move.l	rp_BitMap(a2),a1	; Get bitmap
		move.l	a1,a0
		CALL	BltBitMap		; Do the backfill-0

HookDone:	move.l	a5,a0			; get semaphore
		move.l	_SysBase,a6		; Get ExecBase
		CALL	ReleaseSemaphore	; Release the semaphore
		movem.l	(sp)+,d2-d7/a5/a6	; Restore
		rts

;---------------------------------------------------------------------------

ToMinX:		dc.w	0
ToMinY:		dc.w	0

FromMinX:	dc.w	0
FromMinY:	dc.w	0
FromMaxX:	dc.w	0
FromMaxY:	dc.w	0

SaveMinX:	dc.w	0
SaveMaxX:	dc.w	0
SaveMinX1:	dc.w	0
