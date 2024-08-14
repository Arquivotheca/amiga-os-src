* == rexx.asm ==========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ============================     Rexx     ============================
* Entry point for the REXX interpreter.
* Registers:   A0 -- global data pointer
*              A1 -- message packet
* Return:      D0 -- error code
*              A0 -- result string/reason code
STACKBF  SET      MAXFNLEN
Rexx
         movem.l  d2-d7/a2-a5,-(sp)
         lea      -STACKBF(sp),sp      ; name buffer
         movea.l  a0,a3                ; global pointer
         movea.l  a1,a2                ; packet
         move.l   ACTION(a2),d2        ; message action code

         ; Load the error trap for the internal memory allocator.

         lea      RXErr03(pc),a1       ; trap address
         movem.l  a1/a7,rt_ErrTrap(a3) ; save address and stack

         ; Initialize the global data structure.

         bsr      InitGlobal           ; D0=error
         move.l   d0,d6                ; error?
         bne      RXErr                ; yes

         ; ... then initialize the base environment.  Once the environment
         ; has been set up, the internal memory allocator can be used.

         movea.l  a3,a0                ; base environment
         suba.l   a1,a1                ; no predecessor
         bsr      GetEnv

         ; Allocate the arglist clause.

         movea.l  a3,a0                ; base environment
         bsr      GetClause            ; D0=A0=clause
         move.l   d0,ev_ArgList(a3)    ; install it
         movea.l  d0,a5

         ; Set 'RESULT' flag if a result string was requested.

         btst     #RXFB_RESULT,d2      ; result requested?
         beq.s    1$                   ; no
         bset     #EFB_RESULT,EVFLAGS(a3)

1$:      move.l   d2,d3                ; action code
         swap     d3                   ; code=>lower
         andi.w   #RXCODEMASK>>16,d3   ; command code

         ; Call the appropriate routine to process the message packet.

         movea.l  a3,a0                ; base environment
         move.l   a2,a1                ; message packet
         move.l   d2,d1                ; command code
         lea      FuncPkt(pc),a4       ; load address
         cmpi.w   #RXFUNC>>16,d3       ; a "FUNCTION" packet?
         beq.s    2$                   ; yes ...

         lea      CommPkt(pc),a4       ; load address
         bset     #EFB_CMD,EVFLAGS(a3) ; set flag
         cmpi.w   #RXCOMM>>16,d3       ; a "COMMAND" packet?
         bne      RXErr10              ; no -- error

2$:      jsr      (a4)                 ; D0=error
         move.l   d0,d6                ; error occurred?
         bne      RXErr                ; yes

         ; See if any defaults were supplied with the invocation message.
         ; If so, update the global values and the message packet.

         movea.l  gn_MsgPkt(a3),a4     ; global packet
         move.l   rm_CommAddr(a2),d0   ; Host Address given?
         beq.s    3$                   ; no
         move.l   d0,gn_HostAddr(a3)   ; update host address
         move.l   d0,gn_FileExt(a3)    ; update file extension
         movea.l  d0,a0
         CALLSYS  Strlen               ; D0=D1=length (A0 preserved)

         movea.l  a0,a1                ; Host Command
         movea.l  a3,a0
         moveq    #NSF_STRING!NSF_KEEP,d0
         bsr      AddString            ; D0=A0=string
         move.l   d0,ev_COMMAND(a3)    ; initial command address

         ; Check if a file extension was given ...

3$:      move.l   rm_FileExt(a2),d0    ; file extension?
         beq.s    4$                   ; no
         move.l   d0,gn_FileExt(a3)    ; update global

         ; Install the file extension/host address in the global message.

4$:      move.l   gn_HostAddr(a3),rm_CommAddr(a4)
         move.l   gn_FileExt(a3),d0    ; final file extension
         move.l   d0,rm_FileExt(a4)    ; install it

         ; Determine the type of source file, and attempt to open it.

         movea.l  cTL_Head(a5),a0      ; first token
         movea.l  TSTRING(a0),a1       ; function/command name
         addq.w   #ns_Buff,a1          ; offset to string

         btst     #TFB_QUOTED,t_Flags(a0)
         bne.s    5$                   ; ... quoted string
         btst     #RXFB_STRING,d2      ; a "string file"?
         bne.s    5$                   ; yes

         ; Check for a filing system object (file extension in D0).

         movea.l  a3,a0                ; global pointer
         move.l   sp,d1                ; name buffer
         bsr      LocateFile           ; D0=A0=IoBuff pointer
         beq      RXErr01              ; ... error
         bra.s    6$

         ; A "string file" ...

