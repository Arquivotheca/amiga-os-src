
		include 'exec/types.i'
		include 'exec/resident.i'
		include 'exec/nodes.i'
		include 'exec/lists.i'
		include 'exec/libraries.i'
		include 'exec/alerts.i'
		include 'exec/initializers.i'
		include 'exec/resident.i'
		include 'libraries/dos.i'

		include 'ps_rev.i'

		xref _NewContext
		xref _DestroyContext
		xref _Interpret
		xref EndCode

	IFD	MAKELIBRARY
; these prevent any need for linking with c.o (we're a library after all)

		XDEF	 _SysBase
		XDEF	 __FPERR
		xdef _XCEXIT
		xdef __ONBREAK
		xdef __SIGFPE
		xdef __oserr
		xdef __OSERR
		xdef _DOSBase
		xdef __ProgramName
		xdef _IntuitionBase
		xdef _GfxBase

		xdef _stdin
		xdef _stdout
		xdef _stderr

		xdef _PostScriptBase
	ENDC


;----- Some Exec Library vectors
		XREF	_AbsExecBase
		XREF	_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVORemove,_LVOFreeMem

;----- Some Dos Library vectors
		XREF	_LVOInput,_LVOOutput


	STRUCTURE MyStuff,LIB_SIZE
		ULONG	my_SegList
		UBYTE	my_Flags
		UBYTE	my_pad
	LABEL MyStuff_Sizeof


		SECTION text,code

Library_start	moveq.l	#10,d0
		rts	

;------------------------------------------------------------------
		CNOP 0,4

OURPRIO EQU 0

RomTag:
		DC.W	RTC_MATCHWORD
		DC.L	RomTag
		DC.L	EndCode
		DC.B	RTF_AUTOINIT
		DC.B	VERSION
		DC.B	NT_LIBRARY
		DC.B	OURPRIO
		DC.L	library_name
		DC.L	library_id
		DC.L	InitStructure

;------------------------------------------------------------------
; Some string identifiers
;------------------------------------------------------------------

library_name	DC.B	"postscript.library",$0
		CNOP	0,4

library_id	DC.B	"PostScript Library (c) CBM-Amiga 1991",$d,$a,$0
		CNOP	0,4

VersionID	VSTRING
		CNOP	0,4

DosName		DC.B	"dos.library",0
		CNOP	0,4
IntuiName	DC.B	"intuition.library",0
		CNOP 	0,4
GfxName		DC.B	"graphics.library",0
		CNOP 	0,4

;------------------------------------------------------------------
; Our Initialisation Structure. 
;------------------------------------------------------------------
InitStructure	DC.L	MyStuff_Sizeof		; Size of structure.
		DC.L	FunctionTable		; Table of allowable functions.
		DC.L	0			; Obscure method of data initialization.
		DC.L	InitializeRoutine	; Routine to finish-off initialization.

;------------------------------------------------------------------
; The Function Table where public function vectors are defined
;------------------------------------------------------------------
FunctionTable	DC.L	Open		These four routines must always be
		DC.L	Close		present - they represent the minimum
		DC.L	Expunge		functions that a library must
		DC.L	Reserved	possess, although they are not for
;					calling like the others.

; Put the other function pointers in here.
		DC.L	_NewContext
		DC.L	_DestroyContext
		DC.L	_Interpret
		DC.L	-1		Terminate function table.

;------------------------------------------------------------------
; This Function is Called the very first time we Open the library.
;
; d0 - LibraryBase ptr
; a0 - SegList ptr
;------------------------------------------------------------------

OUR_FLAG	EQU		(LIBF_SUMUSED!LIBF_CHANGED)

InitializeRoutine:
		movem.l d0-d7/a0-a6,-(sp)	Save everything, just to be safe
		movea.l d0,a1
		
		move.l d0,_PostScriptBase

		move.b	#NT_LIBRARY,LN_TYPE(a1)
		pea.l	library_name(pc)
		move.l	(sp)+,LN_NAME(a1)
		move.b  #OUR_FLAG,LIB_FLAGS(a1)
		move.w  #VERSION,LIB_VERSION(a1)
		move.w  #REVISION,LIB_REVISION(a1)
		move.l  #VersionID,LIB_IDSTRING(a1)
	
;------- Save the seglist pointer
		move.l a0,my_SegList(a1)

;------- Global SysBase pointer. Used by our library
		move.l	_AbsExecBase,a6
		move.l	a6,_SysBase

;------- Open Dos library
		lea	DosName(pc),a1
		moveq.l	#0,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,_DOSBase
		beq	No_DOS

;------- Open Graphics and Intuition Libraries for anyone

		lea GfxName(pc),a1
		moveq.l #0,d0
		jsr _LVOOpenLibrary(a6)
		move.l d0,_GfxBase
		
		lea IntuiName(pc),a1
		moveq.l #0,d0
		jsr _LVOOpenLibrary(a6)
		move.l d0,_IntuitionBase

;------- Setup the standard in/out pointers
		move.l	d0,a6
		jsr	_LVOInput(a6)
		move.l	d0,_stdin
		jsr	_LVOOutput(a6)
		move.l	d0,_stdout
		move.l	d0,_stderr

		movem.l (sp)+,d0-d7/a0-a6	returns library base in d0
		rts

No_DOS		movem.l (sp)+,d0-d7/a0-a6
		moveq #0,d0			NULL lib base for an error
		rts

;------- end of InitStructure -------------------------------------


Color		move.l	d0,-(sp)
		moveq #0,d0
loop		move.w #$f00,$dff180
		addq.l #1,d0
		bne loop
		move.l	(sp)+,d0
		rts

;------------------------------------------------------------------
;------------------------------------------------------------------


;------------------------------------------------------------------
; This routine increments the number of users of this library
;
; a6 - PostScriptBase ptr
; d0 - version
;------------------------------------------------------------------
Open		addq.w	#1,LIB_OPENCNT(a6)	 Increment number of library users
		bclr.b	#LIBB_DELEXP,my_Flags(a6) Clear any previous expunge calls
		move.l	a6,d0			 Return PostScriptBase ptr
		rts		


;------------------------------------------------------------------
; This routine decrements the number of users. If zero and delayed
; expunge bit set it realy removes the library
;
; a6 - PostScriptBase ptr
;------------------------------------------------------------------
Close		clr.l	d0
		subq.w	#1,LIB_OPENCNT(a6)	Decrement number of library users
		bne.s	Not_LastOne
		btst.b	#LIBB_DELEXP,my_Flags(a6) Where here so this is the last user.
		bne.s	Expunge			  IF there is a Delayed expunge pending						
Not_LastOne	rts


;------------------------------------------------------------------
; This Routine performs the real library expunge.
;
; a6 - PostScriptBase ptr
;------------------------------------------------------------------
Expunge		movem.l	d2/a5/a6,-(sp)
		move.l	a6,a5
		move.l	_SysBase,a6

;------ Is this the last user of the library ?
		tst.w	LIB_OPENCNT(a5)
		beq.s	Do_Expunge

;------ No, so set the Delayed expunge bit in our library base for
;------ when we next get a expunge call.
		bset.b	#LIBB_DELEXP,my_Flags(a5)

;------	Do not return the seglist as were not de-allocating the library this
;------ time round.
		moveq.l	#0,d0
		bra.s	expunge_end

;------ We return the seglist for this library so that it can be
;------ removed from the system list
Do_Expunge	move.l	my_SegList(a5),d2

;------ Remove node from the library list
		move.l	a5,a1
		jsr	_LVORemove(a6)

;------ Do some custom de-alloc stuff
		move.l	_DOSBase,a1
		jsr	_LVOCloseLibrary(a6)

		move.l _GfxBase,a1
		cmp.w #0,a1
		beq.s Jmp1
		jsr _LVOCloseLibrary(a6)

Jmp1	move.l _IntuitionBase,a1
		cmp.w #0,a1
		beq.s Jmp2
		jsr _LVOCloseLibrary(a6)
Jmp2
		

;------ Calculate how much memory we are going to free from the system.
;------
;------ size = LIB_NEGSIZE+LIB_POSSIZE
;------ start = (libbase - LIB_NEGSIZE) or (a1 - d0)
;------ FreeMem(start,size)
;------
		moveq.l	#0,d0
		move.l	a5,a1
		move.w	LIB_NEGSIZE(a5),d0

		sub.l	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		jsr	_LVOFreeMem(a6)

;------ Stuff our seglist into the return register.
		move.l	d2,d0

expunge_end	movem.l (sp)+,d2/a5/a6
		rts
;------------------------------------------------------------------
; Reserved for future use
;
; a6 - PostScriptBase ptr
;------------------------------------------------------------------
Reserved	rts		
				

		SECTION __MERGED,data

__FPERR:	DC.L 0
_XCEXIT:	DC.L 0
__ONBREAK:	DC.L 0
__SIGFPE:	DC.L 0
__oserr:	DC.L 0
__OSERR:	DC.L 0
_SysBase:	DC.L 0
_DOSBase:	DC.L 0
__ProgramName:	DC.L 0

_IntuitionBase:	DC.L 0
_GfxBase:	DC.L 0

_stdout		DC.L -1
_stdin		DC.L -1
_stderr		DC.L -1

_PostScriptBase DC.L 0

		END
