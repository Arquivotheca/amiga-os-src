/* AMIGA specific rtns and stubs for some Unix rtns */

#ifdef AMIGA

/* ASF 5-Jan-94
   1. Added chmod to handle Unix extended header
   2. created i_chmod call to handle use of _attribute as protection bits 
   3. Corrected Amiga datestamp conversion routine use
*/

long __stack = 20000;

#if 0
/* NOTE! AMIGA CHANGES FOR OTHER FILES!!! */
/*---------------------------------------------------------------------*/

/* I prototyped the entire thing.  Sorry about that


Can ifndef LHEXTRACT out:

	lhadd.c:	everything after temporary_fp declaration
	maketree.c:	all
	append.c:	encode_lzhuf()
	huf.c:		everything in **encoding** section
	slide.c:	encode_define[], encode_alloc() up to decode()
/*---------------------------------------------------------------------*/
/* lharc.c - major changes to main() (compare to .orig) plus this
 * before exit(1) in fatal_error
 */
#ifdef AMIGA
#ifdef FASTFILE
  fcleanup();
#endif
#endif

/*---------------------------------------------------------------------*/
/* header.c: in get_header(): Add this before final return statement */
#ifdef AMIGA
  ados_convert(hdr);
#endif

And, in default case of get_header.c, call generic_to_ados_filename() instead of
generic_to_unix_filename.

/*---------------------------------------------------------------------*/
/* lhext.c: in extract_one(): Add to stack variables: */

#ifdef AMIGA
   int l;
#endif

/* And change 'sprintf (name, "%s/%s", extract_directory, q);' to: */
#ifdef AMIGA
	{
        l = strlen(extract_directory);
        if((extract_directory[l-1]==':')||(extract_directory[l-1]=='/'))
             sprintf (name, "%s%s", extract_directory, q);
        else sprintf (name, "%s/%s", extract_directory, q);
	}
#else
    sprintf (name, "%s/%s", extract_directory, q);
#endif

/* and in open_with_make_path() */
#ifdef AMIGA
	((name[strlen(name)-1] != '/')&&
	  (fp = fopen (name, WRITE_BINARY)) == NULL))
#else
	  (fp = fopen (name, WRITE_BINARY)) == NULL)
#endif


also ...  added a second chmod call, because the filesystem now
   defaults to rwd, which is terrible for an unarchiver

/*---------------------------------------------------------------------*/
/* maketbl.c: in make_table()  - the one that's used
 */
#ifdef AMIGA
	  fatal_error("Bad table (5)\n");
#else
	  error("\nBad table (5)\n");
#endif

/* shift data for make table. */

/*---------------------------------------------------------------------*/

/*
  In append.c I fixed a divide by 0 problem for the progress indicator
*/

#endif /* 0 */


#include <exec/types.h>
#include <dos/dos.h>

#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/datetime.h>

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <ctype.h>
#include <stat.h>
#include <string.h>

#include "lharc.h"

#ifdef LHEXTRACT
#include "lhex_rev.h"
#else
#include "lha_rev.h"
#endif

extern struct Library *DOSBase;
STATIC void generic_to_ados_stamp ( long generic, struct DateStamp * ados );
STATIC void unix_to_ados_stamp ( unsigned long unix, struct DateStamp * ados);

/**********    debug macros     ***********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

#define AMIGA_ALL_READ    (FIBF_OTR_READ    | FIBF_GRP_READ)
#define AMIGA_ALL_WRITE   (FIBF_OTR_WRITE   | FIBF_GRP_WRITE)
#define AMIGA_ALL_EXECUTE (FIBF_OTR_EXECUTE | FIBF_GRP_EXECUTE)
#define AMIGA_ALL_DELETE  (FIBF_OTR_DELETE  | FIBF_GRP_DELETE)

#define AMIGA_DEF_MODE    (AMIGA_ALL_READ|AMIGA_ALL_WRITE|AMIGA_ALL_EXECUTE|AMIGA_ALL_DELETE)

#define AMIGA_DIRECTORY         (1<<7)

char *vers = VERSTAG;

int getpid(void)
    {
    return(0);
    }

int getuid(void)
    {
    return(0);
    }

int umask(int numask)
    {
    static int omask = 0;
    int m;

    m = omask;
    omask = numask;
    return(m);
    }

int chown(char *name, int uid, int gid)
    {
    return(0);
    }


/* Assume that MODE is in the same format as the st_mode defined */
/* in stat.h. If the S_IWRITE is set, also set the delete bit */

