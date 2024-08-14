	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"libraries/dos.i"
        INCLUDE "internal/languagedrivers.i"

	INCLUDE	"nederlands_rev.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVOFreeMem
	XREF	_LVOAlert

;---------------------------------------------------------------------------

	XDEF	LibInit
	XDEF	LibOpen
	XDEF	LibClose
	XDEF	LibExpunge
	XDEF	LibReserved
	XDEF	GetDriverInfo
	XDEF	GetLocaleStr

;---------------------------------------------------------------------------

   STRUCTURE DriverBase,LIB_SIZE
	ULONG   db_SegList
	ULONG   db_SysLib
   LABEL DriverBase_SIZEOF

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
	moveq   #-1,d0
	rts

;-----------------------------------------------------------------------

ROMTAG:
	DC.W    RTC_MATCHWORD		; UWORD RT_MATCHWORD
	DC.L    ROMTAG			; APTR  RT_MATCHTAG
	DC.L    ENDTAG			; APTR  RT_ENDSKIP
	DC.B    RTF_AUTOINIT		; UBYTE RT_FLAGS
	DC.B    VERSION			; UBYTE RT_VERSION
	DC.B    NT_LIBRARY		; UBYTE RT_TYPE
	DC.B    LibPriority		; BYTE  RT_PRI
	DC.L    LibName			; APTR  RT_NAME
	DC.L    LibId			; APTR  RT_IDSTRING
	DC.L    LibInitTable		; APTR  RT_INIT

LibName     DC.B "nederlands.language",0
LibId       VSTRING
LibPriority EQU -100

	CNOP	0,4

LibInitTable:
	DC.L	DriverBase_SIZEOF ; size of library base data space
	DC.L	LibFuncTable
	DC.L	LibDataTable
	DC.L	LibInit

LibFuncTable:
	DC.W	-1
	DC.W	LibOpen-LibFuncTable
	DC.W	LibClose-LibFuncTable
	DC.W	LibExpunge-LibFuncTable
	DC.W	LibReserved-LibFuncTable
	DC.W	GetDriverInfo-LibFuncTable
	DC.W	LibReserved-LibFuncTable
	DC.W	LibReserved-LibFuncTable
	DC.W	LibReserved-LibFuncTable
	DC.W	GetLocaleStr-LibFuncTable
	DC.W   -1

LibDataTable:
	INITBYTE   LN_PRI,LibPriority
	INITBYTE   LN_TYPE,NT_LIBRARY
	INITLONG   LN_NAME,LibName
	INITBYTE   LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
	INITWORD   LIB_VERSION,VERSION
	INITWORD   LIB_REVISION,REVISION
	INITLONG   LIB_IDSTRING,LibId
	DC.W       0

	CNOP	0,4

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
	move.l	d0,a1
        move.l	a0,db_SegList(a1)
        move.l	a6,db_SysLib(a1)
	rts

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has DriverBase, task switching is disabled
; Returns 0 for failure, or DriverBase for success.
LibOpen:
	addq.w	#1,LIB_OPENCNT(a6)
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	move.l	a6,d0
	rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has DriverBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,LIB_OPENCNT(a6)

	; if delayed expunge bit set, then try to get rid of the library
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	LIB_OPENCNT(a6)
	beq.s	DoExpunge

	; still some library users, so just return
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has LocaleBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
	tst.w	LIB_OPENCNT(a6)
	beq.s	DoExpunge

	bset	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

DoExpunge:
	movem.l	d2/a5/a6,-(sp)
	move.l	a6,a5
	move.l	db_SysLib(a5),a6
	move.l	db_SegList(a5),d2

	move.l  a5,a1
	REMOVE

	move.l	a5,a1
	moveq	#0,d0
	move.w	LIB_NEGSIZE(a5),d0
	sub.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	CALL	FreeMem

	move.l	d2,d0
	movem.l	(sp)+,d2/a5/a6
	rts

;-----------------------------------------------------------------------

LibReserved:
	moveq	#0,d0
	rts

;---------------------------------------------------------------------------

GetDriverInfo:
	move.l	#GDIF_GETLOCALESTR,d0
	rts

;---------------------------------------------------------------------------

Strings:
S000 DC.B       "",0
S001 DC.B	"zondag",0
S002 DC.B	"maandag",0
S003 DC.B	"dinsdag",0
S004 DC.B	"woensdag",0
S005 DC.B	"donderdag",0
S006 DC.B	"vrijdag",0
S007 DC.B	"zaterdag",0

S008 DC.B	"zo",0
S009 DC.B	"ma",0
S010 DC.B	"di",0
S011 DC.B	"wo",0
S012 DC.B	"do",0
S013 DC.B	"vr",0
S014 DC.B	"za",0

S015 DC.B	"januari",0
S016 DC.B	"februari",0
S017 DC.B	"maart",0
S018 DC.B	"april",0
S019 DC.B	"mei",0
S020 DC.B	"juni",0
S021 DC.B	"juli",0
S022 DC.B	"augustus",0
S023 DC.B	"september",0
S024 DC.B	"oktober",0
S025 DC.B	"november",0
S026 DC.B	"december",0

S027 DC.B	"jan",0
S028 DC.B	"feb",0
S029 DC.B	"maa",0
S030 DC.B	"apr",0
S031 DC.B	"mei",0
S032 DC.B	"jun",0
S033 DC.B	"jul",0
S034 DC.B	"aug",0
S035 DC.B	"sep",0
S036 DC.B	"oct",0
S037 DC.B	"nov",0
S038 DC.B	"dec",0

S039 DC.B	"Ja",0
S040 DC.B	"Nee",0

S041 DC.B	"VM",0
S042 DC.B	"NM",0

S043 DC.B	"-",0
S044 DC.B	"-",0

S045 DC.B	'"',0
S046 DC.B	'"',0

S047 DC.B	"Yesterday",0
S048 DC.B	"Today",0
S049 DC.B	"Tomorrow",0
S050 DC.B	"Future",0

	CNOP 0,4

;---------------------------------------------------------------------------

GetLocaleStr:
	cmp.l	#51,d0		; 50 being the maximum # of strings
	bcc.s	1$
	asl.w	#2,d0
	move.l	StringTable(pc,d0.w),d0
	rts

1$	moveq	#0,d0
	rts

StringTable:
P000 DC.L S000
P001 DC.L S001
P002 DC.L S002
P003 DC.L S003
P004 DC.L S004
P005 DC.L S005
P006 DC.L S006
P007 DC.L S007
P008 DC.L S008
P009 DC.L S009
P010 DC.L S010
P011 DC.L S011
P012 DC.L S012
P013 DC.L S013
P014 DC.L S014
P015 DC.L S015
P016 DC.L S016
P017 DC.L S017
P018 DC.L S018
P019 DC.L S019
P020 DC.L S020
P021 DC.L S021
P022 DC.L S022
P023 DC.L S023
P024 DC.L S024
P025 DC.L S025
P026 DC.L S026
P027 DC.L S027
P028 DC.L S028
P029 DC.L S029
P030 DC.L S030
P031 DC.L S031
P032 DC.L S032
P033 DC.L S033
P034 DC.L S034
P035 DC.L S035
P036 DC.L S036
P037 DC.L S037
P038 DC.L S038
P039 DC.L S039
P040 DC.L S040
P041 DC.L S041
P042 DC.L S042
P043 DC.L S043
P044 DC.L S044
P045 DC.L S045
P046 DC.L S046
P047 DC.L S047
P048 DC.L S048
P049 DC.L S049
P050 DC.L S050

;---------------------------------------------------------------------------

ENDTAG:

;---------------------------------------------------------------------------

	END
