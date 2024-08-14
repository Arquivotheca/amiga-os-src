/*----------------------------------------------------------------------*/
/*			LHarc Add Command				*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*  V0.00  Original				1988.05.23  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V1.02  Bug fix				1990.01.19  Y.Tagawa	*/
/*  V0.03  LHa for UNIX				1991.12.05  M.Oki	*/
/*----------------------------------------------------------------------*/

#include "lharc.h"

static char new_archive_name_buffer [ FILENAME_LENGTH ];
static char *new_archive_name;
BPTR temporary_fp;

#ifndef LHEXTRACT
STATIC void remove_files (int filec, char **filev);
#endif

/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/



#ifndef LHEXTRACT
STATIC void
add_one (BPTR fp, BPTR nafp, LzHeader *hdr)
{
  long header_pos, next_pos, org_pos, data_pos;
  long v_original_size, v_packed_size;

  reading_filename = hdr->name;
  writting_filename = temporary_name;

  if (!fp && generic_format)	/* [generic] doesn't need directory info. */
    return;
  header_pos = Seek (nafp, 0, OFFSET_CURRENT);
  write_header (nafp, hdr);	/* DUMMY */

  if (hdr->original_size == 0)	/* empty file or directory */
      return;			/* previous write_header is not DUMMY. (^_^) */

  org_pos = Seek (fp, 0, OFFSET_CURRENT);
  data_pos = Seek (nafp, 0, OFFSET_CURRENT);

  hdr->crc = encode_lzhuf (fp, nafp, hdr->original_size,
			   &v_original_size, &v_packed_size, hdr->name,
			   hdr->method);

  if (v_packed_size < v_original_size)
    {
      next_pos = Seek (nafp, 0, OFFSET_CURRENT);
    }
  else
    {				/* retry by stored method */
      Seek (fp, org_pos, OFFSET_BEGINNING);
      Seek (nafp, data_pos, OFFSET_BEGINNING);
      hdr->crc = encode_stored_crc (fp, nafp, hdr->original_size,
				    &v_original_size, &v_packed_size);
      next_pos = Seek (fp, 0, OFFSET_CURRENT);
/*      ftruncate (fileno (nafp), next_pos); */
      bcopy (LZHUFF0_METHOD, hdr->method, METHOD_TYPE_STRAGE);
    }
  hdr->original_size = v_original_size;
  hdr->packed_size = v_packed_size;
  Seek (nafp, header_pos, OFFSET_BEGINNING);
  write_header (nafp, hdr);
  Seek (nafp, next_pos, OFFSET_BEGINNING);
}
#endif

int CMPDATE ( struct DateStamp a, struct DateStamp b )
{
    int   diff;
  
    diff = a.ds_Days - b.ds_Days;
  
    if (diff != 0)
        return diff;

    diff = a.ds_Minute - b.ds_Minute;

    if (diff != 0)
        return diff;
  
    diff = a.ds_Tick - b.ds_Tick;

    return diff;
}


