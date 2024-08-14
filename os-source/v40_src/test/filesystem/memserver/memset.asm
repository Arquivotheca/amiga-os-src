
                XDEF _memSet

        _memSet:                         ; ( void *dst, ULONG pat, ULONG size )
                movem.l d2-d3/a3,-(sp)   ; Save old registers

                movea.l 4*4(sp),a3       ; Get the destination pointer
                move.l  5*4(sp),d2       ; Get the pattern to write
                move.l  6*4(sp),d3       ; Get the size of the memblock

                bne.b   lp2              ; if memblock size != 0, proceed.
                bra.b   done             ; else exit
        loop:   move.l  d2,(a3)+         ; dst++ = pat
        lp2:    dbf     d3,loop          ; size--, if size, goto loop

*        loop:   tst.l   d3               ; == 0?
*                beq.b   done             ; if yes, goto done
*                subq.l  #1,d3            ; d3--
*                move.l  d2,(a3)+         ; dst++ = pat
*                bra.b   loop             ; goto loop

        done:
                movem.l (sp)+,d2-d3/a3   ; restore registers
                rts                      ; we're outta here

end
