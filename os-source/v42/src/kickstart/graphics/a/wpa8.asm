******************************************************************************
*
*   Source Control
*   --------------
*   $Id: wpa8.asm,v 42.0 93/06/16 11:14:48 chrisg Exp $
*
******************************************************************************

    include 'exec/types.i'      * Required data type definitions
	include	'/macros.i'
    include '/gfx.i'        * BitMap structure definition
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/interrupts.i'
    include 'exec/ports.i'
    include 'exec/libraries.i'
    include '/gfxbase.i'
    include 'graphics/clip.i'       * Layer structure definition
    include '/rastport.i'   * RastPort structure definition
    include 'hardware/blit.i'       * Minterm definitions
	include	'submacs.i'

    section graphics

	xref	_LVOWritePixelArray8
    xdef _WritePixelArray8  * Define public entry points
    xdef _WritePixelLine8
	xref	_LVODoHookClipRects
	xref	waitblitdone
	xref	_LVOAllocVec,_LVOInitRastPort,_LVOAllocBitMap,_LVOCopyMem
	xref	_LVOWritePixelLine8,_LVOFreeBitMap,_LVOFreeVec

; build flags:
; SUPPORT_HW if any code other than normal wpa8 should be included
; EMULATE_HW if software emulation of hw should be supported
; FULL_ROUTINE is big but fast routine should be used.
; TEST_HW if hardware should be tested for.


EMUL_W	macro
	ifd	EMULATE_HW
	bsr	hw_emul_write
	endc
	endm

EMUL_R	macro
	ifd	EMULATE_HW
	bsr	hw_emul_read
	endc
	endm

    PAGE

******* graphics.library/WritePixelArray8 **************************************
*
*   NAME
*	WritePixelArray8 -- write the pen number value of a rectangular array
*	of pixels starting at a specified x,y location and continuing
*	through to another x,y location within a certain RastPort. (V36)
*
*   SYNOPSIS
*	count = WritePixelArray8(rp,xstart,ystart,xstop,ystop,array,temprp)
*	D0                       A0 D0:16  D1:16  D2:16 D3:16  A2   A1
*
*	LONG WritePixelArray8(struct  RastPort *, UWORD, UWORD,
*	     UWORD, UWORD, UBYTE *, struct  RastPort *);
*
*   FUNCTION
*	For each pixel in a rectangular region, decode the pen number selector
*	from a linear array of pen numbers into the bit-planes used to describe
*	a particular rastport.
*
*   INPUTS
*	rp     -  pointer to a RastPort structure
*	(xstart,ystart) -  starting point in the RastPort
*	(xstop,ystop)   -  stopping point in the RastPort
*	array  - pointer to an array of UBYTEs from which to fetch the
*	         pixel data. Allocate at least
*	         ((((width+15)>>4)<<4)*(ystop-ystart+1)) bytes.
*	temprp - temporary rastport (copy of rp with Layer set == NULL,
*	         temporary memory allocated for
*	         temprp->BitMap with Rows set == 1,
*	         temprp->BytesPerRow == (((width+15)>>4)<<1),
*	         and temporary memory allocated for
*	         temprp->BitMap->Planes[])
*
*   RESULT
*	count will be set to the number of pixels plotted.
*
*   NOTE
*	xstop must be >= xstart
*	ystop must be >= ystart
*
*   BUGS
*
*   SEE ALSO
*	WritePixel()  graphics/rastport.h
*
******************************************************************************

    xref    _LVOClipBlit
    xref    _LVOWaitBlit

_WritePixelArray8:

    movem.l d2/d3/d4/d5/d6/d7/a2/a3/a4,-(sp)

    sub.w   d0,d2       ; width of rectangle to write
    ext.l   d2
    addq.l  #1,d2
    move.l  d2,d4
    add.w   #$f,d4
    and.w   #$fff0,d4

    ble.s   warray_error ; width non-positive ?
    move.l  d0,d7       ; temp storage for current x
    move.l  d1,d5       ; temp storage for current y

    sub.w   d1,d3       ; height of rectangle to write
    ext.l   d3
    addq.l  #1,d3
    ble.s   warray_error ; height non-positive ?

    movea.l a0,a3       ; protect rastport pointer -- does not modify cc
    movea.l a1,a4       ; protect temprp pointer -- does not modify cc

    moveq.l #0,d6       ; clear count of pixels returned

    bra.s   wrpxaloop_count

