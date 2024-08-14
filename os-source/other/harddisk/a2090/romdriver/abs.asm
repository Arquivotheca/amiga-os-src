*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: abs.asm,v 34.11 87/12/04 19:13:04 bart Exp $
*
*	$Locker:  $
*
*	$Log:	abs.asm,v $
*   Revision 34.11  87/12/04  19:13:04  bart
*   checkpoint
*   
*   Revision 34.10  87/12/04  12:07:11  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.9  87/10/14  14:25:25  bart
*   10-13 rev 1
*   
*   
*   Revision 34.8  87/10/14  14:14:53  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.7  87/06/11  15:46:33  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.6  87/06/11  15:35:53  bart
*   ok
*   
*   Revision 34.5  87/06/03  13:05:30  bart
*   AutoBoot
*   
*   Revision 34.4  87/06/03  10:58:24  bart
*   checkpoint
*   
*   Revision 34.3  87/05/31  16:34:20  bart
*   checkpoint
*   
*   Revision 34.2  87/05/29  19:38:04  bart
*   checkpoint
*   
*   Revision 34.1  87/05/29  17:51:32  bart
*   *** empty log message ***
*   
*   Revision 34.0  87/05/29  17:39:02  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	LLEN	120
	PLEN	60
	LIST

*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


******* Included Files ***********************************************

	SECTION section

	NOLIST
	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE	"exec/interrupts.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE 'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	INCLUDE 'exec/nodes.i'
	ENDC
	IFND	EXEC_PORTS_I
	INCLUDE 'exec/ports.i'
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE 'exec/libraries.i'
	ENDC
	IFND	EXEC_IO_I
	INCLUDE 'exec/io.i'
	ENDC
	IFND	EXEC_DEVICES_I
	INCLUDE 'exec/devices.i'
	ENDC
	IFND	EXEC_TASKS_I
	INCLUDE 'exec/tasks.i'
	ENDC
	IFND	EXEC_MEMORY_I
	INCLUDE 'exec/memory.i'
	ENDC
	IFND	EXEC_EXECBASE_I
	INCLUDE 'exec/execbase.i'
	ENDC
	IFND	EXEC_ABLES_I
	INCLUDE 'exec/ables.i'
	ENDC
	IFND	EXEC_ERRORS_I
	INCLUDE 'exec/errors.i'
	ENDC
	IFND	EXEC_ALERTS_I
	INCLUDE 'exec/alerts.i'
	ENDC
	IFND	EXEC_RESIDENT_I
	INCLUDE 'exec/resident.i'
	ENDC

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'bootrom_rev.i'
	INCLUDE	'libraries/expansion.i'
	LIST
	INCLUDE	'internal.i'

******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	EXTERN_LIB	OpenDevice
	EXTERN_LIB	CloseDevice
	EXTERN_LIB	RemDevice

	XREF	Init
	XREF	EndCode

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	Remove

*------ Data ---------------------------------------------------------

	XDEF	hdName
	XDEF	hdIDString
	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	ExLibName
	XDEF	IntuitLibName
	XDEF	rt_MatchTag
	XDEF	rt_EndCode
	XDEF	rt_Name
	XDEF	rt_IDString
	XDEF	rt_Init
	XDEF	initDDescrip

******* Local Definitions ********************************************


*------	Jump Table of vectors:

		MOVEQ	#0,D0
		RTS

initDDescrip:					;STRUCTURE RT,0
	DC.W	RTC_MATCHWORD		; UWORD RT_MATCHWORD
rt_MatchTag:
	DC.L	initDDescrip		; APTR	RT_MATCHTAG
rt_EndCode:
	DC.L	EndCode				; APTR	RT_ENDSKIP
	DC.B	RTW_COLDSTART		; UBYTE RT_FLAGS
	DC.B	VERSION				; UBYTE RT_VERSION
	DC.B	NT_DEVICE			; UBYTE RT_TYPE
	DC.B	20					; BYTE	RT_PRI
rt_Name:
	DC.L	hdName				; APTR  RT_NAME
rt_IDString:
	DC.L	hdIDString			; APTR	RT_IDSTRING
rt_Init:
	DC.L	Init				; APTR	RT_INIT
								; LABEL RT_SIZE


		CNOP	0,4	; LONG WORD ALIGN
ExLibName	EXPANSIONNAME	; Expansion Library Name

		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
IntuitLibName	
		DC.B	'intuition.library',0
		DS.W	0

		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
hdName:
		HD_NAME

*		;------ our name identification string
		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
hdIDString:	VSTRING

Remove:
*		;----- Open
		LEA	-IOSTD_SIZE(SP),SP
		MOVE.L	SP,A1
		LEA	hdName(PC),A0
		MOVE.L	A0,LN_NAME(A1)

		CLEAR	D0
		CLEAR	D1

		CALLSYS OpenDevice
		TST.L	D0
		BEQ.S	Remove_End		; no device by that name

*		;----- Close
		MOVE.L	SP,A1
		MOVE.L	IO_DEVICE(A1),-(SP)
		CALLSYS CloseDevice

*		;----- Remove
		MOVE.L	(SP)+,A1
		CALLSYS RemDevice

Remove_End:
		LEA	IOSTD_SIZE(SP),SP
		RTS

VERNUM:		EQU	VERSION

REVNUM		EQU	REVISION

	END
