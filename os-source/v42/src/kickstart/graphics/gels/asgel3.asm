*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: asgel3.asm,v 42.0 93/06/16 11:19:23 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	asgel3.asm,v $
;; Revision 42.0  93/06/16  11:19:23  chrisg
;; initial
;; 
* Revision 42.0  1993/06/01  07:21:01  chrisg
* *** empty log message ***
*
*   Revision 39.0  91/08/21  17:33:17  chrisg
*   Bumped
*   
*   Revision 37.2  91/05/02  16:56:33  chrisg
*   killed ".." for lattice
*   
*   Revision 37.1  91/04/19  14:10:58  chrisg
*     Downcoded blitter completion routine for gels.
*   
*   Revision 37.0  91/01/07  15:28:11  spence
*   initial switchover from V36
*   
*   Revision 33.4  90/07/27  16:35:39  bart
*   id
*   
*   Revision 33.3  90/03/28  09:22:51  bart
*   *** empty log message ***
*   
*   Revision 33.2  88/11/16  10:05:13  bart
*   *** empty log message ***
*   
*   Revision 33.1  88/11/16  09:44:02  bart
*   big blits
*   
*   Revision 33.0  86/05/17  15:22:10  bart
*   added to rcs for updating
*   
*
*******************************************************************************


	include 'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/ports.i'
	include 'graphics/gfx.i'
	include 'graphics/rastport.i'
	include 'hardware/blit.i'
	include 'asbob.i'
	include 'hardware/custom.i'
	include '/sane_names.i'

	section	graphics
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
		move.w	BCN1(a1),bltcon1(a0)	* set blitter control word 1
		move.l	FWM(a1),fwmask(a0)		* set the first-wrod mask

		move.w	BMDSRC(a1),d1			* get the source modulo
		ifeq	AGNUS
			asr.w	#1,d1				* if not new aggie, make into byte addr
		endc
		move.w	d1,bltmda(a0)			* set source a/b modulo
		move.w	d1,bltmdb(a0)
		move.w	BMDDST(a1),d1			* get dest modulo and do the same
		ifeq	AGNUS
			asr.w	#1,d1
		endc
		move.w	d1,bltmdc(a0)
		move.w	d1,bltmdd(a0)
	endif

*	test the write-mask for this bit (pass number)
	btst	d0,WRITEMASK(a1)
*	if set then blit this plane
	if <>   .extend

*		set up address of a-source
		ifeq	AGNUS
			move.l	ASRC(a1),d1			* if not aggie, shift address
			asr.l	#1,d1
			move.l	d1,bltpta(a0)
		endc
		ifne	AGNUS
			move.l	ASRC(a1),bltpta(a0)	* else do it direct
		endc
		move.w	#$ffff,adata(a0)
		ext.w	d0
		move.w	d0,d1
		asl.w	#2,d1
		move.l	DESTPTR(a1,d1.w),d1
		ifeq	AGNUS
			asr.l	#1,d1
		endc
		move.l	d1,bltptc(a0)
		move.l	d1,bltptd(a0)

		if	#B2BOBBER<>BLITTYPE(a1).w
doother:
			move.b	SRCINDEX(a1),d1
			ext.w	d1
			ifeq	AGNUS
				move.l	SRCPTR(a1,d1.w),d1
				add.b	#4,SRCINDEX(a1)
				asr.l	#1,d1
				move.l	d1,bltptb(a0)
			endc
			ifne	AGNUS
				move.l	SRCPTR(a1,d1.w),bltptb(a0)
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
		move.w	d1,bltcon0(a0)

		move.w	vposr(a0),d1
		btst	#13,d1	; new agnus?
		if =
			move.w	BLITSIZE(a1),bltsize(a0)
		    else
			move.w	BLITSIZV(a1),d1
			and.w	#$7fff,d1	; upward compatibility
			move.w	d1,bltsizv(a0)
			move.w	BLITSIZH(a1),d1
			and.w	#$7ff,d1	; upward compatibility
			move.w	d1,bltsizh(a0)
		endif
	endif
	addq.b	#1,d0
	move.b	d0,BLISSINDEX(a1)
	sub.b	#1,BLITCNT(a1)
*	condition codes set up for blitter routine based on the above instruction
	rts

	xref	_LVOSignal

BLITTER_SIGNAL equ 4

	xdef _BlsDn
	xref	_blissDone
_BlsDn:
	move.l	a6,-(a7)
	moveq	#1<<BLITTER_SIGNAL,d0
	move.l	WHOSENTME(a1),a1
	move.l	$4,a6
	jsr	_LVOSignal(a6)
	move.l	(a7)+,a6
	rts

	end