#ifndef LHEXTRACT
BPTR
append_it (char *name, BPTR oafp, BPTR nafp)
{
    LzHeader *ahdr, *hdr;
    BPTR fp;
    long old_header;
    int cmp;
    int filec;
    char **filev;
    int i;
    struct FileInfoBlock fib;
    boolean directory;
    BPTR lock;

    if ((lock = Lock(name, ACCESS_READ)))
    {
	if (!Examine(lock, &fib))
	{
	    error ("cannot access", name); /* See cleaning_files, Why? */
	    UnLock(lock);
	    return oafp;
	}
    }
    else
    {
	error ("cannot access", name); /* See cleaning_files, Why? */
	return oafp;
    }

    ahdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);
    hdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);
    
    directory = is_directory (&fib);

    init_header (name, &fib, hdr);

    if (!directory && !noexec)
	fp = xfopen (name, MODE_OLDFILE);
    else
	fp = NULL;

    while (oafp)
    {
	old_header = Seek(oafp, 0, OFFSET_CURRENT);
	if (!get_header (oafp, ahdr))
	{
	    Close(oafp);
	    clearcame(oafp);
	    oafp = NULL;
	    break;
	}
	else
	{
	    cmp = STRING_COMPARE (ahdr->name, hdr->name);
	    if (cmp < 0)
	    {			/* SKIP */
		/* copy old to new */
		if (!noexec)
		{
		    Seek (oafp, old_header, OFFSET_BEGINNING);
		    copy_old_one (oafp, nafp, ahdr);
		}
		else
		    Seek (oafp, ahdr->packed_size, OFFSET_CURRENT);
	    }
	    else if (cmp == 0)
	    {			/* REPLACE */
		/* drop old archive's */
		Seek (oafp, ahdr->packed_size, OFFSET_CURRENT);
		break;
	    }
	    else		/* cmp > 0, INSERT */
	    {
		Seek (oafp, old_header, OFFSET_BEGINNING);
		break;
	    }
	}
    }

    if (update_if_newer)
    {
	if (!oafp ||		/* not in archive */
	    cmp > 0 ||		/*  */
	    CMPDATE(ahdr->ados_last_modified_stamp,
		    hdr->ados_last_modified_stamp) < 0)
	{
	    if (noexec)
		printf ("ADD %s\n", name);
	    else
		add_one (fp, nafp, hdr);
	}
	else			/* cmp == 0 */
	{			/* copy old to new */
	    if (!noexec)
	    {
		Seek (oafp, old_header, OFFSET_BEGINNING);
		copy_old_one (oafp, nafp, ahdr);
	    }
	}
    }
    else
    {
	if (!oafp || cmp > 0)	/* not in archive or dropped */
	{
	    if (noexec)
		printf ("ADD %s\n", name);
	    else
		add_one (fp, nafp, hdr);
	}
	else			/* cmp == 0 */	/* replace */
	{
	    if (noexec)
		printf ("REPLACE\n");
	    else
		add_one (fp, nafp, hdr);
	}
    }

    if (!directory)
    {
	if (!noexec)
	{
	    Close (fp);
	    clearcame(fp);
	}
    }
    else
    {
	/*
	**	Maybe write a record for this directory
	*/
	add_one(NULL, nafp, hdr);

	/* recurcive call */
	if (find_files (name, &filec, &filev))
	{
	    for (i = 0; i < filec; i ++)
		oafp = append_it (filev[i], oafp, nafp);
	    free_files (filec, filev);
	}
    }

    free(hdr);
    free(ahdr);
    
    return oafp;
}



static char fuf_name[FILENAME_LENGTH];

STATIC void
find_update_files (BPTR oafp)		/* old archive */
{
    struct string_pool sp;
    LzHeader *hdr;
    long pos;
    struct FileInfoBlock fib;
    int len;
    BPTR lock;
  
    pos = Seek(oafp,0,OFFSET_CURRENT);

    hdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);
    
    init_sp (&sp);
    while (get_header (oafp, hdr))
    {
	if ((hdr->ados_mode & AMIGA_DIRECTORY) != AMIGA_DIRECTORY)
 	{
	    if ((lock = Lock(hdr->name, ACCESS_READ)))
	    {
	        if (Examine(lock, &fib) >= 0) /* exist ? */
	            add_sp (&sp, hdr->name, strlen (hdr->name) + 1);
		UnLock(lock);
	    }
	}
	else if ((hdr->ados_mode & AMIGA_DIRECTORY) == AMIGA_DIRECTORY)
	{
	    strcpy (fuf_name, hdr->name);
	    len = strlen (fuf_name);
	    if (len > 0 && fuf_name[len - 1] == '/')
	        fuf_name[--len] = '\0'; /* strip tail '/' */
	    if ((lock = Lock(fuf_name, ACCESS_READ)))
	    {
	        if (Examine (lock, &fib) >= 0) /* exist ? */
	            add_sp (&sp, fuf_name, len+1);
		UnLock(lock);
	    }
	}
	Seek (oafp, hdr->packed_size, OFFSET_CURRENT);
    }

    Seek (oafp, pos, OFFSET_BEGINNING);

    finish_sp (&sp, &cmd_filec, &cmd_filev);
    free(hdr);
}

STATIC void
delete (BPTR oafp, BPTR nafp)
{
    LzHeader *ahdr;
    long old_header_pos;

    old_header_pos = Seek(oafp, 0, OFFSET_CURRENT);

    ahdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);

    while (get_header (oafp, ahdr))
    {
        if (need_file (ahdr->name))
	{			/* skip */
#if defined(IDEBUG)
	    printf("Skipping %s, %d bytes\n", ahdr->name, ahdr->total_size);
#endif
	    Seek (oafp, ahdr->packed_size, OFFSET_CURRENT);
	    if (noexec)
	        printf ("DELETE %s\n", ahdr->name);
	    else if (verbose)
	        printf ("Delete %s\n", ahdr->name);
	}
        else
	{			/* copy */
	    if (noexec)
	    {
	        Seek (oafp, ahdr->packed_size, OFFSET_CURRENT);
	    }
	    else
	    {
	        Seek (oafp, old_header_pos, OFFSET_BEGINNING);
#if defined(IDEBUG)
		printf("Copying %s, %d bytes\n", ahdr->name, ahdr->total_size);
#endif
	        copy_old_one (oafp, nafp, ahdr);
	    }
	}
        old_header_pos = (oafp);
    }
    free(ahdr);
    return;
}

