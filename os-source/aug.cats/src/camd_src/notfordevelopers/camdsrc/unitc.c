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
*************************************************************************
*
* unitc.c     - Unit related functions
*
************************************************************************/

#define DAVETEST	1			/* enable early driver loading */

/*
    Notes
    -----

    1.  Locking.

        The central UnitData.UnitsLock is used by functions that can be
        called when MICount==0 (i.e. while CloseUnits() is being
        called).  These functions are the ones that don't require a
        MidiNode pointer (SetMidiOutMask(), etc).  These functions
        additionally don't require using the individual locks since the
        locking used is only to permit single access to the individual
        functions.

        The individual unit->UnitLock is used by the transmit functions
        in unita.asm.  All of these functions require that units be open
        since they are called either by UnitTask() or indirectly by a
        application.  In either case, MICount can't be zero, and
        therefore units must be open.  Because of this, the central unit
        lock needn't be used, only the individual locks need to be used
        to permit only one task to transmit to a given unit at a time.
*/

#include <exec/types.h>
#include <clib/macros.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <libraries/iffparse.h>
#include <prefs/prefhdr.h>
#include <midi/camddevices.h>
#include <midi/midiprefs.h>
#include <string.h>

#include "camdlib.h"


#define Static /* !!! static */   /* cute way to make only certain symbols show up during debugging */


/* Units */

#define NByteHooks 2    /* number of Byte Hooks for each unit */

struct MidiUnit {
    struct SignalSemaphore UnitLock;
    UBYTE UnitNum;              /* index of unit in MidiUnit array (used when given a pointer and division isn't practical) */
    UBYTE Flags;                /* status flags */

    struct Task *UnitTask;      /* this unit's task */
    UBYTE *UnitTaskName;        /* allocated name for task */
    ULONG UnitRecvSigMask;      /* queue active signal mask for this unit */
    BYTE  UnitQuitSigBit;       /* shutdown signal bit for this unit */
    BYTE  UnitOpenCount;	/* */
    UWORD pad1;                 /* for long word alignment */

    UBYTE *XmitQueue, *XmitQueueEnd;
    UBYTE *XmitQueueHead, *XmitQueueTail;
    ULONG XmitQueueSize;        /* actual size of xmit queue in bytes (including overflow pad) */

    UWORD *RecvQueue, *RecvQueueEnd;
    UWORD *RecvQueueHead, *RecvQueueTail;
    ULONG RecvQueueSize;        /* actual size of recv queue in words (including overflow pad) */

    struct MidiLink *InLink;    /* link to input MidiCluster */
    struct MidiLink *OutLink;   /* link to output MidiCluster */
    UBYTE *InName;              /* allocated name for input MidiCluster */
    UBYTE *OutName;             /* allocated name for output MidiCluster */

    struct ParserData *ParserData;      /* ParserData for this unit */

    ULONG OutPortFilter;        /* Output port mask */

    UBYTE RunningStatus;
    BYTE RunningCount;

    UBYTE ErrFlags;             /* error flags (CMEF_) */

    UBYTE MidiDevicePort;       /* copy of MidiUnitDef MidiDevicePort */
    struct MidiDeviceData *MidiDeviceData;      /* MIDI device driver for this unit (NULL if internal) */
    void (*ActivateXmit)();     /* activate xmit interrupt for associated port */
    struct MidiUnitDef *MidiUnitDef;      /* MIDI device driver for this unit (NULL if internal) */
};

#define MUF_SerOpen      0x01   /* serial open */
#define MUF_XmitActive   0x02   /* transmit interrupt is active */


/*
    UnitData notes
    --------------
    . MidiUnit array must be contiguous.
    . MidiPrefs are never be changed while units are active.
    . NUnits contains the number of units when UnitTask is up.
      It contains 0 otherwise.  This causes IsLegalUnit() to
      correctly detect when units are not active.
*/

