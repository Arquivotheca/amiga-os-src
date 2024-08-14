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

#include <stdlib.h>
#include "lharc.h"

STATIC boolean
inquire_extract (char *name)
{
    struct FileInfoBlock fib;
    BPTR lock;
    
    
    if ((lock = Lock(name, ACCESS_READ)) && Examine (lock, &fib))
    {
	UnLock(lock);
	
        if (!is_regularfile (&fib))
	{
	    error ("Already exists (not a file)", name);
	    return FALSE;
	}

        if (noexec)
	{
	    printf ("EXTRACT %s but file exists.\n", name);
	    return FALSE;
	}
        else if (!force)
	{
	    /* ADD TEST !!!! default to N if stdin is NIL: */
	    switch (inquire ("OverWrite ?(Yes/No/All/Quit)", name, "YyNnAaQq"))
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
	      case 6:
	      case 7:		/* A/a */
		myexit(1);
	    }
	}
    }

    if (noexec)
        printf ("EXTRACT %s\n", name);

    return TRUE;
}

STATIC boolean
make_parent_path (char *path)
{
/*    char path[FILENAME_LENGTH]; */
    struct FileInfoBlock fib;
    char *p;
    BPTR lock;
    
    /* make parent directory name into PATH for recursive call */
/*    strcpy (path, name); */



    if ((p = strrchr(path, '/')) != NULL)
    {
	*p = '\0';
    }
    else
    {
        message ("Why?", "ROOT");
        return FALSE;			/* no more parent. */
    }


    if ((lock = Lock(path, ACCESS_READ)) != NULL)
    {
	if (Examine (lock, &fib) != 0)
	{
	    int isdir = is_directory (&fib);
	    UnLock(lock);
            if (isdir)
		return TRUE;
	    
	    error ("Not a directory", path);
	    return FALSE;
	}
	UnLock(lock);
    }


    lock = CreateDir(path);		/* try */
    
    if (lock)
    {
	if (verbose)
            printf ("Made directory \"%s\"\n", path);
	UnLock(lock);
	*p = '/';
        return TRUE;			/* successful done. */
    }


    if (!make_parent_path (path))
        return FALSE;


    lock = CreateDir(path);		/* try */
    
    if (lock)
    {
	if (verbose)
            printf ("Made directory \"%s\"\n", path);
	UnLock(lock);
	*p = '/';
        return TRUE;
    }

    message ("Cannot make directory", path);
    return FALSE;
}

STATIC BPTR open_with_make_path (char *name)
{
    BPTR	fp;
    boolean     madepath;

    if ((fp = Open (name, MODE_NEWFILE)) == NULL)
    {
      madepath = make_parent_path(name);

      if ( (!madepath) || (fp = Open (name, MODE_NEWFILE)) == NULL)
	{
#ifdef AMIGA
	if(name[strlen(name)-1] != '/')
#endif
	  error ("Cannot extract", name);
#ifdef AMIGA
	else if(!quiet) printf("\t     Created directory: %s\n",name);
#endif
	}
    }
    
#ifdef	BIG_BUFFER
    if(fp) SetVBuf(fp, NULL, BUF_FULL, BIG_BUFFER);
#endif

    closeatmyexit(fp);
    return fp;
}

STATIC void
adjust_info (char *name, LzHeader *hdr)
{
/*
kprintf("name = %s \ttype = $%lx \tmode=$%lx\n",name,hdr->extend_type,hdr->ados_mode);
*/
    if (hdr->extend_type == EXTEND_UNIX
	|| hdr->extend_type == EXTEND_AMIGADOS
	|| hdr->ados_mode /* CAS */ )
    {
	SetProtection(name, hdr->ados_mode & ~AMIGA_DIRECTORY);

    }

    if(hdr->ados_last_modified_stamp.ds_Days)
    {
	SetFileDate(name, &hdr->ados_last_modified_stamp);
    }
}

