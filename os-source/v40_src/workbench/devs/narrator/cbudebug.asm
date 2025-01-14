	TTL	'CBUDEBUG.ASM'

	SECTION      speech

*************************************************************************	
*                                                                    	*
*   Copyright 1990, 1991 Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


*************************************************************************
*                                                       		*
*			     Debug code	 				*
*                                                       		*
*	Contains null debug routines.  Associated file debug.asm	*
*	contains corresponding "real" debug routines.  An exception	*
*	to this is that FreeCoef contains real code.			*
*									*
*	Register Usage:  Depends on routine that called me.		*
*									*
*************************************************************************

	INCLUDE 'assembly.i'
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'hardware/custom.i'
	INCLUDE 'hardware/dmabits.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'

_AbsExecBase	EQU	4
io_Size		EQU	IO_SIZE

        INCLUDE    'gloequs.i'
	INCLUDE	   'devices/audio.i'
	INCLUDE    'narrator.i'
	INCLUDE    'private.i'

        EXTERN_SYS FreeMem

        XDEF	SYNDBG_AllocBuffer,SYNDBG_SaveOutput
	XDEF	DEVDBG_StoreParms
	XDEF	NARDBG_DebugMode,NARDBG_FreeCoef



*	;------	Allocate a buffer to use in saving the synthesizer's output.
	;	Called from SYNTH.ASM.

SYNDBG_AllocBuffer	RTS



*	;------	Save the synthesizer output in the user's buffer.
	;	Called from SYNTH.ASM.

SYNDBG_SaveOutput	RTS



*	;------	Store a pointer to the Parms data in the user's IORB.
	;	Called from DEVICE.ASM.

DEVDBG_StoreParms	RTS



*	;------	Test DEBUG bit in IORB.  If set go into infinite loop.
	;	Called from NARRATOR.ASM.

NARDBG_DebugMode	RTS



*	;------	Free the COEF buffer if the KEEPCOEF bit is NOT set.
	;	Called from NARRATOR.ASM

NARDBG_FreeCoef:
        MOVE.L    COEFPTR(A5),A1			;Get COEF buffer ptr
        MOVE.L    COEFSIZE(A5),D0			;Get COEF buffer length
   	BEQ.S	  FreeRtn				;Nothing to free up
	CALLSYS	  FreeMem				;Free COEF buffer
FreeRtn	RTS

	END