struct UnitData
{       /* needs to be visible to unita.asm */
    struct SignalSemaphore UnitsLock;
    UBYTE NUnits;
    UBYTE pad0;
    struct MidiUnit *MidiUnit;
    ULONG MidiUnitSize;

    struct MidiPrefs *MidiPrefs;
    ULONG MidiPrefsSize;

        /* UnitTask startup/shutdown syncronization */
    struct Task *ReplyTask;     /* used to reply to parent task */
    ULONG ReplyMask;
    struct MidiUnit *OpenUnit;  /* for UnitTask to determine which unit it is for (during startup) */
    WORD ErrCode;               /* set by UnitTask during startup */
} UnitData;


    /* unit task stuff */

/* #define UNIT_Name       "CAMD Unit Task" */
#define UNIT_Pri        30L
#define UNIT_Stack      4000L


#define LockUnits()         ObtainSemaphore (&UnitData.UnitsLock)
#define UnlockUnits()       ReleaseSemaphore (&UnitData.UnitsLock)

#define IsLegalUnit(unum)   ((UBYTE)(unum) < UnitData.NUnits)


/* FUNCTION PROTOTYPES */
    /* external */
extern struct MidiPortData *OpenIntSer(APTR);
extern void CloseIntSer(void);

    /* unita.asm */
extern ULONG far UnitTaskSegment;
extern void __asm UnitTaskLoop (register __a0 struct MidiUnit *);
extern struct MidiPortData * __asm OpenMDPort (register __a0 struct MidiUnit *);
extern void __asm CloseMDPort (register __a0 struct MidiUnit *);

    /* internal */
void __saveds UnitTask (void);

    /* private */
Static int GetMidiPrefs (void);
Static int PrepareUnits (void);
Static BOOL AllocUnitLinks (struct MidiUnit *, struct MidiUnitDef *);
static void FreeUnitLinks (struct MidiUnit *unit);
Static BOOL AllocQueue (struct MidiUnit *, struct MidiUnitDef *);
Static void FreeQueue (struct MidiUnit *);
Static int OpenUnitTask (struct MidiUnit *);
Static void CloseUnitTask (struct MidiUnit *);

Static void PurgeUnit (struct MidiUnit *unit);
Static int SetupUnit(struct MidiUnit *unit);

void __asm OutputFunc(
        register __a1 struct MidiMsg *msg,
        register __a2 struct MidiLink *ml);

int __saveds __asm PartFunc(
register __a0 struct Hook *hook,
register __a1 struct cmLink *msg,
register __a2 struct MidiLink *ml);

/* Public Functions */

/***************************************************************
*
*   RethinkCAMD
*
***************************************************************/

/****** camd.library/RethinkCAMD ******************************************
*
*   NAME
*       RethinkCAMD -- Force CAMD library to reload MIDI preferences
*
*   SYNOPSIS
*       LONG RethinkCAMD(void)
*
*   FUNCTION
*       Forces CAMD library to reload MIDI preferences and reassign the
*	hardware unit MidiLinks.
*
*   RESULTS
*       result - 0 if everything went OK, else returns a CME_ error code
*
****************************************************************************
*
*/

LONG __saveds __asm LIBRethinkCAMD (void)
{   LONG result;

    LockUnits();                /* keep in sync with callers trying to access unit data */
    CloseUnits();
    result = OpenUnits();
    UnlockUnits();

    return result;
}

/* Internal Functions */

/***************************************************************
*
*   PartFunc
*
*   FUNCTION
*       Handle participant changes
*
*   INPUTS
*       
*
*   RESULTS
*       None
*
***************************************************************/

struct Hook PartHook = {
        NULL, NULL,
        (void *)PartFunc,
};

static int ManageUnit(struct MidiUnit *unit)
{
    if ( (unit->InLink && MidiLinkConnected(unit->InLink)) ||
	(unit->OutLink && MidiLinkConnected(unit->OutLink)) )
    {	return SetupUnit(unit);
    }

    PurgeUnit(unit);
    return 0;
}

