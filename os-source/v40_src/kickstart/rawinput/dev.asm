	TTL    '$Id: dev.asm,v 36.5 90/04/13 12:45:54 kodiak Exp $
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	device close functions
*
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"

	INCLUDE		"macros.i"
	INCLUDE		"stddevice.i"


*------ Imported Functions -------------------------------------------

	XREF_EXE	AddDevice
	XREF_EXE	FreeMem
	XREF_EXE	MakeLibrary


*------ Exported Functions -------------------------------------------

	XDEF		SuccessfulExpunge
	XDEF		DeferredExpunge
	XDEF		ExtFunc
	XDEF		Close


*------ Expunge support ----------------------------------------------

SuccessfulExpunge:
		MOVE.L	A6,A1
		REMOVE

	    ;------ deallocate device data
		MOVE.L	dd_Segment(A6),-(A7)
		MOVE.L	A6,A1
		MOVE.W	LIB_NEGSIZE(A6),D0
		SUBA.W	D0,A1
		ADD.W	LIB_POSSIZE(A6),D0
		EXT.L	D0
		LINKEXE FreeMem

		MOVE.L	(A7)+,D0
		RTS

DeferredExpunge:
		BSET	#LIBB_DELEXP,LIB_FLAGS(A6)
		MOVEQ	#0,D0		    ;still in use

ExtFunc:
		RTS


*------ rawinput.device/Close ----------------------------------------
*
*   NAME
*	Close - terminate access to a device
*
*   SYNOPSIS
*	Close(iORequest), gameportDev
*	      A1	  A6
*
*   FUNCTION
*	The close routine notifies a device that it will no longer
*	be using the device.  The driver will clear the IO_DEVICE
*	and IO_UNIT entries in the iORequest structure.
*
*	The open count on the device will be decremented, and if it
*	falls to zero and an Expunge is pending, the Expunge will
*	take place.
*
*---------------------------------------------------------------------
Close:
*	    ;-- clear out the pointers
		MOVEQ	#0,D0
		MOVE.L	D0,IO_UNIT(A1)
		MOVE.L	D0,IO_DEVICE(A1)

*	    ;-- check if this should now be expunged
		SUBQ.W	#1,LIB_OPENCNT(A6)
		BNE.S	closeRts
		BTST	#LIBB_DELEXP,LIB_FLAGS(A6)
		BEQ.S	closeRts
		JMP	LIB_EXPUNGE(A6)

closeRts:
		RTS

		END
