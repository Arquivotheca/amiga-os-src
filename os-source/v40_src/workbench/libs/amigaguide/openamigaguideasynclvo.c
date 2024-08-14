/* openamigaguideasynclvo.c
 *
 */

#include "amigaguidebase.h"

#define	PROC_NAME	"AmigaGuide® (amigaguide.library)"

/****** amigaguide.library/OpenAmigaGuideAsyncA ****************************
*
*   NAME
*	OpenAmigaGuideAsyncA - Open an AmigaGuide database async (V34)
*
*   SYNOPSIS
*	handle = OpenAmigaGuideAsyncA (nag, attrs);
*	d0			      a0   d0
*
*	AMIGAGUIDECONTEXT OpenAmigaGuideAsyncA (struct NewAmigaGuide *,
*						struct TagItem *);
*
*	handle = OpenAmigaGuideAsync (nag, tag1, ...);
*
*	AMIGAGUIDECONTEXT OpenAmigaGuideAsyncA (struct NewAmigaGuide *,
*						Tag tag1, ...);
*
*   FUNCTION
*	Opens an AmigaGuide database for ansynchronous use.
*
*	The NewAmigaGuide structure, and its pointers, must stay valid until
*	an ActiveToolID or ToolStatusID message is received by the calling
*	process.
*
*	This function actually spawns OpenAmigaGuide() as another process, so,
*	for further documentation, refer to the OpenAmigaGuide() function.
*
*   INPUTS
*	nag	- Pointer to a valid NewAmigaGuide structure.
*		  (see OpenAmigaGuide() for documentation on its useage).
*
*	attrs	- Additional attributes.  See OpenAmigaGuideA().
*
*   RETURNS
*	handle	- Handle to an AmigaGuide system.
*
*   SEE ALSO
*	OpenAmigaGuideA(), CloseAmigaGuide()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID * ASM OpenAmigaGuideAsyncALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct NewAmigaGuide * nag, REG (d0) struct TagItem *attrs)
{
    struct AmigaGuideMsg *agm;
    struct Client *cl;

    /* Create a Client context */
    if (cl = AllocClient (ag, nag, attrs))
    {
	/* Create a message port for controlling the asynchronous process */
	if (cl->cl_AsyncPort = CreateMsgPort ())
	{
	    /* Create the asynchronous process */
	    if (cl->cl_Process = CreateNewProcTags (
						NP_Output,	Output(),
						NP_CloseOutput,	FALSE,
						NP_Input,	Input(),
						NP_CloseInput,	FALSE,
						NP_StackSize,	8192L,
						NP_Entry,	StartDaemonFunc,
						NP_Name,	PROC_NAME,
						NP_Priority,	0L,
						NP_Cli,		TRUE,
						NP_CommandName,	PROC_NAME,
						TAG_DONE))
	    {
		/* Point back to ourself */
		cl->cl_Process->pr_Task.tc_UserData = cl;

		/* Initialize the startup message */
		agm           = &cl->cl_StartupMsg;
		agm->agm_Type = StartupMsgID;

		/* Initialize the embedded message */
		agm->agm_Msg.mn_Node.ln_Type = NT_MESSAGE;
		agm->agm_Msg.mn_ReplyPort    = cl->cl_AsyncPort;
		agm->agm_Msg.mn_Length       = sizeof (struct AmigaGuideMsg);

		/* Send the message to the sub-process */
		PutMsg (&(cl->cl_Process->pr_MsgPort), &agm->agm_Msg);

		/* Return success */
		return (cl);
	    }
	}
	FreeClient (ag, cl);
    }

    SetIoErr (ERR_NOT_ENOUGH_MEMORY);

    return (NULL);
}

VOID StartDaemonFunc (void)
{
#ifdef	SysBase
#undef	SysBase
#endif
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct StartupMsg *agm;
    struct Process *pr;
    struct Client *cl;

    /* Get a pointer to the process structure */
    pr = (struct Process *) FindTask (NULL);

    /* Wait for the startup message */
    WaitPort (&pr->pr_MsgPort);
    agm = (struct StartupMsg *) GetMsg (&pr->pr_MsgPort);

    /* Get the Client pointer */
    cl = pr->pr_Task.tc_UserData;

    /* Call the main loop */
    MainLoop (cl->cl_AG, cl);

    /* Exit */
    Forbid ();
    ReplyMsg ((struct Message *) agm);
}
