*******************************************************************************
*
*	$Id: downregions.asm,v 42.0 93/06/16 11:17:23 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/memory.i'
    include 'exec/libraries.i'
    include 'graphics/gfxbase.i'
    include 'graphics/regions.i'

*------ library dispatch macros --------------------------------------


LINKEXE		MACRO
		LINKLIB _LVO\1,gb_ExecBase(a6)
		ENDM

CALLGFX		MACRO
		CALLLIB _LVO\1
		ENDM

XREF_EXE	MACRO
		XREF	_LVO\1
		ENDM

XREF_GFX	MACRO
		XREF	_LVO\1
		ENDM

    section	graphics

*   downcoded from c  to assembly language
*   begun 03.21.89 -- bart original c source size 2552 bytes
*
*   chosen for downcoding for these reasons:
*
*   1) limited access to out-of-file routines
*   2) called often from layering code
*   3) some routines recursive
*   4) much leverage used
*
*   in particular, item 2 above indicates that optimisation will 
*   result in subtantial speeup to layering operations, and for
*   testing, any bugs introduced in the process of downcoding 
*   main routines will be easily visible in layers operations.

    XREF_EXE Alert
    XREF_EXE AllocMem
    XREF_EXE FreeMem

    xref _intersect					; in bltrastport.c
    xref _crectXrect					; in misc.asm

dOffsetRegionRectangle equ 1
dadjustregionrectangles equ 1
dNewRegion equ 1
dobscured equ 1
dnewrect equ 1
dfreerr equ 1

dreplaceregion equ 1
dclearregion equ 1
dSettleDown equ 1

dappendrr equ 1
dremoverr equ 1
dadjustregion equ 1

ddisposeregion equ 1

dandregionregion equ 1
dorregionregion equ 1
dxorregionregion equ 1

dandrectregion equ 1
dxorrectregion equ 1
dclearrectregion equ 1

dorrectregion equ 1
drectsplit equ 1

dprependrr equ 1

AN_REGIONMEMORY	equ $8201000B

OPERATION_OR    equ 0
OPERATION_AND	equ 1
OPERATION_XOR	equ 2
OPERATION_CLEAR	equ 3

	ifne dadjustregion
_adjustregion:
	movem.l	a2,-(sp)
	move.l	8(sp),a2
	move.l	12(sp),a0
	move.l	16(sp),a1
	bsr.s	aadjustregion
	movem.l	(sp)+,a2
	rts
	endc

	ifne dreplaceregion
areplaceregion:						; must preserve a0 !!!
	movem.l	a0/a1,-(sp)
	ifne    rg_bounds
		fail
    	endc
	ifne    ra_MinX
		fail
    	endc
	move.l	a1,a0	
	bsr	_ClearRegion				; to in a0
	move.l	(sp),a1					; from in a1
	move.l	(a1),(a0)				; copy minx/y
	move.l	ra_MaxX(a1),ra_MaxX(a0)			; copy maxx/y
	move.l	rg_RegionRectangle(a1),d0		; save regionrect
	clr.l	rg_RegionRectangle(a1)			; clear from donor
	move.l	d0,rg_RegionRectangle(a0)
	beq.s	1$					; skip link and settle
	lea.l	rg_RegionRectangle(a0),a1		; &to->RegionRectangle
	move.l	a1,d1					; store temporarily
	move.l	d0,a1					; retreive regionrects
	move.l	d1,rr_Prev(a1)				; and complete linkage
1$	bsr.s	aSettleDown
 	movem.l	(sp)+,a0/a1
    	rts
	endc

	ifne    dadjustregion
aadjustregion:  ; preserves  rgn in a0, rect in a1, newrgn in a2
	ifne    rg_bounds
		fail
    	endc
	ifne    ra_MinX
		fail
    	endc
    	tst.l	rg_RegionRectangle(a0)
	if	=
		move.l	(a1),(a2)
		move.l	ra_MaxX(a1),ra_MaxX(a2)
	else
		move.l	(a0),d0	
		move.l	ra_MaxX(a0),d1	
		cmp.w	ra_MinY(a1),d0
		ble.s	1$
		move.w	ra_MinY(a1),d0
1$		move.w	d0,ra_MinY(a2)
		swap	d0
		cmp.w	(a1),d0
		ble.s	2$
		move.w	(a1),d0
2$		move.w	d0,(a2)
		swap	d0
		cmp.w	ra_MaxY(a1),d1
		bge.s	3$
		move.w	ra_MaxY(a1),d1
3$		move.w	d1,ra_MaxY(a2)
		swap	d1
		cmp.w	ra_MaxX(a1),d1
		bge.s	4$
		move.w	ra_MaxX(a1),d1
4$		move.w	d1,ra_MaxX(a2)
		move.l	(a0),d1
		cmp.l	(a2),d1				; aha!... better test
		if	<>
			move.w	d0,d1			; newrgn.MinY in d1.w
			sub.w	ra_MinY(a0),d1		; minus rgn.MinY
			swap	d0			; newrgn.MinX in d0.w
			sub.w	(a0),d0			; minus rgn.MinX
			bsr.s 	aadjustregionrectangles ; must preserve scratch
		endif
	endif
	clr.l	rg_RegionRectangle(a2)
	move.l	(a2),(a0)				; copy new bounds to rgn
	move.l	ra_MaxX(a2),ra_MaxX(a0)
    	rts
	endc

	ifne dSettleDown
