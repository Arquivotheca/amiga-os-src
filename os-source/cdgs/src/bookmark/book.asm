************************************************************************
***                                                                  ***
***                  -= BOOKMARK DEVICE DRIVER =-                    ***
***                                                                  ***
************************************************************************
***                                                                  ***
*** CONFIDENTIAL and PROPRIETARY                                 ***
***     Copyright (c) 1990 by Commodore-Amiga, Inc.                  ***
***     By Carl Sassenrath, Sassenrath Research, Ukiah, CA           ***
***                                                                  ***
************************************************************************
    IFD CAPE
    INCDIR  "CDTV:include+V37:include+include:"
    ENDC

    INCLUDE "exec/types.i"
    INCLUDE "exec/nodes.i"
    INCLUDE "exec/tasks.i"
    INCLUDE "exec/libraries.i"
    INCLUDE "exec/devices.i"
    INCLUDE "exec/memory.i"
    INCLUDE "exec/resident.i"
    INCLUDE "exec/io.i"
    INCLUDE "exec/errors.i"
    INCLUDE "exec/execbase.i"
    INCLUDE "defs.i"
    INCLUDE "book_rev.i"
    INCLUDE "internal/copyright.i"

    IFD PRINTING
    XREF    MustPrint,Print
    ENDC

        IFD CDTV_CLASSIC
BOOKMEM     equ $DC8000     ; Location of bookmark memory
MAXBOOKMEM  equ $004000     ; Max allowed in this space

CARDMEM     equ $E00000     ; Location of cardmark memory
MAXCARDMEM  equ $080000     ; Max allowed in this space
        ENDC

        IFD CDGS
MAXBOOKMEM  equ $001000     ; Max allowed in this space
        ENDC

        IFD CDTV_CR
BOOKMEM     equ $B81000     ; Location of bookmark memory
MAXBOOKMEM  equ $001000     ; Max allowed in this space
        ENDC

        IFD CDTV_A690_CR
BOOKMEM     equ $DC8000     ; Location of bookmark memory
MAXBOOKMEM  equ $001000     ; Max allowed in this space

        ENDC

MEMMODSIZE  equ 2048        ; Unit size for additional memory

PERFORMIO   equ -42

COLOR0      equ $DFF180

*
*   Bookmark Device Base
*
db_Reserved equ LIB_SIZE
db_Memory   equ LIB_SIZE+2
db_Print    equ LIB_SIZE+8
db_SizeOf   equ LIB_SIZE+12
*
*   Memory Header
*
mb_MemoryId equ 0   ; Memory Type: 1111 'BK' 'RW' 'RO' 'RD'
mb_HeaderLen    equ 2   ; size of mem header (including above)
mb_HeaderSum    equ 3   ; simple checksum for header
mb_MemSize  equ 4   ; size of memory
mb_MaxSize  equ 8   ; max size of a data entry (32K-1 max)
mb_CheckSum equ 10  ; checksum for the entire memory
mb_FirstMark    equ 12  ; offset to first mark in memory
mb_FirstFree    equ 16  ; offset to first free memory hole
mb_FreeSpace    equ 20  ; amount of free memory
mb_SizeOf   equ 24

*
*    Memory Ids
*
MT_DIAG     equ 1111    Exec diagnostic
MT_BOOKMARK equ 'BK'    bookmark memory
MT_READWRITE    equ 'RW'    read/write ram card
MT_READONLY equ 'RO'    read only rom card
MT_RAMDISK  equ 'RD'    a ram disk
MT_OTHER    equ 'OK'    uses as something else

db      equr    a6
mb      equr    a5

*
*   Bookmark Entry
*
BE_MANFID   equ 0   ; reserve low num for expansion escape
BE_PRODID   equ 2
BE_PRI      equ 4
BE_AGE      equ 5
BE_DATALEN  equ 6
BE_DATA     equ 8   Data begins here

HIGH_PRI    equ 100

*
*   Error Codes
*
BDERR_OPENFAIL  equ -1  ; OpenDevice() failed
BDERR_ABORTED   equ -2  ; Command has been aborted
BDERR_NOTVALID  equ -3  ; IO request not valid
BDERR_NOCMD equ -4  ; No such command
BDERR_BADARG    equ -5  ; Bad command argument
BDERR_EXISTS    equ -6  ; Bookmark already exists
BDERR_NOMARK    equ -7  ; No bookmark present
BDERR_TOOBIG    equ -8  ; Bookmark larger than allowed
BDERR_NOSPACE   equ -9  ; Not enough space in memory
BDERR_PASTEND   equ -10 ; Length + Offset > Bookmark


***
***  CLI Protection
***
        rts

************************************************************************
***
***  Resident Tag
***
***********************************************************************/
*   PUBLIC  ResTag
    XDEF    ResTag

ResTag:     dc.w    RTC_MATCHWORD
        dc.l    ResTag
        dc.l    EndCode     ; end skip
        dc.b    RTF_COLDSTART   ; flags
        dc.b    VERSION     ; version
        dc.b    NT_DEVICE   ; type
        dc.b    64      ; pri (just below GFX)
        dc.l    BookDevName
        dc.l    ModIdent
        dc.l    InitDevice

************************************************************************
***
***  Identification Strings
***
***********************************************************************/

ModIdent:   VSTRING
        dc.b    'Bookmark/CardMark Device Drivers',0
        COPYRIGHT
        dc.b    'Written by Carl Sassenrath, Ukiah, CA',0
        ds.w    0

BookDevName:    dc.b    'bookmark.device',0
        ds.w    0

        IFD CDTV_CLASSIC
CardDevName:    dc.b    'cardmark.device',0
        ds.w    0
        ENDC


