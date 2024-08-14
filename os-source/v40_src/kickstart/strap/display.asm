**
**	$Filename$
**	$Release: 1.4 $
**	$Revision: 40.0 $
**	$Date: 93/03/08 12:17:57 $
**
**	graphics for user prompt to insert workbench disk
**
**	(C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION strap,code

**	Includes

	INCLUDE "strap.i"

	INCLUDE "exec/memory.i"
	INCLUDE "exec/alerts.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/execbase.i"

	INCLUDE	"devices/timer.i"

	INCLUDE "hardware/custom.i"
	INCLUDE "hardware/dmabits.i"

	INCLUDE "graphics/gfx.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE	"graphics/scale.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "internal/librarytags.i"


**	Exports

	XDEF	SMDisplayOn
	XDEF	SMDisplayTick
	XDEF	SMDisplayOff


**	Imports

	XREF	_custom
	XREF	_intena

	XLVO	AllocMem
	XLVO	CloseDevice
	XLVO	CloseLibrary
	XLVO	OpenDevice
	XLVO	OpenLibrary
	XLVO	TaggedOpenLibrary
	XLVO	RawDoFmt

	XLVO	BitMapScale
	XLVO	BltBitMap
	XLVO	BltTemplate
	XLVO	CBump
	XLVO	CMove
	XLVO	CWait
	XLVO	Draw
	XLVO	Flood
	XLVO	FreeColorMap
	XLVO	FreeCopList
	XLVO	FreeCprList
	XLVO	FreeMem
	XLVO	FreeVPortCopLists
	XLVO	GetColorMap
	XLVO	InitBitMap
	XLVO	InitRastPort
	XLVO	InitTmpRas
	XLVO	InitView
	XLVO	InitVPort
	XLVO	LoadRGB4
	XLVO	LoadView
	XLVO	MakeVPort
	XLVO	Move
	XLVO	MrgCop
	XLVO	SetAPen
	XLVO	SetDrMd
	XLVO	SetRGB4
	XLVO	Text
	XLVO	UCopperListInit
	XLVO	WaitBOVP
	XLVO	WaitTOF

	XLVO	ReadEClock

	XREF	SMAlert

**	Locals

CHECKXL0	EQU	 40	; left edge of top of short check
CHECKX1		EQU	 70	; left edge of bottom of checks
CHECKXR0	EQU	160	; left edge of top of long check
CHECKDX		EQU	 17	; width of check color
CHECKSX		EQU	  2	; width of shadow
CHECKPX		EQU	 25	; distance between check pair starts
CHECKYR0	EQU	 05	; top of long check
CHECKYL0	EQU	 55	; top of short check
CHECKY1		EQU	 79	; bottom of checks

TEXTX		EQU	  0	; origin of version & copyright message
TEXTY		EQU	100	;

DRIVEX		EQU	400
DRIVEY		EQU	 55
DRIVEW		EQU	156
DRIVEH		EQU	 20
DRIVESLOTDX	EQU	  4

DISKDX		EQU	 10
DISKDY		EQU	 25
DISKW		EQU	134
DISKH		EQU	 60
DISKEDGEDX	EQU	  3
DISKBOUNCE	EQU	  5

ANIMDEPTH	EQU	  3
ANIMWIDTH	EQU	((DRIVEW+15)/16)*16
ANIMHEIGHT	EQU	DISKDY+DISKH+DISKBOUNCE
ANIMBYTES	EQU	(ANIMWIDTH/8)*ANIMHEIGHT
ANIMFRAMES	EQU	 16

SCREENDEPTH	EQU	  3
SCREENWIDTH	EQU	DRIVEX+ANIMWIDTH
SCREENHEIGHT	EQU	DRIVEY+ANIMHEIGHT
PLANEBYTES	EQU	(SCREENWIDTH/8)*SCREENHEIGHT
TEMPLATEBYTES	EQU	128

;-- copper list size
	IFLT	CHECKY1-DRIVEY
NUMCOPINS	EQU	((CHECKY1-CHECKYR0+1)*3)+((CHECKY1-DRIVEY)*3)+4
	ENDC
	IFGE	CHECKY1-DRIVEY
NUMCOPINS	EQU	((CHECKY1-CHECKYR0+1)*2)+CHECKY1-CHECKYL0+5
	ENDC

 STRUCTURE	StrapDisplayMem,0
	APTR	sdm_GfxBase
	APTR    sdm_ExecBase
	STRUCT	sdm_VP,vp_SIZEOF
	STRUCT	sdm_V,v_SIZEOF
	STRUCT	sdm_RP,rp_SIZEOF
	STRUCT	sdm_TR,tr_SIZEOF
	STRUCT	sdm_RI,ri_SIZEOF
	STRUCT	sdm_CopperPokeAddrs,SCREENDEPTH*2*4
	UWORD	sdm_CopperDisplayOffset
	STRUCT	sdm_BM1,bm_SIZEOF
	STRUCT	sdm_BM2,bm_SIZEOF
	STRUCT	sdm_ABM,bm_SIZEOF*ANIMFRAMES
	UWORD	sdm_Tick
	STRUCT	sdm_BSA,bsa_SIZEOF
	STRUCT	sdm_UCL,ucl_SIZEOF
	STRUCT	sdm_Timer,IO_SIZE
	STRUCT	sdm_EClock1,EV_SIZE
	STRUCT	sdm_EClock2,EV_SIZE

	IFD	TEMPLATE
	STRUCT	sdm_Template,TEMPLATEBYTES
	ENDC

	STRUCT	sdm_Planes1,PLANEBYTES*SCREENDEPTH
	STRUCT	sdm_Planes2,PLANEBYTES*SCREENDEPTH
	STRUCT	sdm_AnimPlanes,ANIMBYTES*ANIMDEPTH*ANIMFRAMES
	LABEL	StrapDisplayMem_SIZEOF

**	Assumptions

	IFNE	sdm_GfxBase
	FAIL	"sdm_GfxBase not zero, recode"
	ENDC


* ALLOCATIONS FOR Hand
*	a3	_rp
*	a2	_vec
*	a0	_pdata
*	d4	_x
*	d3	_y
*	d3	_newx
*	d5	_newy
*	d5	_value
SMDisplayOn:
		movem.l d2-d7/a2-a4/a6,-(a7)

		;------ get memory
		move.l	#StrapDisplayMem_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CHIP!MEMF_CLEAR,d1
		CALLLVO	AllocMem
	
		move.l	d0,a5
		move.l	a6,sdm_ExecBase(a5)
		tst.l	d0
		bne.s	dyMemOK

		move.l	#AN_BootStrap!AG_NoMemory!AO_GraphicsLib,d0
		bra.s	dyAlert

dyMemOK:
		;------ get timer device

		suba.l	a0,a0		;ODTAG_TIMER (save ROM)
	;;	lea	TDName(pc),a0
		moveq	#UNIT_ECLOCK,d0
		lea	sdm_Timer(a5),a1
		moveq	#0,d1
		CALLLVO	OpenDevice
		tst.l	d0
		beq.s	dyTDOK

		clr.l	sdm_Timer+IO_DEVICE(a5)
		move.l	#AN_BootStrap!AG_OpenDev!AO_TimerDev,d0
		bra.s	dyAlert

dyTDOK:
		;------ get graphics library
		moveq	#OLTAG_GRAPHICS,d0
		CALLLVO	TaggedOpenLibrary

		move.l	d0,(a5)		; sdm_GfxBase(a5)
		bne.s	dyGLOK
 
		move.l	#AN_BootStrap!AG_OpenLib!AO_GraphicsLib,d0

dyAlert:
		bsr	SMAlert
		bra	dyDone

dyGLOK:
		;------	set up to use graphics library
		move.l	d0,a6

		; Init the ViewPort structure
		lea	sdm_VP(a5),a0
		CALLLVO	InitVPort

		;   adjust the viewport offsets to center the display
		move.l	#(((640-SCREENWIDTH)/2)<<16)+SCREENHEIGHT,d0
		sub.w	gb_NormalDisplayRows(a6),d0
		neg.w	d0
		lsr.w	#1,d0
		move.l	d0,sdm_VP+vp_DxOffset(a5) ; and vp_DyOffset

		; Init the View structure
		lea	sdm_V(a5),a1
		CALLLVO	InitView

		; Init the Display BitMaps
		lea	sdm_BM1(a5),a2
		lea	sdm_Planes1(a5),a3
		moveq	#1,d3

dyBMInit:
		move.l	a2,a0
		moveq	#SCREENDEPTH,d0
		move.l	#SCREENWIDTH,d1
		move.l	#SCREENHEIGHT,d2
		CALLLVO	InitBitMap

		; set the pointers to the bitplanes in the bitmap
		lea	bm_Planes(a2),a1
		moveq	#SCREENDEPTH-1,d0
dyBMInitPlanes:
		move.l	a3,(a1)+
		lea	PLANEBYTES(a3),a3
		dbf	d0,dyBMInitPlanes

		lea	bm_SIZEOF(a2),a2
		dbf	d3,dyBMInit


		; Init the Anim BitMaps
		moveq	#ANIMFRAMES-1,d3

dyABMInit:
		move.l	a2,a0
		moveq	#ANIMDEPTH,d0
		move.l	#ANIMWIDTH,d1
		moveq	#ANIMHEIGHT,d2
		CALLLVO	InitBitMap

		; set the pointers to the bitplanes in the bitmap
		lea	bm_Planes(a2),a1
		moveq	#ANIMDEPTH-1,d0
dyABMInitPlanes:
		move.l	a3,(a1)+
		lea	ANIMBYTES(a3),a3
		dbf	d0,dyABMInitPlanes

		lea	bm_SIZEOF(a2),a2
		dbf	d3,dyABMInit

		; Get a Color Map
		moveq	#20,d0
		CALLLVO	GetColorMap
		move.l	d0,sdm_VP+vp_ColorMap(a5)

		;------ set top screen colors
		lea	sdm_VP(a5),a0
		lea	colorMap(pc),a1
		moveq	#20,d0
		CALLLVO	LoadRGB4

		; Init the UCopList
		lea	sdm_UCL(a5),a0
		move.l	a0,sdm_VP+vp_UCopIns(a5)
		move.l	#NUMCOPINS,d0
		CALLLVO	UCopperListInit

		; InitRastPort
		
		lea	sdm_RP(a5),a3		; pointer to RastPort in a3
		move.l	a3,a1
		CALLLVO	InitRastPort

		; InitTmpRas
		lea	sdm_TR(a5),a0		; pointer to TmpRas
		move.l	a5,a1			; pointer to buffer
		add.l	#sdm_Planes2,a1
		move.l	#PLANEBYTES,d0		; size of the buffer
		CALLLVO	InitTmpRas

		; set RasInfo to point to the BitMap
		lea	sdm_BM1(a5),a0
		move.l	a0,sdm_RI+ri_BitMap(a5)

		; set the RastPort to point to the BitMap & TmpRas
		move.l	a0,sdm_RP+rp_BitMap(a5)
		lea	sdm_TR(a5),a0
		move.l	a0,sdm_RP+rp_TmpRas(a5)

		; fill in the ViewPort structure
		move.w	#SCREENHEIGHT,sdm_VP+vp_DHeight(a5)
		move.w	#SCREENWIDTH,sdm_VP+vp_DWidth(a5)
		lea	sdm_RI(a5),a0
		move.l	a0,sdm_VP+vp_RasInfo(a5)
		move.w	#V_HIRES,sdm_VP+vp_Modes(a5)

		move.l	a3,a1
		moveq	#RP_JAM1,d0
		CALLLVO	SetDrMd

		lea	VectorArray(pc),a2
*
*   x=
*   -1	subroutine return or call
*	negative y is subroutine return
*	positive y is followed by relative call address
*   -2	end of vector list or pen color and move
*	negative y is end of vector list
*	positive y is pen and area outline color followed by move-to offset
*   -3	absolute position or move-to
*	negative y is followed by absolute coordinate words
*	positive y is followed by move-to offset
*   -4	flood-to
*	y is flood mode followed by flood-to offset
*   -5  text (from V39 exec.library)
*	y is magic constant for TaggedOpenLibrary()
*   -6	masked-template
*	y is mask, followed by position offset bytes, followed by width &
*	height bytes, followed by words of template data
*   >0	draw-to
*	x, y are draw-to offsets
*

dyNextVecPair:
		moveq	#0,d0
		move.b	(a2)+,d0	; newx
		moveq	#0,d1
		move.b	(a2)+,d1	; newy
		move.b	d0,d2
		addq.b	#1,d2		; cmp.b #-1,d0

	IFD	SUBROUTINE
		bne.s	dyCheckPen
		tst.b	d1
		bmi.s	dyEndSub

		;------ subroutine call to relative call address
		move.w	(a2)+,d0	; get relative address
		move.l	a2,-(a7)	; save return address
		lea	-2(a2,d0.w),a2	; get absolute address
		bra.s	dyNextVecPair
dyEndSub:
		;------ subroutine return
		move.l	(a7)+,a2	; restore return address
		bra.s	dyNextVecPair

dyCheckPen:
	ENDC
		addq.b	#1,d2		; cmp.b #-2,d0
		bne.s	dyCheckMove
		tst.b	d1
		bmi.s	dyRenderRevision
		;------ ^ that was end of vector list

		;------ set pen color
		move.b	d1,rp_AOLPen(a3)
		move.l	a3,a1
		move.l	d1,d0
		CALLLVO	SetAPen
		bra.s	dyRelMove

dyCheckMove:
		addq.b	#1,d2		; cmp.b #-3,d0
		bne.s	dyCheckFlood
		tst.b	d1
		bmi.s	dySetAbs

dyRelMove:
		;------	move to offset
		bsr.s	dyNextXY

		CALLLVO	Move
		bra.s	dyNextVecPair

		;------ set absolute coordinate origin
dySetAbs:
		move.w	(a2)+,d3
		move.w	(a2)+,d4
		bra.s	dyNextVecPair

dyNextXY:
		moveq	#0,d0
		move.b	(a2)+,d0
		moveq	#0,d1
		move.b	(a2)+,d1
dyAdjustXY:
		add.w	d3,d0
		add.w	d4,d1
		move.l	a3,a1
		rts

dyCheckFlood:
		addq.b	#1,d2		; cmp.b #-4,d0
		bne.s	dyCheckText

		move.l	d1,d2

		;------	flood to offset
		bsr.s	dyNextXY

		CALLLVO	Flood
		bra.s	dyNextVecPair

		;------ draw to offset
dyRelDraw:
		bsr.s	dyAdjustXY
		CALLLVO	Draw
		bra.s	dyNextVecPair

		;------ text (As of V39, all sits in ExecBase)

dyCheckText:
		addq.b	#1,d2		; cmp.b #-5,d0
		bne.s	dyCheckTemplate

		move.b	d1,d0		; for TaggedOpenLibrary
		ext.w	d0		; byte -> longword
		ext.l	d0
		move.l	sdm_ExecBase(a5),a6
		CALLLVO	TaggedOpenLibrary
		move.l	(a5),a6		; sdm_GfxBase(a5)

		tst.l	d0
		beq	dyNextVecPair	; GfxBase in A6 restored above

		move.l	d0,a1		; count length
		move.l	d0,a0		; string
		moveq	#00,d0
dyGetTextLength:
		tst.b	(a1)+
		beq.s	dyGotTextLength
		addq.w	#1,d0
		bra.s	dyGetTextLength
dyGotTextLength:
		move.l	a3,a1		; rastport
		CALLLVO	Text
		bra	dyNextVecPair


dyCheckTemplate:
	IFD	TEMPLATE
		addq.b	#1,d2		; cmp.b #-6,d0
	ENDC
		bne.s	dyRelDraw

	IFD	TEMPLATE
		;------ masked template
		move.b	d1,rp_Mask(a3)
		bsr.s	dyNextXY

		movem.l	d3-d4,-(a7)	; save absolute loc
		moveq	#0,d2
		move.b	(a2)+,d4
		moveq	#0,d5
		move.b	(a2)+,d5
		move.w	d4,d2
		mulu	d5,d2
		lea	sdm_Template(a5),a0
		move.l	a0,a4
		bra.s	dyTCopyDBF
dyTCopy:
		move.w	(a2)+,(a4)+
dyTCopyDBF:
		dbf	d2,dyTCopy

		move.l	d0,d2
		move.l	d1,d3
		moveq	#0,d0
		add.w	d4,d4
		move.l	d4,d1
		lsl	#3,d4
		move.l	a3,a1
		CALLLVO	BltTemplate
		movem.l	(a7)+,d3-d4
		bra	dyNextVecPair
	ENDC


;-------
dyRenderRevision:
		move.l	sdm_ExecBase(a5),a6

		;-- generate revision string
		lea	revisionFormat(pc),a0	; %02d.%03d)
		;--	generate rom revision address from current address
