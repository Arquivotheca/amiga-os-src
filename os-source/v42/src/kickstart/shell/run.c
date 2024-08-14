;/*
lc -O -M -d -j73 -orun.o -iV:include -iV:inc.lattice -v -csf run
blink run.o to run.ld LIB LIB:amiga.lib lib:debug.lib sc sd nd
protect run.ld +p
quit
*/

/*
 * 		RUN command			*
 *						*
 *						*/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>

#include <string.h>

#include "libhdr.h"
#include "cli_init.h"


#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"

#define TOBPTR(x)	(((LONG) (x)) >> 2)

/*
#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TOBPTR(buf))
*/

#define TEMPLATE    "COMMAND/F"
#define OPT_COM  0
#define OPT_COUNT   1

#define SMALL 1

int cmd_run(void)
{
        struct DosLibrary *DOSBase;
	long opts[OPT_COUNT];
        struct RDArgs *rdargs;
	long rc=RETURN_ERROR;
	struct FileHandle *cvec;
	char *svec;
	char *command;
	struct FileHandle *fh;
	long len;
	struct TagItem stags[] = {
	    SYS_Input, NULL,
	    SYS_Output, NULL,
	    SYS_UserShell,TRUE,
	    TAG_DONE,};

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) {
         PrintFault(IoErr(), NULL);
	 goto err;
      }
      else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif

/*
won't work because of the newlines
        command=(char *)opts[OPT_COM];
        if(opts[OPT_COM])len=strlen(command);
*/
	/* get length and ptr to command - not null-terminated! */
	fh = (void *) BADDR((BPTR)(Input()));
	len = fh->fh_End;
	command = (char *) BADDR(fh->fh_Buf);

	/* Create a suitable SCB */
 	cvec = (struct FileHandle * )
	    AllocVec(sizeof(struct FileHandle)+len+8,MEMF_PUBLIC|MEMF_CLEAR);
	if (!cvec)goto err;


	svec= (char *) ((long) cvec) +sizeof(struct FileHandle);
	memcpy(svec,command,len);
	svec[len] = '\n'; /* terminate with a \n */

	/* this is to provide the characters passed */

/*	we allocated the cvec clear... */

	cvec->fh_Buf = TOBPTR(svec);
	cvec->fh_End = len+1;

	/* call System to do the RUN */
	stags[0].ti_Data=TOBPTR(cvec);
	stags[1].ti_Data=Output();
	if((rc=System(NULL,stags)) == -1)rc=RETURN_ERROR;
      }
err:;
	if (rdargs) FreeArgs(rdargs);
        CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else {OPENFAIL;}
#endif
   return rc;
}

