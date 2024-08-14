/*
 * util.c - part of LHa for UNIX
 *	Feb 26 1992 modified by Masaru Oki
 *	Mar  4 1992 modified by Masaru Oki
 *		#ifndef USESTRCASECMP added.
 *	Mar 31 1992 modified by Masaru Oki
 *		#ifdef NOMEMSET added.
 */

#include <stdio.h>
#include <errno.h>
#ifdef sony_news
#include <sys/param.h>
#endif
#if defined(__STDC__) || defined(NEWSOS)
#include <stdlib.h>
#endif
#include "lharc.h"

#define BUFFERSIZE 2048
#ifndef NULL
#define NULL (char *)0
#endif

extern unsigned short crc;
extern int quiet;

/* return: size of source file */
/* 0: no crc, 1: crc check, 2: extract, 3: append */
long copyfile(BPTR f1, BPTR f2, long size, int crc_flg)
{
    unsigned short xsize;
    char *buf;
    long rsize = 0;

    if ((buf = (char *)malloc(BUFFERSIZE)) == NULL)
	fatal_error("memory exhausted", "");
    crc = 0;
    if ((crc_flg == 2 || crc_flg) && text_mode)
	init_code_cache();
    while (size > 0)
    {
	/* read */
	if (crc_flg == 3 && text_mode)
	{
	    xsize = fread_txt(buf, BUFFERSIZE, f1);
	    if (xsize == 0)
		break;

	    if (xsize < 0)
	    {
		fatal_error("file read error during copy", "");
	    }
	}
	else
	{
	    xsize = (size > BUFFERSIZE) ? BUFFERSIZE : size;
	    if (myFRead(f1, buf, 1, xsize) != xsize)
	    {
		fatal_error("file read error during copy", "");
	    }
	}
	/* write */
	if (f2)
	{
	    if (crc_flg == 2 && text_mode)
	    {
		if (fwrite_txt(buf, xsize, f2))
		{
		    fatal_error("file write error during copy", "");
		}
	    }
	    else
	    {
		if (myFWrite(f2, buf, 1, xsize) != xsize)
		{
		    fatal_error("file write error during copy", "");
		}
	    }
	}
	/* calculate crc */
	if (crc_flg)
	{
	    calccrc(buf, xsize);
	}
	rsize += xsize;
	if (crc_flg != 3 || !text_mode)
	    size -= xsize;
    }
    free(buf);
    return rsize;
}

int
encode_stored_crc (BPTR ifp,  BPTR ofp, long size, long *original_size_var,
			long *write_size_var)
{
	int save_quiet;

	save_quiet = quiet;
	quiet = 1;
	size = copyfile (ifp,ofp,size,3);
	*original_size_var = *write_size_var = size;
	quiet = save_quiet;
	return crc;
}

/***************************************
	convert path delimiter
****************************************
	returns *filename
***************************************/
unsigned char *
convdelim(unsigned char *path, unsigned char delim)
{
  unsigned char c;
  unsigned char *p;

  for (p = path; (c = *p) != 0; p++) {
      if (c == '\\' || c == DELIM || c == DELIM2)
	{
	  *p = delim;
	  path = p + 1;
	}
  }
  return path;
}

/* If TRUE, archive file name is msdos SFX file name. */
boolean
archive_is_msdos_sfx1 (char *name)
{
  return FALSE;
}

static unsigned char buffer[4096];

/* skip SFX header */
boolean
skip_msdos_sfx1_code (BPTR fp)
{
    unsigned char *p, *q;
    int n;

    n = myFRead (fp, buffer, sizeof (char), 4096);


    p = buffer + 2;
    q = buffer + n - 5;
    
    while (p && p < q)
    {
	/* found "-l??-" keyword (as METHOD type string) */
	if (p[0] == '-' && p[1] == 'l' && p[4] == '-')
	{
	    /* size and checksum validate check */
	    if (p[-2] > 20 && p[-1] == calc_sum (p, p[-2]))
	    {
		Seek (fp, ((p - 2) - buffer) - n, OFFSET_CURRENT);
		return TRUE;
	    }
	}
	p = memchr(p, '-', q - p);
    }

    Seek (fp, -n, OFFSET_CURRENT);
    return FALSE;
}

