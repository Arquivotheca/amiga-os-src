**
**	$Id: init.asm,v 1.28 93/02/18 16:26:32 darren Exp $
**
**	CDTV CD+G -- init.asm (initialize mem, data, etc)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE	"cdgprefs.i"
		INCLUDE	"cdgs_hw.i"
		INCLUDE	"cdgs:include/cd/cd.i"
		INCLUDE	"cdgs:src/cd/cdprivate.i"
		INCLUDE	"debug.i"

		INCLUDE "devices/serial.i"	;use serial name macro
** Exports

		XDEF	_CDGInit
		XDEF	_CDGFree
** Imports

		XREF	cdgname			;library.asm

		XREF	CDGPackInt		;interrupts.asm
		XREF	CDGVertInt

		XREF	CDGInitMidiOut		;midi.asm
		XREF	CDGFreeMidiOut

		XREF	_intena			;amiga.lib

		XREF	_LVOPermit		;required for PERMIT macro

** Assumptions

	IFNE	cdd_BitPlane1_1-cdd_BitPlane1_0-4
	FAIL	"cdd_BitPlane1_1 does not follow cdd_BitPlane1_0; recode!"
	ENDC

	IFNE	cdd_BitPlane1_2-cdd_BitPlane1_1-4
	FAIL	"cdd_BitPlane1_2 does not follow cdd_BitPlane1_1; recode!"
	ENDC

	IFNE	cdd_BitPlane1_3-cdd_BitPlane1_2-4
	FAIL	"cdd_BitPlane1_3 does not follow cdd_BitPlane1_2; recode!"
	ENDC

	IFNE	cdd_BitPlane1_Extra-cdd_BitPlane1_3-4
	FAIL	"cdd_Bitplane1_Extra does not follow cdd_Bitplane1_3; recode!"
	ENDC

	IFNE	cdd_BitPlane2_1-cdd_BitPlane2_0-4
	FAIL	"cdd_BitPlane2_1 does not follow cdd_BitPlane2_0; recode!"
	ENDC

	IFNE	cdd_BitPlane2_2-cdd_BitPlane2_1-4
	FAIL	"cdd_BitPlane2_2 does not follow cdd_BitPlane2_1; recode!"
	ENDC

	IFNE	cdd_BitPlane2_3-cdd_BitPlane2_2-4
	FAIL	"cdd_BitPlane2_3 does not follow cdd_BitPlane2_2; recode!"
	ENDC

	IFNE	cdd_BitPlane2_Extra-cdd_BitPlane2_3-4
	FAIL	"cdd_Bitplane2_Extra does not follow cdd_Bitplane2_3; recode!"
	ENDC

	IFNE	cdd_CDIO
	FAIL	"cdd_CDIO not first in CDGData struct; recode!"
	ENDC

	; InitStruct() checking - since cdd_PacketData may be set
	; quite large, its possible to have a CDGData area larger
	; than 64K!!!  Actually this is not a problem, but the
	; programmer needs to check that there are no elements after
	; cdd_PacketData which need to be inited (even if inited to 0
	; since I let InitStruct() clear the memory, and not AllocMem()).
	;
	; InitStruct() uses a DBF loop, which is a WORD size limited
	; loop, to clear memory.
	;

	IFGE	CDGData_SIZEOF-(64*1024)
		FAIL	"CDGData too large for InitStruct(); recode"
	ENDC

	; since I copy this bits, I need to make sure they are in the
	; same bit positions

	IFNE	EF3B_NOECC-CDGB_NOECC
	FAIL	"EF3B_NOECC != CDGB_NOECC; recode!"
	ENDC

	IFNE	EF3B_NOMIDI-CDGB_NOMIDI
	FAIL	"EF3B_NOMIDI != CDGB_NOMIDI; recode!"
	ENDC

	IFNE	EF3B_ALTSPRITES-CDGB_ALTSPRITES
	FAIL	"EF3B_ALTSPRITES != CDGB_ALTSPRITES; recode!"
	ENDC

** Equates


	;GACK!!! CDTV has no idea how big my view is, hence no idea
	;how to center me in PAL mode in Y or X.  X is close enough,
	;but need something for Y.  Should really be done in the
	;player module, except the owner of that module is not
	;available.  25 is based on an NTSC screen, minus what we
	;already subtract for 1/2 font height


PAL_KLUDGE	EQU	25

		SECTION	cdg

*****i* cdg.library/_CDGFree ******************************************
*
*   NAME
*	_CDGFree -- Free data structure
*   
*   SYNOPSIS
*	VOID _CDGFree( CDGBase )
*	                 A5
*
*	VOID _CDGDispatch( struct CDGBase * );
*
*   FUNCTION
*	Free all memory, etc., allocated by _CDGInit()
*
*   RETURNS
*	None.
*
*   NOTES
*	Should only be called if _CDGInit() succeeded!!!
*
***********************************************************************

_CDGFree:
	PRINTF	DBG_ENTRY,<'CDG--_CDGFree($%lx)'>,A5

		movem.l	d2-d7/a2-a6,-(sp)

		move.l	cdg_BaseData(a5),d0
		beq	CDGInit_FAIL0

		move.l	d0,a4
		bra	CDGInit_FREEDATA

