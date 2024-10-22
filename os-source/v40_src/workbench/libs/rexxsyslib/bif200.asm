* == bif200.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     INDEX     =============================
* Usage: INDEX(haystack,needle,[n])
BIFindex
         exg      d0,d1                ; interchange arguments
         exg      a0,a1

* ============================     POS     =============================
* Usage: POS(needle,haystack,[n])
BIFpos
         moveq    #1,d2                ; start at beginning
         moveq    #1,d5                ; search forward
         bra.s    BIF201

* ==========================     LASTPOS     ===========================
* Usage: LASTPOS(needle,haystack,[n])
BIFlastpos
         move.l   d1,d2                ; start at end
         moveq    #-1,d5               ; search backward
         cmp.l    d1,d3                ; start <= length?
         ble.s    BIF201               ; yes
         move.l   d1,d3                ; ... use length

BIF201   cmpi.b   #3,d7                ; three args?
         bne.s    1$                   ; no ...
         move.l   d3,d2                ; starting index
         beq      BFErr18              ; error ...

1$:      moveq    #0,d3                ; default return
         subq.w   #1,d0                ; loop count
         bcs.s    5$                   ; ... nothing to do
         bra.s    3$                   ; jump in

         ; Increment the running index ...

2$:      add.w    d5,d2                ; next index
         beq.s    5$                   ; ... all done

3$:      move.w   d1,d4                ; 'haystack' length
         sub.w    d2,d4                ; remaining count
         bcs.s    5$                   ; all done
         cmp.w    d0,d4                ; enough to match?
         bcs.s    2$                   ; no ... loop back

         move.w   d0,d4                ; loop count
         lea      -1(a1,d2.l),a2       ; 'haystack' string
         movea.l  a0,a3                ; 'needle' string
4$:      cmpm.b   (a2)+,(a3)+          ; matched?
         dbne     d4,4$                ; loop back
         bne.s    2$                   ; not matched -- loop back
         move.w   d2,d3                ; return index of match

5$:      bra      BFNumber

* ==========================     BITCOMP     ===========================
* Usage: BITCOMP(string1,string2)
BIFbitcomp
         cmp.w    d0,d1                ; 1st length > 2nd length?
         bls.s    1$                   ; yes
         exg      d0,d1                ; no -- interchange arguments
         exg      a0,a1

1$:      adda.l   d0,a0                ; end of 1st buffer
         adda.l   d1,a1                ; end of 2nd buffer
         moveq    #-1,d3               ; default return
         subq.w   #1,d0                ; loop count
         bcs.s    5$                   ; nothing to do

         moveq    #-1,d2               ; scan index
2$:      addq.l   #1,d2
         move.b   d5,d4                ; pad character
         cmp.w    d1,d2                ; past end?
         bcc.s    3$                   ; yes -- use pad
         move.b   -(a1),d4             ; no  -- load character
3$:      cmp.b    -(a0),d4             ; matched?
         dbne     d0,2$                ; loop back
         beq.s    5$                   ; fully matched ...

         ; Determine the bit position where the mismatch occurred ...

         move.l   d2,d3                ; byte index
         lsl.l    #3,d3                ; initial bit position
         move.b   (a0),d0              ; last byte tested
         eor.b    d4,d0                ; non-matching byte
         subq.l   #1,d3
4$:      addq.l   #1,d3
         lsr.b    #1,d0                ; bit set? (hope it finds one ...)
         bcc.s    4$                   ; no ...

5$:      bra      BFNumber             ; numeric return

* ===========================     ABBREV     ===========================
* Usage: ABBREV(string,str,[minlength])
BIFabbrev
         cmpi.b   #3,d7                ; third argument?
         bne.s    1$                   ; no ...
         cmp.l    d1,d3                ; long enough?
         bgt.s    3$                   ; no

