/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

#include "standard.h"

#include <workbench/startup.h>

#include "dbug.h"
#include "libfuncs.h"

static BOOL is_leap ( int year );

extern struct WBStartup *WBenchMsg;

/*..............................................................*/
/*								*/
/*		Wait Pointer					*/
/*..............................................................*/

#define WAIT_POINTER_HEIGHT 16L

static USHORT chip wait_pointer[(WAIT_POINTER_HEIGHT * 2) + 4] = {
    0x0000, 0x0000,	/* vert. and horiz. start posn. */
	0x0400,	0x07C0,
	0x0000,	0x07C0,
	0x0100,	0x0380,
	0x0000,	0x07E0,
	0x07C0,	0x1FF8,
	0x1FF0,	0x3FEC,
	0x3FF8,	0x7FDE,
	0x3FF8,	0x7FBE,
	0x7FFC,	0xFF7F,
	0x7EFC,	0xFFFF,
	0x7FFC,	0xFFFF,
	0x3FF8,	0x7FFE,
	0x3FF8,	0x7FFE,
	0x1FF0,	0x3FFC,
	0x07C0,	0x1FF8,
	0x0000,	0x07E0,
    0x0000, 0x0000,	/* reserved, must be NULL */
};


#if 0	/* Old version */
#define WAIT_POINTER_HEIGHT 22L

static USHORT chip wait_pointer[(WAIT_POINTER_HEIGHT * 2) + 4] =
{
            0x0000, 0x0000,
            0x6700, 0xC000,
            0xCFA0, 0xC700,
            0xBFF0, 0x0FA0,
            0x70F8, 0x3FF0,
            0x7DFC, 0x3FF8,
            0xFBFC, 0x7FF8,
            0x70FC, 0x3FF8,
            0x7FFE, 0x3FFC,
            0x7F0E, 0x3FFC,
            0x3FDF, 0x1FFE,
            0x7FBE, 0x3FFC,
            0x3F0E, 0x1FFC,
            0x1FFC, 0x07F8,
            0x07F8, 0x01E0,
            0x01E0, 0x0080,
            0x07C0, 0x0340,
            0x0FE0, 0x07C0,
            0x0740, 0x0200,
            0x0000, 0x0000,
            0x0070, 0x0020,
            0x0078, 0x0038,
            0x0038, 0x0010,
            0x0000, 0x0000,
};
#endif


#define RANGE_POINTER_HEIGHT 16

static USHORT chip range_pointer[(RANGE_POINTER_HEIGHT * 2) + 4] =
{
	0x0000, 0x0000,

	0x0000, 0xFC00,
	0x7C00, 0xFE00,
	0x7C00, 0x8600,
	0x7800, 0x8C00,
	0x7C00, 0x8600,
	0x6E00, 0x9300,
	0x0700, 0x6980,
	0x0380, 0x04C0,
	0xFDC0, 0xFE60,
	0x0C80, 0xF540,
	0x6C00, 0x9480,
	0x6C00, 0x9400,
	0x0C00, 0xF400,
	0x5C00, 0xA400,
	0x6C00, 0x9400,
	0x7400, 0x8C00,

	0x0000, 0x0000
};


static ULONG col2;

static BOOL rangepointer = FALSE;


void SetWaitPointer ( struct Window *window )
{
    DBUG_ENTER ("SetWaitPointer");
    SetPointer (window, &wait_pointer[0], WAIT_POINTER_HEIGHT, 16L, 0L, 0L);

#if 0
    /* outline color */
    col2 = GetRGB4 ((ViewPortAddress (window)) -> ColorMap, 17L);
    SetRGB4 (ViewPortAddress (window), 17L, 0x0L, 0x0L, 0x0L);
#endif

    DBUG_VOID_RETURN;
}

void ClearWaitPointer ( struct Window *window )
{
    DBUG_ENTER ("ClearWaitPointer");

    ClearPointer (window);
#if 0
    SetRGB4 (ViewPortAddress (window), 17L,
	    (col2 & 0xF00) >> 8L, (col2 & 0x0F0L) >> 4L, col2 & 0x00FL);
#endif
	if( rangepointer == TRUE )
	{
		SetRangePointer( window );
	}

    DBUG_VOID_RETURN;
}