************************************************************************
***
***  DEVICE DRIVER INITIALIZATION
***
*** Create and initialize all data structures and hardware.
***
************************************************************************
InitDevice:
        movem.l d1-d7/a0-a6,-(sp)

    ;------ Darken screen from Exec startup (for CDTV):
        move.w  #$111,COLOR0
        move.w  #$111,COLOR0+2

    ;------ Print name:
    IFD PRINTING
        lea ModIdent,a0
        jsr MustPrint
    ENDC

    IFD CDGS
        move.l  #MAXBOOKMEM,d0
        jsr     MustAlloc
        move.l  d0,mb
    ELSE
        move.l  #BOOKMEM,mb
    ENDC    

    ;------ Is BookMark memory already in use, if not, init it:
        bsr MemType
        tst.l   d0      ; CES V4.1
        bgt.s   2$      ; already book mark memory
        blt.s   id_NotBookMem   ; something else
        move.l  #MAXBOOKMEM,d0
        bsr SizeMem
        tst.l   d0
        beq.s   id_NotBookMem   ; no memory
        bsr InitMem
2$:
    ;------ Create the BookMark device:
        lea BookDevName(pc),a0
        bsr CreateDev   ; (a0,a5)
id_NotBookMem:


        IFD CDTV_CLASSIC
    ;------ Is CardMark memory already in use, if not, init it:
        move.l  #CARDMEM,mb
        bsr MemType
        tst.l   d0      ; CES V4.1
        bgt.s   5$      ; already card mark memory
        blt.s   id_NotCardMem   ; something else
        move.l  #MAXCARDMEM,d0
        bsr SizeMem
        tst.l   d0
        beq.s   id_NotCardMem   ; no memory
        bsr InitMem
5$:
    ;------ Create the BookMark device:
        lea CardDevName(pc),a0
        bsr CreateDev   ; (a0,a5)
id_NotCardMem:
        ENDC

;       move.w  #$222,COLOR0
;       move.w  #$222,COLOR0+2


        move.l  db,d0       ; ??? what is this for ???
        movem.l (sp)+,d1-d7/a0-a6
        rts

************************************************************************
***
***  MemType
***
*** Checks to see if the memory is valid for BookMarks, and
*** if it is being used for BookMarks already.
***
*** Return >0 if BookMark, 0 if available, <0 if not available.
***
************************************************************************
MemType:
        move.w  (mb),d1
        moveq.l #1,d0
        cmp.w   #MT_BOOKMARK,d1
        beq.s   9$
        neg.l   d0
        cmp.w   #MT_DIAG,d1
        beq.s   9$
        cmp.w   #MT_READWRITE,d1
        beq.s   9$
        cmp.w   #MT_READONLY,d1
        beq.s   9$
        cmp.w   #MT_RAMDISK,d1
        beq.s   9$
        cmp.w   #MT_OTHER,d1
        beq.s   9$
        clear   d0      ; available
9$:     rts

************************************************************************
***
***  SizeMem
***
*** Determine the size of this memory in bytes.
***
*** A5 -> location of memory
*** D0 == max size of memory
***
*** Return size of memory
***
************************************************************************
* note:  am either going to have to make this routine detect eeproms
* or split cardrom from book
*
SizeMem:
        IFND CDTV_A690_CR
        move.l  mb,a0       ; start
        lea 0(a0,d0.l),a1   ; limit  V6.3 d0.l <- .l added
        clear   d1
        move.l  d1,(a0)     ; clear first one
        move.l  #$AAAA5555,d0   ; test pattern
        tst.l   (a0)        ; check first
        bne.s   9$
2$:     lea MEMMODSIZE(a0),a0 ; next
        cmp.l   a0,a1       ; at limit?
        bls.s   9$
        move.l  d0,(a0)     ; write pattern
        tst.l   (mb)        ; check for wrap
        bne.s   9$
        cmp.l   (a0),d0     ; check pattern
        beq.s   2$
9$:
        move.l  a0,d0
        sub.l   mb,d0       ; calc size
        ENDC

        IFD CDTV_A690_CR
        move.l  #MAXBOOKMEM,d0  ; hardcode size for now
        ENDC
        rts

************************************************************************
***
***  InitMem
***
*** Init memory for use as bookmark memory.  This can be done
*** before creating the device node.  This function does not
*** check to see if memory is already in use.
***
***     a5 -> location of memory
***     d0 == size of memory
***
************************************************************************
InitMem:
        IFND    CDTV_A690_CR
    ;------ Clear memory:
        move.l  d0,d1           ; length of mem
        lsr.l   #2,d1           ; long word length
        sub.l   a1,a1
        move.l  mb,a0
2$:     move.l  a1,(a0)+
        subq.l  #1,d1
        bgt 2$

    ;------ Setup memory header:
        move.w  #MT_BOOKMARK,(mb)
        moveq.l #mb_SizeOf,d1       ;Carl
        move.b  d1,mb_HeaderLen(mb) ;Carl
        move.l  d1,mb_FirstMark(mb) ;Carl
        move.l  d1,mb_FirstFree(mb) ;Carl
        lea 0(mb,d1.l),a1       ;Carl V6.3 .l
        clr.l   (a1)
        move.l  d0,mb_MemSize(mb)
        move.l  d0,mb_FreeSpace(mb)
        sub.l   #mb_SizeOf,mb_FreeSpace(mb)

    ;------ Determine max bookmark size:
        lsr.l   #4,d0           ; divide size by 16
        cmp.l   #$8000,d0       ; size limit
        blt.s   4$          ; smaller
        move.w  #$7fff,d0       ; limit is 32K-1
4$:     move.w  d0,mb_MaxSize(mb)   ; max entry size
        ENDC
        

        IFD CDTV_A690_CR
    ;------ Clear memory:
        save d3

        move.l  d0,d1           ; length of mem
        lsr.l   #1,d1           ; word length since
        sub.l   a1,a1           ; EE only supports word writes
        move.l  mb,a0

2$:     move.w  a1,(a0)         ;(brokeup)
19$:        cmp.w #0,(a0)
        bne.s 19$
        adda #2,a0

        subq.l  #1,d1
        bgt 2$

    ;------ Setup memory header:
        move.w  #MT_BOOKMARK,(mb)
