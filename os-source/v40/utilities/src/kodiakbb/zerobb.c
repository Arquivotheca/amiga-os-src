/*
 *	zerobb
 *
 *	zero the billboard memory
 */
#include	<stdio.h>

writeprotect(bb0, state)
FILE *bb0;
char state;
{
    if (fseek(bb0, 0xf7ffff, 0) == -1) {
	fprintf(stderr, "ERROR: 0xf7ffff seek error on /dev/bb0\n");
	exit(1);
    }
    if (fwrite(&state, 1, 1, bb0) != 1) {
	fprintf(stderr, "ERROR: 0xf7ffff write error on /dev/bb0\n");
	exit(1);
    }
}


main(argc)
int argc;
{
    long i, zero;
    char wp;
    FILE *bb0;

    if (argc != 1) {
	fprintf(stderr, "USAGE: zerobb\n");
	fprintf(stderr,
		"    this function zeros billboard memory $F00000-$F7FDFF\n");
	exit(1);
    }

    bb0 = fopen("/dev/bb0", "w");
    if (bb0 == NULL) {
	fprintf(stderr, "ERROR: cannot open /dev/bb0\n");
	exit(1);
    }

    writeprotect(bb0, 0);
    if (fseek(bb0, 0xf00000, 0) == -1) {
	fprintf(stderr, "ERROR: seek error on /dev/bb0\n");
	exit(1);
    }
    zero = 0;
    for (i = 0; i < 0x7fe00; i+= sizeof(zero)) {
	if (fwrite(&zero, sizeof(zero), 1, bb0) != 1) {
	    fprintf(stderr, "ERROR: write error on /dev/bb0\n");
	    exit(1);
	}
    }
    writeprotect(bb0, 1);
}