static char *methods[10] =
{
	"-lh0-", "-lh1-", "-lh2-", "-lh3-", "-lh4-",
	"-lh5-", "-lzs-", "-lz5-", "-lz4-", NULL
};

static char	name[257];

STATIC
void
extract_one (BPTR afp, LzHeader	*hdr)
{
    BPTR	fp;				/* output file */
    int		crc;
    int		method,l;
    boolean	save_quiet, save_verbose;
    char	*q = hdr->name;

    if (ignore_directory && rindex(hdr->name,'/') )
    {
        q = (char *)rindex(hdr->name,'/') + 1;
    }
    else
        if ( *q=='/' )
	    q++;

    if (extract_directory)
	{
	if(DOSBase->lib_Version >= 37)
	    {
	    strcpy(name,extract_directory);
	    AddPart(name,q,FILENAME_LENGTH);
	    }
        else 
	    {
	    l = strlen(extract_directory);
	    if((extract_directory[l-1]==':')||(extract_directory[l-1]=='/'))
		sprintf (name, "%s%s", extract_directory, q);
	    else sprintf (name, "%s/%s", extract_directory, q);
	    }
	}
    else
        strcpy (name, q);

    if ((hdr->ados_mode & AMIGA_DIRECTORY) == 0)
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
	        if (afp == Input())
		{
		    int i = hdr->packed_size;
		    while(i--)
		        myFGetC(afp);
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


	    crc = decode_lzhuf(afp, Output(), hdr->original_size, hdr->packed_size,
	    			name, method);

	    quiet = save_quiet;
	    verbose = save_verbose;
	}
        else
	{
	    if (!inquire_extract (name))
	        return;

	    if (noexec)
	    {
	        if (afp == Input())
		{
		    int i = hdr->packed_size;
		    while(i--)
		        myFGetC(afp);
		}
	        return;
	    }


	    DeleteFile(name);

	    remove_extracting_file_when_interrupt = TRUE;
	    
	    if ((fp = open_with_make_path (name)) != NULL)
	    {
	        crc = decode_lzhuf(afp, fp, hdr->original_size, hdr->packed_size,
					name, method);
	        Close (fp);

		clearcame(fp);
	    }
	    remove_extracting_file_when_interrupt = FALSE;
 
	    if (!fp)
	        return;
	}

        if (hdr->has_crc && crc != hdr->crc)
	    error ("CRC error", name);

    }
    else if ((hdr->ados_mode & AMIGA_DIRECTORY) == AMIGA_DIRECTORY)
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
/*
kprintf("should be calling adjust (o_t_s = %ld)\n",output_to_stdout);
*/
    if (!output_to_stdout)
        adjust_info (name,hdr);
}


/*----------------------------------------------------------------------*/
/*			EXTRACT COMMAND MAIN				*/
/*----------------------------------------------------------------------*/

void
cmd_extract (void)
{
    LzHeader	*hdr;
    long	pos;
    BPTR	afp;

    /* open archive file */
    if ((afp = open_old_archive ()) == NULL)
    {
        fatal_error("Cannot perform EXTRACT.  Cannot open", archive_name);
    }
	
    if (archive_is_msdos_sfx1 (archive_name))
        skip_msdos_sfx1_code (afp);

    hdr = (LzHeader *)xmalloc(sizeof(LzHeader) + MAX_NAME_LEN + 1);

    /* extract each files */
    while (get_header (afp, hdr))
    {
        if (need_file (hdr->name))
	{
	    pos = Seek(afp, 0, OFFSET_CURRENT);
	    extract_one (afp, hdr);
	    Seek (afp, pos + hdr->packed_size, OFFSET_BEGINNING);
	}
        else
	{
	    if (afp != Input())
	        Seek (afp, hdr->packed_size, OFFSET_CURRENT);
	    else
	    {
	        int i = hdr->packed_size;
	        while(i--)
		    myFGetC(afp);
	    }
	}
    }

    /* close archive file */
    Close (afp);
    clearcame(afp);
    free(hdr);
    return;
}
