**
**	$Id: clip.asm,v 36.47 92/03/24 16:25:18 darren Exp $
**
**      manipulate a console clip
**
**      (C) Copyright 1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE		"cddata.i"

	INCLUDE		"intuition/intuition.i"
	INCLUDE		"utility/hooks.i"

	INCLUDE		"debug.i"

**	Exports

	XDEF	SelectAbort
	XDEF	SelectClip


**	Imports

	XLVO	AllocVec		; Exec
	XLVO	FreeVec			;
	XLVO	ObtainSemaphore		;
	XLVO	ReleaseSemaphore	;

	XLVO	PeekQualifier		; Input

	XREF	LockDRPort
	XREF	UnLockRPort

	XREF	UpdateHighlight


**	Assumptions

	IFNE	(CMAB_SELECTED/8)-1
	FAIL	"CMAB_SELECTED not in high byte, recode"
	ENDC
	IFNE	cd_SelectedAnchorX-cd_SelectedAnchorY+2
	FAIL	"cd_SelectedAnchorY does not follow cd_SelectedAnchorX, recode"
	ENDC
	IFNE	cd_SelectedTailX-cd_SelectedTailY+2
	FAIL	"cd_SelectedTailY does not follow cd_SelectedTailX, recode"
	ENDC
	IFNE	cd_SelectedAnchorX-cd_SelectedTailX+4
	FAIL	"cd_SelectedTail does not follow cd_SelectedAnchor, recode"
	ENDC
	IFNE	(CMAB_IMPLICITNL/8)-1
	FAIL	"CMAB_IMPLICITNL not in high byte, recode"
	ENDC


*------ SelectAbort --------------------------------------------------
SelectAbort:

		bclr	#CUB_SELECTED,cu_Flags(a2)
		beq.s	saDone		;.s!

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)
		clr.l	cd_SelectedUnit(a6)
		bclr	#CUB_CURSSELECT,cu_CursorFlags(a2)

saChkBuffered:
		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	saDone		;.s!

		;-- clear CMAB_SELECTED in display attributes
		move.l	d2,-(a7)
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.w	cu_DisplayXL(a2),d0
		move.w	cu_DisplayYL(a2),d1
		move.w	d1,d2		; adjust a0 to end of display
		addq.w	#1,d2		;
		lsl.w	#2,d2		;
		add.w	d2,a0		;

		move.w	cu_CM+cm_DisplayWidth(a2),d2

saClearYLoop:
		move.l	-(a0),a1
		add.l	a1,a1
		move	#$4,ccr		; zero

		bra.s	saClearXDBF
saClearXLoop:
		bclr	#CMAB_SELECTED&7,(a1)
		addq.l	#2,a1
saClearXDBF:
		dbne	d0,saClearXLoop
		bne.s	saSetXDBF

		move.w	d2,d0
saClearYDBF:
		dbf	d1,saClearYLoop

		bra.s	saHighlightDone	; nothing set, no need for highlight


saSetYLoop:
		move.l	-(a0),a1
		add.l	a1,a1
		bra.s	saSetXDBF

saSetXLoop:
		bclr	#CMAB_SELECTED&7,(a1)
		addq.l	#2,a1
saSetXDBF:
		dbf	d0,saSetXLoop

		move.w	d2,d0
saSetYDBF:
		dbf	d1,saSetYLoop

		bsr	UpdateHighlight

saHighlightDone:
		move.l	(a7)+,d2

saDone:
		bclr	#CDIB_COPY,cd_InputFlags(a6)
		rts


