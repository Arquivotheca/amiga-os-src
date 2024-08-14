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
*	gameport device data definition
*
**********************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"		; for interrupts.i
	INCLUDE	"exec/interrupts.i"

	INCLUDE	"stddevice.i"
	INCLUDE	"gameport.i"


*------ Constants ----------------------------------------------------
GPIS_PRIORITY	EQU 0
GPBUFSIZE	EQU 8*8    ;must be power of 2 greater than 8

MINCONTROLLER	EQU GPCT_MOUSE
MAXCONTROLLER	EQU GPCT_ABSJOYSTICK

*------ UT_FLAGS ------
	BITDEF	UT,KEYDOWNS,0
	BITDEF	UT,KEYUPS,1

 STRUCTURE  GameportTrigger,0
    UBYTE	gt_KeyHold	    ; actually the whole word
    UBYTE	gt_KeyFlags	    ; actually the low byte
    UWORD	gt_TimeDelta
    UWORD	gt_XDelta
    UWORD	gt_YDelta
    LABEL	gt_SIZEOF


*------ gu_Flags ------
	BITDEF	U,INITIALIZE,1	;this port is being initialized
	BITDEF	U,RIGHTPORT,2	;MUST = BIT 2, set for the right game port
	BITDEF	U,RELATIVE,3	;the unit report type is relative

 STRUCTURE  GameportUnit,MP_SIZE
    UWORD	gu_OpenCnt		;count of opens by this task
    UWORD	gu_BufHead		;oldest byte in the buffer
    UWORD	gu_BufTail		;next slot in the buffer
    STRUCT	gu_BufQueue,GPBUFSIZE	;raw report buffer
    STRUCT	gu_Timeout,gt_SIZEOF	;trigger conditions
    STRUCT	gu_AccumTimeout,gt_SIZEOF	;accumulated trigger conditions
    UWORD	gu_LastSave		;last port state
    UWORD	gu_CurrSave		;info about the current state
    UBYTE	gu_Flags		;port flags
    UBYTE	gu_Type			;type of device connected to port
    LABEL	gu_SIZEOF


*------ gd_Flags ------
	BITDEF	GD,EXPUNGED,0	;device to be expunged when all closed

 STRUCTURE	GameportData,dd_SIZEOF
    APTR	gd_PotgoResource    ; the potgo resource
    STRUCT	gd_Unit0,gu_SIZEOF  ; the left game port
    STRUCT	gd_Unit1,gu_SIZEOF  ; the right game port
    STRUCT	gd_IS,IS_SIZE	    ; the interrupt server node
    UWORD	gd_PotAlloc	    ; desired allocated bits in potgo
    UWORD	gd_PotMask	    ; allocated bits in potgo
    UWORD	gd_PotGo	    ; working copy of potgo
    UBYTE	gd_Flags	    ; gameport flags
    ALIGNWORD
    LABEL	gd_SIZEOF
