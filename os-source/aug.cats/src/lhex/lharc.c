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
/*----------------------------------------------------------------------*/

#include "lharc.h"

#ifdef LHEXTRACT
#include "lhex_rev.h"
#define PROGRAM_NAME	"LHex"
#else 
#define PROGRAM_NAME	"LHa"
#endif

/*----------------------------------------------------------------------*/
/*				PROGRAM 				*/
/*----------------------------------------------------------------------*/


#define CMD_UNKNOWN	0
#define CMD_EXTRACT	1
#define CMD_ADD		2
#define CMD_LIST	3
#define CMD_DELETE	4


static int	cmd = CMD_UNKNOWN;
char	**cmd_filev;
int	cmd_filec;

char	*archive_name;
char	expanded_archive_name[FILENAME_LENGTH];
char	temporary_name[FILENAME_LENGTH];
char	backup_archive_name[FILENAME_LENGTH];

/* static functions */
static void sort_files(void);

/* options */
boolean	quiet = FALSE;
boolean	text_mode = FALSE;
boolean	verbose = FALSE;
boolean	noexec = FALSE;	/* debugging option */
boolean	force = FALSE;
boolean	prof = FALSE;
boolean preserve_attributes = FALSE;
boolean ignore_protect = FALSE;

int compress_method = 5;	/* deafult -lh5- */
int header_level = HEADER_LEVEL1;
#ifdef EUC
boolean euc_mode = FALSE;
#endif

/* view command flags */
boolean	verbose_listing = FALSE;
boolean quiet_listing = FALSE;

/* extract command flags */
boolean	output_to_stdout = FALSE;

/* append command flags */
boolean	new_archive = FALSE;
boolean	update_if_newer = FALSE;
boolean	delete_after_append = FALSE;
boolean	generic_format = FALSE;

boolean	remove_temporary_at_error = FALSE;
boolean	recover_archive_when_interrupt = FALSE;
boolean	remove_extracting_file_when_interrupt = FALSE;
boolean get_filename_from_stdin = FALSE;
boolean ignore_directory = FALSE;
boolean verify_mode = FALSE;

char *extract_directory = NULL;
char **xfilev;
int  xfilec = 257;

/*----------------------------------------------------------------------*/
/* NOTES :		Text File Format				*/
/*	GENERATOR		NewLine					*/
/*	[generic]		0D 0A					*/
/*	[MS-DOS]		0D 0A					*/
/*	[OS9][MacOS]		0D					*/
/*	[UNIX]			0A					*/
/*----------------------------------------------------------------------*/


