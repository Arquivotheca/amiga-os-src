/*
 *	symbm
 *
 *	renumber bbload map lines on stdin to stdout
 */
#define	D(f)	printf f

#include	<stdio.h>

char InBuffer[40];
char OutBuffer[40];

main()
{
    int line;
    char *in, *out;

    line = 0;

    while (in = (char *) gets(InBuffer)) {
	while (*++in != ' ');
	sprintf(OutBuffer, "%2d", line);
	out = OutBuffer + strlen(OutBuffer);
	strcpy(out, in);
	puts(OutBuffer);
	line++;
    }
}