5$:      lea      FILELIST(a3),a0      ; global File List
         moveq    #RXIO_STRF,d0        ; "string file"
         moveq    #0,d1                ; no logical name
         CALLSYS  OpenF                ; D0=A0=IoBuff pointer
         beq      RXErr03              ; allocation failure ...

         ; Build the global SOURCE string ...

6$:      move.l   d0,a5                ; save file pointer
         movea.l  a3,a0                ; global pointer
         bsr      BuildSource

         ; Scan the source program and build the segment array.

         movea.l  a3,a0                ; (base) environment
         move.l   a5,a1                ; file buffer
         moveq    #-1,d0               ; allocate Lines/Labels
         bsr      ReadSource           ; D0=error A0=source
         move.l   d0,d6                ; error?
         bne.s    RXSyntax             ; yes

         move.l   a0,gn_SrcSeg(a3)     ; Source Segment Array
         move.l   a1,gn_LineSeg(a3)    ; Lines Array
         move.l   d1,gn_LabSeg(a3)     ; Labels Array
         move.l   a0,ev_Segment(a3)    ; install the source segment

         ; Determine the line number indentation for tracing.

         move.l   (a1),d0              ; (sCount) source lines
         cmpi.w   #TRINDLIM,d0         ; >= crossover?
         bcc.s    7$                   ; yes
         move.w   #TRINDMIN,ev_Indent(a3)

         ; Call the parser to execute the program.

7$:      CALLSYS  rxParse              ; D6=error code
         bne.s    RXErr                ; error ...

         ; Check whether a result string was requested and is available.

         move.l   EVNAME(a3),d0        ; result string available?
         beq.s    9$                   ; no
         move.l   d0,a0
         btst     #EFB_RESULT,EVFLAGS(a3)
         beq.s    8$

         ; A result string was requested ... allocate one from free memory.

         move.w   ns_Length(a0),d0
         addq.w   #ns_Buff,a0          ; offset to buffer area
         CALLSYS  CreateArgstring      ; D0=A0=string
         move.l   d0,d6                ; string allocated?
         beq.s    RXErr03              ; no ...
         bra.s    9$

         ; Check if this was a "command" invocation.  If so, convert the
         ; return string (in A0) to an integer for the return code.

8$:      btst     #EFB_CMD,EVFLAGS(a3) ; a command?
         beq.s    9$                   ; no
         bsr      CVs2i                ; D0=error D1=integer
         move.l   d0,d6                ; conversion error?
         bne.s    RXErr                ; yes
         bra.s    RXOut

         ; Clear error level and exit.

9$:      moveq    #0,d1                ; error level
         bra.s    RXOut

         ; Error codes

RXSyntax movea.l  a3,a0
         moveq    #ACT_SYNTAX,d1       ; syntax error
         bsr      rxTrace              ; report it
         bra.s    RXErr                ; exit

RXErr01  moveq    #ERR10_001,d6        ; "program not found"
         bra.s    RXErr

RXErr03  moveq    #ERR10_003,d6        ; no memory
         bra.s    RXErr

RXErr10  moveq    #ERR10_010,d6        ; invalid message packet

         ; An error occurred.  Get the severity level.

RXErr    move.l   d6,d0                ; reason code
         CALLSYS  ErrorMsg             ; A0=error string
         moveq    #0,d1
         move.w   (IVALUE+2)(a0),d1    ; severity level

         ; Strip the global data structure of any allocated memory.

RXOut    move.l   d1,d2                ; save error level
         movea.l  a3,a0                ; base environment
         bsr      FreeGlobal           ; release global resources

         movea.l  d6,a0                ; result string/reason code
         move.l   d2,d0                ; error level
         lea      STACKBF(sp),sp
         movem.l  (sp)+,d2-d7/a2-a5
         rts

