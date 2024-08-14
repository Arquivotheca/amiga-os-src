/*----------------------------------------------------------------------*/
/*		LHarc Archiver Driver for UNIX				*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*		Thanks to H.Yoshizaki. (MS-DOS LHarc)			*/
/*									*/
/*  V0.00  Original				1988.05.23  Y.Tagawa	*/
/*  V0.01  Alpha Version (for 4.2BSD)		1989.05.28  Y.Tagawa	*/
/*  V0.02  Alpha Version Rel.2			1989.05.29  Y.Tagawa	*/
/*  V0.03  Release #3  Beta Version		1989.07.02  Y.Tagawa	*/
/*  V0.03a Debug				1989.07.03  Y.Tagawa	*/
/*  V0.03b Modified				1989.07.13  Y.Tagawa	*/
/*  V0.03c Debug (Thanks to void@rena.dit.junet)1989.08.09  Y.Tagawa	*/
/*  V0.03d Modified (quiet and verbose)		1989.09.14  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V1.01  Bug Fixed				1989.12.25  Y.Tagawa	*/
/*									*/
/*  DOS-Version Original LHx V C2.01 		(C) H.Yohizaki		*/
/*									*/
/*  V2.00  UNIX Lharc + DOS LHx -> OSK LHx	1990.11.01  Momozou	*/
/*  V2.01  Minor Modified			1990.11.24  Momozou	*/
/*									*/
/*  V0.02  LHx for UNIX				1991.11.18  M.Oki	*/
/*  V0.03  LHa for UNIX				1991.12.17  M.Oki	*/
/*  V0.04  LHa for UNIX	beta version		1992.01.20  M.Oki	*/
/*  V1.00  LHa for UNIX	Fixed			1992.03.19  M.Oki	*/
/*  Make lhextract ("lhex" ?)                   1993        CAS         */
/*----------------------------------------------------------------------*/

#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include "lharc.h"
#ifdef LHEXTRACT
#include "lhex_rev.h"
#else
#include "catslha_rev.h"
#endif
#include "args.h"


static volatile char * sysrev = VERSTAG;


/*----------------------------------------------------------------------*/
/*				PROGRAM 				*/
/*----------------------------------------------------------------------*/


#define LHCMD_UNKNOWN	0
#define LHCMD_EXTRACT	1
#define LHCMD_ADD	2
#define LHCMD_LIST	3
#define LHCMD_DELETE	4

static int	cmd = LHCMD_UNKNOWN;
char		**cmd_filev;
int		cmd_filec;

char		*archive_name;
char		expanded_archive_name[FILENAME_LENGTH];
char		temporary_name[FILENAME_LENGTH];
char		backup_archive_name[FILENAME_LENGTH];

int		S_T_S = 0;
char		STS_Buff[8];

/* static functions */
STATIC void	sort_files(void);

/* options */
boolean		quiet = FALSE;
boolean		text_mode = FALSE;
boolean		verbose = FALSE;
boolean		noexec = FALSE;	/* debugging option */
boolean		force = FALSE;
boolean		prof = FALSE;
int		compress_method = 5;	/* deafult -lh5- */
int		header_level = HEADER_LEVEL0;

/* view command flags */
boolean		verbose_listing = FALSE;

/* extract command flags */
boolean		output_to_stdout = FALSE;

/* append command flags */
boolean		new_archive = FALSE;
boolean		update_if_newer = FALSE;
boolean		delete_after_append = FALSE;
boolean		generic_format = FALSE;

boolean		remove_temporary_at_error = FALSE;
boolean		recover_archive_when_interrupt = FALSE;
boolean		remove_extracting_file_when_interrupt = FALSE;
boolean 	get_filename_from_stdin = FALSE;
boolean 	ignore_directory = FALSE;
boolean 	verify_mode = FALSE;

char		*extract_directory = NULL;
char		**xfilev;
int		xfilec = 257;