20$:        cmp.w   #MT_BOOKMARK,(mb)
        bne.s   20$

        moveq.l #mb_SizeOf,d1       ;Carl

        move.b  d1,mb_HeaderLen(mb) ;Carl
5$:     cmp.b   mb_HeaderLen(mb),d1 ;(brokeup)
        bne.s 5$

        move.w  d1,mb_FirstMark+2(mb)   ;(brokeup)
9$:     cmp.w   mb_FirstMark+2(mb),d1
        bne.s 9$

        move.w  d1,mb_FirstFree+2(mb)   ;(brokeup)
8$:     cmp.w   mb_FirstFree+2(mb),d1
        bne.s 8$

        swap d1

        move.w  d1,mb_FirstMark(mb) ;high word
7$:     cmp.w   mb_FirstMark(mb),d1
        bne.s 7$

        move.w  d1,mb_FirstFree(mb)
6$:     cmp.w   mb_FirstFree(mb),d1
        bne.s 6$

        swap d1             ; put d1 back the way it was

        lea 0(mb,d1.l),a1       ;Carl V6.3 .w

        clr.w   (a1)            ;(brokeup)
17$:        cmp     #0,(a1)
        bne.s   17$
        adda    #2,a1

        clr.w   (a1)
18$:        cmp.w   #0,(a1)
        bne.s   18$
        adda    #2,a1

        move.w  d0,mb_MemSize+2(mb) ;(brokeup)
11$:        cmp.w   mb_MemSize+2(mb),d0
        bne.s   11$

        move.w  d0,mb_FreeSpace+2(mb)   ;(brokeup)
12$:        cmp.w   mb_FreeSpace+2(mb),d0
        bne.s   12$

        swap d0

        move.w  d0,mb_MemSize(mb)   ;now the high word
13$:        cmp.w   mb_MemSize(mb),d0
        bne.s 13$

        move.w  d0,mb_FreeSpace(mb)
14$     cmp.w   mb_FreeSpace(mb),d0
        bne.s 14$

        swap d0

        move.l  mb_FreeSpace(mb),d3
        sub.l   #mb_SizeOf,d3

        move.w  d3,mb_FreeSpace+2(mb) ;(brokeup)
15$     cmp.w   mb_FreeSpace+2(mb),d3
        bne.s 15$

        swap d3

        move.w  d3,mb_FreeSpace(mb) ;(brokeup) done with d3
16$     cmp.w   mb_FreeSpace(mb),d3
        bne.s 16$

    ;------ Determine max bookmark size:
        lsr.l   #4,d0           ; divide size by 16
        cmp.l   #$8000,d0       ; size limit
        blt.s   4$          ; smaller
        move.w  #$7fff,d0       ; limit is 32K-1
4$:     move.w  d0,mb_MaxSize(mb)   ; max entry size
21$:        cmp.w   mb_MaxSize(mb),d0
        bne.s   21$

        restore d3
        ENDC

        rts

************************************************************************
***
***  CreateDev
***
*** Create a new bookmark/cardmark memory device.
***
***     A0 -> Device name
***     A5 -> Memory location (already inited with InitMem)
***
************************************************************************
CreateDev:
        move.l  #db_SizeOf,d0
        clear   d1
        save    a0-a2

        lea DevFuncs(pc),a0
        move.l  d1,a1
        move.l  d1,a2
        exec    MakeLibrary
        restore a0-a2
        tst.l   d0
        beq.s   9$
        move.l  d0,db
        move.b  #NT_DEVICE,LN_TYPE(db)

        move.l  a0,LN_NAME(db)

        move.b  #LIBF_SUMUSED|LIBF_CHANGED,LIB_FLAGS(db)
        move.w  #VERSION,LIB_VERSION(db)
        move.w  #REVISION,LIB_REVISION(db)

        lea ModIdent(pc),a0
        move.l  a0,LIB_IDSTRING(db)
        move.l  mb,db_Memory(db)

    ;------ Add to Exec device list:
        move.l  db,a1
        exec    AddDevice

9$:     rts

************************************************************************
***
***  DEVICE FUNCTION/COMMAND ADDRESS TABLE
***
*** This table contains the addresses for device functions
*** and command functions.  It is used by MakeLibrary to
*** initialize the device node.
***
*** This table could be smaller (word size) but MakeLibrary
*** does not like the word size for some reason!!!
***
************************************************************************

DEVFUN      macro
        dc.l    \1
        endm

DEVCOM      macro
MAX_CMD     set MAX_CMD+1
        dc.l    \1
        endm

MAX_CMD     set 0

DevFuncs
        DEVFUN  Open        ; -6
        DEVFUN  Close       ; -12
        DEVFUN  Expunge     ; -18
        DEVFUN  Reserved    ; -24
        DEVFUN  BeginIO     ; -30
        DEVFUN  AbortIO     ; -36
        DEVFUN  Reserved    ; -42
        DEVFUN  Reserved    ; -48

        DEVCOM  CmdReset    ; 1 (-54)
        DEVCOM  CmdRead     ; 2
        DEVCOM  CmdWrite    ; 3
        DEVCOM  CmdUpdate   ; 4
        DEVCOM  CmdClear    ; 5
        DEVCOM  CmdStop     ; 6
        DEVCOM  CmdStart    ; 7
        DEVCOM  CmdFlush    ; 8

        DEVCOM  CmdTypeMem  ; 9
        DEVCOM  CmdSizeMem  ; 10
        DEVCOM  CmdInitMem  ; 11
        DEVCOM  CmdCreateDev    ; 12

        DEVCOM  CmdCreate   ; 13
        DEVCOM  CmdDelete   ; 14
        DEVCOM  CmdMaxSize  ; 15
        DEVCOM  CmdAvail    ; 16
        DEVCOM  CmdSizeOf   ; 17
        DEVCOM  CmdSetPri   ; 18

        DEVCOM  CmdCheck    ; 19
        DEVCOM  CmdPurge    ; 20
        DEVCOM  CmdDump     ; 21
        DEVCOM  CmdLoad     ; 22

        dc.l    -1      ; end of list