wrpxaloop_start:

    movea.l a3,a0       ; rp
    move.l  d7,d0       ; x
    move.l  d5,d1       ; y (current line)
                        ; width in d2 (see above)
                        ; array in a2 (see above)
    movea.l a4,a1       ; temprp 

    jsr _WritePixelLine8

    addq.l  #1,d5       ; next line
    adda.l  d4,a2       ; increment array base pointer ((width+15)>>4<<4) bytes
    add.l   d0,d6       ; add one line of pixels to pixel count

wrpxaloop_count:

    dbra    d3,wrpxaloop_start

wrpxaloop_end:

    move.l  d6,d0       ; return pixel count
    movem.l (sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
    rts

warray_error:

    moveq.l #-1,d0
    movem.l (sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
    rts

******* graphics.library/WritePixelLine8 ***************************************
*
*   NAME
*	WritePixelLine8 -- write the pen number value of a horizontal line
*	of pixels starting at a specified x,y location and continuing
*	right for count pixels. (V36)
*
*   SYNOPSIS
*	count = WritePixelLine8(rp,xstart,ystart,width,array,temprp)
*	D0                      A0 D0:16  D1:16  D2    A2    A1
*
*	LONG WritePixelLine8(struct RastPort *, UWORD, UWORD,
*	     UWORD, UBYTE *, struct RastPort *);
*
*   FUNCTION
*	For each pixel in a horizontal region, decode the pen number selector
*	from a linear array of pen numbers into the bit-planes used to describe
*	a particular rastport.
*
*   INPUTS
*	rp    -  pointer to a RastPort structure
*	(x,y) - a point in the RastPort
*	width - count of horizontal pixels to write
*	array - pointer to an array of UBYTEs from which to fetch the pixel data
*	        allocate at least (((width+15)>>4)<<4) bytes.
*	temprp - temporary rastport (copy of rp with Layer set == NULL,
*	         temporary memory allocated for
*	         temprp->BitMap with Rows set == 1,
*	         temprp->BytesPerRow == (((width+15)>>4)<<1),
*	         and temporary memory allocated for
*	         temprp->BitMap->Planes[])
*
*   RESULT
*	Count will be set to the number of pixels plotted
*
*   NOTE
*	width must be non negative
*
*   BUGS
*
*   SEE ALSO
*	WritePixel()  graphics/rastport.h
*
******************************************************************************

_WritePixelLine8:
    movem.l d0/d1/d2/d3/d4/d5/d6/d7/a0/a1/a2/a3/a4/a5,-(sp)

; a1 - temprp
; a2 - array
; d2 - width

wbeforeblit:
    movea.l rp_BitMap(a1),a3	; get temprp bitmap pointer
    clr.l   d1
    move.b  bm_Depth(a3),d1     ; get depth (# of bitplanes)
    lsl.w   #2,d1               ; convert to longword index
    lea.l   bm_Planes(a3),a0    ; address of first plane pointer in a0
	ext.l	d2		; clear upper word
    add.w   #$f,d2		; round upwards
    and.w   #$fff0,d2           ; word align...

	move.l	d2,d4		; use d2 for count, d3 for offset, d4 for max

wouter_loop2:

	move.l	d4,d3		; recalculate a3 (array source pointer)
	sub.l	d2,d3

	lea.l   16(a2,d3.l),a3	; funky decrement below
	lsr.l	#3,d3		; adjust for word ptr offset into tmpras

	move.l	(a3),d6		; and make certain that the next
	move.l	4(a3),d7	; 16 bytes of destination array
	move.l	8(a3),a4	; are guaranteed to be saved
	move.l	12(a3),a5	; before decoding the pen nos.

	moveq.l	#0,d0		; will use d0 to count from lo to high

wplane_loop:

	movea.l 0(a0,d0.w),a1   ; get addr of next plane ptr in a1
	lea.l	0(a1,d3.l),a1   ; adjust a1 for next tmpras offset

	clr.l   d5              ; clear all bits for later use

; shoot, I guess we're just gonna have to figure out what's in it!

	move.b	-(a3),d5		; 1
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 2
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 3
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 4
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 5
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 6
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 7
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 8
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 9
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 10
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 11
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 12
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 13
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 14
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 15
	ror.l	#1,d5
	move.b	d5,(a3)
	move.b	-(a3),d5		; 16
	ror.l	#1,d5
	move.b	d5,(a3)

	lea.l	16(a3),a3		; reset a3 for next pass

	swap	d5			; swap tmpdata into lower word of d5
	move.w  d5,(a1)+		; set tmpras data

wplane_count:
	add.b   #4,d0           	; increment long offset
	cmp.b   d0,d1           	; and test for more planes to do
	bgt.s   wplane_loop     	; go back while more planes to do

	move.l	d6,(a3)			; and make certain that the next
	move.l	d7,4(a3)		; 16 bytes of destination array
	move.l	a4,8(a3)		; are guaranteed to be restored
	move.l	a5,12(a3)		; after decoding the pen nos.

wouter_count2:
	sub.l	#16,d2
	bgt	    wouter_loop2    ; go back while more words to do

;*******************************************************************************

do_blit:
    moveq.l #0,d0           ; copy from temprp(0,0)
    moveq.l #0,d1
    movem.l (sp),d2/d3/d4   ; xstart, ystart in d2, d3 width in d4
    moveq.l #1,d5           ; copy one row of pixels
    move.l  #(ABC+ABNC),d6  ; vanilla copy

	movem.l 32(sp),a0/a1	; fetch rp, temprp pointers
	exg		a0,a1 			; now temprp is source, rp dest

    jsr _LVOClipBlit(a6)    ; copy temprp to rp the easy/quick way
    jsr _LVOWaitBlit(a6)    ; and wait for blit to finish 

    movem.l (sp)+,d0/d1/d2/d3/d4/d5/d6/d7/a0/a1/a2/a3/a4/a5
*		  0  4  8  12 16 20 24 28 32 36 40 44 48 52

    move.l	d2,d0 			; return width count
    rts


	opt	p=68020
	ifd	EMULATE_HW
hw_emul_write:
; emulate writes to the chunky-to planar hw
; entr a6=&emul_hw, and has just been stored to
; exit: none modified
; 0 write buf
; 4 
; 8
; 12
; 16
; 20
; 24
; 28
; 32
	move.l	28(a6),32(a6)
	move.l	24(a6),28(a6)
	move.l	20(a6),24(a6)
	move.l	16(a6),20(a6)
	move.l	12(a6),16(a6)
	move.l	8(a6),12(a6)
	move.l	4(a6),8(a6)
	move.l	(a6),4(a6)
	rts

shft_r	macro	op
	move.b	\1,d1
	lsr.b	#1,d1
	move.b	d1,\1
	endm

hw_emul_read:
; update (a6) to reflect a read
; (a6)= lowbits(buf)
; modifies: none
	movem.l	d0/d1,-(a7)
	shft_r	32(a6)
	addx.l	d0,d0
	shft_r	33(a6)
	addx.l	d0,d0
	shft_r	34(a6)
	addx.l	d0,d0
	shft_r	35(a6)
	addx.l	d0,d0

	shft_r	32-4(a6)
	addx.l	d0,d0
	shft_r	33-4(a6)
	addx.l	d0,d0
	shft_r	34-4(a6)
	addx.l	d0,d0
	shft_r	35-4(a6)
	addx.l	d0,d0

	shft_r	32-8(a6)
	addx.l	d0,d0
	shft_r	33-8(a6)
	addx.l	d0,d0
	shft_r	34-8(a6)
	addx.l	d0,d0
	shft_r	35-8(a6)
	addx.l	d0,d0

	shft_r	32-12(a6)
	addx.l	d0,d0
	shft_r	33-12(a6)
	addx.l	d0,d0
	shft_r	34-12(a6)
	addx.l	d0,d0
	shft_r	35-12(a6)
	addx.l	d0,d0

	shft_r	32-16(a6)
	addx.l	d0,d0
	shft_r	33-16(a6)
	addx.l	d0,d0
	shft_r	34-16(a6)
	addx.l	d0,d0
	shft_r	35-16(a6)
	addx.l	d0,d0

	shft_r	32-20(a6)
	addx.l	d0,d0
	shft_r	33-20(a6)
	addx.l	d0,d0
	shft_r	34-20(a6)
	addx.l	d0,d0
	shft_r	35-20(a6)
	addx.l	d0,d0

	shft_r	32-24(a6)
	addx.l	d0,d0
	shft_r	33-24(a6)
	addx.l	d0,d0
	shft_r	34-24(a6)
	addx.l	d0,d0
	shft_r	35-24(a6)
	addx.l	d0,d0

	shft_r	32-28(a6)
	addx.l	d0,d0
	shft_r	33-28(a6)
	addx.l	d0,d0
	shft_r	34-28(a6)
	addx.l	d0,d0
	shft_r	35-28(a6)
	addx.l	d0,d0
	move.l	d0,(a6)
	movem.l	(a7)+,d0/d1
	rts

	endc

do_wpa8_loop	macro	depth
outer_loop\@:
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W

	EMUL_R
	move.l	(a6),d7
	bfins	d7,(a0){d4:d5}
	lea	4(a0),a0

	ifgt	\1-1
	EMUL_R
	move.l	(a6),d7
	bfins	d7,(a1){d4:d5}
	lea	4(a1),a1
	endc

	ifgt	\1-2
	EMUL_R
	move.l	(a6),d7
	bfins	d7,(a2){d4:d5}
	lea	4(a2),a2
	endc

	ifgt	\1-3
	EMUL_R
	move.l	(a6),d7
	bfins	d7,(a3){d4:d5}
	lea	4(a3),a3
	endc

	ifgt	\1-4
	EMUL_R
	move.l	(a6),d7
	bfins	d7,(a4){d4:d5}
	lea	4(a4),a4
	endc

	ifgt	\1-5
	EMUL_R
	move.l	(a6),d7
	bfins	d7,-4(a0,d0.l){d4:d5}
	endc

	ifgt	\1-6
	EMUL_R
	move.l	(a6),d7
	bfins	d7,-4(a0,d1.l){d4:d5}
	endc

	ifgt	\1-7
	EMUL_R
	move.l	(a6),d7
	bfins	d7,-4(a0,d2.l){d4:d5}
	endc

	move.w	horizcount_w+8(a7),d3
	ifnd	EMULATE_HW
	ifgt	\1-7
	bmi	one_lword\@
	else
	bmi.s	one_lword\@
	endc
	else
	bmi	one_lword\@
	endc

	bra.s	hcount_end\@
hcount_lp\@:
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	
	EMUL_R
	move.l	(a6),(a0)+

	ifgt	\1-1
	EMUL_R
	move.l	(a6),(a1)+
	endc

	ifgt	\1-2
	EMUL_R
	move.l	(a6),(a2)+
	endc

	ifgt	\1-3
	EMUL_R
	move.l	(a6),(a3)+
	endc

	ifgt	\1-4
	EMUL_R
	move.l	(a6),(a4)+
	endc

	ifgt	\1-5
	EMUL_R
	move.l	(a6),-4(a0,d0.l)
	endc

	ifgt	\1-6
	EMUL_R
	move.l	(a6),-4(a0,d1.l)
	endc

	ifgt	\1-7
	EMUL_R
	move.l	(a6),-4(a0,d2.l)
	endc
hcount_end\@:
	dbra	d3,hcount_lp\@
; now, do right side
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W

	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a0){0:d6}

	ifgt	\1-1
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a1){0:d6}
	endc

	ifgt	\1-2
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a2){0:d6}
	endc

	ifgt	\1-3
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a3){0:d6}
	endc

	ifgt	\1-4
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a4){0:d6}
	endc

	ifgt	\1-5
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a0,d0.l){0:d6}
	endc

	ifgt	\1-6
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a0,d1.l){0:d6}
	endc

	ifgt	\1-7
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a0,d2.l){0:d6}
	endc

