*************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                   *
*************************************************************************
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines				*
*                                                                       *
*************************************************************************
*
* head.asm    - library header module
*
* Contains ResidentNode, and all version #'s and date stamps.
*
************************************************************************

      nolist
	include "exec/types.i"
	include "exec/initializers.i"
	include "exec/libraries.i"
	include "exec/resident.i"

	include "camdlib.i"
	include "camd_rev.i"        ; Revision - automated revision
      list


	idnt	head.asm

	    ; external
	xref	InitLib 	    ; init function
	xref	FuncTable	    ; function table
	xref	EndSkip 	    ; End of code segment


; code at start of file in case anyone tries to execute the library as a program

	entry	FalseStart
FalseStart
	moveq	#-1,d0
	rts


ResidentNode
	dc.w	RTC_MATCHWORD	    ; RT_MATCHWORD
	dc.l	ResidentNode	    ; RT_MATCHTAG
	dc.l	EndSkip 	    ; RT_ENDSKIP
	dc.b	RTF_AUTOINIT	    ; RT_FLAGS
	dc.b	VERSION	    	    ; RT_VERSION
	dc.b	NT_LIBRARY	    ; RT_TYPE
	dc.b	0		    ; RT_PRI
	dc.l	LibName 	    ; RT_NAME
	dc.l	IDString	    ; RT_IDString
	dc.l	InitTable	    ; RT_SIZE


LibName:	DC.B	'camd.library',0
IDString:	VSTRING
	CNOP	0,2

InitTable
	dc.l	XCamdBase_Size
	dc.l	FuncTable
	dc.l	DataTable
	dc.l	InitLib

DataTable
	    ; standard library stuff
	INITBYTE    LN_TYPE,NT_LIBRARY
	INITLONG    LN_NAME,LibName
	INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD    LIB_VERSION,VERSION
	INITWORD    LIB_REVISION,REVISION
	INITLONG    LIB_IDSTRING,IDString

	    ; library specific stuff

	    ; end of init list
	dc.l	    0

	end