************************************************************************
***
***  OPEN DEVICE
***
*** Open access to the device.  Setup the unit field of the
*** I/O request to identify successful open.
***
*** The system calls this function with:
***     D0 == MANUFACTURER/PRODUCT ID
***     A1 -> IO Request Structure
***     A6 -> Device Node
***
*** If this function fails, it reports the error in the IO_ERROR
*** field of the IO Request.
***
************************************************************************
Open:
        save    ior/mb
        move.l  a1,ior
        move.l  db_Memory(db),mb

        bsr FindMark    ; d0,mb
        move.l  d0,IO_UNIT(ior) ; entry pointer

        bclr.b  #LIBB_DELEXP,LIB_FLAGS(db)
        addq.w  #1,LIB_OPENCNT(db)

        clear   d0
open_exit:  move.b  d0,IO_ERROR(ior)
        restore ior/mb
        rts
open_fail:  moveq.l #BDERR_OPENFAIL,d0
        bra open_exit

************************************************************************
***
***  CLOSE DEVICE
***
*** Conclude access to a particular device unit.
***
*** The system calls this function with:
***     A1 -> IO Request Structure
***     A6 -> Device Node
***
*** Return zero to prevent any kind of expunge.
***
************************************************************************
Close:
    ;------ Invalidate various IO Request fields:
        moveq.l #-1,d0
        move.l  d0,IO_UNIT(a1)
        move.l  d0,IO_DEVICE(a1)

    ;------ Decrement unit and device open counters:
        subq.w  #1,LIB_OPENCNT(db)

close_exit: clear   d0      ; prevent expunge
        rts

************************************************************************
***
***  EXPUNGE DEVICE
***
*** Never expunge.  Driver doesn't take much space.
***
************************************************************************
Expunge:
Reserved:
        clear   d0
        rts

************************************************************************
***
***  BEGINIO
***
*** Process an I/O request.
***
*** This function is called with:
***     A1 -> IO Request Structure
***     A6 -> Device Node
***
************************************************************************
BeginIO:
        save    d2/ior/mb/d5
        move.l  a1,ior      ; save ior pointer
        move.l  db_Memory(db),mb

        IFD PRINTING
        bsr PrCmd
        ENDC

    ;------ Initialize request fields:
        move.b  #NT_MESSAGE,LN_TYPE(ior) ; needed for WaitIO
        clr.l   IO_ACTUAL(ior)

    ;------ Protect from other tasks and power-down or reset
        exec    Forbid
        move.l  (mb),d5     ; save mem header

        IFND    CDTV_A690_CR
        clr.l   (mb)        ; in case of power-off/reset
        ENDC

        IFD CDTV_A690_CR
        clr.w   (mb)        ; in case of power-off/reset brokeup
1$:     cmp.w   #0,(mb)
        bne.s 1$

        clr.w   2(mb)
2$:     cmp.w #0,2(mb)
        bne.s 2$

        ENDC

    ;------ Check command range:
        move.b  #IOERR_NOCMD,IO_ERROR(ior)
        move.w  IO_COMMAND(ior),d0
        ble.s   bio_done    ; command too small
        cmp.w   #MAX_CMD,d0
        bgt.s   bio_done    ; command too large

    ;------ Execute command:
        clr.b   IO_ERROR(ior)
        addq.w  #8,d0       ; bias
        muls    #-6,d0      ; jmp table
        jsr 0(db,d0.w)

bio_done:   
        IFND    CDTV_A690_CR
        move.l  d5,(mb)     ; restore header
        ENDC

        IFD CDTV_A690_CR
        move.w  d5,2(mb)    ; restore low header (brokeup)
1$:     cmp.w   2(mb),d5
        bne.s 1$

        swap    d5

        move.w  d5,(mb) ; restore high part of header (brokeup)
2$      cmp.w   (mb),d5
        bne.s 2$

        ENDC
        exec    Permit

    ;------ Reply (return) the request:
        btst.b  #IOB_QUICK,IO_FLAGS(ior)
        bne.s   bio_exit    ; quickio
        move.l  ior,a1
        exec    ReplyMsg    ; not quickio

bio_exit:   restore d2/ior/mb/d5
        rts

************************************************************************
***
***  ABORTIO
***
************************************************************************
AbortIO:
        rts

************************************************************************
***
***  DEVICE COMMANDS
***
*** Perform the required device operation.
***
************************************************************************

*------ Reset Command --------------------------------------------------
CmdReset:
        move.l  mb_MemSize(mb),d0
        bra InitMem

*------ Read/Write Commands --------------------------------------------
CmdRead:
    ;------ Call common setup routine. Returns d0,a0,a1 for CopyMem.
    ;   NOTE: Will not return if an error occurs.
        bsr SetupXfer
        bsr CopyMem
        rts

CmdWrite:
    ;------ Call common setup routine. Returns d0,a0,a1 for CopyMem.
    ;   NOTE: Will not return if an error occurs.
        bsr SetupXfer
        exg a0,a1
        bsr CopyMem
        rts

SetupXfer:
    ;------ Verify valid mark:
        moveq   #BDERR_NOMARK,d2
        move.l  IO_UNIT(ior),d0
        beq.s   9$          ; no mark
        move.l  d0,a0

    ;------ Check length:
        moveq   #BDERR_PASTEND,d2
        clear   d0
        move.w  BE_DATALEN(a0),d0
        move.l  IO_LENGTH(ior),d1
        bge.s   4$          ; length is specified
        sub.w   IO_OFFSET+2(ior),d0 ; create length
        blt.s   9$          ; offset too large
        bra.s   6$

    ;------ Is length+offset too large?
4$:     add.l   IO_OFFSET(ior),d1
        cmp.w   d1,d0
        blt.s   9$          ; too large

    ;------ Check data pointer and offset:
