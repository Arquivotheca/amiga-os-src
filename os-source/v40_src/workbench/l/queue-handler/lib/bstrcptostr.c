/*****************************************************************************
    BSTRcptoSTR.c

    Mitchell/Ware Systems           Version 1.00            08-Dec-88

    BSTR to STRING copy. Creates a NULL-terminated string.
*****************************************************************************/

#include <exec/types.h>
#include <dos/dosextens.h>

UBYTE *BSTRcptoSTR(ULONG b, UBYTE *to)
{
    UBYTE *bstr, *s;
    int c;

    for (bstr = BADDR(b), s = to, c = *bstr++; c--; *s++ = *bstr++)
        ;

    *s = NULL;
    return to;
}
