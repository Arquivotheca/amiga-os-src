

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/interrupts.h>

#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>

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



/***************************************************************************/

LONG __asm DiskChangeIntr(register __a1 struct ChangeData *cd);

/***************************************************************************/
/*                                                                         */
/*  SendTimeRequest - Generate a timer request                             */
/*                                                                         */
/***************************************************************************/

void SendTimeRequest(ULONG micros, struct V *V) {

#if DEBUGGING
    if ( V->TimeReqOutstanding )
    {
    kprintf("Detected reuse of Time request!\n");
    }
#endif
    V->TimeReq->tr_node.io_Command = TR_ADDREQUEST;                         /* Create timer request */
    V->TimeReq->tr_time.tv_secs    = 0;
    V->TimeReq->tr_time.tv_micro   = micros;

    SendIO(V->TimeReq);                                                     /* Send timer request */

    V->TimeReqOutstanding = 1;                                              /* Timer request is outstanding */
    }



/***************************************************************************/
/*                                                                         */
/*  AbortTimeRequest - Abort a timer request                               */
/*                                                                         */
/***************************************************************************/

void AbortTimeRequest(struct V *V) {

    if (V->TimeReqOutstanding) {                                            /* Is there an active request to abort? */

        AbortIO(V->TimeReq);                                                /* Abort IO Request */

        Wait(1L << V->TimeReplyPort->mp_SigBit);                            /* Wait for message to come back */
        while (GetMsg(V->TimeReplyPort));

        V->TimeReqOutstanding = 0;                                          /* No outstanding timer request */
        }
    }



/***************************************************************************/
/*                                                                         */
/*  SendSQTimeRequest - Generate a Q-Code timer request                    */
/*                                                                         */
/***************************************************************************/

void SendSQTimeRequest(struct V *V) {

    AbortSQTimeRequest(V);
    V->SQTimeReq->tr_node.io_Command = TR_ADDREQUEST;                       /* Create timer request */
    V->SQTimeReq->tr_time.tv_secs    = 0;
    V->SQTimeReq->tr_time.tv_micro   = V->SQUpdateFreq;

    SendIO(V->SQTimeReq);                                                   /* Send timer request */

    V->SQTimeReqOutstanding = 1;                                            /* Timer request is outstanding */
    }



/***************************************************************************/
/*                                                                         */
/*  AbortSQTimeRequest - Abort a Q-Code timer request                      */
/*                                                                         */
/***************************************************************************/

void AbortSQTimeRequest(struct V *V) {

    if (V->SQTimeReqOutstanding) {                                          /* Is there an active request to abort? */

        AbortIO(V->SQTimeReq);                                              /* Abort IO Request */

        Wait(1L << V->SQTimeReplyPort->mp_SigBit);                          /* Wait for message to come back */
        while (GetMsg(V->SQTimeReplyPort));

        V->SQTimeReqOutstanding = 0;                                        /* No outstanding timer request */
        }
    }



void EchoKey(UBYTE Keystroke, struct V *V) {

struct InputEvent InputEvent;

    InputEvent.ie_NextEvent        = 0;
    InputEvent.ie_Class            = IECLASS_RAWKEY;
    InputEvent.ie_SubClass         = 0;
    InputEvent.ie_Code             = 0xFF00|Keystroke;
    InputEvent.ie_Qualifier        = 0;
    InputEvent.ie_position.ie_addr = 0;

    V->InputReq->io_Command = IND_WRITEEVENT;
    V->InputReq->io_Data    = &InputEvent;
    V->InputReq->io_Length  = sizeof(struct InputEvent);
    DoIO(V->InputReq);
    }


static UBYTE doKeyStroke( UWORD keystroke, struct V *V );

struct Message *DoPlayer(struct Message *StartupMsg, struct V *V) {

int   Exit = FALSE;

ULONG Event;
ULONG DiskChangeSignal;
ULONG TimeSignal;
ULONG SQTimeSignal;
ULONG CDPlaySignal;
ULONG TaskMsgSignal;
UBYTE LastDownStroke = 0;

struct Message *TaskMsg;

struct ChangeData InterruptData;
struct Interrupt  InterruptNode = {

    { NULL, NULL, NT_INTERRUPT, 0, "Player Library Eject Interrupt" },
    NULL,
    (VOID (*)())DiskChangeIntr
    };

struct PlayerState *PlayerState = &V->PlayerBase->PlayerState;