int __saveds __asm PartFunc(
register __a0 struct Hook *hook,
register __a1 struct cmLink *msg,
register __a2 struct MidiLink *ml)
{
    if (msg->cml_MethodID == CMSG_Link)
    {
	LockUnits();
	ManageUnit((struct MidiUnit *)ml->ml_UserData);
	UnlockUnits();
    }
    return 0;
}

/***************************************************************
*
*   InitUnits
*
*   FUNCTION
*       Init Unit subsystem on library initialization.
*
*   INPUTS
*       None
*
*   RESULTS
*       None
*
***************************************************************/

struct Hook OutputHook = {
        NULL, NULL,
        (void *)OutputFunc,
};

void InitUnits(void)
{
    struct MidiPrefs *MidiPrefs;

        /* init global semaphore */
    InitSemaphore (&UnitData.UnitsLock);

    UnitData.MidiUnit = AllocMem(sizeof(struct MidiUnit),MEMF_PUBLIC|MEMF_CLEAR);
    if (UnitData.MidiUnit == NULL) { CleanUpUnits(); return; }

    UnitData.MidiUnitSize = sizeof(struct MidiUnit);

    MidiPrefs = UnitData.MidiPrefs = AllocMem(sizeof(struct MidiPrefs),MEMF_PUBLIC|MEMF_CLEAR);
    if (UnitData.MidiPrefs == NULL) { CleanUpUnits(); return; }

    UnitData.MidiPrefsSize = sizeof(struct MidiPrefs);

        /* default prefs */
    MidiPrefs->NUnits = 1;
    MidiPrefs->UnitDef[0].Flags = MUDF_Internal;
    MidiPrefs->UnitDef[0].XmitQueueSize = MinXmitQueueSize;
    MidiPrefs->UnitDef[0].RecvQueueSize = MinRecvQueueSize;
    strcpy(MidiPrefs->UnitDef[0].MidiClusterInName,DefaultIn0Name);
    strcpy(MidiPrefs->UnitDef[0].MidiClusterOutName,DefaultOut0Name);

    CamdBase->HardwareNode = CreateMidi(
        MIDI_Name,     "camd_hardware_node",
        MIDI_RecvHook, &OutputHook,
	MIDI_PartHook, &PartHook,
        /* MIDI_SysExSize,   2048, */
        TAG_DONE);
    if (CamdBase->HardwareNode == NULL) { CleanUpUnits(); return; }

    RethinkCAMD();
}

/***************************************************************
*
*   CleanUpUnits
*
*   FUNCTION
*       Free Allocated Memory
*
*   INPUTS
*       None
*
*   RESULTS
*       None
*
***************************************************************/

void CleanUpUnits(void)
{
    if (CamdBase->HardwareNode)
    {
	LockUnits();                /* keep in sync with callers trying to access unit data */
	CloseUnits();
	UnlockUnits();

        DeleteMidi(CamdBase->HardwareNode);
        CamdBase->HardwareNode = NULL;
    }

    if (UnitData.MidiPrefs != NULL)
    {
        FreeMem(UnitData.MidiPrefs,UnitData.MidiPrefsSize);
        UnitData.MidiPrefsSize = 0;
    }

    if (UnitData.MidiUnit != NULL)
    {
        FreeMem(UnitData.MidiUnit,UnitData.MidiUnitSize);
        UnitData.MidiUnitSize = 0;
    }
}

/* unit open/close */

/***************************************************************
*
*   OpenUnits
*
*   FUNCTION
*       Start up Unit task, allocate queues, and install serial
*       interrupts.  This is called by AddMidi() when MICount
*       goes from 0 to 1.
*
*   INPUTS
*       None
*
*   RESULTS
*       Error code.  0 for success.
*
***************************************************************/

int OpenUnits(void)
{
    int err;

    if ( (err = GetMidiPrefs()) || (err = PrepareUnits()) ) CloseUnits();

    return err;
}

