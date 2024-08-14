/****** MWLib/keyStrDup *******************************************************
    NAME
        keyStrDup -- duplicate a string using AllocRemember()

    VERSION
        1.00    ???
        1.01    14 Mar 1992 - Opens own Intuition.library

    SYNOPSIS
        char *keyStrDup(struct Remember **key, char *string)

    FUNCTION
        Creates a duplicate of a string just like strdup() does, except
        AllocRemember() is used to allocate the memory instead of malloc().

    INPUTS
        key     - pointer to pointer to the Remember structure.
        string  - Null-terminated string to be replicated.

    RESULT
        A ptr to a duplicate, NULL-terminated copy of string is returned,
        or NULL if memory could not be allocated. Memory allocated is
        of the type MEMF_PUBLIC.

    SEE ALSO
        intuition/intuition.h
        strdup()

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <string.h>

char *keyStrDup(struct Remember **key, char *str)
{
    char *dup;
    struct Library *IntuitionBase = OpenLibrary("intuition.library", 33);

    if (dup = (char *)  AllocRemember(key, strlen(str)+1, MEMF_PUBLIC))
        strcpy(dup, str);

    CloseLibrary(IntuitionBase);
    return dup;
}
