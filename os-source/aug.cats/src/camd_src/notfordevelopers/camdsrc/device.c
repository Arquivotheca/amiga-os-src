/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* device.c    - MIDI device driver support
*
************************************************************************/

/* !!! need hook for expunge.  it'd be nice if we could get expunges
       w/o having library usage count == 0 */

/* !!! temp expunge hook in use */

#include <exec/types.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <midi/camddevices.h>
#include <string.h>

#include "camdlib.h"


#define Static static


/* --------------------
   CAMD DATA STRUCTURES
   -------------------- */
struct MidiDeviceNode
{  struct Node Node;
   WORD OpenCount;          /* given this we can do expunges as necessary */
   struct MidiDeviceData *Data;
   BPTR Segment;
};

Static struct DeviceData
{
   struct List DeviceList;
   struct SignalSemaphore DeviceLock;
} DeviceData;


/* -------------------
   FUNCTION PROTOTYPES
   ------------------- */

#define LockDeviceList()    ObtainSemaphore (&DeviceData.DeviceLock)
#define UnlockDeviceList()  ReleaseSemaphore (&DeviceData.DeviceLock)


    /* internal */
static struct MidiDeviceNode *LoadMidiDevice (UBYTE *name);
static struct Node *FindIName (struct List *, UBYTE *name);
static void ExpungeMidiDevice (struct MidiDeviceNode *);
static struct MidiDeviceData *FindMidiDeviceData (BPTR segment);


/* Public Functions */

/***************************************************************
*
*   OpenMidiData
*
***************************************************************/

/****** camd.library/OpenMidiDevice ******************************************
*
*   NAME
*       OpenMidiDevice -- Open a MIDI device driver.
*
*   SYNOPSIS
*       struct MidiDeviceData *OpenMidiDevice (UBYTE *Name)
*                                                a0
*
*   FUNCTION
*       Opens a MIDI device driver.
*
*       This function should only be called by Preferences program that
*       wishes to interrogate a particular MIDI device driver for such
*       information as number of Ports.
*
*       Otherwise this is a private camd.library function.
*
*   INPUTS
*       Name - name of MIDI device driver.
*
*   RESULTS
*       Pointer to MidiDeviceData structure or NULL on failure.
*
*   SEE ALSO
*       CloseMidiDevice()
****************************************************************************
*
*/

struct MidiDeviceData * __saveds __asm LIBOpenMidiDevice (register __a0 UBYTE *name)
{
   register struct MidiDeviceNode *dnode;

   LockDeviceList();

   if ((dnode = (void *)FindIName (&DeviceData.DeviceList, name)) ||
      (dnode = LoadMidiDevice (name)))
      {
      dnode->OpenCount++;
      }

   UnlockDeviceList();

   return dnode ? (void *)dnode->Data : NULL;
}


/***************************************************************
*
*   CloseMidiData
*
***************************************************************/

/****** camd.library/CloseMidiDevice ******************************************
*
*   NAME
*       CloseMidiDevice -- Close a MIDI device driver.
*
*   SYNOPSIS
*       void CloseMidiDevice (struct MidiDeviceData *MidiDeviceData)
*                                         a0
*
*   FUNCTION
*       Closes a MIDI device driver opened by OpenMidiDevice().
*
*   INPUTS
*       MidiDeviceData - pointer to a MidiDeviceData structure returned by
*                    OpenMidiDevice().
*
*   RESULTS
*       None
*
*   SEE ALSO
*       OpenMidiDevice()
****************************************************************************
*
*/

void __saveds __asm LIBCloseMidiDevice (register __a0 struct MidiDeviceData *data)
{
/* dart */
   register struct MidiDeviceNode *dnode;

   LockDeviceList();

   ScanList (&DeviceData.DeviceList, dnode)
      {
      if (dnode->Data == data)
         {
         if (!--dnode->OpenCount) ExpungeMidiDevice (dnode);      /* !!! expunge is temporary here! */
         break;
         }
      }
   UnlockDeviceList();
}


/* Internal Functions */

/***************************************************************
*
*   InitDevices
*
*   FUNCTION
*       Init DeviceData.
*
*   INPUTS
*       None
*
*   RESULTS
*       None
*
***************************************************************/

void InitDevices(void)
{
   NewList (&DeviceData.DeviceList);
   InitSemaphore (&DeviceData.DeviceLock);
}


/* Private Functions */

Static
struct MidiDeviceNode *LoadMidiDevice (UBYTE *name)
{
   register struct MidiDeviceNode *dnode;
   BPTR oldcdir, midilock=0;

   /* lattice warns of uninit'ed auto here.  it's wrong */
   if (!(dnode = AllocMem (sizeof(struct MidiDeviceNode), MEMF_CLEAR|MEMF_PUBLIC))) goto clean;

   if (!(midilock = Lock ("devs:midi", SHARED_LOCK))) goto clean;

   oldcdir = CurrentDir (midilock);
   if (!(dnode->Segment = LoadSeg ((void *)name))) goto clean;
   CurrentDir (oldcdir);
   UnLock (midilock);
   midilock = NULL;

   if (!(dnode->Data = FindMidiDeviceData (dnode->Segment))) goto clean;
   if (stricmp (dnode->Data->Name, name)) goto clean;          /* check for name match */
   dnode->Node.ln_Name = dnode->Data->Name;

   if (!(*dnode->Data->Init)()) goto clean;

   AddTail (&DeviceData.DeviceList, (void *)dnode);

   return dnode;

clean:
   if (dnode)
      {
      if (dnode->Segment) UnLoadSeg (dnode->Segment);
      FreeMem (dnode, (long)sizeof *dnode);
      }
   if (midilock)
      {
      CurrentDir (oldcdir);
      UnLock (midilock);
      }
   return NULL;
}

Static struct MidiDeviceData *FindMidiDeviceData (BPTR segment)
{
   struct MidiDeviceData *mdd = (void *)((CPTR)BADDR(segment) + MDD_SegOffset);

   return mdd->Magic == MDD_Magic ? mdd : NULL;
}


Static void ExpungeMidiDevice (struct MidiDeviceNode *dnode)
{
   if (dnode)
      {
      Remove ((void *)dnode);
      (*dnode->Data->Expunge)();
      UnLoadSeg (dnode->Segment);
      FreeMem (dnode, (long)sizeof *dnode);
      }
}



/* case insensitive stuff */

static struct Node *FindIName (struct List *list, UBYTE *name)
{
   register struct Node *node;

   ScanList (list, node)
      {
      if (!stricmp ((char *)node->ln_Name, (char *)name)) return node;
      }
   return NULL;
}

/* eof */
