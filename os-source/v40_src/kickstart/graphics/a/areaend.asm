*******************************************************************************
*
*	$Id: areaend.asm,v 39.3 92/09/03 15:35:58 spence Exp $
*
*******************************************************************************

	section	graphics
    xdef    _AreaEnd
	xref	_draw_fill_outline
	xref	waitblitdone
******* graphics.library/AreaEnd *********************************************
*
*   NAME
*	AreaEnd -- Process table of vectors and ellipses and produce areafill.
*
*
*   SYNOPSIS
*	error = AreaEnd(rp)
*	  d0  	        A1
*
*	LONG AreaEnd( struct RastPort * );
*
*   FUNCTION
*	Trigger the filling operation.
*	Process the vector buffer and generate required
*	fill into the raster planes. After the fill is complete, reinitialize
*	for the next AreaMove or AreaEllipse. Use the raster set up by
*	InitTmpRas when generating an areafill mask.
*
*   RESULT
*	error - zero for success, or -1 if an error occurred anywhere.
*
*   INPUTS
*	rp - pointer to a RastPort structure which specifies where the filled
*	     regions will be rendered to.
*
*   BUGS
*
*   SEE ALSO
*	InitArea() AreaMove() AreaDraw() AreaEllipse()  InitTmpRas()
*	graphics/rastport.h
*
******************************************************************************

    include 'exec/types.i'
	include '/macros.i'
    include 'graphics/rastport.i'
    include '/c/areafill.i'
	include 'hardware/blit.i'
	include	'hardware/custom.i'
	include '/sane_names.i'
	include 'graphics/gfxbase.i'
	include 'submacs.i'

	xref	_LVOAreaDraw
	xref	_LVOBltClear
	xref	_LVOInitBitMap
	xref	_LVOBltPattern

	xref	_LVODraw
	xref	_LVOMove

	xref	_LVODrawEllipse	* bart - 05.05.86

*	instead of using LVODraw/Move, save time and use BareDraw
	xref	BareDraw
	xref	_Move

	xref	_LVOWritePixel
	xref	bltbytes
	xref	_custom


*	used to terminate in error
bad:
	moveq	#-1,d0
	bra	ae_exit

_AreaEnd:
*	current routine calls a C subroutine to do the rest of work
	movem.l	d2-d7/a2-a5,-(sp)
	move.l	a1,a3				* save RastPort * in a3
	move.l	rp_AreaInfo(a3),a2		* get AreaInfo ptr
	move.l	ai_FlagPtr(a2),a0		* temp FlagPtr

	cmpi.b	#0,-1(a0)			* was a MOVE just done?
	if =
	    cmpi.w	#1,ai_Count(a2)		* was it the only instruction ?
	    ble		ae_exit			* back up to nothing...
	else
	    cmp.w	#2,ai_Count(a2)		* at least two instructions ?
	    blt.s	not_extended

	    btst    #EXTENDEDn,-2(a0)		* was an extended instruction ?
	    if =
not_extended:
		move.l	ai_VctrPtr(a2),a0	* temp VctrPtr
		move.l	ai_FirstX(a2),d7	* get x and y
		cmp.w -2(a0),d7			* check y coordinate
		if =
		    swap	d7
		    cmp.w	-4(a0),d7	* now check x coordinate
		    beq.s	closed
		    swap	d7		* getem back to good order
		endif
*		set up to put last vector in to close this
		move.w	d7,d1			* d1 = y
		swap	d7
		move.w	d7,d0			* d0 = x
		jsr	_LVOAreaDraw(a6)
		bne	bad			* check AreaDraw condition code
	    endif
	endif
closed:

*	a2 now points to AreaInfo
	move.l	ai_VctrTbl(a2),a0		* get pointer to first guy in vector table
	move.w	(a0)+,d0				* prime minx
	move.w	(a0)+,d1				* prime miny
	move.w	d0,d2					* prime maxx
	move.w	d1,d3					* prime maxy
	move.w	ai_Count(a2),d7
	subq.w	#2,d7					* for dbne