one_lword\@
	add.w	chmodulo_w+8(a7),a5
	move.w	plmodulo_w+8(a7),d7
	add.w	d7,a0
	add.w	d7,a1
	add.w	d7,a2
	add.w	d7,a3
	add.w	d7,a4
	subq.w	#1,ycounter_w+8(a7)
	bne	outer_loop\@
	rts
	endm

slow_wcp:
; write a bunch of chunky pixels, slowly!
TEMP_SIZE	set	0
	ARRAYVAR	temprp,rp_SIZEOF

	movem.l d2-d7/a2-a6,-(sp)
	ALLOCLOCALS
	clr.l	temprp+rp_BitMap(a7)
	sub.l	a4,a4
	move.l  d0,d7       ; temp storage for current x
	move.l  d1,d5       ; temp storage for current y
        sub.w   d0,d2       ; width of rectangle to write
	ext.l   d2
	addq.l  #1,d2
	ble.s	wchp_error
	sub.w   d1,d3       ; height of rectangle to write
	ext.l   d3
	addq.l  #1,d3	
	ble.s   wchp_error ; height non-positive ?
	movea.l a0,a3       ; protect rastport pointer -- does not modify cc

	move.l	a6,d6
	move.l	gb_ExecBase(a6),a6
	move.l	d2,d0
	add.l	#16,d0		; for wpa8 stupidity
	moveq	#0,d1
	jsr	_LVOAllocVec(a6)
	exg	a6,d6		; d6 now has ExecBase
	move.l	d0,a4
	tst.l	d0
	beq.s	wchp_error
	lea	temprp(a7),a1
	jsr	_LVOInitRastPort(a6)
	movem.l	d2/d3,-(a7)
	move.l	d2,d0	; sizex
	moveq	#1,d1	; sizey
	moveq	#8,d2	; depth
	moveq	#0,d3	; flags
	sub.l	a0,a0	; friend
	jsr	_LVOAllocBitMap(a6)
	movem.l	(a7)+,d2/d3
	move.l	d0,temprp+rp_BitMap(a7)
	beq.s	wchp_error
	move.l	a2,a5
	bra.s   wchploop_count

