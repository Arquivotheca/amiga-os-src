	TTL    '$Id: gameint.asm,v 36.5 90/04/13 12:43:38 kodiak Exp $
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
*	   **	    VERTB Game Port Interrupt code	 **
*   Source Control
*   --------------
*   $Id: gameint.asm,v 36.5 90/04/13 12:43:38 kodiak Exp $
*
*   $Locker:  $
*
*   $Log:	gameint.asm,v $
*   Revision 36.5  90/04/13  12:43:38  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.4  90/04/02  12:56:54  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.3  89/11/10  13:39:19  kodiak
*   make gameport exclusive access
*   
*   Revision 36.2  88/11/03  12:35:20  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 35.1  88/01/25  11:35:46  kodiak
*   remove unnecessary Disable Enable pair
*   
*   Revision 35.0  87/10/26  11:31:02  kodiak
*   initial from V34, but w/ stripped log
*   
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
	INCLUDE		"exec/strings.i"

	INCLUDE		"hardware/cia.i"
	INCLUDE		"hardware/custom.i"

	INCLUDE		"devices/timer.i"
	INCLUDE		"devices/inputevent.i"

	INCLUDE		"macros.i"
	INCLUDE		"gpdata.i"


*------ Imported Globals ---------------------------------------------

	XREF	    _custom
	XREF	    _ciaapra


*------ Imported Functions -------------------------------------------

	XREF_PGR	WritePotgo

	XREF		GDReadEvent

*------ Exported Functions -------------------------------------------

	XDEF	    GDInterrupt

*------ gameport.device/GDInterrupt ----------------------------------
*
*   NAME
*	GDInterrupt - vertical blank interrupt routine.
*
*   SYNOPSIS
*	continue = GDInterrupt(gamePortDev)
*	CC-zero-bit	       A1
*
*   FUNCTION
*	GDInterrupt uses the vertical blank interrupt to initiate a
*	check of the devices connected to the game ports.  After the
*	ports are read, any reports generated are queued and used to
*	satisfy any pending or subsequent I/O.
*
*   INPUTS
*	gamePortDev - the Game Port device pointer.
*
*   RETURNS
*	CC-zero so that other vertical blank routines may also run.
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*---------------------------------------------------------------------
GDInterrupt:
*	    ;-- D0-D1/A0-A1/A5-A6 are preserved by the server handler
		MOVE.L	A1,A6		    ; but A6 is not set up

*	;-- figure out what kind of controllers these are and interpret
*	;-- the results accordingly
		LEA	gd_Unit0(A6),A0
		BSR.S	gdIntCode
		LEA	gd_Unit1(A6),A0
		BSR.S	gdIntCode
		MOVE.W	gd_PotGo(A6),D0
		BTST	#4,D0
		BEQ.S	writePot
		BSET	#0,D0
writePot:
		MOVE.W	gd_PotMask(A6),D1
		LINKPGR	WritePotgo

		CLR	D0  ;do not presume to be the only VERTB routine
		RTS

*----------------------------------------------------------------------
gdIntCode:
		MOVEM.L D2-D4/A2-A4,-(A7)
		MOVE.L	A0,A2
		BCLR	#UB_INITIALIZE,gu_Flags(A2)
		BNE.S	dDone
*	    ;------ show another tick
		ADDQ.W	#1,gu_AccumTimeout+gt_TimeDelta(A2)
		BNE.S	typeC
		SUBQ.W	#1,gu_AccumTimeout+gt_TimeDelta(A2)	; keep at maximum
typeC:
		MOVE.B	gu_Type(A2),D0
		ble.s	dDone
		EXT.W	D0
		MOVE.B	gu_Flags(A2),D1
		AND.W	#4,D1		    ;0 for left, 4 for right port
		LSL	#2,D0
		MOVE.L	iCodes(D0.W),A1
		MOVE.L	#_custom,A0
		JSR	(A1)
		BRA.S	gotSomething

dDone:
		MOVEM.L (A7)+,D2-D4/A2-A4
		RTS

iCodes		EQU	*-(4*MINCONTROLLER)
		DC.L	pulsedXlator		 1
		DC.L	xYSwitchedXlator	 2
		DC.L	xYSwitchedXlator	 3

gotSomething:
*	    ;------ determine if the distance trigger criteria are met
		MOVEQ	#0,D3
		BTST	#UB_RELATIVE,gu_Flags(A2)
		BNE.S	tryDelta
*	    ;	 ------ determine distance for absolute device
		MOVE.B	D0,D1
		EXT.W	D1
		MOVE.W	D1,gu_AccumTimeout+gt_XDelta(A2)
		SUB.B	gu_LastSave+1(A2),D1	; get distance
		BPL.S	posXDiff
		NEG.B	D1
posXDiff:
		MOVE.W	gu_Timeout+gt_XDelta(A2),D2
		BEQ.S	doYDiff
		EXT.W	D1
		CMP.W	D1,D2
		BGT.S	doYDiff
		MOVE.B	D0,gu_LastSave+1(A2)	; save new position
		ST	D3

