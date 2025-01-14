/*
 *	bench.c - Janus Interface Benchmarking Module, Amiga Side.
 *
 *	Copyright (C) 1990 Active Circuits, Inc.
 *	All Rights Reserved.
 *
 *	Permission granted to Commodore Business Machines to use
 *	this code solely for the purpose of internal testing of
 *	Amiga Janus software/hardware performance.
 */

#include <proto/exec.h>
#include <exec/types.h>
#include <exec/memory.h>
#define	LINT_ARGS
/* NASTY NASTY JANUS NASTY NEEDS NASTY LINT_ARGS NASTY NASTY THING */
#include <janus/janus.h>
#include <Imagelink/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "amw.h"

void bench (void (*func) (void), char *text);
void copy_local (void);
void copy_janus (void);
void pc_ack (void);
void pc_read (void);
void pc_copy (void);
void pc_dupl (void);
void pc_check_janus (void);
void AllReset (void);

int SetTimer (void);
void KillTimer (void);

extern struct StartBody *sbody;
struct JanusBase *JanusBase = NULL;
unsigned char *buf1, *buf2;

struct BenchMessage *bptr;
struct ServiceData *sdata = 0;
int    sigmask  = 0;
SHORT  signum   = -1;

short AddService (struct ServiceData **, ULONG, USHORT, USHORT, USHORT, SHORT, USHORT);
short GetService (struct ServiceData **, ULONG, USHORT, SHORT, USHORT);
LONG	Timer_Bits, Signals, Nloops;

void
main (argc, argv) int argc; char **argv; {
    unsigned short error;
    sdata = NULL;
    signum = -1;
    JanusBase = NULL;

    if (argc != 2)
	Nloops = 100;
    else {
	Nloops = atoi (argv[1]);
	if ((Nloops < 1) || (Nloops > 1000))
	    Nloops = 100;
    }

    buf1 = AllocMem (BUFSIZE, MEMF_CLEAR | MEMF_PUBLIC);
    if (buf1 == NULL) {
 	printf ("Unable to allocate primary buffer memory\n");
        exit (1);
    }
    buf2 = AllocMem (BUFSIZE, MEMF_CLEAR | MEMF_PUBLIC);
    if (buf2 == NULL) {
	printf ("Unable to allocate secondary memory\n");
	FreeMem (buf1, BUFSIZE);
	exit (1);
    }

    JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME,0);
    if (JanusBase == 0) {
        printf ("Unable to open Janus.Library");
	AllReset ();
	FreeMem (buf1, BUFSIZE);
	FreeMem (buf2, BUFSIZE);
	exit (1);
    }

    signum = (SHORT) AllocSignal(-1L);
    if (signum == -1) {
	AllReset ();
	FreeMem (buf1, BUFSIZE);
	FreeMem (buf2, BUFSIZE);
	exit (2);
    }

    sigmask = 1L << signum;

    error = GetService (&sdata, BENCH_APPLICATION_ID, BENCH_LOCAL_ID, signum, GETS_TOPC_ONLY|GETS_FROMPC_ONLY);
    if (error == JSERV_NOSERVICE) {
        error = AddService (&sdata, BENCH_APPLICATION_ID, BENCH_LOCAL_ID,
	                (USHORT) sizeof(struct BenchMessage), (USHORT) MEMF_BUFFER|MEM_BYTEACCESS, signum,
		        ADDS_TOPC_ONLY|ADDS_FROMPC_ONLY);
        if (error != JSERV_OK) {
	    printf ("Error %d from AddService\n", error);
	    sdata = NULL;
	    AllReset ();
	    FreeMem (buf1, BUFSIZE);
	    FreeMem (buf2, BUFSIZE);
	    exit (3);
        }
	printf ("Added Benchmark service\n");
    } else
	printf ("Attached to existing Benchmark service\n");

    bptr = (struct BenchMessage *) sdata->sd_AmigaMemPtr;
    bptr = (struct BenchMessage *) MakeBytePtr((APTR) bptr);

    /* TIMEOUT Code added to avoid infinite loop */
    if (SetTimer() == 0) {
	AllReset ();
	FreeMem (buf1, BUFSIZE);
	FreeMem (buf2, BUFSIZE);
	exit (4);
    }
    bptr->Command = TC_ACK;
    CallService (sdata);
    Signals = Wait (sigmask | Timer_Bits);
    KillTimer();

    /* Did we timeout first ? */
    if (Signals & Timer_Bits) {
	printf ("Benchmark server on PC Side not responding.\n");
	ReleaseService (sdata);
	sdata = NULL;
	AllReset ();
	FreeMem (buf1, BUFSIZE);
	FreeMem (buf2, BUFSIZE);
	exit (5);
    }
    
    printf ("Buffer size = %ld bytes, loops per test = %ld\n\n", BUFSIZE, Nloops);
    bench (copy_local, "Amiga copies Amiga memory to Amiga memory");
    bench (copy_janus, "Amiga copies Amiga memory to Janus memory");
    bench (pc_ack, "PC simply acknowledges");
    bench (pc_read, "PC reads Janus memory");
    bench (pc_copy, "PC copies Janus memory to PC memory");
    bench (pc_dupl, "PC copies PC memory to PC memory");
    bench (pc_check_janus, "PC checks out Janus memory");

    AllReset ();
    FreeMem (buf1, BUFSIZE);
    FreeMem (buf2, BUFSIZE);
    exit (0);
}


