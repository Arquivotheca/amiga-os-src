/*
 * CDPlay
 * Written by Gregory P. Givler
 *
 * Copyright © Feb. 1992 Gregory P. Givler
 *
 * The purpose of this program is to create a program that will play the
 * A570 CD Player from Workbench.
 *
 * Planned Features:
 *
 * o All CDTV DA panel functionality execpt CD+G and CD+MIDI
 *      Have implemented:
 *
 *          [X] Play Selected Tracks - I use Read Args, will use track
 *              grid on GUI
 *
 *          [X] Play Random Tracks
 *          [X] Play Intro of Tracks
 *          [X] Play Intro of Random Tracks
 *          [X] Play Intro of Selected Tracks
 *          [ ] Loop whole disk
 *          [ ] Loop one track
 *          [ ] Loop section of track
 *
 * o A catalog of CD programming, the player will keep a stored file
 *   that knows how each CD is programmed.
 *
 * o Elapsed time clock.
 *
 */

/*
 *
 * CleanUp and Exit
 *
 */

#define MAIN

#include "cdplay.h"

void CleanUp(WORD ReturnCode)
{
    if (CDTVOpen)
        CloseDevice(IOStdReq);

    if (IOStdReq)
        DeleteStdIO(IOStdReq);

    if (MsgPort)
        DeletePort(MsgPort);

    exit(ReturnCode);
}

/*
 *
 * Main
 *
 */

void main(int argc,char *argv[])
{

    BYTE error;
    int firsttrack,lasttrack;
    LONG rom, i, j, Opts[OPT_COUNT], intro = FALSE;
    LONG totalmin, totalseconds, totaltime, *tracks, trackno;
    struct CDTOC *tocarray;
    struct RDArgs *argsptr = NULL; /* struct for Read Args */

    /* Allocate memory for Read Args */
    memset(Opts, 0, sizeof(Opts));

    /* Create a Message Port */
    MsgPort=CreatePort(NULL,0L);
    if (!MsgPort) {
        printf("Couldn't create message port\n");
        CleanUp(RETURN_FAIL);
    }

    /* Create a Standard IO request */

    IOStdReq=CreateStdIO(MsgPort);
    if (!IOStdReq) {
        printf("Couldn't Create IO Request\n");
        CleanUp(RETURN_FAIL);
    }

    /* Open cdtv.device */

    error=OpenDevice("cdtv.device",CDTV_UNIT,
        IOStdReq, CDTV_FLAGS);
    if (error) {
        printf("CDTV device open eror = %d\n", error);
        CleanUp(RETURN_FAIL);
    }

    CDTVOpen=TRUE;

    /* Get a toc */

    tocarray = getTOC(TOC_START,TOC_ENTRIES);
    if (!tocarray) {
        printf("Could not find tocarray!\n");
    }
    firsttrack = getfirsttrack(tocarray);
    lasttrack = getlasttrack(tocarray);
    totaltime = getTotalTime(tocarray);
    totalmin = totaltime/60;
    totalseconds = totaltime - totalmin*60;

    /* Check to see if this a CD-ROM if it is exit */
    rom = isrom();
    if (rom == TRUE && lasttrack == 1) {
        CleanUp(RETURN_FAIL);
    }


    printf("Rom is %d\nFirstTrack = %d\nLastTrack = %d\n",rom,firsttrack,lasttrack);
    printf("Total Time on disk = %d:%02d\n",totalmin,totalseconds);

    /* Now to check the command line */

    argsptr = ReadArgs(TEMPLATE, Opts, NULL);
    if (argsptr == NULL) {
        PrintFault(IoErr(),"ReadArgs");
        CleanUp(RETURN_FAIL);
    }
    if (Opts[OPT_TRACKS] == NULL) {

        if (Opts[OPT_RANDOM] == NULL) {

            /* This should play the whole disk */
            if (Opts[OPT_INTRO] != NULL) {
                intro = TRUE;
            }
            for (i = firsttrack; i <= lasttrack; i++) {  /* d,think about <= lasttrack */
                printf("Playing track %d\n",i);

                if (i == lasttrack && i+1 >= lasttrack) {
                    j = 0;
                }
                else {
                    j = i+1;
                }

                playNextTrack(tocarray, intro, i, j);
            }
            stopPlay();
        }
        else {
            printf("entering randomPlay, lasttrack = %d\n",lasttrack);
            randomPlay(tocarray, intro, lasttrack);
            stopPlay();
        }
    }
    else {
        /* This will play the tracks in the order from readargs */
        tracks = *(LONG **)Opts[OPT_TRACKS];
        for (trackno = 0; tracks[trackno] != NULL ; trackno++) {

            printf("Playing track %ld\n", tracks[trackno]);
            if (tracks[trackno]+1 > lasttrack) {
                playNextTrack(tocarray, intro, tracks[trackno], 0);
            }
            else {
                playNextTrack(tocarray, intro, tracks[trackno], tracks[trackno]+1);
            }
        }
        stopPlay();
    }

    /* Free the memory used for the ReadArgs struct */
    if (argsptr) {
        FreeArgs(argsptr);
    }

    CleanUp(RETURN_OK);

}