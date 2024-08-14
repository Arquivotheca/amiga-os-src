
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>

#include <cdtv/debox.h>
#include <cdtv/cdtvprefs.h>

#include "screensaver.h"

#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"
#include "/basicio/viewmgr.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>

/*------------------------------------------------------------------*/

/*
 * Local forward fucntion declarations.  (I hate ANSI.)
 */
LONG __asm __saveds
LIBInstallScreenSaver(register __a6 struct PlayerPrefsBase *);
LONG __asm __saveds
LIBScreenSaverCommand(register __d0 ULONG,
              register __a6 struct PlayerPrefsBase *);

/****** playerprefs.library/InstallScreenSaver ****************************
*
*   NAME
*   InstallScreenSaver -- Installs a systemwide screen saver facility.
*
*   SYNOPSIS
*   success = InstallScreenSaver ();
*
*   LONG success;
*
*   FUNCTION
*   This routine installs a screen saver.  After a time of inactivity,
*   the prevailing display is replaced with the CDTV logo moving around
*   on the screen.  This is to keep static images from burning into the
*   user's display.
*
*   This call does NOT nest.
*
*   INPUTS
*
*   RESULT
*   success - non-zero if the screen saver task was successfully
*         started, or is already running.
*
*   EXAMPLE
*
*   NOTES
*   The screen saver is implemented as an autonomous task and input
*   handler.  The input handler monitors input events.  If no meaningful
*   input event occurs after a preset time, a signal is sent to the
*   task which brings up the saver display.
*
*   The input event that takes down the saver display is eaten.
*   You should bear this in mind if you intend to permit the saver to
*   come up while your controls are still "active."
*
*   This screen saver is suitable for programs using the input
*   device.  Programs that go direct to the low level resources
*   should use this blanker in manual mode...by sending blank and
*       unblank commands as needed.
*
*   This facility SetFunction()s LoadView(), thus preventing other Views
*   from being brought to the front while the saver is active.  When the
*   saver takes down its animation, the last View that was set with
*   LoadView() will be loaded and LoadView() will be restored to normal
*   operation.
*
*   The screen saver's resource needs, when quiescent, are very modest;
*   roughly 4K of stack, and less than 1K of dynamically allocated
*   structures.  (It gets *real* big when it kicks in, though...)
*
*   BUGS
*   If the screen saver task fails to start for whatever reason, there's
*   no way to find this out, short of noticing that the task is no
*   longer present.
*
*   SEE ALSO
*   ScreenSaverCommand(), cdtv/screensaver.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBInstallScreenSaver(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    return( 1 );
    }
/****** playerprefs.library/ScreenSaverCommand ****************************
*
*   NAME
*   ScreenSaverCommand -- Send a command to the screen saver facility.
*
*   SYNOPSIS
*   result = ScreenSaverCommand (cmd);
*
*   LONG    result;
*   ULONG   cmd;
*
*   FUNCTION
*   This routine locates the screen saver's IPC port and sends it the
*   command supplied.  This routine is synchronous; it returns after
*   the command has completed.  The result returned is command-specific.
*
*   The commands are:
*
*   SCRSAV_DIE:
*       Kill off the screen saver facility.  If the saver is active,
*       it will take down the saver animation.  All resources are
*       freed, and the task is terminated.  Result is always non-
*       zero.
*
*       NOTE:  This operation does NOT nest!  When you call it,
*       it happens immediately, no matter how many other clients
*       opened it.
*
*   SCRSAV_SAVE:
*       Bring up the saver animation now.  This operation observes
*       the current _FAKEIT setting (q.v.).  If it successfully
*       brought up the saver, result will be non-zero.  If any of
*       the allocations failed, result will be zero.
*
*   SCRSAV_UNSAVE:
*       Take down the saver screen now.  This operation observes the
*       current _FAKEIT setting (q.v.).  The inactivity timer is
*       restarted.  Result is always non-zero.
*
*   SCRSAV_FAKEIT:
*       Keep track of inactive time, and return active/inactive when
*       interrogated with SCRSAV_ISACTIVE (q.v.), but don't
*       *actually* save the screen.  This is so outside programs can
*       poll the saver (don't busy wait!) to see if they should take
*       down their own screens in a special way before letting the
*       saver do it's thing.  This command also inhibits calls to
*       SCRSAV_SAVE.  Result is always non-zero.
*
*   SCRSAV_UNFAKEIT:
*       Cancel the above feature; make the saver behave normally.
*       This operation will bring up the saver animation immediately
*       if it's supposed to be up right now (except that it isn't
*       because you performed SCRSAV_FAKEIT earlier).  In this case,
*       result will be zero if it failed to bring up the saver
*       animation.  Otherwise, in all other cases, result will be
*       non-zero.
*
*   SCRSAV_ISACTIVE:
*       Asks the saver if it's currently displaying the saver
*       animation.  Result is non-zero if the saver is active.
*
*   If cmd is OR'ed with SCRSAV_TIMECMD, then the lower 15 bits of cmd
*   are interpreted as the amount of inactive time to wait before
*   activating the saver animation.  This time value is expressed in
*   seconds.  A time value of zero is interpreted as infinity.  Result
*   is always non-zero.
*
*   INPUTS
*   cmd - Command to saver facility.
*
*   RESULT
*   result  - Command-dependent; see above.
*
*   EXAMPLE
*
*   NOTES
*   If the screen saver facility has not been installed, or the supplied
*   command value is invalid, result is always zero.
*
*   BUGS
*
*   SEE ALSO
*   InstallScreenSaver(), cdtv/screensaver.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBScreenSaverCommand(register __d0 ULONG command,
                                          register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    LONG retval = 1;

    /* Trivial Pursuit and maybe others need ISACTIVE to return "no" */
    if ( command == SCRSAV_ISACTIVE )
    {
	retval = 0;
    }
    return( retval );
    }