static void
print_tiny_usage_and_exit (void)
{
#ifndef LHEXTRACT
  fprintf (stderr, "\
LHarc    for UNIX  V 1.02  Copyright(C) 1989  Y.Tagawa\n\
LHx      for MSDOS V C2.01 Copyright(C) 1990  H.Yoshizaki\n\
LHx(arc) for OSK   V 2.01  Modified     1990  Momozou\n\
LHa      for UNIX  V 1.00  Copyright(C) 1992  Masaru Oki\n\
");
  fprintf(stderr, "\
usage: lha [-][qvnfodizg012F][w=<dir>] {axel[q]v[q]udmcpt} archive_file [file...]\n\
commands:                           options:\n\
 a   Add(or replace) to archive      q  quiet\n\
 x,e EXtract from archive            v  verbose\n\
 l,v[q] List / Verbose List          n  not execute\n\
 u   Update newer files to archive   f  force (overwrite at extract)\n\
 d   Delete from archive             t  FILES are TEXT file\n\
 m   Move to archive (means 'ad')    0  use LHarc compatible method (a/u)\n\
 c   re-Construct new archive        a  preserve file attributes\n\
 p   Print to STDOUT from archive    d  delete FILES after (a/u/c)\n\
 t   Test file CRC in archive        i  ignore directory path (x/e)\n\
                                     z  files not compress (a/u)\n\
                                     g  [Generic] format (for compatibility)\n\
                                     0/1/2 header level (a/u)\n\
				     f  force (overwrite files without asking)\n\
				     F  Force (overwrite files without asking,\
						even if delete/write protected)\n\
                                     w=<dir> specify extract directory (x/e)\n\

 Input Redirection: LHEX <filelist   read file list from filelist\n\n\
");
#ifdef EUC
  fprintf (stderr, "\
                                     e  TEXT code convert from/to EUC\n\
");
#endif

#else /* LHEXTRACT */

  fprintf(stderr,"\
LHEX     for Amiga V %ld.%ld  LHA/LZH Extraction Utility\n",VERSION,REVISION);

  fprintf (stderr, "\n\
Based on Freely Redistributable V 1.00 Unix sources attributed to:\n\
LHarc    for UNIX  V 1.02  Copyright(C) 1989  Y.Tagawa\n\
LHx      for MSDOS V C2.01 Copyright(C) 1990  H.Yoshizaki\n\
LHx(arc) for OSK   V 2.01  Modified     1990  Momozou\n\
LHa      for UNIX  V 1.00  Copyright(C) 1992  Masaru Oki\n\n");

  fprintf(stderr, "\
usage: lhex [-qvnfiaFw=<destdir>] {xel[q]v[q]pt} archive_file [file...]\n\
commands (use one):                 -options: (may combine)\n\
 x,e  EXtract from archive           q  quiet\n\
 l[q] List                           v  verbose\n\
 v[q] Verbose List                   n  not execute\n\
 p    Print to STDOUT from archive   a  preserve file attributes\n\
 t    Test file CRC in archive       i  ignore directory path (x/e)\n\
				     f  force (overwrite files without asking)\n\
				     F  Force (overwrite files without asking,\
						even if delete/write protected)\n\
                                     w=<dir> specify extract directory (x/e)\n\
 Input Redirection: LHEX <filelist   read file list from filelist\n\n\
");
#ifdef EUC
  fprintf (stderr, "\
                                     e  TEXT code convert from/to EUC\n\
");
#endif
#endif /* LHEXTRACT */

  exit (1);
}

void
main (int argc, char *argv[])
{
  char *p , inpbuf[256];
int itest;

  if ((argc < 2) || ((argv[1][0]=='?')&&(argc==2)))print_tiny_usage_and_exit ();
  if (argc < 3) {
    cmd = CMD_LIST;
    argv--;
    argc++;
    goto work;
  }

// for(itest=0; itest<argc; itest++)printf("arg #%d is [%s]\n",itest,argv[itest]);


#ifndef LHEXTRACT	/* normal LHA */
  if ( argv[1][0]=='-' ) argv[1]++;
  /* commands */
  switch ( argv[1][0] )
    {
    case 'x':
    case 'e':
      cmd = CMD_EXTRACT;
      break;

    case 'p':
      output_to_stdout = TRUE;
      cmd = CMD_EXTRACT;
      break;

    case 'c':
      new_archive = TRUE;
      cmd = CMD_ADD;
      break;

    case 'a':
      cmd = CMD_ADD;
      break;

    case 'd':
      cmd = CMD_DELETE;
      break;

    case 'u':
      update_if_newer = TRUE;
      cmd = CMD_ADD;
      break;

    case 'm':
      delete_after_append = TRUE;
      cmd = CMD_ADD;
      break;

    case 'v':
      verbose_listing = TRUE;
      cmd = CMD_LIST;
      break;

    case 'l':
      cmd = CMD_LIST;
      break;

    case 't':
      cmd = CMD_EXTRACT;
      verify_mode = TRUE;
      break;

    default:
      print_tiny_usage_and_exit ();
    
    }

  /* options */
  p = &argv[1][1];
  for (p = &argv[1][1]; *p; )
    {
      switch ( (*p++) )
	{
		case 'q':	quiet = TRUE; break;
		case 'f':	force = TRUE; break;
		case 'p':	prof = TRUE; break;
		case 'v':	verbose = TRUE; break;
		case 't':	text_mode = TRUE; break;
#ifdef EUC
		case 'e':	text_mode = TRUE; euc_mode = TRUE; break;
#endif
		case 'n':	noexec = TRUE; break;
		case 'g':	generic_format = TRUE; header_level = 0; break;
		case 'd':	delete_after_append = TRUE; break;
		case '0':	compress_method = 1; header_level = 0; break;
		case 'z':	compress_method = 0; break;
		case 'i':	ignore_directory = TRUE; break;
		case 'w':	if ( *p=='=' ) p++;
	    	        extract_directory=p;
	        	    while (*p) p++;
	            	break;
		case '0':	header_level = HEADER_LEVEL0; break;
		case '1':	header_level = HEADER_LEVEL1; break;
		case '2':	header_level = HEADER_LEVEL2; break;
	    	case 'a':	preserve_attributes = TRUE; break;
		case 'F':	ignore_protect = TRUE; break;
		default:
		  fprintf(stderr, PROGRAM_NAME ": Unknown option '%c'.\n", p[-1]);
		  exit(1);
		}
    }

#else /* LHEXTRACT - with more reasonable -options then command */

    /* options */
    while(*argv[1] == '-')
	{
  	p = &argv[1][1];
  	for (p = &argv[1][1]; *p; )
    	    {
      	    switch ( (*p++) )
		{
		case 'q':	quiet = TRUE; break;
		case 'f':	force = TRUE; break;
		case 'v':	verbose = TRUE; break;
		case 'n':	noexec = TRUE; break;
		case 'i':	ignore_directory = TRUE; break;
		case 'w':	if ( *p=='=' ) p++;
			/* did they leave a space between -w and value ? */
			if(! *p) {
			    argv++;
			    argc--;
			    p = &argv[1][0];		
			    if(! *p) break;
			}
	    	        extract_directory=p;
	        	    while (*p) p++;
	            	break;
	    	case 'a':	preserve_attributes = TRUE; break;
		case 'F':	ignore_protect = TRUE; break;
		default:
		  fprintf(stderr, PROGRAM_NAME ": Unknown option '%c'.\n", p[-1]);
		  exit(1);
		}
	    }
	argv++;
	argc--;
	}

  /* commands */
  switch ( argv[1][0] )
    {
    case 'x':
    case 'e':
      cmd = CMD_EXTRACT;
      break;

    case 'p':
      output_to_stdout = TRUE;
      cmd = CMD_EXTRACT;
      break;

    case 'v':
      verbose_listing = TRUE;
      cmd = CMD_LIST;
      if(argv[1][1]=='q')quiet_listing = TRUE;
      break;

    case 'l':
      cmd = CMD_LIST;
      if(argv[1][1]=='q')quiet_listing = TRUE;
      break;

    case 't':
      cmd = CMD_EXTRACT;
      verify_mode = TRUE;
      break;

    default:
      print_tiny_usage_and_exit ();
    
    }

#endif /*LHEXTRACT */

work:
  /* archive file name */
  archive_name = argv[2];

  if (!strcmp(archive_name, "-"))
    {
      if (!isatty(1) && cmd == CMD_ADD) quiet = TRUE;
    }
  else
    {
      if (argc == 3 && !isatty(0))
	get_filename_from_stdin = TRUE;
    }

  /* target file name */
  if ( get_filename_from_stdin )
  {
  	cmd_filec = 0;
	if ( (xfilev = (char **)malloc(sizeof(char *) * xfilec)) == NULL)
		fatal_error("Virtual memory exhausted\n");
 	while ( gets( inpbuf ) )
 	{
		if ( cmd_filec >= xfilec )
		{
			xfilec += 256;
			cmd_filev = (char **)realloc(xfilev,
						  sizeof(char *) * xfilec);
			if ( cmd_filev == NULL )
				fatal_error("Virtual memory exhausted\n");
			xfilev = cmd_filev;
		}				
 		if ( strlen( inpbuf )<1 ) continue;
 		if ( (xfilev[cmd_filec++]=(char *)strdup(inpbuf))==NULL )
 			fatal_error("Virtual memory exhausted\n");
 	}
 	xfilev[cmd_filec] = NULL;
 	cmd_filev = xfilev;
  }
  else
  {
	cmd_filec = argc - 3;
	cmd_filev = argv + 3;
  }
  sort_files ();

  /* make crc table */
  make_crctable();

  switch (cmd)
    {
    case CMD_EXTRACT:	cmd_extract ();	break;
#ifndef LHEXTRACT
    case CMD_ADD:	cmd_add ();	break;
#endif /* LHEXTRACT */
    case CMD_LIST:	cmd_list ();	break;
#ifndef LHEXTRACT
    case CMD_DELETE:	cmd_delete ();	break;
#endif /* LHEXTRACT */
    }

#ifdef USE_PROF
  if (!prof)
    exit (0);
#endif

#ifdef AMIGA
#ifdef FASTFILE
  fcleanup();
#endif
#endif

  exit (0);
}

static void
message_1 (char *title, char *subject, char *name)
{
  fprintf (stderr, PROGRAM_NAME ": %s%s ", title, subject);
  fflush (stderr);

  if (errno == 0)
    fprintf (stderr, "%s\n", name);
  else
    perror (name);
}

void
message (char *subject, char *name)
{
  message_1 ("", subject, name);
}

void
warning (char *subject, char *name)
{
  message_1 ("Warning: ", subject, name);
}

void
error (char *subject, char *msg)
{
  message_1 ("Error: ", subject, msg);
}

void
fatal_error (char *msg)
{
  message_1 ("Fatal error:", "", msg);

  if (remove_temporary_at_error)
    unlink (temporary_name);

#ifdef AMIGA
#ifdef FASTFILE
  fcleanup();
#endif
#endif

  exit (1);
}

char *writting_filename;
char *reading_filename;

void
write_error (void)
{
  fatal_error (writting_filename);
}

void
read_error (void)
{
  fatal_error (reading_filename);
}

void
interrupt (int signo)
{
  errno = 0;
  message ("Interrupted\n", "");

  if ( temporary_fp ) fclose (temporary_fp);
  unlink (temporary_name);
  if (recover_archive_when_interrupt)
    i_rename (backup_archive_name, archive_name);
  if (remove_extracting_file_when_interrupt)
    {
      errno = 0;
      message ("Removing", writting_filename);
      unlink (writting_filename);
    }
  signal (SIGINT, SIG_DFL);
  signal (SIGHUP, SIG_DFL);
  kill (getpid (), signo);
}



/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

static int
sort_by_ascii (char **a, char **b)
{
  register char *p, *q;
  register int c1, c2;

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

static void
sort_files (void)
{
  if (cmd_filec > 1)
    qsort (cmd_filev, cmd_filec, sizeof (char*), sort_by_ascii);
}

char *xmalloc (int size)
{
  char *p = (char *)malloc (size);
  if (!p)
    fatal_error ("Not enough memory");
  return p;
}

char *xrealloc (char *old, int size)
{
  char *p = (char *)realloc (old, size);
  if (!p)
    fatal_error ("Not enough memory");
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
  sp->size = 1024 - 8;		/* any ( >=0 ) */
  sp->used = 0;
  sp->n = 0;
  sp->buffer = (char*)xmalloc (sp->size * sizeof (char));
}

void
add_sp (struct string_pool *sp, char *name, int len)
	/* name: stored '\0' at tail */
	/* len :include '\0' */
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
finish_sp (register struct string_pool *sp, int *v_count, char ***v_vector)
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
free_sp (char **vector)
{
  vector --;
  free (*vector);		/* free string pool */
  free (vector);
}


/*----------------------------------------------------------------------*/
/*			READ DIRECTORY FILES				*/
/*----------------------------------------------------------------------*/

static boolean
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
  struct stat stbuf;
  register char **filev = *v_filev;
  register int filec = *v_filec;
  register char *p;
  register int i, j;

  if (filec == 0)
    return;

  flags = xmalloc (filec * sizeof (char));

  /* flags & 0x01 :	1: ignore */
  /* flags & 0x02 :	1: directory, 0 : regular file */
  /* flags & 0x04 :	1: need delete */

  for (i = 0; i < filec; i ++)
    if (stat (filev[i], &stbuf) < 0)
      {
	flags[i] = 0x04;
	fprintf (stderr,
		 PROGRAM_NAME ": Cannot access \"%s\", ignored.\n", filev[i]);
      }
    else
      {
	if (is_regularfile (&stbuf))
	  flags[i] = 0x00;
	else if (is_directory (&stbuf))
	  flags[i] = 0x02;
	else
	  {
	    flags[i] = 0x04;
	    fprintf (stderr,
		     PROGRAM_NAME ": Cannot archive \"%s\", ignored.\n", filev[i]);
	  }
      }
  errno = 0;

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

#ifdef NODIRECTORY
/* please need your imprementation */
boolean
find_files (char *name, int *v_filec, char ***v_filev)
{
  return FALSE;			/* DUMMY */
}

void
free_files (int filec, char **filev)
{
  /* do nothing */
}
#else
boolean 
find_files (char *name, int *v_filec, char ***v_filev)
{
  struct string_pool sp;
  char newname[FILENAME_LENGTH];
  int len, n;
  DIR *dirp;
  DIRENTRY *dp;
  struct stat tmp_stbuf, arc_stbuf, fil_stbuf;

  strcpy (newname, name);
  len = strlen (name);
  if (len > 0 && newname[len-1] != '/')
    newname[len++] = '/';

  dirp = opendir (name);
  if (!dirp)
    return FALSE;

  init_sp (&sp);

  GETSTAT(temporary_name, &tmp_stbuf);
  GETSTAT(archive_name, &arc_stbuf);

  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
    {
      n = NAMLEN (dp);
      strncpy (newname+len, dp->d_name, n);
      newname[len + n] = '\0';
      if (GETSTAT(newname, &fil_stbuf) < 0) continue;
      if ((dp->d_ino != 0) &&
	  /* exclude '.' and '..' */
	  ((dp->d_name[0] != '.') ||
	   ((n != 1) &&
	    ((dp->d_name[1] != '.') ||
	     (n != 2)))) &&
	  ((tmp_stbuf.st_dev != fil_stbuf.st_dev ||
	    tmp_stbuf.st_ino != fil_stbuf.st_ino) &&
	   (arc_stbuf.st_dev != fil_stbuf.st_dev ||
	    arc_stbuf.st_ino != fil_stbuf.st_ino)))

	{
	  add_sp (&sp, newname, len + n + 1);
	}
    }
  closedir (dirp);
  finish_sp (&sp, v_filec, v_filev);
  if (*v_filec > 1)
    qsort (*v_filev, *v_filec, sizeof (char*), sort_by_ascii);
  cleaning_files (v_filec, v_filev);

  return TRUE;
}

void
free_files (int filec, char **filev)
{
  free_sp (filev);
}
#endif

/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

/* Build temporary file name and store to TEMPORARY_NAME */
void
build_temporary_name (void)
{
#ifdef TMP_FILENAME_TEMPLATE
  /* "/tmp/lhXXXXXX" etc. */
  strcpy (temporary_name, TMP_FILENAME_TEMPLATE);
  mktemp (temporary_name);
#else
  char *p, *s;

  strcpy (temporary_name, archive_name);
  for (p = temporary_name, s = (char*)0; *p; p ++)
    if (*p == '/')
      s = p;
  strcpy ((s ? s+1 : temporary_name), "lhXXXXXX");
  mktemp (temporary_name);
#endif
}

static void
modify_filename_extention (char *buffer, char *ext)
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
build_standard_archive_name (char *buffer, char *orginal)
{
  strcpy (buffer, orginal);
  modify_filename_extention (buffer, ARCHIVENAME_EXTENTION); /* ".lzh" */
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

FILE *
xfopen (char *name, char *mode)
{
  FILE *fp;

  if ((fp = fopen (name, mode)) == NULL)
    fatal_error (name);

  return fp;
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
int	archive_file_mode;
int	archive_file_gid;

static boolean
open_old_archive_1 (char *name, FILE **v_fp)
{
  FILE *fp;
  struct stat stbuf;

  if (stat (name, &stbuf) >= 0 &&
      is_regularfile (&stbuf) &&
      (fp = fopen (name, READ_BINARY)) != NULL)
    {
      *v_fp = fp;
      archive_file_gid = stbuf.st_gid;
      archive_file_mode = stbuf.st_mode;
      return TRUE;
    }

  *v_fp = NULL;
  archive_file_gid = -1;
  return FALSE;
}

FILE *
open_old_archive (void)
{
  FILE *fp;
  char *p;

  if (!strcmp(archive_name, "-"))
    {
      if (cmd == CMD_EXTRACT || cmd == CMD_LIST) return stdin;
      else return NULL;
    }
  if (p = (char *)rindex(archive_name,'.'))
	{
	  if ( strucmp(".LZH",p)==0
	  	|| strucmp(".LZS",p)==0
	  	|| strucmp(".COM",p)==0		/* DOS SFX */
	  	|| strucmp(".EXE",p)==0
	        || strucmp(".X"  ,p)==0		/* HUMAN SFX */
	  	|| strucmp(".BAK",p)==0 )	/* for BackUp */
	  {
		open_old_archive_1 (archive_name, &fp );
		return fp;
	  }
	}

  if ( open_old_archive_1 (archive_name, &fp) )
        return fp;
  sprintf( expanded_archive_name , "%s.lzh",archive_name);
  if ( open_old_archive_1 (expanded_archive_name, &fp) )
  {
	archive_name = expanded_archive_name;
	return fp;
  }
/*  if ( (errno&0xffff)!=E_PNNF )
  {
  	archive_name = expanded_archive_name;
  	return NULL;
  }
*/
  sprintf( expanded_archive_name, "%s.lzs",archive_name);
  if ( open_old_archive_1 (expanded_archive_name, &fp ) )
  {
  	archive_name = expanded_archive_name;
	return fp;
  }
/*  if ( (errno&0xffff)!=E_PNNF )
  {
  	archive_name = expanded_archive_name;
    return NULL;
  }
*/
/*  sprintf( expanded_archive_name , "%s.lzh",archive_name);
  archive_name = expanded_archive_name;
*/
  return NULL;
}

int
inquire (char *msg, char *name, char *selective)
{
  char buffer[1024];
  char *p;

  for (;;)
    {
      fprintf (stderr, "%s %s ", name, msg);
      fflush (stderr);

      fgets (buffer, 1024, stdin);

      for (p = selective; *p; p++)
	if (buffer[0] == *p)
	  return p - selective;
    }
  /*NOTREACHED*/
}

void
write_archive_tail (FILE *nafp)
{
  putc (0x00, nafp);
}

void
copy_old_one (FILE *oafp, FILE *nafp, LzHeader *hdr)
{
  if (noexec)
    {
      fseek (oafp, (long)(hdr->header_size + 2) + hdr->packed_size, SEEK_CUR);
    }
  else
    {
      reading_filename = archive_name;
      writting_filename = temporary_name;
      copyfile (oafp, nafp, (long)(hdr->header_size + 2) + hdr->packed_size,0);
    }
}
