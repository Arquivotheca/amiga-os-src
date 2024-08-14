/***********************************************************
	crcio.c -- input/output
***********************************************************/
#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#endif

#include <errno.h>
#include "lharc.h"
#include "slidehuf.h"
#include "intrface.h"

extern int text_mode;
extern int prev_char;
extern int verify_mode;

long reading_size;

#define CRCPOLY  0xA001  /* CRC-16 */
#define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

BPTR infile, outfile;
unsigned short crc, bitbuf;

#include	"crctab.h"

static unsigned char  subbitbuf, bitcount;


#ifdef NEED_INCREMENTAL_INDICATOR
extern int quiet;
extern long indicator_count;
extern long indicator_threshold;

STATIC void put_indicator( long count ) ;

STATIC void
put_indicator( count )
long count;
{
    if (!quiet && indicator_threshold)
    {

	while ( count > indicator_count)
	{
#ifdef OLD_BAR_INDICATOR
	    printf(SGR_REVERSE " " SGR_NONE);
#else
	    printf("\r% 9ld",count);
#endif

	    fflush (stdout);
	    indicator_count += indicator_threshold;

/* debug
	    printf("count=%ld\ticount=%ld\tithresh=%ld\n",
		count,indicator_count,indicator_threshold);
	    fflush (stdout);
	    indicator_count += indicator_threshold;
*/
	}
    }
}
#endif


unsigned short calccrc( unsigned char * p, unsigned int n )
{
    reading_size += n;


#ifdef NEED_INCREMENTAL_INDICATOR
    put_indicator( reading_size );
#endif

    while (n-- > 0)
	UPDATE_CRC(*p++);

    return crc;
}

/* Shift bitbuf n bits left, read n bits */
#ifndef	INLINED
void fillbuf( unsigned char n )
{
    while (n > bitcount)
    {
	n -= bitcount;
	bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
	if (compsize != 0)
	{
	    compsize--;
	    subbitbuf = (unsigned char) myFGetC(infile);
	}
	else
	    subbitbuf = 0;
	bitcount = CHAR_BIT;
    }
    bitcount -= n;
    bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
    subbitbuf <<= n;
}
#endif

unsigned short getbits(unsigned char n)
{
    unsigned short x;

    x = bitbuf >> (2 * CHAR_BIT - n);
    fillbuf(n);
    return x;
}

/* Write rightmost n bits of x */
void
putcode( unsigned char n,unsigned short x)
{
    while (n >= bitcount)
    {
	n -= bitcount;
	subbitbuf += x >> (USHRT_BIT - bitcount);
	x <<= bitcount;
	if (compsize < origsize)
	{
	    if (myFWrite(outfile, &subbitbuf, 1, 1) == 0)
		myexit( errno );
	    compsize++;
	}
	else
	    unpackable = 1;
	subbitbuf = 0;  bitcount = CHAR_BIT;
    }
    subbitbuf += x >> (USHRT_BIT - bitcount);
    bitcount -= n;
}

void
putbits(unsigned char n , unsigned short x )  /* Write rightmost n bits of x */
{
    x <<= USHRT_BIT - n;
    while (n >= bitcount)
    {
	n -= bitcount;
	subbitbuf += x >> (USHRT_BIT - bitcount);
	x <<= bitcount;
	if (compsize < origsize)
	{
	    if (myFWrite(outfile, &subbitbuf, 1, 1) == 0)
		myexit( errno );
	    compsize++;
	}
	else
	    unpackable = 1;
	subbitbuf = 0;  bitcount = CHAR_BIT;
    }
    subbitbuf += x >> (USHRT_BIT - bitcount);
    bitcount -= n;
}

int fread_crc(unsigned char *p ,int n , BPTR fp )
{
    if ( text_mode )
	n = fread_txt(p, n, fp);
    else
	n = myFRead(fp, p, 1, n);
    calccrc(p, n);
    return n;
}

void fwrite_crc(unsigned char *p,int n, BPTR fp)
{
    calccrc(p,n);
    if ( verify_mode ) return;

    if ( fp )
    {
	if ( text_mode )
	{
	    if ( fwrite_txt(p , n , fp) )
		fatal_error("write error (filesystem full?) while writing",
			    writting_filename);
	}
	else
	{
	    if (myFWrite(fp, p, 1, n) < n)
		fatal_error("write error (filesystem full?) while writing",
			    writting_filename);
	}
    }
}

void init_code_cache(void)	/* called from copyfile() in util.c */
{
}

void init_getbits(void)
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(2 * CHAR_BIT);
}

void init_putbits(void)
{
	bitcount = CHAR_BIT;  subbitbuf = 0;
}


int
fwrite_txt( unsigned char *p, int n, BPTR fp )
{
  int goterr = 0;
  while ( --n>=0 )
    {
      if ( *p!='\015' && *p!='\032' )
	{
	  if (myFPutC( *p , fp ) == EOF)
	      goterr++;
	}
      prev_char = *p++;
    }
  return ( goterr );/* ??? */
}

int
fread_txt(unsigned char *p, int n, BPTR fp)
{
    int c;
    int cache = EOF;
    int cnt = 0;

    while (cnt < n)
    {
	if (cache != EOF)
        {
	    c = cache;
	    cache = EOF;
        }
	else
        {
	    if ((c = myFGetC(fp)) == EOF)
		break;
	    if (c == '\n')
	    {
		cache = c;
		c = '\r';
	    }
        }
	*p++ = c;
	cnt ++;
    }
    return cnt;
}
