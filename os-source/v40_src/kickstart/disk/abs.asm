
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
* $Id: abs.asm,v 33.3 90/07/13 14:58:56 jesup Exp $
*
* $Locker:  $
*
* $Log:	abs.asm,v $
* Revision 33.3  90/07/13  14:58:56  jesup
* $id.
* 
* Revision 33.2  89/01/23  23:21:17  bryce
* Code shrink
* 
* Revision 33.1  86/05/06  01:49:23  neil
* fixed autodoc stuff.  moved section earlier
* 
* Revision 27.1  85/06/24  13:18:21  neil
* *** empty log message ***
* 
* Revision 26.2  85/06/17  12:48:46  neil
* source control fix ups
* 
* Revision 26.1  85/06/17  12:19:20  neil
* *** empty log message ***
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

	INCLUDE	'disk.i'
        INCLUDE 'disk_rev.i'

;****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	DRInit
	XREF	EndCode

	XREF	diskName
	XREF	VERSTRING

;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

*	XDEF	drStart

*------ Data ---------------------------------------------------------

	XDEF	REVNUM
	XDEF	VERNUM

;****** Local Definitions ********************************************




**drStart:
**		BRA	DRInit	;This code only for test


initDDescrip:
				  ;STRUCTURE RT,0
	  DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
	  DC.L    initDDescrip    ; APTR  RT_MATCHTAG
	  DC.L    EndCode         ; APTR  RT_ENDSKIP
	  DC.B    RTW_COLDSTART   ; UBYTE RT_FLAGS
	  DC.B    VERSION         ; UBYTE RT_VERSION
	  DC.B    NT_RESOURCE     ; UBYTE RT_TYPE
	  DC.B    70              ; BYTE  RT_PRI
	  DC.L    diskName        ; APTR  RT_NAME
	  DC.L    VERSTRING       ; APTR  RT_IDSTRING
	  DC.L    DRInit          ; APTR  RT_INIT
				  ; LABEL RT_SIZE


VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION

		END
