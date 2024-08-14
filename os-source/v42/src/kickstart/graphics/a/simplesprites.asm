*******************************************************************************
*
*	$Id: SimpleSprites.asm,v 42.2 93/09/20 17:15:32 spence Exp $
*
*******************************************************************************

	include	'exec/types.i'
	include	'exec/ables.i'
	include	'/gfxbase.i'
	include	'/view.i'
	include	'/sprite.i'
	include	'/monitor.i'
	include	'/macros.i'
	include	'/copper.i'
	include	'/gels/gelsinternal.i'
	include	'/bitmap_internal.i'
	include	'/vp_internal.i'
	include 'hardware/intbits.i'
	include 'hardware/dmabits.i'
	include	'exec/memory.i'

	xref	_LVOPermit,_LVOObtainSemaphore,_LVOReleaseSemaphore,_LVOGetTagData
	xref	_LVOAllocMem,_LVOAllocVec,_LVOFreeMem,_LVOFreeVec
	xref	_LVONextTagItem,_LVOMoveSprite

	xref	_pokevp

get_sprite_fmode:
; ret d0=fmodebits(spritewidth)
; based on the AA machine's memory architecture, get the fmode bits to display
; a sprite of a given width.
	subq	#1,d0
	beq.s	got_bits	; 1->0
	subq	#1,d0		; 2x?
	bne.s	is_4x
; must be 2x
	moveq	#0,d0
	move.b	gb_MemType(a6),d0
	cmp.b	#BANDWIDTH_4X,d0
	bne.s	got_bits
	moveq	#(BANDWIDTH_2XNML<<2),d0
	rts
is_4x:	moveq	#(BANDWIDTH_4X<<2),d0		; 4x!
got_bits:
	rts

do_nothing:
	tst.w	d0
	rts

change_sprite_width:
; for non-aa systems this should not be called with a width other than 1.
; entr d0=width exit d0=width or 0 for failure. flags reflect d0.
; trashes d0/d1/a0/a1
; d1=sprite number.
; it's ok to change the sprite width iff:
; bitclear(gb_SpriteReserved-gb_SoftSprites,snum)==0
; if (width != gb_SpriteWidth) then
;  gb_SpriteWidth=width;
;  blank soft sprites
;  poke all fmodes in the copper list to the new width
;  call GfxBase->WidthChangedVector
; endif
	cmp.w	gb_SpriteWidth(a6),d0
	beq.s	do_nothing
	move.l	d2,-(a7)
	move.b	gb_SpriteReserved(a6),d2
	sub.b	gb_SoftSprites(a6),d2
	tst.w	d1
	bmi.s	no_clr
	bclr.b	d1,d2
no_clr:
	tst.b	d2
	beq.s	ok_to_change
	moveq	#0,d0
	move.l	(a7)+,d2
	rts
ok_to_change:
; now, blank soft sprites in copinit
	move.l	(a7)+,d2
	move.w	d0,gb_SpriteWidth(a6)
	move.l	a6,-(a7)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	(a7)+,a6
	move.l	gb_copinit(a6),a0
	lea	copinit_sprstop(a0),a1
	lea	copinit_sprstrtup+2(a0),a0
	move.l	d1,-(a7)	; save sprite nunber
	moveq	#0,d1
ss_lp	btst.b	d1,gb_SoftSprites(a6)
	beq.s	noblank
	bset.b	d1,gb_SprMoveDisable(a6)
	move.w	(a1),(a0)
	move.w	2(a1),4(a0)
noblank:
	lea	8(a0),a0
	addq	#1,d1
	cmp	#8,d1
	bne.s	ss_lp
	bsr	get_sprite_fmode	; width -> fmode bits
	move.w	d0,gb_SpriteFMode(a6)
; poke fmode in copinit:
	move.l	gb_copinit(a6),a0
	move.w	copinit_fm0+2(a0),d1
	and.w	#$fff3,d1
	or.w	d0,d1
	move.w	d1,copinit_fm0+2(a0)

; now, do the horrible part.
; v=active view
; for ( each viewport in v)
;  poke the intermediate copper instruction's fmode
; poke the view's copper list fmode bits.
; it might be a good idea to poke the posctl words to be invisible
; when blanking soft sprites ?!?

	move.l	gb_ActiView(a6),d1
	beq.s	done_viewport_copins
	move.l	d1,a0
	move.l	v_ViewPort(a0),a0
vp_loop	cmp	#0,a0
	beq.s	done_viewport_copins
	movem.l	d0/d2/a0,-(a7)
	move.w	d0,d1				; new fmode
	move.w	#fmode,d0
	moveq	#$c,d2				; mask
	bsr	_pokevp
	movem.l	(a7)+,d0/d2/a0
end_vp_loop:
	move.l	vp_Next(a0),a0
	bra.s	vp_loop

done_viewport_copins:
	move.l	(a7)+,d1	; restore sprite number
	move.l	a6,-(a7)
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
	move.l	(a7)+,a6
	moveq	#-1,d0

tweak_intuition:
; trashes a0,a1
; preserves d0
	tst.b	d1
	bmi.s	yes_tweak
	btst.b	d1,gb_SoftSprites(a6)	; was a softsprite changed?
	bne.s	notweak
yes_tweak:
	move.l	d0,-(a7)
	move.l	gb_IVector(a6),d0
	beq.s	1$
	move.l	d0,a0
	move.l	gb_IData(a6),a1
	jsr	(a0)
1$:	move.l	(a7)+,d0
notweak:
	rts


******* graphics.library/GetExtSpriteA *************************************
*
*   NAME
*	GetExtSpriteA -- Attempt to get a sprite for the extended sprite
*					 manager. (V39)
*	GetExtSprite -- varargs stub for GetExtSpriteA. (V39)
*
*   SYNOPSIS
*	Sprite_Number = GetExtSpriteA( sprite, tags )
*	    d0			        a2      a1
*
*	LONG GetExtSpriteA( struct ExtSprite *, struct TagItem * );
*
*	spritenum=GetExtSprite(sprite,tags,...);
*
*   FUNCTION
*	Attempt to allocate one of the eight sprites for private use
*	with the extended sprite manager. 
*
*   INPUTS
*	sprite - ptr to programmer's ExtSprite (from AllocSpriteData()).
*	tags - a standard tag list:
*
*		GSTAG_SPRITE_NUM	specifies a specific sprite to get by number.
*		
*		GSTAG_ATTACHED specifies that you wish to get a sprite pair.
*			the tag data field points to a ExtSprite structure
*			for the second sprite. You must free both sprites.
*
*
*   RESULTS
*	Sprite_number = a sprite number or -1 for an error.
*		This call will fail if no sprites could be allocated, or
*		if you try to allocate a sprite which would require
*		a mode change when there are other sprites of incompatible
*		modes in use.
*
*   BUGS
*
*	GSTAG_ATTACHED does not work in version 39. When running under V39,
*	you should attach the second sprite with a separate GetExtSprite call.
*
*   SEE ALSO
*	FreeSprite() ChangeSprite() MoveSprite() GetSprite() graphics/sprite.h
*
*********************************************************************

