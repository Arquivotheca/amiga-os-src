	IFND UTILITY_DATE_I
UTILITY_DATE_I	SET	1
**
**	$Id: date.i,v 39.1 92/01/20 11:19:41 vertex Exp $
**
**	Date conversion routines ClockData definition.
**
**	(C) Copyright 1989-1992 Commodore-Amiga Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

	IFND EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC

;---------------------------------------------------------------------------

   STRUCTURE CLOCKDATA,0
	UWORD	sec
	UWORD	min
	UWORD	hour
	UWORD	mday
	UWORD	month
	UWORD	year
	UWORD	wday
   LABEL CD_SIZE

;---------------------------------------------------------------------------

	ENDC	; UTILITY_DATE_I
