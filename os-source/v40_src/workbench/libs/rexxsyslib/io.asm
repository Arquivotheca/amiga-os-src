* == io.asm ============================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Input/Output and DOS-related routines.

* ==========================     FindTrace     =========================
* Locates the tracing output stream.  The stream is the logical STDERR if
* it has been defined, and STDOUT otherwise.
* Registers:      A0 -- global pointer
* Return:         D0 -- IoBuff pointer or 0
*                 A0 -- same
*                 A1 -- logical name
FindTrace
         move.l   a0,-(sp)             ; save global

         ; Look for the STDERR stream

         movea.l  rl_STDERR(a6),a1
         bsr.s    FindLogical          ; D0=A0=IoBuff A1=logical
         bne.s    1$                   ; ... found

         ; Look for the STDOUT stream ...

         movea.l  (sp),a0              ; restore global
         move.l   rl_STDOUT(a6),a1     ; fall through
         bsr.s    FindLogical

1$:      addq.w   #4,sp                ; restore stack
         rts

* =======================     FindConsoleHandler     ===================
* Finds an interactive handler associated with a logical name.
* Registers:      A0 -- global data pointer
*                 A1 -- logical name (string structure)
* Return:         D0 -- Handler processid or 0
FindConsoleHandler
         bsr.s    FindLogical          ; D0=A0=IoBuff
         beq.s    1$                   ; ... not found
         move.l   iobDFH(a0),d0        ; a filehandle?
         beq.s    1$                   ; no
         lsl.l    #2,d0
         movea.l  d0,a1                ; FileHandle
         move.l   fh_Interactive(a1),d0; interactive?
         beq.s    1$                   ; no
         move.l   fh_Type(a1),d0       ; ... load handler

1$:      rts

* ========================      FindLogical     ========================
* Searches for the specified logical name in a program's FileList.
* Registers:      A0 -- global data pointer
*                 A1 -- logical name (string structure)
* Return:         D0 -- IoBuff node or 0
*                 A0 -- same
*                 A1 -- logical name
*                 D1 -- default error code
FindLogical
         move.l   a1,-(sp)             ; push logical name
         pea      ERROR_OBJECT_NOT_FOUND

         ; Look for the logical file.

         lea      FILELIST(a0),a0      ; the FileList
         addq.w   #ns_Buff,a1          ; string pointer
         moveq    #RRT_ANY,d0          ; any node
         CALLSYS  FindRsrcNode         ; D0=A0=node

         movem.l  (sp)+,d1/a1          ; pop error/logical name
         rts

* ========================     PullString     ==========================
* Prompts for and reads a string from the specified logical file, using
* the global temporary buffer.
* Registers:      A0 -- global pointer
*                 A1 -- logical name
*                 D0 -- prompt string
* Return:         D0 -- length or -1
*                 A1 -- buffer (if any characters)
PullString
         movem.l  a2/a3,-(sp)
         move.l   gn_TBuff(a0),a3      ; buffer area
         move.l   d0,a2                ; save prompt

         ; Find the logical name in the global list.

         bsr.s    FindLogical          ; D0=A0=IoBuff
         beq.s    2$                   ; ... not found

         ; Check whether to prompt ...

         exg      a0,a2                ; prompt=>A0, IoBuff=>A2
         move.l   iobDFH(a2),d1        ; a filehandle?
         beq.s    1$                   ; no
         lsl.l    #2,d1
         movea.l  d1,a1                ; filehandle address
         tst.l    fh_Interactive(a1)   ; interactive?
         beq.s    1$                   ; no

         ; Display the prompt ...

         CALLSYS  Strlen               ; D0=length (A0 preserved)
         movea.l  a0,a1                ; prompt string
         movea.l  a2,a0                ; restore IoBuff
         CALLSYS  WriteF

         ; Read the string into the global work buffer.