TEMP_SIZE	set	0
	LONGVAR	spriteatt	; pointer to attached data or 0
	LONGVAR	tstate		; for NextTagItem
	WORDVAR	spritenum	; number or -1
	WORDVAR	spriteflags	; flags bit 0=softsprite
	
_GetExtSpriteA::
	movem.l	a2-a5,-(a7)
	ALLOCLOCALS
	clr.l	spriteatt_l(a7)
	move.w	#-1,spritenum_w(a7)
	clr.w	spriteflags_w(a7)
	move.l	a1,tstate_l(a7)
	move.l	a6,a5
	move.l	gb_UtilBase(a6),a6
gst_tag_loop:
	lea	tstate_l(a7),a0
	jsr	_LVONextTagItem(a6)		; d0=next tag item, tstate updated
	tst.l	d0
	beq.s	gst_donetags
	move.l	d0,a0
	move.l	4(a0),d1
	move.l	(a0),d0
	sub.l	#GSTAG_SPRITE_NUM,d0
	bne.s	gst_not_snum
	move.w	d1,spritenum_w(a7)
gst_not_snum:
	subq.l	#2,d0
	bne.s	gst_not_att
	move.l	d1,spriteatt_l(a7)
gst_not_att:
	subq.l	#2,d0
	bne.s	not_softsprite
	tst.l	d1
	beq.s	not_softsprite
	bset.b	#0,spriteflags_w+1(a7)
not_softsprite:
	sub.l	#GSTAG_SCANDOUBLED-GSTAG_SOFTSPRITE,d0
	bne.s	not_sdbl
	tst.l	d1
	beq.s	not_sdbl
	bset	#1,spriteflags_w+1(a7)
not_sdbl:
	bra.s	gst_tag_loop
gst_donetags:
; now, all tags are parsed
; now, check stuff!
; is width available on this system?
	move.l	gb_ExecBase(a5),a6
	FORBID	a6,NOFETCH
	move.w	es_wordwidth(a2),d0
	subq.w	#1,d0				; width=1?
	beq.s	width_ok
	subq.w	#1,d0
	bne.s	is4x
	tst.b	gb_MemType(a5)
	beq	gs_fail
	bra.s	width_ok
is4x:	cmp.w	#2,d0
	bne	gs_fail				; no >4x sprites
	cmp.b	#3,gb_MemType(a5)
	bne	gs_fail
width_ok:
; width is ok, let's see if possible to get a sprite of this width
	move.w	es_wordwidth(a2),d0		; get width
	cmp.w	gb_SpriteWidth(a5),d0		; width=cur width?
	beq.s	can_get_width
	move.b	gb_SpriteReserved(a5),d0
	cmp.b	gb_SoftSprites(a5),d0		; any non-intuition sprites allocated?
	bne.s	gs_fail
can_get_width:
	tst.l	spriteatt_l(a7)			; attached sprite
	bne.s	gs_fail				; must add code
	move.w	spritenum_w(a7),d0
	bpl.s	get_specific_sprite1
; user wants next available sprite
	move.b	gb_SpriteReserved(a5),d0
	moveq	#7,d1
1$:	lsr.b	#1,d0
	dbcc	d1,1$
	bcs.s	gs_fail
	moveq	#7,d0
	sub	d1,d0
; now, d0=sprite # to get
get_specific_sprite1:
	and	#7,d0
	btst.b	d0,gb_SpriteReserved(a5)
	bne.s	gs_fail
	move.w	d0,ss_num(a2)
	bset.b	d0,gb_SpriteReserved(a5)
	bset.b	d0,gb_ExtSprites(a5)
	btst	#0,spriteflags_w+1(a7)
	beq.s	no_soft
	bset.b	d0,gb_SoftSprites(a5)
no_soft:
	btst	#1,spriteflags_w+1(a7)
	beq.s	no_dbl1
	bset.b	d0,gb_ScanDoubledSprites(a5)
no_dbl1:
	move.w	es_wordwidth(a2),d0
	move.w	ss_num(a2),d1
	move.l	a5,a6
	bsr	change_sprite_width
	move.l	gb_ExecBase(a6),a3
	PERMIT	a3,NOFETCH
	moveq	#0,d0
	move.w	ss_num(a2),d0
exit1:
	add.l	#TEMP_SIZE,a7
	movem.l	(a7)+,a2-a5
	rts
gs_fail:
	PERMIT
	move.l	a5,a6
	moveq	#-1,d0
	bra.s	exit1

******* graphics.library/FreeSpriteData **************************************
*
*   NAME
*	FreeSpriteData -- free sprite data allocated by AllocSpriteData() (V39)
*
*   SYNOPSIS
*	FreeSpriteData(extsp)
*			a2
*
*	void FreeSpriteData(struct ExtSprite *);
*
*
*   FUNCTION
*
*   INPUTS
*	extsp - The extended sprite structure to be freed. Passing NULL is a
*	NO-OP.
*
*   SEE ALSO
*	FreeSpriteData() FreeSprite() ChangeSprite() MoveSprite() GetExtSprite()
*	AllocBitMap() graphics/sprite.h
*
******************************************************************************
_FreeSpriteData::
	cmp.w	#0,a2
	beq.s	fspd_null
	move.l	a6,-(a7)
	move.l	gb_ExecBase(a6),a6
	tst.w	es_flags(a2)
	bne.s	no_free_spdata
	move.l	ss_posctldata(a2),a1
	sub.l	#4,a1
	jsr	_LVOFreeVec(a6)
no_free_spdata
	move.l	a2,a1
	move.l	#ExtSprite_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	move.l	(a7)+,a6
fspd_null:
	rts



