*******************************************************************************
*
*	$Id: BltPattern.asm,v 42.1 93/07/20 13:31:31 chrisg Exp $
*
*******************************************************************************

	opt	p=68020
	section	graphics
    xdef    _BltPattern
******* graphics.library/BltPattern *****************************************
*
*   NAME
*	BltPattern --  Using standard drawing rules for areafill,
*					 blit through a mask.
*
*   SYNOPSIS
*       BltPattern(rp, mask, xl, yl, maxx, maxy, bytecnt)
*                  a1,  a0   d0  d1   d2   d3     d4
*
*	void BltPattern
*	   (struct RastPort *, void *, SHORT, SHORT, SHORT, SHORT, SHORT);
*
*   FUNCTION
*       Blit using drawmode,areafill pattern, and mask 
*       at position rectangle (xl,yl) (maxx,maxy).
*
*   INPUTS
*       rp    -  points to the destination RastPort for the blit.
*       mask  -  points to 2 dimensional mask if needed
*                if mask == NULL then use a rectangle.
*       xl,yl -  coordinates of upper left of rectangular region in RastPort
*       maxx,maxy - points to lower right of rectangular region in RastPort
*       bytecnt - BytesPerRow for mask
*
*   RESULT
*
*   SEE ALSO
*	AreaEnd
*
*****************************************************************************

	include 'exec/types.i'
	include	'/bitmap_internal.i'
	include '/rastport.i'
	include '/gfxbase.i'
	include 'hardware/custom.i'
	include '/sane_names.i'
	include 'submacs.i'
	include '/c/gfxblit.i'
	include	'/macros.i'
	include	'utility/hooks.i'

	xref	_oldbltpattern
	xref	waitblitdone
	xref	_clipbltpattern
	xref	_custom
	xref	_LVOOwnBlitter,_LVOWaitBlit,_LVODisownBlitter
	
_BltPattern:
*               current routine calls a C subroutine to do the work

	cmp	#0,a0
	bne.s	no_fast_case
	tst.l	rp_AreaPtrn(a1)
	beq.s	do_fast_case

no_fast_case:
	tst.l	rp_Layer(a1)
	if =
*		no layer just call lowest level routine with defaults
		clr.l	-(sp)
		clr.l	-(sp)
		clr.l	-(sp)
		clr.l	-(sp)
		move.w  d4,-(sp)  
		move.w	#0,-(sp)    * uword bytecnt
		movem.w	d3/d2/d1/d0,-(sp)
		clr.l	-(sp)	    * *ClipRect
		move.l  a0,-(sp)    * push 'char *' on stack
		move.l  a1,-(sp)    * push wrastr * on stack
		jsr _oldbltpattern  * call C routine
		lea 4*10(sp),sp     * remove args from stack
	else
		move.w  d4,-(sp)
		move.w	#0,-(sp)    * uword bytecnt
		movem.w	d3/d2/d1/d0,-(sp)	* for old clipblitpattern
*		move.l  d3,-(sp)    * maxy
*		move.l  d2,-(sp)    * maxx
*		move.l  d1,-(sp)    * yl
*		move.l  d0,-(sp)    * xl
		move.l  a0,-(sp)    * push 'char *' on stack
		move.l  a1,-(sp)    * push wrastr * on stack
		jsr _clipbltpattern * call C routine
		lea 5*4(sp),sp      * remove args from stack
	endif
    rts

	xref	_LVODoHookClipRects

do_fast_case:
; special case for solid fill. fast and efficient.
TEMP_SIZE	set	0
	ARRAYVAR	myhook,h_SIZEOF
	ARRAYVAR	myrect,ra_SIZEOF
	movem.l	a2/a6,-(a7)
	ALLOCLOCALS
	move.l	#rect_rtn,myhook+h_Entry(a7)
	move.l	a6,myhook+h_Data(a7)
	movem.w	d0/d1/d2/d3,myrect(a7)
	lea	myrect(a7),a2
	lea	myhook(a7),a0
	move.l	gb_LayersBase(a6),a6
	jsr	_LVODoHookClipRects(a6)
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,a2/a6
	rts


