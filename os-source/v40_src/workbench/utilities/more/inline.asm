
*
* inLine.asm --- 04/88 cas
*
*   Inputs:  on stack, pointer to \0 term'd line, and \0 term'd string
*   Returns: 1 if string found in line, 0 if not
*
   INCLUDE   "exec/types.i"

   XDEF   _inLine

	SECTION CODE

_inLine:
   movem.l  a0-a3,-(sp)            ;save a0-a3
   move.l   20(sp),a0              ;first (big) string to a0
   move.l   a0,a1                  ; and a1
   move.l   24(sp),a2              ;second (small) string to a2
   move.l   a2,a3                  ; and a3

   tst.b    (a3)                   ;if null small string
   beq.s    nomatch                ;  there's no match

   cmpi.b   #10,(a0)               ;if big string just linefeed
   beq.s    nomatch                ; it's not a match
   
loop:
   tst.b    (a3)                   ;if got to small string null
   beq.s    match                  ;  it's a match

   cmpm.b   (a1)+,(a3)+            ;else compare chars and bump index
   bne.s    nomatchyet             ;if different, try at next position
   bra.s    loop                   ;else check next char

nomatchyet:
   addq.l   #1,a0                  ;try match at next position
   tst.b    (a0)                   ;if at end of big string (null)
   beq.s    nomatch                ; it's not a match
   move.l   a0,a1                  ;else update big string ptr
   move.l   a2,a3                  ;reset small string pointer
   bra.s    loop

match:
   moveq.l  #1,d0
   bra.s    restore

nomatch:
   moveq.l  #0,d0

restore:
   movem.l  (sp)+,a0-a3            ;restore regs
   rts

   END
  