#endif


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
STATIC BPTR 
build_temporary_file ( void )
{
    BPTR afp;

    build_temporary_name ();

    DeleteFile(temporary_name);

    afp = xfopen (temporary_name, MODE_NEWFILE);

    remove_temporary_at_error = TRUE;
    temporary_fp = afp;

    return afp;
}

STATIC void
build_backup_file ( void )
{
  
    build_backup_name (backup_archive_name, archive_name);
    if (!noexec)
    {
	DeleteFile(backup_archive_name);
	
	if (rename (archive_name, backup_archive_name) != 0)
	    fatal_error ("Could not make backup of", archive_name);
	recover_archive_when_interrupt = TRUE;
    }
}

STATIC void
report_archive_name_if_different (void )
{
  if (!quiet && new_archive_name == new_archive_name_buffer)
    {
      /* warning at old archive is SFX */
      printf ("New archive file is \"%s\"\n", new_archive_name);
    }
}

void
temporary_to_new_archive_file (long new_archive_size)
{
    BPTR oafp, nafp;

    oafp = xfopen (temporary_name, MODE_OLDFILE);
	
    DeleteFile(new_archive_name);
    
    nafp = xfopen (new_archive_name, MODE_NEWFILE);
    writting_filename = archive_name;
    
    reading_filename = temporary_name;
    copyfile (oafp, nafp, new_archive_size, 0);

    if (nafp != Output())
    {
	Close (nafp);
	clearcame(nafp);
    }
  
    Close (oafp);
    clearcame(oafp);

    recover_archive_when_interrupt = FALSE;
    DeleteFile (temporary_name);

    remove_temporary_at_error = FALSE;
}

STATIC void
set_archive_file_mode (void)
{
  SetProtection (new_archive_name, archive_file_mode);
}


/*----------------------------------------------------------------------*/
/*		REMOVE FILE/DIRECTORY					*/
/*----------------------------------------------------------------------*/

#ifndef LHEXTRACT
STATIC void
remove_one (char *name)
{
    struct FileInfoBlock fib;
    int filec;
    char **filev;
    BPTR lock;
  
    if ((lock = Lock(name, ACCESS_READ)) == NULL || Examine (lock, &fib) == FALSE)
    {
        warning ("Cannot access", name);
    }
    else
    {
	UnLock(lock);
	
	if (is_directory (&fib))
	{
            if (find_files (name, &filec, &filev))
	    {
	        remove_files (filec, filev);
	        free_files (filec, filev);
	    }
            else
	        warning ("Cannot open directory", name);

            if (noexec)
	        printf ("REMOVE DIRECTORY %s\n", name);
            else if (DeleteFile(name) == FALSE)
	        warning ("Cannot remove directory", name);
	    else if (verbose)
	        printf ("Removed %s.\n", name);
        }
        else if (is_regularfile (&fib))
        {
            if (noexec)
	        printf ("REMOVE FILE %s.\n", name);
            else if (DeleteFile(name) == FALSE)
	        warning ("Cannot remove", name);
            else if (verbose)
	        printf ("Removed %s.\n", name);
        }
        else
        {
            error ("Cannot remove (not a file or directory)", name);
        }
    }
}

STATIC void
remove_files (int filec, char **filev)
{
    int i;

    for (i = 0; i < filec; i++)
        remove_one (filev[i]);
}


/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/

