/*
 *	csumbb
 *
 *	checksum billboard memory
 */
#include	<stdio.h>

FILE *BB0 = 0;


writebb(target, size)
char *target;
int size;
{
    if (fwrite(target, size, 1, BB0) != 1) {
	perror("ERROR: write error on /dev/bb0\n");
	exit(1);
    }
}


seekbb(address)
long address;
{
    if (fseek(BB0, address, 0) == -1) {
	perror("ERROR: seek error on /dev/bb0\n");
	exit(1);
    }
}


writeprotect(state)
char state;
{
    unsigned char old;

    seekbb(0xf7ffff);
    if (fread(&old, 1, 1, BB0) != 1) {
	perror("ERROR: read error on /dev/bb0\n");
	exit(1);
    }

    old = (old & 0xfe) | state;
    seekbb(0xf7ffff);
    writebb(&old, 1);
}


main(argc)
int argc;
{
    long base;
    unsigned long source, cSum, cSumPrev;

    if (argc != 1) {
	fprintf(stderr, "USAGE: csumbb\n");
	fprintf(stderr,
		"this function checksums the first 256K of billboard memory\n");
	exit(1);
    }

    BB0 = fopen("/dev/bb0", "r+");
    if (BB0 == NULL) {
	fprintf(stderr, "ERROR: cannot open /dev/bb0\n");
	exit(1);
    }

    writeprotect(0);

    base = 0xf00000;
    seekbb(base);

    cSum = cSumPrev = 0;
    while (base < 0xf7ffff) {
	if (fread(&source, 4, 1, BB0) != 1) {
	    perror("ERROR: read error on /dev/bb0\n");
	    exit(1);
	}
	if (base == 0xf7ffe8)
	    source = 0;
	if (base == 0xf7fffe)
	    source |= 1;		/* checksum w/ write protect on */
	cSum += source;
	if (cSum < cSumPrev)
	    cSum++;
	cSumPrev = cSum;
	base += 4;
    }
    seekbb(0xf7ffe8);
    writebb(&cSum, 4);

    writeprotect(1);
}
