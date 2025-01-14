* == bif400.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     SUBSTR     ===========================
* Usage: SUBSTR(string,index,[length],[pad])
BIFsubstr
         move.l   (MAXARGS*4+4)(a5),d1 ; starting index
         subq.l   #1,d1                ; ... offset
         bcs      BFErr18              ; error

         cmp.l    d0,d1                ; less than length?
         ble.s    0$                   ; yes 
         move.l   d0,d1                ; no -- use length

0$:      cmpi.b   #3,d7                ; third argument provided?
         bge.s    1$                   ; yes 
         move.l   d0,d3                ; original length
         sub.l    d1,d3                ; new length
1$:      tst.l    d3                   ; length > 0?
         ble      BFNull               ; no -- NULL return
         cmp.l    MaxLen(pc),d3        ; too long?
         bgt      BFErr09              ; yes

         movea.l  a0,a2
         adda.l   d1,a2                ; start of substring
         adda.l   d3,a2                ; end of substring
         move.l   d0,d2
         sub.l    d1,d2                ; length (in original)

         ; Allocate the new string.

         move.w   d3,d1                ; length
         bsr      BIFString            ; D0=A0=string
         lea      ns_Buff(a0,d3.l),a3  ; end of string
         moveq    #0,d1
         subq.w   #1,d3                ; loop count

2$:      move.b   -(a2),d0             ; get character
         cmp.w    d2,d3                ; before end?
         bcs.s    3$                   ; yes ...
         move.b   d5,d0                ; no -- use pad character
3$:      add.b    d0,d1
         move.b   d0,-(a3)
         dbf      d3,2$                ; loop back

         move.b   d1,ns_Hash(a0)       ; store hash code
         rts

* ===========================     INSERT     ===========================
* Inserts a string into another string.
* Usage: INSERT(new,old,[index],[length],[pad])
BIFinsert
         moveq    #0,d4                ; clear flag
         bra.s    BIF400

* ===========================     OVERLAY     ==========================
* Overlays a string into another string.
* Usage: OVERLAY(new,old,[index],[length],[pad])
BIFoverlay
         moveq    #-1,d4               ; 'OVERLAY' flag
         moveq    #1,d6                ; default index

BIF400   cmpi.b   #5,d7                ; pad character?
         beq.s    1$                   ; yes
         moveq    #BLANK,d5            ; default pad

1$:      cmpi.b   #4,d7                ; length argument?
         blt.s    2$                   ; no ...
         tst.l    3*4(a5)              ; argument given?
         bne.s    3$                   ; yes ...
2$:      move.l   d0,d3                ; default length

3$:      cmp.l    MaxLen(pc),d3        ; too long?
         bgt.s    BIF401               ; yes ...

         ; Load the index argument ...

         exg      d2,d6                ; install default
         tst.l    d6                   ; index supplied?
         beq.s    4$                   ; ... use default
         move.l   (MAXARGS*4+8)(a5),d2 ; index argument

         ; Branch to compute total length of result

4$:      lea      -ns_Buff(a0),a2
         lea      -ns_Buff(a1),a3
         tst.w    d4                   ; 'OVERLAY'?
         bne.s    6$                   ; yes

         cmp.l    d1,d2                ; index <= length?
         ble.s    5$                   ; yes
         move.l   d2,d1                ; no -- replace length
5$:      add.l    d3,d1                ; total length
         bra.s    7$

6$:      subq.l   #1,d2                ; offset >= 0?
         bcs      BFErr18              ; no -- error
         move.l   d2,d0                ; starting offset
         add.l    d3,d0                ; overlaid length
         cmp.l    d0,d1                ; original length greater?
         bge.s    7$                   ; yes ...
         move.l   d0,d1                ; use new length

7$:      cmp.l    MaxLen(pc),d1        ; too long?
BIF401   bgt      BFErr09              ; "string too long"

         ; If string is owned, allocate a new one.  Otherwise, reuse it.

         movea.l  a3,a0                ; default string
         bsr      BIFString2           ; D0=A0=string

         lea      ns_Buff(a0),a1       ; destination buffer
         move.l   d7,-(sp)             ; push D7
         moveq    #0,d6                ; hash code
         moveq    #2,d7                ; outer loop count
         bra.s    4$                   ; jump on in

1$:      move.b   d5,d0                ; pad character
         cmp.w    ns_Length(a3),d1     ; past end?
         bcc.s    2$                   ; yes
         move.b   ns_Buff(a3,d1.l),d0  ; no -- load character
2$:      add.b    d0,d6                ; accumulate hash
         move.b   d0,0(a1,d1.l)        ; install it
3$:      dbf      d1,1$                ; loop back

         adda.l   d2,a1                ; update destination buffer
         exg      a2,a3                ; next string
         exg      d2,d3                ; next length
4$:      move.l   d2,d1                ; inner count
         dbf      d7,3$                ; back to inner loop
         move.l   (sp)+,d7             ; pop D7

         tst.w    d4                   ; 'OVERLAY'?
         beq.s    BIF402               ; no
         add.w    d3,d2                ; total length overlaid

