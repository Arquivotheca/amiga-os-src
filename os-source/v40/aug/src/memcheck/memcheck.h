#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/datetime.h>
#include <dos/dosextens.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <libraries/commodities.h>
#include <libraries/gadtools.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/commodities_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/commodities_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*****************************************************************************/

#include "memcheckheader.h"

/*****************************************************************************/

#include "globaldata.h"

/*****************************************************************************/

extern struct GlobalData *gd;
extern ULONG TAG1, TAG2;

/*****************************************************************************/

/* stubs.asm */
void *ASM nAllocMem (REG (d0) ULONG size, REG (d1) ULONG attributes);
void *ASM nAllocAbs (REG (d0) ULONG size, REG (a1) APTR address);
void  ASM nFreeMem  (REG (a1) void *memb, REG (d0) ULONG size);
ULONG ASM nAvailMem (REG (d1) ULONG req);
void *ASM nAllocVec (REG (d0) ULONG size, REG (d1) ULONG attributes);
void  ASM nFreeVec  (REG (a1) void *memb);

/* allocabs.asm */
VOID * ASM AllocAbsolute (REG (d0) ULONG size, REG (a1) APTR addr, REG (a0) struct MemHeader *);

/* setmemory.asm */
void ASM SetMemory (REG (a0) struct GlobalData *gd, REG (d0) ULONG flags);
void ASM MemoryCorrupt (REG (a2) struct MemHeader *mh);

/* misc.asm */
void ASM MemoryCorrupt (REG (a2) struct MemHeader *mh);
void ASM MungMem (REG (a0) char *ptr, REG (d0) ULONG size, REG (d1) ULONG value);
ULONG ASM TestMem (REG (a0) VOID *memb, REG (a6) struct Library *);
STRPTR ASM GetProcessName (REG (a0) struct Task *tc);

/* other... */
extern void NewList (struct MinList *);
extern void kprintf (void *,...);

#include "memcheck_iprotos.h"

