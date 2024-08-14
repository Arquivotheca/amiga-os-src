/****** FatalRequest.module/__background__ ************************************
    NAME
        FatalRequest -- The Ultra-easy request system

    AUTHOR
        Mitchell/Ware Systems, Inc

    VERSION
        2.08    9/15/88
        3.00    25 Sep 1991 - Break up into individual modules

    FUNCTION
        Requesters/Alerts that apprise user of a possibly serious condition,
        or ask a YES/NO question.

        FatalRequest will attempt to open these up as system requests.
        Failing that, they will default to an Alert.

******* FatalRequest.module/SetFatalWindow ************************************

    NAME
        SetFatalWindow -- (OBSOLETE) Tell FatalRequest where to put the requesters

    SYNOPSIS
        SetFatalWindow( window )

        void SetFatalWindow( struct Window * );

    FUNCTION
        Tells FatalRequest what screen to place the requesters on. If this
        function is not executed, or if a NULL is pass, then the requesters
        will appear on the Workbench screen.

    INPUTS
        window      - reference window from which to determine screen.

    RESULT
        none.

    EXAMPLE

    NOTES

    BUGS
        Not reentrant. Window is stored in a static area.
        No longer works. 25 Sep 1991 FM

    SEE ALSO

******* FatalRequest.module/fatal *********************************************

    NAME
        fatal -- Display a fatal alert to user, then exit.

    SYNOPSIS
        bool = fatal( message1 )
        bool = fatal2( message1, message2 )
        bool = fatal3( message1, message2, message3 )

        BOOL fatal( char * );
        BOOL fatal2( char *, char * );
        BOOL fatal3( char *, char *, char * );

    FUNCTION
        Displays the given fatal messages as a requester, then waits for user
        to respond by clicking the YES or NO gadget. If he clicks YES, then
        the programmer-defined routine StateSave() will be called before
        exiting. Exiting is expected to be performed via the
        programmer-defined ShutDown() routine, which should never return.

        fatal() is generally used in the case that your program has reached
        an unrecoverable state, and must exit. It provides the user with
        some useful information, and a last chance to save any work he has
        done up to this point.

        It may be that your program is too cripled to actually save
        sucessfully at this point. That is why the user is told
        (automatically) that a StateSave will be ATTEMPTED.

        Please keep in mind that a fatal request is normally used to indicate
        that a bug in the software has been decteded. Users should
        normally never see a fatal request ootherwise.

    INPUTS
        message1    - First line of text to be displayed. Generally used for
                      explaining the nature of the bug.

        message2    - Second line of text to be displayed. Usually for
                      diagnostic information.

        message3    - Third line of text to be displayed. Usually for more
                      explicit diagnostic information.

    RESULT
        Displays a requester (or an alert, if out of memory), and waits for
        user response. Calls your StateSave() on a positive (YES) response,
        and in both cases calls your ShutDown(). If ShutDown() returns,
        which it should never do, fatal() returns a BOOLean representing
        the user's response.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        warning(), SetFatalScreen().

******* FatalRequest.module/warning *******************************************

    NAME
        warning -- Display a warning requester, and await user's response

    SYNOPSIS
        bool = warning( message1 )
        bool = warning2( message1, message2 )
        bool = warning3( message1, message2, message3 )

        BOOL warning( char * );
        BOOL warning2( char *, char * );
        BOOL warning3( char *, char *, char * );

    FUNCTION
        Displays the given warning messages as a requester, then waits for user
        to respond by clicking the YES or NO gadget. warning() is normally
        used to indicate a user error.

    INPUTS
        message1    - First line of text to be displayed. Generally used for
                      explaining the nature of the warning.

        message2    - Second line of text to be displayed. Usually for
                      discriptive information.

        message3    - Third line of text to be displayed. Usually for more
                      explicit discriptive information.

    RESULT
        Displays a requester (or an alert, if out of memory), and waits for
        user response. Returns a BOOLean indicating the user's response.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        fatal(), ask(), SetFatalScreen().

******* FatalRequest.module/ask ***********************************************

    NAME
        ask -- Display a query.

    SYNOPSIS
        bool = ask( message1 )
        bool = ask2( message1, message2 )
        bool = ask3( message1, message2, message3 )

        BOOL ask( char * );
        BOOL ask2( char *, char * );
        BOOL ask3( char *, char *, char * );

    FUNCTION
        Displays the given query messages as a requester, then waits for user
        to respond by clicking the YES or NO gadget. ask() is normally
        used for querying the user just before doing something unrecoverable,
        such as overwriting a file or deleting information.

    INPUTS
        message1    - First line of text to be displayed. Generally used for
                      explaining the nature of the question.

        message2    - Second line of text to be displayed. Usually for
                      discriptive information.

        message3    - Third line of text to be displayed. Usually for more
                      explicit discriptive information.


    RESULT
        Displays a requester (or an alert, if out of memory), and waits for
        user response. Returns a BOOLean indicating the user's response.

    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        warning(), SetFatalScreen().

*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include "protos.h"

struct Library *_SetIntuition()
{
    return OpenLibrary("intuition.library", 37);
}

void _UndoIntuition(struct Library *IntuitionBase)
{
    CloseLibrary(IntuitionBase);
}

void SetFatalWindow(struct Window *win)
{
}

int _GetResponse(struct Library *IntuitionBase, struct Window *win)
{
    int ret = win;

    if ((BOOL) win != 0 && (BOOL) win != 1)
    {
        while ((ret = SysReqHandler(win, NULL, TRUE)) == -2)
            ;
        FreeSysRequest(win);
    }
    return ret;
}

struct Window *_FRBuildEasyRequest(struct Library *IntuitionBase, struct Window *win, struct EasyStruct *es, ULONG idcmp, ...)
{
    return BuildEasyRequestArgs(win, es, idcmp, (APTR) (((ULONG *) &idcmp)+1));
}
