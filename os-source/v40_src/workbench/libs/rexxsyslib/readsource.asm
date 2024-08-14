* == readsource.asm ====================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     ReadSource     =========================
* Source scanner for the REXX interpreter.  Reads source file and performs
* preliminary processing:
*   (1) multiple blanks replaced by a single blank.
*   (2) blanks suppressed following special characters.
*   (3) comments removed.
*   (4) 'newline' following ',' recognized as continuation.
*   (5) implicit ends ('newline') replaced by ';'
* The IoBuff is closed at exit.

* A Segment structure is always allocated for the Segment Array, but arrays
* for label symbols and line numbers will be created only if requested.
* Registers:   A0 -- storage environment
*              A1 -- IoBuff pointer
*              D0 -- boolean flag: create Lines/Labels arrays?
* Returns:     D0 -- error code or 0
*              A0 -- source array pointer
*              A1 -- Lines Array
*              D1 -- Labels Array

* Character status flag bits (D6 lower word)
STB_BLANK   SET   0                    ; a blank?
STB_SPEC    SET   1                    ; a special character?
STB_COMMA   SET   2                    ; a comma?
STB_SEMI    SET   3                    ; a semi-colon?
STB_STR     SET   4                    ; a string delimiter?
STB_SLASH   SET   5                    ; a '/' character?
STB_STAR    SET   6                    ; a '*' character?
STB_END     SET   15                   ; end of clause (sign bit)

STF_BLANK   SET   1<<STB_BLANK
STF_SPEC    SET   1<<STB_SPEC
STF_COMMA   SET   1<<STB_COMMA
STF_SEMI    SET   1<<STB_SEMI
STF_STR     SET   1<<STB_STR
STF_SLASH   SET   1<<STB_SLASH
STF_STAR    SET   1<<STB_STAR
STF_END     SET   1<<STB_END

* Scan control flag bits (D7 upper word)
RSB_LINE    SET   16                   ; new input line
RSB_DONE    SET   31                   ; all done (sign bit)

         ; The MultiSrcNode is used to track source lines and labels.

MSNCOUNT EQU      10
         STRUCTURE MultiSrcNode,0
         APTR     msn_Next             ; next block
         LONG     msn_Count            ; entries
         STRUCT   msn_Data,4*MSNCOUNT  ; data array
         LABEL    msn_SIZEOF           ; size: 48 bytes

         STRUCTURE StackFrame,0
         LONG     MakeLines            ; lines/labels flag
         APTR     Labels               ; Labels nodes
         APTR     Source               ; Source nodes
         APTR     Lines                ; Lines nodes

         APTR     PPLAB                ; Labels Array pointer
         APTR     PPSEG                ; Source Array pointer
         APTR     PPLINE               ; Lines Array pointer

         LONG     NLAB                 ; label count
         LONG     NSEG                 ; segment count
         LONG     NLINES               ; line count
         LONG     NPOS                 ; input position
         APTR     IOBUFF               ; IoBuff pointer
         LABEL    sf_SIZEOF

ReadSource
         movem.l  d2-d7/a2-a5,-(sp)
         lea      -sf_SIZEOF(sp),sp    ; stack buffer

         movea.l  rl_CTABLE(a6),a2     ; attribute table
         movea.l  a0,a3                ; (base) environment
         movea.l  gn_TBuff(a3),a4      ; global work area

         moveq    #0,d3                ; test character
         moveq    #0,d4                ; output counter
         moveq    #0,d5                ; up: input count  low: string byte
         moveq    #0,d6                ; up: saved status low: status
         moveq    #1<<(RSB_LINE-16),d7 ; up: flags        low: comment level
         swap     d7

         movem.l  d0/d3/d4/d5,(sp)     ; save flag/clear pointers
         movem.l  d3-d6/a1,NLAB(sp)    ; clear counters/save IoBuff

         ; Check whether to increment the lines array ...

RSScan   bclr     #RSB_LINE,d7         ; a new input line?
         beq.s    1$                   ; no ...
         tst.l    (sp)                 ; keep lines array?
         beq.s    1$                   ; no ...

         ; Save the input line information.

         lea      Lines(sp),a1         ; Lines Array list header
         move.l   NPOS(sp),d2          ; input position
         bsr      AddSrcNode           ; allocate and link
         addq.l   #1,NLINES(sp)        ; increment line counter

         ; Check whether to update the current source line number.

         move.w   d7,d0                ; inside a comment?
         or.w     d5,d0                ; inside a string?
         bne.s    1$                   ; yes
         move.l   NLINES(sp),ev_SrcLine(a3)

         ; Check whether we're at end-of-file.

