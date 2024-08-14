/****** CatF *********************************************************
*
*   NAME
*       CatF - concatinate files from a file list to stdout
*
*********************************************************************/
/*#define D(a)	fprintf a*/
#define	D(a)

#include	<stdio.h>
#include	<setjmp.h>

#define	ERROR_PARMCNT	1
#define	ERROR_OPEN	2
#define	ERROR_READ	3
#define	ERROR_WRITE	4

#define	FILENAMEBUFSIZE	16384
char fileNameBuffer[FILENAMEBUFSIZE];
#define COPYBUFSIZE	1024
char copyBuffer[COPYBUFSIZE];

char *errorMsgs[] = {
    "invalid parameters",
    "open failed for file \"%s\"",
    "read error for file \"%s\"",
    "write error on stdout"
};

char *findNextName(char *fileName)
{
    D((stderr, "-- findNextName entry 0x%x \"%s\"\n", fileName, fileName));
    while ((*fileName == ' ') || (*fileName == '\t') ||
	    (*fileName == '\r') || (*fileName == '\n')) {
	if (*fileName == '\n') {
	    D((stderr, "-- findNextName exit  0\n"));
	    return((char *) 0);
	}
	fileName++;
    }
    D((stderr, "-- findNextName exit  0x%x \"%s\"\n", fileName, fileName));
    return(fileName);
}

main(int argc, char *argv[])
{
    jmp_buf env;
    int errorCode;
    FILE *ffile, *file;
    char *fileName=0, *nextFileName;
    int actual;

    if (errorCode = setjmp(env)) {
	fprintf(stderr, "ERROR: ");
	fprintf(stderr, errorMsgs[errorCode-1], fileName);
	fprintf(stderr, "\nUsage: %s <file-list-file>\n", argv[0]);
	exit(5);
    }

    if (argc != 2)
	longjmp(env, ERROR_PARMCNT);

    fileName = argv[1];
    ffile = fopen(fileName, "r");
    if (ffile == 0)
	longjmp(env, ERROR_OPEN);

    fileName = 0;
    for (;;) {
	/* get next file name */
	while (fileName == 0) {
	    /* read next line of file list file */
	    if (fgets(fileNameBuffer, FILENAMEBUFSIZE-1, ffile) == 0) {
		if (feof(ffile))
		    exit(0);
		else {
		    fileName = argv[1];
		    longjmp(env, ERROR_READ);
		}
	    }
	    fileName = findNextName(fileNameBuffer);
	}

	nextFileName = fileName;
	while ((*nextFileName != ' ') && (*nextFileName != '\t') &&
		(*nextFileName != '\r') && (*nextFileName != '\n')) {
	    nextFileName++;
	}
	D((stderr, "-- fileName end   0x%x\n", nextFileName));
	if (*nextFileName == '\n') {
	    *nextFileName = '\0';
	    nextFileName = 0;
	}
	else {
	    *nextFileName = '\0';
	    nextFileName = findNextName(nextFileName+1);
	}
	D((stderr, "-- fileName start 0x%x \"%s\"\n", fileName, fileName));
	D((stderr, "-- nextFileName   0x%x\n", nextFileName));

	file = fopen(fileName, "r");
	if (file == 0)
	    longjmp(env, ERROR_OPEN);

	do {
	    actual = fread(copyBuffer, 1, COPYBUFSIZE, file);
	    if (actual == -1)
		longjmp(env, ERROR_READ);
	    if (actual != 0)
		if (fwrite(copyBuffer, 1, actual, stdout) <= 0)
		    longjmp(env, ERROR_WRITE);
	}
	    while (actual != 0);

	fclose(file);

	fileName = nextFileName;
    }
}
