/****** FatalRequest.module/BusyWait ******************************************

    NAME
        BusyWait -- Asynchronous user display request

    VERSION
        1.00    09 Jan 1992 - Inception

    AUTHOR
        Copyright © 1992 Xenolog, Inc.

    SYNOPSIS
        void *BusyWait(char *text)

    FUNCTION
        Displays a requester for user asynchronously, with a cancel
        button, to allow user a chance to cancel a running operation.

        The program must call BusyCheck() every so often to see if
        user hit the requester, and call BusyClear() to remove the
        requester.

    INPUTS
        text    - text to be display for the user to read.

    RESULTS
        Returns a private handle to be used by BusyCheck() and
        BusyClear(), or NULL if failed.

    SEE ALSO
        BusyCheck() and BusyClear()

******************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

void *BusyWait(char *text)
{
    struct EasyStruct es;
    struct Library *IntuitionBase = _SetIntuition();
    void *ret;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "Wait a Second...";
    es.es_TextFormat = "%s\nHit ABORT to interrupt operation.";
    es.es_GadgetFormat = "Abort";

    ret = (void *) _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, text);
    _UndoIntuition(IntuitionBase);
    return ret;
}

/****** FatalRequest.module/BusyCheck *****************************************

    NAME
        BusyCheck -- Check the status of the BusyWait requester

    VERSION
        1.00    09 Jan 1992

    SYNOPSIS
        long BusyCheck(void *handle, long progress, long goal)

    FUNCTION
        Checks requester for a return status.

    INPUTS
        handle      - Handle gotten from BusyWait().
        progress    - progress made (0 thru goal, inclusive)
        goal        - value progress is to approach. A display
                      proportional to progress / goal may be
                      updated in requester. If not used, then
                      both goal and progress should be zero.

    RESULTS
        Returns a -2 if user has not responded, or a -1 if some other
        non-user event was received, or 0 if the user wishes to abort.

        If the progress/goal indicator is implemented, requester will
        be updated accordingly.

        Remember that you MUST call BusyClear() to actually remove
        the requester.

    SEE ALSO
        BusyWait(), BusyClear().

******************************************************************************/

long BusyCheck(void *handle, long progress, long goal)
{
    struct Library *IntuitionBase = _SetIntuition();
    long ret = 0;

    if (handle)
    {
        ret = SysReqHandler((struct Window *) handle, NULL, FALSE);
    }

    _UndoIntuition(IntuitionBase);
    return ret;
}

/****** FatalRequest.module/BusyClear *****************************************

    NAME
        BusyClear -- Check the status of the BusyWait requester

    VERSION
        1.00    09 Jan 1992

    SYNOPSIS
        void BusyClear(void *handle)

    FUNCTION
        Clears the BusyWait requester. Must be called.

    INPUTS
        handle  - The handle returned by

******************************************************************************/

void BusyClear(void *handle)
{
    struct Library *IntuitionBase = _SetIntuition();

    if (handle)
        FreeSysRequest((struct Window *) handle);

    _UndoIntuition(IntuitionBase);
}
