*
* $Id: slf100.asm,v 36.1 90/08/28 11:33:35 mks Exp $
*
* $Log:	slf100.asm,v $
* Revision 36.1  90/08/28  11:33:35  mks
* Added RCS header information.  ($Id and $Log)
* 
* ===========================     STATEF     ===========================
* Returns information about a file or directory.
* N.B. The global buffer must be large enough to hold a FileInfoBlock + 24
* Usage: STATEF(filename)
ADVANCE  SET      24
SFstatef
         lea      ADVANCE(a3),a2       ; offset the buffer
         move.l   rl_DOSBase(a4),d4    ; DOS library base
         moveq    #' ',d5              ; separator

         ; Attempt to obtain a lock on the file ...

         move.l   a0,d1                ; filename
         moveq    #ACCESS_READ,d2      ; read access
         exg      d4,a6
         CALLSYS  Lock                 ; D0=lock
         move.l   d0,d3                ; lock obtained?
         beq.s    0$                   ; no

         ; Call 'Examine' to get the FileInfoBlock

         move.l   d3,d1                ; lock
         move.l   a2,d2                ; work area (FileInfoBlock)
         CALLSYS  Examine              ; D0=boolean

         ; Release the previously-obtained lock ...

         move.l   d3,d1                ; lock
         move.l   d0,d3                ; save result
         CALLSYS  UnLock

0$:      exg      d4,a6
         tst.l    d3                   ; success?
         beq      SFNull               ; no

         ; Extract the information and format it as
         ; "{DIR | FILE} bytes blocks protect days minutes ticks comment"

         movea.l  a3,a0                ; work area
         lea      TypeDIR(pc),a1       ; directory code
         tst.l    fib_DirEntryType(a2) ; a directory?
         bpl.s    1$                   ; yes
         addq.l   #DIRLEN,a1           ; file code

1$:      move.b   (a1)+,(a0)+          ; copy the type
         bne.s    1$                   ; loop until NULL
         move.b   d5,-1(a0)            ; blank delimiter

         ; Get the file size in bytes and blocks

         move.l   fib_Size(a2),d0      ; bytes
         bsr.s    GetDigits            ; A0=scan
         move.l   fib_NumBlocks(a2),d0 ; blocks
         bsr.s    GetDigits            ; A0=scan

         ; Check the protection attributes.

         move.l   fib_Protection(a2),d0
         moveq    #NUMPRB-1,d1         ; loop count

2$:      move.b   Protect0(pc,d1.w),d2 ; bit-0 attribute
         btst     d1,d0                ; bit set?
         beq.s    3$                   ; no
         move.b   Protect1(pc,d1.w),d2 ; bit-1 attribute
3$:      move.b   d2,(a0)+             ; install it
         dbf      d1,2$                ; loop back
         move.b   d5,(a0)+             ; blank delimiter

         ; Install the date stamp (days, minutes, ticks)

         lea      fib_DateStamp+ds_Days(a2),a1
         moveq    #2,d2

4$:      move.l   (a1)+,d0             ; value
         move.l   a1,-(sp)             ; push pointer
         bsr.s    GetDigits            ; A0=scan
         movea.l  (sp)+,a1             ; pop pointer
         dbf      d2,4$                ; loop back

         ; Copy the comment field (A1 set)

5$:      move.b   (a1)+,(a0)+          ; copy string
         bne.s    5$                   ; ... until null

         subq.l   #1,a0                ; back up
         move.l   a0,d3                ; end of string
         sub.l    a3,d3                ; final length
         bra      SFNewstr

* ==========================     GetDigits     =========================
* Convert an integer value to ASCII digits
* Registers:   D0 -- integer
*              A0 -- buffer
* Return:      A0 -- advanced pointer
GetDigits
         moveq    #12,d1               ; maximum digits
         exg      a4,a6
         CALLSYS  CVi2a                ; D0=length  A0=pointer
         exg      a4,a6
         move.b   #' ',(a0)+           ; blank delimiter
         rts

         ; String constants

NUMPRB   EQU      8                    ; number of attributes
Protect0 dc.b     'DEWR----'           ; bit-0 attributes
Protect1 dc.b     '----APSH'           ; bit-1 attributes
TypeDIR  dc.b     'DIR',0              ; directory type
TypeFILE dc.b     'FILE',0             ; file type
DIRLEN   EQU      TypeFILE-TypeDIR     ; length
         CNOP     0,2

* ==========================     SHOWDIR     ===========================
* Returns the contents of a directory as a string of file names.
* USAGE: DIR(name,['All' | 'Dir' | 'File'],[separator])
ADVANCE  SET      24
SFshowdir
         lea      ADVANCE(a3),a2       ; FileInfoBlock area
         move.l   rl_DOSBase(a4),d4    ; DOS library base
         moveq    #0,d5                ; select code

         moveq    #2,d1                ; argument index
         cmp.b    d1,d7                ; select option or separator?
         blt.s    0$                   ; no

         moveq    #'A',d0              ; default option
         bsr      LoadPad              ; D0=option
         bclr     #LOWERBIT,d0         ; D0=UPPERcase

         subi.b   #'A',d0              ; 'All'?
         beq.s    0$                   ; yes
         moveq    #1,d5                ; flag for 'DIR'
         subq.b   #'D'-'A',d0          ; 'Dir'?
         beq.s    0$                   ; yes
         moveq    #-1,d5               ; flag for 'FILE'
         subq.b   #'F'-'D',d0          ; 'File'?
         bne      SFErr18              ; no -- invalid option

         ; Attempt to obtain a lock on the file ...

