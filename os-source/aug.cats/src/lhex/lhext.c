/*----------------------------------------------------------------------*/
/*			LHarc Extract Command				*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*  V0.00  Original				1988.05.23  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V0.03  LHa for UNIX				1991.12.17  M.Oki	*/
/*----------------------------------------------------------------------*/

#include "lharc.h"

//extern int decode_lzhuf ();

static boolean
inquire_extract (char *name)
{
  struct stat stbuf;

  if (stat (name, &stbuf) >= 0)
    {
      if (!is_regularfile (&stbuf))
	{
	  error ("Already exists (not a file)", name);
	  return FALSE;
	}

      if (noexec)
	{
	  printf ("EXTRACT %s - file already exists.\n", name);
	  return FALSE;
	}
      else if (!force)
	{
	  if (!isatty(0)) {
		if(!quiet)printf("EXTRACT %s - file already exists.\n",name);
		return FALSE;
	  }
	  switch (inquire ("OverWrite ? (Yes/No/All)", name, "YyNnAa"))
	    {
	    case 0:
	    case 1:		/* Y/y */
	      break;
	    case 2:
	    case 3:		/* N/n */
	      return FALSE;
	    case 4:
	    case 5:		/* A/a */
	      force = TRUE;
	      break;
	    }
	}
    }

  if (noexec)
    printf ("EXTRACT %s\n", name);

  return TRUE;
}

static boolean
make_parent_path (char *name)
{
  char path[FILENAME_LENGTH];
  struct stat stbuf;
  register char *p;

  /* make parent directory name into PATH for recursive call */
  strcpy (path, name);
  for (p = path + strlen (path); p > path; p--)
    if (p[-1] == '/')
      {
	*--p = '\0';
	break;
      }

  if (p == path)
    {
      message ("Why?", "ROOT");
      return FALSE;			/* no more parent. */
    }

  if (stat (path, &stbuf) >= 0)
    {
      if (is_directory (&stbuf))
	return TRUE;
      error ("Not a directory", path);
      return FALSE;
    }
  errno = 0;

  if (verbose)
    printf ("Making directory \"%s\".", path);

  if (mkdir (path, 0777) >= 0)/* try */
    return TRUE;			/* successful done. */
  errno = 0;

  if (!make_parent_path (path))
    return FALSE;

  if (mkdir (path, 0777) < 0)	/* try again */
    {
      message ("Cannot make directory", path);
      return FALSE;
    }

  return TRUE;
}

static FILE *
open_with_make_path (char *name)
{
  FILE *fp;

  if ((fp = fopen (name, WRITE_BINARY)) == NULL)
    {
      errno = 0;
      if (!make_parent_path (name) ||
#ifdef AMIGA
	((name[strlen(name)-1] != '/')&&
	  (fp = fopen (name, WRITE_BINARY)) == NULL))
#else
	  (fp = fopen (name, WRITE_BINARY)) == NULL)
#endif
	  error ("Cannot extract", name);
      errno = 0;
    }
  return fp;
}

static void
adjust_info (char *name, LzHeader *hdr)
{
  time_t utimebuf[2];

  if ((hdr->extend_type == EXTEND_UNIX)
      || (hdr->extend_type == EXTEND_OS68K)
      || (hdr->extend_type == EXTEND_XOSK)) {
	/* adjust file stamp */
	utimebuf[0] = utimebuf[1] = hdr->unix_last_modified_stamp;
	utime (name, utimebuf);

#ifdef NOT_COMPATIBLE_MODE
	  Please need your modification in this space.
#else
	  chmod (name, hdr->unix_mode);
#endif

	  if(!getuid())chown (name, hdr->unix_uid, hdr->unix_gid);
	  errno = 0;
	}
#ifdef AMIGA
	// attributes are used directly as mode bits
	else {
  	    utimebuf[0] = utimebuf[1] = hdr->last_modified_stamp;
	    i_utime(name, utimebuf);
	    if(preserve_attributes)i_chmod(name,hdr->attribute);
	    else i_chmod(name,NULL);
	}
#endif
}

static char *methods[10] =
{
	"-lh0-", "-lh1-", "-lh2-", "-lh3-", "-lh4-",
	"-lh5-", "-lzs-", "-lz5-", "-lz4-", NULL
};

