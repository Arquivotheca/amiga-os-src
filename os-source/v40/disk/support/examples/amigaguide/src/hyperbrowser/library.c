/* library.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

STRPTR ShowLibFlags (struct GlobalData * gd, struct Library * lib)
{
    UBYTE flags = lib->lib_Flags;

    memset (gd->gd_FBuffer, 0, sizeof (gd->gd_FBuffer));
    if (flags & LIBF_SUMMING)
	strcat (gd->gd_FBuffer, "summing ");
    if (flags & LIBF_CHANGED)
	strcat (gd->gd_FBuffer, "changed ");
    if (flags & LIBF_SUMUSED)
	strcat (gd->gd_FBuffer, "sumused ");
    if (flags & LIBF_DELEXP)
	strcat (gd->gd_FBuffer, "delexp ");
    return gd->gd_FBuffer;
}

/*****************************************************************************/

void showlibrarylist (struct GlobalData * gd)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct List *list = &SysBase->LibList;
    struct Library *lib;
    struct Node *node;

    /* Build the title */
    strcpy (gd->gd_Node, "@{b}Library Name                     Address   Vers  Rev  Opens@{ub}\n");

    /* Build the library list */
    Forbid ();
    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	lib = (struct Library *) node;
	bprintf (gd, "@{\"%-30s\" link HYPERNOZY.LIBRARY.(%08lx)} @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)} %4ld %5ld %5ld\n",
		 lib->lib_Node.ln_Name, lib,
		 lib, lib, (void *) lib->lib_Version, (void *) lib->lib_Revision, (void *) lib->lib_OpenCnt);
    }
    Permit ();
}

/*****************************************************************************/

void showlibrarybase (struct GlobalData * gd, struct Library * lib)
{
    struct Node *node = (struct Node *) lib;
    UBYTE id[128];
    ULONG i, j;

    /* Prepare the id string */
    if (lib->lib_IdString)
    {
	strcpy (id, lib->lib_IdString);
	j = strlen (id);
	for (i = j - 1; i > 0; i--)
	{
	    if ((id[i] == 0) || (id[i] == '\n') || (id[i] == '\r'))
		id[i] = 0;
	    else
		break;
	}
    }
    else
	id[0] = 0;

    /* Build the library base information */
    bprintf (gd, "        Address: @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", lib, lib);
    bprintf (gd, "           Type: %ld\n", (void *) node->ln_Type);
    bprintf (gd, "       Priority: %ld\n", (void *) node->ln_Pri);
    bprintf (gd, "           Name: %s\n", node->ln_Name);
    bprintf (gd, "          Flags: %s\n", (void *) ShowLibFlags (gd, lib));
    bprintf (gd, "          LVO's: %ld\n", (void *) UDivMod32 ((ULONG) lib->lib_NegSize, 6L));
    bprintf (gd, "           Size: %ld\n", (void *) ((ULONG) (lib->lib_NegSize + lib->lib_PosSize)));
    bprintf (gd, "        Version: %ld.%ld\n", (void *) lib->lib_Version, (void *) lib->lib_Revision);
    bprintf (gd, "      ID String: %s\n", id);
    bprintf (gd, "       Checksum: %ld\n", (void *) ((ULONG) lib->lib_Sum));
    bprintf (gd, "     Open Count: %ld\n", (void *) lib->lib_OpenCnt);
}

/*****************************************************************************/

void showlibrary (struct GlobalData * gd, ULONG address)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct List *list = &SysBase->LibList;
    struct Node *node;

    /* Just in case we don't find the library */
    strcpy (gd->gd_Node, "@{b}library gone@{ub}\n");

    /* Build the library list */
    Forbid ();
    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	if (node == (struct Node *) address)
	{
	    strcpy (gd->gd_Node, "@{b}Library Base@{ub}\n\n");
	    showlibrarybase (gd, (struct Library *) node);
	    break;
	}
    }
    Permit ();
}
