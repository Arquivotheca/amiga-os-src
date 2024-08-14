/* class.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

/* Find the list pointer given a node pointer */
struct MinList *FindHead (struct MinNode * node)
{
    struct MinList *list;

    while (node)
    {
	if (!node->mln_Pred)
	    list = (struct MinList *) node;
	node = node->mln_Pred;
    }

    return (list);
}

/*****************************************************************************/

void showclasslist (struct GlobalData * gd)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct MinList *list;
    struct MinNode *node;
    struct _Object *o;
    struct IClass *cl;
    struct Image *im;
    UBYTE notes[12];
    ULONG min, max;
    ULONG aEntry;
    ULONG cEntry;

    /* Clear notes */
    memset (notes, 0, sizeof (notes));

    /* Get the rom start and stop for this machine */
    min = ((ULONG) (SysBase->LibNode.lib_IdString)) & 0xFFFF0000;
    max = min + 0x80000;

    /* Get a handle on a boopsi object */
    if (im = (struct Image *) NewObjectA (NULL, "frameiclass", NULL))
    {
	/* Convert the image to a boopsi object */
	o = (struct _Object *) (((ULONG) im) - (sizeof (struct MinNode) + sizeof (ULONG)));

	/* Find the pointer to the boopsi class list */
	list = FindHead (&(o->o_Class->cl_Dispatcher.h_MinNode));

	/* Build the title */
	strcpy (gd->gd_Node, "@{b}Name                        Super                      P Objs Subs Address@{ub}\n");

	/* Step through the list */
	for (node = list->mlh_Head; node->mln_Succ; node = node->mln_Succ)
	{
	    cl = (struct IClass *) node;

	    aEntry = (ULONG) cl->cl_Dispatcher.h_Entry;
	    cEntry = (ULONG) cl->cl_Dispatcher.h_SubEntry;

	    sprintf (notes, "%08lx", cEntry);
	    if ((aEntry >= min) && (aEntry <= max))
		strcpy (notes, "ROM");
	    else if ((cEntry >= min) && (cEntry <= max))
		strcpy (notes, "ROM");
	    else
		strcpy (notes, "Disk");

	    bprintf (gd, "@{\"%-25s\" link HYPERNOZY.CLASS.(%08lx)} @{\"%-25s\" link HYPERNOZY.CLASS.(%08lx)} %ld %4ld %4ld @{\"%08lx\" link HYPERNOZY.MEMORY.(%08lx)}\n", cl->cl_ID, cl, ((cl->cl_Super) ? cl->cl_Super->cl_ID : ""), cl->cl_Super, cl->cl_Flags, cl->cl_ObjectCount, cl->cl_SubclassCount, cl, cl);
	}

	/* Delete the object, now that we are done with it */
	DisposeObject (im);
    }
    else
	strcpy (gd->gd_Node, "@{b}couldn't create first object.@{ub}\n");
}
