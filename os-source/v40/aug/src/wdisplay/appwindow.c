/* appwindow.c
 * AppWindow message handling
 *
 */

#include "wdisplay.h"

/* Process AppWindow message */
VOID HandleAppMessage (struct AppInfo * ai)
{
    struct AppMessage *amsg;

    /* Activate the window */
    ActivateWindow (ai->ai_Window);

    /* Pull each message from the AppWindow message port */
    while (amsg = (struct AppMessage *) GetMsg (ai->ai_AWMPort))
    {
	/* From the MenuItem, so bring the screen to front */
	if (amsg->am_ID == 1)
	{
	    ScreenToFront (ai->ai_Window->WScreen);
	}

	/* We only handle one argument... */
	if (amsg->am_NumArgs > 0)
	{
	    /* Make sure we have a name (TOOL or PROJECT) */
	    if (strlen (amsg->am_ArgList->wa_Name))
	    {
		LoadPicture (ai, amsg->am_ArgList->wa_Lock, amsg->am_ArgList->wa_Name);
	    }
	}

	/* Reply to the Workbench message */
	ReplyMsg ((struct Message *) amsg);
    }
}
