*
* Graphics OpenFont, and WeighTAMatch patches
* casm -a debug.asm -o debug.o -iinclude:


*
* includes
*

		INCLUDE	"exec/types.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE "exec/resident.i"
                INCLUDE "exec/execbase.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/ports.i"
		INCLUDE "exec/ables.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/semaphores.i"

		INCLUDE "exec/macros.i"
		INCLUDE	"hardware/intbits.i"

		INCLUDE "devices/audio.i"

		INCLUDE "graphics/gfxbase.i"
		INCLUDE "graphics/text.i"

		INCLUDE	"utility/tagitem.i"
		INCLUDE	"diskfont/diskfonttag.i"

		INCLUDE	"dfdata.i"
		INCLUDE "ddebug.i"

*
* xrefs
*

		XREF	_LVOWeighTAMatch
		XREF	_LVOOpenFont
		XREF	_LVOFindName
		XREF	_LVOPermit


*
* macros
*
LINKEXEGFX	MACRO
		move.l	a6,-(sp)
		move.l	4,a6
		jsr	_LVO\1(a6)
		move.l	(sp)+,a6
		ENDM

* equates
*

EXECBASE	EQU		4

	IFND	TE0B_DUPLICATE
		BITDEF	TE0,DUPLICATE,7	; Duplicate font - differs by DPI
	ENDC
*
* code
*
		XDEF	_InstallGfxPatches

_InstallGfxPatches:


		movem.l	a2/a6,-(sp)

		movea.l		EXECBASE,a6

		moveq	#39,d0
		lea	gfxname(pc),a1

		JSRLIB	OpenLibrary
		tst.l	d0
		beq.s	failed


		FORBID

	; install WeighTAMatch patch first, followed by OpenFont which
	; relies on it

		movea.l		d0,a2
		movea.l		d0,a1
		move.w		#_LVOWeighTAMatch,a0
		move.l		#_WeighTAMatch,d0

		JSRLIB	SetFunction



		movea.l		a2,a1
		move.w		#_LVOOpenFont,a0
		move.l		#_OpenFont,d0

		JSRLIB	SetFunction

		PERMIT

		moveq	#00,d0		; wait
		JSRLIB	Wait

failed:

		movem.l	(sp)+,a2/a6
		rts


*
* patches
*

******* graphics.library/OpenFont ************************************
*
*   NAME
*	OpenFont -- Get a pointer to a system font.
*
*   SYNOPSIS
*	font = OpenFont(textAttr)
*	D0              A0
*
*	struct TextFont *OpenFont(struct TextAttr *);
*
*   FUNCTION
*	This function searches the system font space for the graphics
*	text font that best matches the attributes specified.  The
*	pointer to the font returned can be used in subsequent
*	SetFont and CloseFont calls.  It is important to match this
*	call with a corresponding CloseFont call for effective
*	management of ram fonts.
*
*   INPUTS
*	textAttr - a TextAttr or TTextAttr structure that describes the
*	           text font attributes desired.
*
*   RESULT
*	font is zero if the desired font cannot be found.  If the named
*	font is found, but the size and style specified are not
*	available, a font with the nearest attributes is returned.
*
*   BUGS
*	Prior to V39 this function would return a TextFont pointer
*	for any font which matched exactly in Y size, regardless of
*	differences in DPI; see BUG notes for WeighTAMatch().
*
*	As part of fixing this bug it is REQUIRED that you use pass the
*	same TextAttr (or TTextAttr) to this function that was used when
*	OpenDiskFont() was called.
*
*	WeighTAMatch() was a public graphics function in V36-V37.  It is
*	now a PRIVATE function for use by the system only.
*
*   SEE ALSO
*	CloseFont()  SetFont()
*	diskfont.library/OpenDiskFont  graphics/text.h
*	intuition/intuition.h
*
**********************************************************************
*
*   REGISTERS
*	d0
*	d1	font style and flags
*	d2	font and text attribute style and flags xored
*	d3	working weight
*	d4	best weight so far
*	d5	bit mask
*	d6	longword index

*	a0
*	a1
*	a2	
*	a3	font under consideration (name matches)
*	a4	font with best weight so far
*	a5	textAttr parameter