1$:      tst.l    d7                   ; end of file?
         bmi      RSDone               ; yes

         ; Check for a buffered character.

         swap     d5                   ; input count=>lower
         subq.w   #1,d5                ; any characters?
         bcc.s    2$                   ; yes

         ; No characters ... read from the IoBuff stream.

         movea.l  IOBUFF(sp),a5        ; load IoBuff
         movea.l  a5,a0                ; IoBuff pointer
         CALLSYS  ReadF                ; D0=character or -1
         move.w   (iobRct+2)(a5),d5    ; load count
         clr.w    (iobRct+2)(a5)       ; clear count
         movea.l  iobRpt(a5),a5        ; load pointer
         tst.l    d0                   ; a character?
         bpl.s    3$                   ; yes

         ; End-of-file ... check for prior semicolon.

         cmpi.b   #SPC_SEMI,d3         ; prior semicolon?
         beq      RSDone               ; yes ... all done

         ; Set 'done' and generate a terminating 'newline' character.

         bset     #RSB_DONE,d7         ; set 'done' flag
         moveq    #NEWLINE,d0          ; recode as a 'newline'
         bra.s    RSTest

         ; Load the current character.

2$:      move.b   (a5)+,d0             ; load character

3$:      addq.l   #1,NPOS(sp)          ; increment source position

         ; Proceed with character tests ...

RSTest   swap     d5                   ; string byte=>lower
         move.b   d0,d3                ; test character
         moveq    #0,d0                ; clear status
         moveq    #NEWLINE,d1          ; load 'newline'

         ; Check whether we're inside a comment ...

         tst.w    d7                   ; inside comment?
         beq.s    RSString             ; no

         cmpi.b   #'*',d3              ; an asterisk?
         beq.s    1$                   ; yes
         cmp.b    d1,d3                ; a newline?
         beq.s    RSSetNL              ; yes
         cmpi.b   #'/',d3              ; a slash?
         bne.s    RSSkip1              ; no ... skip character

         ; A '/' ... check for prior '*'

         moveq    #STF_SLASH,d0        ; load status
         btst     #STB_STAR,d6         ; prior asterisk?
         beq.s    RSSkip1              ; no

         ; A '*/' combination ... decrement nesting level.

         subq.w   #1,d7                ; decrement level
         bne.s    2$                   ; ... still nested

         ; Leaving comment mode ... restore status.

         swap     d6                   ; restore status
         move.w   d6,d0                ; new status
         bra.s    RSSkip1

         ; A '*' ... check for prior '/'

1$:      moveq    #STF_STAR,d0         ; load status
         btst     #STB_SLASH,d6        ; prior slash?
         beq.s    RSSkip1              ; no

         ; A '/*' combination ... increment nesting level.

         addq.w   #1,d7                ; increment nesting

2$:      moveq    #0,d0                ; clear status
         bra.s    RSSkip1              ; skip character

         ; Now check for 'string mode'

RSString tst.b    d5                   ; inside a string?
         beq.s    RSBlank              ; no ... check further

         cmp.b    d3,d5                ; matches delimiter?
         bne.s    1$                   ; no ...

         btst     #STB_STR,d6          ; double delimiter?
         bne.s    RSSave1              ; yes (clear status)
         moveq    #STF_STR,d0          ; possible termination
         bra.s    RSSave1              ; save it

1$:      btst     #STB_STR,d6          ; string termination?
         bne.s    2$                   ; yes ...
         cmp.b    d1,d3                ; a 'newline'?
         beq.s    RSSetNL              ; yes
         bra.s    RSSave1

2$:      clr.b    d5                   ; leave 'string mode'

         ; Check for "white space" ... multiple blanks are ignored.

RSBlank  btst     #CTB_SPACE,0(a2,d3.w); white space?
         beq.s    RSCheck              ; no
         cmp.b    d1,d3                ; a newline?
         beq      RSNewln              ; yes

         ; A "white space" character ... remap to blank.

         moveq    #BLANK,d3            ; remap to blank
         tst.w    d4                   ; any characters?
         beq.s    RSSkip1              ; no
         move.w   d6,d0                ; current status
         btst     #STB_SPEC,d6         ; previous special character?
         bne.s    RSSkip1              ; yes ...
         moveq    #STF_BLANK,d0        ; new status
         btst     #STB_BLANK,d6        ; previous blank?
         bne.s    RSSkip1              ; yes

