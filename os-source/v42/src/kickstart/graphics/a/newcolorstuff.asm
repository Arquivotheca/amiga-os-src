*******************************************************************************
*
*	$Id: NewColorStuff.asm,v 42.1 93/07/20 13:35:34 chrisg Exp $
*
*******************************************************************************


	SECTION	graphics

	ifd	AA_ONLY
	OPT	P=68020
	endc

	include	'/view.i'
	include	'/gfx.i'

	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'/gfxbase.i'
	include	'/displayinfo.i'
	include	'/vp_internal.i'
	include	'/macros.i'

	include	'hardware/custom.i'

	xref	_LVOAllocMem,_LVOInitSemaphore,_LVOObtainSemaphore,_LVOReleaseSemaphore
	xref	_LVOSetRGB4,_LVOPermit,_LVONextTagItem
	xref	_LVOSetRGB32,_LVOLoadRGB32

	xref	_new_mode,_hedley_load,_update_top_color

	xdef	_AttachPalExtra,_ReleasePen,_SetRGB32,_LoadRGB32
	xdef	_ObtainPen
	
_AttachPalExtra:
******* graphics.library/AttachPalExtra *****************************************
*
*   NAME
*       AttachPalExtra -- Allocate and attach a palette sharing structure to a 
*	                  colormap. (V39)
*
*
*   SYNOPSIS
*       status=AttachPalExtra( cm, vp)
*	                       a0  a1
*
*	LONG AttachPalExtra( Struct ColorMap *, struct ViewPort *);
*
*   FUNCTION
*	Allocates and attaches a PalExtra structure to a ColorMap.
*	This is necessary for color palette sharing to work. The
*	PalExtra structure will be freed by FreeColorMap().
*	The set of available colors will be determined by the mode
*	and depth of the viewport.
*
*   INPUTS
*	cm  =  A pointer to a color map created by GetColorMap().
*
*	vp   = A pointer to the viewport structure associated with
*	       the ColorMap.
*
*   RESULTS
*	status - 0 if sucessful, else an error number. The only currently
*	         defined error number is out of memory (1).
*
*   BUGS
*
*   NOTES
*	This function is for use with custom ViewPorts and custom ColorMaps,
*	as Intuition attaches a PalExtra to all of its Screens.
*	If there is already a PalExtra associated with the ColorMap, then
*	this function will do nothing.
*
*   SEE ALSO
*	GetColorMap() FreeColorMap() ObtainPen() ObtainBestPenA()
*
*********************************************************************************
;
;  The alloc size for a Palette Extra is pe_SIZEOF+2*cm_Count
;
;  For now, the formula for the maximum number of shared colors to make available
;  is 1<<NBITPLANES.
;
;  Needs to make sprite colors unavailable when sprite banking support is complete.
;			(or intuition could do this)
;  For dual playfields, really won't work. there would have to be 2 structures.
;  for 6 bitplane HAM, should make 0..15 available.
;  for 8 bitplane HAM, should make 0,4,8,12... available.
;
;  colors will be allocated from high to low.
;

1$:	movem.l	a0/a2/a3/a4/a5/a6,-(sp)
	move.l	gb_ExecBase(a6),a6		; get execbase

	FORBID							; onl let one process attach it.
	tst.l	cm_PalExtra(a0)
	bne	end_att
	move.l	a1,a5				; save view pointer for later.
	move.l	a0,a2				; save color map pointer in a2 so it doesn't get trashed.
	move	cm_Count(a2),d0
	ext.l	d0
	move.l	d0,d1
	add.l	d0,d0				;*2
	add.l	d1,d0				;*3, 3 because refcnts are words but alloclist is a byte
	add.l	#pe_SIZEOF,d0
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,cm_PalExtra(a2)
	beq.s	end_att				; no memory - don't attach it.
	move.l	d0,a0
	move.l	a0,a3

	ifne	pe_Semaphore
	fail	"change me!!"
	endc

	jsr	_LVOInitSemaphore(a6)		; inits its semaphore
	move.l	a5,pe_ViewPort(a3)
	move	cm_Count(a2),d0
	move.w	#-1,pe_FirstShared(a3)
	lea	pe_SIZEOF(a3),a4
	move.l	a4,pe_RefCnt(a3)
	lea	0(a4,d0.w),a4			; point a4 at alloc list
	add.w	d0,a4				; double it because refcnt is a word
	move.l	a4,pe_AllocList(a3)
; now, we initialize the alloc list to -1,0,1,2,3,4,...
	subq	#1,d0				; adjust for dbra
	moveq	#-1,d1
2$:	move.b	d1,(a4)+
	addq	#1,d1
	dbra	d0,2$
; now, init NFREE to the appropriate number for the viewport
	move.l	vp_RasInfo(a5),d0
	beq.s	end_att
	move.l	d0,a1
	move.l	ri_BitMap(a1),d0
	beq.s	end_att
	move.l	d0,a1
	moveq	#0,d0
	move.b	bm_Depth(a1),d0
	btst	#3,vp_Modes(a5)	; HAM?
	beq.s	no_ham1
	subq	#2,d0
no_ham1:
	tst.b	vp_Modes+1(a5)	; EHB
	bpl.s	no_ehb1
	subq	#1,d0
no_ehb1:
	moveq	#1,d1
	lsl.l	d0,d1
	move.w	d1,pe_NFree(a3)
	subq	#1,d1
	move.w	d1,pe_FirstFree(a3)
	move.w	d1,pe_SharableColors(a3)	
end_att
	jsr	_LVOPermit(a6)
	movem.l	(sp)+,a0/a2/a3/a4/a5/a6
	move.l	cm_PalExtra(a0),d0
	beq.s	retfail
	moveq	#0,d0
	rts
retfail:
	moveq	#1,d0
	rts

none_free:
	moveq	#-1,d0
	rts

	

_ReleasePen:
******* graphics.library/ReleasePen ********************************************
*
*   NAME
*	ReleasePen -- Release an allocated palette entry to the free pool. (V39)
*
*
*   SYNOPSIS
*	ReleasePen( cm, n)
*	            a0 d0
*
*	void ReleasePen( Struct ColorMap *, ULONG);
*
*   FUNCTION
*	Return the palette entry for use by other applications.
*	If the reference count for this palette entry goes to zero,
*	then it may be reset to another RGB value.
*
*   INPUTS
*       cm  =  A pointer to a color map created by GetColorMap().
*
*	n   =  A palette index obtained via any of the palette allocation
*	       functions. Passing a -1 will result in this call doing
*	       nothing.
*
*   BUGS
*
*   NOTES
*	This function works for both shared and exclusive palette entries.
*
*   SEE ALSO
*	GetColorMap() ObtainPen() ObtainBestPenA()
*
*********************************************************************************
	tst.l	d0
	bmi.s	rts_rel_pen
	movem.l	d2/a2/a6,-(sp)
	move.l	cm_PalExtra(a0),a2
	lea	pe_Semaphore(a2),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)		; lock the palette extra info.
	move.l	pe_RefCnt(a2),a0		; get pointer to ref cnt table
	add.w	d0,a0					; double offset for words
	subq.w	#1,0(a0,d0.w)			; ref[i]--
	bne.s	still_some_refs
	addq.w	#1,pe_NFree(a2)			; nfree++
	move.l	pe_AllocList(a2),a0

; now, see if on shared list, and if so, remove it.
	move.w	pe_NShared(a2),d1		; ctr
	beq.s	freed_it			; none shared, so this one can't be.
	move.w	pe_FirstShared(a2),d2		; j=first
	cmp.w	d2,d0				; if (i=first)
	bne.s	list_loop
	move.b	0(a0,d0.l),pe_FirstShared+1(a2)	;  then first=i->next
	bra.s	freed_shared
list_loop:
	subq	#1,d1
	beq.s	freed_it			; not on shared list.
	cmp.b	0(a0,d2.w),d0			; if (j->next=i)
	beq.s	found_shared
	move.b	0(a0,d2.w),d2
	bra.s	list_loop
found_shared:
	move.b	0(a0,d0.w),0(a0,d2.w)		;   then j->next=i->next
freed_shared:
	subq.w	#1,pe_NShared(a2)
freed_it:
	move.b	pe_FirstFree+1(a2),0(a0,d0.w)	; n->next=first
	move.w	d0,pe_FirstFree(a2)		; first=n

