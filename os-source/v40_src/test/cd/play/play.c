/* Play.c
 * 
 * play specified cd tracks on cdgs.
 * Control-C: abort and quit
 * Control-F: skip (forward?) to next specified track.
 * 
 * 1.1 - initial release
 * 1.2 - added ^C/^F features
 * 1.3 - added WaitIO() after CheckIO() for normal return.  Oops.
 *
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <cd/cd.h>
#include <stdlib.h>
#include <string.h>


/* Defines ------------------------------------------ */
#define PROGNAME "Play"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "TRACKS/M/N/A,NAME=DEVICENAME/K,UNIT/K/N"
#define DEFAULT_UNIT_NAME    "cd.device"
#define DEFAULT_UNIT_NUMBER  0UL

/* Structs ------------------------------ */
struct Opts {
    LONG   **tracks;   /* track numbers to play   */
    STRPTR unitName;   /* name, default cd.device */
    ULONG  *unitNum;   /* unit number, default 0  */
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
static BOOL playIt(struct IOStdReq *);

/* Globals --------------------------------------- */
struct Opts          opts;
struct RDArgs       *rdargs;
struct IOStdReq     *io;
struct MsgPort      *mp;
LONG                 cdOpen;


VOID main(int argc, UBYTE *argv[]) {
    int i=0;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    io->io_Command = CD_PLAYTRACK;    /* Play audio tracks           */
    io->io_Length =  1;               /* Play this many audio tracks */

    while (opts.tracks[i]) {
        io->io_Data = NULL;
        io->io_Offset = *opts.tracks[i];
        if (!playIt(io)) {
            WaitIO((struct IORequest *)io);  /* wait for cd to finish aborting */
            break;
        }
        i++;
    }

    GoodBye(RETURN_OK);
}

/* playIt ===============================================
   Play track asynchronously, allowing abort.
   Returns:
     TRUE means play the next track, done with the current one.
     FALSE means quit the program.
 */
static BOOL playIt(struct IOStdReq *ios) {

    if (CheckSignal(SIGBREAKF_CTRL_C)) {
        return(FALSE);
    }

    /* send the io request */
    SendIO((struct IORequest *)io);

    /* check the io request and loop */
    while (!CheckIO((struct IORequest *)io)) {

        /* does user want to skip the rest of this track? */
        if (CheckSignal(SIGBREAKF_CTRL_F)) {
            PutStr("Continuing with next track...\n");
            AbortIO((struct IORequest *)io);
            WaitIO((struct IORequest *)io);
            return(TRUE);
        }

        /* does user want to quit out? */
        if (CheckSignal(SIGBREAKF_CTRL_C)) {
            PutStr("Aborting PlayList...\n");
            AbortIO((struct IORequest *)io);
            /* waitio is done by caller on False return */
            return(FALSE);
        }

        /* wait a bit over 0.5 sec before looking again */
        Delay(30L);
    }

    /* remove io from our queue... */
    WaitIO((struct IORequest *)io);

    return(TRUE);
}


/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (rdargs) {
        FreeArgs(rdargs);
    }

    if (cdOpen) {
        CloseDevice((struct IORequest *)io);
    }
    if (io) {
        DeleteIORequest((struct IORequest *)io);
    }
    if (mp) {
        DeleteMsgPort(mp);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    STRPTR unitName;
    ULONG  unitNum;

    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }
    unitName = ((opts.unitName) ? opts.unitName : (STRPTR)DEFAULT_UNIT_NAME);
    unitNum  = ((opts.unitNum) ? *opts.unitNum  : DEFAULT_UNIT_NUMBER);

    mp = CreateMsgPort();
    if (!mp) {
        PrintFault(IoErr(), PROGNAME);
        PutStr("Couldn't create message port!\n");
        return(FALSE);
    }

    io = (struct IOStdReq *)CreateIORequest(mp, sizeof(struct IOStdReq));
    if (!io) {
        PrintFault(IoErr(), PROGNAME);
        PutStr("Couldn't create IOStdRequest!\n");
        return(FALSE);
    }


    if (OpenDevice(unitName, unitNum, (struct IORequest *)io, 0UL)) {
        /* error was returned.  0 == success */
        PrintFault(IoErr(), PROGNAME);
        Printf("Couldn't open %s, unit %lu!\n", unitName, unitNum);
        return(FALSE);
    }
    cdOpen = TRUE;

    return(TRUE);
}

