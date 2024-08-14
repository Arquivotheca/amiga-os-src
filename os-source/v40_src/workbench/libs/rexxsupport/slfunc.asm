*
* $Id: slfunc.asm,v 36.1 90/08/28 11:34:02 mks Exp $
*
* $Log:	slfunc.asm,v $
* Revision 36.1  90/08/28  11:34:02  mks
* Added RCS header information.  ($Id and $Log)
* 
* ===========================     SLFunc     ===========================
* Query function for REXX support library.  Message packet has the function
* name as the first argstring.
* Registers:   A0 -- message packet
*              A6 -- Support library base
* Return:      D0 -- error code or 0
*              A0 -- result (an argstring)
NUMBER   SET      16
BOOLEAN  SET      17
NULLSTR  SET      18
NEWSTR   SET      31
SLFunc
         movem.l  d2-d7/a2-a6,-(sp)
         movea.l  a0,a5                ; save message packet
         movea.l  rs_REXXBase(a6),a4   ; REXX library base

         ; Check whether the function name is in the list ...

         moveq    #ERR10_001,d6        ; default error
         movea.l  ARG0(a5),a0          ; function name
         subq.w   #ra_Buff,a0          ; string structure
         bsr      SLFList              ; D0=offset A0=scan pointer
         move.w   d0,d5                ; save offset
         beq      SLFDone              ; "program not found"

         ; Check that the argument count is valid.  A0 points to the
         ; argument byte, packed as MAX<<4 | MIN.

         moveq    #$0F,d2              ; argument mask
         and.b    (a0),d2              ; minimum arguments
         moveq    #0,d7                ; flags (upper word)/args (lower)
         move.b   (ACTION+3)(a5),d7    ; argument count
         cmp.b    d2,d7                ; count >= minimum?
         blt      SLFErr17             ; no -- error

         ; Make sure that none of the required arguments are missing.

         lea      ARG0(a5),a1          ; argument block (function name)
0$:      tst.l    (a1)+                ; present?
         dbeq     d2,0$                ; loop back
         beq      SLFErr17             ; missing ...

         move.b   (a0)+,d2             ; argument byte
         lsr.b    #4,d2                ; shift for maximum
         cmp.b    d2,d7                ; at most the maximum?
         bgt      SLFErr17             ; no -- error

         ; Check if numeric (integer) conversion required.  Conversion is
         ; specified by setting the bit in the argument flags byte.

         lea      ARG1(a5),a2          ; start of argblock
         move.w   d7,d2                ; argument count
         moveq    #0,d3                ; last value converted
         move.b   (a0),d6              ; conversion byte
         bra.s    2$                   ; jump into loop

1$:      movea.l  (a2)+,a0             ; argument string
         asr.b    #1,d6                ; flag bit set?
         bcc.s    2$                   ; no ...
         move.l   a0,d0                ; a string?
         beq.s    2$                   ; no

         ; Convert the number ...

         exg      a4,a6                ; SUPPORT=>A4, REXX=>A6
         subq.w   #ns_Buff,a0          ; string structure
         CALLSYS  CVs2i                ; D0=error D1=value
         exg      a4,a6
         bne      SLFErr18             ; ... conversion error
         move.l   d1,d3                ; result >= 0?
         bmi      SLFErr18             ; no ... invalid

2$:      tst.b    d6                   ; any flags left?
         dbeq     d2,1$                ; ... loop back

         ; Prepare registers and make the call.  Register assignments are:
         ; A0 -- first argument
         ; A1 -- second argument
         ; A2 -- global data structure
         ; A3 -- global work area
         ; A4 -- REXX base             ; (preserved)
         ; A5 -- message packet        ; (preserved)
         ; A6 -- library base          ; (preserved)
         ; D0 -- first argument length
         ; D1 -- second argument length
         ; D2 -- cleared
         ; D3 -- last numeric argument value
         ; D4 -- EXEC base
         ; D5 -- function offset
         ; D6 -- cleared               ; (return error code)
         ; D7 -- flags/argument count
         ; Scratch registers are A0-A3 and D0-D5.  D6 is the error code.

         movem.l  ARG1(a5),a0/a1       ; first/second arguments
         movea.l  rm_TaskBlock(a5),a2  ; global pointer
         movea.l  gn_TBuff(a2),a3      ; work area (TEMPBUFF long)

         moveq    #0,d0
         moveq    #0,d1
         moveq    #0,d2
         move.l   rl_SysBase(a4),d4    ; EXEC base

         cmpi.b   #1,d7                ; one argument?
         beq.s    3$                   ; yes
         blt.s    4$                   ; ... none

         move.l   a1,d6                ; second argument?
         beq.s    3$                   ; no
         move.w   (ns_Length-ns_Buff)(a1),d1

