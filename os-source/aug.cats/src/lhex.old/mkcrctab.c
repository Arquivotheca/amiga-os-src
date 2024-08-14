#include	<limits.h>

# define	CRCPOLY	0xA001
# define	CHAR_BIT	8

main()
{
    unsigned int i, j, r;

    printf("static unsigned short crctable[] = {");
    for (i = 0; i <= UCHAR_MAX; i++) {
	r = i;
	for (j = 0; j < CHAR_BIT; j++)
	    if (r & 1) r = (r >> 1) ^ CRCPOLY;
	    else       r >>= 1;
	printf("%s%#06x,", i % 8 ? " " : "\n    ", r);
    }
    printf("\n};\n");

    exit(0);
}
