* == bif800.asm ========================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     OPEN     =============================
* Opens a file with the specified logical name.
* Usage: OPEN(name,filename,['Append' | 'External' | 'Read' | 'Write'])
BIFopen
         movea.l  ev_GlobalPtr(a4),a2  ; global pointer
         bclr     #LOWERBIT,d5         ; convert to uppercase
         cmpi.b   #3,d7                ; option given?
         blt.s    2$                   ; no

         ; Check for a valid option.

         moveq    #RXIO_APPEND,d0      ; append mode
         subi.b   #'A',d5              ; 'Append'?
         beq.s    3$                   ; yes
         subq.b   #'E'-'A',d5          ; 'External'?
         beq.s    1$                   ; yes
         subi.b   #'R'-'E',d5          ; 'Read'?
         beq.s    2$                   ; yes
         moveq    #RXIO_WRITE,d0       ; write mode
         subq.b   #'W'-'R',d5          ; 'Write'?
         beq.s    3$                   ; yes
         bne.s    BF8002               ; error ... invalid option

1$:      subq.w   #4,d1                ; 4-byte address?
         bne.s    BF8002               ; no
         moveq    #RXIO_EXIST,d0       ; external filehandle
         movea.l  (a1),a1              ; load filehandle
         bra.s    3$

         ; Open the file ...

2$:      moveq    #RXIO_READ,d0        ; default mode

3$:      move.l   a0,d1                ; logical name
         lea      FILELIST(a2),a0      ; global File List
         CALLSYS  OpenF                ; D0=A0=IoBuff
         sne      d3                   ; ... set flag
         bra.s    BF8001               ; boolean return

* ===========================     EXISTS     ===========================
* Tests whether a file exists.
* Usage: EXISTS(filename)
BIFexists
         CALLSYS  ExistF               ; D0=boolean
         move.l   d0,d3
         bra.s    BF8001               ; boolean return

* ============================     CLOSE     ===========================
* Closes a file and returns a boolean flag.
* Usage: CLOSE(name)
BIFclose
         bsr.s    CheckLogicalName     ; D4=A0=node
         beq.s    BF8001               ; not found ...

         ; Check whether to clear current console handler ...

         movea.l  (rt_MsgPort+MP_SIGTASK)(a2),a3
         exg      a2,a0                ; global=>A0, IoBuff=>A2
         move.l   iobDFH(a2),d1        ; a filehandle?
         beq.s    1$                   ; no

         lsl.l    #2,d1
         movea.l  d1,a1                ; FileHandle structure
         move.l   fh_Type(a1),d1       ; load handler
         cmp.l    pr_ConsoleTask(a3),d1; current handler?
         bne.s    1$                   ; no

         ; Fall back to the STDOUT stream as current handler ...

         movea.l  rl_STDOUT(a6),a1     ; logical name
         bsr      FindConsoleHandler   ; D0=handler or 0
         move.l   d0,pr_ConsoleTask(a3); ... install it

         ; Close the IoBuff ...

1$:      movea.l  a2,a0                ; IoBuff pointer
         CALLSYS  CloseF               ; close the file
         moveq    #-1,d3               ; set flag
         bra.s    BF8001               ; boolean return

* =============================     EOF     ============================
* Tests a logical name for end-of-file.
* Usage: EOF(name)
BIFeof
         bsr.s    CheckLogicalName     ; D0=A0=node
         beq.s    BF8002               ; not found ...
         move.b   IOBEOF(a0),d3        ; load flag

BF8001   bra      BFBool               ; boolean return

BF8002   bra      BFErr18              ; invalid operand

* =====================     CheckLogicalName     =======================
* Looks for the specified logical filename and returns the IoBuff pointer.
* Register A1 is preserved.
* Registers:   A0 -- name
*              A2 -- global pointer
* Return:      D4 -- IoBuff or 0
*              A0 -- same
CheckLogicalName
         move.l   a1,-(sp)             ; save A1
         lea      -ns_Buff(a0),a1      ; logical name
         movea.l  a2,a0                ; global pointer
         bsr      FindLogical          ; D0=A0=IoBuff
         movea.l  (sp)+,a1             ; restore A1
         move.l   d0,d4                ; save IoBuff
         rts

* ===========================     READCH     ===========================
* Reads one or more characters from a file.  The default count is 1.
* USAGE: READCH(file,[length])
BIFreadch
         bsr.s    CheckLogicalName     ; D4=A0=IoBuff
         beq.s    BF8002               ; ... not found

         cmpi.b   #2,d7                ; length argument?
         beq.s    1$                   ; yes
         moveq    #1,d3                ; load default

         ; Make sure the request isn't too long ...

