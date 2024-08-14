*** stubs.asm ************************************************************
*
*   stubs.asm - useful things for useful people...
*
*   Copyright 1989-1992, Commodore-Amiga, Inc.
*
*   $Id: stubs.asm,v 39.8 92/10/16 18:29:51 vertex Exp $
*
**************************************************************************

        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	'gadtools.i'
	INCLUDE	'gtinternal.i'
	INCLUDE	'intuition/gadgetclass.i'

        LIST

;---------------------------------------------------------------------------

	XDEF	@findTagItem
	XDEF	@findTagItemGT
	XDEF	@getTagData
	XDEF	@getTagDataGT
	XDEF	@getTagDataGA
	XDEF	@getTagDataSTRING
	XDEF	_removeGadget
	XDEF	_AddRefreshGadget
	XDEF	_closeFont
	XDEF	_setAPen
	XDEF	__CXD33			; Signed divide
	XDEF	__CXD22			; Unsigned divide
	XDEF	__CXM33			; Signed multiply
	XDEF	__CXM22			; Unsigned multiply
	XDEF	@stccpy

;---------------------------------------------------------------------------

	XREF	_LVOSetAPen
	XREF	_LVOCloseFont
	XREF	_LVOAddGList
	XREF	_LVORemoveGadget
	XREF	_LVORefreshGList
	XREF	_LVOGetTagData
	XREF	_LVOFindTagItem

;---------------------------------------------------------------------------

* getTagData() and findTagItem()
* These stubs were originally coded in C.  They're much smaller here.
* (We have them because for very frequently-called functions, it's
* more efficient to call a stub which calls the library.

* getTagDataGT() and findTagItemGT()
* Calls @getTagData() or @findTagItem() with the tag OR'd with GT_TagBase.
* Save much space, since our callers will have byte-sized values
* instead of longs to put in d0


;---------------------------------------------------------------------------

* findTagItem( TagVal d0, TagList a0 )
* findTagItemGT( TagVal d0, TagList a0 )

@findTagItemGT:	ori.l	#GT_TagBase,d0
@findTagItem:	move.l	a6,-(sp)
		movea.l	gtb_UtilityBase(a6),a6
		jsr	_LVOFindTagItem(a6)
		bra.s	PopA6Rts

;---------------------------------------------------------------------------

* getTagData( TagVal d0, Default d1, TagList a0 )
* getTagDataGT( TagVal d0, Default d1, TagList a0 )

@getTagDataSTRING:
		ori.l	#STRINGA_Dummy,d0
		bra.s	@getTagData
@getTagDataGA:	ori.l	#GA_Dummy,d0
		bra.s	@getTagData
@getTagDataGT:	ori.l	#GT_TagBase,d0
@getTagData:	move.l	a6,-(sp)
		movea.l	gtb_UtilityBase(a6),a6
		jsr	_LVOGetTagData(a6)

		; *** FALLS THROUGH!

;---------------------------------------------------------------------------
; just here to save some bytes in all the other routines

PopA6Rts:	movea.l	(sp)+,a6
DoRts:		rts

;---------------------------------------------------------------------------

* removeGadget( window a0, gadget a1)

_removeGadget:	move.l	a0,d1		; protect against a NULL window pointer
		beq.s	DoRts
		move.l	a6,-(sp)
		movea.l	gtb_IntuitionBase(a6),a6
		jsr	_LVORemoveGadget(a6)
		bra.s	PopA6Rts

;---------------------------------------------------------------------------

* AddRefreshGadget( gad a1, win a0, pos d0)

_AddRefreshGadget:
		move.l	a0,d1		; protect against a NULL window pointer
		beq.s	DoRts
		move.l	a6,-(sp)
		move.l	a2,-(sp)
		sub.l	a2,a2			; requester pointer
		moveq	#1,d1			; number of gadgets
		move.l	gtb_IntuitionBase(a6),a6
		movem.l	d1/a0-a1,-(sp)
		jsr	_LVOAddGList(a6)
		movem.l	(sp)+,d0/a0-a1		; NOTE! d1 gets restored to d0
		exg	a0,a1
		jsr	_LVORefreshGList(a6)
		move.l	(sp)+,a2
		bra.s	PopA6Rts

;---------------------------------------------------------------------------

_closeFont:	move.l	a1,d0		; protect against a NULL font pointer
		beq.s	DoRts
		move.l	a6,-(sp)
		movea.l	gtb_GfxBase(a6),a6
		jsr	_LVOCloseFont(a6)
		bra.s	PopA6Rts

;---------------------------------------------------------------------------

_setAPen:	move.l	a6,-(sp)
		movea.l	gtb_GfxBase(a6),a6
		jsr	_LVOSetAPen(a6)
		bra.s	PopA6Rts

;---------------------------------------------------------------------------
; Replacements for the SAS/C 32-bit integer math functions using
; utility.library instead

__CXD33:
		move.l	gtb_SDivMod32(a6),-(sp)	; address of vector
		rts				; go there!
__CXD22:
		move.l	gtb_UDivMod32(a6),-(sp)	; address of vector
		rts				; go there!
__CXM33:
		move.l	gtb_SMult32(a6),-(sp)	; address of vector
		rts				; go there!
__CXM22:
		move.l	gtb_UMult32(a6),-(sp)	; address of vector
		rts				; go there!

;-----------------------------------------------------------------------
; stccpy	- Smaller version of the copy routine...
;  a0 - destination
;  a1 - source
;  d0 - destination length

@stccpy:	subq.l	#1,d0			; One less than given...
		bmi.s	2$			; if we end up with no bytes!
1$:		move.b	(a1)+,(a0)+		; Copy the data...
		dbeq.s	d0,1$			; Loop for max or null...
		clr.b	-(a0)			; Make sure NULL terminated
2$:		rts

;-----------------------------------------------------------------------

        END