1$:      movea.l  a2,a0                ; IoBuff
         movea.l  a3,a1                ; global work buffer
         moveq    #0,d0                ; clear length
         move.w   (ra_Length-ra_Buff)(a3),d0
         CALLSYS  ReadStr              ; D0=length  A1=buffer

2$:      movem.l  (sp)+,a2/a3
         rts

* ============================     OpenF     ===========================
* Allocates a file buffer and opens a DOS file or a "string file".
* Logical names are required to be unique in the list.
* Registers:   A0 -- list header
*              A1 -- filename/filehandle/argstring
*              D0 -- open mode
*              D1 -- logical name
* Return:      D0 -- IoBuff pointer or 0
*              A0 -- same
OpenF
         movem.l  d2/d3/a2/a3,-(sp)
         movea.l  a0,a2                ; save header
         movea.l  a1,a3                ; save filename
         move.b   d0,d3                ; save mode

         ; Check that the logical name hasn't already been used ...

         move.l   d1,d2                ; a name?
         beq.s    1$                   ; no
         movea.l  d2,a1                ; logical name
         moveq    #RRT_ANY,d0          ; any node
         CALLSYS  FindRsrcNode         ; D0=A0=node
         bne.s    6$                   ; ... error

         ; Allocate the IoBuff structure as a resource node.

1$:      movea.l  a2,a0                ; list header
         movea.l  d2,a1                ; logical name
         move.l   #iobSIZEOF,d0
         CALLSYS  AddRsrcNode          ; D0=A0=node
         beq.s    6$                   ; ... failure

         ; Fill in the "auto-delete" function using the 'CloseF' entry.

         movea.l  a0,a2                ; save IoBuff
         move.w   #_LVOCloseF,rr_Func(a2)
         move.l   a6,rr_Base(a2)       ; library base

         lea      iobArea(a2),a1       ; buffer area
         move.l   a1,iobRpt(a2)        ; read/write pointer
         move.l   #RXBUFFSZ,iobBct(a2) ; buffer size

         move.l   a3,d0                ; filename or filehandle
         move.b   d3,IOBMODE(a2)       ; open mode
         bmi.s    4$                   ; ... a filehandle
         bne.s    2$                   ; ... a filename

         ; A "string file" -- no open required.

         move.l   a3,iobRpt(a2)        ; string buffer
         move.w   (ra_Length-ra_Buff)(a3),(iobRct+2)(a2)
         bra.s    7$

         ; A DOS filename ... determine the opening mode.

2$:      movea.l  #MODE_OLDFILE,a1     ; existing file
         cmpi.b   #RXIO_WRITE,d3       ; write mode?
         bne.s    3$                   ; no
         addq.w   #MODE_NEWFILE-MODE_OLDFILE,a1

3$:      movea.l  a3,a0                ; filename
         bsr      DOSOpen              ; D0=filehandle

         ; Install the filehandle and check for 'append' processing.

4$:      move.l   d0,iobDFH(a2)        ; save filehandle
         beq.s    5$                   ; error

         subq.b   #RXIO_APPEND,d3      ; append mode?
         bne.s    7$                   ; no
         movea.l  a2,a0                ; IoBuff
         moveq    #0,d0
         moveq    #RXIO_END,d1         ; end anchor
         CALLSYS  SeekF                ; ... seek to end
         bra.s    7$

         ; Error in opening file ... release IoBuff and clear pointer.

5$:      movea.l  a2,a0                ; IoBuff structure
         CALLSYS  CloseF               ; release it

6$:      suba.l   a2,a2                ; clear IoBuff

7$:      move.l   a2,d0                ; set CCR
         movea.l  d0,a0
         movem.l  (sp)+,d2/d3/a2/a3
         rts

* ===========================     CloseF     ===========================
* Releases any resources associated with an IoBuff structure.  This routine
* is called automatically when an IoBuff node is released.
* Registers:   A0 -- IoBuff pointer
CloseF
         movem.l  a2/a6,-(sp)
         movea.l  a0,a2                ; save node
         tst.l    rr_Base(a2)          ; "auto-delete"?
         beq.s    1$                   ; no
         CALLSYS  RemRsrcNode          ; unlink the node
         bra.s    3$

         ; Check if a filehandle is to be released.

