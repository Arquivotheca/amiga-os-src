*******************************************************************************
*
*	$Id: ScrollVPort.asm,v 42.0 93/06/16 11:13:43 chrisg Exp $
*
*******************************************************************************

	section	graphics

	include 'exec/types.i'
	include '/view.i'
	include	"/vp_internal.i"
	include	"/macros.i"
	include	'/gfxbase.i'
	include	'hardware/custom.i'

    xdef    _ScrollVPort,_scrollvport

GOOD_CLIPSTUFF	equ	1

******* graphics.library/ScrollVPort ***************************************
*
*   NAME
*	ScrollVPort -- Reinterpret RasInfo information in ViewPort to reflect
*			the current Offset values.
*
*   SYNOPSIS
*	ScrollVPort( vp )
*		     a0
*
*	void ScrollVPort(struct ViewPort *vp);
*
*   FUNCTION
*	After the programmer has adjusted the Offset values in
*	the RasInfo structures of ViewPort, change the
*	the copper lists to reflect the the Scroll positions.
*	Changing the BitMap ptr in RasInfo and not changing the
*	the Offsets will effect a double buffering affect.
*
*   INPUTS
*       vp - pointer to a ViewPort structure
*	     that is currently be displayed.
*   RESULTS
*	modifies hardware and intermediate copperlists to reflect
*	new RasInfo
*
*   BUGS
*       pokes not fast enough to avoid some visible hashing of display (V37)
*	This function was re-written in V39 and is ~10 times faster than
*	before.
*
*   SEE ALSO
*	MakeVPort() MrgCop() LoadView()  graphics/view.h
*
*********************************************************************

_ScrollVPort:
	move.l	a0,a1
	vp_to_vector	a1,vt_ScrollVP,_scrollvport
	jmp	(a1)

	xref	_LVOObtainSemaphore
	xref	_LVOReleaseSemaphore
	xref	_getclipstuff,_new_mode

_scrollvport:

TEMP_SIZE	set	0
	ARRAYVAR	bd,bd_SIZEOF	; BuildData structure for viewport
	ARRAYVAR	plane_ptrs,8*4	; massaged bitplane pointers.
	WORDVAR	modes			; new_mode(vp)
	WORDVAR	viewmodes		; mode bits for the View.
	WORDVAR	view_y			; ycoord of view.
	WORDVAR	bpr1			; BytesPerRow of first bitmap
	WORDVAR	bpr2			; BytesPerRow of second bitmap
	BVAR	intermedflag		; set if no intermed_update

	movem.l	a3/a4/a5/d2/d3/d4,-(a7)
	ALLOCLOCALS
	clr.b	intermedflag_b(a7)
	move.l	a0,a5
	move.l	a6,a1
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)
	move.l	a1,a6

	btst.b	#5,vp_Modes(a5)		; vp_hide?
	bne	bad_clipstuff			; nothing to do.
	move.l	a5,-(a7)
	bsr	_new_mode
	lea	4(a7),a7
	move.w	d0,modes_w(a7)
	
	sub.l	a4,a4			; a4=vpe
	move.l	vp_ColorMap(a5),d0
	beq.s	no_setsvpfl
	move.l	d0,a0
	tst.b	cm_Type(a0)
	beq.s	no_setsvpfl
	move.l	cm_vpe(a0),d0
	beq.s	1$
	move.l	d0,a4
;	cmp.l	vpe_ViewPort(a4),a5	; backpointer not always valid?
;	beq.s	1$
;	sub.l	a4,a4
1$:	btst	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
	sne	intermedflag_b(a7)
no_setsvpfl:
	move.l	gb_ActiView(a6),d0
	beq	bad_clipstuff
	move.l	d0,a0
	move.w	v_DyOffset(a0),view_y_w(a7)
	move.w	v_Modes(a0),viewmodes_w(a7)
	pea	bd(a7)
	movem.l	a0/a5,-(a7)
	bsr	_getclipstuff
	lea	4*3(a7),a7
	cmp.l	#GOOD_CLIPSTUFF,d0
	bne	bad_clipstuff
; now, lets set plane_ptrs to the proper format plane pointer array
	lea	plane_ptrs(a7),a1
	move.l	vp_RasInfo(a5),a0
	tst.l	ri_Next(a0)
	beq.s	not_dual
	btst	#10-8,modes_w(a7)	; dualpf?
	bne.s	is_dual
