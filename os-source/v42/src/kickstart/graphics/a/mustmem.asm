	SECTION	graphics
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"graphics/gfxbase.i"
	INCLUDE	"hardware/blit.i"

	XDEF	FreeMustMem
	XDEF	_freemustmem
	XDEF	GetMustMem
	XDEF	_getmustmem

	XREF	_LVOAlert
	XREF	_LVOAllocMem
	XREF	_LVOFreeMem
	XREF	_LVOAttemptSemaphore
	XREF	_LVOObtainSemaphore
	XREF	_LVOReleaseSemaphore

*******	GetMustMem ***************************************************
*
*   memory = GetMustMem(size, constraints), GfxBase
*   d0                  d0    d1            a6
*
**********************************************************************
*
* Jan 18 1991 - if cannot get any memory, and LCM is too small,
* now returns NULL instead of GURUing.
*
* This may cause problems for:
* Text() - shouldn't. Change was made for Text() in the first place!
* bltSetup() in text/bltcopy.asm - now checks for NULL, and returns.
* 		This may cause problems further up the calling chain, though
*		doubtful.
* makemask() in c/blitplate.c - now checks for NULL, and returns.
* makemask() is on;ly called by oldbltpattern(), which checks returned
* value anyway.
* getmaxbytesperrow() - will always (eventually) get LCM.
* bindrecord() in d/startup.c - if you have no memory at startup.....
*
* - spence Jan 18 1991
*
**********************************************************************
_getmustmem:
		movem.l	4(a7),d0/d1
GetMustMem:
		movem.l	d2/d3/a5/a6,-(a7)
		move.l	d0,d2			; save size
		move.l	d1,d3			; save constraints
		move.l	a6,a5			; save GfxBase
		move.l	gb_ExecBase(a6),a6

		;------ see if LCM is appropriate for use
		cmp.l	#MAXBYTESPERROW,d2
		bgt.s	mustTryAlloc

		move.l	gb_LastChanceMemory(a5),a0
		jsr	_LVOAttemptSemaphore(a6)
		tst	d0
		beq.s	mustTryAlloc

		move.l	gb_LCMptr(a5),d0	; get last chance pointer
		bra.s	gotMustMem

mustTryAlloc:
		move.l	d2,d0			; recover size
		move.l	d3,d1			; recover constraints
		jsr	_LVOAllocMem(a6)
		tst.l	d0
		bne.s	gotMustMem

		cmp.l	#MAXBYTESPERROW,d2	; check if size is < max avail
*		ble.s	getLastChance
		bgt.s	gotMustMem

*		;------ this is a dead end alert, don't fret trashing things
*		move.l	d2,a5			; (show size requested)
*		move.l	#AN_GfxNoLCM,d7
*		jsr	_LVOAlert(a6)
getLastChance:
		move.l	gb_LastChanceMemory(a5),a0
		jsr	_LVOObtainSemaphore(a6)
		move.l	gb_LCMptr(a5),d0	; get last chance pointer

gotMustMem:
		movem.l	(a7)+,d2/d3/a5/a6
		rts


*******	FreeMustMem **************************************************
*
*   FreeMustMem(address, size), GfxBase
*               a1       d0     a6
*
**********************************************************************
_freemustmem:
		move.l	4(a7),a1
		move.l	8(a7),d0
FreeMustMem:
		move.l	a6,-(a7)

		move.l	gb_LCMptr(a6),a0
		cmp.l	a0,a1
		beq.s	freeLastChance

		move.l	a1,d1			; getmustmem can now return NULL, so need to check
		beq.s	freedMem		; spence - Jan 16 1991

		move.l	gb_ExecBase(a6),a6
		jsr	_LVOFreeMem(a6)
freedMem:
		move.l	(a7)+,a6
		rts

freeLastChance:
		move.l	gb_LastChanceMemory(a6),a0
		move.l	gb_ExecBase(a6),a6
		jsr	_LVOReleaseSemaphore(a6)
		bra.s	freedMem
	END
