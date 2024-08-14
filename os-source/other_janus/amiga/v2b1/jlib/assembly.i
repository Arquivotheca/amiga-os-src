
;***********************************************************************
;
; assembly.i -- included by all janus code before anything else
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved
;
;***********************************************************************



	SECTION section

INFOLEVEL	EQU	0	; No messages
; INFOLEVEL	EQU	1	; Primary function entry argument messages
; INFOLEVEL	EQU	2	; Primary and support messages
; INFOLEVEL	EQU	59	; Mid-range messages
; INFOLEVEL	EQU	80	; Most messages
; INFOLEVEL	EQU	100	; Practically *all* messages (hoo boy!)


; hard coded signals
ACPSB_CALL	EQU	31
ACPSF_CALL	EQU	$8000
ACPSB_EXIT	EQU	30
ACPSF_EXIT	EQU	$4000

