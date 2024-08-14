

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/interrupts.h>

#include <devices/timer.h>
#include <devices/inputevent.h>

#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/graphics_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <gs:cd/cd.h>

#include "playerlibrary.h"
#include "defs.h"

#include "task_funcs.h"
#include "cd_funcs.h"
#include "playlist_funcs.h"
#include "vfd_funcs.h"



/*******************************************************************************/
/*                                                                             */
/*  GetDiskInfo - Update disk information (disk status has changed)            */
/*                                                                             */
/*******************************************************************************/

void GetDiskInfo(struct V *V) {

struct PlayerState *PlayerState;
int                 error;

    V->CDReq->io_Command = CD_TOCLSN;                                           /* Get table of contents */
    V->CDReq->io_Offset  = 0;
    V->CDReq->io_Length  = 100;
    V->CDReq->io_Data    = (APTR)V->TOC;
    error = DoIO(V->CDReq);

    PlayerState = &V->PlayerBase->PlayerState;

    if (error || ((V->TOC[1].Entry.CtlAdr & CTL_CTLMASK) == CTL_DATA)) {        /* No audio disk, create empty PlayList */

        if (error) PlayerState->AudioDisk = 0;                                  /* Disk not present or not audio disk */
        else       PlayerState->AudioDisk = -1;

        ObtainSemaphore(&V->PlayerBase->PlayListSemaphore);                     /* Create 99 entry PlayList and turn off display */
        PlayerState->Tracks = 0;
        CreatePlayList(V, 99);
        VFDOff(V);
        ReleaseSemaphore(&V->PlayerBase->PlayListSemaphore);
        }

    else {                                                                      /* We have an audio disk, create default PlayList */

        ObtainSemaphore(&V->PlayerBase->PlayListSemaphore);                     /* Create disk's default PlayList */
        CreatePlayList(V, PlayerState->Tracks = V->TOC[0].Summary.LastTrack);
        VFDSize(V);
        PlayerState->AudioDisk  = 1;
        ReleaseSemaphore(&V->PlayerBase->PlayListSemaphore);
        }
    }




/***************************************************************************/
/*                                                                         */
/*  GetQCodePacket - Retrieve current disk position                        */
/*                                                                         */
/***************************************************************************/

int GetQCodePacket(struct V *V) {

    V->CDReq->io_Command = CD_QCODELSN;                                     /* Get Q-Code packet (LSN format) */
    V->CDReq->io_Data    = (APTR)&V->QCodePacket;
    V->CDReq->io_Offset  =
    V->CDReq->io_Length  = 0;

    DoIO(V->CDReq);                                                         /* Do Request */

    return (!V->CDReq->io_Error);
    }





/*******************************************************************************************/
/*                                                                                         */
/*  GetValidQCode - Get a valid Q-Code packet (verify position)                            */
/*                                                                                         */
/*******************************************************************************************/

int GetValidQCode(struct V *V) {

UBYTE CurrentTrack  = GetCurrentItem(V);
ULONG EndTrackPos   = V->TOC[CurrentTrack].Entry.Position.LSN + TrackSize(CurrentTrack, V);

    if (GetQCodePacket(V)) {                                                                /* Get a Q-Code packet */

        if (V->QCodePacket.DiskPosition.LSN == V->InvalidPos.LSN) return(0);                /* Check for not the invalid packet */

        if (V->PlayerBase->PlayerState.PlayMode == PLM_FREV)                                /* In reverse mode? */

            if (V->QCodePacket.DiskPosition.LSN >= EndTrackPos) return(0);                  /* Don't report positions past start */

        V->CurrentPos = V->QCodePacket.DiskPosition.LSN;                                    /* Get current disk position */

        return(1);                                                                          /* Valid Q-Code Packet */
        }

    return(0);                                                                              /* Invalid initial Q-Code packet */
    }




/*******************************************************************************************/
/*                                                                                         */
/*  SkipTrack - Skip forward or backward a track                                           */
/*                                                                                         */
/*******************************************************************************************/

