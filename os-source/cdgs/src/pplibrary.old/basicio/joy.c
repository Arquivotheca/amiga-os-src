/* :ts=4
*
*       joy.c
*
*       William A. Ware                                                 B207
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY                                            *
*   Copyright (C) 1991, Silent Software, Incorporated.                                          *
*   All Rights Reserved.                                                                                                        *
****************************************************************************/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/ports.h>

#include <devices/inputevent.h>
#include <devices/input.h>
#include <devices/gameport.h>
#include <devices/timer.h>

#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include "joymouse.h"

#include <clib/exec_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>

extern char __far _LibName[];
extern char __far TimerDeviceName[];
char __far gameportname[] = "gameport.device";
char __far inputname[]    = "input.device";


char __far JoyMousePortName[] = JOYMOUSEPORTNAME;
/*
 * Spoof the #pragmas
 */
#define SysBase         (PlayerPrefsBase->ppb_ExecBase)
#define DeBoxBase       (PlayerPrefsBase->ppb_DeBoxBase)
#define GfxBase         (PlayerPrefsBase->ppb_GfxBase)



        /* 
         * Get and Set the controller type.  
         * - If the controller is already allocated then the program fails
         *   and exits the program.
         * - If it is not allocated then the controller is set to joystick mode.
         * 
         * The code to do this is in a critical section.
         */

VOID __saveds JoyMouse() {

struct MsgPort                 *inport;
struct IOStdReq                *gp_req = NULL, *in_req = NULL;
struct timerequest             *timer = NULL;
UBYTE                           opened = 0, type = 0;
struct InputEvent               event;
struct GamePortTrigger          gpt; 
register UWORD                  lastcode = -1;
register ULONG                  lastmove = 0, move;
struct PlayerPrefsBase         *PlayerPrefsBase;
UWORD                           notrep = 1;                             /* We have not signaled calling task yet */
struct Message                 *msg = NULL;

    if (!(PlayerPrefsBase = (struct PlayerPrefsBase *) OpenLibrary(_LibName,0L))) return;

    if (!(inport = (struct MsgPort *)CreatePort(JoyMousePortName,0L))) goto xit;        

    if (!(gp_req = AllocIORequest( gameportname, 1, 0, sizeof(struct IOStdReq)))) goto xit;

    if (!(in_req = AllocIORequest( inputname, 0, 0, sizeof(struct IOStdReq)))) goto xit;

    if (!(timer = AllocIORequest( TimerDeviceName, UNIT_VBLANK, 0, sizeof( *timer )))) goto xit;

    Forbid();
    gp_req->io_Command = GPD_ASKCTYPE;
    gp_req->io_Length  = 1;
    gp_req->io_Flags   = IOF_QUICK;
    gp_req->io_Data = (APTR)&type;
    DoIO(gp_req);
    
    if (type == GPCT_NOCONTROLLER) {

        type               = GPCT_ABSJOYSTICK;
        gp_req->io_Command = GPD_SETCTYPE;
        gp_req->io_Flags   = IOF_QUICK;
        gp_req->io_Length  = 1;
        gp_req->io_Data    = (APTR)&type;
        DoIO(gp_req);
        opened = 1;
        Permit();
        }

    else {

        Permit();
        goto xit;
        }

    sigPPTask(PlayerPrefsBase,1);                                           /* We have got this far -- We've made it signal calling task */
    notrep = 0;                                                             /* It is now false we have replied */
    
    gpt.gpt_Keys        = GPTF_UPKEYS + GPTF_DOWNKEYS;                      /* If we have made it this far, then we can send the data for what we want to come from the port. */
    gpt.gpt_Timeout     = 3;
    gpt.gpt_XDelta      = 1;
    gpt.gpt_YDelta      = 1;
    gp_req->io_Command  = GPD_SETTRIGGER;
    gp_req->io_Length   = (LONG)sizeof(struct GamePortTrigger);
    gp_req->io_Data     = (APTR)&gpt;
    DoIO(gp_req);

    gp_req->io_Command  = GPD_READEVENT;                                     /* Set up the structures to recieve the messages from the gameport and send the messages to the input device. */
    gp_req->io_Length   = sizeof(struct InputEvent);
    gp_req->io_Data     = (APTR)&event;
    in_req->io_Command  = IND_WRITEEVENT;
    in_req->io_Flags    = 0;
    in_req->io_Length   = sizeof(struct InputEvent);
    in_req->io_Data     = (APTR)&event;

    timer->tr_node.io_Command = TR_GETSYSTIME;

    while(!(msg = GetMsg(inport))) {                                         /* Will return at least every 3 vert blanks. */

        DoIO(gp_req); 
        move = (ULONG)event.ie_EventAddress;

        if (move || (event.ie_Code != lastcode)) {

            if (lastmove != 0) {

                event.ie_X <<= 3;
                event.ie_Y <<= 3;
                }

            DoIO(timer);
            event.ie_TimeStamp.tv_secs  = timer->tr_time.tv_secs;
            event.ie_TimeStamp.tv_micro = timer->tr_time.tv_micro;
            DoIO(in_req);
            }

        lastmove = move;
        lastcode = event.ie_Code;
        }
xit:
    Forbid();

    if (opened) {

        type                = GPCT_NOCONTROLLER;
        gp_req->io_Command  = GPD_SETCTYPE;
        gp_req->io_Flags    = IOF_QUICK;
        gp_req->io_Length   = 1;
        gp_req->io_Data     = (APTR)&type;
        DoIO(gp_req);
        }

    if (timer)  FreeIORequest(timer);
    if (gp_req) FreeIORequest(gp_req);
    if (in_req) FreeIORequest(in_req);
    if (msg)    ReplyMsg(msg);
    if (inport) {

        while( msg = GetMsg(inport) ) ReplyMsg(msg);
        DeletePort(inport);
        }

    if (notrep) sigPPTask(PlayerPrefsBase,1);

    CloseLibrary(PlayerPrefsBase);
    Permit();
    }



