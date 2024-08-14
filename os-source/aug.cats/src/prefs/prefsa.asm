	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE	"prefs.i"	; created by BumpRev

	SECTION text
	XDEF	_LibraryName,_LibraryID,_SysBase
	
	XREF	_CInit
	XREF	_LibOpen
	XREF	_LibClose
	XREF	_LibExpunge
	XREF	_LibReserved
	XREF	_GetPrefsDrawer
	XREF	_GetPref
	XREF	_SetPref
	XREF	_FreePref
	XREF	_FreePrefList
	XREF	_GetPrefRecord

Start:
	moveq	#-1,d0		; don't allow direct execution
	rts
	
InitDesc:
	; Resident structure
	dc.w	$4AFC		; romtag identifier (RTC_MATCHWORD)
	dc.l	InitDesc	; pointer to romtag identifier
	dc.l	EndCode		; pointer to end of code
	dc.b	0		; resident flags
	dc.b	VERSION		; release version number
	dc.b	9		; type of module (NT_LIBRARY)
	dc.b	0		; initialization priority
	dc.l	_LibraryName	; pointer to node name
	dc.l	_LibraryID	; pointer to ID string
	dc.l	ALibInit	; pointer to init code
	
_LibraryID
	VSTRING
	
_LibraryName	dc.b	'prefs.library',0
	ds.w	0

Funcs:
	dc.w	$4EF9
	dc.l	_GetPrefRecord
	dc.w	$4EF9
	dc.l	_FreePrefList
	dc.w	$4EF9
	dc.l	_FreePref
	dc.w	$4EF9
	dc.l	_SetPref
	dc.w	$4EF9
	dc.l	_GetPref
	dc.w	$4EF9
	dc.l	_GetPrefsDrawer
	dc.w	$4EF9
	dc.l	ALibReserved
	dc.w	$4EF9
	dc.l	_LibExpunge
	dc.w	$4EF9
	dc.l	_LibClose
	dc.w	$4EF9
	dc.l	_LibOpen
	
lib:
	; Node structure
	dc.l	0		; next node
	dc.l	0		; previous node
	dc.b	9		; node type (NT_LIBRARY)
	dc.b	0		; priority
	dc.l	_LibraryName	; node name
	
	; Library structure
	dc.b	6		; flags (CHANGED | SUMUSED | DELEXP)
	dc.b	0		; padding
	dc.w	lib-Funcs	; number of bytes before LIB
	dc.w	_LibBase-lib	; number of bytes after LIB
	dc.w	VERSION		; major version number
	dc.w	REVISION	; minor version number
	dc.l	_LibraryID	; identification
	dc.l	0		; checksum
	dc.w	0		; open count
	
_SegList
	dc.l	0		; pointer to the SegList
_LibBase
	dc.l	0		; pointer to library base
	
ALibInit:
	move.l	4,_SysBase	; open ExecBase
	lea	_SegList,a1	; load the address of the SegList variable
	move.l	a0,(a1)		; save the SegList
	lea	lib,a1		; load the address of the library base
	jsr	_CInit		; call the 'C' initialization routine
	rts
	
ALibReserved:
	clr.l	d0		; return NULL
	rts
	
99$:
	XDEF	__oserr,__OSERR,__FPERR,__SIGFPE
	XDEF	__SIGINT,__ONERR,__ONEXIT,__ONBREAK,__ECS
	XDEF	__ProgramName
	
_SysBase	dc.l	0
__oserr:
__OSERR		dc.l	0
__FPERR		dc.l	0
__SIGFPE	dc.l	0
__SIGINT	dc.l	0
__ONERR		dc.l	0
__ONEXIT	dc.l	0
__ONBREAK	dc.l	0
__ECS		dc.l	0
__ProgramName	dc.l	0
	
	CNOP	0,2
	
EndCode:			; end marker
	
	END
