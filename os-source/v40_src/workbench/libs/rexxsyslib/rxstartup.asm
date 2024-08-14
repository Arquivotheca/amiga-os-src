* === rxstartup.asm ====================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* The startup code for the REXX interpreter.  This is the seglist passed
* to "CreateProc" when a new program is invoked.

         ; The fake seglist for the interpreter task begins here.

         CNOP     0,4
         dc.l     8                    ; seglist "length"
rxSegList
         dc.l     0                    ; BPTR to "next block"

         ; Start up code for the interpreter

rxStartUp
         movea.l  _AbsExecBase,a6      ; EXEC base
         movea.l  ThisTask(a6),a4      ; our task ID

         ; Wait for the wake-up message, which should already be here.
         ; The message carries the RexxTask pointer and library base
         ; in the RESULT1/RESULT2 fields.
         ; N.B. The processid message port is hereafter used by DOS.

         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  WaitPort
         lea      pr_MsgPort(a4),a0    ; our processid
         CALLSYS  GetMsg               ; D0=message
         movea.l  d0,a2                ; the "wake-up" message
         move.l   ACTION(a2),d2        ; action code

         ; Load the global data pointer and library base from the packet.

         movem.l  RESULT1(a2),a3/a5    ; RexxTask pointer/library base
         move.l   rl_DOSBase(a5),d5    ; DOS library base

         ; Get an extra open on the math library (68881 support)

         lea      IEEELib(pc),a1       ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary          ; D0=node

         ; Install default values from the client's process structure.

         moveq    #0,d3                ; clear CLI
         move.l   rt_ClientID(a3),d1   ; client task ID?
         beq.s    2$                   ; no
         movea.l  d1,a1                ; task ID
         cmpi.b   #NT_PROCESS,LN_TYPE(a1)
         bne.s    2$                   ; not a process ...
         move.l   pr_CLI(a1),d3        ; a CLI structure?

         ; Install the input and output streams

         btst     #RXFB_NOIO,d2        ; suppress I/O?
         bne.s    1$                   ; yes
         movem.l  pr_CIS(a1),d1/a0     ; client's input/output streams
         movem.l  d1/a0,pr_CIS(a4)     ; ... install values

         ; Duplicate the directory lock ...

1$:      move.l   pr_CurrentDir(a1),d1 ; a directory lock
         exg      d5,a6
         CALLSYS  DupLock              ; D0=lock
         exg      d5,a6
         move.l   d0,pr_CurrentDir(a4) ; install it

         ; Check whether default streams were provided with the message.

2$:      move.l   rm_Stdin(a2),d0      ; input stream?
         beq.s    3$                   ; no
         move.l   d0,pr_CIS(a4)        ; ... install it

3$:      move.l   rm_Stdout(a2),d1     ; output stream?
         beq.s    4$                   ; no
         move.l   d1,pr_COS(a4)        ; ... install it

         ; Check that the directory name is up-to-date ...

4$:      exg      a5,a6                ; EXEC=>A5 , REXX=>A6
         tst.l    d3                   ; client CLI?
         bne.s    5$                   ; yes
         bsr      SetDirName           ; no ... update name

         ; Call the interpreter.

5$:      movea.l  a3,a0                ; RexxTask structure
         movea.l  a2,a1                ; message packet
         CALLSYS  Rexx                 ; D0=error level  A0=result
         exg      a5,a6                ; REXX=>A5 , EXEC=>A6

         ; Fill in the result fields and check for errors

         movea.l  a2,a1                ; message packet
         movem.l  d0/a0,RESULT1(a2)    ; error level/reason code
         tst.l    d0                   ; an error?
         beq.s    6$                   ; no ...

         ; An error occurred in the interpreter.  Determine the appropriate
         ; action to take:
         ;   (1) If the program wasn't found, the message packet may be
         ;       forwarded to another host.
         ;   (2) If an execution error occurred, fill in the error level
         ;       and return the packet.

         moveq    #ERR10_001,d0        ; "program not found"
         cmp.l    a0,d0                ; not found?
         bne.s    6$                   ; no

         ; Attempt to forward the message, if a PassPort was supplied.

         move.l   rm_PassPort(a2),d0   ; a forwarding port?
         beq.s    6$                   ; no ...
         movea.l  d0,a0
         CALLSYS  PutMsg               ; pass it on
         bra.s    7$

         ; Return the message packet ...

6$:      CALLSYS  ReplyMsg             ; send it back

         ; Close the IEEE library (68881 support)

