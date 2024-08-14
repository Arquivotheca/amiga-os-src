#ifndef REALTIMEBASE_H
#define REALTIMEBASE_H


/****************************************************************************/


#include <exec/lists.h>
#include <exec/types.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos.h>
#include "realtime.h"


/****************************************************************************/


struct ExtConductor
{
    struct Conductor ec_Conductor;

    /* private fields follow. The pl_Reserved#? fields can also be used */
};

#define ec_Stopped         ec_Conductor.cdt_Link.ln_Type
#define ec_Link            ec_Conductor.cdt_Link
#define ec_Players         ec_Conductor.cdt_Players
#define ec_ClockTime       ec_Conductor.cdt_ClockTime
#define ec_StartTime       ec_Conductor.cdt_StartTime
#define ec_ExternalTime    ec_Conductor.cdt_ExternalTime
#define ec_MaxExternalTime ec_Conductor.cdt_MaxExternalTime
#define ec_Metronome       ec_Conductor.cdt_Metronome
#define ec_Flags           ec_Conductor.cdt_Flags
#define ec_State           ec_Conductor.cdt_State

/* flag bits for ExtConductor.ec_Stopped */
#define CONDSTOPF_STOPPED (1<<0)    /* not running state     */
#define CONDSTOPF_NOTICK  (1<<1)    /* extsync & not gottick */
#define CONDSTOPB_STOPPED 0
#define CONDSTOPB_NOTICK  1


/****************************************************************************/


struct ExtPlayer
{
    struct Player ep_Player;

    /* private fields follow. Some of the pl_Reserved#? fields can also be used */
    ULONG         ep_AlarmSigMask;
};

#define ep_AlarmSigBit  ep_Player.pl_Reserved0
#define ep_Link         ep_Player.pl_Link
#define ep_Hook		ep_Player.pl_Hook
#define ep_Source       ep_Player.pl_Source
#define ep_Task         ep_Player.pl_Task
#define ep_MetricTime   ep_Player.pl_MetricTime
#define ep_AlarmTime    ep_Player.pl_AlarmTime
#define ep_UserData     ep_Player.pl_UserData
#define ep_PlayerID     ep_Player.pl_PlayerID
#define ep_Flags        ep_Player.pl_Flags


/****************************************************************************/


struct RealTimeLib
{
    struct RealTimeBase     rtl_Public;

    /* private fields follow */

    struct Library         *rtl_UtilityBase;
    BPTR                    rtl_SegList;
    struct ExecBase        *rtl_SysBase;

    /* this must follow rtb_SysBase in order for things to work */
    struct Interrupt        rtl_TimerSoftInt;
    UBYTE                   rtl_CIAChannel;  /* channel # of allocated CIA timer */
    UBYTE                   rtl_Pad1;        /* long-align                       */
    APTR                    rtl_CIAResource; /* cia resource                     */
    UBYTE                  *rtl_CIACtrlReg;  /* ptr to CIA control register      */
    struct pmTime           rtl_Message;     /* sent by softint to players       */

    struct Interrupt        rtl_CIAInt;
    UWORD                   rtl_ClockFreq;   /* CIA approx clock frequency */
    UWORD                   rtl_CIAHandlerCode[6]; /* code to run as int handler */

    struct SignalSemaphore  rtl_PlayerListLock;  /* arbitrates access to time distribution to Players */
    UWORD                   rtl_Pad3;

    struct MinList          rtl_ConductorList;  /* list of conductors */
    struct SignalSemaphore  rtl_ConductorLock;  /* used to control access to conductor list */
};

#define ASM         __asm
#define REG(x)	    register __ ## x

#define SysBase     RealTimeBase->rtl_SysBase
#define UtilityBase RealTimeBase->rtl_UtilityBase
#define CiaBase     RealTimeBase->rtl_CIAResource

/* this field must not move for compatibility with old code */
#define rtl_TimeFreq rtl_Public.rtb_Reserved1

#define rtl_Time     rtl_Public.rtb_Time
#define rtl_TimeFrac rtl_Public.rtb_TimeFrac
#define rtl_TickErr  rtl_Public.rtb_TickErr


/****************************************************************************/


kprintf(STRPTR,...);

/* scan an exec list */
#define SCANLIST(l,n) for (n = (APTR)((struct MinList *)l)->mlh_Head;\
                      ((struct MinNode *)n)->mln_Succ;\
                      n = (APTR)((struct MinNode *)n)->mln_Succ)


/****************************************************************************/


/* Locking Macros */

#define LockPlayerList()   { ObtainSemaphore(&RealTimeBase->rtl_PlayerListLock); }
#define UnlockPlayerList() { ReleaseSemaphore(&RealTimeBase->rtl_PlayerListLock); }

#define LockCond()   { ObtainSemaphore(&RealTimeBase->rtl_ConductorLock); }
#define UnlockCond() { ReleaseSemaphore(&RealTimeBase->rtl_ConductorLock); }


/****************************************************************************/


#pragma libcall CiaBase AddICRVector 6 90E03
#pragma libcall CiaBase RemICRVector C 90E03
#pragma libcall CiaBase AbleICR 12 0E02
#pragma libcall CiaBase SetICR 18 0E02


/****************************************************************************/


APTR LockRealTime( ULONG lockType );
void UnlockRealTime( APTR lock );
struct Player *CreatePlayerA( struct TagItem *tagList );
struct Player *CreatePlayer( Tag tag1, ... );
void DeletePlayer( struct Player *player );
BOOL SetPlayerAttrsA( struct Player *player, struct TagItem *tagList );
BOOL SetPlayerAttrs( struct Player *player, Tag tag1, ... );
LONG SetConductorState( struct Player *player, unsigned long state, long time );
BOOL ExternalSync( struct Player *player, long minTime, long maxTime );
struct Conductor *NextConductor( struct Conductor *previousConductor );
struct Conductor *FindConductor( STRPTR name );
ULONG GetPlayerAttrsA( struct Player *player, struct TagItem *tagList );
ULONG GetPlayerAttrs( struct Player *player, Tag tag1, ... );

#pragma libcall RealTimeBase LockRealTime 1e 001
#pragma libcall RealTimeBase UnlockRealTime 24 801
#pragma libcall RealTimeBase CreatePlayerA 2a 801
#pragma tagcall RealTimeBase CreatePlayer 2a 801
#pragma libcall RealTimeBase DeletePlayer 30 801
#pragma libcall RealTimeBase SetPlayerAttrsA 36 9802
#pragma tagcall RealTimeBase SetPlayerAttrs 36 9802
#pragma libcall RealTimeBase SetConductorState 3c 10803
#pragma libcall RealTimeBase ExternalSync 42 10803
#pragma libcall RealTimeBase NextConductor 48 801
#pragma libcall RealTimeBase FindConductor 4e 801
#pragma libcall RealTimeBase GetPlayerAttrsA 54 9802
#pragma tagcall RealTimeBase GetPlayerAttrs 54 9802


/****************************************************************************/


#ifdef OS_1_3
#else
#define ALLOCVEC(size,type) AllocVec(size,type)
#define FREEVEC(mem)        FreeVec(mem)
#define ECLOCKFREQ()        (SysBase->ex_EClockFrequency)
#define NEXTTAGITEM(t)      NextTagItem(t)
#define GETTAGDATA(v,d,t)   GetTagData(v,d,t)
#endif


/****************************************************************************/


#endif /* REALTIMEBASE_H */
