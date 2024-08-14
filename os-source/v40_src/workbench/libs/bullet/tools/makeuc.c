#include <stdio.h>

char inBuffer[256];
FILE *fi, *fo;

void
putWord(i)
int i;
{
    short int w;

    w = i;
    if (fwrite((char *) &w, sizeof(w), 1, fo) != 1) {
	fprintf(stderr, "ERROR -- write failure\n");
	exit(2);
    }
}

main(argc, argv)
int argc;
char *argv[];
{
    int unicode, cgcode, uniprev, size;
    char flag;

    /* flag is usage flag */
    flag = (argc != 3);
    if (!flag) {
	fi = fopen(argv[1], "r");
	if (!fi) {
	    fprintf(stderr, "%s: ERROR -- cannot open input file \"%s\"\n",
		    argv[0], argv[1]);
	    flag = 1;
	}
    }
    if (!flag) {
	fo = fopen(argv[2], "w");
	if (!fo) {
	    fprintf(stderr, "%s: ERROR -- cannot open output file \"%s\"\n",
		    argv[0], argv[2]);
	    flag = 1;
	}
    }
    if (flag) {
	fprintf(stderr, "USAGE: %s <input-text> <output-binary>\n", argv[0]);
	exit(1);
    }

    /* flag is start flag */
    uniprev = 0;
    size = 0;
    putWord(0x5543);			/* "UC" */
    putWord(0);
    putWord(0);
    putWord(0);
    while (fgets(inBuffer, sizeof(inBuffer), fi)) {
	if (sscanf(inBuffer, "%x %d", &unicode, &cgcode) == 2) {
	    if ((unicode == 0) && (cgcode == 0)) {
		/* end of data */
		break;
	    }
	    if (unicode <= uniprev) {
		fprintf(stderr,
			"%s: ERROR -- UniCode %04x out of order after %04lx\n",
			argv[0], unicode, uniprev);
		exit(2);
	    }
	    putWord(unicode);
	    putWord(cgcode);
	    if (unicode != 0xffff) {
		uniprev = unicode;
	    }
	    size++;
	    flag = 1;
	}
	else {
	    if (flag) {
		fprintf(stderr, "%s: ERROR -- unrecognized data line...\n%s\n",
			argv[0], inBuffer);
		exit(2);
	    }
	}
    }
    fseek(fo, 4, 0);
    putWord(size);
    printf("%d entries\n", size);
}