*------ SelectClip ---------------------------------------------------
*
*   cd_SelectFlags SELECTDOWN tells that the select button is down
*   cd_SelectFlags EXTENDED tells that the select button is extended
*   cd_SelectFlags CIRCLING tells that drag select criteria not yet
*	established, and neither has origin of that criteria
*   cd_SelectFlags DRAGGING tells that drag select criteria not yet
*	established
*   cd_SelectFlags SELECTING tells that selection anchor found
*   cd_SelectFlags TICKING tells whether the timer is in use to
*	tick for action
*   cd_SelectedUnit and cu_Flags SELECTED tells which unit has a
*	highlighted area
*
*   called w/ shared lock on USemaphore, so cd_Active won't go away
*
SelectClip:
		movem.l	d2-d7/a2-a5,-(a7)

		move.l	cd_Active(a6),d0
		beq	scDone

		move.l	cd_SelectedUnit(a6),d1	;new selection unit
		beq.s	sc_sameone

		cmp.l	d1,d0			;if same one as before?
		beq.s	sc_sameone

		btst	#CDSB_SELECTDOWN,cd_SelectFlags(a6)
		beq	scDone

		movea.l	d1,a2			;else clear old, and exit
						;this time.
		bsr	scClearSelect
		bra	scDone

sc_sameone:
		movea.l	d0,a2			;clip started
						;or continued

		btst	#CDSB_SELECTDOWN,cd_SelectFlags(a6)
		beq	scUp

		;-- verify the current mouse SELECTDOWN state

		move.w	cd_MouseQual(a6),d0
		
		;button down if left mouse button, or
		;left ALT, and left AMIGA keys are down

		bclr	#IEQUALIFIERB_LEFTBUTTON,d0
		bne.s	scDown

		;@@@ -- note that this is hard coded for now,
		;    -- and therefore cannot be remapped.
		;

		and.w	#IEQUALIFIER_LALT!IEQUALIFIER_LCOMMAND,d0
		cmp.w	#IEQUALIFIER_LALT!IEQUALIFIER_LCOMMAND,d0
		bne	scUp

		;-- get mouse position and convert to display position
scDown:
		move.l	cu_Window(a2),a0

		;-- grab mouse position atomically!
		;-- THIS IS AN ATOMIC OPERATION - LEAVE ME

		move.l	wd_MouseY(a0),d0

		;MouseY is first, then MouseX in Window for sorting purposes
		move.w	d0,cd_MouseX(a6)
		swap.w	d0
		move.w	d0,cd_MouseY(a6)

		moveq	#0,d1		; show nothing clipped to bounds
		;--	get x position
		moveq	#0,d2

*		move.w	wd_MouseX(a0),d2
		move.w	cd_MouseX(a6),d2

		sub.w	cu_XROrigin(a2),d2
		bpl.s	scGenXCP
		moveq	#0,d2
		moveq	#1,d1
scGenXCP:
		divu	cu_XRSize(a2),d2
		;--	get y position
		moveq	#0,d3

*		move.w	wd_MouseY(a0),d3
		move.w	cd_MouseY(a6),d3

		sub.w	cu_YROrigin(a2),d3
		bpl.s	scGenYCP
		moveq	#0,d3
		moveq	#1,d1
scGenYCP:
		divu	cu_YRSize(a2),d3
		;--	bound y position


		move.w	cu_XMax(a2),d0

		cmp.w	cu_DisplayYL(a2),d3
		ble.s	scBoundX
		moveq	#1,d1
		move.w	cu_DisplayXL(a2),d2
		move.w	cu_DisplayYL(a2),d3
		;--	bound x position
scBoundX:
		cmp.w	d0,d2
		ble.s	scBounded	;!!! was 'ble'
					;DisplayWidth == cu_XMax + 1
		moveq	#1,d1

		;-- limit drag selection to x bounds
		move.w	d0,d2		;was move.w d0,d2 - overflow

		;--	check if already dragging or selecting
scBounded:
		;-- check what's happening now
		move.b	cd_SelectFlags(a6),d0
		and.b	#CDSF_DRAGGING!CDSF_SELECTING,d0
		bne.s	scActive

		tst	d1
		beq.s	scDragging
		;--	nothing happening, but outside drag anchor limit
		bset	#CDSB_CIRCLING,cd_SelectFlags(a6)
sc2Done:
		bra	scDone

scDragging:
		;--	nothing happening, start to drag
		bset	#CDSB_DRAGGING,cd_SelectFlags(a6)