not_dual:
	moveq	#7,d0
	move.l	ri_BitMap(a0),a0
	cmp.b	#8,bm_Depth(a0)
	beq.s	3$

	bclr.b	#11-8,modes_w(a7)	; clear ham bit if not 8 bit ham
3$:
	move.w	bm_BytesPerRow(a0),d1
	move.w	d1,bpr1_w(a7)
	move.w	d1,bpr2_w(a7)
	lea	bm_Planes(a0),a0
1$:	move.l	(a0)+,d1
	add.l	bd_Offset+bd(a7),d1
	move.l	d1,(a1)+
	dbra	d0,1$
	bra.s	swizzled_ptrs
is_dual:
	move.l	a2,-(a7)
	move.l	ri_Next(a0),a2
	move.l	ri_BitMap(a2),a2
	move.l	ri_BitMap(a0),a0
	move.w	bm_BytesPerRow(a0),bpr1_w+4(a7)
	move.w	bm_BytesPerRow(a2),bpr2_w+4(a7)
	lea	bm_Planes(a0),a0
	lea	bm_Planes(a2),a2
	moveq	#3,d0
2$:	move.l	(a0)+,d1
	add.l	bd_Offset+bd+4(a7),d1
	move.l	d1,(a1)+
	move.l	(a2)+,d1
	add.l	bd_Offset2+bd+4(a7),d1
	move.l	d1,(a1)+
	dbra	d0,2$
	move.l	(a7)+,a2

swizzled_ptrs:
	lea	vpe_cop1ptr(a4),a3	; where to store copcache, or smallnum
	move.l	vp_DspIns(a5),a0
	cmp	#0,a0
	beq	bad_clipstuff
	move.l	cl_CopLStart(a0),a1	; a1=hw list for long frame
	moveq	#0,d2
	bsr	do_hw
; now, do  short frame if there is one
	btst	#2,viewmodes_w+1(a7)	; view laced?
	beq.s	no_2nd_field
	btst	#2,modes_w+1(a7)	; vp laced?
	beq.s	scandoubled_lace
	moveq	#-1,d2			; allow address add
scandoubled_lace:
	lea	vpe_cop2ptr(a4),a3	; cache location, or smallnum
	move.l	vp_DspIns(a5),a0
	move.l	cl_CopSStart(a0),a1
	bsr	do_hw
no_2nd_field:
; now, do intermediate copper list
	tst.b	intermedflag_b(a7)
	bne	bad_clipstuff
	move.l	vp_DspIns(a5),a0
	move.l	cl_CopIns(a0),d0
	beq	bad_clipstuff
	move.l	d0,a0

; now, a0 is pointing at the intermediate copper list
; first, find ddfstrt
; terminated with wait hpos 255
; warning!! this code won't work if the structure of
; an intermediate copper list node changes!
sync_intermed:
	move.w	(a0)+,d0		; opcode
	and.w	#3,d0			; mask other bits
	beq.s	is_move
	subq.w	#1,d0			; is wait?
	beq.s	is_wait
	move.l	(a0),a0			; it's nextbuf
	bra.s	sync_intermed	
is_wait:
	cmp.w	#255,2(a0)		; termination?
	beq	bad_clipstuff
goback:	lea	4(a0),a0
	bra.s	sync_intermed
is_move:
	move.w	(a0),d4
	cmp.w	#ddfstrt,d4		; found?
	bne.s	goback
; now, we're synced!
; a0 points at regnum
	move.w	bd_DDFStrt+bd(a7),2(a0)
	move.w	bd_DDFStop+bd(a7),2+6(a0)
	move.w	bd_bplcon1+bd(a7),2+12(a0)
	move.w	bd_Modulo+bd(a7),2+18(a0)
	lea	24(a0),a0		; point at mod reg
	move.w	(a0),d4
	cmp.w	#bpl2mod,d4		; is 2nd modulo here?
	bne.s	no_2nd_mod
	move.w	bd_Modulo2+bd(a7),2(a0)
	lea	6(a0),a0
no_2nd_mod:
; now, we are pointing at bitplane pointers!
	lea	plane_ptrs(a7),a1
	lea	-2(a0),a0
pl_lp2	move.w	(a0)+,d0		; fetch opcode
	move.w	(a0)+,d1			; fetch register #
	sub.w	#bplpt,d1
	cmp.w	#7*4,d1
	bhi.s	done_ptim
	btst	#11-8,modes_w(a7)
	beq.s	not_ham8i
	sub.w	#2*4,d1
	and.w	#7*4,d1