BIF402   move.w   ns_Length(a3),d4     ; length of string
         sub.w    d2,d4                ; anything left?
         bls.s    2$                   ; no ...

         lea      ns_Buff(a3,d2.l),a2
         subq.w   #1,d4                ; loop count

1$:      add.b    (a2),d6              ; accumulate hash
         move.b   (a2)+,(a1)+          ; copy string
         dbf      d4,1$

2$:      move.b   d6,ns_Hash(a0)       ; install hash
         moveq    #0,d6                ; return code
         rts

* ==========================      UPPER      ===========================
* Translates strings to UPPERcase.
* Usage: UPPER(string)
BIFupper
         lea      -ns_Buff(a0),a1      ; argument
         movea.l  a4,a0                ; environment
         bsr      UpperString          ; D0=A1=string
         movea.l  a1,a0                ; return string
         rts

* ========================      TRANSLATE      =========================
* Usage: TRANSLATE(string,[otable],[itable],[pad])
BIFtranslat
         cmpi.b   #1,d7                ; any tables supplied?
         beq.s    BIFupper             ; no ... UPPERcase conversion

         ; Initialize the translate table from '00'x to 'FF'x.

         moveq    #256/4-1,d0          ; longword count
         move.l   #$00010203,d3        ; initial value
         move.l   #$04040404,d4        ; increment

1$:      move.l   d3,(a3)+             ; store code
         add.l    d4,d3                ; next code
         dbra     d0,1$                ; loop back
         lea      -256(a3),a3          ; restore pointer

         ; Scan the input and output tables to modify the translation table.

         moveq    #0,d3
         adda.l   d1,a1                ; end of output table
         adda.l   d2,a2                ; end of input table
         bra.s    4$                   ; jump in

2$:      move.b   -(a2),d3             ; input table character
         move.b   d5,d0                ; pad character
         cmp.w    d1,d2                ; past end of output?
         bcc.s    3$                   ; yes ...
         move.b   -(a1),d0             ; output table character
3$:      move.b   d0,0(a3,d3.w)        ; modify translate table
4$:      dbf      d2,2$                ; loop back

         ; Allocate a string for the result, if required ...

         movea.l  a0,a2                ; source buffer
         subq.w   #ns_Buff,a0          ; recover input string
         move.w   ns_Length(a0),d1     ; string length
         move.w   d1,d2                ; save length
         bsr      BIFString2           ; D0=A0=string
         lea      ns_Buff(a0),a1       ; destination buffer

         ; Begin the translation loop ...

         moveq    #0,d0                ; clear hash
         bra.s    6$                   ; jump in

5$:      move.b   (a2)+,d3             ; original character
         move.b   0(a3,d3.w),(a1)      ; translated character
         add.b    (a1)+,d0             ; increment hash code
6$:      dbf      d2,5$                ; loop back

         move.b   d0,ns_Hash(a0)       ; save hash code
         rts

* ============================     VERIFY     ==========================
* Usage: VERIFY(string,reference,['M'],[start])
BIFverify
         cmpi.b   #4,d7                ; 4th argument?
         bne.s    1$                   ; no
         subq.l   #1,d3                ; offset
         bcs      BFErr18              ; ... error

         cmp.l    d0,d3                ; too big?
         ble.s    1$                   ; no
         move.l   d0,d3                ; ... use length

1$:      exg      a0,a3                ; array=>A0, string=>A3
         move.l   d0,d4                ; save length
         clr.w    d0                   ; no default ...
         bsr.s    TestArray            ; A0=test array

         moveq    #0,d0                ; clear test
         move.w   d2,d5                ; 'Match' string provided?
         beq.s    2$                   ; no ...

         ; Test the 'match' argument ...

         move.b   (a2),d5              ; first character
         bclr     #LOWERBIT,d5         ; uppercase
         cmpi.b   #'M',d5              ; 'Match' mode?
         seq      d5                   ; ... set flag

2$:      sub.w    d3,d4                ; any characters to check?
         bls.s    4$                   ; no ...
         subq.w   #1,d4                ; loop count
         adda.l   d3,a3                ; first character

3$:      move.b   (a3)+,d0             ; test character
         addq.w   #1,d3                ; advance index
         move.b   0(a0,d0.w),d1        ; character flag
         eor.b    d5,d1                ; reverse sense?
         dbeq     d4,3$                ; loop back
         beq.s    5$                   ; return index

4$:      moveq    #0,d3                ; return 0

5$:      bra      BFNumber             ; numeric return

* ==========================     COMPRESS     ==========================
* Removes selected characters from a string.
* Usage: COMPRESS(string,remove)
BIFcompress
         movea.l  a0,a2                ; save string
         move.w   d0,d3                ; save length

         ; Build the test array ... D1/A1 = length/string.

         move.w   d5,d0                ; default character
         exg      a0,a3                ; array=>A0, string=>A3
         bsr.s    TestArray            ; A0=bit test array
         exg      a0,a3                ; string=>A0, array=>A3

         ; Count the number of characters to be kept ...

         move.w   d3,d0                ; length
         bra.s    2$                   ; jump in

