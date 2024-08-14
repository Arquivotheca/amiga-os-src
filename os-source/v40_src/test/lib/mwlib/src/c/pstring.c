/****** MWLib.library/PString *********************************************************

    NAME
        PString -- construct a parameter string

    VERSION
        1.00    02 Jan 1992 - Copied from Rmake and modified for ints

    AUTHOR
        Copyright © 1992 Mitchell/Ware Systems, Inc.

    SYNOPSIS
        long PString(char *dest, char *parm, char *keys, void *arg, ...);

    FUNCTION
        Assembles a ParmString based on the placement and matching
        of parameters and keys. The keys string is a null-terminated
        array of character-format pairs that map the following
        arguments. The arguments, which can be strings or longs,
        will be substituted for the occurence of the %key sequence
        in the parm string.

        dest points to a buffer assumed long enough to contain the
        generated string.

    INPUTS
        dest    - Destination buffer. Assumed large enough to contain string.
        parm    - Parameter string. Describes the method of substitution.
        keys    - Keys for the arguments that follow. Ordinal position
                  of the character pairs in the keys string MUST
                  correspond to the arguments presented. The format of
                  the character-pairs are as follows:

                    *f

                  Where * repesents the key character, and f represents
                  the format. The following formats are as follows:

                    s   - string
                    c   - single character
                    d   - decimal signed number
                    D   - decimal unsigned number
                    h   - hex signed number
                    H   - hex unsigned number
                    g   - double-length IEEE floating-point number (NIY)

        arg,... - Sequence of arguments that follow. NULL pointers are
                  allowed for strings.

    RESULTS
        Returns the actual number of bytes processed, and is present
        in dest. If there has been an error, will return -1 instead.

    EXAMPLE
        PString(buf, "copy from DF%d:%f to %t %a", "ddfstsas", 0, from, to, arguments);

    SEE ALSO

******************************************************************************/

#include <exec/types.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "o/PString.i"

struct keys {
    char key, format;
    };

long PString(char *dest, char *parm, char *keys, void *arg, ...)
{
    return _PString(dest, parm, (struct keys *) keys, &arg);
}

long _PString(char *dest, char *parm, struct keys *keys, void **arr)
{
    long d, p;  // counters for dest and parm, respectively
    long i; // key index

    for (d = p = 0; parm[p]; ++p)
        if (parm[p] == '%')
        {
            BOOL found;

            ++p;
            for (i = 0, found = FALSE; keys[i].key && keys[i].format; ++i)
                if (found = parm[p] == keys[i].key)
                    break;

            if (found) // did we find it?
            {
                switch(keys[i].format)
                {
                case 's': // string
                    if (arr[i])
                    {
                        strcpy(&dest[d], (char *) arr[i]);
                        d += strlen(arr[i]);
                    }
                    break;

                case 'c':   dest[d++] = (char) arr[i]; break;
                case 'd':   d += stcl_d(&dest[d], (long) arr[i]); break;
                case 'D':   d += stcul_d(&dest[d], (long) arr[i]); break;
                case 'h':   d += stcl_h(&dest[d], (long) arr[i]); break;
                case 'H':   d += stcl_h(&dest[d], (long) arr[i]); break;
                case 'g':   // NIY - later we'll use gcvt() or similar
                default:
                    d = -1;
                    break;
                }

                if (d == -1)
                    break;
            }
            else // nope! We have an error condition
            {
                d = -1;
                break;
            }
        }
        else
            dest[d++] = parm[p], dest[d] = NULL;

    return d;
}
