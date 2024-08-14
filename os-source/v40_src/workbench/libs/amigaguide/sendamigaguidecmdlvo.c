/* SendAmigaGuideCmdLVO.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/****** amigaguide.library/SendAmigaGuideCmdA *******************************
*
*   NAME
*	SendAmigaGuideCmdA - Send a command string to AmigaGuide (V34)
*
*   SYNOPSIS
*	success = SendAmigaGuideCmdA (handle, cmd, attrs );
*	d0				a0    d0   d1
*
*	BOOL SendAmigaGuideCmdA (AMIGAGUIDECONTEXT, STRPTR, struct TagItem *);
*
*	success = SendAmigaGuideCmd (handle, cmd, tag1, ...);
*
*	BOOL SendAmigaGuideCmd (AMIGAGUIDECONTEXT, STRPTR, Tag);
*
*   FUNCTION
*	This function sends a command string to an AmigaGuide system.  The
*	command can consist of any valid AmigaGuide action command.
*
*	The following are the currently valid action commands:
*
*	ALINK <name> - Load the named node into a new window.
*
*	LINK <name> - Load the named node.
*
*	RX <macro> - Execute an ARexx macro.
*
*	RXS <cmd> - Execute an ARexx string file.  To display a picture,
*	    use 'ADDRESS COMMAND DISPLAY <picture name>', to
*	    display a text file 'ADDRESS COMMAND MORE <doc>'.
*
*	CLOSE - Close the window (should only be used on windows
*	    that were started with ALINK).
*
*	QUIT - Shutdown the current database.
*
*   INPUTS
*	handle - Handle to an AmigaGuide system.
*
*	cmd - Command string.
*
*	attrs - Future expansion, must be set to NULL for now.
*
*   TAGS
*	AGA_Context (ULONG) - Data is used as an index into nag_Context
*	    array.  This is used to build and send a LINK command.
*
*   EXAMPLE
*
*	\* bring up help on a particular subject *\
*	SendAmigaGuideCmd(handle, "LINK MAIN", NULL);
*
*   RETURNS
*	Returns TRUE if the message was sent, otherwise returns FALSE.
*
*   BUGS
*	ALINK does not open a new window when using V39.
*
*   SEE ALSO
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM SendAmigaGuideCmdALVO (REG (a6) struct AmigaGuideLib * ag, REG (a0) struct Client * cl, REG (d0) STRPTR cmdLine, REG (d1) struct TagItem * attrs)
{
    struct AmigaGuideMsg *agm;
    LONG retval = FALSE;
    struct TagItem *tag;
    STRPTR cmd = NULL;
    LONG msize;

    DB (kprintf ("SendAmigaGuideCmdALVO : 0x%lx 0x%lx [%s] 0x%lx\n", cl, cl->cl_ChildPort, cmdLine, attrs));

    if (cl && cl->cl_ChildPort)
    {
	if (cl->cl_Context && (tag = FindTagItem (AGA_Context, attrs)))
	{
	    LONG max = 0L;

	    while (cl->cl_Context[max])
		max++;

	    if (tag->ti_Data < max)
	    {
		cl->cl_ContextID = tag->ti_Data;
		sprintf (cl->cl_WorkText, "LINK %s", cl->cl_Context[cl->cl_ContextID]);
		DB (kprintf ("cmd=[%s]\n", cl->cl_WorkText));
		cmd = cl->cl_WorkText;
	    }
	    else
	    {
		DB (kprintf ("%ld >= %ld\n", tag->ti_Data, max));
	    }
	}
	else
	{
	    DB (kprintf ("no context\n"));
	    cmd = cmdLine;
	}

	/* Make sure there is a client and an async process */
	if (cmd)
	{
	    /* Compute the memory block size */
	    msize = AGMSIZE + strlen (cmd) + 1L;

	    /* Allocate the message */
	    if (agm = (struct AmigaGuideMsg *) AllocPVec (ag, ag->ag_MemoryPool, msize))
	    {
		/* Build the message that we're going to send */
		agm->agm_Msg.mn_ReplyPort = cl->cl_AsyncPort;
		agm->agm_Type             = ToolCmdID;
		agm->agm_DSize            = msize;
		agm->agm_Data             = MEMORY_FOLLOWING (agm);
		strcpy (agm->agm_Data, cmd);

		/* send the message */
		DB (kprintf ("putmsg (0x%lx, 0x%lx)\n", cl->cl_ChildPort, agm));
		PutMsg (cl->cl_ChildPort, (struct Message *) agm);

		/* say we where successful */
		retval = TRUE;
	    }
	}
    }
    DB (kprintf ("retval=%ld\n", retval));
    return (retval);
}

LONG SendAmigaGuideCmd (struct AmigaGuideLib *ag, struct Client *cl, STRPTR cmd, Tag tag1, ...)
{
    return (SendAmigaGuideCmdALVO (ag, cl, cmd, (struct TagItem *)&tag1));
}
