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
*	Keyboard device data definition
*   Source Control
*   --------------
*   $Id: kbdata.i,v 36.8 90/12/14 19:00:31 darren Exp $
*
*   $Locker:  $
*
*   $Log:	kbdata.i,v $
*   Revision 36.8  90/12/14  19:00:31  darren
*   Increased max key to $71, which equates with the END key
*   on the 101 keyboard.  Hard coded matrix in the interrupt
*   handler is already big enough (16 bytes of bits to check
*   for numeric keypad keys).  Current size of matrix is
*   now 15 bytes large ($71+8/8)
*   
*   Revision 36.7  90/04/13  12:46:13  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.6  90/04/02  12:59:17  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.5  89/08/31  17:27:01  kodiak
*   perform key handshake timeout w/ timer ReadEClock function
*   
*   Revision 36.4  89/07/11  16:50:31  kodiak
*   add ALIGNWORD to device structure
*   
*   Revision 36.3  89/02/20  13:10:26  kodiak
*   make names reflect usage: CIA-A, not B
*   
*   Revision 36.2  88/11/03  12:35:28  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 35.0  87/10/26  11:34:05  kodiak
*   initial from V34, but w/ stripped log
*   
*   
**********************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"		; for interrupts.i
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"devices/timer.i"

	INCLUDE	"stddevice.i"


*------ Constants ----------------------------------------------------
HIGH_KEYCODE	EQU $71
RESET_CODE	EQU $78
OVERFLOW_CODE	EQU $7D
MATRIX_BYTES	EQU (HIGH_KEYCODE+8)/8
KBBUFSIZE	EQU 128		;must be power of 2 greater than 4
IKC_SIZE	EQU 10

*------ kd_Flags ------
	BITDEF	KD,RESET1,0	; first reset code received
	BITDEF	KD,RESET2,1	; second reset code received

 STRUCTURE	KeyboardData,dd_SIZEOF
    STRUCT	kd_Unit,MP_SIZE     ; the one and only unit
    APTR	kd_CIAAResource	    ;the CIA-A resource address
    STRUCT	kd_IS,IS_SIZE	    ;the interrupt server node
    UWORD	kd_BufHead	    ;oldest byte in the buffer
    UWORD	kd_BufTail	    ;next slot in the buffer
    STRUCT	kd_BufQueue,KBBUFSIZE ;raw key buffer
    STRUCT	kd_Tick,IO_SIZE		; the timer.device
    STRUCT	kd_EClock1,EV_SIZE	; the eclock counter
    STRUCT	kd_EClock2,EV_SIZE	; the eclock counter
    STRUCT	kd_HandlerList,LH_SIZE		; reset handler list
    UWORD	kd_OutstandingResetHandlers	; # of handlers caused
    UBYTE	kd_Flags	    ;keyboard flags
    UBYTE	kd_Shifts	    ;keyboard shift bits
    STRUCT	kd_Matrix,MATRIX_BYTES	;key matrix
    ALIGNWORD
    LABEL	kd_SIZEOF	    ;warning! this may be odd