;		move.l	a0,d0			
;		and.l	#$fff80000,d0
;		or.w	    #$000c,d0
;		move.l	d0,a1

		move.w	SoftVer(a6),-(a7)
		move.w	LIB_VERSION(a6),-(a7)
		move.l	a7,a1

			;--	get rest of RawDoFmt parameters
			lea	dyPutChProc(pc),a2
			subq.l	#8,a7
				move.l	a7,a3
				CALLLVO	RawDoFmt

				moveq	#6,d0
				move.l	a7,a0
				lea	sdm_RP(a5),a3
				move.l	a3,a1
				move.l	(a5),a6		; sdm_GfxBase(a5)
				CALLLVO	Text
			addq.l	#8,a7

		addq.l	#4,a7
		bra.s	dyRenderCheck

dyPutChProc:
		move.b	d0,(a3)+
		rts


;------	dyDrawCheck
;
;	d0	pen
;	d2	x
;	d3	y
;	a3	RastPort
;
dyMoveYC:
		add.w	d2,d0
		move.w	d3,d1
		move.l	a3,a1
		CALLLVO	Move
		rts

dyDrawYC:
		add.w	d2,d0
		move.w	d3,d1
		move.l	a3,a1
		CALLLVO	Draw
		rts
		
dyDrawCheck:
		move.l	a3,a1
		CALLLVO	SetAPen
		moveq	#0,d0
		bsr.s	dyMoveYC
		moveq	#CHECKDX,d0
		bsr.s	dyDrawYC
		moveq	#CHECKPX,d0
		bsr.s	dyMoveYC
		moveq	#CHECKPX+CHECKDX+1,d0
		bsr.s	dyDrawYC
		moveq	#6,d0
		move.l	a3,a1
		CALLLVO	SetAPen
		moveq	#CHECKPX+CHECKDX+CHECKSX,d0
		bsr.s	dyDrawYC
		moveq	#CHECKDX+1,d0
		bsr.s	dyMoveYC
		moveq	#CHECKDX+CHECKSX,d0
		bsr.s	dyDrawYC
		rts


		;-- render the checkmark and build the copper list
