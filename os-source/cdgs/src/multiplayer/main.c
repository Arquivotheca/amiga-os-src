

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/interrupts.h>

#include <devices/timer.h>

#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/graphics_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include "/cdda/cd.h"

#include "playerlibrary.h"
#include "defs.h"

#include "task_funcs.h"
#include "cd_funcs.h"
#include "playlist_funcs.h"
#include "vfd_funcs.h"

#define SysBase (*((struct ExecBase **)0x00000004L))

extern LONG __far initFunc;
extern LONG __far initStruct;

extern void __saveds PlayerTask(void);

#define STACKSIZE   4000L



APTR __asm Main(register __a0 APTR SegList) {

struct PlayerLibrary *PlayerBase;

    if (PlayerBase = (struct PlayerLibrary *)MakeLibrary(&initFunc, &initStruct, 0, sizeof(struct PlayerLibrary), 0)) {

        AddLibrary(PlayerBase);

        PlayerBase->SegList = SegList;

        return(NULL);
        }

    else return(SegList);
    }




struct Task * __asm StartPlayerTask(register __a6 struct PlayerLibrary *PlayerBase) {

struct Message *TaskMsg;

    if (PlayerBase->PlayerTask = (struct Task *)AllocMem(sizeof(struct Task), MEMF_PUBLIC | MEMF_CLEAR)) {

        if (PlayerBase->PlayerTask->tc_SPLower = (struct ULONG *)AllocMem(STACKSIZE, MEMF_PUBLIC | MEMF_CLEAR)) {

            PlayerBase->PlayerTask->tc_SPReg   =
            PlayerBase->PlayerTask->tc_SPUpper = (APTR)((ULONG)PlayerBase->PlayerTask->tc_SPLower + STACKSIZE);

            PlayerBase->PlayerTask->tc_Node.ln_Type = NT_TASK;
            PlayerBase->PlayerTask->tc_Node.ln_Pri  = 0;
            PlayerBase->PlayerTask->tc_Node.ln_Name = "PlayerTask";

            *((struct PlayerLibrary **)(PlayerBase->PlayerTask->tc_SPLower)) = PlayerBase;

            if (PlayerBase->TaskMsgPort = AllocMem(sizeof(struct MsgPort), MEMF_PUBLIC | MEMF_CLEAR)) {

                PlayerBase->TaskMsgPort->mp_Node.ln_Type = NT_MSGPORT;
                PlayerBase->TaskMsgPort->mp_Flags        = PA_SIGNAL;
                PlayerBase->TaskMsgPort->mp_SigTask      = PlayerBase->PlayerTask;
                PlayerBase->TaskMsgPort->mp_SigBit       = TASKMSGSIGBIT;

                NewList(&PlayerBase->TaskMsgPort->mp_MsgList);
                AddPort(PlayerBase->TaskMsgPort);

                if (PlayerBase->TaskReplyPort = AllocMem(sizeof(struct MsgPort), MEMF_PUBLIC | MEMF_CLEAR)) {

                    PlayerBase->TaskReplyPort->mp_Node.ln_Type = NT_MSGPORT;
                    PlayerBase->TaskReplyPort->mp_Flags        = PA_SIGNAL;
                    PlayerBase->TaskReplyPort->mp_SigTask      = FindTask(NULL);
                    PlayerBase->TaskReplyPort->mp_SigBit       = AllocSignal(-1);

                    NewList(&PlayerBase->TaskReplyPort->mp_MsgList);
                    AddPort(PlayerBase->TaskReplyPort);

                    InitSemaphore(&PlayerBase->PlayListSemaphore);
                    InitSemaphore(&PlayerBase->PlayStateSemaphore);

                    if (AddTask(PlayerBase->PlayerTask, PlayerTask, NULL)) {

                        if (TaskMsg = (struct Message *)AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR)) {

                            TaskMsg->mn_Length    = MSG_STARTUP;
                            TaskMsg->mn_ReplyPort = PlayerBase->TaskReplyPort;
                            PutMsg(PlayerBase->TaskMsgPort, TaskMsg);

                            WaitPort(PlayerBase->TaskReplyPort);
                            while(GetMsg(PlayerBase->TaskReplyPort));

                            if (TaskMsg->mn_Length == MSG_STARTUP) {

                                FreeMem(TaskMsg, sizeof(struct Message));
                                return(PlayerBase->PlayerTask);
                                }

                            else FreeMem(TaskMsg, sizeof(struct Message));
                            }

                        RemTask(PlayerBase->PlayerTask);
                        }

                    RemPort(PlayerBase->TaskReplyPort);
                    FreeSignal(PlayerBase->TaskReplyPort->mp_SigBit);
                    FreeMem(PlayerBase->TaskReplyPort, sizeof(struct MsgPort));
                    }

                RemPort(PlayerBase->TaskMsgPort);
                FreeMem(PlayerBase->TaskMsgPort, sizeof(struct MsgPort));
                }

            FreeMem(PlayerBase->PlayerTask->tc_SPLower, STACKSIZE);
            }