lmasktable:	
	dc.l	$ffff7fff,$3fff1fff,$0fff07ff,$03ff01ff
	dc.l	$00ff007f,$003f001f,$000f0007,$00030001,0


rect_rtn:
; called with a2=rastport
; a0=hook
; a1=message = [ &layer,rect]
	movem.l	d2-d7/a2-a6,-(a7)
	move.l	h_Data(a0),a6		; need gfxBase
	move.l	a1,a3
	OWNBLITTER
	movem.w	4(a3),d0/d1/d2/d3			; get rect
	move.l	a2,a1
	move.l	rp_BitMap(a1),a2


rfill_bm:
; rectfill a bitmap.
; d0/d1/d2/d3=x1,y1/x2,y2
; a1=rastport
; a2=bitmap
; a6=gfxbase
; trashes d0-d5/a5/a4/a3
	cmp.w	#UNLIKELY_WORD,bm_Pad(a2)
	bne.s	no_special
	btst	#IBMB_CHUNKY,bm_Flags(a2)
	bne	chunky_rfill
no_special:
	lea	_custom,a0
	sub.w	d1,d3		; endy-=starty
	addq	#1,d3
	muls	bm_BytesPerRow(a2),d1	; convert to byte offset
	moveq	#15,d6
	move.w	d0,d4
	and.w	d6,d4
	add.w	d4,d4
	move.w	lmasktable(pc,d4.w),d4	; d4=fwm
	move.w	d2,d5
	and.w	d6,d5
	add.w	d5,d5
	move.w	lmasktable+2(pc,d5.w),d5	; d5=lwm
	not.w	d5

	lsr.w	#4,d0
	lsr.w	#4,d2
	sub.w	d0,d2		; d2=# of words blitted
	addq	#1,d2
	add.w	d0,d0		; cvt to byte offset
	ext.l	d0
	add.l	d0,d1		; and add to memory base in d1

	move.w	d2,d6		; save width for later
	add.w	d2,d2
	sub.w	bm_BytesPerRow(a2),d2
	neg.w	d2		; d2=modulo
	lea	rp_minterms(a1),a5	; a5=minterm ptr
	lea	bm_Planes(a2),a3	; a3=planeptr
	moveq	#0,d0
	move.b	gb_ChipRevBits0(a6),d7
	WAITBLITDONE	a0
	and.b	#GFXF_BIG_BLITS,d7
	beq.s	no_bigblits
	move.w	d3,bltsizv(a0)
	bra.s	yes_bigblits
no_bigblits:
	lsl.w	#6,d3		; d3=bltsize
	add.w	d6,d3		; now, d3=bltsize
yes_bigblits:
	move.w	d4,bltafwm(a0)
	move.w	d5,bltalwm(a0)
	move.w	d2,bltdmod(a0)
	move.w	d2,bltcmod(a0)
	move.w	d0,bltcon1(a0)
	moveq	#-1,d2
	move.l	d2,bltbdat(a0)
	move.b	bm_Depth(a2),d0
	move.b	rp_Mask(a1),d2
	move.w	#$300,d4
	bra.s	end_lp1
lp1:	move.b	(a5)+,d4
	move.l	(a3)+,a4
	lsr.b	#1,d2
	bcc.s	end_lp1
	add.l	d1,a4
	WAITBLITDONE	a0
	move.w	d4,bltcon0(a0)
	move.l	a4,bltdpt(a0)
	move.l	a4,bltcpt(a0)
	tst.b	d7
	beq.s	old_blit
	move	d6,bltsizh(a0)	; go!
	dbra	d0,lp1
	DISOWNBLITTER
	movem.l	(a7)+,d2-d7/a2-a6
	rts
old_blit:
	move.w	d3,bltsize(a0)		; go!
end_lp1:	dbra	d0,lp1
	DISOWNBLITTER
	movem.l	(a7)+,d2-d7/a2-a6
	rts

ChunkyBlitSetup::

