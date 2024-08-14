	OPTIMON

;---------------------------------------------------------------------------

	XDEF	_RndSqRoot

;---------------------------------------------------------------------------

; This routine returns the root of the nearest perfect root to its
; argument i.e. it rounds instead of truncating.
; note - remainder is also adjusted and can be negative.
;
; LONG ASM RndSqRoot(REG(d1) LONG i);

_RndSqRoot:
            move.l  d2,a0 		; save these
            move.l  d3,a1

            moveq.l #15,d2              ; number of iterations
            moveq   #0,d3               ; clear the remainder
            moveq   #0,d0               ; clear trial value and final result

1$          lsl.l   #1,d0               ; double partial result
            addq.l  #1,d0               ; guess next bit is a 1
            lsl.l   #1,d1               ; fetch 2 new bits
            roxl.l  #1,d3               ; from argument
            lsl.l   #1,d1
            roxl.l  #1,d3
            sub.l   d0,d3               ; do a trial subtraction
            bcc.s   2$                  ; guess was right,
                                        ; append a 1 bit
            add.l   d0,d3               ; guess was wrong, put it back
            subq.l  #1,d0               ; and clean up for next pass
            dbra    d2,1$
            bra.s   3$                  ; go scale result
2$          addq.l  #1,d0               ; convert xxxx01 to xxxx10, i.e.
                                        ; append a 1 bit
            dbra    d2,1$
3$          asr.l   #1,d0               ; divide by 2 to get actual root

            move.w  d0,d2               ; get result in d2
            add.w   d2,d2               ; double it.
            cmp.w   d2,d3               ; compare root*2 with remainder
            bge.s   4$                  ; if remainder less than twice root
            sub.w   d0,d3               ; subtract (d0 + (d0+1)) from remainder
            addq.w  #1,d0               ; then add 1 to result.
            sub.w   d0,d3               ; as above
            move.l  d3,d1               ; and move to d1

4$          move.l  a0,d2		; restore these
	    move.l  a1,d3
            rts

;---------------------------------------------------------------------------

	END
