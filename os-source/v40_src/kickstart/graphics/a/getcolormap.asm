*******************************************************************************
*
*	$Id: GetColorMap.asm,v 39.12 93/05/05 08:58:09 chrisg Exp $
*
*******************************************************************************

	section	graphics

	IFD	AA_ONLY
	OPT	P=68020
	endc

	xref	_AttachPaletteExtra
	xdef    _GetColorMap
	xdef	_dflt_clrs

******* graphics.library/GetColorMap ******************************************
*
*   NAME
*       GetColorMap -- allocate and initialize Colormap
*
*
*   SYNOPSIS
*       cm = GetColorMap( entries )
*       d0		    d0
*
*	struct ColorMap *GetColorMap( ULONG);
*
*   FUNCTION
*       Allocates, initializes and returns a pointer to a ColorMap
*       data structure, later enabling calls to SetRGB4 
*       and LoadRGB4 to load colors for a view port. The ColorTable
*	pointer in the ColorMap structure points to a hardware
*	specific colormap data structure. You should not count on
*	it being anything you can understand. Use GetRGB4() to
*	query it or SetRGB4CM to set it directly.
*
*   INPUTS
*	entries - number of entries for this colormap 
*
*    RESULT
*	The pointer value returned by this routine, if nonzero,
*       may be stored into the ViewPort.ColorMap pointer.
*       If a value of 0 is returned, the system was unable
*       to allocate enough memory space for the required
*       data structures.
*
*   BUGS
*
*   SEE ALSO
*       SetRGB4() FreeColorMap()
******************************************************************************

	include 'exec/types.i'
	include 'exec/memory.i'
	include	'/view.i'
	include	'graphics/videocontrol.i'
	include	'graphics/displayinfo.i'
	include	'graphics/gfxbase.i'

	xref	_LVOAllocMem
	xref	_LVOFreeColorMap

_GetColorMap:
	movem.l	d7/a2/a3/a6,-(sp)		* save GfxBase, scratch reg
	move.l	gb_ExecBase(a6),a6	* get ExecBase
	move.l  d0,d7			* save count

	moveq	#cm_SIZEOF+4,d0
	sub.l	a2,a2
;	using MEMF_CLEAR now, this will make default Type==0
;	for V1.4 and above we must set Type !=0 to indicate that
;	VideoCommand operations are acceptable for this colormap
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq.s	didnt_get_it
	move.l	d0,a2		* save ColorMap * here for now
	move.l	d0,(a2)+	; store my magic cookie
	move.b	#COLORMAP_TYPE_V39,cm_Type(a2) ; new for V1.4 
	move.w	#16,cm_SpriteBase_Even(a2)
	move.w	#16,cm_SpriteBase_Odd(a2)
	move.w	#8,cm_Bp_1_base(a2)
	move.b	#-1,cm_SpriteResolution(a2)
; now, allocate space (zero filled) for low color bits
	move.l	d7,d0
	add.l	d0,d0
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,cm_LowColorBits(a2)
	beq.s	no_colormap
	move.l	d0,a3

	move.l	d7,d0		* get the count again
	move.w	d0,cm_Count(a2)	* stash size of ColorTable
	add.l	d0,d0		* total number of bytes needed for table
	moveq	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,cm_ColorTable(a2)	* save pointer to ColorTable
	beq.s	no_colormap
	move.l	d0,a0		; save pointer to color data
	move.l	d7,d1		* get count again
	lea	_dflt_clrs(pc),a1	* pointer to default colors
	IFD	AA_ONLY
	moveq	#0,d7
	endc
	bra.s	frog
qwe2
	IFND	AA_ONLY
	move.w	(a1)+,d0		* zero remainder of table
	else
	bfextu	(a1){d7:12},d0
	add.w	#12,d7
	endc
	move.w	d0,(a0)+
	move.w	d0,(a3)+
frog	dbra	d1,qwe2
no_oflow2:
	move.l	#INVALID_ID,cm_VPModeID(a2)
