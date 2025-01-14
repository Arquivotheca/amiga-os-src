* == bif300.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ============================     WORD     ============================
* Returns the nth word in a string.
* Usage: WORD(string,n)
BIFword
         moveq    #BIF302-BIF302,d4    ; branch offset
         bset     #NULLSTR,d7
         bra.s    BIF301

* ============================     WORDS     ===========================
* Returns the number of words in a string.
* Usage: WORDS(string)
BIFwords
         moveq    #-1,d3               ; large count
         moveq    #BIF304-BIF302,d4    ; branch offset
         bra.s    BIF300

* ==========================     WORDINDEX     =========================
* Returns the position of the nth word in a string.
* Usage: WORDINDEX(string,n)
BIFwordindx
         moveq    #BIF303-BIF302,d4    ; branch offset
         bra.s    BIF300

* ==========================     WORDLENGTH     ========================
* Returns the length of the nth word in a string.
* Usage: WORDLENGTH(string,n)
BIFwordleng
         moveq    #BIF305-BIF302,d4    ; branch offset

BIF300   bset     #NUMBER,d7           ; numeric return

BIF301:  movea.l  a0,a2
         move.l   d3,d5                ; selected word
         moveq    #0,d1
         moveq    #0,d3
         bra.s    2$                   ; jump in

1$:      addq.w   #1,d3                ; word count
2$:      cmp.l    d3,d5                ; the selected one?
         beq.s    3$                   ; yes ...
         bsr.s    SkipWord             ; D1=length A1=word D0=remain A0=scan
         bne.s    1$                   ; ... word found

3$:      exg      d1,d3                ; count=>D1, length=>D3
         tst.w    d3                   ; word found?
         jmp      BIF302(pc,d4.w)

         ; 'WORD' function: return the selected word

BIF302:  beq.s    BIF305               ; word not found
         bset     #NEWSTR,d7
         movea.l  a1,a3                ; selected word
         rts

         ; 'WORDINDEX' function ... return position of selected word.

BIF303:  beq.s    BIF305               ; not found ...
         suba.l   a2,a1                ; offset in string
         move.l   a1,d3
         addq.w   #1,d3                ; index
         rts

         ; 'WORDS' function ...  return number of words.

BIF304:  move.l   d1,d3                ; word count

BIF305:  rts

* ==========================     SkipWord     ==========================
* Advances to the character immediately following the next word and returns
* the length of the word.  A1 is changed only if a word is found.
* Registers:      A0 -- scan position
*                 D0 -- length
* Return:         D1 -- word length (CCR)
*                 A1 -- start of word
*                 D0 -- remaining length
*                 A0 -- updated scan
SkipWord
         moveq    #0,d1                ; clear length

         ; Scan the string looking for space characters.

1$:      subq.w   #1,d0                ; any left? 
         bcs.s    2$                   ; no

         addq.w   #1,d1                ; increment length
         cmpi.b   #' ',(a0)+           ; space character?
         bne.s    1$                   ; no ... loop back
         subq.w   #1,d1                ; word started?
         beq.s    1$                   ; no ... loop back
         subq.w   #1,a0                ; back up

         ; Update the word pointer if a word was found ...

2$:      addq.w   #1,d0                ; restore count
         tst.l    d1                   ; word found?
         beq.s    3$                   ; no ...
         movea.l  a0,a1                ; end of word
         suba.l   d1,a1                ; start of word

3$:      rts

* ==========================     SUBWORD     ==========================
* Usage: SUBWORD(string,index,[length])
BIFsubword
         subq.l   #1,d3                ; adjust length
         move.l   d0,d6                ; search to end
         moveq    #BIF307-BIF307,d4    ; jump offset
         bra.s    BIF306

* ===========================     DELWORD     ===========================
* Usage: DELWORD(string,index,[length])
BIFdelword
         move.l   d0,d6                ; string length
         neg.l    d6                   ; delete to end
         moveq    #BIF308-BIF307,d4    ; jump offset

BIF306   movea.l  a0,a2                ; save string
         move.l   (MAXARGS*4+4)(a5),d5 ; load index
         beq.s    0$                   ; error ... start <= 0
         cmp.l    MaxLen(pc),d5        ; too long?
         bgt.s    0$                   ; yes

         cmpi.b   #3,d7                ; three arguments?
         bne.s    1$                   ; no ...
         move.l   d3,d6                ; length in words (adjusted)
         add.l    d5,d6                ; last word for search
         bvc.s    1$

0$:      bra.s    BIF312               ; invalid operand

         ; Scan the string for the first and last search words

