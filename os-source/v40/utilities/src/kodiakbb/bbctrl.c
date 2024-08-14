#include	<stdio.h>
#include	<sys/file.h>
#include	<sys/ioctl.h>

char *bbstring[8] = {
    "clear reset\n",
    "set reset\n",
    "clear hold of BGACK\n",
    "set hold of BGACK\n",
    "clear LOCKOUT\n",
    "set LOCKOUT\n",
    "unprotect A500/A1000 billboard memory\n",
    "protect A500/A1000 billboard memory\n"
};

main(argc, argv)
int argc;
char *argv[];
{
    int d, i;
    char a, c;

    if (argc < 2) {
	fprintf(stderr, "USAGE: bbctrl [n]+\n");
	for (i = 0; i < 8; i++)
	    fprintf(stderr, "    %d -- %s", i, bbstring[i]);
	exit(1);
    }

    d = open("/dev/bb0", O_RDWR);
    if (d == -1) {
	perror("ERROR, /dev/bb0");
	exit(1);
    }

    for (i = 1; i < argc; i++) {
	if ((a = argv[i][0]) && (!argv[i][1])) {
	    c = a - '0';
	    if ((c >= 0) && (c <= 5)) {
		printf(bbstring[c]);
		if (ioctl(d, _IO(B, c)) == -1) {
		    perror("ERROR, ioctl of /dev/bb0");
		    exit(1);
		}
		continue;
	    }
	    if ((c == 6) || (c == 7)) {
		printf(bbstring[c]);
		if (lseek(d, 0xf7ffff, L_SET) == -1) {
		    perror("ERROR, lseek of /dev/bb0");
		    exit(1);
		}
		c -= 6;
		if (write(d, &c, 1) != 1) {
		    perror("ERROR, write of /dev/bb0");
		    exit(1);
		}
		continue;
	    }
	}
	fprintf(stderr, "ERROR, parameter not in range 0 to 7\n");
	exit(1);
    }
}