didnt_get_it:
	move.l	a2,d0
	tst.l	d0			* return with condition codes set
	movem.l	(sp)+,d7/a2/a3/a6
	rts

no_colormap:
	move.l	a2,a0
	movem.l	(sp)+,d7/a2/a3/a6
	jsr	_LVOFreeColorMap(a6)	; FreeColorMap is smart now
	clr.l	d0
	rts

_dflt_clrs:
	IFND	AA_ONLY
	dc.w		$0000,$0f00,$00f0,$0ff0,$000f,$0f0f,$00ff,$0fff
	dc.w		$0620,$0e50,$09f1,$0eb0,$055f,$092f,$00f8,$0ccc
	dc.w		$0000,$0111,$0222,$0333,$0444,$0555,$0666,$0777
	dc.w		$0888,$0999,$0aaa,$0bbb,$0ccc,$0ddd,$0eee,$0fff
	dc.w		$0fff,$0888,$0fff,$0ccc,$0444,$0fff,$0fff,$08ff
	dc.w		$0488,$0cff,$06cc,$0244,$0aff,$0eff,$0cff,$0688
	dc.w		$0fff,$09cc,$0344,$0fff,$0fff,$04ff,$0288,$06ff
	dc.w		$03cc,$0144,$05ff,$07ff,$0f8f,$0848,$0fcf,$0c6c
	dc.w		$0424,$0faf,$0fef,$088f,$0448,$0ccf,$066c,$0224
	dc.w		$0aaf,$0eef,$0c8f,$0648,$0fcf,$096c,$0324,$0faf
	dc.w		$0fef,$048f,$0248,$06cf,$036c,$0124,$05af,$07ef
	dc.w		$0fcf,$0868,$0fff,$0c9c,$0434,$0fff,$0fff,$08cf
	dc.w		$0468,$0cff,$069c,$0234,$0aff,$0eff,$0ccf,$0668
	dc.w		$0fff,$099c,$0334,$0fff,$0fff,$04cf,$0268,$06ff
	dc.w		$039c,$0134,$05ff,$07ff,$0f4f,$0828,$0f6f,$0c3c
	dc.w		$0414,$0f5f,$0f7f,$084f,$0428,$0c6f,$063c,$0214
	dc.w		$0a5f,$0e7f,$0c4f,$0628,$0f6f,$093c,$0314,$0f5f
	dc.w		$0f7f,$044f,$0228,$066f,$033c,$0114,$055f,$077f
	dc.w		$0ff8,$0884,$0ffc,$0cc6,$0442,$0ffa,$0ffe,$08f8
	dc.w		$0484,$0cfc,$06c6,$0242,$0afa,$0efe,$0cf8,$0684
	dc.w		$0ffc,$09c6,$0342,$0ffa,$0ffe,$04f8,$0284,$06fc
	dc.w		$03c6,$0142,$05fa,$07fe,$0f88,$0844,$0fcc,$0c66
	dc.w		$0422,$0faa,$0fee,$0888,$0444,$0ccc,$0666,$0222
	dc.w		$0aaa,$0eee,$0c88,$0644,$0fcc,$0966,$0322,$0faa
	dc.w		$0fee,$0488,$0244,$06cc,$0366,$0122,$05aa,$07ee
	dc.w		$0fc8,$0864,$0ffc,$0c96,$0432,$0ffa,$0ffe,$08c8
	dc.w		$0464,$0cfc,$0696,$0232,$0afa,$0efe,$0cc8,$0664
	dc.w		$0ffc,$0996,$0332,$0ffa,$0ffe,$04c8,$0264,$06fc
	dc.w		$0396,$0132,$05fa,$07fe,$0f48,$0824,$0f6c,$0c36
	dc.w		$0412,$0f5a,$0f7e,$0848,$0424,$0c6c,$0636,$0212
	dc.w		$0a5a,$0e7e,$0c48,$0624,$0f6c,$0936,$0312,$0f5a
	dc.w		$0f7e,$0448,$0224,$066c,$0336,$0112,$055a,$077e
	else

	dc.b		$00,$0f,$00,$0f,$0f,$f0,$00,$ff,$0f,$0f,$ff,$ff
	dc.b		$62,$0e,$50,$9f,$1e,$b0,$55,$f9,$2f,$0f,$8c,$cc
	dc.b		$00,$01,$11,$22,$23,$33,$44,$45,$55,$66,$67,$77
	dc.b		$88,$89,$99,$aa,$ab,$bb,$cc,$cd,$dd,$ee,$ef,$ff
	dc.b		$ff,$f8,$88,$ff,$fc,$cc,$44,$4f,$ff,$ff,$f8,$ff
	dc.b		$48,$8c,$ff,$6c,$c2,$44,$af,$fe,$ff,$cf,$f6,$88
	dc.b		$ff,$f9,$cc,$34,$4f,$ff,$ff,$f4,$ff,$28,$86,$ff
	dc.b		$3c,$c1,$44,$5f,$f7,$ff,$f8,$f8,$48,$fc,$fc,$6c
	dc.b		$42,$4f,$af,$fe,$f8,$8f,$44,$8c,$cf,$66,$c2,$24
	dc.b		$aa,$fe,$ef,$c8,$f6,$48,$fc,$f9,$6c,$32,$4f,$af
	dc.b		$fe,$f4,$8f,$24,$86,$cf,$36,$c1,$24,$5a,$f7,$ef
	dc.b		$fc,$f8,$68,$ff,$fc,$9c,$43,$4f,$ff,$ff,$f8,$cf
	dc.b		$46,$8c,$ff,$69,$c2,$34,$af,$fe,$ff,$cc,$f6,$68
	dc.b		$ff,$f9,$9c,$33,$4f,$ff,$ff,$f4,$cf,$26,$86,$ff
	dc.b		$39,$c1,$34,$5f,$f7,$ff,$f4,$f8,$28,$f6,$fc,$3c
	dc.b		$41,$4f,$5f,$f7,$f8,$4f,$42,$8c,$6f,$63,$c2,$14
	dc.b		$a5,$fe,$7f,$c4,$f6,$28,$f6,$f9,$3c,$31,$4f,$5f
	dc.b		$f7,$f4,$4f,$22,$86,$6f,$33,$c1,$14,$55,$f7,$7f
	dc.b		$ff,$88,$84,$ff,$cc,$c6,$44,$2f,$fa,$ff,$e8,$f8
	dc.b		$48,$4c,$fc,$6c,$62,$42,$af,$ae,$fe,$cf,$86,$84
	dc.b		$ff,$c9,$c6,$34,$2f,$fa,$ff,$e4,$f8,$28,$46,$fc
	dc.b		$3c,$61,$42,$5f,$a7,$fe,$f8,$88,$44,$fc,$cc,$66
	dc.b		$42,$2f,$aa,$fe,$e8,$88,$44,$4c,$cc,$66,$62,$22
	dc.b		$aa,$ae,$ee,$c8,$86,$44,$fc,$c9,$66,$32,$2f,$aa
	dc.b		$fe,$e4,$88,$24,$46,$cc,$36,$61,$22,$5a,$a7,$ee
	dc.b		$fc,$88,$64,$ff,$cc,$96,$43,$2f,$fa,$ff,$e8,$c8
	dc.b		$46,$4c,$fc,$69,$62,$32,$af,$ae,$fe,$cc,$86,$64
	dc.b		$ff,$c9,$96,$33,$2f,$fa,$ff,$e4,$c8,$26,$46,$fc
	dc.b		$39,$61,$32,$5f,$a7,$fe,$f4,$88,$24,$f6,$cc,$36
	dc.b		$41,$2f,$5a,$f7,$e8,$48,$42,$4c,$6c,$63,$62,$12
	dc.b		$a5,$ae,$7e,$c4,$86,$24,$f6,$c9,$36,$31,$2f,$5a
	dc.b		$f7,$e4,$48,$22,$46,$6c,$33,$61,$12,$55,$a7,$7e

	endc



	end
