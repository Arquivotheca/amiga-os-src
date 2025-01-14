
**************************************************************
*  Assembler support for the BCPL run-time system under UNIX *
*                                                            *
* (C) Copyright 1985 METACOMCO plc                           *
* 26 Portland Square, Bristol, England                       *
*                                                            *
**************************************************************
*
        INCLUDE  "LIBHDR-i"
        INCLUDE  "SYSHDR-i"
*
RESPACE EQU (SCB.UPB+1)*4*3
*
        XDEF    _main,_bcplmain
        XREF    _free,_malloc,_sbrk,_lseek,_unlink,_stacksize
        XREF    _brk,_open,_close,_read,_write,_exit,_errno
*
_main:
        TST.L   (SP)+           ; Entered via JSR so junk return address
        TST.B   -144(SP)        ; make sure we have some stack
*
* Before we start, ensure that the end of the program data segment
* is longword-aligned. The system store allocation routine can
* only return pointers to word-aligned (NOT lword) addresses.
* Therefore, if we align to lword boundary and always request in
* multiples of 4 bytes, everything should work (??!!) since the
* system memory allocation routine uses a lword link field
*
        MOVE.L  #0,-(sp)        ; push arg for _sbrk
        JSR     _sbrk           ; Get current 'endds'
        ADDQ.L  #4,SP           ; pop arg from stack
        ADDI.L  #3,D0           ; a:=(a+3)&#XFFFFFFFC
        ANDI.L  #$FFFFFFFC,D0   ; nbytes past last lword boundary
*
        MOVE.L  D0,-(SP)        ; push argument for _brk
        JSR     _brk
        ADDQ.L  #4,SP
*
* Now start
*
        LEA.L   MEND(PC),A0     ; pointer to first BCPL module
        MOVEQ   #0,D0           ; current HRG
        LEA.L   _stacksize,A5   ; pointer to last long word of program
HGLOB0:
        MOVEA.L A0,A4           ; a4 = MC addr of section
        CMPA.L  A4,A5           ; Is it last word ?
        BEQ.S   GLOBIN0         ; Yes, so jump
        MOVE.L  (A4),D2         ; No, so d2 = section length (words)
        ASL.L   #2,D2           ; Now in bytes
        LEA.L   0(A4,D2.L),A0   ; a0 = MC addr of next section
        MOVEA.L A0,A3           ; a3 = same
        MOVE.L  -(A3),D1        ; Get MAXGLOB at end of present section
        CMP.L   D0,D1           ; Check with highest so far
        BLE.S   HGLOB0          ; Jump if less than max
        MOVE.L  D1,D0           ; Update maximum
        BRA.S   HGLOB0          ; Process next section
*
* Now go about setting up the global vector
*
GLOBIN0:
*
* First get some space for the Global Vector, Console SCB, etc.
*
        ADDQ    #1,D0              ; GV size = n+1 words
        MOVE.L  D0,-(SP)           ; Save this for now
        ASL.L   #2,D0              ; Into bytes
        ADDI.L  #RESPACE,D0        ; Size of Data Space needed
        ADD.L   _stacksize,D0      ; Plus space for BCPL stack
        MOVE.L  D0,-(SP)           ; Amount of space needed
        JSR     _sbrk              ; Get space at break point
        MOVE.L  D0,A2              ; ptr returned by _sbrk -> a2
        TST.L   (SP)+              ; Pop argument to _sbrk
        MOVE.L  (SP)+,D0           ; restore GV size in words to d0
        SUBQ    #1,D0              ; d0 = HRG
        MOVE.L  D0,(A2)            ; Update global zero with GV size
        ASL.L   #2,D0              ; Back to bytes
        MOVE.L  D0,D6              ; Save gv size in bytes
        MOVE.L  #UNDEF.GN,D1       ; Should cause an error
        MOVEQ.L #0,D2              ; Zero D2
UNGL:   
        ADDQ.L  #4,D2              ; Next Global
        ADDQ.L  #2,D1              ; Gn = UNGLOB+2*n
        MOVE.L  D1,0(A2,D2.L)      ; Make all globals undefined
        CMP.L   D2,D0              ; Are we finished?
        BNE.S   UNGL               ; loop until base of globals
        LEA.L   MEND(PC),A0        ; Pointer to first BCPL module