6$:     moveq   #BDERR_BADARG,d2
        move.l  IO_DATA(ior),d1
        beq.s   9$          ; no data pointer
        move.l  d1,a1
        move.l  IO_OFFSET(ior),d1
        blt.s   9$          ; negative offset

    ;------ Setup for transfer:
        move.b  BE_PRI(a0),BE_AGE(a0)

        IFD CDTV_A690_CR
7$      move.b  BE_PRI(a0),d2
        cmp.b   BE_AGE(a0),d2   ;for EE need check to make sure
        bne 7$          ; write actually happened
        ENDC

        lea BE_DATA(a0,d1.l),a0 ; offset location
        move.l  d0,IO_ACTUAL(ior)
        rts ; d0,a0,a1

    ;------ Return an error:
9$:     addq.l  #4,sp           ; pop rts
        move.b  d2,IO_ERROR(ior)    ; return error
        rts

ErrNoMark:  move.b  #BDERR_NOMARK,IO_ERROR(ior)
        rts

ErrNoData:  move.b  #BDERR_BADARG,IO_ERROR(ior)
        rts

*------ Update/Clear Commands ------------------------------------------

CmdUpdate:
        move.l  IO_UNIT(ior),d0
        beq.s   ErrNoMark
        move.l  d0,a0

        IFND    CDTV_A690_CR
        move.b  BE_PRI(a0),BE_AGE(a0)
        ENDC

        IFD CDTV_A690_CR
        move.b  BE_PRI(a0),d0
        move.b  d0,BE_AGE(a0)
1$      cmp.b   BE_AGE(a0),d0
        bne.s 1$
        move.l  a0,d0   ;restore d0  (prob unneeded)
        ENDC
        rts

CmdClear:
        IFD CDTV_A690_CR
        save d2
        ENDC

        move.l  IO_UNIT(ior),d0
        beq.s   ErrNoMark
        move.l  d0,a0
        move.w  BE_DATALEN(a0),d0
        move.w  d0,IO_ACTUAL+2(ior)
        subq.w  #1,d0       ;Mike's adjust for odd count fix
        lsr.w   #1,d0       ; word count

        clear   d1
        addq.l  #BE_DATA,a0

        IFND    CDTV_A690_CR
2$:     move.w  d1,(a0)+
        ENDC

        IFD CDTV_A690_CR
2$:     move.w  d1,(a0)         ;(brokeup)
3$:     cmp.w   (a0),d1
        bne.s   3$
        adda    #2,a0
        ENDC

        dbf d0,2$

        IFND    CDTV_A690_CR
        move.b  BE_PRI(a0),BE_AGE(a0)
        ENDC

        IFD CDTV_A690_CR
        move.b  BE_PRI(a0),d2   ;(brokeup)
        move.b  d2,BE_AGE(a0)
1$      cmp.b   BE_AGE(a0),d2
        bne.s 1$
        ENDC

        IFD CDTV_A690_CR
        restore d2
        ENDC
        rts

*------ Utility Commands ---------------------------------------------
CmdTypeMem:
        save    mb
        move.l  IO_DATA(ior),mb
        bsr MemType
        move.l  d0,IO_ACTUAL(ior)
        restore mb
        rts

CmdSizeMem:
        save    mb
        move.l  IO_DATA(ior),mb
        move.l  IO_LENGTH(ior),d0   ; max mem size
        bsr SizeMem
        move.l  d0,IO_ACTUAL(ior)   ; size or zero
        restore mb
        rts

CmdInitMem:
        save    mb
        move.l  IO_DATA(ior),mb
        move.l  IO_LENGTH(ior),d0   ; mem size
        bsr InitMem
        restore mb
        rts

CmdCreateDev:
        save    mb/a6
        move.l  IO_DATA(ior),mb
        move.l  IO_OFFSET(ior),a0   ; name of device
        bsr CreateDev
        move.l  db,IO_ACTUAL(ior)
        restore mb/a6
        rts

*------ Create Mark Command ---------------------------------------------
CmdCreate:
    ;------ Is this marker bigger than that allowed:
        moveq.l #BDERR_TOOBIG,d2
        move.l  IO_LENGTH(ior),d0
        clear   d1
        move.w  mb_MaxSize(mb),d1
        cmp.l   d1,d0

        IFND    CDTV_A690_CR
        bgt.s   cm_error
        ENDC

        IFD CDTV_A690_CR
        bgt cm_error
        ENDC

    ;------ See if marker already exists:
        move.l  IO_OFFSET(ior),d0   ; id
        bsr FindMark        ; d0,mb
        moveq.l #BDERR_EXISTS,d2
        tst.l   d0
        bne.s   cm_error

    ;------ See if we can get space for the marker:
        move.l  IO_LENGTH(ior),d0
        addq.l  #BE_DATA,d0
        addq.l  #1,d0           ; next word up
        and.b   #$FE,d0         ; round to words
        bsr NeedSpace       ; returns ptr
        moveq.l #BDERR_NOSPACE,d2
        tst.l   d0
        beq.s   cm_error
        move.l  d0,a0
        move.l  a0,a1

    ;------ Setup the new marker header:
        IFND    CDTV_A690_CR
        move.l  IO_OFFSET(ior),(a0)+
        ENDC

        IFD CDTV_A690_CR
        move.l  IO_OFFSET(ior),d0   ;(brokeup)
        move.w  d0,2(a0)        ;first the low word
4$:     cmp.w   2(a0),d0        ;wait for the EEprom
        bne.s   4$

        swap    d0

        move.w  d0,(a0)         ;now the high word
5$:     cmp.w   (a0),d0
        bne.s   5$
        adda    #4,a0
        ENDC

        IFND    CDTV_A690_CR
        move.b  LN_PRI(ior),(a0)+   ; pri
        move.b  LN_PRI(ior),(a0)+   ; age
        ENDC

        IFD CDTV_A690_CR
        move.b  LN_PRI(ior),d0      ; pri   (brokeup)
        move.b  d0,(a0)