1$:      moveq    #-1,d3               ; assume TRUE return
         subq.w   #1,d1                ; loop count
         bcs.s    4$                   ; nothing to do 

2$:      cmpm.b   (a0)+,(a1)+          ; strings match?
         dbne     d1,2$                ; loop back
         beq.s    4$                   ; ... matched

3$:      moveq    #0,d3                ; FALSE return

4$:      bra      BFBool

* ==========================     COMPARE     ===========================
* Usage: compare(string1,string2)
BIFcompare
         cmp.w    d0,d1                ; 2nd length <= 1st length?
         bls.s    1$                   ; yes ...
         exg      d0,d1                ; no -- interchange operands
         exg      a0,a1

1$:      subq.w   #1,d0                ; loop count
         bcs.s    4$                   ; nothing to do ...

2$:      move.b   d5,d4                ; load pad character
         addq.w   #1,d3                ; bump index
         cmp.w    d1,d3                ; greater than length?
         bhi.s    3$                   ; yes
         move.b   (a1)+,d4             ; load test character
3$:      cmp.b    (a0)+,d4             ; matched?
         dbne     d0,2$                ; loop back
         bne.s    4$
         moveq    #0,d3                ; strings match ...

4$:      bra      BFNumber

* ==========================     REVERSE     ==========================
* Reverses the order ('flips') the characters of a string.
* Usage: reverse(string)
BIFreverse
         lea      -ns_Buff(a0),a1      ; recover string
         move.w   d0,d2                ; save length

         move.b   ns_Flags(a1),d0      ; attribute flags
         andi.b   #NSF_OWNED,d0        ; owned by anyone?
         beq.s    1$                   ; no -- reverse in place

         ; String owned ... create a copy before reversing.

         movea.l  a4,a0                ; environment
         bsr      CopyString           ; D0=A1=string

         ; Install new attributes and revers characters.

1$:      move.b   #NSF_STRING,ns_Flags(a1)
         lea      ns_Buff(a1),a0       ; buffer area
         move.l   d2,d0                ; length
         movea.l  a1,a2                ; save string
         CALLSYS  StrflipN             ; reverse characters
         movea.l  a2,a0                ; restore string
         rts

* ==========================     CENTER     ============================
* Centers a string in the specified width field.  Padding characters are
* added as needed.
* Usage: CENTER(string,width,[pad])
BIFcenter
         movea.l  a0,a2                ; save source buffer
         move.w   d0,d2                ; and length

         ; Allocate the output string ...

         cmp.l    MaxLen(pc),d3        ; too long?
         bgt      BFErr18              ; yes ...
         move.l   d3,d1                ; new length
         bsr      BIFString            ; D0=A0=string
         moveq    #0,d1
         move.w   d3,d0                ; new length
         sub.w    d2,d0                ; difference >= 0?
         bcs.s    1$                   ; no ...

         lsr.w    #1,d0                ; centering offset
         add.w    d0,d2                ; advance length
         sub.w    d0,d1                ; source offset
         bra.s    2$

         ; New string shorter than old.  Compute source offset and set
         ; centering offset to 0.

1$:      sub.w    d0,d1
         lsr.w    #1,d1                ; source offset
         moveq    #0,d0                ; clear the centering offset

2$:      movea.l  a2,a1                ; source buffer
         adda.w   d1,a1                ; offset source buffer
         moveq    #0,d1                ; clear hash
         bra.s    5$                   ; jump in

         ; Copy the string to the output buffer ...

3$:      move.b   d5,d4                ; pad character
         cmp.w    d0,d3                ; index after center offset?
         bcs.s    4$                   ; no ...
         cmp.w    d2,d3                ; index less than offset length?
         bcc.s    4$                   ; no ...
         move.b   0(a1,d3.l),d4        ; load a character
4$:      move.b   d4,ns_Buff(a0,d3.l)  ; store the character
         add.b    d4,d1                ; accumulate the hash code
