/*
 * This is a set of functions that allow me to play a CD
 *
 * Written by Greg Givler © Feb. 1992
 *
 * Function included:
 *      getfirsttrack()
 *      getlasttrack()
 *      getTotalTime()
 *      getTOC()
 *      freeTOC()
 *      isrom()
 *      playTrack()
 *      stopPlay()
 *      playNextTrack()
 *      pausePlay()
 *      myrandom()
 *      randomPlay()
 *      setupInt()
 *      getFrameRate()
 *      trackTime()
 *
 */

#include "cdplay.h"

static VOID dumpIO(VOID);

LONG getfirsttrack(struct CDTOC *toc)
{
    int tocfirst;
    /* This will get the first track from the IOStdReq */
    Printf("getfirsttrack($%08lx)\n", toc);
    tocfirst = toc[0].Track;
    return(tocfirst);
}

LONG getlasttrack(struct CDTOC *toc)
{
    /* This will get the first track from the IOStdReq */
    int toclast;

    Printf("getlasttrack(struct CDTOC $%08lx)\n", toc);
    toclast = toc[0].LastTrack;
    return(toclast);
}

LONG getTotalTime(struct CDTOC *toc)
{
    /* This will get figure and store the Totaltime in a RMSF struct
    */
    int firstmin, firstsec, firstframe, lastmin, lastsec, lastframe;
    int totsec, totalsec;

    Printf("getTotalTime($%08lx)\n", toc);

    firstmin = toc[0].Position.MSF.Minute;
    firstsec = toc[0].Position.MSF.Second;
    firstframe = toc[0].Position.MSF.Frame;
    lastmin = toc[1].Position.MSF.Minute;
    lastsec = toc[1].Position.MSF.Second;
    lastframe = toc[1].Position.MSF.Frame;

    if (lastframe > 0 ) {
        lastsec = lastsec + 1;
        if (lastsec > 60) {
            firstmin = firstmin+1;
            lastsec = lastsec - 60;
        }
    }
    if (firstsec > lastsec) {
        totsec = firstsec - lastsec;
    }
    else {
        totsec = lastsec - firstsec;
    }
    totalsec = (firstmin*60)+totsec;

    return(totalsec);
}

struct CDTOC *getTOC(ULONG start, ULONG entries)
{
    struct CDTOC *toc;
    BYTE error;

    Printf("getTOC(%lu, %lu)\n", start, entries);

    /* This will allocate a CDTOC array using AllocVec() and fill it
    in using the CDTV_TOCMSF command of cdtv.device */

    toc = AllocVec(sizeof(struct CDTOC)*entries, MEMF_CLEAR);
    if (!toc) {
        return(NULL);
    }

    IOStdReq->io_Command=CDTV_TOCMSF;
    IOStdReq->io_Data=toc;
    IOStdReq->io_Length=entries;
    IOStdReq->io_Offset=start;

    Printf("DoIO in getTOC()...\n");
    dumpIO();
    error = DoIO(IOStdReq);
    Printf("Back from DoIO in getTOC()...\n");

    if (error) {
        Printf("Attempt to get TOC returned %ld\n", error);
        FreeVec(toc);
        return(NULL);
    }

    return(toc);
}

void freeTOC(APTR toc)
{
    Printf("freeTOC($%08lx)\n", toc);
    /* This will free a CDTOC array returned by getTOC -- using
    FreeVec() */
    FreeVec(toc);
}

int isrom(void)
{
    int romret;
    BYTE error;

    Printf("ISROM....");

    /* This will find out if it is a rom or not */
    IOStdReq->io_Command=CDTV_ISROM;
    IOStdReq->io_Data=NULL;
    IOStdReq->io_Length=0;
    IOStdReq->io_Offset=0;

    error = DoIO(IOStdReq);

    if (!error) {
        romret=IOStdReq->io_Actual;
    }

    Printf("returned %ld\n", romret);
    return(romret);

}