still_some_refs:
	lea	pe_Semaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)
	movem.l	(sp)+,d2/a2/a6
rts_rel_pen:
	rts

_ObtainPen:
******* graphics.library/ObtainPen **********************************************
*
*   NAME
*	ObtainPen -- Obtain a free palette entry for use by your program. (V39)
*
*
*   SYNOPSIS
*	n = ObtainPen( cm, n, r, g, b, flags)
*	d0	       a0 d0  d1 d2 d3  d4
*
*	LONG ObtainPen(struct ColorMap *,ULONG,ULONG,ULONG,ULONG,ULONG);
*
*   FUNCTION
*	Attempt to allocate an entry in the colormap for use by the application.
*	If successful, you should ReleasePen() this entry after you have finished
*	with it.
*	
*	Applications needing exclusive use of a color register (say for color
*	cycling) will typically call this function with n=-1. Applications needing
*	only the shared use of a color will typically use ObtainBestPenA() instead.
*	Other uses of this function are rare.
*
*   INPUTS
*	cm  =  A pointer to a color map created by GetColorMap().
*	n   =  The index of the desired entry, or -1 if any one is acceptable
*	rgb =  The RGB values (32 bit left justified fractions) to set the new
*	       palette entry to.
*	flags= PEN_EXCLUSIVE - tells the system that you want exclusive
*	       (non-shared) use of this pen value. Default is shared access.
*
*	       PEN_NO_SETCOLOR - tells the system to not change the rgb values
*	       for the selected pen. Really only makes sense for exclusive pens.
*
*
*   RESULTS
*
*	n   =  The allocated pen. -1 will be returned if there is no pen available
*	       for you.
*
*   BUGS
*
*   NOTES
*	When you allocate a palette entry in non-exclusive mode, you
*	should not change it (via SetRGB32), because other programs on the
*	same screen may be using it. With PEN_EXCLUSIVE mode, you can
*	change the returned entry at will.
*
*	To avoid visual artifacts, you should not free up a palette
*	entry until you are sure that your application is not displaying
*	any pixels in that color at the time you free it. Otherwise, another
*	task could allocate and set that color index, thus changing the colors
*	of your pixels.
*
*	Generally, for shared access, you should use ObtainBestPenA()
*	instead, since it will not allocate a new color if there is one
*	"close enough" to the one you want already.
*	If there is no Palextra attached to the colormap, then this
*	routine will always fail. 
*
*   SEE ALSO
*	GetColorMap() ReleasePen() AttachPalExtra() ObtainBestPenA()
*
*********************************************************************************


	movem.l	a2/d6/d7,-(a7)
	movem.l	a6/d1/d2/d3,-(a7)
	moveq	#-1,d7				; return value
	cmp.w	cm_Count(a0),d0
	bgt	return2				; handles negative or > count.
	tst.l	cm_PalExtra(a0)			; colorinfo fattened?
	beq	return2
	move.l	cm_PalExtra(a0),a2		; a2=pal extra

	lea	pe_Semaphore(a2),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)

	tst.w	pe_NFree(a2)			; any available?
	beq	no_set_rgb32
	
	move.l	pe_RefCnt(a2),a0
	tst.l	d0				; a specific one, or the next one?
	bmi.s	want_any_one			; any ol' one will do

	cmp.w	pe_SharableColors(a2),d0	; if not allocable, fail
	bhi.s	return1

	IFND	AA_ONLY
	add.l	d0,d0
	tst.w	0(a0,d0.l)			; ref cnt==0?
	bne.s	return1
	addq.w	#1,0(a0,d0.l)			; bump ref cnt
	lsr.l	#1,d0
	ELSE
	tst.w	(a0,d0.l*2)
	bne.s	return1
	addq.w	#1,(a0,d0.l*2)
	endc


	move.l	d0,d7				; return success
	move.l	pe_AllocList(a2),a0
	subq.w	#1,pe_NFree(a2)			; nfree--
	beq.s	check_exclusive
	move.w	pe_FirstFree(a2),d6
	cmp.w	d6,d0				; first=got?
	bne.s	must_traverse_list
	move.b	0(a0,d0.l),d6
	move.w	d6,pe_FirstFree(a2)		; first=first->next
	bra.s	check_exclusive
must_traverse_list:
	cmp.b	0(a0,d6.w),d0			; i->next=n?
	bne.s	iter_again
	move.b	0(a0,d0.w),0(a0,d6.w)		; i->next=n->next
	bra.s	check_exclusive
iter_again:
	move.b	0(a0,d6.w),d6			; i = i->next
	bra.s	must_traverse_list


want_any_one:
	moveq	#0,d7				; zero upper bits
	subq.w	#1,pe_NFree(a2)			; dec avail count
	move.w	pe_FirstFree(a2),d7		; get next avail
	IFND	AA_ONLY
	add.w	d7,d7
	addq.w	#1,0(a0,d7.w)			; refcnt[alloc]++
	lsr.w	#1,d7
	ELSE
	addq.w	#1,(a0,d7.w*2)
	endc

	move.l	pe_AllocList(a2),a0
	move.b	0(a0,d7.w),pe_FirstFree+1(a2)	; firstfree=alloc->next

check_exclusive:
; now, a0=alloc list a2=palextra, d7=color #
	btst	#0,d4				; PALETTE_EXCLUSIVE?
	bne.s	return1
; not exclusive, so add it to shared list.
	addq.w	#1,pe_NShared(a2)
	move.b	pe_FirstShared+1(a2),0(a0,d7.w)
	move.w	d7,pe_FirstShared(a2)
	
return1:
; now, set the color
	move.l	d7,d0
	bmi.s	no_set_rgb32
	btst	#1,d4
	bne.s	no_set_rgb32
	movem.l	(sp),a6/d1/d2/d3
	move.l	pe_ViewPort(a2),a0
	jsr	_LVOSetRGB32(a6)
	move.l	gb_ExecBase(a6),a6
no_set_rgb32:
	lea	pe_Semaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)
return2:
	move.l	d7,d0
	movem.l	(sp)+,a6/d1/d2/d3
	movem.l	(a7)+,a2/d6/d7
	rts

repl	macro	reg
; replicate 8 bits to all 32
; ab000000 -> abababab
	move.l	\1,d0	; d0=ab000000
	lsr.l	#8,d0	; d0=00ab0000
	or.l	\1,d0	; d0=abab0000
	move.l	d0,\1	; \1=abab0000
	swap	d0	; d0=0000abab
	or.l	d0,\1	; \1=abababab
	endm

******* graphics.library/GetRGB32 ********************************************
*
*   NAME
*	GetRGB32 -- Set a series of color registers for this Viewport. (V39)
*
*   SYNOPSIS
*	GetRGB32(  cm,  firstcolor, ncolors, table )
*	           a0   d0   		d1    a1
*
*       void GetRGB32( struct ColorMap *, ULONG, ULONG, ULONG *);
*
*   INPUTS
*	cm = colormap
*	firstcolor = the first color register to get
*	ncolors = the number of color registers to set.
*	table=a pointer to a series of 32-bit RGB triplets.
*
*   RESULT
*	The ULONG data pointed to by 'table' will be filled with the 32 bit
*	fractional RGB values from the colormap.
*   BUGS
*
*   NOTES
*	'Table' should point to at least ncolors*3 longwords.
*
*   SEE ALSO
*	LoadRGB4() GetColorMap() LoadRGB32() SetRGB32CM() graphics/view.h
******************************************************************************
	xdef	_GetRGB32
_GetRGB32:
	movem.l	a2/a3/d2-d7,-(a7)
	move.l	cm_ColorTable(a0),a2
	move.l	a2,a3
	btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	beq.s	no_lowbits1
	tst.b	cm_Type(a0)
	beq.s	no_lowbits1
	tst.l	cm_LowColorBits(a0)
	beq.s	no_lowbits1
	move.l	cm_LowColorBits(a0),a3
no_lowbits1:
	add.l	d0,d0
	add.l	d0,a2
	add.l	d0,a3
	subq.l	#1,d1			; adjust for DBRA
	move.l	#$f0000000,d5
	move.l	#$0f000000,d6
