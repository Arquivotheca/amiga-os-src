******************************************************************************
*
*	$Id: joyportinternal.i,v 40.3 93/03/26 17:40:21 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	joyportinternal.i,v $
* Revision 40.3  93/03/26  17:40:21  Jim2
* Using another of the JoyPort Status bits.
*
* Revision 40.2  93/03/19  11:44:18  Jim2
* Added bit defs for the double controller.  Removed some data defintions.
*
* Revision 40.1  93/03/10  19:52:02  Jim2
* Changed JP_STATUS flags since I gave them their own byte.
*
* Revision 40.0  93/03/04  11:45:06  Jim2
* Renamed the _2_UNKNOWN constants to _2_RECHECK.
*
* Revision 39.3  93/01/29  16:50:29  brummer
* changed name of internal flag for error checking.
*
* Revision 39.2  93/01/18  18:06:20  brummer
* Added BITDEFs for initialization flags/
*
* Revision 39.1  93/01/07  15:40:25  brummer
* Optimized controller state change numbers.
* Added field definition for internal state variables.
*
* Revision 39.0  93/01/06  14:29:34  Jim2
* Initial Release.
*
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************
;
; POTGOR bit definitions :
;
	BITDEF	POTGOR,DATLX,8
	BITDEF	POTGOR,DATLY,10
	BITDEF	POTGOR,DATRX,12
	BITDEF	POTGOR,DATRY,14
;
; POTGO bit definitions :
;
	BITDEF	POTGO,OE8,9
	BITDEF	POTGO,OE10,11
	BITDEF	POTGO,OE12,13
	BITDEF	POTGO,OE14,15
;
; JOY0DAT and JOY1DAT bit definitons :
;
	BITDEF	JOYDAT,X0,0
	BITDEF	JOYDAT,X1,1
	BITDEF	JOYDAT,Y0,8
	BITDEF	JOYDAT,Y1,9
;
; Joy Port Status flag equates :
;

	BITDEF	JP_STATUS,VBLANK_ACQUIRED,0	; vblank int acquired from potgo
	BITDEF	JP_STATUS,ALREADYRUNNING,1	; ReadJoyPort already running
	BITDEF	JP_STATUS,CLOCKING_CTLR,2	; critical section (no GAMEPORT access)
	BITDEF	JP_STATUS,FIRST_INIT_REQ,3	; first time init required
	BITDEF	JP_STATUS,2nd_CTLR,4
	BITDEF	JP_STATUS,2nd_TIME,5
	BITDEF	JP_STATUS,WAS_JOY,6		;Set if a joystick event is encountered.
;
; State change counter values :
;
JP_MOUSE_2_RECHECK	equ	10		; # of mouse range chgs befor going unknown
JP_JOYSTK_2_RECHECK	equ	8		; # of joystk range chgs befor going unknown

