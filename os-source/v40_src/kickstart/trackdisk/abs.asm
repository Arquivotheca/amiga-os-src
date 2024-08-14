
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* Source Control
* ------ -------
* 
* $Id: abs.asm,v 27.4 90/06/01 23:14:27 jesup Exp $
*
* $Locker:  $
*
* $Log:	abs.asm,v $
* Revision 27.4  90/06/01  23:14:27  jesup
* Conform to include standard du jour
* 
* Revision 27.3  89/04/27  23:28:01  jesup
* fixed autodocs
* 
* Revision 27.2  89/01/05  15:12:06  jesup
* removed obsolete Remove code and branches from resident tag
* 
* Revision 27.1  85/06/24  13:36:12  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:12:53  neil
* *** empty log message ***
* 
* Revision 25.5  85/06/14  18:08:26  neil
* changed type field to "device"
* 
* Revision 25.4  85/05/06  02:38:02  neil
* priority moved back to 20
* 
* Revision 25.3  85/05/01  06:07:09  neil
* Changed priority to -1 so it will initialze AFTER graphics.
* 
* Revision 25.2  85/05/01  04:30:38  neil
* first working 25 version
* 
* Revision 25.1  85/04/25  17:34:52  neil
* initial 25 revision -- before it has been converted
* 
*************************************************************************

****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/resident.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'trackdisk_rev.i'

	SECTION section

****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	EXTERN_LIB	OpenDevice
	EXTERN_LIB	CloseDevice
	EXTERN_LIB	RemDevice

	XREF	Init
	XREF	EndCode

****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

*	XDEF	Remove

*------ Data ---------------------------------------------------------

	XDEF	tdName
	XDEF	tdIDString
	XDEF	VERNUM
	XDEF	REVNUM

****** Local Definitions ********************************************


*------	Jump Table of vectors:

*	commented out - obsolete REJ
*		BRA	Init
*		BRA	Remove

initDDescrip:
				;STRUCTURE RT,0
	DC.W	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	DC.L	initDDescrip	; APTR	RT_MATCHTAG
	DC.L	EndCode		; APTR	RT_ENDSKIP
	DC.B	RTW_COLDSTART	; UBYTE RT_FLAGS
	DC.B	VERSION		; UBYTE RT_VERSION
	DC.B	NT_DEVICE	; UBYTE RT_TYPE
	DC.B	20		; BYTE	RT_PRI
	DC.L	tdName		; APTR  RT_NAME
	DC.L	tdIDString	; APTR	RT_IDSTRING
	DC.L	Init		; APTR	RT_INIT
				; LABEL RT_SIZE


tdName:
		TD_NAME

*		;------ our name identification string
tdIDString:	VSTRING

*	commented out - obsolete REJ
*Remove:
**		;----- Open
*		LEA	-IOSTD_SIZE(SP),SP
*		MOVE.L	SP,A1
*		LEA	tdName,A0
*		MOVE.L	A0,LN_NAME(A1)
*
*		CLEAR	D0
*		CLEAR	D1
*
*		CALLSYS OpenDevice
*		TST.L	D0
*		BEQ.S	Remove_End		; no device by that name
*
**		;----- Close
*		MOVE.L	SP,A1
*		MOVE.L	IO_DEVICE(A1),-(SP)
*		CALLSYS CloseDevice
*
*
**		;----- Remove
*		MOVE.L	(SP)+,A1
*		CALLSYS RemDevice
*
*Remove_End:
*		LEA	IOSTD_SIZE(SP),SP
*		RTS

VERNUM:		EQU	VERSION

REVNUM		EQU	REVISION

	END
