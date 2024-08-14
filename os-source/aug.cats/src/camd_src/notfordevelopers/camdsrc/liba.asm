*************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* liba.asm - main library assembly module for camd
*
************************************************************************

      nolist
        include "exec/types.i"

        include "camdlib.i"
      list


        idnt    liba.asm

            ; internal
        xdef    FuncTable
        xdef    InitLib

            ; external
        xref    InitCEnv
        xref    _InitLibC
        xref    _CleanUpC


****************************************************************
*
*   FuncTable
*
*   C functions are prefixed w/ _LIB
*
****************************************************************

    ; add a library function to the function table
LibFunc macro   ; label
        xref    \1
        dc.l    \1
        endm

        ; !!! clean this up

FuncTable
            ; system required library functions
        dc.l    OpenLib
        dc.l    CloseLib
        dc.l    ExpungeLib
        dc.l    NullFunc

            ; public library functions

                    ; Locks
        LibFunc _LIBLockCAMD            ; (lock)(d0)
        LibFunc _LIBUnlockCAMD          ; (lock)(d0)

                    ; MidiNode
        LibFunc _LIBCreateMidiA         ; (tags)(a0)
        LibFunc _LIBDeleteMidi          ; (mi)(a0)
        LibFunc _LIBSetMidiAttrsA       ; (mi,tags)(a0,a1)
        LibFunc _LIBGetMidiAttrsA       ; (mi,attr)(a0/a1)
        LibFunc _LIBNextMidi            ; (mi)(a0)
        LibFunc _LIBFindMidi            ; (name)(a1)
        LibFunc _LIBFlushMidi           ; (mi)(a0)

                    ; MidiLink
        LibFunc _LIBAddMidiLinkA        ; (mi,type,tags)(a0,d0,a1)
        LibFunc _LIBRemoveMidiLink      ; (ml)(a0)
        LibFunc _LIBSetMidiLinkAttrsA   ; (ml,tags)(a0/a1)
        LibFunc _LIBGetMidiLinkAttrsA   ; (mi,attr)(a0/a1)
        LibFunc _LIBNextClusterLink     ; (mc,ml)(a0/a1,d0)
        LibFunc _LIBNextMidiLink        ; (mi,ml)(a0/a1,d0)
        LibFunc _LIBMidiLinkConnected   ; (ml)(a0)

                    ; MidiCluster
        LibFunc _LIBNextCluster         ; (mc)(a0)
        LibFunc _LIBFindCluster         ; (name)(a0)

                    ; Message
        LibFunc PutMidi                 ; (ml,msgdata)(a0,d0)
        LibFunc GetMidi                 ; (mi,msg)(a0/a1)
        LibFunc WaitMidi                ; (mi,msg)(a0/a1)
        LibFunc PutSysEx                ; (ml,buffer)(a0,a1)
        LibFunc GetSysEx                ; (mi,buffer,len)(a0/a1,d0)
        LibFunc QuerySysEx              ; (mi)(a0)
        LibFunc SkipSysEx               ; (mi)(a0)
        LibFunc GetMidiErr              ; (mi)(a0)
        LibFunc MidiMsgType             ; (msg)(a0)
        LibFunc MidiMsgLen              ; (status)(d0)
        LibFunc ParseMidi               ; (ml,buffer,length)(a0/a1,d0)

                    ; Device
        LibFunc _LIBOpenMidiDevice      ; (name)(a0)
        LibFunc _LIBCloseMidiDevice     ; (mdd)(a0)

					; External Functions
		LibFunc _LIBRethinkCAMD			; ()
		LibFunc _LIBStartClusterNotify  ; (node)(a0)
		LibFunc _LIBEndClusterNotify    ; (node)(a0)

;		PutMidiNoWait(struct MidiLink *ml, MidiMsg *msg); /* doesn't wait if receive buffer full...? */

            ; end of function list
        dc.l    -1


