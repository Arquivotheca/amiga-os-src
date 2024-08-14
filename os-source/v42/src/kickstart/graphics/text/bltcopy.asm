
	TTL    '$Id: bltcopy.asm,v 42.1 93/07/20 13:51:58 chrisg Exp $'
**********************************************************************
*
*		-------------
*		GRAPHICS TEXT	rectangular blit guts
*		-------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control: $Locker:  $
*
*   $Log:	bltcopy.asm,v $
;; Revision 42.1  93/07/20  13:51:58  chrisg
;; mucho chunky support.
;; 
;; Revision 42.0  1993/06/16  11:24:57  chrisg
;; initial
;;
* Revision 42.0  1993/06/01  07:22:18  chrisg
* *** empty log message ***
*
*   Revision 39.8  92/10/20  08:56:08  chrisg
*   preserve d7.
*   
*   Revision 39.7  92/09/03  15:38:40  spence
*   Autodoc spelling corrections
*   
*   Revision 39.6  92/03/19  16:28:27  chrisg
*   short branches.
*   
*   Revision 39.5  92/02/26  11:34:48  chrisg
*    fxied for new interleave scheme.
*   
*   Revision 39.4  91/11/22  15:05:29  chrisg
*    put depth check back in.
*   
*   Revision 39.3  91/11/21  10:39:44  chrisg
*    improved to handle bltbitmap with minterm of 0 ( no source)
*   
*   Revision 39.2  91/11/19  10:34:50  chrisg
*     updated because of new magic cookie for interleaved bitmaps.
*   
*   Revision 39.1  91/11/15  11:04:29  chrisg
*    added support for interleaved bitmaps!
*   
*   Revision 39.0  91/08/21  17:35:29  chrisg
*   Bumped
*   
*   Revision 37.3  91/05/09  14:47:43  spence
*   *** empty log message ***
*   
*   Revision 37.2  91/02/12  15:51:11  spence
*   autodoc
*   
*   Revision 37.1  91/01/18  17:21:59  spence
*   Checks for NULL from getmustmem(), which no longer gurus.
*   Makes a quick exit if NULL found. This may cause guru further up the
*   calling chain - I hope not.
*   
*   Revision 37.0  91/01/07  15:28:41  spence
*   initial switchover from V36
*   
*   Revision 36.10  90/05/10  10:11:27  kodiak
*   optimize lea bm_Planes(an),an to addq.l #bm_Planes,an
*   
*   Revision 36.9  90/04/24  10:46:38  kodiak
*   document blitter usage
*   
*   Revision 36.8  90/04/13  11:52:12  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.7  90/04/02  12:58:56  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.6  90/03/05  16:23:45  kodiak
*   fix superbitmap BltTemplate (and thus Text)
*   
*   Revision 36.5  89/11/02  18:37:36  kodiak
*   recode to use gb_ExecBase
*   
*   Revision 36.4  89/08/16  12:58:24  kodiak
*   document 0/-1 plane pointer hack
*   
*   Revision 36.3  89/07/18  15:50:04  kodiak
*   special case for zero & $ffffffff plane pointers
*   
*   Revision 36.2  89/07/17  15:03:55  kodiak
*   waitblitdone after OwnBlitter for BltTemplate left or right cases
*   
*   Revision 36.1  89/06/08  14:22:42  kodiak
*   define out big blit emulation
*   define EMULATE to put it back in
*   
*   Revision 36.0  89/06/05  14:15:14  kodiak
*   autodoc changes
*   
*   Revision 35.4  87/12/03  12:59:03  kodiak
*   emulate big blits on small blitters
*   
*
**********************************************************************

	SECTION		graphics

	OPTIMON

	OPT	P=68020

*------ Included Files -----------------------------------------------

	include	"exec/types.i"
	include	"exec/nodes.i"
	include	"exec/lists.i"
	include	"exec/ports.i"
	include	"exec/memory.i"
	include	"exec/libraries.i"
	include	"exec/alerts.i"

	include	"/gfx.i"
	include	"graphics/clip.i"
	include	"/rastport.i"
	include	"/gfxbase.i"
	include	"/bitmap_internal.i"

	include	"hardware/blit.i"
	include	"hardware/custom.i"

	include	"macros.i"
	include	"/macros.i"

	IFNE		rp_Layer
	FAIL		"recode rp_Layer instructions"
	ENDC

*------ Imported Names -----------------------------------------------

	XREF		_custom

	XREF		GetMustMem
	XREF		FreeMustMem

	XREF		waitblitdone

	XLVO	UnlockLayerRom		; Graphics
	XLVO	LockLayerRom		;
	XLVO	OwnBlitter		;
	XLVO	DisownBlitter		;


*------ Exported Names -----------------------------------------------

*------ Tables -------------------------------------------------------

	XDEF		_fwmaskTable
	XDEF		_lwmaskTable

*------ Functions ----------------------------------------------------

	XDEF		_BltBitMap
	XDEF		_BltTemplate

	IFNE	BC1F_DESC-2
	FAIL	"recode BC1B_DESC bit tests"
	ENDC

BC1B_DESC	EQU	2

 STRUCTURE	SHADOW,0
    UWORD	SHADOW_CON0
    UWORD	SHADOW_CON1
    UWORD	SHADOW_AFWM
    UWORD	SHADOW_ALWM
    UWORD	SHADOW_AMOD
    UWORD	SHADOW_BMOD
    UWORD	SHADOW_CDMOD
    APTR	SHADOW_APT
    LABEL	SHADOW_SIZEOF
	
	IFNE	SHADOW_CON0
	FAIL	"SHADOW_CON0 not zero: recode"
	ENDC

	IFNE	SHADOW_CON1-2
	FAIL	"SHADOW_CON1 not immediately after SHADOW_CON0: recode"
	ENDC

_lwmaskTable:
	dc.w    $08000
	dc.w	$0C000
	dc.w	$0E000
	dc.w	$0F000
	dc.w	$0F800
	dc.w	$0FC00
	dc.w	$0FE00
	dc.w	$0FF00
	dc.w	$0FF80
	dc.w	$0FFC0
	dc.w	$0FFE0
	dc.w	$0FFF0
	dc.w	$0FFF8
	dc.w	$0FFFC
	dc.w	$0FFFE
_fwmaskTable:
	dc.w    $0FFFF
	dc.w	$07FFF
	dc.w	$03FFF
	dc.w	$01FFF
	dc.w	$00FFF
	dc.w	$007FF
	dc.w	$003FF
	dc.w	$001FF
	dc.w	$000FF
	dc.w	$0007F
	dc.w	$0003F
	dc.w	$0001F
	dc.w	$0000F
	dc.w	$00007
	dc.w	$00003
	dc.w	$00001