void
cmd_add (void)
{
    LzHeader *ahdr;
    BPTR oafp, nafp;
    int i;
    long old_header;
    boolean old_archive_exist;
    long new_archive_size;

    /* exit if no operation */
    if (!update_if_newer && cmd_filec == 0)
    {
	error ("No files given in argument, do nothing.", "");
	return;
    }

    /* open old archive if exist */
    if ((oafp = open_old_archive ()) == NULL)
	old_archive_exist = FALSE;
    else
	old_archive_exist = TRUE;

    if (update_if_newer && cmd_filec == 0 && !oafp)
	fatal_error ("Cannot perform UPDATE.  Cannot open", archive_name);

    if (new_archive && old_archive_exist)
    {
	Close (oafp);
	clearcame(oafp);
	oafp = NULL;
    }

    if (oafp && archive_is_msdos_sfx1 (archive_name))
    {
	skip_msdos_sfx1_code (oafp);
	build_standard_archive_name (new_archive_name_buffer, archive_name);
	new_archive_name = new_archive_name_buffer;
    }
    else
    {
	new_archive_name = archive_name;
    }

    /* build temporary file */
    if (!noexec)
	nafp = build_temporary_file ();

    /* find needed files when automatic update */
    if (update_if_newer && cmd_filec == 0)
	find_update_files (oafp);

    /* build new archive file */
    /* cleaning arguments */
    cleaning_files (&cmd_filec, &cmd_filev);
    if (cmd_filec == 0)
    {
	if (oafp)
	{
	    Close (oafp);
	    clearcame(oafp);
	}

	if  (!noexec)
	{
	    Close (nafp);
	    clearcame(nafp);
	}
	
	return;
    }

    ahdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);

    if (old_archive_exist && !new_archive && oafp)
    {
	(void) Seek (oafp, 0, OFFSET_BEGINNING);
	
	if (get_header (oafp, ahdr))
	{
	    if (header_level > ahdr->header_level)
	    {
		char	buf[20];
		
		sprintf(buf, "Using level %d", ahdr->header_level);
		
		message(buf, "header for compatibility with existing archive");
		header_level = ahdr->header_level;
		
	    }
	}
	(void) Seek (oafp, 0, OFFSET_BEGINNING);
    }
  
    for (i = 0; i < cmd_filec; i ++)
	oafp = append_it (cmd_filev[i], oafp, nafp);

    if (oafp)
    {
	old_header = Seek (oafp, 0, OFFSET_CURRENT);
	while (get_header (oafp, ahdr))
	{
	    if (noexec)
		Seek (oafp, ahdr->packed_size, OFFSET_CURRENT);
	    else
	    {
		Seek (oafp, old_header, OFFSET_BEGINNING);
		copy_old_one (oafp, nafp, ahdr);
	    }
	    old_header = Seek (oafp, 0, OFFSET_CURRENT);
	}
	Close (oafp);
	clearcame(oafp);
    }
    if (!noexec)
    {
	write_archive_tail (nafp);
	new_archive_size = Seek (nafp, 0, OFFSET_CURRENT);
	Close (nafp);
	clearcame(nafp);
    }

    /* build backup archive file */
    if (old_archive_exist)
	build_backup_file ();

    report_archive_name_if_different ();

    /* copy temporary file to new archive file */
    if (!noexec && (!strcmp(new_archive_name, "-") ||
		    rename (temporary_name, new_archive_name) != 0))
	temporary_to_new_archive_file (new_archive_size);

    /* set new archive file mode/group */
    set_archive_file_mode ();

    /* remove archived files */
    if (delete_after_append)
	remove_files (cmd_filec, cmd_filev);

    free(ahdr);
    return;
}


void
cmd_delete (void)
{
  BPTR oafp, nafp;
  long new_archive_size;

  /* open old archive if exist */
  if ((oafp = open_old_archive ()) == NULL)
    fatal_error ("Cannot perform REMOVE.  Cannot open", archive_name);

  /* exit if no operation */
  if (cmd_filec == 0)
    {
      Close (oafp);
      clearcame(oafp);
      warning ("No files given in argument, do nothing.", "");
      return;
    }

  if (archive_is_msdos_sfx1 (archive_name))
    {
      skip_msdos_sfx1_code (oafp);
      build_standard_archive_name (new_archive_name_buffer, archive_name);
      new_archive_name = new_archive_name_buffer;
    }
  else
    {
      new_archive_name = archive_name;
    }

  /* build temporary file */
  if (!noexec)
    nafp = build_temporary_file ();

  /* build new archive file */
  delete (oafp, nafp);
  Close (oafp);
  clearcame(oafp);

  if (!noexec)
    {
      write_archive_tail (nafp);
      new_archive_size = Seek (nafp, 0, OFFSET_CURRENT);
      Close (nafp);
      clearcame(nafp);
    }

  /* build backup archive file */
  build_backup_file ();

  report_archive_name_if_different ();

  /* copy temporary file to new archive file */
  if (!noexec && rename (temporary_name, new_archive_name) != 0)
    temporary_to_new_archive_file (new_archive_size);

  /* set new archive file mode/group */
  set_archive_file_mode ();

  return;
}
#endif