RSSave1  bra      RSSave               ; (local jump)

         ; Skip 'newline' characters inside comments or strings

RSSetNL  bset     #RSB_LINE,d7         ; set flag

RSSkip1  bra      RSSkip               ; (local jump)

         ; Character-specific screening

RSCheck  moveq    #(RSNUM-1),d1        ; loop count
         lea      RSTable(pc),a1       ; start of table

1$:      cmp.b    (a1)+,d3             ; special character?
         dbcc     d1,1$                ; loop back
         bne.s    RSSave1              ; no further checking ...

         move.b   (RSNUM-1)(a1),d1     ; load offset
         adda.w   d1,a1                ; branch address
         jmp      (a1)                 ; branch to process

         ; Table of special characters ... must be in descending order.

RSTable  dc.b     SPC_SEMI             ; hex 3B (search starts here)
         dc.b     SPC_COLON            ; hex 3A
         dc.b     '/'                  ; hex 2F
         dc.b     SPC_COMMA            ; hex 2C
         dc.b     '*'                  ; hex 2A
         dc.b     SPC_CLOSE            ; hex 29
         dc.b     SPC_OPEN             ; hex 28
         dc.b     QUOTE                ; hex 27
         dc.b     DQUOTE               ; hex 22
RSNUM    EQU      *-RSTable            ; number of codes

         ; Table of jump offsets

         dc.b     RSSemiC-RSTable-1
         dc.b     RSColon-RSTable-2
         dc.b     RSSlash-RSTable-3
         dc.b     RSComma-RSTable-4
         dc.b     RSAster-RSTable-5
         dc.b     RSClose-RSTable-6
         dc.b     RSOpenP-RSTable-7
         dc.b     RSQuote-RSTable-8
         dc.b     RSQuote-RSTable-9

         ; A colon ... may define a label.

RSColon  btst     #STB_BLANK,d6        ; a prior blank?
         beq.s    1$                   ; no
         subq.w   #1,d4                ; back up 
1$:      tst.l    (sp)                 ; building lines/labels?
         beq.s    3$                   ; no ...

         ; Scan backwards until a non-symbol character is found.

         move.l   d4,d3                ; borrow D3 for a minute
         subq.w   #1,d3                ; loop count
         bcs.s    3$                   ; ... nothing to check

         lea      0(a4,d4.l),a0        ; current character
         moveq    #CTB_REXXSYM,d0      ; attribute bit

2$:      move.b   -(a0),d1             ; test character
         btst     d0,0(a2,d1.w)        ; a symbol character?
         dbeq     d3,2$                ; loop back

         addq.w   #1,d3                ; starting index of symbol
         move.l   d4,d2
         sub.w    d3,d2                ; symbol length
         beq.s    3$                   ; nothing found ...

         ; Symbol found ... build a label node.

         movea.l  a3,a0                ; (base) environment
         moveq    #NSF_STRING,d0
         move.w   d2,d1                ; length
         bsr      GetString            ; D0=A0=string structure
         exg      d2,d0                ; length=>D0, string=>D2

         ; Copy the label name.

         addq.w   #ns_Buff,a0          ; destination buffer
         lea      0(a4,d3.l),a1        ; source buffer
         CALLSYS  StrcpyU              ; D0=hash
         movea.l  d2,a0                ; string
         move.b   d0,ns_Hash(a0)       ; store the hash code

         ; Calculate the source position.

         move.l   NSEG(sp),d0          ; segment number
         move.w   d3,d1                ; scan offset
         bsr      SrcPosition          ; (A0 preserved)
         move.l   d0,(a0)              ; save the position

         lea      Labels(sp),a1        ; labels list header
         bsr      AddSrcNode           ; saves D2 in node field
         addq.l   #1,NLAB(sp)

3$:      moveq    #SPC_COLON,d3        ; restore the colon
         moveq    #STF_SPEC,d0         ; new status
         bra.s    RSSave 

         ; A '/' ... may be entering comment mode.