1$:      cmp.l    MaxLen(pc),d3        ; too long?
         ble.s    2$                   ; no
         move.l   MaxLen(pc),d3        ; ... use maximum

         ; Allocate a string as the read buffer ... string will become
         ; the return value if the read is successful.

2$:      movea.l  a4,a0                ; environment
         moveq    #NSF_STRING,d0       ; attribute
         move.l   d3,d1                ; length
         bsr      GetString            ; D0=A0=string
         movea.l  a0,a2                ; save string
         lea      ns_Buff(a2),a3       ; update work area

         ; Install in the argument block for recycling ...

         bsr      AddArgument          ; D7=new count

         movea.l  d4,a0                ; IoBuff pointer
         movea.l  a3,a1                ; buffer area
         move.l   d3,d0                ; length

         ; Copy any buffered characters first ...

         move.l   iobRct(a0),d2        ; buffered count
         cmp.l    d2,d3                ; request > buffer?
         bgt.s    4$                   ; yes
         move.l   d3,d2                ; use requested length

4$:      movea.l  iobRpt(a0),a0        ; buffer pointer
         move.l   d2,d1                ; length
         bra.s    6$                   ; jump in

         ; Copy the buffered characters ...

5$:      move.b   (a0)+,(a1)+          ; copy buffer
6$:      dbf      d1,5$                ; loop back

         movea.l  d4,a0                ; IoBuff pointer
         add.l    d2,iobRpt(a0)        ; advance pointer
         sub.l    d2,iobRct(a0)        ; update count
         sub.l    d2,d0                ; remainder count
         beq.s    7$                   ; ... nothing to read

         ; Read the file into the buffer in a single call ...

         movea.l  iobDFH(a0),a0        ; filehandle
         CALLSYS  DOSRead              ; D0=length or -1 D1=secondary
         movea.l  d4,a0                ; restore IoBuff
         bmi.s    BF8004               ; ... error
         seq      IOBEOF(a0)           ; nothing read ... set EOF

7$:      add.l    d2,d0                ; add buffer length
         exg      d0,d3                ; request=>D0, actual=>D3
         sub.l    d3,d0                ; read complete?
         bne.s    BF8003               ; no ... new string required

         ; Read successful ... compute the hash code for the result.

         bra.s    9$                   ; jump in
8$:      add.b    (a3)+,d0             ; compute hash
9$:      dbf      d3,8$                ; loop back
         move.b   d0,ns_Hash(a2)       ; install code
         movea.l  a2,a0                ; return value
         rts

* ===========================     READLN     ===========================
* Reads a line from a file, using the 'newline' as a delimiter.
* Usage: READLN(file)
BIFreadln
         bsr      CheckLogicalName     ; D4=A0=node
         beq.s    BF8007               ; ... not found

         move.w   (ra_Length-ra_Buff)(a3),d2
         subq.w   #1,d2                ; maximum length - 1
         moveq    #NEWLINE,d5          ; delimiter

         ; Read characters until a 'newline' is found.

1$:      CALLSYS  ReadF                ; D0=character or -1  D1=secondary
         movea.l  d4,a0                ; restore IoBuff
         bmi.s    BF8rln01             ; ... error
         cmp.w    d0,d5                ; delimiter?
         beq.s    BF8003               ; yes ... all done
         move.b   d0,0(a3,d3.l)        ; install character
         addq.w   #1,d3                ; increment length
         dbf      d2,1$                ; loop back

BF8003   bra      BFNewstr             ; string return

         ; Check end-of-file flag ... EOF or error?

BF8rln01 tst.b    IOBEOF(a0)           ; end-of-file?
         bne.s    BF8003               ; yes

         ; A read error ... return null string.

BF8004   bset     #NULL,d7             ; null return

         ; An error occurred ... set 'RC' and check for interrupt.

BF8005   movea.l  a4,a0                ; environment
         move.l   d1,d0                ; secondary error code
         moveq    #INB_IOERR,d1        ; interrupt signal
         bsr      rxSignal             ; D0=error D1=flag

         ; Interrupt signalled?  Load error to suppress return value.

         tst.w    d1                   ; trapped?
         beq.s    1$                   ; no
         moveq    #ERR10_012,d6        ; load error

1$:      rts

