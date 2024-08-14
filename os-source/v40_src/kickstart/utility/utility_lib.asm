        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
	INCLUDE	"exec/ables.i"

        INCLUDE "utility_rev.i"
        INCLUDE "utilitybase.i"
	INCLUDE	"name.i"

        LIST

;---------------------------------------------------------------------------

	XREF	FindTagItem
	XREF	GetTagData
	XREF	PackBoolTags
	XREF	NextTagItem
	XREF	FilterTagChanges
	XREF	MapTags
	XREF	AllocateTagItems
	XREF	CloneTagItems
	XREF	FreeTagItems
	XREF	RefreshTagItemClones
	XREF	TagInArray
	XREF	FilterTagItems

	XREF	CallHookPkt

	XREF	Amiga2Date
	XREF	Date2Amiga
	XREF	CheckDate

	IFD	MC68000
	XREF	SMult32S
	XREF	UMult32S
	XREF	SDivMod32S
	XREF	UDivMod32S
	ENDC

	XREF	SMult32H
	XREF	UMult32H
	XREF	SDivMod32H
	XREF	UDivMod32H

	XREF	Stricmp
	XREF	Strnicmp
	XREF	ToUpper
	XREF	ToLower

	XREF	ApplyTagChanges

	IFD 	MC68000
	XREF	SMult64S
	XREF	UMult64S
	ENDC

	XREF	SMult64H
	XREF	UMult64H

	XREF	PackStructureTags
	XREF	UnpackStructureTags

	; NameSpace functions!
	XREF	AddNamedObject
	XREF	AllocNamedObjectA
	XREF	AttemptRemNamedObject
	XREF	FindNamedObject
	XREF	FreeNamedObject
	XREF	NamedObjectName
	XREF	ReleaseNamedObject
	XREF	RemNamedObject

	XREF	_LVOSetFunction
	XREF	_LVOFreeMem
	XREF	_intena

        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
; Start:
; only useful if disk-based
;        moveq   #-1,d0
;        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                     ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                    ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT!RTF_COLDSTART ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY                 ; UBYTE RT_TYPE
        DC.B    103                        ; BYTE  RT_PRI
        DC.L    LibName                    ; APTR  RT_NAME
        DC.L    LibId                      ; APTR  RT_IDSTRING
        DC.L    LibInitTable               ; APTR  RT_INIT

LibName DC.B 'utility.library',0
LibId   VSTRING

        CNOP    0,2

LibInitTable:
        DC.L    UtilityLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    0
        DC.L    LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
        V_DEF	LibOpen
        V_DEF	LibClose
        V_DEF	LibExpunge
        V_DEF	LibReserved

	V_DEF	FindTagItem
	V_DEF	GetTagData
	V_DEF	PackBoolTags
	V_DEF	NextTagItem
	V_DEF	FilterTagChanges
	V_DEF	MapTags
	V_DEF	AllocateTagItems
	V_DEF	CloneTagItems
	V_DEF	FreeTagItems
	V_DEF	RefreshTagItemClones
	V_DEF	TagInArray
	V_DEF	FilterTagItems

	V_DEF	CallHookPkt
	V_DEF	LibReserved

	V_DEF	LibReserved
	V_DEF	Amiga2Date
	V_DEF	Date2Amiga
	V_DEF	CheckDate

	IFD	MC68000
	V_DEF	SMult32S
	V_DEF	UMult32S
	V_DEF	SDivMod32S
	V_DEF	UDivMod32S
	ELSE
	V_DEF	SMult32H
	V_DEF	UMult32H
	V_DEF	SDivMod32H
	V_DEF	UDivMod32H
	ENDC

	V_DEF	Stricmp
	V_DEF	Strnicmp
	V_DEF	ToUpper
	V_DEF	ToLower

	V_DEF	ApplyTagChanges
	V_DEF	LibReserved   ; for MergeTagItems

	IFD	MC68000
	V_DEF	SMult64S
	V_DEF	UMult64S
	ELSE
	V_DEF	SMult64H
	V_DEF	UMult64H
	ENDC

	V_DEF	PackStructureTags
	V_DEF	UnpackStructureTags

	; NameSpace functions!
	V_DEF	AddNamedObject
	V_DEF	AllocNamedObjectA
	V_DEF	AttemptRemNamedObject
	V_DEF	FindNamedObject
	V_DEF	FreeNamedObject
	V_DEF	NamedObjectName
	V_DEF	ReleaseNamedObject
	V_DEF	RemNamedObject

	; Unique sequence number routine...
	V_DEF	GetUniqueID

	; empty functions for setpatch!
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved

        DC.W   -1

	IFD	MC68000