7$:      movea.l  rl_IeeeDPBase(a5),a1 ; IEEE library base
         CALLSYS  CloseLibrary         ; close it

         ; Release the CLI structure ...

         move.l   pr_CLI(a4),d0        ; BPTR to CLI structure
         clr.l    pr_CLI(a4)           ; clear slot
         movea.l  a5,a6                ; REXX base
         CALLSYS  DeleteCLI            ; release it

         ; Release the current directory lock.

         move.l   pr_CurrentDir(a4),d0 ; directory lock
         clr.l    pr_CurrentDir(a4)    ; clear slot
         bsr      UnlockF              ; release it

         ; Set the 'task completed' flag ...

         bset     #RTFB_CLOSE,rt_Flags(a3)

         ; Exit to DOS for process cleanup.

         rts

* =========================     InitGlobal    ==========================
* Initializes the global data structure.
* Registers:      A0 -- global data pointer
* Return:         D0 -- error code
InitGlobal
         movem.l  d2/d3/a2-a4,-(sp)
         movea.l  a0,a3

         ; Initialize the global list headers.

         lea      rt_Header1(a3),a0    ; first list header
         moveq    #(rt_Header5-rt_Header1)/LH_SIZE,d2

1$:      CALLSYS  InitList             ; A0=header
         lea      LH_SIZE(a0),a0       ; advance pointer
         dbf      d2,1$                ; loop back

         ; Initialize the global replyport (part of the RexxTask structure).

         lea      rt_MsgPort(a3),a0    ; message port
         suba.l   a1,a1                ; no name (private port)
         CALLSYS  InitPort             ; D0=signal  A1=port
         move.b   d0,rt_SigBit(a3)     ; save signal bit
         movea.l  MP_SIGTASK(a1),a4    ; this task

         ; Allocate the global message packet.

         movea.l  a1,a0                ; replyport
         CALLSYS  CreateRexxMsg        ; D0=A0=packet
         beq      5$                   ; ... allocation error
         move.l   a0,gn_MsgPkt(a3)     ; save packet

         ; Tag the packet with the "REXX" string.

         move.l   rl_REXX(a6),a1       ; "REXX" string
         addq.w   #ns_Buff,a1          ; offset to string
         move.l   a1,LN_NAME(a0)       ; install it

         ; Set the global file extension and host address.

         move.l   a1,gn_FileExt(a3)    ; set to "REXX"
         move.l   a1,gn_HostAddr(a3)   ; set to "REXX"

         ; Install the global pointer and library base.

         movem.l  a3/a6,rm_TaskBlock(a0)

         ; Allocate the global work buffer as an argstring.

         moveq    #TEMPBUFF>>3,d0      ; 1/8 buffer size
         lsl.w    #3,d0                ; buffer size
         bsr      GetWorkBuffer        ; D0=A0=buffer
         move.l   d0,gn_TBuff(a3)      ; install it
         beq      5$                   ; failure ...

         IFNE     TEMPBUFF-(TEMPBUFF/8)*8
         FAIL     'TEMPBUFF must be a multiple of 8!'
         ENDC

         ; Create the STDIN and STDOUT streams, cloning the filehandles
         ; if possible.

         movem.l  rl_STDIN(a6),d2/d3   ; input/output logical names
         addq.l   #ns_Buff,d2          ; offset to string
         addq.l   #ns_Buff,d3          ; offset to string

         ; Check whether the output stream is interactive.

         move.l   pr_COS(a4),d1        ; output stream?
         beq.s    3$                   ; no ...
         lsl.l    #2,d1
         movea.l  d1,a0                ; FileHandle structure
         tst.l    fh_Interactive(a0)   ; interactive stream?
         beq.s    2$                   ; no ...

         ; An interactive output stream ... install as the current
         ; console handler and attempt to get an open on "*".

         move.l   fh_Type(a0),pr_ConsoleTask(a4)

         lea      FILELIST(a3),a0      ; File List header
         lea      Star(pc),a1          ; "*" name
         moveq    #RXIO_WRITE,d0       ; "write" mode
         move.l   d3,d1                ; output name
         CALLSYS  OpenF                ; D0=A0=IoBuff
         bne.s    4$                   ; success ...

         ; Open on output failed ... fall back to original filehandle.

2$:      lea      FILELIST(a3),a0      ; File List header
         movea.l  pr_COS(a4),a1        ; output stream
         moveq    #RXIO_EXIST,d0       ; existing filehandle
         move.l   d3,d1                ; output name
         CALLSYS  OpenF                ; D0=A0=IoBuff

         ; Now check whether the input stream is interactive.