******* graphics.library/AllocSpriteDataA **************************************
*
*   NAME
*	AllocSpriteDataA -- allocate sprite data and convert from a bitmap. (V39)
*	AllocSpriteData -- varargs stub for AllocSpriteData(). (V39)
*
*   SYNOPSIS
*	SpritePtr | 0 = AllocSpriteDataA(bitmap,taglist)
*	 d0	 	                 a2      a1
*
*	struct ExtSprite *AllocSpriteDataA( struct BitMap *, struct TagItem * );
*
*	extsprite=AllocSpriteData(bitmap,tags,...TAG_END)
*
*    FUNCTION
*	Allocate memory to hold a sprite image, and convert the passed-in
*	bitmap data to the appropriate format. The tags allow specification
*	of width, scaling, and other options.
*
*   INPUTS
*	bitmap - ptr to a bitmap. This bitmap provides the source data for the
*		sprite image.
*
*	tags -
*		SPRITEA_Width specifies how many pixels wide you desire
*		the sprite to be. Specifying a width wider than the hardware
*		can handle will cause the function to return failure. If the
*		bitmap passed in is narrower than the width asked for, then
*		it will be padded on the right with transparent pixels.
*		Defaults to 16.
*
*		SPRITEA_XReplication controls the horizontal pixel replication factor
*		used when converting the bitmap data. Valid values are:
*			0 - perform a 1 to 1 conversion
*			1 - each pixel from the source is replicated twice
*			    in the output.
*			2 - each pixel is replicated 4 times.
*		       -1 - skip every other pixel in the source bitmap
*		       -2 - only include every fourth pixel from the source.
*
*			This tag is useful for converting data from one resolution
*		to another. For instance, hi-res bitmap data can be correctly
*		converted for a lo-res sprite by using an x replication factor
*		of -1. Defaults to 0.
*
*		SPRITEA_YReplication controls the vertical pixel replication factor
*		in the same manner as SPRITEA_XReplication controls the horizontal.
*
*		SPRITEA_OutputHeight specifies how tall the resulting sprite
*		should be. Defaults to the bitmap height. The bitmap MUST be at
*		least as tall as the output height.
*
*		SPRITEA_Attached tells the function that you wish to convert
*		the data for the second sprite in an attached sprite pair.
*		This will cause AllocSpriteData() to take its data from the
*		3rd and 4th bitplanes of the passed in bitmap.
*
*
*	Bitplane data is not required to be in chip ram for this function.
*
*
*   RESULTS
*	SpritePtr = a pointer to a ExtSprite structure, or 0 if there is
*	a failure. You should pass this pointer to FreeSpriteData() when finished
*	with the sprite.
*
*   BUGS
*		Under V39, the appropriate attach bits would not be set in the sprite 
*	data.
*		The work-around is to set the bits manually. Bit 7 of the second
*	word should be set. On a 32 bit sprite, bit 7 of the 3rd word should 
*	also be set. For a 64 bit sprite, bit 7 of the 5th word should also be
*	set. This should NOT be done under V40, as the bug is fixed.
*
*   SEE ALSO
*	FreeSpriteData() FreeSprite() ChangeSprite() MoveSprite() GetExtSpriteA()
*	AllocBitMap() graphics/sprite.h
*
*********************************************************************

; sprite data layout is as follows:
;	1x: pos ctl a b a b pos ctl		high*2+4 words
;	2x: pos x ctl x a a b b pos x ctl x	high*4+8 words
;	4x: pos x x x ctl x x x  a a a a b b b b pos x x x ctl x x x  high*8+16 words
;	so allocsize=high*2*wordwidth+(2<<wordwidth) words


TEMP_SIZE	set	0	; init locals
	LONGVAR	tstate				; tag state for nexttagitem
	WORDVAR	height				; output height
	WORDVAR	wordwidth			; output width
	WORDVAR	hrep				; horizontal repetition factor.
	WORDVAR	vrep				; vertcial repetition factor.
	LONGVAR	vskip				; amount to add to bitplane ptr
						; for each line
	ARRAYVAR tempwords,32			; 16 temp words for scaling.

	WORDVAR	srcwords			; number of words to copy from each line.
	WORDVAR	pstart				; bitplane ptr offset
	LONGVAR	sp_ptr				; pointer to extsprite structure.
	WORDVAR	old_data_flag
	BVAR	attached_flag

_AllocSpriteDataA::
	movem.l	d2-d7/a2-a5,-(a7)
	ALLOCLOCALS
	clr.w	old_data_flag_w(a7)
	move.w	#bm_Planes,pstart_w(a7)
	move.l	a1,tstate_l(a7)
	move.w	bm_Rows(a2),height_w(a7)
	move.w	#1,wordwidth_w(a7)
	clr.w	vrep_w(a7)
	clr.w	hrep_w(a7)
	clr.b	attached_flag_b(a7)
	moveq	#0,d0
	move.w	bm_BytesPerRow(a2),d0
	cmp.w	#UNLIKELY_WORD,bm_Pad(a2)
	bne.s	1$
	btst	#IBMB_INTERLEAVED,bm_Flags(a2)
	beq.s	1$
; it's interleaved
	move.l	bm_Planes+4(a2),d0
	sub.l	bm_Planes(a2),d0
1$:	lsr.l	#1,d0
	move.w	d0,srcwords_w(a7)		; d0= the number of words wide the source is.
	move.l	a6,a5				; save gfxbase
	move.l	gb_UtilBase(a6),a6		; get utiity base
tag_loop:
	lea	tstate_l(a7),a0
	jsr	_LVONextTagItem(a6)		; d0=next tag item, tstate updated
	tst.l	d0
	beq.s	donetags
	move.l	d0,a0
	move.l	4(a0),d1
	move.l	(a0),d0
	sub.l	#SPRITEA_Width,d0
	bne.s	not_spa_width
; it's the width, so round to words and store.
	tst.b	d1
	bne.s	2$
	moveq	#16,d1
2$:
	add.w	#15,d1
	lsr.w	#4,d1
	cmp.w	#3,d1
	bne.s	1$
	moveq	#4,d1
1$:	move.w	d1,wordwidth_w(a7)
not_spa_width:
	subq.l	#2,d0
	bne.s	not_spa_xrep
	move.w	d1,hrep_w(a7)
not_spa_xrep:
	subq.l	#2,d0
	bne.s	not_spa_yrep
	move.w	d1,vrep_w(a7)
not_spa_yrep:
	subq.l	#2,d0
	bne.s	not_spa_oheight
	move.w	d1,height_w(a7)
not_spa_oheight:
	subq.l	#2,d0
	bne.s	not_spa_att
	tst.l	d1
	beq.s	not_spa_att
	move.w	#bm_Planes+8,pstart_w(a7)
	st	attached_flag_b(a7)