6$:     cmp.b   (a0),d0
        bne.s   6$
        adda    #1,a0

*       move.b  LN_PRI(ior),d0      ; age
        move.b  d0,(a0)
7$:     cmp.b   (a0),d0
        bne.s   7$
        adda    #1,a0

        ENDC
        move.l  IO_LENGTH(ior),d0
        addq.l  #1,d0           ; round up to next...
        and.b   #$FE,d0         ;    word size

        IFND    CDTV_A690_CR
        move.w  d0,(a0)+
        ENDC

        IFD CDTV_A690_CR
        move.w  d0,(a0)         ;(brokeup)
8$:     cmp.w   (a0),d0
        bne.s   8$
        adda    #2,a0
        ENDC

    ;------ Clear data area:
        clear   d1
        addq.l  #1,d0           ; for odd byte len
        lsr.w   #1,d0
        bra.s   2$

        IFND    CDTV_A690_CR
1$:     move.w  d1,(a0)+
        ENDC

        IFD CDTV_A690_CR
1$:     move.w  d1,(a0)         ;(brokeup)
3$      cmp.w   (a0),d1
        bne.s   3$
        adda    #2,a0
        ENDC

2$:     dbf d0,1$
        clear   d2          ; clear error return
        move.l  a1,IO_UNIT(ior)

cm_error:   move.b  d2,IO_ERROR(ior)
        rts

*------ Delete Mark Command ---------------------------------------------
CmdDelete:
        move.l  IO_UNIT(ior),d0
        beq ErrNoMark
        bsr PurgeMark
        clr.l   IO_UNIT(ior)    ; don't let us use it again
        rts

*------ Misc Commands ---------------------------------------------------
CmdMaxSize:
        move.w  mb_MaxSize(mb),IO_ACTUAL+2(ior)
        rts

CmdAvail:
        move.l  mb_FreeSpace(mb),IO_ACTUAL(ior)
        rts

CmdSizeOf:
        move.l  IO_UNIT(ior),d0
        beq ErrNoMark
        move.l  d0,a0
        move.w  BE_DATALEN(a0),IO_ACTUAL+2(ior)
        rts

CmdSetPri:
        move.l  IO_UNIT(ior),d0
        beq ErrNoMark
        move.l  d0,a0
        move.b  BE_PRI(a0),d0
        ext.w   d0
        ext.l   d0
        move.l  d0,IO_ACTUAL(ior) ;return old pri
        move.l  IO_OFFSET(ior),d0
        move.b  d0,BE_PRI(a0)
        move.b  d0,BE_AGE(a0)
        rts

CmdPurge:
        move.l  mb_MemSize(mb),d1 ; length of mem
        lsr.l   #2,d1       ; long word length
        clear   d0
        move.l  mb,a0

        IFND    CDTV_A690_CR
2$:     move.l  d0,(a0)+
        ENDC

*** fix .. change loop into word loop for 690

        IFD CDTV_A690_CR
2$      move.w  d0,2(a0)    ;(brokeup)
3$      cmp.w   2(a0),d0
        bne.s   3$
        adda    #2,a0

        swap    d0
        move.w  d0,(a0)
4$      cmp.w   (a0),d0
        bne.s   4$
        adda    #2,a0

        swap    d0      ;put it back!
        ENDC

        subq.l  #1,d1
        bgt 2$
        clr.l   IO_UNIT(ior)    ; no more access
        rts

CmdDump:
        bsr SetupMove   ; returns d0,a0
        move.l  IO_DATA(ior),a1
        bsr CopyMem     ; (a0,a1,d0)
        rts

CmdLoad:
        bsr SetupMove   ; returns d0,a0
        move.l  a0,a1
        move.l  IO_DATA(ior),a0
        bsr CopyMem     ; (a0,a1,d0)
        rts

SetupMove:
        move.l  IO_LENGTH(ior),d0 ; length of xfer
        move.l  d0,d1
        add.l   IO_OFFSET(ior),d1 ; starting offset
        cmp.l   mb_MemSize(mb),d1 ; past end of mem?
        ble.s   2$        ; no
        move.l  mb_MemSize(mb),d0 ; clip to max
        sub.l   IO_OFFSET(ior),d0 ;     possible length
2$:     move.l  d0,IO_ACTUAL(ior)
        move.l  mb,a0
        add.l   IO_OFFSET(ior),a0 ; starting point
        rts ; return d0,a0

CmdCheck:
        clear   d0
        move.l  mb_MemSize(mb),d1
        lsr.l   #1,d1       ; word count
        move.l  mb,a0

    ;------ Sum all words:
2$:     add.w   (a0)+,d0
        subq.l  #1,d1
        bgt.s   2$
        not.w   d0
        beq.s   8$      ; sum is ok, return 0

    ;------ Set new sum?
        tst.l   IO_OFFSET(ior)
        beq.s   8$      ; don't set new sum
        not.w   d0
        sub.w   mb_CheckSum(mb),d0 ; remove old sum
        not.w   d0
        move.w  d0,mb_CheckSum(mb) ; set new sum, return it
        IFD CDTV_A690_CR
7$:     cmp.w   mb_CheckSum(mb),d0
        bne.s 7$
        ENDC
8$:     move.w  d0,IO_ACTUAL+2(ior)
        rts

*------ Dummy Commands -------------------------------------------------
CmdStop:
CmdStart:
CmdFlush:
NoCommand:
        clear   d0
        rts

************************************************************************
***
***  FindMark(bookmark)
***
*** Returns:
***     Address of marker in special memory.
***     NULL if no matching marker.
***
************************************************************************
FindMark:
        move.l  mb_FirstMark(mb),d1
        beq.s   fm_error
        move.l  mb_FirstFree(mb),a1
        lea 0(mb,d1),a0     ;Carl
        add.l   mb,a1           ;Carl
        bra.s   start

next:       move.w  BE_DATALEN(a0),d1
        lea BE_DATA(a0,d1.w),a0
        cmp.l   a0,a1
        bls.s   fm_error

