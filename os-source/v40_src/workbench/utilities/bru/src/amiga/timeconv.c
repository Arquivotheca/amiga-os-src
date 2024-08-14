/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	timeconv.c    routines to convert between AmigaDOS time and Unix time
 *
 *  SCCS
 *
 *	@(#)timeconv.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Routines to convert between AmigaDOS's idea of time and
 *	Unix's idea of time.  Unix keeps time in a long integer,
 *	in seconds since 00:00:00 GMT Jan 1, 1970.  AmigaDOS keeps
 *	the date in an array of three longs (days, min, ticks),
 *	since Jan 1, 1978, with no concept of timezones.
 *
 *	As a quick hack, we can ignore timezone differences, in which
 *	case the difference is a fixed value (252,460,800 seconds).
 *	There does not currently appear to be any supported method
 *	of determining the timezone for the Amiga.
 *
 *	Also, a third time format must be supported, the time format
 *	expected by Lattice library calls.  This format is like Unix
 *	time, except that it is zero based from Jan 1, 1978.  Note that
 *	this conversion just applies the Unix/AmigaDOS time skew.
 *
 */

#include "globals.h"

static VOID tzinit PROTO((void));

#define SECONDS_PER_DAY (60 * 60 * 24)
#define SECONDS_PER_MINUTE (60)
#define TIME_SKEW (252460800L)

extern int daylight;

static int initdone = 0;


/*
 *  FUNCTION
 *
 *	tzinit    attempt to initialize timezone from environment
 *
 *  SYNOPSIS
 *
 *	static VOID tzinit ()
 *
 *  DESCRIPTION
 *
 *	Attempt to set the timezone variable based on the value
 *	of the environment variable "TZ".  The format of the
 *	TZ environment variable follows standard Unix conventions
 *	documented under ctime(3C) in the Unix library.  This
 *	internal routine is called exactly once, the first time
 *	a conversion is attempted.
 *
 *	"The value of TZ must be a three-letter time zone name, followed
 *	by a number representing the difference between local time and
 *	Greenwich Mean Time in hours, followed by an optional three-
 *	letter name for a daylight time zone.  For example, the setting
 *	for New Jersey would be EST5EDT.  The effects of setting TZ are
 *	thus to change the values of the external variables timezone and
 *	daylight; in addition, the time zone names contained in the external
 *	variable tzname[] are set from the environment variable TZ."
 *
 */

static VOID tzinit ()
{
    char *tz;
    int tzdiff = 0;
    extern char *s_getenv ();

    DBUG_ENTER ("tzinit");
    tz = s_getenv ("TZ");
    if (tz != NULL) {
	DBUG_PRINT ("tz", ("TZ='%s'", tz));
	if (strlen (tz) > 3) {
	    tz += 3;
	    while (isdigit (*tz)) {
		tzdiff *= 10;
		tzdiff += (*tz++ - '0');
	    }
	    DBUG_PRINT ("tz", ("tzdiff = %d", tzdiff));
	    timezone = tzdiff * 60 * 60;
	    if (*tz != '\000') {
		daylight = 1;
	    }
	}
    }
    DBUG_PRINT ("tz", ("timezone set to %ld", timezone));
    DBUG_PRINT ("tz", ("daylight set to %d", daylight));
    initdone = 1;
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	dos2unix    convert AmigaDOS time to Unix time
 *
 *  SYNOPSIS
 *
 *	long dos2unix (datep)
 *	struct DateStamp *datep;
 *
 *  DESCRIPTION
 *
 *	Given pointer to an AmigaDOS date structure, convert it
 *	to the equivalent Unix time.
 */

long dos2unix (datep)
struct DateStamp *datep;
{
    long uxtime;

    DBUG_ENTER ("dos2unix");
    DBUG_PRINT ("dostime", ("datep -> ds_Days = %ld", datep -> ds_Days));
    DBUG_PRINT ("dostime", ("datep -> ds_Minute = %ld", datep -> ds_Minute));
    DBUG_PRINT ("dostime", ("datep -> ds_Tick = %ld", datep -> ds_Tick));
    if (!initdone) {
	tzinit ();
    }
    uxtime = datep -> ds_Days * SECONDS_PER_DAY;
    uxtime += datep -> ds_Minute * 60;
    uxtime += datep -> ds_Tick / TICKS_PER_SECOND;
    uxtime += TIME_SKEW;
    uxtime += timezone;
    DBUG_PRINT ("dos2unix", ("unix time = %ld", uxtime));
    DBUG_RETURN (uxtime);
}

/*
 *  FUNCTION
 *
 *	unix2dos    convert Unix time to AmigaDOS time
 *
 *  SYNOPSIS
 *
 *	VOID unix2dos (unixtime, datep)
 *	long unixtime;
 *	struct DateStamp *datep;
 *
 *  DESCRIPTION
 *
 *	Given a time in Unix format, and pointer to an AmigaDOS
 *	time structure, convert the Unix time to the AmigaDOS time.
 */

VOID unix2dos (unixtime, datep)
long unixtime;
struct DateStamp *datep;
{
    DBUG_ENTER ("unix2dos");
    DBUG_PRINT ("unixtime", ("unix time = %ld", unixtime));
    if (!initdone) {
	tzinit ();
    }
    unixtime -= TIME_SKEW;
    unixtime -= timezone;
    datep -> ds_Days = unixtime / SECONDS_PER_DAY;
    unixtime -= datep -> ds_Days * SECONDS_PER_DAY;
    datep -> ds_Minute = unixtime / SECONDS_PER_MINUTE;
    unixtime -= datep -> ds_Minute * SECONDS_PER_MINUTE;
    datep -> ds_Tick = unixtime * TICKS_PER_SECOND;
    DBUG_PRINT ("unix2dos", ("datep -> ds_Days = %ld", datep -> ds_Days));
    DBUG_PRINT ("unix2dos", ("datep -> ds_Minute = %ld", datep -> ds_Minute));
    DBUG_PRINT ("unix2dos", ("datep -> ds_Tick = %ld", datep -> ds_Tick));
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	unix2amiga    convert from Unix time to internal Amiga format
 *
 *  SYNOPSIS
 *
 *	long unix2amiga (utime)
 *	long utime;
 *
 *  DESCRIPTION
 *
 *	Convert from Unix time to the internal form of time expected
 *	by amiga library calls.
 */

long unix2amiga (utime)
long utime;
{
	return (utime - TIME_SKEW);
}


/*
 *  FUNCTION
 *
 *	amiga2unix    convert from internal Amiga format to Unix time
 *
 *  SYNOPSIS
 *
 *	long amiga2unix (mtime)
 *	long mtime;
 *
 *  DESCRIPTION
 *
 *	Convert the internal form of time expected by amiga library calls
 *	to the Unix time format.
 */

long amiga2unix (mtime)
long mtime;
{
	return (mtime + TIME_SKEW);
}
