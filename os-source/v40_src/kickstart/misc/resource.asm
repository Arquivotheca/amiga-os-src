
*************************************************************************
*									*
* Copyright (C) 1985,1989, Commodore-Amiga Inc.  All rights reserved.   *
*									*
*************************************************************************


*************************************************************************
*
* resource.asm
*
* Source Control
* ------ -------
*
* $Id: resource.asm,v 36.11 90/05/06 00:48:25 bryce Exp $
*
* $Locker:  $
*
* $Log:	resource.asm,v $
* Revision 36.11  90/05/06  00:48:25  bryce
* Nuke the STUPID $Header$ inflicted by the last RCS upgrade
* 
* Revision 36.10  90/04/06  17:13:00  bryce
* Upgrade to RCS 4.0
* 
* Revision 27.6  89/05/12  19:54:48  bryce
* Standardize function names (AllocMiscResource,FreeMiscResource).
* Change a DISABLE to FORBID (with no loss of safety).  Remove Alert call
* on init (it's handled dynamically as an out of memory error on Open).
* 
* Revision 27.5  89/03/21  18:09:49  bryce
* Include miscprivate.i
*
* Revision 27.4  89/03/11  22:36:59  bryce
* Aw crum.  Last checkin was of wrong file.  Here is the correct resource.asm
*
* Revision 27.3  89/03/11  21:39:10  bryce
* Shrink 13.2%.  Upgrade to V36
* Documentation now matches reality & the V1.3 RKM
*
* Revision 27.3  89/03/11  20:59:25  bryce
* Upgrade documentation to match the RKM (and reality :-)
* Shrink 14% (36 bytes)
*
* Revision 27.2  86/07/21  13:42:56  neil
* autodoc update - bart
*
* Revision 27.1  85/06/24  13:25:41  neil
* Upgrade to V27
*
* Revision 26.1  85/06/17  12:08:21  neil
* *** empty log message ***
*
* Revision 1.1	85/06/17  11:57:14  neil
* Initial revision
*
* Revision 1.2	85/06/13  04:09:32  neil
* added alert call
*
* Revision 1.1	85/06/07  08:53:32  neil
* First revision
*
*
*************************************************************************

	SECTION section

; ****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/initializers.i'
;	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/alerts.i'

	INCLUDE 'miscprivate.i'
	INCLUDE 'misc.i'
	INCLUDE 'messages.i'
	INCLUDE 'asmsupp.i'

; ****** Imported Globals *********************************************

	XREF	VERNUM
	XREF	REVNUM
	XREF	VERSTRING
	XREF	miscName
	XREF	subsysName

	TASK_ABLES

; ****** Imported Functions *******************************************

	EXTERN_LIB	MakeLibrary
	EXTERN_LIB	AddResource
	EXTERN_LIB	Alert

; ****** Exported Functions *******************************************

	XDEF		MiscInit
	XDEF		AllocMiscResource
	XDEF		FreeMiscResource

; ****** Local Definitions **********************************************

*
* macro to deal with debugging messages
*
INFOLEVEL	EQU	0

*		;------ outsiders equivalent of callsys

CALLSYS 	MACRO
		CALLLIB _LVO\1
		ENDM

* ***** Internal/Resources/MiscInit ********************************
*
*   NAME
*	MiscInit - initialize the misc resource
*
*   SYNOPSIS
*	MiscInit()
*
*
*   FUNCTION
*	This routine starts the misc resource up from scratch
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

MiscInit:
		MOVE.L	A2,-(SP)

		;A6 = exec on ROMTag init
		;MOVE.L _AbsExecBase,A6 	;SysBase

		LEA.L	funcInit(pc),A0         ; function vector init
		LEA.L	structInit(pc),A1       ; data space initialization
		SUBA.L	A2,A2			; init function
		MOVEQ.L #mr_Sizeof,D0		; data size
		CALLSYS MakeLibrary
		TST.L	D0
		BEQ.S	Init_fail

		MOVE.L	D0,A1
		CALLSYS AddResource

*		;------ We have the resource.
		IFGE	INFOLEVEL-50
		MOVE.L	A1,-(SP)
		PUTMSG	30,<'%s/MiscInit: resource is at 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

Init_fail:
		MOVE.L	(SP)+,A2
		RTS

*****		PUTMSG	10,<'%s/Init: cannot MakeLibrary'>
;;;;		ALERT	AN_MiscRsrc!AG_MakeLib
;;;;		BRA.S	Init_End


structInit:
*		;------ Initialize the device
		INITBYTE	LN_TYPE,NT_RESOURCE
		INITLONG	LN_NAME,miscName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERNUM
		INITWORD	LIB_REVISION,REVNUM
		DC.W		0

******* misc.resource/AllocMiscResource *************************************
*
*    NAME
*	AllocMiscResource - allocate one of the miscelaneous resources
*
*   SYNOPSIS
*	CurrentUser = AllocMiscResource( unitNum, name )
*	D0				 D0	  A1
*
*	char * AllocMiscResource(ULONG,char *);
*
*   FUNCTION
*	This routine attempts to allocate one of the miscellaneous resources
*	If the resource had already been allocated, an error is returned. If
*	you do get it, your name is associated with the resource (so a user
*	can see who has it allocated).
*
*	This function may not be called from interrupt code
*
*   DESCRIPTION
*	There are certain parts of the hardware that a multitasking- friendly
*	program may need to take over.	The serial port is a good example. By
*	grabbing the misc.resource for the serial port, the caller would
*	"own" the hardware registers associated with that function.  Nobody
*	else, including the system serial driver, is allowed to interfere.
*
*	Resources are called in exactly the same manner as libraries.
*	From assembly language, A6 must equal the resource base.  The
*	offsets for the function are listed in the resources/misc.i
*	include file (MR_ALLOCMISCRESOURCE for this function).
*
*   INPUTS
*	unitNum - the number of the resource you want to allocate
*		  (eg.  MR_SERIALBITS).
*	name - a mnenonic name that will help the user figure out
*		what piece of software is hogging a resource.
*		(havoc breaks out if a name of null is passed in...)
*
*   RESULTS
*	CurrentUser - if the resource is busy, then the name of
*		the current user is returned.  If the resource is
*		free, then null is returned.
*
*   BUGS
*
*   SEE ALSO
*	resources/misc.i, FreeMiscResource()
*************************************************************************
*
*   IMPLEMENTATION NOTES
*
*

AllocMiscResource:
		MOVE.L	4,A0		;_AbsExecBase,A0
		EXG.L	A0,A6

		LSL.W	#2,D0
		MOVE.W	D0,D1

		FORBID
		MOVE.L	mr_AllocArray(A0,D1.W),D0   ;index defaults to word.
		BNE.S	AMR_Enable
		MOVE.L	A1,mr_AllocArray(A0,D1.W)
AMR_Enable:
		PERMIT		    ;registers preserved
		EXG.L	A0,A6
		RTS

******* misc.resource/FreeMiscResource **************************************
*
*   NAME
*	FreeMiscResource - make a resource available for reallocation
*
*   SYNOPSIS
*	FreeMiscResource( unitNum )
*			  D0
*
*	void FreeMiscResource(ULONG);
*
*   FUNCTION
*	This routine frees one of the resources allocated
*	by AllocMiscResource.  The resource is made available
*	for reuse.
*
*	FreeMiscResource must be called from the same task that
*	called AllocMiscResource.  This function may not be called from
*	interrupt code.
*
*    INPUTS
*	unitNum - the number of the miscellaneous resource to be freed.
*
*    RESULTS
*	Frees the appropriate resource.
*
*    BUGS
*
*    SEE ALSO
*	resources/misc.i, AllocMiscResource()
*
*****************************************************************************

FreeMiscResource:
		LSL.W	#2,D0
		CLR.L	mr_AllocArray(A6,D0.W)  ;index defaults to word
		RTS

funcInit:
		DC.W	-1
		DC.W	AllocMiscResource-funcInit
		DC.W	FreeMiscResource-funcInit
		DC.W	-1

		END