/*----------------------------------------------------------------------*/
/* NOTES :		Text File Format				*/
/*	GENERATOR		NewLine					*/
/*	[generic]		0D 0A					*/
/*	[MS-DOS]		0D 0A					*/
/*	[OS9][MacOS]		0D					*/
/*	[UNIX]			0A					*/
/*----------------------------------------------------------------------*/

STATIC void
print_help_and_exit (void)
{
#ifndef LHEXTRACT
    fprintf(stderr,
	    "usage: catslha command [options] archive_file [file...]\n");
#else
    fprintf(stderr,
	    "usage: lhex command [options] archive_file [file...]\n");
#endif
    fprintf(stderr, "Commands:\n");
#ifndef LHEXTRACT
    fprintf(stderr, "    Add (A)		- Add files\n");
#endif
    fprintf(stderr, "    Extract (X)		- Extract files\n");
    fprintf(stderr, "    List (L)		- List contents\n");
#ifndef LHEXTRACT
    fprintf(stderr, "    Update (U)		- Replace files if newer\n");
    fprintf(stderr, "    Remove	(R)		- Delete files\n");
    fprintf(stderr, "    Create (C)		- Create a new archive (removing if exists)\n");
#endif
    fprintf(stderr, "    Print (P)		- Print files to stdout\n");
    fprintf(stderr, "    Test (T)		- Test integrity of archive\n");
    fprintf(stderr, "    Help (H)		- This help message\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    Quiet (Q)		- Suppress informational messages\n");
    fprintf(stderr, "    Verbose (V)		- Be verbose\n");
    fprintf(stderr, "    Noexec (N)		- Print what would be done\n");
    fprintf(stderr, "    Force (F)		- Overwrite on extract\n");
#ifndef LHEXTRACT
    fprintf(stderr, "    Text		- Files contain only text\n");
    fprintf(stderr,
	    "    Old			- Use LHArc compression (-lh1-)\n");
#endif
    fprintf(stderr,
	    "    Directory=<dir> (D=)- Unpack archive in directory\n");
#ifndef LHEXTRACT
    fprintf(stderr, "    Delete		- Remove files after archiving\n");
#endif
    fprintf(stderr,
	    "    Ignorepath (I)	- Ignore path when unpacking\n");
#ifndef LHEXTRACT
    fprintf(stderr, "    Nocompress		- Disable compression\n");
#endif
    fprintf(stderr, "\n");

    fprintf(stderr, 
"NOTE: 1-letter abbreviations must be separated by spaces\n\n");

    myexit (1);
}

#ifdef LHEXTRACT

#if	defined(IDEBUG)
# define	TEMPLATE	"ARCHIVE/A,FILES/M,X=EXTRACT/S,L=LIST/S,P=PRINT/S,T=TEST/S,H=HELP/S,Q=QUIET/S,V=VERBOSE/S,N=NOEXEC/S,F=FORCE/S,I=IGNOREPATH/S,D=DIRECTORY/K,DEBUG/S"
#else
# define	TEMPLATE	"ARCHIVE/A,FILES/M,X=EXTRACT/S,L=LIST/S,P=PRINT/S,T=TEST/S,H=HELP/S,Q=QUIET/S,V=VERBOSE/S,N=NOEXEC/S,F=FORCE/S,I=IGNOREPATH/S,D=DIRECTORY/K"
#endif

#else

#if	defined(IDEBUG)
# define	TEMPLATE	"ARCHIVE/A,FILES/M,A=ADD/S,X=EXTRACT/S,L=LIST/S,U=UPDATE/S,R=REMOVE/S,C=CREATE/S,P=PRINT/S,T=TEST/S,H=HELP/S,Q=QUIET/S,V=VERBOSE/S,N=NOEXEC/S,F=FORCE/S,TEXT/S,OLD/S,DELETE/S,I=IGNOREPATH/S,NOCOMPRESS/S,D=DIRECTORY/K,HEADER/N,DEBUG/S"
#else
# define	TEMPLATE	"ARCHIVE/A,FILES/M,A=ADD/S,X=EXTRACT/S,L=LIST/S,U=UPDATE/S,R=REMOVE/S,C=CREATE/S,P=PRINT/S,T=TEST/S,H=HELP/S,Q=QUIET/S,V=VERBOSE/S,N=NOEXEC/S,F=FORCE/S,TEXT/S,OLD/S,DELETE/S,I=IGNOREPATH/S,NOCOMPRESS/S,D=DIRECTORY/K,HEADER/N"
#endif

