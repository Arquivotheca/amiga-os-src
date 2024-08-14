/*----------------------------------------------------------------------*/
/*		LHarc List Command					*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*  V0.00  Original				1988.05.23  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V1.01  Bug Fix for month name		1989.12.25  Y.Tagawa	*/
/*----------------------------------------------------------------------*/

#include "lharc.h"

static long packed_size_total;
static long original_size_total;
static int list_files;

/*----------------------------------------------------------------------*/
/*				Print Stuff				*/
/*----------------------------------------------------------------------*/

/*
**	LIST
**
Original  Packed  Ratio   Date      Time   Name
-------- -------- ----- --------- -------- -------------
NNNNNNNN NNNNNNNN NN.N% NN-SSS-NN NN:NN:NN N...
-------- -------- ----- --------- -------- -------------
NNNNNNNN NNNNNNNN NN.N% NN-SSS-NN NN:NN:NN Total N files
**
**	LIST VERBOSE
**
Original  Packed  Ratio   Date      Time     Perms   Method CRC  Name
-------- -------- ----- --------- -------- --------- ------ ---- -------------
NNNNNNNN NNNNNNNN NN.N% NN-SSS-NN NN:NN:NN hsparwed   -lSS- XXXX N...
-------- -------- ----- --------- -------- --------- ------ ---- -------------
NNNNNNNN NNNNNNNN NN.N% NN-SSS-NN NN:NN:NN Total N files
**
*/


STATIC void
print_size (long packed_size, long original_size)
{
long size_diff;

  printf ("%8d ", original_size);
  printf ("%8d ", packed_size);

/* OLD (was backwards from LHA)
  if (original_size == 0L)
    printf (" 0.0% ");
  else
    printf ("%2d.%1d%% ",
	    (int)((packed_size * 100L) / original_size),
	    (int)((packed_size * 1000L) / original_size) % 10);
*/
  size_diff = original_size - packed_size;

  if (size_diff <= 0L)
    printf (" 0.0% ");
  else
    printf ("%2d.%1d%% ",
	    (int)((size_diff * 100L) / original_size),
	    (int)((size_diff * 1000L) / original_size) % 10);

}


#ifdef AMIGA
char *__montbl[12] = 
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
#else
extern char *__montbl[];
#endif

static boolean got_now = FALSE;
static unsigned int threshold;
extern unsigned int dsboy[];

/* need 12 or 17 (when verbose_listing is TRUE) column spaces */
STATIC void
print_stamp (struct DateStamp t)
{
    unsigned int days;
    int year, month, day, hour, min, sec, i;

    if (t.ds_Days == 0 && t.ds_Minute == 0 && t.ds_Tick == 0)
    {
	printf ("%19.19s", ""); /* 19 spaces */
        return;
    }
    
    year = t.ds_Days / 365 + 78;
    days = t.ds_Days % 365 - (t.ds_Days / 365 - 1) / 4;

    for (i = 0; i < 12; i++)
        if (dsboy[i] > days)
            break;
    month = i - 1;
    day = days - dsboy[i - 1];
    
    hour = t.ds_Minute / 60;
    min = t.ds_Minute % 60;
    sec = t.ds_Tick / 50;

    printf ("%02d-%s-%02d %02d:%02d:%02d ",
	    day, __montbl[month], year, hour, min, sec);
}

STATIC void
print_bar (void)
{

    printf ("-------- -------- ----- --------- -------- ");

    if (verbose_listing)
	printf ("--------- ------ ---- -------------\n");
    else
	printf ("-------------\n");

  
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

STATIC void
list_header (void)
{
    printf ("Original  Packed  Ratio   Date      Time   ");

    if (verbose_listing)
	printf ("  Perms   Method CRC  Name\n");
    else
	printf ("Name\n");

    print_bar ();
}

STATIC void
list_one (LzHeader *hdr)
{
    register int mode;
    char method[6];

    strncpy(method,hdr->method,5);
    method[5]='\0';

    print_size (hdr->packed_size, hdr->original_size);

    print_stamp (hdr->ados_last_modified_stamp);

    if (verbose_listing)
    {
	switch ( mode=hdr->extend_type )
	{
	  case EXTEND_UNIX:
	  	/*
		**	Drop through to AMIGADOS, because the mode bits have
		**	already been converted
		*/
		
	  case EXTEND_AMIGADOS:
	    mode=hdr->ados_mode;
	    printf ("%c%c%c%c%c%c%c  ",
		    ((mode & FIBF_SCRIPT) ? 's' : '-'),
		    ((mode & FIBF_PURE) ? 'p' : '-'),
		    ((mode & FIBF_ARCHIVE) ? 'a' : '-'),
		    ((mode & FIBF_READ) ? '-' : 'r'),
		    ((mode & FIBF_WRITE) ? '-' : 'w'),
		    ((mode & FIBF_EXECUTE)  ? '-' : 'e'),
		    ((mode & FIBF_DELETE)  ? '-' : 'd'));
	    break;

	  default:
	    printf("----rwed  ");
	    break;
	}
  
	if (hdr->has_crc)
	    printf (" %5.5s %04x ", method, hdr->crc);
	else
	    printf (" %5.5s ---- ", method);
    }

    printf ("%s\n", hdr->name);
}

STATIC void
list_tailer (void)
{
  struct FileInfoBlock fib;
  BPTR	lock;
  
  print_bar ();

  print_size (packed_size_total, original_size_total);

  if ((lock = Lock(archive_name, ACCESS_READ)) && Examine(lock, &fib) != FALSE)
  {
      print_stamp (fib.fib_Date);
      UnLock(lock);
  }
  else
  {
      printf(" Unknown  Unknown  ");
  }

  printf ("Total %d file%c ",
	  list_files, (list_files == 1) ? ' ' : 's');

  printf ("\n");
}

/*----------------------------------------------------------------------*/
/*		LIST COMMAND MAIN					*/
/*----------------------------------------------------------------------*/

void
cmd_list (void)
{
    BPTR afp;
    LzHeader *hdr;

    /* initialize total count */
    packed_size_total = 0L;
    original_size_total = 0L;
    list_files = 0;

    /* open archive file */
    if ((afp = open_old_archive ()) == NULL)
	fatal_error ("Cannot perform LIST.  Cannot open",archive_name);

    if (archive_is_msdos_sfx1 (archive_name))
	skip_msdos_sfx1_code (afp);

    /* print header message */
    if (!quiet)
	list_header ();

    hdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);

    /* print each file information */
    while (get_header (afp, hdr))
    {
	if (need_file (hdr->name))
	{
	    list_one (hdr);
	    list_files ++;
	    packed_size_total += hdr->packed_size;
	    original_size_total += hdr->original_size;
	}

	Seek (afp, hdr->packed_size, OFFSET_CURRENT);
    }

    /* close archive file */
    Close (afp);
    clearcame(afp);

    /* print tailer message */
    if (!quiet)
	list_tailer ();

    free(hdr);
    return;
}




