#ifndef _CAMDLIB_H
#define _CAMDLIB_H

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
* camdlib.h   - library common include file (library private)
*
************************************************************************/

#ifndef EXEC_LISTS_H
  #include <exec/lists.h>
#endif

#ifndef EXEC_TYPES_H
  #include <exec/types.h>
#endif

#include <midi/camd.h>
#include <midi/camdbase.h>

/* Pragma and prototype files are loaded here instead of individual C
   files so that their usage may be more easily maintained.  Also
   can make better use of precompiled include file */

#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h> /* sprintf() */
#include <clib/camd_protos.h>
#include <clib/cia_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/misc_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/camd_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/misc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

ULONG CallHook (struct Hook *, APTR, ... );

#define callHook CallHook       /* !!! missing from 39.65 clib/alib_protos.h */


/***************************************************************
*
*   Conditional compilation
*
***************************************************************/

#define OS_MIN  LIBRARY_MINIMUM     /* minimum requires OS */
#define OS_MAX  37                  /* max version to do special case code for */


/***************************************************************
*
*   Global Data
*
***************************************************************/

extern struct XCamdBase *CamdBase; /* pointer to extended CamdBase */

/* necessary for #pragmas */
extern struct Library *DOSBase, *UtilityBase;
extern APTR MiscBase;


/***************************************************************
*
*   List Macros
*
***************************************************************/

#define EmptyList(list)     ( (void *)((struct List *)(list))->lh_TailPred == (void *)(list) )

#define ScanList(list,node) for ( (node) = (void *)((struct List *)(list))->lh_Head; \
                                  ((struct Node *)(node))->ln_Succ; \
                                  (node) = (void *)((struct Node *)(node))->ln_Succ )


/***************************************************************
*
*   Internal Data Structures
*
***************************************************************/

#define PDSX_ShortBufSize   514     /* pads structure to next 8 byte boundary */
#define PDSX_LongBufExp     12
#define PDSX_LongBufSize    (1<<PDSX_LongBufExp)

/* Midi Parser state */
struct ParserData
{
            /* current state */
    MidiMsg CurMsg;         /* contains running status and time stamp */
    MidiMsg RTMsg;          /* temp storage for realtime messages */
    ULONG ThreeByte;        /* "ThreeByte" register contents */

            /* sys/ex stuff */
    struct MinList SXList;  /* list of ParserSXNode's */
    UBYTE *SXBuff;          /* current buffer starting address */
    UBYTE *SXBCur;          /* next byte to fill */
    UBYTE *SXBEnd;          /* byte after end of buffer */
    UWORD NSXNodes;         /* number of nodes in SXList */
    UBYTE SXShortBuffer[PDSX_ShortBufSize];
};

struct ParserSXNode
{
    struct MinNode Node;
    UBYTE Buffer[PDSX_LongBufSize];
};


/* internal version of MidiNode */
struct XMidiNode
{
    struct MidiNode Public;     /* public portion (assume that this is long word aligned) */

        /* MidiMsg Queue */
    MidiMsg *MsgQueue;          /* NULL if MsgQueueSize == 0 */
    MidiMsg *MsgQueueEnd;       /* MsgQueue + MsgQueueSize + 1 */
    MidiMsg *MsgQueueHead;      /* next filled MidiMsg in MsgQueue */
    MidiMsg *MsgQueueTail;      /* next available MidiMsg in MsgQueue */

        /* Sys/Ex Queue */
    UBYTE *SysExQueue;          /* NULL if SysExBufSize == 0 */
    UBYTE *SysExQueueEnd;       /* SysExQueue + SysExQueueSize + 1*/
    UBYTE *SysExQueueHead;      /* next filled byte in SysExQueue */
    UBYTE *SysExQueueTail;      /* byte after last completed sys/ex msg in SysExQueue */

#if 0
        /* alarm stuff */
    struct MinNode AlarmNode;   /* node added to AlarmList */
    ULONG AlarmTime;            /* alarm time to wait for */
#endif