Static int GetMidiPrefs(void)
{
    struct Library *IFFParseBase;
    struct MidiPrefs *prefs;
    struct MidiUnit *unit;
    struct IFFHandle *iff;
    struct ContextNode *cn;
    int err = CME_BadPrefs;

    if ( (IFFParseBase = OpenLibrary("iffparse.library",34L)) == NULL)
	return err;

    if (iff = AllocIFF())
    {
	if (iff->iff_Stream = (ULONG) Open ("env:sys/" MidiPrefsName, MODE_OLDFILE))
	{
	    InitIFFasDOS (iff);

	    if (!OpenIFF (iff, IFFF_READ))
	    {
		PropChunk(iff,ID_PREF,ID_PRHD);  /* ignored at this time */
		StopChunk(iff,ID_PREF,ID_MIDI);

		if (!ParseIFF(iff,IFFPARSE_SCAN))
		{
		    cn = CurrentChunk(iff);

		    if (prefs = AllocMem(cn->cn_Size,MEMF_PUBLIC))
		    {
			if (ReadChunkBytes(iff,prefs,cn->cn_Size) == cn->cn_Size)
			{
			    /* successfully read preferences */

			    if (unit = AllocMem(prefs->NUnits * sizeof *unit,MEMF_PUBLIC|MEMF_CLEAR))
			    {
			        if (UnitData.MidiPrefsSize)
				    FreeMem(UnitData.MidiPrefs,UnitData.MidiPrefsSize);

			        UnitData.MidiPrefs = prefs;
		                UnitData.MidiPrefsSize = cn->cn_Size;

		            	if (UnitData.MidiUnitSize)
				    FreeMem(UnitData.MidiUnit,UnitData.MidiUnitSize);

				UnitData.MidiUnit = unit;
		            	UnitData.MidiUnitSize = prefs->NUnits * sizeof *unit;

 			        err = 0;
			    }
			    else FreeMem(prefs,cn->cn_Size);
			}
			else FreeMem(prefs,cn->cn_Size);
		    }
                }
		CloseIFF(iff);
	    }

            Close((BPTR)iff->iff_Stream);
	}
	else err = 0;	/* must not be there (?) */

	FreeIFF(iff);
    }

    CloseLibrary(IFFParseBase);

    return err;
}

Static int PrepareUnits(void)
{
register struct MidiUnit *unit;
register struct MidiUnitDef *def;
int i;
int err = 0;

    for (i=0, def=UnitData.MidiPrefs->UnitDef, unit=UnitData.MidiUnit; i<UnitData.MidiPrefs->NUnits; i++, def++, unit++)
    {
        if (def->Flags & MUDF_Ignore) continue;

        unit->MidiUnitDef = def;
        unit->MidiDevicePort = def->MidiDevicePort;
    	unit->UnitNum = i;

        InitSemaphore(&unit->UnitLock);    /* init single unit semaphore */

        if ( !AllocUnitLinks(unit,def) )
        {
            err = CME_NoMem;
            goto clean;
        }

#if DAVETEST
        if (!(def->Flags & MUDF_Internal))
        {
            if (!(unit->MidiDeviceData = OpenMidiDevice((char *)def->MidiDeviceName)) ||
            unit->MidiDevicePort >= unit->MidiDeviceData->NPorts)
            {
	       err = CME_NoUnit(unit->UnitNum);
               goto clean;
           }
        }
#endif

	ManageUnit(unit);
    }

    UnitData.NUnits = UnitData.MidiPrefs->NUnits;

clean:
    return err;
}