1$:      movea.l  rl_DOSBase(a6),a6
         tst.b    IOBMODE(a2)          ; inherited?
         bmi.s    2$                   ; yes
         move.l   iobDFH(a2),d1        ; a filehandle?
         beq.s    2$                   ; no
         CALLSYS  Close

         ; Check if a lock is to be released.

2$:      move.l   iobLock(a2),d1       ; a lock?
         beq.s    3$                   ; no
         CALLSYS  UnLock

3$:      movem.l  (sp)+,a2/a6
         rts

* ===========================     ReadStr     ==========================
* Reads a string from the specified IoBuff.  The string is terminated when
* a 'newline' is found or the maximum number of characters have been read.
* Registers:      A0 -- IoBuff pointer
*                 A1 -- buffer pointer
*                 D0 -- maximum length (65535 maximum)
* Return:         D0 -- characters read
*                 A1 -- buffer pointer
ReadStr
         movem.l  d2/a1-a3,-(sp)
         movea.l  a0,a2                ; IoBuff
         move.l   a1,a3                ; buffer
         move.l   d0,d2                ; maximum length
         bra.s    2$                   ; jump in

         ; Read characters from the file ...

1$:      movea.l  a2,a0                ; IoBuff
         CALLSYS  ReadF                ; D0=character
         ble.s    3$                   ; ... done
         cmpi.b   #NEWLINE,d0          ; a 'newline'?
         beq.s    3$                   ; yes
         move.b   d0,(a3)+
2$:      dbf      d2,1$                ; loop back

3$:      move.l   a3,d0                ; end of buffer
         movem.l  (sp)+,d2/a1-a3
         sub.l    a1,d0                ; final length
         rts

* ==========================      WriteF     ===========================
* Writes characters to a file specified by an IoBuff pointer.
* Registers:   A0 -- IoBuff
*              A1 -- buffer area
*              D0 -- length
* Return:      D0 -- characters written or -1
*              D1 -- secondary return
WriteF
*         add.l    d0,IOBPOS(a0)        ; advance position
         movea.l  iobDFH(a0),a0        ; filehandle
         jmp      _LVODOSWrite(a6)     ; D0=actual length D1=secondary

* ============================     ReadF     ===========================
* Reads the next character from the file specified by the IoBuff pointer.
* Registers:      A0 -- IoBuff pointer
* Return:         D0 -- character or -1
ReadF
         move.l   iobRct(a0),d0        ; any characters in buffer?
         bgt.s    2$                   ; yes ...

         ; Buffer empty ... attempt to refill it.

         move.l   iobDFH(a0),d1        ; a filehandle?
         beq.s    1$                   ; no
         move.l   a0,-(sp)             ; push IoBuff
         lea      iobArea(a0),a1       ; buffer area
         move.l   a1,iobRpt(a0)        ; reset read pointer
         move.l   iobBct(a0),d0        ; buffer size
         move.l   d1,a0                ; filehandle
         CALLSYS  DOSRead              ; D0=actual length D1=secondary
         move.l   (sp)+,a0             ; pop IoBuff
         bmi.s    3$                   ; ... error

         ; Update the character count and check for EOF ...

1$:      move.l   d0,iobRct(a0)        ; install character count
         beq.s    4$                   ; ... EOF

         ; Character available ... update buffer data.

2$:      subq.l   #1,iobRct(a0)        ; decrement count
         movea.l  iobRpt(a0),a1        ; character pointer
         addq.l   #1,iobRpt(a0)        ; advance pointer
*         addq.l   #1,IOBPOS(a0)        ; update position
         moveq    #0,d0                ; clear return
         move.b   (a1),d0              ; load character

