#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define TEMPLATE	"NAME/A,NAME2/A"

ULONG OpenTest(VOID)
{
    struct Library *SysBase = *(struct Library **)4L;
    struct Library *DOSBase;

    struct RDArgs *rdargs;
    LONG args[4]={0,0,0,0};
    BPTR file1,file2;
    APTR buf;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(rdargs = ReadArgs(TEMPLATE, args, NULL))
        {
            if(buf = AllocMem(512*1024,0))
            {
            	if(file1 = Open((STRPTR)args[0],MODE_OLDFILE))
            	{
            	    if(file2 = Open((STRPTR)args[1],MODE_NEWFILE))
            	    {
            	    	Read(file1,buf,512*1024);
            	    	Write(file2,buf,512*1024);

            	    	Close(file2);
            	    }
            	    Close(file1);
            	}
            	FreeMem(buf,512*1024);
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
    return(0);
}