/* This routine will handle the protection bits on archives made using
   lharcs run on Unix
 */

int chmod(const char *name, int mode) {
       unsigned long amiga_mode = FIBF_READ | FIBF_WRITE | FIBF_DELETE | FIBF_EXECUTE; 
       int err;

	// first four modes are 0 for yes, 1 for no
	if (mode & S_IWRITE)amiga_mode &= ~(FIBF_WRITE | FIBF_DELETE);
	if (mode & S_IREAD)amiga_mode &= ~FIBF_READ;
	if (mode & S_IEXECUTE)amiga_mode &= ~FIBF_EXECUTE;

	// rest of the modes are 0 for no, 1 for yes
	if (mode & S_ISCRIPT)amiga_mode |= FIBF_SCRIPT;
	err = SetProtection((char *)name,amiga_mode);
       return(0 == err);
}


/* This routine will handle protection bits on archives created with some
   of the Amiga lha style archivers.
 */

int i_chmod(const char *name, unsigned long amiga_mode) {
       	int err;
	err = SetProtection((char *)name,amiga_mode);
       	return(0 == err);
}

int link(char *name1, char *name2)
    {
    return(0);
    }

void mktemp(char *tempname)
    {
    /* should modify XXXXXX at end of name */
    }

struct utimbuf { time_t actime, modtime; };

/* Set the file time of the specified file (generic mode) */
utime(char *file, time_t times[]) {
    struct DateStamp ds;
    long ft;

    if(DOSBase->lib_Version < 36)return 0;	// call not in this OS version

    ft = times[1];

    unix_to_ados_stamp(ft,&ds);

    return(!SetFileDate(file,&ds));
}

/* Set the file time of the specified file (generic mode) */
//i_utime(char *file, struct utimbuf *times) {
i_utime(char *file, time_t times[]) {
    struct DateStamp ds;
    long ft;

    if(DOSBase->lib_Version < 36)return 0;	// call not in this OS version

//    ft = times->modtime;
    ft = times[1];
    generic_to_ados_stamp(ft,&ds);

    return(!SetFileDate(file,&ds));
}

void kill(int pid, int signo)
    {
    exit(10);
    }


/*----------------------------------------------------------------------*/
/*									*/
/*	Generic stamp format:						*/
/*									*/
/*	 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16		*/
/*	|<-------- year ------->|<- month ->|<-- day -->|		*/
/*									*/
/*	 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0		*/
/*	|<--- hour --->|<---- minute --->|<- second*2 ->|		*/
/*									*/
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*                   format conversion functions                        */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*           Convert generic filenames to AmigaDOS file names           */
/*              \ is replaced with /                                    */
/*----------------------------------------------------------------------*/
void
generic_to_ados_filename ( unsigned char *name, int len )
{
    int i;

    for (i = 0; i < len; i ++)
    {
        if (name[i] == '\\')
            name[i] = '/';
    }
}

#if 0
/*----------------------------------------------------------------------*/
/*           Convert generic filenames to AmigaDOS file names           */
/*              \ is replaced with /                                    */
/*              if the name is all uppercase, is is shifted to          */
/*              lowercase (mixed case names are left unchanged)         */
/*----------------------------------------------------------------------*/

STATIC void
generic_to_ados_filename ( unsigned char * name, int len )   /* OLD */
{
    int i;
    int mixed = 0;

    for (i = 0; i < len; i ++)
    {
        if (name[i] == '\\')
            name[i] = '/';
        else if (islower (name[i]))
            mixed = 1;
    }
    
    if (!mixed)
        for (i = 0; i < len; i ++)
        {
            if (isupper (name[i]))
                name[i] = tolower (name[i]);
        }
}

