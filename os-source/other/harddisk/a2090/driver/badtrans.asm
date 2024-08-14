
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

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE	"exec/interrupts.i"
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/strings.i'

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

	SECTION section


******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------
	XREF hdName

*------ System Library Functions -------------------------------------

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	BadTrans
*------ Data ---------------------------------------------------------



******* Local Definitions ********************************************

INFO_LEVEL SET 0

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
                IFGE    INFO_LEVEL-60
		MOVE.L D1,-(SP)
                INFOMSG 60,<'%s/BadTrans called with D1 %ld'>
		ADDQ.L #4,SP
                ENDC

		LEA	HDU_BB+BBR_TABLE+BBM_BAD(A3),A0;Point to bad block table
BTLoop1:	; Find block number bigger than or equal to this block #
		
		CMP.L	(A0),D1			; If # is >= D1,
		BLS.S	BTEndLoop		;	Exit loop
		ADDQ.L	#BBM_SIZE,A0		; Else point to next block
		BRA	BTLoop1			;	and check it
BTEndLoop:
		BNE.S	BTDone			; If this block isn't bad, exit

                IFGE    INFO_LEVEL-60
                MOVE.L  D1,-(SP)
                INFOMSG 60,<'%s/BadTrans HIT! Bad Block is %ld'>
                ADDQ.L  #4,SP
                ENDC

		MOVE.L	BBM_GOOD-BBM_BAD(A0),D1	; Get good block number
		
                IFGE    INFO_LEVEL-60
                MOVE.L  D1,-(SP)
                INFOMSG 60,<'%s/BadTrans mapped Block is %ld'>
                ADDQ.L  #4,SP
                ENDC

		CMP.B	D0,D0			; Set "equal" status
BTDone:
		RTS

		END