not_spa_att:
	subq.l	#2,d0
	bne.s	not_old_fmt
	move.w	d1,old_data_flag_w(a7)
not_old_fmt:
	bra	tag_loop			; ignore bad tags
donetags:
; all tags are done, so let's initialize
; vskip=bytesperrow<<max(0,-vrep)
; vrep=1<<max(0,vrep)
; srcwords=min(srcwords,wordwidth)
	move.w	vrep_w(a7),d0
	neg.w	d0
	bpl.s	1$
	moveq	#0,d0
1$:	moveq	#0,d1
	move.w	bm_BytesPerRow(a2),d1
	tst.w	old_data_flag_w(a7)
	beq.s	not_old1
	moveq	#4,d1
	move.w	#1,srcwords_w(a7)
not_old1:
	lsl.l	d0,d1
	move.l	d1,vskip_l(a7)
	move.w	wordwidth_w(a7),d0
	move.w	hrep_w(a7),d1
	bpl.s	3$
	neg.w	d1
	lsl.w	d1,d0
	bra.s	4$
3$:	lsr.w	d1,d0
	bne.s	4$
	moveq	#1,d0
4$:	cmp.w	srcwords_w(a7),d0
	bhs.s	2$
	move.w	d0,srcwords_w(a7)
2$:
; now, lets do some allocations!
	move.l	gb_ExecBase(a5),a6
	moveq	#0,d1
	move.l	#ExtSprite_SIZEOF,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,sp_ptr_l(a7)
	beq	failure
	move.l	d0,a3
	move.w	old_data_flag_w(a7),d0
	beq.s	not_old
;	BRA.s	not_old
	tst.w	hrep_w(a7)
	bne.s	not_old
	tst.w	vrep_w(a7)
	bne.s	not_old
	cmp.w	#1,wordwidth_w(a7)
	bne.s	not_old
	move.w	#1,es_wordwidth(a3)
	move.l	a2,ss_posctldata(a3)
	move.w	height_w(a7),ss_height(a3)
	st	es_flags(a3)
	bra	done_term
	
not_old:
; now, lets allocvec the sprite data!
;	so allocsize=high*2*wordwidth+(2<<wordwidth) words+ 1 lword to align
	clr.w	es_flags(a3)
	move.w	wordwidth_w(a7),es_wordwidth(a3)
	move.w	height_w(a7),d0
	move.w	d0,ss_height(a3)
	mulu	wordwidth_w(a7),d0
	add.l	d0,d0				; 2 planes
	moveq	#2,d1
	move.w	wordwidth_w(a7),d2
	lsl.l	d2,d1
	add.l	d1,d0
	add.l	d0,d0				; words->bytes
	add.l	#4,d0
	move.l	#MEMF_CHIP,d1
	jsr	_LVOAllocVec(a6)
	tst.l	d0
	beq	failure
	add.l	#4,d0
	move.l	d0,ss_posctldata(a3)
; now, let's convert it!
; d7=free
	moveq	#0,d7
	tst.b	attached_flag_b(a7)
	beq.s	1$
	bset	#7,d7
1$:
	move.l	d0,a4
	move.l	d7,(a4)+				; clear posctl but may set attached flag
	cmp.w	#2,wordwidth_w(a7)
	blt.s	gotw
	move.w	d7,(a4)+				; maybe another attach bit and clear	
	move.w	d7,(a4)+
	cmp.w	#4,wordwidth_w(a7)
	blt.s	gotw
	move.w	d7,(a4)+
	clr.w	(a4)+
	clr.l	(a4)+
gotw:
; now, a4 is pointing past the position/control data
; point a1=plane 0 data
; 	a2=plane 1 data
	tst.w	old_data_flag_w(a7)
	beq.s	is_bm
	lea	6(a2),a2
	lea	-2(a2),a1
	bra.s	got_ptrs
is_bm:
	add.w	pstart_w(a7),a2
	move.l	(a2)+,a1			; first plane
	move.l	(a2),a2				; second plane

got_ptrs:
	move.w	height_w(a7),d7		; d7=height counter
	add.l	d7,d7				; double becuase of two planes
	bra.s	end_hloop
hloop:						; clear 32 bytes=8 lwords
	lea	tempwords(a7),a0
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
	clr.l	(a0)+
; now, move source data!
	move.w	srcwords_w(a7),d0
	subq.w	#1,d0
	move.l	a2,d1
	lea	tempwords(a7),a0
1$:	move.w	(a2)+,(a0)+
	dbra	d0,1$
	move.l	d1,a2
; now, one plane of data is in tempwords!
	tst.w	hrep_w(a7)			; scale up or down?
	bpl.s	scale_up_maybe
	bsr	scale_down			; 1/2
	btst	#0,hrep_w+1(a7)			; >-1?
	bne.s	done_scale
	bsr	scale_down
done_scale:
	move.w	wordwidth_w(a7),d0
	subq	#1,d0
	lea	tempwords(a7),a0
mover:
	move.w	(a0)+,(a4)+			; output one word
	dbra	d0,mover
	add.l	vskip_l(a7),a2
end_hloop:
	exg.l	a1,a2				; swap plane ptrs
	dbra	d7,hloop
	clr.l	(a4)+				; clear posctl
	cmp.w	#2,wordwidth_w(a7)
	blt.s	done_term
	clr.l	(a4)+
	cmp.w	#4,wordwidth_w(a7)
	blt.s	done_term
	clr.l	(a4)+
	clr.l	(a4)+
done_term
	move.l	a3,d0
ex1:
	move.l	a5,a6
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d2-d7/a2-a5
	rts
failure:
	move.l	sp_ptr_l(a7),d0
	beq.s	ex1				; no cleanup, ret val=0
	move.l	d0,a1
	move.l	#ExtSprite_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	moveq	#0,d0
	bra.s	ex1


scale_up_maybe:
	beq	done_scale
	bsr.s	scale_up
	btst	#1,hrep_w+1(a7)		; could it be 2?
	beq	done_scale
	pea	done_scale(pc)
; fall scale_up	
scale_up:
; pixel magnify the 32 byte array at tempwords+4(a7)
	move.l	a1,d3
	lea	tempwords+4(a7),a0
	lea	-32(a7),a7
	move.l	a7,a1
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	a7,a1
	lea	-32(a0),a0
	moveq	#15,d0
	moveq	#0,d1
