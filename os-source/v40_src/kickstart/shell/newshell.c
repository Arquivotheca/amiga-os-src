;/*
lc -O -M -d -j73 -onewshell.o -iV:include -iV:inc.lattice -v -csf newshell
blink newshell.o to newshell.ld LIB LIB:amiga.lib lib:debug.lib sc sd nd
protect newshell.ld +p
quit
*/

/*
 * 		NewShell command		*
 *						*
 *						*/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <clib/macros.h>

#include <string.h>

#include "libhdr.h"
#include "cli_init.h"

#include "internal/commands.h"
#include "global.h"
#include "fault.h"

#define TOBPTR(x)	(((LONG) (x)) >> 2)

/*
#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TOBPTR(buf))
*/

#define SMALL 1

#define TEMPLATE    "WINDOW,FROM"
#define OPT_WINDOW  0
#define OPT_FROM    1
#define OPT_COUNT   2

int OptToBstr (LONG *entry,UBYTE *buffer,LONG size);

int cmd_newshell(VOID)
{
        struct DosLibrary *DOSBase;
	long opts[OPT_COUNT];
        struct RDargs *rdargs;
	int rc=RETURN_ERROR,res2=NULL;
	struct Process   *task;
	struct Process   *tid;
	struct Segment *cliseg;
	struct DosPacket *pkt;
	struct DosPacket *backpkt;
	// These must end up BSTR's, so max 256
	UBYTE window[256]="CON:0/0//130/AmigaShell/CLOSE";
	UBYTE command[256];

	struct TagItem tags[] = {
	    { NP_Seglist,(LONG) NULL},
	    { NP_FreeSeglist, (LONG) FALSE },
	    { NP_StackSize, 1600},
	    { NP_Cli, TRUE },
	    { NP_HomeDir, NULL},
	    { NP_CloseInput, FALSE},
	    { NP_CloseOutput, FALSE},
	    { NP_CurrentDir, NULL},
	    { NP_Input, NULL},
	    { NP_Output, NULL},
	    { NP_CopyVars , TRUE},
	    { NP_Name,(LONG) "Shell Process" },
	    TAG_DONE,};

#ifndef SMALL
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);
      if (rdargs == NULL) {
         PrintFault(IoErr(), NULL);
      }
	else {
#else
      if(rdargs=getargs(&DOSBase,NULL,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
	tid = THISPROC;
	if (tid->pr_StackSize > 1600)tags[2].ti_Data = tid->pr_StackSize;
	if(!(cliseg = FindSegment("SHELL",0,TRUE)))goto err;
	tags[0].ti_Data = cliseg->seg_Seg;

	if (!OptToBstr(&(opts[OPT_WINDOW]),window,sizeof(window)))
	{
		PrintFault(ERROR_LINE_TOO_LONG,(char *)opts[OPT_WINDOW]);
		goto err;
	}
	// If no FROM file specified, leave it NULL.  CliInitNewcli will
	// then try to use S:Shell-startup.
	if (opts[OPT_FROM] &&
	    !OptToBstr(&(opts[OPT_FROM]),command,sizeof(command)))
	{
		PrintFault(ERROR_LINE_TOO_LONG,(char *)opts[OPT_FROM]);
		goto err;
	}

        task=CreateNewProc(tags);
	if (!task)PrintFault(STR_CANT_CREATE_PROC,"Newshell");
	else {
	    pkt=(struct DosPacket *)AllocDosObject(DOS_STDPKT, NULL);
            if (pkt) {
	    	pkt->dp_Type = 1; /* bcpl_newcli_init; */
	    	pkt->dp_Res1 = TRUE;
	    	pkt->dp_Res2 = 0;
	    	pkt->dp_Arg3 = 0; /* copy from process */
	    	pkt->dp_Arg1 = DupLock(tid->pr_CurrentDir);
	    	pkt->dp_Arg2 = TOBPTR(opts);
	    	pkt->dp_Arg4 = TOBPTR(Cli());
	    	pkt->dp_Arg5 = (LONG)GetFileSysTask();
	    	SendPkt(pkt,&(task->pr_MsgPort),&(tid->pr_MsgPort));
  	    	backpkt=WaitPkt();
		/* if the parameter packet failed, and the shell */
		/* didn't start up, we need to unlock the currentdir */
		if(!(backpkt->dp_Res1)) {
		    res2=backpkt->dp_Res2;
		    PrintFault(res2,"NewShell");
		    UnLock(pkt->dp_Arg1);
		}
		else rc=RETURN_OK;
	    	FreeDosObject(DOS_STDPKT,pkt);
	    }
	}
      }
err:	if (rdargs)FreeArgs(rdargs);
	if(rc) {
	    PrintFault(STR_NEWSHELL_FAILED,NULL);
	    if(res2)SetIoErr(res2);
	}
	CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else {OPENFAIL;}
#endif
   return rc;
}

VOID CtoBstr(string)
UBYTE *string;
{
LONG len = strlen(string);
long i;

for(i=len; i>0; i--)string[i]=string[i-1];
string[0]=len;
}

int
OptToBstr (LONG *entry,UBYTE *buffer,LONG size)
{
	LONG len;

	if (*entry) {
		len = strlen((char *)*entry);
		if (len >= size)
			return 0;

		memcpy(buffer+1,(char *)*entry,len);
		*buffer = len;
	} else
		CtoBstr(buffer); /* in place BtoC */

	*entry = TOBPTR(buffer);
	return 1;
}