dyRenderCheck:
		move.l	a5,-(a7)
		lea	sdm_UCL(a5),a2		; user copper list
		lea	dyColor4Table(pc),a4	; long check color table
		lea	dyColor5Table(pc),a5	; short check color table
		moveq	#CHECKYR0,d3		; start @ top of long check
		move.w	(a4)+,d4		; initial long check color
		move.w	(a5)+,d5		; initial short check color
		move.w	(a4)+,d6		; current counter for swizzle
		move.w	(a5)+,d7		;   long, then short

dyCheckLoop:
		;-- wait for beginning of line
		move.l	d3,d0
		moveq	#0,d1			; line start
		move.l	a2,a1
		CALLLVO	CWait
		bsr	dyucCBump

		;-- check to render short check
		cmp.w	#CHECKYL0,d3
		blt.s	dyRightCheck

		;--	render check
		moveq	#CHECKY1,d2
		sub.w	d3,d2
		MULS	#CHECKXL0-CHECKX1,d2
		DIVS	#CHECKY1-CHECKYL0,d2
		add.w	#CHECKX1,d2
		moveq	#5,d0
		bsr.s	dyDrawCheck

		;--	swizzle color 5
		moveq	#5*2,d0
		move.w	d5,d1
		bsr	dyucSetXColor
		subq.w	#1,d7
		bne.s	dyAddLeftDelta
		addq.l	#2,a5
		move.w	(a5)+,d7
