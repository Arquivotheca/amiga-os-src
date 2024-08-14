/*
 * File handle cache data structure definitions
 */

#ifndef CLI_FH_CACHE
#define CLI_FH_CACHE

struct fh_cache {
	u_long	ca_offset;	/* position of file for first data byte */
	u_short	ca_nbytes;	/* number of bytes in cache */
	u_short	ca_nreads;	/* number of reads */
	u_short	ca_hits;	/* number of hits */
	u_char	ca_valid;	/* true if cache contains valid data */
	u_char	ca_data[1];	/* cache data (stub) array */
};

#endif CLI_FH_CACHE
