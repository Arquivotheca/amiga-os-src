/****** mwlib/BSTRcptoSTR ****************************************************
    Mitchell/Ware Systems           Version 1.00            08-Dec-88

    NAME
        BSTRcptoSTR -- copy a BSTR to a 'C' STR.

    SYNOPSIS
        void BSTRcptoSTR (UBYTE *bstr, UBYTE *buf)

    FUNCTION
        Copies a BSTR to 'C' string buffer. The buffer is assumed to
        be large enough to contain the string plus the terminating NULL.

    INPUT
        bstr    - The BSTR
        buf     - The receiving 'C' string buffer.

    RESULT
        None.
        Fills the given buffer with the C-style string.

    SEE ALSO
        BSTRtoSTR()
*****************************************************************************/

#include <exec/types.h>
#include <dos/dosextens.h>

void BSTRcptoSTR (UBYTE *bstr, UBYTE *buf)
{
    bstr = (UBYTE *) BADDR(bstr);
    CopyMem(bstr+1,buf,*bstr);
    buf[*bstr] = 0;
}