* ===========================     WRITELN     ==========================
* Writes a string to a file, appending a 'newline' as a delimiter.
* USAGE: WRITELN(name,string)
BIFwriteln
         movem.l  d1/a0,-(sp)          ; push length/name

         ; Copy the string and add it to the argument block.

         movea.l  a4,a0                ; environment
         subq.w   #ns_Buff,a1          ; recover string
         bsr      CopyString           ; D0=A1=string
         bsr      AddArgument          ; D7=new count

         movem.l  (sp)+,d1/a0          ; pop length/name
         addq.w   #ns_Buff,a1          ; start of string
         move.b   #NEWLINE,0(a1,d1.l)  ; install 'newline'
         addq.l   #1,d1                ; new length

* ===========================     WRITECH     ==========================
* Writes a string of one or more characters to a file.
* Usage: WRITECH(name,string)
BIFwritech
         move.l   d1,d3                ; save length
         bsr      CheckLogicalName     ; D4=A0=node (A1 preserved)
         beq.s    BF8007               ; ... not found

         move.l   d3,d0                ; length
         CALLSYS  WriteF               ; DO=count D1=secondary code

         ; Numeric return ... save return value and check for an I/O error.

BF8006   bset     #NUMBER,d7           ; set flag
         move.l   d0,d3                ; value >= 0?
         bmi.s    BF8005               ; no ... error
         rts

BF8007   bra      BFErr18              ; invalid operand

* =============================     SEEK     ===========================
* Seeks to the specified position in a file.  The new position is returned.
* Usage: SEEK(name,offset,['Begin','Current','End'])
BIFseek
         movea.l  ev_GlobalPtr(a4),a2  ; global pointer
         bsr      CheckLogicalName     ; D4=A0=IoBuff (A1 preserved)
         beq.s    BF8007               ; ... not found

         moveq    #RXIO_CURR,d2        ; default anchor
         cmpi.b   #3,d7                ; anchor argument given?
         bne.s    1$                   ; no

         ; Determine the anchor point.

         bclr     #LOWERBIT,d5         ; convert to uppercase
         subi.b   #'C',d5              ; 'Current'?
         beq.s    1$                   ; yes
         moveq    #RXIO_END,d2         ; 'End' anchor
         subq.b   #'E'-'C',d5          ; 'End'?
         beq.s    1$                   ; yes
         moveq    #RXIO_BEGIN,d2       ; 'Begin' anchor
         addq.b   #'E'-'B',d5          ; 'Begin'?
         bne.s    BF8007               ; no

         ; Convert the offset argument to an integer.

1$:      lea      -ns_Buff(a1),a0      ; recover string
         bsr      CVs2i                ; D0=error D1=value
         bne.s    BF8007               ; ... conversion error

         ; Seek to the specified position.

         move.l   d1,d0                ; offset
         move.l   d2,d1                ; anchor mode
         movea.l  d4,a0                ; IoBuff pointer
         CALLSYS  SeekF                ; D0=position D1=secondary
         bra.s    BF8006               ; check return

* ===========================     LINES     ============================
* Returns the number of lines available from an interactive stream.
* Usage: LINES([name])
BIFlines
         tst.b    d7                   ; argument given?
         bne.s    1$                   ; yes
         movea.l  rl_STDIN(a6),a0      ; default name
         addq.w   #ns_Buff,a0          ; offset to buffer

1$:      bsr      CheckLogicalName     ; D4=A0=IoBuff
         beq.s    BF8007               ; not found

         ; Wait for a character ... secondary return is line count.

         movea.l  iobDFH(a0),a0        ; filehandle
         moveq    #100,d0              ; microseconds
         bsr      DOSWaitChar          ; D0=boolean D1=lines
         beq.s    BF8006               ; no characters ... return 0
         move.l   d1,d0                ; return lines
         bra.s    BF8006

