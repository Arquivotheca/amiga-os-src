* == rxsupport.asm =====================================================
*
* $Id: rxsupport.asm,v 36.2 90/08/28 11:51:20 mks Exp $
*
* Copyright � 1987, 1988, 1989 by William S. Hawes.  All Rights Reserved.
*
* $Log:	rxsupport.asm,v $
* Revision 36.2  90/08/28  11:51:20  mks
* Cleaned up two macro names to match the release include files.
* It now assembles once again.
* 
* Revision 36.1  90/08/28  11:34:24  mks
* Added RCS header information.  ($Id and $Log)
*
* ======================================================================
* External Support library for the REXX interpreter

         IDNT     RexxSupport
         SECTION  RexxSupport,CODE

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/rexxio.i"
         INCLUDE  "rexx/errors.i"
         INCLUDE  "internal/rexx.i"
         INCLUDE  "internal/rexxenv.i"

         INCLUDE  "exec/resident.i"
         INCLUDE  "exec/initializers.i"
         INCLUDE  "exec/interrupts.i"
         INCLUDE  "exec/execbase.i"

         INCLUDE  "libraries/dos.i"
         INCLUDE  "libraries/dosextens.i"

RSNAME   MACRO
         dc.b     'rexxsupport.library',0
         ENDM

RSID     MACRO
         dc.b     'rexxsupport 34.9 (01-DEC-89)',10,0
         ENDM

         ; Structure definition for the Rexx Support Library

         STRUCTURE RexxSupport,LIB_SIZE
         APTR     rs_SysBase           ; EXEC base
         APTR     rs_REXXBase          ; Rexx System Library base
         LONG     rs_SegList           ; library seglist
         LABEL    rs_SIZEOF

RSVERS   EQU      34                   ; library version
RSREV    EQU      9                    ; library revision

         ; DOS library entry points

         XLIB     CreateDir
         XLIB     Delay
         XLIB     DeleteFile
         XLIB     Examine
         XLIB     ExNext
         XLIB     IoErr
         XLIB     Lock
         XLIB     Rename
         XLIB     UnLock

         ; EXEC library entry points

         XLIB     AddPort
         XLIB     AddTail
         XLIB     AllocMem
         XLIB     CloseLibrary
         XLIB     FindName
         XLIB     FindPort
         XLIB     Forbid
         XLIB     FreeMem
         XLIB     GetMsg
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg
         XLIB     Remove
         XLIB     RemPort
         XLIB     ReplyMsg
         XLIB     Wait

         ; Guard against accidental execution

Start    moveq    #0,d0
         rts

         ; The "romtag" structure follows

romtag   dc.w     RTC_MATCHWORD        ; a magic word
         dc.l     romtag               ; insurance
         dc.l     EndCode              ; end marker for checksum
         dc.b     RTF_AUTOINIT
         dc.b     RSVERS               ; library version
         dc.b     NT_LIBRARY
         dc.b     0                    ; priority
         dc.l     RSLib                ; the library name
         dc.l     RSId                 ; an ID string
         dc.l     Init                 ; initialization table

         CNOP     0,4
Init     dc.l     rs_SIZEOF
         dc.l     FuncTable
         dc.l     DataTable
         dc.l     InitEntry

         ; Table of entry point offsets in jump table order.

FuncTable
         dc.w     -1
         dc.w     Open-FuncTable        ; required entry point
         dc.w     Close-FuncTable       ; required
         dc.w     Expunge-FuncTable     ; required
         dc.w     LibReserved-FuncTable ; reserved
         dc.w     SLFunc-FuncTable      ; 'Query' entry
         dc.w     -1                    ; table end marker

         ; Initializers for the allocated library structure

DataTable
         INITBYTE LN_TYPE,NT_LIBRARY
         INITLONG LN_NAME,RSLib
         INITBYTE LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
         INITWORD LIB_VERSION,RSVERS
         INITWORD LIB_REVISION,RSREV
         INITLONG LIB_IDSTRING,RSId
         dc.l     0

* This routine is called after the library has been loaded.  The library
* pointer is returned if no initialization errors occur.
* Registers:      A0 -- DOS seglist
*                 A6 -- the EXEC base
*                 D0 -- the library pointer
InitEntry
         move.l   a5,-(sp)
         movea.l  d0,a5
         move.l   a6,rs_SysBase(a5)    ; save EXEC base
         move.l   a0,rs_SegList(a5)    ; save seglist

         ; Open the required libraries.

         lea      RXSLib(pc),a1        ; REXX systems library
         moveq    #RXS_LIB_VERSION,d0  ; required version
         CALLSYS  OpenLibrary          ; D0=library node
         move.l   d0,rs_REXXBase(a5)   ; opened?
         beq.s    1$                   ; no -- error
         move.l   a5,d0

1$:      movea.l  (sp)+,a5
         rts

* ============================     Open     ============================
* Increments the open count and returns the library pointer.
* Registers:      A6 -- library base
* Return:         D0 -- library base
Open
         bclr     #LIBB_DELEXP,LIB_FLAGS(a6)
         addq.w   #1,LIB_OPENCNT(a6)   ; increment open count
         move.l   a6,d0                ; return library base
         rts

* ============================     Close     ===========================
* Decrements the open count and checks whether the library is still open.
* If not, and if a 'delayed expunge' is pending, it is expunged.
* Registers:      A6 -- library base
* Return:         D0 -- seglist or 0
Close
         subq.w   #1,LIB_OPENCNT(a6)   ; still open?
         bne.s    Null                 ; yes ...

         ; Check whether to expunge the library.  A closed library is
         ; normally left in memory in case someone else opens it.

         btst     #LIBB_DELEXP,LIB_FLAGS(a6)
         beq.s    Null                 ; ... no delayed expunge

* ==========================     Expunge     ===========================
* If the library is still open, the 'delayed expunge' flag is set, but the
* library is left in memory.
* Registers:      A6 -- library base
* Return:         D0 -- seglist or 0
Expunge
         bset     #LIBB_DELEXP,LIB_FLAGS(a6)
         tst.w    LIB_OPENCNT(a6)      ; still open?
         bne.s    Null                 ; yes

         move.l   rs_SegList(a6),d0    ; the seglist
         movem.l  d0/a5/a6,-(sp)
         movea.l  a6,a5                ; Support base
         movea.l  rs_SysBase(a5),a6    ; EXEC base
         movea.l  a5,a1
         CALLSYS  Remove               ; unlink us

         ; Close the external libraries that were opened ...

         movea.l  rs_REXXBase(a5),a1   ; REXX base
         CALLSYS  CloseLibrary         ; close it

         ; Calculate the size of the library node and release it.

         movea.l  a5,a1                ; node
         movem.w  LIB_NEGSIZE(a5),d0/d1; load NEGSIZE/POSSIZE
         suba.w   d0,a1                ; low memory extent
         add.w    d1,d0                ; total length
         CALLSYS  FreeMem              ; release it

         movem.l  (sp)+,d0/a5/a6       ; seglist=>D0
         rts

Null
LibReserved
         moveq    #0,d0
         rts

         INCLUDE  "slfunc.asm"
         INCLUDE  "slf100.asm"
         INCLUDE  "slf200.asm"

         ; String constants

RSLib    RSNAME                        ; the library name
RSId     RSID                          ; an ID string
RXSLib   RXSLIBNAME                    ; REXX system library
         CNOP     0,2

EndCode                                ; a mysterious marker

         END