        /* misc */
/*    struct Task *SigTask;*/   /* task that owns this MidiNode */
/*    struct ParserData *ParserData; */ /* pointer to ParserData, may be NULL if no parser is attached */
    UBYTE SysFlags;             /* misc receive flags (MIF_ below) */
    UBYTE ErrFlags;             /* accumulated CMEF_ error flags received since last GetMidiErr() call */
};

    /* SysFlags */

#define MIF_InSysEx     0x01    /* In the middle of reading a Sys/Ex message.
                                   Will only be set if there's a Sys/Ex buffer.
                                   Managed by GetMidi(), GetSysEx(), etc. */

#define MIF_MySXBuf     0x02    /* SysExQueue created by CreateMidi() */

#define MIF_AlarmOK     0x04    /* Alarm functions may be called (i.e. there's
                                   an alarm signal bit) */

#define MIF_Ready       0x80    /* MidiNode is operational */


/* internal version of CamdBase */
struct XCamdBase
{
    struct CamdBase Public;                 /* public portion */

    /* camd library base private data */
    struct Library *SysBase;                /* cache these pointers here */
    struct Library *DOSBase;                /* "" */
    struct Library *UtilityBase;            /* "" */
    APTR MiscBase;                          /* "" */
    ULONG SegList;                          /* really a BPTR */

    struct SignalSemaphore MidiListLock;    /* arbitrates access to msg distribution to MidiNodes */
    struct SignalSemaphore CamdSemaphores[CD_NLocks]; /* used to control access to lists */

    struct MinList       DriverList,        /* list of device drivers loaded*/
                         BytePortList,      /* list of open hardware units */
                         MidiList,          /* list of midi interfaces */
                         ClusterList;       /* list of midi clusters */

    struct MidiNode      *HardwareNode;     /* for external input/output  */

    ULONG		 TimeLess;	    /* where timeless links look */

    UWORD		 PrivateLockBits;
    UWORD		 PublicLockBits;

    struct MinList	 NotifyList;	   /* cluster change notification */
};

#define DefaultIn0Name	"in.0"
#define DefaultOut0Name	"out.0"

/***************************************************************
*
*   Locking Macros
*
***************************************************************/

enum {
    MidiListBit=0
};

#define LockMidiList()    { ObtainSemaphore (&CamdBase->MidiListLock); CamdBase->PrivateLockBits |= (1 << MidiListBit); }
#define UnlockMidiList()  { CamdBase->PrivateLockBits &= ~(1 << MidiListBit); ReleaseSemaphore (&CamdBase->MidiListLock); }

#define LockLinks()   { ObtainSemaphore(&CamdBase->CamdSemaphores[CD_Linkages]); CamdBase->PublicLockBits |= (1 << CD_Linkages); }
#define UnlockLinks() { CamdBase->PublicLockBits &= ~(1 << CD_Linkages); ReleaseSemaphore(&CamdBase->CamdSemaphores[CD_Linkages]); }


/***************************************************************
*
*   Internal Function Prototypes
*
***************************************************************/

    /* device.c */
void InitDevices(void);

    /* misc.c */
ULONG EClockFreq (void);
void SetErrorCode (Tag errortag, struct TagItem *taglist, ULONG errorcode);

#if OS_MIN < 36
  struct TagItem *next_tag (struct TagItem **);
  struct TagItem *find_tag (struct TagItem *, Tag);
  ULONG get_tagdata (Tag, ULONG defaultval, struct TagItem *);
#else
  #define next_tag          NextTagItem
  #define find_tag(ti,tag)  FindTagItem (tag, ti)
  #define get_tagdata       GetTagData
#endif

    /* parse.asm */
struct ParserData * __asm AllocParser (void);
void __asm FreeParser (register __a0 struct ParserData *);

    /* unitc.c */
void InitUnits(void);
void CleanUpUnits(void);
int OpenUnits(void);
void CloseUnits(void);

	/* debugging */
void kprintf(char *,...);

#endif