start:      cmp.l   (a0),d0     ; same manufid & prod code?
        bne.s   next

        move.l  a0,d0
fm_exit:    rts

fm_error:   clear   d0
        bra fm_exit



************************************************************************
***
***  NeedSpace(size)
***
*** Allocate space from Bookmark memory.
***
***     D0 -> size of block needed
***
*** Returns:
***     Address.
***     NULL if no space can be found.
***
************************************************************************
NeedSpace:
        IFND CDTV_A690_CR
        save    d2
        move.l  d0,d2

    ;------ Do we have the space?
0$:     sub.l   d2,mb_FreeSpace(mb)
        bgt.s   1$          ; enough free space?
        add.l   d2,mb_FreeSpace(mb) ; not enough, return it

    ;------ No space, then find a mark to purge:
        bsr.s   FindOld     ; find candidate to purge
        tst.l   d0      ; did we find one?
        beq.s   ns_exit     ; bad news

    ;------ Garbage collect memory:
        bsr PurgeMark   ; D0->mark
        bra.s   0$      ; one more time

    ;------ Give us memory for the mark:
1$:     move.l  mb_FirstFree(mb),a0
        add.l   d2,mb_FirstFree(mb)
        add.l   mb,a0       ;Carl
        move.l  a0,d0
ns_exit:
        restore d2

    ;   save    d0
    ;   move.l  d0,-(sp)
    ;   move.l  sp,a1
    ;   lea t1,a0
    ;   jsr MustPrint
    ;   add #4,sp
    ;   restore d0

;       save    d0
;       lea mb_FirstMark(mb),a1
;       lea t2,a0
;       jsr MustPrint
;       restore d0

        rts
        ENDC

        IFD CDTV_A690_CR
        save    d2/d3

        move.l  d0,d2
        move.l mb_FreeSpace(mb),d3  ; move into temp variable

    ;------ Do we have the space?

0$:     sub.l   d2,d3
        bgt.s   1$          ; enough free space?
        add.l   d2,d3 ; not enough, return it

        move.w  d3,mb_FreeSpace+2(mb)  ;save free space (brokeup)
2$:     cmp.w   mb_FreeSpace+2(mb),d3
        bne.s   2$

        swap d3
        move.w  d3,mb_FreeSpace(mb)  ;brokeup
3$:     cmp.w   mb_FreeSpace(mb),d3
        bne.s   3$

        swap d3

    ;------ No space, then find a mark to purge:
        bsr.s   FindOld     ; find candidate to purge
        tst.l   d0      ; did we find one?
        beq.s   ns_exit     ; bad news

    ;------ Garbage collect memory:
        bsr PurgeMark   ; D0->mark
        bra.s   0$      ; one more time

    ;------ Give us memory for the mark:
1$:     move.w  d3,mb_FreeSpace+2(mb)  ;save free space (brokeup)
4$:     cmp.w   mb_FreeSpace+2(mb),d3
        bne.s   4$

        swap d3

        move.w  d3,mb_FreeSpace(mb)  ;brokeup (done with d3)
5$:     cmp.w   mb_FreeSpace(mb),d3
        bne.s   5$

        move.l  mb_FirstFree(mb),a0
        move.l  a0,d3   ;add.l  d2,mb_FirstFree(mb)
        add.l   d2,d3
        move.w  d3,mb_FirstFree+2(mb)   ;(brokeup)
6$:     cmp.w   mb_FirstFree+2(mb),d3
        bne.s   6$

        swap    d3

        move.w  d3,mb_FirstFree(mb) ;(brokeup)
7$:     cmp.w   mb_FirstFree(mb),d3
        bne.s   7$

        add.l   mb,a0       ;Carl
        move.l  a0,d0

ns_exit:
        restore d2/d3
        rts
        ENDC

************************************************************************
***
***  FindOld
***
*** Scans all markers and returns the best candidate for being
*** purged from memory.  All markers are aged (except high pri
*** markers).
***
************************************************************************
FindOld:
    ;   lea FindOldMsg,a0
    ;   jsr MustPrint

        save    d2/a2
        sub.l   a2,a2       ; clear it
        move.b  #$7F,d2     ; highest positive (value)

    ;------ Set up memory parameters:
        move.l  mb_FirstMark(mb),d0
        beq.s   9$      ; not allowed
        lea 0(mb,d0.l),a0   ;Carl
        move.l  mb_FirstFree(mb),a1
        add.l   mb,a1       ;Carl
        bra.s   2$

    ;------ Advance to next marker:
1$:     move.w  BE_DATALEN(a0),d1
        lea BE_DATA(a0,d1.w),a0
        cmp.l   a0,a1
        bls.s   4$      ; end of list

    ;------ Ignore high priority marker:
2$:     cmp.b   #HIGH_PRI,BE_PRI(a0)
        bge.s   1$

    ;------ Age marker:
        move.b  BE_AGE(a0),d0
        cmp.b   #$80,d0     ; as low as it can go?
        beq.s   3$      ; already low as possible
        subq.b  #1,d0       ; age it
        move.b  d0,BE_AGE(a0)
3$:     cmp.b   d0,d2       ; is it the oldest?
        ble.s   1$      ; no
        move.b  d0,d2       ; yes
        move.l  a0,a2
        bra.s   1$

4$:
    ;   save    d0
    ;   lea (a2),a1
    ;   lea t3,a0
    ;   jsr MustPrint
    ;   restore d0

        move.l  a2,d0
9$:     restore d2/a2
        rts

************************************************************************
***
***  PurgeMark
***
*** Remove a marker by compacting it away.
***
*** D0 -> marker to purge.
***
************************************************************************
PurgeMark:
        IFND    CDTV_A690_CR
        save    d2
        ENDC
        IFD CDTV_A690_CR
        save    d2/d3
        ENDC
        move.l  d0,a0

    ;------ Clear Bookmark ID (just in case first bookmark):  V4.2
        IFND    CDTV_A690_CR
        clr.l   (a0)
        ENDC


        IFD CDTV_A690_CR

        clr.w   (a0)
