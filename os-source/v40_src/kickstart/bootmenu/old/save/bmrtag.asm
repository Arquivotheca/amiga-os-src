;   bmrtag.asm -- romtag for bootlib

   include   'exec/types.i'
   include   'exec/resident.i'
   include   'exec/nodes.i'
   include   'exec/libraries.i'

   include   'bootmenu_rev.i'

MYPRI       equ   -50

   xref  @MyInit
   xref  EndCode
   xref  SC_rtag
   csect code

_myRomTag:
   dc.w   RTC_MATCHWORD    ; UWORD RT_MATCHWORD
   dc.l   _myRomTag        ; APTR  RT_MATCHTAG
   IF SYSCHECK
   dc.l   SC_rtag          ; APTR  RT_ENDSKIP
   ELSE
   dc.l   EndCode          ; APTR  RT_ENDSKIP
   ENDC
   dc.b   RTF_COLDSTART    ; UBYTE RT_FLAGS
   dc.b   VERSION          ; UBYTE RT_VERSION
   dc.b   0                ; UBYTE RT_TYPE
   dc.b   MYPRI            ; BYTE  RT_PRI
   dc.l   MyName           ; APTR  RT_NAME
   dc.l   MyID             ; APTR  RT_IDSTRING
   dc.l   @MyInit          ; APTR  RT_INIT
endtag:

MyName:  dc.b 'bootmenu'
         dc.b 0
MyID:    VSTRING

   end
