#ifndef EXEC_EXECBASE_H
#define EXEC_EXECBASE_H
/*
**	$Filename: exec/execbase.h $
**	$Release: 1.3 $
**	$Revision: 36.8 $
**	$Date: 89/12/18 16:55:41 $
**
**	definition of the exec.library base structure
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_LISTS_H
#include "exec/lists.h"
#endif !EXEC_LISTS_H

#ifndef EXEC_INTERRUPTS_H
#include "exec/interrupts.h"
#endif !EXEC_INTERRUPTS_H

#ifndef EXEC_LIBRARIES_H
#include "exec/libraries.h"
#endif !EXEC_LIBRARIES_H

#ifndef EXEC_TASKS_H
#include "exec/tasks.h"
#endif !EXEC_TASKS_H


/* Definition of the Exec library base structure (pointed to by location 4).
** Most fields are not to be viewed or modified by user programs.  Use
** extreme caution.
*/
struct ExecBase {
	struct Library LibNode; /* Standard library node */

/******** Static System Variables ********/

	UWORD	SoftVer;	/* kickstart release number (obs.) */
	WORD	LowMemChkSum;	/* checksum of 68000 trap vectors */
	ULONG	ChkBase;	/* system base pointer complement */
	APTR	ColdCapture;	/* coldstart soft capture vector */
	APTR	CoolCapture;	/* coolstart soft capture vector */
	APTR	WarmCapture;	/* warmstart soft capture vector */
	APTR	SysStkUpper;	/* system stack base   (upper bound) */
	APTR	SysStkLower;	/* top of system stack (lower bound) */
	ULONG	MaxLocMem;	/* top of chip memory */
	APTR	DebugEntry;	/* global debugger entry point */
	APTR	DebugData;	/* global debugger data segment */
	APTR	AlertData;	/* alert data segment */
	APTR	MaxExtMem;	/* top of extended mem, or null if none */

	UWORD	ChkSum; 	/* for all of the above (minus 2) */

/****** Interrupt Related ***************************************/

	struct	IntVector IntVects[16];

/****** Dynamic System Variables *************************************/

	struct	Task *ThisTask; /* pointer to current task (readable) */

	ULONG	IdleCount;	/* idle counter */
	ULONG	DispCount;	/* dispatch counter */
	UWORD	Quantum;	/* time slice quantum */
	UWORD	Elapsed;	/* current quantum ticks */
	UWORD	SysFlags;	/* misc internal system flags */
	BYTE	IDNestCnt;	/* interrupt disable nesting count */
	BYTE	TDNestCnt;	/* task disable nesting count */

	UWORD	AttnFlags;	/* special attention flags (readable) */

	UWORD	AttnResched;	/* rescheduling attention */
	APTR	ResModules;	/* resident module array pointer */
	APTR	TaskTrapCode;
	APTR	TaskExceptCode;
	APTR	TaskExitCode;
	ULONG	TaskSigAlloc;
	UWORD	TaskTrapAlloc;


/****** System Lists (private!) ********************************/

	struct	List MemList;
	struct	List ResourceList;
	struct	List DeviceList;
	struct	List IntrList;
	struct	List LibList;
	struct	List PortList;
	struct	List TaskReady;
	struct	List TaskWait;

	struct	SoftIntList SoftInts[5];

/****** Other Globals *******************************************/

	LONG	LastAlert[4];

	/* these next two variables are provided to allow
	** system developers to have a rough idea of the
	** period of two externally controlled signals --
	** the time between vertical blank interrupts and the
	** external line rate (which is counted by CIA A's
	** "time of day" clock).  In general these values
	** will be 50 or 60, and may or may not track each
	** other.  These values replace the obsolete AFB_PAL
	** and AFB_50HZ flags.
	*/
	UBYTE	VBlankFrequency;	/* (readable) */
	UBYTE	PowerSupplyFrequency;	/* (readable) */

	struct	List SemaphoreList;

	/* these next two are to be able to kickstart into user ram.
	** KickMemPtr holds a singly linked list of MemLists which
	** will be removed from the memory list via AllocAbs.  If
	** all the AllocAbs's succeeded, then the KickTagPtr will
	** be added to the rom tag list.
	*/
	APTR	KickMemPtr;	/* ptr to queue of mem lists */
	APTR	KickTagPtr;	/* ptr to rom tag queue */
	APTR	KickCheckSum;	/* checksum for mem and tags */

/****** V36 Exec additions start here **************************************/

	UWORD	ex_Pad0;
	ULONG	ex_Reserved0;
	APTR	ex_RamLibPrivate;
	/* The next ULONG contains the system "E" clock frequency,
	** expressed in Hertz.	The E clock is used as a timebase for
	** the Amiga's 8520 I/O chips. (E is connected to "02").
	** Typical values are 715909 for NTSC, or 709379 for PAL.
	*/
	ULONG	ex_EClockFrequency;	/* (readable) */
	ULONG	ex_CacheControl;	/* Private to CacheControl calls */
	ULONG	ex_TaskID;		/* Next available task ID */

	ULONG	ex_PuddleSize;
	ULONG	ex_PoolThreshold;
	struct	MinList ex_PublicPool;

	APTR	ex_MMUframe;		/* ptr to MMU configuration */
	struct	List ex_VirtMemList;	/* Virtual free memory list */
};


/****** Bit defines for AttnFlags (see above) ******************************/

/*  Processors and Co-processors: */
#define AFB_68010	0	/* also set for 68020 */
#define AFB_68020	1	/* also set for 68030 */
#define AFB_68030	2	/* also set for 68040 */
#define AFB_68040	3
#define AFB_68881	4	/* also set for 68882 */
#define AFB_68882	5

#define AFF_68010	(1L<<0)
#define AFF_68020	(1L<<1)
#define AFF_68030	(1L<<2)
#define AFF_68040	(1L<<3)
#define AFF_68881	(1L<<4)
#define AFF_68882	(1L<<5)

/* #define AFB_RESERVED8   8 */
/* #define AFB_RESERVED9   9 */


/****** Selected flag definitions for Cache manipulation calls **********/

#define CACRF_EnableI	    (1L<<0)  /* Enable instruction cache */
#define CACRF_FreezeI	    (1L<<1)  /* Freeze instruction cache */
#define CACRF_ClearI	    (1L<<3)  /* Clear instruction cache  */
#define CACRF_IBE	    (1L<<4)  /* Instruction burst enable */
#define CACRF_EnableD	    (1L<<8)  /* 68030 Enable data cache  */
#define CACRF_FreezeD	    (1L<<9)  /* 68030 Freeze data cache  */
#define CACRF_ClearD	    (1L<<11) /* 68030 Clear data cache   */
#define CACRF_DBE	    (1L<<12) /* 68030 Instruction burst enable */
#define CACRF_WriteAllocate (1L<<13) /* 68030 Write-Allocate mode
					(must always be set!)    */

#endif	/* EXEC_EXECBASE_H */
