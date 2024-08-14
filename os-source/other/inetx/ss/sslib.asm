; ------------------------------------------------------------------------------
; sslib.asm       initial round of shared socket lib stuff   Manx 5.0e !!!
;
; $Locker$
;
; $Id$
;
; $Revision$
;
; $Header$
;
;
; assemble: as sslib.asm
; link:     ln -o ss.library sslib.o func2.o < _c_funcs.o ...> -lc
;
; bj
;-------------------------------------------------------------------------------

        SECTION section

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libsupp.i"
        INCLUDE "sslib_def.i
        INCLUDE "sslib_rev.i"

        LIST

; --------------------------------------------------------

        XDEF    libInit                 ; lib init
        XDEF    ssname
        XDEF    Open                    ; library standard routines
        XDEF    Close
        XDEF    Expunge
        XDEF    ExtFunc
        XDEF    Func0                   ; library user routines
        XDEF    Func1
        XDEF    _Func2                 ; this one in C
        XDEF    Func3
        XDEF    Func4
        XDEF    Func5
        XDEF    Func6
        XDEF    Func7
        XREF    _AbsExecBase
        XDEF    _sSBase
        XDEF    _iNetBase

; internal functions
       
        XREF    _testfunc           ;    in C

        XLIB    OpenLibrary
        XLIB    CloseLibrary
        XLIB    Alert
        XLIB    Remove
        XLIB    AllocMem
        XLIB    FreeMem
        XLIB    FindTask

; -----------------------------------------------------------
; first executable location. This returns an error in case
; someone tries to run the library as a program.
;

LibStart:
        CLEAR d0
        rts

; -----------------------------------------------------------
; various defs and equates
; -----------------------------------------------------------

PRI             EQU     0               ; lib priority - position in 
                                        ; system library list

; =====================================================================
; get the following from sslib_rev.i !!
; VERSION       EQU     1               ; major version number
; REVISION      EQU     1               ; particular revision


;--------------------------------------------------------
; A romtag structure
;---------------------------------------------------------

initLibDescrip:                         ; structure RT,0
        dc.w    RTC_MATCHWORD           ; UWORD RT_MATCHWORD
        dc.l    initLibDescrip          ; APTR RT_MATCHTAG
        dc.l    EndLibCode              ; APTR RT_ENDSKIP
        dc.b    RTF_AUTOINIT            ; UBYTE RT_FLAGS
        dc.b    VERSION                 ; UBYTE RT_VERSION
        dc.b    NT_LIBRARY              ; UBYTE RT_TYPE
        dc.b    PRI                     ; UBYTE RT_PRI
        dc.l    ssname                  ; APTR RT_NAME
        dc.l    libIdString             ; APTR RT_IDSTRING
        dc.l    libInit                 ; APTR RT_INIT
                                        ; LABEL RT_SIZE

ssname  SSLIBNAME                       ; library name
inetname INETLIBNAME

_sSBase:
        dc.l  0

_iNetBase:
        dc.l  0

sockstring
        dc.b  'Socket Client',10,0

;---------------------------------------------------------

libIdString     VSTRING                 ; from sslib_rev.i

dosName DOSNAME                         ; name of library to open

        ds.w    0                       ; word alignment

; -------------------------------------------------------'
; The romtag specifies "RTF_AUTOINIT". The RT_INIT structure field
; points to the following table. If RTF-AUTOINIT was not set, then
; RT_INIT would point to an initialization routine to run.
; These data are used by the loading program as parameters to
; AddLibrary() library initialization.
; ------------------------------------------------------------
libInit:
        dc.l    SSLib_Sizeof            ; data space size
        dc.l    funcTable               ; pointer to func initializer
s
        dc.l    dataTable               ; pointer to data Initializers
        dc.l    initRoutine             ; routine to run

;-------------------------------------------------------------
; table of addresses of sslib functions
; -----------------------------------------------------------
funcTable:
        ; standard system routines - These MUST always be included.

        dc.l    Open
        dc.l    Close
        dc.l    Expunge
        dc.l    ExtFunc
            ; ss.library's  definitions
        dc.l    Func0
        dc.l    Func1
        dc.l    _Func2   ; this is in C
        dc.l    Func3
        dc.l    Func4
        dc.l    Func5
        dc.l    Func6
        dc.l    Func7
            ; function table end marker
        dc.l    -1

; ------------------------------------------------------------
; this data table inits static data structures. The format is
; specified in the Exec/initstruct routines manual page.
; The first arg is the offset from the library base for this
; byte/word/long.  The second arg is the value to put in that
; cell.  The table is null terminated.
;-------------------------------------------------------------

dataTable:

        INITBYTE        LH_TYPE,NT_LIBRARY
        INITLONG        LN_NAME,ssname
        INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
        INITWORD        LIB_VERSION,VERSION
        INITWORD        LIB_REVISION,REVISION
        INITLONG        LIB_IDSTRING,libIdString
        dc.l            0

; ------------------------------------------------------------
; This is the initialization routine. It is called after the library
; has been allocated.
;
; If it returns non-zero then the library will be linked into the
; library list at position given by priority.
; --------------------------------------------------------------


initRoutine:            ; ( D0: sslib. ptr.   A0: segment list )

            ; get the library pointer into A5
        movem.l  a4/a5,-(sp)
        move.l  d0,a5

            ; save pointer to exec
        move.l  a6,ss_SysLib(a5)

            ; save a pointer to loaded code segment list
        move.l  a0,ss_SegList(a5)

            ;open the DOS library
        lea     dosName(pc),a1          ; ptr to 'dos.library' string
        CLEAR   d0                      ; any version will do
        CALLSYS OpenLibrary
        move.l  d0,ss_DosLib(a5)        ; success.
        bne.s   1$

            ; can't open DOS
        ALERT   AG_OpenLib!AO_DOSLib    ; blow up the machine!!!! :) :)

