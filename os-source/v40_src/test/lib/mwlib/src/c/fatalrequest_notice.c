/****** FatalRequest.module/notice ********************************************

    NAME
        notice -- Display information.

    SYNOPSIS
        notice( format, arg1, arg2, ... )

        void notice( char *, arg ... );

    FUNCTION
        Displays the given message as a requester, then waits for user
        to respond by clicking the OK gadget. notice() is normally
        used for giving the user information that a specific event did or
        did not occur.

    INPUTS
        format          - printf() type format.

        arg1, arg2 ...  - arguments for the notice.

    RESULT
        Displays a requester (or an alert, if out of memory), and waits for
        user response. Returns nothing.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        ask(), SetFatalScreen().
******************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

void notice(char *f, ...)
{
    struct Library *IntuitionBase = _SetIntuition();
    struct EasyStruct es;
    ULONG *a = (ULONG *) &f;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "Notice";
    es.es_TextFormat = f;
    es.es_GadgetFormat = "OK";

    _GetResponse(IntuitionBase, _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]), a[10], a[11]);
    _UndoIntuition(IntuitionBase);
}
