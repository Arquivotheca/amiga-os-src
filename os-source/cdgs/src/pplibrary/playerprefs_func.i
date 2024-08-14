; :ts=8 
*
*	playerprefs_func.i 
*
*	William A. Ware			AC04
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


*
* Programs 
*
	FUNCDEF	DoPlayer
	FUNCDEF	DoPrefs
*
*
* Title Screen
*
	FUNCDEF	CDTVTitle
	FUNCDEF	SetVersionStr
*
*
* Prefs Function
*
	FUNCDEF	FillCDTVPrefs
	FUNCDEF	ConfigureCDTV
*
*
* Screen Blanker
*
	FUNCDEF	InstallScreenSaver
	FUNCDEF	ScreenSaverCommand
*
* Key click module.
*
	FUNCDEF	InstallKeyClick
	FUNCDEF	KeyClickCommand
*
* Utilities
* 
	FUNCDEF	SafeWaitIO
	FUNCDEF	FreeIORequest
	FUNCDEF	AllocIORequest
	FUNCDEF	CenterCDTVView
*
* Bookmark Utilities
*
	FUNCDEF	SaveCDTVPrefs
	FUNCDEF	DeleteBookMark
	FUNCDEF	WriteBookMark
	FUNCDEF	ReadBookMark
*
* TRADEMARK SCREEN
*
	FUNCDEF	PrepareTM
	FUNCDEF	FreeTM
	FUNCDEF	DisplayTM
*
* Color map manipulation
*
	FUNCDEF	FindViewRGB
	FUNCDEF	LoadFoundRGB
	FUNCDEF	LoadFoundRGBFade
	FUNCDEF	LevelColor
	FUNCDEF	BetweenColor
	FUNCDEF	FadeCMap
*
*
* Extra Bit map routines 
*
	FUNCDEF	GrabBm
	FUNCDEF	FreeGrabBm
	FUNCDEF	CreateMask
	FUNCDEF	FillBackDrop
	FUNCDEF	FillForground
	FUNCDEF	BuildBg
*
*
* View Manager stuff - private
*
	FUNCDEF	StartViewMgr
	FUNCDEF	CloseViewMgr
	FUNCDEF	LoadCycleView
*
*
* Other - Very private routines.
*
	FUNCDEF	WriteTinyStr
	FUNCDEF	DecompTradeMark
*
* - More General utilities.
*
	FUNCDEF SafeDoIO
*
*
*
	FUNCDEF InstallJoyMouse
	FUNCDEF RemoveJoyMouse
