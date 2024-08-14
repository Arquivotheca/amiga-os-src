
#include <exec/memory.h>
#include <string.h>
#include <stddef.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "realtimebase.h"
#include "timerinit.h"


/*****************************************************************************/


typedef VOID ASM (*STATEFUNC)(REG(a0) struct Hook *, REG(a1) struct pmState *, REG(a2) struct ExtPlayer *);
typedef VOID ASM (*TIMEFUNC)(REG(a0) struct Hook *, REG(a1) struct pmTime *, REG(a2) struct ExtPlayer *);


/*****************************************************************************/


/* notify_players - Calls Player hooks to inform of Conductor play
 *	            state change.
 */

static VOID notify_players(struct ExtConductor *cond, ULONG oldState)
{
struct ExtPlayer *player;
struct pmState     msg;

    msg.pms_Method   = PM_STATE;
    msg.pms_OldState = oldState;

    SCANLIST(&cond->ec_Players,player)
    {
        if (player->ep_Hook)
            ((STATEFUNC)player->ep_Hook->h_Entry)(player->ep_Hook,&msg,player);
    }

}


/*****************************************************************************/


/* clear_ready -  Clears ready bits of Player's with hooks. */

static VOID clear_ready(struct ExtConductor *cond)
{
struct ExtPlayer *player;

    cond->ec_Flags &= ~CONDUCTF_GOTTICK;

    if (cond->ec_Flags & CONDUCTF_EXTERNAL)
	cond->ec_Stopped |= CONDSTOPF_NOTICK;

    SCANLIST(&cond->ec_Players,player)
    {
	if (player->ep_Hook)
            player->ep_Flags &= ~PLAYERF_READY;
    }
}


/*****************************************************************************/


/* check_ready - Checks ready bits to see if should go from LOCATING to
 *	         RUNNING.
 */

static VOID check_ready(struct ExtConductor *cond)
{
struct ExtPlayer *player;

    if (cond->ec_State == CONDSTATE_LOCATE)
    {
	SCANLIST(&cond->ec_Players,player)
	{
	    if (player->ep_Hook && !(player->ep_Flags & PLAYERF_READY))
                return;
	}

	cond->ec_State    = CONDSTATE_RUNNING;
	cond->ec_Stopped &= ~CONDSTOPF_STOPPED;
	notify_players(cond,CONDSTATE_LOCATE);
    }
}


/*****************************************************************************/


/* notify_conducted_players - Calls Player hooks to inform of Conductor
 * 			      play state change
 */

static VOID notify_conducted_players(struct ExtPlayer *maestro, ULONG oldState)
{
struct ExtPlayer *player;
struct pmState     msg;

    msg.pms_Method   = PM_STATE;
    msg.pms_OldState = oldState;

    SCANLIST(&maestro->ep_Link,player)
    {
	if (player->ep_Flags & PLAYERF_CONDUCTED)
            ((STATEFUNC)player->ep_Hook->h_Entry)(player->ep_Hook,&msg,player);
    }

}


/*****************************************************************************/


/* notify_locate_players - Calls Player hooks to inform of Conductor play
 *	                   state change
 */

static VOID notify_locate_players(struct ExtConductor *cond, ULONG oldState)
{
struct ExtPlayer *player;
struct pmState     msg;

    msg.pms_Method   = PM_STATE;
    msg.pms_OldState = oldState;

    SCANLIST(&cond->ec_Players,player)
    {
        if (player->ep_Hook)
            ((STATEFUNC)player->ep_Hook->h_Entry)(player->ep_Hook,&msg,player);

	if (player->ep_Flags & PLAYERF_CONDUCTED)
            return;
    }
}


/*****************************************************************************/


/* find_maestro - Attempts to locate Player with highest priority.
 *                If no such Player exists, return NULL.
 */

static struct ExtPlayer *find_maestro(struct ExtConductor *cond)
{
struct ExtPlayer *player;

    SCANLIST(&cond->ec_Players,player)
    {
	if (player->ep_Flags & PLAYERF_CONDUCTED)
            return(player);
    }

