	TTL	'MULTCHAN.ASM'


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
	INCLUDE 'hardware/dmabits.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'

	INCLUDE	'devices/audio.i'
	INCLUDE	'narrator.i'
	INCLUDE	'private.i'


******* Imported Functions *******************************************

	EXTERN_SYS AllocMem
	EXTERN_SYS FreeMem
	EXTERN_SYS DoIO
	EXTERN_SYS WaitIO
	EXTERN_SYS FindTask
	EXTERN_SYS AllocSignal
	EXTERN_SYS FreeSignal
	

******* Exported Functions *******************************************

	XDEF	AllocMultChan,FreeMultChan


******* Imported Globals   *******************************************

	XREF	DMANodeName



*******	Private/Device/Narrator/AllocMultChan*************************
*
*   NAME
*	AllocMultChan - Handles initialization of channels
*
*
*   SYNOPSIS
*
*
*   FUNCTION
*	AllocMultChan allocates the audio channels specified by
*	the channel masks array in the user's IORB; creates the
*	message port for the audio DMA requests; and allocates
*	and fills the DMA IORBs.
*	
*
*   INPUTS
*	A2 - User IORB
*	A3 - Device node
*	A5 - Synthesizer globals area
*	A6 - AbsExecBase
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
*	Stomps on registers A0, A1, A4, D0, D1, and D3
*
*
*   IMPLEMENTATION
*
*


AllocMultChan:

*	;------	Initialize 

	CLR.B	NDI_CHANMASK(A2)
	CLR.B	NDI_NUMCHAN(A2)
	CLR.L	ND_AUDDMAIORB(A3)
	CLR.L	ND_MSGPORT(A3)

	
*	;------	Allocate the audio channels

	LEA	ND_IORB3(A3),A1
	MOVE.B	#75,LN_PRI(A1)				;!!!sam 10/23/85!!!
	MOVE.B	#ADIOF_NOWAIT+IOF_QUICK,IO_FLAGS(A1)
	MOVE.L	NDI_CHMASKS(A2),ioa_Data(A1)
*	MOVE.W	NDI_NUMMASKS(A2),ioa_Length(A1)		;!!!sam 7/12/85!!!
	MOVE.W	NDI_NUMMASKS(A2),ioa_Length+2(A1)	;!!!sam 7/12/85!!!
	MOVE.W	#ADCMD_ALLOCATE,IO_COMMAND(A1)
	MOVE.L	A1,-(SP)				;!!!sam 7/12/85!!!
	BEGINIO
	MOVE.L	(SP),A1					;!!!sam 7/12/85!!!
	CALLSYS	WaitIO					;!!!sam 7/12/85!!!
*	CALLSYS	DoIO					;!!!sam 7/12/85!!!
	MOVE.L	(SP)+,A1				;!!!sam 7/12/85!!!
	TST.B	IO_ERROR(A1)
	BEQ.S	AMC_AllocOK
	MOVE.B	#ND_CantAlloc,D0
	BRA	AMC_Return


AMC_AllocOK:
*	;------	Stop all channels.  THIS CODE MOVE TO SYNTH (qv).
*
*	MOVE.W	#CMD_STOP,IO_COMMAND(A1)
*	MOVE.L	A1,-(SP)				;!!!sam 7/12/85!!!
*	CALLSYS	DoIO
*	MOVE.L	(SP)+,A1				;!!!sam 7/12/85!!!


*	;------	Save channel mask in user's IORB

	MOVE.L	IO_UNIT(A1),D1
	MOVE.B	D1,NDI_CHANMASK(A2)
	


*	;------	Create message port for audio DMA requests

	MOVE.L	#MP_SIZE,D0			;Allocate memory
	MOVE.L	#MEMF_PUBLIC+MEMF_CLEAR,D1
	CALLSYS	AllocMem
	TST.L	D0
	BNE.S	AMC_MPAllocOK
	MOVEQ	#ND_NoMem,D3
	BRA	AMC_Return

AMC_MPAllocOK:
	MOVE.L	D0,A4
	MOVE.L	A4,ND_MSGPORT(A3)
	MOVE.B	#NT_MSGPORT,MP+LN_TYPE(A4)	;Type
	LEA	DMANodeName,A0			;Node name
	MOVE.L	A0,MP+LN_NAME(A4)
	MOVEQ	#-1,D0				;Signal
	CALLSYS	AllocSignal
	MOVE.B	D0,MP_SIGBIT(A4)
	SUB.L	A1,A1				;Task
	CALLSYS	FindTask
	MOVE.L	D0,MP_SIGTASK(A4)
	LEA	MP_MSGLIST(A4),A0		;Initialize msg list
	NEWLIST	A0