3$:      move.l   pr_CIS(a4),d1        ; input stream?
         beq.s    41$                  ; no ...
         lsl.l    #2,d1
         movea.l  d1,a0                ; FileHandle structure
         tst.l    fh_Interactive(a0)   ; interactive stream?
         beq.s    4$                   ; no ...

         ; An interactive input stream ... install it as the current
         ; console handler and attempt to get an open on "*".

         move.l   fh_Type(a0),pr_ConsoleTask(a4)

         lea      FILELIST(a3),a0      ; File List header
         lea      Star(pc),a1          ; "*" name
         moveq    #RXIO_READ,d0        ; "read" mode
         move.l   d2,d1                ; input name
         CALLSYS  OpenF                ; D0=A0=IoBuff
         bne.s    41$                  ; success ...

         ; No open on input stream ... fall back to original filehandle.

4$:      lea      FILELIST(a3),a0      ; File List header
         movea.l  pr_CIS(a4),a1        ; input stream
         moveq    #RXIO_EXIST,d0       ; existing filehandle
         move.l   d2,d1                ; input name
         CALLSYS  OpenF                ; D0=A0=IoBuff
41$:
         ; Inherit values from the CLI structure ...

         move.l   pr_CLI(a4),d0        ; CLI structure?
         beq.s    5$                   ; no
         lsl.l    #2,d0
         movea.l  d0,a0
         move.l   cli_FailLevel(a0),ev_TraceLim(a3)
         moveq    #0,d0                ; clear error
         bra.s    6$

5$:      moveq    #ERR10_003,d0        ; allocation failure

6$:      movem.l  (sp)+,d2/d3/a2-a4
         rts

* ========================     GetWorkBuffer     =======================
* Allocates the work buffer as an argstring.
* Registers:      D0 -- size
* Return:         D0 -- argstring
*                 A0 -- same
GetWorkBuffer
         suba.l   a0,a0                ; no string
         jmp      _LVOCreateArgstring(a6)

* ======================     DoubleWorkBuffer     ======================
* Doubles the size of the global work buffer, copying the current contents.
* Registers:      A0 -- global pointer
* Return:         D0 -- new buffer or 0
DoubleWorkBuffer
         movea.l  gn_TBuff(a0),a1      ; current buffer
         move.w   (ra_Length-ra_Buff)(a1),d0
         add.w    d0,d0                ; double size

* ======================     ResizeWorkBuffer     ======================
* Resizes the temporary work buffer and copies the old data.  The new size
* must be greater than the existing buffer size.
* Registers:      A0 -- global pointer
*                 D0 -- new size
* Return:         D0 -- new buffer
*                 A0 -- same
ResizeWorkBuffer
         movem.l  a2/a3,-(sp)
         movea.l  a0,a3                ; global pointer
         movea.l  gn_TBuff(a3),a2      ; current work buffer

         ; Make sure the (word) size is valid.

         cmp.w    (ra_Length-ra_Buff)(a2),d0
         bls.s    3$                   ; ... too small

         ; Allocate the new work buffer.

         bsr.s    GetWorkBuffer        ; D0=A0=buffer
         exg      a0,a2                ; old=>A0, new=>A2
         beq.s    3$                   ; ... failure

         ; New buffer allocated ... copy contents of old.

         move.l   a2,gn_TBuff(a3)      ; install new
         move.w   (ra_Length-ra_Buff)(a0),d1
         movea.l  a0,a1                ; old buffer pointer
         movea.l  a2,a3                ; new buffer pointer
         bra.s    2$                   ; jump in

1$:      move.l   (a1)+,(a3)+          ; longword copy
2$:      subq.w   #4,d1                ; any more?
         bcc.s    1$                   ; yes

         ; Release the old work buffer.

         CALLSYS  DeleteArgstring      ; release old
         bra.s    4$

         ; Failure ... clear return.

3$:      suba.l   a2,a2                ; clear return

4$:      move.l   a2,d0                ; set CCR
         movea.l  a2,a0                ; new buffer or 0
         movem.l  (sp)+,a2/a3
         rts

* ==========================     LocateFile     ========================
* Checks whether a file is a REXX program.  If the filename has the form
* "[dev:dir]name.ext", only that name is checked.  Otherwise, the default
* extension is appended and checked; if that name fails, the original name
* is checked.
* The search is performed for the current directory and for each directory
* in the search path (for which the default is REXX:).  Two passes are
* made; the first considers only mounted volumes, and the second will
* consider all volumes.
* Registers:   A0 -- global pointer
*              A1 -- file name string (null-terminated)
*              D0 -- default extension (null-terminated)
*              D1 -- name buffer
* Return:      D0 -- IoBuff pointer or 0
*              A0 -- same
STACKBF  SET      MAXFNLEN             ; stack buffer
LocateFile
         movem.l  d2-d7/a2-a5,-(sp)
         lea      -STACKBF(sp),sp      ; stack buffer
         movea.l  d0,a5                ; file extension
         movea.l  a0,a3                ; global pointer
         movea.l  gn_TBuff(a3),a2      ; global work area

         moveq    #0,d6                ; mount flag
         moveq    #0,d7                ; search flags

         ; Copy the search path into the temp buffer (no check for length).

         movea.l  rl_RexxDir(a6),a4    ; search path
         movea.l  a2,a0                ; buffer