getrgb32_lp:
	move.w	(a2)+,d0		; 0RGB
	lsl.w	#4,d0			; RGB0
	swap	d0
	move.l	d0,d2
	and.l	d5,d2			; R000000
	lsl.l	#4,d0
	move.l	d0,d3
	and.l	d5,d3			; G000000
	lsl.l	#4,d0
	move.l	d0,d4
	and.l	d5,d4
	move.w	(a3)+,d0		; 0RGB
	swap	d0
	clr.w	d0
	move.l	d0,d7
	and.l	d6,d7			; 0R000000
	or.l	d7,d2
	lsl.l	#4,d0
	move.l	d0,d7
	and.l	d6,d7
	or.l	d7,d3
	lsl.l	#4,d0
	and.l	d6,d0
	or.l	d0,d4
	repl	d2
	repl	d3
	repl	d4
	movem.l	d2/d3/d4,(a1)
	lea	4*3(a1),a1	
	dbra	d1,getrgb32_lp
	movem.l	(a7)+,d2-d7/a2-a3
	rts


******* graphics.library/LoadRGB32 ********************************************
*
*   NAME
*	LoadRGB32 -- Set a series of color registers for this Viewport. (V39)
*
*   SYNOPSIS
*	LoadRGB32(  vp,  table )
*	            a0   a1
*
*	void LoadRGB32( struct ViewPort *, ULONG *);
*
*   INPUTS
*	vp = viewport
*	table = a pointer to a series of records which describe which colors to
*	        modify.
*   RESULT
*	The selected color registers are changed to match your specs.
*   BUGS
*
*   NOTES
*
*	Passing a NULL "table" is ignored.
*	The format of the table passed to this function is a series of records,
*	each with the following format:
*
*	        1 Word with the number of colors to load
*	        1 Word with the first color to be loaded.
*	        3 longwords representing a left justified 32 bit rgb triplet.
*	        The list is terminated by a count value of 0.
*
*	   examples:
*	        ULONG table[]={1l<<16+0,0xffffffff,0,0,0} loads color register
*	                0 with 100% red.
*	        ULONG table[]={256l<<16+0,r1,g1,b1,r2,g2,b2,.....0} can be used
*	                to load an entire 256 color palette.
*
*	Lower order bits of the palette specification will be discarded,
*	depending on the color palette resolution of the target graphics
*	device. Use 0xffffffff for the full value, 0x7fffffff for 50%,
*	etc. You can find out the palette range for your screen by
*	querying the graphics data base.
*
*	LoadRGB32 is faster than SetRGB32, even for one color.
*
*   SEE ALSO
*	LoadRGB4() GetColorMap() GetRGB32() SetRGB32CM() graphics/view.h
******************************************************************************
_LoadRGB32:
	move.l	a1,d0
	beq	loadrgb_exit
	movem.l	d2-d7/a2-a5,-(a7)
	move.l	a6,-(a7)
	move.l	a0,a2
	move.l	vp_ColorMap(a2),a0
	move.l	cm_ColorTable(a0),a3
	move.l	a3,a4
	tst.b	cm_Type(a0)
	beq.s	old_cmap
	move.l	cm_LowColorBits(a0),a4
old_cmap:
	move.w	#$0f00,d5
	move.w	#$f000,d6
loadrgb_lp:
	move.w	(a1)+,d1	; count
	beq.s	done_loadlp
	move.w	(a1)+,d0	; start color
; a=firstc+count-cm_ncolors
; if a>0, { count -=a; modulo=12*a }
; else modulo=0;
	move	d1,d7
	add.w	d0,d7
	sub.w	cm_Count(a0),d7
	ext.l	d7
	bgt.s	non_zero_modulo
	moveq	#0,d7
non_zero_modulo:
	sub	d7,d1
	add.l	d7,d7		;*2
	add.l	d7,d7		;*4
	move.l	d7,a5
	add.l	d7,d7		;*8
	add.l	a5,d7		;*8+*4=*12

	IFND	AA_ONLY
	add.w	d0,d0
	lea	0(a3,d0.w),a5
	lea	0(a4,d0.w),a6
	ELSE
	lea	(a3,d0.w*2),a5
	lea	(a4,d0.w*2),a6
	endc
	bra.s	lrgb_end_lp1		; d1 could be zero
lrgbinner:
	movem.l	(a1),d2/d3/d4			; rgb
	swap	d2
	and.w	d5,d2
	swap	d3
	and.w	d5,d3
	lsr.w	#4,d3
	or.w	d3,d2
	swap	d4
	and.w	d5,d4
	lsr.w	#8,d4
	or.w	d4,d2
	move.w	d2,(a6)+
	movem.l	(a1)+,d2/d3/d4
	swap	d2
	and.w	d6,d2
	swap	d3
	and.w	d6,d3
	lsr.w	#4,d3
	or.w	d3,d2
	swap	d4
	and.w	d6,d4
	lsr.w	#8,d4
	or.w	d4,d2
	lsr.w	#4,d2
	move.w	(a5),d3
	and.w	#$8000,d3
	or.w	d3,d2
	move.w	d2,(a5)+
lrgb_end_lp1:
	dbra	d1,lrgbinner
	add.l	d7,a1
	bra	loadrgb_lp
done_loadlp:
	move.l	(a7)+,a6
	bsr.s	_pokecolors			; vp in a2
	movem.l	(a7)+,d2-d7/a2-a5
loadrgb_exit:
	rts


******* graphics.library/SetRGB32 ********************************************
*
*   NAME
*	SetRGB32 -- Set one color register for this Viewport. (V39)
*
*   SYNOPSIS
*	SetRGB32(  vp,  n,   r,    g,    b)
*	           a0  d0   d1    d2    d3
*
*	void SetRGB32( struct ViewPort *, ULONG, ULONG, ULONG, ULONG );
*
*   INPUTS
*	vp = viewport
*	n = the number of the color register to set.
*	r = red level   (32 bit left justified fraction)
*	g = green level (32 bit left justified fraction)
*	b = blue level  (32 bit left justified fraction)
*
*   RESULT
*	If there is a ColorMap for this viewport, then the value will
*	be stored in the ColorMap.
*	The selected color register is changed to match your specs.
*	If the color value is unused then nothing will happen.
*
*   BUGS
*
*   NOTES
*	Lower order bits of the palette specification will be discarded,
*	depending on the color palette resolution of the target graphics
*	device. Use 0xffffffff for the full value, 0x7fffffff for 50%,
*	etc. You can find out the palette range for your screen by
*	querying the graphics data base.
*
*   SEE ALSO
*	GetColorMap() GetRGB32() SetRGB32CM() LoadRGB32() graphics/view.h
******************************************************************************

_SetRGB32:
	clr.l	-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	or.l	#$10000,d0
	move.l	d0,-(a7)
	move.l	a7,a1
	jsr	_LVOLoadRGB32(a6)
	lea	20(a7),a7
an_rts:
	rts

	xdef	_pokecolors
_pokecolors:
; pokecolors - repoke the hardware copper lists and the CopIns for the colors of
; a viewport.
;
;	void _pokecolors(register __a2 struct ViewPort *vp)
;
;   algorithm:
;
;	obtain cpr list semaphore
;	for each long word in mask
;		set ptr=the next write to bplcon3.
;		if the longword is not zero
;			skip bplcon3
;			for i=0 to 32
;				if (this color used)
;					if (*ptr) != 180+i	abort
;					poke high word
;				else skip
;			skip bplcon3
;			for i=0 to 32
;				if (this color used)
;					if (*ptr) != 180+i	abort
;					poke low word
;				else skip


_pokecolors:
	tst.l	vp_ColorMap(a2)
	beq.s	an_rts
	move.l	a2,a0
	vp_to_vector	a0,vt_PokeColors,_DefaultPokeColors(pc)
	jmp	(a0)

	xdef	_DefaultPokeColors

_DefaultPokeColors:
	movem.l	d2-d7/a0-a6,-(a7)
	move.l	a2,-(a7)
	jsr	_new_mode
	addq.l	#4,a7
	move.l	d0,d5					; d5=modes(vp)
	lea	-64(a7),a7				; 256 bits+32 words (a7)=bits 32(a7)=color scramble data

	move.w	#$7fff,d6					; d6=color AND word
	move.l	vp_ColorMap(a2),a1
	btst	#0,cm_Flags(a1)			; colormap_transparency?
	beq.s	no_chroma_pen
	moveq	#-1,d6
