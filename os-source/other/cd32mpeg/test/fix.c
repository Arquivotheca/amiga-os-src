#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "asyncio.h"

#define TEMPLATE	"INFILE/A,OUTFILE/A"

struct Library *SysBase;
struct Library *DOSBase;

ULONG OpenTest(VOID)
{
    struct RDArgs *rdargs;
    LONG args[4]={0,0,0,0};
    struct AsyncFile *file,*out;
    LONG len;
    UBYTE lineBuff[100];

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE, args, NULL))
        {
            if(file = OpenAsync((STRPTR)args[0],MODE_READ,262144))
            {
                if(out = OpenAsync((STRPTR)args[1],MODE_WRITE,262144))
                {
                    while(len = ReadAsync(file,lineBuff,80))
                    {
                        lineBuff[len] = '\n';
                        WriteAsync(out,lineBuff,len+1);
                        if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
                            break;
                    }
                    CloseAsync(out);
                }
                CloseAsync(file);
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
    return(0L);
}