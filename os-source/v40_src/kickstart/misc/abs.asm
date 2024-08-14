
*************************************************************************
*									*
*  Copyright (C) 1985,1989 Commodore-Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* abs.asm
*
* Source Control
* ------ -------
* 
* $Id: abs.asm,v 36.11 90/05/06 00:45:20 bryce Exp $
*
* $Locker:  $
*
* $Log:	abs.asm,v $
* Revision 36.11  90/05/06  00:45:20  bryce
* Nuke the STUPID $Header$ inflicted by the last RCS upgrade
* 
* Revision 36.10  90/04/06  17:12:26  bryce
* Upgrade to RCS 4.0
* 
* Revision 27.3  89/03/11  21:36:35  bryce
* Shrink 13.2%  - Remove obsolete code & upgrade to V36
* 
* Revision 27.3  89/03/11  21:00:19  bryce
* Shrink: As small as it is, misc.resource had fat in it...
* 
* Revision 27.2  86/07/21  13:42:36  neil
* autodoc update - bart
* 
* Revision 27.1  85/06/24  13:25:23  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  12:08:17  neil
* *** empty log message ***
* 
* Revision 1.1  85/06/17  11:57:09  neil
* Initial revision
* 
* 
*
*************************************************************************

* ****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/resident.i'

	INCLUDE	'misc.i'
	INCLUDE	'misc_rev.i'


* ****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	MiscInit
	XREF	EndCode

* ****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

***	XDEF	miscStart

*------ Data ---------------------------------------------------------

	XDEF	miscName
	XDEF	subsysName
	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	VERSTRING

* ****** Local Definitions ********************************************


	SECTION	section


*** miscStart:
***		BRA	MiscInit
initDDescrip:
				  ;STRUCTURE RT,0
	  DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
	  DC.L    initDDescrip    ; APTR  RT_MATCHTAG
	  DC.L    EndCode         ; APTR  RT_ENDSKIP
	  DC.B    RTW_COLDSTART   ; UBYTE RT_FLAGS
	  DC.B    VERSION         ; UBYTE RT_VERSION
	  DC.B    NT_RESOURCE     ; UBYTE RT_TYPE
	  DC.B    70              ; BYTE  RT_PRI
	  DC.L    miscName        ; APTR  RT_NAME
	  DC.L    VERSTRING       ; APTR  RT_IDSTRING
	  DC.L    MiscInit        ; APTR  RT_INIT
				  ; LABEL RT_SIZE
											      
subsysName:
miscName:	MISCNAME
VERSTRING:	VSTRING
VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION

		END
