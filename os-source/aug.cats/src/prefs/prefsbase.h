#ifndef LIBRARIES_PREFSBASE_H
#define LIBRARIES_PREFSBASE_H
/* prefsbase.h
 *
 * $Id
 *
 * $Log
 *
 */
#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */
	
#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */
	
#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */
	
#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif /* EXEC_SEMAPHORES_H */

#include <iff/ilbm.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/iffparse.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/icon_protos.h>
#include <pragmas/exec.h>
#include <pragmas/dos.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/utility.h>
#include <pragmas/iffparse.h>
#include <pragmas/icon.h>
#include <string.h>
#include <stdio.h>

#ifndef PREFS_INTERNAL
#define	PREFS_INTERNAL TRUE
#endif

#include "prefs_lib.h"
#include "prefs_pragmas.h"

/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))

/* global variables required for library */
struct LibBase
{
    struct SignalSemaphore lb_Lock;	/* Exclusive access */
    BPTR lb_SysPrefs;			/* Lock on ENV:sys */
    STRPTR lb_PrefName[PREFS_KINDS];	/* Pointer to default names */
    UBYTE lb_WorkText[512];		/* Temporary work area */
};

/* Shared libraries that are needed */
extern struct Library *SysBase;
extern struct Library *PrefsBase;
extern struct Library *DOSBase;
extern struct Library *UtilityBase;
extern struct Library *IFFParseBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

struct ExtLibrary
{
    struct Library el_Lib;
    LONG el_SegList;
    struct LibBase *el_Base;
};

SHORT CompairLocks(BPTR Lock1, BPTR Lock2);

#endif /* LIBRARIES_PREFSBASE_H */
