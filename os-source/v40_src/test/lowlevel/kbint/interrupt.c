/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <devices/inputevent.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/keymap_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/lowlevel_pragmas.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes
 */

#include "kbint.h"

/****** KBInt/kbUpdate ******************************************
*
*   NAME
*       kbUpdate -- update keyboard status
*
*   SYNOPSIS
*       kbUpdate();
*
*       void kbUpdate(void);
*
*   FUNCTION
*       Update keyboard status.
*
*   INPUTS
*       None
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
void kbUpdate(void)
{

    static struct InputEvent ie;        /* InputEvent for use
                                           with MapRawKey() */
    static UBYTE mapBuffer[KB_MAP_LENGTH+1]; /* Mapped key event
                                                (with terminator) */

    WORD actual;

    /* Map raw key */
    ie.ie_Class=IECLASS_RAWKEY;         /* Simulated IECLASS_RAWKEY event */
    ie.ie_SubClass=0;
    ie.ie_Code=kbCode&0xFFFF;           /* Key code is low-order word */
    ie.ie_Qualifier=
        (kbCode&LLKF_LSHIFT)?IEQUALIFIER_LSHIFT:NULL|
        (kbCode&LLKF_RSHIFT)?IEQUALIFIER_RSHIFT:NULL|
        (kbCode&LLKF_CAPSLOCK)?IEQUALIFIER_CAPSLOCK:NULL|
        (kbCode&LLKF_CONTROL)?IEQUALIFIER_CONTROL:NULL|
        (kbCode&LLKF_LALT)?IEQUALIFIER_LALT:NULL|
        (kbCode&LLKF_RALT)?IEQUALIFIER_RALT:NULL|
        (kbCode&LLKF_LAMIGA)?IEQUALIFIER_LCOMMAND:NULL|
        (kbCode&LLKF_RAMIGA)?IEQUALIFIER_RCOMMAND:NULL;
                                        /* Translate qualifiers to
                                           InputEvent format */
    ie.ie_EventAddress=&ie;
    actual=MapRawKey(&ie,mapBuffer,KB_MAP_LENGTH,NULL);
                                        /* Map using default keymap */
    mapBuffer[actual]='\0';             /* Null-terminate mapped key event */
    GT_SetGadgetAttrs(keyGadget,window,NULL,
        GTTX_Text, mapBuffer,
        TAG_DONE);                      /* Update Key gadget */

    /* Update Code gadget */
    GT_SetGadgetAttrs(codeGadget,window,NULL,
        GTNM_Number, (kbCode&0xFFFF)&(~IECODE_UP_PREFIX),
                                        /* Key code is low-order word;
                                           mask out up prefix */
        TAG_DONE);

    /* Update Transition gadget */
    GT_SetGadgetAttrs(transitionGadget,window,NULL,
        GTTX_Text, kbCode&IECODE_UP_PREFIX?"Release":"Press",
        TAG_DONE);

    /* Update LShift gadget */
    GT_SetGadgetAttrs(lShiftGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_LSHIFT,
        TAG_DONE);

    /* Update RShift gadget */
    GT_SetGadgetAttrs(rShiftGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_RSHIFT,
        TAG_DONE);

    /* Update Caps Lock gadget */
    GT_SetGadgetAttrs(capsLockGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_CAPSLOCK,
        TAG_DONE);

    /* Update Control gadget */
    GT_SetGadgetAttrs(controlGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_CONTROL,
        TAG_DONE);

    /* Update LAlt gadget */
    GT_SetGadgetAttrs(lAltGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_LALT,
        TAG_DONE);

    /* Update RAlt gadget */
    GT_SetGadgetAttrs(rAltGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_RALT,
        TAG_DONE);

    /* Update LAmiga gadget */
    GT_SetGadgetAttrs(lAmigaGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_LAMIGA,
        TAG_DONE);

    /* Update RAmiga gadget */
    GT_SetGadgetAttrs(rAmigaGadget,window,NULL,
        GTCB_Checked, kbCode&LLKF_RAMIGA,
        TAG_DONE);

}

/****** kbInt/kbInterrupt ******************************************
*
*   NAME
*       kbInterrupt -- keyboard interrupt handler
*
*   SYNOPSIS
*       kbInterrupt(rawKey,intData);
*
*   void __asm __interrupt __saveds kbInterrupt(register __d0 UBYTE rawKey,
*       register __a1 APTR intData)
*
*   FUNCTION
*       Keyboard interrupt handler.
*
*   INPUTS
*       rawKey -- raw key code
*       intData -- interrupt data; should be KBINT_COOKIE
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
void __asm __interrupt __saveds kbInterrupt(register __d0 UBYTE rawKey,
    register __a1 APTR intData)
{

    /* Check cookie; if bad ... */
    if (intData!=(APTR) KBINT_COOKIE) {
        /* Increment bad cookie counter */
        kbIntBadData++;
    }

    /* Get keyboard code */
    kbCode=GetKey();

    /* Signal main task */
    Signal(kbIntTask,1<<kbSignal);

}