5$:      dbf      d3,3$                ; loop back

         move.b   d1,ns_Hash(a0)       ; install hash byte
         rts

* =============================     LEFT     ===========================
* Usage: LEFT(string,length,[pad])
BIFleft
         move.l   d0,d2                ; old string length
         move.l   d3,d4                ; new string length
         cmp.l    d0,d3                ; new string longer?
         bge.s    BIF202               ; yes ...
         move.l   d3,d0                ; no -- get shorter length
         bra.s    BIF202

* ============================     RIGHT     ===========================
* Usage: RIGHT(string,length,[pad])
BIFright
         moveq    #0,d2
         move.l   d3,d4                ; new length
         sub.l    d0,d4                ; excess characters

BIF202   cmp.l    MaxLen(pc),d3        ; too long?
         bgt      BFErr18              ; yes

         movea.l  a0,a2                ; old string
         adda.l   d0,a2                ; end of string
         move.w   d3,d1                ; new length
         bsr      BIFString            ; D0=A0=string
         moveq    #0,d1                ; clear hash
         bra.s    4$                   ; jump in

1$:      move.b   d5,d0                ; pad character
         cmp.l    d2,d3                ; LEFT test:  within string?
         blt.s    2$                   ; yes ...
         cmp.l    d4,d3                ; RIGHT test: within string?
         blt.s    3$                   ; no
2$:      move.b   -(a2),d0             ; load character
3$:      move.b   d0,ns_Buff(a0,d3.l)  ; store character
         add.b    d0,d1                ; accumulate hash
4$:      dbf      d3,1$                ; loop back

         move.b   d1,ns_Hash(a0)       ; store the hash code
         rts

* ==========================      COPIES     ===========================
* Usage: COPIES(string,number)
BIFcopies
         movea.l  a0,a2                ; save string
         not.w    d2                   ; maximum multiplier
         cmp.l    d2,d3                ; multiplier too big?
         bgt      BFErr18              ; yes ... error

         ; Compute the total length for the result ...

         move.w   d0,d2                ; save length
         mulu     d3,d0                ; new length
         beq      BFNull               ; ... null return
         cmp.l    MaxLen(pc),d0        ; result too long?
         bgt      BFErr09              ; yes ... error

         move.w   d0,d1                ; total length
         bsr      BIFString            ; D0=A0=string
         lea      ns_Buff(a0),a3       ; output buffer
         moveq    #0,d1                ; clear hash
         bra.s    4$                   ; jump in

1$:      movea.l  a2,a1                ; source string
         move.w   d2,d0                ; inner count
         bra.s    3$                   ; jump in

2$:      add.b    (a1),d1              ; accumulate hash
         move.b   (a1)+,(a3)+          ; copy string
3$:      dbf      d0,2$                ; loop back
4$:      dbf      d3,1$                ; loop back

         move.b   d1,ns_Hash(a0)       ; install hash
         rts

* ==========================     XRANGE     ============================
* Generates a sequence of characters between the beginning and ending
* values specified in the arguments.  The default is $00 through $FF.
* Usage: xrange([start],[end])
BIFxrange
         moveq    #-1,d3               ; default end
         tst.b    d7                   ; any args?
         beq.s    2$                   ; no ...

         tst.w    d1                   ; end character?
         beq.s    1$                   ; no ...
         move.b   (a1),d3              ; load end character

1$:      tst.w    d0                   ; start character?
         beq.s    2$                   ; no ...
         move.b   (a0),d2              ; first argument

2$:      movea.l  a3,a1                ; output buffer
         bra.s    4$                   ; jump in ...

3$:      addq.b   #1,d2                ; increment character
4$:      move.b   d2,(a1)+             ; install it
         cmp.b    d2,d3                ; done yet?
         bne.s    3$                   ; no -- loop back

         move.l   a1,d3                ; end of string
         sub.l    a3,d3                ; final length (at least one)
         bra      BFNewstr
