******************************************************************************
*
*	$Id: nvram.i,v 40.2 93/03/05 10:27:45 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	nvram.i,v $
* Revision 40.2  93/03/05  10:27:45  brummer
* Add address and ID for idetifying the Arizona Gate array
* in the game machine.
* 
* Revision 40.1  93/03/01  15:31:42  brummer
* Use tst.b of ciaa for delay so push/pop is not required in all
* the timing delay macros for NVRAM device access.
* Use andi and ori instead of move to CCR register in clear/set
* carry macros.
*
* Revision 40.0  93/02/25  12:04:44  brummer
* fix revision to 40.0
*
* Revision 39.4  93/02/25  11:30:06  brummer
* added equate for NVRAM device size (1024 bytes)
*
* Revision 39.3  93/02/03  14:22:29  brummer
* fix revision number
*
* Revision 1.3  93/02/02  18:11:26  brummer
* Shortened up delay macros.  Still within limits on game machine
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************
;
; Nonvolatile device address and interface equates :
;
nvram_device_size	equ	1024		; size in bytes of device
nvram_signal		equ	%10100000	; address signal
nvram_port_map		equ	$B80030		; AMIGA memory mapped address for chip data
nvram_dir_map		equ	$B80032		; AMIGA memory mapped address for chip OE
nvram_delay_count	equ	6		; loop count for device (*3 us)

	BITDEF	READCMD,,24		; read command bit

	BITDEF	CLOCK,HIGH,31		; clock bit
	BITDEF	DATA,HIGH,30		; data bit
	BITDEF	CLOCK,OE,15		; clock output enable bit
	BITDEF	DATA,OE,14		; data output enable bit
;
; Arizona gate array address and ID tags :
;
nvram_arizona_id_adrs	equ	$B80000		; location of long word ID
nvram_arizona_id1	equ	$BEEF		; first ID
nvram_arizona_id2	equ	$CAFE		; second id
;
; Delay macros for nonvolatile device timing :
;
DELAY_SUSTA	macro			; Start Set-Up time is 4.7 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_HDSTA	macro			; Start Hold time is 4.0 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_SUSTO	macro			; Stop Set-Up time is 4.7 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_CLKHIGH	macro			; Clock pulse width high is 4.0 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_CLKLOW	macro			; Clock pulse width low is 4.7 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_BUF	macro			; Clock pulse width low is 4.7 us
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_AA	macro			; Wait for data to become valid
		NOLIST
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		LIST
                endm

DELAY_WR	macro			; Wait for device write cycle
		NOLIST
		movem.l	d1,-(sp)	;
		move.w	#1000,d1	;
13$:		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		tst.b	_ciaa+ciapra	;
		dbra	d1,13$		;
		movem.l	(sp)+,d1	;
		LIST
                endm
;
; Miscellaneous coding macros :
;
CLEAR_CARRY	macro
		andi	#~1,CCR
		endm

SET_CARRY	macro
		ori	#1,CCR		;
		endm