0$:      move.b   (a4)+,(a0)+          ; copy string
         bne.s    0$                   ; loop back

         movea.l  d1,a0                ; name buffer
         move.l   a0,gn_FileName(a3)   ; resolved filename
         move.l   a1,gn_SrcFile(a3)    ; source filename
         movea.l  rl_SysBase(a6),a4    ; EXEC base
         movea.l  ThisTask(a4),a4      ; our task ID

         ; Copy the file name to the output buffer.

         moveq    #MAXFNLEN-1,d1       ; maximum filename length
         moveq    #0,d2                ; clear 'extension' flag
         moveq    #0,d3                ; clear 'device' flag
         moveq    #'.',d4

1$:      move.b   (a1)+,d0             ; load character
         cmp.b    d4,d0                ; a period?
         seq      d5                   ; set flag
         or.b     d5,d2                ; update extension flag
         cmpi.b   #'/',d0              ; directory separator?
         beq.s    2$                   ; yes
         cmpi.b   #':',d0              ; device separator?
         bne.s    3$                   ; no
         moveq    #-1,d3               ; set device flag
2$:      moveq    #0,d2                ; clear extension flag
3$:      move.b   d0,(a0)+             ; store the character
         dbeq     d1,1$                ; loop back until NULL
         bne      LFNoFile             ; name too long?

         tst.b    d2                   ; extension found?
         bne.s    LFStart              ; yes -- check file

         ; No file extension found ... check for a default.

         tst.b    (a5)                 ; an extension?
         seq      d2                   ; update flag
         beq.s    LFStart              ; no extension

         ; Append the default extension.

         move.b   d4,-1(a0)            ; insert a period
4$:      move.b   (a5)+,(a0)+          ; copy the string
         dbeq     d1,4$                ; loop back
         bne      LFNoFile             ; ... too long

         ; Start with the current directory

LFStart  tst.b    d3                   ; explicit path?
         beq.s    LFAgain              ; no
         move.l   pr_CurrentDir(a4),d0 ; current directory
         clr.l    -(sp)                ; push zero lock
         bra.s    LFCheck              ; jump in

         ; File not found yet ... try searching unmounted volumes.

LFNext   not.b    d7                   ; toggle flag
         and.b    d6,d7                ; any unmounted?
         beq      LFNoFile             ; no ...

         ; Load the search path, beginning with the current directory.

LFAgain  movea.l  a2,a5                ; search path
         moveq    #0,d5                ; clear flag
         clr.b    (sp)                 ; current directory
         bra.s    LFLock               ; jump in

         ; Force search of unmounted volumes

LFMount  moveq    #-1,d6               ; set 'mount' flag

         ; Scan the REXX search path string.

LFScan   tst.b    d5                   ; at end?
         bne.s    LFNext               ; yes
         movea.l  sp,a0                ; buffer area
         moveq    #STACKBF-1,d1        ; maximum length
         moveq    #';',d4              ; separator

1$:      clr.b    (a0)                 ; install a null
         move.b   (a5)+,d0             ; test character
         cmp.b    d4,d0                ; ";" separator?
         beq.s    LFLock               ; yes
         cmpi.b   #',',d0              ; "," separator?
         beq.s    LFLock               ; yes
         move.b   d0,(a0)+             ; install it
         dbeq     d1,1$                ; loop back

         seq      d5                   ; set flag
         bne.s    LFScan               ; too long ...

         ; Get a lock on the search directory.

LFLock   move.l   pr_CurrentDir(a4),d0 ; current directory

         ; Check whether a device was included in the name ...

         movea.l  sp,a1                ; scan pointer
         subq.b   #';'-':',d4          ; ":" device separator

1$:      move.b   (a1)+,d1             ; test character
         beq.s    2$                   ; no device ...
         cmp.b    d4,d1                ; device separator?
         bne.s    1$                   ; no
         clr.b    -(a1)                ; temporary null

         ; See if the device is in the DOS DeviceList ...

         movea.l  sp,a0                ; device name
         moveq    #-1,d0               ; any type
         move.l   a1,-(sp)             ; push pointer
         CALLSYS  FindDevice           ; D0=A0=device node
         movea.l  (sp)+,a1             ; pop pointer
         beq.s    LFScan               ; not found ...

         move.b   d4,(a1)              ; re-install separator
         move.l   dl_Lock(a0),d0       ; test lock