GLOBIN1:
        MOVEA.L A0,A4              ; a4 = MC address of the section
        CMPA.L  A4,A5              ; Is it end of prog
        BEQ.S   GLOBIN3            ; Yes, so jump
        MOVE.L  (A4),D2            ; No so d2 = Section length in words
        ASL.L   #2,D2              ; Now in bytes
        LEA.L   0(A4,D2.L),A0      ; a0 = MC addr of next section
        MOVEA.L A0,A3              ; Into a3
        TST.L   -(A3)              ; move past MAXGLOB
GLOBIN2:
        MOVE.L  -(A3),D2           ; d2 = entry relative pointer
        BEQ.S   GLOBIN1            ; Jump if at end of Gn/Ln pairs
        MOVE.L  -(A3),D3           ; d3 = the global number
        ADD.L   A4,D2              ; d2 = MC addr of entry
        ASL.L   #2,D3              ; d3 = Gvec subscript in bytes
        MOVE.L  D2,0(A2,D3.L)      ; Put entry in Global Vector
        BRA.S   GLOBIN2            ; Deal with next Gn/Ln pair
GLOBIN3:
*
* Set up scbs for STDIN,STDOUT,STDERR,CIS,COS
*
        LEA.L   4(A2,D6.L),A4          ; Ptr to beyond GV
        MOVE.L  #0,SCB.POS*4(A4)       ; Zero Position
        MOVE.L  #0,SCB.END*4(A4)       ; Zero End
        MOVE.L  #ID.INSCB,SCB.ID*4(A4)       ; Stream type
        MOVE.L  #FD.STDIN,SCB.FILDES*4(A4)   ; Store File Descriptor
        MOVE.L  A4,D0                  ; Into D0
        ASR.L   #2,D0                  ; Make BCPL ptr
        MOVE.L  D0,G_STDIN*4(A2)       ; Ptr to scb for stdin
        MOVE.L  D0,G_CIS*4(A2)         ; Which will be cis
        LEA.L   (SCB.UPB+1)*4(A4),A4   ; Advance past scb
        MOVE.L  #0,SCB.POS*4(A4)       ; Zero Position
        MOVE.L  #0,SCB.END*4(A4)       ; Zero End
        MOVE.L  #ID.OUTSCB,SCB.ID*4(A4)      ; Stream type
        MOVE.L  #FD.STDOUT,SCB.FILDES*4(A4)  ; Store File Descriptor
        MOVE.L  A4,D0                  ; Into D0
        ASR.L   #2,D0                  ; Make BCPL ptr
        MOVE.L  D0,G_STDOUT*4(A2)      ; Ptr to scb for stdout
        MOVE.L  D0,G_COS*4(A2)         ; Which will be cos
        LEA.L   (SCB.UPB+1)*4(A4),A4   ; Advance past scb
        MOVE.L  #0,SCB.POS*4(A4)       ; Zero Position
        MOVE.L  #0,SCB.END*4(A4)       ; Zero End
        MOVE.L  #ID.OUTSCB,SCB.ID*4(A4)      ; Stream type
        MOVE.L  #FD.STDERR,SCB.FILDES*4(A4)  ; Store File Descriptor
        MOVE.L  A4,D0                  ; Into D0
        ASR.L   #2,D0                  ; Make BCPL ptr
        MOVE.L  D0,G_STDERR*4(A2)      ; Ptr to scb for stderr
        LEA.L   (SCB.UPB+1)*4(A4),A4   ; Advance past scb
*
* Assign BCPL stack pointer P, STACKTOP, STACKBASE, STACKSIZE
*
        MOVE.L  _stacksize,D0          ; get stacksize
        MOVE.L  D0,G_STACKSIZE*4(A2)   ; Store as global
        MOVEA.L A4,A1                  ; Ptr to BCPL Stack
        MOVE.L  A4,G_STACKBASE*4(A2)   ; Store as global
        LEA.L   0(A4,D0),A4            ; Advance past stack
        MOVE.L  A4,G_STACKTOP*4(A2)    ; Store as global
*
        SUBA.L  A0,A0                  ; Zero Z
        LEA.L   SAVE(PC),A5            ; Set S
        LEA.L   RET(PC),A6             ; And R
*
* We now have a BCPL environment
*
        MOVE.L  (SP),D1                ; 'argc'
        MOVE.L  4(SP),D2               ; 'argv'
        MOVE.L  #12,D0                 ; Min Stack frame
        MOVEA.L 4*G_TRANSARGS(A2),A4   ; Address of routine to call
        JSR     (A5)
*
* Now we're ready to call 'start'
*
        MOVE.L  (SP),D1                ; 'argc'
        MOVE.L  4(SP),D2               ; 'argv'
        MOVE.L  #12,D0                 ; Min Stack frame
        MOVEA.L 4*G_START(A2),A4       ; Address of routine to call