void SetRangePointer ( struct Window *window )
{
    DBUG_ENTER ("SetRangePointer");
    SetPointer (window, &range_pointer[0], RANGE_POINTER_HEIGHT, 16L,
		-2L, -1L);

	rangepointer = TRUE;

#if 0
    /* outline color */
    col2 = GetRGB4 ((ViewPortAddress (window)) -> ColorMap, 17L);
    SetRGB4 (ViewPortAddress (window), 17L, 0x0L, 0x0L, 0x0L);
#endif
    DBUG_VOID_RETURN;
}


void ClearRangePointer ( struct Window *window )
{
    DBUG_ENTER ("ClearRangePointer");
    ClearPointer (window);

	rangepointer = FALSE;
#if 0
    SetRGB4 (ViewPortAddress (window), 17L,
	    (col2 & 0xF00) >> 8L, (col2 & 0x0F0L) >> 4L, col2 & 0x00FL);
#endif
    DBUG_VOID_RETURN;
}



/*..............................................................*/
/*								*/
/*	Date and Time Conversions				*/
/*..............................................................*/


#define BASE_YEAR 1978

#define DAYS_PER_BUNCH ((365*4)+1)

static int dmtable[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int leap_dmtable[12] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int daytable[12] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

static int leap_daytable[12] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335
};

static char text_month[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


static BOOL is_leap ( int year )
{
    DBUG_ENTER ("is_leap");
    if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
	DBUG_RETURN (TRUE);
    } else {
	DBUG_RETURN (FALSE);
    }
}

LONG amigados_to_secs ( LONG days, LONG secs, LONG ticks )
{
    DBUG_ENTER ("amigados_to_secs");
    DBUG_RETURN ((days * 86400) + secs);
}

void secs_to_amigados ( LONG fullsecs, LONG *days, LONG *secs,
	LONG *ticks )
{
    DBUG_ENTER ("secs_to_amigados");
    *days = fullsecs / 86400;
    *secs = fullsecs % 86400;
    *ticks = 0;
    DBUG_VOID_RETURN;
}



/*................................*/
/*	Convert numeric to string */

/*
 * Convert days since baseyear to a date string in the format:
 *      DD-MMM-YY   e.x.:	"12-JAN-88"
 *							" 1-Mar-90"
 */

void days_to_string ( LONG days, char *string )
{
    int day;
    int month;
    int year;

    DBUG_ENTER ("days_to_string");
    days_to_date (days, &day, &month, &year);
    sprintf (string, "%2d-%s-%02d", day, text_month[month - 1],
		year % 100);
    DBUG_VOID_RETURN;
}


/*
 * Convert days since baseyear to a date string in the format:
 *      DD-MMM-YY   e.x.:	"12JAN88"
 *							" 1Mar90"
 */

void days_to_string2 ( LONG days, char *string )
{
    int day;
    int month;
    int year;

    DBUG_ENTER ("days_to_string2");
    days_to_date (days, &day, &month, &year);
    sprintf (string, "%02d%s%02d", day, text_month[month - 1], year % 100);
    DBUG_VOID_RETURN;
}


/*
 * This will convert days since baseyear to day, month, year
 */

void days_to_date ( LONG days, int *day_ptr, int *month_ptr,
	int *year_ptr )
{
    int year;		/* BASE_YEAR thru ? */
    int month;		/* 1 thru 12 */
    int day;		/* 1 thru 31 */
    LONG i;

    DBUG_ENTER ("days_to_date");
    /* Find year */
    i = days + 1;		/* not sure why +1 needed here... */
    year = BASE_YEAR;
    while (i > 0) {
	i -= 365;
	if (is_leap (year)) {
	    i--;
	}
	year++;
    }
    year--;
    i += is_leap (year) ? 366 : 365;
    /* We now have the year */
    /* Find month */
    month = 0;
    while (i > 0) {
	if (is_leap (year)) {
	    i -= leap_dmtable[month];
	} else {
	    i -= dmtable[month];
	}
	month++;
    }
    i += is_leap (year) ? leap_dmtable[month - 1] : dmtable[month - 1];
    /* We now have month */
    day = i;
    /* Store the results */
    *year_ptr = year;
    *month_ptr = month;
    *day_ptr = day;
    DBUG_VOID_RETURN;
}

