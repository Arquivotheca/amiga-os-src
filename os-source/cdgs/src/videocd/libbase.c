/* libbase.c
 * standard library base stuff
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "libbase.h"

/*****************************************************************************/

extern ASM ULONG LowMemory (REG (a0) struct MemHandlerData *, REG (a6) struct ExecBase *);

/*****************************************************************************/

struct Library *ASM LibInit (REG (d0) struct LibBase *lb, REG (a0) ULONG *seglist, REG (a6) struct Library * sysbase)
{
    struct Interrupt *is = &lb->lb_IS;

    /* Initialize all of our stuff */
    lb->lb_SegList = seglist;
    lb->lb_SysBase = sysbase;
    InitSemaphore (&lb->lb_Lock);
    NewList ((struct List *) &lb->lb_HandleList);

    if (lb->lb_SysBase->lib_Version >= 39)
    {
	/* Open the libraries that we need */
	lb->lb_UtilityBase = OpenLibrary ("utility.library", 39);

	/* Initialize the low memory handler */
	is->is_Node.ln_Pri  = 1;
	is->is_Node.ln_Name = lb->lb_Lib.lib_Node.ln_Name;
	is->is_Data         = (APTR) lb;
	is->is_Code         = (void (*)()) LowMemory;
	AddMemHandler (is);

	return lb;
    }
    return NULL;
}

/*****************************************************************************/

LONG ASM LibOpen (REG (a6) struct LibBase *lb)
{
    /* Use an internal use counter */
    lb->lb_Lib.lib_OpenCnt++;
    lb->lb_Lib.lib_Flags &= ~LIBF_DELEXP;

    return ((LONG) lb);
}

/*****************************************************************************/

LONG ASM LibClose (REG (a6) struct LibBase *lb)
{
    LONG retval = NULL;

    /* Decrement the use count */
    if (lb->lb_Lib.lib_OpenCnt)
	lb->lb_Lib.lib_OpenCnt--;

    if (lb->lb_Lib.lib_Flags & LIBF_DELEXP)
	retval = LibExpunge (lb);

    return (retval);
}

/*****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct LibBase *lb)
{
    ULONG *seg = lb->lb_SegList;

    /* Remove the library */
    Remove ((struct Node *) lb);

    /* Remove the memory handler */
    RemMemHandler (&lb->lb_IS);

    /* Close the libraries that we opened */
    CloseLibrary (lb->lb_UtilityBase);

    /* Free all that funky memory */
    FreeMem ((APTR)((ULONG)(lb) - (ULONG)(lb->lb_Lib.lib_NegSize)), lb->lb_Lib.lib_NegSize + lb->lb_Lib.lib_PosSize);

    return ((LONG) seg);
}