_bcplmain:
        JSR     (A5)
*
* End program if start returns
*
        MOVEA.L G_STOP*4(A2),A4    ; Entry point of STOP
        MOVEQ   #12,D0             ; push increment
        MOVEQ   #0,D1              ; Exit status = 0
        JSR     (A5)               ; Do it
*
* Subroutine called via R register
*
RET:
        MOVEM.L -12(A1),A1/A3      ; Standard return sequence
        MOVEA.L -4(A1),A4
        JMP     (A3)
*
*************************************************************
* The standard SAVE routine which is used to call a routine *
*       Entered with B pointing to routine address          *
*************************************************************
SAVE:
        MOVEA.L (SP)+,A3                | (00)
        MOVEM.L A1/A3/A4,-12(A1,D0.L)   | (02)
        ADDA.L  D0,A1                   | (08)
        MOVEM.L D1-D4,(A1)              | (0A)
        JMP     (A4)                    | (0E)
*
* Subroutines which are called relative to the S register
*
**********************************************************
*  Branches to arithmetic routines                       *
**********************************************************
MULT:
        BRA.S   MULTIPLY        | (10)
DIVID:
        BRA.S   DIVIDE          | (12)
DIVX:
        BRA.S   DIV01           | (14)
*
******************************************************
*       Stack Check                                  *
* Entered with D0 = Stack frame size of this routine *
******************************************************
*
STACKCHECK:
*       ADD.L   P,D0                New top of stack      [16]
*       MOVE.L  G_SBASE(G),D5       Current stackbase as BCPL address
*       LSL.L   #2,D5               MC address
*       cmp.l   C_SEND(Z,D5.L),D0   Compare stack limit with stack top
*       BGE.S   STACKOVER           J if top > limit
*       RTS                         OK
STACKOVER:
*       MOVEQ   #84,D0              Stack overflow error
*       TRAP    #0
        RTS
*
******************************************************
*       Multiply    32 bit multiply routine          *
*       Performs D1 := D1 * D2                       *
******************************************************
*
MULTIPLY:
        MOVEM.L D2-D4,-(SP)         Save registers used
        MOVE.W  D1,D3               Low word into D3
        MOVE.W  D2,D4               Low word into D4
        SWAP    D1                  High word into D1
        SWAP    D2                  High word into D2
        MULU    D3,D2               D2H * D1L
        MULU    D4,D1               D1H * D2L
        MULU    D4,D3               D1L * D2L
        ADD.W   D2,D1               D2H*D1L + D1H"D2L
        SWAP    D1                  Get into high word
        CLR.W   D1                  Clear bottom word
        ADD.L   D3,D1               (D2H*D1L + D1H*D2L) << 16  +  D1L*D2L
        MOVEM.L (SP)+,D2-D4         Restore registers
        RTS
        PAGE
*
******************************************************
*  Divide          32 bit division routine   *
*      Performs D1 := D1/D2; D2 := D1 REM D2         *
*      Corrupts all data registers                   *
******************************************************
*
DIVIDE:
        TST.L   D2                  Denominator < 0 ?
        BPL     DIV01
        neg.l   D2
        bsr     DIV01
        neg.l   D1
        RTS
DIV01:
        TST.L   D1                  Numerator < 0 ?
        BPL     DIV02
        neg.l   D1
        bsr     DIV02               Divide abs(d1) by d2
        neg.l   D1
        neg.l   D2                  Correct for sign
        RTS
*
* DIV02 performs division on unsigned numbers. It first sees if the
* denominator is small enough for straight-line code using DIVU to
* be applicable
* set D1, D2 := D1/D2, D1 Rem D2.
* D1 and D2 are treated as unsigned numbers, and 0<=D1<=#X80000000
* and 0<D2<=#X80000000.
*     Note that both D1 and D2 can be as big as one greater than the
*     largest ordinary positive integer held in a 32-bit (signed) word.
* If D2=0 the 68000 will take a 'divide by zero' trap.
* The only case that can lead to an answer that is out of the range
* of normal (signed) numbers is #X80000000/1 which can arise when
* Divide1 was called on #X80000000/(-1)
DIV02:
        CMPI.L  #$0000FFFF,D2
        BHI     DIV04               Denominator is larger than 16 bits