qwe:
		move.w	(a0)+,d6
		if d6<d0.w
			move.w	d6,d0
		else
			if d6>d2.w
				move.w	d6,d2
			endif
		endif
		move.w	(a0)+,d6
		if d6<d1.w
			move.w	d6,d1
		else
			if d6>d3.w
				move.w	d6,d3
			endif
		endif
	dbra	d7,qwe

	and.w	#$FFF0,d0				* word align for faster bltpattern
	move.l	ai_VctrTbl(a2),a0		* get pointer to first guy in vector table
	move.w	ai_Count(a2),d7
	subq.w	#1,d7					* for dbne
qwe2:
		sub.w	d0,(a0)+			* relocate x
		sub.w	d1,(a0)+			* relocate y
	dbra	d7,qwe2

*
*	calculate bltbytes and pass it in
	move.l	d0,d6	* save here / minx is now in d6
	move.l	d1,d7	* miny is now in d7
	move.w	d2,d1	* bltbytes(d0,d1) (xmin,xmax)
	bsr		bltbytes	* returns in d0.l does not touch a0/a1
	move.l	d0,d4		* d4 = bltbytes=fillbytes, d0 = minx now

* set up local bitmap now
LOCAL_BM_SIZE	equ	2+2+1+1+2+4		* enough for 1 bitplane
LOCAL_BM		equ rp_SIZEOF		* offset from sp to start
LOCAL_RP		equ 0				* offset from sp to start
	lea		-LOCAL_BM_SIZE-rp_SIZEOF(sp),sp	* sp now points to bitmap

*	do the bltclear part here
	move.l	rp_TmpRas(a3),a1
	move.l	tr_RasPtr(a1),a1	* get pointer to buffer
*	set up Plane ptr in local BitMap
	move.l	a1,LOCAL_BM+bm_Planes(sp)
	move.w	d3,d0				* d0 = maxy
	sub.w	d7,d0				* subtract miny from d0
	addq.w	#1,d0				* add 1 to get height
	swap	d0					* put in upper word of d0
	move.w	d4,d0				* construct rows/bytes for bltclear
	moveq	#2,d1
	jsr		_LVOBltClear(a6)

	lea	LOCAL_BM(sp),a0					* &bitmap
	lea	LOCAL_RP(sp),a1					* &newras

*	initialize local RastPort for line draw
	clr.l	(a1)						* newras.Layer = 0
	move.l	a0,rp_BitMap(a1)			* newras.BitMap = &bitmap
	moveq	#-1,d0
	move.w	d0,rp_LinePtrn(a1)			* LinePtrs = -1
	move.b	d0,rp_Mask(a1)				* writemask = -1
	move.w	#ONE_DOT,rp_Flags(a1)		* Flags = ONE_DOT
	move.b	#NANBC+NABC+ANBC+ABNC,rp_minterms(a1)	* complement mode
	move.b	#RP_COMPLEMENT,rp_DrawMode(a1)		* complement mode

* initialize local bitmap
	moveq	#1,d0					* depth
	move.l	d4,d1
	asl.l	#3,d1					* width
	move.l	d2,-(sp)				* save d2.l on stack
	move.w	d3,d2
	sub.w	d7,d2
	addq.w	#1,d2					* height
	ext.l	d2
	jsr		_LVOInitBitMap(a6)
*	move.l	(sp)+,d2				* restore d2

	move.l	d4,-(sp)
LOCAL_OFFSET	equ 8
	move.l	ai_VctrTbl(a2),a4
	move.l	ai_FlagTbl(a2),a5
	clr.b	d4	* last_dir = FALSE	going_up = 0
*									going_down = 1
*									unknown = -1
	moveq	#1,d5			* frst_dir = ~last_dir (= TRUE )
	move.w	ai_Count(a2),d2	* counter
	if <>	.extend
		repeat
			lea	LOCAL_OFFSET+LOCAL_RP(sp),a1
			bset	#FRST_DOTn,rp_Flags+1(a1)
			move.w	(a4)+,d0		* get tmpx
			move.w	(a4)+,d1		* get tmpy
			btst	#MDFLAG1n,(a5)+
			if <>	.extend

			    btst    #EXTENDEDn,-1(a5)	* bart -04.28.86

			    if <>
*				process extended instruction
*				do switch

				jsr	extended
				subq.w	#1,d2 * adjust point count bart 05.05.89

			    else
