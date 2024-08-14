*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: badtrans.asm,v 34.10 88/01/21 18:04:00 bart Exp $
*
*	$Locker:  $
*
*	$Log:	badtrans.asm,v $
*   Revision 34.10  88/01/21  18:04:00  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.9  87/12/04  19:13:23  bart
*   checkpoint
*   
*   Revision 34.8  87/12/04  12:07:55  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.7  87/10/14  14:31:33  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:15:26  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:00:26  bart
*   *** empty log message ***
*   
*   Revision 34.4  87/06/11  15:47:15  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:58:37  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:35:18  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:38:55  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:16  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* badtrans.asm
*
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
	IFND	EXEC_STRINGS_I
	INCLUDE 'exec/strings.i'
	ENDC

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST



******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------

*------ System Library Functions -------------------------------------

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	BadTrans
*------ Data ---------------------------------------------------------


******* Local Definitions ********************************************


******* System/Drivers/HD/BadTrans ***************************************
*
*   NAME
*	BadTrans - translate logical block number to physical block number,
*		   mapping the addresses of bad blocks to good blocks.
*
*   SYNOPSIS
*	BadTrans( Logical Block ),	UnitPtr,	DevPtr
*			D1	 	A3,		A6
*
*   FUNCTION
*	This routine searches the bad block table for a matching block.
*	If a match is found, the block number in D1 is replaced with the
*	mapped good block.  If no match is found, D1 is left unchanged,
*	with A0 pointing to the number of the next largest bad block.
*
*   INPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*

BadTrans:
		LEA	HDU_BB+BBR_TABLE+BBM_BAD(A3),A0;Point to bad block table
BTLoop1:	; Find block number bigger than or equal to this block #
		CMP.L	(A0),D1			; If # is >= D1,
		BLS.S	BTEndLoop		;	Exit loop
		ADDQ.L	#BBM_SIZE,A0		; Else point to next block
		BRA	BTLoop1			;	and check it
BTEndLoop:
		BNE.S	BTDone			; If this block isn't bad, exit
		MOVE.L	BBM_GOOD-BBM_BAD(A0),D1	; Get good block number
		CMP.B	D0,D0			; Set "equal" status
BTDone:
		RTS

		END

