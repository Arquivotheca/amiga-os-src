/*
 *	pokebb <address> <byte-value>*
 */
#include	<stdio.h>

long
getnum(arg)
char *arg;
{
    long result;

    if (*arg == '$') {
	if (sscanf(arg+1, "%lx", &result) == 0) {
	    fprintf(stderr, "ERROR: illegal hex number %s\n", arg);
	    exit(1);
	}
    }
    else {
	if (sscanf(arg, "%ld", &result) == 0) {
	    fprintf(stderr, "ERROR: illegal decimal number %s\n", arg);
	    exit(1);
	}
    }
    return(result);
}


main(argc, argv)
int argc;
char *argv[];
{
    long lValue;
    char cValue;
    FILE *bb0;

    if (argc < 3) {
	fprintf(stderr, "USAGE: %s <address> <byte-lValue>*\n", *argv);
	exit(1);
    }

    bb0 = fopen("/dev/bb0", "w");
    if (bb0 == NULL) {
	fprintf(stderr, "ERROR: cannot open /dev/bb0\n");
	exit(1);
    }

    *argv++;
    argc--;
    lValue = getnum(*argv, 0);

    if (fseek(bb0, lValue, 0) == -1) {
	fprintf(stderr, "ERROR: seek error (address $06lx) on /dev/bb0\n",
		lValue);
	exit(1);
    }

    while (--argc > 0) {
	*argv++;
	cValue = getnum(*argv, 0);
	if (fwrite(&cValue, 1, 1, bb0) != 1) {
	    fprintf(stderr, "ERROR: write error on /dev/bb0\n");
	    exit(1);
	}
    }
    fclose(bb0);
}
