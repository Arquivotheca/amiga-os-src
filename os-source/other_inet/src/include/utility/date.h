#ifndef UTILITY_DATE_H
#define UTILITY_DATE_H
/*
**	$Id: date.h,v 39.1 92/01/20 11:19:24 vertex Exp $
**
**	Date conversion routines ClockData definition.
**
**	(C) Copyright 1989-1992 Commodore-Amiga Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/*****************************************************************************/


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


/*****************************************************************************/


#endif /* UTILITY_DATE_H */