    InterruptNode.is_Data      = (APTR)&InterruptData;
    InterruptData.LibraryTask  = FindTask(NULL);
    InterruptData.Signal       = 1L << DISKCHANGESIGBIT;

    V->DiskChangeReq->io_Command = CD_ADDCHANGEINT;
    V->DiskChangeReq->io_Length  = sizeof(struct Interrupt);
    V->DiskChangeReq->io_Data    = (APTR)&InterruptNode;
    SendIO(V->DiskChangeReq);

    DiskChangeSignal = 1L << DISKCHANGESIGBIT;
    TimeSignal       = 1L << V->TimeReplyPort->mp_SigBit;
    SQTimeSignal     = 1L << V->SQTimeReplyPort->mp_SigBit;
    CDPlaySignal     = 1L << V->CDPlayReplyPort->mp_SigBit;
    TaskMsgSignal    = 1L << V->PlayerBase->TaskMsgPort->mp_SigBit;

    GetDiskInfo(V);

    ReplyMsg(StartupMsg);

    while (!Exit) {

        Event = Wait(DiskChangeSignal | TimeSignal | SQTimeSignal | CDPlaySignal | TaskMsgSignal);

        if (Event & DiskChangeSignal) GetDiskInfo(V);

        if (Event & SQTimeSignal) {

            while (GetMsg(V->SQTimeReplyPort));

            V->SQTimeReqOutstanding = 0;

            if (PlayerState->PlayState == PLS_PLAYING) if (GetValidQCode(V)) UpdateVFDPosition(V);

            if ( !V->SQTimeReqOutstanding ) SendSQTimeRequest(V);
            }

        if (Event & TimeSignal) {

            while (GetMsg(V->TimeReplyPort));

            V->TimeReqOutstanding = 0;

            if (PlayerState->PlayMode == PLM_SKIPFWD || PlayerState->PlayMode == PLM_SKIPREV) {

                if (!LastDownStroke) {

                    if (PlayerState->PlayState >= PLS_PLAYING) {

                        Play(V);
                        PlayerState->PlayMode = PLM_NORMAL;
                        }
                    }

                else {

                    SkipTrack(LastDownStroke, V);
                    SendTimeRequest(SKIPTRACKCOUNT, V);
                    }
                }

            else {

                if      (LastDownStroke == PKS_FORWARD) FastForward(V);
                else if (LastDownStroke == PKS_REVERSE) FastReverse(V);                 
                }
            }

        if (Event & CDPlaySignal) {

            while (GetMsg(V->CDPlayReplyPort));

            AbortSQTimeRequest(V);

            V->CDPlayReqOutstanding = 0;

            if ((V->CDPlayReq->io_Error) && (V->CDPlayReq->io_Error != CDERR_ABORTED)) {

                Stop(V);
                }

            if (PlayerState->PlayState != PLS_STOPPED) PlayNextPrevTrack(V);
            else                                       Stop(V);
            }

        if (Event & TaskMsgSignal) {

            while (TaskMsg = GetMsg(V->PlayerBase->TaskMsgPort)) {

                if (TaskMsg->mn_Length == MSG_SHUTDOWN) {

                    Exit = TRUE;
                    break;
                    }

                else if (TaskMsg->mn_Length == MSG_TIMEMODE) {

                    if (PlayerState->PlayState == PLS_SELECTED || PlayerState->PlayState == PLS_PAUSED) {

                        UpdateVFDPosition(V);
                        }

                    FreeMem(TaskMsg, sizeof(struct Message));
                    }

                else {

                    UWORD keystroke = TaskMsg->mn_Length;

                    if (LastDownStroke) {

                        if ( ( keystroke & PKSF_STROKEDIR ) == PKSF_PRESS ) {

                            LastDownStroke = doKeyStroke( LastDownStroke | PKSF_RELEASE, V );
                            }

                        else {

                            if ( ( keystroke & PKSF_KEY ) != LastDownStroke ) keystroke = 0;
                            }
                        }

                    else {

                        if ( ( keystroke & PKSF_STROKEDIR ) == PKSF_RELEASE ) keystroke = 0;
                        }

                    if ( keystroke ) LastDownStroke = doKeyStroke( keystroke, V );

                    FreeMem(TaskMsg, sizeof(struct Message));
                    }
                }
            }
        }

    Stop(V);