3$:      move.l   a0,d6                ; first argument?
         beq.s    4$                   ; no
         move.w   (ns_Length-ns_Buff)(a0),d0

4$:      moveq    #0,d6                ; default return code

SLFJump  jsr      SLFJump(pc,d5.w)     ; call the function
         tst.l    d6                   ; an error?
         bne.s    SLFError             ; yes

         ; Check whether a return string needs to be prepared.

         move.l   d7,d0
         swap     d0                   ; flags=>lower
         tst.w    d0                   ; any flags?
         beq.s    SLFDone              ; no

         ; Decode the special cases.

         tst.l    d7                   ; (NEWSTR) allocate a string?
         bpl.s    1$                   ; no
         movea.l  a3,a0                ; buffer area
         move.l   d3,d0                ; length
         bra.s    4$                   ; ... allocate string

         ; Check for an integer return ...

1$:      btst     #NUMBER,d7           ; an integer?
         beq.s    2$                   ; no
         move.l   d3,d0
         movea.l  a4,a6                ; REXX=>A6
         CALLSYS  CVi2arg              ; D0=A0=argstring
         bra.s    5$                   ; check result

         ; Check for a BOOLEAN return

2$:      movea.l  rl_NULL(a4),a0       ; the global NULL
         moveq    #0,d0                ; zero length
         btst     #BOOLEAN,d7          ; a boolean value?
         beq.s    3$                   ; no

         moveq    #1,d0                ; length
         movem.l  rl_FALSE(a4),a0/a1   ; FALSE/TRUE strings
         tst.l    d3                   ; false?
         beq.s    3$                   ; yes
         movea.l  a1,a0                ; TRUE string

3$:      addq.w   #ra_Buff,a0          ; advance pointer

         ; Allocate an argstring for the result ...

4$:      movea.l  a4,a6                ; REXX=>A6
         CALLSYS  CreateArgstring      ; D0=A0=argstring

         ; Result string allocated?

5$:      bne.s    SLFDone              ; all ok

         ; Error return codes

SLFErr03 moveq    #ERR10_003,d6        ; allocation failure
         bra.s    SLFError

SLFErr17 moveq    #ERR10_017,d6        ; wrong number of arguments
         bra.s    SLFError

SLFErr18 moveq    #ERR10_018,d6        ; invalid argument

SLFError suba.l   a0,a0                ; clear the return

SLFDone  move.l   d6,d0                ; return code
         movem.l  (sp)+,d2-d7/a2-a6
         rts

         ; Shared returns

SFNewstr bset     #NEWSTR,d7
         rts

SFNumber bset     #NUMBER,d7
         rts

SFBool   bset     #BOOLEAN,d7
         rts

SFNull   bset     #NULLSTR,d7
         rts

         ; Error returns

SFErr03  moveq    #ERR10_003,d6
         rts

SFErr18  moveq    #ERR10_018,d6
         rts

* ===========================     SLFList     ==========================
* Checks whether the supplied function name is in the Rexx Support Library.
* If so, the jump offset and argument data are returned.
* Registers:   A0 -- function name (string structure)
* Return:      D0 -- function offset or 0
*              A0 -- scan pointer
* Scratch:     D2/D3/A2/A3
MAXNAME  SET      9                    ; length of longest name
SLFList
         lea      ra_Hash(a0),a3       ; start of test string
         moveq    #0,d2                ; clear index
         move.w   ra_Length(a0),d3     ; test string length

         ; See if the test string is too long ...

         moveq    #MAXNAME,d0          ; maximum name length
         cmp.w    d0,d3                ; too long?
         bhi.s    4$                   ; yes ...

         move.b   SLCount(pc,d3.w),d0  ; load count
         move.b   SLIndex(pc,d3.w),d2  ; load index
         bra.s    3$                   ; jump in

         ; Scan through the function names looking for a match.

