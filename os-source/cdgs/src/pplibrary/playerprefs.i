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

 STRUCTURE 	LibInfo,0
	APTR	LI_name
	LONG	LI_version
	APTR	LI_libbase
	LABEL   LibInfo_SIZEOF


 STRUCTURE CycIntInfo,IS_SIZE
	APTR	cintd_BMInfo
	APTR	cintd_View
	APTR	cintd_CCList
	LONG	cintd_VCountDown
	LONG	cintd_VCountUp
	UBYTE	cintd_Bump
	UBYTE	cintd_Flags
	UBYTE	cintd_Fade
	UBYTE	cintd_DestFade
	LONG	cintd_MicroFadeDelay
	APTR	cintd_LoadCCList
	APTR	cintd_LoadView
		; private
	LONG	cintd_vtime 
	LONG	cintd_fadetimer
 	APTR	cintd_GfxBase
 	APTR	cintd_DeBoxBase
	APTR	cintd_oldview
	LABEL	cintdata_SIZEOF

CINTD_NOCYCLEB	equ	0


 STRUCTURE	CDTVEvent,MN_SIZE
	UWORD	cdtve_Qualifier
	UWORD	cdtve_Code
	LABEL	cdtve_SIZE


 STRUCTURE	InputData,IS_SIZE
 	STRUCT 	IPD_MsgPort,MP_SIZE
 
	LONG	IPD_MouseWait
 	UWORD	IPD_MouseTDist
	STRUCT	IPD_mousetime,TV_SIZE
	UWORD	IPD_mousemove
	UBYTE	IPD_mousedir
	UBYTE	IPD_CurrButtonPos
	UWORD	IPD_mousequal
	
 	APTR	IPD_input_io
	STRUCT	IPD_returnport,MP_SIZE	; Where the messages come back 
	WORD	IPD_MouseXmove
	WORD	IPD_MouseYmove
	LABEL	InputData_SIZEOF


 BITDEF IPD,BUTTONA,14
 BITDEF IPD,BUTTONB,13
 BITDEF IPD,UP,1

BUTTONA_DOWN    equ 	$4000
BUTTONA_UP      equ	$4001
BUTTONB_DOWN    equ	$2000
BUTTONB_UP      equ	$2001

BOXMOVE         equ	$8000

 BITDEF	IPD,CURRBUTTONA,1
 BITDEF IPD,CURRBUTTONB,0

DIR_UP          equ	$8004
DIR_DOWN        equ	$8008
DIR_LEFT        equ	$8001
DIR_RIGHT       equ	$8002

