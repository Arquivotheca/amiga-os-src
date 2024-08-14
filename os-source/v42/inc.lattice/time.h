/**
*
* This structure contains the unpacked time as returned by "gmtime".
*
*/
struct tm 
	{
	int tm_sec, tm_min, tm_hour,
           tm_mday, tm_mon, tm_year,
           tm_wday, tm_yday, tm_isdst;
	};

/**
*
* External functions
*
*/
#ifndef NARGS
void utunpk(long, char *);
long utpack(char *);
extern void tzset(void);
int timer(unsigned int *);
extern char *asctime(struct tm *);
extern char *ctime(long *);
extern struct tm *gmtime(long *);
extern struct tm *localtime(long *);
extern long time(long *);
#else
void utunpk();
long utpack();
extern void tzset();
extern char *asctime();
int timer();
extern char *ctime();
extern struct tm *gmtime();
extern struct tm *localtime();
extern long time();
#endif
typedef long time_t;

/*
*
* External variables
*
*/
extern int daylight;
extern long timezone;
extern char *tzname[2];
extern char tzstn[4];
extern char tzdtn[4];

#define TZ  "CST6"  /* would be an envrironment var if AmigaDOS had such things */
#define YEAR0  1978
#define DAY0  0  /* Jan 1, 1978 is a Sunday */