1$:      move.b   (a0)+,d5             ; test character
         tst.b    0(a3,d5.w)           ; keep it?
         bne.s    2$                   ; no
         addq.w   #1,d2                ; keep count
2$:      dbf      d0,1$                ; loop back

         ; Check the remaining length ...

         move.w   d2,d1                ; anything left?
         beq      BFNull               ; no ...

         lea      -ns_Buff(a2),a0      ; original string
         cmp.w    d1,d3                ; original length?
         beq.s    5$                   ; ... return original string

         ; Allocate a new string and copy the selected characters.

         bsr      BIFString            ; D0=A0=string
         lea      ns_Buff(a0),a1       ; destination
         moveq    #0,d0                ; hash code
         subq.w   #1,d3                ; loop count

3$:      move.b   (a2)+,d5             ; test character
         tst.b    0(a3,d5.w)           ; keep it?
         bne.s    4$                   ; no ...
         add.b    d5,d0                ; update hash
         move.b   d5,(a1)+             ; install character
4$:      dbf      d3,3$                ; loop back
         move.b   d0,ns_Hash(a0)

5$:      rts

* =========================     TestArray     ==========================
* Builds a test array for a string of characters.  The array is cleared
* and the byte corresponding to each character code is set.
* Registers:   A0 -- output area (256 bytes)
*              A1 -- test string
*              D0 -- default test character
*              D1 -- length of string or 0
* Return:      A0 -- bit array
TestArray
         move.w   d1,-(sp)             ; push length

         ; Clear 256 bytes in the test array ...

         moveq    #256/8-1,d1          ; loop count

1$:      clr.l    (a0)+                ; clear slot
         clr.l    (a0)+                ; clear slot
         dbf      d1,1$                ; loop back
         lea      -256(a0),a0          ; restore array

         move.w   (sp)+,d1             ; test string?
         beq.s    3$                   ; no ... use default character
         subq.w   #1,d1                ; loop count

2$:      move.b   (a1)+,d0             ; load character
3$:      st       0(a0,d0.w)           ; set byte
         dbf      d1,2$                ; loop back

         rts

* ============================     TRIM     ============================
* Removes trailing blanks.
* Usage: TRIM(string)
BIFtrim
         moveq    #'T',d4              ; 'Trailing' option
         moveq    #BLANK,d5            ; trim blanks
         bra.s    BIF410

* ============================     STRIP     ===========================
* Removes leading and/or trailing characters in "remove" string.
* Usage: STRIP(string,[{B,L,T}],[remove])
BIFstrip
         moveq    #'B',d4              ; default option

BIF410   exg      a0,a3                ; buffer=>A0, string=>A3
         move.l   d0,d3                ; save length
         exg      a1,a2                ; remove=>A1, option=>A2
         exg      d1,d2                ; remove length=>D1, option length=>D2
         move.w   d5,d0                ; default pad
         bsr.s    TestArray            ; A0=test array

         move.w   d4,d0                ; default option
         cmp.b    #2,d7                ; option parameter?
         blt.s    1$                   ; no ...
         tst.w    d2                   ; option given?
         beq.s    1$                   ; no
         move.b   (a2),d0              ; yes -- load it
         bclr     #LOWERBIT,d0         ; uppercase

         ; Set up registers for the loop

1$:      moveq    #0,d2                ; beginning index
         movea.l  d3,a2                ; ending index
         moveq    #4$-4$,d4            ; branch offset
         moveq    #1,d5                ; initial increment

         ; Check for valid options:  'B', 'L', or 'T'

         cmpi.b   #'T',d0              ; 'Trailing'?
         beq.s    4$                   ; yes ... jump in
         moveq    #-1,d2               ; pre-decrement
         cmpi.b   #'B',d0              ; 'Both'?
         beq.s    2$                   ; yes 
         cmpi.b   #'L',d0              ; 'Leading'?
         bne      BFErr18              ; no ... invalid option
         moveq    #5$-4$,d4            ; branch offset

2$:      subq.w   #1,d3                ; loop count?
         bcs      BFNull               ; ... nothing left

3$:      add.l    d5,d2                ; next index
         move.b   0(a3,d2.l),d0        ; character to test
         tst.b    0(a0,d0.w)           ; delete it?
         dbeq     d3,3$                ; loop back

         addq.w   #1,d3                ; restore length
         beq      BFNull               ; ... nothing left
         jmp      4$(pc,d4.w)          ; leave here if 'Leading'
4$:      exg      d2,a2                ; enter here if 'Trailing'
         neg.l    d5                   ; reverse direction
         bmi.s    2$                   ; check again?

         ; Check whether to return the original string.

5$:      lea      -ns_Buff(a3),a0      ; recover string
         adda.l   d2,a3                ; starting location
         cmp.w    ns_Length(a0),d3     ; original length?
         bne      BFNewstr             ; no ... allocate a string
         rts
