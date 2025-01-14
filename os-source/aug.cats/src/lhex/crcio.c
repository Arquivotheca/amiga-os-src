/***********************************************************
	crcio.c -- input/output
***********************************************************/
#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#endif

#include <errno.h>
#include "slidehuf.h"
#include "intrface.h"

extern int text_mode;
extern int prev_char;
extern int verify_mode;
#ifdef EUC
extern int euc_mode;
extern int generic_format;
#endif

#include "lharc.h"

long reading_size;

#define CRCPOLY  0xA001  /* CRC-16 */
#define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

FILE *infile, *outfile;
unsigned short crc, bitbuf;

static unsigned short crctable[UCHAR_MAX + 1];
static unsigned char  subbitbuf, bitcount;
#ifdef EUC
static int putc_euc_cache;
#endif
static int getc_euc_cache;

void make_crctable(void)
{
	unsigned int i, j, r;

	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i;
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 1) r = (r >> 1) ^ CRCPOLY;
			else       r >>= 1;
		crctable[i] = r;
	}
}


#ifdef NEED_INCREMENTAL_INDICATOR
extern int quiet;
extern int indicator_count;
extern int indicator_threshold;
static void
put_indicator( long int count)
{
	if (!quiet && indicator_threshold)
	{
		while ( count > indicator_count) {
			putchar ('o');
			fflush (stdout);
			indicator_count += indicator_threshold;
		}
	}
}
#endif

unsigned short calccrc( unsigned char *p, unsigned int n)
{
	reading_size += n;
#ifdef NEED_INCREMENTAL_INDICATOR
	put_indicator( reading_size );
#endif
	while (n-- > 0) UPDATE_CRC(*p++);
	return crc;
}

/* Shift bitbuf n bits left, read n bits */
void fillbuf(unsigned char n)  

{
  while (n > bitcount) {
    n -= bitcount;
    bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
    if (compsize != 0) {
      compsize--;  subbitbuf = (unsigned char) getc(infile);
    } else subbitbuf = 0;
    bitcount = CHAR_BIT;
  }
  bitcount -= n;
  bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
  subbitbuf <<= n;
}

unsigned short getbits(unsigned char n)
{
	unsigned short x;

	x = bitbuf >> (2 * CHAR_BIT - n);  fillbuf(n);
	return x;
}

/* Write rightmost n bits of x */
void putcode(unsigned char n, unsigned short x)
{
  while (n >= bitcount) {
    n -= bitcount;
    subbitbuf += x >> (USHRT_BIT - bitcount);
    x <<= bitcount;
    if (compsize < origsize) {
      if (fwrite(&subbitbuf, 1, 1, outfile) == 0)
	/*	fileerror(WTERR, outfile); */
	exit( errno );
      compsize++;
    } else unpackable = 1;
    subbitbuf = 0;  bitcount = CHAR_BIT;
  }
  subbitbuf += x >> (USHRT_BIT - bitcount);
  bitcount -= n;
}

/* Write rightmost n bits of x */
void putbits(unsigned char n, unsigned short x)
{
	x <<= USHRT_BIT - n;
	while (n >= bitcount) {
		n -= bitcount;
		subbitbuf += x >> (USHRT_BIT - bitcount);
		x <<= bitcount;
		if (compsize < origsize) {
			if (fwrite(&subbitbuf, 1, 1, outfile) == 0)
				/* fileerror(WTERR, outfile); */
				exit( errno );
			compsize++;
		} else unpackable = 1;
		subbitbuf = 0;  bitcount = CHAR_BIT;
	}
	subbitbuf += x >> (USHRT_BIT - bitcount);
	bitcount -= n;
}

int fread_crc( unsigned char *p, int n, FILE *fp)
{
	if ( text_mode )
	  n = fread_txt(p, n, fp);
	else
	  n = fread(p, 1, n, fp);
	calccrc(p, n);
	return n;
}