dyAddLeftDelta:
		add.w	(a5),d5

		;-- render long check
dyRightCheck:
		moveq	#CHECKY1,d2
		sub.w	d3,d2
		MULS	#CHECKXR0-CHECKX1,d2
		DIVS	#CHECKY1-CHECKYR0,d2
		add.w	#CHECKX1,d2
		moveq	#4,d0
		bsr	dyDrawCheck

		;--	swizzle color 4
		moveq	#4*2,d0
		move.w	d4,d1
		bsr	dyucSetXColor
		subq.w	#1,d6
		bne.s	dyAddRightDelta
		addq.l	#2,a4
		move.w	(a4)+,d6
dyAddRightDelta:
		add.w	(a4),d4

		cmp.w	#CHECKY1,d3
		beq.s	dyRestoreDriveColors
		cmp.w	#DRIVEY,d3
		blt.s	dyNextCheckLine

		;-- set color at middle of line
dyRestoreDriveColors:
		move.l	d3,d0
		move.w	#$98,d1			; line middle
		move.l	a2,a1
		CALLLVO	CWait
		bsr	dyucCBump
		moveq	#4*2,d0
		move.w	colorMap+(4*2)(pc),d1
		bsr	dyucSetColor		; set color 4
		moveq	#5*2,d0
		move.w	colorMap+(5*2)(pc),d1
		bsr	dyucSetColor		; set color 5