aSettleDown:
	movem.l	d2,-(sp)
	ifne    rr_Next
		fail
    	endc
	ifne    rg_bounds
		fail
    	endc
	ifne    ra_MinX
		fail
    	endc
    	lea.l	rg_RegionRectangle(a0),a1
    	move.w	#15000,d0				; set x in d0, y in d1
    	move.w	d0,d1					; enough for big blits?
    	move.l	(a1),d2					; get first regionrect
	if	<>
		repeat
			movea.l	d2,a1		
			move.l	(a1),d2			; next rect ptr
			cmp.w	rr_bounds+ra_MinX(a1),d0
			ble.s	1$
			move.w	rr_bounds+ra_MinX(a1),d0
1$ 			cmp.w	rr_bounds+ra_MinY(a1),d1
			ble.s	2$
			move.w	rr_bounds+ra_MinY(a1),d1
2$ 			tst.l	d2
		until   =
    	endif
	bsr.s	aadjustregionrectangles			; must preserve scratch
	add.w	d0,(a0)					; adjust region bounds
	add.w	d0,ra_MaxX(a0)
	add.w	d1,ra_MinY(a0)
	add.w	d1,ra_MaxY(a0)
	movem.l	(sp)+,d2
    	rts
	endc

	ifne dOffsetRegionRectangle
aOffsetRegionRectangle:					; must preserve scratch!
	sub.w	d0,rr_bounds+ra_MinX(a1)
	sub.w	d0,rr_bounds+ra_MaxX(a1)
	sub.w	d1,rr_bounds+ra_MinY(a1)
	sub.w	d1,rr_bounds+ra_MaxY(a1)
    	rts
	endc

	ifne dadjustregionrectangles
aadjustregionrectangles:				; must preserve scratch!
	movem.l	a0/a1/d0/d1/d2,-(sp)			; rgn:a0, x:d0.w, y:d1.w
	ifne    rr_Next
		fail
    	endc
    	lea.l	rg_RegionRectangle(a0),a1
    	move.l	(a1),d2					; get first regionrect
	if	<>
		repeat
			movea.l	d2,a1		
			move.l	(a1),d2		      	; next rect ptr
			bsr.s	aOffsetRegionRectangle 	; must preserve scratch
 			tst.l	d2
		until   =
    	endif
	movem.l	(sp)+,a0/a1/d0/d1/d2
    	rts
	endc

	ifne dandrectregion
	xdef	_andrectregion
	xdef	_AndRectRegion
_andrectregion:
	move.l	4(sp),a0				; rgn
	move.l	8(sp),a1				; rect
_AndRectRegion: 
*	movem.l a0/a1,-(sp)
*	jsr     _andrectregion
*	movem.l (sp)+,a0/a1
*	rts
	ifne    rg_bounds
		fail
    	endc
	ifne    ra_MinX
		fail
    	endc
	movem.l	a0/a1/a2/a3/a4/a5/d2,-(sp)
	suba.l	#rg_SIZEOF,sp
	move.l	sp,a2					; newrgn in a2
	clr.l	rg_RegionRectangle(a2)
	tst.l	rg_RegionRectangle(a0)			; null region?
	beq	arr_rts
	movem.l	a0/a1/a2,-(sp)				; psh args
	jsr	_intersect
	movem.l	(sp)+,a0/a1/a2				; pop args
	move.l	(a2),d0					; minx/y for newrgn
	cmp.l	(a0),d0					; rgn minx/y differ?
	beq.s	1$					; no, skip adjustment
	sub.w	ra_MinY(a0),d0
	move.w	d0,d1	
	swap	d0
	sub.w	(a0),d0
	bsr.s	aadjustregionrectangles			; must preserve scratch
1$	move.l	(a2),(a0)				; now everybody's in the
	move.l	ra_MaxX(a2),ra_MaxX(a0) 		; same coordinate system
	suba.l	#rr_SIZEOF,sp				; rr_tmp
	lea.l	rr_bounds(sp),a5			; &rr_tmp.bounds in a5
	move.l	(a1),(a5)				; rr_tmp.bounds = *rect
	move.l	ra_MaxX(a1),ra_MaxX(a5)
	move.l	sp,a1					; sub rr_tmp for rect
	move.w	(a0),d0	
	move.w	ra_MinY(a0),d1	
	bsr	aOffsetRegionRectangle			; preserves a0/a1
	move.l	a0,a3					; protect rgn ptr in a3
	move.l	rg_RegionRectangle(a3),d2 		; for all rgn->rects
	if	<>
	    repeat
		movea.l	d2,a4				; r in a4
		move.l	(a4),d2				; next rgn rr ptr
		pea.l	rr_bounds(a4)			; &r->bounds
		move.l	a5,-(sp)			; &rr_tmp.bounds
		jsr	_crectXrect
		tst.l	d0
		beq.s	arr_nox
		pea.l	rr_bounds(a4)			; psh &r->bounds
		bsr	_obscured			; check one way
		addq.l	#4,sp				; pop &r->bounds
		tst.l	d0
		if	=				; cut it up into parts
		    bsr	    	_obscured		; args already set up
		    tst.l   	d0
		    if	<>
			move.l	a3,a0			; rgn
			move.l	a4,a1			; r
			bsr	aremoverr
			move.l	a2,a0			; newrgn
			move.l	a4,a1			; r
			bsr	aprependrr
		    else
			moveq.l	#OPERATION_AND,d0	; psh args
			move.l	d0,-(sp)		; operation
			pea.l	rr_bounds(a4)		; &r->bounds
			move.l	ra_MaxX(a5),-(sp)	; pass by value
			move.l	(a5),-(sp)		; rr_tmp.bounds
			move.l	a2,-(sp)		; &newrgn
			bsr	_rectsplit		; and call c interface
			lea.l	20(sp),sp		; restore stack
		    endif
		else					; accept it completely
			bsr	_newrect
			move.l	d0,rg_RegionRectangle(a2)
			beq.s	arr_abt			; emergency abort
			move.l	d0,a1						
			move.l	(a5),rr_bounds+ra_MinX(a1)
			move.l	ra_MaxX(a5),rr_bounds+ra_MaxX(a1)