* =========================     CurrentEnv     =========================
* Returns the current environment.  Only register A0 is changed.
* Registers:   A0 -- global pointer (base environment)
* Return:      A0 -- current environment
CurrentEnv:
         movea.l  gn_CurrEnv(a0),a0
         rts

* ===========================     CommPkt     ==========================
* Processes a command packet by parsing the command string into tokens.
* Each token defines an argument string and is linked into the argument
* list.
* Registers:      A0 -- base environment
*                 A1 -- message packet
*                 D0 -- clause
*                 D1 -- command code
* Return:         D0 -- error code or 0
CommPkt
         movem.l  d2-d4/a2-a5,-(sp)
         movea.l  a0,a3                ; base environment
         movea.l  gn_TBuff(a3),a4      ; temporary buffer
         movea.l  d0,a5                ; clause

         moveq    #0,d2                ; clear token count
         move.l   d1,d3                ; action code

         ; Make sure there's a command string!

         move.l   ARG0(a1),d0          ; command string?
         beq.s    CPError              ; no ...
         movea.l  d0,a2

         ; Make sure the command isn't too long.

         movea.l  a2,a0                ; command
         CALLSYS  Strlen               ; D0=D1=length
         cmp.l    MaxLen(pc),d0        ; length <= maximum?
         bhi.s    CPError              ; no ... error

         ; Check whether the temporary buffer is big enough.

         cmp.w    (ra_Length-ra_Buff)(a4),d0
         bls.s    CPScan               ; buffer big enough ...

         ; Temporary buffer too small ... allocate a new one.

         movea.l  a3,a0                ; global pointer
         bsr      ResizeWorkBuffer     ; D0=A0=work buffer
         beq.s    CPError              ; ... failure
         movea.l  a0,a4                ; new buffer

         ; Scan the command string, breaking out tokens.

CPScan   movea.l  a2,a0                ; scan pointer
         CALLSYS  StcToken             ; D0=quote D1=length A0=scan A1=token
         tst.l    d1                   ; a token?
         beq.s    CPCheck              ; no ... all done

         btst     #RXFB_STRING,d3      ; string file?
         bne.s    1$                   ; yes
         tst.w    d2                   ; first token?
         beq.s    2$                   ; yes
         btst     #RXFB_TOKEN,d3       ; tokenize?
         bne.s    2$                   ; yes

         ; String is not to be tokenized ... get length of remainder.

1$:      movea.l  a1,a0                ; start of token
         CALLSYS  Strlen               ; D0=D1=length (A0/A1 preserved)
         adda.l   d1,a0                ; advance to end
         moveq    #0,d0                ; not quoted

         ; Check for a quoted string and remove the quotes.

2$:      movea.l  a0,a2                ; update scan pointer

         moveq    #0,d4                ; clear flags
         tst.b    d0                   ; quoted string?
         beq.s    3$                   ; no ...
         moveq    #1<<TFB_QUOTED,d4    ; 'quoted' flag

         ; Strip quotes and replace double-delimiters in the string ...
         ; token length never includes the closing quote or null byte.

         movea.l  a4,a0                ; temporary buffer
         addq.w   #1,a1                ; advance pointer
         subq.l   #1,d1                ; decrement length
         bsr      Strcopy              ; D1=length A1=string

         ; Allocate a string structure and a token for the argument string.

3$:      movea.l  a3,a0                ; (base) environment
         moveq    #NSF_STRING,d0
         bsr      AddString            ; D0=A0=string

         ; Allocate and link a token into the arglist clause.

         movea.l  a3,a0                ; (base) environment
         movea.l  a5,a1                ; clause
         moveq    #T_STRING,d1
         bsr      AddToken             ; D0=A0=token
         move.b   d4,t_Flags(a0)       ; install flags

         addq.w   #1,d2                ; increment token count
         bra.s    CPScan               ; loop back

         ; Scan finished ... install the argument count.

