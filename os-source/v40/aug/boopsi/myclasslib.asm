*************************************************************************
*
*   classlib.asm -- public boopsi class library
*
*   Copyright (C) 1985, 1989, 1990 Commodore Amiga Inc.
*	All rights reserved.
*
*************************************************************************

   SECTION   section

   NOLIST
   INCLUDE "exec/types.i"
   INCLUDE "exec/libraries.i"
   INCLUDE "exec/lists.i"
   INCLUDE "exec/alerts.i"
   INCLUDE "exec/initializers.i"
   INCLUDE "exec/resident.i"
   INCLUDE "libraries/dos.i"

   INCLUDE "myclassbase.i"

VERSION		EQU	36
REVISION	EQU	1
VSTRING	MACRO
		dc.b	'myclass 36.1 (18.6.90)',13,10,0
	ENDM


CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

CALLSYS	MACRO
	JSR	_LVO\1(A6)
	ENDM

XLIB	MACRO
	XREF	_LVO\1
	ENDM

   LIST

   XREF		_myLibInit
   XREF		_myLibOpen
   XREF		_myLibClose
   XREF		_myLibExpunge

   XLIB   FreeMem
   XLIB   Remove

Start:
   MOVEQ   #-1,d0
   rts

; ---- library definition ----
myName:	DC.B   'myclass.library',0
MYPRI   	EQU   -20
idString	VSTRING		; macro from automatic myclass_rev.i

   ; force word alignment
   ds.w   0

initDDescrip:
               ;STRUCTURE RT,0
     DC.W    RTC_MATCHWORD      ; UWORD RT_MATCHWORD
     DC.L    initDDescrip       ; APTR  RT_MATCHTAG
     DC.L    EndCode            ; APTR  RT_ENDSKIP
     DC.B    RTF_AUTOINIT       ; UBYTE RT_FLAGS
     DC.B    VERSION            ; UBYTE RT_VERSION
     DC.B    NT_LIBRARY         ; UBYTE RT_TYPE
     DC.B    MYPRI              ; BYTE  RT_PRI
     DC.L    myName         	; APTR  RT_NAME
     DC.L    idString           ; APTR  RT_IDSTRING
     DC.L    Init               ; APTR  RT_INIT

   ; force word alignment
   ds.w   0

Init:
   DC.L   MyLibBase_SIZEOF  ; size of library base data space
   DC.L   myFuncTable        ; pointer to function initializers
   DC.L   dataTable         ; pointer to data initializers
   DC.L   _myLibInit       ; routine to run


dataTable:
   INITBYTE   LN_TYPE,NT_LIBRARY
   INITLONG   LN_NAME,myName
   INITBYTE   LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
   INITWORD   LIB_VERSION,VERSION
   INITWORD   LIB_REVISION,REVISION
   INITLONG   LIB_IDSTRING,idString
   DC.L   0


Null:
   CLEAR   d0
   rts

myFuncTable:
	dc.l   _myLibOpen
	dc.l   _myLibClose
	dc.l   _myLibExpunge
	dc.l   Null
	dc.l   -1

EndCode:

   END