* =========================     SOURCELINE     =========================
* Returns the specified line from the current source file.
* Usage: SOURCELINE([line])
BIFsourceln
         move.l   d3,d2                ; save line number

         ; Get the total line count ...

         movea.l  gn_LineSeg(a2),a0    ; lines array
         move.l   (a0),d3              ; line count ...
         subq.l   #1,d3                ; minus one
         tst.b    d7                   ; any args?
         beq      BFNumber             ; no

         ; Make sure the line number is valid ...

         cmp.l    d3,d2                ; requested line <= end?
         bgt      BFErr18              ; no ... error
         lsl.l    #2,d2                ; scale for 4 bytes
         beq      BFErr18              ; ... error

         ; Determine the position and length of the line.

         movem.l  0(a0,d2.l),d2/d3     ; start/next position
         sub.l    d2,d3                ; line length

         ; Check for a "string file" ... no open necessary.

         move.l   gn_FileName(a2),d1   ; source file?
         beq.s    1$                   ; no ... string file

         ; Open the source file, if possible.

         lea      FILELIST(a2),a0      ; global File List
         movea.l  d1,a1                ; filename
         moveq    #RXIO_READ,d0        ; open mode
         moveq    #0,d1                ; no logical name
         CALLSYS  OpenF                ; D0=A0=IoBuff
         beq      BF8005               ; ... failure

         ; Seek to the starting position ...

         movea.l  a0,a2                ; save IoBuff
         move.l   d2,d0                ; starting offset
         moveq    #RXIO_BEGIN,d1       ; anchor
         CALLSYS  SeekF                ; D0=position

         ; ... and read the line into the temporary buffer.

         movea.l  iobDFH(a2),a0        ; DOS filehandle
         movea.l  a3,a1                ; temporary buffer
         move.l   d3,d0                ; line length
         CALLSYS  DOSRead              ; D0=count or -1 D1=secondary
         move.l   d0,d2                ; save count

         ; Close the IoBuff and check for errors ...

         movea.l  a2,a0                ; IoBuff
         CALLSYS  CloseF               ; close it
         cmp.l    d2,d3                ; full line read?
         bne      BF8005               ; no ... error
         bra.s    2$

         ; A "string file" ... get the source string.

1$:      movea.l  ev_ArgList(a2),a0    ; global arglist
         movea.l  cTL_Head(a0),a0      ; first token
         movea.l  TSTRING(a0),a1       ; source string
         lea      ns_Buff(a1,d2.l),a3  ; offset to string

         ; Check for a trailing 'newline' and trim it.

2$:      cmpi.b   #NEWLINE,-1(a3,d3.l) ; a newline?
         bne.s    3$                   ; no
         subq.l   #1,d3                ; ... trim it

3$:      bra      BFNewstr             ; new string

* ===========================     SHOW     =============================
* Shows the names in the local or global resource lists.
* Usage: SHOW(list,[name],[pad])
BIFshow
         movea.l  ev_GlobalPtr(a4),a2
         move.b   (a0),d0              ; option character
         bclr     #LOWERBIT,d0         ; convert option to uppercase

         lea      rl_ClipList(a6),a0   ; ClipList header
         subi.b   #'C',d0              ; 'Clip'?
         beq.s    1$                   ; yes
         lea      FILELIST(a2),a0      ; Files header
         subq.b   #'F'-'C',d0          ; 'Files'?
         beq.s    1$                   ; yes
         lea      PORTLIST(a2),a0      ; REXX ports
         subq.b   #'I'-'F',d0          ; 'Internal'?
         beq.s    1$                   ; yes
         lea      rl_LibList(a6),a0    ; LibList header
         subq.b   #'L'-'I',d0          ; 'Libraries'?
         beq.s    1$                   ; yes
         movea.l  rl_SysBase(a6),a0    ; EXEC base
         lea      PortList(a0),a0      ; EXEC ports
         subq.b   #'P'-'L',d0          ; 'Ports'?
         bne.s    BF8008               ; no -- invalid option

         ; Look for a particular name?

1$:      cmpi.b   #2,d7                ; name argument?
         blt.s    2$                   ; no
         move.l   a1,d1                ; supplied?
         beq.s    2$                   ; no

         moveq    #RRT_ANY,d0          ; any node
         CALLSYS  FindRsrcNode         ; D0=A0=node
         sne      d3                   ; ... set flag
         bra      BFBool               ; boolean return

         ; Build a list of all names, then copy into internal memory.
         ; Task switching is forbidden while walking the list (D1 = 0).

2$:      move.b   d5,d0                ; separator character.
         CALLSYS  ListNames            ; D0=A0=argstring
         beq.s    3$                   ; ... allocation error
         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; argstring
         bra      CVarg2s              ; D0=A0=string

3$:      moveq    #ERR10_003,d6        ; allocation failure
         rts

BF8008   bra      BFErr18              ; invalid operand

* ==========================      SETCLIP     ==========================
* Adds a string to the global Clip List.
* Usage: SETCLIP(name,value)
BIFsetclip
         movea.l  gn_MsgPkt(a2),a3     ; message packet
         move.l   #RXADDCON,d2         ; "add value" command
         move.l   a1,ARG1(a3)          ; value string
         move.l   d1,ARG2(a3)          ; length > 0?
         bne.s    BF8010               ; yes

         ; Remove string if null value supplied ...

         move.l   #RXREMCON,d2         ; "remove value" command
         bra.s    BF8010