        FreeMem(PlayerBase->PlayerTask, sizeof(struct Task));
        }

    return(NULL);
    }



void __asm DeletePlayerTask(register __a6 struct PlayerLibrary *PlayerBase) {

struct Message *TaskMsg, *ShutdownMsg;

    if (TaskMsg = (struct Message *)AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR)) {

        TaskMsg->mn_Length    = MSG_SHUTDOWN;
        TaskMsg->mn_ReplyPort = PlayerBase->TaskReplyPort;
        PutMsg(PlayerBase->TaskMsgPort, TaskMsg);

        WaitPort(PlayerBase->TaskReplyPort);

        while(1) {

            ShutdownMsg = GetMsg(PlayerBase->TaskReplyPort);

            if (ShutdownMsg) if (ShutdownMsg->mn_Length == MSG_SHUTDOWN) break;
            }

        FreeMem(TaskMsg, sizeof(struct Message));

        RemTask(PlayerBase->PlayerTask);

        RemPort(PlayerBase->TaskReplyPort);
        FreeSignal(PlayerBase->TaskReplyPort->mp_SigBit);
        FreeMem(PlayerBase->TaskReplyPort, sizeof(struct MsgPort));

        RemPort(PlayerBase->TaskMsgPort);
        FreeMem(PlayerBase->TaskMsgPort, sizeof(struct MsgPort));

        FreeMem(PlayerBase->PlayerTask->tc_SPLower, STACKSIZE);
        FreeMem(PlayerBase->PlayerTask, sizeof(struct Task));
        }
    }


#if 0
*****i* player.library/Expunge() **************************************
*
*   NAME
*       Expunge -- Free memory if not in use
*
*   SYNOPSIS
*       Expunge()
*
*       void Expunge(void);
*
*   FUNCTION
*       Removes library from system memory
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*
***********************************************************************
#endif

APTR __asm LibExpunge(register __a6 struct PlayerLibrary *PlayerBase) {

    if (PlayerBase->SegList) {

        if (!PlayerBase->PlayerLib.lib_OpenCnt) {

            Remove(PlayerBase);
            FreeMem((UBYTE *)PlayerBase - PlayerBase->PlayerLib.lib_NegSize,
                PlayerBase->PlayerLib.lib_NegSize + PlayerBase->PlayerLib.lib_PosSize);

            return(PlayerBase->SegList);
            }

        else PlayerBase->PlayerLib.lib_Flags |= LIBF_DELEXP;
        }

    return(0);
    }



#if 0
*****i* player.library/Open() *****************************************
*
*   NAME
*       Open -- Allocate player
*
*   SYNOPSIS
*       PlayerLibrary = Open()
*       D0
*
*       struct PlayerLibrary *Open(void);
*
*   FUNCTION
*       Allocate the CDTV internal player.  This device is concurrently
*       shared with the front panel.
*
*   RESULTS
*       NULL means library could not open
*
*   NOTES
*       This is an exclusive access library
*
*   SEE ALSO
*       Close()
*
***********************************************************************
#endif

