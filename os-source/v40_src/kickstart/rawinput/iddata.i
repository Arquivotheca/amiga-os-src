**********************************************************************
*                                                                    *
*   Copyright 1985,1991, Commodore-Amiga Inc.   All rights reserved. *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	input task data definitions
*
*   Source Control
*   --------------
*   $Id: iddata.i,v 36.6 91/05/03 10:42:51 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	iddata.i,v $
*   Revision 36.6  91/05/03  10:42:51  bryce
*   _Major_ speedup here.  The input.device stack imbedded in the device
*   base was _perfectly_ misaligned.  All input, input task switch, Intuition,
*   and Intuition started layers activity was running with this bogus stack.
*   Benchmarks show this one-liner change results in a 10% improvement in
*   overally Intuition processing.  Should be even better for layers tests.
*   (Improvement will be noticed on 68020/68030/68040 systems with 16 or 32
*   bit ram).
*   
*   Revision 36.5  91/01/21  12:19:28  darren
*   Some defines for start of, and end of qualifier keys.
*   
*   Revision 36.4  90/05/10  18:47:58  kodiak
*   use GetSysTime function instead of GETSYSTIME command
*   
*   Revision 36.3  90/04/13  12:46:20  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.2  90/04/02  12:57:32  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.1  90/01/19  18:05:53  kodiak
*   make SetMXxxx commands synchronous
*   
*   Revision 35.3  89/08/08  09:54:08  kodiak
*   fix repeat strategy to allow corruption of repeat input event on food chain
*   
*   Revision 35.2  89/07/12  13:28:23  kodiak
*   have input task ReadEvent multiple events from gameport
*   (and have it work)
*   
*   Revision 35.1  89/07/07  14:08:31  kodiak
*   double buffer requests to keyboard device
*   
*   Revision 35.0  87/10/26  11:33:34  kodiak
*   initial from V34, but w/ stripped log
*   
*
**********************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"devices/timer.i"
	INCLUDE	"gameport.i"


*------ constants ----------------------------------------------------
ID_STKSIZE	EQU	$1000
ID_PRIORITY	EQU	20

ID_TEVENTSECS	EQU	0
ID_TEVENTMICRO	EQU	100000		; 10 times/sec

ID_THRESHSECS	EQU	0
ID_THRESHMICRO	EQU	800000		; .8 seconds

ID_PERIODSECS	EQU	0
ID_PERIODMICRO	EQU	100000		; 10 times/sec

ID_QUALMASK	EQU	$00FF
ID_KEYMASK	EQU	$07FF
ID_MOUSEMASK	EQU	$F000

MOUSEAHEAD	EQU	7


START_QUAL    	EQU	$60		;Qualifier keys start
END_QUAL	EQU	$6A		;and end inclusive

 STRUCTURE	InputData,dd_SIZEOF
	STRUCT	id_Unit,MP_SIZE   ; the one and only unit
	STRUCT	id_TC,TC_SIZE		; task space
	WORD	id_RepeatCode		; repeating key code (or -1 for none)
	STRUCT	id_Stk,ID_STKSIZE	;	and stack space
	STRUCT	id_HandlerList,LH_SIZE	; input handler function list
	STRUCT	id_IEPort,MP_SIZE	; Input Event message port,
	STRUCT	id_TData,ie_SIZEOF	;	timer event area,
	STRUCT	id_TIOR,IOTV_SIZE	;	and timer request block
	STRUCT	id_MData,ie_SIZEOF*MOUSEAHEAD ;	mouse data area,
	STRUCT	id_MIOR,IOSTD_SIZE	;	and mouse request block
	STRUCT	id_K1Data,ie_SIZEOF	;	keyboard data area 1,
	STRUCT	id_K1IOR,IOSTD_SIZE	;	and keyboard request block
	STRUCT	id_K2Data,ie_SIZEOF	;	keyboard data area 2,
	STRUCT	id_K2IOR,IOSTD_SIZE	;	and keyboard request block
	STRUCT	id_RIOR,IOTV_SIZE	;	repeat key timer request block
	STRUCT	id_Thresh,TV_SIZE	; repeat key threshold
	STRUCT	id_Period,TV_SIZE	; repeat key period
	STRUCT	id_RepeatKey,ie_SIZEOF	; repeating key event
	UWORD	id_RepeatNumeric	;   IEQUALIFIER_NUMERICPAD set or clear
	UWORD	id_Qualifier		; the current key & mouse qualifiers
	LABEL	id_Prev1Down		; the most previous down key
	UBYTE	id_Prev1DownCode	;	high byte is keycode
	UBYTE	id_Prev1DownQual	;	low byte is low of qualifiers
	UWORD	id_Prev2Down		; the second most previous down key
	UBYTE	id_MPort		; the mouse controller port
	UBYTE	id_MType		; the mouse controller type
	STRUCT	id_MTrig,gpt_SIZEOF	; the mouse controller trigger
	UBYTE	id_KRActiveMask		; $00 or $ff, to double buffer keyboard
	LABEL	id_SIZEOF
