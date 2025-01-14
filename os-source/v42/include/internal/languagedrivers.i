        IFND    INTERNAL_LANGUAGEDRIVERS_I
INTERNAL_LANGUAGEDRIVERS_I      SET     1
**
**      $Id: languagedrivers.i,v 38.1 91/06/26 15:23:01 vertex Exp $
**
**      language driver stuff for locale.library
**
**      (C) Copyright 1991 Commodore-Amiga, Inc.
**      All Rights Reserved
**

;---------------------------------------------------------------------------

; Functions implemented by a driver. These values are used in the longword
; returned by the GetDriverInfo() entry point of a language driver.

	BITDEF GDI,CONVTOLOWER,0
	BITDEF GDI,CONVTOUPPER,1
	BITDEF GDI,GETCODESET,2
	BITDEF GDI,GETLOCALESTR,3
	BITDEF GDI,ISALNUM,4
	BITDEF GDI,ISALPHA,5
	BITDEF GDI,ISCNTRL,6
	BITDEF GDI,ISDIGIT,7
	BITDEF GDI,ISGRAPH,8
	BITDEF GDI,ISLOWER,9
	BITDEF GDI,ISPRINT,10
	BITDEF GDI,ISPUNCT,11
	BITDEF GDI,ISSPACE,12
	BITDEF GDI,ISUPPER,13
	BITDEF GDI,ISXDIGIT,14
	BITDEF GDI,STRCONVERT,15
	BITDEF GDI,STRNCMP,16

ALL_FUNCS EQU $0001FFFF

;---------------------------------------------------------------------------

        ENDC    ; INTERNAL_LANGUAGEDRIVERS_I