/*----------------------------------------------------------------------*/
/*          Convert Macintosh filenames to AmigaDOS file names          */
/*              Replace ':' with '/'                                    */
/*----------------------------------------------------------------------*/

STATIC void
macos_to_ados_filename ( unsigned char * name, int len )
{
    int i;

    for (i = 0; i < len; i ++)
    {
      if (name[i] == ':')
        name[i] = '/';
    }
}

/*----------------------------------------------------------------------*/
/*  Convert MS-DOS filenames to AmigaDOS file names (replace \ with / ) */
/*----------------------------------------------------------------------*/

STATIC void
msdos_to_ados_filename ( unsigned char * name, int len )
{
    int i;

    for (i = 0; i < len; i ++)
    {
      if (name[i] == '\\')
        name[i] = '/';
      else if (isupper (name[i]))
        name[i] = tolower (name[i]);
    }
}

#endif /* 0 */

/*----------------------------------------------------------------------*/
/*           Convert generic filenames to AmigaDOS file names           */
/*              If the first character of the name is /, it is deleted  */
/*----------------------------------------------------------------------*/
STATIC void
unix_to_ados_filename ( unsigned char * name, int len )
{
    if (name[0] == '/')
    {
	memmove(name, name + 1, len - 1);
	name[len-1] = 0;
    }
}

#if 1
/*----------------------------------------------------------------------*/
/*  Convert AmigaDOS filenames to generic file names                    */
/*----------------------------------------------------------------------*/

STATIC void
ados_to_generic_filename ( unsigned char * name, int len )
{
}

unsigned int dsboy[12] = { 0, 31, 59, 90, 120, 151,
                                      181, 212, 243, 273, 304, 334};
/*----------------------------------------------------------------------*/
/*         Convert generic timestamp to an AmigaDOS DateStamp           */
/*----------------------------------------------------------------------*/

STATIC void
generic_to_ados_stamp ( long generic, struct DateStamp * ados )
{
    int year, month, day, hour, min, sec;
    unsigned int days;

    /*
    **  special case:
    **          if date and time were zero, then we set time to be zero here too.
    */
    if (generic == 0)
    {
	ados->ds_Days   = 0;
	ados->ds_Minute = 0;
	ados->ds_Tick   = 0;
        return;
    }

    year  = ((int)(generic >> 25) & 0x7f) + 1980;
    month =  (int)(generic >> 21) & 0x0f;     /* 1..12 means Jan..Dec */
    day   =  (int)(generic >> 16) & 0x1f;     /* 1..31 means 1st,...31st */
    hour  =  ((int)generic >> 11) & 0x1f;
    min   =  ((int)generic >> 5)  & 0x3f;
    sec   =  ((int)generic        & 0x1f) * 2;

                                        /* Calculate days since 1978.01.01 */
    days = (365 * (year - 1978) +       /* days due to whole years */
           (year - 1978 + 1) / 4 +      /* days due to leap years */
           dsboy[month-1] +             /* days since beginning of this year */
           day-1);                      /* days since beginning of month */

    ados->ds_Days = days;
    ados->ds_Minute = hour * 60 + min;
    ados->ds_Tick = sec * 50;
}

#endif /* 0 */

/*----------------------------------------------------------------------*/
/*         Convert unix timestamp to an AmigaDOS DateStamp              */
/*----------------------------------------------------------------------*/

