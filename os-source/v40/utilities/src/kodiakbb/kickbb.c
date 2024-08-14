/*
 *	kickbb
 *
 *	load binary kickit image into billboard
 */
#include	<sys/file.h>
#include	<sys/ioctl.h>
#include	<stdio.h>

#define	COLDCODEADDR	0x20000

int BB0 = 0;
FILE *FKick;

unsigned long
readkick()
{
    unsigned long source;

    if (fread(&source, sizeof(source), 1, FKick) != 1) {
	perror("ERROR: read error on kick file\n");
	exit(1);
    }
    return(source);
}


readbb(target, size)
char *target;
int size;
{
    if (read(BB0, target, size) != size) {
	perror("ERROR, read of /dev/bb0");
	exit(1);
    }
}


writebb(target, size)
char *target;
int size;
{
    if (write(BB0, target, size) != size) {
	perror("ERROR, write of /dev/bb0");
	exit(1);
    }
}


seekbb(address)
long address;
{
    if (lseek(BB0, address, L_SET) == -1) {
	perror("ERROR: lseek of /dev/bb0");
	exit(1);
    }
}


writeprotect(state)
char state;
{
    unsigned char old;

    seekbb(0xf7ffff);
    readbb(&old, 1);

    old = (old & 0xfe) | state;
    seekbb(0xf7ffff);
    writebb(&old, 1);
}


bbctrl(code)
char code;
{
    if (ioctl(BB0, _IO(B, code)) == -1) {
	perror("ERROR, ioctl of /dev/bb0");
	exit(1);
    }
}


main(argc, argv)
int argc;
char *argv[];
{
    FILE *afile;
    long base, size;
    unsigned long address, source, cSum, cSumPrev;

    if ((argc < 2) || (argc > 3)) {
	fprintf(stderr, "USAGE: kickbb <kick.f0> [<address-file>]\n");
	fprintf(stderr, "%s\n%s\n",
		"    this function loads the kick file into the billboard, and",
		"    optionally writes the start of unused rom into a file");
	exit(1);
    }

    FKick = fopen(argv[1], "r");
    if (FKick == NULL) {
	fprintf(stderr, "ERROR: cannot open %s\n", argv[1]);
	exit(1);
    }

    afile = 0;
    if (argc == 3) {
	afile = fopen(argv[2], "w");
	if (afile == NULL) {
	    fprintf(stderr, "ERROR: cannot open %s\n", argv[2]);
	    exit(1);
	}
    }

    BB0 = open("/dev/bb0", O_RDWR);
    if (BB0 == -1) {
	perror("ERROR: open of /dev/bb0");
	exit(1);
    }

    bbctrl(3);				/* set hold of BGACK */
    writeprotect(0);			/* clear write protect */

    source = readkick();
    if (source != 0) {
	size = 0x40000;			/* 256K image */
    }
    else {
	size = readkick();
	source = readkick();
    }

    address = 0;
    base = 0xf00000;
    size /= sizeof(source);
    seekbb(base);

    cSum = cSumPrev = 0;
    while (size--) {
	if (base == 0xf00000)
	    source = (source & 0x0000ffff) | 0x11110000;
	else {
	    source = readkick();
	    if (size == 5)		/* this is the checksum long */
		source = 0;
	}
	writebb(&source, sizeof(source));
	if (base == 0xf7ffe4)
	    address = source;		/* cache the end of used rom */
	if (base == 0xf7fffe)
	    source |= 1;		/* checksum w/ write protect on */
	cSum += source;
	if (cSum < cSumPrev)
	    cSum++;
	cSumPrev = cSum;
	base += sizeof(source);
    }
    seekbb(base-0x18);
    writebb(&cSum, sizeof(cSum));

    if (afile) {
	if (address) {
	    address = (address + 3) & 0xfffffffc;
	    fprintf(afile, "%06lx\n", address);
	}
	else {
	    fprintf(stderr, "WARNING: address in %s not valid\n", argv[2]);
	    fprintf(afile, "f40000\n");
	}
    }

    writeprotect(1);			/* set write protect */
    bbctrl(1);				/* set reset */
    bbctrl(2);				/* clear hold of BGACK */
    bbctrl(0);				/* clear reset */
}
