*******************************************************************************
*
*	$Id: draw.asm,v 39.1 92/04/27 14:30:10 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'graphics/gfxbase.i'
    include 'graphics/gfx.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/ports.i'
    include 'exec/interrupts.i'
    include 'exec/libraries.i'

    include 'graphics/clip.i'
    include 'graphics/rastport.i'
    include 'hardware/blit.i'
    include 'hardware/custom.i'
    include 'submacs.i'
    include '/sane_names.i'

    section	graphics
    xdef    _Draw
    xref    cdraw
    xref    waitblitdone

    xref    _custom

******* graphics.library/Draw *************************************************
*
*   NAME
*       Draw -- Draw a line between the current pen position
*                       and the new x,y position.
*
*   SYNOPSIS
*       Draw( rp,   x,     y)
*             a1  d0:16  d1:16
*
*	void Draw( struct RastPort *, SHORT, SHORT);
*
*   FUNCTION
*       Draw a line from the current pen position to (x,y).
*
*   INPUTS
*
*	rp - pointer to the destination RastPort
*	x,y - coordinates of where in the RastPort to end the line.
*
*   BUGS
*
*   SEE ALSO
*	Move() graphics/rastport.h
*
******************************************************************************


doflat:
*   do fast horizontal lines
*   this goes left to right
    btst    #ONE_DOTn,rp_Flags+1(a1)
    bne     nodoflat
*   begin reinstalling fast horizontal lines
*   bra     nodoflat

    move.w  d0,d1	* scratch
    move.w  d2,d5	* scratch
    add.w   d5,d1       * ending x position
    asr.w   #3,d5       * get x byte address in a row
    ext.l   d5
    asr.w   #1,d5
    lsl.w   #4,d5
    sub.w   d5,d1
    asr.w   #4,d1
    addq.w  #1,d1
    or.w    #$40,d1     * 1 row plus this many words d0= blitsize
    cmp.w   #$40,d1
    ble.s   continue_flat
    clr.w   d1		* restore
    bra	    nodoflat	* too big for now -- resort to alternate method
continue_flat:

    move.w  rp_LinePtrn(a1),d7  * get drawing pattern
    move.b  rp_linpatcnt(a1),d1
    sub.b   d0,rp_linpatcnt(a1)
    moveq   #15,d5
    sub.w   d5,d1
    add.w   d2,d1
    and.w   d5,d1          * this line may not be needed?
    ror.w   d1,d7

    move.w  d7,d6
    moveq   #-1,d7
    lsr.w   d4,d7
    lea     _custom,a0
    OWNBLITTER
    WAITBLITDONE a0
*    WAITBLITDONE a0
    move.w  d7,fwmask(a0)
    moveq   #-1,d7
    add.w   d2,d0           * ending x position
    move.w  d5,d1
    sub.w   d0,d1
    and.w   d5,d1          * this line may be needed?
    asl.w   d1,d7
    move.w  d7,lwmask(a0)
    asr.w   #3,d2           * get x byte address in a row
    ext.l   d2
    add.l   d2,d3           * address offset into raster planes
    asr.w   #1,d2
    lsl.w   #4,d2
    sub.w   d2,d0
    asr.w   #4,d0
    addq.w  #1,d0
    or.w    #$40,d0     * 1 row plus this many words d0= blitsize
    move.w  d4,d2   * save for compit
    move.w  #DEST+SRCC,d4
    move.w  #$FFFF,adata(a0)
    clr.l   d5
    bra     flatback

_Draw:
*	rp_layer must be 0 offset of layer
	ifne	rp_Layer
		fail
	endc
	tst.l	(a1)
	bne		cdraw

; void __asm baredraw(register __a1 struct RastPort *rp, register __d0 LONG x, register __d1 LONG y);
_baredraw::
BareDraw::
    movem.l d2-d7/a2/a3,-(sp)    * save registers on stack
    movem.w rp_cp_x(a1),d2/d3   * get cp_x and cp_y
    movem.w d0/d1,rp_cp_x(a1)   * new cp_x and cp_y

    move.l  rp_BitMap(a1),a2

    moveq   #15,d4
    and.w   d2,d4       * bit position b

    sub.w   d3,d1       * dy = y - cpy
    ifne	bm_BytesPerRow
	    fail
    endc
    mulu    (a2),d3   * bperrow time number of rows
    sub.w   d2,d0       * dx = x - cpx
    if >=
		if d1>=#0.w
	    	beq         doflat
