******************************************************************************
*
*	Source Control
*	--------------
*	$Id: rpa.asm,v 1.8 92/01/28 14:15:26 davidj Exp $
*
*	$Locker:  $
*
*	$Log:	rpa.asm,v $
*   Revision 1.8  92/01/28  14:15:26  davidj
*   compiled native
*   
*   Revision 1.7  90/07/27  02:19:25  bryce
*   The #Header line is a real pain; converted to #Id
*
*   Revision 1.6  90/04/06  19:25:22  daveb
*   for rcs 4.x header change
*
*   Revision 1.5  88/03/15  15:55:04  daveb
*   changed add.w to add.l in rp2rp which prevented bit planes with more than
*   32K of data from dumping correctly.
*   V1.3 Gamma 9 release
*
*   Revision 1.4  88/02/16  16:11:10  daveb
*   removed spurious autodocs
*
*   Revision 1.3  88/01/30  12:22:04  daveb
*   V1.3 Gamma 7 release
*
*   Revision 1.2  87/12/21  10:48:49  daveb
*   fixed bug in rp2rp which assumed that the rastport had a layer
*   V1.3 Gamma 5 release
*
*   Revision 1.1  87/10/27  15:51:17  daveb
*   added support for non-obscured non-chip ram
*   V1.3 gamma 1 check-in
*
*   Revision 1.0  87/08/21  17:28:25  daveb
*   added to rcs
*
******************************************************************************

	include 'exec/types.i'     	* Required data type definitions
	include 'graphics/gfx.i'    	* BitMap structure definition
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/interrupts.i'
	include 'exec/ports.i'
	include 'exec/libraries.i'
	include 'graphics/gfxbase.i'
	include 'graphics/clip.i'   	* Layer structure definition
	include 'graphics/rastport.i'	* RastPort structure definition
	include 'hardware/blit.i'   	* Minterm definitions
	include 'submacs.i'		* Macros for subroutines called

	section	graphics

	xdef _ReadPixelArray  * Define public entry points
	xdef _ReadPixelLine
	xdef _rp2rp

	PAGE

; graphics.library/ReadPixelArray **************************************
*
*   NAME
*       ReadPixelArray -- read the pen number value of a rectangular array
*		of pixels starting at a specified x,y location and continuing
*		through to another x,y location within a certain RastPort.
*
*   SYNOPSIS
*     count = ReadPixelArray(rp,xstart,ystart,xstop,ystop,array,temprp,noblit)
*       d0                    a0 d0:16  d1:16  d2:16 d3:16  a2   a1      d4
*
*	LONG	count;
*	struct	RastPort *rp;
*	UWORD	xstart, ystart;
*	UWORD	xstop, ystop;
*	UWORD	*array;
*	struct	RastPort *temprp;
*
*   FUNCTION
*		For each pixel in a rectangular region, combine the bits from each
*		of the bit-planes used to describe a particular RastPort into the pen
*		number selector which that bit combination normally forms for the
*       system hardware selection of pixel color.
*
*   INPUTS
*       rp -  pointer to a RastPort structure
*       (xstart,ystart) starting point in the RastPort
*       (xstop,ystop) stopping point in the RastPort
*		array - pointer to an array of uwords in which to store the pixel data
*		temprp - temporary rastport (copy of rp with Layer set == NULL,
*									temporary memory allocated for temp BitMap
*									temprp->BitMap with Rows set == 1,
*									and with BytesPerrow set ==
*									((((xstart-xstop+1)+0xf)>>4)<<1),
*									and temporary memory allocated for
*									temprp->BitMap->Planes[])
*       noblit - 0-ok to use blitter, 1-do not use blitter
*
*   RESULT
*		For each pixel in the array:
*       	Pen - (0..4095) number at that position is returned
*
*	NOTE
*		xstop must be >= xstart
*		ystop must be >= ystart
*
*   BUGS
*
*   SEE ALSO
*       WritePixel	graphics/rastport.h
*
******************************************************************************
	xref	_LVOWaitBlit
	xref	_LVOBltClear
	xref	_LVOClipBlit
	xref	_GfxBase
	xref	_LVODebug
	xref	_AbsExecBase