******* graphics.library/BltBitMap ***********************************
*
*   NAME
*	BltBitMap -- Move a rectangular region of bits in a BitMap.
*
*   SYNOPSIS
*	planecnt = BltBitMap(SrcBitMap, SrcX, SrcY, DstBitMap,
*	D0                   A0         D0:16 D1:16 A1
*	    DstX, DstY, SizeX, SizeY, Minterm, Mask [, TempA])
*	    D2:16 D3:16 D4:16  D5:16  D6:8     D7:8   [A2]
*
*	ULONG BltBitMap(struct BitMap *, WORD, WORD, struct BitMap *,
*	    WORD, WORD, WORD, WORD, UBYTE, UBYTE, UWORD *);
*
*   FUNCTION
*	Perform non-destructive blits to move a rectangle from one
*	area in a BitMap to another area, which can be on a different
*	BitMap.
*	This blit is assumed to be friendly: no error conditions (e.g.
*	a rectangle outside the BitMap bounds) are tested or reported.
*
*   INPUTS
*	SrcBitMap, DstBitMap - the BitMap(s) containing the
*	      rectangles
*	    - the planes copied from the source to the destination are
*	      only those whose plane numbers are identical and less
*	      than the minimum Depth of either BitMap and whose Mask
*	      bit for that plane is non-zero.
*	    - as a special case, if a plane pointer in the SrcBitMap
*	      is zero, it acts as a pointer to a plane of all zeros, and
*	      if the plane pointer is 0xffffffff, it acts as a pointer
*	      to a plane of all ones.  (Note: new for V36)
*	    - SrcBitMap and DstBitMap can be identical if they point
*	      to actual planes.
*	SrcX, SrcY - the x and y coordinates of the upper left corner
*	    of the source rectangle.  Valid range is positive
*	    signed integer such that the raster word's offset
*	    0..(32767-Size)
*	DstX, DstY - the x and y coordinates of the upper left
*	    corner of the destination for the rectangle.  Valid
*	    range is as for Src.
*	SizeX, SizeY - the size of the rectangle to be moved.  Valid
*	    range is (X: 1..976; Y: 1..1023 such that final raster
*	    word's offset is 0..32767)
*	Minterm - the logic function to apply to the rectangle when
*	    A is non-zero (i.e. within the rectangle).  B is the
*	    source rectangle and C, D is the destination for the
*	    rectangle.
*	    - $0C0 is a vanilla copy
*	    - $030 inverts the source before the copy
*	    - $050 ignores the source and inverts the destination
*	    - see the hardware reference manual for other combinations
*	Mask - the write mask to apply to this operation.  Bits set 
*	    indicate the corresponding planes (if not greater than
*	    the minimum plane count) are to participate in the
*	    operation.  Typically this is set to 0xff.
*	TempA - If the copy overlaps exactly to the left or right
*	    (i.e. the scan line addresses overlap), and TempA is
*	    non-zero, it points to enough chip accessible memory
*	    to hold a line of A source for the blit (ie CHIP RAM).
*	    BltBitMap will allocate (and free) the needed TempA if
*	    none is provided and one is needed.  Blit overlap is
*	    determined from the relation of the first non-masked
*	    planes in the source and destination bit maps.
*
*   RESULTS
*	planecnt - the number of planes actually involved in the blit.
*
*   NOTES
*	o   This function may use the blitter.
*
*   SEE ALSO
*	ClipBlit()  graphics/gfx.h  hardware/blit.h
*
**********************************************************************

_BltBitMap:
	cmp.w	#UNLIKELY_WORD,bm_Pad(a1)
	bne	old_entry

	btst	#IBMB_CHUNKY,bm_Flags(a1)
	bne	is_chunky_destination

	btst	#IBMB_INTERLEAVED,bm_Flags(a1)
	bne	old_entry
	cmp.b	#-1,d7
	bne	old_entry
	move.b	bm_Depth(a1),d7
	tst.b	d6	; minterm=0?
	beq.s	no_ck_2nd

	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
	bne	old_entry_1
	btst	#IBMB_INTERLEAVED,bm_Flags(a0)
	bne.s	old_entry_1
	cmp.b	bm_Depth(a0),d7
	bne.s	old_entry_1
no_ck_2nd:
	movem.l	d1/d3/d5/d7/a2,-(a7)
	lea	-bm_SIZEOF*2(a7),a7
	tst.b	d6
	beq.s	no_ck_2nd_1
	move.l	bm_Planes+4(a0),a2
	sub.l	bm_Planes(a0),a2
	cmp.w	#0,a2
	beq.s	old_1
	cmp.w	bm_BytesPerRow(a0),a2
	bhi.s	old_1
no_ck_2nd_1:
	move.l	bm_Planes+4(a1),a2
	sub.l	bm_Planes(a1),a2
	cmp.w	bm_BytesPerRow(a1),a2
	bhi.s	old_1

	ext.w	d7
	mulu	d7,d3	; dsty*=depth
	mulu	d7,d5	; high*=depth
	mulu	d7,d1	; srcy*=depth
	move.b	#1,bm_Depth(a7)
	move.b	#1,bm_SIZEOF+bm_Depth(a7)
	move.l	bm_Planes+4(a0),a2
	sub.l	bm_Planes(a0),a2
	move.w	a2,bm_BytesPerRow(a7)
	move.l	bm_Planes+4(a1),a2
	sub.l	bm_Planes(a1),a2
	move.w	a2,bm_SIZEOF+bm_BytesPerRow(a7)
	move.l	bm_Planes(a0),bm_Planes(a7)
	move.l	bm_Planes(a1),bm_SIZEOF+bm_Planes(a7)
	move.l	a7,a0
	lea	bm_SIZEOF(a7),a1
old_1:	st	d7
	sub.l	a2,a2									; don't use temp
	bsr.s	old_entry
	lea	bm_SIZEOF*2(a7),a7
	movem.l	(a7)+,d1/d3/d5/d7/a2
	st	d7
	rts

old_entry_1:
	st	d7
old_entry:
	movem.l d2-d7/a0-a5,-(a7)
	sub.w	#SHADOW_SIZEOF,a7
	move.l	a7,a5
PARM_MINTERM	EQU	SHADOW_SIZEOF+16	; then MASK, SRCBMAP, DESTBMAP

	    ;-- get lowest plane count
	moveq	#0,d6
	move.b	bm_Depth(a0),d6
	beq.s	noPlanes
	cmp.b	bm_Depth(a1),d6
	ble.s	countHarmony
	move.b	bm_Depth(a1),d6
	beq.s	noPlanes
countHarmony:
	move.w	d6,-(a7)
	    ;-- set up blitter
						; d0-d5 from BltBitMap args
	move.l	a2,SHADOW_APT(a5)	; TmpRas
	move.w	bm_BytesPerRow(a0),a2	; srcModulo
	move.w	bm_BytesPerRow(a1),a3	; destModulo
		;-- get unmasked srcPlane, destPlane
findUnMaskedPlane:
	subq	#1,d6
	bmi.s	noPlanes2		; no unmasked planes
	btst	d6,d7			; check this plane's mask
	beq.s	findUnMaskedPlane
	lsl.w	#2,d6			; get plane index
	move.l	bm_Planes(a0,d6.w),a0
	move.l	bm_Planes(a1,d6.w),a1
						; a6 from BltBitMap args

	bsr	bltSetup

	    ;-- recover BltBitMap arguments
	movem.l	PARM_MINTERM(a5),d2/d7/a0-a1
	addq.l	#bm_Planes,a0		; srcPlane
	addq.l	#bm_Planes,a1		; destPlane
		;-- adjust CON0
	andi.w	#$F0,d2			; mask minterm to 4:4
	or.w	d2,(a5)			; SHADOW_CON0

	    ;-- set up for plane loops
	moveq	#0,d5			; actual plane count
	move.w	(a7)+,d6		; recover plane count

	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	bigBBM
	
	IFD	EMULATE
		;-- check if blit will fit wholly in old chip bltsize
	move.l	d0,d2
	and.l	#$ffc0fc00,d2		; check for less than 1Kx1K
	beq.s	smallBBM
	cmp.w	#$0400,d0		; check Y vs. 1K
	bgt.s	bbmEmulate
	move.l	d0,d2
	swap	d2
	cmp.w	#$0040,d2		; check X vs. 1K
	bgt.s	bbmEmulate
	ENDC	; EMULATE

smallBBM:

		;-- do a blit with just the bltsize register
	move.w	d0,d2			; cache Y
	lsl.w	#6,d2			; shift up Y
	swap	d0			; get X
	and.w	#$003f,d0		; fix 1K to zero
	or.w	d2,d0			; form bltsize register
	bra.s	planesDBF

bigBBM:
	move.w	d0,bltsizv(a4)
	swap	d0
	bra	planebDBF

	    ;-- return quickly when there is no work to do
noPlanes2:
	addq	#2,a7			; pop.w d6
noPlanes:
	moveq	#0,d0			; no planes actually blitted
	bra	bbmRts

	IFD	EMULATE
bbmEmulate:
	sub.w	#EMUL_SIZEOF,a7
	move.l	d1,EMUL_SOFFSET(a7)
	move.l	d3,EMUL_DOFFSET(a7)
	    ;-- calculate counters
		;-- COUNTY1
	move.w	d0,d1
	moveq	#10,d2
	lsr.w	d2,d1
	move.w	d0,d2
	and.w	#$3ff,d2
	bne.s	bbmeYCount
	subq	#1,d1
bbmeYCount:
	move.w	d1,EMUL_COUNTY1(a7)
		;-- COUNTX1
	swap	d0
	move.w	d0,d1
	lsr.w	#6,d1
	move.w	d0,d2
	and.w	#$3f,d2
	bne.s	bbmeXCount
	subq	#1,d1
	move.w	#$40,d2
bbmeXCount:
	move.w	d1,EMUL_COUNTX1(a7)
	move.w	d1,d4		; save for mod fixup
		;-- SIZE22
	move.w	d0,d1
	move.w	d1,d3		; save for mod fixup
	and.w	#$3f,d1
	swap	d0
	move.w	d0,d2
	lsl.w	#6,d2
	or.w	d2,d1
	move.w	d1,EMUL_SIZE22(a7)
	    ;-- calculate new modulos
	sub.w	#$40,d3		; size sans intermediate
	add.w	d3,d3
	lsl.w	#7,d4		; size sans last
		;-- A modulo
	move.w	SHADOW_AMOD(a5),d0
	move.w	d0,d1
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXAModDesc
	add.w	d3,d0
	add.w	d4,d1
	bra.s	bbmeXAMod
bbmeXAModDesc:
	sub.w	d3,d0
	sub.w	d4,d1
bbmeXAMod:
	move.w	d0,EMUL_AMOD1(a7)
	move.w	d1,EMUL_AMOD2(a7)
		;-- B modulo
	move.w	SHADOW_BMOD(a5),d0
	move.w	d0,d1
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXBModDesc
	add.w	d3,d0
	add.w	d4,d1
	bra.s	bbmeXBMod
bbmeXBModDesc:
	sub.w	d3,d0
	sub.w	d4,d1
bbmeXBMod:
	move.w	d0,EMUL_BMOD1(a7)
	move.w	d1,EMUL_BMOD2(a7)
		;-- C & D modulos
	move.w	SHADOW_CDMOD(a5),d0
	move.w	d0,d1
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXCDModDesc
	add.w	d3,d0
	add.w	d4,d1
	bra.s	bbmeXCDMod
bbmeXCDModDesc:
	sub.w	d3,d0
	sub.w	d4,d1
bbmeXCDMod:
	move.w	d0,EMUL_CDMOD1(a7)
	move.w	d1,EMUL_CDMOD2(a7)
	bra	bbmeDBF
	    ;--	cycle through planes and patch together small blits
bbmeLoop:
	move.l	(a0)+,a2
	move.l	(a1)+,a3
		;-- check for masked plane
	lsr.b	#1,d7
	bcc	bbmeBumpPlane
		;-- adjust planes by offsets
	add.l	EMUL_SOFFSET(a7),a2
	add.l	EMUL_DOFFSET(a7),a3
	move.l	SHADOW_APT(a5),d0
	move.l	d0,EMUL_POINTER(a7)
	bsr	waitblitdone
	move.l	d0,bltapt(a4)
	move.l	a2,bltbpt(a4)
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
	move.w	EMUL_COUNTX1(a7),d3
	move.w	EMUL_AMOD1(a7),bltamod(a4)
	move.w	EMUL_BMOD1(a7),bltbmod(a4)
	move.w	EMUL_CDMOD1(a7),bltcmod(a4)
	move.w	EMUL_CDMOD1(a7),bltdmod(a4)
	moveq	#-1,d1
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXHeadMaskDesc
	move.w	d1,bltalwm(a4)
	bra.s	bbmeXDBF
bbmeXHeadMaskDesc:
	move.w	d1,bltafwm(a4)
	bra.s	bbmeXDBF
	    ;-- loop through intermediate x
bbmeXLoop:
	move.w	EMUL_COUNTY1(a7),d2
	bra.s	bbmeY1DBF
	    ;-- loop through intermediate x intermediate y
bbmeY1Loop:
	bsr	waitblitdone
	move.w	#0,bltsize(a4)
bbmeY1DBF:
	dbf	d2,bbmeY1Loop
	    ;-- intermediate x last y
	move.w	EMUL_SIZE22(a7),d0
	and.w	#$ffc0,d0
	bsr	waitblitdone
	move.w	d0,bltsize(a4)
		;-- ensure fwm/lwm are both $ffff for middle
	bsr	waitblitdone
	moveq	#-1,d1
	move.l	d1,bltafwm(a4)		; and bltalwm
		;-- advance pointers along in x
	move.l	EMUL_POINTER(a7),d0
	move.l	#$80,d1
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXAdvanceDesc
	add.l	d1,d0
	add.w	d1,a2
	add.w	d1,a3
	bra.s	bbmeXAdvance
bbmeXAdvanceDesc:
	sub.l	d1,d0
	sub.w	d1,a2
	sub.w	d1,a3
bbmeXAdvance:
	move.l	d0,EMUL_POINTER(a7)
	move.l	d0,bltapt(a4)
	move.l	a2,bltbpt(a4)
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
bbmeXDBF:
	dbf	d3,bbmeXLoop
	    ;-- last x
	move.w	EMUL_COUNTY1(a7),d2
	move.w	EMUL_AMOD2(a7),bltamod(a4)
	move.w	EMUL_BMOD2(a7),bltbmod(a4)
	move.w	EMUL_CDMOD2(a7),bltcmod(a4)
	move.w	EMUL_CDMOD2(a7),bltdmod(a4)
	btst	#BC1B_DESC,SHADOW_CON1+1(a5)
	bne.s	bbmeXTailDesc
	move.w	SHADOW_ALWM(a5),bltalwm(a4)
	bra.s	bbmeY2DBF
bbmeXTailDesc:
	move.w	SHADOW_AFWM(a5),bltafwm(a4)
	bra.s	bbmeY2DBF
	    ;-- loop through last x intermediate y
bbmeY2Loop:
	move.w	EMUL_SIZE22(a7),d0
	and.w	#$003f,d0
	bsr	waitblitdone
	move.w	d0,bltsize(a4)
bbmeY2DBF:
	dbf	d2,bbmeY2Loop
	    ;-- last x last y
	bsr	waitblitdone
	move.w	EMUL_SIZE22(a7),bltsize(a4)

bbmeBumpPlane:
	addq.w	#1,d5
bbmeDBF:
	dbf	d6,bbmeLoop
	add.w	#EMUL_SIZEOF,a7
	bra.s	releaseBlitter

	ENDC	; EMULATE


	    ;--	cycle through small planes
planesLoop:
	move.l	(a0)+,a2
	move.l	(a1)+,a3
		;-- check for masked plane
	lsr.l	#1,d7
	bcc.s	planesDBF
	bsr	waitblitdone
		;-- check for special source
	move.l	a2,d2
	beq.s	planesSolid
	addq.l	#1,d2		; cmp.l	#-1,d2
	beq.s	planesSolid
	move.l	(a5),bltcon0(a4)	; SHADOW_CON0, SHADOW_CON1
		;-- adjust planes by offsets
	add.l	d1,a2
	move.l	a2,bltbpt(a4)
planesBlit:
	add.l	d3,a3
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
	move.w	d0,bltsize(a4)
	addq.w	#1,d5

planesDBF:
	dbf	d6,planesLoop
	bra.s	releaseBlitter


planesSolid:
	move.l	(a5),d2			; SHADOW_CON0, SHADOW_CON1
	and.l	#((~SRCB)<<16)!$0fff,d2
	move.l	d2,bltcon0(a4)		; , bltcon1
	move.w	a2,bltbdat(a4)
	bra.s	planesBlit

planebSolid:
	move.l	(a5),d2			; SHADOW_CON0, SHADOW_CON1
	and.l	#((~SRCB)<<16)!$0fff,d2
	move.l	d2,bltcon0(a4)		; , bltcon1
	move.w	a2,bltbdat(a4)
	bra.s	planebBlit


	    ;--	cycle through big planes
planebLoop:
	move.l	(a0)+,a2
	move.l	(a1)+,a3
		;-- check for masked plane
	lsr.l	#1,d7
	bcc.s	planebDBF
	bsr	waitblitdone
		;-- check for special source
	move.l	a2,d2
	beq.s	planebSolid
	addq.l	#1,d2		; cmp.l	#-1,d2
	beq.s	planebSolid
	move.l	(a5),bltcon0(a4)	; SHADOW_CON0, SHADOW_CON1
		;-- adjust planes by offsets
	add.l	d1,a2
	move.l	a2,bltbpt(a4)
planebBlit:
	add.l	d3,a3
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
	move.w	d0,bltsizh(a4)
	addq.w	#1,d5

planebDBF:
	dbf	d6,planebLoop

releaseBlitter:
	    ;-- release the blitter
	CALLLVO DisownBlitter
	    ;-- release allocated A data, if necessary
		;-- check if A data used
	move.l	d4,d0
	beq.s	donePlanes
		;-- free allocated A data
	bsr	waitblitdone
	move.l	SHADOW_APT(a5),a1
	bsr	FreeMustMem

donePlanes:
	    ;-- return actual plane count
	move.l	d5,d0
bbmRts:
	add.w	#SHADOW_SIZEOF,a7
	movem.l (a7)+,d2-d7/a0-a5
	rts


;------ bltSetup -----------------------------------------------------
;
;   NAME
;	bltSetup
;
;   INPUTS
;	d0.w	srcX
;	d1.w	srcY		then .l srcWordOffset
;	d2.w	destX
;	d3.w	destY		then .l destWordOffset
;	d4.w	sizeX
;	d5.w	sizeY
;
;	a0.l	srcPlane
;	a1.l	destPlane
;	a2.w	srcModulo
;	a3.w	destModulo
;	a5.l	SHADOW
;	a6.l	GfxBase
;	a7.l	4 bytes return address,
;		then 6 bytes for bltafwm, bltalwm, and decending flag
;
;   OUTPUTS
;	d0	.w bltsize or .l bltsizh/bltsizv (horizontal in high word)
;	d1.l	srcWordOffset
;	d2.w	bltcon0 w/o minterm
;	d3.l	destWordOffset
;	d4	A data size allocated here
;	d5	destroyed
;	d6	destroyed
;	d7	destroyed
;
;	a0	destroyed
;	a1	destroyed
;	a2	destroyed
;	a3	destroyed
;	a4.l	_custom
;	a5	SHADOW
;		    SHADOW_CON0
;		    SHADOW_CON1
;		    SHADOW_AFWM
;		    SHADOW_ALWM
;		    SHADOW_AMOD
;		    SHADOW_BMOD
;		    SHADOW_CDMOD
;		    SHADOW_APT
;	a6	preserved
;
bltSetup:
	;-- find initial plane offsets
	    ;-- word offsets
	move.w	a2,d7		; for source
	muls	d7,d1
	move.w	d0,d7
	asr.w	#4,d7
	add.w	d7,d7
	ext.l	d7
	add.l	d7,d1
	move.w	a3,d7		; for destination
	muls	d7,d3
	move.w	d2,d7
	asr.w	#4,d7
	add.w	d7,d7
	ext.l	d7
	add.l	d7,d3

	    ;-- bit offsets
	and.w	#$0f,d0		; for source
	and.w	#$0f,d2		; for destination

	;-- determine the blit direction & modulo
	    ;-- apply equal modulo criterion for overlapping blits
	cmpa.w	a2,a3
	beq.s	mightOverlap

	    ;-- simply look at shift direction: modulo cannot be "-2"
	cmp.w	d0,d2
	blt	increasingReverse
	bra	decreasingForward

mightOverlap:
	    ;-- comparing addresses
		;-- word addresses
	adda.l	d1,a0
	adda.l	d3,a1
	cmp.l	a0,a1
	bge	decreasing
		;-- bit offsets
	cmp.w	d0,d2
	blt.s	increasingScanTest

	    ;------ forward blit, positive modulo
	bsr	forwardCore

	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	sub.w	d6,a2		; adjust srcModulo
	move.w	a2,SHADOW_BMOD(a5)
	move.w	a2,bltbmod(a4)
	sub.w	d6,a3		; adjust destModulo
	move.w	a3,SHADOW_CDMOD(a5)
	move.w	a3,bltcmod(a4)
	move.w	a3,bltdmod(a4)
	rts
	    ;==========


	    ;-- test if in the same scan line
increasingScanTest:
	move.w	d2,d7		; get destX bit offset
	add.w	d4,d7		; add sizeX
	subq.w	#1,d7		; produce final x on dest line
	lsr.w	#4,d7		; make word offset
	add.w	d7,d7		; make byte offset
	ext.l	d7
	add.l	a1,d7		; make dest final x line address
	cmp.l	a0,d7		; compare with source address
	blt	increasingReverse

	    ;------ forward blit, positive modulo, with A source

	    ;-- calculate blit size
	move.w	d2,d6		; destX bit offset
	add.w	d4,d6		;   + sizeX
	subq.w	#1,d6		;   - 1
	move.w	d6,-(a7)	; save dest + size - 1
	lsr.w	#4,d6		; number of words to blit - 2
	    ;-- get A data
	moveq	#0,d4
	move.l	SHADOW_APT(a5),d7	; check for TempA
	bne.s	gotForwardA
	move.w	d6,d4		; number of words to blit - 2
	add.w	d4,d4		; number of bytes to blit - 4
	addq	#4,d4		; number of bytes to blit
	movem.l	d0-d1,-(a7)
	move.l	d4,d0
	moveq	#MEMF_CHIP,d1
	bsr	GetMustMem
	move.l	d0,SHADOW_APT(a5)
	move.l	d0,d7
	movem.l	(a7)+,d0-d1
	beq		increasingScanTest.	; spence - Jan 18 1991

gotForwardA:
	    ;-- build A data
		;-- grab the blitter
	CALLLVO	OwnBlitter	; (before the waitblitdone)

	move.l	d7,a1		; recover A data memory
		;-- ensure A data not in use
	bsr	waitblitdone
		;-- first two words in A data
	clr.w	(a1)+			; clear first word
	move.w	d2,d7			; using dest x bit...
	add.w	d7,d7			; 
	lea	_fwmaskTable(pc),a0	; get
	move.w	0(a0,d7.W),(a1)+	;   second word like a fwmask
		;-- set intermediate words in A data
	moveq	#-1,d7
	bra.s	midForwardA
initForwardA:
	move.w	d7,(a1)+
midForwardA:
	dbf	d6,initForwardA

	lea	_custom,a4
		;-- set adata
	move.l	SHADOW_APT(a5),bltapt(a4)
		;-- set hardware fwmask and lwmask
	move.l	d7,SHADOW_AFWM(a5)
	move.l	d7,bltafwm(a4)
		;-- set last word in A data
	move.w	(a7)+,d6	; recover dest + size - 1
	move.w	d6,d7
	and.w	#$0F,d7
	add.w	d7,d7
	lea	_lwmaskTable(pc),a0
	move.w	0(a0,d7.w),d7	; last word like lwmask
	and.w	d7,-2(a1)	;   which can overlap fwmask
	    ;-- get B shift into con1
	move.w	d2,d7		; get dest x bit
	sub.w	d0,d7		; subtract src x bit
	and.w	#$0F,d7		; "add 16" to the negative result
	ror.w	#4,d7
	move.w	d7,SHADOW_CON1(a5)
	move.w	d7,bltcon1(a4)
	    ;-- partial con0 (minterm and A shift)
	move.w	#NABC+NANBC+SRCA+SRCB+SRCC+DEST,(a5)	; SHADOW_CON0
	    ;-- blit size into d0
	lsr.w	#4,d6		; number of words to blit - 2
	addq.w	#2,d6		; blit word width (1 extra word)
	move.w	d6,d0
	swap	d0
	move.w	d5,d0

	    ;-- adjust start address offsets
	subq.l	#2,d3

	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	sub.w	d6,a2		; adjust srcModulo
	move.w	a2,SHADOW_BMOD(a5)
	move.w	a2,bltbmod(a4)
	sub.w	d6,a3		; adjust destModulo
	move.w	a3,SHADOW_CDMOD(a5)
	move.w	a3,bltcmod(a4)
	move.w	a3,bltdmod(a4)
	neg.w	d6
	move.w	d6,SHADOW_AMOD(a5)
	move.w	d6,bltamod(a4)
increasingScanTest.:
	rts
	    ;==========


	;------ reverse blit, negative modulo
increasingReverse:
	bsr	reverseCore

	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	move.w	d6,d7
	add.w	a2,d6		; adjust srcModulo
	neg.w	d6
	move.w	d6,SHADOW_BMOD(a5)
	move.w	d6,bltbmod(a4)
	add.w	a3,d7		; adjust destModulo
	neg.w	d7
	move.w	d7,SHADOW_CDMOD(a5)
	move.w	d7,bltcmod(a4)
	move.w	d7,bltdmod(a4)
	rts
	    ;==========


	    ;-- decreasing blit
decreasing:
	cmp.w	d0,d2		; compare src to dest bit
	ble	decreasingReverse

	    ;-- test if in the same scan line
	move.w	d0,d7		; get srcX bit offset
	add.w	d4,d7		; add sizeX
	subq.w	#1,d7		; produce final x on src line
	lsr.w	#4,d7		; make word offset
	add.w	d7,d7		; make byte offset
	ext.l	d7
	add.l	a0,d7		; make src final x line address
	cmp.l	d7,a1		; compare with destination address
	bgt	decreasingForward

	;------ reverse blit, negative modulo, with A source
	    ;-- calculate blit size
	move.w	d2,d6		; destX bit offset
	add.w	d4,d6		;   + sizeX
	subq.w	#1,d6		;   - 1
	move.w	d6,-(a7)	; save dest + size - 1
	lsr.w	#4,d6		; number of words to blit - 2
	    ;-- get A data
	moveq	#0,d4
	move.l	SHADOW_APT(a5),d7	; check for TempA
	bne.s	gotReverseA
	move.w	d6,d4		; number of words to blit - 2
	add.w	d4,d4		; number of bytes to blit - 4
	addq	#4,d4		; number of bytes to blit
	movem.l	d0-d1,-(a7)
	move.l	d4,d0
	moveq	#MEMF_CHIP,d1
	bsr	GetMustMem
	move.l	d0,SHADOW_APT(a5)
	move.l	d0,d7
	movem.l	(a7)+,d0-d1
	beq		increasingScanTest.	; spence - Jan 18 1991

gotReverseA:
	    ;-- build A data
		;-- grab the blitter
	CALLLVO	OwnBlitter	; (before the waitblitdone)

	move.l	d7,a1
		;-- first word in A data
	move.w	d2,d7			; using dest x bit...
	add.w	d7,d7			; 
	lea	_fwmaskTable(pc),a0	; get
		;-- ensure A data not in use
	bsr	waitblitdone
	move.w	0(a0,d7.W),(a1)+	;   second word like a fwmask
		;-- set intermediate words in A data
	moveq	#-1,d7
	bra.s	midReverseA
initReverseA:
	move.w	d7,(a1)+
midReverseA:
	dbf	d6,initReverseA

	lea	_custom,a4
		;-- set hardware fwmask and lwmask
	move.l	d7,SHADOW_AFWM(a5)
	move.l	d7,bltafwm(a4)
		;-- set penultimate word in A data
	move.w	(a7)+,d6	; recover dest + size - 1
	move.w	d6,d7
	and.w	#$0F,d7
	add.w	d7,d7
	lea	_lwmaskTable(pc),a0
	move.w	0(a0,d7.w),d7	; last word like lwmask
	and.w	d7,-2(a1)	;   which can overlap fwmask
		;-- clear last word in A data
	clr.w	(a1)			; clear last word
		;-- set adata (at end of line)
	move.l	a1,bltapt(a4)
	    ;-- get B shift into con1
	move.w	d0,d7		; get src x bit
	sub.w	d2,d7		; subtract dest x bit
	and.w	#$0F,d7		; "add 16" to the negative result
	ror.w	#4,d7
	or.w	#BC1F_DESC,d7
	move.w	d7,SHADOW_CON1(a5)
	move.w	d7,bltcon1(a4)
	    ;-- blit line length for address adjustments
	lsr.w	#4,d6		; number of words to blit - 2
		;-- adjust start address offsets
	move.w	d6,d7
	add.w	d7,d7		; get line bytes (sans 2 words)
	ext.l	d7
	add.l	d7,d1		; start at last line word in src
	addq.l	#2,d7		; bump to just sans 1 word
	add.l	d7,d3		; start at last line word in dest
	    ;-- partial con0 (minterm and A shift) into SHADOW_CON0
	move.w	#NABC+NANBC+SRCA+SRCB+SRCC+DEST,(a5)	; SHADOW_CON0
	    ;-- blit size into d0
	addq.w	#2,d6		; blit word width (1 extra word)
	move.w	d6,d0
	swap	d0
	move.w	d5,d0
	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	move.w	d6,d7
	add.w	a2,d7		; adjust srcModulo
	neg.w	d7
	move.w	d7,SHADOW_BMOD(a5)
	move.w	d7,bltbmod(a4)
	move.w	d6,d7
	add.w	a3,d7		; adjust destModulo
	neg.w	d7
	move.w	d7,SHADOW_CDMOD(a5)
	move.w	d7,bltcmod(a4)
	move.w	d7,bltdmod(a4)
	neg.w	d6
	move.w	d6,SHADOW_AMOD(a5)
	move.w	d6,bltamod(a4)
	rts
	    ;==========


	;------ forward blit, negative modulo
decreasingForward:
	bsr.s	forwardCore

	    ;-- fix up blit start addresses to last line of rectangle
	subq.w	#1,d5
	move.w	a2,d7		; adjust srcWordOffset
	mulu	d5,d7
	add.l	d7,d1
	move.w	a3,d7		; adjust destWordOffset
	mulu	d5,d7
	add.l	d7,d3

	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	move.w	d6,d7
	add.w	a2,d6		; adjust srcModulo
	neg.w	d6
	move.w	d6,SHADOW_BMOD(a5)
	move.w	d6,bltbmod(a4)
	add.w	a3,d7		; adjust destModulo
	neg.w	d7
	move.w	d7,SHADOW_CDMOD(a5)
	move.w	d7,bltcmod(a4)
	move.w	d7,bltdmod(a4)
	rts

	;------ reverse blit, positive modulo
decreasingReverse:
	bsr	reverseCore

	    ;-- fix up blit start addresses to last line of rectangle
	subq.w	#1,d5
	move.w	a2,d7		; adjust srcWordOffset
	mulu	d5,d7
	add.l	d7,d1
	move.w	a3,d7		; adjust destWordOffset
	mulu	d5,d7
	add.l	d7,d3

	    ;-- construct modulos
	add.w	d6,d6		; get blit bytes (blit words << 1)
	sub.w	d6,a2		; adjust srcModulo
	move.w	a2,SHADOW_BMOD(a5)
	move.w	a2,bltbmod(a4)
	sub.w	d6,a3		; adjust destModulo
	move.w	a3,SHADOW_CDMOD(a5)
	move.w	a3,bltcmod(a4)
	move.w	a3,bltdmod(a4)
	rts


;------ forwardCore --------------------------------------------------
forwardCore:
	    ;-- grab the blitter
	CALLLVO	OwnBlitter
	lea	_custom,a4
	    ;-- fwmask for dest
	move.w	d2,d7
	add.w	d7,d7
	lea	_fwmaskTable(pc),a0
	bsr	waitblitdone
	move.w	0(a0,d7.W),d7
	move.w	d7,SHADOW_AFWM(a5)
	move.w	d7,bltafwm(a4)
	    ;-- get B shift into con1
	move.w	d2,d7		; get dest x bit
	sub.w	d0,d7		; subtract src x bit
	ror.w	#4,d7
	move.w	d7,SHADOW_CON1(a5)
	move.w	d7,bltcon1(a4)
	    ;-- lwmask for dest + size - 1
	move.w	d2,d7		; destX bit offset
	add.w	d4,d7		;   + sizeX
	subq.w	#1,d7		;   - 1
	move.w	d7,d6		; save dest + size - 1
	and.w	#$0F,d7		; get associated bit
	add.w	d7,d7
	lea	_lwmaskTable(pc),a0
	move.w	0(a0,d7.W),d7
	move.w	d7,SHADOW_ALWM(a5)
	move.w	d7,bltalwm(a4)
	    ;-- blit size into d0
	lsr.w	#4,d6		; number of words to blit - 1
	addq.w	#1,d6		; blit word width
	move.w	d6,d0
	swap	d0
	move.w	d5,d0
	    ;-- partial con0 (minterm and A shift) into SHADOW_CON0
	move.w	#NABC+NANBC+SRCB+SRCC+DEST,(a5)		; SHADOW_CON0
	    ;-- constant A data
	moveq	#0,d4
	move.w	#$ffff,bltadat(a4)
	rts

;------ reverseCore --------------------------------------------------
reverseCore:
	    ;-- grab the blitter
	CALLLVO	OwnBlitter
	lea	_custom,a4
	    ;-- lwmask for src
	move.w	d0,d7
	add.w	d7,d7
	lea	_fwmaskTable(pc),a0
	bsr	waitblitdone
	move.w	0(a0,d7.W),d7
	move.w	d7,SHADOW_ALWM(a5)
	move.w	d7,bltalwm(a4)
	    ;-- fwmask for src + size - 1
	move.w	d0,d7		; srcX bit offset
	add.w	d4,d7		;   + sizeX
	subq.w	#1,d7		;   - 1
	move.w	d7,d6		; save src + size - 1
	and.w	#$0F,d7
	add.w	d7,d7
	lea	_lwmaskTable(pc),a0
	move.w	0(a0,d7.W),d7
	move.w	d7,SHADOW_AFWM(a5)
	move.w	d7,bltafwm(a4)
	    ;-- blit line length for address adjustments
	lsr.w	#4,d6		; number of words to blit - 1
		;-- adjust start address offsets
	move.w	d6,d7
	add.w	d7,d7		; get line bytes
	ext.l	d7
	add.l	d7,d1		; start at last line word in src
	add.l	d7,d3		; start at last line word in dest
	    ;-- get B shift into con1
	move.w	d0,d7		; get src x bit
	sub.w	d2,d7		; subtract dest x bit
	ror.w	#4,d7
	move.w	d7,d2		; save for con0
	or.w	#BC1F_DESC,d7
	move.w	d7,SHADOW_CON1(a5)
	move.w	d7,bltcon1(a4)
	    ;-- blit size into d0
	addq.w	#1,d6		; blit word width
	move.w	d6,d0
	swap	d0
	move.w	d5,d0
	    ;-- partial con0 (minterm and A shift) into SHADOW_CON0
	or.w	#NABC+NANBC+SRCB+SRCC+DEST,d2
	move.w	d2,(a5)		; SHADOW_CON0
	    ;-- constant A data
	moveq	#0,d4
	move.w	#$ffff,bltadat(a4)
	rts


******* graphics.library/BltTemplate ******************************************
*
*   NAME
*	BltTemplate -- Cookie cut a shape in a rectangle to the RastPort.
*
*   SYNOPSIS
*	BltTemplate(SrcTemplate, SrcX, SrcMod, rp,
*	            A0           D0:16  D1:16  A1
*	    DstX,  DstY, SizeX, SizeY)
*	    D2:16  D3:16 D4:16  D5:16
*
*	void BltTemplate(UWORD *, WORD, WORD, struct RastPort *,
*	     WORD, WORD, WORD, WORD);
*
*   FUNCTION
*	This function draws the image in the template into the
*	RastPort in the current color and drawing mode at the
*	specified position.  The template is assumed not to overlap
*	the destination.
*	If the template falls outside the RastPort boundary, it is
*	truncated to that boundary.
*
*	Note: the SrcTemplate pointer should point to the "nearest" word
*	   (rounded down) of the template mask. Fine alignment of the mask
*	   is achieved by setting the SrcX bit offseet within the range
*	   of 0 to 15 decimal.
*
*   INPUTS
*	SrcTemplate  - pointer to the first (nearest) word of the template mask.
*	SrcX         - x bit offset into the template mask (range 0..15).
*	SrcMod       - number of bytes per row in template mask.
*	rp           - pointer to destination RastPort.
*	DstX, DstY   - x and y coordinates of the upper left
*	               corner of the destination for the blit.
*	SizeX, SizeY - size of the rectangle to be used as the
*	               template.
*
*   NOTES
*	o   This function may use the blitter.
*
*   SEE ALSO
*	BltBitMap()  graphics/rastport.h
*
*******************************************************************************
 STRUCTURE  TEMP,SHADOW_SIZEOF
	LONG	TEMP_CLIPRECT
	WORD	TEMP_SRC		; d0
	LONG	TEMP_DEST		; d2/d3
	LONG	MYTEMPSIZE		; d4/d5
	LABEL	TEMP_SIZEOF

 STRUCTURE  PARM,TEMP_SIZEOF
	LONG	PARM_SRCMOD		; d1
	LONG	PARM_DESTX		; d2
	LONG	PARM_DESTY		; d3
	LONG	PARM_SIZEX		; d4
	LONG	PARM_SIZEY		; d5
	LONG	PARM_UNUSEDD6		; d6
	LONG	PARM_UNUSEDD7		; d7
	LONG	PARM_SRCPLANE		; a0
	LONG	PARM_RASTPORT		; a1

_BltTemplate:
	movem.l d1-d7/a0-a5,-(a7)
	sub.w	#TEMP_SIZEOF,a7
	move.l	a7,a5

	;-- check for simple RastPort
	move.l	(a1),d7			; rp_Layer
	bne.s	clipRects
	move.w	d1,a2
	moveq	#0,d1
	move.l	rp_BitMap(a1),a4
	bsr	tDoBlit
	bra	btRts

clipRects:
	;-- lock clip windows
	move.l	a5,a4
	move.l	d7,a5
	CALLLVO	LockLayerRom		; does NOT hit d0/d1/a0/a1
	move.l	a4,a5

	    ;-- adjust destination for clip rectangles
	move.l	d7,a3
	add.w	lr_MinX(a3),d2
	sub.w	lr_Scroll_X(a3),d2
	add.w	lr_MinY(a3),d3
	sub.w	lr_Scroll_Y(a3),d3
	movem.w	d0/d2-d5,TEMP_SRC(a5)	; & DEST & SIZE

	lea.l	lr_ClipRect(a3),a3	; get first ClipRect
	bra.s	nextClipRect

clipRectLoop:
	bsr	tClipCode
	bne.s	recoverCoords

	    ;-- find bitmap
	tst.l	cr_lobs(a3)		; using RastPort BitMap?
	bne.s	crBitMap
	move.l	rp_BitMap(a1),a4
	bra.s	doCBlit
crBitMap:
	move.l	cr_BitMap(a3),d7
	beq.s	recoverCoords
	move.l	d7,a4
	move.w	cr_MinX(a3),d7		; adjust x for offscreen memory
	and.w	#$0FFF0,d7		;	with origin @ MinX
	sub.w	d7,d2			;
	sub.w	cr_MinY(a3),d3		; adjust y for offscreen memory
						;	with origin @ MinY

doCBlit:
	move.l	PARM_SRCPLANE(a5),a0	; recover srcPlane
	move.w	PARM_SRCMOD+2(a5),a2	; recover srcModulo
	move.l	a3,TEMP_CLIPRECT(a5)
	bsr	tDoBlit
	move.l	TEMP_CLIPRECT(a5),a3
recoverCoords:
	movem.w	TEMP_SRC(a5),d0/d2-d5	; recover srcX, destXY, sizeXY
nextClipRect:
	move.l	(a3),d7			; check for more clip regions
	move.l	d7,a3
	bne.s	clipRectLoop

	;-- check for super bit map
	move.l	(a1),a3			; rp_Layer
	move.l	lr_SuperClipRect(a3),d7
	beq.s	btCEnd
	move.l	lr_SuperBitMap(a3),a4
	    ;-- get original dest without layer Min & Scroll effects
	move.w	PARM_DESTX+2(a5),d2
	move.w	PARM_DESTY+2(a5),d3
	movem.w	d2/d3,TEMP_DEST(a5)

	move.l	d7,a3			; ClipRect

vClipRectLoop:
	bsr.s	tClipCode
	bne.s	recoverSuperCoords

	move.l	PARM_SRCPLANE(a5),a0	; recover srcPlane
	move.w	PARM_SRCMOD+2(a5),a2	; recover srcModulo
	move.l	a3,TEMP_CLIPRECT(a5)
	bsr.s	tDoBlit
	move.l	TEMP_CLIPRECT(a5),a3
recoverSuperCoords:
	movem.w	TEMP_SRC(a5),d0/d2-d5	; recover srcX, destXY, sizeXY
	move.l	(a3),d7		    ;check for more clip regions
	move.l	d7,a3
	bne.s	vClipRectLoop

btCEnd:
	    ;-- unlock clip windows
	move.l	(a1),a5			; rp_Layer
	CALLLVO	UnlockLayerRom		; does NOT hit d0/d1/a0/a1


btRts:
	add.w	#TEMP_SIZEOF,a7
	movem.l (a7)+,d1-d7/a0-a5
	rts


;------ tClipCode ----------------------------------------------------
;
;   NAME
;	tClipCode 
;
;   INPUTS
;	d0.w	srcX
;	d1.w
;	d2.w	destX
;	d3.w	destY
;	d4.w	sizeX
;	d5.w	sizeY
;
;	a3.l	clipRect
;
;   OUTPUTS
;	d0.w	srcX
;	d1.w	srcY
;	d2.w	destX
;	d3.w	destY
;	d4.w	sizeX
;	d5.w	sizeY
;	d6	preserved
;	d7	non-zero indicates all clipped (also in condition code)
;
;	aN	preserved
;
;---------------------------------------------------------------------
tClipCode:
	cmp.w	cr_MinX(a3),d2
	bge.s	xMinOK
	    ;-- clip X to low edge
	sub.w	d2,d0		; start fixup for srcX
	add.w	d2,d4		;   and sizeX
	move.w	cr_MinX(a3),d2
	add.w	d2,d0		; complete fixup for srcX
	sub.w	d2,d4		;   and sizeX
xMinOK:
	move.w	d4,d7
	add.w	d2,d7
	subq.w	#1,d7
	cmp.w	cr_MaxX(a3),d7
	ble.s	xMaxOK
	    ;-- clip X to max edge
	sub.w	d7,d4
	add.w	cr_MaxX(a3),d4
xMaxOK:
	tst.w	d4		; check for non-zero positive size
	ble.s	tccAllClipped

	moveq	#0,d1		; srcY starts at zero
	cmp.w	cr_MinY(a3),d3
	bge.s	yMinOK
	    ;-- clip Y to low edge
	sub.w	d3,d1		; start fixup for srcY
	add.w	d3,d5		;   and sizeY
	move.w	cr_MinY(a3),d3
	add.w	d3,d1		; complete fixup for srcY
	sub.w	d3,d5		;   and sizeY
yMinOK:
	move.w	d5,d7
	add.w	d3,d7
	subq.w	#1,d7
	cmp.w	cr_MaxY(a3),d7
	ble.s	yMaxOK
	    ;-- clip Y to max edge
	sub.w	d7,d5
	add.w	cr_MaxY(a3),d5
yMaxOK:
	tst.w	d5		; check for non-zero positive size
	ble.s	tccAllClipped
	moveq	#0,d7
	rts

tccAllClipped:
	moveq	#-1,d7
	rts


;------ tDoBlit ------------------------------------------------------
;
;   NAME
;	tDoBlit
;
;   INPUTS
;	d0.w	srcX
;	d1.w	srcY
;	d2.w	destX
;	d3.w	destY
;	d4.w	sizeX
;	d5.w	sizeY
;
;	a0.l	srcPlane
;	a1.l	rastPort
;	a2.w	srcModulo
;	a4.l	bitMap
;	a6.l	GfxBase
;
;   OUTPUTS
;	d0	destroyed
;	d1	destroyed
;	d2	destroyed
;	d3	destroyed
;	d4	destroyed
;	d5	destroyed
;	d6	destroyed
;	d7	destroyed
;
;	a0	destroyed
;	a1	preserved
;	a2	destroyed
;	a3	destroyed
;	a4	preserved
;	a5	destroyed
;
;---------------------------------------------------------------------
tDoBlit:
	move.l	a4,-(a7)		; save bitMap
	cmp.w	#UNLIKELY_WORD,bm_Pad(a4)
	bne.s	not_chunky_blit
	btst	#IBMB_CHUNKY,bm_Flags(a4)
	bne	do_chunky_template

not_chunky_blit:
	tst.b	bm_Depth(a4)
	beq.s	tdbRts
	    ;-- set up the blit
	movem.l	a0-a1,-(a7)
	move.w	bm_BytesPerRow(a4),a3
	move.l	bm_Planes(a4),a1
	bsr	bltSetup
	movem.l	(a7)+,a0-a1
		;-- get write mask and partial minterms
	move.b	rp_Mask(a1),d5
	move.w	(a5),d2			; SHADOW_CON0
		;-- adjust source
	add.l	d1,a0
		;-- get plane loop counters
	moveq	#0,d7			; count up
	moveq	#0,d6			; count down
	move.l	(a7),a2			; get bitMap
	move.b	bm_Depth(a2),d6
		;-- get destPlane array
	addq.l	#bm_Planes,a2
	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	bigBT

smallBT:
		;-- do a blit with just the bltsize register
	move.w	d0,d1			; cache Y
	lsl.w	#6,d1			; shift up Y
	swap	d0			; get X
	and.w	#$003f,d0		; fix 1K to zero
	or.w	d1,d0			; form bltsize register
	bra.s	tdbsDBF

bigBT:
	move.w	d2,bltcon0(a4)
	move.w	d0,bltsizv(a4)
	swap	d0
	bra.s	tdbbDBF

	    ;--	cycle through small planes
tdbsLoop:
	move.l	(a2)+,a3
		;-- check for masked plane
	lsr.b	#1,d5
	bcc.s	tdbsBumpPlane
		;-- set CON0
	moveq	#0,d1
	move.b	rp_minterms(a1,d7.w),d1
	or.w	d2,d1
	bsr	waitblitdone
	move.w	d1,bltcon0(a4)
		;-- adjust planes by offsets
	add.l	d3,a3
	move.l	a0,bltbpt(a4)
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
	move.w	d0,bltsize(a4)
tdbsBumpPlane:
	addq.w	#1,d7
tdbsDBF:
	dbf	d6,tdbsLoop

	    ;-- release the blitter
tdbDisown:
	CALLLVO DisownBlitter

tdbRts:
	move.l	(a7)+,a4
	rts

	    ;--	cycle through big planes
tdbbLoop:
	move.l	(a2)+,a3
		;-- check for masked plane
	lsr.b	#1,d5
	bcc.s	tdbbBumpPlane
		;-- set CON0
	moveq	#0,d1
	bsr	waitblitdone
	move.b	rp_minterms(a1,d7.w),bltcon0l(a4)
		;-- adjust planes by offsets
	add.l	d3,a3
	move.l	a0,bltbpt(a4)
	move.l	a3,bltcpt(a4)
	move.l	a3,bltdpt(a4)
	move.w	d0,bltsizh(a4)
tdbbBumpPlane:
	addq.w	#1,d7
tdbbDBF:
	dbf	d6,tdbbLoop
	bra.s	tdbDisown

	xref	ChunkyBlitSetup

do_chunky_template::
;   INPUTS
; d0/d1/d2/d3=sx sy dx dy
; d4/d5=sizex sizey
; a0=src data a1=rp a2=src modulo a4=bitmap a6=gfxbase
; can trash d0-d7/a0/a2-a4

; chunky template conversion happens four pixels at a time.
; the next 4 bits of the template are fetched and used
; to table-lookup the chunky data.

TEMP_SIZE	set	0
	ARRAYVAR	chunkydata,16*4
	LONGVAR	srcx
	move.l	a5,-(a7)
	ALLOCLOCALS

; initialize differently depending upon
; drawmode:
;   JAM1 : chdata[n] & (inverse?BgPen:FgPen) & mask
;   JAM2 : mask & (chdata[n] & (inverse?BgPen:FgPen) | (~chdata[n] & (inverse?Fg:bg)))
;   COMPLEMENT: mask & chdata[n]

	lea	chunkydata(a7),a3
	movem.l	d0/d1/d2/d3,-(a7)
	lea	chdata(pc),a5
	move.b	rp_Mask(a1),d0
	replicate	d0
	move.b	rp_FgPen(a1),d1
	move.b	rp_BgPen(a1),d2
	tst.b	rp_DrawMode(a1)
	bpl.s	no_inverse_vid
	exg	d1,d2
no_inverse_vid
; we always use the JAM2 equation, but kill some terms depending upn draw mode
	btst	#1,rp_DrawMode(a1)	; complement?
	beq.s	no_complement
	moveq	#-1,d1
	moveq	#0,d2
no_complement:
	btst	#0,rp_DrawMode(a1)	; jam2?
	bne.s	is_jam2
	moveq	#0,d2				; kill second pen if jam1
is_jam2:
	replicate	d1
	replicate	d2

; now, out=d0 & ((chdata & d1) | (~chdata & d2))
	moveq	#15,d7
init_local_array:
	move.l	(a5)+,d6	; chdata
	move.l	d6,d3
	and.l	d1,d3
	not.l	d6
	and.l	d2,d6
	or.l	d6,d3
	and.l	d0,d3
	move.l	d3,(a3)+
	dbra	d7,init_local_array

	movem.l	(a7)+,d0/d1/d2/d3

	exg	d4,d0	
	exg	d5,d1		; d0=sizex d1=sizey d4=srcx d5=srcy
	exg	d0,d2
	exg	d1,d3		; d0=destx d1=desty d2=sizex d3=sizey
	add	d0,d2	; generate right x
	subq	#1,d2
	add	d1,d3
	subq	#1,d3	; generate bottom y
	move.l	bm_Planes(a4),a3
	move.l	a2,a5
	move.l	a4,a2
	move.l	d5,-(a7)		; save src ofs
	bsr	ChunkyBlitSetup		; complicated routine in a/bltpattern.asm
	move.l	a5,a2
; now, d4/d5=srcx/y
	move	a2,d7
	move.l	(a7)+,d1		; pop src ofs
	mulu	d7,d1
	add	d1,a0			; update src data pointer
; we need to subtract the number of skipped pixels in the first lword
; from the srcx coordinate
	move	d0,d1
	lsr.w	#3,d1		; /8=# of pixels skipped
	sub	d1,d4
	ext.l	d4
	move.l	d4,srcx_l(a7)

; just for laughs, let's try it this way
	lea	chunkydata(a7),a5
do_jam2:
template_yloop:
	move.l	srcx_l(a7),d4
	move.w	d5,d1
	bfextu	(a0){d4:4},d7	; grab 4 bits
	addq.l	#4,d4
	move.l	(a5,d7.w*4),d7
	bfins	d7,(a3){d0:d6}
	lea	4(a3),a3
	tst	d5
	bmi.s	skip_all
	bra.s	xloop_end
xloop:	bfextu	(a0){d4:4},d7
	addq.l	#4,d4
	move.l	(a5,d7.w*4),(a3)+
xloop_end:
	dbra	d1,xloop
; now, do right side
	bfextu	(a0){d4:4},d7
	move.l	(a5,d7.w*4),d7
	rol.l	d2,d7
	bfins	d7,(a3){0:d2}
	lea	4(a3),a3
skip_all:
	add.l	a4,a3	; update dest pointer
	add.l	a2,a0	; and source pointer
	dbra	d3,template_yloop
	lea	TEMP_SIZE(a7),a7
	move.l	(a7)+,a5
	bra	tdbRts


chdata::
	dc.l	$00000000,$000000ff,$0000ff00,$0000ffff
	dc.l	$00ff0000,$00ff00ff,$00ffff00,$00ffffff
	dc.l	$ff000000,$ff0000ff,$ff00ff00,$ff00ffff
	dc.l	$ffff0000,$ffff00ff,$ffffff00,$ffffffff

*	planecnt = BltBitMap(SrcBitMap, SrcX, SrcY, DstBitMap,
*	D0                   A0         D0:16 D1:16 A1
*	    DstX, DstY, SizeX, SizeY, Minterm, Mask [, TempA])
*	    D2:16 D3:16 D4:16  D5:16  D6:8     D7:8   [A2]
*
*	ULONG BltBitMap(struct BitMap *, WORD, WORD, struct BitMap *,
*	    WORD, WORD, WORD, WORD, UBYTE, UBYTE, UWORD *);
*

chunky_to_chunky_blit:
	movem.l	d3/d4/d5/d6/a2/a3,-(a7)
	move	bm_BytesPerRow(a0),d6
	mulu	d6,d1
	move.l	bm_Planes(a0),a2
	add.w	d0,a2
	add.l	d1,a2

	mulu	bm_BytesPerRow(a1),d3	; y*bpr
	move.l	bm_Planes(a1),a0
	move.w	bm_BytesPerRow(a1),a1
	add.w	d2,a0
	add.l	d3,a0
	move	d4,d0
	and.l	#3,d0
	bne.s	1$
	subq.l	#4,d4
1$:	lsl.w	#3,d0			; last bfins size
	lsr.w	#2,d4			; # of lwords
	sub	d4,a1
	sub	d4,a1
	sub	d4,a1
	sub	d4,a1
	sub	d4,d6
	sub	d4,d6
	sub	d4,d6
	sub	d4,d6
	cmp.l	a2,a0
	bhi.s	copy_backwards

	lea	copy_xloopend(pc),a3
	move	d4,d2
	lsr.w	#3,d4
	and.w	#7,d2
	neg.w	d2
	lea	(a3,d2.w*2),a3

	bra.s	copy_yloopend
copy_yloop:
	move	d4,d2
	jmp	(a3)
copy_xloop:
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
	move.l	(a2)+,(a0)+
copy_xloopend:
	dbra	d2,copy_xloop
	move.l	(a2),d1
	rol.l	d0,d1
	bfins	d1,(a0){0:d0}
	add.l	a1,a0	
	add.w	d6,a2
copy_yloopend:
	dbra	d5,copy_yloop
	movem.l	(a7)+,d3/d4/d5/d6/a2/a3
	rts
	
copy_backwards:
; now, d4=lword count
; a0=dest ptr
; a1-dest modulo
; a2=src ptr
; d4=#lwords/line
; d5=ycount
; d6=src modulo
; d0=width of last field on line
; need to update pointers to point to the end of the last line
; first do source
	move	d4,d1
	lsl.w	#2,d1	; to bytes
	add.w	d6,d1	; d1 now = # of bytes per line
	move	d5,d2
	subq	#1,d2
	mulu	d2,d1	; mult by # of lines-1. now pointing at first byte of last line
	add.l	d1,a2	; adjust source pointer
	lea	(4,a2,d4.w*4),a2	; and point one past last longword on the line

	move	d4,d1
	lsl.w	#2,d1	; to bytes
	add.w	a1,d1	; d1 now = # of bytes per line
	move	d5,d2
	subq	#1,d2
	mulu	d2,d1	; mult by # of lines-1. now pointing at first byte of last line
	add.l	d1,a0	; adjust dest pointer
	lea	(4,a0,d4.w*4),a0	; and point one past last longword on the line
	sub	#4,a1
	sub	#4,d6
	bra.s	backwards_copy_yloopend
backwards_copy_yloop:
	move	d4,d2
	move.l	-(a2),d1
	rol.l	d0,d1
	bfins	d1,(a0){0:d0}
	subq.l	#4,a0
	bra.s	backwards_copy_xloopend
backwards_copy_xloop:
	move.l	-(a2),-(a0)
backwards_copy_xloopend:
	dbra	d2,backwards_copy_xloop
	sub.l	a1,a0
	sub.w	d6,a2
backwards_copy_yloopend:
	dbra	d5,backwards_copy_yloop
	movem.l	(a7)+,d3/d4/d5/d6/a2/a3
	rts
	

its_a_minterm_0:
; so clear the raster. Need to add masking!
	movem.l	d3/d4/d5,-(a7)
	mulu	bm_BytesPerRow(a1),d3	; y*bpr
	move.l	bm_Planes(a1),a0
	move.w	bm_BytesPerRow(a1),a1
	add.w	d2,a0
	add.l	d3,a0
	move	d4,d0
	and.l	#3,d0
	bne.s	1$
	subq.l	#4,d4
1$:	lsl.w	#3,d0			; last bfins size
	lsr.w	#2,d4			; # of lwords
	sub	d4,a1
	sub	d4,a1
	sub	d4,a1
	sub	d4,a1
	moveq	#0,d1
	bra.s	clr_yloopend
clr_yloop:
	move	d4,d2
	bra.s	clr_xloopend
clr_xloop:
	move.l	d1,(a0)+
clr_xloopend:
	dbra	d2,clr_xloop
	bfins	d1,(a0){0:d0}
	add.l	a1,a0	
clr_yloopend:
	dbra	d5,clr_yloop
	movem.l	(a7)+,d3/d4/d5
	rts


is_chunky_destination:
	tst.b	d6
	beq	its_a_minterm_0
	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)	; chunky src?
	bne.s	not_chunky_dest
	btst	#IBMB_CHUNKY,bm_Flags(a0)
	bne	chunky_to_chunky_blit