3$:      tst.l    d0                   ; set CCR
         rts

         ; End-of-file ... set flag and return EOF code.

4$:      st       IOBEOF(a0)           ; set/clear EOF flag
         moveq    #EOFCHAR,d0          ; ... return EOF
         rts

* ===========================      QueueF     ==========================
* Queues a string to the stream specified by the IoBuff.
* Registers:   A0 -- IoBuff pointer
*              A1 -- buffer area
*              D0 -- length
* Return:      D0 -- character count or -1
QueueF:
         move.l   #ACTION_QUEUE,d1
         bra.s    SF001

* ===========================      StackF     ==========================
* Stacks a string to the stream specified by the IoBuff.
* Registers:   A0 -- IoBuff pointer
*              A1 -- buffer area
*              D0 -- length
* Return:      D0 -- character count or -1
*              D1 -- error code
StackF:
         move.l   #ACTION_STACK,d1

SF001:   movem.l  d2-d4/a2,-(sp)
         move.l   a1,d2                ; save buffer
         move.l   d0,d3                ; save length
         move.l   d1,d4                ; save action

         ; Make sure we have a filehandle, and that it's not NIL:

         move.l   iobDFH(a0),d0        ; a filehandle?
         beq.s    2$                   ; no ... error
         lsl.l    #2,d0                ; convert from BPTR
         movea.l  d0,a2

         ; Create and initialize a DOS standard packet.

         CALLSYS  CreateDOSPkt         ; D0=A0=packet
         beq.s    2$                   ; failure ...

         ; Install arguments in the packet.

         move.l   fh_Arg1(a2),d1
         movem.l  d1/d2/d3,(sp_Pkt+dp_Arg1)(a0)
         move.l   d4,(sp_Pkt+dp_Type)(a0)

         ; Check whether it's a NIL: filehandle.

         exg      a0,a2                ; filehandle=>A0, message=>A2
         move.l   fh_Type(a0),d0       ; a handler processid?
         beq.s    1$                   ; no ... NIL: filehandle

         ; Send the packet to the handler's processid.

         movea.l  d0,a0                ; handler processid
         movea.l  a2,a1                ; message packet
         CALLSYS  SendDOSPkt           ; A0=our processid

         ; Wait at our processid

         moveq    #0,d0                ; wait mask
         CALLSYS  WaitDOSPkt           ; D0=A0=message

         ; Get the returned length and release the message packet.

1$:      movem.l  (sp_Pkt+dp_Res1)(a2),d2/d3
         movea.l  a2,a0                ; standard packet
         CALLSYS  DeleteDOSPkt         ; release it
         bra.s    3$

         ; Error return

2$:      moveq    #-1,d2               ; error flag
         moveq    #0,d3                ; no code

3$:      move.l   d3,d1                ; error code
         move.l   d2,d0                ; actual length
         movem.l  (sp)+,d2-d4/a2
         rts

* ===========================     ExistF     ===========================
* Checks whether a file exists and returns a boolean flag.
* Registers:   A0 -- filename
* Return:      D0 -- boolean
ExistF
         moveq    #RXIO_READ,d0        ; use read mode
         bsr.s    LockF                ; D0=lock D1=secondary
         move.l   d0,-(sp)             ; success?
         beq.s    1$                   ; no
         bsr.s    UnlockF              ; release lock
         bra.s    2$

         ; Lock failed ... check whether the object was in use.

1$:      cmpi.w   #ERROR_OBJECT_IN_USE,d1
         bne.s    2$                   ; ... no object
         not.l    (sp)                 ; set flag

2$:      move.l   (sp)+,d0             ; set CCR
         rts