_OpenFont:
		movem.l d2-d6/a2-a5,-(a7)
		move.l	a0,a5

		move.l	gb_ExecBase(a6),a0
		FORBID	a0,NOFETCH

		move.l	(a5),a1		; ta_Name(a5) get name

		lea	gb_TextFonts(a6),a0
		LINKEXEGFX	FindName

*	    ;------ check if name exists

		tst.l	d0
		beq.s	openRts
		moveq	#0,d4		; no previous best match

testMatch:
		move.l	d0,a3

		move.l	d0,a0
		suba.l	a1,a1
		JSRLIB	ExtendFont
		tst.l	d0
		beq.s	noBetter

		move.l	a5,a0
		lea	tf_YSize-ta_YSize(a3),a1
		move.l	tf_Extension(a3),a2
		move.l	tfe_Tags(a2),a2
		JSRLIB	WeighTAMatch

	    ;-- Check private return to see if caller asked for
	    ;-- DPI or DotSize (means they are Tag Aware)

		;--	check for request aspect
		tst.w	d1
		bne.s	opf_DPISpecified

	    ;------ otherwise only accept match of a font which has
	    ;------ not been marked a duplicate.

		move.l	tf_Extension(a3),a0
		btst	#TE0B_DUPLICATE,tfe_Flags0(a0)
		beq.s	opf_DPISpecified	; acceptable

		moveq	#00,d0			; not acceptable


opf_DPISpecified:

		cmp.w	#MAXFONTMATCHWEIGHT,d0
		bne.s	notPerfect

		move.l	a3,a4
		bra.s	gotBest

notPerfect:
		cmp.w	d0,d4		; compare with best weight
		bge.s	noBetter	;   not bigger, already have best
		move.w	d0,d4
		move.l	a3,a4

noBetter:
	    ;------ check for any more of this name
		move.l	a3,a0		; start with this font
		move.l	(a5),a1		; ta_Name(a5) get name
		LINKEXEGFX FindName

		tst.l	d0
		BNE_S	testMatch

		move.l	d4,d0		; check if acceptable font
		beq.s	openRts

gotBest:
		addq.w	#1,tf_Accessors(a4)
		move.l	a4,d0


openRts:
		move.l	gb_ExecBase(a6),a0
		PERMIT	a0,NOFETCH

		movem.l (a7)+,d2-d6/a2-a5
		rts


*****i* graphics.library/WeighTAMatch ********************************
*
*   NAME
*	WeighTAMatch -- Get a measure of how well two fonts match. (V36)
*
*   SYNOPSIS
*	weight = WeighTAMatch(reqTextAttr, targetTextAttr, targetTags)
*	D0                    A0           A1              A2
*	D1
*
*	WORD WeighTAMatch(struct TTextAttr *, struct TextAttr *,
*	     struct TagItem *);
*
*	weight = WeighTAMatchTags(reqTextAttr, targetTextAttr, Tag1, ...) (V39)
*
*	WORD WeighTAMatchTags(struct TTextAttr *,struct TextAttr *, ULONG, ...)
*
*   FUNCTION
*	This function provides a metric to describe how well two fonts
*	match.  This metric ranges from MAXFONTMATCHWEIGHT (perfect match)
*	through lower positive numbers to zero (unsuitable match).
*
*   INPUTS
*	reqTextAttr    - the text attributes requested.
*	targetTextAttr - the text attributes of a potential match.
*	targetTags     - tags describing the extended target attributes, or
*	                 zero if not available.
*
*	The [t]ta_Name fields of the [T]TextAttr structures are not used.
*
*	The tags affect the weight only when both a) the reqTextAttr
*	has the FSF_TAGGED bit set in ta_Style, and b) targetTags is
*	not zero.  To fairly compare two different weights, the inclusion
*	or exclusion of tags in the weighing must be the same for both.
*
*   RESULTS
*	weight -- a positive weight describes suitable matches, in
*	      increasing desirability.  MAXFONTMATCHWEIGHT is a perfect
*	      match.  A zero weight is an unsuitable match.
*
*	D1 -- 1-DPI specified in request.  2-DotSize specified in request.
*	      3-DPI and DotSize specified in request.  0-Neither
*	      specified in request.  To be used as a TRUE/FALSE value
*	      for OpenFont() to decide if caller is TAG aware.
*
*   BUGS
*	Prior to V39, this function did not factor differences in
*	font DPI if the Y size of the reqTextAttr, and targetTextAttr
*	matched.  This function also failed to compared DPI if the
*	target did not have a TA_DeviceDPI tag.  As of V39, this function
*	assumes the target has a 1:1 DPI if the TA_DeviceDPI tag is
*	absent.   As of V39, this function will return an imperfect
*	match if the OT_DotSize tag of the source font does not exactly
*	match the request.
*
*	This function is now private; it is meant to be used by diskfont
*	and OpenFont() only.
*
*   SEE ALSO
*	OpenFont()
*
**********************************************************************
*
*   REGISTERS
*	d2	temp
*	d3	temp
*	d4	flag (00-no tags; 01-DPI specified; 02-DOT specified; 03-BOTH)
*	d5	DPI error