    return(NULL);
}


/*****************************************************************************/


/* find_conductor - Attempts to locate a conductor by name on the
 *		    ConductorList. If no such conductor exists, it will be
 *		    created by this function.
 *
 *                  Pass this the name of the Conductor to find or create.
 *                  If ~0, never match existing Conductor, always create one.
 *
 *                  Pass this a NULL name, and you get NULL back.
 */

static struct ExtConductor *find_conductor(STRPTR name, struct RealTimeLib *RealTimeBase)
{
struct ExtConductor *cond;

    if (!name)
        return(NULL);

    if (name != (STRPTR)~0)
    {
	if (cond = (struct ExtConductor *)FindName((struct List *)&RealTimeBase->rtl_ConductorList,name))
            return cond;
    }
    else
    {
        name = "";
    }

    if (cond = ALLOCVEC(sizeof(struct ExtConductor) + strlen(name) + 1,MEMF_CLEAR))
    {
        cond->ec_Link.ln_Name = (STRPTR)&cond[1];
        strcpy(cond->ec_Link.ln_Name,name);
        cond->ec_Stopped = CONDSTOPF_STOPPED;
        NewList((struct List *)&cond->ec_Players);

        if (!name[0])
            cond->ec_Flags = CONDUCTF_PRIVATE;

        AddHead((struct List *)&RealTimeBase->rtl_ConductorList,cond);
    }

    return(cond);
}


/*****************************************************************************/


/* flush_conductor - Flushes a conductor if its Player list is empty. */

static VOID flush_conductor(struct ExtConductor *cond, struct RealTimeLib *RealTimeBase)
{
    if (IsListEmpty((struct List *)&cond->ec_Players))
    {
	Remove(cond);
	FREEVEC(cond);
    }
}


/*****************************************************************************/


static VOID SetErrorCode(struct TagItem *tagList, ULONG error,
                         struct RealTimeLib *RealTimeBase)
{
ULONG *errbuf;

    if (errbuf = (ULONG *)GETTAGDATA(PLAYER_ErrorCode, NULL, tagList))
        *errbuf = error;
}


/*****************************************************************************/


