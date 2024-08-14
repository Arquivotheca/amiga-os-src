;*************************************************************************
;
; jddata.i -- the software data structures for the jdisk device
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************

        IFND    EXEC_LIBRARIES_I
        INCLUDE "exec/libraries.i"
        ENDC
        IFND    EXEC_INTERRUPTS_I
        INCLUDE "exec/interrupts.i"
        ENDC


;------ extensions to IO_FLAGS -------------------------------------------
    BITDEF  IO,CURRENT,4
    BITDEF  IO,DONE,5
    BITDEF  IO,BIOSERR,7

;------ Trackbuffer size made absolute since it only made format a little
;------ more efficient.  Since format now takes ages because of all the
;------ sector testing, this efficiency is lost and the extra code required
;------ to find the maximum track size is just not worth the effort.  Also
;------ the loop that used to find max track size is now used to read in
;------ the bad block info if it exists.
TB_SIZE EQU 17*512              ; this should handle nearly all cases (poof)

;------ the user should never see this, but here it is -------------------
JD_WRONGLENGTH  EQU $30

;------ device structure -------------------------------------------------
 STRUCTURE  JDiskDevice,LIB_SIZE
    APTR    jd_Segment              ; the dos segment for this device
    APTR    jd_ExecBase
    APTR    jd_JanusBase
    APTR    jd_TrackBuffer          ; the actual track buffer in janus mem
    APTR    jd_AmigaDskReq          ; general disk request structure
    STRUCT  jd_CmdQueue,LH_SIZE     ; the command queue
    STRUCT  jd_CmdTerm,IS_SIZE      ; cmd for JSERV_HARDDISK termination
    ULONG   jd_RequestedCount       ; the length asked of the 8088 code
    APTR    jd_Units                ; pointer to unit list for bad blocks
    LABEL   JDiskDevice_SIZEOF

;===========================================================================
; Extra structures added for bad block handling using the last 2 cylinders.
; These had to be attached to the device structure itself because there was
; no way of safely attaching them to the individual units as DOS sees them.
;                       Steve Beats -- January 1987
;===========================================================================

;------ structure of a single bad block entry -----------------------------
 STRUCTURE  OneBlock,0
    ULONG   OB_PhysicalBlock        ; number of block that was bad
    ULONG   OB_MappedBlock          ; and where it was mapped to
    LABEL   OB_SIZEOF

MAXMAPPED EQU (1024-8)/OB_SIZEOF    ; max number of blocks to be mapped

;------ unit structure for mapping bad blocks on each unit ---------------
 STRUCTURE  BBMUnit,0
    APTR    BB_Next                 ; next unit in this list
    UWORD   BB_Unit                 ; unit this is attached to
    UWORD   BB_TotalBlocks          ; total number of blocks in 2 cylinders
    ULONG   BB_BadMapOffset         ; offset to the bad block map
    ULONG   BB_RealMapOffset        ; real offset if first blk is bad
;------ this part of the structure is written to disk --------------------
    ULONG   BB_Valid                ; contains $DEFACED if this is valid
    UWORD   BB_MaxBlocks            ; max blocks that can be mapped
    UWORD   BB_NumBlocks            ; Current # of mapped blocks
    STRUCT  BB_BlockMaps,MAXMAPPED*OB_SIZEOF
    STRUCT  BB_DummyEntry,OB_SIZEOF ; to stop search running off the end
    LABEL   BB_SIZEOF