not_ham8i:
	move.l	0(a1,d1.w),d2		; fetch pointer
	add.w	d0,d0			; short frame?
	bpl.s	ok1
	move.w	bpr1_w(a7),d3
	btst	#2,d1
	beq.s	even_planei
	move.w	bpr2_w(a7),d3
even_planei:
	ext.l	d3
	add.l	d3,d2
ok1:	move.w	d2,6(a0)
	swap	d2
	move.w	d2,(a0)
	lea	6+2(a0),a0
	bra.s	pl_lp2
done_ptim:
; now, there could be a diwhigh followed by an fmode
; a0 points at a value field, so bump it back to point to an opcode
	lea	-4(a0),a0
sniff_diw:
	move.l	(a0),d4
	and.l	#$00ffffff,d4	; clear shf, lof, other bits
	cmp.l	#(COPPER_MOVE<<16)+diwhigh,d4
	bne.s	no_diwskip
	lea	6(a0),a0
	bra.s	sniff_diw	; could be lof and shf, so loop
no_diwskip:
	move.l	(a0),d4
	cmp.l	#(COPPER_MOVE<<16)+fmode,d4
	bne.s	not_fmode
	move.w	bd_FudgedFMode+bd(a7),4(a0)
not_fmode:

; still to do:
; beam-sync
bad_clipstuff:
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	a6,a1
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
	move.l	a1,a6
no_svp:
	lea	TEMP_SIZE(a7),a7	; pop locals
	movem.l	(a7)+,a3/a4/a5/d2/d3/d4
	rts


do_hw:
; a1 points at a hw copper list
; a3 is either <=sizeof(vpe), or points at a location cache
; d2=offset ANDer (0 for frame0, -1 for frame1)
; accesses local variables one call above!!!
; now, search for the bplcon0
;AA: ddfstrt ddfstop bplcon1 bpl1mod bpl2mod ptrs diwhigh fmode
;ECS:ddfstrt ddfstop bplcon1 bpl1mod bpl2mod ptrs diwhigh
;A:  ddfstrt ddfstop bplcon1 bpl1mod bpl2mod ptrs wait

	cmp.w	#0,a1
	beq	done_hw
	cmp.w	#vpe_SIZEOF,a3
	ble.s	sync_hw_long
	move.l	(a3),d1
	beq.s	sync_hw_long
	move.l	d1,a1			; cache hit! on a 256 color screen, this will save >1024
					; iterations of the loop below!
sync_hw_long:
	cmp.w	#ddfstrt,(a1)
	beq.s	synced_long
	lea	4(a1),a1
	bra.s	sync_hw_long
synced_long:
	cmp.w	#vpe_SIZEOF,a3
	ble.s	no_updcache
	move.l	a1,(a3)
no_updcache:
	move.w	4+bd_DDFStrt+bd(a7),2(a1)
	move.w	4+bd_DDFStop+bd(a7),6(a1)
	move.w	4+bd_bplcon1+bd(a7),10(a1)
	move.w	4+bd_Modulo+bd(a7),14(a1)
	lea	4*4(a1),a1
; now, we are pointing at a bpl2mod or at the pointers.
	cmp.w	#bpl2mod,(a1)
	bne.s	no_2nd_modulo
	move.w	4+bd_Modulo2+bd(a7),2(a1)
	lea	4(a1),a1
no_2nd_modulo:
; now, we are pointing at the plane pointers.
	lea	4+plane_ptrs(a7),a0
pl_lp:	move.w	(a1),d1
	sub.w	#bplpt,d1
	cmp.w	#7*4,d1
	bhi.s	done_ptrs
	btst	#11-8,modes_w+4(a7)
	beq.s	not_ham8
	sub.w	#2*4,d1	; compensate for ham stupid mode
	and.w	#7*4,d1
not_ham8:
	move.w	bpr1_w+4(a7),d3
	btst	#2,d1
	beq.s	even_plane
	move.w	bpr2_w+4(a7),d3
even_plane:
	and.l	d2,d3
	move.l	0(a0,d1.w),d1
	add.l	d3,d1
	move.w	d1,6(a1)
	swap	d1
	move.w	d1,2(a1)
	lea	8(a1),a1
	bra.s	pl_lp
done_ptrs:
; now, we are pointing at fmode (maybe)
	cmp.w	#diwhigh,(a1)
	bne.s	no_dwh
	lea	4(a1),a1
no_dwh
	cmp.w	#fmode,(a1)
	bne.s	no_fmpoke
	move.w	4+bd_FudgedFMode+bd(a7),2(a1)
no_fmpoke:
done_hw:
	rts
	end