*		move.w	wd_MouseX(a0),cd_SelectDownMX(a6)
*		move.w	wd_MouseY(a0),cd_SelectDownMY(a6)
		move.l	cd_MouseX(a6),cd_SelectDownMX(a6)	;and Y

		movem.w	d2/d3,cd_SelectDownCPX(a6)
		bra.s	sc2Done

scActive:
		btst	#CDSB_SELECTING,cd_SelectFlags(a6)
		bne	scOld

		;--	look for drag criteria
*		move.w	wd_MouseX(a0),d0
		move.w	cd_MouseX(a6),d0

		sub.w	cd_SelectDownMX(a6),d0
		bpl.s	scnPosXDiff
		neg.w	d0
scnPosXDiff:
		move.w	d0,d1
		add.w	d0,d0
		add.w	d0,d0
		cmp.w	cu_XRSize(a2),d0
		bge.s	scnSelecting

*		move.w	wd_MouseY(a0),d0
		move.w	cd_MouseY(a6),d0

		sub.w	cd_SelectDownMY(a6),d0
		bpl.s	scnPosYDiff
		neg.w	d0
scnPosYDiff:
		move.w	d0,d1
		add.w	d0,d0
		add.w	d0,d0
		cmp.w	cu_YRSize(a2),d0
		blt.s	sc2Done

scnSelecting:
		;-- recover original select down origin
		movem.w	cd_SelectDownCPX(a6),d2/d3

		bset	#CDSB_SELECTING,cd_SelectFlags(a6)

		;-- is this selection new, or extended?

		tst.l	cd_SelectedUnit(a6)
		beq.s	scnNewAnchor

		btst	#CDSB_EXTENDED,cd_SelectFlags(a6)
		bne.s	scExtended

scnNew:
		;-- unselect the previously selected area

		bsr	LockDRPort
		move.b	cd_SelectFlags(a6),d4
		bsr	SelectAbort
		move.b	d4,cd_SelectFlags(a6)
		bsr	UnLockRPort

scnNewAnchor:
		;-- select the mouse position
		move.l	a2,cd_SelectedUnit(a6)
		bset	#CUB_SELECTED,cu_Flags(a2)

		bsr	LockDRPort

		movem.w	d2/d3,cd_SelectedAnchorX(a6)
		movem.w	d2/d3,cd_SelectedTailX(a6)
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		lsl.w	#2,d3
		move.l	0(a0,d3.w),a0
		add.w	d2,a0
		add.l	a0,a0

		;some code to fix that half char artifact

		tst.w	d2		;if first column, always start
		beq.s	scStartSelect
		tst.w	(a0)
		bmi.s	scStartSelect	;if a valid character, start select
		tst.w	-2(a0)
		bmi.s	scStartSelect
		bset	#CDSB_CIRCLING,cd_SelectFlags(a6)
		bra.s	scStartCircle

scStartSelect:
		bset	#CMAB_SELECTED&7,(a0)
scStartCircle:

		bsr	UpdateHighlight

		bsr	UnLockRPort
		bra	scDone

		;-- selection intermediate action
scOld:
		bsr	LockDRPort
		bra.s	scContinue


scExtended:
		bsr	LockDRPort
		;-- compare current w/ direction of anchor to tail
		move.w	cd_SelectedTailY(a6),d0
		cmp.w	cd_SelectedAnchorY(a6),d0
		bgt.s	scForward
		blt.s	scReverse
		move.w	cd_SelectedTailX(a6),d0
		cmp.w	cd_SelectedAnchorX(a6),d0
		bgt.s	scForward
		blt.s	scReverse
		bra.s	scContinue

		;-- check if current is before anchor
scForward:
		cmp.w	cd_SelectedAnchorY(a6),d3
		bgt.s	scContinue
		blt.s	scReversal
		cmp.w	cd_SelectedAnchorX(a6),d2
		bge.s	scContinue
		bra.s	scReversal

		;-- check if current is after anchor
