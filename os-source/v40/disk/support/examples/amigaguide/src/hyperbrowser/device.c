/* device.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

void showdevicelist (struct GlobalData * gd)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct List *list = &SysBase->DeviceList;
    struct Library *lib;
    struct Node *node;

    /* Build the title */
    strcpy (gd->gd_Node, "@{b}Device Name                      Address   Vers  Rev  Opens@{ub}\n");

    /* Build the device list */
    Forbid ();
    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	lib = (struct Library *) node;
	bprintf (gd, "@{\"%-30s\" link HYPERNOZY.DEVICE.(%08lx)} @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)} %4ld %5ld %5ld\n",
		 lib->lib_Node.ln_Name, lib,
		 lib, lib, (void *) lib->lib_Version, (void *) lib->lib_Revision, (void *) lib->lib_OpenCnt);
    }
    Permit ();
}

/*****************************************************************************/

void showdevice (struct GlobalData * gd, ULONG address)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct List *list = &SysBase->DeviceList;
    struct Node *node;

    /* Just in case we don't find the device */
    strcpy (gd->gd_Node, "@{b}device gone@{ub}\n");

    /* Build the device list */
    Forbid ();
    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	if (node == (struct Node *) address)
	{
	    strcpy (gd->gd_Node, "@{b}Device Base@{ub}\n\n");
	    showlibrarybase (gd, (struct Library *) node);
	    break;
	}
    }
    Permit ();
}
