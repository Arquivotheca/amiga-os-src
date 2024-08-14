/*
 *	map99bb <module> <segment-offset>
 *
 *	construct a map file for wack from a memory library or device
 */
#define	D(a)
#include	<stdio.h>
#include	<strings.h>

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


main(argc, argv)
int argc;
char *argv[];
{
    long offset, node, next, temp;
    char *s, c, deviceFlag;
    short i;

    /* test parameter suitability */
    if (    /* check for three parameters */
	    (argc == 3) &&
	    /* find '.' in module name */
	    ((s = rindex(argv[1], '.')) != 0) &&
	    /* verify .library or .device */
	    (((deviceFlag = strcmp(s, ".library")) == 0) ||
		(strcmp(s, ".device") == 0)) &&
	    /* check for useful offset */
	    ((offset = atoi(argv[2])) != 0)) {

	BB0 = fopen("/dev/bb0", "r+");
	if (BB0 == NULL) {
	    fprintf(stderr, "ERROR: cannot open /dev/bb0\n");
	    exit(1);
	}

	/* get exec list */
	seekbb(4);
	readbb(&node, sizeof(node));

	if (deviceFlag) {
	    node += 350;
	}
	else {
	    node += 378;
	}
	
	seekbb(node);
	readbb(&node, sizeof(node));

	for (;;) {
	    seekbb(node);		/* ln_Succ */
	    readbb(&next, sizeof(next));
	    if (next == 0)
		break;
	    seekbb(node+10);		/* ln_Name */
	    readbb(&temp, sizeof(temp));
	    seekbb(temp);
	    i = 0;
	    do {
		readbb(&c, 1);
	    }
		while (argv[1][i++] == c);
	    if ((i >= 2) && (argv[1][i-2] == 0)) {
		/* found the module */
		seekbb(node+offset);

		/* print the map */
		i = 0;
		for (;;) {
		    readbb(&node, sizeof(node));
		    if (node == 0)
			exit(0);
		    node <<= 2;
		    seekbb(node-4);
		    readbb(&temp, sizeof(temp));
		    printf("%2ld %8lX %8lX\n", i++, node+4, temp-4);
		}
		break;
	    }
	    node = next;
	}
    }
    fprintf(stderr, "USAGE: mapbb <module> <offset>\n");
    exit(1);
}