void
bench (func, text) void (*func) (void); char *text; {
	register unsigned short count;
	unsigned int before[2], after[2];
	int secs, milli;
	printf ("%s\n", text);
	
	timer (&(before[0]));
	for (count = 0; count < Nloops; count ++) {
		(*func) ();
	}
	timer (&(after[0]));
	secs = after[0] - before[0];
	milli = after[1] - before[1];
	if (milli < 0) {
		milli += 1000000;
		secs -= 1;
	}
	printf ("Total time: %ld.%06ld\n\n", secs, milli);
}

void
copy_local () {
	/* Copy Amiga memory to Amiga memory */
	register unsigned short i;
	register unsigned char *in, *out;
	in = buf1;
	out = buf2;
	i = BUFSIZE;

	while (i > 0) {
		*out ++ = *in ++;
		i --;
	}
}

void
copy_janus () {
	/* Copy Amiga memory to Janus memory */
	register unsigned short i;
	register unsigned char *in, *out;
	in = buf1;
	out = &(bptr->Buffer[0]);
	i = BUFSIZE;

	while (i > 0) {
		*out ++ = *in ++;
		i --;
	}
}

void
pc_ack () {
	/* PC simply acknowledges */
	bptr->Command = TC_ACK;
	CallService (sdata);
	Wait (sigmask);
}

void
pc_read () {
	/* Tell PC to just read Janus memory */
	bptr->Command = TC_MEM_READ;
	CallService (sdata);
	Wait (sigmask);
}

void
pc_copy () {
	/* Tell PC to copy Janus memory to PC memory */
	bptr->Command = TC_MEM_COPY;
	CallService (sdata);
	Wait (sigmask);
}

void
pc_dupl () {
	/* Tell PC to copy PC memory to PC memory */
	bptr->Command = TC_MEM_DUPL;
	CallService (sdata);
	Wait (sigmask);
}

void
pc_check_janus () {
	register unsigned short i, j;
	register unsigned char *out;
	out = &(bptr->Buffer[0]);
	i = BUFSIZE;

	for (i=0; i<BUFSIZE; i++) {
		j = i & 0xff;
		*out = j;
		out ++;
	}

	/* Tell PC to copy PC memory to PC memory */
	bptr->Command = TC_CHECK_JANUS;
	CallService (sdata);
	Wait (sigmask);
}
	

void
AllReset () {
	if (sdata) {
		bptr->Command = TC_EXIT;
		CallService (sdata);
		Wait (sigmask);
		ReleaseService (sdata);
	}
	if (signum != -1) {
		FreeSignal (signum);
	}
	if (JanusBase) {
		CloseLibrary ((struct Library *) JanusBase);
	}
}

