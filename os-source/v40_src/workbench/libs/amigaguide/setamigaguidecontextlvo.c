/* setamigaguidecontextlvo.c
 *
 */

#include "amigaguidebase.h"

#define	DB(x)	;

/****** amigaguide.library/SetAmigaGuideContextA ****************************
*
*   NAME
*	SetAmigaGuideContextA - Set the context ID for an AmigaGuide system.
*                                                               (V34)
*   SYNOPSIS
*	success = SetAmigaGuideContextA ( handle, context, attrs );
*	d0				 a0	    d0	     d1
*
*	BOOL SetAmigaGuideContextA (AMIGAGUIDECONTEXT, ULONG, struct TagItem *);
*
*	success = SetAmigaGuideContext (handle, context, tag1, ...);
*
*	BOOL SetAmigaGuideContext (AMIGAGUIDECONTEXT, ULONG, Tag, ...);
*
*   FUNCTION
*	This function, and the SendAmigaGuideContext() function, are used to
*	provide a simple way to display a node based on a numeric value,
*	instead of having to build up a slightly more complex command
*	string.
*
*   INPUTS
*	handle	- Handle to an AmigaGuide system.
*
*	context	- Index value of the desired node to display.
*
*	future	- Future expansion, must be set to NULL for now.
*
*   EXAMPLE
*
*	\* sample context table *\
*	STRPTR ContextArray[] =
*	{
*	    "MAIN",
*	    "FILEREQ",
*	    "PRINT",
*	    "ABOUT",
*	    NULL
*	};
*
*	\* quickie defines *\
*	#define	HELP_MAIN	0
*	#define	HELP_FILEREQ	1
*	#define	HELP_PRINT	2
*	#define	HELP_ABOUT	3
*
*	...
*
*	struct NewAmigaGuide nag = {NULL};
*
*	\* initialize the context table *\
*	nag.nag_Context = ContextArray;
*
*	...
*
*	\* bring up help on a particular subject *\
*	SetAmigaGuideContext(handle, HELP_ABOUT, NULL);
*
*   RETURNS
*	success	- Returns TRUE if a valid context ID was passed,
*	    otherwise returns FALSE.
*
*   SEE ALSO
*	SendAmigaGuideContextA(), SendAmigaGuideCmdA()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM SetAmigaGuideContextALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client * cl, REG (d0) ULONG id, REG (d1) struct TagItem *attrs)
{
    LONG max = 0L;

    DB (kprintf ("SetAmigaGuideContextALVO (0x%lx, %ld)\n", cl, id));
    if (cl && cl->cl_Context)
    {
	while (cl->cl_Context[max])
	    max++;

	if (id < max)
	{
	    DB (kprintf ("context set\n"));
	    cl->cl_ContextID = id;
	    return TRUE;
	}
    }
    DB (kprintf ("unable to set context\n"));
    return FALSE;
}
