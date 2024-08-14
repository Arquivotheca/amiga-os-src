/****** MWLib/StrFlag *********************************************************
    NAME
        StrFlag -- StringFlag Conversion

    SYNOPSIS
        ULONG StrFlag (ULONG *pflag, char *str);

    FUNCTION
        Sets and resets bits according to the given string. String is a
        stream of 1's and 0's with the following interpretation:
            if '1' then set the corresponding bit
            if '0' then reset the corresponding bit
            if 'X' then ignore the bit.
        Least significant bit is to the left, not the right.

    INPUTS
        pflag   - A pointer to the ULONG flag field to be modified.
        str     - The string modifier.

    RESULT
        bits in *pflag become modified according to the specifications. The
        new pattern is also returned.

        If an error is generated, a requester is displayed, and the return
        will not match the contents of *pflag.
******************************************************************************/

#include <exec/types.h>

ULONG StrFlag(ULONG *pf, char *s)
{
    int i;

    for (i = 0; i < 32 && s[i]; ++i)
        switch(s[i])
        {
        case '1':   *pf |= 1 << i; break;
        case '0':   *pf &= ~(1 << i); break;
        case 'x':
        case 'X':   break;
        default:    notice("StrFlag: ?String %s given,\nError at char %ld", s, i); return ~*pf;
        }
    return *pf;
}
