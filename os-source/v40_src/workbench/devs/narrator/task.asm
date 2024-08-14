
	TTL	'TASK.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


               	SECTION speech

_AbsExecBase	EQU	4


******* Included Files ***********************************************


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

io_Size	EQU	IO_SIZE
        INCLUDE    'featequ.i'
        INCLUDE    'gloequs.i'
        INCLUDE    'devices/audio.i'
	INCLUDE    'narrator.i'
	INCLUDE    'private.i'



******* Imported Functions *******************************************


	EXTERN_SYS Signal
	EXTERN_SYS AllocSignal
	EXTERN_SYS Wait
	EXTERN_SYS GetMsg
	EXTERN_SYS FindTask



******* Exported Functions *******************************************

	XDEF	SDTaskStart
	XREF	PerformIO

*	XREF	_AbsExecBase



******* Device/Speech/Task **********************************
*
*   NAME

*
*   SYNOPSIS

*
*   FUNCTION
*
*
*   INPUTS

*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*



SDTaskStart:
		MOVE.L	_AbsExecBase,A6
		MOVE.L	4(SP),A2		;Device Node
		MOVE.L	A2,A3			;Copy it
		LEA	ND_UNIT(A2),A2		;Get unit pointer


*		;------	Setup task signalling
		SUB.L	A1,A1			;Store pointer to TCB in msgport
		CALLSYS	FindTask
		MOVE.L	D0,MP_SIGTASK(A2)

		MOVEQ	#0,D2			;Allocate and store task signal bit
		MOVEQ	#-1,D0
		CALLSYS	AllocSignal
		BSET	D0,D2
		MOVE.B	D0,MP_SIGBIT(A2)

		MOVEQ	#-1,D0			;Allocate and store task start signal
		CALLSYS	AllocSignal
		BSET	D0,D2
		MOVE.L	D2,ND_SIGNALS(A3)

SDTst:
		BSR.S	ND_Per

SDTask_Wait:	
		MOVE.L	ND_SIGNALS(A3),D0
		CALLSYS	Wait
		BRA.S	SDTst

ND_Per:
		BTST	#UNITB_STOP,UNIT_FLAGS(A2)	;Device stopped?
		BNE.S	SDTask_End			;Yes, wait for next signal

		BSET 	#UNITB_ACTIVE,UNIT_FLAGS(A2) 	;Device busy?
		BNE.S	SDTask_End			;Yes, wait for next signal

*		BEQ.S	SDTask_Cont			;No, continue
*		MOVE.L	#$00020000,D0
*		MOVEQ	#0,D0
*		MOVE.B	ND_WAITSIG(A3),D1
*		BSET	D1,D0
*		CALLSYS	Wait

SDTask_Cont	MOVE.L	A2,A0
		CALLSYS	GetMsg
		TST.L	D0
		BEQ.S	SDTask_Clear

		MOVE.L	D0,A1				;Move IORB into A1
		CMP.W	#CMD_READ,IO_COMMAND(A1)	;Is this a Read?
		BEQ.S	SDTask_1			;Yes, don't save IORB in devNode
		MOVE.L	A1,ND_USERIORB(A3)		;Save in devNode
SDTask_1	BSET	#UNITB_INTASK,UNIT_FLAGS(A2)
		JSR	PerformIO 			;Go to Command interpreter???
		BRA.S	ND_Per

SDTask_Clear	BCLR	#UNITB_INTASK,UNIT_FLAGS(A2)	;Clear in task flag
		BCLR	#UNITB_ACTIVE,UNIT_FLAGS(A2)	;Clear driver active flag

SDTask_End	RTS
		END




