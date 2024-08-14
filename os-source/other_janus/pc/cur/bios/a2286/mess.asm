;*************************************************************************
;	Commodore Copyright message
;*************************************************************************
commodore:
 db '�������������������������������������������������������������Ŀ',0dh,0ah
IF BRIDGEBOARD
 db '� Commodore A2286 BIOS Rev. 4.3               380682 - 04     �',0dh,0ah
 DB '�                                             380683 - 04     �',0dh,0ah
ELSE
 db '� Commodore 286 BIOS  Rev. 2.04.00            390339 - 05     �',0dh,0ah
 DB '�                                             390340 - 05     �',0dh,0ah
ENDIF
 db '�������������������������������������������������������������Ĵ',0dh,0ah
IF BRIDGEBOARD
 db '� Copyright (C) 1988, 1991 by Commodore Electronics Ltd.      �',0dh,0ah
ELSE
 db '� Copyright (C) 1988, 1991 by Commodore Electronics Ltd.      �',0dh,0ah
ENDIF
 db '�                    All Rights Reserved.                     �',0dh,0ah
 db '���������������������������������������������������������������',0dh,0ah
 db 0dh,0ah
comm_length	equ	$-commodore

commodore_s:
 db '�������������������������������������Ŀ',0dh,0ah
IF BRIDGEBOARD
 db '� Commodore A2286 BIOS      380682-04 �',0dh,0ah
 DB '�       Rev. 4.3            380683-04 �',0dh,0ah
ELSE
 db '� Commodore 286 BIOS V2.04  390339-05 �',0dh,0ah
 DB '�                           390340-05 �',0dh,0ah
ENDIF
 db '�������������������������������������Ĵ',0dh,0ah
IF BRIDGEBOARD
 db '�     Copyright (C) 1988, 1991 by     �',0dh,0ah
 db '�     Commodore Electronics Ltd.      �',0dh,0ah
 db '�        All Rights Reserved.         �',0dh,0ah
ELSE
 db '�     Copyright (C) 1988, 1991 by     �',0dh,0ah
 db '�     Commodore Electronics Ltd.      �',0dh,0ah
 db '�        All Rights Reserved.         �',0dh,0ah
ENDIF
 db '���������������������������������������',0dh,0ah
 db 0dh,0ah
comm_length_s	equ	$-commodore_s

;*************************************************************************
;	assorted machine resource messages
;*************************************************************************
mess1:		db	'Com     at '
mess1_length	equ	$-mess1
mess2:		db	'Lpt     at '
mess2_length	equ	$-mess2
mess3:		db	'Mouse   at '
mess3_length	equ	$-mess3
mess6:		db	0dh,0ah,'Expansion '
mess6_length	equ	$-mess6
mess7:		db	0dh,0ah,'Onboard   '
mess7_length	equ	$-mess7
mess10:		db	0dh,0ah,'80287 Numeric Coprocessor Installed'
		db	0dh,0ah
mess10_length	equ	$-mess10

mess8		db	'Game    at '
mess8_length	equ	$-mess8

COM_ERR_MSG	DB	0dh,0ah,'-- The CMOS setting for on board COM',0dh,0ah
		db	'   conflicts with external COM port',0dh,0ah
COM_ERR_LENGTH	EQU	$-COM_ERR_MSG

LPT_ERR_MSG	DB	0dh,0ah,'-- The CMOS setting for on board LPT',0dh,0ah
		db	'   conflicts with external LPT port',0dh,0ah
LPT_ERR_LENGTH	EQU	$-LPT_ERR_MSG