/* Conversion from string to numeric */

/* day is 1 thru 31
 * month is 1 thru 12
 * year is BASE_YEAR thru unknown
 *
 * result is days since Jan 1, BASE_YEAR
 */

LONG days_since ( int day, int month, int year )
{
    ULONG days_since = 0;
    int i;

    DBUG_ENTER ("days_since");
    for (i = BASE_YEAR; i < year; i++) {
	days_since += 365;
	if (is_leap (i)) {
	    days_since++;
	}
    }
    if (is_leap (year)) {
	days_since += leap_daytable[month - 1];
    } else {
	days_since += daytable[month - 1];
    }
    days_since += day - 1;
    /* days is now since baseyear */
    DBUG_RETURN ((LONG) days_since);
}



/*
 * This takes a string in the format "DD-MMM-YY" and converts
 * it to seconds since base year.
 */

LONG ascwhen_to_secs ( char *string )
{
    int day;
    int month=-1;
    int year=-1;
    long days;
    long secs;
	int i;

    DBUG_ENTER ("ascwhen_to_secs");

/*	printf( "convert: %s\n", string ); */

	day = atoi( string );

	/* Index past the day and seperator character */
	while(  isdigit(*string) || (*string=='-') || (*string=='/')
		|| (ispunct(*string)) || (isspace(*string))  )
	{
		string++;
	}

	/* Convert the month */
	for( i=0; i<12; i++ )
	{
		if(  strnicmp( string, &text_month[i][0], 3 ) == 0  )
		{
			month = i+1;
			break;
		}
	}

	/* Index to the year */
	while(  isalpha(*string) || (*string=='-') || (*string=='/')
		|| (ispunct(*string)) || (isspace(*string))  )
	{
		string++;
	}

	if( isdigit(*string) )
	{
		year = atoi( string );
	}

/*	printf( "day %d  month %d  year %d\n", day, month, year ); */

    if (year<0) {
	DBUG_RETURN (0);
    }

    if (year < 78) {
	year += 2000;
    } else {
	if ((year < BASE_YEAR)) {
	    year += 1900;
	}
    }
    if ((month < 1) || (month>12)) {
	DBUG_RETURN (0);
    }
    if ((day < 1) || (day > 31)) {
	DBUG_RETURN (0);
    }

    days = days_since (day, month, year);
    secs = days * 86400;
    DBUG_RETURN (secs);
}


#if 0	/* Old (and broken) version.  DTM 21-Feb-90 */
LONG ascwhen_to_secs (string)
char *string;
{
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
    long days;
    long secs;

    DBUG_ENTER ("ascwhen_to_secs");
/*	printf( "convert: %s\n", string ); */

    sscanf (string, "%d-%d-%d %d:%d:%d",
            &day, &month, &year, &hour, &minute, &second);
/*	printf( "day %d  month %d  year %d\n", day, month, year ); */

    if (year < 78) {
	year += 2000;
    } else {
	if ((year < BASE_YEAR)) {
	    year += 1900;
	}
    }
    if (month == 0) {
	DBUG_RETURN (0);
    }
    if ((day < 1) || (day > 31)) {
	DBUG_RETURN (0);
    }
    if (hour > 23) {
	DBUG_RETURN (0);
    }
    if (minute > 59) {
	DBUG_RETURN (0);
    }
    if (second > 59) {
	DBUG_RETURN (0);
    }
    days = days_since (day, month, year);
    secs = (days * 86400) + (hour * 3600) + (minute * 60) + second;
    DBUG_RETURN (secs);
}
#endif