/****** playerprefs.library/InstallJoyMouse ********************************
*
*   NAME
*       InstallJoyMouse -- Install joystick event rerouting task.
*
*   SYNOPSIS
*       success = InstallJoyMouse ();
*         D0
*
*       LONG    success;
*
*   FUNCTION
*       The CDTV remote control has this little harmless-looking button
*       labelled "JOY/MOUSE."  This enables the IR remote to appear as
*       either a mouse in gameport 0 or as a joystick in gameport 1.
*
*       This routine installs a task that sniffs gameport 1 and sends any
*       events it finds there down the input food chain.  The events are
*       identical to ordinary mouse events.  This enables applications
*       expecting a mouse to work no matter what mode the IR remote is in.
*
*       This call does NOT nest.
*
*   INPUTS
*
*   RESULT
*       success - non-zero if task successfully installed or already there;
*                 zero otherwise
*
*   EXAMPLE
*
*   NOTES
*       The only difference between joystick and mouse events is very
*       subtle; the ie_SubCode field of the InputEvent is set to the
*       gameport unit number from which the event originated.
*
*   BUGS
*
*   SEE ALSO
*       RemoveJoyMouse()
*
*****************************************************************************
*/

__asm LIBInstallJoyMouse(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

int ans = 1;

    ObtainSemaphore(&PlayerPrefsBase->ppb_LibLock);

    Forbid();

    if (!(FindPort(JoyMousePortName))) {

        Permit();
        ans = launchPPTask(JoyMousePortName, 2, (APTR)JoyMouse, 1000, PlayerPrefsBase);
        }

    else Permit();

    ReleaseSemaphore(&PlayerPrefsBase->ppb_LibLock);
    return(ans);
    }



/****** playerprefs.library/RemoveJoyMouse *********************************
*
*   NAME
*       RemoveJoyMouse -- Remove joystick event rerouter.
*
*   SYNOPSIS
*       RemoveJoyMouse ();
*
*   FUNCTION
*       This routine kills off the joystick rerouting task started with
*       InstallJoyMouse().  All resources it allocated are freed.  A
*       corresponding call to InstallJoyMouse() need not have been made.
*
*       This call does NOT nest; it happens immediately.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       InstallJoyMouse()
*
*****************************************************************************
*/

VOID __asm LIBRemoveJoyMouse(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

struct Message msg;
struct MsgPort *port,*rp;
        
    Forbid();

    if (port = FindPort(JoyMousePortName)) {

        MemSet( &msg, 0, sizeof(struct Message));

        if (rp = (struct MsgPort *)CreatePort(0L, 0L)) {

            msg.mn_ReplyPort    = rp;
            msg.mn_Node.ln_Type = NT_MESSAGE;
            PutMsg(port, &msg);
            Permit(); 
            WaitPort(rp);
            DeletePort(rp);
            }

        else Permit();
        }

    else Permit();
    }