****************************************************************
*
*   InitLib
*
*   FUNCTION
*       Gets called by library loader once normal library node
*       initialization has been done.
*
*   INPUTS
*       D0 - library pointer
*       A0 - segment list
*       A6 - SysBase
*
*   RESULTS
*       D0 - library pointer on success, NULL on failure
*
****************************************************************

InitLib
        Push    a5
        move.l  d0,a5                       ; A5 = CamdBase
        move.l  a6,camd_SysBase(a5)         ; set SysBase
        move.l  a0,camd_SegList(a5)

        bsr     InitCEnv                    ; do compiler specific C inits

        move.l  a5,a0
        bsr     _InitLibC                   ; init C data (A0 = CamdBase)
                                            ; this is a C function declared w/ __saveds __asm interface
        tst.w   d0
        beq.s   il_fail

        move.l  a5,d0                       ; set return value
        bra.s   il_rtn

il_fail
        bsr.s   CleanUp
        CLEAR   d0

il_rtn
        Pop
        rts


****************************************************************
*
*   OpenLib
*
*   FUNCTION
*       Open Library - called from user's OpenLibrary() function
*       call.
*
*   INPUTS
*       A6 - CamdBase
*       D0 - version
*
*   RESULT
*       D0 - Library Base if OK, or 0 if error
*
****************************************************************

OpenLib
        addq.w  #1,LIB_OPENCNT(a6)
        bclr.b  #LIBB_DELEXP,LIB_FLAGS(a6)
        move.l  a6,d0
        rts


****************************************************************
*
*   CloseLib
*
*   FUNCTION
*       Close Library
*
*   INPUTS
*       A6 - CamdBase
*
*   RESULT
*       D0 - Segment list if expunging, 0 otherwise
*
****************************************************************

CloseLib
        CLEAR   d0
        subq.w  #1,LIB_OPENCNT(a6)
        btst.b  #LIBB_DELEXP,LIB_FLAGS(a6)
        bne.s   ExpungeLib
        rts


****************************************************************
*
*   ExpungeLib
*
*   FUNCTION
*       Expunge Library
*
*   INPUTS
*       A6 - CamdBase
*
*   RESULT
*       D0 - Segment list to expunge, 0 otherwise
*
****************************************************************

ExpungeLib
        Push    d2/a5/a6
        move.l  a6,a5                   ; A5 = CamdBase
        move.l  camd_SysBase(a5),a6     ; A6 = SysBase

        tst.w   LIB_OPENCNT(a5)         ; is library still open?
        bne.s   exp_delay

            ; do local expunge work here (currently none)

        move.l  camd_SegList(a5),d2     ; D2 = our segment list

        move.l  a5,a1                   ; remove our library node
        REMOVE

        bsr.s   CleanUp

        move.l  d2,d0                   ; set D0 to segment list for return
        bra.s   exp_ret

exp_delay
        bset.b  #LIBB_DELEXP,LIB_FLAGS(a5)
        CLEAR   d0                      ; clear D0 to avoid UnloadSeg()

exp_ret
        Pop
        rts


****************************************************************
*
*   CleanUp
*
*   FUNCTION
*       CleanUp resources allocated by InitLib.  Called by
*       InitLib on failure and ExpungeLib.  Guaranteed to
*       be called exactly once during life of library.
*
*   INPUTS
*       A5 - CamdBase
*       A6 - SysBase
*
*   RESULT
*       None
*
****************************************************************

CleanUp
        bsr     _CleanUpC               ; cleanup C data

        CLEAR   d0                      ; free our library structure
        move.w  LIB_POSSIZE(a5),d0      ; D0 = pos size
        CLEAR   d1
        move.w  LIB_NEGSIZE(a5),d1      ; D1 = neg size
        move.l  a5,a1
        sub.l   d1,a1
        add.l   d1,d0
        JSRLIB  FreeMem

        rts


****************************************************************
*
*   NullFunc
*
*   FUNCTION
*       Extra standard lib vector.
*
*   INPUTS
*       A6 - CamdBase
*
*   RESULT
*       D0 - 0
*
****************************************************************

NullFunc
        CLEAR   d0
        rts

        end