static void
extract_one (FILE *afp, LzHeader *hdr)
/* afp   archive file */
{
  FILE *fp;					/* output file */
  char name[257];
  int crc;
  int method;
  boolean save_quiet, save_verbose;
  char *q = hdr->name , c;
#ifdef AMIGA
  int l;
#endif

  if ( ignore_directory && rindex(hdr->name,'/') )
    {
      q = (char *)rindex(hdr->name,'/') + 1;
    }
  else
    {
      if ( *q=='/' )
	{
	  q++;
	  /*
	   *	if OSK then strip device name
	   */
	  if (hdr->extend_type == EXTEND_OS68K
	      || hdr->extend_type == EXTEND_XOSK )
		{
		  do c=(*q++); while ( c && c!='/' );
		  if ( !c || !*q ) q = ".";	/* if device name only */
		}
	}
    }

  if (extract_directory)
#ifdef AMIGA
   {
#if 0
        l = strlen(extract_directory);
	/* are there quotes ? */
	if((extract_directory[0]=='\"') && (extract_directory[l-1] == '\"')) {
	    /* yes, so zap them */
	    strcpy(name,&extract_directory[1]);
	    name[strlen(name)-1]=0;
	}
	else 
#endif
	strcpy(name,extract_directory);
	l = strlen(name);
	/* now handle the file/directory joining */
        if((name[l-1]!=':')&&(name[l-1]!='/'))strcat(name,"/");
	strcat(name,q);
   }
#else
    sprintf (name, "%s/%s", extract_directory, q);
#endif
  else
    strcpy (name, q);

  if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_REGULAR)
    {
      for (method = 0; ; method++)
	{
	  if (methods[method] == NULL)
	    {
	      error ("Unknown method skipped ...", name);
	      return;
	    }
	  if (bcmp (hdr->method, methods[method], 5) == 0)
	    break;
	}

      reading_filename = archive_name;
      writting_filename = name;
      if (output_to_stdout || verify_mode)
	{
	  if (noexec)
	    {
	      printf ("%s %s\n", verify_mode ? "VERIFY" : "EXTRACT",name);
	      if (afp == stdin)
		{
		  int i = hdr->packed_size;
		  while(i--) fgetc(afp);
		}
	      return;
	    }

	  save_quiet = quiet;
	  save_verbose = verbose;
	  if (!quiet && output_to_stdout)
	    {
	      printf ("::::::::\n%s\n::::::::\n", name);
	      quiet = TRUE;
	      verbose = FALSE;
	    }
	  else if (verify_mode)
	    {
	      quiet = FALSE;
	      verbose = TRUE;
	    }

	  crc = decode_lzhuf
	    (afp, stdout, hdr->original_size, hdr->packed_size, name, method);
	  quiet = save_quiet;
	  verbose = save_verbose;
	}
      else
	{
	  if (!inquire_extract (name))
	    return;

	  if (noexec)
	    {
	      if (afp == stdin)
		{
		  int i = hdr->packed_size;
		  while(i--) fgetc(afp);
		}
	      return;
	    }

	  signal (SIGINT, interrupt);
	  signal (SIGHUP, interrupt);

	  if(ignore_protect)chmod(name,S_IWRITE);
	  unlink (name);
	  errno = 0;
	  remove_extracting_file_when_interrupt = TRUE;
	  if ((fp = open_with_make_path (name)) != NULL)
	    {
	      crc = decode_lzhuf
		(afp, fp, hdr->original_size, hdr->packed_size, name, method);
	      fclose (fp);
	    }
	  remove_extracting_file_when_interrupt = FALSE;
	  signal (SIGINT, SIG_DFL);
	  signal (SIGHUP, SIG_DFL);

	  if (!fp)
	    return;
	}

      errno = 0;
      if (hdr->has_crc && crc != hdr->crc)
	error ("CRC error", name);

    }
  else if ((hdr->unix_mode & UNIX_FILE_TYPEMASK) == UNIX_FILE_DIRECTORY)
    {
      if (!ignore_directory && !verify_mode)
	{
	  if (noexec)
	    {
	      printf ("EXTRACT %s (directory)\n", name);
	      return;
	    }
	  /* NAME has trailing SLASH '/', (^_^) */
	  if (!output_to_stdout && !make_parent_path (name))
	    return;
	}
    }
  else
    {
      error ("Unknown information", name);
    }

  if (!output_to_stdout)
    adjust_info (name,hdr);
}


/*----------------------------------------------------------------------*/
/*			EXTRACT COMMAND MAIN				*/
/*----------------------------------------------------------------------*/

void
cmd_extract (void)
{
  LzHeader hdr;
  long pos;
  FILE *afp;

  /* open archive file */
  if ((afp = open_old_archive ()) == NULL)
    fatal_error (archive_name);
  if (archive_is_msdos_sfx1 (archive_name))
    skip_msdos_sfx1_code (afp);

  /* extract each files */
  while (get_header (afp, &hdr))
    {
      if (need_file (hdr.name))
	{
	  pos = ftell (afp);
	  extract_one (afp, &hdr);
	  fseek (afp, pos + hdr.packed_size, SEEK_SET);
	}
      else
	{
	  if (afp != stdin)
	    fseek (afp, hdr.packed_size, SEEK_CUR);
	  else
	    {
	      int i = hdr.packed_size;
	      while(i--) fgetc(afp);
	    }
	}
    }

  /* close archive file */
  fclose (afp);

  return;
}
