#ifndef INTERNAL_CROSSDOS_H
#define INTERNAL_CROSSDOS_H
/*
**	$Id: crossdos.h,v 39.1 92/10/01 14:41:28 vertex Exp $
**
**	Private structures for CrossDOS commodity <--> handler interfacing
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif

#ifndef EXEC_NODES_H
#include "exec/nodes.h"
#endif

#ifndef EXEC_LISTS_H"
#include "exec/lists.h"
#endif

#ifndef EXEC_TASKS_H
#include "exec/tasks.h"
#endif

#ifndef EXEC_SEMAPHORES_H
#include "exec/semaphores.h"
#endif


/*****************************************************************************/


/* This is the global semaphore structure from which life begins */
struct CrossDOSLock
{
    struct SignalSemaphore cdl_Lock;
    UWORD                  cdl_StructSize;    /* size of this structure  */
    struct List            cdl_Handlers;      /* list of CrossDOSHandler */
    struct List            cdl_TransTables;   /* list of CrossDOSTrans   */
    UBYTE                  cdl_Reserved[16];  /* for future expansion    */
};


/*****************************************************************************/


/* There is one of these per handler.
 * The cdh_TransTable field points to a translation table. This field may
 * be NULL in which case the default translation should be applied
 */
struct CrossDOSHandler
{
    struct Node           cdh_Link;
    UWORD                 cdh_StructSize; /* size of this structure */
    ULONG                 cdh_Flags;
    struct CrossDOSTrans *cdh_TransTable;

    /* When the handler changes the cdh_Flags field, it can notify
     * a client task that has installed itself in the handler structure
     * using the cdh_NotifyTask field.
     */
    struct Task          *cdh_NotifyTask;
    ULONG                 cdh_NotifySigBit;

    APTR                  cdh_UserData;   /* for handler use */
};

/* Flag bits for CrossDOSHandler.cdh_Flags */
#define CDF_FILTER    (1<<0)
#define CDF_TRANSLATE (1<<1)

#define CDB_FILTER    1
#define CDB_TRANSLATE 2


/*****************************************************************************/


/* Character translation table */
struct CrossDOSTrans
{
    struct Node cdt_Link;
    UWORD       cdt_StructSize;  /* size of this structure */
    UBYTE       cdt_AToM[256];   /* Amiga  -> MS-DOS       */
    UBYTE	cdt_MToA[256];   /* MS-DOS -> Amiga        */
};


/*****************************************************************************/


/* Name of the semaphore */
#define CROSSDOSNAME "� CrossDOS �"


/*****************************************************************************/


#endif /* INTERNAL_CROSSDOS_H */
