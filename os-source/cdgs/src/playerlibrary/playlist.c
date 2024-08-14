

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



/*******************************************************************************************/
/*                                                                                         */
/*  TrackSize - Determine size of track (in frames)                                        */
/*                                                                                         */
/*******************************************************************************************/

ULONG TrackSize(UBYTE Entry, struct V *V) {
                
ULONG Size;

    if (Entry == V->TOC[0].Summary.LastTrack) Size = V->TOC[0].Summary.LeadOut.LSN           /* Determine size of track */
                                                   - V->TOC[Entry].Entry.Position.LSN;
    else                                      Size = V->TOC[Entry + 1].Entry.Position.LSN
                                                   - V->TOC[Entry].Entry.Position.LSN;
    return(Size);
    }



/*******************************************************************************/
/*                                                                             */
/*  CreatePlayList - Create default playlist of length (ListCount)             */
/*                                                                             */
/*******************************************************************************/

void CreatePlayList(struct V *V, UBYTE ListCount) {

struct PlayerState *PlayerState;
struct PlayList    *PlayList;
int                 i;

    ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);

    PlayerState = &V->PlayerBase->PlayerState;                                  /* Quick references */
    PlayList    = &V->PlayerBase->PlayList;

    PlayerState->ListIndex  =                                                   /* Set default PlayerStates */
    PlayerState->PlayMode   =
    PlayerState->PlayState  = 0;
    PlayerState->LastModify = 1;

    for (i=0; i!=ListCount;) PlayList->Entry[i] = ++i | PLEF_ENABLE;            /* Create PlayList */
    PlayList->EntryCount = i;

    ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);
    }



/*******************************************************************************************/
/*                                                                                         */
/*  GetNextItem - Get next item in PlayList (return 0 if end of PlayList)                  */
/*                                                                                         */
/*******************************************************************************************/

UBYTE GetNextItem(struct V *V) {

UBYTE Item;
int   i, j;

struct PlayList *PlayList = &V->PlayerBase->PlayList;                                       /* Quick reference */

    for (i=0,j=V->PlayerBase->PlayerState.ListIndex; i!=PlayList->EntryCount; i++) {        /* Search through all entries */

        if (++j == PlayList->EntryCount) {                                                  /* Check for wrap condition */

            if (V->PlayerBase->PlayerOptions.Loop) j = 0;                                   /* Wrap only if LoopMode is enabled */
            else                                   break;
            }

        if ((Item = PlayList->Entry[j]) & PLEF_ENABLE) {                                    /* Is this track enabled? */

            V->PlayerBase->PlayerState.ListIndex = j;                                       /* This is our newly selected track */
            return((UBYTE)(Item & PLEF_TRACK));
            }
        }

    return(0);                                                                              /* No enabled tracks in list */
    }



/*******************************************************************************************/
/*                                                                                         */
/*  GetPreviousItem - Get previous item in PlayList (return 0 if end of PlayList)          */
/*                                                                                         */
/*******************************************************************************************/

UBYTE GetPreviousItem(struct V *V) {

UBYTE Item;
int   i, j;

struct PlayList *PlayList = &V->PlayerBase->PlayList;                                       /* Quick reference */

    for (i=0,j=V->PlayerBase->PlayerState.ListIndex; i!=PlayList->EntryCount; i++) {        /* Search through all entries */

        if (--j < 0) {                                                                      /* Check for wrap condition */

            if (V->PlayerBase->PlayerOptions.Loop) j = PlayList->EntryCount-1;              /* Wrap only if LoopMode is enabled */
            else                                   break;
            }

        if ((Item = PlayList->Entry[j]) & PLEF_ENABLE) {                                    /* Is this track enabled? */

            V->PlayerBase->PlayerState.ListIndex = j;                                       /* This is our newly selected track */
            return((UBYTE)(Item & PLEF_TRACK));
            }
        }

    return(0);                                                                              /* No enabled tracks in list */
    }



/***************************************************************************/
/*                                                                         */
/*  GetFirstItem - Get first item in PlayList (return 0 if PlayList empty) */
/*                                                                         */
/***************************************************************************/

UBYTE GetFirstItem(struct V *V) {

UBYTE Item;
int   i;

struct PlayList *PlayList = &V->PlayerBase->PlayList;                       /* Quick reference */

    for (i=0; i!=PlayList->EntryCount; i++) {                               /* Search through all entries */

        if ((Item = PlayList->Entry[i]) & PLEF_ENABLE) {                    /* Is this track enabled? */

            V->PlayerBase->PlayerState.ListIndex = i;                       /* This is our newly selected track */
            return((UBYTE)(Item & PLEF_TRACK));
            }
        }

    return(0);                                                              /* No enabled tracks in list */
    }



/*******************************************************************************************************/
/*                                                                                                     */
/*  GetCurrentItem - Get current item being used in PlayList                                           */
/*                                                                                                     */
/*******************************************************************************************************/

UBYTE GetCurrentItem(struct V *V) {

    return((UBYTE)(V->PlayerBase->PlayList.Entry[V->PlayerBase->PlayerState.ListIndex] & PLEF_TRACK));  /* Return current item */
    }