1$:	move.b	(a1),d1
	lsr.b	#4,d1
	move.b	magtbl(pc,d1.w),(a0)+
	move.b	(a1)+,d1
	and.b	#$0f,d1
	move.b	magtbl(pc,d1.w),(a0)+
	dbra	d0,1$
	lea	32(a7),a7
	move.l	d3,a1
	rts

magtbl:
	dc.b	%00000000
	dc.b	%00000011
	dc.b	%00001100
	dc.b	%00001111
	dc.b	%00110000
	dc.b	%00110011
	dc.b	%00111100
	dc.b	%00111111
	dc.b	%11000000
	dc.b	%11000011
	dc.b	%11001100
	dc.b	%11001111
	dc.b	%11110000
	dc.b	%11110011
	dc.b	%11111100
	dc.b	%11111111

scale_down:
; pixel shrink the 32 byte array at tempwords+4(a7)
	moveq	#15,d0
	lea	tempwords+4(a7),a0	; input
	move.l	a1,d3
	move.l	a0,a1			; output
1$:	moveq	#0,d1
	move.w	(a0)+,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	add.w	d2,d2
	addx.w	d1,d1
	add.w	d2,d2
	move.b	d1,(a1)+
	dbra	d0,1$
	clr.l	(a1)+
	clr.l	(a1)+
	clr.l	(a1)+
	clr.l	(a1)+
	move.l	d3,a1
	rts


	xdef    _GetSprite
	xref    _getsprite
******* graphics.library/GetSprite ******************************************
*
*   NAME
*	GetSprite -- Attempt to get a sprite for the simple sprite manager.
*
*   SYNOPSIS
*	Sprite_Number = GetSprite( sprite, pick )
*	    d0			    a0      d0
*
*	WORD GetSprite( struct SimpleSprite *, WORD );
*
*   FUNCTION
*	Attempt to allocate one of the eight sprites for private use
*	with the simple sprite manager. This must be done before using
*	further calls to the simple sprite machine. If the programmer
*	wants to use 15 color sprites, they must allocate both sprites
*	and set the 'SPRITE_ATTACHED' bit in the odd sprite's posctldata
*	array. 
*
*   INPUTS
*	sprite - ptr to programmers SimpleSprite structure.
*	pick - number in the range of 0-7 or
*	  -1 if programmer just wants the next one.
*
*   RESULTS
*	If pick is 0-7 attempt to allocate the sprite. If the sprite
*	is already allocated then return -1.
*	If pick -1 allocate the next sprite starting search at 0.
*	If no sprites are available return -1 and fill -1 in num entry
*	of SimpleSprite structure.
*	If the sprite is available for allocation, mark it allocated
*	and fill in the 'num' entry of the SimpleSprite structure.
*	If successful return the sprite number.
*
*   BUGS
*
*   SEE ALSO
*	FreeSprite() ChangeSprite() MoveSprite() GetSprite() graphics/sprite.h
*
*********************************************************************
_GetSprite:
	move.l	gb_ExecBase(a6),a1
	FORBID	a1,NOFETCH
	movem.l	d0/a0,-(a7)
	moveq	#1,d0
	moveq	#-1,d1
	bsr	change_sprite_width
	tst.w	d0
	movem.l	(a7)+,d0/a0
	beq.s	no_sprite_to_get
	tst.w	d0
	bpl.s	get_specific_sprite
; user wants next available sprite
	move.b	gb_SpriteReserved(a6),d0
	moveq	#7,d1
1$:	lsr.b	#1,d0
	dbcc	d1,1$
	bcs.s	no_sprite_to_get
	moveq	#7,d0
	sub	d1,d0
; now, d0=sprite # to get
get_specific_sprite:
	and	#7,d0
	btst.b	d0,gb_SpriteReserved(a6)
	bne.s	no_sprite_to_get
	bset.b	d0,gb_ScanDoubledSprites(a6)	; those who come in through the old interface
	bset.b	d0,gb_SpriteReserved(a6)		; always get scan-doubled sprites.
	bclr.b	d0,gb_ExtSprites(a6)			; not an extsprite!
got_sp	move.l	gb_ExecBase(a6),a1
	PERMIT	a1,NOFETCH
	move.w	d0,ss_num(a0)
	rts
no_sprite_to_get:
	moveq	#-1,d0
	bra.s	got_sp


	xdef    _FreeSprite

******* graphics.library/FreeSprite ***********************************
*
*   NAME
*       FreeSprite -- Return sprite for use by others and virtual
*					  sprite machine.
*
*   SYNOPSIS
*       FreeSprite( pick )
*                    d0
*
*	void FreeSprite( WORD );
*
*   FUNCTION
*	Mark sprite as available for others to use.
*       These sprite routines are provided to ease sharing of sprite
*	hardware and to handle simple cases of sprite usage and
*	movement.  It is assumed the programs that use these routines
*	do want to be good citizens in their hearts. ie: they will
*	not FreeSprite unless they actually own the sprite.
*	The Virtual Sprite machine may ignore the simple sprite machine.
*
*   INPUTS
*       pick - number in range of 0-7
*
*   RESULTS
*	sprite made available for subsequent callers of GetSprite
*	as well as use by Virtual Sprite Machine.
*
*   BUGS
*
*   SEE ALSO
*       GetSprite() ChangeSprite() MoveSprite() graphics/sprite.h
*
*********************************************************************
_FreeSprite:
	move.l	gb_ExecBase(a6),a1
	FORBID	a1,NOFETCH
	bclr.b	d0,gb_SpriteReserved(a6)
	bclr.b	d0,gb_ExtSprites(a6)
	bclr.b	d0,gb_SoftSprites(a6)
	move.w	d0,d1
	add.w	d1,d1
	add.w	d1,d1
	move.l	gb_SimpleSprites(a6),a0
	add.w	d1,a0
	move.l	(a0),d1
	clr.l	(a0)

; set the sprite to the blank sprite by poking copinit
	move.l	gb_copinit(a6),a0
	lsl.w	#3,d0
	lea	copinit_sprstrtup(a0,d0.w),a1
	move.l	#copinit_sprstop,d0
	add.l	a0,d0
	move.w	d0,6(a1)
	swap	d0
	move.w	d0,2(a1)
; now, see if it is time to switch back to the desired intuition mode
	moveq	#-1,d1
	move.w	gb_DefaultSpriteWidth(a6),d0
	bsr	change_sprite_width		; attempt to change width
	move.l	gb_ExecBase(a6),a1
	PERMIT	a1,NOFETCH	
	rts