BOOL ASM LIBSetPlayerAttrsA(REG(a0) struct ExtPlayer *player,
                            REG(a1) struct TagItem *tagList,
                            REG(a6) struct RealTimeLib *RealTimeBase)
{
ULONG                err;
BOOL	             no_cond;
BOOL	             ready;
struct ExtConductor *cond;
struct TagItem      *tag;
struct TagItem      *tags = tagList;
ULONG                data;

    err   = 0;
    ready = FALSE;

    LockCond();
    LockPlayerList();

    no_cond = IsListEmpty((struct List *)&RealTimeBase->rtl_ConductorList);

    while (tag = NEXTTAGITEM(&tags))
    {
        data = tag->ti_Data;

        switch (tag->ti_Tag)
        {
            case PLAYER_Name        : player->ep_Link.ln_Name = (STRPTR)data;
                                      break;

	    case PLAYER_AlarmSigTask: player->ep_Task = (struct Task *)data;
				      break;

	    case PLAYER_AlarmSigBit : player->ep_AlarmSigBit  = (BYTE)data;
	                              player->ep_AlarmSigMask = (1 << data);
                                      break;

	    case PLAYER_UserData    : player->ep_UserData = (VOID *)data;
				      break;

	    case PLAYER_ID          : player->ep_PlayerID = (UWORD)data;
				      break;

            case PLAYER_Hook        : player->ep_Hook = (struct Hook *)data;
				      break;

	    case PLAYER_AlarmTime   : player->ep_AlarmTime = (LONG)data;
                                      player->ep_Flags     |= PLAYERF_ALARMSET;
                                      break;

            case PLAYER_Priority    : player->ep_Link.ln_Pri = (BYTE)data;
                                      if (cond = (struct ExtConductor *)player->ep_Source)
                                      {
                                          Remove(&player->ep_Link);
                                          Enqueue((struct List *)&cond->ec_Players,player);
                                      }
                                      break;

            case PLAYER_Conductor   : cond = find_conductor((STRPTR)data,RealTimeBase);
                                      if (cond != player->ep_Source)
                                      {
                                          if (player->ep_Source)
                                          {
                                              Remove(player);
                                              if (player->ep_Flags & PLAYERF_EXTSYNC)
                                              {
                                                  player->ep_Flags &= ~PLAYERF_EXTSYNC;
                                                  ((struct ExtConductor *)player->ep_Source)->ec_Flags &= ~CONDUCTF_EXTERNAL;
                                              }
                                              flush_conductor(player->ep_Source,RealTimeBase);
                                          }
                                          player->ep_Source = cond;

                                          if (cond)
                                              Enqueue((struct List *)&cond->ec_Players,player);
                                      }

                                      if (!cond && data)
                                      {
                                          err = RTE_NOMEMORY;
                                      }
                                      break;

            case PLAYER_ExtSync     : if (cond = player->ep_Source)
                                      {
                                          if (data)
                                          {
                                              if (!(cond->ec_Flags & CONDUCTF_EXTERNAL))
                                              {
                                                  player->ep_Flags |= PLAYERF_EXTSYNC;
                                                  cond->ec_Flags |= CONDUCTF_EXTERNAL;
                                              }
                                              else
                                              {
                                                  /* !!! error? !!! */
                                              }
                                          }
                                          else
                                          {
                                              if (player->ep_Flags & PLAYERF_EXTSYNC)
                                              {
                                                  player->ep_Flags &= ~PLAYERF_EXTSYNC;
                                                  cond->ec_Flags &= ~CONDUCTF_EXTERNAL;
                                              }
                                          }
                                      }
                                      break;

            case PLAYER_Ready       : if (data)
                                      {
			                  player->ep_Flags |= PLAYERF_READY;
                                          ready = TRUE;
                                      }
                                      else
                                      {
                                          player->ep_Flags &= ~PLAYERF_READY;
                                      }
                                      break;

	    case PLAYER_Quiet       : if (data)
                                      {
                                          player->ep_Flags |= PLAYERF_QUIET;
                                      }
                                      else
                                      {
                                          player->ep_Flags &= ~PLAYERF_QUIET;
                                      }
                                      break;

	    case PLAYER_Alarm       : if (data)
                                      {
                                          player->ep_Flags |= PLAYERF_ALARMSET;
                                      }
                                      else
                                      {
                                          player->ep_Flags &= ~PLAYERF_ALARMSET;
                                      }
                                      break;

	    case PLAYER_Conducted   : if (data)
                                      {
                                          player->ep_Flags |= PLAYERF_CONDUCTED;
                                      }
                                      else
                                      {
                                          player->ep_Flags &= ~PLAYERF_CONDUCTED;
                                      }
                                      break;
        }

        if (err)
            break;
    }

    if (player->ep_Hook == NULL)
    {
        player->ep_Flags &= ~PLAYERF_CONDUCTED;
    }
    else if (ready && player->ep_Source)
    {
        check_ready(player->ep_Source);
    }

    if ((player->ep_Task == NULL) || (player->ep_AlarmSigBit == -1))
    {
        player->ep_Flags &= ~PLAYERF_ALARMSET;
    }

    if (no_cond != IsListEmpty((struct List *)&RealTimeBase->rtl_ConductorList))
    {
	if (no_cond)
	{
            err = OpenTimer(RealTimeBase);
        }
	else
	{
            CloseTimer(RealTimeBase);
        }
    }

    UnlockPlayerList();
    UnlockCond();

    if (err)
    {
        SetErrorCode(tagList, err, RealTimeBase);
        return(FALSE);
    }

    return(TRUE);
}


/*****************************************************************************/


