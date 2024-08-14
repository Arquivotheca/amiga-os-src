; :ts=8 
*
*	cdtv.i -- Routines 
*
*	William A. Ware			AC04
*
*************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY			*
*   Copyright (C) 1990, Silent Software Incorporated.			*
*   All Rights Reserved.						*
*************************************************************************

 INCLUDE "exec/interrupts.i"
 INCLUDE "exec/io.i"
 INCLUDE "devices/timer.i"


 STRUCTURE CycIntInfo,IS_SIZE
	APTR	cintd_BMInfo
	APTR	cintd_View
	APTR	cintd_CCList
	LONG	cintd_VCountDown
	LONG	cintd_VCountUp
	UBYTE	cintd_Bump
	UBYTE	cintd_Flags
; fade
	LONG	cintd_FadeMask
; fade 1
	UBYTE	cintd_Fade0
	UBYTE	cintd_DestFade0
	LONG	cintd_MicroFadeDelay0
; fade 2
	UBYTE	cintd_Fade1
	UBYTE	cintd_DestFade1
	LONG	cintd_MicroFadeDelay1
	APTR	cintd_AltCMap
; load
	APTR	cintd_LoadCCList
	APTR	cintd_LoadView
; private
	LONG	cintd_vtime 
	LONG	cintd_fadetimer0
	LONG	cintd_fadetimer1
 	APTR	cintd_PlayerPrefsBase
	APTR	cintd_GfxBase
 	APTR	cintd_DeBoxBase
	APTR	cintd_oldview
	LABEL	cintdata_SIZEOF

CINTDB_NOCYCLE	equ	0

