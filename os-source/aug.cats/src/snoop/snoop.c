/*
 * snoop.c
 */

#include "exec/types.h"
#include "exec/exec.h" 
#include "exec/execbase.h" 
#include "libraries/dos.h"
#include "libraries/dosextens.h"

extern struct ExecBase *SysBase;
extern LVOAllocMem;
extern LVOFreeMem;
extern LVOAllocVec;
extern LVOFreeVec;
extern APTR SetFunction();
extern SnoopAllocMem();
extern SnoopFreeMem();
extern SnoopAllocVec();
extern SnoopFreeVec();
extern struct Task *FindTask();

#define TASKNAME        "SnoopMem"

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#endif

void printf(UBYTE *fmt,...);

char *vers = "\0$VER: snoop 36.13";
char *Copyright = 
  "snoop v36.13\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage = "Usage: snoop (outputs all allocs and frees serially)";

APTR  OldAllocMem, OldFreeMem;
APTR  NewAllocMem, NewFreeMem;
APTR  OldAllocVec, OldFreeVec;
APTR  NewAllocVec, NewFreeVec;


main(argc,argv)
    int  argc;
    char *argv[];
{
    short   b;
    long    size;
    struct ExecBase *execlib = SysBase;
    int Loop=TRUE;
    struct Task *task;
    char *program;
    char *OldName=NULL;
    BOOL Vecs = FALSE;

    if(SysBase->LibNode.lib_Version >= 36) Vecs = TRUE;

    program = argv[0];

    if(argc) {
	if(argv[1][0] == '?') {
            printf("%s\n%s\n",Copyright,usage);
            exit(0);
	}
    }
    if (task = FindTask(TASKNAME)) {
	 Signal(task, SIGBREAKF_CTRL_C);
	 exit(0);
	}
	Forbid();
	printf ("Alloc/Free Memory Snooper V36.13\n");
	printf ("(does not use caller's stack...");
	printf ("type Control-C to exit.\n");

	OldName = (task = FindTask(0) )->tc_Node.ln_Name;
	FindTask(0)->tc_Node.ln_Name = TASKNAME;

	OldAllocMem = SetFunction (execlib, &LVOAllocMem, SnoopAllocMem);
	OldFreeMem = SetFunction (execlib, &LVOFreeMem, SnoopFreeMem);
	if(Vecs)
	    {
	    OldAllocVec = SetFunction (execlib, &LVOAllocVec, SnoopAllocVec);
	    OldFreeVec = SetFunction (execlib, &LVOFreeVec, SnoopFreeVec);
	    }

        while (Loop) {
            Wait(SIGBREAKF_CTRL_C);
	    Forbid();
	    NewAllocMem=SetFunction (execlib, &LVOAllocMem, OldAllocMem);
	    NewFreeMem=SetFunction (execlib, &LVOFreeMem, OldFreeMem);
	    if(Vecs)
		{
	    	NewAllocVec=SetFunction (execlib, &LVOAllocVec, OldAllocVec);
	    	NewFreeVec=SetFunction (execlib, &LVOFreeVec, OldFreeVec);
		}

            if((NewAllocMem != (APTR) SnoopAllocMem) ||
	       (NewFreeMem  != (APTR) SnoopFreeMem)  ||
	       (Vecs && ((NewAllocVec != (APTR) SnoopAllocVec) ||
	       (NewFreeVec  != (APTR) SnoopFreeVec)))) {
                /* someone else has replaced the vectors ! */
                /* we cannot exit safely now, until the vectors are back */
		SetFunction (execlib, &LVOAllocMem, NewAllocMem);
		SetFunction (execlib, &LVOFreeMem, NewFreeMem);
		if(Vecs)
		    {
		    SetFunction (execlib, &LVOAllocVec, NewAllocVec);
		    SetFunction (execlib, &LVOFreeVec, NewFreeVec);
		    }
                printf("Error! %s cannot exit now.\nAllocMem/FreeMem vector has been replaced by another process!\n",program);
	    }
	    else {
		task->tc_Node.ln_Name = OldName; /* reset the name */
		Loop = FALSE;
	    }
	    Permit();
	}
	printf ("%s: terminated.\n", program);
}

