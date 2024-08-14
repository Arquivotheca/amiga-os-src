/*
 * $Id: infoevent.c,v 39.1 92/05/31 01:52:09 mks Exp $
 *
 * $Log:	infoevent.c,v $
 * Revision 39.1  92/05/31  01:52:09  mks
 * Now is the ASYNC, LVO based INFO...
 * 
 * Revision 38.1  91/06/24  11:36:15  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include "string.h"
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>

#include "info.h"
#include "support.h"

#include <clib/gadtools_protos.h>
#include "proto.h"

/*------------------------------------------------------------------------*/

/*  Function Prototypes: */

void HandleTTGadget(struct Window *infowindow, struct Gadget *gad, UWORD code);
BOOL HandleGadgetUp(struct Window *infowindow, struct Gadget *gad, UWORD code);

/*------------------------------------------------------------------------*/

/*/ HandleTTGadget()
 *
 * Function to handle the GADGETUP message for the save.
 *
 * Created:  17-Jul-89, Peter Cherna
 * Modified: 17-Apr-90, Peter Cherna
 *
 * Returns:  none
 *
 * Bugs:
 *
 */

void HandleTTGadget(infowindow, gad, code)

    struct Window *infowindow;
    struct Gadget *gad;
    UWORD code;

    {
    struct Gadget *stringgad;
    struct InfoTag *itag;
    struct Node *currentnode;
    WORD pos;
    BOOL activatestring;
    BOOL refreshlist;
    WORD count;

    activatestring = FALSE;
    pos = -1;
    itag = (struct InfoTag *) infowindow->UserData;

    stringgad = itag->it_TTString;

    /*  This is the node whose string we will manipulate: */
    currentnode = itag->it_CurrentTTNode;

    /*  If there is stuff in the string gadget, we must transfer it: */
    if (currentnode)
	{
	/*  Transfer string to the current node: */
	pos = RemoveGList(infowindow, stringgad, 1);
	strcpy(currentnode->ln_Name, StrInfo(stringgad)->Buffer);
	/*  We should refresh the list */
	refreshlist = TRUE;
	/*  If currentnode isn't on the list, add it
	    (non-zero ln_Pri means not on list) */
	if (currentnode->ln_Pri)
	    {
	    /*  Check for empty string */
	    if (currentnode->ln_Name[0])
		{
		currentnode->ln_Pri = 0;
		Insert(&itag->it_ToolTypeList, currentnode, itag->it_AfterNode);
		itag->it_AfterNode = currentnode;
		refreshlist = TRUE;
		}
	    else
		{
		/*  Free the currentnode since the string was empty (don't
		    bother refreshing the list) */
		FREEVEC(currentnode);
		currentnode = NULL;
		refreshlist = FALSE;
		}
	    }
	}

    switch (gad->GadgetID & GADTOOLMASK)
	{
	case G_TTLIST:
	    /*  User clicked on one from the list.  Set currentnode
		to be that one, and cause the string gadget to be
		activated. */
	    currentnode = HeadNode(&itag->it_ToolTypeList);
	    for (count = 0; count < code; count++)
		{
		currentnode = currentnode->ln_Succ;
		}
	    itag->it_AfterNode = currentnode;
	    activatestring = TRUE;
	    break;

	case G_TTSTRING:
	    /*  No further work is required, except to make the
		current string go away */
	    currentnode = NULL;
	    break;

	case G_TTNEW:
	    /*  New ToolType:  Create a new node (not on the list) to
		hold the contents.  Also, cause the string to be
		activated.  Does nothing if Alloc failed */
	    if (currentnode = AllocNode(""))
		{
		currentnode->ln_Pri = 1;
		activatestring = TRUE;
		}
	    break;

	case G_TTDEL:
	    /*  Delete ToolType:  currentnode will always be
		on the list if it still exists by this point.  So,
		remove the currentnode from the list and delete it, and
		cause the list to be refreshed.  If AfterNode was
		pointing at the about-to-be-disposed node, then
		we have to account for that too */
	    if (currentnode)
		{
		if (itag->it_AfterNode == currentnode)
		    {
		    /*  Set AfterNode to be the predecessor if any of
			this node: */
		    if (currentnode == HeadNode(&itag->it_ToolTypeList))
			{
			itag->it_AfterNode = NULL;
			}
		    else
			{
			itag->it_AfterNode = currentnode->ln_Pred;
			}
		    }
		REMOVE(currentnode);
		FREEVEC(currentnode);
		currentnode = NULL;
		refreshlist = TRUE;
		}
	    break;
	}

    itag->it_CurrentTTNode = currentnode;
    strcpy(StrInfo(stringgad)->Buffer,
	(currentnode) ? currentnode->ln_Name : "");
    if (pos != -1)
	AddGList(infowindow, stringgad, pos, 1, NULL);

    if (currentnode)
	{
	EnableGadget(infowindow, itag->it_TTString);
	EnableGadget(infowindow, itag->it_TTDel);
	}
    else
	{
	DisableGadget(infowindow, itag->it_TTString);
	DisableGadget(infowindow, itag->it_TTDel);
	}

    RefreshGList(stringgad, infowindow, NULL, 1);

    if (refreshlist)
	{
	GT_SetGadgetAttrs(itag->it_TTList, infowindow, NULL,
	    GTLV_Labels, &itag->it_ToolTypeList,
	    TAG_DONE);
	}
    if (activatestring)
	{
	ActivateGadget(stringgad, infowindow, NULL);
	}
    }