no_chroma_pen:
; now for the fun part!
	move.l	a6,-(a7)
	move.b	gb_ChipRevBits0(a6),d4
	moveq	#0,d7
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	cmp.l	SysStkUpper(a6),a7			; might we be in an interrupt?
	bhi.s	no_interrupt1
	cmp.l	SysStkLower(a6),a7
	blo.s	no_interrupt1
	cmp.w	#-1,SS_QUEUECOUNT(a0)
	bne.s	yes_interrupt1
no_interrupt1:
	move.l	a0,d7
	jsr	_LVOObtainSemaphore(a6)
yes_interrupt1:
	move.l	vp_DspIns(a2),d0
	beq	no_dspins
	move.l	d0,a4					; a4=DspIns=CopList ptr
	move.w	vp_Modes(a2),d0
	and.w	#V_VP_HIDE,d0
	bne	no_dspins
	move.l	cl_CopLStart(a4),a5			; a5=long frame ptr
	move.l	cl_CopSStart(a4),a4			; a4=short frame pointer
	move.l	cm_ColorTable(a1),a6
	move.l	cm_LowColorBits(a1),a0
	tst.b	cm_Type(a1)
	bne.s	new_colormap1
	move.l	a6,a0					; set low words=high words
new_colormap1:
; now:
; a0=low color words
; a1=color map
; a2=vp
; a3=
; a4=short frame hw copper list
; a5=long frame hw copper list
; a6=colortable
; d4=chip rev bits
; d5=viewport modes
; d6=AND value for color words.
	btst	#5,d5					; shres?
	beq.s	no_scramble1
	btst.b	#GFXB_AA_MLISA,d4
	bne.s	no_scramble1
;  now, repoint a0 and a6 at the scrambled workspace
	move.l	a1,-(a7)
	lea	8(a7),a1				; a6 and a1 are on stack right now
	move.l	a6,a0
	bsr	_scramble_colors			; *(a1++)=scrambled data
	lea	32(a6),a0				; scramble colors 16-19
	bsr	_scramble_colors
	move.l	(a7)+,a1
	lea	4(a7),a6				; a6 still on stack
	move.l	a6,a0
no_scramble1:
	move.w	#color,d0
	move.w	#bplcon3-color,d2
	move.w	#32*2,d3
	move.w	#%1110000000000000,d4
; now:
; a0=low color words
; a1=color map
; a2=vp
; a3=
; a4=short frame hw copper list
; a5=long frame hw copper list
; a6=colortable
; d0=#color
; d1=scratch
; d2=#bplcon3-color0
; d3=64 (color limit)
; d4=%1110000000000000 (bplcon3 bank mask)
; d5=scratch
; d6=AND value for color words.
; d7=interrupt flag

	move.l	a5,d5
	move.l	a4,d1
	beq.s	skip_shf
	lea	skip_shf(pc),a3
do_hw_clist:
; poke a hardware copper list for colors
; entr a4=&cl 
; a0=low colors a6=high colors
; usage:
; a5=cur list + offset
; a3=where to return to
	move.l	a6,a5
1$:	cmp.w	(a4),d0
	lea	4(a4),a4
	bne.s	1$
	move.w	(a5),d1
	and.w	d6,d1
	move.w	d1,-2(a4)
; now, we have handled color 0. We must now enter the following loop:
; switch (*c++)
;  case COLORnn
;   *c++=*(a5+nn) & d6
;  case BPLCON3:
;   if (*c & 512) a5=a0 else a5=a6
;   a5 += (*c & %1110000000000000) >> 8
;  else
;    done
next_hwins
	move.w	(a4)+,d1
	sub.w	d0,d1
	blo.s	not_color_load
	cmp.w	d3,d1
	bhs.s	done_hwlist
	move.w	0(a5,d1.w),d1
	and.w	d6,d1
	move.w	d1,(a4)+
	move.w	(a4)+,d1
	sub.w	d0,d1
	blo.s	not_color_load
	cmp.w	d3,d1
	bhs.s	done_hwlist
	move.w	0(a5,d1.w),d1
	and.w	d6,d1
	move.w	d1,(a4)+
	bra.s	next_hwins
not_color_load:
	cmp.w	d2,d1
	bne.s	done_hwlist
; it's bplcon3!
	move.w	(a4)+,d1
	move.l	a0,a5
	btst	#9,d1
	bne.s	1$
	move.l	a6,a5
1$:	and.w	d4,d1
	lsr.w	#7,d1
	add.w	d1,a5
	bra.s	next_hwins
done_hwlist:
	jmp	(a3)
skip_shf:
	tst.l	d5
	beq.s	skip_lof
	lea	skip_lof(pc),a3
	move.l	d5,a4
	bra.s	do_hw_clist

skip_lof:
	btst	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a1)
	bne.s	dont_do_intermed
	move.l	a6,a5
	move.l	vp_DspIns(a2),a2
	move.l	cl_CopIns(a2),a2
	cmp.w	#0,a2
	bne.s	find_color0_noinc
dont_do_intermed:
no_dspins:
	move.l	(a7)+,a6					; get back gfxbase
	move.l	gb_current_monitor(a6),-(a7)
	bsr	_update_top_color
	addq.l	#4,a7
	tst.l	d7
	beq.s	i_was_in_an_interrupt
	move.l	d7,a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
i_was_in_an_interrupt:
	lea	64(a7),a7
	movem.l	(a7)+,d2-d7/a0-a6
	rts

find_color0_intermed:
	lea	ci_SIZEOF(a2),a2
find_color0_noinc:
	move.w	ci_OpCode(a2),d1
	and.w	#3,d1
	bne.s	find_color0_intermed
	move.w	ci_DestAddr(a2),d1
	cmp.w	d0,d1					; color?
	bne.s	find_color0_intermed
; now, we are pointing at color0!
	move.w	(a5),d1
	and.w	d6,d1
	move.w	d1,ci_DestData(a2)
	lea	ci_SIZEOF(a2),a2
; now, we must execute the same basic algorithm as in do_hwclist, except that it is more
; annoying.
next_intermed_opcode:
	move.w	(a2)+,d1	; opcode
	and.w	#3,d1
	bne.s	dont_do_intermed
	move.w	(a2)+,d1
	sub.w	d0,d1
	blo.s	not_color_load_i
	cmp.w	d3,d1
	bhs.s	dont_do_intermed
	move.w	0(a5,d1.w),d1
	and.w	d6,d1
	move.w	d1,(a2)+
	move.w	(a2)+,d1	; opcode
	and.w	#3,d1
	bne.s	dont_do_intermed
	move.w	(a2)+,d1
	sub.w	d0,d1
	blo.s	not_color_load_i
	cmp.w	d3,d1
	bhs.s	dont_do_intermed
	move.w	0(a5,d1.w),d1
	and.w	d6,d1
	move.w	d1,(a2)+
	bra.s	next_intermed_opcode

not_color_load_i:
	cmp.w	d2,d1
	bne.s	dont_do_intermed
	move.w	(a2)+,d1
	move.l	a0,a5
	btst	#9,d1
	bne.s	1$
	move.l	a6,a5
1$:	and.w	d4,d1
	lsr.w	#7,d1
	add.w	d1,a5
	bra.s	next_intermed_opcode




get_32bit_color:
; a3=cm
; d0=index
; exit d5/d6/d7=rgbcolors
; trashes d2,d0,a0
	add.w	d0,d0
	move.l	cm_ColorTable(a3),a0
	moveq	#0,d5
	move.w	0(a0,d0.w),d5
	move.l	d5,d6
	move.l	d5,d7
	and.w	#$0f00,d5
	and.w	#$00f0,d6
	and.w	#$000f,d7
	lsl.l	#4,d5
	swap	d5				; d5=red
	lsl.l	#8,d6
	swap	d6				; d6=green
	ror.l	#4,d7				; d7=red
	move.l	cm_LowColorBits(a3),a0
	move.w	0(a0,d0.w),d2
	and.l	#$0f00,d2
	swap	d2
	or.l	d2,d5
	move.w	0(a0,d0.w),d2
	and.l	#$00f0,d2
	lsl.l	#4,d2
	swap	d2
	or.l	d2,d6
	move.w	0(a0,d0.w),d2
	and.l	#$000f,d2
	lsl.l	#8,d2
	swap	d2
	or.l	d2,d7				; yuck! now, we finally have the RGB
						; values from the colormap in fractional
						; format in d5/d6/d7

	rts