scReverse:
		cmp.w	cd_SelectedAnchorY(a6),d3
		blt.s	scContinue
		bgt.s	scReversal
		cmp.w	cd_SelectedAnchorX(a6),d2
		ble.s	scContinue

scReversal:
		;-- exchange anchor and tail
		move.l	cd_SelectedAnchorX(a6),d0
		move.l	cd_SelectedTailX(a6),cd_SelectedAnchorX(a6)
		move.l	d0,cd_SelectedTailX(a6)

		;-- set new selection extent
scContinue:
		;-- clear currently selected extent
		;--	check for change in tail
		movem.w	cd_SelectedTailX(a6),d0/d1
		cmp.w	d0,d2
		bne.s	sccGetAnchor
		cmp.w	d1,d3
		beq	scUnlock	; no change in tail

sccGetAnchor:
		movem.w	cd_SelectedAnchorX(a6),d4/d5

		;--	check which is first, tail or anchor
		cmp.w	d1,d5
		bgt.s	scClear
		blt.s	sccSwap
		cmp.w	d0,d4
		bge.s	scClear
sccSwap:
		exg	d0,d4
		exg	d1,d5

		;--	clear d0/d1 to d4/d5
scClear:
		sub.w	d1,d5

		move.w	cu_XMax(a2),d6	; cm_DisplayWidth-1

		move.l	cu_CM+cm_AttrDispLines(a2),a4
		lsl.w	#2,d1
		add.w	d1,a4

sccYLoop:
		move.l	(a4)+,a3
		add.w	d0,a3
		add.l	a3,a3
		tst.w	d5
		bne.s	sccHaveMaxX
		move.w	d4,d6
sccHaveMaxX:
		move.w	d6,d1
		sub.w	d0,d1

sccXLoop:
		bclr	#CMAB_SELECTED&7,(a3)
		addq.l	#2,a3
		dbf	d1,sccXLoop

		moveq	#0,d0

sccYDBF:
		dbf	d5,sccYLoop


		;--	set newly selected extent
		movem.w	cd_SelectedAnchorX(a6),d4/d5
		movem.w	d2/d3,cd_SelectedTailX(a6)

		;--	check which is first, new tail or anchor
		cmp.w	d3,d5
		bgt.s	scSet
		blt.s	scsSwap
		cmp.w	d2,d4
		bge.s	scSet
scsSwap:
		exg	d2,d4
		exg	d3,d5

		;--	set d2/d3 to d4/d5
scSet:
		cmp.w	cu_YCCP(a2),d3
		bgt.s	scsCursOutSpan	; cursor < span
		bne.s	scsChkCursYHi
		cmp.w	cu_XCCP(a2),d2
		bgt.s	scsCursOutSpan	; cursor < span
scsChkCursYHi:
		cmp.w	cu_YCCP(a2),d5
		blt.s	scsCursOutSpan	; cursor > span
		bne.s	scsCursInSpan
		cmp.w	cu_XCCP(a2),d4
		blt.s	scsCursOutSpan	; cursor > span
scsCursInSpan:
		bset	#CUB_CURSSELECT,cu_CursorFlags(a2)
		bra.s	scsChkNRender


scsCursOutSpan:
		bclr	#CUB_CURSSELECT,cu_CursorFlags(a2)

scsChkNRender:	move.w	d3,d0
	
		moveq	#$FFFFFFFF,d3	;bit pattern - all 1's

		cmp.w	cu_DisplayYL(a2),d5
		bne.s	scsRender	;not last line

		cmp.w	cu_DisplayXL(a2),d4
		blt.s	scsRender	;if less than 1st invalid X
					;position, its renderable

		;-- turn off rendered, and highlight bit of 1st
		;-- invalid character in buffer.

		move.w	#(~(CMAF_RENDERED!CMAF_HIGHLIGHT))&$FFFF,d3
scsRender:

		sub.w	d0,d5

		move.w	cu_XMax(a2),d6	; cm_DisplayWidth-1

		moveq	#0,d7		; assume initially not prior rendered
		tst.w	d2
		bne.s	scsHavePrior
		move.w	#CMAF_SELECTED,d7 ; ensure first char on line selected
