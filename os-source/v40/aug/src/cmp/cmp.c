#include <stdio.h>

void main(int argc, char **argv)
	{
	FILE *f1;
	FILE *f2;

	if (argc < 3)
		{
		fprintf(stderr, "USAGE: %s: file1 file2\n", *argv);
		exit(100);
		}

	if (!strcmp(argv[1], "-"))
		{
		f1 = stdin;
		}
	else
		{
		if (!(f1 = fopen(argv[1], "r")))
			{
			exit(5);
			}
		}

	if (!(f2 = fopen(argv[2], "r")))
		{
		exit(5);
		}

	while (!feof(f1) && !feof(f2))
		{
		if (getc(f1) != getc(f2))
			{
			exit(5);
			}
		}

	if (!feof(f1) || !feof(f2))
		{
		exit(5);
		}
	}