*				not extended instruction
*				check if knee
				if d4.b=#0
					if rp_cp_y(a1)<d1.w
						bclr	#FRST_DOTn,rp_Flags+1(a1)
					else
						moveq	#1,d4
					endif
				else
					if d4.b>#0		* = 1?
						if rp_cp_y(a1)>d1.w
							bclr	#FRST_DOTn,rp_Flags+1(a1)
						else
							clr.b	d4
						endif
					else
						if rp_cp_y(a1)>d1.w
							moveq	#1,d4
						else
							clr.b	d4
						endif
					endif
				endif
				bsr	BareDraw
*				jsr	_LVODraw(a6)
				if d5.b<#0			* unknown? -1
					move.b	d4,d5	* frst_dir = last_dir
				endif
			    endif	* bart - 04.28.86
			else
				btst	#MDFLAG2n,-1(a5)
				if =
*				don't write any spurious pixels for extended instructions
				    btst    #EXTENDEDn,-1(a5)	* bart -04.28.86
				    if =

					if d4=d5.b
						if d4.b>=#0	* last_dir != -1
							movem.l	d0/d1/a1,-(sp)
							movem.w	rp_cp_x(a1),d0/d1
							jsr	_LVOWritePixel(a6)
							movem.l	(sp)+,d0/d1/a1
						endif
					endif
					moveq	#-1,d4	* unknown */
					move.b	d4,d5

				    endif
				endif
				bsr	_Move
*				jsr	_LVOMove(a6)
			endif
			subq.w	#1,d2
		until =
	endif
	lea	LOCAL_OFFSET+LOCAL_RP(sp),a1
	move.w	#FRST_DOT,rp_Flags(a1)
	move.b	#15,rp_linpatcnt(a1)
	if d4=d5.b
		if d4.b>=#0
			movem.w	rp_cp_x(a1),d0/d1
			jsr	_LVOWritePixel(a6)
		endif
	endif
	move.l	(sp)+,d4
	move.l	(sp)+,d2

*	d4 = fillbytes
*	pea		LOCAL_RP(sp)			* newras
*	move.l	a2,-(sp)				* AreaInfo pointer
*    move.l  a6,-(sp)    * push gfxbase on stack
*    jsr     _areaend	* do rest of routine now
*    lea     12(sp),sp    * reset stack


* now do the actual fill operation with the blitter
	move.l	LOCAL_BM+bm_Planes(sp),a1	* get RasPtr again
	move.w	LOCAL_BM+bm_Rows(sp),d1

	lea	_custom,a0
	add.l	d4,a1
	subq.l	#2,a1		* first i
	moveq	#-1,d0		* for fw/lwmask
	move.w	d4,d5
	add.w	d5,d5
	neg.w	d5		* for bltmdc/d
	OWNBLITTER
	WAITBLITDONE a0
	move.l	a1,bltptc(a0)
	move.l	a1,bltptd(a0)
	move.l	d0,fwmask(a0)	* and lwmask
	move.w	d5,bltmdc(a0)
	move.w	d5,bltmdd(a0)
	move.w	#NANBC+NABC+ANBC+ABC+DEST+SRCC,bltcon0(a0)
	move.w	#FILL_XOR+BLITREVERSE,bltcon1(a0)

	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	if =
*		old chips
		asl.w	#6,d1		* put row in correct place
		moveq	#$7f,d5
		and.w	d4,d5		* |(0x3f&(fillbytes>>1))
		asr.w	#1,d5
		or.w	d5,d1		* construct bltsize
		move.w	d1,bltsize(a0)
	else
*		new chips
		move.w	d4,d5
		asr.w	#1,d5
		and.w	#$7ff,d5
		and.w	#$7fff,d1
		move.w	d1,bltsizv(a0)
		move.w	d5,bltsizh(a0)
	endif
	DISOWNBLITTER

* now fill in any extra missing dots around border
	lea	LOCAL_RP(sp),a1
	clr.b	rp_DrawMode(a1)	* back to jam mode
	if #2=rp_DrawMode(a3)