arr_abt			addq.l	#8,sp			; clean stack
			bra.s	arr_brk			; break loop
		endif
arr_nox		addq.l	#8,sp				; clean stack
		tst.l	d2
	    until =
	endif
arr_brk	move.l	a2,a0					; from newrgn
	move.l	a3,a1					; to rgn
	bsr	areplaceregion				; clean out the old list
	adda.l	#rr_SIZEOF,sp				; done, pop rr_tmp
arr_rts	adda.l	#rg_SIZEOF,sp				; exit, pop newrgn
	movem.l	(sp)+,a0/a1/a2/a3/a4/a5/d2
	rts
	endc

	ifne dorrectregion
	xdef _orrectregion				
	xdef _OrRectRegion
_orrectregion:
	move.l	4(sp),a0				; rgn
	move.l	8(sp),a1				; rect
_OrRectRegion:
	ifne    ra_MinX
		fail
    	endc
	movem.l	a0/a1/a2/a3/d2/d3/d4,-(sp)
	suba.l	#rg_SIZEOF,sp
	move.l	sp,a2					; newrgn in a2
	bsr	aadjustregion
	bsr	_newrect
	tst.l	d0
	beq	orr_rts
	move.l	a2,a0					; newrgn in a0
	move.l	d0,a1					; r in a1
	bsr	aappendrr
	move.l	16+rg_SIZEOF(sp),a2			; rect in a2
	move.l	(a2),rr_bounds+ra_MinX(a1)		; copy rectangle
	move.l	ra_MaxX(a2),rr_bounds+ra_MaxX(a1)	; into real storage
	move.l	12+rg_SIZEOF(sp),a2			; rgn in a2
	move.w	(a2),d0
	move.w	ra_MinY(a2),d1
	bsr	aOffsetRegionRectangle
    	lea.l	rg_RegionRectangle(a2),a2
    	move.l	(a2),d2					; get first regionrect
	if	<>
	    repeat
		movea.l	d2,a3				; r in a3
		move.l	(a3),d2				; next rgn rr ptr
		lea.l	(sp),a0				; newrgn in a0
		lea.l	rg_RegionRectangle(a0),a1	; inner loop
		move.l	(a1),d3				; r1 in d3
		if	<>
		    repeat
			movea.l	d3,a1			; r1 in a1
			move.l	(a1),d4			; r2 == r1->Next
			pea.l	rr_bounds(a1)		; &r1->bounds
			pea.l	rr_bounds(a3)		; &r->bounds
			jsr	_crectXrect		; short branch possible?
			tst.l	d0
			beq.s	1$
			bsr	_obscured
			tst.l	d0
			bne.s	2$
			moveq.l	#OPERATION_OR,d0	; operation
			lea.l	8(sp),a2		; newrgn in a2
			move.l	4(sp),a1		; &r1->bounds
			move.l	(sp),a0			; &r->bounds
			bsr	arectsplit		; do rectsplit stuff
			tst.l	d4
			bne.s	2$
			movea.l	d3,a1			; r1 in a1
			move.l	(a1),d4			; r2 == r1->Next
2$			lea.l	8(sp),a0		; newrgn in a0
			movea.l	d3,a1			; r1 in a1
			bsr	aremoverr
			bsr	afreerr			; free r1
1$			addq.l	#8,sp			; clean stack
 			move.l	d4,d3			; r1 == r2
		    until   =
		endif
		tst.l	d2
	    until   =
    	endif
	move.l	12+rg_SIZEOF(sp),a0			; rgn in a0
    	lea.l	rg_RegionRectangle(sp),a1		; newrgn.RegionRectangle
    	move.l	(a1),d2					; get first regionrect
	if	<>
	    repeat
		movea.l	d2,a1
		move.l	(a1),d2				; next newrgn rr ptr
		bsr	aprependrr			; preserve a0 thru loop
		tst.l	d2
	    until   =
    	endif
	moveq.l	#1,d0
orr_rts	adda.l	#rg_SIZEOF,sp
	movem.l	(sp)+,a0/a1/a2/a3/d2/d3/d4
    	rts
	endc

	ifne dorregionregion
	xdef _orregionregion 
	xdef _OrRegionRegion
_orregionregion:
	move.l	4(sp),a0				; rgnsrc
	move.l	8(sp),a1				; rgndst