******* graphics.library/ChangeExtSpriteA *****************************************
*
*   NAME
*       ChangeExtSpriteA -- Change the sprite image pointer. (V39)
*	ChangeExtSprite  -- varargs stub for ChangeExtSpriteA(). (V39)
*
*   SYNOPSIS
*       ChangeExtSpriteA( vp, oldsprite, newsprite, tags)
*                     	  a0  a1   	 a2	    a3
*
*	success=ChangeExtSpriteA(struct ViewPort *, struct ExtSprite *,
*			struct ExtSprite *, struct TagList *);
* 
*	success=ChangeExtSprite(vp,old_sp,new_sp,tag,....);
*
*   FUNCTION 
*	Attempt to change which sprite is displayed for a given
*	sprite engine.
*
*   INPUTS
*       vp - pointer to ViewPort structure that this sprite is
*		  relative to,  or 0 if relative only top of View
*	oldsprite - pointer the old ExtSprite structure
*	newsprite - pointer to the new ExtSprite structure. 
*
*   RESULTS 
* 	success - 0 if there was an error.
*   BUGS 
* 
*   SEE ALSO
*	FreeSprite() ChangeSprite() MoveSprite() AllocSpriteDataA()
*	graphics/sprite.h
* 
*********************************************************************
TEMP_SIZE	set	0
	LONGVAR	tstate

_ChangeExtSpriteA::
	movem.l	d7/a0-a2/a5-a6,-(a7)
	ALLOCLOCALS
	move.l	a3,tstate_l(a7)
	moveq	#0,d7			; dbl flag
	movem.l	a0/a1/a6,-(a7)
	move.l	gb_UtilBase(a6),a6
cst_tag_loop:
	lea	tstate_l+4*3(a7),a0
	jsr	_LVONextTagItem(a6)
	tst.l	d0
	beq.s	cst_donetags
	move.l	d0,a0
	move.l	4(a0),d1
	move.l	(a0),d0
	sub.l	#GSTAG_SCANDOUBLED,d0
	bne.s	cst_nodbl
	move.l	d1,d7
cst_nodbl:
	bra.s	cst_tag_loop
cst_donetags:
	movem.l	(a7)+,a0/a1/a6

	move.l	gb_ExecBase(a6),a5
	FORBID	a5,NOFETCH
	move.w	es_wordwidth(a2),d0
	move.w	ss_num(a1),d1
	move.l	a1,-(a7)
	bsr	change_sprite_width
	move.l	(a7)+,a1
	tst.l	d0
	bne.s	its_cool
	move.w	ss_num(a1),ss_num(a2)	; should I be dooing this?
 	move.l	a5,a6
	jsr	_LVOPermit(a6)
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d7/a0-a2/a5-a6
	moveq	#0,d0
	rts
its_cool:
	move.w	ss_num(a1),d0
	bclr.b	d0,gb_SprMoveDisable(a6)
	bclr.b	d0,gb_ScanDoubledSprites(a6)
	tst.l	d7
	beq.s	no_ctsdbl1
	bset.b	d0,gb_ScanDoubledSprites(a6)
no_ctsdbl1:
	move.w	d0,ss_num(a2)
	move.l	a5,a6
	jsr	_LVOPermit(a6)
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d7/a0-a2/a5-a6
	movem	ss_x(a1),d0/d1
	move.l	a2,a1
	jsr	_LVOMoveSprite(a6)
	moveq	#-1,d0
	rts

	xdef    _ChangeSprite
	xref    _changesprite
******* graphics.library/ChangeSprite ******************************************
*
*   NAME
*       ChangeSprite -- Change the sprite image pointer.
*
*   SYNOPSIS
*       ChangeSprite( vp, s, newdata)
*                     a0  a1   a2
*
*	void ChangeSprite(struct ViewPort *, struct SimpleSprite *, void * )
* 
*   FUNCTION 
*	The sprite image is changed to use the data starting at newdata
*
*   INPUTS
*       vp - pointer to ViewPort structure that this sprite is
*		  relative to,  or 0 if relative only top of View
*	s - pointer to SimpleSprite structure
*	newdata	- pointer to data structure of the following form.
*		struct spriteimage
*		{
*		    UWORD    posctl[2];	/* used by simple sprite machine*/
*		    UWORD    data[height][2];   /* actual sprite image */
*		    UWORD    reserved[2];	/* initialized to */
*			                             /*  0x0,0x0 */
*		};
*	The programmer must initialize reserved[2].  Spriteimage must be
*	in CHIP memory. The height subfield of the SimpleSprite structure
*	must be set to reflect the height of the new spriteimage BEFORE
*	calling ChangeSprite(). The programmer may allocate two sprites to
*	handle a single attached sprite.  After GetSprite(), ChangeSprite(),
*	the programmer can set the SPRITE_ATTACHED bit in posctl[1] of the
*	odd numbered sprite.
*	If you need more than 8 sprites, look up VSprites in the
*	graphics documentation.
*
*   RESULTS 
* 
*   BUGS 
* 
*   SEE ALSO
*	FreeSprite() ChangeSprite() MoveSprite() AddVSprite() graphics/sprite.h
* 
*********************************************************************
_ChangeSprite:
	move.l  a2,ss_posctldata(a1)
	movem.w	ss_x(a1),d0/d1
;	fall MoveSprite

******* graphics.library/MoveSprite ******************************************
*
*   NAME
*	MoveSprite -- Move sprite to a point relative to top of viewport.
*
*   SYNOPSIS
*	MoveSprite(vp, sprite, x, y)
*	           A0  A1      D0 D1
*
*	void MoveSprite(struct ViewPort *,struct SimpleSprite *, WORD, WORD);
*
*   FUNCTION
*	Move sprite image to new place on display.
*
*   INPUTS
*	vp - pointer to ViewPort structure
*	     if vp = 0, sprite is positioned relative to View.
*	sprite - pointer to SimpleSprite structure
*	(x,y)  - new position relative to top of viewport or view.
*
*   RESULTS
*	Calculate the hardware information for the sprite and
*	place it in the posctldata array. During next video display
*	the sprite will appear in new position.
*
*   BUGS
*	Sprites really appear one pixel to the left of the position you specify.
*	This bug affects the apparent display position of the sprite on the
*	screen,	but does not affect the numeric position relative to the
*	viewport or view. This behaviour only applies to SimpleSprites,
*	not to ExtSprites.
*
*
*   SEE ALSO
*	FreeSprite()  ChangeSprite()  GetSprite()  graphics/sprite.h
*
*********************************************************************

	xdef	_MoveSprite