Static int SetupUnit(struct MidiUnit *unit)
{
/* dart */
   register struct MidiUnitDef *def = unit->MidiUnitDef;
   int err = 0;

   if (def->Flags & MUDF_Ignore || unit->UnitTask != NULL) return 0;

   if (!(unit->ParserData = AllocParser()))
   {
      /* have to allocate parser for before doing CMP_In() */
      err = CME_NoMem;
      goto clean;
   }

   if ( !AllocQueue (unit,def) )
   {
      err = CME_NoMem;
      goto clean;
   }

#if !DAVETEST
   if (!(def->Flags & MUDF_Internal))
   {
      if (!(unit->MidiDeviceData = OpenMidiDevice((char *)def->MidiDeviceName)) ||
         unit->MidiDevicePort >= unit->MidiDeviceData->NPorts)
      {
	  err = CME_NoUnit(unit->UnitNum);
          goto clean;
      }
   }
#endif

   err = OpenUnitTask (unit);

clean:
   return err;
}


/***************************************************************
*
*   CloseUnits
*
*   FUNCTION
*       Shut down Unit task, free queus, remove serial interrupts.
*       This is called by CloseMidi() when MICount goes from 1 to 0.
*
*   INPUTS
*       None
*
*   RESULTS
*       None
*
***************************************************************/

void CloseUnits (void)
{
/* dart */
   register struct MidiUnit *unit;
   int i;

   UnitData.NUnits = 0; /* this prevents stupidity in unit message distribution */

   for (i=0, unit=UnitData.MidiUnit; i<UnitData.MidiPrefs->NUnits; i++, unit++)
   {
      PurgeUnit(unit);

#if DAVETEST
      if (unit->MidiDeviceData)
      {
         CloseMidiDevice (unit->MidiDeviceData);
         unit->MidiDeviceData = NULL;
      }
#endif

      FreeUnitLinks (unit);

      unit->RunningStatus = 0;
      unit->Flags = unit->ErrFlags = 0; /* clear all flags */
   }
}

Static void PurgeUnit (struct MidiUnit *unit)
{
    CloseUnitTask (unit);

#if !DAVETEST
    if (unit->MidiDeviceData)
    {
        CloseMidiDevice (unit->MidiDeviceData);
        unit->MidiDeviceData = NULL;
    }
#endif

    FreeQueue (unit);

    if (unit->ParserData)
    {
         FreeParser (unit->ParserData);
         unit->ParserData = NULL;
    }

    return;
}

Static BOOL AllocUnitLinks (struct MidiUnit *unit, struct MidiUnitDef *def)
{
int len;

    len = strlen(def->MidiClusterInName);
    if ( !(unit->InName = AllocMem(len+1,MEMF_PUBLIC)) ) goto clean;
    strcpy(unit->InName,def->MidiClusterInName);

    len = strlen(def->MidiClusterOutName);
    if ( !(unit->OutName = AllocMem(len+1,MEMF_PUBLIC)) ) goto clean;
    strcpy(unit->OutName,def->MidiClusterOutName);

    if ( !(unit->InLink = AddMidiLink (CamdBase->HardwareNode, MLTYPE_Sender,
        MLINK_Location,   unit->InName,
        MLINK_Comment,    def->MidiDeviceComment,
        MLINK_UserData,   unit,
        TAG_DONE ))) goto clean;

    if ( !(unit->OutLink = AddMidiLink (CamdBase->HardwareNode, MLTYPE_Receiver,
        MLINK_Location,   unit->OutName,
        MLINK_Comment,    def->MidiDeviceComment,
        MLINK_UserData,   unit,
        TAG_DONE ))) goto clean;

    return TRUE;

clean:
    return FALSE;
}

static void FreeUnitLinks (struct MidiUnit *unit)
{
    if (unit->InLink)
    {
        RemoveMidiLink(unit->InLink);
        unit->InLink = NULL;
    }

    if (unit->OutLink)
    {
        RemoveMidiLink(unit->OutLink);
        unit->OutLink = NULL;
    }

    if (unit->InName)
    {
        FreeMem(unit->InName,strlen(unit->InName)+1);
        unit->InName = NULL;
    }

    if (unit->OutName)
    {
        FreeMem(unit->OutName,strlen(unit->OutName)+1);
        unit->OutName = NULL;
    }
}

