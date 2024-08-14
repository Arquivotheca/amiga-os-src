/****** CALib.library/MakeRDA *************************************************

    NAME
        MakeRDA -- Make an RDArgs structure for scanning a buffer

    VERSION
        1.00    09 Aug 1991 - Taken from AACommand module

    AUTHOR
        Fred Mitchell

    SYNOPSIS
        struct RDArgs *MakeRDA(char *buf)

    FUNCTION
        Sets up an RDArgs structure for parsing a NULL-terminated
        buffer by ReadArgs().

    INPUTS
        buf - pointer to a NULL-terminated buffer to be parsed.
              A copy of buf will be made.

    RESULTS
        Returns a pointer to a RDArgs structure, which must be freed
        by calling DestroyRDA(). Returns a NULL pointer if out of
        memory.

    EXAMPLE
        if (rda = MakeRDA(buf))
        {
            if (ReadArgs(TEMPLATE, &args, rda))
            {
                ...
                FreeArgs(rda);
            }
            DestroyRDA(rda);
        }
        ...

    SEE ALSO
        DestroyRDA()

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#include <proto/exec.h>

struct RDArgs *MakeRDA(char *buf)
{
    struct RDArgs *rda;
    char *str;

    if (rda = AllocVec(sizeof(*rda), MEMF_CLEAR|MEMF_ANY))
    {
        if (str = AllocVec(strlen(buf)+2, MEMF_CLEAR|MEMF_ANY))
        {
            strcat(strcpy(str, buf), "\n");
            rda->RDA_Source.CS_Buffer = str;
            rda->RDA_Source.CS_Length = strlen(str);
        }
        else // memory allocation error
        {
            FreeVec(rda);
            rda = NULL;
        }
    }

    return rda;
}

/****** CALib.library/DestroyRDA *************************************************

    NAME
        DestroyRDA -- Deallocate memory used by MakeRDA()

    VERSION
        1.00    09 Aug 1991 - Taken from the AACommand module

    SYNOPSIS
        void DestroyRDA(struct RDArgs *rda)

    FUNCTION
        Frees memory used by MakeRDA(), returning it back to the system.
        DOES NOT do a FreeArgs() - you must do that yourself (see MakeRDA()).

    INPUTS
        rda - The RDArgs structure allocated by MakeRDA().

    RESULTS
        Nothing's returned.

    SEE ALSO
        MakeRDA().

*********************************************************************************/

void DestroyRDA(struct RDArgs *rda)
{
    FreeVec(rda->RDA_Source.CS_Buffer);
    FreeVec(rda);
}
