
* *****************************************************************************
* 
* abs.asm -- resident tag for low level janus code
* 
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 
* *****************************************************************************



   INCLUDE 'assembly.i'

   NOLIST
   INCLUDE 'exec/types.i'
   INCLUDE 'exec/nodes.i'
   INCLUDE 'exec/resident.i'
   INCLUDE 'janus.library_rev.i'
   LIST

   XDEF   Start
;   XDEF   VERNUM
;   XDEF   REVNUM
   XDEF   MY_VERSION
   XDEF   MY_REVISION
   XDEF   VERSTRING
   XDEF   subsysName

   XREF   InitTable
   XREF   EndCode

Start:
   moveq   #0,d0
   rts


;VERNUM   EQU   33
;REVNUM   EQU   4

VERSTRING   VSTRING
;VERSTRING   DC.B    'janus 33.4 -- 11 Sep 1989',10,0
subsysName     DC.B   'janus.library',0
MY_VERSION    DC.W   VERSION
;MY_VERSION    DC.W   33
MY_REVISION   DC.W   REVISION
     DS.W 0

initDDescrip:

   dc.w   RTC_MATCHWORD       ; UWORD RT_MATCHWORD 
   dc.l   initDDescrip       ; APTR  RT_MATCHTAG
   dc.l   EndCode        ; APTR  RT_ENDSKIP
   dc.b   RTF_AUTOINIT       ; UBYTE RT_FLAGS
;   dc.b   VERNUM          ; UBYTE RT_VERSION
   dc.b   VERSION
   dc.b   NT_LIBRARY       ; UBYTE RT_TYPE
   dc.b   0          ; UBYTE RT_PRI
   dc.l   subsysName       ; APTR  RT_NAME
   dc.l   VERSTRING       ; APTR  RT_IDSTRING
   dc.l   InitTable       ; APTR  RT_INIT

   END