not_chunky_dest:
	movem.l	d3/d4/a2,-(a7)
	mulu	bm_BytesPerRow(a1),d3
	move.w	bm_BytesPerRow(a1),a2
	move.l	bm_Planes(a1),a1
	add	d2,a1
	add.l	d3,a1
	move	d4,d2
	move	d5,d3
	move.l	d7,d4
	bsr.s		_Chunkify
	movem.l	(a7)+,d3/d4/a2
	rts


; void __asm Chunkify(register __a0 struct BitMap *src, register __a1 UBYTE *dest,
; /* __asm */		register __d0 ULONG startx, register __d1 UWORD starty,
; /* __asm */		register __d2 UWORD width, register __d3 UWORD height,
; /* __asm */		register __d4 UBYTE mask, register __a2 UWORD destmodulo);
; this routine copies planar data from a bitmap structure to chunky
; data.

_Chunkify::

TEMP_SIZE	set	0
	LONGVAR	srcx
	LONGVAR	onebits

	movem.l	d2-d7/a2-a6,-(a7)
	ALLOCLOCALS

	move.l	d0,srcx_l(a7)

	mulu	bm_BytesPerRow(a0),d1	; cvt to pointer offset
	move.w	bm_BytesPerRow(a0),a6
	move.l	d1,a3
