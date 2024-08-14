/*
 *	namw.c - PC Janus Benchmark application
 *
 *	Copyright (C) 1990 Active Circuits, Inc.
 *	All Rights Reserved
 *
 *	Permission granted to Commodore Business Machines to use
 *	this code solely for the purpose of internal testing of
 *	Amiga Janus software/hardware performance.
 */
#include <dos.h>
#include <janus/janus.h>
#include "amw.h"

int AMW_Handler();

UBYTE GetService(ULONG,UWORD,UWORD,int(*)(),struct ServiceData **);
UBYTE ReleaseService(struct ServiceData *);
UBYTE GetBase();

unsigned char buf1[BUFSIZE], buf2[BUFSIZE];

struct ServiceData *SData;
volatile int Terminate;
volatile int Reset;
struct BenchMessage *TM;

main(argc,argv)
int argc;
char *argv[];
{
    UWORD  Flags = GETS_WAIT;
    int i, error;

    error = GetBase (JSERV_PCDISK,&i,&i,&i);
    if (error != JSERV_OK) {
	printf ("Error %d from GetBase\n", error);
	exit (3);
    }
Top:
    Terminate = 0;
    Reset = 0;
    printf ("Connecting to Amiga ...\n");
    error = GetService (BENCH_APPLICATION_ID, BENCH_LOCAL_ID,
		       Flags, 0, &SData);
    if (error != JSERV_OK) {
	printf ("Error %d, couldn't get the benchmark service.\n", error);
	exit (4);
    }
    TM = (struct Message *) SData->sd_PCMemPtr;

    while (Terminate == 0) {
	while (SData->sd_Flags & SERVICE_PCWAIT)
	    ;

	AMW_Handler();
    }

   printf ("Releasing Service.\n");
   ReleaseService (SData);
   if (Reset) {
       printf ("Resetting ...\n");
       goto Top;
   }
   exit (0);
}

char *array[] = {
	"MESSAGE ZERO",
	"TC_CLEAR",	"TC_HEADER",	"TC_LINE",
	"TC_EXIT",	"TC_SHELL",	"TC_TARGA",
	"TC_RESET",	"TC_ACK",	"TC_MEM_READ",
	"TC_MEM_COPY",	"TC_MEM_DUPL",	"TC_MEM_TARGA",
	"TC_CHECK_JANUS"
};

int
AMW_Handler () {
    static lastcommand = 0;
    if (lastcommand != TM->Command) {
	lastcommand = TM->Command;
	printf ("Received command <%s>\n", array[lastcommand]);
    }
    switch (TM->Command) {
    case TC_EXIT:
	Terminate = 1;
	Reset = 0;
	TM->Result = TC_EXIT;
	break;
    case TC_RESET:
	Terminate = 1;
	Reset = 1;
	TM->Result = TC_RESET;
	break;
    case TC_ACK:
	/* Don't do anything */
	break;
    case TC_MEM_READ:
	/* Read every byte of the Janus buffer */
	tc_read_mem ();
	break;
    case TC_MEM_COPY:
	/* Copy Janus buffer to non-Janus buffer */
	tc_copy_mem ();
	break;
    case TC_MEM_DUPL:
	/* Copy non-Janus memory around */
	tc_dupl_mem ();
	break;
    case TC_CHECK_JANUS:
	/* Check that Janus memory holds correct contents */
	tc_check_janus ();
	break;
    }
    CallService (SData);
    return;
}

tc_read_mem () {
	/* Read every byte of the Janus memory */
        register unsigned char receiver, *ptr;
	register unsigned short i;
	ptr = TM->Buffer;
	i = BUFSIZE;

	while (i > 0) {
		receiver = *ptr ++;
		i --;
	}
}

tc_copy_mem () {
	/* Copy Janus buffer to non-Janus memory */
	register unsigned short i;
	register unsigned char *in, *out;
	in = TM->Buffer;
	out = buf1;
	i = BUFSIZE;

	while (i > 0) {
		*out ++ = *in ++;
		i --;
	}
}

tc_dupl_mem () {
	/* Copy non-Janus memory around */
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

tc_check_janus () {
	/* Check Janus memory holds Amiga-written contents */
	register unsigned long i;
	register unsigned short j;
	register unsigned char *in;
	in = TM->Buffer;

	for (i=0; i<BUFSIZE; i++) {
		j = i & 0xff;
		if (j == 0) {
			printf ("Checkpoint at %d\n", i);
		}
		if ((*in) != j) {
			printf ("Entry %d was %d, should be %d\n", i, *in, j);
		}
		in ++;
	}
}