* =============================     SeekF     ==========================
* Moves to the specified position in a file, and returns the new position.
* Registers:      A0 -- IoBuff
*                 D0 -- offset
*                 D1 -- position anchor
* Return:         D0 -- new position or -1
SeekF
         lea      iobArea(a0),a1       ; buffer area
         move.l   a1,iobRpt(a0)        ; reset read/write pointer
         clr.b    IOBEOF(a0)           ; clear EOF

         ; Check for offset from current ... adjust for buffer count.

         movea.l  d0,a1                ; offset
         move.l   d1,d0                ; offset from 'current'?
         bne.s    1$                   ; no
         suba.l   iobRct(a0),a1        ; adjust offset
1$:      clr.l    iobRct(a0)           ; clear read count

         ; Seek to the specified position.

         movea.l  iobDFH(a0),a0        ; filehandle
         move.l   a0,-(sp)             ; push filehandle
         bsr.s    DOSSeek              ; D0=error D1=secondary
         movea.l  (sp)+,a0             ; pop filehandle

         ; Get the current position as an offset from the beginning.

         suba.l   a1,a1                ; clear offset
         moveq    #OFFSET_CURRENT,d0   ; current position

* ===========================     DOSSeek     ==========================
* Seeks to a new position in a file.
* Registers:      A0 -- filehandle
*                 A1 -- offset
*                 D0 -- seek anchor
* Return:         D0 -- position or -1
*                 D1 -- secondary result
DOSSeek
         move.w   #_LVOSeek,d1         ; function offset
         bra.s    DR001

* ===========================     LockF     ============================
* Checks whether a file can be locked in the specified mode.
* Registers:   A0 -- filename
*              D0 -- access mode
* Return:      D0 -- lock
*              D1 -- secondary return
LockF
         moveq    #ACCESS_READ,d1      ; read ("shared") mode
         subq.b   #RXIO_WRITE,d0       ; write access?
         bcs.s    1$                   ; no
         moveq    #ACCESS_WRITE,d1     ; write mode

1$:      movea.l  d1,a1                ; mode
         move.w   #_LVOLock,d1         ; function offset
         bra.s    DR001

* ===========================     UnlockF     ==========================
* Releases a DOS lock.
* Registers:   D0 -- lock
UnlockF
         movea.l  d0,a0
         move.w   #_LVOUnLock,d1       ; function offset
         bra.s    DR001

* ===========================     DOSParent     ========================
* Returns a lock to the parent of a file or directory.
* Registers:   D0 -- lock
DOSParent
         movea.l  d0,a0
         move.w   #_LVOParentDir,d1    ; function offset
         bra.s    DR001

* ===========================     DOSOpen     ==========================
* Opens a file.
* Registers:   A0 -- name
*              A1 -- mode
DOSOpen
         move.w   #_LVOOpen,d1         ; function offset
         bra.s    DR001

* ========================     DOSWaitChar     =========================
* Waits for a character at the specified DOS filehandle.
* Registers:   A0 -- filehandle
*              D0 -- time interval
* Return:      D0 -- boolean
*              D1 -- secondary result
DOSWaitChar
         movea.l  d0,a1                ; interval
         move.w   #_LVOWaitForChar,d1  ; function offset
         bra.s    DR001

* ==========================     DOSWrite     ==========================
* Writes a buffer of characters to a DOS filehandle.
* Registers:   A0 -- filehandle
*              A1 -- buffer
*              D0 -- length
* Return:      D0 -- characters written or -1
*              D1 -- secondary result
DOSWrite:
         move.w   #_LVOWrite,d1        ; function offset
         bra.s    DR001

* ==========================     DOSRead     ===========================
* Reads a buffer of characters from a DOS filehandle.
* Registers:   A0 -- DOS filehandle
*              A1 -- buffer pointer
*              D0 -- buffer length
* Return:      D0 -- characters read or -1
*              D1 -- secondary result
DOSRead
         move.w   #_LVORead,d1