2$:      tst.b    d7                   ; search all?
         bne.s    3$                   ; yes

         ; See if the volume is mounted ...

         bsr.s    IsMounted            ; D0=boolean
         beq.s    LFMount              ; force mount

         ; Acquire a lock on the directory

3$:      movea.l  sp,a0                ; directory name
         moveq    #RXIO_READ,d0        ; read access
         bsr      LockF                ; D0=lock
         beq.s    LFScan               ; not found
         move.l   d0,-(sp)             ; push lock

         ; Save the current directory and install the search directory.

LFCheck  move.l   pr_CurrentDir(a4),-(sp)
         move.l   d0,pr_CurrentDir(a4)
         moveq    #0,d4                ; clear IoBuff

         ; Check first using the resolved filename.

         movea.l  gn_FileName(a3),a1   ; resolved filename
         bclr     #15,d7               ; clear flag
         bra.s    2$                   ; jump in

         ; File not found ... try removing the extension if one was added.

1$:      tst.b    d2                   ; extension added?
         bne.s    3$                   ; no
         bset     #15,d7               ; already tested?
         bne.s    3$                   ; yes
         movea.l  gn_SrcFile(a3),a1    ; original filename

         ; Check if it's a REXX file ... if so, the name is expanded to
         ; the full filename relative to the current path.

2$:      movea.l  gn_FileName(a3),a0   ; name buffer
         bsr.s    IsRexx               ; D0=A0=IoBuff D1=boolean
         move.l   d0,d4                ; found?
         bne.s    3$                   ; yes

         tst.l    d1                   ; file found?
         beq.s    1$                   ; no ... loop back
         moveq    #-1,d3               ; end search

         ; Restore the current directory and release the new lock.

3$:      move.l   (sp)+,pr_CurrentDir(a4)
         move.l   (sp)+,d0             ; pop lock
         bsr      UnlockF              ; release it

         move.l   d4,d0                ; file found?
         bne.s    LFOut                ; yes

         tst.b    d3                   ; keep searching?
         beq      LFScan               ; yes

         ; File not found ...

LFNoFile moveq    #0,d0                ; not found

LFOut    movea.l  d0,a0                ; IoBuff
         lea      MAXFNLEN(sp),sp      ; restore stack
         movem.l  (sp)+,d2-d7/a2-a5
         rts

* ==========================     IsMounted     =========================
* Checks whether the volume associated with a lock is currently mounted.
* Registers:   D0 -- lock
* Return:      D0 -- processid or 0
IsMounted
         lsl.l    #2,d0                ; a filelock?
         beq.s    1$                   ; no
         movea.l  d0,a0
         move.l   fl_Volume(a0),d0     ; BPTR to volume
         lsl.l    #2,d0                ; node address
         movea.l  d0,a0
         move.l   dl_Task(a0),d0       ; processid or 0

1$:      rts

* ===========================     IsRexx     ===========================
* Checks whether a file is a REXX program.  The file is opened and the
* first non-blank characters are checked for '/*'.  If found, the IoBuff
* pointer is returned.  Otherwise, the file is closed and 0 returned.
* The secondary return (in D1) indicates whether the file exists.
* Registers:   A0 -- name buffer
*              A1 -- filename (null-terminated)
* Return:      D0 -- IoBuff pointer or 0
*              A0 -- same
*              D1 -- existence flag
IsRexx
         movem.l  d2/d3/a2/a5,-(sp)
         movea.l  a0,a2                ; save buffer
         movea.l  a1,a5                ; save name

         ; Check whether the file exists ...

         movea.l  a5,a0                ; filename
         moveq    #RXIO_READ,d0        ; read access
         bsr      LockF                ; D0=lock
         move.l   d0,d3                ; a lock?
         beq.s    3$                   ; no

         ; Open the file (with no logical name)

         lea      FILELIST(a3),a0      ; global filelist
         movea.l  a5,a1                ; filename
         moveq    #RXIO_READ,d0        ; read access
         moveq    #0,d1                ; no logical name
         CALLSYS  OpenF                ; D0=A0=IoBuff pointer
         movea.l  a0,a5                ; save buffer
         beq.s    3$                   ; not opened ...

         ; Look for the first non-"white space" character.

         moveq    #RDTEST,d2           ; characters to test