_OrRegionRegion:
	movem.l	a0/a1/a2/d2,-(sp) 			; must preserve a0/a1 !!
	ifne    rr_Next
		fail
    	endc
	ifne    rg_bounds
		fail
    	endc
	move.l	a0,a2
	move.l	a1,a0
    	lea.l	rg_RegionRectangle(a2),a1		; preserve a0 thru loop
    	move.l	(a1),d2					; get first regionrect
	if	<>
		repeat
			movea.l	d2,a1		
			move.l	(a1),d2			; next rect ptr
			move.l	rr_bounds+ra_MinX(a1),d0
			move.l	rr_bounds+ra_MaxX(a1),d1
			add.w	ra_MinY(a2),d0
			swap	d0
			add.w	ra_MinX(a2),d0
			swap	d0
			add.w	ra_MinY(a2),d1
			swap	d1
			add.w	ra_MinX(a2),d1
			swap	d1
			movem.l d1/d0,-(sp)		; temp rect on stack
			move.l	sp,a1			; and pass to xorrect
			bsr	_OrRectRegion		; preserve a0, a1
			addq.l  #8,sp			; pop temp rect 
			tst.l	d2
		until   =
    	endif
	movem.l	(sp)+,a0/a1/a2/d2
	moveq.l	#1,d0
    	rts
	endc

	ifne dclearrectregion
	xdef _ClearRectRegion
_ClearRectRegion:
	movem.l	a0/a1,-(sp)				; prepare stack
	bsr	_NewRegion
	tst.l	d0
	beq.s	1$
	move.l	d0,a1					; rgntmp in a1
	move.l	(sp),a0					; rgn
	bsr.s	_OrRegionRegion
	move.l	a1,a0					; rgntmp in a0
	move.l	4(sp),a1				; rect in a1
	bsr	_AndRectRegion
	move.l	(sp),a1					; rgn in a1
	bsr.s	_XorRegionRegion
	bsr	_DisposeRegion				; args already set up
	moveq.l	#1,d0					; success
1$ 	addq.l	#8,sp					; restore stack
	rts
	endc

	ifne dxorregionregion
	xdef _xorregionregion 
	xdef _XorRegionRegion
_xorregionregion:
	move.l	4(sp),a0				; rgnsrc
	move.l	8(sp),a1				; rgndst
_XorRegionRegion:
	movem.l	a0/a1/a2/d2,-(sp) 			; must preserve a0/a1 !!
	ifne    rr_Next
		fail
    	endc
	ifne    rg_bounds
		fail
    	endc
	move.l	a0,a2
	move.l	a1,a0
    	lea.l	rg_RegionRectangle(a2),a1		; preserve a0 thru loop
    	move.l	(a1),d2					; get first regionrect
	if	<>
		repeat
			movea.l	d2,a1		
			move.l	(a1),d2			; next rect ptr
			move.l	rr_bounds+ra_MinX(a1),d0
			move.l	rr_bounds+ra_MaxX(a1),d1
			add.w	ra_MinY(a2),d0
			swap	d0
			add.w	ra_MinX(a2),d0
			swap	d0
			add.w	ra_MinY(a2),d1
			swap	d1
			add.w	ra_MinX(a2),d1
			swap	d1
			movem.l d1/d0,-(sp)		; temp rect on stack
			move.l	sp,a1			; and pass to xorrect
			bsr	_XorRectRegion		; preserve a0, a1
			addq.l  #8,sp			; pop temp rect 
			tst.l	d2
		until   =
    	endif
	movem.l	(sp)+,a0/a1/a2/d2
	moveq.l	#1,d0
    	rts
	endc

	ifne dNewRegion
	xdef _NewRegion	 
_NewRegion:
	move.l 	#MEMF_CLEAR,d1
    	move.l	#rg_SIZEOF,d0
    	LINKEXE	AllocMem
    	rts
	endc

	ifne dandregionregion
	xdef _andregionregion
	xdef _AndRegionRegion
_andregionregion:
	move.l	4(sp),a0				; src in a0
	move.l	8(sp),a1				; dst in a1
_AndRegionRegion:
	movem.l	a0/a1/a2/a3/a4/d2,-(sp)			; must preserve a0/a1 !!
	bsr.s  	_NewRegion
	tst.l	d0
	beq.s	1$
	move.l	d0,a2					; newrgn in a2
	bsr.s	_NewRegion
	tst.l	d0
	bne.s	2$
	move.l	a2,a1
	bsr.s	bdisposeregion
	moveq.l	#0,d0
1$ 	movem.l	(sp)+,a0/a1/a2/a3/a4/d2			; error exit
    	rts
