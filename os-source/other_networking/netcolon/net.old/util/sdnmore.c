#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include "hyper_lib.h"
#include "hyper_protos.h"
#include "hyper_pragmas.h"

STRPTR context[] =
{
    "MAIN",
    "INTRO",
    NULL
};

struct Library *HyperBase;
struct Library *IntuitionBase;
struct NewHyper nh;

/* NewWindow Structures */
struct NewWindow NewWindowStructure1 =
{
    0, 0, 640, 100, -1, -1,
    RAWKEY | CLOSEWINDOW,
    WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE
    | SIZEBRIGHT | ACTIVATE | NOCAREREFRESH,
    NULL, NULL, NULL, NULL, NULL, 150, 50, 65535, 65535, WBENCHSCREEN,
};

main (int argc, char **argv)
{
    IntuitionBase = OpenLibrary ("intuition.library", 0);

    /* set the document name */
    nh.nh_Name = "SDN.hyper";
    if (argc >= 2)
    {
	nh.nh_Name = argv[1];
    }

    nh.nh_BaseName = "htmore";

    if (HyperBase = OpenLibrary ("hyper.library", 33))
    {
	HYPERCONTEXT handle = NULL;
	struct HyperMsg *hmsg;
	ULONG sigr = 0L, sigb = 0L;
	BOOL going = TRUE;

	/* establish the base name to use for hypertext ARexx port */
	nh.nh_ClientPort = "HTMORE";

	/* set up the context table */
	nh.nh_Context = context;

	/* open the help system */
	handle = OpenHyperAsync (&nh, NULL);

	/* get our signal bit */
	sigb = HyperSignal (handle);

	SetHyperContext (handle, 0L, NULL);

	while (going)
	{
	    /* wait for a signal */
            sigr = Wait (sigb);

            /* process hyper messages */
	    while (hmsg = GetHyperMsg (handle))
	    {
	        /* check message types */
		switch (hmsg->hmsg_Type)
		{
			    /* tool is active and ready */
			case ActiveToolID:
			    /* align on the first */
	 		    SendHyperCmd (handle, "LINK MAIN", NULL);
			    break;

			    /* this is a reply to our cmd */
			case ToolCmdReplyID:
			    if (hmsg->hmsg_Pri_Ret)
			    {
				/* uh-oh, there was an error */
				printf ("error %ld\n", hmsg->hmsg_Sec_Ret);
			    }
			    break;

			    /* this is a status message */
			case ToolStatusID:
			    if (hmsg->hmsg_Pri_Ret)
			    {
				/* uh-oh, there was an error */
				printf ("error %ld\n", hmsg->hmsg_Sec_Ret);
			    }
			    break;

			    /* shutdown message */
			case ShutdownMsgID:
			    if (hmsg->hmsg_Pri_Ret)
			    {
				/* uh-oh, there was an error */
				printf ("error %ld\n", hmsg->hmsg_Sec_Ret);
			    }

			    going = FALSE;
			    break;

			default:
			    break;
		    }

		    /* reply to the message */
		    ReplyHyperMsg (hmsg);
		}
	}

	/* shutdown the help system */
	CloseHyper (handle);

	/* close the library */
	CloseLibrary (HyperBase);
    }

    CloseLibrary (IntuitionBase);
}