void playTrack(int start, int end)
{
    BYTE error;
    /* this function will play a CD starting at start and playing to
    end */

    Printf("PlayTrack ... %ld %ld\n", start, end);

    IOStdReq->io_Command=CDTV_PLAYTRACK;
    IOStdReq->io_Data=NULL;
    IOStdReq->io_Offset=start;
    IOStdReq->io_Length=end;

    error = DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }
}
void stopPlay()
{
    BYTE error;

    /* This function will stop the CD from playing. */

    Printf("stopPlay...\n");
    IOStdReq->io_Command=CDTV_STOPPLAY;
    IOStdReq->io_Command=NULL;
    IOStdReq->io_Length=0;
    IOStdReq->io_Offset=0;

    error = DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }
}
void playNextTrack(struct CDTOC *toc, int intro, int currenttrack, int nexttrack)
{
    int currentmin,nextmin,currentsec,nextsec,currentframe,nextframe;
    int lasttrack;
    BYTE error;


    Printf("playNextTrack(%08lx, %ld, %ld, %ld)\n", toc, intro, currenttrack, nexttrack);

    /* This sets everything up to be inserted in the PLAYMSF command
     */
    lasttrack =toc[0].LastTrack;
    if (lasttrack == currenttrack) {
        nexttrack = 0; /* this will get the totaltime on the disk */
    }
    currentmin=toc[currenttrack].Position.MSF.Minute;
    currentsec=toc[currenttrack].Position.MSF.Second;
    currentframe=toc[currenttrack].Position.MSF.Frame;


    if (intro == FALSE) {
        nextsec=toc[nexttrack].Position.MSF.Second;
        nextmin=toc[nexttrack].Position.MSF.Minute;
        nextframe=toc[nexttrack].Position.MSF.Frame;
    }
    else {
        nextsec= currentsec+10;
        nextmin= currentmin;
        nextframe = currentframe;
        if (nextsec > 60) {
            nextmin = currentmin+1;
            nextsec = nextsec-60;
        }

    }

    /* start debug printfs */
    printf("currenttrack = %d:%02d:%02d nexttrack = %d:%02d:%02d\n",currentmin,currentsec,currentframe,nextmin,nextsec,nextframe);
    /* end debug */

    /* Now to fill in the play command */
    IOStdReq->io_Command=CDTV_PLAYMSF;
    IOStdReq->io_Data=NULL;
    IOStdReq->io_Offset=TOMSF(currentmin,currentsec,currentframe);
    IOStdReq->io_Length=TOMSF(nextmin,nextsec,nextframe);

    error = DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }
}
void pausePlay()
{
    static int pause;
    BYTE error;

    Printf("Paws!\n");

    if (pause==0) {
        pause = 1;
        printf("Play is paused\n");
    }
    else {
        pause = 0;
        printf("Continuing Play\n");
    }

    IOStdReq->io_Command=CDTV_PAUSE;
    IOStdReq->io_Data=NULL;
    IOStdReq->io_Length=pause;
    IOStdReq->io_Offset=0;

    error = DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }

}

int myrandom(int range)
{
    short seed[3];
    long ti;
    double y;

    Printf("myrandom %ld\n", range);

    /* this is the random number stuff */
    ti = time(NULL);

    seed[0] = ti>>16;
    seed[1] = ti & 0xffff;
    seed[2] = 0;

    y = erand48(seed);
    y = (y*range)+1;

    return((int) y);
}