LONG ascdate_to_days ( char *string )
{
    int i;
    int day;
    int month;
    int year;
    char buf[10];
    LONG rtnval;

    DBUG_ENTER ("ascdate_to_days");
    if (strlen (string) == 8) {
	buf[0] = '0';
	strncpy (&buf[1], string, 8);
    } else {
	strncpy (buf, string, 9);
    }
    /* Get day of month */
    day = atoi (buf);
    if ((day < 1) || (day > 31)) {
	DBUG_RETURN (0);
    }
    /* Get year */
    year = atoi (&buf[7]) + 1900;
    /* See if we have rolled to year 2000. * If so, add 100 to year */
    if (year < BASE_YEAR) {
	year += 100;
    }
    if ((year < BASE_YEAR)) {
	DBUG_RETURN (0);
    }
    /* Get month */
    buf[6] = '\0';
    for (i = 0; i < 12; i++) {
	if (strequal (&buf[3], text_month[i]) == TRUE) {
	    month = i + 1;
	    break;
	}
    }
    if (month == 0) {
	DBUG_RETURN (0);
    }
    rtnval = days_since (day, month, year);
    DBUG_RETURN (rtnval);
}



/*
 * This function takes a pattern which is a subset of the pattern
 * understood by the ANSI 'C' function strftime().
 * This function also assumes we want to format the current time.
 *
 * The return is the address of a static string buffer.
 */

char *when_pattern( char *pattern )
{
	struct DateStamp ds;
	int hours, minutes, seconds;
	int day, month, year;
	char c;
	static char buffer[64];
	char *buf;


	/* Get all the date stuff ready-to-hand */

	DateStamp(&ds);	/* This cast is needed to make this
			 * compile with the 5.1 Lattice headers. */

    days_to_date (ds.ds_Days, &day, &month, &year);


	hours = ds.ds_Minute / 60;
	minutes = ds.ds_Minute % 60;
	seconds = ds.ds_Tick / TICKS_PER_SECOND;


	buffer[0] = '\0';
	buf = buffer;

	while(  ( c = *pattern++ ) != '\0'  )
	{
		if( c != '%' )
		{
			/* Pass through unchanged */
			*buf++ = c;
			*buf = '\0';
			continue;
		}

		/* It is a special format specifier */
		c = *pattern++;
		switch( c )
		{
			case '\0':
				/* this is improper, but... */
				pattern--;
				break;

			case 'd':
				/* Day of month in 01-31 format */
				sprintf( buf, "%02d", day );
				buf = strchr( buf, '\0' );
				break;

			case 'b':
				/* Month in the format "Jan" */
				strcat( buf, text_month[month-1] );
				buf = strchr( buf, '\0' );
				break;

			case 'y':
				/* Year in the format "90" */
				sprintf( buf, "%02d", year % 100 );
				buf = strchr( buf, '\0' );
				break;

			case 'Y':
				/* Year in the format "1990" */
				sprintf( buf, "%04d", year );
				buf = strchr( buf, '\0' );
				break;

#if 0
			case 'a':
				/* Day of week in the format "Wed" */
				sprintf( buf, "", );
				buf = strchr( buf, '\0' );
				break;
#endif

			case 'm':
				/* Month in the format "01" thru "12" */
				sprintf( buf, "%02d", month );
				buf = strchr( buf, '\0' );
				break;

			case 'H':
				/* Hours in the format "22" */
				sprintf( buf, "%02d", hours );
				buf = strchr( buf, '\0' );
				break;

			case 'M':
				/* Minutes in the format "55" */
				sprintf( buf, "%02d", minutes );
				buf = strchr( buf, '\0' );
				break;

			case 'S':
				/* Seconds in the format "59" */
				sprintf( buf, "%02d", seconds );
				buf = strchr( buf, '\0' );
				break;

			case '%':
				*buf++ = c;
				*buf = '\0';
				break;

			default:
				/* Unknown format char.  Should be an error, but
				 * since this is a lenient routine, pass it through.
				 * Will be obvious somebody oopsed from the result...
				 */
				*buf++ = c;
				*buf = '\0';
				break;
		}
	}

	return( buffer );
}