; input a2=destbm a3=base address
; d0/d1/d2/d3-x1,y1,x2,y2
; ouput:
;  a0-a2=preserved 
;  a3=a3 + adr(x1,y1)
;  a4=dest adr inc
;  a5-a6=preserved
;  d0=left bit offset
;  d1=trashed
;  d2=right width (offset is 0)
;  d3=dbra vertical counter
;  d4=preserved
;  d5=# of longwords : -1=just left, 0=left+right, 1=left+one+right, ...
;  d6=left bit width
;  d7=preserved
; if d5 is negative (meaning only one word to be blitted,
;  the d0:d6 pair will be adjusted properly

	sub.w	d1,d3	; y1-y2=dbra count
	muls	bm_BytesPerRow(a2),d1	; *bpr=yofs
	add.l	d1,a3			; adjust dest offset
; now, convert x coordinates to longword addresses.
	move	d0,d1
	lsr.w	#2,d1			; x1/4=leftlword
	lea	(a3,d1.w*4),a3		; adjust dest pointer to point to first lword
	move	d2,d5
	lsr.w	#2,d5			; d5=right lword
; now:
; left ofs=(d0 & 3)*8
; left width=32-left ofs
	and	#3,d0			; x1 & 3
	lsl.w	#3,d0			; 8*(x1 & 3) d0=left ofs

	moveq	#32,d6
	sub	d0,d6			; d6=left width
; now:
; right ofs=0
; right width=((rx & 3)+1)*8
	and	#3,d2
	addq	#1,d2
	lsl	#3,d2			; d2=right width
; now, handle iteration
; # lwords=1+(d5-d1)
; #inner lwords=#lwords-2
; if (inner_lwords <0), same lword!
	sub	d1,d5
	addq	#1,d5			; d5=#lwords
	move.w	bm_BytesPerRow(a2),a4
	sub	d5,a4
	sub	d5,a4
	sub	d5,a4
	sub	d5,a4
	subq	#2,d5			; # inner lwords
	bpl.s	no_one_lword
; if both are in the same longword, than we have to adjust
; the left field width.
; left field width should=pixel width=rightwidth-left offset
; so, let d6=d2-d0
	move	d2,d6
	sub	d0,d6
no_one_lword:
	rts


chunky_rfill:
; rectfill a chunky bitmap.
; d0/d1/d2/d3=x1,y1/x2,y2
; a1=rastport
; a2=bitmap
; a6=gfxbase
; calculations:
; starty=y1*bpr
	move.l	bm_Planes(a2),a3
	bsr	ChunkyBlitSetup

	move.b	rp_Mask(a1),d7		; keep this around
	replicate	d7

	move.b	rp_DrawMode(a1),d1
	and.b	#3,d1
	cmp.b	#RP_COMPLEMENT,d1
	beq	do_complement
	move.b	rp_FgPen(a1),d4
	tst.b	rp_DrawMode(a1)	; inversevid?
	bpl.s	no_swap
	move.b	rp_BgPen(a1),d4
no_swap:	
	replicate	d4		; 0xaa -> 0xaaaaaaaa
	addq.l	#1,d7
	bne	do_masked

do_jam:	lea	skip_right_and_middle(pc),a5
	tst.w	d5
	bmi.s	got_jmp_offset

	lea	do_right(pc),a5
	beq.s	got_jmp_offset		; this is testing d5 from above
	lea	fill_lp(pc),a5
	move.w	d5,d1
	lsr.w	#3,d5
	subq	#1,d5
	and.w	#7,d1
	beq.s	got_jmp_offset
	neg.w	d1
	lea	(16,a5,d1.w*2),a5
	addq.w	#1,d5
got_jmp_offset:
; now, let's attempt this!
yloop:	bfins	d4,(a3){d0:d6}		; update left edge
	lea	4(a3),a3
	move	d5,d1
	jmp	(a5)

fill_lp:
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
	move.l	d4,(a3)+
end_x_loop:
	dbra	d1,fill_lp
; now, do right edge
do_right:
	bfins	d4,(a3){0:d2}
	lea	4(a3),a3
skip_right_and_middle:
	add.l	a4,a3
	dbra	d3,yloop
done_chfill:
	DISOWNBLITTER
	movem.l	(a7)+,d2-d7/a2-a6
	rts

do_complement:
	lea	complement_skip_right_and_middle(pc),a5
	tst.w	d5
	bmi.s	complement_got_jmp_offset

	lea	complement_do_right(pc),a5
	beq.s	complement_got_jmp_offset		; testing d5 from above

	lea	complement_fill_lp(pc),a5
	move.w	d5,d1
	lsr.w	#3,d5
	subq	#1,d5
	and.w	#7,d1
	beq.s	complement_got_jmp_offset
	subq	#8,d1
	neg.w	d1
	lea	(a5,d1.w*2),a5
	addq.w	#1,d5
complement_got_jmp_offset:
; the complement loop uses d6 for the lwm and d2 for the rwm
; we must massage the (d0=leftofs,d6=leftwidth) and (d2=right width)
; pairs into d6 and d2
	moveq	#0,d1
	bfset	d1{d0:d6}
	and.l	d7,d1
	move.l	d1,d6
	moveq	#0,d1
	bfset	d1{0:d2}
	and.l	d7,d1
	move.l	d1,d2
; done


complement_yloop:
	eor.l	d6,(a3)+		; update left edge
	move	d5,d1
	jmp	(a5)

complement_fill_lp:
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
	eor.l	d7,(a3)+
complement_end_x_loop:
	dbra	d1,complement_fill_lp
; now, do right edge
complement_do_right:
	eor.l	d2,(a3)+
complement_skip_right_and_middle:
	add.l	a4,a3
	dbra	d3,complement_yloop
	bra	done_chfill


do_masked:
; entr d7=mask +1
	subq.l	#1,d7	; rastore mask
	and.l	d7,d4	; and-out unmasked bits
	not.l	d7	; and change sense of mask

	lea	masked_skip_right_and_middle(pc),a5
	tst.w	d5
	bmi.s	masked_got_jmp_offset

	lea	masked_do_right(pc),a5
	beq.s	masked_got_jmp_offset	; testing d5 from above

	lea	masked_fill_lp(pc),a5
	move.w	d5,d1
	lsr.w	#3,d5
	subq	#1,d5
	and.w	#7,d1
	beq.s	masked_got_jmp_offset
	subq	#8,d1
	neg.w	d1
; now, we need to multiply by the number of instructions per lword=4 words
	lea	(a5,d1.w*8),a5
	addq.w	#1,d5
masked_got_jmp_offset:
	move	d3,d1

; now, let's attempt this!
masked_yloop:
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	bfins	d3,(a3){d0:d6}		; update left edge
	lea	4(a3),a3
	swap	d1
	move	d5,d1
	jmp	(a5)

masked_fill_lp:
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+
	move.l	(a3),d3
	and.l	d7,d3
	or.l	d4,d3
	move.l	d3,(a3)+

masked_end_x_loop:
	dbra	d1,masked_fill_lp
; now, do right edge
masked_do_right:
	bfextu	(a3){0:d2},d3
	and.l	d7,d3
	or.l	d4,d3
	bfins	d3,(a3){0:d2}
	lea	4(a3),a3
masked_skip_right_and_middle:
	add.l	a4,a3
	swap	d1
	dbra	d1,masked_yloop
	bra	done_chfill

	xref	_patblit
	xref	_clrblit
	xdef	_bp_downcode
_bp_downcode:
*	d1 = Depth
*	a2 = RastPort
*	a1 = bltnode ptr
	movem.l	d2/d4/a2/a3/a4,-(sp)
	movem.l	4+5*4(sp),d4/a1/a2/a3		* rasdx,get bltnode*, RastPort*,pptr
	move.l	rp_BitMap(a2),a0	* get BitMap*
	move.b	bm_Depth(a0),d1		* set up Depth
	move.w	gn_bltcon0(a1),d2	* set up bltcon0
	if <>	.extend * need at least 1 bitplane to do it to
		lea	_custom,a0
		OWNBLITTER
		WAITBLITDONE	a0
		move.l	gn_fwmask(a1),fwmask(a0)	* and lwmask
		move.w	gn_bltcon1(a1),bltcon1(a0)
		moveq	#-1,d0
		move.l	d0,bdata(a0)	* and adata
		move.l	gn_bltmda(a1),d0	* get bltmda and modd
		move.l	d0,bltmda(a0)	* and bltmdd
		move.w	d0,bltmdc(a0)
		move.l	rp_AreaPtrn(a2),d0
		if <>.extend		/* fill pattern? */
			move.l	d0,gn_bltptb(a1)
			and.w	#$3F,gn_bltsize(a1)
			move.b	rp_AreaPtSz(a2),d0
			if < 	/* 2 color pattern?*/
				neg.b	d0
			endif
			move.b	d0,gn_filln(a1)
			moveq	#1,d2
			asl.w	d0,d2
			move.w	d2,gn_jstop(a1)
			clr.b	gn_j(a1)
			move.l	rp_BitMap(a2),a4
			ifne bm_BytesPerRow
				fail	next instruction
			endc
*			move.w	bm_BytesPerRow(a4),gn_bpw(a1)
			move.w	(a4),gn_bpw(a1)
			move.b	bm_Depth(a4),d2
			clr.w	d0		* counter
			if rp_AreaPtSz(a2)>=#0.b
				repeat
					btst	d0,rp_Mask(a2)
					if <>
						move.b	rp_minterms(a2,d0.w),gn_bltcon0+1(a1)
						move.l	(a3),a4
						add.l	d4,a4
						move.l	a4,gn_bltptr(a1)
						movem.l	d0/d1/a0/a1,-(sp)	save all regs
						repeat
							WAITBLITDONE	a0
							move.l	a0,-(sp)	* &custom
							move.l	a1,-(sp)	* bltnode *
							jsr		_patblit
							move.l	(sp)+,a1
							move.l	(sp)+,a0
						until d0=#0
						movem.l	(sp)+,d0/d1/a0/a1
					endif
					addq.w	#4,a3
					addq.w	#1,d0
				until d0=d1.b
			else						/* multicolor pattern */
				repeat
					btst	d0,rp_Mask(a2)
					if <>
						move.b	rp_minterms(a2,d0.w),gn_bltcon0+1(a1)
						move.l	(a3),a4
						add.l	d4,a4
						move.l	a4,gn_bltptr(a1)
						movem.l	d0/d1/a0/a1,-(sp)	save all regs
						repeat
							WAITBLITDONE	a0
							move.l	a0,-(sp)	* &custom
							move.l	a1,-(sp)	* bltnode *
							jsr		_clrblit
							move.l	(sp)+,a1
							move.l	(sp)+,a0
						until d0=#0
						movem.l	(sp)+,d0/d1/a0/a1
					endif
					addq.w	#4,a3
					addq.w	#1,d0
				until d0=d1.b
			endif
		else		/* no fill pattern */
			clr.w	d0		* counter
			repeat
				btst d0,rp_Mask(a2)
				if <>
					WAITBLITDONE	a0
					move.l	gn_bltpta(a1),bltpta(a0)
					move.b	rp_minterms(a2,d0.w),d2
					move.w	d2,bltcon0(a0)
					move.l	(a3),a4
					add.l	d4,a4
					move.l	a4,bltptc(a0)
					move.l	a4,bltptd(a0)
					btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
					
					if =
						move.w	gn_bltsize(a1),bltsize(a0)
					else
						swap	d2
						move.w	gn_blitrows(a1),d2
						and.w	#$7fff,d2
						move.w	d2,bltsizv(a0)
						move.w	gn_blitwords(a1),d2
						and.w	#$7ff,d2
						move.w	d2,bltsizh(a0)
						swap	d2
					endif
				endif
				addq.l	#4,a3
				addq.w	#1,d0
			until d0=d1.b
		endif
	DISOWNBLITTER 		* bart -- nest disown to match own 11.09.88
	endif
	movem.l	(sp)+,d2/d4/a2/a3/a4
	rts
	

	end