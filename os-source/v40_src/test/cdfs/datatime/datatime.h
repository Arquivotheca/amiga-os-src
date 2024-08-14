/*
 * This is the header file for DataTime
 * Written by Gregory P. Givler
 * Copyright © May 1993 CISCO
 * $VER: DataTime_h 1.1 19 May 1993 © CISCO
 */

/*
 *
 * System Include Section
 *
 */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>

#include <cd/cd.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>

#include <libraries/lowlevel.h>

#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/timer_protos.h>
#include <clib/lowlevel_protos.h>
#include "timelib/timelib_protos.h"

/*
 *
 * ANSI/UNIX Include Section
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 *
 * Constant Definition Section
 *
 */

#define CD_UNIT 0
#define CD_FLAGS 0
#define DEFAULT_SIZE 262144 /* 256K */

/*
 *
 * ReadArgs Definition Section
 *
 */

#define TEMPLATE "NORMAL/S,DOUBLE/S,S=SIZE/K/N,I=ITERATIONS/K/N,F=FILE/K,CD=CDREAD/S,OFF=OFFSET/K/N"
#define OPT_COUNT 7
#define OPT_NORMAL 0
#define OPT_DOUBLE 1
#define OPT_SIZE 2
#define OPT_ITERATIONS 3
#define OPT_FILE 4
#define OPT_CDREAD 5
#define OPT_OFFSET 6

/*
 *
 * Global Variable Section
 *
 */

struct IOStdReq *IOStdReq=NULL;
struct MsgPort *MsgPort=NULL;
BOOL CDOpen=FALSE;
struct Library *TimerBase;
struct timerequest *TimerIO;
struct MsgPort *TimerMP;
BOOL TimerOpen=FALSE;
struct RDArgs *argsptr=NULL;
struct EClockVal *start=NULL, *stop=NULL;
APTR memoryptr=NULL;
ULONG readsize=NULL;

/*
 *
 * My Prototypes
 *
 */

/* from main.c */
void CleanUp(WORD ReturnCode);
void main(int argc, char *argv[]);

