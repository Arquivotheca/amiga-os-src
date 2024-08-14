#ifndef UTILITY_DATE_H
#define UTILITY_DATE_H 1
/*
**	$Id: date.h,v 36.1 90/07/12 15:39:41 rsbx Exp $
**
**	Date conversion routines ClockData definition.
**
**	(C) Copyright 1989,1990 Commodore-Amiga Inc.
**		All Rights Reserved
*/

#include "exec/types.h"


struct ClockData
	{
	UWORD sec;
	UWORD min;
	UWORD hour;
	UWORD mday;
	UWORD month;
	UWORD year;
	UWORD wday;
	};

#endif
