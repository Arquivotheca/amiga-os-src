/*
 *	This program is used to test the "file data changed" 
 *	detection mechanism in bru.  It opens the file named
 *	as the one and only argument and then hangs in a tight
 *	loop seeking to the beginning of the file and writing
 *	random data there, overwriting previous data.  It
 *	can only be stopped by killing it.
 *
 *	To use it, start it running on a reasonably sized junk file
 *	(several kilobytes for example) and then use bru to archive
 *	the junk file.  You should get a warning about the contents
 *	changed during archiving.
 *
 *	Note that if the file takes less than one second for bru to
 *	open, read, and archive, then bru can miss noting that the
 *	contents have changed.
 */

#include <stdio.h>

main (argc, argv)
int argc;
char *argv[];
{
	int fd;
	auto char buf[32];

	if (argc != 2) {
		fprintf (stderr, "usage: change <filename>\n");
	} else if ((fd = open (argv[1], 1)) < 0) {
		perror ("can't open file");
		exit (1);
	} else {
		for (;;) {
			lseek (fd, 0L, 0);
			sprintf (buf, "%d", rand ());
			write (fd, buf, strlen (buf));
		}
	}	
}