scsHavePrior:
		move.l	cu_CM+cm_AttrDispLines(a2),a4
		lsl.w	#2,d0
		add.w	d0,a4

scsYLoop:
		move.l	(a4)+,a3
		add.w	d2,a3
		add.l	a3,a3
		tst.w	d5
		bne.s	scsHaveMaxX
		move.w	d4,d6
scsHaveMaxX:
		move.w	d6,d1
		sub.w	d2,d1

scsXLoop:
		move.w	(a3),d0
		and.w	#CMAF_RENDERED,d0
		lsr.w	#CMAB_RENDERED-CMAB_SELECTED,d0
		or.w	d0,d7
		or.w	d7,(a3)+
		move.w	d0,d7
		dbf	d1,scsXLoop

		moveq	#0,d2
		move.w	#CMAF_SELECTED,d7 ; ensure first char on line selected

* scsYDBF:
		dbf	d5,scsYLoop
		and.w	d3,-(a3)	;clears SELECT & HIGHLIGHT bits
					;if outside range of valid
					;x/y characters


		;--	update highlight
		bsr	UpdateHighlight

scUnlock:
		bsr	UnLockRPort

scDone:
		bclr	#CDIB_COPY,cd_InputFlags(a6)

		movem.l	(a7)+,d2-d7/a2-a5
		rts


*********************************************************************
*
* This bit of code use to be called when the user drag selected,
* and let up on the mouse button.  It still is, but
*
* It is now recalled when the user presses RIGHT AMIGA C.  The key
* press is caught by the inputhandler which signals the console.device
* task.  A bit is also set in a new field in console.device base.
*
* This new field is used now, and will be used in the future as
* a means of indicating the user pressed a key(s) which are trapped
* by the console device input handler as having some special
* meaning.
*
* Remember that we DONT want the input handler doing much work, so
* it is correct to signal the task to do anything which is time
* consuming.
*
* In this case the user selects a piece of text, and verifies it
* by pressing RIGHT AMIGA C.  The COPY bit in cd_InputFlags
* is immediately cleared if all the conditions in the following
* code are not met.  Any such bits in this field must be cleared
* immediately - so that they don't hang around being left as
* TRUE long after the keypress, or other such event occured.
*

scUp:
		;-- check if any selection in progress
TEMPMASK	EQU	(~(CDSF_CIRCLING!CDSF_DRAGGING!CDSF_SELECTDOWN))&$ff
		and.b	#TEMPMASK,cd_SelectFlags(a6)

************************************************************
* No longer care if we are selecting - may copy later
*		btst	#CDSB_SELECTING,cd_SelectFlags(a6)
*		beq.s	scDone

		;-- make sure it's the active unit
		btst	#CUB_SELECTED,cu_Flags(a2)
		beq.s	scDone

		;-- make sure its the selected unit
		movea.l	cd_SelectedUnit(a6),a0
		cmpa.l	a0,a2
		bne.s	scDone

		;-- make sure the user pressed Right Amiga C
		
		btst	#CDIB_COPY,cd_InputFlags(a6)
		beq.s	scDone

		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		;-- clear out old selection
		move.l	cd_SelectionSnip(a6),d0
		beq.s	scuNoOldClip
		move.l	d0,a1
		subq.w	#1,snip_Access(a1)
		bpl.s	scuNoOldClip
		LINKEXE	FreeVec

scuNoOldClip:
	    ;-- first implementation: store chars, tabs, and line terminators
		movem.w	cd_SelectedAnchorX(a6),d2/d3/d4/d5
		cmp.w	d3,d5
		bgt.s	scuOrdered
		blt.s	scuSwap
		cmp.w	d2,d4
		bge.s	scuOrdered
scuSwap:
		exg	d2,d4
		exg	d3,d5


		;--	get memory for clip