_SearchColorMap:
*       Search for a close color match, using a user-supplied procedure. (V39)
*
*   SYNOPSIS
*       color | -1 =SearchColorMap(  cm,  proc, R, G, B, MaxError, MaxColor, RealMax)
*                                    a0     a1 d1  d2 d3     d4	   d5		d6
*
*       ULONG SearchColorMap( struct ColorMap *, (*int)(), ULONG, ULONG, ULONG,
*				ULONG, ULONG );
*
*   INPUTS
*	cm = colormap
*       proc = the procedure to call for each color candidate.
*       R = red level   (32 bit left justified fraction)
*       G = green level (32 bit left justified fraction)
*       B = blue level  (32 bit left justified fraction)
*       MaxError = the maximum permissible error value for a match.
*	MaxColor = the maximum color index that you want. -1 for any valid one.
*   RESULT
*	Your procedure will be called for each shared entry in the colormap.
*	Your procedure will be passed the RGB values supplied to the call
*	in d1/d2/d3, and the RGB values from the color map in d5/d6/d7.
*	Your function should return the error value in d0. Your function will
*	be called until it returns a zero, or until there are no more colors
*	to examine.
*	If no match was found with a return of less then the value passed in
*	the MaxError parameter, then a new one will be allocated if available,
*	and its RGB values set to the ones passed. This color will be returned.
*	If no palette entry was found within the error tolerance, and there are none
*	available for allocation, then the closest one found will be returned.
*	If one was found within the tolerance passed by the application, then
*	it will be returned, and its reference count will be bumped.
*	If there are no sharable palette entries available, then -1 will be returned.
*
*   BUGS
*
*   NOTES
*	If this call succceeds, then you must call ReleasePen() when you are done
*	with the color.
*	Lower order bits of the palette specification will be discarded,
*	depending on the color palette resolution of the target graphics
*	device. Use 0xffffffff for the full value, 0x7fffffff for 50%,
*	etc. You can find out the palette range for your screen by
*	querying the graphics data base.
*	For most applications, you should call ObtainBestPenA(), and not this
*	function. This is provided for an application which might want to
*	optimize its palette error metrics differently. ObatinBestPen() also
*	works better in shared screens, because it changes its tolerances based
*	on the amount of space free in the palette.
*
*   SEE ALSO
*	GetColorMap() ObtainBestPenA() ReleasePen() ObtainPen()
******************************************************************************
ofs_closest_error	equ	0
ofs_closest_color	equ	ofs_closest_error+4
ofs_match_rgb		equ	ofs_closest_color+4
ofs_maxerr		equ	ofs_match_rgb+12
ofs_jmpvector		equ	ofs_maxerr+4
ofs_counter		equ	ofs_jmpvector+4
ofs_gbase		equ	ofs_counter+4
ofs_flags		equ	ofs_gbase+4
ofs_colormask	equ	ofs_flags+4
ofs_realmax	equ	ofs_colormask+4
ofs_stacksize		equ	ofs_realmax+4


	move.l	cm_PalExtra(a0),d0
	movem.l	d2-d7/a2-a6,-(a7)
	sub.l	#ofs_stacksize,a7
	move.l	d6,ofs_realmax(a7)
	move.l	d5,ofs_flags(a7)
	move.l	a6,ofs_gbase(a7)
	move.l	#-1,ofs_closest_error(a7)
	move.l	#-1,ofs_closest_color(a7)
	move.l	gb_ColorMask(a6),d7
	move.l	d7,ofs_colormask(a7)
	and.l	d7,d1
	and.l	d7,d2
	and.l	d7,d3
	movem.l	d1/d2/d3,ofs_match_rgb(a7)
	move.l	d4,ofs_maxerr(a7)
	move.l	a1,ofs_jmpvector(a7)

	move.l	d0,a2				; a2=palextra
	move.l	a0,a3				; a3=color map
	move.l	gb_ExecBase(a6),a6		; a6=execbase
	lea	pe_Semaphore(a2),a0		
	jsr	_LVOObtainSemaphore(a6)
	move.l	pe_AllocList(a2),a4		; a4=alloc list

	moveq	#0,d0
	move.w	pe_NShared(a2),d0		; load and sign extend
	beq.s	no_match_try_new_one
	move.l	d0,ofs_counter(a7)		; number of shared entries to examine
	move.w	pe_FirstShared(a2),a5		; a5=color index being checked
check_next_color:
; now, a5=color index to try. Unpack it to d5/d6/d7
	move	a5,d0
	bsr	get_32bit_color

	move.l	ofs_colormask(a7),d1
	and.l	d1,d5
	and.l	d1,d6
	and.l	d1,d7
	movem.l	ofs_match_rgb(a7),d1/d2/d3
	move.l	ofs_jmpvector(a7),a1
	jsr	(a1)
	tst.l	d0				; exact match?
	beq.s	exact_match
	cmp.l	ofs_closest_error(a7),d0
	bhs.s	no_closer
	move.l	d0,ofs_closest_error(a7)
	move.l	a5,ofs_closest_color(a7)
no_closer:
	subq.l	#1,ofs_counter(a7)
	beq.s	got_last
	moveq	#0,d0
	move.b	0(a4,a5),d0			; i=i->next
	move.l	d0,a5
	bra	check_next_color
exact_match:
	move.l	a5,ofs_closest_color(a7)
	bra.s	can_use_old
got_last:
	move.l	ofs_maxerr(a7),d1
	cmp.l	ofs_closest_error(a7),d1
	blo.s	no_match_try_new_one
	move.l	ofs_realmax(a7),d1
	cmp.l	ofs_closest_error(a7),d1
	bhs.s	can_use_old
	bra.s	cant_get
no_match_try_new_one:
	tst.w	pe_NFree(a2)			; any free?
	bne.s	can_do_new
	tst.l	ofs_flags(a7)
	beq.s	can_use_old			; none free, so use whatever was closest
cant_get:
	move.l	#-1,ofs_closest_color(a7)
	bra.s	can_use_old
can_do_new:
; must allocate a new one.
	subq.w	#1,pe_NFree(a2)
	addq.w	#1,pe_NShared(a2)
	moveq	#0,d0
	move.w	pe_FirstFree(a2),d0
	move.l	d0,ofs_closest_color(a7)
	moveq	#0,d1
	move.b	0(a4,d0.w),d1
	move.w	d1,pe_FirstFree(a2)
	move.w	pe_FirstShared(a2),d1
	move.b	d1,0(a4,d0.w)
	move.w	d0,pe_FirstShared(a2)
	move.l	ofs_gbase(a7),a6
	move.l	pe_ViewPort(a2),a0
	move.l	ofs_closest_color(a7),d0
	movem.l	ofs_match_rgb(a7),d1/d2/d3
	jsr	_LVOSetRGB32(a6)			; should call thru vector!!!
	move.l	gb_ExecBase(a6),a6

can_use_old:
	move.l	ofs_closest_color(a7),d0	; return value
	bmi.s	dont_bump_refcnt		; valid color or failure?
	move.l	pe_RefCnt(a2),a0
	IFND	AA_ONLY
	add.l	d0,d0
	addq.w	#1,0(a0,d0.l)
	lsr.l	#1,d0
	ELSE
	addq.w	#1,(a0,d0.l*2)
	endc
dont_bump_refcnt:
	lea	pe_Semaphore(a2),a0
	jsr	_LVOReleaseSemaphore(a6)
	move.l	ofs_closest_color(a7),d0
	add.l	#ofs_stacksize,a7
	movem.l	(a7)+,d2-d7/a2-a6
	rts

				
default_color_matcher:
; passed rgb in d1/d2/d3 and d5/d6/d7
	swap	d1
	swap	d2
	swap	d3
	swap	d5
	swap	d6
	swap	d7
	lsr.w	#4,d1
	lsr.w	#4,d2
	lsr.w	#4,d3
	lsr.w	#4,d5
	lsr.w	#4,d6
	lsr.w	#4,d7

	sub.w	d1,d5
	bpl.s	1$
	neg.w	d5
1$:	sub.w	d2,d6
	bpl.s	2$
	neg.w	d6
2$:	sub.w	d3,d7
	bpl.s	3$
	neg.w	d7
3$:
	cmp.w	d6,d5
	bgt.s	not_d6
	exg		d5,d6
not_d6:
	cmp.w	d7,d5
	bgt.s	not_d7
	exg		d7,d5