void SkipTrack(UBYTE KeyPress, struct V *V) {

UBYTE Track;

struct PlayerState *PlayerState = &V->PlayerBase->PlayerState;                              /* Quick reference */

    ObtainSemaphore(&V->PlayerBase->PlayListSemaphore);                                     /* Obtain PlayList */
                            
    PlayerState->PlayMode = (KeyPress == PKS_FORWARD ? PLM_SKIPFWD:PLM_SKIPREV);            /* Set PlayMode */

    if (PlayerState->PlayState == PLS_STOPPED | PlayerState->PlayState == PLS_NUMENTRY) {   /* First track selection? */

        if (Track = GetFirstItem(V)) {                                                      /* Get first list item */

            PlayerState->Track     = Track;                                                 /* Select first item if present */
            PlayerState->PlayState = PLS_SELECTED;

            VFD0000(V);                                                                     /* Show disk position if possible */
            }
        }

    else {

        if (PlayerState->PlayMode == PLM_SKIPFWD) Track = GetNextItem(V);                   /* Get next/previous track */
        else                                      Track = GetPreviousItem(V);

        if (Track) {                                                                        /* Next/Previous track exist? */

            PlayerState->Track = Track;                                                     /* Select it */

            VFD0000(V);                                                                     /* Show disk position if possible */
            }
        }

    ReleaseSemaphore(&V->PlayerBase->PlayListSemaphore);                                    /* Release PlayList */
    }





/*******************************************************************************************/
/*                                                                                         */
/*  PlayNextPrevTrack - Play next or previous track.  Stop play if end of playlist         */
/*                                                                                         */
/*******************************************************************************************/

UBYTE PlayNextPrevTrack(struct V *V) {

UBYTE CurrentTrack = GetCurrentItem(V);
UBYTE NextTrack;

    if (V->PlayerBase->PlayerState.PlayMode == PLM_FREV) {                                  /* Are we playing fwd or rev? */

        if (NextTrack = GetPreviousItem(V)) {                                               /* Can we switch to another track? */

            V->PlayerBase->PlayerState.Track = NextTrack;                                   /* Track/time for previous track */
            VFD0000(V);

            V->SQUpdateFreq = SLOWUPDATEFREQ;                                               /* Slow update frequency again */

            if (NextTrack != CurrentTrack - 1) {                                            /* Is the next track in sequence? */

                AbortPlay(V);                                                               /* Seek to next track */
                Play(V);
                }
            }

        else Stop(V);                                                                       /* No previous track, stop playing */
        }

    else {

        if (NextTrack = GetNextItem(V)) {                                                   /* Get next track */

            V->PlayerBase->PlayerState.Track = NextTrack;                                   /* Track/time for next track */
            VFD0000(V);

            V->SQUpdateFreq = SLOWUPDATEFREQ;                                               /* Slow update frequency again */ 

            if (V->PlayerBase->PlayerOptions.Intro || (NextTrack != CurrentTrack + 1)) {    /* Is the next track in sequence? */

                AbortPlay(V);                                                               /* Seek to next track */
                Play(V);
                }
            }

        else Stop(V);                                                                       /* No next track, stop playing */
        }

    return(NextTrack);                                                                      /* Return new track */
    }





/*******************************************************************************************/
/*                                                                                         */
/*  Pause - Pause a play in progress                                                       */
/*                                                                                         */
/*******************************************************************************************/

void Pause(struct V *V) {

    V->CDReq->io_Command = CD_PAUSE;                                                        /* Pause CD */
    V->CDReq->io_Length  = 1;

    DoIO(V->CDReq);                                                                         /* Do Request */

    if (!V->CDReq->io_Error) V->PlayerBase->PlayerState.PlayState = PLS_PAUSED;             /* We are now paused */
    }



/*******************************************************************************************/
/*                                                                                         */
/*  Resume - Resume play after pause                                                       */
/*                                                                                         */
/*******************************************************************************************/

