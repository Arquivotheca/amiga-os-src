/*
 *	rev99bb
 *
 *	set all resident tags to version 99 in billboard memory
 */
#include	<stdio.h>

FILE *BB0 = 0;

readbb(target, size)
char *target;
int size;
{
    if (fread(target, size, 1, BB0) != 1) {
	perror("ERROR: read error on /dev/bb0\n");
	exit(1);
    }
}


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
    seekbb(0xf7ffff);
    writebb(&state, 1);
}



#define RTC_MATCHWORD	0x4AFC	/* The 68000 "ILLEGAL" instruction */

main(argc)
int argc;
{
    long base, skip, temp;
    short matchWord;
    char name[128], version;
    int i;

    if (argc != 1) {
	fprintf(stderr, "USAGE: rev99bb\n");
	fprintf(stderr, "%s\n%s\n",
		"    this function sets all resident tags in billboard memory",
		"    ($F00000-$F7FDFF) to have a version of 99");
	exit(1);
    }

    BB0 = fopen("/dev/bb0", "r+");
    if (BB0 == NULL) {
	fprintf(stderr, "ERROR: cannot open /dev/bb0\n");
	exit(1);
    }

    writeprotect(0);

    version = 99;
    base = 0xf00000;
    seekbb(base);

    while (base < 0xf7fe00) {
	readbb(&matchWord, sizeof(matchWord));
	if (matchWord == RTC_MATCHWORD) {
	    readbb(&temp, sizeof(temp));
	    if (temp == base) {
		readbb(&skip, sizeof(skip));
		seekbb(base+11);
		writebb(&version, sizeof(version));
		seekbb(base+18);
		readbb(&temp, sizeof(temp));
		seekbb(temp);
		for (i = 0; i < 127; i++) {
		    readbb(&name[i], 1);
		    if (name[i] == '\0')
			break;
		}
		name[i] = '\0';
		printf("    $%06x - $%06x %s", base, skip, name);
		seekbb(skip);
		base = skip;
		continue;
	    }
	    base += 2;
	    seekbb(base);
	    continue;
	}
	base += 2;
    }

    writeprotect(1);
}