wchploop_start:


	move.l	a5,a0
	move.l	a4,a1
	move.l	d2,d0
	exg	a6,d6
	jsr	_LVOCopyMem(a6)
	exg	a6,d6


	movea.l a3,a0       ; rp
	lea	temprp(a7),a1
	move.l  d7,d0       ; x
	move.l  d5,d1       ; y (current line)
	move.l	a4,a2

	jsr _LVOWritePixelLine8(a6)

	addq.l  #1,d5       ; next line
	adda.l  d4,a5       ; increment array base pointer ((width+15)>>4<<4) bytes

wchploop_count:

    dbra    d3,wchploop_start

wchploop_end:
wchp_error:
	move.l	temprp+rp_BitMap(a7),a0
	jsr	_LVOWaitBlit(a6)
	jsr	_LVOFreeBitMap(a6)
nofbm:	move.l	a6,d6
	move.l	gb_ExecBase(a6),a6
	move.l	a4,a1
	jsr	_LVOFreeVec(a6)
	move.l	d6,a6

	lea	TEMP_SIZE(a7),a7
	movem.l (sp)+,d2-d7/a2-a6
	rts

******* graphics.library/WriteChunkyPixels **************************************
*
*   NAME
*	WriteChunkyPixels -- write the pen number value of a rectangular array
*	of pixels starting at a specified x,y location and continuing
*	through to another x,y location within a certain RastPort. (V40)
*
*   SYNOPSIS
*	WriteChunkyPixels(rp,xstart,ystart,xstop,ystop,array,bytesperrow)
*	                  A0 D0     D1     D2    D3    A2     D4
*
*	VOID WriteChunkyPixels(struct  RastPort *, LONG, LONG,
*	     LONG, LONG, UBYTE *, LONG);
*
*   FUNCTION
*	For each pixel in a rectangular region, decode the pen number selector
*	from a linear array of pen numbers into the bit-planes used to describe
*	a particular rastport.
*
*   INPUTS
*	rp     -  pointer to a RastPort structure
*	(xstart,ystart) -  starting point in the RastPort
*	(xstop,ystop)   -  stopping point in the RastPort
*	array  - pointer to an array of UBYTEs from which to fetch the
*	         pixel data.
*	bytesperrow - The number of bytes per row in the source array.
*		This should be at least as large as the number of pixels
*		being written per line.
*
*   RESULT
*
*   NOTE
*	xstop must be >= xstart
*	ystop must be >= ystart
*	The source array can be in fast RAM.
*
*   ===chunky-to-planar conversion HW:
*
*   GfxBase->ChunkyToPlanarPtr is either NULL, or a pointer to a HW
*   register used to aid in the process of converting 8-bit chunky 
*   pixel data into the bit-plane format used by the Amiga custom
*   display chips. If NULL, then such hardware is not present.
*
*   If an expansion device provides hardware which operates compatibly,
*   than it can install the HW address into this pointer at boot time,
*   and the system will use it.
*
*   This pointer may be used for direct access to the chunky-to-planar
*   conversion HW, if more is desired than the straight chunky-pixel
*   copy that is performed by WriteChunkyPixels().
*
*   If using the hardware directly, it should only be accessed when
*   the task using it has control of the blitter (via OwnBlitter()),
*   since this is the locking used to arbitrate usage of this device.
*
*   The hardware may be viewed as a device which accepts 32 8-bit
*   chunky pixels and outputs 8 longwords of bitplane data.
*
*   For proper operation, exactly 8 longwords (containing 32 pixels)
*   of chunky data should be written to *(GfxBase->ChunkyToPlanarPtr).
*   After the data is written, bitplane data (starting with plane 0)
*   can be read back a longword at a time. There is no need to read
*   back all 8 longwords if the high-order bitplanes are not needed.
*
*   Since WriteChunkyPixels is not (currently) particularly fast on 
*   systems without the chunky-to-planar hardware, time critical
*   applications (games, etc) may want to use their own custom conversion
*   routine if GfxBase->ChunkyToPlanarPtr is NULL, and call
*   WriteChunkyPixels() otherwise.
*
*   This pointer is only present in GfxBase in versions of graphics.library
*   >= 40, so this should be checked before the pointer is read.
*
*   BUGS
*	Not very fast on systems without chunky-to-planar conversion
*	hardware.
*
*   SEE ALSO
*	WritePixel()  graphics/rastport.h
*
******************************************************************************
_WriteChunkyPixels::
	ifnd	EMULATE_HW
	tst.l	gb_HWEmul(a6)
	beq	slow_wcp
	endc
	ifd	SUPPORT_HW
