

#include <exec/types.h>
#include <dos/dos.h>
#include <string.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>


/*****************************************************************************/


struct FakeFileHandle
{
    UBYTE *ffh_Data;
    LONG   ffh_Size;
    LONG   ffh_Offset;
};


/*****************************************************************************/


static LONG __asm ReadFunc(register __d1 struct FakeFileHandle *fh,
                           register __d2 STRPTR buff,
                           register __d3 ULONG len)
{
LONG result;

    if (len > fh->ffh_Size - fh->ffh_Offset)
        result = fh->ffh_Size - fh->ffh_Offset;
    else
        result = len;

    memcpy(buff, fh->ffh_Data + fh->ffh_Offset, result);
    fh->ffh_Offset += result;

    return (result);
}


/*****************************************************************************/


static VOID * __asm AllocFunc(register __d0 ULONG size,
                              register __d1 ULONG flags,
                              register __a6 struct Library *SysBase)
{
    return(AllocMem(size, flags));
}


/*****************************************************************************/


static VOID __asm FreeFunc(register __a1 APTR mem,
                           register __d0 ULONG size,
                           register __a6 struct Library *SysBase)
{
    FreeMem(mem,size);
}


/*****************************************************************************/


BPTR MemoryLoadSeg(struct Library *DOSBase, APTR buffer, ULONG bufSize)
{
struct FakeFileHandle fh;
ULONG                 fa[3];
LONG                  dummy;

    fh.ffh_Data   = buffer;
    fh.ffh_Size   = bufSize;
    fh.ffh_Offset = 0;

    fa[0] = (LONG)ReadFunc;
    fa[1] = (LONG)AllocFunc;
    fa[2] = (LONG)FreeFunc;

    return(InternalLoadSeg((BPTR)&fh, NULL, fa, &dummy));
}