0$:      move.l   a0,d1                ; name
         moveq    #ACCESS_READ,d2      ; access mode
         exg      d4,a6
         CALLSYS  Lock                 ; D0=lock
         move.l   d0,d3                ; lock obtained?
         beq.s    1$                   ; no

         ; Call 'Examine' to get the FileInfoBlock

         move.l   d3,d1                ; lock
         move.l   a2,d2                ; FileInfoBlock
         CALLSYS  Examine              ; D0=boolean

1$:      exg      d4,a6
         moveq    #0,d2                ; clear value
         tst.l    d0                   ; success?
         beq.s    8$                   ; no

         tst.l    fib_DirEntryType(a2) ; a directory?
         bmi.s    8$                   ; no

         ; Initialize the list header

         exg      a4,a6                ; SUPP=>A4 , REXX=>A6
         movea.l  a3,a0                ; list header
         CALLSYS  InitList             ; initialize it

         ; Scan through the directory, checking each file in turn.

2$:      move.l   d3,d1                ; directory lock
         move.l   a2,d2                ; FileInfoBlock
         exg      d4,a6
         CALLSYS  ExNext               ; D0=boolean
         exg      d4,a6
         tst.l    d0                   ; success?
         beq.s    4$                   ; no

         ; Check that the file is of the right type

         tst.l    d5                   ; unconditional selection?
         beq.s    3$                   ; yes
         smi      d0
         tst.l    fib_DirEntryType(a2) ; Dir => -1, File => 1
         smi      d1
         eor.b    d1,d0                ; selected?
         bne.s    2$                   ; no ... loop back

         ; Copy the filename into a resource node

3$:      movea.l  a3,a0                ; list header
         lea      fib_FileName(a2),a1  ; filename (null-terminated)
         moveq    #rr_SIZEOF,d0
         CALLSYS  AddRsrcNode          ; D0=A0=node
         move.l   d0,d2                ; allocated?
         bne.s    2$                   ; ... loop back
         bra.s    6$                   ; ... error

         ; Scan complete ... allocate the return string.

4$:      moveq    #BLANK,d0            ; default separator
         moveq    #3,d1                ; argument index
         movea.l  (ARG2+4)(a5),a1      ; argstring
         bsr.s    LoadPad              ; D0=pad character

         movea.l  a3,a0                ; list header
         CALLSYS  ListNames            ; D0=A0=argstring
         move.l   d0,d2                ; an argstring?
         bne.s    7$                   ; yes

         ; Allocation error ... set code.

6$:      moveq    #ERR10_003,d6        ; allocation failure

         ; Release the temporary list nodes.

7$:      movea.l  a3,a0                ; list header
         CALLSYS  RemRsrcList
         exg      a4,a6                ; REXX=>A4 , SUPP=>A6

         ; Release the directory lock ...

8$:      move.l   d3,d1                ; directory lock
         exg      d4,a6
         CALLSYS  UnLock               ; release it
         exg      d4,a6

         ; Determine the return value.

         movea.l  d2,a0                ; result string
         tst.l    d2                   ; a value?
         beq      SFNull               ; no
         rts

* ===========================     LoadPad     ==========================
* Loads a pad character if an argument was supplied. (A0 preserved).
* Registers:   D0 -- default pad
*              D1 -- argument index
*              A1 -- argument
* Return:      D0 -- pad
LoadPad
         cmp.b    d1,d7                ; enough arguments?
         blt.s    1$                   ; no
         move.l   a1,d1                ; an argument?
         beq.s    1$                   ; no
         tst.w    (ra_Length-ra_Buff)(a1)
         beq.s    1$                   ; ...no characters
         move.b   (a1),d0              ; load separator
1$:      rts

* ============================     DELAY     ===========================
* Implements the DELAY() function, protected against delay = 0.
* Usage: DELAY(ticks)
SFdelay
         tst.l    d3                   ; delay > 0?
         ble.s    1$                   ; no
         move.w   #_LVODelay,d1        ; load offset
         movea.l  d3,a0                ; argument
         bsr.s    SFDOS                ; D0=boolean
         move.l   d0,d3
1$:      bra      SFBool

* ============================     DELETE     ==========================
* Implements the DELETE() function.
* Usage: DELETE(filename)
SFdelete
         move.w   #_LVODeleteFile,d1
         bsr.s    SFDOS                ; D0=boolean
         move.l   d0,d3
         bra      SFBool

* ===========================     RENAME     ===========================
* Implements the RENAME() function.
* Usage: RENAME(oldname,newname)
SFrename
         move.w   #_LVORename,d1
         bsr.s    SFDOS                ; D0=boolean
         move.l   d0,d3
         bra      SFBool

* ===========================     MAKEDIR     ==========================
* Implements the MAKEDIR() function.
* Usage: MAKEDIR(directory)
SFmakedir
         move.w   #_LVOCreateDir,d1
         bsr.s    SFDOS                ; D0=lock
         move.l   d0,d3                ; save result

         move.l   d0,a0                ; lock argument
         move.w   #_LVOUnLock,d1
         bsr.s    SFDOS
         bra      SFBool

         ; Makes a DOS library call ... arguments in A0/A1/D0

SFDOS    exg      a0,d1                ; first argument=>D1, offset=>A0
         move.l   a1,d2                ; second argument
         move.l   d0,d3                ; third argument
         move.l   rl_DOSBase(a4),d4
         exg      d4,a6
         jsr      0(a6,a0.w)           ; D0=boolean D1=secondary
         exg      d4,a6
         rts
