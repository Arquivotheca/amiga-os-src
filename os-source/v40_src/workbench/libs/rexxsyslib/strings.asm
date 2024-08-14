* == strings.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     CmpString     ==========================
* Compares two NexxStr structures for equality.  The strings are compared
* only if the lengths and hash bytes match.
* Registers:      A0 -- first string (structure)
*                 A1 -- second string
* Return:         D0 -- 0 if unequal, -1 for exact match
CmpString
         move.w   ns_Length(a0),d1     ; length of first string
         cmp.w    ns_Length(a1),d1     ; lengths equal?
         bne.s    CSL001               ; no ...

         ; Compare the strings (including the hash code).

         addq.w   #ns_Hash,a0          ; advance to hash byte
         addq.w   #ns_Hash,a1

* Compares strings for equality.
* Registers:      A0 -- first string
*                 A1 -- second string
*                 D1 -- loop count
* Return:         D0 -- boolean
CmpStringLen
1$:      cmpm.b   (a0)+,(a1)+          ; strings match?
         dbne     d1,1$                ; loop back

CSL001   seq      d0                   ; set flag
         ext.w    d0                   ; extend
         ext.l    d0
         rts

* ==========================     StrcmpU     ===========================
* Compares two strings in UPPERcase for the specified number of characters.
* Registers:   A0 -- first string
*              A1 -- second string
*              D0 -- length
* Return:      D0 -- -1 if first less, 0 if exact match, 1 if first greater
StrcmpU
         subq.w   #1,d0                ; loop count
         bcs.s    SN002                ; ... nothing to do

         movem.l  d2/a2,-(sp)          ; delayed save
         movea.l  rl_CTABLE(a6),a2     ; attribute table
         clr.w    d1                   ; clear register
         clr.w    d2                   ; clear register

1$:      move.b   (a0)+,d2             ; load character
         tst.b    0(a2,d2.w)           ; lowercase?
         bpl.s    2$                   ; no
         bclr     #LOWERBIT,d2         ; clear bit

2$:      move.b   (a1)+,d1             ; load character
         tst.b    0(a2,d1.w)           ; lowercase?
         bpl.s    3$                   ; no
         bclr     #LOWERBIT,d1         ; clear bit

3$:      cmp.b    d2,d1                ; matched?
         dbne     d0,1$                ; loop back

         movem.l  (sp)+,d2/a2          ; (CCR preserved)
         bra.s    SN001

* ===========================     StrcmpN     ==========================
* Compares two strings for the specified number of characters.
* Registers:   A0 -- first string
*              A1 -- second string
*              D0 -- length
* Return:      D0 -- -1 if first less, 0 if exact match, 1 if first greater
StrcmpN
         subq.w   #1,d0                ; loop count
         bcs.s    SN002                ; ... nothing to do

1$:      cmpm.b   (a0)+,(a1)+          ; matched?
         dbne     d0,1$                ; loop back

SN001    slt      d0                   ; -1 or 0
         sgt      d1                   ; 0 or -1
         sub.b    d1,d0                ; -1, 0, or 1
         ext.w    d0                   ; extend word
         ext.l    d0                   ; extend long
         rts

SN002    moveq    #0,d0                ; exact match
         rts

* ==========================     ToUpper     ===========================
* Converts an extended ASCII character to uppercase.  Only D0 is changed.
* Registers:   D0 -- character to be converted
* Return:      D0 -- UPPERcase character
ToUpper
         move.l   a2,-(sp)
         movea.l  rl_CTABLE(a6),a2     ; attribute table

         andi.w   #$00FF,d0            ; clear byte
         tst.b    0(a2,d0.w)           ; lowercase?
         bpl.s    1$                   ; no
         bclr     #LOWERBIT,d0         ; ... clear bit

1$:      movea.l  (sp)+,a2
         rts

* ==========================     StrcpyA     ===========================
* Copies a string of characters, converting to ASCII by clearing the
* high bit (the high-order bit is used to flag quotes in strings).
* No null byte is appended.
* Registers:   A0 -- destination
*              A1 -- source
*              D0 -- byte count
* Return:      D0 -- sum-of-bytes hash code
StrcpyA
         moveq    #0,d1                ; clear hash
         bra.s    2$                   ; jump in

1$:      move.b   (a1)+,(a0)           ; copy character
         bclr     #ASCIIBIT,(a0)       ; clear bit
         add.b    (a0)+,d1             ; increment hash count
2$:      dbf      d0,1$                ; loop back

         move.l   d1,d0                ; hash code
         rts

* ==========================     StrcpyN     ===========================
* Copies a string of characters, which may include nulls.  No null byte
* is appended.
* Registers:   A0 -- destination
*              A1 -- source
*              D0 -- byte count
* Return:      D0 -- sum-of-bytes hash code
StrcpyN 
         moveq    #0,d1                ; clear hash
         bra.s    2$                   ; jump in

1$:      move.b   (a1)+,(a0)           ; copy character
         add.b    (a0)+,d1             ; increment hash
2$:      dbf      d0,1$                ; loop back

         move.l   d1,d0
         rts
   
* ==========================     StrcpyU     ===========================
* Copies a string of characters, converting them to UPPERCASE.  No null
* is appended.
* Registers:   A0 -- destination
*              A1 -- source
*              D0 -- byte count
* Return:      D0 -- sum-of-bytes hash code
StrcpyU
         move.l   a2,-(sp)
         move.w   d2,-(sp)
         movea.l  rl_CTABLE(a6),a2     ; attribute table
         moveq    #0,d1                ; clear hash
         clr.w    d2                   ; clear lower register
         bra.s    3$                   ; jump in

