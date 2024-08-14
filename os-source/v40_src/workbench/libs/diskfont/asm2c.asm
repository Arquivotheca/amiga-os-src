**
**	$Id: asm2c.asm,v 39.0 92/07/08 14:32:38 darren Exp $
**
**      diskfont.library assembler to C function interfaces
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	diskfont

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/lists.i"

	INCLUDE	"graphics/text.i"
	INCLUDE	"graphics/gfxbase.i"

	INCLUDE	"diskfont.i"
	INCLUDE	"dfdata.i"
	INCLUDE	"macros.i"

	INCLUDE "ddebug.i"

	XLVO	AddFont

	XDEF	_AddDiskFont
	XDEF	_EqualAspect

	XDEF	_EOpenEngine
	XDEF	_ECloseEngine
	XDEF	_ESetInfo
	XDEF	_EObtainInfo
	XDEF	_EReleaseInfo


** Private bit in gfx/text/txtdata.i

	IFND	TE0B_DUPLICATE
		BITDEF	TE0,DUPLICATE,7	; Duplicate font - differs by DPI
	ENDC

_AddDiskFont:
		;-- AddFont(&dfh->dfh_TF);
		move.l	a6,-(a7)
		move.l	8(a7),a6
		move.l	12(a7),a1

	PRINTF	DBG_ENTRY,<'AddDiskFont($%lx,$%lx)'>,A6,A1

	;-- loop through font list, looking for dups of name, Y size, and
	;-- Style/Flag combos ... mark dups as dups so they cannot
	;-- be found by OpenFont() if called without DPI tag info

			movem.l	a1-a6,-(sp)
			
			lea	dfh_TF(a1),a3	; TextFont
			move.l	dfh_TF+tf_Extension(a1),a4

			move.l	dfl_GfxBase(a6),a5
			lea	gb_TextFonts(a5),a0
			move.l	dfl_SysBase(a6),a6

notadup:
			move.l	LN_NAME(a3),a1

			JSRLIB	FindName
			tst.l	d0
			BEQ_S	nofontdups

			move.l	d0,a0

	PRINTF	DBG_FLOW,<'  AddDiskFont ... Name Match ... '>

	;-- check Y size


	PUSHWORD	tf_YSize(a3)
	PRINTF	DBG_FLOW,<'    NewFont YSize = %ld'>
	PUSHWORD	tf_YSize(a0)
	PRINTF	DBG_FLOW,<'    Compare YSize = %ld'>
	POPLONG		2


			move.w	tf_YSize(a3),d0
			cmp.w	tf_YSize(a0),d0
			BNE_S	notadup


	;-- check relevant Style bits

	PUSHBYTE	tf_Style(a3)
	PRINTF	DBG_FLOW,<'    NewFont Style = $%lx'>
	PUSHBYTE	tf_Style(a0)
	PRINTF	DBG_FLOW,<'    Compare Style = $%lx'>
	POPLONG		2

	;-- Compare for diffs in Italic, Bold, Underlined,
	;
	;-- Ignore Intrinsic of COLORFONT, EXTENDED
	;
	;-- Ignore bits only suitable for query: FSF_TAGGED
	;

			move.b	tf_Style(a3),d0
			move.b	tf_Style(a0),d1

			and.b	#FSF_UNDERLINED!FSF_BOLD!FSF_ITALIC,d0
			and.b	#FSF_UNDERLINED!FSF_BOLD!FSF_ITALIC,d1

			cmp.b	d0,d1
			BNE_S	notadup

	;-- check relevant Flag bits?
	;
	;-- ROMFONT and DISKFONT are source info; they will differ
	;-- depending on if source is/was ROM, DISK, SCALED, or OUTLINE.
	;-- 
	;-- REVPATH, TALLDOT, WIDEDOT, and PROPORTIONAL are set for
	;-- an entire font family, so should not differ.
	;--
	;-- DESIGNED is meaningful for QUERY, and type
	;
	;-- REMOVED is only meaningful for fonts not on the font list.


	PUSHBYTE	tf_Flags(a3)
	PRINTF	DBG_FLOW,<'    NewFont Flags = $%lx'>
	PUSHBYTE	tf_Flags(a0)
	PRINTF	DBG_FLOW,<'    Compare Flags = $%lx'>
	POPLONG		2


	;-- Note that we have a possible DUP, but need to check what
	;-- are comparing against first to see if it is already marked
	;-- as a DUPLICATE.  If so, this one we are going to add may
	;-- not be.
			suba.l	a1,a1		;No tags
			move.l	a0,a2		;cache
			exg	a5,a6		;Use GfxBase, Cache ExecBase

			JSRLIB	ExtendFont

			move.l	a2,a0		;restore
			exg	a5,a6		;Use ExecBase, Cache GfxBase
			
			tst.l	d0
			BEQ_S	notadup		;what?? Low on memory

			move.l	tf_Extension(a0),a1
			btst	#TE0B_DUPLICATE,tfe_Flags0(a1)
			BNE_S	notadup

	;-- Well, we found a DUPLICATE on the list that has not been
	;-- marked as a DUPLICATE, so this one will be added as a DUPLICATE.


	PRINTF	DBG_FLOW,<'      NewFont marked as DUPLICATE'>

			bset	#TE0B_DUPLICATE,tfe_Flags0(a4)