RSSlash  swap     d6                   ; save status
         moveq    #STF_SLASH,d0        ; new status
         bra.s    RSSave               ; save it

         ; A string delimiter (' or ")

RSQuote  move.b   d3,d5                ; enter 'string mode'
         bra.s    RSSave

         ; A comma ... may be a continuation character.

RSComma  moveq    #(STF_SPEC!STF_COMMA),d0
         bra.s    RSSupp

         ; A asterisk ... check for prior '/' to enter comment.

RSAster  btst     #STB_SLASH,d6        ; a previous '/'?
         beq.s    RSSave               ; no ... keep it

         ; '/*' combination ... enter comment mode.

         subq.w   #1,d4                ; back up counter
         moveq    #1,d7                ; enter comment mode
         bra.s    RSSkip               ; skip it

         ; A 'newline' character.  Set the 'NEWINP' flag.

RSNewln  bset     #RSB_LINE,d7         ; a new input line ...
         btst     #STB_SEMI,d6         ; prior semi-colon?
         bne.s    RSSkip               ; yes ...
         moveq    #SPC_SEMI,d3         ; create a semi-colon
         btst     #STB_COMMA,d6        ; prior comma?
         beq.s    RSSemiC              ; no ...

         ; Previous comma was a continuation.  Replace it with a blank.

         move.b   #BLANK,-1(a4,d4.l)   ; install a blank
         moveq    #STF_BLANK,d0        ; change status
         bra.s    RSSkip

         ; A semi-colon ... end of the clause.

RSSemiC  move.w   #(STF_END!STF_SPEC!STF_SEMI),d0
         bra.s    RSSupp

         ; An open parenthesis ... retain preceding blank if present.

RSOpenP  moveq    #STF_SPEC,d0
         bra.s    RSSave 

         ; A close parenthesis.  Don't flag as 'special', since we
         ; need to retain any following blanks.

RSClose

         ; Check whether to suppress a prior blank.

RSSupp   btst     #STB_BLANK,d6        ; prior blank?
         beq.s    RSSave               ; no
         subq.w   #1,d4                ; back up

         ; Check whether the new character will fit in the clause buffer.

RSSave   move.w   d0,d6                ; save D0
         cmp.w    (ra_Length-ra_Buff)(a4),d4
         bcs.s    1$                   ; space available ...
         
         ; No space ... attempt to allocate a larger work buffer.

         movea.l  a3,a0                ; (base) environment
         bsr      DoubleWorkBuffer     ; D0=A0=buffer
         movea.l  d0,a4                ; update pointer
         bne.s    1$                   ; success ...

         ; Allocation failed ... load error code and exit.

         moveq    #ERR10_003,d6        ; "allocation failure"
         bra.s    RSCleanUp

         ; Store the current character in the clause buffer.

1$:      move.w   d6,d0                ; restore D0
         move.b   d3,0(a4,d4.l)        ; install character
         addq.w   #1,d4                ; advance counter

         ; Set the new character attribute status word.

RSSkip   move.w   d0,d6                ; new status
         bpl.s    1$                   ; ... clause not finished

         ; Clause ended ... allocate a string structure for it.

         movea.l  a3,a0                ; (base) environment
         movea.l  a4,a1                ; buffer
         moveq    #NSF_STRING,d0       ; attributes
         move.w   d4,d1                ; length
         bsr      AddString            ; D0=A0=string
         move.l   NLINES(sp),(a0)      ; current source line

         lea      Source(sp),a1        ; Source Array header
         move.l   d0,d2                ; clause string
         bsr      AddSrcNode           ; save it
         addq.l   #1,NSEG(sp)          ; segment count

         moveq    #0,d4                ; reset output count
         moveq    #(STF_SPEC!STF_SEMI),d6

1$:      bra      RSScan

         ; Scan completed ... check for errors.

RSDone   moveq    #ERR10_005,d6        ; "EOF inside a string"
         tst.b    d5                   ; inside a string?
         bne.s    RSCleanUp            ; yes -- error

         moveq    #ERR10_006,d6        ; "EOF inside a comment"
         tst.w    d7                   ; inside a comment?
         bne.s    RSCleanUp            ; yes -- error

         ; Scan completed ... (always) allocate the Source Array.

         moveq    #0,d6                ; clear return code
         lea      Source(sp),a1        ; Source Array header
         move.l   NSEG(sp),d2          ; segment count
         bsr.s    BuildSeg             ; D0=segment
         move.l   d0,PPSEG(sp)         ; save it

         ; Check whether Lines and Labels Arrays to be allocated.

         tst.l    (sp)                 ; build lines/labels arrays?
         beq.s    RSCleanUp            ; no ...

         lea      Labels(sp),a1        ; labels list header
         move.l   NLAB(sp),d2          ; labels count (may be 0)
         bsr.s    BuildSeg             ; D0=segment
         move.l   d0,PPLAB(sp)         ; save pointer

         lea      Lines(sp),a1         ; Lines list header
         move.l   NLINES(sp),d2        ; lines count
         bsr.s    BuildSeg             ; D0=segment
         move.l   d0,PPLINE(sp)        ; save pointer

         ; Release the MultiSrcNode lists ...

RSCleanUp
         lea      Labels(sp),a4        ; Labels Array
         bsr.s    FreeNodes
         addq.w   #(Source-Labels),a4  ; Source Array
         bsr.s    FreeNodes
         addq.w   #(Lines-Source),a4   ; Lines Array
         bsr.s    FreeNodes

         ; Close the IoBuff ...

         movea.l  IOBUFF(sp),a0        ; IoBuff pointer
         CALLSYS  CloseF               ; close it

         movem.l  PPLAB(sp),d1/a0/a1   ; Labels/Source/Lines Arrays
         move.l   d6,d0                ; return code
         lea      sf_SIZEOF(sp),sp     ; restore stack
         movem.l  (sp)+,d2-d7/a2-a5
         rts

* =========================     FreeNodes     ==========================
* Releases MultiSrcNodes from a list.  Private to 'ReadSource'.
* Registers:   A3 -- environment
*              A4 -- list header
FreeNodes
         bra.s    2$                   ; jump in

1$:      movea.l  a3,a0                ; environment
         movea.l  d1,a1                ; node
         move.l   (a1),(a4)            ; update list
         moveq    #msn_SIZEOF,d0       ; size
         CALLSYS  FreeSpace            ; release it

2$:      move.l   (a4),d1              ; a node?
         bne.s    1$                   ; yes
         rts

* ==========================     BuildSeg     ==========================
* Builds a segment array from a linked list of MultiSrcNodes.  This routine
* is private to 'ReadSource'.
* Registers:   A1 -- list header
*              A3 -- storage environment
*              D2 -- number of segments
* Return:      D0 -- segment array
BuildSeg
         movea.l  a3,a0                ; environment
         move.l   d2,d0                ; segment count
         lsl.l    #2,d0                ; 4 bytes per entry
         addq.l   #4,d0                ; ... plus count slot
         move.l   (a1),-(sp)           ; push first node
         bsr      FindSpace            ; D0=A0=segment array
         move.l   (sp)+,d1             ; pop first node

         ; Build the segment in inverse order

         move.l   d2,(a0)+             ; install segment count
         lsl.l    #2,d2                ; scale index
         adda.l   d2,a0                ; end of segment
         bra.s    3$                   ; jump in

1$:      movea.l  d1,a1                ; current node
         move.l   (a1)+,d1             ; next node
         move.l   (a1)+,d2             ; load index
         adda.w   d2,a1                ; end of data

2$:      move.l   -(a1),-(a0)          ; copy data
         subq.w   #4,d2                ; any left?
         bne.s    2$                   ; loop back

3$:      tst.l    d1                   ; another node?
         bne.s    1$                   ; yes
         rts

* =========================     AddSrcNode     =========================
* Saves a data value in a MultiSrcNode, allocating and linking a new node
* into the given list if necessary.  Private to 'ReadSource'.
* Registers:   A1 -- list header
*              A3 -- storage environment
*              D2 -- data to save
AddSrcNode
         move.l   (a1),d0              ; a node?
         beq.s    1$                   ; no
         movea.l  d0,a0                ; first node
         addq.w   #msn_Count,a0        ; count slot
         moveq    #MSNCOUNT*4,d1       ; maximum index
         cmp.l    (a0),d1              ; any slots?
         bhi.s    2$                   ; yes

         ; Node full ... allocate a new one.

1$:      movea.l  a3,a0                ; environment
         moveq    #msn_SIZEOF,d0       ; node size
         move.l   a1,-(sp)             ; push list header
         bsr      FindSpace            ; D0=A0=node
         movea.l  (sp)+,a1             ; pop list header
         move.l   (a1),(a0)+           ; install chain
         move.l   d0,(a1)              ; new node
         clr.l    (a0)                 ; clear index

         ; Increment the index and install the data ...

2$:      addq.l   #4,(a0)              ; increment index
         adda.l   (a0),a0              ; index to data slot
         move.l   d2,(a0)              ; ... install data
         rts