_ReadPixelArray:
	movem.l	d2/d3/d4/a2/a6,-(sp)
	move.l	_GfxBase,a6
	move.l  24(sp),a0			; rp
	movem.l	28(sp),d0/d1/d2/d3	; xstart, ystart, xstop, ystop
	movea.l	44(sp),a2			; array
	movea.l	48(sp),a1			; temprp
	move.l	52(sp),d4			; noblit
	jsr readpixelarray
	movem.l	(sp)+,d2/d3/d4/a2/a6
	rts

readpixelarray:

	movem.l	d2/d3/d4/d5/d6/d7/a2/a3/a4,-(sp)

	sub.w	d0,d2		; width of rectangle to read
	ext.l	d2
	addq.l	#1,d2
	ble.s	array_error ; width non-positive ?
	move.l	d0,d7		; temp storage for current x
	move.l	d1,d5		; temp storage for current y

	sub.w	d1,d3		; height of rectangle to read
	ext.l	d3
	addq.l	#1,d3
	ble.s	array_error ; height non-positive ?

	movea.l	a0,a3		; protect rastport pointer -- does not modify cc
	movea.l	a1,a4		; protect temprp pointer -- does not modify cc

	moveq.l	#0,d6		; clear count of pixels returned

	bra.s	rpxaloop_count

rpxaloop_start:

	movea.l	a3,a0		; rp
	move.l	d7,d0		; x
	move.l	d5,d1		; y (current line)
						; width in d2 (see above)
						; array in a2 (see above)
	movea.l	a4,a1		; temprp

	jsr	readpixelline

	addq.l	#1,d5		; next line
	adda.l	d2,a2		; increment base pointer to array by width words
	adda.l	d2,a2
	add.l	d2,d6		; add one line of pixels to pixel count

rpxaloop_count:

	dbra	d3,rpxaloop_start

