	TTL	'PRIVATE.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


ioa_Size	EQU	ioa_SIZEOF


CALLSYS		MACRO	* &sysroutine
		JSR	_LVO\1(A6)
		ENDM


LINKSYS		MACRO	* &func
		LINKLIB _LVO\1,ND_SYSLIB(A6)
		ENDM


*		;------ Speech driver device block
ND_STACKSIZE	EQU	$200


 STRUCTURE ND,DD_SIZE
	STRUCT	ND_UNIT,UNIT_SIZE	;Unit 
	STRUCT  ND_TCB,TC_SIZE  	;Task control block
	APTR	ND_SYSLIB		;Pointer to system library
	STRUCT	ND_IORB3,ioa_Size	;IORB for audio commands
	APTR	ND_AUDDMAIORB		;Pointer to audio DMA IORBs
	APTR	ND_MSGPORT		;MsgPort for audio output commands
	APTR	ND_CMSGPORT		;MsgPort for audio control functions
	APTR	ND_GLOBALS		;Pointer to device global area
	APTR	ND_SEGLIST		;Segment list pointer
	ULONG	ND_PSEUDO		;Pseudo unit counter
	STRUCT	ND_STACK,ND_STACKSIZE	;Task's stack
	APTR	ND_USERIORB		;Pointer to currently active IORB
	ULONG	ND_SIGNALS		;The device signals
	LABEL	ND_SIZE			;Size of device structure


*		;------ Private device options.  These EQU's define
*			bits in the flags field of the IORB.

NDB_RTNPARMS	EQU    	4		;Return the addr of the Parms data
NDB_KEEPCOEF	EQU    	5		;Return COEF buffer to user
NDB_KEEPSYNTH	EQU    	6		;Return SYNTH output to user
NDB_DEBUG	EQU    	7		;Run synthesizer in debug mode


NDF_RTNPARMS	EQU	1 << NDB_RTNPARMS
NDF_KEEPCOEF	EQU	1 << NDB_KEEPCOEF
NDF_KEEPSYNTH	EQU	1 << NDB_KEEPSYNTH
NDF_DEBUG	EQU	1 << NDB_DEBUG


*		;------	Device equates

UNITB_SHUT	EQU	7
UNITB_STOP	EQU	6
UNITB_EXPUNGE	EQU	5

IOB_ABORT	EQU	5
IOB_ACTIVE	EQU	6
IOB_FIRST	EQU	7