    V->DiskChangeReq->io_Command = CD_REMCHANGEINT;
    V->DiskChangeReq->io_Length  = sizeof(struct Interrupt);
    V->DiskChangeReq->io_Data    = (APTR)&InterruptNode;
    DoIO(V->DiskChangeReq);

    return(TaskMsg);
    }


static UBYTE doKeyStroke( UWORD keystroke, struct V *V ) {

UWORD forceplay = 0;
UBYTE KeyPressed;
struct PlayerState *PlayerState = &V->PlayerBase->PlayerState;
UBYTE Track;

    if ((keystroke & PKSF_STROKEDIR) == PKSF_PRESS) KeyPressed = keystroke & PKSF_KEY;
    else                                            KeyPressed = 0;

    if ((keystroke & PKSF_KEY) == PKS_EJECT) {

        keystroke = ( keystroke & PKSF_STROKEDIR ) | PKS_PLAYPAUSE;
        forceplay = 1;
        }

    EchoKey(keystroke & (PKSF_KEY | PKSF_STROKEDIR), V);

    switch(keystroke) {

        case (PKS_PLAYPAUSE | PKSF_PRESS):

            switch (PlayerState->PlayState) {

                case PLS_PLAYING: if (!forceplay) Pause(V);  break;
                case PLS_PAUSED:  Resume(V);                 break;
                case PLS_STOPPED:
                case PLS_NUMENTRY: 

                    ObtainSemaphore(&V->PlayerBase->PlayListSemaphore);
                    V->PlayListAllocated = 1;

                    if (Track = GetFirstItem(V)) {

                        ObtainSemaphore(&V->PlayerBase->PlayStateSemaphore);

                        VFD0000(V);
                        V->PlayerBase->PlayerState.Track     = Track;
                        V->PlayerBase->PlayerState.PlayState = PLS_SELECTED;

                        ReleaseSemaphore(&V->PlayerBase->PlayStateSemaphore);

                        NormalPlay(V);
                        Play(V);
                        }

                    else {

                        ReleaseSemaphore(&V->PlayerBase->PlayListSemaphore);
                        V->PlayListAllocated = 0;
                        }

                    break;

                case PLS_SELECTED:

                    ObtainSemaphore(&V->PlayerBase->PlayListSemaphore);
                    V->PlayListAllocated = 1;

                    NormalPlay(V);
                    Play(V);
                    break;
                } 

            break;

        case (PKS_STOP | PKSF_PRESS):

            Stop(V);
            break;

        case (PKS_REVERSE | PKSF_PRESS):
        case (PKS_FORWARD | PKSF_PRESS):

            AbortTimeRequest(V);

            if (PlayerState->PlayState < PLS_PLAYING
                || PlayerState->PlayState == PLS_PAUSED
                || PlayerState->PlayMode  == PLM_SKIPFWD
                || PlayerState->PlayMode  == PLM_SKIPREV) SkipTrack(keystroke, V);

            SendTimeRequest(FASTMODECOUNT, V);

            break;

        case (PKS_REVERSE | PKSF_RELEASE):
        case (PKS_FORWARD | PKSF_RELEASE):

            AbortTimeRequest(V);

            if (PlayerState->PlayState >= PLS_PLAYING) {

                if (PlayerState->PlayMode == PLM_FFWD || PlayerState->PlayMode == PLM_FREV) NormalPlay(V);

                else {

                    AbortPlay(V);

                    if (PlayerState->PlayMode != PLM_SKIPFWD && PlayerState->PlayMode != PLM_SKIPREV) {

                        if ((keystroke & PKSF_KEY) == PKS_FORWARD) {

                            SkipTrack(keystroke & PKSF_KEY, V);
                            }

                        else {

                            VFD0000(V);
                            PlayerState->PlayMode = PLM_SKIPREV;
                            }
                        }

                    SendTimeRequest(FASTMODECOUNT, V);
                    }
                }

            break;
        }

    return( KeyPressed );
    }


LONG __asm DiskChangeIntr(register __a1 struct ChangeData *cd) {

    Signal(cd->LibraryTask, cd->Signal);
    return(0);
    }