nodoflat:
	    	if d0>d1.w
				moveq  #LINEMODE+OCTANT1,d7
	    	else
				moveq  #LINEMODE+OCTANT2,d7
				exg d0,d1
	    	endif
		else
	    	neg.w   d1  * dy = -dy
	    	if d0>d1.w
				moveq  #LINEMODE+OCTANT8,d7
	    	else
				moveq  #LINEMODE+OCTANT7,d7
				exg d0,d1
	    	endif
		endif
    else
		neg.w   d0      * dx = -dx
		if d1>=#0.w
	    	if d0>d1.w
				moveq  #LINEMODE+OCTANT4,d7
	    	else
				moveq  #LINEMODE+OCTANT3,d7
				exg d0,d1
	    	endif
		else
	    	neg.w   d1  * dy = -dy
	    	if d0>d1.w
				moveq  #LINEMODE+OCTANT5,d7
	    	else
				moveq  #LINEMODE+OCTANT6,d7
				exg d0,d1
	    	endif
		endif
    endif
    add.w   d1,d1       * use dy as 'i' in bresblit

*   now using d1 as 'i' and d0 as dx or count
    asr.w   #3,d2           * get x byte address in a row
    ext.l   d2
    add.l   d2,d3           * address offset into raster planes
* calculate initial error term
    move.w  d1,d2
    sub.w   d0,d2           * d2 = initial error term
    if <
		or.w    #SIGNFLAG,d7
    endif
    btst    #ONE_DOTn,rp_Flags+1(a1)
    if <>
		or.w    #ONEDOT,d7
    endif
*   load up ptr to registers in a0
    lea _custom,a0

    move.w  d2,a3               * bltpta reloaded each pass
    move.w  d1,d6 		* save for later use
    sub.w   d0,d2
    move.w  d2,d1
    swap    d2			* stash in upper word 

    OWNBLITTER
    WAITBLITDONE a0

    move.w  d6,bltmdb(a0)
    swap    d6			* stash in upper word 
    move.w  rp_LinePtrn(a1),d6  * reloaded each pass
    move.w  d1,bltmda(a0)

    moveq.l #-1,d1
    move.l  d1,fwmask(a0)   * and ar_lwmask
    move.w  #$8000,adata(a0)
    move.w  (a2),bltmdc(a0)

    move.b  rp_linpatcnt(a1),d1 * reusing d1

	swap    d1
    clr.w   d1
    asr.l   #4,d1
    or.w    d1,d7               * put preshift for pattern in con1
    sub.b   d0,rp_linpatcnt(a1) * update to next shift value
    move.w  d7,d5

    move.w  d4,d2       * save here for compit
    swap    d4
    asr.l   #4,d4       * a real 12 bit left shift
    or.w    #DEST+SRCA+SRCC,d4

*   setup bltsize
    addq.w  #1,d0
    ext.l	d0
    asl.l   #HSIZEBITS,d0
    addq.w  #2,d0
    if #$10002<d0.l
	swap    d4
	move.w  #15,d4
	sub.w   d2,d4	* save bit offset for big_pain
	swap    d4
	bra	big_line
    endif

flatback:
    clr.w   d1                  * i

*   next statement true only for FRSTDOTn<8
    bclr    #FRST_DOTn,rp_Flags+1(a1)
    if =	* first dot?
		cmp.b	#2,rp_DrawMode(a1)
		beq.s	compit	* must recomplement first dot
    endif

    move.b  bm_Depth(a2),d2         * count to
    lea     bm_Planes(a2),a2    * get ptr to raster table
    repeat
		move.l  (a2)+,d7    * get a ptr to a raster
		btst    d1,rp_Mask(a1)
		if <>
	    	swap    d5
	    	move.w  d4,d5
	    	move.b  rp_minterms(a1,d1.w),d5
	    	swap    d5
	    	add.l   d3,d7   * d7 is destination ptr
	    	WAITBLITDONE a0
