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
 *	date.c    routines to do date conversion
 *
 *  SCCS
 *
 *	@(#)date.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Routines to convert an ascii date string to an internal
 *	time.  The date may be given in one of the forms:
 *
 *		DD-MMM-YY[,HH:MM:SS]
 *		MM/DD/YY[,HH:MM:SS]
 *		MMDDHHMM[YY]
 *
 */
 
#include "globals.h"

#define FEBRUARY	1	/* February is a special month */
#define LEAPSIZE	29	/* February has 29 days in leap years */
#define HOURS_PER_DAY	24	/* Maybe this will be bigger some day! */
#define MIN_PER_HOUR	60	/* Minutes per hour */
#define SEC_PER_MIN	60	/* Seconds per minute */


#define DAYS_IN_YEAR(year) (((year) % 4) ? 365 : 366)

static char *mtable[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int month_size[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static BOOLEAN time_sane PROTO((struct tm *tp));
static time_t conv_time PROTO((struct tm *tp));
static int conv_month PROTO((char *mstr));
static BOOLEAN match PROTO((char *cp1, char *cp2));
static VOID tm_init PROTO((struct tm *tp));


/*
 *  FUNCTION
 *
 *	date    convert ascii date string to internal time format
 *
 *  SYNOPSIS
 *
 *	time_t date (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to an ascii date string (cp), converts the
 *	string to an internal time and returns that value.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin date
 *	    Default time is 1-Jan-70,00:00:00
 *	    Initialize the time structure
 *	    If the string contains a '-' character then
 *		Clear the month string
 *		Convert string of first form
 *		Convert the month
 *	    Else if string contains a "/" character then
 *		Convert string of second form
 *		Adjust month
 *	    Else
 *		Convert a string of the third form
 *		Adjust month
 *	    End if
 *	    If the time does not pass sanity checks then
 *		Tell user about conversion error
 *	    Else
 *		Convert the time
 *	    End if
 *	    Return time
 *	End date
 *
 */

time_t date (cp)
char *cp;
{
    time_t mytime;		/* Time as converted */
    char mstr[16];		/* Month in string form */
    struct tm t;		/* Components of time */

    DBUG_ENTER ("date");
    DBUG_PRINT ("date", ("convert '%s'", cp));
    mytime = 0;
    tm_init (&t);
    if (s_strchr (cp, '-')) {
	mstr[0] = EOS;
 	(VOID) sscanf (cp, "%2d-%3s-%2d,%2d:%2d:%2d", &t.tm_mday, mstr,
		&t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec);
	t.tm_mon = conv_month (mstr);
    } else if (s_strchr (cp, '/')) {
	(VOID) sscanf (cp, "%2d/%2d/%2d,%2d:%2d:%2d", &t.tm_mon, &t.tm_mday,
		&t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec);
	t.tm_mon--;
    } else {
	(VOID) sscanf (cp, "%2d%2d%2d%2d%2d", &t.tm_mon, &t.tm_mday,
		&t.tm_hour, &t.tm_min, &t.tm_year);
	t.tm_mon--;
    }
    if (!time_sane (&t)) {
	bru_message (MSG_NTIME, cp);
    } else {
	mytime = conv_time (&t);
    }
    DBUG_RETURN (mytime);
}


/*
 *  FUNCTION
 *
 *	time_sane    perform sanity check on time components
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN time_sane (tp)
 *	struct tm *tp;
 *
 *  DESCRIPTION
 *
 *	Performs some boundry condition checks on the time to catch
 *	obvious errors.
 *
 *	Returns TRUE if things look ok, FALSE otherwise.
 *	
 *	Note that leap days on the wrong year will NOT be caught.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin time_sane
 *	    Test seconds for sanity
 *	    Test minutes for sanity
 *	    Test hours for sanity
 *	    Test month for sanity
 *	    Test day of month lower bound
 *	    If sane so far then
 *		If month is FEBRUARY then
 *		    Test upper bound day of february
 *		Else
 *		    Test upper bound day of other month
 *		End if
 *	    End if
 *	    Test year for sanity
 *	    Return results of test
 *	End time_sane
 *
 */


static BOOLEAN time_sane (tp)
struct tm *tp;
{
    BOOLEAN sanity;

    DBUG_ENTER ("time_sane");
    DBUG_PRINT ("date", ("seconds = %d", tp -> tm_sec));
    DBUG_PRINT ("date", ("minutes = %d", tp -> tm_min));
    DBUG_PRINT ("date", ("hours = %d", tp -> tm_hour));
    DBUG_PRINT ("date", ("mon = %d", tp -> tm_mon));
    DBUG_PRINT ("date", ("monthday = %d", tp -> tm_mday));
    DBUG_PRINT ("date", ("year = %d", tp -> tm_year));
    sanity = (0 <= tp -> tm_sec && tp -> tm_sec < 60);
    sanity &= (0 <= tp -> tm_min && tp -> tm_min < 60);
    sanity &= (0 <= tp -> tm_hour && tp -> tm_hour < 24);
    sanity &= (0 <= tp -> tm_mon && tp -> tm_mon < 12);
    sanity &= (0 < tp -> tm_mday);
    if (sanity) {
	if (tp -> tm_mon == FEBRUARY) {
	    sanity &= (tp -> tm_mday <= LEAPSIZE);
	} else {
	    sanity &= (tp -> tm_mday <= month_size[tp -> tm_mon]);
	}
    }
    sanity &= (0 < tp -> tm_year && tp -> tm_year < 100);
    DBUG_RETURN (sanity);
}


/*
 *  FUNCTION
 *
 *	conv_time    convert time to internal format
 *
 *  SYNOPSIS
 *
 *	static time_t conv_time (tp)
 *	struct time *tp;
 *
 *  DESCRIPTION
 *
 *	Converts time buffer pointed to by "tp" to the internal
 *	format (seconds since 00:00:00 GMT, January 1, 1970).
 *
 *	Note that this is the converse of the standard library
 *	routine "localtime" in ctime(3C).   Why is it not
 *	in the library!!!!.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin conv_time
 *	    Initialize time
 *	    Get given year
 *	    For each year since 1970 up to given year
 *		Add days to time
 *	    End for
 *	    If given year is leap year and past february then
 *		Add another day for leap year
 *	    End if
 *	    For each month up to previous month
 *		Add days for given year
 *	    End for
 *	    Add days for given month
 *	    Convert total days to hours
 *	    Add given hours
 *	    Convert total hours to minutes
 *	    Add given minutes
 *	    Convert total minutes to seconds
 *	    Add given seconds
 *	    Add time zone offset
 *	    If daylight savings time in effect then
 *		Set back by one hour
 *	    End if
 *	    Return time
 *	End conv_time
 *
 */

static time_t conv_time (tp)
struct tm *tp;
{
    time_t xtime;
    int year;
    int month;
    int y;

    DBUG_ENTER ("conv_time");
    xtime = 0;	
    year = 1900 + tp -> tm_year;
    for (y = 1970; y < year; y++) {
	xtime += (time_t) DAYS_IN_YEAR (y);
    }
    if ((DAYS_IN_YEAR(year) == 366) && (tp -> tm_mon > 1)) {
	xtime++;
    }
    for (month = 0; month < tp -> tm_mon; month++) {
	xtime += (time_t) month_size[month];
    }
    xtime += (time_t) (tp -> tm_mday - 1);
    xtime *= (time_t) HOURS_PER_DAY;
    xtime += (time_t) tp -> tm_hour;
    xtime *= (time_t) MIN_PER_HOUR;
    xtime += (time_t) tp -> tm_min;
    xtime *= (time_t) SEC_PER_MIN;
    xtime += (time_t) tp -> tm_sec;
    xtime += (time_t) s_timezone ();
    if ((localtime (&xtime)) -> tm_isdst) {
	xtime -= (time_t) (MIN_PER_HOUR * SEC_PER_MIN);
    }
    DBUG_RETURN (xtime);
}


/*
 *  FUNCTION
 *
 *	conv_month    convert string month to integer month
 *
 *  SYNOPSIS
 *
 *	static int conv_month (mstr)
 *	char *mstr;
 *
 *  DESCRIPTION
 *
 *	Converts string pointed to by "mstr" to an integer month
 *	by comparing with internal table.  Note that comparison
 *	is case independent but there must be exactly 3 characters.
 *	This could be fixed with a small amount of effort.
 *
 *	Note that the month is zero-based, to match the form in
 *	the "tm" structure.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin conv_month
 *	    Set default to illegal month
 *	    For each month in the month table
 *		If the name matches month string then
 *		    Remember numeric month
 *		    Break month scan loop
 *		End if
 *	    End for
 *	    Return month
 *	End conv_month
 *
 */

static int conv_month (mstr)
char *mstr;
{
    int i;
    int month;

    DBUG_ENTER ("conv_month");
    month = -1;
    for (i = 0; i < 12; i++) {
	if (match (mtable[i], mstr)) {
	    month = i;
	    break;
	}
    }
    DBUG_RETURN (month);
}


/*
 *  FUNCTION
 *
 *	match    case independent match test
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN match (cp1, cp2)
 *	char *cp1;
 *	char *cp2;
 *
 *  DESCRIPTION
 *
 *	Perform case independent match test on two strings.  Returns
 *	TRUE if match found, FALSE otherwise.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin match
 *	    Scan for end of either string or mismatch
 *	    Result is TRUE only if end of both strings
 *	    Return result
 *	End match
 *
 */

static BOOLEAN match (cp1, cp2)
char *cp1;
char *cp2;
{
    BOOLEAN result;

    DBUG_ENTER ("match");
    if (cp1 == NULL || cp2 == NULL) {
	bru_message (MSG_BUG, "match");
	result = FALSE;
    } else {
	result = TRUE;
	while (*cp1 != EOS && *cp2 != EOS && result) {
	    result = (s_tolower (*cp1++) == s_tolower (*cp2++));
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	tm_init    initialize a tm structure to all zeros
 *
 *  SYNOPSIS
 *
 *	static VOID tm_init (tp)
 *	struct tm *tp;
 *
 *  DESCRIPTION
 *
 *	Used to initialize a tm structure (possibly allocated on stack)
 *	to all zeros.
 *
 */

static VOID tm_init (tp)
struct tm *tp;
{
    DBUG_ENTER ("tm_init");
    tp -> tm_sec = 0;
    tp -> tm_min = 0;
    tp -> tm_hour = 0;
    tp -> tm_mday = 0;
    tp -> tm_mon = 0;
    tp -> tm_year = 0;
    DBUG_VOID_RETURN;
}
