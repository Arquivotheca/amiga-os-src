        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "libraries/dos.i"
	INCLUDE "utility/tagitem.i"
        INCLUDE "accounts_rev.i"
        INCLUDE "accountsbase.i"

        LIST

;---------------------------------------------------------------------------

        XREF    _AllocUserInfo
        XREF    _AllocGroupInfo
        XREF    _FreeUserInfo
        XREF	_FreeGroupInfo
        XREF	_VerifyUser
        XREF	_MemberOf
        XREF	_NameToUser
        XREF	_NameToGroup
        XREF	_IDToUser
        XREF	_IDToGroup
        XREF	_NextUser
        XREF	_NextGroup
        XREF	_NextMember
	XREF	_ECrypt
	XREF	_VerifyUserCrypt
        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD           ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                  ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                 ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT            ; UBYTE RT_FLAGS
        DC.B    VERSION                 ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY              ; UBYTE RT_TYPE
        DC.B    0                       ; BYTE  RT_PRI
        DC.L    LibName                 ; APTR  RT_NAME
        DC.L    LibId                   ; APTR  RT_IDSTRING
        DC.L    LibInitTable            ; APTR  RT_INIT

LibName DC.B 'accounts.library',0
LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    AccountsLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    LibDataTable
        DC.L    LibInit

V_DEF   MACRO
        DC.W    \1+(*-LibFuncTable)
        ENDM

LibFuncTable:
        DC.W    -1
        V_DEF   LibOpen
        V_DEF   LibClose
        V_DEF   LibExpunge
        V_DEF   LibReserved

        V_DEF   _AllocUserInfo
        V_DEF   _AllocGroupInfo
        V_DEF	_FreeUserInfo
        V_DEF	_FreeGroupInfo
        V_DEF	_VerifyUser
        V_DEF	_MemberOf
        V_DEF	_NameToUser
        V_DEF	_NameToGroup
        V_DEF	_IDToUser
        V_DEF	_IDToGroup
        V_DEF	_NextUser
        V_DEF	_NextGroup
        V_DEF	_NextMember
	V_DEF	_ECrypt
	V_DEF	_VerifyUserCrypt

        DC.W   -1

LibDataTable:
        INITWORD   LIB_REVISION,REVISION
        DC.W       0

        CNOP    0,4

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l a0/a5,-(sp)				; Preserve a few registers
        move.l  d0,a5					; ACNTSBase -> a5
        move.l  a6,ACNTS_SysBase(a5)			; Save SysBase
        move.l  a0,ACNTS_SegList(a5)			; Save Seglist

        clr.l   ACNTS_NIPCBase(a5)			; Clear out pointers
        clr.l	ACNTS_Entity(a5)			; "" ""
  	clr.l	ACNTS_AccEntity(a5)			; "" ""
	move.l	#$C0EDBABE,ACNTS_HTag(a5)
	lea.l	ACNTS_Lock(a5),a0			; Get pointer to our semaphore
	CALL	InitSemaphore				; and initialize it

	move.l	#37,d0					; V37 required
	lea.l	DosName(pc),a1				; "dos.library"
	CALL	OpenLibrary				; OpenLibrary
	move.l	d0,ACNTS_DOSBase(a5)			; Save DOSBase
	beq.b	1$					; If it failed, exit with 0 in d0

	move.l	#37,d0					; V37
	lea.l	UtilityName(pc),a1			; "utility.library"
	CALL	OpenLibrary				; OpenLibrary()
	move.l	d0,ACNTS_UtilityBase(a5)		; Save UtilityBase
	bne.b	2$					; If okay, return ACNTSBase

	move.l	ACNTS_DOSBase(a5),a1			; Opeing Utility failed, so
	CALL	CloseLibrary				; close dos.library and
	clr.l	d0					; exit with 0 in d0
	bra.b	1$

2$      move.l  a5,d0					; return ACNTSBase if DOS opened
1$      movem.l (sp)+,a0/a5				; restore registers
        rts