* ===========================      ADDLIB     ==========================
* Adds a function library or function host to the global Library List.
* Usage: ADDLIB(library,priority,[offset,version])
BIFaddlib
         movea.l  a0,a3                ; save name
         move.l   #RXADDFH,d2          ; "add host" command
         moveq    #0,d4                ; clear offset
         move.l   d3,d5                ; save [version]

         ; Convert the priority entry (may be negative).

         lea      -ns_Buff(a1),a0      ; second parameter
         bsr      CVs2i                ; D0=error  D1=value
         bne.s    BF8008               ; error ...

         move.l   d1,d3                ; save priority
         moveq    #127,d0              ; maximum priority
         cmp.l    d0,d3                ; too high?
         bgt.s    BF8008               ; yes
         neg.l    d0                   ; minimum priority
         cmp.l    d0,d3                ; too low?
         ble.s    BF8008               ; yes

         ; Check if this is a function host entry (two parameters).

         cmpi.b   #2,d7                ; function host?
         beq.s    1$                   ; yes

         ; Convert the offset entry (which is negative).

         move.l   a2,d2                ; parameter supplied?
         beq.s    BF8008               ; no ... error
         lea      -ns_Buff(a2),a0      ; third parameter
         bsr      CVs2i                ; D0=error  D1=value
         bne.s    BF8008               ; error ...
         move.l   d1,d4                ; save offset
         move.l   #RXADDLIB,d2         ; "add library" command

         ; Prepare the message packet.

1$:      movea.l  a3,a0
         movea.l  ev_GlobalPtr(a4),a2  ; global pointer
         movea.l  gn_MsgPkt(a2),a3     ; message packet
         movem.l  d3/d4/d5,ARG1(a3)    ; priority/offset/version
         bra.s    BF8010

* ===========================      REMLIB     ==========================
* Removes a function library from the global Library List.
* Usage: REMLIB(library)
BIFremlib
         movea.l  gn_MsgPkt(a2),a3     ; message packet
         move.l   #RXREMLIB,d2         ; action code

         ; Send the packet, then process the return codes ...

BF8010   move.l   d2,ACTION(a3)        ; action code
         move.l   a0,ARG0(a3)          ; name string

         ; Fill in the (undocumented) timeout count ...

         moveq    #2,d0                ; default timeout
         move.l   d0,RESULT1(a3)       ; install it

         ; Send the packet to the "REXX" port.

         movea.l  a2,a0
         movea.l  rl_REXX(a6),a1       ; "REXX" port
         addq.w   #ns_Buff,a1          ; string pointer
         move.l   a3,d0                ; packet
         bsr      SendRexxMsg          ; D0=error
         move.l   d0,d6                ; packet sent ok?
         bne.s    1$                   ; no

         ; Check the return codes and set the boolean return.

         tst.l    RESULT1(a3)          ; error?
         seq      d3                   ; ... set flag
         beq.s    1$                   ; no error
         move.l   RESULT2(a3),d6       ; secondary error

1$:      bra      BFBool               ; boolean return

* ===========================     GETCLIP     ==========================
* Retrieves the specified value string from the global Clip List.
* The name match is case-sensitive.
* Usage: GETCLIP(name)
BIFgetclip
         moveq    #0,d0                ; lock all resources
         CALLSYS  LockRexxBase         ; (registers preserved)

         ; Look for the name node in the Clip List

         movea.l  a0,a1                ; name string
         lea      rl_ClipList(a6),a0   ; list header
         moveq    #RRT_ANY,d0          ; any node
         CALLSYS  FindRsrcNode         ; D0=A0=node
         beq.s    1$                   ; not found ... null return

         ; Copy the value string to a temporary argstring ...

         movea.l  CLVALUE(a0),a0       ; value string
         moveq    #0,d0                ; clear length
         move.w   (ra_Length-ra_Buff)(a0),d0
         CALLSYS  CreateArgstring      ; D0=A0=argstring
         beq.s    2$                   ; failure ...

         ; Convert to internal memory and release the temporary ...

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; argstring
         bsr      CVarg2s              ; D0=A0=string
         bra.s    3$                   ; all ok

1$:      movea.l  rl_NULL(a6),a0       ; null return
         bra.s    3$

2$:      moveq    #ERR10_003,d6        ; allocation failure

         ; Release the resource lock ... registers preserved.

3$:      moveq    #0,d0                ; all resources
         jmp      _LVOUnlockRexxBase(a6)
