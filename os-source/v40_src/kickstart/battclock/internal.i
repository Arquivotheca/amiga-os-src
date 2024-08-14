*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* internal.i
*
* Source Control
* ------ -------
* 
* $Id: internal.i,v 36.5 91/01/09 10:04:53 rsbx Exp $
*
*************************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/semaphores.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"devices/timer.i"
	INCLUDE	"asmsupp.i"
	INCLUDE	"utility/date.i"
	INCLUDE	"battclock.i"
	INCLUDE	"battclock_rev.i"


NO_CLOCK	equ	0
OKI_CLOCK	equ	1
RICOH_CLOCK	equ	2
OKI_RESET	equ	3
RICOH_RESET	equ	4
MAGIC		equ	1234

CLOCKBASE	equ	$dc0000


 STRUCTURE ClockTemp,0
	UBYTE	Oki_one_sec	; Ricoh_one_sec
	UBYTE	Oki_ten_sec	; Ricoh_ten_sec
	UBYTE	Oki_one_min	; Ricoh_one_min
	UBYTE	Oki_ten_min	; Ricoh_ten_min
	UBYTE	Oki_one_hour	; Ricoh_one_hour
	UBYTE	Oki_ten_hour	; Ricoh_ten_hour
	UBYTE	Oki_one_mday	; Ricoh_one_wday
	UBYTE	Oki_ten_mday	; Ricoh_one_mday
	UBYTE	Oki_one_month	; Ricoh_ten_mday
	UBYTE	Oki_ten_month	; Ricoh_one_month
	UBYTE	Oki_one_year	; Ricoh_ten_month
	UBYTE	Oki_ten_year	; Ricoh_one_year
	UBYTE	Oki_one_wday	; Ricoh_ten_year
	UBYTE	padd
	LABEL	ClockTempSize


 STRUCTURE RawClock,0
	STRUCT	filler0,3
	UBYTE	Reg0
	STRUCT	filler1,3
	UBYTE	Reg1
	STRUCT	filler2,3
	UBYTE	Reg2
	STRUCT	filler3,3
	UBYTE	Reg3
	STRUCT	filler4,3
	UBYTE	Reg4
	STRUCT	filler5,3
	UBYTE	Reg5
	STRUCT	filler6,3
	UBYTE	Reg6
	STRUCT	filler7,3
	UBYTE	Reg7
	STRUCT	filler8,3
	UBYTE	Reg8
	STRUCT	filler9,3
	UBYTE	Reg9
	STRUCT	fillera,3
	UBYTE	Rega
	STRUCT	fillerb,3
	UBYTE	Regb
	STRUCT	fillerc,3
	UBYTE	Regc
	STRUCT	fillerd,3
	UBYTE	Regd
	STRUCT	fillere,3
	UBYTE	Rege
	STRUCT	fillerf,3
	UBYTE	Regf
	LABEL	RawClockSize

 STRUCTURE BattClockResource,LIB_SIZE
	UWORD	BTC_Magic

	APTR	BTC_Exec
	APTR	BTC_UtilLib
	APTR	BTC_ClockBase

	STRUCT	BTC_Semaphore,SS_SIZE

	LABEL	BTC_SIZE
