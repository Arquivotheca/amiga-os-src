*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* date.i
*
* Source Control
* ------ -------
* 
* $Header$
*
* $Locker$
*
*************************************************************************

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