2$	move.l	d0,a1					; rgn in a1
	ifne    rr_Next
		fail
    	endc
	ifne    rg_bounds
		fail
    	endc
	move.l	4(sp),a4				; retreive rgnsrc
    	lea.l	rg_RegionRectangle(a4),a3
    	move.l	(a3),d2					; get first regionrect
	if	<>
		repeat
			move.l	8(sp),a0		; retrieve rgndst in a0
			bsr	_OrRegionRegion		; rgn already in a1
			movea.l	d2,a3			
			move.l	(a3),d2			; next rect ptr
			move.l	rr_bounds+ra_MinX(a3),d0
			move.l	rr_bounds+ra_MaxX(a3),d1
			add.w	ra_MinY(a4),d0
			swap	d0
			add.w	ra_MinX(a4),d0
			swap	d0
			add.w	ra_MinY(a4),d1
			swap	d1
			add.w	ra_MinX(a4),d1
			swap	d1
			move.l	a1,a0			; rgn now in a0
			movem.l d1/d0,-(sp)		; temp rect on stack
			move.l	sp,a1			; and pass to xorrect
			bsr	_AndRectRegion
			addq.l  #8,sp			; pop temp rect 
			move.l	a2,a1			; newrgn now in a1
			bsr	_OrRegionRegion		; rgn already in a0
			bsr.s	_ClearRegion		; clear the rgn in a0
			move.l	a0,a1			; rgn back in a1
			tst.l	d2
		until   =
    	endif
	bsr.s	bdisposeregion				; rgn already cleared
	move.l	a2,a0					; newrgn now in a0
	move.l	8(sp),a1				; retrieve rgndst in a1
	bsr	areplaceregion				; saves a0, destroys a1
	bsr.s	_DisposeRegion				; fully dispose newrgn 
	moveq.l	#1,d0
 	movem.l	(sp)+,a0/a1/a2/a3/a4/d2
    	rts
	endc

	ifne	ddisposeregion
	xdef _disposeregion
	xdef _DisposeRegion
_disposeregion:
    	move.l 	4(sp),a0
_DisposeRegion:
    	bsr.s 	bclearregion				; must free regionrects
    	move.l 	a0,a1
bdisposeregion:						; just free the sucker
    	move.l 	#rg_SIZEOF,d0
    	LINKEXE FreeMem
    	rts
	endc

	ifne	dclearregion
	xdef	_clearregion
	xdef	_ClearRegion
_clearregion:
	move.l	4(sp),a0
_ClearRegion: 	
	ifne    rg_bounds
		fail
    	endc
	ifne    ra_MinX
		fail
    	endc
	clr.l	(a0)					; clear MinX/MinY
	clr.l	ra_MaxX(a0)				; clear MaxX/MaxY
bclearregion:	; must preserve a0 !!
	movem.l	a0/d2,-(sp)
	ifne    rr_Next
		fail
    	endc
    	lea.l	rg_RegionRectangle(a0),a1
    	move.l	(a1),d2					; get first regionrect
	if	<>
		repeat
			movea.l	d2,a1		
			move.l	(a1),d2			; next rect ptr
			bsr.s	afreerr			; free this regionrect
			tst.l	d2
		until   =
    	endif
	movem.l	(sp)+,a0/d2
	clr.l	rg_RegionRectangle(a0)			; all gone
    	rts
	endc

	ifne	dobscured
	xdef _obscured
_obscured:
	move.l	4(sp),a0				; rect1 in a0
	move.l	8(sp),a1				; rect2 in a1
aobscured:
	moveq.l	#0,d0					; default return false
	move.w	ra_MinX(a0),d1
	sub.w	ra_MinX(a1),d1
	bgt.s   1$
	move.w	ra_MaxX(a0),d1
	sub.w	ra_MaxX(a1),d1
	blt.s   1$
	move.w	ra_MinY(a0),d1
	sub.w	ra_MinY(a1),d1
	bgt.s   1$
	move.w	ra_MaxY(a0),d1
	sub.w	ra_MaxY(a1),d1
	blt.s   1$
	moveq.l	#1,d0					; obscured == TRUE
1$: 	rts
	endc

	ifne dfreerr
afreerr:
    	move.l 	#rr_SIZEOF,d0 				; regionrect ptr in a1
    	LINKEXE FreeMem
    	rts
	endc

	ifne dremoverr
aremoverr:
	movem.l	a0/a1,-(sp)
	ifne    rr_Next
		fail
    	endc
	move.l	rr_Prev(a1),d1				; prev rect ptr
	move.l	(a1),d0					; next rect ptr
	if	<>
		move.l	d0,a1
		move.l	d1,rr_Prev(a1)
	endif
	move.l	d1,a0
	move.l	d0,(a0)
	movem.l	(sp)+,a0/a1
    	rts
	endc

	ifne drectsplit
	xdef _rectsplit
_rectsplit:
	move.l	a2,-(sp)
	move.l	8(sp),a2
	lea.l	12(sp),a1
	move.l	20(sp),a0
	move.l	24(sp),d0
	bsr.s	arectsplit
	move.l	(sp)+,a2
	rts
arectsplit:
	movem.l	d2/d3/d4/d5,-(sp)
	move.l	ra_MaxX(a0),d5
	move.l	ra_MinX(a0),d4				; r2
	move.l	ra_MaxX(a1),d3				
	move.l	ra_MinX(a1),d2				; r1
	moveq.l	#0,d1					; xrect false
	cmp.w	d5,d2					
	bgt.s	1$					; r2->MaxY < r1->MinY ?
	cmp.w	d4,d3					
	blt.s	1$					; r2->MinY > r1->MaxY ?
	swap	d5
	swap	d4
	swap	d3
	swap	d2
	cmp.w	d5,d2					
	bgt.s	2$					; r2->MaxX < r1->MinX ?
	cmp.w	d4,d3					
	blt.s	2$					; r2->MinX > r1->MaxX ?
	moveq.l	#1,d1
2$	swap	d2
	swap	d3
	swap	d4
	swap	d5
1$	cmp.l	#OPERATION_OR,d0			; check op 
	bne	spnd
