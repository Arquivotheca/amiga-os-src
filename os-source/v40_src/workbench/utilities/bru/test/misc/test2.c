/*
 *	This test determines the behavior when
 *	attempting to read past end of file
 *	on device.
 *
 *		test1 start blkfac /dev/???
 *
 */

#include <stdio.h>
#include <rmt.h>

static int start;
static int blkfac;

#define BLKSIZE 512

main (argc, argv)
int argc;
char *argv[];
{
    char *buffer;
    extern long lseek ();
    extern char *malloc ();
    long to;
    long where;
    int bytes;
    int rbytes;
    int begun;
    int fd;
    extern int errno;

    start = atoi (argv[1]);
    blkfac = atoi (argv[2]);
    buffer = malloc (blkfac * BLKSIZE);
    if (buffer == NULL) {
	perror ("can't allocate buffer");
    } else {
	if ((fd = open (argv[3], 0)) < 0) {
		perror (argv[3]);
		exit (1);
	}
	to = start;
	to *= BLKSIZE;
fprintf (stderr, "seek to %ld\n", to);
	where = lseek (fd, to, 0);
fprintf (stderr, "seek complete at %ld\n", where);
	if (where != to) {
	    fprintf (stderr, "can't seek to %ld\n", to);
	    perror ("");
	} else {
	    bytes = blkfac * BLKSIZE;
	    begun = 0;
	    while ((rbytes = read (fd, buffer, bytes)) == bytes) {
		if (begun == 0) {
			fprintf (stderr,"first read done\n");
			begun = 1;
		}	
		start += blkfac;
		fprintf (stderr, "reading...\n");
	    }
	    start += rbytes/BLKSIZE;
	    fprintf (stderr, "errno = %d ", errno);
	    perror ("found end of device");
	    fprintf (stderr, "read %d of %d bytes\n", rbytes, bytes);
	    fprintf (stderr, "read %d of %d blocks\n", rbytes/BLKSIZE,
		bytes/BLKSIZE);
	    fprintf (stderr, "total of %d blocks on device\n", start);
	}
    }
}

        
