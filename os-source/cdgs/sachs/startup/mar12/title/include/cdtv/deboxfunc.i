; :ts=8
*
*	deboxfunc.i
*
*	William A. Ware			9006.20
*
*************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY			*
*   Copyright (C) 1990, Silent Software Incorporated.			*
*   All Rights Reserved.						*
*************************************************************************


FUNCDEF     MACRO    *function
_LVO\1      EQU      FUNC_CNT
FUNC_CNT    SET      FUNC_CNT-6
            ENDM
FUNC_CNT    SET      5*-6

	; Private
	FUNCDEF Decomp
	FUNCDEF	STDDecomp
	FUNCDEF Eor
	; Public
	;   header
	FUNCDEF CheckHeader
	FUNCDEF	HeaderSize
	FUNCDEF	NextComp
	;   decompression
	FUNCDEF DecompData
	;   bitmap decompression
	FUNCDEF	FindBMInfoSize
	FUNCDEF	DecompBMInfo
	FUNCDEF FreeBMInfo
	FUNCDEF DecompBitMap
	;   other
	FUNCDEF	MemSet
	FUNCDEF	AllocBitMap
	FUNCDEF FreeBitMap
	;   view
	FUNCDEF	CreateView
	FUNCDEF	DeleteView
	FUNCDEF	CenterViewPort
	;   cycle
	FUNCDEF	CycleColors