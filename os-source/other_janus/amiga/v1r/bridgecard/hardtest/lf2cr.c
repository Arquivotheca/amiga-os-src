#include    "exec/types.h"
#include    "libraries/dos.h"

main(argc, argv)
int argc;
char *argv[];
{
    int fi, fo;
    char buffer[512];
    int len, i; 

    if (argc != 3) {
	printf("USAGE: %s <file name> <cr file name>\n", argv[0]);
    }
    else {
	if ((fi = Open(argv[1], MODE_OLDFILE)) == 0) {
	    printf("ERROR: %s/ cannot open \"%s\"\n", argv[0], argv[1]);
	}
	else {
	    if ((fo = Open(argv[2], MODE_NEWFILE)) == 0) {
		printf("ERROR: %s/ cannot open \"%s\"\n", argv[0], argv[2]);
	    }
	    else {
		while ((len = Read(fi, buffer, 512)) > 0) {
		    for (i = 0; i < len; i++)
			if (buffer[i] == '\n') buffer[i] = '\r';
		    Write(fo, buffer, len);
		}
		Close(fi);
		Close(fo);
		printf("%s/ %s copied to %s with LFs changed to CRs\n",
			argv[0], argv[1], argv[2]);
	    }
	}
    }
}