/*------------------------------------------------------------------------*/

/*/ HandleGadgetUp()
 *
 * Function to handle the GADGETUP message for the specified gadget.
 *
 * Created:  17-Jul-89, Peter Cherna
 * Modified: 09-Nov-89, Peter Cherna
 *
 * Returns:  (BOOL) TRUE if Info window should close.
 *
 * Bugs:
 *
 */

BOOL HandleGadgetUp(infowindow, gad, code)

    struct Window *infowindow;
    struct Gadget *gad;
    UWORD code;

    {
    struct InfoTag *itag = (struct InfoTag *)infowindow->UserData;
    BOOL terminate = FALSE;

    switch (gad->GadgetID & GADTOOLMASK)
	{
	case G_TTLIST:
	case G_TTSTRING:
	case G_TTNEW:
	case G_TTDEL:
	    DP(("HGU:  Tooltypes GADGETUP\n"));
	    HandleTTGadget(infowindow, gad, code);
	    break;

	case G_SAVE:
            {
            struct Node *currentnode;
            struct InfoTag *itag;

                itag = (struct InfoTag *) infowindow->UserData;

                /*  Remove the gadgets so that buffers become stable: */
                RemoveGList(infowindow, itag->it_FirstGadget, -1);

                /*  The last change to the ToolTypes may not have been saved into
                    our ToolTypes list if the user didn't hit <ENTER> in the
                    string gadget after making the last change, so we copy it
                    for posterity: */

                currentnode = itag->it_CurrentTTNode;

                if (currentnode)
                {
                    strcpy(currentnode->ln_Name, StrInfo(itag->it_TTString)->Buffer);
                    /*  If currentnode isn't on the list, add it at the end
                        (non-zero ln_Pri means not on list) */
                    if (currentnode->ln_Pri)
                    {
                        /*  Check for empty string */
                        if (currentnode->ln_Name[0])
                        {
                            currentnode->ln_Pri = 0;
                            /*  !!! Later we'll establish a better way of picking
                                the place to add the node */
                            ADDTAIL(&itag->it_ToolTypeList, currentnode);
                        }
                        else
                        {
                            /*  Free the currentnode since the string was empty */
                            FREEVEC(currentnode);
                        }
                    }
                }
	    }
            /*  Attempt to save Info: */
            InfoSave((struct InfoTag *) infowindow->UserData);

	    /* Drop into Quit */

	case G_QUIT:
	    terminate = TRUE;
	    break;

	case G_SCRIPT:
	case G_ARCHIVED:
	case G_READABLE:
	case G_WRITABLE:
	case G_EXECUTABLE:
	case G_DELETABLE:
	    itag->it_Protection ^= (ULONG)gad->UserData;
	    break;

#ifdef	DEBUGGING
	default:
	    DP(("HGU:  Default\n"));
	    break;
#endif
	}
    return(terminate);
    }
