**************************************************************************
*
*  downcode.asm - contains downcoded Intuition library functions,
*                 and some stubs that we use to call ourselves
*
*  $Id: downcode.asm,v 38.11 92/12/09 18:11:28 peter Exp $
*
**************************************************************************

	INCLUDE 'ibase.i'
	INCLUDE 'intuall.i'

	xdef	viewportaddress
	xdef	reportmouse
	xdef	_openWindowOnlyTags
	xdef	_openScreenOnlyTags

	xdef	_LOCKSTATE
	xdef	_LOCKGADGETS
	xdef	_LOCKIBASE
	xdef	_LOCKVIEW
	xdef	_LOCKVIEWCPR
	xdef	_LOCKRP
	xdef	_UNLOCKSTATE
	xdef	_UNLOCKGADGETS
	xdef	_UNLOCKIBASE
	xdef	_UNLOCKVIEW
	xdef	_UNLOCKVIEWCPR
	xdef	_UNLOCKRP

* divMod( ULONG dividend, ULONG divisor, ULONG *remainderptr )
*
* Unsigned-divides dividend by divisor, returning result in D0, and
* storing longword remainder in address pointed at by remainderptr.

	xdef	_divMod
	xref	_UtilityBaseOffset
	xref	_LVOUDivMod32

_divMod:	
	movem.l	4(sp),d0-d1		; set up dividend and divisor
	move.l	_UtilityBaseOffset(a6),a0 ; utility math takes libbase anywhere
	jsr	_LVOUDivMod32(a0)
	move.l	12(sp),a0		; pointer to remainder storage
	move.l	d1,(a0)			; store remainder
	rts

*
* Greenhills ulmult(d0,d1) = d0*d1
* Greenhills lmult(d0,d1) = d0*d1
*
	xdef	lmult
	xdef	ulmult
	xref	_LVOUMult32
lmult:
ulmult:
	move.l	_UtilityBaseOffset(a6),a0 ; utility math takes libbase anywhere
	jmp	_LVOUMult32(a0)		; do it	

*
* Greenhills uldivt(d0,d1) = d0/d1
*
	xdef	uldivt
	xref	_LVOUDivMod32
uldivt:
	move.l	_UtilityBaseOffset(a6),a0 ; utility math takes libbase anywhere
	jmp	_LVOUDivMod32(a0)	; do it

*
* Greenhills ldivt(d0,d1) = d0/d1
*
	xdef	ldivt
	xref	_LVOSDivMod32
ldivt:
	move.l	_UtilityBaseOffset(a6),a0 ; utility math takes libbase anywhere
	jmp	_LVOSDivMod32(a0)	; do it

*
* Greenhills lmodt(d0,d1) = d0%d1
*
	xdef	lmodt
	xref	_LVOSDivMod32
lmodt:
	move.l	_UtilityBaseOffset(a6),a0 ; utility math takes libbase anywhere
	jsr	_LVOSDivMod32(a0)	; do it
	move.l	d1,d0			; return the remainder
	rts

**
** The stub that Graphics calls when our pointer is killed for
** compatibility with other sprites.
**

	xdef	_pointerKill
	xref	_notePointerKill

_pointerKill:
	move.l	a1,-(sp)
	jsr	_notePointerKill	; C routine that notes the fact
	addq	#4,sp
	rts

**
** The intuition/ReportMouse() function:
**

reportmouse
	tst.l	d0
	beq.s	rm_OFF
	ori.w	#REPORTMOUSE,wd_Flags+2(a0)
	rts
rm_OFF	andi.w	#~REPORTMOUSE,wd_Flags+2(a0)
	rts


**
** The intuition/ViewPortAddress() function:
**

viewportaddress
	move.l	wd_WScreen(a0),a0
	lea.l	sc_ViewPort(a0),a0
	move.l	a0,d0
	rts

**
** Varargs conversion stub that calls OpenWindowTagList(), with a NULL NewWindow
**
		xref	_LVOOpenWindowTagList

_openWindowOnlyTags:
		lea	4(a7),a1	; TagList becomes pointer to tag
		move.l	#0,a0		; NULL NewWindow
		jsr	_LVOOpenWindowTagList(a6)
		rts

**
** Varargs conversion stub that calls OpenScreenTagList(), with a NULL NewScreen
**
		xref	_LVOOpenScreenTagList