/*..............................................................*/
/*								*/
/*		ToolType functions				*/
/*..............................................................*/

/*
 * This will do my version of the icon.library FindToolType() function.
 * I am writing my own code for this, since it appears that there may
 * be a bug in the library call, if the tooltype array you search is
 * not from an icon.
 *
 * This routine takes an array of pointers to strings, each of which
 * may have the form:
 *
 *	<name>=<value>
 *
 * and a pointer to a particular value of <name> to search for, and
 * scans the array, looking for a match.  If one is found, a pointer
 * to <value> is returned.  Otherwise, if a null string is found, or
 * a null pointer, a null pointer is returned.
 *
 */

char *MyFindToolType ( char **ttype, char *tname )
{
    register int length;
    register char *ep;

    DBUG_ENTER ("MyFindToolType");
    DBUG_PRINT ("tooltype", ("look for value for '%s'", tname));
    length = strlen (tname);
    while (*ttype != NULL && **ttype != '\000') {
	/* See if there is an equals sign in the string */
	if ((ep = strchr (*ttype, '=')) != NULL) {
	    /* There is an equal's sign */
	    /* compare length of lefthand part of string */
	    if (length == (ep - *ttype)) {
		/* Length of typename matches */
		if (strncmp (tname, *ttype, length) == 0) {
		    ep++;
		    DBUG_PRINT ("tooltype", ("found value '%s'", ep));
		    DBUG_RETURN (ep);
		}
	    }
	}
	ttype++;
    }
    DBUG_PRINT ("tooltype", ("no match found"));
    DBUG_RETURN ((char *) NULL);
}

BOOL MyMatchToolValue ( char *string, char *substring )
{
    char c;

    DBUG_ENTER ("MyMatchToolValue");
    while (string != NULL) {
	if (strncmp (string, substring, strlen (substring)) == 0) {
	    /* perhaps a match */
	    c = *(string + strlen (substring));
	    if ((c == '|') || (c == '\0') || (c == ' ')) {
		/* A true match */
		DBUG_RETURN (TRUE);
	    }
	}
	/* Not a match, find next entry (after a vertical bar |). */
	string = strchr (string, '|');
	if (string != NULL) {
	    string++;	/* point at the character after the '|'. */
	}
    }
    DBUG_RETURN (FALSE);
}

/*
 * This function will take a TextAttr structure pointer and attempt
 * to open the EXACT font specified therein.  If successful a pointer
 * to the TextFont structure will be returned.  Both RAM and disk fonts
 * are searched.  Failure will result in a 0 being returned.
 * Exact is defined as being the same Ysize and name.
 */

struct TextFont *GetFont ( struct TextAttr *ta )
{
    struct TextFont *tf;

    DBUG_ENTER ("TextFont");
    if (tf = (struct TextFont *) OpenFont (ta)) {
	/* Found it in RAM */
	if ((ta -> ta_YSize != tf -> tf_YSize)) {
	    /* darn, not an exact match so throw it back. */
	    CloseFont (tf);
	    tf = NULL;
	}
    }
    /* Not in RAM, look on disk. */
    if (!tf && (tf = (struct TextFont *) OpenDiskFont (ta))) {
	/* Found it on Disk. */
	if ((ta -> ta_YSize != tf -> tf_YSize)) {
	    /* But still not an exact match! */
	    CloseFont (tf);
	    tf = NULL;
	}
    }
    DBUG_RETURN (tf);
}

/*
 * Loads the .info file's DiskObject structure into memory if
 * possible and returns a pointer to it.
 * Name should be argv[0], or garbage if we were started from Workbench.
 *
 * Example calling sequence:
 *		struct DiskObject *mydiskobject = NULL;
 *		mydiskobject = ReadInfoFile (&mydiskobject, argv[0]);
 *
 * Note that it is the callers responsibility to free up the allocated
 * DiskObject structure.
 */