spor:	tst.l	d1
	beq	3$
	cmp.w	d5,d3					; r2->MaxY < r1->MaxY ?
	ble.s	4$					; no
	bsr	_newrect				; yes
	tst.l	d0
	beq	sprt
	move.l	d0,a1
	move.l	d2,rr_bounds+ra_MinX(a1)
	move.l	d3,rr_bounds+ra_MaxX(a1)
	move.w	d5,d3					; r1->MaxY = r2->MaxY
	add.w	#1,d5
	move.w	d5,rr_bounds+ra_MinY(a1)
	move.l	a2,a0
	bsr	aappendrr
4$ 	cmp.w	d4,d2					; r2->MinY > r1->MinY ?
	bge.s	5$					; no
	bsr	_newrect				; yes
	tst.l	d0
	beq	sprt
	move.l	d0,a1
	move.l	d2,rr_bounds+ra_MinX(a1)
	move.l	d3,rr_bounds+ra_MaxX(a1)
	move.w	d4,d2					; r1->MinY = r2->MinY
	sub.w	#1,d4
	move.w	d4,rr_bounds+ra_MaxY(a1)
	move.l	a2,a0
	bsr	aappendrr
5$ 	swap	d5
	swap	d4
	swap	d3
	swap	d2
	cmp.w	d5,d3					; r2->MaxX < r1->MaxX ?
	ble.s	6$					; no
	bsr	_newrect				; yes
	tst.l	d0
	beq	sprt
	move.l	d0,a1
	swap	d2
	swap	d3
	move.l	d2,rr_bounds+ra_MinX(a1)
	move.l	d3,rr_bounds+ra_MaxX(a1)
	swap	d3
	swap	d2
	move.w	d5,d3					; r1->MaxX = r2->MaxX
	add.w	#1,d5
	move.w	d5,rr_bounds+ra_MinX(a1)
	move.l	a2,a0
	bsr	aappendrr
6$ 	cmp.w	d4,d2					; r2->MinX > r1->MinX ?
	bge.s	7$					; no
	bsr	_newrect				; yes
	tst.l	d0
	beq.s	sprt
	move.l	d0,a1
	swap	d2
	swap	d3
	move.l	d2,rr_bounds+ra_MinX(a1)
	move.l	d3,rr_bounds+ra_MaxX(a1)
	swap	d3
	swap	d2
	move.w	d4,d2					; r1->MinX = r2->MinX
	sub.w	#1,d4
	move.w	d4,rr_bounds+ra_MaxX(a1)
	move.l	a2,a0
	bsr.s	aappendrr
7$	bra.s	sprt
3$ 	bsr.s	_newrect
	tst.l	d0
	beq.s	sprt
	move.l	d0,a1
	bra.s	spr1
spnd:	tst.l	d1
	beq.s	sprt
	bsr.s	_newrect
	tst.l	d0
	beq.s	sprt
	move.l	d0,a1
	cmp.w	d5,d3
	ble.s	8$					; r2->MaxY < r1->MaxY ?
	move.w	d5,d3					; r1->MaxY = r2->MaxY
8$ 	cmp.w	d4,d2				
	bge.s	9$					; r2->MinY > r1->MinY ?
	move.w	d4,d2					; r1->MinY = r2->MinY
9$	swap	d5
	swap	d4
	swap	d3
	swap	d2
	cmp.w	d5,d3
	ble.s	10$					; r2->MaxX < r1->MaxX ?
	move.w	d5,d3					; r1->MaxX = r2->MaxX
10$ 	cmp.w	d4,d2				
	bge.s	11$					; r2->MinX > r1->MinX ?
	move.w	d4,d2					; r1->MinX = r2->MinX
11$	swap	d2
	swap	d3
	swap	d4
	swap	d5
spr1:   move.l	d2,rr_bounds+ra_MinX(a1)
	move.l	d3,rr_bounds+ra_MaxX(a1)
	move.l	a2,a0
	bsr.s	aappendrr
sprt:	movem.l	(sp)+,d2/d3/d4/d5
  	rts
	endc

	ifne dappendrr
aappendrr:						; preserve rgn in a0
	move.l	a0,-(sp)
	ifne    rr_Next
		fail
  	endc
    	lea.l	rg_RegionRectangle(a0),a0
    	move.l	(a0),d0					; get first regionrect
	if	<>
		repeat
			movea.l	d0,a0		
			move.l	(a0),d0			; next rect ptr
		until   =
 	endif
	move.l	a0,rr_Prev(a1)				; set tail prev
	clr.l	(a1)					; clr tail next
	move.l	a1,(a0)					; link rect to list tail
	move.l	(sp)+,a0
    	rts
	endc

	ifne	dnewrect
_newrect:
    	move.l 	#MEMF_CLEAR,d1
    	move.l 	#rr_SIZEOF,d0
    	LINKEXE AllocMem
    	rts
	endc

	ifne dprependrr
aprependrr:						; preserve rgn in a0
	move.l	a0,-(sp)				; try prepending
	ifne    rr_Next
		fail
   	endc
    	lea.l	rg_RegionRectangle(a0),a0
	move.l	a0,d1
	move.l	(a0),d0
	beq.s	1$
	move.l	d0,a0
	move.l	a1,rr_Prev(a0)
1$ 	move.l	d0,(a1)
	move.l	d1,rr_Prev(a1)
	move.l	d1,a0
	move.l	a1,(a0)
	move.l	(sp)+,a0
    	rts
	endc

	ifne dxorrectregion
	xdef _XorRectRegion