_openScreenOnlyTags:
		lea	4(a7),a1
		move.l	#0,a0
		jsr	_LVOOpenScreenTagList(a6)
		rts

**
** BorderPatrol() is now a stub which invokes the real function via
** stackSwap().  Here's why:
**
** BorderPatrol() can get invoked by any innocent bystander.  Say your
** application closes its window.  BorderPatrol() is invoked on that
** application's task to repair damage underneath the window.  Say the
** damaged area includes some complex boopsi object.  Now this simple
** application has to supply stack for the boopsi refresh code.
**

		xdef	_BorderPatrol
		xref	_internalBorderPatrol
		xref	_stackSwap
_BorderPatrol:
	move.l		4(sp),a0	; get the screen parameter off the stack
	move.l		a0,-(sp)	; put it back in a different place
	pea		_internalBorderPatrol	; stackSwap is to call this guy
	jsr		_stackSwap
	addq.l		#4,sp
	rts

* Peter 18-Mar-92: Downcoded most of the lock stuff to save space
* The individual lock functions lock the named lock.  No return code.

		xref	_lockIBase

_LOCKSTATE:
		moveq	#ISTATELOCK,d0
		bra.s	lock_d0
_LOCKGADGETS:
		moveq	#GADGETSLOCK,d0
		bra.s	lock_d0
_LOCKIBASE:
		moveq	#IBASELOCK,d0
		bra.s	lock_d0
_LOCKVIEW:
		moveq	#VIEWLOCK,d0
		bra.s	lock_d0
_LOCKVIEWCPR:
		moveq	#VIEWCPRLOCK,d0
		bra.s	lock_d0
_LOCKRP:
		moveq	#RPLOCK,d0
		;fall through to lock_d0

* lock_d0 locks the IBase lock specified by the number in d0.
* d0 is preserved.

lock_d0:
		move.l	d0,-(a7)
		jsr	_lockIBase
;		addq	#4,a7
		move.l	(a7)+,d0
		rts

		xref	_unlockIBase

_UNLOCKSTATE:
		moveq	#ISTATELOCK,d0
		bra.s	unlock_d0
_UNLOCKGADGETS:
		moveq	#GADGETSLOCK,d0
		bra.s	unlock_d0
_UNLOCKIBASE:
		moveq	#IBASELOCK,d0
		bra.s	unlock_d0
_UNLOCKVIEW:
		moveq	#VIEWLOCK,d0
		bra.s	unlock_d0
_UNLOCKVIEWCPR:
		moveq	#VIEWCPRLOCK,d0
		bra.s	unlock_d0
_UNLOCKRP:
		moveq	#RPLOCK,d0
		;fall through to unlock_d0

* unlock_d0 unlocks the IBase lock specified by the number in d0.
* d0 is preserved.

unlock_d0:
		move.l	d0,-(a7)
		jsr	_unlockIBase
;		addq	#4,a7
		move.l	(a7)+,d0
		rts


* Downcoded the external LockIBase() and UnlockIBase() functions.
* They arrive here with the lock-number in D0.  LockIBase() must
* return that lock number.

		XDEF	lockibase;
		XDEF	unlockibase;