DR001:   movem.l  d2/d3/a4/a6,-(sp)
         exg      a0,d1                ; filehandle=>D1 , offset=>A0
         move.l   a1,d2                ; buffer
         move.l   d0,d3                ; length
         movea.l  rl_SysBase(a6),a1    ; EXEC base
         movea.l  ThisTask(a1),a4      ; task ID

         ; Call the DOS library function.

         movea.l  rl_DOSBase(a6),a6    ; DOS base
         jsr      0(a6,a0.w)           ; D0=characters

         ; Get the secondary return code from the process structure.

         move.l   pr_Result2(a4),d1    ; secondary return
         tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2/d3/a4/a6
         rts

* ==========================     DOSExamine     ========================
* Returns a lock to the parent of a file or directory.
* Registers:   D0 -- lock
*              A1 -- FileInfoBlock
DOSExamine
         movea.l  d0,a0
         move.w   #_LVOExamine,d1      ; function offset
         bra.s    DR001

* =========================     SysTime     ============================
* Checks the 'timer valid' flag and updates the DateTime structure.
* Registers:      A0 -- storage environment
SysTime
         move.w   #_LVODateStamp,d1    ; function offset
         bset     #EFB_TIMER,EVFLAGS(a0)
         lea      ev_Date(a0),a0       ; DateTime structure
         beq.s    DR001                ; ... not valid
         rts

* ==========================     DOSCommand     ========================
* Issues a command to AmigaDOS from the current task.
* Registers:      A0 -- command string
*                 A1 -- output filehandle
*                 D0 -- input filehandle
* Return:         D0 -- boolean
*                 D1 -- return code
DOSCommand
         exg      a1,d0                ; input=>A1, output=>D0
         tst.l    d0                   ; output stream?
         bne.s    1$                   ; yes
         move.l   rl_NIL(a6),d0        ; use default NIL: filehandle

         ; Check the version level of the DOS library.

1$:      move.l   a0,d1                ; command string
         movea.l  rl_DOSBase(a6),a0    ; DOS base
         cmpi.w   #36,LIB_VERSION(a0)  ; version >= 36?
         bcc.s    DOSSystem            ; yes

         ; Version < 36 ... use limited Execute() function.

         movea.l  d1,a0                ; command string
         suba.l   a1,a1                ; clear input stream
         move.w   #_LVOExecute,d1      ; load offset
         bra.s    DR001

         ; Newer library available ... use advanced System() function.

DOSSystem
         movem.l  d2/d7/a6,-(sp)
         movea.l  a0,a6                ; DOS base
         move.l   sp,d7                ; save stack

         ; Create the tag array on the stack.

         clr.l    -(sp)                ; (TAG_DONE)

         move.l   d0,-(sp)             ; output stream
         pea      SYS_Output           ; output tag

         move.l   a1,-(sp)             ; input stream
         pea      SYS_Input            ; input tag

         moveq    #-1,d0
         move.l   d0,-(sp)             ; TRUE flag
         pea      SYS_UserShell        ; user shell

         ; Call the DOS (V36) System() function.

         move.l   sp,d2                ; tag pointer
         CALLSYS  System               ; D0=return code
         movea.l  d7,sp                ; restore stack

         ; Decode the return ... -1 implies failure.

         move.l   d0,d1                ; return code
         addq.l   #1,d0                ; failure?
         beq.s    1$                   ; yes
         moveq    #-1,d0               ; success

1$:      movem.l  (sp)+,d2/d7/a6
         rts

* =========================     FindDevice     =========================
* Checks the DOS DeviceList for the specified device.
* Registers:      A0 -- device name (null-terminated)
*                 D0 -- device type
* Rreturn:        D0 -- device node or 0
*                 A0 -- same
FindDevice
         movem.l  d2-d4/a2/a3,-(sp)
         movea.l  a0,a2                ; save search string
         move.l   d0,d4                ; save type

         CALLSYS  Strlen               ; D0=length
         move.l   d0,d2                ; save it

         movea.l  rl_DOSBase(a6),a0    ; DOS library base
         movea.l  dl_Root(a0),a0       ; pointer to root node
         movea.l  rn_Info(a0),a0       ; BPTR to Info structure
         adda.l   a0,a0
         adda.l   a0,a0
         move.l   di_DevInfo(a0),d3    ; BPTR to DevList

         ; Scan the device list looking for a matching entry.