fast_wpa8::
;void wpa8(register __a0 struct RastPort *rp,
;		register __d0 int xstart
;		register __d1 int ystart
;		register __d2 int xstop,
;		register __d3 int ystop,
;		register __a2 UBYTE *array,
;		register __d4 LONG modulo);

; first, build the hook structure on the stack

	movem.w	d0/d1,-(a7)	; save dstx,dst
	move.l	a6,-(a7)	; save gfxbase
	move.l	a2,-(a7)	; save &chunky
	move.l	a0,-(a7)	; save &rastport	
	move.l	a0,a1		; a1=rport for dhcr
	pea	wpa8_hook(pc)	; &entry point	
	lea	-MLN_SIZE(a7),a7
	move.l	a7,a0		; &hook for dhcr
	movem.w	d0/d1/d2/d3,-(a7) ; save rect on stack
	move.l	a7,a2		; &rect for dhcr
	move.l	d4,-(a7)	; save modulo
	
	move.l	gb_LayersBase(a6),a6
	jsr	_LVODoHookClipRects(a6)
	lea	MLN_SIZE+7*4(a7),a7
	move.l	(a7)+,a6
	rts



wpa8_hook:
; a0=structure of &hook:
; -12   modulo for chunky data
;-8	x1 y1 x2 y2  (.w)
;0 useless min_node
;8	entry point
;12	&rport
;16	&chunkypixels
;20	gfxbase
;24 srcx.w srcy.w
; a1=structure of &message passed:
; 0 &layer or 0
; 4 dstrect
; 12 srcx offset.l
; 16 srcy offset.l

