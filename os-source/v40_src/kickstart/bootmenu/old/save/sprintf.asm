
   XDEF  _sprintf

   XREF  _AbsExecBase
   XREF  _LVORawDoFmt

   csect code

_sprintf:   ;(ostring,format,{values})
   movem.l  a2/a3/a6,-(sp)

   move.l   4*4(sp),a3  ;output string ptr
   move.l   5*4(sp),a0  ;format string ptr
   lea.l    6*4(sp),a1  ;ptr to datastream
   lea.l    stuffChar(pc),a2
   move.l   _AbsExecBase,a6
   jsr      _LVORawDoFmt(a6)

   movem.l  (sp)+,a2/a3/a6
   rts

stuffChar:
   move.b   d0,(a3)+ ;put data to output string
   rts

   end
