/* bptr.c */

#include    "exec/types.h"

extern LONG stdin;

sgethex(si)
char *si;
{
    char *s, c;
    BOOL error;
    LONG value;

    s = si;
    value = 0;
    error = FALSE;
    while ((!error) && (c = *s++)) {
	error = TRUE;
	if ((c >= '0') && (c <= '9')) {
	    value *= 16;
	    value += c - '0';
	    error = FALSE;
	}
	if ((c >= 'A') && (c <= 'F')) { 		 
	    value *= 16;
	    value += c - 'A' + 10;
	    error = FALSE;
	}
	if ((c >= 'a') && (c <= 'f')) { 		 
	    value *= 16;
	    value += c - 'a' + 10;
	    error = FALSE;
	}
	if (error) {
	    printf("ERROR -- ill formed parameter \"%s\".\n", si);
	    value = 0;
	}
    }
    return(value);
}


main(argc, argv)
int argc;
char *argv[];
{
    int i, len;
    UBYTE buffer[80];

    if (argc > 1) {
	/* convert all arguments */
	for (i = 1; i < argc; i++) {
	    printf("%lx\n", sgethex(argv[i])<<2);
	}
    }
    else {
	/* convert stdin till eof */
	while ((len = Read(stdin, buffer, 80)) > 0) {
	    buffer[len-1] = 0;
	    printf("%lx\n", sgethex(buffer)<<2);
	}
    }
}
