******************************************************************************
*
*   Source Control
*   --------------
*   $Id: rpa8.asm,v 42.1 93/07/20 13:33:09 chrisg Exp $
*
******************************************************************************

    include 'exec/types.i'      * Required data type definitions
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
	include	'/macros.i'
	include	'/bitmap_internal.i'

    section graphics

    xdef _ReadPixelArray8  * Define public entry points
    xdef _ReadPixelLine8
	xref	_LVOReadPixelLine8

    PAGE

******* graphics.library/ReadPixelArray8 **************************************
*
*   NAME
*	ReadPixelArray8 -- read the pen number value of a rectangular array
*	of pixels starting at a specified x,y location and continuing 
*	through to another x,y location within a certain RastPort. (V36)
*
*   SYNOPSIS
*	count = ReadPixelArray8(rp,xstart,ystart,xstop,ystop,array,temprp)
*	D0                      A0 D0:16  D1:16  D2:16 D3:16 A2    A1
*
*     LONG ReadPixelArray8(struct  RastPort *, UWORD, UWORD, UWORD, UWORD,
*	   UBYTE *, struct RastPort *);
*
*   FUNCTION
*	For each pixel in a rectangular region, combine the bits from each
*	of the bit-planes used to describe a particular RastPort into the pen
*	number selector which that bit combination normally forms for the
*	system hardware selection of pixel color.
*
*   INPUTS
*	rp    -  pointer to a RastPort structure
*	(xstart,ystart) - starting point in the RastPort
*	(xstop,ystop)   - stopping point in the RastPort
*	array - pointer to an array of UBYTEs from which to fetch the pixel data
*	        allocate at least ((((width+15)>>4)<<4)*(ystop-ystart+1)) bytes.
*	temprp - temporary rastport (copy of rp with Layer set == NULL,
*	         temporary memory allocated for
*	         temprp->BitMap with Rows set == 1,
*	         temprp->BytesPerRow == (((width+15)>>4)<<1),
*	         and temporary memory allocated for 
*	         temprp->BitMap->Planes[])
*
*   RESULT
*	For each pixel in the array:
*	    Pen - (0..255) number at that position is returned
*	count   - the number of pixels read.
*
*   NOTE
*	xstop must be >= xstart
*	ystop must be >= ystart
*
*   BUGS
*
*   SEE ALSO
*	ReadPixel()  ReadPixelLine8()  graphics/rastport.h
*
******************************************************************************

    xref    _LVOClipBlit
    xref    _LVOWaitBlit

_ReadPixelArray8:

    movem.l d2/d3/d4/d5/d6/d7/a2/a3/a4,-(sp)

    sub.w   d0,d2       ; width of rectangle to read
    ext.l   d2
    addq.l  #1,d2
	move.l	d2,d4
	add.w	#$f,d4
	and.w	#$fff0,d4

    ble.s   array_error ; width non-positive ?
    move.l  d0,d7       ; temp storage for current x
    move.l  d1,d5       ; temp storage for current y

    sub.w   d1,d3       ; height of rectangle to read
    ext.l   d3
    addq.l  #1,d3
    ble.s   array_error ; height non-positive ?

    movea.l a0,a3       ; protect rastport pointer -- does not modify cc
    movea.l a1,a4       ; protect temprp pointer -- does not modify cc

    moveq.l #0,d6       ; clear count of pixels returned

    bra.s   rpxaloop_count

rpxaloop_start:

    movea.l a3,a0       ; rp
    move.l  d7,d0       ; x
    move.l  d5,d1       ; y (current line)
                        ; width in d2 (see above)
                        ; array in a2 (see above)
    movea.l a4,a1       ; temprp 

    jsr _LVOReadPixelLine8(a6)

    addq.l  #1,d5       ; next line
    adda.l  d4,a2       ; increment base pointer to array by width bytes
    add.l   d0,d6       ; add one line of pixels to pixel count

rpxaloop_count:

    dbra    d3,rpxaloop_start

