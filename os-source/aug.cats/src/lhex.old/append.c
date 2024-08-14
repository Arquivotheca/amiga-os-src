/***********************************************************
	append.c -- append to archive
***********************************************************/
#include "lharc.h"

#ifdef NEED_INCREMENTAL_INDICATOR
#define MAX_INDICATOR_COUNT	32
long		indicator_count;
long		indicator_threshold;
#endif

extern int quiet;
extern int compress_method;
extern long int reading_size;
extern unsigned short dicbit;

struct interfacing interface;

#ifndef LHEXTRACT
int encode_lzhuf (BPTR infp, BPTR outfp, long size, long *original_size_var,
		  long *packed_size_var, char *name, char *hdr_method)
{
	static int method = -1;

	if (method < 0) {
		method = compress_method;
		if (method > 0) method = encode_alloc(method);
	}

	interface.method = method;

	if (interface.method > 0) {
		interface.infile = infp;
		interface.outfile = outfp;
		interface.original = size;
		start_indicator (name, size, "Freezing",1<<dicbit, BOOL);
		encode(&interface);
		*packed_size_var = interface.packed;
		*original_size_var = interface.original;
	} else {
	  copyfile(infp, outfp, size, 1);
	  *packed_size_var = *original_size_var = size;
	}
	bcopy ("-lh -", hdr_method, 5);
	hdr_method[3] = interface.method + '0';

	finish_indicator2 (name, "Frozen",
			   (int)((*packed_size_var * 100L) / *original_size_var ));
	return crc;
}
#endif

/* NEW */
void
start_indicator (char *name, long size, const char *msg,
		long def_indicator_threshold, BOOL show_size)
{
#ifdef NEED_INCREMENTAL_INDICATOR
#ifdef OLD_IND_CALC
	int	m;
#endif
#endif

	if (quiet)
		return;

#ifdef NEED_INCREMENTAL_INDICATOR
#ifdef OLD_IND_CALC
	m = MAX_INDICATOR_COUNT - strlen (name);
	if (m <= 3)		/* was 0 */
		m = 3;		/* (^_^) */

	indicator_threshold =  size / m;
#else
	if (4096 > size)		indicator_threshold = size / 2;
	else 				indicator_threshold = 4096;
	if(indicator_threshold < 4)	indicator_threshold = 2;
#endif

	indicator_count = 0;
	if (show_size) 	printf ("\r% 8ld\033[4C%s%s", size,msg,name);
	else 		printf ("\r\033[12C%s%s", msg,name);
#else	
	printf (" %s%s", msg,name);
#endif
	fflush(stdout);
	reading_size = 0L;
}


void
finish_indicator2 (char *name, 	const char *msg, int pcnt)
{
	if (quiet)
		return;

	if (pcnt > 100) pcnt = 100;	/* (^_^) */
#ifdef NEED_INCREMENTAL_INDICATOR
	printf ("\r\033[12C%s%s\n",msg,name);
#else
	printf ("\r %s%s\n",msg,name);
#endif
	myFlush(Output());
}

void
finish_indicator (char *name, 	const char *msg)
{
	if (quiet)
		return;

#ifdef NEED_INCREMENTAL_INDICATOR
	printf ("\r\033[12C%s%s\n",msg,name);
#else
	printf ("\r %s%s\n",msg,name);
#endif
	myFlush(Output());
}


