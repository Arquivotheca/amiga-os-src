*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: asgel3.i,v 37.0 91/01/07 15:32:44 spence Exp $
*
*	$Locker:  $
*
*	$Log:	asgel3.i,v $
*   Revision 37.0  91/01/07  15:32:44  spence
*   initial switchover from V36
*   
*   Revision 33.2  90/07/27  16:35:50  bart
*   id
*   
*   Revision 33.1  90/03/28  09:27:39  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:23:38  bart
*   added to rcs for updating
*   
*
*******************************************************************************


	include 'types.i'
	include 'gfx/rastport.i'
	include 'gfx/blit.i'
	include 'gfx/asbob.i'

	xdef	_qBlissIt
* qBlissIt
*	interrupt-queued bob blitter-driver
*
*	a1 pointer to blissObj
*	a0 pointer to i/o space "a00000"
*	d0/d1/a0/a1/a5	can be scratch registers
_qBlissIt:

*	load d0 with pass index
	move.b	BLISSINDEX(a1),d0

*	if this is pass 0, set up the blitter
	if =
		move.w	BCN1(a1),BLTCON1(a0)	* set blitter control word 1
		move.l	FWM(a1),FWMASK(a0)		* set the first-wrod mask

		move.w	BMDSRC(a1),d1			* get the source modulo
		ifeq	AGNUS
			asr.w	#1,d1				* if not new aggie, make into byte addr
		endc
		move.w	d1,BLTMDA(a0)			* set source a/b modulo
		move.w	d1,BLTMDB(a0)
		move.w	BMDDST(a1),d1			* get dest modulo and do the same
		ifeq	AGNUS
			asr.w	#1,d1
		endc
		move.w	d1,BLTMDC(a0)
		move.w	d1,BLTMDD(a0)
	endif

*	test the write-mask for this bit (pass number)
	btst	d0,BLTMASK(a1)
*	if set then blit this plane
	if <>

*		set up address of a-source
		ifeq	AGNUS
			move.l	ASRC(a1),d1			* if not aggie, shift address
			asr.l	#1,d1
			move.l	d1,BLTPTA(a0)
		endc
		ifne	AGNUS
			move.l	ASRC(a1),BLTPTA(a0)	* else do it direct
		endc
		move.w	#$ffff,ADATA(a0)
		ext.w	d0
		move.w	d0,d1
		asl.w	#2,d1
		move.l	DESTPTR(a1,d1.w),d1
		ifeq	AGNUS
			asr.l	#1,d1
		endc
		move.l	d1,BLTPTC(a0)
		move.l	d1,BLTPTD(a0)

		if	#B2BOBBER<>BLITTYPE(a1).w
doother:
			move.b	SRCINDEX(a1),d1
			ext.w	d1
			ifeq	AGNUS
				move.l	SRCPTR(a1,d1.w),d1
				add.b	#4,SRCINDEX(a1)
				asr.l	#1,d1
				move.l	d1,BLTPTB(a0)
			endc
			ifne	AGNUS
				move.l	SRCPTR(a1,d1.w),BLTPTB(a0)
				addq.b	#4,d1
				move.b	d1,SRCINDEX(a1)
			endc
			move.w	MINTERM(a1),d1
		else
			btst	d0,PPICK+1(a1)
			bne		doother
			btst	d0,PONOFF+1(a1)
			if <>
				move.w	#FILLSHADOW,d1
			else
				move.w	#CLEARSHADOW,d1
			endif
		endif
		or.w	PBCN0(a1),d1
		move.w	d1,BLTCON0(a0)
		move.w	BLITSIZE(a1),BLTSIZE(a0)
	endif
	addq.b	#1,d0
	move.b	d0,BLISSINDEX(a1)
	sub.b	#1,BLITCNT(a1)
*	condition codes set up for blitter routine based on the above instruction
	rts

	xdef _BlsDn
	xref	_blissDone
_BlsDn:
	move.l	a1,-(sp)	* stack the arg
	jsr	_blissDone
	addq.l	#4,sp
	rts