1$:      CALLSYS  ReadF                ; D0=character
         movea.l  a5,a0                ; restore buffer
         bmi.s    2$                   ; error or EOF

         movea.l  rl_CTABLE(a6),a1     ; character attribute table
         btst     #CTB_SPACE,0(a1,d0.w)
         dbeq     d2,1$                ; loop back

         ; For historical reasons, REXX programs start with a comment /* */

         cmpi.b   #'/',d0              ; a '/'?
         bne.s    2$                   ; no ...

         CALLSYS  ReadF                ; D0=next character
         movea.l  a5,a0                ; restore buffer
         cmpi.b   #'*',d0              ; a '*'?
         bne.s    2$                   ; ... not a REXX program

         ; File appears to be a REXX program ... seek back to beginning.

         moveq    #0,d0                ; position
         moveq    #RXIO_BEGIN,d1       ; anchor
         CALLSYS  SeekF

         ; Expand the filename into the name buffer.

         movea.l  a2,a0                ; buffer
         moveq    #MAXFNLEN-1,d0       ; maximum length
         move.l   d3,d1                ; lock
         bsr.s    PathName             ; D0=length
         bra.s    4$

         ; Not a REXX program ... close the IoBuff.

2$:      CALLSYS  CloseF               ; close the file

3$:      suba.l   a5,a5                ; no buffer

         ; Release the lock on the file.

4$:      move.l   d3,d0                ; lock
         bsr      UnlockF              ; release it

         move.l   d3,d1                ; boolean (lock)
         move.l   a5,d0                ; set CCR
         movea.l  d0,a0
         movem.l  (sp)+,d2/d3/a2/a5
         rts

* ========================     SetDirectory     ========================
* Sets the current directory for the task and updates the CLI name.
* Registers:   D0 -- directory lock
* Return:      D0 -- old lock
SetDirectory
         movea.l  rl_SysBase(a6),a0    ; EXEC base
         movea.l  ThisTask(a0),a1      ; task ID
         move.l   pr_CurrentDir(a1),d1 ; old lock
         move.l   d0,pr_CurrentDir(a1) ; new lock

* ========================     SetDirName     ==========================
* Sets the current directory name in the CLI structure.
* Registers:   D1 -- old lock (optional)
* Return:      D0 -- old lock
SetDirName
         move.l   d1,-(sp)             ; push old lock
         move.l   a4,-(sp)

         movea.l  rl_SysBase(a6),a0    ; EXEC base
         movea.l  ThisTask(a0),a4      ; task ID
         move.l   pr_WindowPtr(a4),-(sp)
         moveq    #-1,d1               ; "no requestors"
         move.l   d1,pr_WindowPtr(a4)  ; install flag

         ; Check whether we have a CLI structure

         move.l   pr_CLI(a4),d0        ; CLI structure?
         beq.s    1$                   ; no
         lsl.l    #2,d0
         movea.l  d0,a0                ; CLI structure
         movea.l  cli_SetName(a0),a0   ; BPTR to name buffer
         adda.l   a0,a0
         adda.l   a0,a0                ; name buffer
         addq.w   #1,a0                ; skip length
         moveq    #64-2,d0             ; maximum length
         move.l   pr_CurrentDir(a4),d1 ; directory lock
         bsr.s    PathName             ; D0=length A0=buffer
         move.b   d0,-(a0)             ; install length byte

1$:      move.l   (sp)+,pr_WindowPtr(a4)
         movea.l  (sp)+,a4
         move.l   (sp)+,d0             ; pop old lock
         rts

* ============================     PathName     ========================
* Creates the full pathname associated with a lock.
* Registers:   A0 -- buffer
*              D0 -- maximum length
*              D1 -- lock
* Return:      D0 -- length
*              A0 -- buffer
STACKBF  SET      4+fib_SIZEOF
PathName
         movem.l  d2-d5/a0/a2/a5,-(sp)
         link     a4,#-STACKBF         ; stack buffer
         movea.l  a0,a5                ; save buffer
         move.l   d0,d2                ; maximum length

         ; Get the FileInfoBlock on the stack.

         move.l   sp,d0
         addq.l   #3,d0                ; round up
         andi.b   #$FC,d0              ; longword address
         movea.l  d0,a2                ; FileInfoBlock

         ; Build a stack of locks to the parent directories ...

         moveq    #-1,d3               ; initial count
         move.l   d2,d4                ; maximum depth
         move.l   d1,d0                ; initial lock

1$:      move.l   d0,-(sp)             ; push lock
         addq.l   #1,d3                ; lock count

         bsr      DOSParent            ; D0=lock
         dbeq     d4,1$                ; loop back

         ; Pop each lock and append the name, releasing all but the last.

         moveq    #':',d4              ; initial separator
         moveq    #0,d5                ; index
         bra.s    3$                   ; jump in

2$:      bsr      UnlockF              ; release lock

3$:      move.l   (sp),d0              ; top lock
         movea.l  a2,a1                ; FileInfoBlock
         bsr      DOSExamine           ; D0=boolean
         lea      fib_FileName(a2),a0  ; filename

         ; Append the filename to the buffer