11$:        cmp.w   #0,(a0)
        bne.s 11$

        clr.w   2(a0)
12$:        cmp.w #0,2(a0)
        bne.s 12$

        ENDC

    ;------ Calculate source address for compaction copy:
        moveq.l #BE_DATA,d1 ; head size
        add.w   BE_DATALEN(a0),d1
        move.l  d1,d2       ; save length as long
        move.l  a0,a1
        add.l   d1,a1

    ;------ Determine size to copy:
        move.l  mb_FirstFree(mb),d0
        add.l   mb,d0       ;Carl
        sub.l   a1,d0       ; at or past end?
        bgt.s   2$      ; no
        sub.l   mb,a0       ;Carl

        IFND    CDTV_A690_CR
        move.l  a0,mb_FirstFree(mb) ; yes, use it.
        ENDC

        IFD CDTV_A690_CR
        move.l  a0,d3
        move.w  d3,mb_FirstFree+2(mb) ; yes, use it. (brokeup)
5$:     cmp.w   mb_FirstFree+2(mb),d3
        bne.s   5$

        swap d3

        move.w  d3,mb_FirstFree(mb) ; (brokeup)
6$:     cmp.w   mb_FirstFree(mb),d3
        bne.s   6$
        ENDC

        bra.s   8$

    ;------ Compact (copy) over it:
2$:
    ;   save    d0/d1/a0/a1
    ;   pea (a0)
    ;   pea (a1)
    ;   move.l  d1,-(sp)
    ;   move.l  d0,-(sp)
    ;   lea     (sp),a1
    ;   lea PurgeMsg2,a0
    ;   jsr MustPrint
    ;   add #4*4,sp
    ;   restore d0/d1/a0/a1

        IFND    CDTV_A690_CR
        lsr.l   #2,d0
3$:     move.l  (a1)+,(a0)+
        dbf d0,3$
        ENDC

        IFD CDTV_A690_CR
        lsr.l   #1,d0       ; need to do word copy

3$:     move.w  (a1),d3     ;brokeup

        move.w  d3,(a0)
4$:     cmp.w   (a0),d3
        bne.s   4$      ; wait for EEprom to be ready
        adda    #2,a1
        adda    #2,a0
        dbf d0,3$
        ENDC

    ;------ Adjust free pointer:

        IFND    CDTV_A690_CR
        sub.l   d2,mb_FirstFree(mb)
8$:     add.l   d2,mb_FreeSpace(mb)
        ENDC

        IFD CDTV_A690_CR
        move.l mb_FirstFree(mb),d3
        sub.l   d2,d3 ;(brokeup)
        move.w  d3,mb_FirstFree+2(mb)
10$:        move.w  mb_FirstFree+2(mb),d3
        bne.s   10$

        swap d3

        move.w  d3,mb_FirstFree(mb) ;and the high word
14$:        cmp.w   mb_FirstFree(mb),d3
        bne.s   14$

8$:     move.l mb_FreeSpace(mb),d3 ;(brokeup)
        add.l   d2,d3
        move.w  d3,mb_FreeSpace+2(mb)
7$:     cmp.w   mb_FreeSpace+2(mb),d3
        bne.s   7$

        swap d3

        move.w  d3,mb_FreeSpace(mb)
9$:     cmp.w   mb_FreeSpace(mb),d3
        bne.s   9$

        ENDC

*       bsr PrintMarks

        IFND    CDTV_A690_CR
        restore d2
        ENDC

        IFD CDTV_A690_CR
        restore d2/d3
        ENDC
        rts

************************************************************************

MustAlloc:  ; d0=size
        clear   d1
        exec    AllocMem
        tst.l   d0
        rts         ; returns condition code!

CopyMem:    ; a0->source, a1->target, d0=count

        IFND CDTV_A690_CR
        exec    CopyMem
        rts
        ENDC

        IFD CDTV_A690_CR
CopyMemByte:    ; ( a0: source, a1: dest, d0: numBytes )
    ;------ do byte by byte copy
        save d3

        move.w  d0,d1       ; least significant word
        swap    d0      ; most significant word
        bra.s   bc_bytesentry
bc_bytesloop:
        move.b  (a0),d3 ;   ;brokeup
        move.b  d3,(a1)
1$:     cmp.b   (a1),d3
        bne.s   1$
        adda    #1,a0
        adda    #1,a1
bc_bytesentry:
        dbra    d1,bc_bytesloop
        dbra    d0,bc_bytesloop
                            
        restore d3
        rts
        ENDC

    IFD PRINTING
PrCmd:
        save    d0-d1/a0-a1
        lea IO_COMMAND(ior),a1
        lea cmdmsg,a0
        jsr Print
        restore d0-d1/a0-a1
        rts

cmdmsg      dc.b    'CMD(%d) %x %lx LEN(%lx) DAT(%lx) OFF(%lx)',10,0
        ds.w    0

PurgeMsg    dc.b    'Purge',10,0
PurgeMsg2   dc.b    'Purge $%lx $%lx $%lx $%lx',10,0
FindOldMsg  dc.b    'Find',10,0
t1      dc.b    'Return %lx',10,0
t2      dc.b    '----- Mark $%lx - First $%lx - Free $%lx -----',10,0
t3      dc.b    '      Mark $%lx Pri/Age: $%x Len: %d ---',10,0
        ds.w    0

PrintMarks:
        save    d0/d1/d2/a0/a1/a2

        moveq.l #$1A,d2

        lea mb_FirstMark(mb),a1
        lea t2,a0
        jsr MustPrint

        move.l  mb_FirstMark(mb),a2

1$:     move.l  a2,a1
        lea t3,a0
        jsr MustPrint

        move.w  BE_DATALEN(a2),d1
        lea BE_DATA(a2,d1.w),a2
        dbf d2,1$

        restore d0/d1/d2/a0/a1/a2
        rts
    ENDC

EndCode
        END