not_d7:
	cmp.w	d7,d6
	bgt.s	no_d6_lowest
	exg		d7,d6
no_d6_lowest:
; now, d5-d6-d7 are ordered
	lsr.w	#1,d6
	lsr.w	#2,d7
	moveq	#0,d0
	move.w	d5,d0
	add.w	d6,d0
	add.w	d7,d0
	rts


******* graphics.library/FindColor *****************************************
*
*   NAME
*	FindColor -- Find the closest matching color in a colormap. (V39)
*
*   SYNOPSIS
*	color = FindColor(  cm,  R,   G,    B , maxpen)
*	                   a3   d1   d2    d3   d4
*
*	ULONG FindColor( struct ColorMap *, ULONG, ULONG, ULONG,LONG);
*
*   INPUTS
*	cm = colormap
*	R = red level   (32 bit left justified fraction)
*	G = green level (32 bit left justified fraction)
*	B = blue level  (32 bit left justified fraction)
*	MaxPen = the maximum entry in the color table to search. A value of
*	        -1 will limt the search to only those pens which could be
*	        rendered in (for instance, it will not examine the sprite
*	        colors on a 4 color screen).
*
*
*   RESULT
*	The system will attempt to find the color in the passed colormap
*	which most closely matches the RGB values passed. No new pens will
*	be allocated, and you should not ReleasePen() the returned pen.
*	This function is not sensitive to palette sharing issues. Its
*	intended use is for:
*
*	        (a) programs which pop up on public screens when those
*	            screens are not using palette sharing. You might
*	            use this function as a fallback when ObtainBestPenA()
*	            says that there are no sharable pens.
*
*	        (b) Internal color matching by an application which is
*	            either running on a non-public screen, or which
*	            wants to match colors to an internal color table
*	            which may not be associated with any displayed screen.
*
*   BUGS
*
*   NOTES
*	In order to use the MaxPen=-1 feature, you must have initialized
*	palette sharing via AttachPalExtra() (all intuition screens do this).
*	Otherwise, MaxPen=-1 will search all colors in the colormap.
*
*
*   SEE ALSO
*	ObtainBestPenA() GetColorMap() ObtainPen() ReleasePen()
******************************************************************************
_FindColor::
	movem.l	d2-d7/a4,-(a7)
	tst.l	d4
	bpl.s	no_def_maxpen
	moveq	#0,d4
	move.w	cm_Count(a3),d4
	move.l	cm_PalExtra(a3),d0
	beq.s	no_def_maxpen
	move.l	d0,a0
	move.w	pe_SharableColors(a0),d4
no_def_maxpen:
	moveq	#-1,d0
	move.l	d0,a1			; a1-retval
	move.l	d0,a4			; a4=best match so far
	movem.l	d1/d2/d3,-(a7)
search_lp:
	move.l	d4,d0
	bsr	get_32bit_color		; d5/d6/d7=cm[d0]
	movem.l	(a7),d1/d2/d3		; get back user's colors
	bsr	default_color_matcher	; d0=err value
	cmpa.l	d0,a4
	bls.s	end_lp1
	move.l	d0,a4
	move.l	d4,a1
end_lp1:
	dbra	d4,search_lp
	lea	3*4(a7),a7
	move.l	a1,d0
	movem.l	(a7)+,d2-d7/a4
	rts

_ObtainBestPenA::
******* graphics.library/ObtainBestPenA ****************************************
*
*   NAME
*	ObtainBestPenA --- Search for the closest color match, or allocate a
*	                   new one. (V39)
*	ObtainBestPen  --- varargs stub for ObtainBestPenA
*
*   SYNOPSIS
*	color | -1 =ObtainBestPenA(  cm,  R,   G,    B,    taglist)
*	                             a0   d1   d2    d3       a1
*
*	LONG ObtainBestPenA( struct ColorMap *, ULONG, ULONG, 
*			ULONG, struct TagItem *);
*
*	color = ObtainBestPen(cm,r,g,b,tags....);
*
*   INPUTS
*	cm = colormap
*	R = red level   (32 bit left justified fraction)
*	G = green level (32 bit left justified fraction)
*	B = blue level  (32 bit left justified fraction)
*	taglist = a pointer to a standard tag list specifying the color
*	          matching settings desired:
*
*	        OBP_Precision - specifies the desired precision for the
*	                match. Should be PRECISION_GUI, PRECISION_ICON, or
*	                PRECISION_IMAGE or PRECISION_EXACT.
*	                Defaults to PRECISION_IMAGE.
*
*	        OBP_FailIfBad - specifies that you want ObtainBestPen to return
*	                a failure value if there is not a color within the
*	                given tolerance, instead of returning the closest color.
*	                With OBP_FailIfBad==FALSE, ObtainBestPen will only fail
*	                if the ViewPort contains no sharable colors.
*	                Defaults to FALSE.
*
*
*   FUNCTION
*	This function can be used by applications to figure out
*	what pen to use to represent a given color.
*
*	The system will attempt to find the color in your viewport closest
*	to the specified color. If there is no color within your tolerance,
*	then a new one will be allocated, if available. If none is available,
*	then the closest one found will be returned. 
*	
*   RESULT
*	The correct pen value, or -1 if no sharable palette entries are available.
*
*
*   BUGS
*
*   NOTES
*	If this call succceeds, then you must call ReleasePen() when you are
*	done with the color.
*
*	The error metric used for ObtainBestPen() is based on the magnitude
*	squared between the two RGB values, scaled by the percentage of free
*	entries.
*
*   SEE ALSO
*	GetColorMap() ObtainPen() ReleasePen() 
******************************************************************************
; the formula for what error value to pass is:
;
;	(TotalColors+Precision)*8
;	-------------------------   all squared.
;		#FreeColors



TEMP_SIZE	set	0
	LONGVAR	precision
	LONGVAR	tstate
	LONGVAR	colormap
	LONGVAR	save_d1
	LONGVAR	flags

	movem.l	d4/d5/d6/a5,-(a7)
	ALLOCLOCALS
	clr.l	flags_l(a7)
	move.l	d1,save_d1_l(a7)
	move.l	a1,tstate_l(a7)
	move.l	a0,colormap_l(a7)
	clr.l	precision_l(a7)
	move.l	a6,a5
	move.l	gb_UtilBase(a6),a6
tagparse:
	lea	tstate_l(a7),a0
	jsr	_LVONextTagItem(a6)
	tst.l	d0
	beq.s	donetags
	move.l	d0,a0
	move.l	(a0)+,d0
	move.l	(a0)+,d1
	sub.l	#OBP_Precision,d0
	bne.s	not_precision
	move.l	d1,precision_l(a7)
not_precision:
	subq.l	#1,d0
	bne.s	not_failifbad
	move.l	d1,flags_l(a7)
not_failifbad:
	bra.s	tagparse
donetags:
	move.l	a5,a6
	move.l	colormap_l(a7),a0
	move.l	precision_l(a7),d4
	bmi.s	try_exact
	move.l	cm_PalExtra(a0),a1
	add.w	pe_SharableColors(a1),d4
	ext.l	d4
	lsl.l	#3,d4
	move.w	pe_NFree(a1),d0
	beq.s	all_out_of_colors
	divu	d0,d4
	mulu	d4,d4
doit:	lea	default_color_matcher(pc),a1
	moveq	#-1,d6			; realmax
	move.l	flags_l(a7),d5
	beq.s	no_fifbad
	move.l	precision_l(a7),d6
	bmi.s	do_exact
	addq.w	#1,d6
	lsl.l	#5,d6
	mulu	d6,d6
	bra.s	no_fifbad
do_exact:
	moveq	#0,d6
no_fifbad:
	move.l	save_d1_l(a7),d1
	bsr	_SearchColorMap
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d4/d5/d6/a5
	rts

all_out_of_colors:
	moveq	#-1,d4
	bra.s	doit

try_exact:
	moveq	#0,d4
	bra.s	doit


	xdef	_create_color_bits

	xref	_fwmaskTable
	xref	_lwmaskTable