rpxaloop_end:

	move.l	d6,d0		; return pixel count
	movem.l	(sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
	rts

array_error:

	moveq.l	#-1,d0
	movem.l	(sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
	rts

; graphics.library/ReadPixelLine ***************************************
*
*   NAME
*       ReadPixelLine -- read the pen number value of a horizontal line
*		of pixels starting at a specified x,y location and continuing
*		right for count pixels.
*
*   SYNOPSIS
*       count = ReadPixelLine(rp,xstart,ystart,width,array,temprp,noblit)
*       d0                    a0 d0:16  d1:16   d2     a2    a1    d4
*
*	LONG	count;
*	struct	RastPort *rp;
*	UWORD	xstart, ystart;
*	UWORD	width;
*	UWORD	*array;
*	struct	RastPort *temprp;
*
*   FUNCTION
*	For each pixel in a rectangular region, combine the bits from each
*	of the bit-planes used to describe a particular RastPort into the pen
*	number selector which that bit combination normally forms for the
*       system hardware selection of pixel color.
*
*   INPUTS
*       rp -  pointer to a RastPort structure
*       (x,y) a point in the RastPort
*		width - count of horizontal pixels to read
*		array - pointer to an array of uwords in which to store the pixel data
*		temprp - temporary rastport (copy of rp with Layer set == NULL,
*					 temporary memory allocated for
*					 temprp->BitMap with Rows set == 1,
*					 and temporary memory allocated for
*					 temprp->BitMap->Planes[])
*	       noblit - 0-ok to use blitter, 1-do not use blitter
*
*   RESULT
*		For each pixel in the array:
*       	Pen - (0..4095) number at that position is returned
*
*   NOTE
*		width must be non negative
*
*   BUGS
*
*   SEE ALSO
*       WritePixel	graphics/rastport.h
*
******************************************************************************

_ReadPixelLine:
	movem.l	d2/d3/d4/a2/a6,-(sp)
	move.l	_GfxBase,a6
	move.l  24(sp),a0		; rp
	movem.l	28(sp),d0/d1/d2		; xstart, ystart, width
	movea.l	40(sp),a2		; array
	movea.l	44(sp),a1		; temprp
	move.l	48(sp),d4		; noblit
	jsr readpixelline
	movem.l	(sp)+,d2/d3/d4/a2/a6
	rts

readpixelline:

	movem.l	d2/d3/d4/d5/d6/a2/a3,-(sp)

******************************************************************************

	tst	d4				; ok to use blitter?
	beq useblit				; yes

	movem.l	a0/a1,-(sp)
	bsr	rp2rp				; 68000 clipblit (sort of).
	movem.l	(sp)+,a0/a1
	move.l	d2,d4
	bra.s	afterblit

useblit:
	movem.l	d0/d1/a0/a1,-(sp)	; save xstart, ystart, rp, temprp
	movea.l	rp_BitMap(a1),a1	; get BitMap ptr in a1
	clr.l	d0
	move.w	bm_BytesPerRow(a1),d0	; # of bytes/plane
	clr.l	d1
	move.b	bm_Depth(a1),d1		; # of planes
	mulu	d1,d0			; total # of bytes to clear in d0
	lea.l	bm_Planes(a1),a1	; address of plane pointer array in a1
	movea.l	(a1),a1			; address of first plane ptr in a1
	clr.l	d1			; dont wait for blit to finish
	jsr	_LVOBltClear(a6)	; clear tmpras the easy/quick way
	movem.l	(sp)+,d0/d1/a0/a1	; restore xstart, ystart, rp, temprp

******************************************************************************

					; xstart, ystart in d0, d1
					; rp, temprp in a0, a1
	move.l	d2,d4		; save "width" pixels, copy this many pixels
	moveq.l	#1,d5			; copy one row of pixels
	move.l	#(ABC+ABNC),d6		; vanilla copy
	moveq.l	#0,d2			; copy to temprp(0,0)
	moveq.l	#0,d3
	move.l	a1,-(sp)		; save temprp
	jsr	_LVOClipBlit(a6)	; copy rp to temprp the easy/quick way

	move.l	(sp)+,a1		; restore temprp
					; width now in d4

******************************************************************************
; a1 - temprp
; a2 - array
; d4 - width
afterblit:
	movea.l	rp_BitMap(a1),a1	; get BitMap ptr
	clr.l	d1
	move.b	bm_Depth(a1),d1		; get depth (# of bitplanes)
	subq.b	#1,d1
	lsl.b	#2,d1			; convert to longword index
	lea.l	bm_Planes(a1),a0	; address of first plane pointer in a0
	add.w	#15,d4
	lsr.w	#4,d4			; convert pixel width to word width
	subq.w	#1,d4			; adjust for dbra

	move.l	d4,d2			; copy of width in words in d2
	movea.l	0(a0,d1),a1		; get addr of last plane ptr in a1
	movea.l	a2,a3			; copy of dest ptr in a3
	clr.l	d5			; clear all bits for later use

outer_loop1:

	move.w	(a1)+,d5		; get source data, anything in it?
	beq		zero1		; nope, so just zero the dest

	; shoot, I guess we're just gonna have to figure out what's in it!
	moveq.l	#0,d3			; 1
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 2
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 3
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 4
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 5
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 6
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 7
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 8
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 9
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 10
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 11
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 12
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 13
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 14
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 15
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word
	moveq.l	#0,d3			; 16
	roxl.w	#1,d5			; put left-most bit in extend bit
	addx.w	d3,d3			; put extend bit in right-most bit
	move.w	d3,(a3)+		; init word

outer_count1:

	dbra	d2,outer_loop1		; go back while more words to do

	bra		plane_count	; do rest of planes (if any)

zero1:	; source word (16 bits) is zero, clear next 16 dest words; d5 = 0

	move.l	d5,(a3)+		; 1 2
	move.l	d5,(a3)+		; 3 4
	move.l	d5,(a3)+		; 5 6
	move.l	d5,(a3)+		; 7 8
	move.l	d5,(a3)+		; 9 10
	move.l	d5,(a3)+		; 11 12
	move.l	d5,(a3)+		; 13 14
	move.l	d5,(a3)+		; 15 16
	bra		outer_count1

zero2:	; source word (16 bits) is zero, shift next 16 dest words

	lsl.w	(a3)+			; 1
	lsl.w	(a3)+			; 2
	lsl.w	(a3)+			; 3
	lsl.w	(a3)+			; 4
	lsl.w	(a3)+			; 5
	lsl.w	(a3)+			; 6
	lsl.w	(a3)+			; 7
	lsl.w	(a3)+			; 8
	lsl.w	(a3)+			; 9
	lsl.w	(a3)+			; 10
	lsl.w	(a3)+			; 11
	lsl.w	(a3)+			; 12
	lsl.w	(a3)+			; 13
	lsl.w	(a3)+			; 14
	lsl.w	(a3)+			; 15
	lsl.w	(a3)+			; 16
	bra		outer_count2

plane_loop:

	move.l	d4,d2			; copy of width in words in d2
	movea.l	0(a0,d1),a1		; get addr of next plane ptr in a1
	movea.l	a2,a3			; copy of dest ptr in a3

outer_loop2:

	move.w	(a1)+,d5		; get source data, anything in it?
	beq		zero2		; nope, so just left-shift the dest

	; shoot, I guess we're just gonna have to figure out what's in it!
	roxl.w	#1,d5			; put left-most bit in extend bit 1
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 2
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 3
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 4
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 5
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 6
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 7
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 8
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 9
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 10
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 11
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 12
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 13
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 14
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 15
	roxl.w	(a3)+			; put extend bit in right-most bit
	roxl.w	#1,d5			; put left-most bit in extend bit 16
	roxl.w	(a3)+			; put extend bit in right-most bit
outer_count2:
	dbra	d2,outer_loop2		; go back while more words to do
plane_count:
	subq.b	#4,d1			; decrement long offset
	bge		plane_loop	; go back while more planes to do

	movem.l	(sp)+,d2/d3/d4/d5/d6/a2/a3
	rts

;=============================================================================
; rp2rp( xstart, ystart, width, rp1, rp2 )
;          d0      d1      d2    a0   a1
;
; Transfer width pixels at xstart,ystart from rp1 to position 0,0 in rp2.
; The second rastport is assumed to have as many planes as the first.
;=============================================================================
_rp2rp:
		move.l	d2,-(sp)		'C' entry point
		movem.l	8(sp),d0-d2/a0-a1
		bsr.s	rp2rp
		move.l	(sp)+,d2
		rts

rp2rp:
		movem.l	d2-d5/a2-a3,-(sp)
		movea.l	rp_Layer(a0),a2
		cmpa.l	#0,a2		;does this rastport have a layer?
		beq	nolayer		;no, so bypass minx,miny inclusion
		add.w	lr_MinX(a2),d0	;don't forget the x offset
		add.w	lr_MinY(a2),d1	;don't forget the y offset
nolayer:
		movea.l	rp_BitMap(a0),a0	a0=source bitmap
		mulu.w	bm_BytesPerRow(a0),d1
		moveq.l	#7,d3			calculate bit offset
		and.w	d0,d3			d3 = #left shifts required
		lsr.w	#3,d0
		add.l	d1,d0			d0=byte offset to source bits
		clr.w	d1
		move.b	bm_Depth(a0),d1		d1=#planes to do
		lea.l	bm_Planes(a0),a0	(a0)=first source bitmap
		movea.l	rp_BitMap(a1),a1
		lea.l	bm_Planes(a1),a1	(a1)=first dest bitmap
		addq.w	#7,d2			round width up to nearest byte
		lsr.w	#3,d2			d2 = bytes per row for xfer
		bra.s	OuterEnd

; main loop for each plane, transfer bytes to the destination rastport (rp2)
OuterStart:
		movea.l	(a0)+,a2		a2 = source
		adda.l	d0,a2			offset to correct byte
		movea.l	(a1)+,a3		a3 = destination
		move.w	d2,d4			d4 = byte counter
		bra.s	20$
10$		move.b	(a2)+,(a3)+		transfer bytes
20$		dbra	d4,10$

; now we have transferred the bytes we must shift them to the correct place
; (a3) is pointing 1 byte beyond the end of the destination bitplane.
		move.w	d3,d5			shift count
		bra.s	InnerEnd
InnerStart:
		movea.l	a3,a2			start from the right
		move.w	d2,d4			byte counter
		bra	20$
;10$		roxl.b	-(a2)
10$		roxl.w	-(a2)
20$		dbra	d4,10$			shift bytes left
InnerEnd:
		dbra	d5,InnerStart

; if there are any more bitplanes to do then go back and do them
OuterEnd:
		dbra	d1,OuterStart		go for the next plane
		movem.l	(sp)+,d2-d5/a2-a3
		rts

		END