ULONG ASM LIBGetPlayerAttrsA(REG(a0) struct ExtPlayer *player,
                             REG(a1) struct TagItem *tagList,
                             REG(a6) struct RealTimeLib *RealTimeBase)
{
ULONG	             count;
struct TagItem      *tag;
struct TagItem      *tags = tagList;
ULONG                data;
struct ExtConductor *cond;

    count = 0;

    if (player)
    {
        LockCond();
        LockPlayerList();

        while (tag = NEXTTAGITEM(&tags))
        {
            count++;
            data = tag->ti_Data;

            switch (tag->ti_Tag)
            {
                case PLAYER_Name       : *(STRPTR *)data = player->ep_Link.ln_Name;
                                         break;

                case PLAYER_AlarmSigTask: *(struct Task **)data = player->ep_Task;
                                          break;

                case PLAYER_AlarmSigBit: *(LONG *)data = player->ep_AlarmSigBit;
                                         break;

                case PLAYER_UserData   : *(VOID **)data = player->ep_UserData;
                                         break;

                case PLAYER_ID         : *(LONG *)data = player->ep_PlayerID;
                                         break;

                case PLAYER_Hook       : *(struct Hook **)data = player->ep_Hook;
                                         break;

                case PLAYER_AlarmTime  : *(LONG *)data = player->ep_AlarmTime;
                                         break;

                case PLAYER_Priority   : *(LONG *)data = player->ep_Link.ln_Pri;
                                         break;

                case PLAYER_Conductor  : *(STRPTR *)data = (cond = (struct ExtConductor *)player->ep_Source ? cond->ec_Link.ln_Name : NULL);
                                         break;

                case PLAYER_ExtSync    : *(ULONG *)data = ((player->ep_Flags | PLAYERF_EXTSYNC) != 0);
                                         break;

                case PLAYER_Ready      : *(ULONG *)data = ((player->ep_Flags | PLAYERF_READY) != 0);
                                         break;

                case PLAYER_Quiet      : *(ULONG *)data = ((player->ep_Flags | PLAYERF_QUIET) != 0);
                                         break;

                case PLAYER_Alarm      : *(ULONG *)data = ((player->ep_Flags | PLAYERF_ALARMSET) != 0);
                                         break;

                case PLAYER_Conducted  : *(ULONG *)data = ((player->ep_Flags | PLAYERF_CONDUCTED) != 0);
                                         break;

                default                : count--;
            }
        }

        UnlockPlayerList();
        UnlockCond();
    }

    return(count);
}


/*****************************************************************************/


struct ExtPlayer * ASM LIBCreatePlayerA(REG(a0) struct TagItem *tagList,
				     REG(a6) struct RealTimeLib *RealTimeBase)
{
struct ExtPlayer *player;

    if (player = AllocMem(sizeof(struct ExtPlayer),MEMF_CLEAR))
    {
        player->ep_AlarmSigBit = -1;

        if (SetPlayerAttrsA(player,tagList))
            return(player);

        DeletePlayer(player);
    }
    else
    {
        SetErrorCode(tagList, RTE_NOMEMORY, RealTimeBase);
    }

    return(NULL);
}


/*****************************************************************************/


VOID ASM LIBDeletePlayer(REG(a0) struct ExtPlayer *player,
			 REG(a6) struct RealTimeLib *RealTimeBase)
{
    if (player)
    {
	SetPlayerAttrs(player,PLAYER_Conductor,NULL,
                              TAG_DONE);

	FreeMem(player, sizeof(struct ExtPlayer));
    }
}


/*****************************************************************************/


struct ExtConductor * ASM LIBNextConductor(REG(a0) struct ExtConductor *last,
					REG(a6) struct RealTimeLib *RealTimeBase)
{
struct ExtConductor *cond;

    if (!last)
        last = (struct ExtConductor *)&RealTimeBase->rtl_ConductorList;

    cond = (struct ExtConductor *)last->ec_Link.ln_Succ;
    if (!cond->ec_Link.ln_Succ)
        cond = NULL;

    return(cond);
}


/*****************************************************************************/


