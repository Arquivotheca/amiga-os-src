	IFND	SANA2_SANA2SPECIALSTATS_I
SANA2_SANA2SPECIALSTATS_I	SET	1
**
**	$Id: sana2specialstats.i,v 1.2 91/10/17 12:59:56 dlarson Exp $
**
**	Defined ids for SANA-II special statistics.
**
**
**	(C) Copyright 1991 Commodore-Amiga Inc.
**		All Rights Reserved
**


	IFND	SANA2_SANA2DEVICE_I
	INCLUDE	"sana2/sana2device.i"
	ENDC	!SANA2_SANA2DEVICE_I

;
; The SANA-II special statistic identifier is an unsigned 32 number.
; The upper 16 bits identify the type of network wire type to which
; the statistic applies and the lower 16 bits identify the particular
; statistic.
;

; If you'd like to add new statistic identifiers, contact CATS.

;
; defined ethernet special statistics
;

S2SS_ETHERNET_BADMULTICAST	EQU	((((S2WIRETYPE_ETHERNET)&$ffff)<<16)|$0000)
;
; This count will record the number of times a received packet tripped
; the hardware's multicast filtering mechanism but was not actually in
; the current multicast table.
;

S2SS_ETHERNET_RETRIES		EQU	((((S2WIRETYPE_ETHERNET)&$ffff)<<16)|$0001)
;
; This count records the total number of retries which have resulted
; from transmissions on this board.
;


	ENDC	SANA2_SANA2SPECIALSTATS_I
