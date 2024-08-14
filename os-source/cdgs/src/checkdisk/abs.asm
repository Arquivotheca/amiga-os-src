
*************************************************************************
*									*
* Copyright (C) 1985,1988, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* abs.asm
*
* Source Control
* ------ -------
* 
* $Id: abs.asm,v 1.1 92/03/11 23:50:57 jesup Exp $
*
* $Log:	abs.asm,v $
* Revision 1.1  92/03/11  23:50:57  jesup
* Initial revision
* 
*
*************************************************************************

	SECTION	section

;****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/resident.i'

        INCLUDE 'checkdisk_rev.i'

;****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	_DRInit
	XREF	_EndCode

	XDEF	_drName

;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

*------ Data ---------------------------------------------------------

;****** Local Definitions ********************************************




**drStart:
**		BRA	DRInit	;This code only for test

*
* MUST be lower priority than timer.device!
*
initDDescrip:
				  ;STRUCTURE RT,0
	  DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
	  DC.L    initDDescrip    ; APTR  RT_MATCHTAG
	  DC.L    _EndCode        ; APTR  RT_ENDSKIP
	  DC.B    RTW_COLDSTART   ; UBYTE RT_FLAGS
	  DC.B    VERSION         ; UBYTE RT_VERSION
	  DC.B    NT_RESOURCE     ; UBYTE RT_TYPE
	  DC.B    49              ; BYTE  RT_PRI
	  DC.L    MyName          ; APTR  RT_NAME
	  DC.L    VERSTRING       ; APTR  RT_IDSTRING
	  DC.L    _DRInit         ; APTR  RT_INIT
				  ; LABEL RT_SIZE


VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION
MyName:		dc.b	'checkdisk',0
_drName:	dc.b	'disk.resource',0

VERSTRING:      VSTRING
		CNOP	0,2

		END
