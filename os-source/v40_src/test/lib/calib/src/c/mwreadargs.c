/****** CALib.library/MWReadArgs **********************************************

    NAME
        MWReadArgs -- Substitute for ReadArgs

    AUTHOR
        Fred Mitchell

    VERSION
        1.00    16 Sep 1991

    SYNOPSIS
        struct RDArgs *ReadArgs(UBYTE *template,
                        LONG *array, struct RDArgs *rdargs)

    FUNCTION
        Replacement for the ReadArgs() supplied by dos.library. This
        implementation is a drop-in replacement for ReadArgs() and more.
        The differences are:

            *   /M can appear anywhere in the template and
                behave as as expected.

            *   Multiple synonyms can be specified
                e.g. "W=WHOLE=WHOLE_THING/K"

            *   Can be made keyword case-sensitive, specifiable
                from template.
                i.e. "Del/C"

            *   Options can be delimited a character of your
                choosing.
                e.g. "//D,..." uses slash as a delimiter

            *   Help can be placed in the template.
                e.g. "W=WHOLE(Create The Whole Thing)/K"

        All other rules concerning ReadArgs() should apply.

*****i* CALib.library/MWReadArgs **********************************************

    Switches grouped by class

        Switch Actions
            /S  - Switch
            /T  - Toggle

        Parse Attributes
            /K  - Keyword
            /A  - Required

        Type Attributes
            /N  - Number
            /F  - Rest of line
            /M  - Multiple Strings

        Keyword attributes
            /C  - Case sensitive keyword

        Delimiter
            /D  - Delimiter - MUST be at beginning of string, as in "//D"

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include "LinkTools.h"
#include "protos.h"

// Switches

#define MWRF_SWITCH     (1 << 0)
#define MWRF_TOGGLE     (1 << 1)
#define MWRF_KEYWORD    (1 << 2)
#define MWRF_REQUIRED   (1 << 3)
#define MWRF_NUMBER     (1 << 4)
#define MWRF_REST       (1 << 5)
#define MWRF_MULTIPLE   (1 << 6)
#define MWRF_CASE       (1 << 7)
#define MWRF_DELIMITER  (1 << 8)

// Structures

struct MWArgument {
    Link list;          // linked list of arguments
    ULONG swi;          // bits for switches
    char *keyword;      // pointer to keyword & synonyms OR pointer to delimiter
    long kcnt;          // count of keywords and synonyms
    char *comment;      // comment attached
    };

struct RDArgs *MWReadArgs(UBYTE *template, LONG *array, struct RDArgs *rdargs)
{
    UBYTE *t, *w;
    struct RDArgs *rda;
    long temlen;    // template length
    Link mwahead;
    struct MWArgument *mwa;

    InitLinkHead(&mwahead);

    if (rda = AllocDosObject(DOS_RDARGS, NULL))
    {
        if (rdargs)
        {
            *rda = *rdargs;
            rdargs->RDA_DAList = (LONG) rda;
        }
        else // init ourselves
        {
            rda->RDA_Buffer = NULL;
            rda->RDA_BufSiz = NULL;
            rda->RDA_ExtHelp = NULL;
            rda->RDA_Flags = NULL;
            memset(&rda->RDA_Source, NULL, sizeof(rda->RDA_Source));
        }
        rda->RDA_DAList = (LONG) rda; // we will use this as our delete vector

        if ((rda->RDA_Buffer && rda->RDA_BufSiz >= (temlen = strlen(template)+1))
            || (rda->RDA_BufSiz = NULL, rda->RDA_Buffer = AllocVec(temlen, MEMF_CLEAR)))
        {
            // RDA_BufSiz is a flag for us. If zero, we allocated the buffer!
            // parse template
            strcpy(rda->RDA_Buffer, template);
            for (t = w = rda->RDA_Buffer, mwa = NULL; *t; ++t)
            {
                switch(*t)
                {
                case '/':
                    switch(*++t)
                    {
                    case 'S':   // Switch
                    case 'T':   // Toggle
                    case 'K':   // Keyword
                    case 'A':   // Required

                    case 'N':   // Number
                    case 'F':   // Rest of line
                    case 'M':   // Multiple Strings

                    case 'C':   // Case sensitive keyword
                    case 'D':   // Delimiter - MUST be at beginning of string, as in "//D"
                    }
                    break;

                case '=':
                case '(':
                case ')':
                case ',':
                }
            }
        }
        else // failed -- clean up
        {
            FreeDosObject(DOS_RDARGS, rda);
            rda = NULL;
        }
    }

    return rda;
}

/****i* CALib.library/MWReadItem *****
    Spaces and '=' serve as delimiters
    "" are processed as a single item
    if delim is not NULL, use it instead of spaces as delimiter.
    "\0" and "\n" are terminators.
*************************************/

long MWReadItem(UBYTE *buffer, long maxchars, struct CSource *cs, char delim)
{
    char *b;
    long c;
    BOOL quote, done;
    long ret = ITEM_NOTHING;

    if (!delim)
        delim = ' ';

    for (b = buffer, done = quote = FALSE;
                !done && (ULONG) b - (ULONG) buffer < maxchars; )
    {
        switch (c = MWReadChar(cs))
        {
        case '\n':
        case '\0':  // terminator
            *b++ = NULL;
            done = TRUE;
            MWUnReadChar(cs);
            break;

        case '"':
            quote = !quote;
            break;

        default:
            if (((c == delim) || (c == '=')) && !quote)
            {
                *b++ = NULL;
                done = TRUE;
                MWUnReadChar(cs);
            }
            else // a legit character
                *b++ = c;

            break;
        }
    }

    return ret;
}

long MWReadChar(struct CSource *cs)
{
    long ch;

    if (cs && cs->CS_Buffer)
        ch = cs->CS_Buffer[cs->CS_CurChr++];
    else
        ch = FGetC(Input());

    return ch;
}

BOOL MWUnReadChar(struct CSource *cs)
{
    BOOL ret;

    if (ret = cs && cs->CS_Buffer)
        --cs->CS_CurChr;
    else
        ret = UnGetC(Input(), -1);

    return ret;
}