nofontdups:
			movem.l	(sp)+,a1-a6

		lea	dfh_TF(a1),a1
		LINKGFX	AddFont

		;-- dfh->dfh_TF.tf_Extension->tfe_Flags0 |= TE0F_NOREMFONT;
		move.l	12(a7),a1
		move.l	dfh_TF+tf_Extension(a1),a0
		bset	#TE0B_NOREMFONT,tfe_Flags0(a0)


		;-- dfh->dfh_TF.tf_Accessors = 1;
		move.w	#1,dfh_TF+tf_Accessors(a1)
		;-- AddTail(&DiskfontBase->dfl_DiskFonts, dfh);
		lea	dfl_DiskFonts(a6),a0
		ADDTAIL
		;-- (DiskfontBase->dfl_NumLFonts)++;
		addq.w	#1,dfl_NumLFonts(a6)
		move.l	(a7)+,a6
		rts

_EqualAspect:
		movem.w	4(a7),d0-d1
		mulu	10(a7),d0
		mulu	8(a7),d1
		cmp.l	d0,d1
		seq	d0
		ext.w	d0
		ext.l	d0
		rts


_EOpenEngine:
		move.l	a6,-(a7)
		move.l	8(a7),a6
		jsr	-30(a6)		;_LVOOpenEngine(a6)
		move.l	(a7)+,a6
		rts

_ECloseEngine:
		move.l	a6,-(a7)
		move.l	8(a7),a0
		move.l	(a0),a6
		jsr	-36(a6)		;_LVOCloseEngine(a6)
		move.l	(a7)+,a6
		rts

_ESetInfo:
		move.l	a6,-(a7)
		lea	8(a7),a1
		move.l	(a1)+,a0
		move.l	(a0),a6
		jsr	-42(a6)		;_LVOSetInfo(a6)
		move.l	(a7)+,a6
		rts

_EObtainInfo:
		move.l	a6,-(a7)
		lea	8(a7),a1
		move.l	(a1)+,a0
		move.l	(a0),a6
		jsr	-48(a6)		;_LVOObtainInfo(a6)
		move.l	(a7)+,a6
		rts

_EReleaseInfo:
		move.l	a6,-(a7)
		lea	8(a7),a1
		move.l	(a1)+,a0
		move.l	(a0),a6
		jsr	-54(a6)		;_LVOReleaseInfo(a6)
		move.l	(a7)+,a6
		rts

	END