_MoveSprite:
; do necessary pokes and call the movesprite ptr in the vector table
	move.l	a2,-(a7)
	cmp	#0,a0
	beq.s	view_relative_spmove
	move.l	a0,a2
got_vp:	vp_to_vector	a2,vt_MoveSprite,_DefaultMoveSprite(pc)
got_a2:	jsr	(a2)
	move.l	(a7)+,a2
	rts
view_relative_spmove:
	move.l	gb_ActiView(a6),a0
	cmp	#0,a0
	beq.s	goat
	move.l	v_ViewPort(a0),a2
	sub.l	a0,a0
	cmp	#0,a2
	bne.s	got_vp
goat:	lea	_DefaultMoveSprite(pc),a2
	bra.s	got_a2

	xref	_intena
	xdef    _DefaultMoveSprite

_DefaultMoveSprite:

ms_amovesprite:


	move.w	d0,ss_x(a1)
	move.w	d1,ss_y(a1)
	movem.l	d2-d7/a2-a4,-(sp)	; preserve registers
	move.w	ss_num(a1),d6
	btst.b	d6,gb_SprMoveDisable(a6)
	bne	moves_exit
	move.w	gb_SpriteWidth(a6),d6		; d6=sprite width in words

got_spritesize:
	move.l	a0,a2		  	; viewport pointer
	move.l	a1,a3		  	; simplesprite pointer
	move.l	ss_x(a3),d2	  	; x in d2 high, y in d2 low

;   safe to call self after this point

ms_safe: 
	move.l	gb_ActiView(a6),d3
	beq	ms_arts
	move.l	d3,a0

ms_view:
 
	move.l	v_DyOffset(a0),d3 
	add.w	d3,d3
	add.w	d3,d3			; convert to superhires
	swap	d3 		  	; startx in d3 high, starty in d3 low

ms_check_attached:

	move.l	gb_SimpleSprites(a6),a4 ; ns
	move.w	ss_num(a3),d4
	add.w	d4,d4			; convert sprite num
	move.w	d4,d5			; needed later
	add.w	d4,d4			; to longword offset
	move.l	a3,0(a4,d4.w)	 	; ns[num] = s;

; check for sprite attached
	btst	#1,d5			; odd sp?
	bne.s	ms_not_attached
	move.l	4(a4,d4.w),d0
	beq.s	ms_not_attached		; not attached if ss[n+1]==0
	move.l	d0,a0
	move.l	ss_posctldata(a0),a0
	btst	#7,3(a0)		; attach bit set?
	beq.s	ms_not_attached

	move.l	a2,a0		 	; viewport
	move.l	d0,a1	      		; attached simplesprite!
	move.w	ss_x(a3),d0
	move.w	ss_y(a3),d1
	ext.l	d0
	ext.l	d1
	bsr	ms_amovesprite		; assembly direct
ms_not_attached:

	move.l	ss_posctldata(a3),a1	; k = s->posctldata
	move.l	a1,d0			
	beq	ms_arts

	moveq.l	#0,d4			; 140 ns scrolling

	move.l	a2,-(sp)		; viewport?
	bne.s	ms_yesvp
	swap	d2
	add.w	d2,d2
	add.w	d2,d2
	swap	d2
	bra.s	ms_novp	; was short

ms_yesvp:
;   move.w	vp_Modes(a2),d0 ; don't use vp->Modes directly for 1.4

	xref	_new_mode
	jsr	_new_mode
;	move.l	d0,(sp)			; remember to pop from stack after clipping sprite

	move.l	ss_posctldata(a3),a1	; reload k after "C" call
	move.l	vp_DxOffset(a2),d1	; dxoffset in d1 high, dyoffset in d1 low

	add.w	d1,d2			; y += vp->DyOffset -- greater accuracy
	btst	#3,d0
	beq.s	no_double
	add.w	d2,d2
no_double:

	btst.l	#5,d0
	beq.s	ms_nosuper
	bclr.l	#15,d0

ms_nosuper:
	btst.l	#2,d0
	beq.s	ms_nolace
	asr.w	#1,d2			; y >>= 1

ms_nolace: 
	swap	d1			; vp->dxoffset now in d1 low
	swap	d2 			; will have to swap this back later	
	swap	d3 			; will have to swap this back later	

	add.w	d1,d2			; x += vp->DxOffset -- greater accuracy

	btst	#5,d0
	bne.s	shifted_x
	add.w	d2,d2
	btst	#15,d0
	bne.s	shifted_x
	add.w	d2,d2
shifted_x:
; now have scroll value in superhires pixels
; now, if ECS, then we must clear the 35ns bit and 70ns unless we are shires
	btst	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	bne.s	ms_swapback
	bclr	#0,d2		; clear 35ns bit
	btst	#5,d0		; shres
	bne.s	ms_swapback
	bclr	#1,d2		; clear 70ns bit
	

ms_swapback:
	move.w	ss_num(a3),d1
	btst	d1,gb_ExtSprites(a6)	; is it an ExtSprite?
	beq.s	no_fixbug
	subq.w	#4,d2					; if so, no off-by-1 bug!

no_fixbug:	swap	d2 			; swap this back now
	swap	d3		 	; swap this back now

ms_novp: 
	add	#4,sp
	add.w	d2,d3			; starty += y;
	move.w	d3,d2

	move.w	#21,d1			; outdated - presupposes NTSC - bart 

	move.l	gb_current_monitor(a6),d0 ; fetch correct min_row instead - bart
	beq.s	ms_nomspc
	move.l	d0,a0
	move.w	ms_min_row(a0),d1
	btst	#GFXB_AA_MLISA,gb_ChipRevBits0(a6)
	bne.s	ms_nomspc
	btst	#MSB_REQUEST_SPECIAL,ms_Flags+1(a0)
	beq.s	ms_nomspc
	addq.w	#1,d1

ms_nomspc:
	sub.w	d1,d2 
	bge.s	ms_noclip
	sub.w	d2,d3			; starty = MAX(21,starty);
ms_noclip:
	move.w	d3,d1		        ; starty for jam
	swap	d2
	swap	d3
	add.w	d2,d3			; startx += x;
	add.w	ss_height(a3),d1	; stopy for jam

ms_check_wrap:
	cmp.w	#2048,d0
	blo.s	no_satur
	move.w	#2047,d0
no_satur:

	moveq.l	#0,d0			; build jam in d0

ms_jamhigh: 
	lsr.w	#1,d3			; (startx>>1)
	bcc.s	no_35ns
	bset	#3,d0
no_35ns:
	lsr.w	#1,d3
	bcc.s	no_70ns
	bset	#4,d0
