/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
* libc.c      - main C module for library (just initialization really)  *
************************************************************************/

#include "camdlib.h"

/* -----------
   Global Data
   ----------- */

struct XCamdBase *CamdBase;
struct Library *DOSBase, *UtilityBase;
APTR MiscBase;


/***************************************************************
*
*   Global Functions
*
***************************************************************/

/***************************************************************
*
*   InitLibC
*
*   FUNCTION
*       Init C vars and subsystems during library init.
*       Called by InitLib() assembly code.
*
*       Any partial initialization from here gets cleaned
*       up in CleanUpC().  Guaranteed to be called only
*       once during life of library.
*
*   INPUTS
*       camdbase - CamdBase pointer
*
*   RESULTS
*       Success.
*
***************************************************************/

BOOL __saveds __asm InitLibC (register __a0 struct XCamdBase *camdbase)
{
    short i;

    CamdBase = camdbase;

        /* Open resources and libraries */
    if (!(MiscBase = camdbase->MiscBase = OpenResource ("misc.resource")) ||
        !(DOSBase = camdbase->DOSBase = OpenLibrary ("dos.library", OS_MIN))) goto clean;

    UtilityBase = camdbase->UtilityBase = OpenLibrary ("utility.library", OS_MIN);
  #if OS_MIN >= 36
    if (!UtilityBase) goto clean;
  #endif

        /* init CamdBase lists */
    NewList ((struct List *)&camdbase->DriverList);
    NewList ((struct List *)&camdbase->BytePortList);
    NewList ((struct List *)&camdbase->MidiList);
    NewList ((struct List *)&camdbase->ClusterList);
    NewList ((struct List *)&camdbase->NotifyList);

        /* init semaphores */
    InitSemaphore (&camdbase->MidiListLock);
    for (i=0; i<CD_NLocks; i++) InitSemaphore (&camdbase->CamdSemaphores[i]);

    InitDevices();
    InitUnits();

    return TRUE;

clean:
    return FALSE;
}


/***************************************************************
*
*   CleanUpC
*
*   FUNCTION
*       Cleanup anything allocated by InitLibC().
*       Called by CleanUp() assembly code.
*
*       Guaranteed to be called exactly once during life of
*       library.
*
*   INPUTS
*       none
*
*   RESULTS
*       none
*
***************************************************************/

void __saveds CleanUpC (void)
{
    CleanUpUnits();

    if (UtilityBase) CloseLibrary (UtilityBase);
    if (DOSBase) CloseLibrary (DOSBase);
}
