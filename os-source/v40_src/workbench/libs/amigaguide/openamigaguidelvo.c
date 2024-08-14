/* openamigaguidelvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/OpenAmigaGuideA **********************************
*
*    NAME
*	OpenAmigaGuideA - Open a synchronous AmigaGuide database.
*
*    SYNOPSIS
*	handle = OpenAmigaGuideA (nag, attrs);
*	d0			 a0   a1
*
*	AMIGAGUIDECONTEXT OpenAmigaGuideA (struct NewAmigaGuide *,
*					   struct TagItem *);
*
*	handle = OpenAmigaGuide (nag, tag1, ...);
*
*	AMIGAGUIDECONTEXT OpenAmigaGuide (struct NewAmigaGuide *,
*					  Tag tag1, ...);
*
*    FUNCTION
*	Opens a AmigaGuide database, complete with the first viewing
*	window, for synchronous activity.
*
*	Before you call OpenAmigaGuide(), you must initialize a NewAmigaGuide
*	structure.  NewAmigaGuide is a structure that contains all the
*	information needed to open a database.  The NewAmigaGuide structure
*	must be retained until the call returns.
*
*	The function will not return until the user closes all the
*	windows.
*
*    INPUTS
*	nag - Pointer to an instance of a NewAmigaGuide structure.  That
*	    structure is initialized with the following data.
*
*		  nag_Lock
*		  Lock on the directory that the database is located in.
*		  Not needed if nag_Name contains the complete path name.
*
*		  nag_Name
*		  Name of the AmigaGuide database.
*
*		  nag_Screen
*		  Screen to open the viewing windows on, NULL for the
*		  Workbench screen.
*
*		  nag_PubScreen
*		  Pointer to the name of the public screen to open on.
*		  Must already be opened.
*
*		  nag_HostPort
*		  Name of the applications' ARexx port (currently not used).
*
*		  nag_ClientPort
*		  Base name to use for the databases' ARexx port.
*
*		  nag_Flags
*		  Used to specify the requirements of this database.  The
*		  flags are defined in <libraries/amigaguide.h>.
*
*		  nag_Context
*		  NULL terminated array of context nodes, in the form of:
*
*			\* context array *\
*			STRPTR context[] =
*			{
*			    "MAIN",
*			    "INTRO",
*			    "GADGETS",
*			    NULL
*			};
*
*		  The context array is not copied, but referenced,
*		  therefore must remain static throughout the useage of
*		  the AmigaGuide system.  This array is only referenced
*		  when using the SetAmigaGuideContext() function.
*
*		  nag_Node
*		  Node to start at (does not work with OpenAmigaGuideAsync()).
*
*		  nag_Line
*		  Line to start at (does not work with OpenAmigaGuideAsync()).
*
*		  nag_Extens
*		  Used by V37 and beyond to pass additional arguments.
*
*		  nag_Client
*		  This is a private pointer, MUST be initialized to NULL.
*
*	attrs - Additional attributes.
*
*    TAGS
*	AGA_HelpGroup (ULONG) -- Unique identifier used to identify the
*	    AmigaGuide help windows.  See OpenWindow() and GetUniqueID().
*
*	    Default for this tag is NULL.  Applicability is (I). (V39)
*
*    EXAMPLE
*
*	\* Short example showing synchronous AmigaGuide access *\
*	LONG ShowAmigaGuideFile (STRPTR name, STRPTR node, LONG line)
*	{
*	    struct NewAmigaGuide nag = {NULL};
*	    AMIGAGUIDECONTEXT handle;
*	    LONG retval = 0L;
*
*	    \* Fill in the NewAmigaGuide structure *\
*	    nag.nag_Name = name;
*	    nag.nag_Node = node;
*	    nag.nag_Line = line;
*
*	    \* Open the AmigaGuide client *\
*	    if ( handle = OpenAmigaGuide(&nag, NULL))
*	    {
*		\* Close the AmigaGuide client *\
*		CloseAmigaGuide(handle);
*	    }
*	    else
*	    {
*		\* Get the reason for failure *\
*		retval = IoErr();
*	    }
*
*	    return (retval);
*	}
*
*    RETURNS
*	handle - Handle to a AmigaGuide system.
*
*    SEE ALSO
*	OpenAmigaGuideAsyncA(), CloseAmigaGuide()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID * ASM OpenAmigaGuideALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct NewAmigaGuide *nag, REG (a1) struct TagItem *attrs)
{
    struct Client *cl;

    /* Create a Client context */
    if (cl = AllocClient (ag, nag, attrs))
    {
	MainLoop (ag, cl);
	SetIoErr (cl->cl_ErrorNumber);
    }
    return (cl);
}
