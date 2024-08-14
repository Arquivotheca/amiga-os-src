/****** mwlib/c/StrDupVec *****************************************************

    NAME
        StrDupVec -- Copy a string using AllocVec()

    SYNOPSIS
        char *StrDupVec(char *string)

    FUNCTION
        Copies string, allocating space for it using AllocVec().
        Null pointers and null strings are allowed.

        This allows a way to pass ownership of strings to other
        tasks.

    INPUTS
        string  - string to be duplicated

    RESULTS
        Returns NULL if out of memory or a NULL pointer was passed,
        else a pointer to the copy of string is returned.

        NOTE, unlike strdup(), strings copied with StrDupVec()
        must be freed with FreeVec() when you are done.

    NOTES

    SEE ALSO
        keyStrDup()

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct Library *SysBase;

char *StrDupVec(char *string)
{
    char *dup = NULL;

    if (string && (dup = AllocVec(strlen(string)+1, MEMF_CLEAR|MEMF_PUBLIC)))
        strcpy(dup, string);

    return dup;
}