STATIC void
unix_to_ados_stamp ( unsigned long unix, struct DateStamp * ados )
{
    unsigned int        days;
    unsigned int        minutes;
    unsigned int        ticks;

    /*
    **  special case:
    **          if date and time were zero, then we set time to be zero here too.
    */
    if (unix == 0)
    {
	ados->ds_Days   = 0;
	ados->ds_Minute = 0;
	ados->ds_Tick   = 0;
        return;
    }

    /*
    **  This shifts the time from seconds since 1978 to seconds since 1978
    **
    **  365 days per year TIMES 8 years  PLUS 2 leap days (1972 & 1976)
    **  All TIMES 24 hours per day TIMES 3600 seconds per minute
    */
    unix = unix - (((365 * 8) + 2) * 24 * 3600);
        
    days = unix / (24*3600);
    minutes = (unix - (days * 24 * 3600)) / 60;
    ticks = (unix - (days * 24 * 3600) - (minutes * 60)) * 50;

    ados->ds_Days = days;
    ados->ds_Minute = minutes;
    ados->ds_Tick = ticks;
}

#if 0
unsigned long
ados_to_generic_stamp ( struct DateStamp ds )
{
    unsigned long generic = 0;
    int year, days, month, day, hour, min, sec, i;
           
    if (ds.ds_Days == 0 && ds.ds_Minute == 0 && ds.ds_Tick == 0)
    {
        return 0;
    }
    
    year = ds.ds_Days / 365 + 78;
    days = ds.ds_Days % 365 - (ds.ds_Days / 365 - 1) / 4;
    for (i = 0; i < 12; i++)
        if (dsboy[i] > days)
            break;
    month = i - 1;
    day = days - dsboy[i];
    hour = ds.ds_Minute / 60;
    min = ds.ds_Minute % 60;
    sec = ds.ds_Tick / 50;

    generic = year - 80 ;
    generic <<= 4;
    generic |= month & 0xF;
    generic <<= 5;
    generic |= day & 0x1F;
    generic <<= 5;
    generic |= hour & 0x1F;
    generic <<= 6;
    generic |= min & 0x3F;
    generic <<= 5;
    generic |= (sec / 2) & 0x1F;
    
    return generic;
}
unsigned long
ados_to_unix_stamp ( struct DateStamp ds )
{
    unsigned long       unix = 0L;
    
    unix = ds.ds_Days * 24 * 3600;
    unix += ds.ds_Minute * 60;
    unix += ds.ds_Tick / 50;
    
    unix += (((365 * 8) + 2) * 24 * 3600);
    
    return unix;
}
#endif /* 0 */


#if 0
unsigned long
unix_to_ados_mode ( int mode )
{
    unsigned long amode = AMIGA_ALL_DELETE;
/*
    kprintf("unix to ados mode\n");
*/
    
    /*
    **	AmigaDOS does permissions kind of bass ackwards
    **
    **	Owner permissions use a SET bit to DENY permissions
    **	and a CLEAR bit to GRANT permissions
    */

    if ((mode & UNIX_OWNER_READ_PERM) == 0)
        amode |= FIBF_READ;
    
    if ((mode & UNIX_OWNER_WRITE_PERM) == 0)
        amode |= FIBF_WRITE;

    if ((mode & UNIX_OWNER_EXEC_PERM) == 0)
        amode |= FIBF_EXECUTE;

    /*
    **	Group and Other permissions, which were added later
    **	use a SET bit to GRANT the permission and CLEAR to DENY
    */

    if (mode & UNIX_GROUP_READ_PERM)
        amode |= FIBF_GRP_READ;
    
    if (mode & UNIX_GROUP_WRITE_PERM)
        amode |= FIBF_GRP_WRITE;

    if (mode & UNIX_GROUP_EXEC_PERM)
        amode |= FIBF_GRP_EXECUTE;
        
    if (mode & UNIX_OTHER_READ_PERM)
        amode |= FIBF_OTR_READ;
    
    if (mode & UNIX_OTHER_WRITE_PERM)
        amode |= FIBF_OTR_WRITE;

    if (mode & UNIX_OTHER_EXEC_PERM)
        amode |= FIBF_OTR_EXECUTE;
        
    return amode;
}
#endif /* 0 */


void ados_convert(struct LzHeader *hdr)
{
    unix_to_ados_filename(hdr->name,strlen(hdr->name));
    D(bug("name=%s method=%.5s\n",hdr->name, hdr->method));
}



#endif /* AMIGA */