4$:      cmp.w    d2,d5                ; too long?
         bhi.s    5$                   ; yes
         addq.l   #1,d5                ; new length
         move.b   (a0)+,-1(a5,d5.l)    ; install character
         bne.s    4$                   ; loop back

5$:      move.b   d4,-1(a5,d5.l)       ; install separator
         moveq    #'/',d4              ; new separator

         move.l   (sp)+,d0             ; pop lock
         dbf      d3,2$                ; loop back

         ; Check for a trailing '/' separator ...

         cmp.b    -1(a5,d5.l),d4       ; terminal separator?
         bne.s    6$                   ; no
         subq.l   #1,d5                ; back up
6$:      clr.b    0(a5,d5.l)           ; end string

         move.l   d5,d0                ; final length
         unlk     a4                   ; restore stack
         movem.l  (sp)+,d2-d5/a0/a2/a5
         rts

* ============================     SetStack     ========================
* Installs a new default stack size in the CLI structure.
* Registers:      D1 -- new stack (bytes)
* Return:         D0 -- old stack
SetStack
         moveq    #4000>>5,d0          ; 1/32 minimum stack
         lsl.w    #5,d0                ; minimum stack
         cmp.l    d0,d1                ; stack >= minimum?
         bge.s    1$                   ; yes
         move.l   d0,d1                ; ... use minimum
1$:      lsr.l    #2,d1                ; convert to longwords

         movea.l  rl_SysBase(a6),a0
         movea.l  ThisTask(a0),a1      ; our task
         move.l   pr_CLI(a1),d0        ; a CLI?
         beq.s    2$                   ; no
         lsl.l    #2,d0
         movea.l  d0,a0                ; CLI structure
         move.l   cli_DefaultStack(a0),d0
         move.l   d1,cli_DefaultStack(a0)

2$:      lsl.l    #2,d0                ; return stack
         rts

         STRUCTURE ExtendedLineInterface,0
         STRUCT   CLIStruct,cli_SIZEOF
         STRUCT   eli_DBuff,64         ; directory name
         STRUCT   eli_PBuff,64         ; prompt string
         STRUCT   eli_CBuff,4          ; command name (stub only)
         LABEL    eli_SIZEOF

         STRUCTURE PathNode,0
         LONG     pn_Next              ; BPTR to next
         LONG     pn_Lock              ; lock
         LABEL    pn_SIZEOF

* ==========================     CreateCLI     =========================
* Creates and initializes a CLI structure, and inherits appropriate values
* from an existing structure.
* Registers:      D0 -- BPTR to CLI structure
* Return:         D0 -- BPTR to CLI structure
*                 A0 -- APTR to CLI structure
CreateCLI
         movem.l  d2/a2/a3,-(sp)
         move.l   d0,d2                ; save CLI

         ; Allocate an extended CLI structure.

         moveq    #eli_SIZEOF/2,d0     ; 1/2 size
         add.w    d0,d0                ; full size
         bsr      GetDOSMem            ; D0=A0=memory
         movea.l  a0,a3                ; save structure
         beq.s    5$                   ; ... failure

         move.w   #4000/4,(cli_DefaultStack+2)(a3)
         move.w   #10,(cli_FailLevel+2)(a3)

         ; Initialize the BPTR buffer pointers.

         lea      eli_DBuff(a3),a0     ; directory buffer
         move.l   a0,d0
         lsr.l    #2,d0                ; BPTR to buffer
         move.l   d0,cli_SetName(a3)   ; install it

         moveq    #(eli_PBuff-eli_DBuff)/4,d1
         add.l    d1,d0
         move.l   d0,cli_Prompt(a3)    ; install it

         moveq    #(eli_CBuff-eli_PBuff)/4,d1
         add.l    d1,d0
         move.l   d0,cli_CommandName(a3)

         ; Check the supplied CLI structure for inheritance.

         lsl.l    #2,d2                ; CLI structure?
         beq.s    5$                   ; no ...
         movea.l  d2,a2

         ; Install the directory name ... A0 is destination buffer.

         move.l   cli_SetName(a2),d0   ; BPTR to directory name
         bsr.s    CopyBSTR             ; install it

         ; Install the prompt string.

         move.l   cli_Prompt(a2),d0    ; BPTR to prompt
         lea      eli_PBuff(a3),a0     ; prompt buffer
         bsr.s    CopyBSTR             ; install it

         move.l   cli_DefaultStack(a2),d0 ; stack size?
         beq.s    1$                      ; no
         move.l   d0,cli_DefaultStack(a3) ; install it

1$:      move.l   cli_FailLevel(a2),d0 ; failure level?
         beq.s    2$                   ; no
         move.l   d0,cli_FailLevel(a3) ; ... use it

         ; Duplicate the path locks