1$:      move.l   d3,d0                ; a node?
         beq.s    3$                   ; no ... all done

         lsl.l    #2,d3                ; convert to address
         movea.l  d3,a3                ; device node
         move.l   (a3),d3              ; (dl_Next) next node
         tst.l    d4                   ; check type?
         bmi.s    2$                   ; no
         cmp.l    dl_Type(a3),d4       ; correct type?
         bne.s    1$                   ; no ... keep looking

         ; Compare the device name with the search string (in UPPERcase).

2$:      move.l   d2,d0                ; length
         movea.l  a2,a0                ; search string
         movea.l  dl_Name(a3),a1       ; BSTR to device name
         adda.l   a1,a1
         adda.l   a1,a1                ; real address
         cmp.b    (a1)+,d0             ; lengths match?
         bne.s    1$                   ; no ... loop back
         CALLSYS  StrcmpU              ; D0=test
         bne.s    1$                   ; ... no match
         move.l   a3,d0                ; set CCR

3$:      movea.l  d0,a0                ; return node
         movem.l  (sp)+,d2-d4/a2/a3
         rts

* =======================     CreateDOSPkt     =========================
* Allocates and initializes a DOS StandardPacket.
* Return:      D0 -- packet
*              A0 -- same
CreateDOSPkt
         moveq    #sp_SIZEOF,d0        ; packet length
         bsr      GetMem               ; D0=A0=packet
         beq.s    1$                   ; failure ...

         ; Link the message and packet

         lea      (sp_Pkt+dp_Link)(a0),a1 ; start of DOS packet
         move.l   a0,(a1)                 ; backward link
         move.l   a1,LN_NAME(a0)          ; forward link

1$:      rts

* ========================     DeleteDOSPkt     ========================
* Releases a DOS StandardPacket.
* Registers:      A0 -- packet
DeleteDOSPkt
         moveq    #sp_SIZEOF,d0        ; packet length
         bra      GiveMem              ; release it

* =========================      SendDOSPkt     ========================
* Sends a DOS packet to the specified processid.
* Registers:   A0 -- handler processid
*              A1 -- message packet
* Return:      A0 -- task processid
SendDOSPkt
         movem.l  d7/a3/a6,-(sp)
         movea.l  LN_NAME(a1),a3       ; DOS packet
         movea.l  rl_SysBase(a6),a6

         ; Find our processid (message port)

         moveq    #pr_MsgPort,d7       ; processid offset
         add.l    ThisTask(a6),d7      ; ... plus task ID
         move.l   d7,MN_REPLYPORT(a1)  ; install replyport
         move.l   d7,dp_Port(a3)       ; install replyport

         CALLSYS  PutMsg               ; send it

         movea.l  d7,a0                ; our processid
         movem.l  (sp)+,d7/a3/a6
         rts

* =========================      WaitDOSPkt     ========================
* Waits at the specified message port for a message or other signal.
* Registers:   A0 -- message port
*              D0 -- wait mask
* Return:      D0 -- message packet
*              D1 -- signals received
*              A0 -- message packet
WaitDOSPkt
         move.l   a6,-(sp)
         move.l   a0,-(sp)             ; push message port
         movea.l  rl_SysBase(a6),a6

         move.b   MP_SIGBIT(a0),d1     ; port signal
         bset     d1,d0                ; wait mask
         CALLSYS  Wait                 ; D0=signals

         movea.l  (sp)+,a0             ; pop message port
         move.l   d0,-(sp)             ; push signals
         CALLSYS  GetMsg               ; D0=message

         move.l   (sp)+,d1             ; pop signals
         tst.l    d0                   ; set CCR
         movea.l  d0,a0                ; EXEC message
         movea.l  (sp)+,a6
         rts