;===========================================================================
; MACRO to provide a printf debugging call, use it like the 'C' version. ie.
;
;       printf  <'value=%ld pointer=%lx count=%d\n'>,d0,a0,d1
;       printf  <'program name = %s\n'>,#ProgName
;       printf  <'port name = %s\n'>,a0
;       printf  <'character received = \t%c\n'>,d0
;       printf  <'Program length = %d\n'>,#(ProgEnd-ProgStart)
;       printf  <'%c'>,#7
;
; Your program must include this macro file and link with sprintf.lib to
; work correctly with serial output.  If you want output to go to a console
; window then you must link with cprintf.lib instead.  Whichever version
; you decide to use, you must FIRST execute the DEBUGINIT macro before
; making use of the printf macro.  If you linked with cprintf.lib then you
; must also execute the DEBUGKILL macro before exiting your program. (this
; is because the cprintf function uses the DOS library).  A maximum of 8
; arguments can be sent to the printf function.  Executing DEBUGENABLE sets
; a constant called DEBUG_CODE which is tested by the debug macros to see
; if macro expansion should be performed.  This means you can leave all of
; your diagnostic printf's in place and just comment out the DEBUGENABLE call
; to strip all debug code from your program.  This works because all of the
; actual code for the printf functions is contained in libraries and is
; only linked to your program if a routine in this library is referenced.
;
; IMPORTANT NOTE:
; You should only call DEBUGINIT once in your program (usually in the first
; module).  If you need to use printf in other object modules of the same
; program then you should just use the DEBUGENABLE macro.  This merely sets
; the DEBUG_CODE variable and does not try to call the init routine again.
; On the same note, you should only call DEBUGKILL once in your program
; too (usually just before exiting your code).
;==========================================================================

printf  MACRO
        IFD DEBUG_CODE                          ; only if DEBUGINIT called
        NOLIST
; first stack up to eight arguments for the printf routine
        IFGE    NARG-9
                LIST
                move.l  \9,-(sp)                ; stack arg8
                NOLIST
        ENDC
        IFGE    NARG-8
                LIST
                move.l  \8,-(sp)                ; stack arg7
                NOLIST
        ENDC
        IFGE    NARG-7
                LIST
                move.l  \7,-(sp)                ; stack arg6
                NOLIST
        ENDC
        IFGE    NARG-6
                LIST
                move.l  \6,-(sp)                ; stack arg5
                NOLIST
        ENDC
        IFGE    NARG-5
                LIST
                move.l  \5,-(sp)                ; stack arg4
                NOLIST
        ENDC
        IFGE    NARG-4
                LIST
                move.l  \4,-(sp)                ; stack arg3
                NOLIST
        ENDC
        IFGE    NARG-3
                LIST
                move.l  \3,-(sp)                ; stack arg2
                NOLIST
        ENDC
        IFGE    NARG-2
                LIST
                move.l  \2,-(sp)                ; stack arg1
                NOLIST
        ENDC
; Now the actual printf call itself, only if there is an argument string
        IFGE    NARG-1
STKOFF  SET     NARG<<2                         ; actual stack space used
        XREF    _printf                         ; in case not used before
                LIST
                pea.l   str\@(pc)               ; push string address
                jsr     _printf                 ; call printf function
                lea.l   STKOFF(sp),sp           ; scrap stuff on stack
                bra.s   done\@
str\@           dc.b    \1,0                    ; the actual string
                CNOP    0,2
done\@:
                NOLIST
        ENDC
        LIST
        ENDC                                    ; end DEBUG_CODE conditional
        ENDM

;===========================================================================
; MACRO to enable all of the debug routines.  If not called, no extra code.
;===========================================================================

DEBUGENABLE MACRO

DEBUG_CODE EQU 1

        ENDM

;===========================================================================
; MACRO to initialise debug routines, works for cprintf.lib and sprintf.lib.
;===========================================================================

DEBUGINIT       MACRO
                IFD     DEBUG_CODE
                NOLIST
                XREF    _DebugInit
                LIST
                jsr     _DebugInit
                ENDC
                ENDM

;==========================================================================
; MACRO to kill debug routines and libraries if cprintf.obj was linked with
;==========================================================================

DEBUGKILL       MACRO
                IFD     DEBUG_CODE
                NOLIST
                XREF    _DebugKill
                LIST
                jsr     _DebugKill
                ENDC
                ENDM