*****i* cdg.library/_CDGInit ******************************************
*
*   NAME
*	_CDGInit -- Initialize data module
*   
*   SYNOPSIS
*	result = _CDGInit( CDGBase, CDGPrefs )
*	D0                 A5		A4
*
*	struct CDGData *_CDGDispatch( struct CDGBase *, struct CDGPrefs * );
*
*   FUNCTION
*	Allocate memory, initialize data, etc.
*
*   RETURNS
*	result - Pointer to initialized CDGData structure, or NULL
*	indicating failure.
*
***********************************************************************
_CDGInit:

	PRINTF	DBG_ENTRY,<'CDG--_CDGInit($%lx,$%lx)'>,A5,A4

		movem.l	d2-d7/a2-a6,-(sp)

		move.l	a4,a3			;cache

		move.l	cdg_ExecLib(a5),a6
		move.l	#CDGData_SIZEOF,d0

		move.l	#MEMF_PUBLIC,d1

	PRINTF	DBG_OSCALL,<'CDG--AllocMem($%lx,$%lx)'>,D0,D1

		JSRLIB	AllocMem

	PRINTF	DBG_OSCALL,<'CDG--CDGData @ $%lx'>,D0

		tst.l	d0
		beq	CDGInit_FAIL0

		move.l	d0,a4

	; init struct stuff

		move.l	d0,a2
		move.l	#CDGData_SIZEOF,d0
		lea	initdata(pc),a1

	PRINTF	DBG_OSCALL,<'CDG--InitStruct($%lx,$%lx,$%lx)'>,A1,A2,D0

		JSRLIB	InitStruct

	; initialize back pointer

		move.l	a5,cdd_CDGBase(a4)

	; and ExecBase pointer

		move.l	a6,cdd_ExecLib(a4)

	; and CDGPrefs

		move.l	a3,cdd_CDGPrefs(a4)

	; copy relevant data that we don't want to dereference for lookup

		move.l	cdgp_SigMask(a3),cdd_SigMask(a4)
		move.l	cdgp_SigTask(a3),cdd_SigTask(a4)

		move.b	cdgp_Control+1(a3),cdd_ExtraFlags3(a4)


	; Obtain VBLANK rate from execbase.  While perhaps not perfect,
	; it is adequate for simple timing needs

		move.b	VBlankFrequency(a6),cdd_VBlankRate+1(a4)

	; Open cd.device

		lea	cdd_CDPORT(a4),a0

		move.l	a0,cdd_CDIO+MN_REPLYPORT(a4)
		move.b	#NT_REPLYMSG,cdd_CDIO+LN_TYPE(a4)
		move.w	#IOSTD_SIZE,cdd_CDIO+MN_LENGTH(a4)

		move.b	#NT_MSGPORT,LN_TYPE(a0)
		move.b	#PA_SIGNAL,MP_FLAGS(a0)
		move.b	#SIGB_SINGLE,MP_SIGBIT(a0)
		move.l	ThisTask(a6),MP_SIGTASK(a0)
		lea	MP_MSGLIST(a0),a1
		NEWLIST	a1

		lea	cddevname(pc),a0
		moveq	#00,d0
		move.l	a4,a1		;iorequest is 1st in structure!
		moveq	#00,d1		;which is ideal for FRAMECALL interrupt

	PRINTF	DBG_OSCALL,<'CDG--OpenDevice($%lx,$%lx,$%lx,$%lx)'>,A0,D0,A1,D1

		JSRLIB	OpenDevice
		
		tst.l	d0
		bne	CDGInit_FAIL1

		move.l	IO_DEVICE(a4),a1
		move.l	db_CDSUBPage(a1),cdd_SubCodeBank(a4)

	PRINTF	DBG_FLOW,<'CDG--SubCodes @$%lx'>,cdd_SubCodeBank(a4)

	; open graphics.library

		lea	gfxname(pc),a1
		moveq	#37,d0

	PRINTF	DBG_OSCALL,<'CDG--OpenLibrary($%lx,$%lx)'>,A1,D0

		JSRLIB	OpenLibrary

		tst.l	d0
		beq	CDGInit_FAILVER

	PRINTF	DBG_OSCALL,<'CDG--GfxBase @ $%lx'>,D0

		move.l	d0,cdd_GfxLib(a4)
		move.l	d0,a3

		
		exg	a3,a6			;use gfxbase, cache exec

	;
	; Make sure blitter interrupts are disabled, as they should
	; be, however there is a bug in V37 graphics.library (and
	; earlier versions) which cause blitter interrupts to be
	; left enabled at startup time.  This happens when Graphics
	; calls CIA AddICRVector(), and the CIA-B TOD ALARM is in
	; a state that it generates an immediate CIA-B TOD interrupt.
	;
	; Graphics then enables interrupts, and leaves them enabled
	; until GELS are used once (which uses the CIA-B TOD counter
	; for beam collision detection during blits).  Once the blit
	; is done, blitter interrupts are then disabled again.  However
	; if the machine comes up with blitter interrupts enabled
	; (about 1 in 10x on 68000 machines), then all blitter operations
	; end up causing an interrupt, and hence, considerably slower.
	;
	; This bug is fixed for V38 graphics, but ... not in time for
	; CDTV-CR.  A fix is in SetPatch though.
	;
		JSRLIB	OwnBlitter
		JSRLIB	WaitBlit
		move.w	#INTF_BLIT,_intena
		JSRLIB	DisownBlitter

	; Initialize graphics structures (bitmaps first)

		lea	cdd_BitMap1(a4),a0
		moveq	#DISPLAY_DEPTH,d0
		move.w	#DISPLAY_WIDTH,d1
		move.w	#DISPLAY_HEIGHT,d2

	PRINTF	DBG_OSCALL,<'CDG--InitBitMap($%lx,$%lx,$%lx,$%lx)'>,A0,D0,D1,D2

		JSRLIB	InitBitMap

	; Allocate DISPLAY_DEPTH-1 bitplanes for BitMap structure

		lea	cdd_BitMap1(a4),a2
		lea	cdd_RasInfo1(a4),a0
		move.l	a2,ri_BitMap(a0)

		move.w	#DISPLAY_WIDTH,d0
		move.w	#((DISPLAY_DEPTH-1)*DISPLAY_HEIGHT),d1

	PRINTF	DBG_OSCALL,<'CDG--AllocRaster($%lx,$%lx)'>,D0,D1

		JSRLIB	AllocRaster

		tst.l	d0
		beq	CDGInit_FAIL2

		;stash pointer for BltClear

		move.l	d0,a1

		; store bit plane pointers

		moveq	#DISPLAY_DEPTH-2,d2
		lea	bm_Planes(a2),a0
		lea	cdd_BitPlane1_0(a4),a2

CDGInit_BM1:

	PRINTF	DBG_FLOW,<'CDG--BitMap Raster @ $%lx'>,D0

		move.l	d0,(a0)+
		move.l	d0,(a2)+
		addq.w	#APTR_SIZE,d1		;old code - could be removed
		add.l	#((DISPLAY_WIDTH*DISPLAY_HEIGHT)/8),d0
		dbf	d2,CDGInit_BM1

	; BltClear this memory - Use BytesPerRow mode, and do NOT wait
	; Shift DISPLAY_HEIGHT << 18 (16 to move to upper word, plus
	; * 4 for 4 bitplanes)

		move.l	#((DISPLAY_HEIGHT<<18)!(DISPLAY_WIDTH/8)),d0
		move.l	#(1<<1),d1

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear

	; Allocate DISPLAY_DEPTH-1 bitplanes for BitMap2

		move.w	#DISPLAY_WIDTH,d0
		move.w	#((DISPLAY_DEPTH-1)*DISPLAY_HEIGHT),d1

	PRINTF	DBG_OSCALL,<'CDG--AllocRaster($%lx,$%lx)'>,D0,D1

		JSRLIB	AllocRaster

		tst.l	d0
		beq	CDGInit_FAIL3

		;stash pointer for BltClear

		move.l	d0,a1

		; store bit plane pointers

		moveq	#DISPLAY_DEPTH-2,d2
		lea	cdd_BitPlane2_0(a4),a0