2$:      move.l   cli_CommandDir(a2),d2; first node
         lea      cli_CommandDir(a3),a2; start of chain
         bra.s    4$                   ; jump in

3$:      movea.l  d2,a0                ; current node
         move.l   (a0),d2              ; next node
         move.l   pn_Lock(a0),d0       ; load lock
         bsr.s    CopyPathNode         ; D0=BPTR A0=node
         beq.s    5$                   ; ... failure
         move.l   d0,(a2)              ; link to chain
         movea.l  a0,a2                ; update end node
4$:      lsl.l    #2,d2                ; a node?
         bne.s    3$                   ; yes

5$:      movea.l  a3,a0
         move.l   a0,d0
         lsr.l    #2,d0                ; BPTR return
         movem.l  (sp)+,d2/a2/a3
         rts

* ==========================     CopyBSTR     ==========================
* Copies a BSTR to a buffer area.
* Registers:      D0 -- BPTR to BSTR
*                 A0 -- buffer area
CopyBSTR
         lsl.l    #2,d0                ; a string?
         beq.s    2$                   ; no

         movea.l  d0,a1
         move.b   (a1),d1              ; load length
         moveq    #64-2,d0             ; maximum length
         cmp.b    d0,d1                ; length <= max?
         bls.s    1$                   ; yes
         move.b   d0,d1                ; ... use max

1$:      move.b   (a1)+,(a0)+          ; copy character
         subq.b   #1,d1                ; done?
         bcc.s    1$                   ; no ... loop back

2$:      clr.b    (a0)                 ; install a null
         rts

* =========================     CopyPathNode     =======================
* Copy a path node, making a duplicate of the lock.
* Registers:   D0 -- lock
* Return:      D0 -- BPTR to PathNode
*              A0 -- APTR to Pathnode
CopyPathNode
         move.l   d0,-(sp)             ; push lock
         moveq    #pn_SIZEOF,d0
         bsr.s    GetDOSMem            ; D0=A0=block
         movea.l  (sp)+,a1             ; pop lock
         beq.s    1$                   ; failure ...

         ; Duplicate the old lock ...

         move.l   a1,d1                ; old lock
         movem.l  a0/a6,-(sp)          ; push node
         movea.l  rl_DOSBase(a6),a6
         CALLSYS  DupLock              ; D0=lock
         movem.l  (sp)+,a0/a6          ; pop node
         move.l   d0,pn_Lock(a0)       ; install lcok

1$:      move.l   a0,d0
         lsr.l    #2,d0                ; BPTR to node
         rts

* ==========================     GetDOSMem     =========================
* Allocates memory as a DOS structure.
* Registers:   D0 -- size
* Return:      D0 -- memory
*              A0 -- same
GetDOSMem
         addq.l   #7,d0                ; effective size
         andi.b   #$FC,d0              ; round to longword
         move.l   d0,-(sp)             ; push size
         bsr      GetMem               ; D0=A0=memory
         movea.l  (sp)+,a1             ; pop size
         beq.s    1$                   ; ... failure
         move.l   a1,(a0)+             ; install size

1$:      move.l   a0,d0                ; set CCR

GDM001   rts

* ==========================     DeleteCLI     =========================
* Strips and releases a CLI structure.
* Registers:      D0 -- BPTR to CLI
DeleteCLI
         lsl.l    #2,d0                ; a structure?
         beq.s    GDM001               ; no

         movem.l  d2/a2,-(sp)          ; delayed save
         movea.l  d0,a2

         ; Release the path nodes.

         move.l   cli_CommandDir(a2),d2
         bra.s    2$                   ; jump in

1$:      movea.l  d2,a0                ; path node
         move.l   (a0),d2              ; next node
         bsr.s    FreePathNode
2$:      lsl.l    #2,d2                ; a node?
         bne.s    1$                   ; yes ... loop back

         ; Release the CLI structure itself.

         movea.l  a2,a0
         movem.l  (sp)+,d2/a2
         bra.s    FreeDOSMem

* =========================     FreePathNode     =======================
* Releases a PathNode and the associated lock.
* Registers:      A0 -- APTR to node
FreePathNode
         move.l   a0,-(sp)             ; push node
         move.l   pn_Lock(a0),d0       ; load lock
         bsr      UnlockF              ; release it
         movea.l  (sp)+,a0             ; pop node

* =========================      FreeDOSMem     ========================
* Frees DOS-allocated memory.
* Registers:      A0 -- memory
FreeDOSMem
         move.l   -(a0),d0             ; load size
         bra      GiveMem              ; release it

         ; String constants

Star     dc.b     '*',0
         CNOP     0,2