void Resume(struct V *V) {

    V->CDReq->io_Command = CD_PAUSE;                                                        /* Resume Play */
    V->CDReq->io_Length  = 0;

    DoIO(V->CDReq);                                                                         /* Do Request */

    if (!V->CDReq->io_Error) V->PlayerBase->PlayerState.PlayState = PLS_PLAYING;            /* We are now playing */
    }



/*******************************************************************************************/
/*                                                                                         */
/*  FastForward - Switch to fast forward mode                                              */
/*                                                                                         */
/*******************************************************************************************/

void FastForward(struct V *V) {

    V->PlayerBase->PlayerOptions.Intro = 0;                                                 /* Disable intro mode */

    V->CDReq->io_Command = CD_SEARCH;                                                       /* Fast forward mode */
    V->CDReq->io_Length  = CDMODE_FFWD;

    DoIO(V->CDReq);                                                                         /* Do Request */

    V->SQUpdateFreq = QUICKUPDATEFREQ;                                                      /* Update VFD more frequently */

    V->PlayerBase->PlayerState.PlayMode = PLM_FFWD;                                         /* We are now in fast forward mode */
    }



/*******************************************************************************************/
/*                                                                                         */
/*  FastReverse - Switch to fast reverse mode                                              */
/*                                                                                         */
/*******************************************************************************************/

void FastReverse(struct V *V) {

    V->PlayerBase->PlayerOptions.Intro = 0;                                                 /* Disable intro mode */

    AbortPlay(V);                                                                           /* Abort Play */

    V->CDReq->io_Command = CD_SEARCH;                                                       /* Fast reverse mode */
    V->CDReq->io_Length  = CDMODE_FREV;

    DoIO(V->CDReq);                                                                         /* Do Request */

    V->PlayerBase->PlayerState.PlayMode = PLM_FREV;                                         /* We are now in fast reverse mode */

#if DEBUGGING
    if ( V->CDPlayReqOutstanding )
    {
	kprintf("Detected reuse of CDPlay request in FastReverse!\n");
    }
#endif
    V->CDPlayReq->io_Command = CD_PLAYLSN;                                                  /* Play from current position */
    V->CDPlayReq->io_Offset  = V->TOC[1].Entry.Position.LSN;                                /*   to beginning of disk */ 
    V->CDPlayReq->io_Length  = V->CurrentPos - V->TOC[1].Entry.Position.LSN - 1;

    SendIO(V->CDPlayReq);                                                                   /* Send Request */

    V->CDPlayReqOutstanding = 1;                                                            /* We are playing again */

    V->SQUpdateFreq = QUICKUPDATEFREQ;                                                      /* Update VFD more frequently */
    SendSQTimeRequest(V);
    }



/*******************************************************************************************/
/*                                                                                         */
/*  NormalPlay - Switch to normal play mode                                                */
/*                                                                                         */
/*******************************************************************************************/

void NormalPlay(struct V *V) {

    if (V->PlayerBase->PlayerState.PlayState >= PLS_PLAYING) AbortPlay(V);                  /* Abort Play if playing */

    V->CDReq->io_Command = CD_SEARCH;                                                       /* Normal play mode */
    V->CDReq->io_Length  = CDMODE_NORMAL;

    DoIO(V->CDReq);                                                                         /* Do Request */

    V->PlayerBase->PlayerState.PlayMode = PLM_NORMAL;                                       /* We are now in normal play mode */

    if (V->PlayerBase->PlayerState.PlayState >= PLS_PLAYING) {

#if DEBUGGING
	if ( V->CDPlayReqOutstanding )
	{
	    kprintf("Detected reuse of CDPlay request in NormalPlay!\n");
	}
#endif
        V->CDPlayReq->io_Command = CD_PLAYLSN;                                              /* Play from current position */
        V->CDPlayReq->io_Offset  = V->CurrentPos;                                           /*   to end of disk */
        V->CDPlayReq->io_Length  = V->TOC[0].Entry.Position.LSN - V->CurrentPos - 1;

        SendIO(V->CDPlayReq);                                                               /* Send Request */

        V->CDPlayReqOutstanding = 1;                                                        /* We are playing again */

        SendSQTimeRequest(V);                                                               /* Start Q-Code timer */
        }

    V->CDReq->io_Command = CD_PAUSE;                                                        /* Take us out of paused mode */
    V->CDReq->io_Length  = 0;

    DoIO(V->CDReq);                                                                         /* Do Request */

    V->SQUpdateFreq = SLOWUPDATEFREQ;                                                       /* Update VFD every 10th of a sec */
    }