LIBVERSION	EQU  37
NIPCName	DC.B "nipc.library",0
UtilityName	DC.B "utility.library",0
AccServName	DC.B "Accounts Server",0
DosName		DC.B "dos.library",0
HostEnvName	DC.B "Envoy/AccountsServer",0
        CNOP    0,4

EntityTags	DC.L TAG_DONE

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has AccountsBase, task switching is disabled
; Returns 0 for failure, or ServicesBase for success.
LibOpen:
	addq.w	#1,LIB_OPENCNT(a6)			; One more opener
	tst.l	ACNTS_AccEntity(a6)			; Are we connected?
	bne	4$					; Yes, continue

	; Enforce single threading since opening nipc.library
	; will cause a Wait().

        lea.l   ACNTS_Lock(a6),a0			; Address of our Semaphore
        move.l  a5,-(sp)				; Save a5
        movea.l	a6,a5					; AccountsBase -> a5
        move.l  ACNTS_SysBase(a6),a6			; SysBase -> a6
        CALL    ObtainSemaphore				; Lock everyone else out

	; Open nipc.library.

        lea.l   NIPCName(pc),a1				; "nipc.library"
        clr.l   d0					; any version
        CALL    OpenLibrary				; Open it...
        move.l  d0,ACNTS_NIPCBase(a5)			; ...and save it's library base
        beq.s   1$					; Oops! nipc couldn't open!

        ; Create our entity

        move.l	d0,a6					; NIPCBase -> a6
        lea.l	EntityTags(pc),a0			; Misc entity stuff
        CALL	CreateEntityA				; Create our entity...
        move.l	d0,ACNTS_Entity(a5)			; ...and save it's pointer
        beq.s	3$					; Oops! Couldn't create our Entity!

	; See who to connect to

	movea.l	ACNTS_DOSBase(a5),a6			; DOSBase -> a6
	lea.l	HostEnvName(pc),a0
	move.l	a0,d1
	lea.l	ACNTS_HostName(a5),a0
	move.l	a0,d2
	move.l	#128,d3
	moveq.l	#0,d4
	CALL	GetVar
	move.l	d0,ACNTS_GVCode(a5)
	bmi.b	5$
	lea.l	ACNTS_HostName(a5),a0
	bra.b	6$

        ; Try to connect to the local accounts server

5$      suba.l	a0,a0					; Clear a0
6$      lea.l	AccServName(pc),a1			; "AccountsServer"
        movea.l	ACNTS_Entity(a5),a2			; ACNTS_Entity
        movea.l	ACNTS_NIPCBase(a5),a6
        CALL	FindEntity				; Find it...
	move.l	d0,ACNTS_AccEntity(a5)			; Save it's pointer
	bne.s	1$

	; Shoot...we couldn't connect so delete our entity.

	movea.l	ACNTS_Entity(a5),a0			; Pointer to our entity
	CALL	DeleteEntity				; Delete it.
;        clr.l	ACNTS_Entity(a5)

	; We couldn't create our entity. Close nipc.library
	; and clear the pointer to it's library base.

3$      movea.l	a6,a1					; NIPCBase -> a1
	movea.l	ACNTS_SysBase(a5),a6			; Get SysBase back in a6
	CALL	CloseLibrary				; Close it...
;	clr.l	ACNTS_NIPCBase(a5)			; ...and clear it's pointer

        ; Release our semaphore...

1$      movea.l	ACNTS_SysBase(a5),a6			; SysBase -> a6
	lea.l   ACNTS_Lock(a5),a0			; Get address of semaphore
        CALL    ReleaseSemaphore			; Release it...
        movea.l	a5,a6					; AccountsBase back in a6
        move.l  (sp)+,a5				; fix a5

2$      tst.l	ACNTS_AccEntity(a6)			; Did we connect?
	bne.s	4$					; Yes...
	clr.l	d0					; Nope....fail the OpenLibrary() call
	subq.w	#1,LIB_OPENCNT(a6)			; Sorry, back off...
	rts						; Return