; calc d7=right field width=(width & 3)*8
	move	d2,d7
	and.l	#3,d7
	bne.s	1$
	subq.l	#4,d2
1$:	lsl.l	#3,d7
	lsr.w	#2,d2			; cvt to # of full lwords
	sub	d2,a2
	sub	d2,a2
	sub	d2,a2
	sub	d2,a2
	lea	BitTable(pc),a4
	moveq	#0,d5
	move.b	bm_Depth(a0),d5
	lea	bm_Planes(a0),a5
	moveq	#0,d1
	move.l	#$01010101,d6
	bra.s	4$
2$:	tst.l	(a5)+
	bpl.s	3$
	or.l	d6,d1
3$:	add.l	d6,d6
4$:	dbra	d5,2$
	move.l	d1,onebits_l(a7)

	moveq	#0,d5
	move.b	bm_Depth(a0),d5

	dc.l	$2a705db0
	dc.l	depth_jmp_table
;	move.l	(depth_jmp_table.l,d5.l*4),a5
	lea	bm_Planes(a0),a0
	pea	ret_point(pc)
	move.l	a5,-(a7)

	move.l	onebits_l+8(a7),a5
	rts

ret_point:
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d2-d7/a2-a6
	rts

depth_jmp_table:
	dc.l	do_depth_0
	dc.l	do_depth_1
	dc.l	do_depth_2
	dc.l	do_depth_3
	dc.l	do_depth_4
	dc.l	do_depth_5
	dc.l	do_depth_6
	dc.l	do_depth_7
	dc.l	do_depth_8