* DIV03 is used as a subroutine called from the general case code.
* It returns with D1,D2 := D1/D2, D1 rem D2, and D3 and D6 corrupted
DIV03:
        MOVE.L  D1,D6
        CLR.W   D1
        SWAP    D1                  High part of numerator
        DIVU    D2,D1
        MOVE.L  D1,D3               Save partial result away
        MOVE.W  D6,D1
        DIVU    D2,D1               Other part of the division
        SWAP    D1
        CLR.L   D2
        MOVE.W  D1,D2
        MOVE.W  D3,D1
        SWAP    D1
        RTS
*
* DIV04 is reached if the denominator does not fit in 16 bits. It takes
* a quick exit if the numerator is less than the denominator
*
DIV04:
        cmp.l   D1,D2
        BLS     DIV05               Branch in general case
        MOVE.L  D1,D2
        MOVEQ.L #0,D1               Result IS zero
        RTS
*
* At DIV05, since the denominator is >= 2^16 and the numerator is
* < 2^32, the quotient will be < 2^16. An approximation to the
* required quotient is produced by dividing both numerator & denominator
* by a scale factor computed as 1+(D2/#X10000), and then doing a
* short division by the (now) 16-bit denominator. The scale factor fits in
* 16 bits even after the '+1' because the original D2 had #X80000000 as its
* largest possible value, so the largest scale factor is #X00008001.
*
DIV05:
        MOVE.L  D2,D7
        CLR.W   D7
        SWAP    D7                  Top half of D2 = D2/#X10000
        addq.l  #1,D7               Increment it to get safe scale factor
        MOVE.L  D1,D4
        MOVE.L  D2,D5               Grab a safe copy of the input
        MOVE.L  D7,D2
        BSR     DIV03               D1 := D1 / Scalefactor
        MOVE.L  D5,D2
        DIVU    D7,D2               now D2 (word) = scaled denominator
*
* the choice of scalefactor here was intended to ensure that the
* scaled denominator now in D2 was less than 16 bits long, and entry
* conditions imply that the quotient I compute will be small enough that
* a single DIVU order can be used to get it
        DIVU    D2,D1
* D1 (word) now holds an estimate for the quotient that I want.
* Check it, and produce a remainder, by multiplying by original
* denominator & comparing with original numerator.
        ANDI.L  #$0000FFFF,D1       Get rid of top half, since not wanted
DIV06:
        MOVE.L  D5,D2
        MOVE.L  D5,D3
        SWAP    D3
        MULU    D1,D2
        MULU    D1,D3
        SWAP    D3            Top half of D3 should have been zero
        ADD.L   D3,D2      and this ADD can not cause a carry
        sub.l   D4,D2
        BHI     DIV08      overshot, since remainder seems negative
        neg.l   D2
        cmp.l   D2,D5
        BHI     DIV07      OK, remainder is in range
        addq.l  #1,D1               Crank up the quotient
        BRA     DIV06
DIV07:
        RTS
DIV08:
        SUBQ.L  #1,D1
        BRA     DIV06               Adjust quotient & try again
*
        CNOP    0,4
MEND:
*
*
MODS:
        DC.L    (MODE-MODS)/4       Start a BCPL section
*
* Temporary globals for now
*
        SECT    <'multipl'>
MUL:
        JSR     $10(A5)         ; Call off S
        JMP     (A6)
*
        SECT    <'divide '>
DIV:
        JSR     $12(A5)
        JMP     (A6)
*
        SECT    <'remaind'>
REM:
        JSR     $12(A5)
        MOVE.L  D2,D1
        JMP     (A6)
*
* Stop(rc)
*
        SECT    <'stop   '>
GSTOP:
        MOVE.L  D1,-(SP)        ; push exit status ready for call
        MOVE.L  G_COS*4(A2),D1  ; Is the output stream still open?
        BEQ.S   STOP1           ; No, good, so we can finish
*
* Flush COS before closing down
*
        MOVEQ   #12,D0              ; minimum stackframe size
        MOVEA.L G_DEPLETE*4(a2),a4  ; Address of Deplete
        JSR     (A5)                ; SCB pointer already in D1
*
STOP1:
        JSR     _exit    ; EXIT(status)
*
        JMP     (A6)            ; Should not get here
*
*
* Space Management.
*
* Unix provides two low-level routines for allocating space;
* these are _brk and _sbrk. With these all it is possible to do is
* to increase/decrease the 'end of data section' pointer. To
* selectively allocate/free sections of memory it is necessary
* to make use of two higher level functions - _malloc and _free -
* which make use of _sbrk and _brk, but implement a linked list
* structure within the program's data segment.
*
        SECT    <'getvec '>
GETVEC:
        MOVE.L  A1,-(SP)        | save P
        ADDQ.L  #1,D1           | add one to requested size
        ASL.L   #2,D1           | words -> bytes
        MOVE.L  D1,-(SP)        | arg for _malloc on stack
        JSR     _malloc         | Syscall _malloc
        ADDQ.L  #4,SP           | pop arg for _malloc
GETVEC1:
        MOVEQ   #0,D1           | clear d1
        TST.L   D0              | Did it work?
        BEQ.S   GETVEC2         | No
        MOVE.L  D0,D1           | ptr to allocated space
        LSR.L   #2,D1           | as BCPL ptr
GETVEC2:
        SUBA.L  A0,A0           | restore Z
        MOVE.L  (SP)+,A1        | restore P
        JMP     (A6)            | and home
*
* freevec(ptr)
*
        SECT    <'freevec'>
FREEVEC:
        MOVE.L  A1,-(SP)        | save P 
        LSL.L   #2,D1           | make MC address
        MOVE.L  D1,-(SP)        | argument for _free
        JSR     _free           | Syscall _free
        ADDQ.L  #4,SP           | restore stack
        BRA     GETVEC2
*
*********************  I/O SECTION  ****************************
*
* Low-level call to UNIX open
* fildes := OPEN(name,oflag,mode)
*
        SECT    <'open   '>
OPEN:
        MOVE.L   A1,-(SP)               Save P
        MOVE.L   D3,-(SP)               push permissions [mode]
        MOVE.L   D2,-(SP)               push flag
        MOVE.L   D1,-(SP)               push pointer to pathname
        JSR      _open                  Call open
        LEA.L    12(SP),SP              Restore Stack
        MOVE.L   D0,D1                  Return result in D1
        MOVE.L   _errno,G_RESULT2*4(G)  If failed, give reason
        BRA      GETVEC2
*
* CLOSE( fildes )
*
        SECT    <'close  '>
CLOSE:
        MOVE.L A1,-(SP)                 Save P
        MOVE.L D1,-(SP)                 Push File Descriptor
        JSR    _close                   CLOSE(fildes)
        ADDQ.L #4,SP                    Pop Argument
        MOVE.L   D0,D1                  Return result in D1
        MOVE.L   _errno,G_RESULT2*4(G)  If failed, give reason
        BRA    GETVEC2 
*
****************************
* READ(fildes,buff,nbytes) *
****************************
*
        SECT    <'read   '>
READ:
        MOVE.L   A1,-(SP)               Save P
        MOVE.L   D3,-(SP)               push nbytes
        MOVE.L   D2,-(SP)               push buffer pointer
        MOVE.L   D1,-(SP)               push file descriptor
        JSR      _read                  Call read
        LEA.L    12(SP),SP              Restore Stack
        MOVE.L   D0,D1                  Return result in D1
        MOVE.L   _errno,G_RESULT2*4(G)  If failed, give reason
        BRA      GETVEC2
*
****************************
* WRITE(fildes,buff,nbytes) *
****************************
*
        SECT    <'write  '>
WRITE:
        MOVE.L   A1,-(SP)               Save P
        MOVE.L   D3,-(SP)               push nbytes
        MOVE.L   D2,-(SP)               push buffer pointer
        MOVE.L   D1,-(SP)               push file descriptor
        JSR      _write                 Call read
        LEA.L    12(SP),SP              Restore Stack
        MOVE.L   D0,D1                  Return result in D1
        MOVE.L   _errno,G_RESULT2*4(G)  If failed, give reason
        BRA      GETVEC2
*
********************************************************************************
*   Muldiv        (A * B) / C  routine with 64 bit working                     *
********************************************************************************
*
*  (A * B) / C routine for BCPL, where the intermediate result
*  (A * B) is stored as a 64 bit number.
*
            SECT    <'MULDIV '>
MULDIV
            MOVE.L  D3,D4
            MOVEQ.L #0,D3
            JMP     MULDIV1(PC)
*
*******************************************************************************
*        Muldiv1          (A * B + C) / D  routine with 64 bit working        *
*******************************************************************************
*
            SECT   <'MULDIV1'>
*
MULDIV1:
*
* First there is some straightforward code that computes the
* 64-bit product of A and B, leaving the result in the register
* pair (d6,d7).
            MOVE.L  D1,D0               (D1*d2)/d3
            MOVE.L  D2,D5
            SWAP    D0
            SWAP    D5
            MOVE.L  D1,D7
            MULU    D2,D7               A1L*A2L
            MOVE.L  D0,D6
            MULU    D5,D6               A1H*A2H
            MULU    D1,D5
            MULU    D2,D0               A1H*A2L and A1L*A2H
            ADD.L   D0,D5               sum of cross terms (X bit now relevant)
            CLR.L   D0
            MOVE.W  D5,D0               low order parts of cross terms
            SWAP    D0                  allign them in a sensible place
            CLR.W   D5                  leave only top half of D5
* in the next instruction I want something like addx.w  #0,d5, but
* ADDX only allows register operands. Hence I use d0 which happens to have
* its low 16 bits all zero.
            ADDX.W  D0,D5               put in the carry bit
            SWAP    D5                  17 bit part of cross terms now tidily in D5
            ADD.L   D0,D7
            ADDX.L  D5,D6               (D6,d7) now contains the (unsigned) 64 bit product
*                                       of d1 and d2
*
* Correct the product to allow for the fact that A and B should have been
* treated as being signed numbers
*
            TST.L   D1
            BPL.S   MULDIV01
            SUB.L   D2,D6
MULDIV01    TST.L   D2
            BPL.S   MULDIV02
            SUB.L   D1,D6
MULDIV02    ADD.L   D3,D7               Now add in C, allowing for carry
            CLR.L   D3
            ADDX.L  D3,D6
            MOVE.L  A6,-(SP)             Push a return address onto stack,
*                                       so that rts behaves as jmp (a6) would have.
* There now follows some code that is rather similar to that at Divide1,
* it adjusts quotient and remainder calculation to allow for the possibility
* of negative arguments.
* muldiv03 is called as a subroutine to divide (d6,d7) by d4. It must
* preserve d4, hand back a quotient in d1 and leave a remainder in
* RESULT2. Otherwise its use of data registers is unrestricted.
MULDIV03    TST.L   D4                  Even allow for negative divisors
            BPL.S   MULDIV04
            NEG.L   D4
            BSR.S   MULDIV04
            NEG.L   D1
            RTS
MULDIV04    TST.L   D6
            BPL.S   MULDIV05            Branches on sign of numerator...
            NEG.L   D7
            NEGX.L  D6
            BSR.S   MULDIV05            Perform adjustments to get sign of
            NEG.L   D1                  quotient and remainder both right
            NEG.L   G_RESULT2*4(A2)
            RTS
* At muldiv05 I have to divide the 64-bit unsigned number (d6,d7) (D6 is the
* high word) by the unsigned integer in d4, leaving a quotient in d1 and
* a remainder in RESULT2. I can return with an rts, and need not bother to
* save any of the D registers
MULDIV05    CMPI.L  #$0000FFFF,D4
            BHI.S   MULDIV06            Full treatment needed for big divisor
* Here d4 is known to be short. Perform short division,
* using d1 as a work register. This is done in such a way that the
* quotient returned is the low 32 bits of whatever the true quotient of
* (d6,d7) by d4 would be when the true quotient is bigger than 2^32.
            MOVE.L  D6,D1
            CLR.W   D1
            SWAP    D1                  Top 16 bits of numerator
            DIVU    D4,D1               quotient gets thrown away as overflow!
            MOVE.W  D6,D1
            DIVU    D4,D1               again the quotient is not wanted!
            SWAP    D7
            MOVE.W  D7,D1
            DIVU    D4,D1
            MOVE.W  D1,D6               will be high part of final quotient
            SWAP    D7
            MOVE.W  D7,D1
            DIVU    D4,D1
            SWAP    D6
            MOVE.W  D1,D6               assembled final quotient
            CLR.W   D1
            SWAP    D1
            MOVE.L  D1,G_RESULT2*4(A2)
            MOVE.L  D6,D1
            RTS
MULDIV06    TST.L   D6
            BNE     MULDIV07            it really is a big division
            tst.l   d7
            BMI     MULDIV07            .. treat as big if (d6,d7)>=0,#X80000000
            move.l  d7,d1
            move.l  d4,d2
            move.l  d4,-(sp)            preserve D4 over the call to DIV01
* NOTE: d1 may be +ve or -ve here, but d2 must be treated as positive
* even if it has the value #X80000000.
            jsr     DIV01               Can use normal 32-bit division here
            move.l  (sp)+,d4
            MOVE.L  D2,G_RESULT2*4(A2)
            rts
MULDIV07    move.l  d4,d5               Here I scale both numbers & get an ...
            clr.w   d5                  ... approximate quotient
            swap    d5
            addq.l  #1,d5               calculate a scale factor
            move.l  d4,d0
            divu    d5,d0               D0 (16 bits) = scaled divisor
            move.l  d6,d1
            clr.w   d1
            swap    d1
            divu    d5,d1
            move.w  d1,d2               first 16 bits of scaled dividend
            move.w  d6,d1
            divu    d5,d1
            swap    d2
            move.w  d1,d2               Top 32 bits of scaled dividend complete
            swap    d7
            move.w  d7,d1
            divu    d5,d1
            move.w  d1,d3
            swap    d7
            divu    d5,d1
            swap    d3
            move.w  d1,d3               Now (d2,D3) is scaled dividend
* A further short division will get an approximation, roughly 16 bits
* accurate, to the final required quotient. I need to compute
* (d2,d3)/d0
            move.l  d2,d1
            clr.w   d1
            swap    d1
            divu    d0,d1
            move.w  d1,d5
            move.w  d2,d1
            divu    d0,d1
            swap    d5
            move.w  d1,d5
            swap    d3
            move.w  d3,d1
            divu    d0,d1
            move.w  d1,d2
            swap    d3
            move.w  d3,d1
            divu    d0,d1
            swap    d2
            move.w  d1,d2
* Now (d5,d2) is my approximate final quotient, d4 still contains the
* original denominator, and (d6,d7) the 64-bit numerator. I want to
* multiply-up and subtract to get a provisional remainder.
            move.l  d2,d0
            mulu    d4,d0               V0*Q0
            move.l  d5,d1
            mulu    d4,d1               V2*Q0
            sub.l   d0,d7
            subx.l  d1,d6               subtracted from numerator
            move.l  d2,d0
            swap    d0
            mulu    d4,d0               V1*Q0
            swap    d4
            move.l  d2,d1
            mulu    d4,d1               V0*Q1
            move.l  d2,d3
            swap    d3
            mulu    d4,d3               V1*Q1
            sub.l   d3,d6
            clr.l   d3
            add.l   d0,d1
            move.l  d5,d0
            mulu    d4,d0               V2*Q1
            addx.l  d0,d3
            swap    d4                  restore D4 to normal state
            clr.l   d0
            move.w  d1,d0
            swap    d0
            move.w  d3,d1
            swap    d1
            sub.l   d0,d7
            subx.l  d1,d6               Remainder now in (D6,d7)
            move.l  d2,-(sp)
* Call muldiv04 to try dividing the residual (d6,d7) by d4.
* Each recursion here puts (almost) 16 bits more onto the quotient
* that is being developed, so after 2 recursive calls the answer should
* be so nearly complete that an exit through DIV01 gets taken.
* NOTE: (d6,d7) may be negative here, but d4 must be treated as
* being positive, even if it is #X80000000.
            bsr     MULDIV04            Recurse to deal with bit at the end
            add.l   (sp)+,d1
            tst.l   G_RESULT2*4(a2)     Now I have to allow for the fact that
            bpl.s   MULDIV08            .. the recursive call to muldiv03 did
            add.l   d4,G_RESULT2*4(a2)  .. a signed rather than unsigned job.
            subq.l  #1,d1               Result should only need changing by 1.
MULDIV08    rts
*
******************************************************************
*                                                                *
*              APTOVEC( Function, Upperbound )                   *
*                                                                *
******************************************************************
*
          SECT        <'aptovec'>
*
APTOVEC   MOVEA.L     D1,A4            Save routine entry point
*
          MOVE.L      A1,D1            Vector base is D1
          MOVE.L      A1,D0            save byte version of sddress
          LSR.L       #2,D1
          SUBQ.L      #3,D1            allow for aptovec's savespace
*
          MOVE.L      D2,D3
          ASL.L       #2,D3            Size of required vector in bytes
          LEA         4(A1,D3.L),A1    reset P pointer to leave space for vector
*
          MOVEM.L     D1/D2,(A1)       args for callee on stack as well as regs
*
*
          MOVE.L      A4,-4(A1)                Write in the base
          MOVE.L      -8(A0,D0.L),-8(A1)       return address
          MOVE.L      -12(A0,D0.L),-12(A1)     old p pointer
*
          JMP         (A4)              enter the called function
*
******************************************************************
*                                                                *
*                  Get2bytes(vector, wordoffset)                 *
*                                                                *
******************************************************************
*
          SECT        <'GET2BYT'>
*
GET2BYTES asl.l       #2,d1
          asl.l       #1,d2
          add.l       d1,d2               D2th byte on MC vector
          clr.l       d1
          move.w      0(a0,d2.l),d1        Fetch word
          jmp         (a6)
*
*****************************************************************
*                                                               *
*          Put2bytes(vector,wordoffset,word)                    *
*                                                               *
*****************************************************************
*
          SECT         <'PUT2BYT'>
*
PUT2BYTES asl.l        #2,d1
          asl.l        #1,d2
          add.l        d1,d2
          move.w       d3,0(a0,d2.l)       Put the given word there
          jmp          (a6)
*
******************************************************************
*                                                                *
*                  Gbytes(ba, size)                              *
*                                                                *
******************************************************************
*
          SECT        <'GBYTES '>
*
GBYTES    MOVEA.L     D1,A4                B = MC addr of next byte
          CLR.L       D1
*
GBYTES1   LSL.L       #8,D1
          ADD.B       (A4)+,D1             Add in next byte
          SUBQ.B      #1,D2
          BNE.S       GBYTES1
*
          JMP         (A6)
*
*****************************************************************
*                                                               *
*                   Pbytes(ba,size,word)                        *
*                                                               *
*****************************************************************
*
          SECT         <'PBYTES '>
*
PBYTES    movea.l      d1,a4               B = MC addr of first byte
*
PBYTES1   move.b       d3,-1(a4,d2.l)      Move a byte (last first)
          lsr.l        #8,d3              Position word for next
          subq.b       #1,d2
          bne.s        PBYTES1            Continue if necessary
          jmp          (a6)
*
****************************************************************
*                                                              *
*                   Level()                                    *
*                                                              *
****************************************************************
*
          SECT         <'LEVEL  '>
*
LEVEL     MOVE.L        -12(A1),D1
          JMP           (A6)
*
****************************************************************
*                                                              *
*                   Longjump(stackp,label)                     *
*                                                              *
****************************************************************
*
          SECT          <'LONGJMP'>
*
LONGJUMP  MOVEA.L       D1,A1
          MOVEA.L       -4(A1),A4
          JMP           0(A0,D2.L)
*
****************************************************************
*                                                              *
*            LSEEK( fildes, offset, whence )                   *
*                                                              *
****************************************************************
*
          SECT         <'lseek  '>
* 
LSEEK     MOVE.L   A1,-(SP)               Save P
          MOVE.L   D3,-(SP)               Push whence
          MOVE.L   D2,-(SP)               Push Offset
          MOVE.L   D1,-(SP)               Push Fildes
          JSR      _lseek                 lseek(fildes,offset,whence)
          LEA.L    12(SP),SP              Restore stack
          MOVE.L   D0,D1                  Result in D1
          MOVE.L   _errno,G_RESULT2*4(G)  If failed, give reason
          BRA      GETVEC2
*
****************************************************************
*                                                              *
*          UNLINK( filename )                                  *
*                                                              *
****************************************************************
*
          SECT  <'unlink '>
*
UNLINK
           MOVE.L   A1,-(SP)              save P
           MOVE.L   D1,-(SP)              Ptr to C string
           JSR      _unlink               unlink(filename)
           ADDQ.L   #4,SP                 restore stack
           MOVE.L   D0,D1                 return code in d1
           MOVE.L   _errno,G_RESULT2*4(G) if failed, give reason
           BRA      GETVEC2
* 
* Global Linkage
*
        CNOP    0,4             ; align to long word boundary
*
        DC.L    0               ; end if Gn/Ln pairs
*
        DC.L    G_STOP,GSTOP-MODS
        DC.L    G_MULTIPLY,MUL-MODS
        DC.L    G_DIVIDE,DIV-MODS
        DC.L    G_REMAINDER,REM-MODS
        DC.L    G_GETVEC,GETVEC-MODS
        DC.L    G_FREEVEC,FREEVEC-MODS
        DC.L    G_READ,READ-MODS
        DC.L    G_WRITE,WRITE-MODS
        DC.L    G_OPEN,OPEN-MODS
        DC.L    G_CLOSE,CLOSE-MODS
        DC.L    G_UNLINK,UNLINK-MODS
        DC.L    G_LSEEK,LSEEK-MODS
        DC.L    G_MULDIV,MULDIV-MODS
        DC.L    G_GBYTES,GBYTES-MODS
        DC.L    G_PBYTES,PBYTES-MODS
        DC.L    G_GET2BYTES,GET2BYTES-MODS
        DC.L    G_PUT2BYTES,PUT2BYTES-MODS
        DC.L    G_LEVEL,LEVEL-MODS
        DC.L    G_LONGJUMP,LONGJUMP-MODS
        DC.L    G_APTOVEC,APTOVEC-MODS
*
        DC.L    100             ; dummy HRG
*
MODE:
        END
*
***************************************
* END of file 'first'                 *
***************************************