1$:      move.b   (a1)+,d2             ; load character
         tst.b    0(a2,d2.w)           ; lowercase?
         bpl.s    2$                   ; no
         bclr     #LOWERBIT,d2         ; ... clear bit

2$:      move.b   d2,(a0)+             ; install it
         add.b    d2,d1                ; increment hash
3$:      dbf      d0,1$                ; loop back

         move.l   d1,d0                ; hash code
         bra.s    ST001

* ===========================     StcToken     =========================
* Extracts the next token from a null-terminated string.  Quoted strings
* allow "white space" to be included in the token, and double-delimiters
* are recognized within the string.  The scan stops when a token is found
* or the null byte is reached.
* Registers:   A0 -- scan pointer
* Return:      A0 -- updated scan
*              A1 -- start of token
*              D0 -- quote character or 0
*              D1 -- length of token
StcToken
         move.l   a2,-(sp)
         move.w   d2,-(sp)
         movea.l  rl_CTABLE(a6),a2     ; attribute table
         clr.w    d2                   ; clear lower

         ; Initialize length and save start of token

0$:      moveq    #-1,d1               ; initial length
         movea.l  a0,a1                ; start pointer

         ; Clear saved quote character

1$:      moveq    #0,d0                ; clear quote

2$:      addq.l   #1,d1                ; increment length
         move.b   (a0),d2              ; next character
         beq.s    ST001                ; ... end of string
         addq.w   #1,a0                ; advance scan

         tst.b    d0                   ; quoted string?
         bne.s    3$                   ; yes

         ; Check for a "white space" character

         btst     #CTB_SPACE,0(a2,d2.w)
         bne.s    4$

         ; Not "white space" ... check if token started yet.

         tst.w    d1                   ; token started?
         bne.s    2$                   ; yes

         ; Start of token ... see if it begins with a quote.

         move.b   d2,d0                ; save character
         subi.b   #DQUOTE,d2           ; a double quote?
         beq.s    2$                   ; yes
         subq.b   #QUOTE-DQUOTE,d2     ; a single quote?
         beq.s    2$                   ; yes
         bra.s    1$                   ; ... clear quote

         ; Inside a quoted string ... check for delimiter.

3$:      cmp.b    d0,d2                ; matches delimiter?
         bne.s    2$                   ; no
         cmp.b    (a0),d0              ; double-delimiter?
         bne.s    ST001                ; no ... all done

         ; A double-delimiter ... skip to next character

         addq.l   #1,d1                ; increment length
         addq.w   #1,a0                ; advance scan
         bra.s    2$                   ; loop back

         ; A "white space" character ... don't count it

4$:      tst.w    d1                   ; token started?
         beq.s    0$                   ; no ... reset

ST001    move.w   (sp)+,d2
         movea.l  (sp)+,a2
         rts

* ===========================     Strcopy     ==========================
* Copies a quoted string to a buffer, removing double-delimiter sequences.
* Registers:   A0 -- buffer area
*              A1 -- string
*              D0 -- quote character
*              D1 -- initial length
* Return:      D1 -- final length
*              A1 -- buffer
Strcopy
         move.l   a0,-(sp)             ; push buffer
         move.w   d3,-(sp)
         bra.s    2$                   ; jump in

1$:      move.b   (a1)+,(a0)           ; copy character
         cmp.b    (a0)+,d0             ; a quote?
         bne.s    2$                   ; no

         not.w    d3                   ; previous quote?
         bne.s    3$                   ; no
         subq.w   #1,a0                ; back up

2$:      clr.w    d3                   ; reset flag
3$:      dbf      d1,1$                ; loop back

         move.w   (sp)+,d3
         movea.l  (sp)+,a1             ; pop buffer
         move.l   a0,d1                ; end of string
         sub.l    a1,d1                ; final length
         rts

* =========================     StrflipN     ===========================
* Reverses ('flips') a string of characters.
* Registers:   A0 -- string to be flipped
*              D0 -- byte count
StrflipN
         moveq    #0,d1                ; clear length
         move.w   d0,d1                ; length of string
         movea.l  a0,a1                ; start of string
         adda.l   d1,a1                ; end of string
         bra.s    2$                   ; jump in

1$:      move.b   -(a1),d0             ; character from end
         move.b   (a0),(a1)            ; copy to end
         move.b   d0,(a0)+             ; copy to beginning

2$:      subq.l   #2,d1                ; all done?
         bcc.s    1$                   ; no ... loop back
         rts

* ===========================     Strlen     ===========================
* Returns the length of a null-terminated string.  A0/A1 are preserved.
* Registers:   A0 -- string
* Return:      D0 -- length
*              D1 -- same
Strlen
         move.l   a0,d0                ; save start

1$:      tst.b    (a0)+                ; null byte?
         bne.s    1$                   ; loop back
         subq.w   #1,a0                ; back up

         exg      d0,a0                ; end=>D0, start=>A0
         sub.l    a0,d0                ; ... string length
         move.l   d0,d1                ; set CCR
         rts

* =========================     TrimLeading     ========================
* Advances a pointer past the leading blanks of a string structure.
* Registers A1 and D1 are preserved.
* Registers:   A0 -- string structure
* Return:      A0 -- pointer to first non-blank character
*              D0 -- new length
TrimLeading
         moveq    #0,d0
         move.w   ns_Length(a0),d0     ; initial length
         addq.w   #ns_Buff,a0          ; start of string

1$:      cmpi.b   #BLANK,(a0)+         ; blank?
         dbne     d0,1$                ; loop back

         subq.w   #1,a0                ; back up pointer
         rts
