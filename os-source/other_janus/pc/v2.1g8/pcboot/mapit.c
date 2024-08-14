#include <stdio.h>
#include <ctype.h>

unsigned long htoul(char *str)
{
static char *hex = "0123456789ABCDEF";
unsigned long retval;
int i;

	retval = 0;
	
	while (*str) {
		if (islower(*str))
			*str = toupper(*str);
		for (i = 0; i < 16; i++) {
			if (hex[i] == *str) {
				retval = (retval << 4) + i;
				break;
			}
		}
		if (i == 16)
			break;
		str++;
	}

	return retval;
}

unsigned long scanmap(char *mapfile, char *sym)
{
char lin[256];
int len;
unsigned long retval;
FILE *fin;

	retval = 0;

	if (!(fin = fopen(mapfile, "rt")))
		return retval;

	len = strlen(sym);
	
	while (fgets(lin, sizeof(lin)-1, fin)) {
		if (memcmp(lin+17, sym, len) == 0) {
			retval = htoul(lin+6);
			return retval;
		}
	}

	fclose(fin);

	return retval;
}

void write_dummy_null(char *fname, unsigned long size)
{
FILE *fout;

	if (!(fout = fopen(fname, "wt"))) {
		return;
	}

	fprintf(fout, "dummy_null db %ld dup ('a')\n", size);

	fclose(fout);
}

void main(int argc, char *argv[])
{
unsigned long dummy_null, dataend;
long stackleft, blksize;

	if (argc != 2) {
		printf("usage: mapit janus.map\n");
		exit(0);
	}

	dummy_null = scanmap(argv[1], "DUMMY_NULL");
	if (dummy_null == 0) {
		printf("couldn't find DUMMY_NULL\n");
		exit(100);
	}

	if (dummy_null > 0x2000) {
		printf("OUCH!  code exceeds offset 2000H by %d bytes!\n",
			dummy_null - 0x2000);
		exit(200);
	}

	dataend = scanmap(argv[1], "DATAEND");

	if (dataend == 0) {
		printf("couldn't find DATAEND\n");
		exit(300);
	}

	stackleft = 0x3000 - dataend;
	printf("%ld bytes of stack left\n", stackleft);

	blksize = 0x2000 - dummy_null;
	printf("%ld bytes of code left\n", blksize);
	
	write_dummy_null("DUMNUL.INC", blksize);
}