; a2=&rastport

; register usage in inner loop
; a0 pl0
; a1 pl1
; a2 pl2
; a3 pl3
; a4 pl4
; a5 &chunkydata
; a6 &hw
; d0 pl5-pl0
; d1 pl6-pl0
; d2 pl7-pl0
; d3 horiz count
; d4 left bitfield offset
; d5 left bitfield width
; d6 right bitfield width
; d7 spare
;
; stack variables:
; ycounter_w	vertical count
; plmodulo_w	modulo for dest data
; chmodulo_w	chunky pixel modulo
; horizcount_w	# of counts per line

TEMP_SIZE	set	0	; init locals
	WORDVAR	ycounter
	WORDVAR	plmodulo
	WORDVAR chmodulo
	WORDVAR	horizcount
	WORDVAR	rpmask

	movem.l	a2-a6/d2-d7,-(a7)	; save regs
	ALLOCLOCALS
; first, calculate chunkydata pointer in a5
	move.l	20(a0),a6		; get gfxbase
	move.w	-10(a0),chmodulo_w(a7)
	move.l	16(a0),a5		; &chunky
	tst.l	(a1)			; layer? don't add src offsets if not
	beq.s	no_add_srcofs
	move.w	18(a1),d0		; srcy ofs
	sub.w	26(a0),d0		; - dsty
	mulu	-10(a0),d0		; bpr*srcy offset
	add.l	d0,a5				; chdata now points at correct line
	add.w	14(a1),a5		; chdata now points at correct src pixel
	sub.w	24(a0),a5		; and subtract dstx
no_add_srcofs
	lea		4(a1),a1		; a1 now points at destination rectangle
	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0		; max-min=correct dbra value
	addq	#1,d0				; adjust because we are using a subq loop
	move.w	d0,ycounter_w(a7)
; now, we have to comput the x related stuff (y is easy!)
; lword(leftx)=lx/32
	move.w	ra_MinX(a1),d4		; leftx
	move	d4,d1			; save lx
	lsr.w	#5,d1			; d1=lword(lx)
; now, compute how much to subtract from chstart in order to align it with
; the destination. We're amazingly lucky the 020 and up can do un-aligned
; lword accesses.
; left field start=lx & 31; wid=32-start
	and.l	#31,d4			; left field start
	sub.l	d4,a5			; adjust for start
	moveq	#32,d5			; gee we sure are using the constants 31+32 a lot
	sub	d4,d5			; left field width
