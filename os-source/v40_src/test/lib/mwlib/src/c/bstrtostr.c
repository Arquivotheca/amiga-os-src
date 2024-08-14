/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.

    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.
*/
/****** mwlib/BSTRtoSTR *****************************************************
    Mitchell/Ware Systems           Version 1.01            08-Dec-88

    NAME
        BSTRtoSTR -- allocate and copy BSTR to a STR.

    SYNOPSIS
        UBYTE *BSTRtoSTR(BPTR b)

    FUNCTION
        Converts a BSTR to 'C' STRING, allocating a new string with calloc().

    INPUT
        b   - The BSTR

    RESULT
        Returns a pointer to a 'C' string, or NULL if out of memory.

    SEE ALSO
        BSTRcptoSTR()
*****************************************************************************/

#include <exec/types.h>
#include <dos/dosextens.h>

UBYTE *BSTRtoSTR(b)
ULONG b;
{
    UBYTE *bstr, *str, *s;
    int c;

    bstr = BADDR(b);
    str = s = calloc(1, (c = *bstr++) + 1);
    while(c--)
        *s++ = *bstr++;

    return str;
}
