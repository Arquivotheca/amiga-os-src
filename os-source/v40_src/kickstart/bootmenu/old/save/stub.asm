
   xdef _MyBltBitMapRastPort
   xref _LVOBltBitMapRastPort

   csect code

_MyBltBitMapRastPort:

   jsr _LVOBltBitMapRastPort(a6)
   rts

   end