doYDiff:
		LSR.W	#8,D0
		MOVE.B	D0,D1
		EXT.W	D1
		MOVE.W	D1,gu_AccumTimeout+gt_YDelta(A2)
		SUB.B	gu_LastSave(A2),D1	; get distance
		BPL.S	posYDiff
		NEG.B	D1
posYDiff:
		MOVE.W	gu_Timeout+gt_YDelta(A2),D2
		BEQ.S	chkBuffer
		EXT.W	D1
		CMP.W	D1,D2
		BGT.S	chkBuffer
		MOVE.B	D0,gu_LastSave(A2)	; save new position
		ST	D3
		BRA.S	chkBuffer


tryDelta:
*	    ;	 ------ determine distance for relative device
		MOVE.B	D0,D1
		EXT.W	D1
		ADD.W	gu_AccumTimeout+gt_XDelta(A2),D1
		MOVE.W	D1,gu_AccumTimeout+gt_XDelta(A2)
		BPL.S	posXDelta
		NEG.W	D1
posXDelta:
		MOVE.W	gu_Timeout+gt_XDelta(A2),D2
		BEQ.S	doYTrig
		CMP.W	D1,D2
		SLE	D3
doYTrig:
		LSR.W	#8,D0
		EXT.W	D0
		ADD.W	gu_AccumTimeout+gt_YDelta(A2),D0
		MOVE.W	D0,gu_AccumTimeout+gt_YDelta(A2)
		BPL.S	posYDelta
		NEG.W	D0
posYDelta:
		MOVE.W	gu_Timeout+gt_YDelta(A2),D2
		BEQ.S	chkBuffer
		CMP.W	D0,D2
		BGT.S	chkBuffer
		ST	D3

chkBuffer:
*	    ;------ check for buffer overflow now
		MOVE.W	gu_BufTail(A2),D1
		ADDQ.W	#8,D1
		ANDI.W	#(GPBUFSIZE-1),D1
		CMP.W	gu_BufHead(A2),D1
		BEQ	dDone		    ; make no attempt to report

*	    ;------ check for key trigger
		SWAP	D0
		MOVEQ	#-1,D4
		MOVE.W	D0,D1
		MOVE.W	gu_AccumTimeout+gt_KeyHold(A2),D2
		EOR.W	D2,D1
		BEQ	trigYet
		AND.W	D0,D1
		BEQ.S	tryUpTrig
		BTST	#UTB_KEYDOWNS,gu_Timeout+gt_KeyFlags(A2)
		BEQ.S	tryJustUpTrig
*	    ;--	    find the most important button to change
		MOVEQ	#15,D2
findDown:
		LSL	#1,D1
		DBCS	D2,findDown
		MOVE.B	keyCode(D2.W),D4
		MOVEQ	#-1,D3
		MOVE.W	gu_AccumTimeout+gt_KeyHold(A2),D1
		BSET	D2,D1
		MOVE.W	D1,gu_AccumTimeout+gt_KeyHold(A2)
		BRA.S	gotATrig

keyCode:
		DC.B	$050,$051,$052,$053,$054,$055,$056,$057
		DC.B	$058,$059,$05A,$05B,$0FF,$06A,$069,$068

tryJustUpTrig:
		OR.W	D1,gu_AccumTimeout+gt_KeyHold(A2)	; set downs
		
tryUpTrig:
		MOVE.W	D0,D1
		MOVE.W	gu_AccumTimeout+gt_KeyHold(A2),D2
		EOR.W	D2,D1
		NOT.W	D0
		AND.W	D0,D1
		BEQ.S	trigYet
		BTST	#UTB_KEYUPS,gu_Timeout+gt_KeyFlags(A2)
		BEQ.S	trigJustYet

*	    ;--	    find the most important button to change
		MOVEQ	#15,D2
findUp:
		LSL	#1,D1
		DBCS	D2,findUp
		MOVE.B	keyCode(D2.W),D4
		OR.B	#$080,D4		; key up report
		MOVEQ	#-1,D3
		MOVE.W	gu_AccumTimeout+gt_KeyHold(A2),D1
		BCLR	D2,D1
		MOVE.W	D1,gu_AccumTimeout+gt_KeyHold(A2)
		BRA.S	trigYet

trigJustYet:
		NOT.W	D1
		AND.W	D1,gu_AccumTimeout+gt_KeyHold(A2)	; clear ups

trigYet:
		TST.B	D3
		BNE.S	gotATrig

*	    ;------ look for a timeout trigger
		MOVE.W	gu_Timeout+gt_TimeDelta(A2),D0
		BEQ	dDone
		CMP.W	gu_AccumTimeout+gt_TimeDelta(A2),D0
		BHI	dDone

gotATrig:
*	    ;-- enqueue the report
		MOVE.W	gu_BufTail(A2),D1
		MOVE.B	D4,gu_BufQueue(A2,D1.W)
		MOVE.B	gu_AccumTimeout+gt_KeyHold(A2),D0  ; stash 3 buttons
		LSR.B	#1,D0			    ;	for ie_Qualifiers
		AND.B	#$070,D0
		BTST	#UB_RELATIVE,gu_Flags(A2)    ; stash relative bit
		BEQ.S	gotRelQualifier
		OR.B	#IEQUALIFIER_RELATIVEMOUSE>>8,D0    ; better be $080