; d0 contains lock number
; If lock number is zero, lock both ISTATELOCK (#0) and IBASELOCK (#4)

lockibase:
		bsr.s	lock_d0		; lock the requested lock (preserves D0)
		tst.l	d0
		bne.s	lib_notibase	; This wasn't LockIBase(0)...
		bsr.s	_LOCKIBASE	; If d0 is zero, also lock IBASELOCK
		moveq	#0,d0		; restore zero to d0
lib_notibase:	rts


; a0 contains lock number
; If lock number is zero, unlock both IBASELOCK (#4) and ISTATELOCK (#0).

unlockibase:
		move.l	a0,d0		; Get lock in d0, and test Z flag
		bne.s	ulib_notibase	; This wasn't UnlockIBase(0)...

		bsr.s	_UNLOCKIBASE	; If d0 is zero, also unlock IBASELOCK
		moveq	#0,d0		; restore zero to d0

ulib_notibase:	bsr.s	unlock_d0	; unlock the requested lock (preserves D0)
		rts




* This file contains a number of replacement functions for Intuition
* that used to be in C and now are in assembly (since that made it smaller)
*
* The function headers define where the functions came from and
* what they do (as best as possible since many of the functions did not
* explain this to start out with)
*
*******************************************************************************
*
	NOLIST
		INCLUDE	'graphics/gfx.i'	; For Rectangle
		INCLUDE	'intuition/intuition.i'	; For Window structure
	LIST
*
*******************************************************************************
*
* The routines below are from misc.c
*
*******************************************************************************
*
* d0=iabs(int a)
*
		xdef	_iabs
_iabs:
		move.l	4(sp),d0	; Get argument
		bpl.s	iabs_rts	; If not negative, skip...
		neg.l	d0		; a=-a
iabs_rts:	rts
*
* d0=imax(int a, int b)
*
		xdef	_imax
_imax:
		movem.l	4(sp),d0-d1	; Get a and b
		cmp.l	d0,d1		; Check them...
		ble.s	imax_rts	; If a is larger, skip...
		move.l	d1,d0		; move b into d0
imax_rts:	rts
*
* d0=imin(int a, int b)
*
		xdef	_imin
_imin:
		movem.l	4(sp),d0-d1	; Get a and b
		cmp.l	d0,d1		; Check them...
		bge.s	imin_rts	; If a is smaller, skip...
		move.l	d1,d0		; move b into d0
imin_rts:	rts
*
*******************************************************************************
*
* jstreq - Check for exact string match
* d0=jstreq(BYTE *a, BYTE *b)
*
	xdef	_jstreq
_jstreq:
		movem.l	4(sp),a0-a1		; Get two string pointers
streq_loop:	move.b	(a0)+,d0		; Get next char
		beq.s	streq_eos		; End of string?
		cmp.b	(a1)+,d0		; Does it match?
		beq.s	streq_loop		; Keep looping if so
streq_bad:	moveq.l	#0,d0			; Return FALSE (no match)
		rts
streq_eos:	cmp.b	(a1),d0			; End the same?
		bne.s	streq_bad		; If not, return FALSE
		moveq.l	#1,d0			; Return TRUE
		rts
*
* jstrncpy - Size limited string copy
* void jstrncpy(char *dest, char *src, int num)
*
	xdef	_jstrncpy
_jstrncpy:
		movem.l	4(sp),a0-a1		; Get destination & source
		move.l	12(sp),d0		; Get max chars
		subq.l	#1,d0			; One less than given...
stccpy_loop:	move.b	(a1)+,(a0)+		; Copy the data...
		dbeq	d0,stccpy_loop		; Loop for max or null...
		clr.b	-(a0)			; Make sure NULL terminated
		rts
*
* d0=strlen(BYTE *text)
*
	xdef	_strlen
_strlen:
		move.l	4(sp),a0		; Get start...
		move.l	a0,d0			; Store it...
strlen_loop:	move.b	(a0)+,d1		; Check for NULL
		bne.s	strlen_loop		; Keep going...
		exg	a0,d0			; swap...
		sub.l	a0,d0			; subtract...
		subq.l	#1,d0			; Don't count the NULL
		rts
*
*******************************************************************************
*
*
*******************************************************************************
*
* The routines below are from rect.c   Only four routines are not implemented
* and they are not important any way...
*
*******************************************************************************
*
* rectWidth - Returns the width from a rectangle
* d0=rectWidth(struct Rectangle *r)
*
	xdef	_rectWidth
_rectWidth:
		move.l	4(sp),a0	; Get the rectangle...
		bra.s	recttobox	; Do common code
*
* rectHeight - Returns the height from a rectangle
* d0=rectHeight(struct Rectangle *r)
*
	xdef	_rectHeight
_rectHeight:
		move.l	4(sp),a0	; Get rectangle pointer
recttobox2:	addq.l	#2,a0		; Slide up to the Y co-ord
*
* Tricky optimization.  Since the rectangle is very much the same for
* both x and y, we make this code use X co-ords and just slide the box to
* when we look for the y...
*
recttobox:	moveq.l	#1,d0		; Clear d0 and set to 1...
		add.w	ra_MaxX(a0),d0	; Get Max
		sub.w	(a0),d0		; Subtract Min  ra_MinX(a0) == (a0)
		ext.l	d0		; Make 32-bit...
		rts
*
* rectToBox - Converts the rectangle into the box given
* void rectToBox(struct Rectangle *rect, struct IBox *box)
*
	xdef	_rectToBox
_rectToBox:
		movem.l	4(sp),a0-a1	; Rectangle in a0, box in a1
		move.l	(a0),(a1)+	; Copy MinX/MinY and bump pointer...
		bsr.s	recttobox	; Do above code
		move.w	d0,(a1)+	; Store Width
		bsr.s	recttobox2	; Do it for Y
		move.w	d0,(a1)+	; Store Height
		rts
*
*******************************************************************************
*
* boxRight - Returns the right side from a box (inverse of rectWidth)
* d0=boxRight(struct IBox *b)
*
	xdef	_boxRight
_boxRight:
		move.l	4(sp),a0	; Get the box...
		bra.s	boxtorect	; Do common code
*
* boxBottom - Returns the bottom side from a box (inverse of rectHeight)
* d0=boxBottom(struct IBox *b)
*
	xdef	_boxBottom
_boxBottom:
		move.l	4(sp),a0	; Get the box...
boxtorect2:	addq.l	#2,a0		; Slide up to the Y co-ord
*
* Tricky optimization.  Since the box is very much the same for
* both x and y, we make this code use X co-ords and just slide the box to
* when we look for the y...
*
boxtorect:	move.w	(a0),d0		; Get Min (top/left) ra_MinX(a0)==(a0)
		add.w	ra_MaxX(a0),d0	; Add width/height
		subq.w	#1,d0		; Subtract
		ext.l	d0		; Make into a 32-bit number...
		rts
*
* boxToRect - Converts the box into the rectangle given.  Ok if box and
* rect point at same thing.
* void boxToRect(struct IBox *box, struct Rectangle *rect)
*
	xdef	_boxToRect
_boxToRect:
		movem.l	4(sp),a0-a1	; Box in a0, rectangle in a1
		move.l	(a0),(a1)+	; Copy MinX/MinY and bump pointer...
		bsr.s	boxtorect	; Do above code
		move.w	d0,(a1)+	; Store MaxX
		bsr.s	boxtorect2	; Do it for Y
		move.w	d0,(a1)+	; Store MaxY
		rts
*
*******************************************************************************
*
* windowInnerBox - Get the inner window region relative to the window (fills in box)
* void windowInnerBox(struct Window *w,struct IBox *box)
*
	xdef	_windowInnerBox
_windowInnerBox:
		movem.l	4(sp),a0-a1	; Get window in a0, box in a1
*
		moveq.l	#0,d0		; Clear d0
		move.b	wd_BorderLeft(a0),d0
		move.w	d0,(a1)+	; Auto advance to the next one
*
		moveq.l	#0,d1		; Clear d1
		move.b	wd_BorderTop(a0),d1
		move.w	d1,(a1)+	; Auto advance to the next one
*
		move.w	wd_Width(a0),(a1)	; Get width
		sub.w	d0,(a1)			; Subtract BorderLeft
		move.b	wd_BorderRight(a0),d0	; Get border right
		sub.w	d0,(a1)+		; Subtract that too and advance
*
		move.w	wd_Height(a0),(a1)	; Get height
		sub.w	d1,(a1)			; Subtract BorderTop
		move.b	wd_BorderBottom(a0),d1	; Get border bottom
		sub.w	d1,(a1)			; Subtract and we are done
*
		rts
*
*******************************************************************************
*
* currMouse - Fills in the point structure based on the window co-ords
* d0=currMouse(struct Window *w,struct Point *point)
* New feature:  Since the point can be a single LONG, d0 contain the
* point on return...
*
	xdef	_currMouse
_currMouse:
		movem.l	4(sp),a0-a1	; Get window in a0, point in a1
		move.l	wd_MouseY(a0),d0	; Get mouse Y and X
		swap	d0		; Swap X and Y into right spots
		move.l	d0,(a1)		; Store it
		rts
*
*******************************************************************************
*
* transfPoint - Transforms the point passed by reference by the X/Y
* void transfPoint(struct Point *point,struct Point ul)
*
* ** Note Assumes that ul is passed as a LONG X/Y pair **
*
	xdef	_transfPoint
_transfPoint:
		move.l	4(sp),a0	; Get point
		move.l	8(sp),d0	; Get UL  (upper left)
		sub.w	d0,2(a0)	; Subtract Y
		swap	d0		; Get X into lower word
		sub.w	d0,(a0)		; Subtract X
		rts
*
*******************************************************************************
*
* inrect - Tells if the point is within the rectangle given
* d0=inrect(int x, int y, struct Rectangle *j)
*
	xdef	_inrect
_inrect:
		move.l	12(sp),a0	; Get rectangle
		moveq.l	#0,d0		; Assume it is not in the rectangle
		move.l	4(sp),d1	; Get X  (Only a word is important)
		cmp.w	(a0),d1		; Check against ra_MinX(a0)==(a0)
		blt.s	return		; Return if left of rectangle
		cmp.w	ra_MaxX(a0),d1	; Check against ra_MaxX
		bgt.s	return		; Return if right of rectangle
		move.l	8(sp),d1	; Get Y (Only a word is important)
		cmp.w	ra_MinY(a0),d1	; Check against ra_MinY
		blt.s	return		; If above rectangle, return
		cmp.w	ra_MaxY(a0),d1	; Check against ra_MaxY
		bgt.s	return		; If below rectangle, return
return_ok:	moveq.l	#1,d0		; Set TRUE (in rectangle)
return:		rts
*
* ptInBox - Tells if the point is within the box given
* d0=ptInBox(struct Point pt,struct IBox *b)
*
	xdef	_ptInBox
_ptInBox:
		movem.l	4(sp),d1/a0	; Get point into d1, and box in a0
*
		moveq.l	#0,d0		; Assume failure
		sub.w	ra_MinY(a0),d1	; Subtract Y (box top)
		blt.s	return		; If less that top, return
		sub.w	ra_MaxY(a0),d1	; Subtract Height
		bge.s	return		; If greater than bottom, return
*
		swap	d1		; Get X from point
		sub.w	(a0),d1		; Subtract X (left)  ra_MinX(a0)==(a0)
		blt.s	return		; If less that left, return
		sub.w	ra_MaxX(a0),d1	; Subtract Width
		bge.s	return		; If greater than right, return
		bra.s	return_ok	; Return OK...

*
* nonDegenerate - Tells if the rectangle is not inside-out
* d0=nonDegenerate( struct Rectangle *rect )
*
	xdef	_nonDegenerate
_nonDegenerate:
		move.l	4(sp),a0	; Get rectangle
		moveq.l	#0,d0		; Assume it is degenerate
		move.w	ra_MaxX(a0),d1	; d1 = ra_MaxX(a0)
		cmp.w	(a0),d1		; Check against ra_MinX(a0)==(a0)
		blt.s	return		; Return if left of MinX

		move.w	ra_MaxY(a0),d1	; d1 = ra_MaxY(a0)
		cmp.w	ra_MinY(a0),d1	; Check against ra_MinY
		blt.s	return		; If above MinY, return
		bra.s	return_ok	; Return TRUE (non-degenerate)

*
* collide - This is much like the ptInBox but the arguments are
*           split out...  So, we just unsplit them
* d0=collide(int x, int y, boxx, boxy, xsize, ysize)
*
*	xdef	_collide
*_collide:
*		lea	4+6*4(sp),a0	; quick pointer to end of args...
*		moveq.l	#4-1,d1		; Number of args to copy (-1)
*copyargs:	move.l	-(a0),d0	; Get arg as a long
*		move.w	d0,-(sp)	; Save it on the stack
*		dbra	d1,copyargs	; Do all of them
*		move.l	sp,-(sp)	; Save pointer to IBox
*		; Make the Point structure on stack now...
*		move.l	-(a0),d0	; Get arg as a long
*		move.w	d0,-(sp)	; Save it on the stack
*		move.l	-(a0),d0	; Get arg as a long
*		move.w	d0,-(sp)	; Save it on the stack
*		bsr.s	_ptInBox	; Do ptInBox
*		subq.l	#8,sp		; Restore the stack
*		subq.l	#8,sp		; (16 bytes =  6 words + 1 long)
*		rts
*
*******************************************************************************
*
* limitRect - Constrains rectangle to limiting rectangle
* void limitRect(struct Rectangle *r,struct Rectangle *limit)
*
	xdef	_limitRect
_limitRect:
		movem.l	4(sp),a0-a1	; Rectangle in a0, limit in a1
*
		move.w	(a0),d0		; Get MinX
		move.w	(a1)+,d1	; Get limit of MinX
		cmp.w	d0,d1		; Check it...
		ble.s	ok_MinX		; If MinX is OK...
		exg	d0,d1		; Swap d0/d1
ok_MinX:	move.w	d0,(a0)+	; Store it, move to next...
*
		move.w	(a0),d0		; Get MinY
		move.w	(a1)+,d1	; Get limit of MinY
		cmp.w	d0,d1		; Check it...
		ble.s	ok_MinY		; If MinY is OK...
		exg	d0,d1		; Swap d0/d1
ok_MinY:	move.w	d0,(a0)+	; Store it, move to next...
*
		move.w	(a0),d0		; Get MaxX
		move.w	(a1)+,d1	; Get limit of MaxX
		cmp.w	d0,d1		; Check it...
		bge.s	ok_MaxX		; If MaxX is OK...
		exg	d0,d1		; Swap d0/d1
ok_MaxX:	move.w	d0,(a0)+	; Store it, move to next...
*
		move.w	(a0),d0		; Get MaxY
		move.w	(a1)+,d1	; Get limit of MaxY
		cmp.w	d0,d1		; Check it...
		bge.s	ok_MaxY		; If MaxY is OK...
		exg	d0,d1		; Swap d0/d1
ok_MaxY:	move.w	d0,(a0)+	; Store it, move to next...
		rts
*
*******************************************************************************
*
* limitLongPoint
*
	xdef	_limitLongPoint
_limitLongPoint:
		movem.l	4(sp),a0-a1	; LongPoint = a0, limit in a1
		bsr.s	clamp		; Clamp X into low word of d1
		swap	d1		; Move X to top word
		bsr.s	clamp		; Clamp Y into low word of d1
		move.l	d1,-(sp)	; Put on clamped point onto stack
		move.l	sp,d0		; Save pointer
		move.l	a1,-(sp)	; Limit rect...
		move.l	d0,-(sp)	; Point
		bsr.s	_limitPoint	; Clamp the point...
		addq.l	#8,sp		; Clean up pointers...
		move.l	(sp)+,d0	; Get point...
		move.l	4(sp),a0	; Get longpoint...
		move.w	d0,d1		; Get y over there...
		swap	d0		; x in low word
		ext.l	d0		; extend it...
		move.l	d0,(a0)+	; Save X (long)
		ext.l	d1		; extend y
		move.l	d1,(a0)		; Save Y (long)
		rts
;
clamp:		move.l	(a0)+,d0	; Get next coord
		move.w	d0,d1		; Store in d1 for a moment...
		swap	d0		; Put high into low...
		tst.w	d0		; Check if it is ok...
		beq.s	no_clamp	; Small positive: no need to clamp
		addq.w	#1,d0		; Checking for -1 in "high" word
		beq.s	no_clamp	; Small negative: no need to clamp
		bmi.s	neg_clamp	; If large negative, clamp -32767
		move.w	#32767,d1	; Clamp to max int...
		rts
neg_clamp:	move.w	#-32767,d1	; Clamp to max -int
no_clamp:	rts
*
* limitPoint - Constrains point to limiting rectangle
* void limitPoint(struct Point *p,struct Rectangle *limit)
*
	xdef	_limitPoint
_limitPoint:
		movem.l	4(sp),a0-a1	; Point in a0, limit in a1
*
		move.w	(a0),d0		; Get X
		move.w	(a1)+,d1	; Get MinX
		cmp.w	d0,d1		; Check it
		ble.s	ok1_MinX	; If MinX is OK...
		exg	d0,d1		; Swap d0/d1
ok1_MinX:	move.w	2(a1),d1	; Get MaxX
		cmp.w	d0,d1		; Check it...
		bge.s	ok1_MaxX	; If MaxX is OK...
		exg	d0,d1		; Swap d0/d1
ok1_MaxX:	move.w	d0,(a0)+	; Store new point...
*
		move.w	(a0),d0		; Get Y
		move.w	(a1)+,d1	; Get MinY
		cmp.w	d0,d1		; Check it
		ble.s	ok1_MinY	; If MinY is OK...
		exg	d0,d1		; Swap d0/d1
ok1_MinY:	move.w	2(a1),d1	; Get MaxY
		cmp.w	d0,d1		; Check it...
		bge.s	ok1_MaxY	; If MaxY is OK...
		exg	d0,d1		; Swap d0/d1
ok1_MaxY:	move.w	d0,(a0)+	; Store new point...
		rts
*
*******************************************************************************
*
* Silly code to fill in a degenerated rectangle
* void degenerateRect(struct Rectangle *r)
*
	xdef	_degenerateRect
_degenerateRect:
		move.l	4(sp),a0	; Get Rectangle
		move.w	#$7FFF,d0	; Get 32767
		move.w	d0,(a0)+	; MinX
		move.w	d0,(a0)+	; MinY
		neg.w	d0		; Get -32767
		move.w	d0,(a0)+	; MaxX
		move.w	d0,(a0)		; MaxY
		rts
*
*******************************************************************************
*
* offsetRect - Generate a new rectangle at dx/dy offset from current
* void offsetRect(struct Rectangle *r,int dx, int dy)
*
	xdef	_offsetRect
_offsetRect:
		move.l	4(sp),a0	; Get Rectangle
		movem.l	8(sp),d0-d1	; dx in d0, dy in d1
		add.w	d0,(a0)+	; MinX+dx
		add.w	d1,(a0)+	; MinY+dy
		add.w	d0,(a0)+	; MaxX+dx
		add.w	d1,(a0)		; MaxY+dy
		rts
*
*******************************************************************************
*
* windowBox - Copies the window's IBox into the Box given...
* void windowBox(struct Window *w,struct IBox *box)
*
	xdef	_windowBox
_windowBox:
		movem.l	4(sp),a0-a1	; Window in a0, rectangle in a1
		addq.l	#wd_LeftEdge,a0	; Window pointer to LeftEdge of window
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+	; Copy box from window...
		rts
*
*******************************************************************************
*
* boxFit - fits box in container, minimizing movement, always keeping
*          upperleft of box visible
* void boxFit(struct IBox *box, struct IBox *container, struct IBox *result)
*
	xdef	_boxFit
_boxFit:
		movem.l	4(sp),d0/a0-a1	; box=d0, container=a0, result=a1
		exg	a0,d0		; Swap a0/d0
		move.l	(a0)+,(a1)+	; Copy
		move.l	(a0),(a1)	; ... box to result
		subq.l	#4,a1		; Backup result again...
		move.l	d0,a0		; Get container back...
*
		bsr.s	do_MaxMin	; Do MaxMin for X
		addq.l	#2,a0		; Move to the next
		; Drop down into the MaxMin code...
*
do_MaxMin:	move.w	(a1),d1		; Get Result...
		move.w	(a0),d0		; Get Container...
		add.w	ra_MaxX(a0),d0	; Add size...
		sub.w	ra_MaxX(a1),d0	; Subtract result size...
		cmp.w	d0,d1		; Check it
		ble.s	ok_Min		; Min is ok
		move.w	d0,d1		; d0 was less...
ok_Min:		move.w	(a0),d0		; Get container
		cmp.w	d0,d1		; Check it
		bge.s	ok_Max		; Max is ok
		move.w	d0,d1		; d0 was greater...
ok_Max:		move.w	d1,(a1)+	; Store result...
		rts
*
*******************************************************************************
*
* rectHull - Expands big to contain r2
* upperRectHull - Same as RectHull only not downwards
* void rectHull(struct Rectangle *big,struct Rectangle *r2)
*
	xdef	_rectHull
_rectHull:
		movem.l	4(sp),a0-a1	; big=a0, r2=a1
		move.w	ra_MaxY(a0),d0
		move.w	ra_MaxY(a1),d1	; Get the two values
		cmp.w	d0,d1		; Check if r2 is bigger
		ble.s	ok2_MaxY	; If not, we are done here
		move.w	d1,ra_MaxY(a0)	; Store the bigger r2
ok2_MaxY:	; Fall into the upperRectHull
*
	xdef	_upperRectHull
_upperRectHull:
		movem.l	4(sp),a0-a1	; big=a0, r2=a1
		move.w	(a1)+,d1	; Get MinX (and advance)
		move.w	(a0),d0		; Get other MinX
		cmp.w	d0,d1		; Check if r2 is less than big
		bge.s	ok2_MinX	; If not we skip...
		move.w	d1,d0		; Get r2 MinX
ok2_MinX:	move.w	d0,(a0)+	; Store and advance...
*
		move.w	(a1)+,d1	; Get MinY (and advance)
		move.w	(a0),d0		; Get other MinY
		cmp.w	d0,d1		; Check if r2 is less than big
		bge.s	ok2_MinY	; If not we skip...
		move.w	d1,d0		; Get r2 MinY
ok2_MinY:	move.w	d0,(a0)+	; Store and advance...
*
		move.w	(a1)+,d1	; Get MaxX (and advance)
		move.w	(a0),d0		; Get other MaxX
		cmp.w	d0,d1		; Check if r2 is biffer than big
		ble.s	ok2_MaxX	; If not we skip...
		move.w	d1,d0		; Get r2 MaxX
ok2_MaxX:	move.w	d0,(a0)+	; Store and advance...
		rts
*
*******************************************************************************
*
*
* windowObscured(window)
*
* Function to find out if a window is obscured by a window higher
* up.  It needs to walk the layer list for the ordering, but it
* had better not get confused by requester layers, which aren't
* deemed to obscure their parent window.
* Needs to be able to handle a layer with a NULL Window pointer
* (eg. the bar-layer)
*
		xref	_LockLayerInfo
		xref	_UnlockLayerInfo	; We use the Intuition stubs
		xdef	_windowObscured
_windowObscured:
		move.l	4(sp),a1		; Get Window pointer...
		move.l	wd_WScreen(a1),a0	; Get Screen...
		lea	sc_LayerInfo(a0),a0	; Get address of layer info...
		move.l	a0,-(sp)		; Save this
		jsr	_LockLayerInfo		; Call lock stub...
		move.l	(sp),a0			; Get LayerInfo again...
		subq.l	#lr_back,a0		; Fudge LI for ((struct Layer *)li)->back
		move.l	a2,-(sp)		; Save a2...
		move.l	12(sp),a2		; Get window pointer...
		move.l	wd_RPort(a2),a1		; Get RastPort...
		btst.b	#2,wd_Flags+2(a2)	; Check for WFLG_GIMMEZEROZERO
		beq.s	notGZZ
		move.l	wd_BorderRPort(a2),a1	; Get Border RastPort...
notGZZ:		move.l	(a1),a1			; Get RP->Layer (rp_Layer==0)
*
* Ok, so now a2 is our window pointer, a1 is our layer (from window)
* a0 is layer pointer (and a pointer to a layer pointer...)
*
search_loop:	move.l	lr_back(a0),d0		; Get next layer
		beq.s	end_of_list		; huh?  Should never happen
		move.l	d0,a0			; Get layer pointer...
		moveq.l	#0,d0			; Initial FALSE reading
		cmp.l	lr_Window(a0),a2	; Is it our window
		beq.s	end_of_list		; If so, we found none...
		move.w	lr_MaxX(a0),d1		; Check Max vs Min X collision
		cmp.w	lr_MinX(a1),d1
		blt.s	search_loop		; If we miss, we skip...
		move.w	lr_MinX(a0),d1		; Check the other X direction
		cmp.w	lr_MaxX(a1),d1
		bgt.s	search_loop
		move.w	lr_MaxY(a0),d1		; Check Max vs Min Y collision
		cmp.w	lr_MinY(a1),d1
		blt.s	search_loop
		move.w	lr_MinY(a0),d1		; Check the other Y direction
		cmp.w	lr_MaxY(a1),d1
		bgt.s	search_loop
		moveq.l	#1,d0			; Set TRUE flag for collision
*
* Now, d0 is result...
*
end_of_list:	move.l	(sp)+,a2		; Restore a2...
		move.l	(sp)+,a0		; Get LayerInfo
		move.l	d0,-(sp)		; Save result...
		move.l	a0,-(sp)		; For unlocklayerinfo...
		jsr	_UnlockLayerInfo	; Call intuition stub...
		addq.l	#4,sp			; Clean off LayerInfo
		move.l	(sp)+,d0		; Get Result...
		rts
*
*******************************************************************************


*	value = GetTagData(tagValue,defaultVal,tagList);
*	D0		   D0	    D1	       A0

		xdef	_GetTagDataUser0
		xdef	_GetTagDataUser
		xref	_LVOGetTagData

_GetTagDataUser0:	; GetTagDataUser0( tagValue, tagList)
	move.l	8(sp),a0
	clr.l	d1
	bra.s	label

_GetTagDataUser:	; GetTagDataUser( tagValue, defaultVal, tagList )
	move.l	8(sp),d1
	move.l	$C(sp),a0
label:
	move.l	4(sp),d0
	bset.l	#31,d0			; Set TAG_USER
	move.l	a6,-(sp)
	move.l	_UtilityBaseOffset(a6),a6
	jsr	_LVOGetTagData(a6)
	move.l	(sp)+,a6
	rts

*******************************************************************************


		end
