#ifndef DOS_DATETIME_H
#define DOS_DATETIME_H

/*
**	$VER: datetime.h 36.7 (12.7.90)
**	Includes Release 40.15
**
**	Date and time C header for AmigaDOS
**
**	(C) Copyright 1989-1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
*/

#ifndef DOS_DOS_H
#include "dos/dos.h"
#endif

/*
 *	Data structures and equates used by the V1.4 DOS functions
 * StrtoDate() and DatetoStr()
 */

/*--------- String/Date structures etc */
struct DateTime {
	struct DateStamp dat_Stamp;	/* DOS DateStamp */
	UBYTE	dat_Format;		/* controls appearance of dat_StrDate */
	UBYTE	dat_Flags;		/* see BITDEF's below */
	UBYTE	*dat_StrDay;		/* day of the week string */
	UBYTE	*dat_StrDate;		/* date string */
	UBYTE	*dat_StrTime;		/* time string */
};

/* You need this much room for each of the DateTime strings: */
#define	LEN_DATSTRING	16

/*	flags for dat_Flags */

#define DTB_SUBST	0		/* substitute Today, Tomorrow, etc. */
#define DTF_SUBST	1
#define DTB_FUTURE	1		/* day of the week is in future */
#define DTF_FUTURE	2

/*
 *	date format values
 */

#define FORMAT_DOS	0		/* dd-mmm-yy */
#define FORMAT_INT	1		/* yy-mm-dd  */
#define FORMAT_USA	2		/* mm-dd-yy  */
#define FORMAT_CDN	3		/* dd-mm-yy  */
#define FORMAT_MAX	FORMAT_CDN

#endif /* DOS_DATETIME_H */