*    WAITBLITDONE a0
	    	move.l  d5,bltcon0(a0)  * and bltcon1
	    	move.w  a3,bltpta+2(a0)
	    	move.l  d7,bltptc(a0)
	    	move.l  d7,bltptd(a0)
	    	move.w  d6,bdata(a0)    * and draw thru bdata shifter
	    	move.w  d0,bltsize(a0)  * start blit
		endif
		addq.b  #1,d1
    until d1=d2.b
compit_return:
    DISOWNBLITTER
    movem.l (sp)+,d2-d7/a2/a3    * restore parameters from stack
    rts

compit:
*           have to reblast this dot, but what about the current pattern?
* this code uses d2 differently
    moveq   #15,d1
    sub.w   d2,d1
    move.w  d1,d2

    clr.w   d1                  * i
	move.l	a4,-(sp)
	move.l	a2,a4
    lea     bm_Planes(a4),a2    * get ptr to raster table
    repeat
		move.l  (a2)+,d7    * get a ptr to a raster
		btst    d1,rp_Mask(a1)
		if <>
	    	swap    d5
	    	move.w  d4,d5
	    	move.b  rp_minterms(a1,d1.w),d5
	    	swap    d5
	    	add.l   d3,d7   * d7 is destination ptr
	    	move.l  a3,-(sp)
	    	move.l  d7,a3
	    	bchg    d2,(a3)
	    	move.l  (sp)+,a3
	    	WAITBLITDONE a0
*    WAITBLITDONE a0
	    	move.w  a3,bltpta+2(a0)
	    	move.l  d7,bltptc(a0)
	    	move.l  d7,bltptd(a0)
	    	move.l  d5,bltcon0(a0)  * and bltcon1
	    	move.w  d6,bdata(a0)    * and draw thru bdata shifter
	    	move.w  d0,bltsize(a0)  * start blit
		endif
		addq.b  #1,d1
    until bm_Depth(a4).b=d1
	move.l	(sp)+,a4
    bra compit_return

big_line:
    clr.w   d1                  * i

*   next statement true only for FRSTDOTn<8
    bclr    #FRST_DOTn,rp_Flags+1(a1)
    if =	* first dot?
		cmp.b	#2,rp_DrawMode(a1)
		beq	bigcompit	* must recomplement first dot
    endif

    move.b  bm_Depth(a2),d2         * count to
    lea     bm_Planes(a2),a2    * get ptr to raster table
    repeat
		move.l  (a2)+,d7    * get a ptr to a raster
		btst    d1,rp_Mask(a1)
		if <>	.extend
    			swap	d5
			exg	a3,d4
			move.w	d4,d5
    			swap	d4
			move.w	d5,d4
    			swap	d4
			exg	a3,d4
		    	move.w  d4,d5
		    	move.b  rp_minterms(a1,d1.w),d5
		    	swap    d5
		    	add.l   d3,d7   * d7 is destination ptr
		    	WAITBLITDONE a0
*    WAITBLITDONE a0
		    	move.l  d5,bltcon0(a0)  * and bltcon1
		    	move.w  a3,bltpta+2(a0)
		    	move.l  d7,bltptc(a0)
		    	move.l  d7,bltptd(a0)
		    	move.w  d6,bdata(a0)    * and draw thru bdata shifter
			move.l	d0,-(sp) 	* save total bltsize
			repeat
			move.w	#2,bltsize(a0)
			btst    #ONE_DOTn,rp_Flags+1(a1) * one_dot ?
			beq.s   bigline_cnt
			btst	#4,d5		* horizontal ?
			beq.s   bigline_cnt
			jsr	big_pain	* better be careful
bigline_cnt:		sub.l	#$10000,d0
			WAITBLITDONE a0
			until	#$10002>d0.l
			cmp.l	#$42,d0
			blt.s	bigline_exact
		    	move.w  d0,bltsize(a0)  * start blit
bigline_exact:		move.l (sp)+,d0		* restore total bltsize
		endif
		addq.b  #1,d1
    until d1=d2.b
bigcompit_return:
    DISOWNBLITTER
    movem.l (sp)+,d2-d7/a2/a3    * restore parameters from stack
    rts

