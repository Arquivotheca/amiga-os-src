
#include <exec/types.h>
#include <exec/ports.h>
#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "cduibase.h"
#include "doanimation.h"
#include "transitions.h"


/*****************************************************************************/


/* Animation states:
 *
 * as_Nothing is the initial state. Blank screen, with nothing happening.
 *
 * When the startup packet comes in, we switch to either as_MainAnim
 * or as_BootingAnim. as_MainAnim is what happens when there is no disk
 * to boot from. This is the big one. as_BootingAnim is what we do when
 * there is a title that is starting to boot.
 *
 * When we are in as_MainAnim state, and a disk insertion event occurs,
 * we switch to the as_DoorClosed state. This is a transition state,
 * where the CD starts spinning. Once in the as_DoorClosed state, we can
 * be asked to reenter the as_MainAnim state, or as_BootingAnim state. This
 * depends whether the user actually inserted a bootable CDGS title.
 *
 * Once in the as_BootingAnim state, the only possible state transition is
 * to as_ClosingAnim, which is where the animation code completely shuts down
 * and gets out of the system.
 *
 * STATE       MESSAGE             NEW STATE       COMMENT
 * --------------------------------------------------------------------------
 * as_Nothing  ANIMMSG_STARTANIM   as_MainAnim     Start playing main
 *                                                 animation
 *
 * as_Nothing  ANIMMSG_BOOTING     as_BootingAnim  Start playing the low-cost
 *                                                 booting animation as a
 *                                                 title is in the process of
 *                                                 booting.
 *
 * as_MainAnim ANIMMSG_DOORCLOSED  as_DoorClosed   Transition from the main
 *                                                 animation to the "disk
 *                                                 was inserted" animation.
 *
 * as_MainAnim Controller Button   as_ClosingAnim  Transition from the main
 *                                                 animation directly to the
 *                                                 closing animation sequence,
 *                                                 as the user requested that
 *                                                 the language selector or
 *                                                 nvram editor be displayed.
 *
 * as_DrClosed ANIMMSG_BOOTING     as_BootingAnim  Transition from the "disk
 *                                                 was inserted" animation to
 *                                                 the low-cost "booting"
 *                                                 animation, as a bootable
 *                                                 title was inserted.
 *
 * as_DrClosed ANIMMSG_RESTARTANIM as_MainAnim     Transition from the "disk
 *                                                 was inserted" animation to
 *                                                 the full-blown animation.
 *                                                 Although the CD door was
 *                                                 closed, there is no bootable
 *                                                 CD title in it.
 *
 * as_DrClosed ANIMMSG_SHUTDOWN    as_ClosingAnim  Transition from the "disk
 *                                                 was inserted" animation to
 *                                                 the closing animation, as
 *                                                 a CDTV title is booting.
 *
 * as_Booting  ANIMMSG_SHUTDOWN    as_ClosingAnim  Transition from the
 *                                                 "booting" animation to the
 *                                                 exit animation. This has to
 *                                                 be very low-cost and clean
 *                                                 up everything.
 *
 *
 * DoAnimation() must be as responsive as possible to incoming messages,
 * especially to the controller button sequences. Therefore, as many sub-states
 * as possible must exist in order to allow exiting of any major state at any
 * moment.
 *
 * Messages come in to the port. mn_Length of the messages hold the message
 * information, and cause state transitions as per the transition table above.
 * Controller button presses are also monitored when in the as_MainAnim
 * state to determine if the user wishes to enter either of the two UI panels.
 *
 * Messages received by DoAnimation() should be replied quickly, except for
 * ANIMMSG_SHUTDOWN messages, which should be held until everything is
 * completely shutdown and the code is about to return. In this case,
 * DoAnimation() should return *in Forbidden state* with a CDUI_EXIT result.
 *
 * If the animation is in the as_MainAnim state, and detects button presses,
 * the function should transition to as_ClosingAnim, and return with a
 * CDUI_LANGUAGE_SELECTOR or CDUI_NVRAM_EDITOR result. This will cause the
 * UI panels to be displayed by the code in cdui.c. Once the user is done
 * with the UI panels, control will come back to DoAnimation(), which should
 * reenter as_MainAnim state.
 */

enum AnimStates
{
    as_Nothing,
    as_MainAnim,
    as_DoorClosedAnim,
    as_BootingAnim,
    as_ClosingAnim
};


/*****************************************************************************/