void fwrite_crc( unsigned char *p, int n, FILE *fp)
{
  calccrc(p,n);
  if ( verify_mode ) return;

  if ( fp )
    {
      if ( text_mode )
	{
	  if ( fwrite_txt(p , n , fp) )
	    fatal_error("File write error\n");
	}
      else
	{
	  if (fwrite(p, 1, n, fp) < n)
	    fatal_error("File write error\n");
	}
    }
}

void init_code_cache(void)	/* called from copyfile() in util.c */
{
#ifdef EUC
	putc_euc_cache = EOF;
#endif
	getc_euc_cache = EOF;
}

void init_getbits(void)
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(2 * CHAR_BIT);
#ifdef EUC
	putc_euc_cache = EOF;
#endif
}

void init_putbits(void)
{
	bitcount = CHAR_BIT;  subbitbuf = 0;
	getc_euc_cache = EOF;
}

#ifdef EUC
void putc_euc(int c, FILE *fd)
{
  int d;

  if (putc_euc_cache == EOF)
    {
      if (!euc_mode || c < 0x81 || c > 0xFC)
        {
          putc(c, fd);
          return;
        }
      if (c >= 0xA0 && c < 0xE0)
        {
          putc(0x8E, fd);      /* single shift */
          putc(c, fd);
          return;
        }
      putc_euc_cache = c;      /* save first byte */
      return;
    }
  d = putc_euc_cache;
  putc_euc_cache = EOF;
  if (d >= 0xA0)
    d -= 0xE0 - 0xA0;
  if (c > 0x9E)
    {
      c = c - 0x9F + 0x21;
      d = (d - 0x81) * 2 + 0x22;
    }
  else
    {
      if (c > 0x7E)
        c --;
      c -= 0x1F;
      d = (d - 0x81) * 2 + 0x21;
    }
  putc(0x80 | d, fd);
  putc(0x80 | c, fd);
}
#endif

int fwrite_txt( unsigned char *p, int n, FILE *fp)
{
  while ( --n>=0 )
    {
      if ( *p!='\015' && *p!='\032' )
	{
#ifdef EUC
	  putc_euc( *p , fp );
#else
	  putc( *p , fp );
#endif
	}

      prev_char = *p++;
    }
  return ( ferror( fp ) );
}

int fread_txt( unsigned char *p, int n, FILE *fp)
{
  int c;
  int cnt = 0;

  while (cnt < n)
    {
      if (getc_euc_cache != EOF)
        {
          c = getc_euc_cache;
          getc_euc_cache = EOF;
        }
      else
        {
          if ((c = fgetc(fp)) == EOF)
            break;
          if (c == '\n') {
            getc_euc_cache = c;
            c = '\r';
	  }
#ifdef EUC
	  else if (euc_mode && (c == 0x8E || 0xA0 < c && c < 0xFF))
	    {
	      int d = fgetc(fp);
	      if (d == EOF)
	        {
		  *p++ = c;
	          cnt ++;
	          break;
	        }
	      if (c == 0x8E)	/* single shift (KANA) */
	        {
		  if ((0x20 < d && d < 0x7F) || (0xA0 < d && d < 0xFF))
		    c = d | 0x80;
		  else
		    getc_euc_cache = d;
	        }
	      else
	      	{
	      	  if (0xA0 < d && d < 0xFF)	/* if GR */
	      	    {
	      	      c &= 0x7F;	/* convert to MS-kanji */
	      	      d &= 0x7F;
	      	      if (!(c & 1))
	      	        {
	      	          c --;
	      	          d += 0x7F - 0x21;
	      	        }
		      if ((d += 0x40 - 0x21) > 0x7E)
			d ++;
		      if ((c = (c >> 1) + 0x71) >= 0xA0)
		        c += 0xE0 - 0xA0;
	      	    }
		  getc_euc_cache = d;
	      	}
	    }
#endif
        }
      *p++ = c;
      cnt ++;
    }
  return cnt;
}
