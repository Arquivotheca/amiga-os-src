#ifndef SOUNDPATCH_H
#define SOUNDPATCH_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/semaphores.h>


/*****************************************************************************/


struct SoundPatch
{
    struct SignalSemaphore sp_Semaphore;
    char		   sp_Name[14];
    ULONG		   sp_Flash;
    APTR	           sp_OldDisplayBeep;
    APTR		   sp_SysBase;
    APTR		   sp_PatchCodeStart;
};


/*****************************************************************************/


#endif /* SOUNDPATCH_H */