_XorRectRegion:
	ifne    rg_bounds
		fail
   	endc
	ifne    ra_MinX
		fail
   	endc
	tst.l	rg_RegionRectangle(a0)
	bne.s	1$
	bsr	_OrRectRegion
	rts
1$	movem.l	a0/a1/a2/a3/a4/a5/d2/d3,-(sp)
	suba.l	#rg_SIZEOF+rg_SIZEOF,sp
	move.l	sp,a2					; outsideregion
	lea.l	rg_SIZEOF(sp),a3			; insideregion
	bsr	aadjustregion				; preserves a0/a1/a2
	move.l	(a2),(a3)				; make identical
	move.l	ra_MaxX(a2),ra_MaxX(a3)
	move.l	rg_RegionRectangle(a2),rg_RegionRectangle(a3)
	move.l	a0,a4					; save rgn ptr
	move.l	a1,a5					; save rect ptr
	bsr	_newrect				; get a new rectangle
	tst.l	d0
	beq	xrr_rts
	move.l	d0,a1
	move.l	d0,a3					; save nrect ptr
	move.l	(a5),rr_bounds+ra_MinX(a1)		; nrect->bounds = *rect
	move.l	ra_MaxX(a5),rr_bounds+ra_MaxX(a1)
	move.w	(a4),d0					; d0:rgn->bounds.MinX
	move.w	ra_MinY(a4),d1				; d1:rgn->bounds.MinY
	bsr	aOffsetRegionRectangle			; preserves a1
	lea.l	rg_RegionRectangle(a4),a5		; done with rect ptr
	move.l	(a5),d2					; construct outside list
	if	<>
	    repeat
		    movea.l	d2,a5		
		    move.l	(a5),d2			; next r
		    moveq.l	#OPERATION_OR,d0	; operation
		    move.l	sp,a2			; outsideregion
		    lea.l	rr_bounds(a5),a1	; &r->bounds
		    lea.l	rr_bounds(a3),a0	; &nrect->bounds
		    bsr		arectsplit
		    tst.l	d2
	    until   =
    	endif
	lea.l	rg_SIZEOF(sp),a0			; insideregion
	move.l	a3,a1					; nrect ptr
	bsr	aprependrr				; start with everything
	lea.l	rg_RegionRectangle(a4),a5		; construct inside list
	move.l	(a5),d2					; get first regionrect
	if	<>
	    repeat
		    move.l   d2,a5		
		    move.l   (a5),d2			; next r
		    lea.l    rg_SIZEOF+rg_RegionRectangle(sp),a3
		    move.l   (a3),d3			; r1
		    clr.l    (a3)			; start from scratch
		    tst.l    d3
		    if	<>
			repeat
			    movea.l d3,a3		; current r1
			    moveq.l #OPERATION_OR,d0
			    lea.l   rg_SIZEOF(sp),a2	; insideregion
			    lea.l   rr_bounds(a3),a1	; &r1->bounds
			    lea.l   rr_bounds(a5),a0	; &r->bounds
			    bsr     arectsplit
			    move.l  (a3),d3		; next r1
			    move.l  a3,a1
			    bsr	    afreerr		; free r1
			    tst.l   d3
			until   =
		    endif
		    tst.l	d2
	    until   =
	endif
	move.l  sp,a0					; outsideregion
	lea.l    rg_SIZEOF+rg_RegionRectangle(sp),a1
	move.l   (a1),d2				; combine regions
	if	<>
	    repeat
		movea.l d2,a1		
		move.l	(a1),d2
		lea.l   rg_SIZEOF(sp),a0		; insideregion
		bsr	aremoverr			; preserves a1
		move.l  sp,a0				; outsideregion
		bsr	aprependrr			; preserves a0
		tst.l   d2
	    until   =
	endif
	move.l	a4,a1					; from outside to rgn
	bsr	areplaceregion				; all done
	moveq.l	#1,d0
xrr_rts	adda.l	#rg_SIZEOF+rg_SIZEOF,sp			; clean stack
	movem.l	(sp)+,a0/a1/a2/a3/a4/a5/d2/d3
	rts
	endc