bigcompit:
*           have to reblast this dot, but what about the current pattern?
* this code uses d2 differently
    moveq   #15,d1
    sub.w   d2,d1
    move.w  d1,d2

    clr.w   d1                  * i
	move.l	a4,-(sp)
	move.l	a2,a4
    lea     bm_Planes(a4),a2    * get ptr to raster table
    repeat
		move.l  (a2)+,d7    * get a ptr to a raster
		btst    d1,rp_Mask(a1)
		if <>	.extend
		    	swap    d5
			exg	a3,d4
			move.w	d4,d5
			swap	d4
			move.w	d5,d4
    			swap	d4
			exg	a3,d4
		    	move.w  d4,d5
		    	move.b  rp_minterms(a1,d1.w),d5
		    	swap    d5
		    	add.l   d3,d7   * d7 is destination ptr
		    	move.l  a3,-(sp)
		    	move.l  d7,a3
		    	bchg    d2,(a3)
		    	move.l  (sp)+,a3
		    	WAITBLITDONE a0
*    WAITBLITDONE a0
		    	move.w  a3,bltpta+2(a0)
		    	move.l  d7,bltptc(a0)
		    	move.l  d7,bltptd(a0)
		    	move.l  d5,bltcon0(a0)  * and bltcon1
		    	move.w  d6,bdata(a0)    * and draw thru bdata shifter
			move.l	d0,-(sp) 	*s ave total bltsize
			repeat
			move.w	#2,bltsize(a0)
			btst    #ONE_DOTn,rp_Flags+1(a1) * one_dot ?
			beq.s   bigcomp_cnt
			btst	#4,d5		* horizontal ?
			beq.s   bigcomp_cnt
			jsr	big_pain	* better be careful
bigcomp_cnt:    	sub.l	#$10000,d0
			WAITBLITDONE a0
			until	#$10002>d0.l
			cmp.l	#$42,d0
			blt.s	bigcomp_exact
			move.w  d0,bltsize(a0)  * start blit
bigcomp_exact:		move.l (sp)+,d0		* restore total bltsize
		endif
		addq.b  #1,d1
    until bm_Depth(a4).b=d1
	move.l	(sp)+,a4
    bra bigcompit_return


*	ok, the hardware may be in ONEDOT mode. restarting this line will
*	render an extraneous dot unless the bresenham step has just occurred.
*	so, we may have to modify the initial pixel for each succesive segment
*	unless we are just about to render the ONEDOT. yuck! bart

*	a cusp condition (B10406) can occur when final error term is
*	exactly 4y -2x, upon blitter restart an extraneous bit is blitted
*	to fix this we must complement the extraneous bit as well. bart

big_pain:	
	movem.l d0-d2/a0,-(sp)		* preserve context
	moveq	#0,d0
	swap	d4
	move.w	d4,d2			* recover offset into byte
	swap	d4
	exg	a3,d4
	swap	d2			* retrieve 2y-2x from upper word
	swap	d4			* retrieve error from upper word
	swap	d6			* retrieve 2y    from upper word
	move.w	#1024,d1
	bset	#31,d0 
	tst.w	d4
	bra.s	doit
loop    bgt.s	down
	bset	#31,d0
	bne.s	next
	add.w	#1,d0
next	add.w   d6,d4
	bra.s	doit
down	bclr	#31,d0
	add.w   d2,d4
doit	dbra    d1,loop
	bgt.s	just			* if test greater than more
	bchg	#31,d0			* if just negative will set bit
	bne.s	just
	add.w	d6,d2			* calculate 4y - 2x
	cmp.w	d4,d2			* if error = 4y - 2x then
	bne.s	fixx			* blitter cusp condition exists
	addq.w	#1,d0			* so guaranteed positive, fall thru
just	bmi.s	fixx			* if test negative we are all done
	bclr	#31,d0			* if just greater or equal clear bit
fixx	swap	d2
	swap	d4					
	swap	d6
	exg	a3,d4
	btst	#31,d0
	bne.s	brts
	move.l  rp_BitMap(a1),a0
	mulu	(a0),d0			* bytesperrow * rows
	btst	#3,d5
	beq.s	ypos
	neg.l	d0
ypos    moveq	#0,d1
	move.w	d2,d1
	btst	#2,d5			* octant xdirection?
	beq.s	xpos
xneg	sub.l	#128,d0
	bra.s	padd
xpos	add.l	#128,d0
padd    add.l   d0,d7
	move.l  d7,a0			* point to the pix
	bchg    d1,(a0)			* complement pixel
brts	movem.l (sp)+,d0-d2/a0 		* restore context
	rts

	end