*		if complement then must clip away the border
		move.b	#NANBC+NABC,rp_minterms(a1)
	else
		move.b	#NANBC+NABC+ANBNC+ANBC+ABNC+ABC,rp_minterms(a1)
	endif
* process list again and do standard line draw around edge
	move.l	ai_VctrTbl(a2),a4
	move.l	ai_FlagTbl(a2),a5
	move.w	ai_Count(a2),d5
	if <>
		repeat
			lea	LOCAL_RP(sp),a1
			move.w	(a4)+,d0
			move.w	(a4)+,d1
			btst	#MDFLAG2n,(a5)+
			if <>

*			    test for extended instruction
			    btst    #EXTENDEDn,-1(a5)	* bart -04.28.86

			    if <>

*				process extended instruction

				jsr	extended
				subq.w	#1,d5 * adjust point count bart 05.05.89

			    else

*				not extended instruction
				bsr	BareDraw
*				jsr	_LVODraw(a6)

			    endif
			else
				bsr	_Move
*				jsr	_LVOMove(a6)
			endif
			subq.w	#1,d5
		until =
	endif
* end new downcode

	move.l	LOCAL_BM+bm_Planes(sp),a0	* get RasPtr again

	lea		LOCAL_BM_SIZE+rp_SIZEOF(sp),sp	* drop local bm from stack

*	downcoded stuff from end of areaend.c
*	remember	a2 points to AreaInfo
*				a3 points to RastPort
*				d7 is miny
*				d6 is minx
*				d2 in maxx
*				d3 is maxy
*				d4 is fillbytes
	move.l		d6,d0	* minx
	move.l		d7,d1	* miny
	move.l		a3,a1	* w (RastPort)
*						* a0 already set up
	jsr		_LVOBltPattern(a6)
	btst	#RPB_AREAOUTLINE,rp_Flags+1(a3)
	if <>
		move.l	d7,-(sp)			* miny
		move.l	d6,-(sp)			* minx
		move.l	a3,-(sp)			* RastPort *w
		jsr		_draw_fill_outline
		lea		12(sp),sp
	endif
	moveq	#0,d0		* no error return
ae_exit:
	clr.w	ai_Count(a2)
	move.l	ai_VctrTbl(a2),ai_VctrPtr(a2)
	move.l	ai_FlagTbl(a2),ai_FlagPtr(a2)

	movem.l	(sp)+,d2-d7/a2-a5
    rts

*********************  process extended instructions  **************************

extended:

	cmpi.b	#CIRCLE,(a5)	

	if =
*					process circle
					    
		movem.l	d2/d3,-(sp)	* save regs
		
		move.w	(a4)+,d2	* get xmax
		move.w	(a4)+,d3	* get ymax

*		bart - 05.05.86 change in args to DrawEllipse 

		add.w	d2,d0
		asr.w	#1,d0		* cx in d0

		add.w	d3,d1
		asr.w	#1,d1		* cy in d1

		sub.w	d0,d2		* a in d2
		sub.w	d1,d3		* b in d3

*		end bart - 05.05.86 change in args to DrawEllipse 

		jsr	_LVODrawEllipse(a6)	* do ellipse

		movem.l	(sp)+,d2/d3	* restore regs

		bra.s	ext_done
	endif

not_circle:

	cmpi.b	#ELLIPSE,(a5)	

	if =
*					process ellipse

		movem.l	d2/d3,-(sp)	* save regs

		move.w	(a4)+,d2	* get xmax
		move.w	(a4)+,d3	* get ymax

*		bart - 05.05.86 change in args to DrawEllipse 

		add.w	d2,d0
		asr.w	#1,d0		* cx in d0

		add.w	d3,d1
		asr.w	#1,d1		* cy in d1

		sub.w	d0,d2		* a in d2
		sub.w	d1,d3		* b in d3

*		end bart - 05.05.86 change in args to DrawEllipse 

		jsr	_LVODrawEllipse(a6)	* do ellipse

		movem.l	(sp)+,d2/d3	* restore regs

		bra.s	ext_done
	endif

not_ellipse:
	tst.l	(a4)+			* increment a4 past second "point"
ext_done:
	addq.l	#1,a5 	* bart - 04.28.86 point to next

	rts

*********************  end processing extended instructions  *******************

	end