*	;-----	Allocate audio DMA IORBs.  Two per channel.
	
	LEA	Mask2Num,A0
	MOVEQ	#0,D0
	MOVEQ	#0,D1
	MOVE.B	NDI_CHANMASK(A2),D1
	MOVE.B	0(A0,D1),D0
	BNE.S	AMC_GotChans
	MOVE.B	#ND_CantAlloc,D0		;Die if 0 channels
	BRA	AMC_Return

AMC_GotChans:
	MOVE.B	D0,NDI_NUMCHAN(A2)		;Save in user IORB	
	ADD.W	D0,D0
	MULU	#ioa_Size,D0
	MOVE.L	#MEMF_PUBLIC+MEMF_CLEAR,D1
	CALLSYS	AllocMem
	TST.L	D0
	BNE.S	AMC_GotMem
	MOVE.B	#ND_NoMem,D0
	BRA	AMC_Return

AMC_GotMem:
	MOVE.L	D0,ND_AUDDMAIORB(A3)		;Save ptr in devNode


*	;------	Got memory for IORBs.  Copy control
*		IORB information to DMA IORBs. 

	MOVE.L	A2,-(SP)
	MOVEQ	#0,D3
	MOVE.B	NDI_NUMCHAN(A2),D3		;Number of IORB pairs
	SUBQ.W	#1,D3
	LEA	ND_IORB3(A3),A0			;Control IORB
	MOVE.L	D0,A1				;DMA IORB 1

AMC_IORBCopy:
	LEA	ioa_Size(A1),A2			;DMA IORB 2
	MOVE.W	#(ioa_Size/2)-1,D1

AMC_CopyLoop:
	MOVE.W	(A0)+,D0
	MOVE.W	D0,(A1)+
	MOVE.W	D0,(A2)+
	DBF	D1,AMC_CopyLoop

	LEA	ND_IORB3(A3),A0			;Reset control IORB
	MOVE.L	A2,A1				;Next IORB pair
	DBF	D3,AMC_IORBCopy
	MOVE.L	(SP)+,A2
	MOVEQ	#0,D0				;Good return code

AMC_Return:
*	;------	Time to go away
	
	RTS


Mask2Num:
*	;------	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	DC.B	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4




*******	Private/Device/Narrator/FreeMultChan *************************
*
*   NAME
*	FreeMultChan - Frees the channels allocated  by AllocMultChan
*
*
*   SYNOPSIS
*
*
*   FUNCTION
*	FreeMultChan frees the channels in use and returns the
*	memory allocated for the DMA IORBs and the msgport.
*	
*
*   INPUTS
*	A1 - User IORB
*	A3 - Device node
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
*	All registers are preserved
*
*
*   IMPLEMENTATION
*
*

FreeMultChan:
	MOVEM.L	A0/A1/A2/A6/D0/D1,-(SP)		;Save registers
	MOVE.L	_AbsExecBase,A6


*	;------	Free allocated channels

	MOVE.L	A1,A2
	LEA	ND_IORB3(A3),A1
	MOVE.B	NDI_CHANMASK(A2),IO_UNIT+3(A1)
	MOVE.W	#ADCMD_FREE,IO_COMMAND(A1)
	CALLSYS	DoIO


*	;------	Free DMA IORB memory

	MOVE.L	ND_AUDDMAIORB(A3),D0
	BEQ.S	FMC_FreeMsgport
	MOVE.L	D0,A1
	MOVEQ	#0,D0
	MOVE.B	NDI_NUMCHAN(A2),D0
	ADD.W	D0,D0
	MULU	#ioa_Size,D0
	CALLSYS	FreeMem


FMC_FreeMsgport:
	MOVE.L	ND_MSGPORT(A3),D0
	BEQ.S	FMC_ResetKey
	MOVE.L	D0,A1


*	;------ Free up the signal

	MOVE.B	MP_SIGBIT(A1),D0
	CALLSYS	FreeSignal
	

*	;------	Free DMA Msgport

	MOVE.L	ND_MSGPORT(A3),A1
	MOVE.L	#MP_SIZE,D0
	CALLSYS	FreeMem


FMC_ResetKey:
*	;------	Reset Allocation key in control IORB

	LEA	ND_IORB3(A3),A0
	CLR.W	ioa_AllocKey(A0)


	MOVEM.L	(SP)+,A0/A1/A2/A6/D0/D1
	RTS

	END
	