#endif

#ifdef LHEXTRACT
char * Keywords[32] =
{
    "ARCHIVE",    "FILES",    "EXTRACT",     "LIST",
    "PRINT",      "TEST",
    "HELP",       "QUIET",    "VERBOSE",  "NOEXEC",      "FORCE",
    "IGNOREPATH",
    "DIRECTORY",
#if	defined(IDEBUG)
    "DEBUG"
#endif
};

#else

char * Keywords[OPT_COUNT] =
{
    "ARCHIVE",    "FILES",    "ADD",      "EXTRACT",     "LIST",
    "UPDATE",     "REMOVES",  "CREATE",   "PRINT",       "TEST",
    "HELP",       "QUIET",    "VERBOSE",  "NOEXEC",      "FORCE",
    "TEXT",       "OLD",      "DELETE",   "IGNOREPATH",  "NOCOMPRESS", 
    "DIRECTORY",  "HEADER",
#if	defined(IDEBUG)
    "DEBUG"
#endif
};
#endif

struct RDArgs	*Args = NULL;
LONG		Result[32];

static char inpbuf[256];

long	__stack = 16 * 1024;
long	__STKNEED = 1024;

#ifdef LHEXTRACT
char	__stdiowin[] = "CON:/10/10/320/160/LHEX (c) 1993 Commodore-Amiga, Inc.";
#else
char	__stdiowin[] = "CON:/10/10/320/160/CATSLHA (c) 1993 Commodore-Amiga, Inc.";
#endif

#if	defined(BETA)
#define	BETAMSG	" BETA "
#define FREEDIS " "
#else
#define BETAMSG " "
#define FREEDIS " Freely Redistributable, "
#endif