FunctionMap:
	DC.W	_LVOSMult32
	DC.L	SMult32H

	DC.W	_LVOUMult32
	DC.L	UMult32H

	DC.W	_LVOSDivMod32
	DC.L	SDivMod32H

	DC.W	_LVOUDivMod32
	DC.L	UDivMod32H

	DC.W	_LVOSMult64
	DC.L	SMult64H

	DC.W	_LVOUMult64
	DC.L	UMult64H
	ENDC

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l d0/d2/a0/a1/a2/a3,-(sp)
        move.l  d0,a3
        move.l  a6,ub_SysBase(a3)

	move.w	#REVISION,LIB_REVISION(a3)

	lea	NullS(pc),a0		; Null string...
	lea	MyTags(pc),a1		; My tags...

	exg	a6,a3			; Our library now...
	CALL	AllocNamedObjectA	; Create the root namespace...
	move.l	d0,ub_MasterSpace(a6)	; Store it
	exg	a6,a3			; Back to normal

	IFD	MC68000
        move.w	AttnFlags(a6),d0
        and.w	#AFF_68020!AFF_68030!AFF_68040,d0
        beq.s	2$

; replace the 68000 math functions by the 68020 versions
	move.l	#5,d2
	lea	FunctionMap(pc),a2

1$:	move.l	a3,a1			; load UtilityBase
	move.w	(a2)+,a0		; load a function offset
	move.l	(a2)+,d0		; load a the replacement function POINTER
	CALL	SetFunction		; do the magic
	dbra	d2,1$			; next one please...

	ENDC

2$:	movem.l (sp)+,d0/d2/a0/a1/a2/a3
	rts

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has UtilityBase, task switching is disabled
; Returns 0 for failure, or UtilityBase for success.
LibOpen:
        addq.w  #1,LIB_OPENCNT(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has UtilityBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set, which won't happen for a ROM
;   module
LibClose:

	subq.w	#1,LIB_OPENCNT(a6)

*** FALLS THROUGH !!!

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has UtilityBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit. Won't happen since this is
; a ROM module
LibExpunge:

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

MyTags:	dc.l	ANO_NameSpace,1
NullS:	dc.l	0

******* utility.library/GetUniqueID *******************************************
*
*   NAME
*	GetUniqueID -- return a relatively unique number. (V39)
*
*   SYNOPSIS
*	id = GetUniqueID();
*	D0
*
*	ULONG GetUniqueID(VOID);
*
*   FUNCTION
*	Returns a unique value each time it is called. This is useful for
*	things that need unique ID such as the GadgetHelp ID, etc.
*	Note that this is only unique for 4,294,967,295 calls to this
*	function. Under normal use this is not a problem.
*	This function is safe in interrupts.
*
*   RESULT
*	id - a 32-bit value that is unique.
*
*******************************************************************************
GetUniqueID:
	move.l	ub_SysBase(a6),a0	; Get execbase...
	DISABLE	a0,NOFETCH		; Disable for a moment...
	addq.l	#1,ub_Sequence(a6)	; Do the bump...
	move.l	ub_Sequence(a6),d0	; Get the long...
	ENABLE	a0,NOFETCH		; Enable again...
	rts				; return...

;-----------------------------------------------------------------------

        END