ULONG DoAnimation(struct CDUILib *CDUIBase, struct MsgPort *port)
{
struct Message *msg;
struct Message *holdMsg;
ULONG           portInfo;
ULONG           key;
enum AnimStates state;
ULONG           eventType;

    /* startup state */
    state   = as_Nothing;
    holdMsg = NULL;

    CDUIBase->cb_XOffset = 0;
    CDUIBase->cb_YOffset = 0;

    if (CDUIBase->cb_DidAnimation)
    {
        /* we've done the animation before, redo it again */
        eventType = ANIMMSG_STARTANIM;
    }
    else
    {
        /* if we've never been here before, then wait for the startup packet */
        WaitPort(port);
        msg = GetMsg(port);
        eventType = msg->mn_Length;
        ReplyMsg(msg);
        CDUIBase->cb_DidAnimation = TRUE;
    }

    while (TRUE)
    {

        switch (state)
        {
            case as_Nothing        : if (eventType == ANIMMSG_STARTANIM)
                                     {
                                         NothingToMain(CDUIBase);
                                         state = as_MainAnim;
                                     }
                                     else if (eventType == ANIMMSG_BOOTING)
                                     {
                                         NothingToBooting(CDUIBase);
                                         state = as_BootingAnim;
                                     }

                                     else if ((eventType == ANIMMSG_RED_BUTTON)
                                          ||  (eventType == ANIMMSG_BLUE_BUTTON)) {

                                         state = as_ClosingAnim;
                                         CDUIBase->cb_DidAnimation = FALSE;
                                         }

                                     break;

            case as_MainAnim       : if (eventType == ANIMMSG_DOORCLOSED)
                                     {
                                         MainToDoorClosed(CDUIBase);
                                         state = as_DoorClosedAnim;
                                     }
                                     else if ((eventType == ANIMMSG_RED_BUTTON)
                                       ||     (eventType == ANIMMSG_BLUE_BUTTON)
                                       ||     (eventType == ANIMMSG_SHUTDOWN))
                                     {
                                         MainToClosing(CDUIBase);
                                         state = as_ClosingAnim;
                                     }
                                     break;

            case as_DoorClosedAnim : if (eventType == ANIMMSG_BOOTING)
                                     {
                                         DoorClosedToBooting(CDUIBase);
                                         state = as_BootingAnim;
                                     }
                                     else if (eventType == ANIMMSG_RESTARTANIM)
                                     {
                                         DoorClosedToMain(CDUIBase);
                                         state = as_MainAnim;
                                     }
                                     else if (eventType == ANIMMSG_SHUTDOWN)
                                     {
                                         DoorClosedToClosing(CDUIBase);
                                         state = as_ClosingAnim;
                                     }
                                     break;

            case as_BootingAnim    : if (eventType == ANIMMSG_SHUTDOWN)
                                     {
                                         BootingToClosing(CDUIBase);
                                         state = as_ClosingAnim;
                                     }
                                     break;

            case as_ClosingAnim    : /* we don't care what we get here, we're leaving */
                                     break;

            default                : /* bogus state */
                                     break;
        }

        if (state == as_ClosingAnim)
        {
            if (holdMsg)
            {
                RemoveAllocPatch(CDUIBase);
                Forbid();
                ReplyMsg(holdMsg);
                return(CDUI_EXIT);
            }

            if (eventType == ANIMMSG_RED_BUTTON)
                return(CDUI_NVRAM_EDITOR);

            if (eventType == ANIMMSG_BLUE_BUTTON)
                return(CDUI_LANGUAGE_SELECTOR);
        }

        if (state == as_BootingAnim)
        {
            if (holdMsg)
            {
                RemoveAllocPatch(CDUIBase);
                ReplyMsg(holdMsg);
                holdMsg = NULL;
            }
        }

        eventType = ANIMMSG_NOP;
        do
        {
            WaitTOF();

            if (msg = GetMsg(port))
            {
                eventType = msg->mn_Length;
                if (eventType == ANIMMSG_SHUTDOWN)
                {
                    // don't reply to shutdown messages until we're shut down
                    holdMsg = msg;
                }
                else if ((eventType == ANIMMSG_BOOTING) && (state == as_DoorClosedAnim))
                {
                    holdMsg = msg;
                }
                else
                {
                    ReplyMsg(msg);
                }
            }
            else if (state == as_MainAnim)
            {
                portInfo = ReadJoyPort(1) & ~JP_TYPE_MASK;
                if (portInfo == 0)
                    portInfo = ReadJoyPort(0) & ~JP_TYPE_MASK;

                key = GetKey();
                if ((portInfo & JPF_BUTTON_RED) || (key == 0x50))
                {
                    eventType = ANIMMSG_RED_BUTTON;
                }
                else if ((portInfo & JPF_BUTTON_BLUE) || (key == 0x51))
                {
                    eventType = ANIMMSG_BLUE_BUTTON;
                }
            }
        }
        while (eventType == ANIMMSG_NOP);
    }
}
