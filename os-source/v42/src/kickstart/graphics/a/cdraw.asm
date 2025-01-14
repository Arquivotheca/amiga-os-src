*******************************************************************************
*
*	$Id: cdraw.asm,v 42.0 93/06/16 11:13:27 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
	include 'graphics/rastport.i'
	include 'graphics/clip.i'
	include 'hardware/blit.i'
	include '/c/cdrawinfo.i'
	include 'submacs.i'

	section	graphics
    xdef    cdraw
	xref	_cdraw

cdraw:
	movem.l	d2/d3/d4/d5/d6/d7/a2/a3/a5,-(sp)
	lea		-cd_sizeof(sp),sp
	move.l	(a1),a5		* get layer pointer
	LOCKLAYER
	movem.w	rp_cp_x(a1),d2/d3	* get old cp_x,cp_y

*	bart - 06.30.86 - don't update cp_x, cp_y yet !!!
*	movem.w	d0/d1,rp_cp_x(a1)	* new cp_x,cp_y

*	this code assumes top of sp has cdrawinfo structure
	move.w	d0,d4				* use d4/d5 for scratch
	move.w	d1,d5
	sub.w	d2,d4				* generate dx
	sub.w	d3,d5				* genereate dy
	move.w	d4,cd_dx(sp)
	if <						* get abs(dx)
		neg.w	d4
	endif
	move.w	d5,cd_dy(sp)
	if <						* get abs(dy)
		neg.w	d5
	endif
	if d5>d4.w					* octant 2/3/6/7
		clr.b	cd_xmajor(sp)
		move.w	d4,cd_absdy(sp)	* swap them
		move.w	d5,cd_absdx(sp)
		if cd_dy(sp)<#0			* octant 6/7
			if cd_dx(sp)<#0		* octant 6
				moveq	#OCTANT6,d4
			else				* octant 7
				moveq	#OCTANT7,d4
			endif
		else					* octant 2/3
			if cd_dx(sp)<#0		* octant 3
				moveq	#OCTANT3,d4
			else				* octant 2
				moveq	#OCTANT2,d4
			endif
		endif
	else						* octant 1/4/5/8
		move.b	#1,cd_xmajor(sp)
		movem.w	d4/d5,cd_absdx(sp)	* set up standard absdx/dy
		if cd_dy(sp)<#0			* octant 5/8
			if cd_dx(sp)<#0		* octant 5
				moveq	#OCTANT5,d4
			else				* octant 8
				moveq	#OCTANT8,d4
			endif
		else					* octant 1/4
			if cd_dx(sp)<#0		* octant 4
				moveq	#OCTANT4,d4
			else				* octant 1
				moveq	#OCTANT1,d4
			endif
		endif
	endif
	move.w	d4,cd_con1(sp)

	movem.w	lr_MinX(a5),d4/d5	* get bounds.MinX/MinY
	sub.w	lr_Scroll_X(a5),d4	* real delta's
	sub.w	lr_Scroll_Y(a5),d5	* "
	move.l	d0,d6				* copies of to_x,to_y
	move.l	d1,d7
	add.w	d4,d0				* real tx,ty
	add.w	d5,d1
	add.w	d2,d4				* oldx,oldy
	add.w	d3,d5
*	pack in a single long
	swap	d4
	move.w	d5,d4				* d4.l = oldx.oldy
	move.l	d0,d5
	swap	d5
	move.w	d1,d5				* d5.l = tx.ty
*	d0/d1 unused at this time

	move.l	a1,a3				* rastport *
	move.l	lr_ClipRect(a5),d0
	while <>
		move.l d0,a2
		move.l	sp,a0		* save pointer to cdi
*		move.l	d5,-(sp)	* t
*		move.l	d4,-(sp)	* old
*		move.l	a0,-(sp)	* cdi
*		move.l	a2,-(sp)	* cr
*		move.l	a3,-(sp)	* rastport
		bsr.s		cr_cdraw
*		lea		5*4(sp),sp
		tst.l	d0
		bne.s	my_exit
		move.l (a2),d0		* get next ClipRect
	whend
*   only need to do this if superbitmap
	if	lr_SuperBitMap(a5).l
		move.l	sp,-(sp)		* pointer to cdrawinfo
		move.w	d7,-(sp)		* new x,y
		move.w	d6,-(sp)
		move.w	d3,-(sp)		* old cp_x,cp_y
		move.w	d2,-(sp)
		move.l	a3,-(sp)
		jsr		_cdraw
		lea		16(sp),sp
	endif
my_exit:
	UNLOCKLAYER