1$:      moveq    #0,d3                ; clear count
         movea.l  d3,a3                ; clear address
         bra.s    5$                   ; jump in

2$:      addq.w   #1,d3                ; word count
         cmp.w    d3,d5                ; first word found?
         bhi.s    5$                   ; no -- keep looking
         bne.s    3$                   ; no -- already found
         movea.l  a1,a3                ; save start of first word

         ; Look for the last word after the first one has been found.  If
         ; the last word precedes the first, set the word length to 0.

3$:      cmp.l    d5,d6                ; last word after first?
         bge.s    4$                   ; yes ...
         moveq    #0,d1                ; no -- set word length to 0
4$:      move.l   d1,d2                ; save word length
         cmp.l    d3,d6                ; last word?
         ble.s    6$                   ; yes -- search ends
5$:      bsr.s    SkipWord             ; D1=length A1=word
         bne.s    2$                   ; word found ... loop back

6$:      lea      -ns_Buff(a2),a0      ; original string
         move.w   ns_Length(a0),d5     ; original length
         moveq    #0,d6                ; restore return code
         move.l   a3,d3                ; first word found?
         jmp      BIF307(pc,d4.w)      ; branch to finish

         ; 'SUBWORD' function:  set pointer and length, then return

BIF307:  beq.s    BIF310               ; NULL return
         movea.l  a1,a2
         adda.l   d2,a2                ; end of last word
         move.l   a2,d3
         sub.l    a3,d3                ; new length > 0?
         beq.s    BIF310               ; no -- NULL return

         cmp.w    d3,d5                ; matches original length?
         bne      BFNewstr             ; no
         rts                           ; ... return origianl

BIF312:  bra      BFErr18              ; invalid operand

         ; 'DELWORD' function:  compute length of final string ...

BIF308   beq.s    BIF313               ; return original string
         tst.w    d1                   ; last word matched?
         bne.s    1$                   ; yes ...
         movea.l  a2,a1
         adda.l   d5,a1                ; end of string

1$:      move.l   a1,d4                ; start of last word
         sub.l    a2,d4                ; index of upper part
         sub.l    a2,d3                ; index of lower part

         ; Compute length and allocate a new string if > 0 ...

BIF309   move.l   d3,d1                ; lower length
         sub.l    d4,d1                ; difference?
         beq.s    BIF313               ; no -- return original string
         add.l    d5,d1                ; add original length
         beq.s    BIF310               ; zero? ... NULL return
         bsr      BIFString            ; D0=A0=string

         moveq    #0,d0                ; hash code
         moveq    #1,d1                ; outer loop count
         moveq    #0,d2                ; initial offset
         lea      ns_Buff(a0),a3       ; output buffer

1$:      movea.l  a2,a1
         adda.l   d2,a1                ; source buffer
         sub.l    d2,d3                ; length of string
         bra.s    3$                   ; jump on in ...

2$:      add.b    (a1),d0              ; accumulate hash
         move.b   (a1)+,(a3)+          ; copy string
3$:      dbf      d3,2$                ; inner loop
         move.l   d5,d3                ; second length
         move.l   d4,d2                ; second offset
         dbf      d1,1$                ; outer loop

         move.b   d0,ns_Hash(a0)
         rts

BIF310:  bra      BFNull               ; null return

BIF313:  rts

* ===========================     DELSTR     ===========================
* Usage: DELSTR(string,n,[length])
BIFdelstr:
         move.l   d0,d5                ; length > 0?
         beq.s    BIF310               ; no -- NULL return

         movea.l  a0,a2                ; save string
         subq.w   #ns_Buff,a0          ; string structure
         move.l   d3,d4                ; save length (?)
         subq.w   #ns_Buff,a1
         move.l   (a1),d3              ; starting index
         subq.l   #1,d3                ; offset ...
         bcs.s    BIF312               ; error

         cmp.l    d3,d5                ; less than length?
         bge.s    1$                   ; yes ...
         move.l   d5,d3                ; no -- use length

1$:      cmpi.b   #3,d7                ; third argument given?
         bne.s    2$                   ; no ...
         add.l    d3,d4                ; add start index to length
         bvs.s    BIF312               ; overflow??

         cmp.l    d4,d5                ; less than total?
         bge.s    BIF309               ; yes

2$:      move.l   d5,d4                ; default upper index
         bra.s    BIF309

* =========================     WordPos     ===========================
* Builds an array of position/length pairs, using the global work area.
* Registers:      A0 -- string
WordPos:
         movea.l  a0,a2                ; save start of string
         moveq    #0,d2
         moveq    #0,d3                ; array index
         bra.s    2$                   ; jump in ...

