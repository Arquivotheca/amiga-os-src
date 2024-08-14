/*
 *	This program is used to test the "file data changed" 
 *	detection mechanism in bru.
 *
 *	It hangs in a tight loop, opening the file named as the
 *	one and only argument, writing some junk to the beginning
 *	of the file, and then closing the file and starting over
 *	again.  When the file open fails, it silently exits.
 *
 *	To use it, start it running on a reasonably sized junk file
 *	(several kilobytes for example) and then use bru to archive
 *	the junk file.  You should get a warning about the contents
 *	changed during archiving.  You make it go away by removing
 *	the junk file.
 *
 *	Note that if the file takes less than one second for bru to
 *	open, read, and archive, then bru can miss noting that the
 *	contents have changed.
 */

main (argc, argv)
int argc;
char *argv[];
{
	int fd;
	auto char buf[32];

	for (;;) {
		if ((fd = open (argv[1], 1)) < 0) {
			exit (0);
		} else {
			write (fd, "junk", 4);
			close (fd);
		}
	}	
}
