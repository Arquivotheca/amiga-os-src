#include <stdio.h>

unsigned char buf[16384];

void main(int argc, char *argv[])
{
FILE *fin, *fout;

	if (argc != 3) {
		printf("usage: padit infile outfile\n");
		exit(0);
	}

	if (!(fin = fopen(argv[1], "rb"))) {
		printf("couldn't open %s for input\n", argv[1]);
		exit(0);
	}

	if (!(fout = fopen(argv[2], "wb"))) {
		printf("couldn't open %s for output\n", argv[2]);
		exit(0);
	}

	memset(buf, 0, sizeof(buf));
	fread(buf, sizeof(buf), 1, fin);
	fwrite(buf, sizeof(buf), 1, fout);

	fclose(fin);
	fclose(fout);

	exit(0);
}
