;   bootlibtag.asm -- romtag for bootlib

   include   'exec/types.i'
   include   'exec/resident.i'
   include   'exec/nodes.i'
   include   'exec/libraries.i'

   include   'Magic_rev.i'

MYPRI       equ   -45

   xref  @MyInit
   xref  EndCode
   csect code

_myRomTag:
   dc.w   RTC_MATCHWORD    ; UWORD RT_MATCHWORD
   dc.l   _myRomTag        ; APTR  RT_MATCHTAG
   dc.l   EndCode          ; APTR  RT_ENDSKIP
   dc.b   RTF_COLDSTART    ; UBYTE RT_FLAGS
   dc.b   VERSION          ; UBYTE RT_VERSION
   dc.b   0                ; UBYTE RT_TYPE
   dc.b   MYPRI            ; BYTE  RT_PRI
   dc.l   MyName           ; APTR  RT_NAME
   dc.l   MyID             ; APTR  RT_IDSTRING
   dc.l   @MyInit          ; APTR  RT_INIT
endtag:

MyName:  VERS
         dc.b 0
MyID:    VSTRING

   end
