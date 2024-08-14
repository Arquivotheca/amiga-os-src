    ifnd    _CAMDLIB_I
_CAMDLIB_I  set 1

*************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* camdlib.i   - library common include file (library private)
*
*************************************************************************

        ifnd    EXEC_TYPES_I
        include "exec/types.i"
        endc

        ifnd    MIDI_CAMD_I
        include "midi/camd.i"
        endc

        ifnd    MIDI_CAMDBASE_I
        include "midi/camdbase.i"
        endc

        optimon $ffffffff       ; turn on all C.A.P.E optimizations


****************************************************************
*
*   Conditional compilation
*
****************************************************************

OS_MIN  equ LIBRARY_MINIMUM     ; minimum requires OS
OS_MAX  equ 37                  ; max version to do special case code for


****************************************************************
*
*   Internal Data Structures
*
****************************************************************

  ifne    0
    STRUCTURE HookTblEntry,0
        APTR    hte_hook                ; Hook
        APTR    hte_mi                  ; MidiNode
        LABEL   HookTblEntry_Size
  endc

PDSX_ShortBufSize  equ 514              ; pads to next 8 byte boundary
PDSX_LongBufExp    equ 12
PDSX_LongBufSize   equ 1<<PDSX_LongBufExp

    STRUCTURE ParserData,0
                    ; current state
        STRUCT  pd_CurMsg,MidiMsg_Size  ; contains running status and time stamp
        STRUCT  pd_RTMsg,MidiMsg_Size   ; temp storage space for realtime messages
        ULONG   pd_ThreeByte            ; "ThreeByte" register contents

                    ; sys/ex stuff
        STRUCT  pd_SXList,MLH_SIZE      ; list of SysExNode's
        APTR    pd_SXBuff               ; current buffer starting address
        APTR    pd_SXBCur               ; next byte to fill
        APTR    pd_SXBEnd               ; byte after end of buffer
        UWORD   pd_NSXNodes             ; number of nodes in SXList
        STRUCT  pd_SXShortBuffer,PDSX_ShortBufSize
        LABEL   pd_SXShortBufferEnd
        LABEL   ParserData_Size

    STRUCTURE ParserSXNode,MLN_SIZE
        STRUCT  psxn_Buffer,PDSX_LongBufSize
        LABEL   psxn_BufferEnd
        LABEL   ParserSXNode_Size


    STRUCTURE XMidiNode,MidiNode_PublicSize     ; internal version of MidiNode

            ; MidiMsg Queue
        APTR    mi_MsgQueue             ; NULL if MsgQueueSize == 0
        APTR    mi_MsgQueueEnd          ; MsgQueue + (MsgQueueSize + 1) * MidiMsg_Size.  +1 is padding
        APTR    mi_MsgQueueHead         ; next filled MidiMsg in MsgQueue
        APTR    mi_MsgQueueTail         ; next available MidiMsg in MsgQueue

            ; Sys/Ex Queue
        APTR    mi_SysExQueue           ; NULL if SysExQueueSize == 0
        APTR    mi_SysExQueueEnd        ; SysExQueue + SysExQueueSize + 1.  +1 is padding
        APTR    mi_SysExQueueHead       ; next filled byte in SysExQueue
        APTR    mi_SysExQueueTail       ; byte after last completed sys/ex msg in SysExQueue

        ifne    0
            ; alarm stuff
        STRUCT  mi_AlarmNode,MLN_SIZE   ; node added to AlarmList
        ULONG   mi_AlarmTime
        endc

            ; misc
;       APTR    mi_SigTask              ; task that owns this MidiNode
;       APTR    mi_ParserData           ; pointer to ParserData, may be NULL if no parser is attached
        UBYTE   mi_SysFlags             ; misc receive flags (MIF_ below)
        UBYTE   mi_ErrFlags             ; CMEF_ error flags

        LABEL   XMidiNode_Size


    ; SysFlags

        BITDEF  MI,InSysEx,0        ; In the middle of reading a Sys/Ex message.
                                    ; This will only be set if there is a Sys/Ex Queue.
                                    ; Managed by GetMidi(), GetSysEx(), etc.

        BITDEF  MI,MySXBuf,1        ; SysExQueue created by CreateMidi()

        BITDEF  MI,AlarmOK,2        ; Alarm functions may be called (i.e. there's
                                    ; an alarm signal bit)

        BITDEF  MI,Ready,7          ; MidiNode is operational


    STRUCTURE XCamdBase,CamdBase_PublicSize     ; internal version of CamdBase
        APTR    camd_SysBase                    ; cache these pointers
        APTR    camd_DOSBase
        APTR    camd_UtilityBase
        APTR    camd_MiscBase
        ULONG   camd_SegList                    ; really a BPTR

        STRUCT  camd_MidiListLock,SS_SIZE       ; arbitrates access to msg distribution to MidiNodes
        STRUCT  camd_CamdSemaphores,CD_NLocks*SS_SIZE

        STRUCT  camd_DriverList,MLH_SIZE
        STRUCT  camd_BytePortList,MLH_SIZE
        STRUCT  camd_MidiList,MLH_SIZE
        STRUCT  camd_ClusterList,MLH_SIZE

        APTR    camd_HardwareNode               ; for external input/output

        ULONG	camd_TimeLess			; where timeless links look

        UWORD	camd_PrivateLockBits
        UWORD	camd_PublicLockBits

    	STRUCT	camd_NotifyList,MLH_SIZE	; cluster change notification

        LABEL   XCamdBase_Size

