#include <stdio.h>

void main(int argc, char **argv)
{
FILE *fin, *fout;
long len;
unsigned char buf[4], sum;

	if (argc != 3) {
		printf("check8 infile outfile\n");
		exit(1);
	}

	if (!(fin = fopen(argv[1], "rb"))) {
		printf("couldn't open input file\n");
		exit(1);
	}

	if (!(fout = fopen(argv[2], "wb"))) {
		printf("couldn't open output file\n");
		exit(1);
	}

	if (fseek(fin, 0, 2) == -1) {
		printf("fseek() error on input file");
		exit(1);
	}
	len = ftell(fin);
	if (fseek(fin, 0, 0) == -1) {
		printf("fseek() error on input file");
		exit(1);
	}

	sum = 0;
	
	while (len-- > 1) {
		if (fread(buf, 1, 1, fin) != 1) {
			printf("fread() error on input file");
			exit(1);
		}

		sum += buf[0];

		if (fwrite(buf, 1, 1, fout) != 1) {
			printf("fwrite() error on output file");
			exit(1);
		}
	}

	buf[0] = ~sum + 1;

	if (fwrite(buf, 1, 1, fout) != 1) {
		printf("fwrite() error on output file");
		exit(1);
	}

	exit(0);
}
