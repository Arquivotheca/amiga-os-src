

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


/****************************************************************************/
/*                                                                          */
/*  LSNtoMSF - Convert LSN value to an MSF value                            */
/*                                                                          */
/****************************************************************************/

void LSNtoMSF(union LSNMSF *Time) {

ULONG LSN;

    LSN = Time->LSN;                                                         /* Temporary LSN space */

    Time->MSF.Frame    = LSN % 75;                                           /* Convert to MSF */
    Time->MSF.Second   = (LSN /=75) % 60;
    Time->MSF.Minute   = LSN / 60;
    Time->MSF.Reserved = 0;
    }



/***************************************************************************************/
/*                                                                                     */
/*  ComputeTime - Calculate total play time within a specified playlist range          */
/*                                                                                     */
/***************************************************************************************/

void ComputeTime(int StartIndex, int EndIndex, struct CompTime *CompTime, struct V *V) {

struct PlayList *PlayList;
UBYTE            Entry;
int              i;

    PlayList = &V->PlayerBase->PlayList;                                                /* Quick reference */

    CompTime->Tracks   = 0;                                                             /* Reset accumulators */
    CompTime->Time.LSN = 0L;

    for (i=StartIndex; i<EndIndex; i++) {                                               /* Accumulate track information */

        if ((Entry = PlayList->Entry[i]) & PLEF_ENABLE) {

            Entry &= PLEF_TRACK;                                                        /* Mask off enable bit */

            if (CompTime->Time.LSN != 452925) {                                         /* Don't accumulate past 99:59 */

                CompTime->Time.LSN += TrackSize(Entry, V);                              /* Add track size to total time */
                }

            if (CompTime->Time.LSN >= 450000) CompTime->Time.LSN = 452925;              /* Overflow means 99:99 */

            CompTime->Tracks++;                                                         /* Accumulate track count */
            }
        }

    }



/***************************************************************************/
/*                                                                         */
/*  VFDOff - Blank track/minutes/seconds fields of time display            */
/*                                                                         */
/***************************************************************************/

void VFDOff(struct V *V) {

    V->PlayerBase->PlayerState.Track  =                                     /* Turn off Track/Time display */
    V->PlayerBase->PlayerState.Minute =
    V->PlayerBase->PlayerState.Second = 100;
    V->PlayerBase->PlayerState.Minus  = 0;
    }



/***************************************************************************/
/*                                                                         */
/*  VFDSize - Display total PlayList time                                  */
/*                                                                         */
/***************************************************************************/

void VFDSize(struct V *V) {

struct CompTime CompTime;

    V->PlayerBase->PlayerState.ListIndex = 0;                               /* Reset current track index */

    ComputeTime(0, V->PlayerBase->PlayList.EntryCount, &CompTime, V);       /* Calculate total play time of PlayList */
    V->TotalPlayTime = CompTime.Time.LSN;

    if (CompTime.Time.LSN == 452925) {                                      /* Overflow? */

        CompTime.Time.MSF.Minute = 99;                                      /* Display 99:99 */
        CompTime.Time.MSF.Second = 99;
        }

    else LSNtoMSF(&CompTime.Time);                                          /* Convert to MSF format */

    if (CompTime.Tracks == 100) CompTime.Tracks = 0;                        /* If 100 tracks are enabled, display 0 */

    ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);

    V->PlayerBase->PlayerState.Track  = CompTime.Tracks;                    /* Report total play time of PlayList */
    V->PlayerBase->PlayerState.Minute = CompTime.Time.MSF.Minute;
    V->PlayerBase->PlayerState.Second = CompTime.Time.MSF.Second;
    V->PlayerBase->PlayerState.Minus  = 0;

    ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);
    }



/***********************************************************************************/
/*                                                                                 */
/*  VFD0000 - Display total PlayList time                                          */
/*                                                                                 */
/***********************************************************************************/

void VFD0000(struct V *V) {

UBYTE           CurrentTrack = GetCurrentItem(V);

struct CompTime CompTime;
union LSNMSF    Time;
int             Mode;
BOOL            negative = FALSE;

    if (V->PlayerBase->PlayerState.AudioDisk <= 0) return;                          /* Can't display disk position if no disk */

    ComputeTime(0, V->PlayerBase->PlayerState.ListIndex, &CompTime, V);             /* Compute play time to beginning of track */
    V->BegTrackPlayTime = CompTime.Time.LSN;

    Mode = V->PlayerBase->PlayerOptions.TimeMode;                                   /* Retrieve current time mode */

    if (V->PlayerBase->PlayerState.PlayMode == PLM_FREV) {                          /* Forward or Reverse mode? */

        V->CurrentPos = V->TOC[CurrentTrack].Entry.Position.LSN                     /* Current position = start position (REV) */
                      + TrackSize(CurrentTrack, V)
                      - 1;

        Mode += 4;                                                                  /* In reverse mode, adjust time calculation */
        }

    else V->CurrentPos = V->TOC[CurrentTrack].Entry.Position.LSN;                   /* Current position = start position (FWD) */

    switch (Mode) {

        case 5:                                                                     /* - Track relative (FREV) */
            negative = TRUE;
	    /* Fall through */
        case 0:                                                                     /* Track relative */

            Time.LSN = 0L;
            break;

        case 1:                                                                     /* - Track relative */
            negative = TRUE;
	    /* Fall through */
        case 4:                                                                     /* Track relative (FREV) */

            Time.LSN = TrackSize(CurrentTrack, V) - 1;
            break;

        case 2:                                                                     /* Disk relative */

            Time.LSN = V->BegTrackPlayTime;
            break;

        case 3:                                                                     /* - Disk relative */

            ComputeTime(0, V->PlayerBase->PlayList.EntryCount, &CompTime, V);

            Time.LSN = CompTime.Time.LSN - V->BegTrackPlayTime;
            negative = TRUE;
            break;

        case 6:                                                                     /* Disk relative (FREV) */

            Time.LSN = V->BegTrackPlayTime + TrackSize(CurrentTrack, V) - 1;
            break;

        case 7:                                                                     /* - Disk relative (FREV) */

            ComputeTime(0, V->PlayerBase->PlayList.EntryCount, &CompTime, V);

            Time.LSN = CompTime.Time.LSN - V->BegTrackPlayTime
                     - TrackSize(CurrentTrack, V) + 1;
            negative = TRUE;
            break;
        }

    LSNtoMSF(&Time);                                                                /* Convert to MSF format */

    ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);

    V->PlayerBase->PlayerState.Minute = Time.MSF.Minute;                            /* Store time */
    V->PlayerBase->PlayerState.Second = Time.MSF.Second;
    V->PlayerBase->PlayerState.Minus  = negative;

    ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);
    }