scuOrdered:
		move.w	d5,d0
		sub.w	d3,d0
		addq.w	#1,d0			; for each line, full width
		move.w	cu_CM+cm_DisplayWidth(a2),d1
		addq.w	#1,d1			; linefeeds may occur
		mulu	d1,d0
		sub.w	d2,d0			; unused chars on first line
		sub.w	d4,d1			; unused chars on last line
		sub.w	d1,d0			;   (d1 was width+1, and
		add.w	#3+snip_Data,d0		;   bounds are inclusive,
						;   and null terminated)
		moveq	#0,d1
		LINKEXE	AllocVec
		move.l	d0,cd_SelectionSnip(a6)
		beq	scFailClipData
		move.l	d0,a5
		clr.w	snip_Access(a5)
		addq.l	#snip_Data,a5

		;-- copy from d2/d3 to d4/d5
		sub.w	d3,d5		; # of lines -1 for dbf loop

		move.w	cu_XMax(a2),d6	; cm_DisplayWidth-1


		move.l	cu_CM+cm_AttrDispLines(a2),a4
		lsl.w	#2,d3
		add.w	d3,a4
		moveq	#0,d3		; show first line

scuYLoop:
		move.l	(a4)+,a3
		add.w	d2,a3
		move.l	a3,a0
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a3,a3		; A3 = attribute map, A0 = char map
		tst.w	d3
		beq.s	scuStartLine
		move.w	(a3),d0		; get attribute
		bpl.s	scuLineSeperator ; not renderable, put in seperator now
		and.w	#CMAF_IMPLICITNL,d0
		bne.s	scuStartLine	; implicit newline, no seperator
scuLineSeperator:
		move.b	#$0a,(a5)+	; insert explicit line seperator
scuStartLine:
		tst.w	d5		; for last line, special case
		bne.s	scuHaveMaxX
		move.w	d4,d6		; max X is in D4
scuHaveMaxX:
		move.w	d6,d1		; calc length of last line
		sub.w	d2,d1		; length of line -1 for dbf loop
		move.w	d1,d0

scuXLoop:
		;check for these bits:
		; RENDERED
		; bogus character at end of map is cleared above

		tst.w	(a3)+
		bpl.s	scuYNext	;quick check for CMAF_RENDERED
					;assumes CMAF_RENDERED is high bit

		move.b	(a0)+,(a5)+
		dbf	d1,scuXLoop

scuYNext:
		moveq	#0,d2
		moveq	#1,d3		; not first line

scuYDBF:
		dbf	d5,scuYLoop

		;-- check what was ended on
		cmp.w	d0,d1		; already got NL at scuLineSeperator?
		beq.s	scuDone		;   yes
		addq.w	#1,d1		; cmp.w #-1,d1
		beq.s	scuDone		; ended on renderable character

		;--	insert a line terminator
		move.b	#$0a,(a5)+	; add newline if ended on
					; bogus character

		;-- determine selection text length
scuDone:
		clr.b	(a5)		; null termination not in actual
		move.l	cd_SelectionSnip(a6),a0
		sub.l	a0,a5
		subq.l	#snip_Data,a5
		move.w	a5,snip_Length(a0)

		pea	snip_Data(a0)
		move.l	a5,-(a7)
scuCallHooks:
		clr.l	-(a7)		; type zero message
		move.l	cd_SelectionHooks(a6),a5
scuHookLoop:
		move.l	(a5),d0
		beq.s	scuHooksDone
		move.l	a5,a0
		move.l	d0,a5
		move.l	a7,a1
		pea	scuHookLoop(pc)		; jsr   (h_Entry(a1))
		move.l	h_Entry(a0),-(a7)	; bra.s scuHookLoop
		rts				;

scuHooksDone:
		lea	12(a7),a7
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

		bsr.s	scClearSelect
		bra	scDone

scFailClipData:
		clr.l	-(a7)		; null buffer
		clr.l	-(a7)		; zero actual
		bra.s	scuCallHooks

scClearSelect:
		bsr	LockDRPort
		bsr	SelectAbort
		bsr	UnLockRPort
		rts
	END
