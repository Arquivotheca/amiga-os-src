/*----------------------------------------------------------------------*/
/*	header.c (from lharc.c)	-- header manipulate functions		*/
/*	Original					by Y.Tagawa	*/
/*	modified  Dec 16 1991				by M.Oki	*/
/*----------------------------------------------------------------------*/

/*
** Functions:
**
**      Public:
**              int calc_sum(UBYTE * p, int len)
**              boolean get_header (BPTR  fp, LzHeader * hdr);
**              void init_header (UBYTE * name, struct stat *vstat, LzHeader *hdr);
**              void write_header ( BPTR fp, LzHeader * hdr)
**              
**              
**      Private:
**              ushort get_word(void);
**              void put_word(uint);
**              long get_longword(void);
**              void put_longword(long);
**              void msdos_to_unix_filename (UBYTE * name, int len);
**              void generic_to_unix_filename(UBYTE * name, int len);
**              void macos_to_unix_filename(UBYTE * name, int len);
**              void unix_to_generic_filename(UBYTE * name, int len);
**              long gettz();
**              struct tm * msdos_to_unix_stamp_tm(long); <<<unused>>>
**              time_t generic_to_unix_stamp(long);
**              long unix_to_generic_stamp(time_t);
**              
*/
#include <dos.h>	/* required for Chk_Abort() */

#include "lharc.h"
extern int header_level;

int
calc_sum (UBYTE *p, int len)
{
  int sum;

  for (sum = 0; len; len --)
    sum += *p++;

  return sum & 0xff;
}

static UBYTE *get_ptr;
#define setup_get(PTR)		(get_ptr = (PTR))
#define get_byte()		(*get_ptr++ & 0xff)
#define put_ptr			get_ptr
#define setup_put(PTR)		(put_ptr = (PTR))
#define put_byte(c)		(*put_ptr++ = (UBYTE)(c))

STATIC unsigned short
get_word (void)
{
  unsigned short        b0, b1;

  b0 = get_byte ();
  b1 = get_byte ();
  return ((unsigned short)((b1 << 8) + b0));
}

STATIC void
put_word (unsigned int v)
{
  put_byte (v);
  put_byte (v >> 8);
}

