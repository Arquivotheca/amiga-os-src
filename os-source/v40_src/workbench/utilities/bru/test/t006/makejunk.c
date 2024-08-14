/*
 *	Create a file full of junk given by the one and only
 *	argument.
 */

main (argc, argv)
int argc;
char *argv[];
{
	int fd;
	int count;
	char buf[1024];

	for (count = 0; count < sizeof (buf); count++) {
		buf[count] = count;
	}
	if ((fd = creat (argv[1], 0666)) == -1) {
		perror ("create failed");
		exit (1);
	} else {
		for (count = 0; count < 256; count++) {
			if (write (fd, buf, sizeof (buf)) != sizeof (buf)) {
				perror ("write failed");
				exit (1);
			}
		}
	}
	exit (0);
}