Static BOOL AllocQueue (struct MidiUnit *unit, struct MidiUnitDef *def)
{
/* dart */
   unit->XmitQueueSize = MAX(def->XmitQueueSize,MinXmitQueueSize);

   if (!(unit->XmitQueue = AllocMem (unit->XmitQueueSize, MEMF_PUBLIC))) goto clean;   /* bytes */
   unit->XmitQueueEnd = unit->XmitQueue + unit->XmitQueueSize;
   unit->XmitQueueHead = unit->XmitQueueTail = unit->XmitQueue;

   unit->RecvQueueSize = MAX(def->RecvQueueSize,MinRecvQueueSize);

   if (!(unit->RecvQueue = AllocMem (unit->RecvQueueSize * 2, MEMF_PUBLIC))) goto clean;   /* words */
   unit->RecvQueueEnd = unit->RecvQueue + unit->RecvQueueSize;  /* words, not bytes */
   unit->RecvQueueHead = unit->RecvQueueTail = unit->RecvQueue;

   return TRUE;

clean:
    return FALSE;
}

Static void FreeQueue (struct MidiUnit *unit)
{
/* dart */
   if (unit->XmitQueue)
      {
      FreeMem (unit->XmitQueue, unit->XmitQueueSize);
      unit->XmitQueue = NULL;
      }
   if (unit->RecvQueue)
      {
      FreeMem (unit->RecvQueue, unit->RecvQueueSize * 2);     /* words */
      unit->RecvQueue = NULL;
      }
}



/***************************************************************
*
*   UnitTask
*
***************************************************************/

Static BOOL alloctaskname (struct MidiUnit *unit);
Static void freetaskname (struct MidiUnit *unit);


/***************************************************************
*
*   OpenUnitTask
*
*   FUNCTION
*       Opens UnitTask for a MidiUnit.
*
*   INPUTS
*       unit - MidiUnit pointer
*
*   RESULTS
*       0 on success, non-zero error code on failure.
*
***************************************************************/

Static int OpenUnitTask (struct MidiUnit *unit)
{
   int replybit;
   int err = 0;

   if ((replybit = AllocSignal(-1L)) == -1)
   {
      err = CME_NoSignals;
      goto clean;
   }

   UnitData.ReplyTask = FindTask(NULL);
   UnitData.ReplyMask = 1L << replybit;

   if (!alloctaskname(unit))
   {
      err = CME_NoMem;
      goto clean;
   }

   UnitData.OpenUnit = unit;        /* put unit pointer about to open where
                                       UnitTask() can find it during startup */

/* !!! use CreateNewProc() if camd becomes 2.0 only! */
/*      --- could do away w/ segment! */

   if (!CreateProc (unit->UnitTaskName, UNIT_Pri, MKBADDR(&UnitTaskSegment), UNIT_Stack))
   {
      err = CME_NoMem;
      goto clean;
   }

   Wait (UnitData.ReplyMask);

   err = UnitData.ErrCode;          /* ErrCode is set during UnitTask startup */

clean:
    if (replybit != -1) FreeSignal ((long)replybit);
    return err;
}


/***************************************************************
*
*   CloseUnitTask
*
*   FUNCTION
*       Closes UnitTask for a MidiUnit.  It's ok to call
*       this function if the UnitTask never got started.
*
*   INPUTS
*       unit - MidiUnit pointer
*
*   RESULTS
*       None
*
***************************************************************/

Static void CloseUnitTask (struct MidiUnit *unit)
{
   register int replybit;

   if (unit->UnitTask)
   {
      UnitData.ReplyTask = FindTask(NULL);

      if ((replybit = AllocSignal(-1L)) != -1)
         UnitData.ReplyMask = 1L << replybit;
      else
         UnitData.ReplyMask = SIGF_SINGLE;

      if (replybit == -1) SetSignal (0L, UnitData.ReplyMask);

      Signal (unit->UnitTask, 1L << unit->UnitQuitSigBit);

      Wait (UnitData.ReplyMask);

      if (replybit != -1) FreeSignal ((long)replybit);

   }
   freetaskname (unit);
}