*	bart - 06.30.86 - OK to update cp_x, cp_y now !!!
	movem.w	d6/d7,rp_cp_x(a3)

	move.w	cd_absdx(sp),d0
	sub.b	d0,rp_linpatcnt(a3)
	lea		cd_sizeof(sp),sp
	movem.l	(sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a5
	rts

*new_draw_vect:
*	movem.l	d2-d7/a2-a3,-(sp)
*	move.l	(8*4)+4(sp),a1	* get rastport
*	move.l	rp_BitMap(a1),a2
*	move.l	(8*4)+8(sp),a0	* get cdi
*	movem.l	(8*4)+12(sp),d2/d3	* get (x0,y0)
*	muls	(a2),d3
*	move.l	d2,d4
*	asr.l	#3,d2
*	add.l	d2,d3				* d3 = offset into bitmap
*	and.w	#15,d4				* bit number to start
*	move.w	cd_absdx(a0),d1
*	add.w	d1,d1				* 2*abs(dy)
*	move.l	(8*4)+24(sp),d2		* initial error term
*	move.l	(8*4)+20(sp),

	xdef	_cr_cdraw
_cr_cdraw:
*	take stuff off stack
	movem.l	d4/d5/a2/a3,-(sp)
		move.l	16+4(sp),a3	* rastport
		move.l	16+8(sp),a2	* cr
		move.l	16+12(sp),a0	* cdi
		move.l	16+16(sp),d4	* old
		move.l	16+20(sp),d5	* t
		bsr.s		cr_cdraw
	movem.l	(sp)+,d4/d5/a2/a3
	rts

	xref	_c_cr_cdraw
	xref	getcode
	xref	BareDraw
	xref	_clipdraw
cr_cdraw:
*   cdi->code1 = getcode(cr,old.x,old.y);
*   cdi->code2 = getcode(cr,t.x,t.y);
	move.w	d4,d1			* transfer y old
	move.l	d4,d0
	swap d0
	bsr	getcode
	move.b	d0,cd_code1(a0)
	move.w	d5,d1			* transfer y  't'
	move.l	d5,d0
	swap d0
	bsr	getcode
	move.b	d0,cd_code2(a0)
	or.b	cd_code1(a0),d0
	if =
*		trivial accept
		if cr_lobs(a2)=#0.l
*			on screen
			move.l	d4,rp_cp_x(a3)
			move.w	d5,d1
			move.l	d5,d0
			swap	d0
			move.l	a3,a1		* rastport
			move.w	rp_linpatcnt(a3),-(sp)
			bsr	BareDraw		* BareDraw also updates linpatcnt - spence Dec  7 1990
			move.w	(sp)+,rp_linpatcnt(a3)
		else
*			obscured
			move.l	cr_BitMap(a2),d0
			if <>
*				is there a bitmap to put bits in?
				move.l	rp_BitMap(a3),-(sp)	* save old BitMap on stack
				move.l	d0,rp_BitMap(a3)	* use new BitMap
				moveq	#~15,d1
				and.w	cr_MinX(a2),d1
				move.l	d4,d0		* compute offset x
				swap	d0
				sub.w	d1,d0
				move.w	d4,d1		* compute offset y
				sub.w	cr_MinY(a2),d1
				movem.w	d0/d1,rp_cp_x(a3)
				moveq	#~15,d1
				and.w	cr_MinX(a2),d1
				move.l	d5,d0		* compute offset x
				swap	d0
				sub.w	d1,d0
				move.w	d5,d1		* compute offset y
				sub.w	cr_MinY(a2),d1
				move.w	rp_linpatcnt(a3),-(sp)
				bsr	BareDraw		* BareDraw also updates linpatcnt - spence Dec  7 1990
				move.w	(sp)+,rp_linpatcnt(a3)
				move.l	(sp)+,rp_BitMap(a3)
			endif
		endif
		moveq	#1,d0
	else
		move.b	cd_code1(a0),d0
		and.b	cd_code2(a0),d0
		if =		* cross over somewhere
			if cr_BitMap(a2).l
*				call clipdraw routine d5=t d4=old
or_entry:
*				move.l	d5,-(sp)		* to
				move.w	d5,d0
				ext.l	d0
				move.l	d0,-(sp)
				move.l	d5,d0
				swap	d0
				ext.l	d0
				move.l	d0,-(sp)

*				move.l	d4,-(sp)		* from
				move.w	d4,d0
				ext.l	d0
				move.l	d0,-(sp)
				move.l	d4,d0
				swap	d0
				ext.l	d0
				move.l	d0,-(sp)

				move.l	a0,-(sp)		* cdi
				move.l	a2,-(sp)		* cr
				move.l	a3,-(sp)		* rp
				jsr	_clipdraw
				lea	7*4(sp),sp
			else
				tst.l	cr_lobs(a2)
				beq.s	or_entry
			endif
		endif
		moveq	#0,d0
	endif
	rts

	end
