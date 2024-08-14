/* lvonodehost.c
 *
 */

#include "amigaguidebase.h"
#include "hosthandle.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DH(x)	;

/*****************************************************************************/

ULONG ASM LVOAddAGHostA (REG (a6) struct AGLib * ag, REG (a0) struct Hook * h, REG (d0) STRPTR name, REG (a1) struct TagItem * attrs)
{
    struct DatabaseData *dbd;
    struct HostHandle *hh;
    Object *member;
    Object *ostate;

    /* Lock the global data */
    ObtainSemaphore (&ag->ag_Lock);

    /* Search through the database list for a matching database */
    ostate = (Object *) ag->ag_DatabaseList.mlh_Head;
    while (member = NextObject (&ostate))
    {
	/* Convert the member to a database object */
	dbd = INST_DATA (ag->ag_DatabaseClass, member);

	/* See if it matches */
	if (Stricmp (dbd->dbd_Name, name) == 0)
	{
	    /* Release the global data */
	    ReleaseSemaphore (&ag->ag_Lock);
	    return NULL;
	}
    }

    /* Allocate the memory required for the node host */
    if (hh = AllocVec (HHSIZE, MEMF_CLEAR))
    {
        /* Initialize the dispatcher */
        hh->hh_Dispatcher = *h;

        /* Create a database object */
        if (hh->hh_DB = newobject (ag, ag->ag_DatabaseClass, NULL,
                                   DBA_Name,		name,
                                   DBA_HostHandle,	hh,
                                   TAG_DONE))
        {
            /* Add the node host to the list */
            AddTail ((struct List *) &ag->ag_HostList, (struct Node *) hh);
        }
	else
	{
            /* Free the host handle */
            FreeVec (hh);
	    hh = NULL;
	}
    }

    /* Release the global data */
    ReleaseSemaphore (&ag->ag_Lock);

    return ((ULONG) hh);
}

/*****************************************************************************/

LONG ASM LVORemoveAGHostA (REG (a6) struct AGLib * ag, REG (a0) VOID * handle, REG (a1) struct TagItem * attrs)
{
    struct HostHandle *hh = (struct HostHandle *)handle;
    struct DatabaseData *dbd;
    LONG retval = -1;

    if (hh)
    {
	/* Lock the global data */
	ObtainSemaphore (&ag->ag_Lock);

	/* Get a pointer to the database object */
	dbd = INST_DATA (ag->ag_DatabaseClass, hh->hh_DB);

	/* Make sure we can close down */
	if ((retval = dbd->dbd_UseCnt) == 1)
	{
	    /* Must close the database! */
	    DisposeObject (hh->hh_DB);

	    /* Remove the host from the Host list */
	    Remove ((struct Node *) hh);

	    /* Free the handle */
	    FreeVec (hh);

	    /* Indicate success */
	    retval = 0L;
	}

	/* Release the global data */
	ReleaseSemaphore (&ag->ag_Lock);
    }

    return retval;
}
