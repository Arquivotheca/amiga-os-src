* == extfunction.asm ===================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     BuildArgPkt     ========================
* Loads a REXX message packet with the arguments found in an arglist clause.
* The first token always carries the function name, and is not included in
* the argument count.
* Registers:   A0 -- message packet
*              A1 -- clause
* Return:      D0 -- argument count or -1
BuildArgPkt
         move.l   d2,-(sp)
         move.l   cTL_Head(a1),d2      ; first token
         lea      ARG0(a0),a1          ; start of argblock
         moveq    #-1,d0               ; initial argument count

1$:      movea.l  d2,a0                ; token to check
         move.l   (a0),d2              ; next token?
         beq.s    3$                   ; no ...

         cmpi.w   #MAXRMARG,d0         ; argblock full yet?
         bge.s    4$                   ; yes -- error
         move.l   TSTRING(a0),d1       ; an argument string?
         beq.s    2$                   ; no ...
         addq.l   #ns_Buff,d1          ; add the buffer offset

2$:      move.l   d1,(a1)+             ; store the string
         addq.l   #1,d0                ; increment count
         bra.s    1$                   ; loop back

         ; Check for special case: single argument

3$:      subq.l   #1,d0                ; single argument?
         bne.s    5$                   ; no ...
         tst.l    d1                   ; an argument string?
         bne.s    5$                   ; yes ...
         moveq    #-1,d0               ; no -- null argument list
         bra.s    5$

4$:      moveq    #-2,d0               ; too many arguments

5$:      move.l   (sp)+,d2
         addq.l   #1,d0                ; final count
         rts

* =========================     ExtFunction     ========================
* Searches the Library List for an external function, either as part of
* a function library or as a function host.  If found, the function is
* called and returns with an error code in D0 and the result string in A0.
* A result string is expected only if no errors occurred (D0 cleared);
* A0 should be cleared if no result is available.
* [instruction module environment]
* Return:      D6 -- error code
*              A0 -- result string
* Scratch:     D2-D5/A2/A3
ExtFunction
         moveq    #0,d3                ; clear index
         move.l   rl_SysBase(a6),d4    ; EXEC base

         ; Prepare the message packet from the argument list ...

         movea.l  gn_MsgPkt(a3),a2     ; global message packet
         movea.l  a2,a0
         movea.l  a5,a1                ; function clause
         bsr.s    BuildArgPkt          ; D0=argument count or -1
         bmi      EFErr17              ; error

         ; Fill in the required fields ...

         ori.l    #RXFUNC!RXFF_RESULT,d0
         move.l   d0,ACTION(a2)        ; install action

         ; Search the Libraries List (maintained by the REXX server) for
         ; the selected function library or function host.

1$:      bsr      Forbid               ; (registers preserved)
         move.w   d3,d0                ; skip count
         addq.w   #1,d3                ; bump counter
         lea      rl_LibList(a6),a5    ; list header

2$:      movea.l  (a5),a5              ; current node
         move.l   (a5),d5              ; next node?
         dbeq     d0,2$                ; loop back
         beq.s    3$

         ; Load data from the node before reenabling.

         movea.l  RRNAME(a5),a1        ; library name string
         move.l   LLVERS(a5),d0        ; library version
         move.b   RRTYPE(a5),d1        ; node type
         move.l   LLOFFSET(a5),d2      ; offset to "Query" entry

3$:      bsr      Permit               ; (registers preserved)
         tst.l    d5                   ; node found?
         beq.s    EFErr15              ; no

         ; Check whether the node is a function library or a function host.

         subq.b   #RRT_LIB,d1          ; a library?
         bne.s    4$                   ; no

         ; Open the library.  A failure here will terminate the search.

         exg      d4,a6                ; REXX=>D4 , EXEC=>A6
         CALLSYS  OpenLibrary
         exg      d4,a6                ; EXEC=>D4 , REXX=>A6
         tst.l    d0                   ; opened?
         beq.s    EFErr14              ; no -- library not found

         ; Call the function package.  If the requested function is not
         ; found, register D0 is set to "not found" and the search continues.
         ; If the function name is found, the call is made and the result
         ; is returned (as an argstring) in A0.

         movea.l  a2,a0                ; message packet
         exg      d0,d2                ; offset=>D0 , library=>D2
         exg      d2,a6
         jsr      0(a6,d0.l)           ; D0=error A0=result
         exg      d2,a6
         move.l   d0,d6                ; save error code
         move.l   a0,-(sp)             ; push result

         ; Close the function library between calls.

         movea.l  d2,a1                ; base address
         exg      d4,a6                ; REXX=>D4 , EXEC=>A6
         CALLSYS  CloseLibrary
         exg      d4,a6                ; EXEC=>D4 , REXX=>A6

         ; Check whether the function was found.  If not, keep searching.

         movea.l  (sp)+,a0             ; pop result
         move.l   d6,d2                ; "backup" error
         beq.s    6$                   ; all ok
         bra.s    5$

         ; A "function host" found ... send the invocation message packet.
         ; If the return code indicates "program not found" (ERR10_001),
         ; the search continues.  Otherwise, all errors are reported as
         ; "function returned error" (ERR10_012).

4$:      movea.l  a3,a0                ; global data pointer
         move.l   a2,d0                ; packet
         bsr      SendRexxMsg          ; D0=error
         move.l   d0,d6                ; packet sent ok?
         bne.s    EFOut                ; no ...

         movea.l  RESULT2(a2),a0       ; result string (maybe)
         tst.l    RESULT1(a2)          ; execution error?
         beq.s    6$                   ; no
         moveq    #ERR10_012,d2        ; backup error
         move.l   a0,d6                ; ... error code

         ; Determine the correct error code to return ...

5$:      moveq    #ERR10_001,d0        ; "program not found"
         cmp.l    d0,d6                ; not found?
         beq      1$                   ; yes ... keep looking
         move.l   d2,d6                ; install backup error
         bra.s    EFOut

         ; Copy the return string into REXX internal memory and release it.

6$:      move.l   a0,d0                ; a result?
         beq.s    EFOut                ; no

         movea.l  a0,a1                ; result string
         movea.l  a4,a0                ; environment
         bsr      CVarg2s              ; D0=A0=string
         bra.s    EFOut

         ; Error codes

EFErr14  moveq    #ERR10_014,d6        ; required library not found
         bra.s    EFOut

EFErr15  moveq    #ERR10_015,d6        ; function not found
         bra.s    EFOut

EFErr17  moveq    #ERR10_017,d6        ; wrong number of arguments

EFOut    move.l   d6,d0                ; set CCR
         rts