struct DiskObject *ReadInfoFile ( char *name )
{
    struct DiskObject *disko;
    BPTR oldlock;
    char *pname;

    DBUG_ENTER ("ReadInfoFile");
    if (WBenchMsg == NULL) {
	/* started from CLI */
	pname = name;
    } else {
	/* Launched by the WorkBench */
	pname = WBenchMsg -> sm_ArgList[0].wa_Name;
	oldlock = CurrentDir (WBenchMsg -> sm_ArgList[0].wa_Lock);
    }
    disko = GetDiskObject (pname);
    if (WBenchMsg) {
	(void) CurrentDir (oldlock);
    }
    DBUG_RETURN (disko);
}

void *RemAlloc ( ULONG size, ULONG flags )
{
    ULONG *p = NULL;
    ULONG asize = size + 4;

    DBUG_ENTER ("RemAlloc");
    if (size > 0) {
	p = AllocMem (asize, flags);
    }
    if (p != NULL) {
	*p++ = asize;
    }
    DBUG_RETURN ((void *) p);
}

void *RemFree ( void *p )
{
    long pp;

    DBUG_ENTER ("RemFree");
    if (p != NULL) {
	pp = (long) p;
	pp -= 4;
	FreeMem ((void *) (pp), *((long *) pp));
    }
    DBUG_RETURN ((void *) NULL);
}

/*
 * Copies a string from 'src' to 'dest' with a width (including the
 * terminating null) not to exceed 'width'.  Differs from strncpy
 * in that this will always make sure that the buffer ends in a null.
 */


void strlcpy ( char *dest, char *src, unsigned int width )
{
    strncpy (dest, src, width);
    dest[width - 1] = '\0';
}

/*
 * Adds the string pointed to by  dir  to the string pointed to by
 *  path  .  Puts the appropriate AmigaDOS seperator between them.
 */

void add_path ( char *path, char *dir )
{
    char c;

    DBUG_ENTER ("add_path");
    c = path[strlen (path) - 1];
    if ((c != '/') && (c != ':')) {
	strcat (path, "/");
    }
    strcat (path, dir);
    DBUG_VOID_RETURN;
}

/*
 * Deletes any of the character 'c' found at the begining of the string
 * pointed to by 'p'.  Does this in the same buffer by moving the string
 * down in the buffer if need be.
 */

void trim_leading ( char *p, char c )
{
    char *start = p;

    DBUG_ENTER ("trim_leading");
    while (*(p++) == c) {
	;
    }
    if (p != start) {
	strcpy (start, --p);
    }
    DBUG_VOID_RETURN;
}

/*
 * Subtracts a path name component from the string pointed at by  path  .
 * Does nothing if path is already a root volume (e.g. DH0:)
 */

void subtract_path ( char *path )
{
    char *p;

    DBUG_ENTER ("subtract_path");
    if ((strchr (path, ':') == NULL) && (strchr (path, '/') == NULL)) {
	/* Cannot truncate past the root? */
	DBUG_VOID_RETURN;
    }
    if (path[strlen (path) - 1] == ':') {
	/* We're at the root device already */
	DBUG_VOID_RETURN;
    }
    p = path + strlen (path) - 1;/* point at last char */
    while ((*p != '/') && (*p != ':')) {
	p--;
    }
    if (*p == ':') {
	p++;
	*p = '\0';
    } else {
	*p = '\0';
    }
    DBUG_VOID_RETURN;
}

/*
 * This does a case insensitive comparison of two strings.  A Boolean
 * TRUE is returned if they match, FALSE if they differ.
 */

BOOL strequal ( char *s1, char *s2 )
{
    SHORT c1;
    SHORT c2;

    DBUG_ENTER ("strequal");
    for (;;) {
	c1 = *s1++;
	if ((c1 >= 'a') && (c1 <= 'z')) {
	    c1 = c1 - 'a' + 'A';
	}
	c2 = *s2++;
	if ((c2 >= 'a') && (c2 <= 'z')) {
	    c2 = c2 - 'a' + 'A';
	}
	if (c1 != c2) {
	    DBUG_RETURN (FALSE);
	}
	if (c1 == '\0') {
	    DBUG_RETURN (TRUE);
	}
    }
}

