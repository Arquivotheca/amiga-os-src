    IFND    SPECIALFX_SPECIALFXBASE_I
SPECIALFX_SPECIALFXBASE_I SET 1
**
**	$Id: specialfxbase.i,v 39.8 93/09/17 18:18:47 spence Exp $
**
**	SpecialFX base definitions
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

    IFND    EXEC_LISTS_I
    include 'exec/lists.i'
    ENDC
    IFND    EXEC_LIBRARIES_I
    include 'exec/libraries.i'
    ENDC

MAX_VPLIST	equ	2
MAX_AHLIST	equ	20

    STRUCTURE	VPList,MLN_SIZE
	UWORD	vpl_Count		; count the entries
	UWORD	vpl_Pad
	STRUCT	vpl_ViewPorts,(MAX_VPLIST*4)	; space for 20 ViewPorts
    LABEL	vpl_SIZEOF

    STRUCTURE	AHList,MLN_SIZE
	UWORD	ahl_Count		; count the entries
	UWORD	ahl_Pad
	STRUCT	ahl_AnimHandles,(MAX_AHLIST*4)	; space for 20 AnimHandles
    LABEL	ahl_SIZEOF

    STRUCTURE  SpecialFXBase,LIB_SIZE
	UWORD	sfxb_Flags
	APTR	sfxb_ExecBase
	APTR	sfxb_GfxBase
	APTR	sfxb_UtilityBase
	APTR	sfxb_SegList
	APTR	sfxb_VPListSemaphore
	STRUCT	sfxb_VPListHead,MLH_SIZE
	STRUCT	sfxb_VPList,vpl_SIZEOF
	APTR	sfxb_AHListSemaphore
	STRUCT	sfxb_AHListHead,MLH_SIZE
	STRUCT	sfxb_AHList,ahl_SIZEOF
	APTR	sfxb_AllocVectors
	APTR	sfxb_CountVectors
	APTR	sfxb_InstallVectors
	APTR	sfxb_RemoveVectors
	APTR	sfxb_AnimateVectors
	APTR	sfxb_RebuildVectors
	APTR	sfxb_OldMrgCop
	APTR	sfxb_CopperLCache
	APTR	sfxb_CopperSCache
	ULONG	sfxb_CopperLSize
	ULONG	sfxb_CopperSSize
	APTR	sfxb_AnimationSemaphore
	APTR	sfxb_OldLoadView
    LABEL	SpecialFXBase_SIZEOF

    ENDC