struct PlayerLibrary * __asm LibOpen(register __a6 struct PlayerLibrary *PlayerBase) {

    if (PlayerBase->PlayerLib.lib_OpenCnt) return(0);

    if (!FindTask("PlayerTask")) {

        PlayerBase->PlayerLib.lib_OpenCnt = 1;

        if (!StartPlayerTask(PlayerBase)) {

            PlayerBase->PlayerLib.lib_OpenCnt = 0;
            return(0);
            }
        }

    else PlayerBase->PlayerLib.lib_OpenCnt = 1;

    return(PlayerBase);
    }


#if 0
*****i* player.library/Close() ****************************************
*
*   NAME
*       Close -- Release Player
*
*   SYNOPSIS
*       Close()
*
*       void Close(void);
*
*   FUNCTION
*       Releases the CDTV internal player from Amiga control.
*
*   RESULTS
*
*   NOTES
*
*   SEE ALSO
*       Open()
*
***********************************************************************
#endif

APTR __asm LibClose(register __a6 struct PlayerLibrary *PlayerBase) {

    if (PlayerBase->PlayerLib.lib_OpenCnt) {

        PlayerBase->PlayerLib.lib_OpenCnt = 0;
        DeletePlayerTask(PlayerBase);
        }

    if (PlayerBase->PlayerLib.lib_Flags & LIBF_DELEXP) return(LibExpunge(PlayerBase));
    else                                               return(0);
    }



#if 0
******* player.library/GetOptions() **********************************
*
*   NAME
*       GetOptions -- Retrieve player mode settings
*
*   SYNOPSIS
*       success = GetOptions(Options);
*       D0                   A1
*
*       UWORD GetOptions(struct PlayerOptions *);
*
*   FUNCTION
*       Retrieves current loop mode, intro mode, time display mode,
*       and subcode settings from internal player and stores settings
*       in PlayerOptions structure.
*
*   RESULTS
*       1 = Operation successfull, 0 = Operation failed (do not own
*       library).  PlayerOptions structure is updated.
*
*   NOTES
*       struct PlayerOptions {
*
*           BYTE    Loop;           /* 0 = Disable, 1 = Enable */
*
*           BYTE    Intro;          /* 0 = Disable, 1 = Enable */
*
*           BYTE    TimeMode;       /* 0 =  Track Relative     */
*                                   /* 1 = -Track Relative     */
*                                   /* 2 =  Disk Absolute      */
*                                   /* 3 = -Disk Absolute      */
*
*           BYTE    Subcode;        /* 0 = Disable, 1 = Enable */
*           };
*
*   SEE ALSO
*           SetOptions()
*
***********************************************************************
#endif

UWORD __asm GetOptions(register __a1 struct PlayerOptions *Options, register __a6 struct PlayerLibrary *PlayerBase) {

    Options->Loop     = PlayerBase->PlayerOptions.Loop;
    Options->Intro    = PlayerBase->PlayerOptions.Intro;
    Options->TimeMode = PlayerBase->PlayerOptions.TimeMode;
    Options->Subcode  = PlayerBase->PlayerOptions.Subcode;

    return(0);
    }



#if 0
******* player.library/SetOptions() **********************************
*
*   NAME
*       SetOptions -- Set one or more player options
*
*   SYNOPSIS
*       success = SetOptions(Options);
*       D0                   A1
*
*       UWORD SetOptions(struct PlayerOptions *);
*
*   FUNCTION
*       Sets current loop mode, intro mode, time display mode, and/or
*       subcode settings of internal player based on PlayerOptions
*       structure.
*
*   RESULTS
*       1 = Operation successfull, 0 = Operation failed (do not own
*       library).
*
*   NOTES
*       struct PlayerOptions {
*
*           BYTE    Loop;           /*  0 =  Disable            */
*                                   /*  1 =  Enable             */
*                                   /* -1 =  Don't modify       */
*
*           BYTE    Intro;          /*  0 =  Disable            */
*                                   /*  1 =  Enable             */
*                                   /* -1 =  Don't modify       */
*
*           BYTE    TimeMode;       /*  0 =  Track Relative     */
*                                   /*  1 = -Track Relative     */
*                                   /*  2 =  Disk Absolute      */
*                                   /*  3 = -Disk Absolute      */
*                                   /* -1 =  Don't modify       */
*
*           BYTE    Subcode;        /*  0 =  Disable            */
*                                   /*  1 =  Enable             */
*                                   /* -1 =  Don't modify       */
*           };
*
*   SEE ALSO
*       GetOptions()
*
***********************************************************************
#endif

