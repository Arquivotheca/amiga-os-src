#include <exec/types.h>

#include <string.h>
#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"
#include "libhdr.h"

#define SMALL 1

#define TEMPLATE    ""
#define OPT_COUNT   1

int cmd_endcli(void)
{
   struct DosLibrary *DOSBase;
   int rc = RETURN_ERROR;
   long opts[OPT_COUNT];
   struct RDArgs *rdargs;
   struct Segment *seg;

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL)PrintFault(IoErr(), NULL);
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
	rc=RETURN_OK;
	do_end(DOSBase);
        Forbid();
        seg = searchsegs(DOSBase,"CLI",NULL);
        if(seg && seg->seg_UC > 0)seg->seg_UC--;
        Permit();
        FreeArgs(rdargs);
      }
   CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL }
#endif
   return(rc);
}

void do_end(DOSBase)
struct DosLibrary *DOSBase;
{
struct FileHandle *fh;
struct CommandLineInterface *clip;
char buffer[48];

        clip = Cli();
        fh = (struct FileHandle *)BADDR(clip->cli_StandardInput);
	fh->fh_End = 0;

      	if (clip->cli_CurrentInput != clip->cli_StandardInput) {
	    Close(clip->cli_CurrentInput);
            clip->cli_CurrentInput = clip->cli_StandardInput;
	}

        clip->cli_Background = DOS_TRUE;
        if(clip->cli_Interactive)
		if(Fault(STR_PROCESS_ENDING,NULL,buffer,44))
		      writef(DOSBase,buffer,THISPROC->pr_TaskNum);
}