******* graphics.library/AndRectRegion **************************************
*
*   NAME
*	AndRectRegion -- Perform 2d AND operation of rectangle
*			 with region, leaving result in region.
*
*   SYNOPSIS
*	AndRectRegion(region,rectangle)
* 			a0	a1
*
*	void AndRectRegion( struct Region *, struct Rectangle * );
*
*   FUNCTION
*	Clip away any portion of the region that exists outside
*	of the rectangle. Leave the result in region.
*
*   INPUTS
*	region - pointer to Region structure
*	rectangle - pointer to Rectangle structure
*
*   NOTES
*	Unlike the other rect-region primitives, AndRectRegion() cannot
*	fail.
*
*   BUGS
*
*   SEE ALSO
*	AndRegionRegion() OrRectRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/OrRectRegion **************************************
*
*   NAME 
*       OrRectRegion -- Perform 2d OR operation of rectangle
*                       with region, leaving result in region.
* 
*   SYNOPSIS 
*       status = OrRectRegion(region,rectangle) 
*         d0                    a0      a1 
*
*	BOOL OrRectRegion( struct Region *, struct Rectangle * );
* 
*   FUNCTION 
*       If any portion of rectangle is not in the region then add
*       that portion to the region.
* 
*   INPUTS 
*       region - pointer to Region structure
*       rectangle - pointer to Rectangle structure 
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
* 
*   BUGS 
* 
*   SEE ALSO
*	AndRectRegion() OrRegionRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/OrRegionRegion ************************************
*
*   NAME 
*       OrRegionRegion -- Perform 2d OR operation of one region
*                       with second region, leaving result in second region 
* 
*   SYNOPSIS 
*       status = OrRegionRegion(region1,region2) 
*         d0                       a0      a1 
*
*	BOOL OrRegionRegion( struct Region *, struct Region * );
* 
*   FUNCTION 
*       If any portion of region1  is not in the region then add
*       that portion to the region2
* 
*   INPUTS 
*       region1 - pointer to Region structure
*       region2 - pointer to Region structure
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
* 
*   BUGS 
*
*   SEE ALSO
* 	OrRectRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/XorRectRegion **************************************
*
*   NAME
*       XorRectRegion -- Perform 2d XOR operation of rectangle
*                       with region, leaving result in region
*
*   SYNOPSIS
*       status = XorRectRegion(region,rectangle)
*         d0                     a0      a1
*
*	BOOL XorRectRegion( struct Region *, struct Rectangle * );
*
*   FUNCTION
*	Add portions of rectangle to region if they are not in
*	the region.
*	Remove portions of rectangle from region if they are
*	in the region.
*
*   INPUTS
*       region - pointer to Region structure
*       rectangle - pointer to Rectangle structure
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
*
*   BUGS
*
*   SEE ALSO
*	OrRegionRegion() AndRegionRegion() graphics/regions.h
*
*********************************************************************


******* graphics.library/NewRegion **************************************
*
*   NAME 
*       NewRegion -- Get an empty region.
* 
*   SYNOPSIS 
*       region = NewRegion()
*	d0
*
*	struct Region *NewRegion();
* 
*   FUNCTION 
*	Create a Region structure, initialize it to empty, and return
*	a pointer it.
*
*   RESULTS
*	region - pointer to initialized region. If it could not allocate
*		required memory region = NULL.
* 
*   INPUTS 
*	none
* 
*   BUGS 
* 
*   SEE ALSO
*	graphics/regions.h
*
*********************************************************************


******* graphics.library/ClearRegion **************************************
*
*   NAME 
*       ClearRegion -- Remove all rectangles from region.
* 
*   SYNOPSIS 
*       ClearRegion(region)
*                     a0
*
*	viod ClearRegion( struct Region * );
* 
*   FUNCTION 
*       Clip away all rectangles in the region leaving nothing.
* 
*   INPUTS 
*       region - pointer to Region structure
* 
*   BUGS 
*
*   SEE ALSO
*	NewRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/DisposeRegion **************************************
*
*   NAME
*       DisposeRegion -- Return all space for this region to free
*			 memory pool.
*
*   SYNOPSIS
*       DisposeRegion(region)
*                      a0 
*
*	void DisposeRegion( struct Region * );
*
*   FUNCTION
*       Free all RegionRectangles for this Region then
*	free the Region itself.
*
*   INPUTS
*       region - pointer to Region structure
*
*   BUGS
*
*   SEE ALSO
*	NewRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/ClearRectRegion **************************************
*
*   NAME
*	ClearRectRegion -- Perform 2d CLEAR operation of rectangle
*			with region, leaving result in region.
*
*   SYNOPSIS
*	status = ClearRectRegion(region,rectangle)
*	 d0	 	 	  a0 	  a1
*
*	BOOL ClearRectRegion(struct Region *, struct Rectangle * );
*
*   FUNCTION
*	Clip away any portion of the region that exists inside
*	of the rectangle. Leave the result in region.
*
*   INPUTS
*	region - pointer to Region structure
*	rectangle - pointer to Rectangle structure
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
*
*   BUGS
*
*   SEE ALSO
*	AndRectRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/AndRegionRegion ************************************
*
*   NAME 
*       AndRegionRegion -- Perform 2d AND operation of one region
*                       with second region, leaving result in second region.
* 
*   SYNOPSIS 
*       status = AndRegionRegion(region1,region2) 
*          d0                       a0      a1 
*
*	BOOL AndregionRegion(struct Region *, struct Region * );
* 
*   FUNCTION 
*       Remove any portion of region2 that is not in region1.
* 
*   INPUTS 
*       region1 - pointer to Region structure
*       region2 - pointer to Region structure to use and for result
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
* 
*   BUGS 
* 
*   SEE ALSO
*	OrRegionRegion() AndRectRegion() graphics/regions.h
*
*********************************************************************

******* graphics.library/XorRegionRegion ************************************
*
*   NAME 
*       XorRegionRegion -- Perform 2d XOR operation of one region
*                       with second region, leaving result in second region 
* 
*   SYNOPSIS 
*       status = XorRegionRegion(region1,region2) 
*         d0                        a0      a1 
*
*	BOOL XorRegionRegion( struct Region *, struct Region * );
* 
*   FUNCTION 
*	Join the regions together. If any part of region1 overlaps
*	region2 then remove that from the new region.
* 
*   INPUTS 
*       region1      = pointer to Region structure
*       region2      = pointer to Region structure
*
*   RESULTS
*	status - return TRUE if successful operation
*		 return FALSE if ran out of memory
* 
*   BUGS 
* 
*
*********************************************************************


    	end   