void
main (int argc, char *argv[])
{
    int	command = 0, i;
    char	**files = NULL;
    void	interrupt(int);
    extern long	__stack;

#ifdef LHEXTRACT    
    fprintf(stderr, "LHEX"
#else
    fprintf(stderr, "CATSLHA"
#endif
	    BETAMSG
	    "%d.%d" FREEDIS "Disclaimer: All use at your own risk\n",
	    VERSION, REVISION);

#if	defined(BETA)
    fprintf(stderr, "\n");
    fprintf(stderr, "      This is for Beta testing *ONLY*!\n");
    fprintf(stderr, "          Do *NOT* redistribute.\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
	    "      Report bugs to <carolyn@cbmvax.cbm.commodore.com>\n\n");

#endif
    
    signal(SIGINT, interrupt);
    
    if (DOSBase->lib_Version >= 36)
    {
	Args = ReadArgs(TEMPLATE, Result, NULL);
    }
    else
    {
	S_T_S = 1;		/* *Wimper*    1.3, Just say no! */
	Args = _myparseargs(argc, argv, Result);
    }
    
    if (Args == NULL)
	print_help_and_exit();

    for (i = 2 /* was OPT_ADD */; i < OPT_QUIET; i++)
    {
	if ((int)Result[i] != 0)
	    if (command)
	    {
		fprintf(stderr, "You cannot use %s with %s\n", Keywords[i],
			Keywords[command - 1]);
		print_help_and_exit ();
	    }
	    else
		command = i + 1; /* +1 because Result[0] is valid */
    }

    if (command)
	command--;
    else
	print_help_and_exit ();  /* was command = OPT_LIST; */

    switch(command)
    {
#ifndef LHEXTRACT
      case OPT_ADD:
	cmd = LHCMD_ADD;
	break;
#endif
	
      case OPT_EXTRACT:
	cmd = LHCMD_EXTRACT;
	break;
	
      case OPT_LIST:
	cmd = LHCMD_LIST;
	break;

#ifndef LHEXTRACT
      case OPT_UPDATE:
	update_if_newer = TRUE;
	cmd = LHCMD_ADD;
	break;
	
      case OPT_REMOVE:
	cmd = LHCMD_DELETE;
	break;
	
      case OPT_CREATE:
	new_archive = TRUE;
	cmd = LHCMD_ADD;
	break;
#endif
	
      case OPT_PRINT:
	output_to_stdout = TRUE;
	cmd = LHCMD_EXTRACT;
	break;
	
      case OPT_TEST:
	verify_mode = TRUE;
	cmd = LHCMD_EXTRACT;
	break;
	
      case OPT_HELP:
	print_help_and_exit ();
	break;
    }

    if (Result[OPT_QUIET])
	quiet = TRUE;
    
    if (Result[OPT_VERBOSE])
    {
	if (cmd == LHCMD_LIST)
	    verbose_listing = TRUE;
	verbose = TRUE;
    }
    
    if (Result[OPT_NOEXEC])
	noexec = TRUE;

    if (Result[OPT_FORCE])
	force = TRUE;

#ifndef LHEXTRACT    
    if (Result[OPT_TEXT])
	text_mode = TRUE;

    if (Result[OPT_OLD])
    {
	compress_method = 1;
	header_level = 0;
    }

    if (Result[OPT_DELETE])
	delete_after_append = TRUE;
#endif

    if (Result[OPT_IGNOREPATH])
	ignore_directory = TRUE;

#ifndef LHEXTRACT
    if (Result[OPT_NOCOMPRESS])
	compress_method = 0;
#endif

    if (Result[OPT_DIRECTORY])
	extract_directory = (char *)Result[OPT_DIRECTORY];

#ifndef LHEXTRACT
    if (Result[OPT_HEADER])
	header_level = *((LONG *)Result[OPT_HEADER]);
#endif

    archive_name = (char *)Result[OPT_ARCHIVE];
    if ((files = (char **)Result[OPT_FILES]) == NULL)
    {
	cmd_filec = 0;
	cmd_filev = NULL;
    }
    else
    {
	for (i = 0; files[i]; i++)
	    ;
	    
	cmd_filec = i;
	cmd_filev = files;
    }
    
    sort_files ();

    switch (cmd)
    {
      case LHCMD_EXTRACT:
	cmd_extract ();
	break;

#ifndef LHEXTRACT      
      case LHCMD_ADD:
	cmd_add ();
	break;
#endif
      
      case LHCMD_LIST:
	cmd_list ();
	break;

#ifndef LHEXTRACT      
      case LHCMD_DELETE:
	cmd_delete ();
	break;
#endif
    }

    myexit (0);
}

# define	MAXOPEN	32
BPTR	openfiles[MAXOPEN];
int	maxopen = 0;

void closeatmyexit(BPTR fp)
{
    int	i;

    if(!fp)	return;
    
    if (maxopen < MAXOPEN)
    {
	openfiles[maxopen++] = fp;
    }
    else
    {
	for (i = 0; i < maxopen; i++)
	    if (openfiles[i] == NULL)
	    {
		openfiles[i] = fp;
		break;
	    }
	if (i >= maxopen)
	{
	    Close(fp);
	    myexit(15);
	}
    }
}

void clearcame(BPTR fp)
{
    int	i;

    if(!fp)	return;
    
    for (i = 0; i < maxopen; i++)
	if (openfiles[i] == fp)
	{
	    openfiles[i] = NULL;
	    break;
	}
}

void myexit(int code)
{
    int	i;
    
    for (i = 0; i < maxopen; i++)
	if (openfiles[i] != NULL)
	{
	    Close(openfiles[i]);
	}
    
    if (Args)
	if (S_T_S)
	    _myfreeargs(Args);
	else
	    FreeArgs(Args);
    exit(code);
}

STATIC void
message_1 ( const char * title, const char * subject, const char * name )
{
#ifdef LHEXTRACT
  fprintf (stderr, "LHEX: %s%s ", title, subject);
#else
  fprintf (stderr, "LHa: %s%s ", title, subject);
#endif
  fflush (stderr);

    fprintf (stderr, "%s\n", name);
}

void
message ( const char * subject, const char * name )
{
  message_1 ("", subject, name);
}

void
warning ( const char * subject, const char * name )
{
  message_1 ("Warning: ", subject, name);
}

void
error ( const char * subject, const char * msg )
{
  message_1 ("Error: ", subject, msg);
}

void
fatal_error ( const char * msg1, const char * msg2 )
{
    message_1 ("Fatal error:", msg1, msg2);

    if (remove_temporary_at_error)
	DeleteFile(temporary_name);

    myexit (1);
}

const char *writting_filename;
const char *reading_filename;

void
write_error ( void )
{
    fatal_error ("Write error (filesystem full?) writing", writting_filename);
}

void
read_error ( void )
{
    fatal_error ("Read error while reading", reading_filename);
}

void
interrupt (int signo)
{
    message ("Interrupted\n", "");

    if ( temporary_fp )
    {
	Close (temporary_fp);
	clearcame(temporary_fp);
    }
  
    DeleteFile (temporary_name);

    if (recover_archive_when_interrupt)
	Rename (backup_archive_name, archive_name);

    if (remove_extracting_file_when_interrupt)
    {
	message ("Removing", writting_filename);
	DeleteFile ((char *)writting_filename);
    }

    myexit(0);
  
}



/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

STATIC int
sort_by_ascii (char ** a, char ** b )
{
    char *p, *q;
    int c1, c2;

    p = *a, q = *b;
    if (generic_format)
    {
	do
	{
	    c1 = *(unsigned char*)p ++;
	    c2 = *(unsigned char*)q ++;
	    if (!c1 || !c2)
		break;
	    if (islower (c1))
		c1 = toupper (c1);
	    if (islower (c2))
		c2 = toupper (c2);
	}
	while (c1 == c2) ;
	return c1 - c2;
    }
    else
    {
	while (*p == *q && *p != '\0')
	    p ++, q ++;
	return *(unsigned char*)p - *(unsigned char*)q;
    }
}

STATIC void
sort_files (void)
{
    if (cmd_filec > 1)
	qsort (cmd_filev, cmd_filec, sizeof (char*), sort_by_ascii);
}

char *xmalloc (int size)
{
    char *p = (char *)malloc (size);
    if (!p)
	fatal_error ("memory exhausted", "");
    return p;
}

char *xcalloc (int num, int size)
{
    char *p = (char *)calloc (num, size);
    if (!p)
	fatal_error ("memory exhausted", "");
    return p;
}

char *xstrdup (char * s)
{
    char *p = (char *)strdup (s);

    if (!p)
	fatal_error ("memory exhausted", "");
    return p;
}

char *xrealloc (char *old, int size)
{
    char *p = (char *)realloc (old, size);
    if (!p)
	fatal_error ("memory exhausted", "");
    return p;
}

/*----------------------------------------------------------------------*/
/*				STRING POOL				*/
/*----------------------------------------------------------------------*/

/*
 * string pool :
 *	+-------------+-------------+---     ---+-------------+----------+
 *	| N A M E 1 \0| N A M E 2 \0|    ...    | N A M E n \0|	         |
 *	+-------------+-------------+---     ---+-------------+----------+
 *	^						      ^		 ^
 * buffer+0					    buffer+used  buffer+size
 */

/*
 * vector :
 *	+---------------+---------------+-------------     -------------+
 *	| pointer to	| pointer to	| pointer to   ...  pointer to	|
 *	|  string pool	|  N A M E 1	|  N A M E 2   ...   N A M E n	|
 *	+---------------+---------------+-------------     -------------+
 *	^		^
 *   malloc base      returned
 */

void
init_sp (struct string_pool *sp)
{
    sp->size = 1024 - 8;	/* any ( >=0 ) */
    sp->used = 0;
    sp->n = 0;
    sp->buffer = (char*)xmalloc (sp->size * sizeof (char));
}

void
add_sp (struct string_pool *sp, char *name, int len)
/* stored '\0' at tail */
/* include '\0' */
{
    while (sp->used + len > sp->size)
    {
	sp->size *= 2;
	sp->buffer = (char*) xrealloc (sp->buffer, sp->size * sizeof (char));
    }
    bcopy (name, sp->buffer + sp->used, len);
    sp->used += len;
    sp->n ++;
}

void
finish_sp (struct string_pool * sp, int *v_count, char *** v_vector)
{
    int i;
    register char *p;
    char **v;

    v = (char**) xmalloc ((sp->n + 1) * sizeof (char*));
    *v++ = sp->buffer;
    *v_vector = v;
    *v_count = sp->n;
    p = sp->buffer;
    for (i = sp->n; i; i --)
    {
	*v++ = p;
	if (i - 1)
	    p += strlen (p) + 1;
    }
}

void
free_sp (char ** vector)
{
    vector --;
    free (*vector);		/* free string pool */
    free (vector);
}


/*----------------------------------------------------------------------*/
/*			READ DIRECTORY FILES				*/
/*----------------------------------------------------------------------*/

STATIC boolean
include_path_p (char *path, char *name)
{
    char *n = name;
    while (*path)
	if (*path++ != *n++)
	    return (path[-1] == '/' && *n == '\0');
    return (*n == '/' || (n != name && path[-1] == '/' && n[-1] == '/'));
}

#define STREQU(a,b)	(((a)[0] == (b)[0]) ? (strcmp ((a),(b)) == 0) : FALSE)
void
cleaning_files (int *v_filec, char ***v_filev)
{
    char *flags;
    struct FileInfoBlock fib;
    register char **filev = *v_filev;
    register int filec = *v_filec;
    register char *p;
    register int i, j;
    BPTR	lock;
  
    if (filec == 0)
	return;

    flags = xmalloc (filec * sizeof (char));

    /* flags & 0x01 :	1: ignore */
    /* flags & 0x02 :	1: directory, 0 : regular file */
    /* flags & 0x04 :	1: need delete */

    for (i = 0; i < filec; i ++)
    {
	
        if ((lock = Lock(filev[i],ACCESS_READ)) == NULL
	    || Examine(lock, &fib) == FALSE)
        {
	    flags[i] = 0x04;
#ifdef LHEXTRACT
	    fprintf (stderr, "LHEX: Cannot access \"%s\", ignored.\n",
		     filev[i]);
#else
	    fprintf (stderr, "LHa: Cannot access \"%s\", ignored.\n",
		     filev[i]);
#endif
        }
        else
        {
	    UnLock(lock);
	    if (is_regularfile (&fib))
	        flags[i] = 0x00;
	    else if (is_directory (&fib))
	        flags[i] = 0x02;
	    else
	    {
	        flags[i] = 0x04;
#ifdef LHEXTRACT
	        fprintf (stderr, "LHEX: Cannot archive \"%s\", ignored.\n",
			 filev[i]);
#else
	        fprintf (stderr, "LHa: Cannot archive \"%s\", ignored.\n",
			 filev[i]);
#endif
	    }
	}
    }

    for (i = 0; i < filec; i ++)
    {
	p = filev[i];
	if ((flags[i] & 0x07) == 0x00)
	{			/* regular file, not deleted/ignored */
	    for (j = i + 1; j < filec; j ++)
	    {
		if ((flags[j] & 0x07) == 0x00)
		{		/* regular file, not deleted/ignored */
		    if (STREQU (p, filev[j]))
			flags[j] = 0x04; /* delete */
		}
	    }
	}
	else if ((flags[i] & 0x07) == 0x02)
	{			/* directory, not deleted/ignored */
	    for (j = i + 1; j < filec; j ++)
	    {
		if ((flags[j] & 0x07) == 0x00)
		{		/* regular file, not deleted/ignored */
		    if (include_path_p (p, filev[j]))
			flags[j] = 0x04; /* delete */
		}
		else if ((flags[j] & 0x07) == 0x02)
		{		/* directory, not deleted/ignored */
		    if (include_path_p (p, filev[j]))
			flags[j] = 0x04; /* delete */
		}
	    }
	}
    }

    for (i = j = 0; i < filec; i ++)
    {
	if ((flags[i] & 0x04) == 0)
	{
	    if (i != j)
		filev[j] = filev[i];
	    j ++;
	}
    }
    *v_filec = j;

    free (flags);
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

/* Build temporary file name and store to TEMPORARY_NAME */
void
build_temporary_name (void)
{
    char *p, *s;

    strcpy (temporary_name, archive_name);
    for (p = temporary_name, s = (char*)0; *p; p ++)
	if (*p == '/')
	    s = p;
    strcpy ((s ? s+1 : temporary_name), "lhXXXXXX");
}

STATIC void
modify_filename_extention (char *buffer, const char *ext)
{
    register char *p, *dot;

    for (p = buffer, dot = (char*)0; *p; p ++)
    {
	if (*p == '.')
	    dot = p;
	else if (*p == '/')
	    dot = (char*)0;
    }

    if (dot)
	p = dot;

    strcpy (p, ext);
}

/* build backup file name */
void
build_backup_name (char *buffer, char *original)
{
    strcpy (buffer, original);
    modify_filename_extention (buffer, BACKUPNAME_EXTENTION); /* ".bak" */
}

void
build_standard_archive_name (char *buffer, char *original)
{
    strcpy (buffer, original);
    modify_filename_extention (buffer, ARCHIVENAME_EXTENTION); /* ".lha" */
}

/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

boolean
need_file (char *name)
{
    int i;

    if (cmd_filec == 0)
	return TRUE;

    for (i = 0; i < cmd_filec; i ++)
    {
	if (patmatch(cmd_filev[i], name, 0 ) )
	    return TRUE;
    }

    return FALSE;
}

BPTR xfopen (char *name, int mode)
{
    BPTR	fp;

    if ((fp = Open (name, mode)) == NULL)
	fatal_error ("Could not open", name);

#ifdef	BIG_BUFFER
    SetVBuf(fp, NULL, BUF_FULL, BIG_BUFFER);
#endif

    closeatmyexit(fp);
    return fp;
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
int	archive_file_mode = AMIGA_DEF_MODE & ~AMIGA_ALL_EXECUTE;
int	archive_file_gid;

STATIC boolean
open_old_archive_1 (char *name, BPTR *v_fp)
{
    BPTR			fp;
    struct FileInfoBlock	fib;
    BPTR			lock;
  
  
    if ((lock = Lock(name, ACCESS_READ)) && Examine(lock, &fib) &&
	is_regularfile (&fib) && (fp = Open (name, MODE_OLDFILE)) != NULL)
    {
#ifdef	BIG_BUFFER
	SetVBuf(fp, NULL, BUF_FULL, BIG_BUFFER);
#endif

        *v_fp = fp;
        archive_file_gid = fib.fib_OwnerGID;
        archive_file_mode = fib.fib_Protection;
	UnLock(lock);
	closeatmyexit(fp);
        return TRUE;
    }
    
    if (lock)
        UnLock(lock);

    *v_fp = NULL;
    archive_file_gid = -1;
    return FALSE;
}

BPTR
open_old_archive (void)
{
    BPTR	fp;
    char	*p;

    if (!strcmp(archive_name, "-"))
    {
	if (cmd == LHCMD_EXTRACT || cmd == LHCMD_LIST) return Input();
	else return NULL;
    }
    if (p = (char *)strrchr(archive_name,'.'))
    {
#pragma msg 104 ignore
	if ( stricmp(".LZH",p)==0
	    || stricmp(".LZS",p)==0
	    || stricmp(".LHA",p)==0
	    || stricmp(".COM",p)==0 /* DOS SFX */
	    || stricmp(".EXE",p)==0
	    || stricmp(".X"  ,p)==0 /* HUMAN SFX */
	    || stricmp(".BAK",p)==0 ) /* for BackUp */
#pragma msg 104 warning
	{
	    open_old_archive_1 (archive_name, &fp );
	    return fp;
	}
    }

    if ( open_old_archive_1 (archive_name, &fp) )
        return fp;

    sprintf( expanded_archive_name , "%s.lha",archive_name);
    if ( open_old_archive_1 (expanded_archive_name, &fp) )
    {
	archive_name = expanded_archive_name;
	return fp;
    }
    sprintf( expanded_archive_name , "%s.lzh",archive_name);
    if ( open_old_archive_1 (expanded_archive_name, &fp) )
    {
	archive_name = expanded_archive_name;
	return fp;
    }
    sprintf( expanded_archive_name, "%s.lzs",archive_name);
    if ( open_old_archive_1 (expanded_archive_name, &fp ) )
    {
  	archive_name = expanded_archive_name;
	return fp;
    }
    return NULL;
}

int
inquire (const char *msg, const char *name, const char *selective)
{
    char buffer[10];
    const char *p;

    for (;;)
    {
	fprintf (stderr, "%s %s ", name, msg);
	fflush (stderr);

	myFGets (Input(), buffer, 1024);

	p = strchr(selective, buffer[0]);
      
	if (p)
	    return p - selective;
      
    }
    /*NOTREACHED*/
}

void
write_archive_tail (BPTR nafp)
{
    myFPutC(0x00, nafp);
}

void
copy_old_one ( BPTR oafp, BPTR nafp, LzHeader * hdr )
{
    if (noexec)
    {
	Seek (oafp, (long)(hdr->header_size + 2) + hdr->packed_size,
	      OFFSET_CURRENT);
    }
    else
    {
	reading_filename = archive_name;
	writting_filename = temporary_name;
	copyfile (oafp, nafp, hdr->total_size, 0);
    }
}

static char newname[FILENAME_LENGTH];

find_files (char *name, int *v_filec, char ***v_filev)
{
    struct string_pool sp;
    int len, n;
    BPTR	dirlock;
    BPTR	tmplock;
    BPTR	arclock;
    BPTR	fillock;
    struct FileInfoBlock dir_fib;
    boolean	ok;
    
    strcpy (newname, name);
    len = strlen (name);
    if (len > 0 && newname[len-1] != '/')
        newname[len++] = '/';

    dirlock = Lock (name, ACCESS_READ);
    if (!dirlock)
        return FALSE;

    init_sp (&sp);

    tmplock = Lock(temporary_name, ACCESS_READ);
    
    arclock = Lock(archive_name, ACCESS_READ);


    for (ok = Examine(dirlock, &dir_fib); ok; ok = ExNext(dirlock, &dir_fib))
    {
        n = strlen(dir_fib.fib_FileName);
        strncpy (newname+len, dir_fib.fib_FileName, n);
        newname[len + n] = '\0';
	if ((fillock = Lock(newname, ACCESS_READ)))
	{
	    if (fillock == arclock || fillock == tmplock)
	    {
	        UnLock(fillock);
		continue;
	    }
	    UnLock(fillock);
       	    add_sp (&sp, newname, len + n + 1);
	}
    }
    if (arclock)
        UnLock(arclock);
    if (tmplock)
        UnLock(tmplock);
    if (dirlock)
        UnLock(dirlock);

    finish_sp (&sp, v_filec, v_filev);
    if (*v_filec > 1)
        qsort (*v_filev, *v_filec, sizeof (char*), sort_by_ascii);
    cleaning_files (v_filec, v_filev);

    return TRUE;
}

void
free_files (int filec, char **filev )
{
    free_sp (filev);
}

int is_directory(struct FileInfoBlock * x)
{
    return x->fib_DirEntryType > 0;
}

int is_regularfile(struct FileInfoBlock * x)
{
    return x->fib_DirEntryType < 0;
}