/*
 * This will return a pointer to the rightmost n characters of
 * a string.  A null pointer is returned if the string is less than n
 * characters long.
 */

char *stringright ( char *string, unsigned int width )
{
    int x;

    DBUG_ENTER ("stringright");
    if ((x = strlen (string)) < width) {
	DBUG_RETURN ((char *) NULL);
    }
    DBUG_RETURN (string + x - width);
}

/*
 * This will take the string and append blanks onto it to bring
 * it to the length specified.  If the string is already as long
 * as the desired length (or longer) no action will be taken.
*/

void padstring ( char string[], int width )
{
    int len;
    int i;

    DBUG_ENTER ("padstring");
    len = strlen (string);
    if (len >= width) {
	/* This string is already long enough */
	DBUG_VOID_RETURN;
    }
    for (i = len; i < width; string[i++] = ' ') {
	;
    }
    string[width] = '\0';
    DBUG_VOID_RETURN;
}

/*
 * This is intended to be a very memory efficient parsing procedure.
 * The length of the options is limited only by the length of the
 * input line buffer (owned by the caller).  The number of arguments which
 * can be parsed out of an input string is limited by the size of the
 * pointer array, which is also owned by the caller.
 * Note that this WILL MODIFY the contents of the line passed in the
 * buffer.  If this is un-desirable, copy the string to a scratch buffer
 * and pass that.
 *
 * The calling should look something like:
 *		char *myargpointers[10];
 *		int myargcount;
 *		char buf[100];
 *
 *		gets (buf);
 *		myargcount = inplace_parse (buf, myargpointers, 10);
 */

int parse_inplace ( char *p, char **argptr, int maxargs )
{
    BOOL inparen = FALSE;
    BOOL inarg = FALSE;
    char *argstart = 0;		/* Does not really need to be initialized */
    int argcount = 0;

    DBUG_ENTER ("parse_inplace");
    while ((*p != '\0') && (argcount < maxargs)) {
	switch (*p) {
	    case ' ': 
	    case '\t': 
		/* Whitespace */
		if ((inarg == TRUE) && (inparen == FALSE)) {
		    /* We got one! */
		    *p = '\0';	/* place a delimiting null */
		    argptr[argcount++] = argstart;
		    inarg = FALSE;
		    inparen = FALSE;
		}
		break;
	    case '"': 
		if (inparen == TRUE) {
		    /* This is closing paren */
		    /* We got one! */
		    *p = '\0';		/* place a delimiting null */
		    argptr[argcount++] = argstart;
		    inarg = FALSE;
		    inparen = FALSE;
		} else {
		    /* Opening paren */
		    if (inarg == TRUE) {
			/* Deal with the previous arg first */
			*p = '\0';/* place a delimiting null */
			argptr[argcount++] = argstart;
		    }
		    inparen = TRUE;
		    inarg = TRUE;
		    argstart = p + 1;
		}
		break;
	    default: 
		/* A character we want in an argument string */
		if (inarg == FALSE) {
		    inarg = TRUE;
		    argstart = p;
		}
		break;
	}
	p++;
    }
    if ((inarg == TRUE) && (argcount < maxargs)) {
	argptr[argcount++] = argstart;
    }
    DBUG_RETURN (argcount);
}


#if 0	/* Use stricmp() instead... */
/*
 * This does a case insensitive comparison of two strings.
 *
 * Returns:
 *	 1 if s1 greater than s2
 *	 0 if s1 equal to s2
 *	-1 if s1 less than s2 
 */