STATIC unsigned long
get_longword ( void )
{
  unsigned long b0, b1, b2, b3;

  b0 = get_byte ();
  b1 = get_byte ();
  b2 = get_byte ();
  b3 = get_byte ();
  return (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
}

STATIC void
put_longword (unsigned long v)
{
  put_byte (v);
  put_byte (v >> 8);
  put_byte (v >> 16);
  put_byte (v >> 24);
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
/*              if the name is all uppercase, is is shifted to          */
/*              lowercase (mixed case names are left unchanged)         */
/*----------------------------------------------------------------------*/
STATIC void
generic_to_ados_filename ( UBYTE * name, int len )
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
macos_to_ados_filename ( UBYTE * name, int len )
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
msdos_to_ados_filename ( UBYTE * name, int len )
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

/*----------------------------------------------------------------------*/
/*           Convert generic filenames to AmigaDOS file names           */
/*              If the first character of the name is /, it is deleted  */
/*----------------------------------------------------------------------*/
STATIC void
unix_to_ados_filename ( UBYTE * name, int len )
{
    if (name[0] == '/')
    {
	memmove(name, name + 1, len - 1);
	name[len-1] = 0;
    }
}

/*----------------------------------------------------------------------*/
/*  Convert AmigaDOS filenames to generic file names                    */
/*      ??? Do we need to do anything???                                */
/*----------------------------------------------------------------------*/

STATIC void
ados_to_generic_filename ( UBYTE * name, int len )
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

/*----------------------------------------------------------------------*/
/*			build header functions				*/
/*----------------------------------------------------------------------*/

static UBYTE gh_dirname[FILENAME_LENGTH];
static UBYTE h_data[LZHEADER_STRAGE];

# define	STD_NAME(addr)	((unsigned char *)addr + sizeof(LzHeader))

boolean readhead_0( BPTR fp, LzHeader * hdr )
{
    int i;
    int name_length;
    int checksum;

    /*
    **	Need to read 2 more bytes for file CRC
    */
    if (myFRead (fp, h_data + hdr->header_size, sizeof (UBYTE), 2) < 2)
	fatal_error("Invalid header (LHarc file ?)", "");
	
    /*
    **	Get the checksum for the header
    */
    setup_get (h_data + I_HEADER_CHECKSUM);
    checksum = get_byte();

    /*
    **	Save the compression method
    */
    bcopy (h_data + I_METHOD, hdr->method, METHOD_TYPE_STRAGE);

    /*
    **	Get the compressed size, original size, modification date and
    **	file attribute
    */
    setup_get (h_data + I_PACKED_SIZE);
    hdr->packed_size	= get_longword ();
    hdr->total_size = hdr->packed_size + hdr->header_size + 2;
    hdr->original_size	= get_longword ();
    hdr->last_modified_stamp = get_longword ();
    hdr->attribute	= get_byte ();

/*
    kprintf("attrib=$%02lx\n",hdr->attribute);
*/

    (void) get_byte();	/* bump the pointer past the header_level */

    if (calc_sum (h_data + I_METHOD, hdr->header_size) != checksum)
	warning ("Checksum error (LHarc file?)", "");

    name_length = get_byte ();

    hdr->name = STD_NAME(hdr);

    for (i = 0; i < name_length; i ++)
	hdr->name[i] =(UBYTE)get_byte ();

    hdr->name[name_length] = '\0';

    convdelim(hdr->name, DELIM);

    hdr->ados_mode = 0;	/* CAS */

    /*
    **	If the header size (less the name) is 24 or more, this is
    **	an extended header.  The name is followed by a 2-byte CRC
    **	checksum for the file and a 1-byte extension type
    */
    if (hdr->header_size - name_length > 25)
    {
	/*
	**	EXTEND FORMAT
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	**	2		CRC
	**	1		extension type
	**	2		extension length
	*/
	hdr->crc				= get_word ();
	hdr->extend_type			= get_byte ();
	hdr->has_crc = TRUE;

/*
	kprintf("hdrsz>=25, extend_type is $%lx\n",hdr->extend_type);
*/

    }
    /*
    **	If the header size is exactly 22, then a 2-byte CRC checksum
    **	follows the name
    */
    else if (hdr->header_size - name_length == 22)
    {
	/*
	**	Generic with CRC
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	**	2		CRC
	*/
	hdr->crc				= get_word ();
	hdr->extend_type			= EXTEND_GENERIC;
	hdr->has_crc = TRUE;
/*
	kprintf("hdrsz==22, extend_type is $%lx\n",hdr->extend_type);
*/

    }
    /*
    **	If the header is 20 bytes, there is no CRC checksum
    */
    else if (hdr->header_size - name_length == 20)
    {
	/*
	**	Generic no CRC
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	*/
	hdr->extend_type			= EXTEND_GENERIC;
	hdr->has_crc = FALSE;
/*
	kprintf("hdrsz==20, extend_type is $%lx\n",hdr->extend_type);
*/
    }
    else
    {
	warning ("Unknown header or header size corrupt", "");
	return FALSE;
    }

    hdr->unix_uid = hdr->unix_gid = (unsigned short)(~0);

    /*
    **	If this is an extended UNIX header, and it is a Level 0
    **	header, then these fields are present:
    **
    **	Designation		size in bytes
    **	-------------------------------------
    **	minor version			1
    **	date				4
    **	mode				2
    **	uid				2
    **	gid				2
    */
    if (hdr->extend_type == EXTEND_UNIX)
    {
	hdr->minor_version		= get_byte ();
	unix_to_ados_stamp(get_longword(), &hdr->ados_last_modified_stamp);
	hdr->ados_mode			= unix_to_ados_mode(get_word ());
	hdr->unix_uid = get_word ();
	hdr->unix_gid = get_word ();
	return TRUE;
    }

    /*
    **	Use the entended type as a clue to decoding the OS specific
    **	fields
    */
    switch (hdr->extend_type)
    {
      case EXTEND_MSDOS:
	msdos_to_ados_filename (hdr->name, name_length);
	/* FALL THROUGH for date processing... */

      case EXTEND_HUMAN:
	/*
	**	Level 2 headers store the time in Unix format
	**	Seconds since Jan 1 1978 GMT
	**
	**	Levels 0 & 1 store the time as an encoding of
	**	the local time.
	*/
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	break;

      case EXTEND_UNIX:
	unix_to_ados_filename (hdr->name, name_length);
	break;
 
      case EXTEND_MACOS:
	macos_to_ados_filename (hdr->name, name_length);
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	break;

      default:

	generic_to_ados_filename (hdr->name, name_length);
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);

	hdr->ados_mode = hdr->attribute; /* CAS */
/*
	kprintf("default: ados_mode = $%08lx\n",hdr->ados_mode);
*/
    }

    return TRUE;
}

void writehead_0( BPTR fp, LzHeader * hdr )
{
    int	name_length;
    int	header_size;
    
    setup_put (h_data + I_PACKED_SIZE);

    put_longword (hdr->packed_size);
    put_longword (hdr->original_size);
    put_longword (hdr->last_modified_stamp);
    put_byte (0x20);
    put_byte (0);

    convdelim(hdr->name, DELIM2);

    name_length = strlen(hdr->name);

    if (name_length > 256)
    {
	fatal_error("Name is too long for Level 0 headers", hdr->name);
    }
    
    put_byte (name_length);
    memcpy (h_data + I_NAME, hdr->name, name_length);
    setup_put (h_data + I_NAME + name_length);

    put_word (hdr->crc);

    /*
    **	The first 2 bytes (header_size and checksum) are not counted
    **	in the header_size.
    */
    header_size = get_ptr - h_data - 2;

    h_data[I_HEADER_SIZE] = header_size;
    h_data[I_HEADER_CHECKSUM] = calc_sum (h_data + I_METHOD, header_size);

    if (myFWrite (fp, h_data, sizeof (UBYTE), put_ptr - h_data) == 0)
	fatal_error ("Cannot write to temporary file", "");

    convdelim(hdr->name, DELIM);
}

boolean readhead_1( BPTR fp, LzHeader * hdr )
{
    int		header_size;
    int		i;
    UBYTE	*ptr;
    int		name_length;
    int		dir_length = 0;
    int		checksum;
    UBYTE	*ext_dir;

    /*
    **	Need to read 2 more bytes for extension length
    */
    if (myFRead (fp, h_data + hdr->header_size, sizeof (UBYTE), 2) < 2)
	fatal_error("Invalid header (LHarc file ?)", "");
	
    /*
    **	Get the checksum for the header
    */
    setup_get (h_data + I_HEADER_CHECKSUM);
    checksum = get_byte();

    /*
    **	Save the size of the header and the compression method
    */
    bcopy (h_data + I_METHOD, hdr->method, METHOD_TYPE_STRAGE);

    /*
    **	Get the compressed size, original size, modification date and
    **	file attribute
    */
    setup_get (h_data + I_PACKED_SIZE);
    hdr->packed_size	= get_longword ();
    hdr->total_size = hdr->packed_size + hdr->header_size + 2;
    hdr->original_size	= get_longword ();
    hdr->last_modified_stamp = get_longword ();

    /* bump the pointer past the reserved byte and the header_level */
    (void) get_word();
    
    if (calc_sum (h_data + I_METHOD, hdr->header_size) != checksum)
	warning ("Checksum error (LHarc file?)", "");

    name_length = get_byte ();

    hdr->name = STD_NAME(hdr);

    for (i = 0; i < name_length; i ++)
	hdr->name[i] =(UBYTE)get_byte ();

    hdr->name[name_length] = '\0';

    hdr->ados_mode = 0;	/* CAS */

    /*
    **	If the header size (less the name) is 24 or more, this is
    **	an extended header.  The name is followed by a 2-byte CRC
    **	checksum for the file, a 1-byte extension type, and a 2-byte
    **	extension size.
    */
    if (hdr->header_size - name_length >= 25)
    {
	/*
	**	EXTEND FORMAT
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	**	2		CRC
	**	1		extension type
	**	2		extension length
	*/
	hdr->crc				= get_word ();
	hdr->extend_type			= get_byte ();
	hdr->has_crc = TRUE;
    }
    /*
    **	If the header size is exactly 22, then a 2-byte CRC checksum
    **	follows the name
    */
    else if (hdr->header_size - name_length == 22)
    {
	/*
	**	Generic with CRC
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	**	2		CRC
	*/
	hdr->crc				= get_word ();
	hdr->extend_type			= EXTEND_GENERIC;
	hdr->has_crc = TRUE;
    }
    /*
    **	If the header is 20 bytes, there is no CRC checksum
    */
    else if (hdr->header_size - name_length == 20)
    {
	/*
	**	Generic no CRC
	**
	**	1		header size
	**	1		checksum
	**	20		header
	**	name_length	name
	*/
	hdr->extend_type			= EXTEND_GENERIC;
	hdr->has_crc = FALSE;
    }
    else
    {
	warning ("Unknown header (LHarc file ?)", "");
	return FALSE;
    }

    hdr->unix_uid = hdr->unix_gid = (unsigned short)(~0);

    setup_get(h_data + hdr->header_size);

    /*
    **	Save the pointer
    */
    ptr = get_ptr;
	
    /*
    **	Get the size of the first extension header
    */
    while((header_size = get_word()) != 0)
    {
	    
	/*
	**	If there is space in the data buffer
	**		Try to head the extension block
	**
	**	If there is not enough space, or the read fails
	**		bail out!
	*/
	if ((sizeof(h_data) - (get_ptr - h_data)) < header_size ||
	     myFRead(fp, get_ptr, sizeof(UBYTE), header_size) < header_size)
	{
	    fatal_error ("Invalid header", "");
	    /* NOTREACHED */
	}

	/*
	**	All extension headers consist of:
	**
	**	Extension header type		1
	**	Extension data			XX
	**	Size of next header		2
	*/
	switch (get_byte())
	{
	  case 0:
	    /*
	    **	Header CRC
	    **
	    **	DCM - I have no docs on how to calc this checksum, so I'm
	    **	skipping it for now.  Specifically, I don't know how much
	    **	of the header is counted.  Is the file CRC included?  The
	    **	extension data?  What about the header CRC?
	    */
	    (void) get_word();
	    break;

	  case 1:
	    /*
	    **	Extended File name
	    */
	    if (header_size - 3 > 0)
	    {
		if ((header_size - 2) > 256)
		{
		    hdr->name = xmalloc(header_size - 2);
		}
	    
		memcpy(hdr->name, get_ptr, header_size - 3);
		hdr->name[header_size - 3] = '\0';
	    }
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 2:
	    /*
	    **	Extended directory
	    */
	    dir_length = header_size - 3;
	    
	    if (dir_length > 0)
	    {
		if (get_ptr[dir_length - 1] == DELIM2)
		    ext_dir = xmalloc(dir_length + 1);
		else
		    ext_dir = xmalloc(dir_length + 2);

		memcpy(ext_dir, get_ptr, dir_length);

		if (get_ptr[dir_length - 1] != DELIM2)
		{
		    /*
		    **	Feh!  Someones archiver didn't leave the
		    **	trailing delimiter.  Add it now.
		    */
		    ext_dir[dir_length] = DELIM2;
		    ext_dir[dir_length + 1] = '\0';
		    dir_length++;
		}
		else
		    ext_dir[dir_length] = '\0';
		
		convdelim(ext_dir, DELIM);
	    }
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x3F:
	    /*
	    **	File Comments
	    */
	    hdr->comment = xmalloc(header_size - 2);
	    memcpy(hdr->comment, get_ptr, header_size - 3);
	    hdr->comment[header_size - 3] = '\0';
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x40:
	    /*
	    **	Extended MS-DOS attribute
	    */
	    if (hdr->extend_type == EXTEND_MSDOS ||
		hdr->extend_type == EXTEND_HUMAN ||
		hdr->extend_type == EXTEND_GENERIC)
		hdr->attribute = get_word();
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x50:
	    /*
	    **	Extended UNIX permission
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
		hdr->ados_mode = unix_to_ados_mode(get_word());
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x51:
	    /*
	    **	Extended UNIX gid and uid
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
	    {
		hdr->unix_gid = get_word();
		hdr->unix_uid = get_word();
	    }
	    break;

	  case 0x52:
	    /*
	    **	Extended UNIX group name
	    **	Unused, discard
	    */
	  case 0x53:
	    /*
	    **	Extended UNIX user name
	    **	Unused, discard
	    */
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x54:
	    /*
	    **	Extended UNIX last modified time
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
		unix_to_ados_stamp(get_longword(),&hdr->ados_last_modified_stamp);
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case EXTEND_AMIGADOS_DATE:
	    if (hdr->extend_type == EXTEND_AMIGADOS)
	    {
		hdr->ados_last_modified_stamp.ds_Days = get_longword();
		hdr->ados_last_modified_stamp.ds_Minute = get_longword();
		hdr->ados_last_modified_stamp.ds_Tick = get_longword();
	    }
	    else
		setup_get(get_ptr + header_size - 3);
	    break;
		
	  case EXTEND_AMIGADOS_MODE:
	    if (hdr->extend_type == EXTEND_AMIGADOS)
	    {
		hdr->ados_mode = get_longword();
	    }
	    else
		setup_get(get_ptr + header_size - 3);
	    break;
		
	  default:
	    /*
	    **	other extended headers
	    **	Unused, discard
	    */
	    setup_get(get_ptr + header_size - 3);
	    break;
	}
    }

    /*
    **	In Level 1 headers, the "packed" size includes the extension
    **	headers.  To maintaina a consistent presentation, move the
    **	size of the extension data to the header size.
    **
    **  (If the difference between the pointers is exactly 2, there
    **	was no extension data.)
    */
    if (get_ptr - ptr != 2)
    {
	hdr->packed_size -= (get_ptr - ptr - 2);
	hdr->header_size += (get_ptr - ptr - 2);
    }

    /*
    **	Directory encoded separately
    **	tack the name and directory together.
    **	The dirname must end in an appropriate separator character
    */
    if (dir_length)
    {
	UBYTE	*temp;
	int	nlen;
	
	nlen = strlen(hdr->name);
	
	if (hdr->name == STD_NAME(hdr))
	{
	    if ((nlen + dir_length) <= 256)
	    {
		temp = xstrdup(STD_NAME(hdr));
		strcpy(STD_NAME(hdr), ext_dir);
		strcat(STD_NAME(hdr), temp);
		free(temp);
		temp = STD_NAME(hdr);
	    }
	    else
	    {
		temp = xmalloc(nlen + dir_length + 1);
		strcpy(temp, ext_dir);
		strcat(temp, STD_NAME(hdr));
	    }
	}
	else
	{
	    temp = xrealloc(ext_dir, nlen + dir_length + 1);
	    ext_dir = NULL;
	    strcat(temp, STD_NAME(hdr));
	}
	
	name_length += dir_length;

	if (hdr->name != STD_NAME(hdr))
	    free(hdr->name);

	hdr->name = temp;

	if (ext_dir)
	    free(ext_dir);
    }

    convdelim(hdr->name, DELIM);
    
    /*
    **	Use the entended type as a clue to decoding the OS specific
    **	fields
    */
    switch (hdr->extend_type)
    {
      case EXTEND_MSDOS:
	msdos_to_ados_filename (hdr->name, name_length);
	/* FALL THROUGH for date processing... */

      case EXTEND_HUMAN:
	/*
	**	Level 2 headers store the time in Unix format
	**	Seconds since Jan 1 1978 GMT
	**
	**	Levels 0 & 1 store the time as an encoding of
	**	the local time.
	*/
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	break;

      case EXTEND_UNIX:
	unix_to_ados_filename (hdr->name, name_length);
	break;
 
      case EXTEND_MACOS:
	macos_to_ados_filename (hdr->name, name_length);
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	break;
	
      case EXTEND_AMIGADOS:
	break;	/* already there */

      default:
	generic_to_ados_filename (hdr->name, name_length);
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	hdr->ados_mode = hdr->attribute; /* CAS */

    }

    if (hdr->ados_mode == 0)
    {
	hdr->ados_mode = AMIGA_DEF_MODE;
    }

    return TRUE;
}

void writehead_1( BPTR fp, LzHeader * hdr )
{
    int		length, header_size;
    char	*p;
    
    setup_put (h_data + I_ORIGINAL_SIZE);

    put_longword (hdr->original_size);

    put_longword (hdr->last_modified_stamp);

    put_byte (0x20);
    put_byte (1);

    if ((p = strrchr(hdr->name, '/')) != NULL)
    {
	p++;
	length = strlen(p);
    }
    else
	if ((p = strrchr(hdr->name, ':')) != NULL)
	{
	    p++;
	    length = strlen(p);
	}
	else
	{
	    length = strlen(hdr->name);
	}

    if (length <= MAX_L1_NAMELEN)
    {
	if (p)
	{
	    put_byte (length);
	    memcpy(h_data + I_NAME, p, length);
	    setup_put (h_data + I_NAME + length);
	}
	else
	{
	    convdelim(hdr->name, DELIM2);

	    put_byte (length);
	    memcpy(h_data + I_NAME, hdr->name, length);
	    setup_put (h_data + I_NAME + length);
	}
    }
    else
    {
	put_byte(0);
    }
    
    put_word (hdr->crc);

    /*
    **	Extended format is AmigaDOS ('a')
    */
    put_byte(EXTEND_AMIGADOS);

    header_size = get_ptr - h_data;
    
    if (length > MAX_L1_NAMELEN || p)
    {
	UBYTE	*ext_name, *ext_dir, c;
	
	ext_dir = hdr->name;
	
	if (p)
	{
	    ext_name = p;
	}
	else
	{
	    ext_name = hdr->name;
	    ext_dir = NULL;
	}
	
	/*
	**	Extended file name (0x01)
	**
	**	Check the length.  If it was <= MAX_L1_NAMELEN, the
	**	name was stored in the header proper, so don't store
	**	it here.
	*/
	if (ext_name && length > MAX_L1_NAMELEN)
	{
	    put_word(1 + length + 2);
	    put_byte(1);
	    bcopy (ext_name, put_ptr, length);
	    setup_put (put_ptr + length);
	}
	
	/*
	**	Extended Directory Name (0x02)
	*8
	**	Always store the directory if there was one present.
	*/
	if (ext_dir)
	{
	    c = *ext_name;
	    *ext_name = '\0';
	
	    length = strlen(ext_dir);
	    put_word(1 + length + 2);
	    put_byte(2);
	    convdelim(ext_dir, DELIM2);
	    bcopy (ext_dir, put_ptr, length);
	    setup_put (put_ptr + length);

	    *ext_name = c;
	}
    }
    
    put_word(1 + 4 + 2);		/* type + LONG mode + 2-byte size */

    /*
    **	Extended AmigaDOS file mode, including extra bits for Envoy FS (0x80)
    */
    put_byte(EXTEND_AMIGADOS_MODE);
    put_longword(hdr->ados_mode);
    put_word(1 + 4 + 4 + 4 + 2);	/* type + 3-LONG date + 2-byte size */

    /*
    **	DateStamp format timestamp, speeds unpacking (0x81)
    */
    put_byte(EXTEND_AMIGADOS_DATE);
    put_longword(hdr->ados_last_modified_stamp.ds_Days);
    put_longword(hdr->ados_last_modified_stamp.ds_Minute);
    put_longword(hdr->ados_last_modified_stamp.ds_Tick);

    put_word(0);
    length = put_ptr - h_data;
    
    setup_put (h_data + I_SKIP_SIZE);

    put_longword (hdr->packed_size + length - header_size - 2);
    
    h_data[I_HEADER_SIZE] = header_size;
    h_data[I_HEADER_CHECKSUM] = calc_sum (h_data + I_METHOD, header_size);

    if (myFWrite (fp, h_data, sizeof (UBYTE), length) != length)
	fatal_error ("Cannot write to temporary file", "");

    convdelim(hdr->name, DELIM);
}

boolean readhead_2( BPTR fp, LzHeader * hdr )
{
    int 	header_size;
    int 	name_length;
    int 	dir_length = 0;
    UBYTE	*ext_dir;
    
    setup_get(h_data);
    
    /*
    **	Save the size of the header and the compression method
    */
    header_size = get_word();

    if (header_size > hdr->header_size)
    {
    }
    

    bcopy (h_data + I_METHOD, hdr->method, METHOD_TYPE_STRAGE);

    /*
    **	Get the compressed size, original size, modification date and
    **	file attribute
    */
    setup_get (h_data + I_PACKED_SIZE);
    hdr->packed_size	= get_longword ();
    hdr->total_size = hdr->packed_size + hdr->header_size;
    hdr->original_size	= get_longword ();
    hdr->last_modified_stamp = get_longword ();

    /* bump the pointer past the reserved byte and the header_level */
    (void) get_word();
    

    generic_to_ados_stamp(hdr->last_modified_stamp,
			  &hdr->ados_last_modified_stamp);
    name_length = 0;

    hdr->crc				= get_word ();
    hdr->extend_type			= get_byte ();
    hdr->has_crc = TRUE;

    hdr->unix_uid = hdr->unix_gid = (unsigned short)(~0);

    /*
    **	Get the size of the first extension header
    */
    while((header_size = get_word()) != 0)
    {
	    
	/*
	**	All extension headers consist of:
	**
	**	Extension header type		1
	**	Extension data			XX
	**	Size of next header		2
	*/
	switch (get_byte())
	{
	  case 0:
	    /*
	    **	Header CRC
	    **
	    **	DCM - I have no docs on how to calc this checksum, so I'm
	    **	skipping it for now.  Specifically, I don't know how much
	    **	of the header is counted.  Is the file CRC included?  The
	    **	extension data?  What about the header CRC?
	    */
	    (void) get_word();
	    break;

	  case 1:
	    /*
	    **	Extended File name
	    */
	    if (header_size - 3 > 0)
	    {
		if (header_size - 3 > 256)
		    hdr->name = xmalloc(header_size - 2);
		else
		    hdr->name = STD_NAME(hdr);
		
		memcpy(hdr->name, get_ptr, header_size - 3);
		hdr->name[header_size - 3] = '\0';
	    }
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 2:
	    /*
	    **	Extended directory
	    */
	    dir_length = header_size - 3;

	    if (dir_length > 0)
	    {
		if (get_ptr[dir_length - 1] == DELIM2)
		    ext_dir = xmalloc(dir_length + 1);
		else
		    ext_dir = xmalloc(dir_length + 2);

		memcpy(ext_dir, get_ptr, dir_length);

		if (get_ptr[dir_length - 1] != DELIM2)
		{
		    /*
		    **	Feh!  Someones archiver didn't leave the
		    **	trailing delimiter.  Add it now.
		    */
		    ext_dir[dir_length] = DELIM2;
		    ext_dir[dir_length + 1] = '\0';
		    dir_length++;
		}
		else
		    ext_dir[dir_length] = '\0';
		
		convdelim(ext_dir, DELIM);
	    }
	    
	    setup_get(get_ptr + dir_length);
	    break;

	  case 0x3F:
	    /*
	    **	File Comments
	    */
	    hdr->comment = xmalloc(header_size - 2);
	    
	    memcpy(hdr->comment, get_ptr, header_size - 3);
	    hdr->comment[header_size - 3] = '\0';
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x40:
	    /*
	    **	Extended MS-DOS attribute
	    */
	    if (hdr->extend_type == EXTEND_MSDOS ||
		hdr->extend_type == EXTEND_HUMAN ||
		hdr->extend_type == EXTEND_GENERIC)
		hdr->attribute = get_word();
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x50:
	    /*
	    **	Extended UNIX permission
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
		hdr->ados_mode = unix_to_ados_mode(get_word());
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x51:
	    /*
	    **	Extended UNIX gid and uid
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
	    {
		hdr->unix_gid = get_word();
		hdr->unix_uid = get_word();
	    }
	    break;

	  case 0x52:
	    /*
	    **	Extended UNIX group name
	    **	Unused, discard
	    */
	  case 0x53:
	    /*
	    **	Extended UNIX user name
	    **	Unused, discard
	    */
	    setup_get(get_ptr + header_size - 3);
	    break;

	  case 0x54:
	    /*
	    **	Extended UNIX last modified time
	    */
	    if (hdr->extend_type == EXTEND_UNIX)
		unix_to_ados_stamp(get_longword(),&hdr->ados_last_modified_stamp);
	    else
		setup_get(get_ptr + header_size - 3);
	    break;

	  case EXTEND_AMIGADOS_DATE:
	    if (hdr->extend_type == EXTEND_AMIGADOS)
	    {
		hdr->ados_last_modified_stamp.ds_Days = get_longword();
		hdr->ados_last_modified_stamp.ds_Minute = get_longword();
		hdr->ados_last_modified_stamp.ds_Tick = get_longword();
	    }
	    else
		setup_get(get_ptr + header_size - 3);
	    break;
		
	  case EXTEND_AMIGADOS_MODE:
	    if (hdr->extend_type == EXTEND_AMIGADOS)
	    {
		hdr->ados_mode = get_longword();
	    }
	    else
		setup_get(get_ptr + header_size - 3);
	    break;
		
	  default:
	    /*
	    **	other extended headers
	    **	Unused, discard
	    */
	    setup_get(get_ptr + header_size - 3);
	    break;
	}
    }

    /*
    **	Directory encoded separately
    **	tack the name and directory together.
    **	The dirname must end in an appropriate separator character
    */
    if (dir_length)
    {
	UBYTE	*temp;
	int	nlen;
	
	nlen = strlen(hdr->name);
	
	if (hdr->name == STD_NAME(hdr))
	{
	    if ((nlen + dir_length) <= 256)
	    {
		temp = xstrdup(STD_NAME(hdr));
		strcpy(STD_NAME(hdr), ext_dir);
		strcat(STD_NAME(hdr), temp);
		free(temp);
		temp = STD_NAME(hdr);
	    }
	    else
	    {
		temp = xmalloc(nlen + dir_length + 1);
		strcpy(temp, ext_dir);
		strcat(temp, STD_NAME(hdr));
	    }
	}
	else
	{
	    temp = xrealloc(ext_dir, nlen + dir_length + 1);
	    ext_dir = NULL;
	    strcat(temp, STD_NAME(hdr));
	}
	
	name_length += dir_length;

	if (hdr->name != STD_NAME(hdr))
	    free(hdr->name);

	hdr->name = temp;

	if (ext_dir)
	    free(ext_dir);
    }

    convdelim(hdr->name, DELIM);
    
    /*
    **	Use the entended type as a clue to decoding the OS specific
    **	fields
    */
    switch (hdr->extend_type)
    {
      case EXTEND_MSDOS:
	msdos_to_ados_filename (hdr->name, name_length);
	/* FALL THROUGH for date processing... */

      case EXTEND_HUMAN:
	/*
	**	Level 2 headers store the time in Unix format
	**	Seconds since Jan 1 1978 GMT
	**
	**	Levels 0 & 1 store the time as an encoding of
	**	the local time.
	*/
	unix_to_ados_stamp(hdr->last_modified_stamp,
			   &hdr->ados_last_modified_stamp);
	break;

      case EXTEND_UNIX:
	unix_to_ados_filename (hdr->name, name_length);
	break;
 
      case EXTEND_MACOS:
	macos_to_ados_filename (hdr->name, name_length);
	generic_to_ados_stamp (hdr->last_modified_stamp,
			       &hdr->ados_last_modified_stamp);
	break;

      case EXTEND_AMIGADOS:
	break;	/* we are already finished */
	
      default:
	generic_to_ados_filename (hdr->name, name_length);
	unix_to_ados_stamp(hdr->last_modified_stamp,
			   &hdr->ados_last_modified_stamp);
    }
     

    if (hdr->ados_mode == 0)
    {
	hdr->ados_mode = AMIGA_DEF_MODE;
    }

    return TRUE;
}

void writehead_2( BPTR fp, LzHeader * hdr )
{
    UBYTE	*ext_name, *ext_dir, c;
    UBYTE	*ptr;
    int		name_length;
    
    setup_put (h_data + I_PACKED_SIZE);

    put_longword (hdr->packed_size);
    put_longword (hdr->original_size);

    put_longword (hdr->last_modified_stamp);

    put_byte (0x20);
    put_byte (2);

    put_word (hdr->crc);

    /*
    **	This is an amigados extended format
    */
    put_byte(EXTEND_AMIGADOS);
    put_word(1 + 4 + 2);

    /*
    **	The file mode, including extra bits for Envoy FS (0x80)
    */
    put_byte(EXTEND_AMIGADOS_MODE);
    put_longword(hdr->ados_mode);
    put_word(1 + 4 + 4 + 4 + 2);

    /*
    **	DateStamp format date, speeds unpacking (0x81)
    */
    put_byte(EXTEND_AMIGADOS_DATE);
    put_longword(hdr->ados_last_modified_stamp.ds_Days);
    put_longword(hdr->ados_last_modified_stamp.ds_Minute);
    put_longword(hdr->ados_last_modified_stamp.ds_Tick);
    
    ext_dir = hdr->name;
	
    if ((ext_name = strrchr(ext_dir, '/')) == NULL &&
	(ext_name = strrchr(ext_dir, ':')) == NULL)
    {
	ext_name = STD_NAME(hdr);
	ext_dir = NULL;
    }
    else
    {
	ext_name++;
	c = *ext_name;
    }
    
    /*
    **	Extended file name (0x01)
    */
    if (ext_name)
    {
	name_length = strlen(ext_name);
	put_word(1 + name_length + 2);
	put_byte(1);
	bcopy (ext_name, put_ptr, name_length);
	setup_put (put_ptr + name_length);
    }
    
    /*
    **	Extended directory name (0x02)
    */
    if (ext_dir)
    {
	*ext_name = '\0';
	
	name_length = strlen(ext_dir);
	put_word(1 + name_length + 2);
	put_byte(2);
	convdelim(ext_dir, DELIM2);
	bcopy (ext_dir, put_ptr, name_length);
	setup_put (put_ptr + name_length);

	*ext_name = c;
    }
    
    put_word(0);

    /*
    **	Save the current position
    **	set the pointer to the start of the buffer, and
    **	write the difference between the two addresses (the header size)
    **	as a 16-bit value
    */
    ptr = put_ptr;
    setup_put (h_data);
    put_word(ptr - h_data);
    
    if (myFWrite (fp, h_data, sizeof (UBYTE), ptr - h_data) != (ptr - h_data))
	fatal_error ("Cannot write to temporary file", "");

    convdelim(hdr->name, DELIM);
}

struct hdrrdr Header_Levels [] =
{
    {0,	readhead_0, writehead_0},
    {1,	readhead_1, writehead_1},
    {2,	readhead_2, writehead_2},
    {-1, NULL}
};

boolean
get_header (BPTR fp, LzHeader *hdr )
{
    int header_size;
    int i;

    Chk_Abort();
    
    memset(hdr, 0, sizeof (LzHeader));

    /*
    **	First byte is header size
    */

    h_data [0] = header_size = myFGetC (fp);
    if (header_size == EOF || header_size == 0)
    {
	return FALSE;		/* finish */
    }

    if (header_size < I_MINIMUM_HEADER_SIZE)
	header_size = I_MINIMUM_HEADER_SIZE;
    
    hdr->header_size = header_size;

    /*
    **	The header size includes the size byte, which we've
    **	already read.  So, read the rest of the header
    **
    **	But Wait!!  There's more!!
    **
    **	For a L0 or L1 header, the size starts counting *after* the
    **	checksum byte, so the header is really 2 bytes longer than it
    **	claims.  However, until the header has been read the header
    **	level is unknown.
    **
    **	So, get_header will a number of bytes equaling the unsigned
    **	value of the first character.  If a particular header type
    **	needs more than that, the appropriate readhead_*() function
    **	must fetch the excess.
    */
    i = myFRead (fp, h_data + I_HEADER_CHECKSUM, sizeof (UBYTE),
		 header_size - 1);
    
    if (i + 1 < header_size)
    {
	fatal_error ("Invalid header", "");
    }
    
    /*
    **	Get the header level
    **	It may be 0, 1, or 2
    */
    setup_get(h_data + I_HEADER_LEVEL);
    hdr->header_level = get_byte();

    for (i = 0; Header_Levels[i].level >= 0; i++)
    {
	if (Header_Levels[i].level == hdr->header_level)
	{
	    return (*(Header_Levels[i].readhdr))(fp, hdr);
	}
    }
    
    return FALSE;
}



void
init_header (UBYTE *name, struct FileInfoBlock * v_stat, LzHeader *hdr)
{
    int		len, slen;

    if ( compress_method == 5 )
	memcpy (hdr->method, LZHUFF5_METHOD, METHOD_TYPE_STRAGE);
    else if ( compress_method )
	memcpy (hdr->method, LZHUFF1_METHOD, METHOD_TYPE_STRAGE);
    else
	memcpy (hdr->method, LZHUFF0_METHOD, METHOD_TYPE_STRAGE);

    hdr->packed_size		= 0;
    hdr->original_size		= v_stat->fib_Size;
    hdr->last_modified_stamp	= ados_to_generic_stamp(v_stat->fib_Date);
    hdr->attribute		= GENERIC_ATTRIBUTE;
    hdr->header_level		= header_level;
    hdr->crc			= 0;

    slen = len = strlen (name);

    hdr->extend_type		= EXTEND_AMIGADOS;
    hdr->ados_last_modified_stamp	= v_stat->fib_Date;
    hdr->ados_mode		= v_stat->fib_Protection;
    hdr->unix_uid		= v_stat->fib_OwnerUID;
    hdr->unix_gid		= v_stat->fib_OwnerGID;

    if (is_directory(v_stat))
    {
	hdr->ados_mode |= AMIGA_DIRECTORY;
	if (len > 0 && name[len-1] != '/')
	    slen++;
    }

    if (hdr->name && hdr->name != STD_NAME(hdr))
	free(hdr->name);
	
    hdr->name = STD_NAME(hdr);
    
    if (slen > 256)
    {
	if (header_level == 0 || slen > (unsigned short)(~0))
	    fatal_error("File name is too long", name);

	hdr->name = xstrdup(name);
    }
    else
    {
	strcpy (STD_NAME(hdr), name);
	if (is_directory(v_stat))
	{
	    memcpy (hdr->method, LZHDIRS_METHOD, METHOD_TYPE_STRAGE);
	    hdr->attribute = GENERIC_DIRECTORY_ATTRIBUTE;
	    hdr->original_size = 0;
	    if (len > 0 && hdr->name[len-1] != '/')
		strcpy (&hdr->name[len++], "/");
	}
    }
    

    if (generic_format)
    {
	ados_to_generic_filename (hdr->name, len);
    }
}


/* Write generic header. */
void
write_header (BPTR fp, LzHeader *hdr)
{
    int	i;

    Chk_Abort();
    
    memcpy (h_data + I_METHOD, hdr->method, METHOD_TYPE_STRAGE);

    for (i = 0; Header_Levels[i].level >= 0; i++)
    {
	if (Header_Levels[i].level == hdr->header_level)
	{
	    return (*(Header_Levels[i].writehdr))(fp, hdr);
	}
    }

    sprintf(h_data, "%d", hdr->header_level);
    
    fatal_error("Don't know how to write this kind of header", h_data);
}