struct ExtConductor * ASM LIBFindConductor(REG(a0) STRPTR name,
					REG(a6) struct RealTimeLib *RealTimeBase)
{
    return((struct ExtConductor *)FindName((struct List *)&RealTimeBase->rtl_ConductorList,name));
}


/*****************************************************************************/


BOOL ASM LIBExternalSync(REG(a0) struct ExtPlayer *player,
                         REG(d0) LONG minTime,
                         REG(d1) LONG maxTime,
			 REG(a6) struct RealTimeLib *RealTimeBase)
{
struct ExtConductor *cond;

    cond = player->ep_Source;
    if (cond && (player->ep_Flags & PLAYERF_EXTSYNC))
    {
        cond->ec_Flags          |= CONDUCTF_GOTTICK;
        cond->ec_Stopped        &= ~CONDSTOPF_NOTICK;
        cond->ec_ExternalTime    = minTime;
        cond->ec_MaxExternalTime = maxTime;
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


LONG ASM LIBSetConductorState(REG(a0) struct ExtPlayer *player,
                              REG(d0) ULONG state,
                              REG(d1) LONG time,
			      REG(a6) struct RealTimeLib *RealTimeBase)
{
struct ExtConductor *cond;
struct ExtPlayer    *maestro;
UBYTE             oldState;
LONG              err;

    err = 0;

    if (cond = player->ep_Source)
    {
        LockCond();

	oldState = cond->ec_State;

	switch (state)
        {
            case CONDSTATE_LOCATE : cond->ec_State = CONDSTATE_STOPPED;
                                    clear_ready(cond);
                                    cond->ec_StartTime = RealTimeBase->rtl_Time - time;
                                    cond->ec_ClockTime = time;
                                    cond->ec_State     = state;
                                    cond->ec_Stopped  |= CONDSTOPF_STOPPED;
                                    notify_locate_players(cond,oldState);
                                    break;

            case CONDSTATE_RUNNING: if (oldState != CONDSTATE_PAUSED)
                                    {
                                        cond->ec_State = CONDSTATE_STOPPED;
                                        clear_ready(cond);
                                        cond->ec_StartTime = RealTimeBase->rtl_Time - time;
                                        cond->ec_ClockTime = time;
                                    }
                                    cond->ec_State = state;
                                    cond->ec_Stopped &= ~CONDSTOPF_STOPPED;
                                    notify_players(cond,oldState);
                                    break;

            case CONDSTATE_STOPPED:
            case CONDSTATE_PAUSED : cond->ec_State    = state;
                                    cond->ec_Stopped |= CONDSTOPF_STOPPED;
                                    notify_players(cond,oldState);
                                    break;

            case CONDSTATE_METRIC : if (maestro = find_maestro(cond))
                                    {
                                    struct pmTime msg;

                                        msg.pmt_Method = PM_POSITION;
                                        msg.pmt_Time   = time;

                                        if (maestro->ep_Hook)
                                            ((TIMEFUNC)maestro->ep_Hook->h_Entry)(maestro->ep_Hook,&msg,maestro);
                                    }
                                    break;

            case CONDSTATE_SHUTTLE: if ((oldState == CONDSTATE_STOPPED) || (oldState == CONDSTATE_PAUSED))
                                    {
                                    struct ExtPlayer *player;
                                    struct pmTime  msg;

                                        msg.pmt_Method = PM_SHUTTLE;
                                        msg.pmt_Time   = time;

                                        SCANLIST(&cond->ec_Players,player)
                                        {
                                            if (player->ep_Hook)
                                                ((TIMEFUNC)player->ep_Hook->h_Entry)(player->ep_Hook,&msg,player);
                                        }
                                    }
                                    else
                                    {
                                        err = RTE_PLAYING;
                                    }
                                    break;

            case CONDSTATE_LOCATE_SET: if (find_maestro(cond) == player)
                                           notify_conducted_players(player,oldState);

                                       break;
	}

        UnlockCond();
    }
    else
    {
        err = RTE_NOCONDUCTOR;
    }

    return (err);
}