/*******************************************************************************************/
/*                                                                                         */
/*  Play - Begin Play                                                                      */
/*                                                                                         */
/*******************************************************************************************/

void Play(struct V *V) {

int StartTrack;

    StartTrack = GetCurrentItem(V);                                                         /* Get current selected track */

#if DEBUGGING
    if ( V->CDPlayReqOutstanding )
    {
	kprintf("Detected reuse of CDPlay request in Play!\n");
    }
#endif
    V->CDPlayReq->io_Command = CD_PLAYLSN;                                                  /* Play LSN mode */

    if (V->PlayerBase->PlayerState.PlayMode == PLM_FREV) {                                  /* In reverse mode? */

        V->CDPlayReq->io_Offset = V->TOC[1].Entry.Position.LSN;                             /* Play from EOT to BOD */
        V->CDPlayReq->io_Length = V->TOC[StartTrack].Entry.Position.LSN
                                + TrackSize(StartTrack, V)
                                - V->TOC[1].Entry.Position.LSN - 150;
        }

    else {

        V->CDPlayReq->io_Offset = V->TOC[StartTrack].Entry.Position.LSN;                    /* Play from BOT to EOD */
        V->CDPlayReq->io_Length = V->TOC[0].Entry.Position.LSN
                                - V->TOC[StartTrack].Entry.Position.LSN - 1;
        }

    SendIO(V->CDPlayReq);                                                                   /* Send Request */

    if (V->PlayerBase->PlayerState.PlayState != PLS_PAUSED)                                 /* Play request has been sent */

        V->PlayerBase->PlayerState.PlayState = PLS_PLAYING;

    V->CDPlayReqOutstanding = 1;

    V->SQUpdateFreq = SLOWUPDATEFREQ;                                                       /* Begin VFD updating */
    SendSQTimeRequest(V);
    }



/*******************************************************************************************/
/*                                                                                         */
/*  AbortPlay - Abort a play (do not change PLAYSTATE)                                     */
/*                                                                                         */
/*******************************************************************************************/

void AbortPlay(struct V *V) {

    if (V->CDPlayReqOutstanding) {                                                          /* Is there a request to abort? */

        AbortIO(V->CDPlayReq);                                                              /* Abort IO Request */

        Wait(1L << V->CDPlayReplyPort->mp_SigBit);                                          /* Wait for message to come back */
        while (GetMsg(V->CDPlayReplyPort));

        V->CDPlayReqOutstanding = 0;                                                        /* No outstanding play request */

        AbortSQTimeRequest(V);                                                              /* Halt VFD updating */
        }
    }



/*******************************************************************************************/
/*                                                                                         */
/*  Stop - Stop Playing                                                                    */
/*                                                                                         */
/*******************************************************************************************/

void Stop(struct V *V) {

    /* AbortPlay() only aborts if there's an outstanding request */
    AbortPlay(V);

    ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);                                /* Obtain PlayState semaphore */

    V->PlayerBase->PlayerOptions.Intro   = 0;                                           /* Turn off intro mode, reset index */
    V->PlayerBase->PlayerState.ListIndex = 0;

    V->PlayerBase->PlayerState.PlayState = PLS_STOPPED;                                 /* We are now stopped */

    NormalPlay(V);                                                                      /* Normal play mode */

    VFDSize(V);                                                                         /* Display disk size */

    ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);                               /* Release PlayState semaphore */

    if (V->PlayListAllocated) {                                                         /* Free PlayList if allocated */

        ReleaseSemaphore(&V->PlayerBase->PlayListSemaphore);                            /* Release PlayList */
        V->PlayListAllocated = 0;
        }
    }