1$:      lea      SLTable(pc,d2.w),a0  ; function name
         movea.l  a3,a1                ; test string
         move.w   d3,d1                ; length (loop count + 1)

2$:      cmpm.b   (a0)+,(a1)+          ; check the string
         dbne     d1,2$                ; loop back if equal
         beq.s    5$                   ; name matched ...

         add.w    d3,d2                ; skip string
         addq.w   #3,d2                ; skip hash and argument bytes
3$:      dbf      d0,1$                ; loop back

4$:      moveq    #0,d0                ; function not found ...
         rts

         ; Function name found ... load jump address offset.

5$:      move.b   SLTotal(pc,d3.w),d3  ; cumulative count
         sub.w    d0,d3                ; back up
         add.w    d3,d3                ; scale for 2 bytes
         lea      (SLJump-2)(pc),a1    ; jump table
         adda.w   d3,a1                ; index to entry
         move.w   (a1),d0              ; load offset
         rts

         ; Count of names to check indexed by length.

SLCount  dc.b     0,0,0,0,2,3,8,6,3,1  ; 0-9

         ; Cumulative names indexed by length.

SLTotal  EQU      *-4                  ; 0-3
         dc.b     2,5,13,19,22,23      ; 4-9

         ; This patch of the table is the starting index for the string
         ; length.  Only strings of the same length as the test string
         ; are checked.

SLIndex  EQU      *-4                  ; 0-3
         dc.b     SL4-SLTable          ; 4
         dc.b     SL5-SLTable          ; 5
         dc.b     SL6-SLTable          ; 6
         dc.b     SL7-SLTable          ; 7
         dc.b     SL8-SLTable          ; 8
         dc.b     SL9-SLTable          ; 9

         ; Names for the Support Library functions.  Each name is preceded
         ; by the hash byte and followed by two argument bytes.  The first
         ; argument byte holds the maximum and minimum number of arguments,
         ; and the second byte specifies numeric argument conversions.

SLTable
SL4      dc.b     63,'NEXT',$21,$02
         dc.b     59,'NULL',$00,$00

SL5      dc.b     93,'BADDR',$11,$00
         dc.b     111,'DELAY',$11,$01
         dc.b     140,'REPLY',$31,$02

SL6      dc.b     179,'DELETE',$11,$00
         dc.b     182,'FORBID',$00,$00
         dc.b     186,'GETARG',$21,$02
         dc.b     207,'GETPKT',$11,$00
         dc.b     199,'OFFSET',$22,$00
         dc.b     209,'PERMIT',$00,$00
         dc.b     184,'RENAME',$22,$00
         dc.b     199,'STATEF',$11,$00

SL7      dc.b     21,'FORWARD',$22,$00
         dc.b     1,'FREEMEM',$22,$02
         dc.b     253,'MAKEDIR',$11,$00
         dc.b     32,'SHOWDIR',$31,$00
         dc.b     49,'TYPEPKT',$21,$00
         dc.b     36,'WAITPKT',$11,$00

SL8      dc.b     74,'ALLOCMEM',$21,$01
         dc.b     119,'OPENPORT',$11,$00
         dc.b     125,'SHOWLIST',$41,$00

SL9      dc.b     187,'CLOSEPORT',$11,$00
         CNOP     0,2                  ; force alignment

         ; The jump table contains the offsets for the actual function code.

SLJump   dc.w     SFnext-SLFJump
         dc.w     SFnull-SLFJump

         dc.w     SFbaddr-SLFJump
         dc.w     SFdelay-SLFJump
         dc.w     SFreply-SLFJump

         dc.w     SFdelete-SLFJump
         dc.w     SFforbid-SLFJump
         dc.w     SFgetarg-SLFJump
         dc.w     SFgetpkt-SLFJump
         dc.w     SFoffset-SLFJump
         dc.w     SFpermit-SLFJump
         dc.w     SFrename-SLFJump
         dc.w     SFstatef-SLFJump

         dc.w     SFforward-SLFJump
         dc.w     SFfreemem-SLFJump
         dc.w     SFmakedir-SLFJump
         dc.w     SFshowdir-SLFJump
         dc.w     SFtypepkt-SLFJump
         dc.w     SFwaitpkt-SLFJump

         dc.w     SFallocmem-SLFJump
         dc.w     SFopenport-SLFJump
         dc.w     SFshowlist-SLFJump

         dc.w     SFcloseport-SLFJump