*	a0	passed in
*	a1	passed in
*	a2	passed in


WEIGHTSHIFT	EQU	5		; * 32
WEIGHTXMAX	EQU	(1<<WEIGHTSHIFT)

wfmGetDPI:
		beq.s	wfmDefaultTag	; test D1
		move.l	#TA_DeviceDPI,d3
wfmGetTag:

		movem.l	a0-a1/a6,-(a7)
		move.l	d1,a0
		move.l	d0,d1		; set default
		move.l	d3,d0		; tag to search for
		move.l	gb_UtilBase(a6),a6
		JSRLIB	GetTagData
		movem.l	(a7)+,a0-a1/a6
wfmDefaultTag:
		rts

wfmGetDOT:
		moveq	#00,d0		; set default
		tst.l	d1		; test D1
		beq.s	wfmDefaultTag
		move.l	#OT_DotSize,d3
		bra.s	wfmGetTag

_WeighTAMatch:
		movem.l	d2-d5,-(a7)

		;-- check for aspect effect

		moveq	#00,d5		; Set Default DPI error (no-error)

		moveq	#00,d4		; assume DPI/DOT tags not requested

		;--	check for target aspect
		move.l	#$00200020,d0	; default is 1:1
		move.l	a2,d1
		bsr.s	wfmGetDPI
		move.l	d0,d2

		;--	check for request aspect
		btst	#FSB_TAGGED,tta_Style(a0)
		beq.s	wfmGetYComponent
		moveq	#00,d0		; default is DPI tag not specified
		move.l	tta_Tags(a0),d1
		bsr.s	wfmGetDPI

		;--	calculate aspect correction

		move.l	d0,d1
		beq.s	wfmNoAspect	; No DPI specified?

		moveq	#01,d4		; DPI specified flag

		swap	d0
		mulu	d2,d0		; Req XDPI * Target YDPI
		beq.s	wfmNoAspect
		swap	d2
		mulu	d2,d1		; Req YDPI * Target XDPI
		beq.s	wfmNoAspect
		cmp.l	d0,d1

		;--	ratio MUST be <= 1:1
		;--	Divisor must be >= Dividend (BCC is unsigned BGE)

		bcc.s	wfmCalcAspectCorrection
		exg	d0,d1

wfmCalcAspectCorrection:

		;--     Calculate a DPI error value of <= a minimum
		;--	Y error.  A DPI error of 0 means no error, or
		;--	perfect match.  A DPI error of 1 means the
		;--	DPI of the TARGET and REQUEST are very close,
		;--	but not perfect.  A DPI error of WEIGHTMAX is
		;--     possible, but unlikely.  It requires comparing
		;--	a 1:1 DPI font with a 1:33, or 33:1 DPI font.
		;--	In actual use such distorted fonts are rare, and
		;--	we can assume that somewhere on the font list
		;--	a perfect match exists, so the calculated weight
		;--	for this comparison is essentially ignored.
		;
		;--     ratio of <= 1:1 * WEIGHTXMAX:1 MUST be <= WEIGHTXMAX
		;--

		lsl.l	#WEIGHTSHIFT,d0
		movem.l	a0-a1/a6,-(a7)
		move.l	gb_UtilBase(a6),a6
		JSRLIB	UDivMod32
		movem.l	(a7)+,a0-a1/a6

		;--	transform WEIGHTXMAX-0 into 0-WEIGHTXMAX; this value
		;--	is then subtracted from MAXFONTMATCHWEIGHT along with
		;--	some integer multiple of WEIGHTXMAX if there is any
		;--	difference in Y size.
		
		moveq	#WEIGHTXMAX,d5
		sub.l	d0,d5