no_70ns:
	lsr.w	#1,d3
	bcc.s	no_140ns
	addq.w	#1,d0
no_140ns:
	move.b	d3,d2 			; ((startx>>1)&0xff)
	swap	d0			; (startx & 1) in upper word, zero in lower word
	swap	d3			; starty now in lower word
	move.w	d3,d0
	lsl.w	#8,d0			; (starty<<8)
	move.b	d2,d0   		; (starty<<8)|((startx>>1)&0xff) in lower word
	btst	#MSB_DOUBLE_SPRITES,gb_MonitorFlags+1(a6)	; not set in ECS.
	beq.s	no_sdbl
	move.b	ss_num+1(a3),d2
	btst	d2,gb_ScanDoubledSprites(a6)
	beq.s	no_sdbl
	add.w	ss_height(a3),d1	; double height
	bset	#7,d0

no_sdbl:	
	swap	d0			; upper word of jam is now finished
ms_jamlow: 

	move.b	3(a1),d2
	and.b	#$80,d2			; sprite attached?
	or.b	d2,d0
	btst.l	#8,d3
	beq.s	no_sv8
	bset	#2,d0

no_sv8:
	btst.l	#9,d3
	beq.s	no_sv9
	bset	#6,d0

no_sv9:
	move.b	d1,d2			; stopy low byte
	lsl.w	#8,d2
	or.w	d2,d0			; ((stopy&0xff)<<8)
	btst.l	#8,d1
	beq.s	no_ev8
	bset	#1,d0

no_ev8:
	btst.l	#9,d1
	beq.s	no_ev9
	bset	#5,d0

no_ev9:

	move.w	#3,d1
	move.w	#2,d2
	sub.w	d4,d2
	lsr.w	d2,d1
	beq.s	ms_jammit
	and.w	ss_x(a3),d1
	add.w	#3,d2
	lsl.w	d2,d1
	or.w	d1,d0

ms_jammit:

	include 'hardware/custom.i'

	xref	_custom
	xref	_LVODisable
	xref	_LVOEnable

FIRST_SPRITE_DMA	equ	$18	; end of first sprite dma slot
LAST_SPRITE_DMA	equ	$34		; end of last sprite dma slot

	btst	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	bne.s	no_zero_ecs_bits
	and.w	#$ffff-$18,d0	; zero low order hpos bits
no_zero_ecs_bits:
	moveq	#LAST_SPRITE_DMA,d2
	move.l	a5,-(sp)
	move.l	d0,d4
	move.l	a1,a5
	lea	_custom+vhposr,a4	; for beam pos check code below
	move.l	gb_ExecBase(a6),a0
	DISABLE	a0,NOFETCH
	move.w	(a4),d1			; horizontal pos in d1:8
	cmp.b	d2,d1
	bhi.s	do_jammit		; after sprite dma slots
	moveq	#FIRST_SPRITE_DMA,d2
	add.b	d5,d2			; 4 slots per sprite (from above)
1$:
	move.w	(a4),d1			; horizontal pos in d1:8
	cmp.b	d2,d1
	bls.s	1$			; busy wait until after this sprite's slot (Yeuchh!)
do_jammit:
; d6=sprite size
	move.l	d4,(a5)			; jam that sucker!	
	btst	#0,d6
	bne.s	done_jammit
	btst	#2,d6
	beq.s	sprite_2x
	addq	#4,a5
sprite_2x:
	move.w	d4,4(a5)
done_jammit:
	ENABLE	a0,NOFETCH
	move.l	(sp)+,a5
	
; now, 
; if viewport relative then
;   if  (vp->Modes & VP_HIDE)
;    or (vp->DHeight < spritey)
;    or (y>>lace) + shheight <0
;	then make it disappear
;  else
;	make it appear
;

; a2=viewport (if any)
; a3=simplesprite

ms_arts:
	move.l	gb_copinit(a6),a0
	lea	copinit_sprstrtup(a0),a0
	move.w	ss_num(a3),d0
	add.w	d0,d0
	add.w	d0,d0			; do=sprite#*4
	move	d0,d2
	add.w	d2,d2			; now (a0,d2) points at copins

	cmp	#0,a2
	beq.s	make_them_appear
	btst.b	#5,vp_Modes(a2)		; VP_HIDE?
	bne.s	make_them_disappear
	move.w	ss_y(a3),d1		; spritey
	cmp.w	vp_DHeight(a2),d1	; y>high
	bgt.s	make_them_disappear
	btst	#2,vp_Modes+1(a2)	; lace?
	beq.s	nolace
	asr.w	#1,d1
nolace	add.w	ss_height(a3),d1	; high+y
	bpl.s	make_them_appear

make_them_disappear:
; now, poke copinit
	lea	copinit_sprstop-copinit_sprstrtup(a0),a1
	move.w	copinit_sprstop-copinit_sprstrtup+2(a0),d7
	move.l	a1,d6
	move.w	d6,d7
	swap	d6


	move.w	d6,2(a0,d2.w)
	move.w	d7,6(a0,d2.w)

	btst	#2,d0			; odd sprite? can't be attached
	bne.s	moves_exit
	move.l	gb_SimpleSprites(a6),a1
	move.l	4(a1,d0.w),d1
	beq.s	moves_exit		; not attached if ss ptr=nil
	move.l	d1,a1
	move.l	ss_posctldata(a1),a1
	btst	#7,3(a1)		; attach bit set?
	beq.s	moves_exit
	move.w	d6,10(a0,d2.w)
	move.w	d7,14(a0,d2.w)
moves_exit:
	movem.l	(sp)+,d2-d7/a2-a4	; restore registers
	rts

make_them_appear:
; now, poke copinit
	move.l	ss_posctldata(a3),d7
	move.w	d7,6(a0,d2.w)
	swap	d7
	move.w	d7,2(a0,d2.w)

	btst	#2,d0			; odd sprite? can't be attached
	bne.s	moves_exit
	move.l	gb_SimpleSprites(a6),a1
	move.l	4(a1,d0.w),d1
	beq.s	moves_exit		; not attached if ss ptr=nil
	move.l	d1,a1
	move.l	ss_posctldata(a1),a1
	btst	#7,3(a1)		; attach bit set?
	beq.s	moves_exit
	move.l	a1,d6
	move.w	d6,14(a0,d2.w)
	swap	d6
	move.w	d6,10(a0,d2.w)
	movem.l	(sp)+,d2-d7/a2-a4	; restore registers
	rts

	end