1$:

            ;open the inet.library
        lea     _iNetBase,a4
        lea     inetname(pc),a1         ; ptr to 'inetlibrary' string
        CLEAR   d0                      ; any version will do
        CALLSYS OpenLibrary
        move.l  d0,(a4)
        move.l  d0,ss_InetLib(a5)        ; success.
        bne.s   8$

            ; can't open inet.library
        ALERT   AG_OpenLib!AO_Unknown    ; blow up the machine!!!! :) :)

8$:
            ; build the static data that we need
            ;
            ; ####### library specific initialiaztions go here ########

        move.l  a5,d0                   ; lib base in d0 = ok init
        lea _sSBase,a5
        move.l  d0,(a5)                 ; lib base into a global
        
bail:
        movem.l  (sp)+,a4/a5
        rts

;------------------------------------------------------------
; System interface commands begin here
;
; Calling OpenLibrary/CloseLibrary/RemoveLibrary trranslates into a call
; to the routines Open/Close/Expunge.
;
; The library base pointer is in a6. Task switching has been turned off
; while in these routines (via Forbid/Permit), so don't hang about.
; -------------------------------------------------------------

; ------------------------------------------------------------
; Open returns the library pointer in D0 if the open was successful.
; If the open failed then NULL/0 is returned.
; --------------------------------------------------------------

Open:       ; (libptr: A6,  version: D0)

            ; increment open counter
        addq.w  #1,LIB_OPENCNT(a6)

            ;prevent delayed expunges
        bclr    #LIBB_DELEXP,ss_Flags(a6)

        move.l  a6,d0
        rts

; ------------------------------------------------------------------
; If the library is no longer open, Close returns the segment list if
; the delayed expunge flag is set. Otherwise, Close returns NULL.
; -----------------------------------------------------------------

Close:      ; (libptr: a6)

        movem.l a5/a6,-(sp)


            ; set the return value
        CLEAR   d0

            ; decrement library open counter
        subq.w  #1,LIB_OPENCNT(a6)

            ; anyone got us open ?
        bne.s   1$

            ; any delayed expunge pending?
        btst    #LIBB_DELEXP,ss_Flags(a6)
        beq.s   1$

            ; do the expunge
        bsr     Expunge

1$:
        movem.l (sp)+,a5/a6
        rts

; ---------------------------------------------------------------------
; If the library is no longer open, then Expunge returns the segment
; list for the library code.  Otherwise, the delayed expunge flag is
; set and NULL is returned in d0. Expunge should never Wait() or take
; a long time to complete.
; -------------------------------------------------------------------

Expunge:    ; (libptr: A6)
        movem.l d2/a5/a6,-(sp)       
        move.l  a6,a5                   ; save sslib ptr
        move.l  ss_SysLib(a5),a6        ; get exec ptr into a6

            ;anyone got us open ?
        tst.w   LIB_OPENCNT(a5)
        beq     1$

            ;if still open, set delayed expunge flag
        bset    #LIBB_DELEXP,ss_Flags(a5)
        CLEAR   d0
        bra.s   Expunge_End

1$:
            ;we can remove ourselves here
        move.l  ss_SegList(a5),d2       ; a5 = sslib ptr

            ;unlink from library list
        move.l  a5,a1
        CALLSYS Remove


;=================================================
; library specific closings here
; ================================================


            ;close the inet library
        move.l  ss_InetLib(a5),a1        ; a5 = sslib
        CALLSYS CloseLibrary

            ;close the DOS library
        move.l  ss_DosLib(a5),a1        ; a5 = sslib
        CALLSYS CloseLibrary

            ;free our memory
        CLEAR   d0                      ; !!
        move.l  a5,a1                   ; a1 = sslib addr
        move.w  LIB_NEGSIZE(a5),d0      ; here we calc the size
                                        ; of the library so we 
        sub.l   d0,a1                   ; can call FreeMem(size)
        add.w   LIB_POSSIZE(a5),d0      ; on ourselves.

        CALLSYS FreeMem                 ; which we do...

            ; set up our return value
        move.l  d2,d0                   ; d2 = sslib ptr

Expunge_End:
        movem.l (sp)+,d2/a5/a6          ; put things back
        rts


; -----------------------------------------------------------------
; ExtFunc just returns 0 in D0
; -----------------------------------------------------------------

ExtFunc:
        CLEAR   d0
        rts


; -----------------------------------------------------------------
; The library specific commands begin here.
;
; MyFuncs 0 - 5
; -----------------------------------------------------------------

Func0:
        move.l #0,d0
        rts


Func1:
        
        ; move.l  a1,d0
        jsr _testfunc
        rts


Func3:
        move.l  a3,d0
        rts


Func4:
        move.l  a4,d0
        rts


Func5:
        move.l  a5,d0
        rts

; --------------------------------------------------------
; MyFunc 6  returns FindTask(0) result
; --------------------------------------------------------

Func6:
        movem.l a5/a6,-(sp)             ; save stuff
        move.l  a6,a5                   ; sslib into a5
        move.l  ss_SysLib(a5),a6        ; exec lib into a6
        move.l  #0,a1                   ; FindTask( 0L )
        CALLSYS FindTask
        movem.l (sp)+,a5/a6             ; put it back
        rts

; --------------------------------------------------------
; MyFunc7 returns a value of A7 register in the d0 register
; --------------------------------------------------------

Func7:
        move.l  a7,d0                   ; a7 register
        rts


; ----------------------------------------------------------------
; this is the end of the library code
; ----------------------------------------------------------------
        ds.w    0                       ; word alignment

EndLibCode:

        END

;   ************** end of file sslib.asm ****************
