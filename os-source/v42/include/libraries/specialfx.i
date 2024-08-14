    IFND	LIBRARIES_SPECIALFX_I
LIBRARIES_SPECIALFX_I	SET	1

**
**	$Id: specialfx.i,v 39.15 93/09/14 18:54:43 spence Exp $
**
**	Special Effects header file
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

SFX_ColorControl	EQU	$80000001
SFX_LineControl		EQU	$80000002
SFX_IntControl		EQU	$80000003
SFX_FineVideoControl	EQU	$80000004
SFX_SpriteControl	EQU	$80000005

* Pass these tags to InstallFXA()
SFX_InstallEffect	EQU	$80001000
SFX_InstallErrorCode	EQU	$80001001
SFX_DoubleBuffer	EQU	$80001002

    STRUCTURE	ColorControl,0
	ULONG	cc_Pen
	UWORD	cc_Horizontal
	UWORD	cc_Line
	ULONG	cc_Red
	ULONG	cc_Green
	ULONG	cc_Blue
	UWORD	cc_Flags;
	UWORD	cc_Pad
    LABEL	cc_SIZEOF

    BITDEF	CC,MODIFY,0	; Set to modify this Colour
    BITDEF	CC,RESTORE,1	; Set to restore the pen to its original value


    STRUCTURE	LineControl,0
	APTR	lc_RasInfo
	UWORD	lc_Line
	UWORD	lc_Count
	UWORD	lc_Flags
	WORD	lc_SkipRateEven
	WORD	lc_SkipRateOdd
	UWORD	lc_Pad
    LABEL	lc_SIZEOF

    BITDEF	LC,MODIFY,0


    STRUCTURE	InterruptControl,0
	UWORD	inc_Line
	UWORD	inc_Flags
    LABEL	inc_SIZEOF

    BITDEF	INC,SET,0
    BITDEF	INC,RESET,1


    STRUCTURE	FineVideoControl,0
	APTR	fvc_TagList
	UWORD	fvc_Line
	WORD	fvc_Count
	UWORD	fvc_Flags
	UWORD	fvc_Pad
    LABEL	fvc_SIZEOF

    BITDEF	FVC,MODIFY,0


VTAG_SFX_DEPTH_SET	EQU	$c0000000
VTAG_SFX_HAM_SET	EQU	$c0000001
VTAG_SFX_EHB_SET	EQU	$c0000002
VTAG_SFX_NORM_SET	EQU	$c0000003
VTAG_SFX_DPF_SET	EQU	$c0000004
VTAG_SFX_PF1PRI		EQU	$c0000005
VTAG_SFX_PF2PRI		EQU	$c0000006


    STRUCTURE	SpriteControl,0
	APTR	spc_ExtSprite
	APTR	spc_TagList
	UWORD	spc_Line
	UWORD	spc_Flags
    LABEL	spc_SIZEOF

    BITDEF	SPC,MODIFY,0

SFX_ERR_NoMem	EQU	1
SFX_ERR_MakeVP	EQU	2
SFX_ERR_Animating	EQU	3
SFX_ERR_NotSupported	EQU	4


SPECIALFXNAME	MACRO
    dc.b	"specialfx.library",0
    ENDM

    ENDC	; LIBRARIES_SPECIALFX_I