CPCheck  moveq    #0,d0                ; clear error
         subq.w   #1,d2                ; argument count
         move.w   d2,c_Count(a5)       ; save it
         bpl.s    CPOut                ; all ok ...

         ; Error ... invalid packet.

CPError  moveq    #ERR10_010,d0        ; error code

CPOut    tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2-d4/a2-a5
         rts

* ===========================     FuncPkt     ==========================
* Processes function packets by moving the argument strings into tokens in
* the arglist clause.  The tokens are of type T_FUNCTION, since the
* argument strings are not to be released.
* Registers:      A0 -- storage environment
*                 A1 -- message packet
*                 D0 -- clause
*                 D1 -- command code
* Return:         D0 -- error code or 0
FuncPkt
         movem.l  d2/a2/a3/a5,-(sp)
         movea.l  a0,a3                ; base environment
         movea.l  d0,a5                ; clause

         lea      ARG0(a1),a2          ; address of first argument
         moveq    #0,d2
         move.b   d1,d2                ; argument count (excludes name)
         move.w   d2,c_Count(a5)       ; save count

1$:      move.l   (a2)+,d0             ; an argument string?
         beq.s    2$                   ; no ...
         subq.l   #ra_Buff,d0          ; subtract buffer offset

2$:      movea.l  a3,a0                ; (base) environment
         movea.l  a5,a1                ; clause
         moveq    #T_FUNCTION,d1       ; token type
         bsr      AddToken             ; link it in
         dbf      d2,1$                ; decrement and loop

         moveq    #0,d0                ; all ok
         movem.l  (sp)+,d2/a2/a3/a5
         rts

* =========================     BuildSource     ========================
* Constructs the global SOURCE string.  The string is formatted:
* "COMMAND 0 invocation-name resolved-name file-extension host-address"
* Registers:   A0 -- global pointer
BuildSource
         move.l   a3,-(sp)
         movea.l  a0,a3                ; base environment

         ; Push pointers to the filename and default strings ...

         lea      BSTable(pc),a0       ; start of table
         moveq    #4-1,d1              ; loop count

1$:      pea      QMark(pc)            ; push default
         moveq    #0,d0                ; clear offset
         move.b   (a0)+,d0             ; structure offset
         move.l   0(a3,d0.w),d0        ; string available?
         beq.s    2$                   ; no
         move.l   d0,(sp)              ; ... install it
2$:      dbf      d1,1$                ; loop back

         ; Check whether a result string was requested.

         moveq    #1<<EFB_RESULT,d0    ; load mask
         and.b    EVFLAGS(a3),d0       ; select bit
         lsr.b    #EFB_RESULT,d0       ; align it ... 0 or 1
         move.w   d0,-(sp)             ; push flag

         ; Get the invocation mode ("COMMAND" or "FUNCTION")

         pea      BSComm(pc)           ; default
         btst     #EFB_CMD,EVFLAGS(a3) ; a command?
         bne.s    3$                   ; yes
         addq.l   #BSCOMM,(sp)         ; advance pointer

         ; Format the data stream ... format string already in A0.

3$:      movea.l  sp,a1                ; data stream
         move.l   gn_TBuff(a3),d0      ; buffer area
         bsr      FormatString         ; D0=D1=length A0=buffer
         lea      (4+4+4+4+2+4)(sp),sp ; restore stack

         ; Allocate a string structure for the SOURCE string.

         movea.l  a0,a1                ; string buffer
         movea.l  a3,a0                ; environment
         move.b   #NSF_STRING!NSF_SOURCE,d0
         bsr      AddString            ; D0=A0=string
         move.l   d0,gn_SOURCE(a3)     ; install it

         movea.l  (sp)+,a3
         rts

         ; Structure offsets for default strings

BSTable  dc.b     gn_HostAddr          ; host address
         dc.b     gn_FileExt           ; file extension
         dc.b     gn_FileName          ; resolved name
         dc.b     gn_SrcFile           ; invocation name

         ; Constant strings

BSFormat dc.b     '%s %d %s %s %s %s',0
BSComm   dc.b     'COMMAND',0
BSCOMM   EQU      *-BSComm
         dc.b     'FUNCTION',0
         CNOP     0,2                  ; realignment