int strucmp ( char *s1, char *s2 )
{
    char c1;
    char c2;

    DBUG_ENTER ("strucmp");
    for (;;) {
	c1 = *s1++;
	if ((c1 >= 'a') && (c1 <= 'z')) {
	    c1 = c1 - 'a' + 'A';
	}
	c2 = *s2++;
	if ((c2 >= 'a') && (c2 <= 'z')) {
	    c2 = c2 - 'a' + 'A';
	}
	if (c1 > c2) {
	    DBUG_RETURN (1);
	}
	if (c1 < c2) {
	    DBUG_RETURN (-1);
	}
	if (c1 == '\0') {
	    DBUG_RETURN (0);
	}
    }
}
#endif


/* Convert a numeric string in the format <n><unit> where <n> is a decimal
 * integer and <unit> is "k" or "m", into a long int.
 * Case is not significant.
 * A boolean TRUE will be returned if the value was formated correctly.
 */

BOOL kmstring_to_long( LONG *lp, char *string )
{
	int x;
	char *p;
	LONG num, mult;


	p = string;
	x = 0;

	while( *p != '\0' )
	{
		if( isdigit(*p) == 0 )
		{
			x++;
		}

		p++;
	}

	if( x > 1 )
	{
		/* Too many alpha characters */
		return( FALSE );
	}

	num = atol( string );

	/* Find first char not a space or a digit */
	p = string;
	while( *p != '\0' )
	{
		if(  ( ! isdigit(*p) ) && ( ! isspace(*p) )  )
		{
			break;
		}

		p++;
	}

	switch( *p )
	{
		case '\0':
			mult = 1;
			break;

		case 'k':
		case 'K':
			mult = 1024;
			break;

		case 'm':
		case 'M':
			mult = (1024 * 1024);
			break;

		default:
			return( FALSE );
	}

	*lp = num * mult;

	return( TRUE );
}



/*
 *	This function is derived from a Usenet posting by Bryce Nesbitt
 *	called stack_test.c, dated 4-Dec-88.  Thanks Bryce!
 *
 */

LONG stacksize ( void )
{
    register struct Process *Process;
    register struct CommandLineInterface *CLI;
    register long stack;

    DBUG_ENTER ("stacksize");
    Process = (struct Process *) FindTask (0L);
    if (CLI = (struct CommandLineInterface *) (Process -> pr_CLI << 2)) {
	DBUG_PRINT ("stack", ("this is a CLI process"));
	stack = CLI -> cli_DefaultStack << 2;
    } else {
	DBUG_PRINT ("stack", ("this is not a CLI process"));
	stack = Process -> pr_StackSize;
    }
    DBUG_PRINT ("stack", ("actual stack allocated is %ld bytes", stack));
    DBUG_RETURN (stack);
}



/* DTM_NEW */

/* This will take a filename and return a code indicating wether
 * it is a file or a directory.
 * 0 is returned if the given directory exists and is indeed a directory
 * and not a file.
 *
 * A positive 1 indicates a file, a -1 indicates the object doesn't
 * exist.
*/

int is_a_dir( char *fname )
{
	int rc = -1;					/* default to not found */
	BPTR lock;
	struct FileInfoBlock *fib;


	/* get a lock on the file so we can look at it */
	if(  lock = Lock( fname, ACCESS_READ )  )
	{
		/* create a longword aligned FileInfoBlock */
		if(  fib = (struct FileInfoBlock *)
			RemAlloc( (long)sizeof(struct FileInfoBlock),
			MEMF_PUBLIC )  )
		{
			/* fill our FileInfoBlock with info about our file */
			if( Examine( lock, fib ) != 0 )
			{
				/* finally, look at the data! */
				if(  ( fib->fib_DirEntryType ) > 0  )
				{
					rc = 0;
				}
				else
				{
					rc = 1;
				}
			}

			RemFree( fib );
		}

		UnLock( lock );
	}

	return( rc );
}


/*-----------------------------------------------------------------*/

/* This should really be in amiga.lib or something */

LONG EasyRequest( struct Window *win, struct EasyStruct *es, ULONG *idcmp,
	APTR arg1, ... )
{
	return(  EasyRequestArgs( win, es, idcmp, &arg1 )  );
}

/*-----------------------------------------------------------------*/