BitTable:
; converts 4 bits of planar data into 4 chunky pixels
	dc.l	$00000000,$00000001,$00000100,$00000101
	dc.l	$00010000,$00010001,$00010100,$00010101
	dc.l	$01000000,$01000001,$01000100,$01000101
	dc.l	$01010000,$01010001,$01010100,$01010101


doplane	macro	pl
;		\1
	move.l	(a0)+,d1		; next bitplane
	ble.s	doneplane\@
	bfextu	(a3,d1.l){d0:4},d1
	IFne	\1
	move.l	(a4,d1.l*4),d1
	lsl.l	#\1,d1
	or.l	d1,d5
	ELSE
	or.l	(a4,d1.l*4),d5
	endc
doneplane\@:
	endm

gencvt	macro	depth
yloop\@:
	move.l	srcx_l+4(a7),d0		; restart x counter
	move.w	d2,d6			; clone width
	ifge	\1-5
	bra	end_xloop\@
	ELSE
	bra.s	end_xloop\@
	endc
xloop\@:
	move.l	a5,d5			; accumulator
	IFge	\1-1
	doplane	0
	endc
	IFge	\1-2
	doplane	1
	endc
	IFge	\1-3
	doplane	2
	endc
	IFge	\1-4
	doplane	3
	endc
	IFge	\1-5
	doplane	4
	endc
	IFge	\1-6
	doplane	5
	endc
	IFge	\1-7
	doplane	6
	endc
	IFge	\1-8
	doplane	7
	endc
	lea	-\1*4(a0),a0
	move.l	d5,(a1)+
	addq.l	#4,d0			; skip bits
end_xloop\@:
	dbra	d6,xloop\@
; now, do right edge
	move.l	a5,d5			; accumulator
	IFge	\1-1
	doplane	0
	endc
	IFge	\1-2
	doplane	1
	endc
	IFge	\1-3
	doplane	2
	endc
	IFge	\1-4
	doplane	3
	endc
	IFge	\1-5
	doplane	4
	endc
	IFge	\1-6
	doplane	5
	endc
	IFge	\1-7
	doplane	6
	endc
	IFge	\1-8
	doplane	7
	endc
	bfins	d5,(a1){0:d7}
	lea	-\1*4(a0),a0
	add.l	a2,a1			; add dest modulo
	add.l	a6,a3			; add src modulo
do_depth_\1:
	dbra	d3,yloop\@
	rts
	endm



	gencvt	0
	gencvt	1
	gencvt	2
	gencvt	3
	gencvt	4
	gencvt	5
	gencvt	6
	gencvt	7
	gencvt	8

	END