gotRelQualifier:
		MOVE.B	D0,gu_BufQueue+1(A2,D1.W)
		MOVE.W	gu_AccumTimeout+gt_XDelta(A2),gu_BufQueue+2(A2,D1.W)
		MOVE.W	gu_AccumTimeout+gt_YDelta(A2),gu_BufQueue+4(A2,D1.W)
		MOVE.W	gu_AccumTimeout+gt_TimeDelta(A2),gu_BufQueue+6(A2,D1.W)
		ADDQ.W	#8,D1
		ANDI.W	#(GPBUFSIZE-1),D1
		MOVE.W	D1,gu_BufTail(A2)

		CLR.W	gu_AccumTimeout+gt_TimeDelta(A2)
		CLR.W	gu_AccumTimeout+gt_XDelta(A2)
		CLR.W	gu_AccumTimeout+gt_YDelta(A2)

*	    ;-- if I/O is already pending, see if this will satisfy it
		MOVE.L	MP_MSGLIST(A2),A1
		TST.L	(A1)
		BEQ.S	nooneWaiting
		BSR	GDReadEvent
		BRA	dDone

nooneWaiting:
		BRA	dDone


*---------------------------------------------------------------------


*----------------------------------------------------------------------
*   pulsedXlator -- return codes for a mouse/trackball controller.
*
*   ENTRY
*	D1 -- =0 for left port, =4 for right port
*	A2 -- gu_ variables
*	A0 -- _custom
*   EXIT
*	D0 -- returned longword of port data
*	D1, D2 destroyed
*
pulsedXlator:
*	;-- read pins 9 & 5 to get buttons 2 & 3
		BSR	g23Buttons

*	;-- read counters
		LSR	#1,D1
		MOVE.W	joy0dat(A0,D1.W),D0
		MOVE.W	D0,D2
		ROR.W	#8,D0
		SUB.B	gu_CurrSave(A2),D0
		ROR.W	#8,D0
		SUB.B	gu_CurrSave+1(A2),D0
		MOVE.W	D2,gu_CurrSave(A2)   ;save for next time

*	;-- get button 1
		ADD.W	D1,D1

*----------------------------------------------------------------------
*   g1Button -- return button 1 from pin 6.
*
*   ENTRY
*	D1 -- =0 for left port, =4 for right port
*   EXIT
*	D0 -- only buttons 1 in bit 31 is affected
*	D1 destroyed
*
g1Button:
		LSR	#2,D1
		ADDQ	#6,D1
		BTST	D1,_ciaapra
		BNE.S	g1B1
		BSET	#31,D0
g1B1:
		RTS


*----------------------------------------------------------------------
*   g23Buttons -- return buttons 2 & 3 from pins 9 & 5.
*
*   ENTRY
*	D1 -- =0 for left port, =4 for right port
*	A0 -- _custom
*   EXIT
*	D0 -- zeroed, then buttons 2 & 3 in bits 30, 29
*
g23Buttons:
		MOVE.W	potinp(A0),D0
		LSR	D1,D0
		NOT.W	D0
		ANDI.W	#$0500,D0	;mask to pins 9, 5
		ROL.W	#7,D0
		ROR.B	#1,D0
		ROR.W	#2,D0
		SWAP	D0		;move buttons to bits 30, 29
		RTS


*----------------------------------------------------------------------
*   xYSwitchedXlator -- return codes for a switch X-Y controller.
*
*   ENTRY
*	D1 -- =0 for left port, =4 for right port
*	A0 -- _custom
*	A2 -- gu_ variables
*   EXIT
*	D0 -- returned longword of port data
*	D1, D2 destroyed
*
xYSwitchedXlator:
	;-- read pins 9 & 5 to get buttons 2 & 3
		BSR.S	g23Buttons

	;-- read the JOYxDAT to get pins 1,2,3,4
		MOVE.W	D1,D0
		LSR.W	#1,D0
		MOVE.W	joy0dat(A0,D0.W),D0
		ANDI.W	#$0303,D0
		ROR.B	#2,D0
		LSR.W	#5,D0
		MOVE.W	switchedTable(D0.W),D0

	;-- get button 1
		BRA.S	g1Button

switchedTable:
	    ;---    y, x: indexed by Y1,Y0,X1,X0
		DC.B	0,0
		DC.B	1,0
		DC.B	1,1
		DC.B	0,1
		DC.B	-1,0
		DC.B	0,0	;(illegal combination)
		DC.B	0,1	;(illegal combination)
		DC.B	-1,1
		DC.B	-1,-1
		DC.B	0,0	;(illegal combination)
		DC.B	0,0	;(illegal combination)
		DC.B	-1,0	;(illegal combination)
		DC.B	0,-1
		DC.B	1,-1
		DC.B	1,0	;(illegal combination)
		DC.B	0,0	;(illegal combination)

		END