rpxaloop_end:

    move.l  d6,d0       ; return pixel count
    movem.l (sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
    rts

array_error:

    moveq.l #-1,d0
    movem.l (sp)+,d2/d3/d4/d5/d6/d7/a2/a3/a4
    rts

******* graphics.library/ReadPixelLine8 ***************************************
*
*   NAME
*	ReadPixelLine8 -- read the pen number value of a horizontal line
*	of pixels starting at a specified x,y location and continuing
*	right for count pixels. (V36)
*
*   SYNOPSIS
*	count = ReadPixelLine8(rp,xstart,ystart,width,array,temprp)
*	D0                     A0 D0:16  D1:16  D2    A2    A1
*
*	LONG ReadPixelLine8(struct RastPort *, UWORD, UWORD, UWORD,
*	     UBYTE *, struct RastPort * );
*
*   FUNCTION
*	For each pixel in a rectangular region, combine the bits from each
*	of the bit-planes used to describe a particular RastPort into the pen
*	number selector which that bit combination normally forms for the
*	system hardware selection of pixel color.
*
*   INPUTS
*	rp    - pointer to a RastPort structure
*	(x,y) - a point in the RastPort
*	width - count of horizontal pixels to read
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
*	For each pixel in the array:
*	    Pen - (0..255) number at that position is returned
*	count   - the number of pixels read.
*
*   NOTE
*	width must be non negative
*
*   BUGS
*
*   SEE ALSO
*	ReadPixel()  graphics/rastport.h
*
******************************************************************************

_ReadPixelLine8:

    movem.l d2/d3/d4/d5/d6/a2/a3,-(sp)

    tst.l	rp_Layer(a0)
    beq.s	do_blit

************************* initialize tmprp to null *****************************

tmploop_init:
    movem.l		a0/a1,-(sp)	

    movea.l    	rp_BitMap(a1),a1	; get BitMap pointer
    moveq.l    	#0,d5
    move.b    	bm_Depth(a1),d5         ; Depth in d5
    cmp.w	#UNLIKELY_WORD,bm_Pad(a1)	; interleaved?
    bne.s	no_interleaved			; no, use depth
	btst	#IBMB_INTERLEAVED,bm_Flags(a1)
	beq.s	no_interleaved
    moveq	#1,d5			; yes, only clear one plane
no_interleaved:
    move.w    	bm_BytesPerRow(a1),d3  	; numbytes in d3
    lsr.w    	#1,d3                	; convert to numwords
    lea.l    	bm_Planes(a1),a1    	; address of first plane pointer in a1

    moveq.l	#0,d6			; clear tmpras with this data
    bra.s    	tmpshift_count

tmpshift_start:    		        ; fill temprp with defval
    movea.l    	(a1)+,a0        	; get plane pointer
    move.w    	d3,d4                	; init numwords for loop
    lsr.w	#1,d4
    bcc.s	no_odd_words
    move.w	d6,(a0)+
no_odd_words:
    lsr.w	#1,d4
    bcc.s	tmploop_count
    move.l	d6,(a0)+
    bra.s    	tmploop_count		; clear this plane

tmploop_start:
    move.l    	d6,(a0)+        	; stuff proper word into tmpras plane
    move.l    	d6,(a0)+        	; stuff proper word into tmpras plane

tmploop_count:
    dbra    	d4,tmploop_start    	; d4 words

tmpshift_count:
    dbra	d5,tmpshift_start

    movem.l		(sp)+,a0/a1

;*******************************************************************************

do_blit:
                    		; xstart, ystart in d0, d1
                    		; rp, temprp in a0, a1
    move.l  d2,d4       	; save "width" pixels, copy this many pixels
    moveq.l #1,d5           ; copy one row of pixels
    move.l  #(ABC+ABNC),d6  ; vanilla copy
    moveq.l #0,d2           ; copy to temprp(0,0)
    moveq.l #0,d3
    move.l  a1,-(sp)        ; save temprp
    jsr _LVOClipBlit(a6)    ; copy rp to temprp the easy/quick way
    jsr _LVOWaitBlit(a6)    ; wait for blit to finish 
    move.l  (sp)+,a1        ; restore temprp
                    		; width now in d4

;*******************************************************************************
; a1 - temprp
; a2 - array
; d4 - width
afterblit:
    movea.l rp_BitMap(a1),a1    ; get BitMap ptr
    clr.l   d1
    move.b  bm_Depth(a1),d1     ; get depth (# of bitplanes)
    subq.b  #1,d1
    lsl.b   #2,d1               ; convert to longword index
    lea.l   bm_Planes(a1),a0    ; address of first plane pointer in a0
    add.w   #15,d4
    lsr.w   #4,d4               ; convert pixel width to word width
    subq.w  #1,d4               ; adjust for dbra

    move.l  d4,d2               ; copy of width in words in d2
    movea.l 0(a0,d1),a1         ; get addr of last plane ptr in a1
    movea.l a2,a3               ; copy of dest ptr in a3

outer_loop1:

    clr.l   d5                  ; clear all bits for later use
    move.l	d5,(a3)		; and make certain that the next
    move.l	d5,4(a3)	; 16 bytes of destination array
    move.l	d5,8(a3)	; are guaranteed to be cleared
    move.l	d5,12(a3)	; before building the pen nos.

    move.w  (a1)+,d5        	; get source data, anything in it?
    beq     zero1       	; nope, so just zero the dest

; shoot, I guess we're just gonna have to figure out what's in it!

    moveq.l #0,d3           ; 1
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 2
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 3
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 4
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 5
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 6
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 7
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 8
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 9
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 10
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 11
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 12
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 13
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 14
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 15
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte
    moveq.l #0,d3           ; 16
    roxl.w  #1,d5           ; put left-most bit in extend bit
    addx.w  d3,d3           ; put extend bit in right-most bit
    move.b  d3,(a3)+        ; init byte

outer_count1:

    dbra    d2,outer_loop1      ; go back while more words to do

    bra     plane_count ; do rest of planes (if any) 

zero1:  ; source word (16 bits) is zero, next 16 dest bytes already 0; incr a3

    lea.l   16(a3),a3	    ; these 16 bytes already zero'd, skip over
    bra     outer_count1

zero2:  ; source word (16 bits) is zero, shift 0 into next 16 dest bytes

    lsl.w   (a3)+           ; 1 2
    lsl.w   (a3)+           ; 3 4
    lsl.w   (a3)+           ; 5 6
    lsl.w   (a3)+           ; 7 8
    lsl.w   (a3)+           ; 9 10
    lsl.w   (a3)+           ; 11 12
    lsl.w   (a3)+           ; 13 14
    lsl.w   (a3)+           ; 15 16
    bra.s     outer_count2

plane_loop:

    move.l  d4,d2           ; copy of width in words in d2
    movea.l 0(a0,d1),a1     ; get addr of next plane ptr in a1
    movea.l a2,a3           ; copy of dest ptr in a3

outer_loop2:

    move.w  (a1)+,d5        ; get source data, anything in it?
    beq     zero2           ; nope, so just left-shift the dest

    ; shoot, I guess we're just gonna have to figure out what's in it!

	swap	d5		; tmpdata upper word of d5.l, lower word clear
	move.b	(a3),d5		    	; 1
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5		    	; 2
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5		    	; 3
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 4
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 5
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 6
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 7
	rol.l	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 8
	rol.l	#1,d5
	move.b	d5,(a3)+

	swap	d5		; tmpdata upper byte of d5.w, lower byte clear
	move.b	(a3),d5			; 9
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 10
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 11
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 12
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 13
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 14
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 15
	rol.w	#1,d5
	move.b	d5,(a3)+
	move.b	(a3),d5			; 16
	rol.w	#1,d5
	move.b	d5,(a3)+

outer_count2:
    dbra    d2,outer_loop2  ; go back while more words to do
plane_count:
    subq.b  #4,d1           ; decrement long offset
    bge     plane_loop  	; go back while more planes to do

    movem.l (sp)+,d2/d3/d4/d5/d6/a2/a3
    move.l	d2,d0			; return count pixels
    rts

    END