4$	bclr    #LIBB_DELEXP,LIB_FLAGS(a6)		; Clear delayed expunge
        move.l  a6,d0					; AccountsBase -> d0
        rts						; return

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has ServicesBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
; due to delayed expunge bit being set
LibClose:
        subq.w  #1,LIB_OPENCNT(a6)			; One less opener
        bne.s	5$					; People still have us open

        ; First, we need to do some locking.

        move.l	a5,-(sp)				; Save a5
        movea.l	a6,a5					; AccountsBase -> a5
        movea.l	ACNTS_SysBase(a6),a6			; SysBase -> a6
        lea.l	ACNTS_Lock(a5),a0			; Pointer to our semaphore
        CALL	ObtainSemaphore				; Lock everyone else out

        ; Okay, no more openers, so let's disconnect from
        ; the accounts server and close nipc.library.  This is
        ; so that we never have to Wait() during expunge.

	movea.l	ACNTS_NIPCBase(a5),a6			; NIPCBase -> a6
	movea.l	ACNTS_AccEntity(a5),a0			; AccEntity -> a0
	CALL	LoseEntity				; Disconnect

	; Now we're disconnected, so let's delete our
	; entity

	movea.l	ACNTS_Entity(a5),a0			; Our Entity
	CALL	DeleteEntity				; Delete it.

	; Now close nipc.library

	movea.l	a6,a1					; NIPCBase -> a1
	movea.l	ACNTS_SysBase(a5),a6			; SysBase -> a6
	CALL	CloseLibrary				; Close nipc.library

	; Unlock

	lea.l	ACNTS_Lock(a5),a0			; Pointer to our semaphore
	CALL	ReleaseSemaphore			; Unlock it

	; Clear pointers

	clr.l	ACNTS_NIPCBase(a5)
	clr.l	ACNTS_Entity(a5)
	clr.l	ACNTS_AccEntity(a5)

	movea.l	a5,a6					; AccountsBase -> a6
	movea.l	(sp)+,a5				; restore a5

        ; if delayed expunge bit set, then try to get rid of the library
        btst    #LIBB_DELEXP,LIB_FLAGS(a6)		; should we expunge?
        bne.s   DoExpunge				; Yes...

        ; delayed expunge not set, so stick around
5$      moveq   #0,d0					; Don't free us...
        rts						; return

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has ServicesBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
        tst.w   LIB_OPENCNT(a6)				; Any openers?
        beq.s   DoExpunge				; Nope...expunge.

        bset    #LIBB_DELEXP,LIB_FLAGS(a6)		; Set the delayed expunge bit
        moveq   #0,d0					; and return failure
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)				; preserve registers
        move.l  a6,a5					; ServicesBase -> a5
        move.l  ACNTS_SegList(a5),d2			; Seglist in d2

        move.l  a5,a1					; ServicesBase -> a1
        REMOVE						; Remove() it from the library
        						; list

        move.l  ACNTS_SysBase(a5),a6			; SysBase -> a6

	movea.l	ACNTS_UtilityBase(a5),a1		; UtilityBase -> a1
	CALL	CloseLibrary				; Close utility.library

        movea.l	ACNTS_DOSBase(a5),a1			; DOSBase -> a1
        CALL	CloseLibrary				; Close dos.library

        move.l  a5,a1					; ServicesBase -> a1
        moveq   #0,d0					; clear d0
        move.w  LIB_NEGSIZE(a5),d0			; bytes before the library base
        sub.l   d0,a1					; a1 points to start of function table
        add.w   LIB_POSSIZE(a5),d0			; add length beyond the library base
        CALL    FreeMem					; Free the library base

        move.l  d2,d0					; return seglist in d0
        movem.l (sp)+,d2/a5/a6				; restore registers...
        rts						; ...and exit

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

        END