UWORD __asm SetOptions(register __a1 struct PlayerOptions *Options, register __a6 struct PlayerLibrary *PlayerBase) {

struct Message *TaskMsg;

    if (Options->Loop     != -1) PlayerBase->PlayerOptions.Loop     = Options->Loop;
    if (Options->Intro    != -1) PlayerBase->PlayerOptions.Intro    = Options->Intro;
    if (Options->Subcode  != -1) PlayerBase->PlayerOptions.Subcode  = Options->Subcode;

    if (Options->TimeMode != -1) {

        PlayerBase->PlayerOptions.TimeMode = Options->TimeMode;

        if (!(TaskMsg = (struct Message *)AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR))) return(1);

        TaskMsg->mn_Length = MSG_TIMEMODE;
        PutMsg(PlayerBase->TaskMsgPort, TaskMsg);
        }

    return(0);
    }

#if 0
******* player.library/GetPlayerState() ******************************
*
*   NAME
*       GetPlayerState -- Determine current state of player
*
*   SYNOPSIS
*       Success = GetPlayerState(State)
*       D0                       A1
*
*       UWORD GetPlayerState(struct PlayerState *);
*
*   FUNCTION
*       Returns the current state of the internal player.
*
*   RESULTS
*       1 = Operation successfull, 0 = Operation failed (do not own
*       library).
*
*   NOTES
*       struct PlayerState {
*       
*         BYTE   AudioDisk;  /*  1 = An Audio disk is present        */
*                            /*  0 = No disk present                 */
*                            /* -1 = Non audio disk is present       */
*         UBYTE  Tracks;     /* Number of tracks on audio disk       */
*         UBYTE  ListIndex;  /* Current position of player in list   */
*                            /*   (values = 1-99 & 0 (not selected)) */
*         UBYTE  LastModify; /* Last to modify PlayList              */
*                            /* (0 = 68000, 1 = internal player)     */
*         UBYTE  PlayMode;   /* PLM_NORMAL, PLM_FFWD, PLM_FREV,      */
*                            /*   PLM_SKIPFWD, PLM_SKIPREV           */
*         UBYTE  PlayState;  /* PLS_STOPPED, PLS_PLAYING, PLS_PAUSED */
*   
*         UBYTE  Track;      /* Current value in TRACK field of VFD  */
*         UBYTE  Minute;     /* Current value in MINUTE field of VFD */
*         UBYTE  Second;     /* Current value in HOUR field of VFD   */
*                            /*   (values = 0-99 & 100 (blank))      */
*         UBYTE  Minus;      /* Current value in MINUS-SIGN field of */
*                            /* VFD time display                     */
*         };
*   
*   SEE ALSO
*       ObtainPlayList()
*
*
***********************************************************************
#endif

UWORD __asm GetPlayerState(register __a1 struct PlayerState *State, register __a6 struct PlayerLibrary *PlayerBase) {

    ObtainSemaphore(&PlayerBase->PlayStateSemaphore);
    *State = PlayerBase->PlayerState;        
    ReleaseSemaphore(&PlayerBase->PlayStateSemaphore);

    return(0);
    }