Static BOOL alloctaskname (struct MidiUnit *unit)
{
/* dart */
   char b[64];

   /* assumes we get amiga.lib's RawDoFmt() implementation */
   sprintf (b, "CAMD Unit %ld Parser Task", (long)unit->UnitNum);

   if (!(unit->UnitTaskName = AllocMem (strlen(b)+1, MEMF_PUBLIC))) return FALSE;

   strcpy (unit->UnitTaskName,b);

   return TRUE;
}

Static void freetaskname (struct MidiUnit *unit)
{
/* dart */

   if (unit->UnitTaskName)
      {
      FreeMem (unit->UnitTaskName, strlen(unit->UnitTaskName)+1);
      unit->UnitTaskName = NULL;
      }
}


/***************************************************************
*
*   UnitTask
*
*   FUNCTION
*       Entry point for each MidiUnit's UnitTask.  This code
*       is shared by each UnitTask.
*
*   INPUTS (valid until reply Signal())
*       UnitData.OpenUnit - pointer to MidiUnit being for this *                           task.
*       UnitData.ReplyTask - parent task to signal on reply
*       UnitData.ReplyMask - signal mask for reply
*
*   RESULTS (valid after reply Signal())
*       UnitData.ErrCode - 0 for success, non-zero for error
*                          during startup
*
***************************************************************/

Static BOOL allocsigs (struct MidiUnit *);
Static BOOL openser (struct MidiUnit *);
Static void closeser (struct MidiUnit *);

void __saveds UnitTask (void)
{
   struct MidiUnit *unit = UnitData.OpenUnit;

/* don't worry about freeing these on exit, they go away with task */
   if (!allocsigs (unit))
   {
      UnitData.ErrCode = CME_NoSignals;
      goto clean;
   }

   unit->UnitTask = FindTask(NULL);    /* indicates that task is alive to CloseUnitTask() */
   UnitData.ErrCode = 0;                /* ErrCode == 0 indicates to OpenUnitTask() that task is ok */

   if (!openser(unit))
   {
      UnitData.ErrCode = CME_NoUnit (unit->UnitNum);
      goto clean;
   }


   Signal (UnitData.ReplyTask, UnitData.ReplyMask);

   /* careful... UnitData.OpenUnit is no longer valid for this task after Signal() */

   UnitTaskLoop (unit);

clean:
   closeser (unit);
   Forbid();
   unit->UnitTask = NULL;
   Signal (UnitData.ReplyTask, UnitData.ReplyMask);
}

Static BOOL allocsigs (struct MidiUnit *unit)
{
/* dart */
   int sigbit;

   if ((sigbit = AllocSignal(-1L)) == -1) return FALSE;
   unit->UnitRecvSigMask = 1L << sigbit;
   return (BOOL)((unit->UnitQuitSigBit = AllocSignal(-1L)) != -1);
}

Static BOOL openser (struct MidiUnit *unit)
{
/* dart */
   struct MidiPortData *mpd;

   unit->ErrFlags = 0;  /* clear error flags */

                                /* PrepareUnits() checks for legal port #, so no need to worry about it here */
   if (!(mpd = unit->MidiDeviceData ? OpenMDPort (unit) : OpenIntSer (unit))) return FALSE;

   unit->ActivateXmit = mpd->ActivateXmit;
   unit->Flags |= MUF_SerOpen;

   return TRUE;
}

Static void closeser (struct MidiUnit *unit)
{
/* dart */
   if (unit->Flags & MUF_SerOpen)
      {

      while (unit->Flags & MUF_XmitActive) 
      {
	Delay (5L);        /* wait for queue to go inactive */
      }

      if (unit->MidiDeviceData) CloseMDPort (unit);
      else CloseIntSer();

      unit->Flags &= ~MUF_SerOpen;
     }
}
/* eof */
