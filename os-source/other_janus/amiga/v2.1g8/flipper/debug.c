#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>
/* #include <local/sccs.h> */

#include "share.h"

/* static char SccsId[] = SCCS_STRING; */

LONG __stdargs fprintf(BPTR, const char *, ...);

#ifdef VERBOSE
char window[] = "CON:10/10/520/150/FlipperDebug";

BPTR outfile = NULL;

void Debug0(format)
char * format;
{
    if(outfile != NULL) {
        fprintf(outfile, format);
    }
}

void Debug1(format, parameter1)
char * format;
ULONG parameter1;
{
    if(outfile != NULL) {
        fprintf(outfile, format, parameter1);
    }
}

void Debug2(format, parameter1, parameter2)
char * format;
ULONG parameter1, parameter2;
{
    if(outfile != NULL) {
        fprintf(outfile, format, parameter1, parameter2);
    }
}

void DebugInit()
{
    outfile = (Open(window, MODE_NEWFILE));
}

DebugEnd()
{
    Delay(250);
    if(outfile != NULL)
        Close(outfile);
    outfile = NULL;
}
#endif
