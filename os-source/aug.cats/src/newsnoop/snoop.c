/*
 * snoop.c
 */

#include "exec/types.h"
#include "exec/exec.h"
#include "exec/execbase.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "snoop_rev.h"

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;
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

void printf(UBYTE * fmt,...);
BOOL strEqu(UBYTE *, UBYTE *);
UBYTE toUpper(UBYTE);

char *vers = VERSTAG;
char *vstring = VSTRING;
char *Copyright = "Copyright (c) 1991 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage = "Usage: snoop [vec] (snoop AllocVec too).\nOutputs information on serial port";

APTR OldAllocMem, OldFreeMem;
APTR NewAllocMem, NewFreeMem;
APTR OldAllocVec, OldFreeVec;
APTR NewAllocVec, NewFreeVec;

main(argc, argv)
int argc;
char *argv[];
{
	struct ExecBase *execlib = SysBase;
	int Loop = TRUE;
	struct Task *task;
	char *program;
	char *OldName = NULL;
	BOOL Vecs = FALSE;

	program = argv[0];

	if (argc) {
		if (argv[1][0] == '?') {
			printf("%s%s\n", vstring, Copyright);
			printf("%s\n", usage);
			exit(0);
		} else if ((argc == 2) && (strEqu(argv[1], "vec"))) {
			if (SysBase->LibNode.lib_Version >= 36)
				Vecs = TRUE;
			else {
				printf("Can't do. V%ld hasn't got AllocVec()\n", SysBase->LibNode.lib_Version);
				exit(0);
			}
        }
	}
	if (task = FindTask(TASKNAME)) {
		Signal(task, SIGBREAKF_CTRL_C);
		exit(0);
	}
	Forbid();
	printf("Alloc/FreeMem%s %s", ((Vecs) ? "&Vec" : ""), vstring);
	printf("(does not use caller's stack...");
	printf("type Control-C to exit.\n");

	OldName = (task = FindTask(0))->tc_Node.ln_Name;
	FindTask(0)->tc_Node.ln_Name = TASKNAME;

	OldAllocMem = SetFunction((struct Library *)execlib, (LONG)&LVOAllocMem, SnoopAllocMem);
	OldFreeMem = SetFunction((struct Library *)execlib, (LONG)&LVOFreeMem, SnoopFreeMem);
	if (Vecs) {
		OldAllocVec = SetFunction((struct Library *)execlib, (LONG)&LVOAllocVec, SnoopAllocVec);
		OldFreeVec = SetFunction((struct Library *)execlib, (LONG)&LVOFreeVec, SnoopFreeVec);
	}
	Permit();

	while (Loop) {
		Wait(SIGBREAKF_CTRL_C);
		Forbid();
		NewAllocMem = SetFunction((struct Library *)execlib, (LONG)&LVOAllocMem, OldAllocMem);
		NewFreeMem = SetFunction((struct Library *)execlib, (LONG)&LVOFreeMem, OldFreeMem);
		if (Vecs) {
			NewAllocVec = SetFunction((struct Library *)execlib, (LONG)&LVOAllocVec, OldAllocVec);
			NewFreeVec = SetFunction((struct Library *)execlib, (LONG)&LVOFreeVec, OldFreeVec);
		}
		if ((NewAllocMem != (APTR) SnoopAllocMem) ||
		    (NewFreeMem != (APTR) SnoopFreeMem) ||
		    (Vecs && ((NewAllocVec != (APTR) SnoopAllocVec) ||
			      (NewFreeVec != (APTR) SnoopFreeVec)))) {
			/* someone else has replaced the vectors ! */
			/* we cannot exit safely now, until the vectors are back */
			SetFunction((struct Library *)execlib, (LONG)&LVOAllocMem, NewAllocMem);
			SetFunction((struct Library *)execlib, (LONG)&LVOFreeMem, NewFreeMem);
			if (Vecs) {
				SetFunction((struct Library *)execlib, (LONG)&LVOAllocVec, NewAllocVec);
				SetFunction((struct Library *)execlib, (LONG)&LVOFreeVec, NewFreeVec);
			}
			printf("Error! %s cannot exit now.\nAllocMem/FreeMem vector has been replaced by another process!\n", program);
		} else {
			task->tc_Node.ln_Name = OldName;	/* reset the name */
			Loop = FALSE;
		}
		Permit();
	}
	printf("%s: terminated.\n", program);
}

BOOL strEqu(UBYTE * p, UBYTE * q)
{
	while (toUpper(*p) == toUpper(*q)) {
		if (*(p++) == 0)
			return (TRUE);
		++q;
	}
	return (FALSE);
}

UBYTE toUpper(UBYTE c)
{
	UBYTE u = c;
	if (((u >= 'a') && (u <= 'z')) || ((u >= 0xe0) && (u <= 0xfe)))
		u = u & 0xDF;
	return (u);
}