; mostly done with left side, now let's do right
	move.w	ra_MaxX(a1),d6
	move.w	d6,d0
	lsr.w	#5,d0			; d0=lword(rightx)
	and	#31,d6			; 
	addq	#1,d6			; d6=right field width
; now, determine # of lwords=lword(rx)-lword(lx)
	sub.w	d1,d0			; d0=lw(rx)-lw(lx)

	move.w	d0,horizcount_w(a7)
	subq	#1,horizcount_w(a7)	; adjust to # of middle-words

	move.l	rp_BitMap(a2),a0
	move.w	bm_BytesPerRow(a0),d2
	lsl	#2,d0
	sub	d0,d2				; update bpl modulo
	addq	#4,d0
	lsl.w	#3,d0
	sub	d0,chmodulo_w(a7)	; update chunky modulo
; now, we just need to figure out where to store to in bitplanes.
; now, let's adjust for one-lword case
	tst.w	horizcount_w(a7)
	bpl.s	no_one_lword
; one lword, so:
; chptr +=(32-rwid) & 31
	moveq	#32,d0
	sub	d6,d0
	and	#31,d0
	sub	d0,a5
	move	d6,d5
	sub	d4,d5			; lwid=rwid-lstart
	subq	#4,d2
no_one_lword:
	move.w	d2,plmodulo_w(a7)
	move.w	ra_MinY(a1),d0
	muls	bm_BytesPerRow(a0),d0	; desty*bpr
	move.l	d0,a1
	lea	(a1,d1.w*4),a1		; add lword(lx)
	move.l	a1,d7
; now:
; a0=&bitmap
; d7=planar offset
; d4/d5/d6=lofs, lw, rwidth
	move.l	a2,d3	; save &rp
	movem.l	bm_Planes+20(a0),d0/d1/d2
	movem.l	bm_Planes(a0),a0/a1/a2/a3/a4
	sub.l	a0,d0
	sub.l	a0,d1
	sub.l	a0,d2
	add.l	d7,a0
	add.l	d7,a1
	add.l	d7,a2
	add.l	d7,a3
	add.l	d7,a4
; now, we need to dispatch to the correct routine based upon
; the rp depth and mask

	OWNBLITTER
	WAITBLITDONE
	move.l	a6,-(a7)
	ifd	EMULATE_HW
	lea	gb_HWEmul(a6),a6
	else
	move.l	gb_HWEmul(a6),a6
	endc
; now, we need to clear all bits >(depth-1) in d7
; clear pos=0
; clear len=32-depth
	move.b	(rp_Mask,d3.l),d7
	move.l	(rp_BitMap,d3.l),d3
	move.b	(bm_Depth,d3.l),d3
	cmp.b	#5,d3		; depth<=5 do slowly (save code space)
	ble.s	is_masked
	cmp.b	#$ff,d7		; all masked?
	beq.s	do_unmasked
is_masked:
	sub.w	#32,d3
	neg.w	d3
	bfclr	d7{0:d3}	; clear high bits of mask
	bfins	d7,d7{16:8}	; replicate mask
	move.w	d7,rpmask_w+4(a7)
	pea	did_call(pc)
	bra.s	do_wpa8_masked
do_unmasked:
	and.w	#$f,d3
	dc.w	$2630,$35b0
	dc.l	depth_tbl-6*4
	jsr	(d3.l)
;	jsr	([depth_tbl,pc,d3.w*4])
did_call:
	move.l	(a7)+,a6
	DISOWNBLITTER
	lea	TEMP_SIZE(a7),a7	; clean up locals
	movem.l	(a7)+,a2-a6/d2-d7
depth_0:
	rts



do_wpa8_masked:
	move.w	rpmask_w+8(a7),d3

outer_loop_masked
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl0
	bfins	d7,(a0){d4:d5}
skip_pl0:
	lea	4(a0),a0

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl1
	bfins	d7,(a1){d4:d5}
	lea	4(a1),a1
skip_pl1:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl2
	bfins	d7,(a2){d4:d5}
	lea	4(a2),a2
skip_pl2:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl3
	bfins	d7,(a3){d4:d5}
	lea	4(a3),a3
skip_pl3:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl4
	bfins	d7,(a4){d4:d5}
	lea	4(a4),a4
skip_pl4:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl5
	bfins	d7,-4(a0,d0.l){d4:d5}