wfmNoAspect:

		;--	Check OT_DotSize tag for diffs (FSF_TAGGED already
		;--	known to be TRUE)

		move.l	tta_Tags(a0),d1
		bsr.s	wfmGetDOT
		move.l	d0,d2
		beq.s	wfmGetYComponent

		addq.w	#02,d4		; OT_DotSize tag specified in request

		;--	OT_DotSize requested, see if the font has one, and
		;--	compare for exact match

		move.l	a2,d1
		bsr.s	wfmGetDOT
		
                cmp.l	d0,d2		; dotsize must match exactly
		beq.s	wfmGetYComponent

		addq.l	#1,d5		; increment error by fudge factor
					; to indicate imperfect match		
wfmGetYComponent:
		;-- get Y effect
		moveq	#0,d1
		move.w	ta_YSize(a0),d1
		ble.s	wfmFail		; if YSize <= 0 bail out here
		move.w	ta_YSize(a1),d2
		ble.s	wfmFail		; if YSize <= 0 bail out here

		sub.w	d2,d1		; req - target
		bpl.s	wfmApplyAspectCorrection ; target <= req?

		;	target is larger than req
		neg.w	d1
		lsl.l	#2,d1		; (e.g., -128, not -32, for every pixel larger)

wfmApplyAspectCorrection:

		lsl.l	#WEIGHTSHIFT,d1	; (Y diff * MAX) + 0-MAX for X diff
		add.l	d5,d1		

	; start with largest

		move.l	#MAXFONTMATCHWEIGHT,d0
		sub.l	d1,d0

		;-- involve weights for style & flags
		move.w	ta_Style(a0),d2	; and ta_Flags(a0)
		move.w	ta_Style(a1),d3	; and ta_Flags(a1)
		lea	wfmR1T0Weights(pc),a0
		lea	wfmR0T1Weights(pc),a1

		;--	ensure "designed" is set appropriately for target
		move.w	d3,d1
		and.w	#FPF_ROMFONT!FPF_DISKFONT,d1
		beq.s	wfmBitWeights

		or.w	#FPF_DESIGNED,d3

wfmBitWeights:
		moveq	#0,d1
		eor.w	d2,d3

		;--	loop while some bits differ
wfmBitLoop:
		beq.s	wfmDone
		bpl.s	wfmbNext
		tst.w	d2		; check current (MSB) of req
		bpl.s	wfmbR0T1

		;--	    set in request, clear in target
		move.w	(a0),d1
		bra.s	wfmbDiff

		;--	    clear in request, set in target
wfmbR0T1:
		move.w	(a1),d1
wfmbDiff:
		sub.l	d1,d0

wfmbNext:
		addq.l	#2,a0
		addq.l	#2,a1
		add.w	d2,d2
		add.w	d3,d3
		bra.s	wfmBitLoop

wfmDone:
		tst.l	d0
		bge.s	wfmRts

wfmFail:
		moveq	#0,d0		; collapse all unsuitable to 0

wfmRts:
		move.w	d4,d1		; return PRIVATE

		movem.l	(a7)+,d2-d5
		rts



;	range of numbers for size difference:
;	    y differences: 32..<8K would be nice>?
;
;			,color,,extended,italic,bold,underlined
;			,designed,propor,wide,tall,revpath,diskfont,romfont
wfmR1T0Weights:
		dc.w	0,0,0,0,0,16,8,4
		dc.w	0,MAXFONTMATCHWEIGHT,0,0,0,MAXFONTMATCHWEIGHT,0,0

wfmR0T1Weights:
		dc.w	0,0,0,0,0,1024,512,2048
		dc.w	0,0,0,0,0,MAXFONTMATCHWEIGHT,0,0


gfxname:	dc.b	'graphics.library',0

		END