dyNextCheckLine:
		addq	#1,d3
		cmp.w	#CHECKY1,d3
		ble	dyCheckLoop


		;-- done building checkmark
		move.l	(a7)+,a5
		move.l	#10000,d0
		move.l	#255,d1
		move.l	a2,a1
		CALLLVO	CWait			; CEND
		bsr	dyucCBump



		;-- copy display into alternate buffer
		movem.w	screenCopy(pc),d0-d7
		lea	sdm_BM1(a5),a0
		lea	sdm_BM2(a5),a1
		CALLLVO	BltBitMap

		;-- copy drive into anim frames
		moveq	#ANIMFRAMES-1,d7
		lea	sdm_ABM(a5),a2
		lea	animDiskBottomDelta(pc),a3

dyAnimDrive:
		swap	d7
		movem.w	animDriveCopy(pc),d0-d6
		move.w	#$ffff,d7
		lea	sdm_BM1(a5),a0
		move.l	a2,a1
		CALLLVO	BltBitMap

		movem.w	animDiskCopy(pc),d0-d5
		add.w	(a3)+,d3
		ble.s	dyadNext
		lea	sdm_BM1(a5),a0
		move.l	a2,a1
		CALLLVO	BltBitMap

dyadNext:
		lea	bm_SIZEOF(a2),a2
		swap	d7
		dbf	d7,dyAnimDrive

		;-- scale disk into anim frames
		move.w	#DRIVEX+DISKDX,sdm_BSA+bsa_SrcX(a5) ; source origin
		move.w	#DRIVEY+DISKDY,sdm_BSA+bsa_SrcY(a5)
		move.w	#DISKW,d0
		moveq	#DISKH-DISKEDGEDX-1,d1
		move.w	d0,sdm_BSA+bsa_SrcWidth(a5)	; source size
		move.w	d1,sdm_BSA+bsa_SrcHeight(a5)
		move.w	d0,sdm_BSA+bsa_XSrcFactor(a5)	; denominators of scale
		move.w	d1,sdm_BSA+bsa_YSrcFactor(a5)
		move.w	#DISKDX,sdm_BSA+bsa_DestX(a5)	; destination origin
		move.w	d0,sdm_BSA+bsa_XDestFactor(a5)	; numerators of scale
		lea	sdm_BM1(a5),a1
		move.l	a1,sdm_BSA+bsa_SrcBitMap(a5)	; source BitMap
		lea	sdm_ABM(a5),a1
		move.l	a1,sdm_BSA+bsa_DestBitMap(a5)	; destination BitMap
		clr.l	sdm_BSA+bsa_Flags(a0)		; reserved.

		moveq	#ANIMFRAMES-1,d2
dyAnimDisk:
		moveq	#ANIMFRAMES-1,d1
		sub.w	d2,d1
		add.w	d1,d1
		lea	animDiskTop(pc),a0
		add.w	d1,a0
		move.w	(a0)+,d0
		blt.s	dyNextAnimDisk
		move.w	d0,sdm_BSA+bsa_DestY(a5)

		lea	animDiskBottomDelta(pc),a1
		add.w	d1,a1
		moveq	#DISKDY+DISKH-DISKEDGEDX,d1
		add.w	(a1)+,d1
		sub.w	d0,d1
		move.w	d1,sdm_BSA+bsa_YDestFactor(a5)
		lea	sdm_BSA(a5),a0
		CALLLVO	BitMapScale
