/* misc types */
#include <exec/types.h>

/* struct DateStamp */
#include "..\dslib\dslib.h"

/* ourselves */
#include "dates.h"

unsigned char normal_months[] =
	 { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

unsigned char leap_months[] =
	 { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* convert normal date to ms-dos date */
void date_normal_to_msdos(int year, int month, int day, unsigned *msdos)
{
	if (year < 1980)
		*msdos = (month * 32) + day;
	else
		*msdos = ((year - 1980) * 512) + (month * 32) + day;
}

/* convert normal time to ms-dos time */
void time_normal_to_msdos(int hour, int minute, int second, unsigned *msdos)
{
	*msdos = (hour * 2048) + (minute * 32) + (second / 2);
}

/* convert normal date to amiga date */
void date_normal_to_amiga(int year, int month, int day, struct DateStamp *ds)
{
unsigned char *month_ptr;

	ds->ds_Days = ((year - 1978) * 365);		/* normal years */

	ds->ds_Days += ((year - 1978 + 2 - 1) / 4);		/* plus leap years */

	if ((year - 1978 + 2) % 4)					/* is this year a leap? */
		month_ptr = normal_months;
	else
		month_ptr = leap_months;

	month--;									/* plus months this year */
	while (month--)
		ds->ds_Days += month_ptr[month];

	ds->ds_Days += day - 1;						/* plus today */
}

/* convert normal time to amiga time */
void time_normal_to_amiga(int hour, int minute, int second, struct DateStamp *ds)
{
	ds->ds_Minute = (hour * 60) + minute;
	ds->ds_Tick = (second * TICKS_PER_SECOND);
}

/* convert ms-dos date to normal date */
void date_msdos_to_normal(unsigned msdos, int *year, int *month, int *day)
{
	*year = (msdos / 512) + 1980;
	*month = ((msdos % 512) / 32);
	*day = (msdos % 32);
}

/* convert ms-dos time to normal time */
void time_msdos_to_normal(unsigned msdos, int *hour, int *minute, int *second)
{
	*hour = (msdos / 2048);
	*minute = ((msdos % 2048) / 32);
	*second = (msdos % 32) * 2;
}

/* convert amiga date to normal date */
void date_amiga_to_normal(struct DateStamp *ds, int *year, int *month, int *day)
{
ULONG amiga_days, days_in_year;
unsigned char leap_count;
unsigned char *month_ptr;

	amiga_days = ds->ds_Days + 1;

/* get year */
	leap_count = 2;
	*year = 1978;

	while (1) {
		if (leap_count & 0x03)
			days_in_year = 365;
		else
			days_in_year = 366;

		if (amiga_days <= days_in_year)
			break;

		amiga_days -= days_in_year;
		(*year)++;
		leap_count++;
	}

/* get month */
	if (leap_count & 0x03)
		month_ptr = &normal_months[0];
	else
		month_ptr = &leap_months[0];

	*month = 1;

	while (1) {
		if (amiga_days < *month_ptr)
			break;
		amiga_days -= *month_ptr++;
		(*month)++;
	}

/* get day */
	*day = amiga_days;
}

/* convert amiga time to normal time */
void time_amiga_to_normal(struct DateStamp *ds, int *hour, int *minute, int *second)
{
	*hour = ds->ds_Minute / 60;
	*minute = ds->ds_Minute % 60;
	*second = ds->ds_Tick / TICKS_PER_SECOND;
}