CDGInit_BM2:

	PRINTF	DBG_FLOW,<'CDG--BitMap Raster @ $%lx'>,D0

		move.l	d0,(a0)+
		add.l	#((DISPLAY_WIDTH*DISPLAY_HEIGHT)/8),d0
		dbf	d2,CDGInit_BM2

	; BltClear this memory - Use BytesPerRow mode, and do NOT wait
	; Shift DISPLAY_HEIGHT << 18 (16 to move to upper word, plus
	; * 4 for 4 bitplanes)

		move.l	#((DISPLAY_HEIGHT<<18)!(DISPLAY_WIDTH/8)),d0
		move.l	#(1<<1),d1

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear

	; Allocate an extra bit-plane, and store as bitplane 3.  Also
	; swap bitplane 3 as stored above into bitplane 4.
	;
	; This allows us to do the following -
	;
	; Bit plane 3 is ALL 1's all the time.  Hence the minimum displayable
	; color is color 8.
	;
	; Colors 0-7 are never used.
	;
	; Color 0 is the border color!!! And settable independently
	; of the display.  Important requirement for CD+G specification.
	;
	; Colors 8-15, and 24-31 are useable as the 16 colors needed for the
	; graphics display.
	;
	; Colors 16-23 are used for our 8 color sprite when channel
	; selection is being displayed.
	;
	; Here is how it looks on a graph
	;
	;	BitPlanes -> 4 3 2 1 0
	;
	;                    x 1 x x x  (minimum %01000 max %11111 )
	;                               (        #$08       #$31   )
	;
	; The following are possible
	;
	;	BINARY  DECIMAL  USE
	;	COLOR # COLOR #  FOR CD+G DISPLAY
	;
	;	01000   08	 Graphics color 0   T C = 16 (08-15 & 24-31)
	;	01001   09       Graphics color 1   O O
	;       ...                                 T L
	;       01111   15       Graphics color 7   A O
	;       11000   24       Graphics color 8   L R
	;       ...                                   S
	;	11111   31       Graphics color 15
	;
	; Note that bitplanes 0, 1, 2, and 4 can be therefore be treated
	; as a set of 4 bitplanes, and are allocated as contiguous memory
	; for blitter efficiency reasons.  Bitplane 3 can be ignored
	; and is always set to 1's.  Color index 0-7 are unused, making
	; it possible to set border color.  Color index 16-23 are unused,
	; and are used for sprites, which must use colors 16-31.
	;

		move.w	#DISPLAY_WIDTH,d0
		move.w	#DISPLAY_HEIGHT,d1

		JSRLIB	AllocRaster

	PRINTF	DBG_OSCALL,<'CDG--$%lx=AllocRaster()'>,D0

		tst.l	d0
		beq	CDGInit_FAIL4

		move.l	d0,cdd_ExtraRaster1(a4)
		move.l	d0,cdd_BitPlane1_Extra(a4)
		move.l	d0,cdd_BitPlane2_Extra(a4)

	; store as 3rd bitplane in bitmap structs

		lea	cdd_BitMap1(a4),a0
		lea	bm_Planes(a0),a0
		move.l	12(a0),16(a0)		;move bitplane 3->4
		move.l	d0,12(a0)		;store extra plane as plane 3

	; BltClear this memory - Use BytesPerRow mode, and DO wait
	; Shift DISPLAY_HEIGHT << 18 (16 to move to upper word, plus
	; * 4 for 4 bitplanes)
	;
	; We want this to WAIT for the blitter to finish so we are
	; sure all memory is set-up at first LoadView time!
	;
	; This is the extra bitplane which is always set to 1's,
	; and BltClear can do this by passing in a value in the FLAGS
	; argument to fill with.

		move.l	d0,a1
		move.l	#((DISPLAY_HEIGHT<<16)!(DISPLAY_WIDTH/8)),d0
		move.l	#(($FFFF<<16)!(1<<0)!(1<<1)!(1<<2)),d1

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear

	; allocate extra memory for 4 sprites, and work room for
	; TV GRAPHICS FONT data

		move.l	cdd_CDGPrefs(a4),a0

		moveq	#00,d0
		move.w	cdgp_SpriteHeight(a0),d0
		addq.l	#2,d0			;plus 1 line terminator, and 1 posctrl
		lsl.l	#4,d0			;X (4 bytes per line, and 4 sprites)

	; add enough extra memory for 4 12 bit high TV GFX FONTS -
	; but make them 4 bytes wide for convienent blitter manipulation

		add.l	#(FONT_HEIGHT*4*4),d0
		move.l	d0,cdd_4SpriteSize(a4)	;cache
		move.l	#MEMF_CHIP!MEMF_CLEAR,d1
		exg	a3,a6			;use execbase

	PRINTF	DBG_OSCALL,<'CDG--AllocMem($%lx,$%lx)'>,D0,D1

		JSRLIB	AllocMem

	PRINTF	DBG_OSCALL,<'CDG--$%lx=AllocMem()'>,D0

		tst.l	d0
		beq	CDGInit_FAIL5

	; stash pointers to sprite data memory

		move.l	d0,cdd_4SpriteData(a4)
		move.l	cdd_CDGPrefs(a4),a0
		moveq	#00,d1
		move.w	cdgp_SpriteHeight(a0),d1
		addq.l	#2,d1			;plus 1 terminator, and 1 posctrl
		lsl.l	#2,d1			;* 4 bytes per line
		add.l	d1,d0
		move.l	d0,cdd_SpriteData3(a4)
		add.l	d1,d0
		move.l	d0,cdd_SpriteData4(a4)
		add.l	d1,d0
		move.l	d0,cdd_SpriteData5(a4)
		add.l	d1,d0

	; and pointers to TFV GFX FONT data
		move.l	d0,cdd_FontPlane(a4)
		moveq	#(FONT_HEIGHT*4),d1
		add.l	d1,d0
		move.l	d0,cdd_NegPlane(a4)
		add.l	d1,d0
		move.l	d0,cdd_1sPlane(a4)
		add.l	d1,d0
		move.l	d0,cdd_0sPlane(a4)

	; pre-fill 1's plane

		moveq	#(FONT_HEIGHT-1),d1
		move.l	cdd_1sPlane(a4),a0
		move.l	#$FC000000,d0
CDGInit_Fill1s:
		move.l	d0,(a0)+
		dbf	d1,CDGInit_Fill1s

		exg	a3,a6			;use gfxbase

	; Initialize other Gfx structures - these are already 0's,
	; but its better to use the standard Gfx calls to init these
	; for future compatability, and ease of modification later

		lea	cdd_View1(a4),a2
		move.l	a2,a1

	; set the initial ActiveView ptr up too since we have the
	; pointer handy

	PRINTF	DBG_OSCALL,<'CDG--InitView($%lx)'>,A1

		JSRLIB	InitView

		lea	cdd_ViewPort1(a4),a0
		move.l	a0,v_ViewPort(a2)
		move.l	a0,a2

	PRINTF	DBG_OSCALL,<'CDG--InitVPort($%lx)'>,A0

		JSRLIB	InitVPort

		; Initialization of ViewPort

		lea	cdd_RasInfo1(a4),a0
		move.w	#VISUAL_X,ri_RxOffset(a0)
		move.w	#VISUAL_Y,ri_RyOffset(a0)

		move.l	a0,vp_RasInfo(a2)
		move.w	#VISUAL_WIDTH,vp_DWidth(a2)
		move.w	#VISUAL_HEIGHT,vp_DHeight(a2)
		or.w	#V_SPRITES,vp_Modes(a2)

	; calculate min left edge, and set DxOffset.  Minimum is
	; based on worst case position possible without losing any
	; sprite DMA

		lea	cdd_View1(a4),a0

		move.l	cdd_CDGPrefs(a4),a1
		move.w	cdgp_DisplayX(a1),d0
		cmp.w	cdd_MinDisplayX(a4),d0
		bge.s	CDGInit_XInRange	;signed compare!

		move.w	cdd_MinDisplayX(a4),d0	;limit

CDGInit_XInRange:
		cmp.w	#26,d0			;realistic limits
		blt.s	CDGInit_XMaxRange
		move.w	#26,d0
CDGInit_XMaxRange:
		move.w	d0,cdd_MinDisplayX(a4)	;cache for future reference

		add.w	#CENTER_X,d0
		cmp.w	#350,d0			;well, almost max of +362
		ble.s	CDGInit_XMaxed
		move.w	#350,d0
CDGInit_XMaxed:

		add.w	d0,v_DxOffset(a0)

	; same idea for Y, but there is no sprite limitation - there is
	; a reasonable limit however meant to give us suitable VBLANK
	; bandwidth

		moveq	#-14,d1			;not quite true limit of -15 PAL
		move.w	cdgp_DisplayY(a1),d0
		cmp.w	d1,d0
		bge.s	CDGInit_YInRange
		move.w	d1,d0
CDGInit_YInRange:
		move.w	#CENTER_Y,d1

		move.w	gb_DisplayFlags(a6),d1
		and.w	#PAL,d1
		beq.s	CDGInit_NotPal

		add.w	#PAL_KLUDGE,d1		;kludge for PAL centering (yuck!)
CDGInit_NotPal:
		add.w	d1,d0			;and center screen

		cmp.w	#200,d0			;well, almost max of +217
		ble.s	CDGInit_YMaxed
		move.w	#200,d0
CDGInit_YMaxed:
		add.w	d0,v_DyOffset(a0)

		; final step is to obtain a color map

		moveq	#(1<<DISPLAY_DEPTH),d0

	PRINTF	DBG_OSCALL,<'CDG--GetColorMap($%lx)'>,D0

		JSRLIB	GetColorMap

	PRINTF	DBG_OSCALL,<'CDG--$%lx=GetColorMap()'>,D0

		tst.l	d0
		beq	CDGInit_FAIL6

		move.l	d0,vp_ColorMap(a2)

	; Create intermediate copper lists

		lea	cdd_View1(a4),a0
		lea	cdd_ViewPort1(a4),a1

	PRINTF	DBG_OSCALL,<'CDG--MakeVPort($%lx,$%lx)'>,A0,A1

		JSRLIB	MakeVPort

	;;;	TST.L	D0		-- not implemented for V37
	;;;	BNE.s	CDGInit_FAIL8	-- usable in V39???

	; Create copper lists

		lea	cdd_View1(a4),a1

	PRINTF	DBG_OSCALL,<'CDG--MrgCop($%lx)'>,A1

		JSRLIB	MrgCop

	PRINTF	DBG_OSCALL,<'CDG--$%lx=MrgCop()'>,D0

		tst.l	d0		; error returned implemented
		bne	CDGInit_FAIL10	; for V37 graphics

	; Get 4 sprites - so we can have left & right (32 bit wide)
	; 15 color sprites - only use 8 colors though

		move.l	cdd_SpriteBase(a4),d0

		lea	cdd_SimpleSprite2(a4),a2
		move.l	a2,a0

		move.l	cdd_CDGPrefs(a4),a1
		move.w	cdgp_SpriteHeight(a1),d2

	PRINTF	DBG_OSCALL,<'CDG--GetSprite($%lx,$%lx)'>,A0,D0

		JSRLIB	GetSprite

		tst.l	d0
		bmi	CDGInit_FAIL12


		move.w	d2,ss_height(a2)
		move.w	#RELSPRITE_X,ss_x(a2)
		move.w	#RELSPRITE_Y,ss_y(a2)

		move.l	cdd_SpriteBase(a4),d0
		addq.b	#1,d0

		lea	cdd_SimpleSprite3(a4),a2
		move.l	a2,a0

	PRINTF	DBG_OSCALL,<'CDG--GetSprite($%lx,$%lx)'>,A0,D0

		JSRLIB	GetSprite
		tst.l	d0
		bmi	CDGInit_FAIL13

		move.w	d2,ss_height(a2)
		move.w	#RELSPRITE_X,ss_x(a2)
		move.w	#RELSPRITE_Y,ss_y(a2)

		move.l	cdd_SpriteBase(a4),d0
		addq.b	#2,d0

		lea	cdd_SimpleSprite4(a4),a2
		move.l	a2,a0

	PRINTF	DBG_OSCALL,<'CDG--GetSprite($%lx,$%lx)'>,A0,D0

		JSRLIB	GetSprite
		tst.l	d0
		bmi	CDGInit_FAIL14

		move.w	d2,ss_height(a2)
		move.w	#(RELSPRITE_X+SPRITE_WIDTH),ss_x(a2)
		move.w	#RELSPRITE_Y,ss_y(a2)

		move.l	cdd_SpriteBase(a4),d0
		addq.b	#3,d0

		lea	cdd_SimpleSprite5(a4),a2
		move.l	a2,a0

	PRINTF	DBG_OSCALL,<'CDG--GetSprite($%lx,$%lx)'>,A0,D0

		JSRLIB	GetSprite
		tst.l	d0
		bmi	CDGInit_FAIL15

		move.w	d2,ss_height(a2)
		move.w	#(RELSPRITE_X+SPRITE_WIDTH),ss_x(a2)
		move.w	#RELSPRITE_Y,ss_y(a2)

	; Set initial colors for ViewPort

		move.l	cdd_CDGPrefs(a4),a0
		lea	cdgp_SpriteColors(a0),a0
		lea	cdd_ColorTable1(a4),a2
		move.l	a2,a1

	; bump pointer to colors by 16 words, so we will copy our 8
	; sprite colors to colors 16-23 inclusive

		add.w	#(16*2),a1

		moveq	#3,d0		;4 long words = 8 words of colors!

CDGInit_SpriteColors:
		move.l	(a0)+,(a1)+
		dbf	d0,CDGInit_SpriteColors

		lea	cdd_ViewPort1(a4),a0
		move.l	a2,a1
		moveq	#32,d0

	PRINTF	DBG_OSCALL,<'CDG--LoadRGB4($%lx,$%lx,$%lx)'>,A0,A1,D0

		JSRLIB	LoadRGB4

		BSRSELF	SniffCopperList

	; Try to obtain ownership of serial misc, but not required.
	; If not obtained, then CD+MIDI just won't work

		move.l	a3,a6		;restore execbase

	; but don't bother if prefs flag says don't bother

		btst	#CDGB_NOMIDI,cdd_ExtraFlags3(a4)
		BNE_S	CDGInit_MidiDisabled

	; First try to remove serial.device which (ACK!!) does not
	; release the MISC/SERIAL bits until expunged!

		FORBID

		lea	DeviceList(a6),a0
		lea	sername(pc),a1

		JSRLIB	FindName
		tst.l	d0
		beq.s	CDGInit_NoSerial

		move.l	d0,a1
		JSRLIB	RemDevice

CDGInit_NoSerial:
		PERMIT

		lea	miscname(pc),a1
		JSRLIB	OpenResource

	PRINTF	DBG_OSCALL,<'CDG--$%lx=OpenResource("misc.resource")'>,D0

		tst.l	d0
		BEQ_S	CDGInit_NoMiscRes	;???

		move.l	d0,a6

		moveq	#MR_SERIALBITS,d0
		lea	cdgname(pc),a1
		JSR	MR_ALLOCMISCRESOURCE(a6)

		tst.l	d0
		BNE_S	CDGInit_NoMiscRes

	PRINTF	DBG_OSCALL,<'CDG--Owned MR_SERIALBITS'>

		moveq	#MR_SERIALPORT,d0
		lea	cdgname(pc),a1
		JSR	MR_ALLOCMISCRESOURCE(a6)

		tst.l	d0
		bne.s	CDGInit_FreeSerBits

	PRINTF	DBG_OSCALL,<'CDG--Owned MR_SERIALPORT'>

	; mark as we own resource bits, and cache resource base for
	; Free when CDGEnd() is called

		move.l	a6,cdd_MiscResource(a4)

	; see midi.asm for the code which inits the serial port for midi out

		BSRSELF	CDGInitMidiOut

		bra.s	CDGInit_NoMiscRes

CDGInit_FreeSerBits:

		moveq	#MR_SERIALBITS,d0
		JSR	MR_FREEMISCRESOURCE(a6)

	PRINTF	DBG_OSCALL,<'CDG--Free MR_SERIALBITS'>

CDGInit_NoMiscRes:

		move.l	a3,a6		;restore A6 - execbase

CDGInit_MidiDisabled:

	; reset border color to color 0

		lea	cdd_CLUTTable(a4),a0
		move.l	a0,cdd_BorderPen(a4)

	; Initialize midi out buffer

		lea	cdd_MidiBytes(a4),a0
		move.l	a0,cdd_PutMidiByte(a4)
		move.l	a0,cdd_GetMidiByte(a4)

		lea	cdd_EndMidiBytes(a4),a0
		move.l	a0,cdd_MaxMidiByte(a4)

	; Initialize PACKET buffer pointers

		lea	cdd_PacketData(a4),a0
		move.l	a0,cdd_PutPacket(a4)
		move.l	a0,cdd_GetPacket(a4)

		lea	cdd_EndPackets(a4),a0
		move.l	a0,cdd_MaxPacket(a4)

	; Initialize BUCKET pointers

		lea	cdd_Buckets(a4),a0
		move.l	a0,cdd_NextBucket(a4)		;prime
		move.l	a0,a2				;cache

	PRINTF	DBG_FLOW,<'CDG--First Bucket @ $%lx'>,A0

	; Initialize Next pointer; first for speed; circular

		moveq	#6,d1

CDGInit_Buckets:

		lea	CDGBucket_SIZEOF(a0),a1
		move.l	a1,cdt_Next(a0)

	PRINTF	DBG_FLOW,<'CDG--Store $%lx in cdt_Next($%lx)'>,A1,A0

		move.l	a1,a0
		dbf	d1,CDGInit_Buckets

	PRINTF	DBG_FLOW,<'CDG--Store $%lx in cdt_Next($%lx)'>,A2,A0

		move.l	a2,cdt_Next(a0)

	; Initialize active/inactive bitplane pointers

		lea	cdd_BitPlane1_0(a4),a0
		move.l	a0,cdd_ActivePlanes(a4)

		lea	cdd_BitPlane2_0(a4),a0
		move.l	a0,cdd_InactivePlanes(a4)

	; Initialize, and add interrupt servers

		lea	cdd_IntVBlank(a4),a1
		move.l	a4,IS_DATA(a1)
		moveq	#INTB_VERTB,d0

	PRINTF	DBG_OSCALL,<'CDG--AddIntServer($%lx,$%lx)'>,D0,A1

		JSRLIB	AddIntServer

	; Add notification hook in cd.device

		move.l	a4,a1		

		move.w	#CD_ADDFRAMEINT,IO_COMMAND(a1)
		move.l	#IS_SIZE,IO_LENGTH(a1)
		lea	cdd_CDINT(a1),a0
		move.l	a0,IO_DATA(a1)	;link

		lea	CDGPackInt(pc),a0
		move.l	a0,cdd_CDINT+IS_CODE(a1)
		move.l	a1,cdd_CDINT+IS_DATA(a1)

	; cd.device is using Cause() - needs to be documented

		move.b	#NT_INTERRUPT,cdd_CDINT+LN_TYPE(a1)
		move.b	#32,cdd_CDINT+LN_PRI(a1)

	PRINTF	DBG_OSCALL,<'CDG--SendIO($%lx)'>,A1

		JSRLIB	SendIO

	;-- FINALLY DONE!!!  Store pointer to this data!!

		move.l	a4,d0		;return SUCCESS!!
		movem.l	(sp)+,d2-d7/a2-a6

	PRINTF	DBG_EXIT,<'CDG--$%lx=_CDGInit()'>,D0

		rts


CDGInit_FREEDATA:

	; disable subcodes

		move.l	cdd_ExecLib(a4),a6
		moveq	#00,d0
		move.l	#SIGF_SINGLE,d1
		JSRLIB	SetSignal		;clear it

		move.l	a4,a1
		move.w	#CD_REMFRAMEINT,IO_COMMAND(a1)

	PRINTF	DBG_OSCALL,<'CDG--DoIO($%lx)'>,A1

		JSRLIB	DoIO

	; Remove Interrupt servers

		move.l	cdd_ExecLib(a4),a6

		lea	cdd_IntVBlank(a4),a1
		moveq	#INTB_VERTB,d0

	PRINTF	DBG_OSCALL,<'CDG--RemIntServer($%lx,$%lx)'>,D0,A1

		JSRLIB	RemIntServer

	; Free misc.resource bits (for CD+MIDI data)

		move.l	cdd_MiscResource(a4),d0
		beq.s	CDGInit_FAIL16		;owned??

	; see midi.asm for code which restores the serial port

		BSRSELF	CDGFreeMidiOut

		move.l	d0,a6			;yes, free serial port stuff

		moveq	#MR_SERIALBITS,d0
		JSR	MR_FREEMISCRESOURCE(a6)

		moveq	#MR_SERIALPORT,d0
		JSR	MR_FREEMISCRESOURCE(a6)

	PRINTF	DBG_OSCALL,<'CDG--Free MR_SERIALPORT/BITS'>

	; Free Sprite #5

CDGInit_FAIL16:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_SpriteBase(a4),d0
		addq.b	#3,d0

	PRINTF	DBG_OSCALL,<'CDG--FreeSprite($%lx)'>,D0

		JSRLIB	FreeSprite

	; Free Sprite #4

CDGInit_FAIL15:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_SpriteBase(a4),d0
		addq.b	#2,d0

	PRINTF	DBG_OSCALL,<'CDG--FreeSprite($%lx)'>,D0

		JSRLIB	FreeSprite

	; Free Sprite #3

CDGInit_FAIL14:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_SpriteBase(a4),d0
		addq.b	#1,d0

	PRINTF	DBG_OSCALL,<'CDG--FreeSprite($%lx)'>,D0

		JSRLIB	FreeSprite

	; Free Sprite #2

CDGInit_FAIL13:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_SpriteBase(a4),d0

	PRINTF	DBG_OSCALL,<'CDG--FreeSprite($%lx)'>,D0

		JSRLIB	FreeSprite


	; Free Copper List

CDGInit_FAIL12:


		move.l	cdd_GfxLib(a4),a6
		lea	cdd_View1(a4),a0
		move.l	v_LOFCprList(a0),a0

	PRINTF	DBG_OSCALL,<'CDG--FreeCprList($%lx)'>,A0

		JSRLIB	FreeCprList

	; Free Intermediate copper List 1

CDGInit_FAIL10:


		move.l	cdd_GfxLib(a4),a6
		lea	cdd_ViewPort1(a4),a0

	PRINTF	DBG_OSCALL,<'CDG--FreeVPortCopLists($%lx)'>,A0

		JSRLIB	FreeVPortCopLists

	; Free ColorMap1

CDGInit_FAIL8:

		move.l	cdd_GfxLib(a4),a6
		lea	cdd_ViewPort1(a4),a2
		move.l	vp_ColorMap(a2),a0

	PRINTF	DBG_OSCALL,<'CDG--FreeColorMap($%lx)'>,A0

		JSRLIB	FreeColorMap

	; ColorMap allocation failure 1

	; Free Sprite Data buffer

CDGInit_FAIL6:
		move.l	cdd_ExecLib(a4),a6
		
		move.l	cdd_4SpriteSize(a4),d0
		move.l	cdd_4SpriteData(a4),a1

	PRINTF	DBG_OSCALL,<'CDG--FreeMem($%lx,$%lx)'>,A1,D0

		JSRLIB	FreeMem

	; Free ExtraRaster1

CDGInit_FAIL5:
		move.l	cdd_GfxLib(a4),a6
		
		move.w	#DISPLAY_WIDTH,d0
		move.w	#DISPLAY_HEIGHT,d1
		move.l	cdd_ExtraRaster1(a4),a0

	PRINTF	DBG_OSCALL,<'CDG--FreeRaster($%lx,$%lx,$%lx)'>,A0,D0,D1

		JSRLIB	FreeRaster

	; Free 2nd set of bitplanes

CDGInit_FAIL4:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_BitPlane2_0(a4),a0

		move.l	#DISPLAY_WIDTH,d0
		move.l	#((DISPLAY_DEPTH-1)*DISPLAY_HEIGHT),d1

	PRINTF	DBG_OSCALL,<'CDG--FreeRaster($%lx,$%lx,$%lx)'>,A0,D0,D1

		JSRLIB	FreeRaster

	; Free 1st set bitplanes

CDGInit_FAIL3:

		move.l	cdd_GfxLib(a4),a6
		move.l	cdd_BitPlane1_0(a4),a0

		move.l	#DISPLAY_WIDTH,d0
		move.l	#((DISPLAY_DEPTH-1)*DISPLAY_HEIGHT),d1

	PRINTF	DBG_OSCALL,<'CDG--FreeRaster($%lx,$%lx,$%lx)'>,A0,D0,D1

		JSRLIB	FreeRaster

	; close Gfx library too

CDGInit_FAIL2:
		move.l	cdd_ExecLib(a4),a6
		move.l	cdd_GfxLib(a4),a1


	PRINTF	DBG_OSCALL,<'CDG--CloseLibrary($%lx)'>,A1

		JSRLIB	CloseLibrary

CDGInit_FAILVER:

	; Close cdtv.device

		move.l	cdd_ExecLib(a4),a6
		move.l	a4,a1

	PRINTF	DBG_OSCALL,<'CDG--CloseDevice($%lx)'>,A1

		JSRLIB	CloseDevice

	; release structure memory

CDGInit_FAIL1:
		move.l	cdd_ExecLib(a4),a6
		move.l	a4,a1
		move.l	#CDGData_SIZEOF,d0

	PRINTF	DBG_OSCALL,<'CDG--FreeMem($%lx,$%lx)'>,A1,D0

		JSRLIB	FreeMem

	; couldn't even allocate memory for structure

CDGInit_FAIL0:
		moveq	#00,d0
		movem.l	(sp)+,d2-d7/a2-a6

	PRINTF	DBG_EXIT,<'CDG--$%lx=_CDGInit()'>,D0

		rts


		CNOP	0,4

	; InitStruct() initialization table

initdata:
		INITLONG	cdd_IntVBlank+LN_NAME,cdgname
		INITLONG	cdd_IntVBlank+IS_CODE,CDGVertInt
		INITBYTE	cdd_IntVBlank+LN_TYPE,NT_INTERRUPT
		INITBYTE	cdd_IntVBlank+LN_PRI,1
		INITWORD	cdd_MinDisplayX,0
		INITLONG	cdd_SpriteBase,2
		INITBYTE	cdd_ActiveChannel,1		
		INITBYTE	cdd_VBlankFlags,VBFF_BACKSCREEN
		INITBYTE	cdd_GPackCount,GPACKCOUNT
		INITBYTE	cdd_MPackCount,MPACKCOUNT

		dc.w		0

gfxname:		
		GRAPHICSNAME

miscname:
		MISCNAME

sername:
		SERIALNAME

	; GACK!! no assembly include file!

cddevname:
		dc.b	'cd.device',0


******* cdg.library/--background-- ************************************
*
*   CD+G
*
*	CD+G is a standard format for encoding graphics data on
*	audio CD's.  This data is stored with the audio data such
*	that the graphics data retrieved is synchronized with the
*	audio.
*
*	CD+G currently defines a limited set of graphics operations
*	including --
*
*	o Setting border color (0..15)
*
*	o Drawing a 6 bit wide, by 12 bit high graphic image
*	  at a specific ROW/COLUMN position.  The graphic image
*	  is drawn in 2 colors.  The pen colors to use are encoded
*	  in the graphic data.
*
*	  A 6x12 pixel element is known as a FONT.
*
*	o Exclusive-OR a 6x12 bit graphic image with another.
*	  EOR makes it possible to draw a graphic image, and
*	  color it in more than 2 colors by EORing the previous
*	  bit-pattern with a new one using 2 new pen colors.
*
*	o Clear the entire display with a color (0..15).
*
*	o Change the color table (0..15).  Separate CD+G instructions
*	  are used to load pen colors 0-7, and 8-15.
*
*	o Scroll screen left/right, up/down.  The vacated portion
*	  is filled in with a pen color (0..15).  Smooth scrolling
*	  is possible.
*
*	o Scroll screen left/right, up/down.  The vacated portion
*	  is filled in with the data which is scrolled off screen.
*
*	There are currently two kinds of graphics --
*
*	o Line Graphics - the display is a VISIBLE 2 rows, by
*	  48 columns.  Each FONT element in the display is 6 pixels
*	  wide by 12 pixels high.  This mode is currently NOT
*	  supported.
*
*	o TV Graphics - the display is a VISIBLE 16 rows, by
*	  48 columns.  Each FONT element in the display is 6 pixels
*	  wide by 12 pixels high.  This mode IS supported by
*	  cdg.library.
*
*   CHANNELS -
*
*	Two of the CD+G drawing instructions can be divided into
*	channel numbers.  These are the WRITE FONT intruction
*	which draws a 2 color 6x12 image on the screen, and the EOR
*	FONT instruction which uses EOR logic to draw over an existing
*	font.
*
*	The primary channel number is 0, and would typically be used
*	to draw a picture.  Infact, some CD+G disks will only use
*	channel 0.
*
*	In addition to channel 0, there are 15 other channels which
*	can be used for additional graphics imagery, such as the
*	text for more than one language.
*
*	By default the cdg.library uses channels 0, and 1 for
*	graphic data.  The cdg.library also provides a function
*	so that channels 2-15 can be used instead of channel
*	1.  Therefore, two channels are always active.
*
*	While the CD+G specification recommends that there be
*	a mechanism for selectively turning on/off all 16 channels,
*	it is also not entirely clear if, or how the data should
*	be mixed on screen.
*
*	Because there are already variations in CD+G implementations
*	regarding channel selection, and usage, the scheme provided
*	for in cdg.library is probably the least complicated for the
*	user.  It is however admittedly not completely within the
*	ideal specification.
*
*   PACKS & PACKETS
*
*	CD+G data is stored in something called a PACKET.  There
*	are up to 4 TV GRAPHIC'S PACKS per PACKET.  Each PACK
*	in a PACKET can be a separate graphics instruction from
*	those listed above.
*
*	Because a new PACKET is available 75x/second, CD+G requires
*	most of the available CPU time on a 68000 based system to
*	keep up with the data.
*
*   ERROR CORRECTION
*
*	Because of the nature of CD's, data can be munged.  Because
*	audio data must be played in real-time, it is not possible
*	to retry a bad block of data.  The CD standard includes
*	an ERROR CORRECTION method which helps to minimize data loss.
*
*	As of V39 of the cdg.library, Reed-Solomon error
*	correction has been implemented in software.  While the current
*	software implementation is quite efficient, a mechanism
*	has been provided for disabling error correction.
*
*	In the case of a 68000 Amiga-CD unit running a 7Mhz 68000,
*	ERROR DETECTION requires approximately 550-600us per PACK.
*	Correction of ONE error requires an additional 350-400us
*	per PACK.  Correction of TWO errors requires an additional
*	800-1000us per PACK.  Correction of more than two errors
*	per PACK is not possible, and PACKS with more than two
*	errors are discarded.
*
*	If error correction has been disabled, all PACKS are processed
*	as is.  However because the typical case is no errors, the
*	additional 2 milliseconds per PACKET (4 PACKS) still makes it
*	possible to leave approximately 50% of the available CPU time
*	free when CD+G is running with error correction enabled.
*
*	***NOTE***
*
*		The CD+G/+MIDI module was originally designed for
*		use on a 68000 based system.  Therefore timing
*		is obviously improved on a 68020-68040 based system.
*
*	In addition to the Reed-Solomon error correction, PACK
*	data is 8x interleaved on disk.  Therefore even if 16
*	bytes of contiguous data are munged by a scratch, or
*	other imperfection, it is likely that all 16 bytes can
*	be corrected.
*
*	It is possible that in the future error correction may be
*	implemented in hardware, in which case error correction might
*	always be enabled.
*
*	Even with error correction enabled, it is possible to have more
*	than two errors per PACK, or to have so many errors that the
*	error detection code cannot calculate the actual number of
*	errors.
*
*   cdg.library (V39)
*
*	The cdg.library provides a set of functions for use with   
*	Commodore Amiga CD systems.  These functions are meant to be
*	used by the Player Preferences module which lets the user
*	select audio tracks to play.  Note that these functions
*	were not available in the initial cdg.library, and the V39
*	functions are not compatable with the earlier version of
*	the CDTV cdg.library.  It is also possible to write a custom
*	player program which makes use of cdg.library to play CD+G, and
*	CD+MIDI disks.
*
*	***NOTE***
*
*		The cdg.library was primarily designed for use with
*		the audio disk player module, so does not use Intuition
*		screens.  Infact the player module installs an input
*		handler to lock-out Intuition.  Use of the cdg.library
*		in an Intuition environment requires similar locking
*		and use of cdg.library functions to manage screen
*		ordering.  The primary reason for writing your own
*		+G/+MIDI player would be for a Kaoroke player, or
*		other multi-media environments in which the Intuition
*		model does not apply well.
*
*	The caller of cdg.library functions is responsible for managing
*	user input, and CD activity, including selecting tracks to
*	play, starting play, stopping play, etc.  These kinds of
*	disk control functions are provided by the cd.device.
*
*	In the typical case, the following steps are recommended -
*
*	1.) Initialize a CDGPrefs structure.  This structure
*	must be entirely filled in, and contains many pointers to
*	sprite data which must be initialized.  CDGAllocPrefs()
*	should be used to allocate the CDGPrefs structure.  Likewise
*	use CDGFreePrefs() to free the structure.
*
*	The sprites are used to display activity information to the
*	user when the CD+G display is active.  An example would be
*	a change in channel selection.  A new channel number is
*	automatically displayed for a few moments for the user's
*	benefit.  Sprites are also used to display disk activity
*	such as PAUSE mode.
*	
*	Each image is composed of 4 sprites.  Two sprites are
*	used for the left side of the symbol, and two for the
*	right.  The image can use 8 colors because attached
*	sprites are used.  Each symbol requires pointers to
*	FOUR sprites arranged in this order -
*
*		1st ptr -> Left side sprite
*		2nd ptr -> Left side, attached sprite
*		3rd ptr -> Right side sprite
*		4th ptr -> Right side, attached sprite
*
*	The ordering arrangement is designed to be convienent
*	for the cdg.library, and provides enough colors to make
*	attractive images.  Because sprites are used, the total width
*	of each image is always 32 bits wide, and the height is
*	variable.  It is highly recommended that the images not be
*	more than 32 bits high to insure they fit on the screen, and
*	avoid excessive memory usage.
*
*	Sprite data should be in CHIP RAM, and be stored in the
*	simple sprite format including space for the posctrl words, and
*	NULL terminating words.  The height of all sprites must be
*	the same.
*
*	The CDGPrefs structure includes other fields defined in the
*	cdgprefs.i/h file such as display position, and an array of
*	8 colors to be used for the sprites.
*
*	2.) Call CDGBegin() with the CDGPrefs() structure as an argument.
*	This function starts the cdg.library.  The call may fail due to
*	lack of memory, or inability to open a needed system resource.
*	Of interest, the cdg.library will only process MIDI packets IF
*	the serial port, and control bits have not been allocated by
*	some other task (e.g., the serial.device).
*
*	Once started, the cdg.library starts collecting PACKETS as
*	available.  The PACKETS are buffered, but because of the
*	rate and size of the PACKETS assume no more than 1 seconds
*	worth of PACKET data will be buffered before it is overwritten
*	by more recent PACKETS.
*
*	3.) Call CDGPlay(), and start the CD playing.  Once the CD starts
*	playing, assume the cdg.library is receiving CD+G PACKETS.  At this
*	point your code should call CDGDraw() as often as possible.  During
*	every call, CDGDraw() will process any pending PACKS.
*
*	CDGDraw() also returns a value indicating if ANY CD+G, or CD+MIDI
*	PACKS have been processed.  This is meant to be used as an indicator
*	that the CD+G screen should be brought to front, or perhaps a gadget
*	would be displayed indicating that CD+G is available.  The user will
*	probably prefer the CD+G screen automatically coming to front, or they
*	will miss the initial portion of the animation.
*
*	To bring the CD+G screen to front, use the CDGFront() call.
*	It is recommended that you first call CDGDraw() so that
*	any pending PACKS will be processed before the screen is
*	made visible.
*
*	4.) The player program is now responsible for switching to
*	CD+G mode by efficiently checking for user input, and regularly
*	calling the CDGDraw() function.  The loop should be coded
*	to provide maximum CPU time for CDGDraw().  A mechanism is
*	provided so that Wait() can be used to sleep between calls.
*
*	5.) If the user presses a key, or button indicating they want 
*	to bring the Player program to front, you must use the CDGBack()
*	function to tell CD+G to turn off any sprites it has on screen, and
*	notify it not to bring its own screen to front again until CDGFront()
*	is called.
*
*	6.) The cdg.library provides a number of functions for
*	telling it what the CD is doing, as well the ability to
*	change the active channels 1-15.  These functions are also
*	used to provide some visual indication to the user that their input
*	has been noticed, and what is happening.  In addition to the channel
*	selection, there are sprite images in CDGPrefs for the following
*	modes --
*
*		PLAY	- Image is displayed for a few moments.
*			  Indicates the disk has started PLAY.
*			  Should be displayed whenever the disk
*			  starts playing, including after PAUSE,
*			  STOP, FF, and REW.
*
*		PAUSE	- Image is displayed indefinitely.
*
*		STOP	- Image is displayed indefinitely.
*
*		FF	- Image is displayed indefinitely.
*
*		REW	- Image is displayed indefinitely.
*
*		NEXT TRACK - Image is displayed for a few moments.
*			     PLAY is implied.
*
*		PREV TRACK - Image is displayed for a few moments.
*			     PLAY is implied.
*
*		CHANNEL    - Image is displayed for a few moments.
*
*	See AutoDocs for the functions used to display these images.
*
*	7.) If the CD is removed, the CDGDiskRemoved() function should
*	be called.
*
*	8.) To close down the cdg.library, use the CDGEnd() function
*	to free all memory, and resources allocated by CDGBegin().
*
*	Note that while your own screen is displayed, it is not
*	necessary to call CDGDraw() once per PACKET, but it should
*	still be called as often as possible of data may be lost.
*	Assume no more than 1/2-3/4 seconds of PACKET data will be
*	buffered before it is overwritten by new data.
*
*	Note that sprite images can be replaced by a more recent
*	sprite image if the image has not already been removed
*	from the display.  For example, if the user is changing
*	channel numbers, you can call CDGChannel() at any time,
*	and the most recent channel will be displayed.
*
*	If you call CDGBack() while a sprite image is being
*	displayed, the sprite image is turned off at the next
*	VBLANK.  The last sprite image is NOT automatically restored
*	when you call CDGFront() again.  Therefore if the disk
*	is still in PAUSE mode, you will probably want to call
*	CDGPause() again after bring the CD+G screen to front.
*
*    SPECIAL CONSIDERATIONS
*
*	During FF and REW, CD+G data is missed.  Because data is
*	missed, the CD+G display is cleared during these operations,
*	and the display may not be properly updated.  For example, if
*	the color tables were supposed to have been changed during the
*	section of the CD which is skipped in FF/REW modes, the colors
*	will be wrong when the disk starts to PLAY again.  Fortunately
*	many CD+G titles refresh the colors regularly, but not all.  Some
*	titles draw static images may not be updated for long sections of
*	play.
*
*	Because there is no work around for this problem, you can
*	either disable FF/REW during CD+G, or allow the problem.
*	Ideally CD+G instructions will contain enough repetative
*	information that this will not be a significant problem.
*
*	Another special problem is that some PACKS are missed when the
*	CD is taken out of PAUSE mode, and placed in PLAY mode.  There is
*	no good work around for this problem, and it is generally not
*	too annoying.
*
*    MIDI DATA
*
*	CD's can also contain MIDI data.  The cdg.library optionally
*	handles MIDI PACKETS during CDGDraw(), and sends this data
*	at MIDI Baud rates out the serial port.  It is therefore important
*	to call CDGDraw() often enough to avoid long delays between
*	MIDI PACK handling.
*
*	MIDI PACKS can contain 0-12 bytes per PACK.  Therefore it is
*	possible to exceed the maximum transmission rate allowed by
*	MIDI.  In actual use though designers of +MIDI disks are warned
*	about this, and will typically send much less data than the
*	maximum possible.  MIDI PACK data is stored in a serial output queue,
*	and transmitted via TBE interrupts.  It is therefore possible to
*	interleave +G, and +MIDI data such that new graphics are being drawn
*	at the same time +MIDI data is being transmitted.
*
*	CDGStop(), CDGPause(), CDGFastForward(), CDGRewind(),
*	CDGNextTrack(), CDGPrevTrack(), and CDGDiskRemoved() will
*	send a sequence of bytes to tell all midi channels to be
*	quiet if any +MIDI data has been sent since the sequence
*	was last sent.  This is done to avoid 'stuck' notes.
*
*    SYNCHRONIZATION
*
*	One of problems with CD+MIDI is synchronization of audio, and
*	+MIDI data.  Depending on the subcode hardware implementation,
*	software may receive subcodes a byte at a time, or a FRAME
*	at a time.  Assuming the later, there is still the problem of
*	synchronizing +MIDI data from a FRAME which has already	passed
*	with audio which is currently being played.  When you add in
*	software/hardware overhead/delays, only approximate
*	synchronization can be expected.
*
*	In actual use synchronization is not a significant problem;
*	our audio perception is limited.  However if you fail to call
*	CDGDraw() at regular intervals, the cdg.library may buffer
*	the data, but audio synchronization could be audibly affected.
*
*	The same problem is of course true for +G (though less
*	significant).  There is another problem with +G though.  There
*	is no synchronization between VBLANK rate, and scrolling effects
*	in +G disks.  So a +G scroll command may look smooth on one
*	system, but "jerk" slightly on another.  New PACKETS are
*	received 75x per second, while VBLANK rate may be 60x or 50x
*	per second.
*
*	The cdg.library batches up color changes, and scroll commands for
*	the next complete video frame (around VBLANK time).  This is done to
*	so that each video frame has a stable/solid look, but means that a
*	scroll command or color change command may have to wait as much
*	as one complete video frame before being displayed.  For color
*	changes this is not visually noticeable, but for smooth scrolling
*	effects lack of synchronization can be seen.
*
***********************************************************************


		END