1$:      addq.w   #4,d3                ; advance frame
         cmpi.w   #TEMPBUFF,d3         ; too many words?
         bhi.s    3$                   ; yes ...

         sub.l    a2,a1                ; word index
         move.w   a1,-4(a3,d3.w)       ; store index
         move.w   d1,-2(a3,d3.w)       ; store length
         add.w    d1,d2                ; total length of words
2$:      bsr      SkipWord             ; D1=length A1=word D0=remain A0=scan
         bne.s    1$                   ; ... word found

3$:      lsr.w    #2,d3                ; final word count
         rts

* ============================     SPACE     ===========================
* Usage: SPACE(string,n,[pad])
BIFspace
         cmp.l    MaxLen(pc),d3        ; too large?
         bhi.s    5$                   ; yes ...

         ; Build the position/index array.

         move.l   d3,d4                ; spacing count
         bsr.s    WordPos              ; D3=word count D2=word length
         beq      BFNull               ; ... nothing to do
         cmpi.w   #TEMPBUFF>>2,d3      ; too many words?
         bhi      BFErr18              ; yes ... invalid operand
         subq.w   #1,d3                ; compute loop count

         ; Make sure the total length is <= MAXLEN.

         move.w   d3,d1                ; word count - 1
         mulu     d4,d1                ; total spacing count
         cmp.l    MaxLen(pc),d1        ; too large?
         bhi.s    5$                   ; yes ...
         add.w    d2,d1                ; add total word length
         bvs.s    5$                   ; overflow?

         bsr      BIFString            ; D0=A0=string
         addq.w   #ns_Buff,a0          ; output buffer
         bra.s    3$                   ; jump in

1$:      move.w   d4,d1
         subq.w   #1,d1                ; spacing count?
         bcs.s    3$                   ; no
2$:      add.b    d5,d6
         move.b   d5,(a0)+             ; install spacing characters
         dbf      d1,2$

3$:      move.l   (a3)+,d2             ; position/length
         move.w   d2,d1                ; get length
         subq.w   #1,d1                ; loop count (always >= 0)
         clr.w    d2
         swap     d2                   ; get position
         movea.l  a2,a1
         adda.l   d2,a1                ; start of word

4$:      add.b    (a1),d6              ; accumulate hash
         move.b   (a1)+,(a0)+          ; copy string
         dbf      d1,4$                ; loop back
         dbf      d3,1$                ; loop back

         movea.l  d0,a0                ; string to return
         move.b   d6,ns_Hash(a0)       ; store hash code
         moveq    #0,d6                ; return code
         rts

5$:      bra      BFErr09              ; string too long

* ============================     FIND     ============================
* Returns the word position of a phrase in a string.
* Usage: FIND(string,phrase)
BIFfind
         movem.l  d0/a0,(a3)           ; save length/string
         move.l   d1,d4                ; phrase length
         move.l   a1,d5                ; phrase string

         ; Match failed ... restart at beginning of phrase.

1$:      movem.l  (a3),d0/a0           ; restart position
         move.l   d4,d2                ; phrase length
         movea.l  d5,a2                ; phrase string
         addq.w   #1,d6                ; update word count

         ; Get the next word from the string ...

2$:      bsr      SkipWord             ; D1=length A1=word D0=remain A0=scan
         move.l   d1,d3                ; word found?
         beq.s    3$                   ; no

         movem.l  d0/a0,8(a3)          ; update running length/scan
         cmp.l    d2,d4                ; first word?
         bne.s    21$                  ; no
         movem.l  d0/a0,(a3)           ; ... update restart

         ; Get the next word from the phrase

21$:     movea.l  a2,a0                ; phrase position
         move.l   d2,d0                ; remaining length
         movea.l  a1,a2                ; save word
         bsr      SkipWord             ; D1=length A1=word D0=remain A0=scan
         beq.s    4$                   ; ... end of phrase

         exg      a0,a2                ; word=>A0, scan=>A2
         move.l   d0,d2                ; update length

         ; Check whether the phrase word matches the string word.

         cmp.w    d1,d3                ; lengths match?
         bne.s    1$                   ; no ... restart phrase
         move.l   d3,d0                ; length
         CALLSYS  StrcmpN              ; D0=flag
         bne.s    1$                   ; ... restart phrase
         movem.l  8(a3),d0/a0          ; running length/scan
         bra.s    2$                   ; check next word

         ; Search completed ... check whether any words matched.

3$:      cmp.l    d2,d4                ; words matched?
         beq.s    5$                   ; no

4$:      move.l   d6,d3                ; initial word

5$:      moveq    #0,d6                ; clear error
         bra      BFNumber