_create_color_bits:
; void __asm create_color_bits(register __a0 struct ViewPort * vp, register __a1 void * bit_mask);
; given a viewport, and a bit vector (256 bits=32 bytes=8 longs), set
; a bit for each color used.
	movem.l	d2/d7/a2/a3/a4/a5,-(a7)
	moveq	#0,d0
	move.l	d0,(a1)+			; init all bits to 0
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	lea	-32(a1),a1
	movem.l	a0/a1,-(a7)
	move.l	a0,-(a7)
	jsr	_new_mode
	lea	4(a7),a7
	move.l	d0,d7				; d7=modes
	movem.l	(a7)+,a0/a1
; depth=bm_depth
; if HAM then bm_depth-=2
; if EHB then depth = MIN(bm_depth, 5)
; if (the viewport has a 3.0 style color map)
;	fillbits(bp0_base,1<<depth)
;	if dualpf then fillbits(bp1_base,1<<bp1_depth)
;	uwordmask[spofs0*2] |= $7fff		set sprite bits. sprites use 1,2,3,5,..
;	uwordmask[spofs1*2] |= $7fff		or 1-15 if attached!
; else
;	fillbits(0,1<<depth)
;	if dualpf then fillbits(8,1<<bp1_depth)
;	if sprites then uwordmask[1] |= $ffff	(yep, even colour 16)
	btst	#5,d7				; super-hires?
	beq.s	no_scramble
	btst	#GFXB_AA_MLISA,gb_ChipRevBits0(a6)		; lisa? if so, no scrambling
	bne.s	no_scramble
	move.l	#-1,(a1)			; if shires, then must load all 32 colors
no_scramble:
	move.l	vp_ColorMap(a0),a3
	cmp	#0,a3
	beq	no_colormap
	moveq	#0,d2
	tst.b	cm_Type(a3)			; 3.0 type?
	beq.s	old_type
	move.b	cm_AuxFlags(a3),d2
	btst	#CMAB_FULLPALETTE,d2
	bne	load_full
	btst	#CMAB_NO_COLOR_LOAD,d2
	bne	load_none

old_type:
	move.l	vp_RasInfo(a0),d0
	beq	no_bitplanes
	move.l	d0,a2
	move.l	ri_BitMap(a2),d1
	beq.s	no_bitplanes
	move.l	d1,a4
	moveq	#0,d1
	move.b	bm_Depth(a4),d1
	btst	#10,d7				; dualpf???
	beq.s	no_dpf1
	btst	#CMAB_DUALPF_DISABLE,d2
	beq.s	no_dpf1
	move.l	ri_Next(a2),d2
	beq.s	no_dpf1
	move.l	d2,a5
	move.l	ri_BitMap(a5),d2
	beq.s	no_dpf1
	move.l	d2,a5
	add.b	bm_Depth(a5),d1
	bclr	#10,d7				; kill dpf!!!

no_dpf1:
	btst	#11,d7				; HAM?
	beq.s	no_ham
	; if ham, if depth is 5 or 6, then depth = 4
	;         if depth is 7 or 8, then depth = 6
	cmp.w	#4,d1
	ble.s	no_ham
	cmp.w	#7,d1
	blt.s	ham6fudge
	moveq	#6,d1
	bra.s	no_ham
ham6fudge:
	moveq	#4,d1
no_ham:
	btst.b	#7,d7				; EHB?
	beq.s	no_ehb
	cmp.b	#5,d1
	ble.s	no_ehb
	moveq	#5,d1
no_ehb:
; now, have d1=playfield 0 depth!!!
	moveq	#1,d0
	lsl.w	d1,d0
	moveq	#0,d1				; d1=playfield 0 base
	bsr.s	setbits
; now, do playfield 1
	btst	#10,d7				; DUALPF?
	beq.s	no_bitplanes
	move.l	ri_Next(a2),d0
	beq.s	no_bitplanes
	move.l	d0,a2
	move.l	ri_BitMap(a2),d0
	beq.s	no_bitplanes
	move.l	d0,a2
	moveq	#1,d0
	moveq	#0,d1
	move.b	bm_Depth(a2),d1
	lsl.w	d1,d0				; d0=ncolors
	moveq	#8,d1				; d1=color base
	bsr.s	setbits
no_bitplanes:
	btst	#6,vp_Modes(a0)			; SPRITES?
	beq.s	bye_ccb
	or.w	#$ffff,2(a1)			; set sprite colors!
bye_ccb:
	movem.l	(a7)+,d2/d7/a2/a3/a4/a5
	rts
load_full:					; if the cm->AuxFlags has FULLPALETTE,
	moveq	#-1,d0				; than load all 256 colors.
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	move.l	d0,(a1)+
	lea	-32(a1),a1
	bra.s	bye_ccb
; if CMAF_NO_COLOR_LOAD is set, then just set colour 0.
load_none:
	bset.b	#7,(a1)
	bra.s	bye_ccb

no_colormap:
 	move.l	#-1,(a1)			; use 32 colours as default.
	bra.s	bye_ccb

setbits:
; d1=bit base, d0=#bits to set, a1=dest
; trashes d1 d2 d0 a4
	ifd	AA_ONLY
1$:	moveq	#32,d2
	cmp.w	d2,d0
	bge.s	2$
	move	d0,d2
2$:	bfset	(a1){d1:d2}
	add	d2,d1
	sub	d2,d0
	bgt.s	1$
	rts

	ELSE

	move.l	a1,a4				; a4=byte base
	move.w	d1,d2
	lsr.w	#3,d2				; d2=byte offset
	add	d2,a4
	and.w	#$7,d1				; d1=bit offset
	beq.s	bitofs_already_0
set_next_unaligned:
	moveq	#7,d2
	sub	d1,d2
	bset	d2,(a4)
	subq.w	#1,d0
	beq.s	done_setbits
	subq.w	#1,d1
	bne.s	set_next_unaligned
	addq.l	#1,a4
; now, bit offset=0, d0=#bits
bitofs_already_0:
	move	d0,d2
	lsr.w	#3,d2
	bra.s	sb_eloop
setbits_lp:
	st	(a4)+
sb_eloop:
	dbra	d2,setbits_lp
no_full_byte:
	and.w	#7,d0
	beq.s	done_setbits
	subq	#1,d0
	add.w	d0,d0
	move.l	a1,d1
	move.l	#_lwmaskTable,a1
	move.b	0(a1,d0.w),d0
	or.b	d0,(a4)
	move.l	d1,a1
done_setbits:
	rts
	endc


	xdef	_numbits
_numbits:
; int __asm numbits(register __d1 ULONG lword1);
; return d0=number of bits set in d1
; trashes a0 d0 d1
	move.l	d2,a0
	moveq	#0,d0
	moveq	#0,d2
1$	add.l	d1,d1
	addx	d2,d0
	tst.l	d1
	bne.s	1$
	move.l	a0,d2
	rts


	xdef	_color_ofs_bits
_color_ofs_bits:
; int __asm color_ofs_bits(register __d1 number);
; return d0=the 3 bit value for color offset NN
;	128->7 64->6 32->5 16->4 8->3 4->2 2->1 0->0
; trashes d1,d0
	tst.b	d1
	beq.s	2$
	moveq	#8,d0
1$:	subq	#1,d0
	add.b	d1,d1
	bcc.s	1$
	rts
2$:	moveq	#0,d0
	rts


* get_bplcon4() - BPLAMx, ESPRMx, OSPRMx
	xdef	_get_bplcon4
_get_bplcon4:
; int __asm get_bplcon4(register __a0 struct ColorMap * cm );
; trashes d0,d1
	move.l	a0,d0
	beq.s	def_bplcon4
	tst.b	cm_Type(a0)
	beq.s	def_bplcon4
	moveq	#0,d0
	move.w	cm_Bp_0_base(a0),d0
	lsl.w	#8,d0
	move.w	cm_SpriteBase_Even(a0),d1
	and.w	#$f0,d1
	or.w	d1,d0
	move.w	cm_SpriteBase_Odd(a0),d1
	lsr.w	#4,d1
	or.w	d1,d0
	rts
def_bplcon4:
	moveq	#$11,d0
	rts
		
* get_bplcon3() - PF2OFx, SPRESx, BRDRBLNKM, BRDRNTRAN, BRDRSPRT
	xdef	_get_bplcon3
_get_bplcon3:
; int __asm get_bplcon3(register __a0 struct ColorMap * cm );
; trashes d0,d1
	move.l	#$c00,d0	; default value
	move.l	a0,d1
	beq.s	got_bplcon3
	tst.b	cm_Type(a0)
	beq.s	got_ofs
	move.w	cm_Bp_1_base(a0),d1
	sub.w	cm_Bp_0_base(a0),d1
	bsr	_color_ofs_bits
