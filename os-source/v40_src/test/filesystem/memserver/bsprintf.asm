*
*
* Bryce SPrintF() function.  Lifted from doc/exec.doc.
* This thing (a) ought to work, and (b) should be assembled with CAPE.
*
* ANOTHER WARNING:  This does NO LENGTH CHECKING.
*                   Make very sure that you don't exceed the size of
*                   your target buffer!
*
*        ;
*        ; Simple version of the C "sprintf" function.  Assumes C-style
*        ; stack-based function conventions.
*        ;
*        ;   long eyecount;
*        ;   eyecount=2;
*        ;   sprintf(string,"%s have %ld eyes.","Fish",eyecount);
*        ;
*        ; would produce "Fish have 2 eyes." in the string buffer.
*        ;
                XDEF _bsprintf
_LVORawDoFmt     EQU     -$020A          ; bleargh, but it beats linking with amiga.lib
                                         ; for residentable code...

        _bsprintf:       ; ( ostring, format, {values} )
                movem.l a2/a3/a6,-(sp)   ; Save old registers

                move.l  4*4(sp),a3       ; Get the output string pointer
                move.l  5*4(sp),a0       ; Get the FormatString pointer
                lea.l   6*4(sp),a1       ; Get the pointer to the DataStream
                lea.l   stuffChar(pc),a2 ; stuff the character in the output stream
                move.l  (4),a6           ; get ready for Exec call
                jsr     _LVORawDoFmt(a6) ; actually call RawDoFmt()

                movem.l (sp)+,a2/a3/a6   ; restore registers
                rts                      ; we're outta here

        ;------ PutChProc function used by RawDoFmt -----------
        stuffChar:
                move.b  d0,(a3)+        ;Put data to output string
                rts

*   WARNING
*        This Amiga ROM function formats word values in the data stream.  If
*        your compiler defaults to longs, you must add an "l" to your
*        % specifications.  This can get strange for characters, which might
*        look like "%lc".
*
*   SEE ALSO
*        amiga.lib/printf()

end