void randomPlay(struct CDTOC *toc, int intro, int last)
{
    /* this will shuffle play the disk */
    /* rightnow this function does not play anything. */

    int order[100], random[100];
    int i, getorder, range;

    Printf("RandomPlay($%08lx, %ld %ld)\n", toc, intro, last);

    /* first load the array the first member of the array has the
    total count */

    order[0] = last;

    for (i=1;i<(last+1);i++) {
        order[i] = i;
    }

    /* now to load the random play tracks */

    for (i=1;i<(last+1);i++) {
        range = order[0];
        getorder = myrandom(range);
        random[i] = order[getorder];
        if (getorder != last) {
            order[getorder] = order[range];
            order[0] = order[0] - 1;
        }
        else {
            order[0] = order[0] - 1;
        }
    }
    for (i=1 ; i < (last+1) ; i++) {
        printf("Random Playing of track %d\n", random[i]);
        playNextTrack(toc, intro, random[i],random[i]+1);
    }
}
#ifdef BREAK_THE_COMPILER
int __saveds __asm trackTime()
{
    int tracksectors;
    struct CDSubQ *subq;
    BYTE error;

    /* This function will return the total seconds played in the
    track. */
    IOStdReq->io_Command=CDTV_SUBQLSN;
    IOStdReq->io_Data=subq;
    IOStdReq->io_Length=0;
    IOStdReq->io_Offset=0;

    error=DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }
    tracksectors = subq.TrackPosition.LSN;
    return(tracksectors);
}
int getFrameRate()
{
    int framerate;
    BYTE error;

    /* Get the framerate to set in the setupInt routine */

    IOStdReq->io_Command=CDTV_INFO;
    IOStdReq->io_Data=NULL;
    IOStdReq->io_Length=0;
    IOStdReq->io_Offset=CDTV_INFO_FRAME_RATE;

    error = DoIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }

    framerate = IOStdReq->io_Actual;

    return(framerate);

}
void setupInt(int framerate)
{
    BYTE error;

    /* Let's see if this works. */
    IOStdReq->io_Command=CDTV_FRAMECALL;
    IOStdReq->io_Data=trackTime;
    IOStdReq->io_Length=0;
    IOStdReq->io_Offset=framerate;

    error = SendIO(IOStdReq);
    if (error) {
        CleanUp(RETURN_FAIL);
    }

}

/*
 *
 * This may not be needed.
 *
 * int trackTime(struct CDTOC *toc)
 * {
 *     int trackmin, tracksec;
 *     struct CDSubQ *subq;
 *     BYTE error;
 *
 *     /* This function will return the total seconds played in the
 *     track. */
 *     IOStdReq->io_Command=CDTV_SUBQLSN;
 *     IOStdReq->io_Data=subq;
 *     IOStdReq->io_Length=0;
 *     IOStdReq->io_Offset=0;
 *
 *     error=DoIO(IOStdReq);
 *     if (error) {
 *         CleanUp(RETURN_FAIL);
 *     }
 *     tracksectors = subq.TrackPosition.LSN
 *     trackmin=subq.TrackPosition.MSF.Minute
 *     tracksec=subq.TrackPosition.MSF.Second
 *
 *     printf("Time into track: %02d:%02d\n›2A" trackmin, tracksec);
 *
 * }
 */
#endif /* BREAK_THE_COMPILER */

static VOID dumpIO(VOID) {
    struct IOStdReq *i;

    i = IOStdReq;

    Printf("struct  Device  *io_Device=%lx", i->io_Device);
    Printf("struct  Unit    *io_Unit=$%08lx\n", i->io_Unit);
    Printf("UWORD   io_Command=$%04lx\n", (ULONG)(i->io_Command));
    Printf("UBYTE   io_Flags=$%08lx\n", (ULONG)(i->io_Flags));
    Printf("BYTE    io_Error=$%08lx\n", (ULONG)(i->io_Error));
    Printf("ULONG   io_Actual=$%08lx\n", (ULONG)(i->io_Actual));
    Printf("ULONG   io_Length=$%08lx\n", (ULONG)(i->io_Length));
    Printf("APTR    io_Data=$%08lx\n", (ULONG)(i->io_Data));
    Printf("ULONG   io_Offset=$%08lx\n", (ULONG)(i->io_Offset));

}

