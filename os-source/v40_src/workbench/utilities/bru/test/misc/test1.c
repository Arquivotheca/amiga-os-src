/*
 *	This test determines the behavior when
 *	attempting to write past end of file
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
    extern int errno;
    char *buffer;
    extern long lseek ();
    extern char *malloc ();
    long to;
    long where;
    int bytes;
    int wbytes;
    int begun;
    int fd;

    start = atoi (argv[1]);
    blkfac = atoi (argv[2]);
    buffer = malloc (blkfac * BLKSIZE);
    if (buffer == NULL) {
	perror ("can't allocate buffer");
    } else {
	if ((fd = open (argv[3], 1)) <0) {
		perror (argv[3]);
		exit (1);
	}
	to = start;
	to *= BLKSIZE;
fprintf (stderr,"seek to %ld\n", to);
	where = lseek (fd, to, 0);
fprintf (stderr,"seek complete at %ld\n", where);
	if (where != to) {
	    fprintf (stderr, "can't seek to %ld\n", to);
	    perror ("");
	} else {
	    bytes = blkfac * BLKSIZE;
	    begun = 0;
	    while ((wbytes = write (fd, buffer, bytes)) == bytes) {
		if (begun == 0) {
		    begun = 1;
		    fprintf (stderr, "first write done\n");
		}
		start += blkfac;
		fprintf (stderr, "writing...\n");
	    }
	    start += wbytes/BLKSIZE;
	    fprintf (stderr, "errno = %d ", errno);
	    perror ("found end of device");
	    fprintf (stderr, "wrote %d of %d bytes\n", wbytes, bytes);
	    fprintf (stderr, "wrote %d of %d blocks\n", wbytes/BLKSIZE,
		bytes/BLKSIZE);
	    fprintf (stderr, "total of %d blocks on device\n", start);
	}
    }
}

        