skip_pl5:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_pl6
	bfins	d7,-4(a0,d1.l){d4:d5}
skip_pl6:

	ror.w	#1,d3
	bcc.s	skip_pl7
	EMUL_R
	move.l	(a6),d7
	bfins	d7,-4(a0,d2.l){d4:d5}
skip_pl7:

	swap	d3
	move.w	horizcount_w+8(a7),d3
	swap	d3
	bmi	one_lword_masked
	ifnd	EMULATE_HW
	bra.s	hcount_end_masked
	else
	bra	hcount_end_masked
	endc
hcount_lp_masked
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	
	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m0
	move.l	d7,(a0)
skip_m0:
	lea	4(a0),a0

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m1
	move.l	d7,(a1)+
skip_m1:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m2
	move.l	d7,(a2)+

skip_m2:
	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m3
	move.l	d7,(a3)+
skip_m3:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m4
	move.l	d7,(a4)+
skip_m4:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m5
	move.l	d7,-4(a0,d0.l)
skip_m5:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_m6
	move.l	d7,-4(a0,d1.l)
skip_m6:

	ror.w	#1,d3
	bcc.s	skip_m7
	EMUL_R
	move.l	(a6),-4(a0,d2.l)
skip_m7:

hcount_end_masked
	sub.l	#$10000,d3
	bpl	hcount_lp_masked
; now, do right side
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W
	move.l	(a5)+,(a6)
	EMUL_W

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r0
	rol.l	d6,d7
	bfins	d7,(a0){0:d6}
skip_r0:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r1
	rol.l	d6,d7
	bfins	d7,(a1){0:d6}
skip_r1

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r2
	rol.l	d6,d7
	bfins	d7,(a2){0:d6}
skip_r2:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r3
	rol.l	d6,d7
	bfins	d7,(a3){0:d6}
skip_r3:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r4
	rol.l	d6,d7
	bfins	d7,(a4){0:d6}
skip_r4:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r5
	rol.l	d6,d7
	bfins	d7,(a0,d0.l){0:d6}
skip_r5:

	EMUL_R
	move.l	(a6),d7
	ror.w	#1,d3
	bcc.s	skip_r6
	rol.l	d6,d7
	bfins	d7,(a0,d1.l){0:d6}
skip_r6:

	ror.w	#1,d3
	bcc.s	skip_r7
	EMUL_R
	move.l	(a6),d7
	rol.l	d6,d7
	bfins	d7,(a0,d2.l){0:d6}
skip_r7

one_lword_masked
	add.w	chmodulo_w+8(a7),a5
	move.w	plmodulo_w+8(a7),d7
	add.w	d7,a0
	add.w	d7,a1
	add.w	d7,a2
	add.w	d7,a3
	add.w	d7,a4
	subq.w	#1,ycounter_w+8(a7)
	bne	outer_loop_masked
	rts

depth_tbl:
	dc.l	depth_6
	dc.l	depth_7
	dc.l	depth_8
	dc.l	depth_8
	dc.l	depth_8
	dc.l	depth_8
	dc.l	depth_8
	dc.l	depth_8
	dc.l	depth_8

depth_6	do_wpa8_loop	6
depth_7	do_wpa8_loop	7
depth_8	do_wpa8_loop	8

	endc	; end (if support_hw)

_TestForChunkyHardware::
	ifd	TEST_HW
	ifd	SUPPORT_HW
	ifnd	EMULATE_HW
	lea	$b80002,a1
	cmp.w	#$cafe,(a1)	; correct chip id?
	bne.s	no_hw
	lea	$36(a1),a1		; address of fancy hardware
	move.l	#$55550000,d0
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	d0,(a1)
	move.l	#$cccccccc,d0
	moveq	#0,d1
	cmp.l	(a1),d0		; pl 0 - should be 1.
	bne.s	no_hw
	cmp.l	(a1),d1		; pl 1 - should be 0's
	bne.s	no_hw
	cmp.l	(a1),d0		; pl 2 - should be 1
	bne.s	no_hw
	cmp.l	(a1),d1		; pl 3 - should be 0
	bne.s	no_hw
	move.l	a1,gb_HWEmul(a6)
	lea	$25-$38(a1),a1	; point at control bits
	bset.b	#23-16,(a1)	; set to ntsc
	btst	#2,gb_DisplayFlags+1(a6)	; pal?
	beq.s	1$						; jump if ntsc
	bclr	#23-16,(a1)	; set pal bit
1$:
	endc
	endc
	endc
no_hw:
	rts