got_ofs:
	lsl.w	#8,d0
	add	d0,d0
	add	d0,d0
	move.b	cm_Flags(a0),d1
	btst	#CMB_BRDRBLNK,d1
	beq.s	1$
	bset	#5,d0
1$:	btst	#CMB_BRDNTRAN,d1
	beq.s	2$
	bset	#4,d0
2$:	btst	#CMB_BRDRSPRT,d1
	beq.s	3$
	addq	#2,d0
3$:	move.b	cm_SpriteResolution(a0),d1
	bpl.s	4$
	move.b	cm_SpriteResDefault(a0),d1
4$:	ror.b	#2,d1
	or.b	d1,d0
got_bplcon3:
	or.b	gb_BP3Bits(a6),d0
	rts

* get_bplcon2() - ZDBPSELx, ZDBPEN, ZDCTEN, PF1Px, PF2Px
	xdef	_get_bplcon2
_get_bplcon2:
; int __asm get_bplcon2(register __a0 struct ColorMap * cm );
; trashes d0,d1
	movem.l	a0-a2/d2/d3,-(a7)
	moveq	#$24,d0		; default value
	move.l	a0,d1
	beq	got_bplcon2
	tst.b	cm_Type(a0)
	beq	got_bplcon2
	moveq	#0,d0		; modes
	move.l	cm_vp(a0),a2
	move.l	a0,-(a7)
	move.l	a2,-(a7)
	beq.s	bad_cm_vp
	jsr	_new_mode
bad_cm_vp:
	lea	4(a7),a7
	move.l	(a7)+,a0
	move.l	d0,d2
; now, a0=cm, a2=vp|NULL, d2=modes
	move.b	cm_Flags(a0),d1
	moveq	#0,d0
	btst.b	#CMB_CMTRANS,d1
	beq.s	1$
	or.w	#$400,d0	; ZDCTEN
1$:	btst.b	#CMB_CPTRANS,d1
	beq.s	no_cptrans
	or.w	#$800,d0	; ZDBPEN
	moveq	#0,d1
; if chroma_plane, then scrozzle the bits if 8 bitplane ham
	move.b	cm_TransparencyPlane(a0),d1
	btst	#11,d2		; HAM?
	beq.s	no_scrozzle
	cmp.w	#0,a2		; vp null?
	beq.s	no_scrozzle
	move.l	vp_RasInfo(a2),a1
	cmp.w	#0,a1
	beq.s	no_scrozzle
	move.l	ri_BitMap(a1),a1
	cmp.w	#0,a1
	beq.s	no_scrozzle
	cmp.b	#8,bm_Depth(a1)
	bne.s	no_scrozzle
	addq.b	#2,d1
	and.b	#7,d1
no_scrozzle:
	ror.w	#4,d1		; into bits 12-14
	or.w	d1,d0		; ZDBPSELx
no_cptrans:
	cmp.w	#0,a2
	beq.s	no_vp_2
	move.b	vp_SpritePriorities(a2),d1
; now, check if the viewport is superhires on ecs-denise
	btst	#GFXB_AA_MLISA,gb_ChipRevBits0(a6)	; lisa doesn't need this
	bne.s	bp2_sprite
	btst	#5,d2			; superhires?
	beq.s	bp2_sprite
	lsr.b	#1,d1			; shift priorities because of color scramble.
	bclr.b	#2,d1

bp2_sprite:
	or.b	d1,d0
got_bplcon2:
	movem.l	(a7)+,a0-a2/d2/d3
	rts
no_vp_2:
	moveq	#$24,d1		; default value
	bra.s	bp2_sprite

; see page 302 of HW manual

sc_A	equ	0*8+3
sc_B	equ	0*8+2
sc_C	equ	1*8+7
sc_D	equ	1*8+6
sc_E	equ	1*8+3
sc_F	equ	1*8+2
sc_G	equ	2*8+3
sc_H	equ	2*8+2
sc_I	equ	3*8+7
sc_J	equ	3*8+6
sc_K	equ	3*8+3
sc_L	equ	3*8+2
sc_M	equ	4*8+3
sc_N	equ	4*8+2
sc_O	equ	5*8+7
sc_P	equ	5*8+6
sc_Q	equ	5*8+3
sc_R	equ	5*8+2
sc_S	equ	6*8+3
sc_T	equ	6*8+2
sc_U	equ	7*8+7
sc_V	equ	7*8+6
sc_W	equ	7*8+3
sc_X	equ	7*8+2

; the NO_SCRAMBLE symbols disables ecs shires color-scrambling. This is
; to save space in AA machines.

	IFND	AA_ONLY
scramble_table:
; for each output bit (12x16), select input bit.
; format is bofs*8+bit#
	dc.b	sc_A,sc_B,sc_A,sc_B,sc_C,sc_D,sc_C,sc_D,sc_E,sc_F,sc_E,sc_F
	dc.b	sc_G,sc_H,sc_A,sc_B,sc_I,sc_J,sc_C,sc_D,sc_K,sc_L,sc_E,sc_F
	dc.b	sc_M,sc_N,sc_A,sc_B,sc_O,sc_P,sc_C,sc_D,sc_Q,sc_R,sc_E,sc_F
	dc.b	sc_S,sc_T,sc_A,sc_B,sc_U,sc_V,sc_C,sc_D,sc_W,sc_X,sc_E,sc_F
	dc.b	sc_A,sc_B,sc_G,sc_H,sc_C,sc_D,sc_I,sc_J,sc_E,sc_F,sc_K,sc_L
	dc.b	sc_G,sc_H,sc_G,sc_H,sc_I,sc_J,sc_I,sc_J,sc_K,sc_L,sc_K,sc_L
	dc.b	sc_M,sc_N,sc_G,sc_H,sc_O,sc_P,sc_I,sc_J,sc_Q,sc_R,sc_K,sc_L
	dc.b	sc_S,sc_T,sc_G,sc_H,sc_U,sc_V,sc_I,sc_J,sc_W,sc_X,sc_K,sc_L
	dc.b	sc_A,sc_B,sc_M,sc_N,sc_C,sc_D,sc_O,sc_P,sc_E,sc_F,sc_Q,sc_R
	dc.b	sc_G,sc_H,sc_M,sc_N,sc_I,sc_J,sc_O,sc_P,sc_K,sc_L,sc_Q,sc_R
	dc.b	sc_M,sc_N,sc_M,sc_N,sc_O,sc_P,sc_O,sc_P,sc_Q,sc_R,sc_Q,sc_R
	dc.b	sc_S,sc_T,sc_M,sc_N,sc_U,sc_V,sc_O,sc_P,sc_W,sc_X,sc_Q,sc_R
	dc.b	sc_A,sc_B,sc_S,sc_T,sc_C,sc_D,sc_U,sc_V,sc_E,sc_F,sc_W,sc_X
	dc.b	sc_G,sc_H,sc_S,sc_T,sc_I,sc_J,sc_U,sc_V,sc_K,sc_L,sc_W,sc_X
	dc.b	sc_M,sc_N,sc_S,sc_T,sc_O,sc_P,sc_U,sc_V,sc_Q,sc_R,sc_W,sc_X
	dc.b	sc_S,sc_T,sc_S,sc_T,sc_U,sc_V,sc_U,sc_V,sc_W,sc_X,sc_W,sc_X

; void __asm scramble_colors(register __a0 UWORD *srcdata, register __a1 UWORD *dest);
; generates 16 uwords from 
	endc
	xdef	_scramble_colors

_scramble_colors:
	IFND	AA_ONLY
	movem.l	a2/d2/d3/d4,-(a7)
	lea	scramble_table(pc),a2
	moveq	#15,d0					; output ctr
	moveq	#0,d3
1$:	moveq	#0,d1					; output word
	moveq	#11,d2					; bit ctr
2$:	move.b	(a2)+,d3				; d3=table entry
	move	d3,d4
	lsr.w	#3,d4
	add.w	d1,d1
	btst	d3,0(a0,d4.w)
	beq.s	3$
	addq	#1,d1
3$:	dbra	d2,2$
	move.w	d1,(a1)+
	dbra	d0,1$
	movem.l	(a7)+,a2/d2/d3/d4
	ENDC
	rts

	end
