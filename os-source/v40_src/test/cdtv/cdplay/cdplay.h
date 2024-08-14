/*
 *
 * This is a header file for CDPlay
 *
 * Written by Greg Givler © Feb. 1992
 *
 */

/*
 *
 * System Include Section
 *
 */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>

#include <cd/cdtv.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

/*
 * ANSI/UNIX Include Section
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


/*
 *
 * Constant Definition Section
 *
 */

#define CDTV_UNIT 0 /* According to Autodocs this should be set to 0 */
#define CDTV_FLAGS 0 /* Autodocs say that this should be set to 0 */
#define TOC_START 0 /* Start for Table Of Contents */
#define TOC_ENTRIES 100 /* Number of Entries for the Array */
#define MAX_TRACKS 100 /* Max numbers of track on a CD per spec */
#define MAX_CHAR 80 /* Max characters for name of disc in database */

/*
 *  ReadArgs defines
 *
 */

#define TEMPLATE "TRACKS/M/N,RANDOM/S,INTRO/S"
#define OPT_COUNT 3
#define OPT_TRACKS 0
#define OPT_RANDOM 1
#define OPT_INTRO 2

/*
 *   Playmode defines
 */

#define PLAY_NORMAL 0
#define PLAY_FF 1
#define PLAY_REW 2

/* this is for CDTV_CR do not use in CDTV Classic or A570 */

#define PLAY_DOUBLE 3

/*
 *
 * Global Variable Section
 *
 */

#ifdef MAIN

struct IOStdReq *IOStdReq=NULL;
struct MsgPort *MsgPort=NULL;
BOOL CDTVOpen=FALSE;

#else

extern struct IOStdReq *IOStdReq;
extern struct MsgPort *MsgPort;
extern BOOL CDTVOpen;

#endif /* MAIN */

/*
 * Prototypes
 */

/* from main.c */
void CleanUp(WORD ReturnCode);
void main(int argc, char *argv[]);

/* from functions.c */
LONG getfirsttrack(struct CDTOC *toc);
LONG getlasttrack(struct CDTOC *toc);

LONG getTotalTime(struct CDTOC *toc);

struct CDTOC *getTOC(ULONG start,ULONG entries);
void freeTOC(APTR toc);

int isrom(void);
int myrandom(int range);

void playTrack(int start, int end);
void stopPlay(void);
void playNextTrack(struct CDTOC *toc, int intro, int currenttrack, int nexttrack);
void pausePlay(void);
void randomPlay(struct CDTOC *toc, int intro, int last);

/*
 *  Custom Structures
 *
 */

#ifdef I_HAVE_FINISHED_WRITING_THIS_THING
struct cdinfo {
    ULONG   recordlength;
    char    cdtitle[MAX_CHAR];
    char    cdartist[MAX_CHAR];
    char    cdorch[MAX_CHAR];
    int     totalsecs;
    int     firsttrack;
    int     lasttrack;
    int     trackorder[MAX_TRACK];
    int     totalsectors;
    int     upcnumber;
};
#endif
