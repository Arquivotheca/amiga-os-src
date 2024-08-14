******************************************************************************
*
*	$Id: game.i,v 40.2 93/02/19 17:51:11 Jim2 Exp $
*
******************************************************************************
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

    IFND    LIBRARIES_GAME_I
LIBRARIES_GAME_I  SET 1

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

    IFND	UTILITY_TAGITEM_I
	INCLUDE	'utility/tagitem.i'
    ENDC

 STRUCTURE KeyQuery,0
	UWORD	kq_KeyCode
	UWORD	kq_Pressed
	LABEL	KeyQuery_SIZEOF


	BITDEF	GK,LSHIFT,16
	BITDEF	GK,RSHIFT,17
	BITDEF	GK,CAPSLOCK,18
	BITDEF	GK,CONTROL,19
	BITDEF	GK,LALT,20
	BITDEF	GK,RALT,21
	BITDEF	GK,LAMIGA,22
	BITDEF	GK,RAMIGA,23

;
; ***** ReadJoyPort() Return value equates *****
;

;
; Port Type Equates :
;
JP_TYPE_MASK		equ	(15<<28)	; controller type

JP_TYPE_NOTAVAIL	equ	(00<<28)	; port data unavailable
JP_TYPE_GAMECTLR	equ	(01<<28)	; port has game controller
JP_TYPE_MOUSE		equ	(02<<28)	; port has mouse
JP_TYPE_JOYSTK		equ	(03<<28)	; port has joystick
JP_TYPE_UNKNOWN		equ	(04<<28)	; port has unknown device

;
; Button Equates :
;
	BITDEF	JP,BTN1,23			; Blue - Stop; Right Mouse
	BITDEF	JP,BTN2,22			; Red - Select; Left Mouse; Joystick Fire
	BITDEF	JP,BTN3,21			; Yellow - Repeat
	BITDEF	JP,BTN4,20			; Green - Shuffle
	BITDEF	JP,BTN5,19			; Charcoal - Forward
	BITDEF	JP,BTN6,18			; Charcoal - Reverse
	BITDEF	JP,BTN7,17			; Grey - Play/Pause
	BITDEF	JP,RSVDBTN,16			; Reserved
;
; Direction Equates :
;
	BITDEF	JP,UP,3				; Up
	BITDEF	JP,DOWN,2			; Down
	BITDEF	JP,LEFT,1			; Left
	BITDEF	JP,RIGHT,0			; Right
;
; Mouse Equates :
;
JP_MHORZ_MASK		equ	(255<<0)	; horizontal position
JP_MVERT_MASK		equ	(255<<8)	; vertical position



*
* Tags for SystemControl()
*

SCON_TakeOverSys	EQU	(TAG_USER+$00C00000)
SCON_KillReq	EQU	(SCON_TakeOverSys+1)
SCON_CDReboot	EQU	(SCON_KillReq+1)
SCON_StopInput	EQU	(SCON_CDReboot+1)

CDReboot_On	EQU	1
CDReboot_Off	EQU	0
CDReboot_Default EQU	2

*
* Human Language Defines
*
GAME_LANG_UNKNOWN	EQU	0
GAME_LANG_AMERICAN	EQU	1	/* American English	*/
GAME_LANG_ENGLISH	EQU	2	/* British English	*/
GAME_LANG_GERMAN	EQU	3
GAME_LANG_FRENCH	EQU	4
GAME_LANG_SPANISH	EQU	5
GAME_LANG_ITALIAN	EQU	6
GAME_LANG_PORTUGUESE	EQU	7
GAME_LANG_DANISH	EQU	8
GAME_LANG_DUTCH		EQU	9
GAME_LANG_NORWEGIAN	EQU	10
GAME_LANG_FINNISH	EQU	11
GAME_LANG_SWEDISH	EQU	12
GAME_LANG_JAPANESE	EQU	13
GAME_LANG_CHINESE	EQU	14
GAME_LANG_ARABIC	EQU	15
GAME_LANG_GREEK		EQU	16
GAME_LANG_HEBREW	EQU	17
GAME_LANG_KOREAN	EQU	18

    ENDC
