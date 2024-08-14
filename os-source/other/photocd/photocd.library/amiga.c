/*** amiga.c *****************************************************************
 *
 *  $Id: amiga.c,v 1.2 93/11/18 19:59:37 jjszucs Exp $
 *  photocd.library
 *  Commonly-used Amiga Functions Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:	amiga.c,v $
 * Revision 1.2  93/11/18  19:59:37  jjszucs
 * Added RCS id and log substitions
 * 
*/

/*
 *  Amiga includes
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/tasks.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/****************************************************************************
 *                                                                          *
 *  SetTagItem()    -   set tag item                                        *
 *                                                                          *
 ****************************************************************************/

/* N.B.:

  This function is not implemented as a macro to allow uses such as:
    SetTagItem(thisTag++,PCDDisc_Ver,specVersion);
*/

void SetTagItem(struct TagItem *tagItem,Tag tag,ULONG data)
{

    tagItem->ti_Tag=tag;
    tagItem->ti_Data=data;

}

/****************************************************************************
 *                                                                          *
 *  LengthOfMonth() -   look up length of month                             *
 *                                                                          *
 ****************************************************************************/

int LengthOfMonth(int month)
{

    /* Table of number of days per month;
       a useful mnemonic is
        "30 days has September, April, June, and November" */
    static int daysPerMonth[MONTHS_PER_YEAR] ={
        31, /* January */
        28, /* February (non-leap-year) */
        31, /* March */
        30, /* April */
        31, /* May */
        30, /* June */
        31, /* July */
        31, /* August */
        30, /* September */
        31, /* October */
        30, /* November */
        31  /* December */
    };

    /* No bounds-checking */
    return daysPerMonth[month];

}

/****************************************************************************
 *                                                                          *
 *  DateComponentToAmiga()   -   convert date components to Amiga format    *
 *                                                                          *
 *  Does not extensively bounds-check arguments.                            *
 *                                                                          *
 ****************************************************************************/

BOOL DateComponentToAmiga(int month,int day,int year,
    int hour,int minute,int second,
    struct DateStamp *pDateStamp)
{

    int loop;

    /* Bounds-check year to 1978-2045 */
    if (year<1978 || year>2045) {
        return FALSE;
    }

    /* Bounds-check month to 1-12 */
    if (month<1 || month>12) {
        return FALSE;
    }

    /* Bounds-check day to 1-31;
       do not check for varying month lengths */
    if (day<1 || day>31) {
        return FALSE;
    }

    /* N.B.: The century rule is not included in date calculation because
             2000 C.E. is a leap year, due to the millenium rule.
             On the other hand, 2100 C.E. is not a leap year due
             to the century rule but this is irrelevant because:
             A) AmigaDOS date stamps can only represent dates up to 2045 C.E.
             B) The artifically-intelligent self-aware Amigas that will be
                available 106 years in the future will realize this and make
                the necessary adjustments :-).
    */

    /* Compute first day of year */
    pDateStamp->ds_Days=
        (year-AMIGA_START_YEAR)*DAYS_PER_YEAR+
        ((year-AMIGA_FIRST_LEAP_YEAR)/LEAP_YEAR_INTERVAL)*(DAYS_PER_LEAP_YEAR-DAYS_PER_YEAR);

    /* Loop through months */
    for (loop=1;loop<month;loop++) {
        pDateStamp->ds_Days+=LengthOfMonth(loop)+
            ((year%LEAP_YEAR_INTERVAL && month==LEAP_MONTH)?1:0);
    }

    /* Add days into month */
    pDateStamp->ds_Days+=day-1;

    /* Compute minute */
    pDateStamp->ds_Minute=hour*MINUTES_PER_HOUR+minute;

    /* Compute tick */
    pDateStamp->ds_Tick=second*TICKS_PER_SECOND;

    /* Return */
    return TRUE;

}
