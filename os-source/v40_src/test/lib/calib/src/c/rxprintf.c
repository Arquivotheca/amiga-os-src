/****** CALib.library/RXPrintf ************************************************

    NAME
        RXPrintf -- formats an Argstring.

    VERSION
        1.00    09 Aug 1991 - Taken from AACommand

    AUTHOR
        Fred Mitchell

    SYNOPSIS
        UBYTE *RXPrintf(char *fmt, ULONG p1, ...)

    FUNCTION
        Uses RawDoFmt() to format an Argstring for ARexx. Follows all of the
        conventions of RawDoFmt().

    INPUTS
        fmt     - the format string
        p1, ... - The list of parameters

    RESULTS
        returns a pointer to an ARexx Argstring, which can be passed
        back to ARexx as a result string.

    NOTE
        To use this function, the rexxsyslib.library MUST be opened!!!

    SEE ALSO
        exec/RawDoFmt

******************************************************************************/

#include <exec/types.h>
#include <rexx/storage.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>

UBYTE *RXPrintf(char *fmt, ULONG p1, ...)   // Convert parameters to an ArgString
{
    extern void StuffChar();    // assembly routine to use with RawDoFmt()
    char buf[128];

    RawDoFmt(fmt, &p1, &StuffChar, &buf);
    return CreateArgstring(buf, strlen(buf));
}