dyNextAnimDisk:
		add.l	#bm_SIZEOF,sdm_BSA+bsa_DestBitMap(a5)
		dbf	d2,dyAnimDisk


		; point the View to the ViewPort
		lea	sdm_V(a5),a0		; pointer to the View
		lea	sdm_VP(a5),a1
		move.l	a1,sdm_V+v_ViewPort(a5)
		CALLLVO	MakeVPort

		lea	sdm_V(a5),a1		; pointer to the View
		CALLLVO	MrgCop

		lea	sdm_V(a5),a1		; pointer to the View
		CALLLVO	LoadView

		CALLLVO	WaitTOF
		move.w	#DMAF_SETCLR+DMAF_RASTER,_custom+dmacon	; ON_DISPLAY

		; find bitplane pointers in View, don't bother testing that
		; they're not there
		move.l	sdm_VP+vp_DspIns(a5),a1
		move.l	cl_CopLStart(a1),a1
		lea.l	sdm_CopperPokeAddrs(a5),a2
		moveq	#(SCREENDEPTH*2)-1,d0
		move.w	#bplpt,d1
dyCopLoop:
		move.l	a1,a0
dyCopSearch:
		cmp.w	(a0)+,d1
		bne.s	dyCopSearch
		move.l	a0,(a2)+
		addq.w	#2,d1
		dbf	d0,dyCopLoop

		; check if bitplane pointers in View contain prefetch
		move.l	sdm_CopperPokeAddrs+4(a5),a0
		move.w	(a0),d0
		sub.w	sdm_BM1+bm_Planes+2(a5),d0
		move.w	d0,sdm_CopperDisplayOffset(a5)	; always 0 or -2


dyDone:
		movem.l (a7)+,d2-d7/a2-a4/a6
		rts	

dyucSetXColor:

		;			.   .   .   .   +   .   .   .   
		;-- start with		?????????????????rrrr?gggg?bbbb?	
		ror.l	#5,d1		bbbb??????????????????rrrr?gggg?
		ror.w	#5,d1		bbbb????????????gggg???????rrrr?
		lsr.b	#1,d1		bbbb????????????gggg????0???rrrr
		rol.w	#4,d1		bbbb????????????????0???rrrrgggg
		rol.l	#4,d1		????????????????0???rrrrggggbbbb
		and.w	#$0fff,d1			0000rrrrggggbbbb
dyucSetColor:
		move.l	a2,a1
		add.l	#$DFF180,d0		; color n
		CALLLVO	CMove
dyucCBump:
		move.l	a2,a1
		CALLLVO	CBump
		rts



SMDisplayTick:
		movem.l	d2-d7/a2-a3/a6,-(a7)
		;-- check for valid environment
		move.l	a5,d0
		beq	dtDone

		;-- check if time to swap buffers
		move.l	sdm_Timer+IO_DEVICE(a5),d0
		beq	dtDone
		move.l	d0,a6
		lea	sdm_EClock2(a5),a0
		CALLLVO	ReadEClock
		divu	#1000,d0		; ticks/msec

		movem.l	sdm_EClock2(a5),d2/d3	; EV_HI, EV_LO
	
		cmp.l	sdm_EClock1+EV_HI(a5),d2
		bcs	dtDone
		cmp.l	sdm_EClock1+EV_LO(a5),d3
		bcs	dtDone

		;-- start timer for next offscreen buffer
		move.w	sdm_Tick(a5),d4
		move.w	d4,d5
		addq.w	#1,d5
		cmp.w	#ANIMFRAMES,d5
		blt.s	dtSetNextTick
		moveq	#0,d5
dtSetNextTick:
		move.w	d5,sdm_Tick(a5)
		add.w	d4,d4
		lea	animTicks(pc),a1
		add.w	d4,a1

		moveq	#0,d6
		move.w	(a1),d7
		mulu	d0,d7
		add.l	d7,d3
		addx.l	d6,d2
		movem.l	d2/d3,sdm_EClock1(a5)

		;-- check again for valid environment
		move.l	(a5),d0		; sdm_GfxBase(a5)
		beq.s	dtDone
		move.l	d0,a6

		;-- swap buffers
		move.l	sdm_ExecBase(a5),a0
		DISABLE	a0,NOFETCH

		CALLLVO	WaitTOF		; breaks Disable w/ Wait

		lea	sdm_BM1(a5),a0
		lea	sdm_BM2(a5),a3
		btst	#1,d4
		beq.s	dtSwapBuffers
		lea	bm_SIZEOF(a0),a0
		lea	-bm_SIZEOF(a3),a3
dtSwapBuffers:
		lea	bm_Planes(a0),a0
		lea	sdm_CopperPokeAddrs(a5),a1
		moveq	#SCREENDEPTH-1,d0
