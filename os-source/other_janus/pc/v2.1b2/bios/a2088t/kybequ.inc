	IFNDEF LISTINC
.XLIST
.XCREF
	ENDIF
;	NAME	KYBEQU
;	TITLE	Equates for the Keyboard Controller
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
;
;------------------------------------------------------------------------------
;
; Keyboard LED and Flags Data Area bit equates
;
;	EQU	80h			; BIT7 = Used for a flag to indicate
					;   	 3 failures of sending data 
					;   	 to keyboard
LEDUPDT	EQU	40h			; BIT6 = LED update in progress
RSNDRCV	EQU	20h			; BIT5 = Resend received from keyboard
ACKRCV	EQU	10h			; BIT4 = Acknowledge received from keyboard
;	EQU	08h			; BIT3 = 
CAPLEDS	EQU	04h			; BIT2 = Capslock LED status
NUMLEDS	EQU	02h			; BIT1 = Numlock LED status
SCRLEDS	EQU	01h			; BIT0 = Scroll lock LED status
;
;------------------------------------------------------------------------------
;
; Keyboard Shift Flags Bit equates
;
INSTAT	EQU	80h			; BIT7 = INSERT STATE ON/OFF
CAPLCK	EQU	40h			; BIT6 = CAPS LOCK ON/OFF
NUMLCK	EQU	20h			; BIT5 = NUM LOCK ON/OFF
SCRLCK	EQU	10h			; BIT4 = SCROLL LOCK ON/OFF
ALTSHF	EQU	08h			; BIT3 = ALT KEY DEPRESSED
CTRLSHF	EQU	04h			; BIT2 = CTRL KEY DEPRESSED
LEFTSHF	EQU	02h			; BIT1 = LEFT SHIFT KEY DEPRESSED
RGHTSHF	EQU	01h			; BIT0 = RIGHT SHIFT KEY DEPRESSED
;
;------------------------------------------------------------------------------
;
.CREF
.LIST
