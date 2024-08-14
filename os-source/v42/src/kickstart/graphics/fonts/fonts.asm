**
**	$Id: fonts.asm,v 42.0 93/06/16 11:18:37 chrisg Exp $
**
**      font initialization code
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		graphics

**	Includes

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/strings.i"
	INCLUDE	"exec/initializers.i"

	INCLUDE	"graphics/gfxbase.i"
	INCLUDE	"graphics/text.i"

	INCLUDE	"macros.i"


**	Exports

	XDEF	_SFInit


**	Imports

	XLVO	AllocMem		; Exec
	XLVO	InitStruct		;


	XLVO	AddFont			; Graphics
	XLVO	ExtendFont		;


fInit:
		move.l	#tf_SIZEOF,d0
		moveq	#0,d1
		move.l	gb_ExecBase(a5),a6
		CALLLVO AllocMem
		tst.l	d0
		beq.s	fiErr
		move.l	d0,a2
		move.l	a3,a1
		move.w	#tf_SIZEOF,d0
		CALLLVO InitStruct

		move.l	a2,a1
		move.l	a5,a6
		CALLLVO	AddFont
		addq.w	#1,tf_Accessors(a2)	; never allow to be purged
		move.l	a2,a0
		suba.l	a1,a1
		CALLLVO	ExtendFont
		tst.l	d0
		beq.s	fiErr
		move.l	tf_Extension(a2),a0
		bset	#TE0B_NOREMFONT,tfe_Flags0(a0)

fiErr:
		rts


_SFInit:
		movem.l a2-a3/a5-a6,-(a7)
		;-- Get the graphics library
		move.l	20(sp),a5

		;-- initialize the system font structure
		lea	gb_TextFonts(a5),a0
		NEWLIST	a0		; initialize empty list

		lea	font9Init(pc),a3
		bsr.s	fInit
		move.l	a2,gb_DefaultFont(a5)

		lea	font8Init(pc),a3
		bsr.s	fInit

*		bart - 09.11.90 removed topaz11 for space reasons
*		lea	font11Init(pc),a3
*		bsr.s	fInit
*		bart - 09.11.90 removed topaz11 for space reasons

		movem.l (a7)+,a2-a3/a5-a6
		rts


;------ Data ---------------------------------------------------------

fontName:
		STRING	<'topaz.font'>

;------ 64 Column font descriptor ------------------------------------

font9Init:
	    INITLONG	LN_NAME,fontName
	    INITSTRUCT	1,tf_YSize,,<((tf_Accessors-tf_YSize)/2)-1>
		dc.w	9	    ;font height
		dc.b	FSF_EXTENDED ;font style
		dc.b	FPF_DESIGNED+FPF_ROMFONT+FPF_TALLDOT
		dc.w	10	    ;nominal font width
		dc.w	6	    ;baseline
		dc.w	1	    ;bold smear
	    INITSTRUCT	1,tf_LoChar,,<(20/2)-1>

	INCLUDE	    "open9.i"

;------ 80 Column font descriptors -----------------------------------

font8Init:
	    INITLONG	LN_NAME,fontName
	    INITBYTE	LN_PRI,10
	    INITSTRUCT	1,tf_YSize,,<((tf_Accessors-tf_YSize)/2)-1>
		dc.w	8	    ;font height
		dc.b	FS_NORMAL   ;font style
		dc.b	FPF_DESIGNED+FPF_ROMFONT
		dc.w	8	    ;nominal font width
		dc.w	6	    ;baseline
		dc.w	1	    ;bold smear
	    INITSTRUCT	1,tf_LoChar,,<(20/2)-1>

	INCLUDE	    "open8.i"

*		bart - 09.11.90 removed topaz11 for space reasons
*font11Init:
*	    INITLONG	LN_NAME,fontName
*	    INITSTRUCT	1,tf_YSize,,<((tf_Accessors-tf_YSize)/2)-1>
*		dc.w	11	    ;font height
*		dc.b	FS_NORMAL   ;font style
*		dc.b	FPF_DESIGNED+FPF_ROMFONT
*		dc.w	8	    ;nominal font width
*		dc.w	8	    ;baseline
*		dc.w	1	    ;bold smear
*	    INITSTRUCT	1,tf_LoChar,,<(20/2)-1>
*
*	INCLUDE	    "open11.i"
*		bart - 09.11.90 removed topaz11 for space reasons

	END