dtPokeCopper:
		move.l	(a0)+,a2
		add.w	sdm_CopperDisplayOffset(a5),a2
		move.l	a2,d1
		swap	d1
		move.l	(a1)+,a2
		move.w	d1,(a2)
		swap	d1
		move.l	(a1)+,a2
		move.w	d1,(a2)
		dbf	d0,dtPokeCopper

		move.l	sdm_ExecBase(a5),a0
		ENABLE	a0,NOFETCH

		;-- render new offscreen buffer
		lea	sdm_VP(a5),a0
		CALLLVO	WaitBOVP
		mulu	#bm_SIZEOF/2,d4
		lea	sdm_ABM(a5),a0
		add.l	d4,a0
		move.l	a3,a1
		movem.w	animCopy(pc),d0-d7
		CALLLVO	BltBitMap

dtDone:
		movem.l	(a7)+,d2-d7/a2-a3/a6
		rts


SMDisplayOff:
		move.l	a6,-(a7)
		move.l	a5,d0
		beq.s	dnDone
		move.l	(a5),d0		; sdm_GfxBase(a5)
		beq.s	dnCloseTimer
		move.l	d0,a6

		move.w	#DMAF_RASTER,_custom+dmacon		; OFF_DISPLAY
		moveq	#0,d0

		move.l	d0,a1
		CALLLVO	LoadView	  ; LoadView(0)

		move.l	sdm_V+v_LOFCprList(a5),a0
		CALLLVO	FreeCprList

		move.l	sdm_UCL+ucl_FirstCopList(a5),a0
		CALLLVO	FreeCopList
		clr.l	sdm_VP+vp_UCopIns(a5)

		move.l	sdm_VP+vp_ColorMap(a5),a0
		CALLLVO	FreeColorMap

		lea	sdm_VP(a5),a0
		CALLLVO	FreeVPortCopLists ; The viewport has now gone

		move.l	a6,a1		; GfxBase
		move.l	sdm_ExecBase(a5),a6
		CALLLVO	CloseLibrary

dnCloseTimer:
		move.l	sdm_ExecBase(a5),a6
		tst.l	sdm_Timer+IO_DEVICE(a5)
		beq.s	dnFreeSDM
		lea	sdm_Timer(a5),a1
		CALLLVO	CloseDevice

dnFreeSDM:
		move.l	a5,a1
		move.l	#StrapDisplayMem_SIZEOF,d0
		CALLLVO	FreeMem

dnDone:
		move.l	(a7)+,a6
		rts


revisionFormat:
		dc.b	'%02d.%03d',0
		ds.w	0

;------ Swizzle Color Tables
;
;	word of initial color, followed by
;	swizzle count / increment pairs
COLOR5	MACRO
		dc.w	(\1<<10)+(\2<<5)+\3
	ENDM

dyColor4Table:
		COLOR5	$1e,0,$4	; $  f  0  4
		dc.w	5
		COLOR5	0,0,-1		;         -.5-> f  0  0 : 5
		dc.w	31
		COLOR5	0,1,0		;      +.5   -> f  f  0 : 31
		dc.w	5,0		;            -> f  f  0 : 5
		dc.w	30
		COLOR5  -1,0,0		;   -.5      -> 0  f  0 : 30
		dc.w	15
		COLOR5	0,0,4		;         +2 -> 0  f  ?

dyColor5Table:
		COLOR5	$0,$7,$1f	; $  0  3  f
		dc.w	8
		COLOR5	0,1,-1		;      +.5-.5-> 0  7  c : 8
		dc.w	8
		COLOR5	0,2,0		;      +1    -> 0  f  f : 8
		dc.w	15
		COLOR5	0,0,-1		;         -.5-> 0  f  0 : 15

colorMap:
		dc.w	$0414		; background	bart likes 679
		dc.w	$0ea8		; drive tan, text
		dc.w	$0a76		; drive tan shadow
		dc.w	$0000		; drive black

		dc.w	$0238		; disk blue, checkmark long side
		dc.w	$0226		; disk blue shadow, checkmark short side
		dc.w	$0987		; disk metal, checkmark shadow
		dc.w	$0fff		; disk label white

		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	$0414		; cursor color 1
		dc.w	$0414		; cursor color 2
		dc.w	$0414		; cursor color 3