/*******************************************************************************************/
/*                                                                                         */
/*  UpdateVFDPosition - Update time field in PlayerState structure                         */
/*                                                                                         */
/*******************************************************************************************/

void UpdateVFDPosition(struct V *V) {

UBYTE CurrentTrack = GetCurrentItem(V);
ULONG TrackIntPosF = V->TOC[CurrentTrack].Entry.Position.LSN + TrackSize(CurrentTrack, V);
ULONG TrackIntPosR = V->TOC[CurrentTrack].Entry.Position.LSN;

union LSNMSF Time;
ULONG        RTime;
ULONG	     skipped = 0;
BOOL         negative = FALSE;

    if ( V->CurrentPos >= V->TOC[CurrentTrack].Entry.Position.LSN ) {
	RTime = V->CurrentPos - V->TOC[CurrentTrack].Entry.Position.LSN;                        /* Calculate relative track time */
	}
    else {
	/* Something must have jarred the player, since the current time
	 * is less than the time at the start of the current track.
	 */
	RTime = V->TOC[CurrentTrack].Entry.Position.LSN - V->CurrentPos;                        /* Calculate relative track time */
	skipped = 1;
	}


    if (V->PlayerBase->PlayerState.PlayMode == PLM_FREV) {                                  /* Are we in reverse mode? */

        if (V->CurrentPos <= TrackIntPosR + 20) V->SQUpdateFreq = QUICKUPDATEFREQ;          /* Do quick update if close to EOT */

        if (V->CurrentPos <= TrackIntPosR) {                                                /* Time to switch to next track? */

            PlayNextPrevTrack(V);
            return;
            }
        }

    else {

        if (V->CurrentPos >= TrackIntPosF - 20) V->SQUpdateFreq = QUICKUPDATEFREQ;          /* Do quick update if close to EOT */

        if ((V->PlayerBase->PlayerOptions.Intro) && (skipped == 0) && (RTime >= 749))                             /* Are we in intro mode and 00:10 past? */

            V->CurrentPos = TrackIntPosF;

        if (V->CurrentPos >= TrackIntPosF) {                                                /* Time to switch to next track? */

            PlayNextPrevTrack(V);                                                           /* Switch away */
            return;
            }
        }

    if ( !skipped ) {
	switch (V->PlayerBase->PlayerOptions.TimeMode) {

	    case 0:                                                                             /* Track relative */
	        Time.LSN = RTime;
	        break;

	    case 1:                                                                             /* - Track relative */
	        Time.LSN = TrackSize(CurrentTrack, V) - 1 - RTime;
	        negative = TRUE;
	        break;

	    case 2:                                                                             /* Disk relative */
	        Time.LSN = V->BegTrackPlayTime + RTime;
	        break;

	    case 3:                                                                             /* - Disk relative */
	        Time.LSN = V->TotalPlayTime - 1 - (V->BegTrackPlayTime + RTime);
	        negative = TRUE;
	        break;
	    }
	} else {
	/* A skip happened due to physical jarring.  In this case, the variable RTime
	 * actually represents a (small, we hope) negative number.  Since it's a ULONG,
	 * RTime is actually the magnitude of that number, so we flip the sign of
	 * RTime before using it here.
	 */
	switch (V->PlayerBase->PlayerOptions.TimeMode) {

	    case 0:                                                                             /* Track relative */
		/* Can't flip the sign since LSN must be >= 0, so we set the negative flag */
	        Time.LSN = RTime;
	        negative = TRUE;
	        break;

	    case 1:                                                                             /* - Track relative */
	        Time.LSN = TrackSize(CurrentTrack, V) - 1 + RTime;
	        negative = TRUE;
	        break;

	    case 2:                                                                             /* Disk relative */
	        Time.LSN = V->BegTrackPlayTime - RTime;
	        break;

	    case 3:                                                                             /* - Disk relative */
	        Time.LSN = V->TotalPlayTime - 1 - (V->BegTrackPlayTime - RTime);
	        negative = TRUE;
	        break;
	    }
	}


    LSNtoMSF(&Time);                                                                        /* Convert to MSF format */

    ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);

    V->PlayerBase->PlayerState.Minute = Time.MSF.Minute;                                    /* Store time */
    V->PlayerBase->PlayerState.Second = Time.MSF.Second;
    V->PlayerBase->PlayerState.Minus  = negative;

    ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);
    }