#if 0
******* player.library/ObtainPlayList() ******************************
*
*   NAME
*       ObtainPlayList - Obtain pointer to player's PlayList structure
*
*   SYNOPSIS
*       List = ObtainPlayList()
*       D0
*
*       struct PlayList *ObtainPlayList(void)
*
*   FUNCTION
*       Returns pointer to internal player's PlayList structure.
*       The playlist may be modified by the application.  Changes will
*       take effect immediately.  Only valid tracks should be stored
*       in the list.  When modifying the number of entries in the list,
*       special care should be taken.  When adding an entry, the entry
*       should be added, then the entry count should be incremented.
*       When subtracting an entry, the entry count should be
*       decremented, then the entry may be destroyed.  When doing a
*       modification of multiple entries, the list should first be
*       invalidated by setting the entry count to 0, then modified,
*       then the entry count should be set to the number of entries
*       in the new list.
*
*   RESULTS
*       NULL = Operation failed (do not own library), otherwise
*              pointer to PlayList structure.
*
*   NOTES
*       struct PlayList {
*       
*           UBYTE   EntryCount; /* Number of items in list */
*           UBYTE   Entry[100]; /* List of Tracks to play  */
*           UBYTE   pad;
*           };
*   
*   SEE ALSO
*       GetPlayerState()
*
***********************************************************************
#endif

struct PlayList * __asm ObtainPlayList(register __a6 struct PlayerLibrary *PlayerBase) {

    return(&PlayerBase->PlayList);
    }


#if 0
******* player.library/ModifyPlayList() ******************************
*
*   NAME
*       ModifyPlayList - Notify player when PlayList is being modified
*
*   SYNOPSIS
*       Success = ModifyPlayList(TrueFalse)
*       D0                       D1
*
*       UWORD ModifyPlayList(UWORD state)
*
*   FUNCTION
*       When the 68000 wants to modify the PlayList, this function
*       should be called with a TRUE state.  The modification can
*       then take place, then this function should be called again
*       with a FALSE state.
*
*       Since the internal player can modify the playlist too,
*       precautions must be taken so that both the 68000 and the
*       internal player don't modify the PlayList at the same time.
*
*       If this function is not called before modifying the PlayList
*       bazzar things may happen to the list.
*
*   RESULTS
*       1 = Operation successfull, 0 = Operation failed (do not own
*       library or player is currently busy with PlayList).
*
*   NOTES
*   
*   SEE ALSO
*       ObtainPlayList()
*
***********************************************************************
#endif

UWORD __asm ModifyPlayList(register __d1 UWORD state, register __a6 struct PlayerLibrary *PlayerBase) {

    if (state) {

        if (AttemptSemaphore(&PlayerBase->PlayListSemaphore)) PlayerBase->PlayerState.LastModify = 0;
        else                                                  return(0);
        }

    else ReleaseSemaphore(&PlayerBase->PlayListSemaphore);

    return(1);
    }


#if 0
******* player.library/SubmitKeyStroke() *****************************
*
*   NAME
*       SubmitKeyStroke - Submit up/down player keystrokes to player
*
*   SYNOPSIS
*       success = SubmitKeyStroke(keystroke)
*       D0                        D1
*
*       UWORD SubmitKeyStroke(UBYTE)
*
*   FUNCTION
*       Emulates a player keystroke on the front panel.
*
*   RESULTS
*       1 = Operation successfull, 0 = Operation failed (do not own
*       library).
*
*   NOTES
*       If a second down keystroke is sent before an up keystroke of
*       the previous key, an up keystroke is automatically generated
*       for that key.
*
*       When a keystroke is submitted, that keystroke will
*       automatically be interpretted by the player and echoed to the
*       Amiga (just like when a player key on the front panel is
*       pressed.
*
*   SEE ALSO
*
***********************************************************************
#endif

UWORD __asm SubmitKeyStroke(register __d1 UBYTE keystroke, register __a6 struct PlayerLibrary *PlayerBase) {

struct Message *TaskMsg;

    if (!(TaskMsg = (struct Message *)AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR))) return(1);

    TaskMsg->mn_Length = MSG_KEYSTROKE | keystroke;
    PutMsg(PlayerBase->TaskMsgPort, TaskMsg);
    return(0);
    }


