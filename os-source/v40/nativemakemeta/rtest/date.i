	IFND	UTILITY_DATE_I
UTILITY_DATE_I	SET	1
**
**	$Id: date.i,v 36.1 90/07/12 15:40:00 rsbx Exp $
**
**	Date conversion routines ClockData definition.
**
**	(C) Copyright 1989,1990 Commodore-Amiga Inc.
**		All Rights Reserved
**

	INCLUDE	"exec/types.i"

 STRUCTURE CLOCKDATA,0
	UWORD	sec
	UWORD	min
	UWORD	hour
	UWORD	mday
	UWORD	month
	UWORD	year
	UWORD	wday
	LABEL	CD_SIZE

	ENDC	; UTILITY_DATE_I