void __saveds PlayerTask(void) {

struct V             *V;
struct Message       *StartupMsg;
struct Message       *ShutdownMsg = NULL;
struct PlayerLibrary *PlayerBase;

    PlayerBase = *((struct PlayerLibrary **)(FindTask(NULL)->tc_SPLower));

    Wait(1L << PlayerBase->TaskMsgPort->mp_SigBit);

    while (1) {

        StartupMsg = GetMsg(PlayerBase->TaskMsgPort);

        if (StartupMsg) if (StartupMsg->mn_Length == MSG_STARTUP) break;
        }

    if (V = AllocMem(sizeof(struct V), MEMF_PUBLIC | MEMF_CLEAR)) {

        V->PlayerBase = PlayerBase;

        if (V->CDReplyPort = CreateMsgPort()) {

            AddPort(V->CDReplyPort);

            if (V->CDReq = CreateStdIO(V->CDReplyPort)) {

                if (V->CDPlayReplyPort = CreateMsgPort()) {

                    AddPort(V->CDPlayReplyPort);

                    if (V->CDPlayReq = CreateStdIO(V->CDPlayReplyPort)) {

                        if (V->DiskChangeReplyPort = CreateMsgPort()) {

                            AddPort(V->DiskChangeReplyPort);

                            if (V->DiskChangeReq = CreateStdIO(V->DiskChangeReplyPort)) {

                                if (!OpenDevice("cd.device", 0, V->CDReq, 0)) {

                                    V->CDPlayReq->io_Device  = V->DiskChangeReq->io_Device = V->CDReq->io_Device;
                                    V->CDPlayReq->io_Unit    = V->DiskChangeReq->io_Unit   = V->CDReq->io_Unit;

                                    InstallSampler(V);

                                    if (V->TimeReplyPort = CreateMsgPort()) {

                                        AddPort(V->TimeReplyPort);

                                        if (V->TimeReq = (struct timerequest *)
                                            CreateExtIO(V->TimeReplyPort, sizeof(struct timerequest))) {

                                            if (V->SQTimeReplyPort = CreateMsgPort()) {

                                                AddPort(V->SQTimeReplyPort);

                                                if (V->SQTimeReq = (struct timerequest *)
                                                    CreateExtIO(V->SQTimeReplyPort, sizeof(struct timerequest))) {

                                                    if (V->InputReplyPort = CreateMsgPort()) {

                                                        if (V->InputReq = CreateStdIO(V->InputReplyPort)) {

                                                            if (!OpenDevice("input.device", 0, (struct IORequest *)V->InputReq, 0)) {

                                                                if (!OpenDevice("timer.device", UNIT_VBLANK, V->TimeReq, 0)) {

                                                                    V->SQTimeReq->tr_node.io_Device = V->TimeReq->tr_node.io_Device;
                                                                    V->SQTimeReq->tr_node.io_Unit   = V->TimeReq->tr_node.io_Unit;

                                                                    ShutdownMsg = DoPlayer(StartupMsg, V);

                                                                    CloseDevice(V->TimeReq);
                                                                    }

                                                                CloseDevice(V->InputReq);
                                                                }

                                                            DeleteStdIO(V->InputReq);
                                                            }

                                                        DeleteMsgPort(V->InputReplyPort);
                                                        }

                                                    DeleteExtIO(V->SQTimeReq);
                                                    }

                                                RemPort(V->SQTimeReplyPort);
                                                DeleteMsgPort(V->SQTimeReplyPort);
                                                }

                                            DeleteExtIO(V->TimeReq);
                                            }

                                        RemPort(V->TimeReplyPort);
                                        DeleteMsgPort(V->TimeReplyPort);
                                        }

                                    RemoveSampler(V);
                                    CloseDevice(V->CDReq);
                                    }

                                DeleteStdIO(V->DiskChangeReq);
                                }

                            RemPort(V->DiskChangeReplyPort);
                            DeleteMsgPort(V->DiskChangeReplyPort);
                            }

                        DeleteStdIO(V->CDPlayReq);
                        }

                    RemPort(V->CDPlayReplyPort);
                    DeleteMsgPort(V->CDPlayReplyPort);
                    }

                DeleteStdIO(V->CDReq);
                }

            RemPort(V->CDReplyPort);
            DeleteMsgPort(V->CDReplyPort);
            }

        FreeMem(V, sizeof(struct V));
        }

    if (!ShutdownMsg) {

        StartupMsg->mn_Length = 0;
        ReplyMsg(StartupMsg);
        }

    else ReplyMsg(ShutdownMsg);

    Wait(0);
    }