MidiListBit		equ		0

****************************************************************
*
*   Macros
*
****************************************************************

****************************************************************
*
*   Lock/Unlock MidiList
*
*   FUNCTION
*       Lock/Unlock CamdBase->MidiList.  Calls Obtain/
*       Release Semaphore().
*
*   INPUTS
*       A5 - CamdBase
*       A6 - SysBase
*
*   RESULTS
*       None
*
****************************************************************

LockMidiList macro
        lea.l   camd_MidiListLock(a5),a0
        JSRLIB  ObtainSemaphore
		bset.b	#(1<<MidiListBit),1+camd_PrivateLockBits(a5)
        endm

UnlockMidiList macro
		bclr.b	#(1<<MidiListBit),1+camd_PrivateLockBits(a5)
        lea.l   camd_MidiListLock(a5),a0
        JSRLIB  ReleaseSemaphore
        endm

****************************************************************
*
*   CopyMem (macro)
*
*   SYNOPSIS
*       CopyMem
*
*   FUNCTION
*       Copies memory.  Small copies are performed inline.
*       Larger copies are passed to the system function
*       CopyMem().
*
*   INPUTS
*       A0 - source
*       A1 - dest
*       D0 - length
*       A6 - SysBase
*
*   RESULTS
*       A0 - updated source ptr
*       A1 - updated dest ptr
*
****************************************************************

        xref    _LVOCopyMem

MaxShortCopy equ 20

CopyMem macro

        moveq.l #MaxShortCopy,d1
        cmp.l   d1,d0                   ; if D0 <= MaxShortCopy, do inline copy
        bls.s   cpm\@_inline

        movem.l d0/a0-a1,-(sp)
        jsr     _LVOCopyMem(a6)         ; (can't use a macro here 'cuz of as bug)
        movem.l (sp)+,d0/a0-a1
        add.l   d0,a0
        add.l   d0,a1
        bra.s   cpm\@_done

cpm\@_loop
        move.b  (a0)+,(a1)+
cpm\@_inline
        dbra    d0,cpm\@_loop

cpm\@_done

        endm



****************************************************************
*
*   Push / Pop (macro)
*
*   SYNOPSIS
*       Push [regset]
*       Pop
*
*   FUNCTION
*       Push an optional set of registers using MOVEM for
*       recovery by a matching Pop macro.  These macros nest,
*       but must be balanced (i.e. can't have 1 Push with 2
*       Pops [yet]).
*
*   INPUTS
*       regset - a register set like a2/d2-d3
*
*   RESULTS
*       Registers, if any, are pushed onto the stack by Push
*       and recovered by Pop.
*
****************************************************************

push_Count                  set     0       ; index to push_Reg
push_Nest                   set     0       ; index to push_Index

Push                        macro   ; regset

                            ifgt    NARG-1
                            fail    !!!! Too many arguments to PUSHM !!!!
                            endc

                            ifgt    NARG
push_Reg\*VALOF(push_Count) reg     \1      ; only set _RegNN and push if there's an arg
                            movem.l push_Reg\*VALOF(push_Count),-(sp)
                            endc

push_Index\*VALOF(push_Nest) set     push_Count
push_Count                  set     push_Count+1
push_Nest                   set     push_Nest+1

                            endm


Pop                         macro

                            ifle    push_Nest
                            fail    !!!! POPM without PUSHM !!!!
                            endc

push_Nest                   set     push_Nest-1
push_TmpIndex\@             set     push_Index\*VALOF(push_Nest)

                            ifd     push_Reg\*VALOF(push_TmpIndex\@)
                            movem.l (sp)+,push_Reg\*VALOF(push_TmpIndex\@)
                            endc

                            endm

    endc
