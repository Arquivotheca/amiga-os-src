***
*
* libent.asm - Modified Lattice Library RomTag
*
***
	include 'exec/types.i'
	include 'exec/resident.i'
	include 'exec/nodes.i'
	include 'exec/libraries.i'
***
*
* The following include is used to store the library version and name
* as follows:
*
* VERSION		EQU	36
* REVISION		EQU	1
* DATE		MACRO
*			dc.b	'27 Nov 1989'
*		ENDM
* VERS		MACRO
*			dc.b	'examplelib 36.1'
*		ENDM
* VSTRING	MACRO
*			dc.b	'examplelib 36.1 (27 Nov 1989)',13,10,0
*		ENDM
*
***
	include 'authentication.library_rev.i'
***
*
* Library init priority.  For user (NON-ROM) libraries, should be 0
*
***
PRI	equ	0
***
*
* Define some the external references...
*
***
	xref	__LibName
	xref	RESLEN
	xref	_BSSBAS		; linker defined base of BSS
	xref	_BSSLEN		; linker defined length of BSS
	xref	_LibFuncTab
	xref	__LibInitTab
	xref	_LibInit
	xdef	__LibRomTag
	xdef	__VerString
***
*
* This is the first hunk and is where the RomTag is needed...
*
***
	SECTION	text,CODE	; romtag must be in first hunk
***
*
* We need to make sure that if some user tries to execute this
* file as an executable that we return an error...
*
***
	moveq	#20,d0
	rts
***
*
* Place the version string into the tag.  This makes the
* version displayable.  It also makes it possible to "TYPE"
* the library file to find out the version...
*
***
__VerString:	VSTRING
***
*
* The RomTag that is needed for a library
*
***
	cnop	0,2		; We need to word-align the RomTag
__LibRomTag:
	dc.w	RTC_MATCHWORD
	dc.l	__LibRomTag
	dc.l	endtag
	dc.b	RTF_AUTOINIT
	dc.b	VERSION
	dc.b	NT_LIBRARY
	dc.b	PRI
	dc.l	__LibName
	dc.l	__VerString
	dc.l	__LibInitTab
endtag:
***
*
* Start of the merged data area that the linker puts
* into the library structure for the global data.
*
***
	section	__MERGED,data
	xdef	__Libmergeddata
__Libmergeddata	dc.l	0
	end