VectorArray:
		dc.b	-3,-1		; drive
		dc.w	DRIVEX,DRIVEY
		dc.b	-2,1		; drive tan
		dc.b	0,0
		dc.b	155,0
		dc.b	155,18
		dc.b	0,18
		dc.b	0,0
		dc.b	-3,0
		dc.b	7,2
		dc.b	148,2
		dc.b	148,7
		dc.b	7,7
		dc.b	7,2
		dc.b	-4,0
		dc.b	1,1
		dc.b	-2,2		; drive tan shadow
		dc.b	118,14
		dc.b	118,11
		dc.b	142,11
		dc.b	142,12
		dc.b	-2,3		; drive light off
		dc.b	25,14
		dc.b	32,14
		dc.b	-4,1		; drive slot
		dc.b	8,3
		dc.b	-3,-1		; disk
		dc.w	DRIVEX+DISKDX,DRIVEY+DISKDY
		dc.b	-2,5		; disk shadow
		dc.b	18,0
		dc.b	121,1
		dc.b	121,59
		dc.b	4,59
		dc.b	4,1
		dc.b	18,1
		dc.b	-4,1
		dc.b	18,2
		dc.b	-2,4		; disk blue
		dc.b	0,2
		dc.b	5,0
		dc.b	13,0
		dc.b	13,20
		dc.b	18,21
		dc.b	99,21
		dc.b	104,20
		dc.b	104,1
		dc.b	107,0
		dc.b	124,0
		dc.b	133,4
		dc.b	133,59
		dc.b	122,59
		dc.b	122,26
		dc.b	117,25
		dc.b	18,25
		dc.b	13,26
		dc.b	13,59
		dc.b	0,59
		dc.b	0,2
		dc.b	-3,0		; arrow shadow hole
		dc.b	6,2
		dc.b	2,6
		dc.b	5,6
		dc.b	5,8
		dc.b	8,8
		dc.b	8,6
		dc.b	11,6
		dc.b	7,2
		dc.b	-3,0		; write protect hole
		dc.b	3,53
		dc.b	10,53
		dc.b	10,56
		dc.b	3,56
		dc.b	3,53
		dc.b	-4,0		; flood disk blue
		dc.b	1,3
		dc.b	-2,7		; label color
		dc.b	16,28
		dc.b	19,27
		dc.b	116,27
		dc.b	119,28
		dc.b	119,59
		dc.b	16,59
		dc.b	16,28
		dc.b	-4,0		; flood label
		dc.b	17,29
		dc.b	-2,6		; disk metal
		dc.b	37,20
		dc.b	34,19
		dc.b	34,0
		dc.b	103,0
		dc.b	-3,0		; metal hole
		dc.b	74,3
		dc.b	89,3
		dc.b	89,19
		dc.b	74,19
		dc.b	74,3
		dc.b	-4,1
		dc.b	35,1

		dc.b	-3,-1		; copyright message
		dc.w	TEXTX,TEXTY
		dc.b	-2,1
		dc.b	0,0
		dc.b	-5,OLTAG_COPYRIGHT5
		dc.b	-3,0
		dc.b	0,12
		dc.b	-5,OLTAG_COPYRIGHT2
		dc.b	-3,0
		dc.b	0,24
		dc.b	-5,OLTAG_COPYRIGHT3
		dc.b	-3,0
		dc.b	0,36
		dc.b	-5,OLTAG_COPYRIGHT4
		dc.b	-3,0		; revision string
		dc.b	100,0

		dc.b	-2,-1		; end of display

screenCopy:		; d0-d7
		dc.w	0,0,0,0,SCREENWIDTH,SCREENHEIGHT,$c0,$ff

animDriveCopy:		; d0-d6
		dc.w	DRIVEX,DRIVEY,0,0,ANIMWIDTH,DRIVEH,$c0

animDiskCopy:		; d0-d2/d4-d5
		dc.w	DRIVEX+DISKDX,DRIVEY+DISKDY+DISKH-DISKEDGEDX
		dc.w	DISKDX,DISKDY+DISKH-DISKEDGEDX,DISKW,DISKEDGEDX

BOTTOMFACTOR	EQU	DRIVESLOTDX-(DISKDY+DISKH-DISKEDGEDX)
animDiskBottomDelta:
		dc.w	0
		dc.w	DISKBOUNCE
		dc.w	(01*BOTTOMFACTOR+12*DISKBOUNCE)/13
		dc.w	(02*BOTTOMFACTOR+11*DISKBOUNCE)/13
		dc.w	(03*BOTTOMFACTOR+10*DISKBOUNCE)/13
		dc.w	(04*BOTTOMFACTOR+09*DISKBOUNCE)/13
		dc.w	(05*BOTTOMFACTOR+08*DISKBOUNCE)/13
		dc.w	(06*BOTTOMFACTOR+07*DISKBOUNCE)/13
		dc.w	(07*BOTTOMFACTOR+06*DISKBOUNCE)/13
		dc.w	(08*BOTTOMFACTOR+05*DISKBOUNCE)/13
		dc.w	(09*BOTTOMFACTOR+04*DISKBOUNCE)/13
		dc.w	(10*BOTTOMFACTOR+03*DISKBOUNCE)/13
		dc.w	(11*BOTTOMFACTOR+02*DISKBOUNCE)/13
		dc.w	(12*BOTTOMFACTOR+01*DISKBOUNCE)/13
		dc.w	BOTTOMFACTOR
		dc.w	-(DISKDY+DISKH+1)	; (no image)

animDiskTop:
		dc.w	DISKDY
		dc.w	DISKDY+DISKBOUNCE
		dc.w	DRIVESLOTDX+((12*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((11*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((10*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((09*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((08*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((07*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((06*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((05*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((04*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((03*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((02*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	DRIVESLOTDX+((01*(DISKDY+DISKBOUNCE-DRIVESLOTDX))/13)
		dc.w	-1
		dc.w	-1

animCopy:		; d0-d7
		dc.w	0,0,DRIVEX,DRIVEY,ANIMWIDTH,ANIMHEIGHT,$c0,$ff

animTicks:
		;	ticks at 1KHz
		dc.w	1000,2000,160,65,65,65,65,65
		dc.w	65,65,65,65,65,65,65,145

	END
