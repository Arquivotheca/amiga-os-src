/* Handles the list of ModeIDs. */

#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <graphics/displayinfo.h>
#include <graphics/modeid.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include "db_tm.h"

static UBYTE *ModeNames = NULL;
#define STRLEN 11

struct List *BuildModeIDList(struct TMData *TMData, ULONG *entry)
{
    struct List *list;
    struct Node *node;
    UBYTE *name;
    ULONG ModeID = INVALID_ID;
    ULONG count = 0;

    *entry = 0;

    /* First, count the number of ModeIDs.
     * NB - no locking. AAAaaarrgghhhhh!!
     */

    while ((ModeID = NextDisplayInfo(ModeID)) != INVALID_ID)
    {
	count++;
    }

    if (list = AllocMem(sizeof(struct List), MEMF_PUBLIC | MEMF_CLEAR))
    {
	if (node = AllocVec((sizeof(struct Node) * count), MEMF_PUBLIC))
	{
		if (ModeNames = AllocVec((STRLEN * count), MEMF_PUBLIC))
		{
			name = ModeNames;
			/* Initialise the list */
			list->lh_Head = list->lh_TailPred = node;
			ModeID = INVALID_ID;
			count = 0;	/* what is the entry number of the desired ModeID from the CLI? */
			while ((ModeID = NextDisplayInfo(ModeID)) != INVALID_ID)
			{
				sprintf(name, "0x%08lx", ModeID);
				node->ln_Name = name;
				AddTail(list, (struct Node *)node);
				name += STRLEN;
				node++;
				if (ModeID == (ULONG)TMData->UserData)
				{
					*entry = count;
				}
				count++;
			}
		}
		else
		{
			FreeVec(node);
			FreeMem(list, sizeof(struct List));
			node = NULL;
			list = NULL;
		}
	}
	else
	{
		FreeMem(list, sizeof(struct List));
		list = NULL;
	}
	
    }

    return(list);
}

void FreeModeIDList(struct List *list)
{
    if (list)
    {
	if (list->lh_Head)
	{
		FreeVec(list->lh_Head);
	}
	FreeMem(list, sizeof(struct List));
    }
    if (ModeNames)
    {
	FreeVec(ModeNames);
    }
}


/* ToggleGadget() will toggle the status of a CheckMark Gadget, input
 * coming from the keyboard for the _Underscored letter.
 */
void ToggleGadget(struct Window *win, struct Gadget *gad)
{
    ULONG result;
    GT_GetGadgetAttrs(gad, win, NULL,
                      GTCB_Checked, (ULONG)&result,
                      TAG_DONE);

    result = !result;
    GT_SetGadgetAttrs(gad, win, NULL,
                      GTCB_Checked, (ULONG)result,
                      TAG_DONE);
}

