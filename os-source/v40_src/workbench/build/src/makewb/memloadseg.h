#ifndef MEMLOADSEG_H
#define MEMLOADSEG_H


/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif


/*****************************************************************************/


BPTR MemoryLoadSeg(struct Library *DOSBase, APTR buffer, ULONG bufSize);


/*****************************************************************************/


#endif  /* MEMLOADSEG_H */
