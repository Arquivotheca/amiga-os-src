/* test.c
 * test for the tapedeck.gadget
 *
 */

#include <dos/dos.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "tapedeck.h"

extern struct Library *SysBase, *DOSBase;

struct Library *IntuitionBase;

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN \
			| IDCMP_MOUSEMOVE | IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS


void main (int argc, char **argv)
{
    struct Library *tapedeck;
    struct Window *win;
    struct Gadget *g;

    struct IntuiMessage *imsg;
    BOOL going = TRUE;
    ULONG sigr;

    if (IntuitionBase = OpenLibrary ("intuition.library", 39))
    {
	if (tapedeck = OpenLibrary ("gadgets/tapedeck.gadget", 39))
	{
	    if (win = OpenWindowTags (NULL,
				      WA_InnerWidth,	202,
				      WA_InnerHeight,	16,
				      WA_IDCMP,		IDCMP_FLAGS,
				      WA_DragBar,	TRUE,
				      WA_DepthGadget,	TRUE,
				      WA_CloseGadget,	TRUE,
#define	SIZING	TRUE
#if SIZING
				      WA_SizeGadget,	TRUE,
#endif
				      WA_MaxHeight,	1024,
				      WA_MaxWidth,	1024,
				      WA_Title,		(ULONG) "TapeDeck Gadget",
				      WA_SimpleRefresh,	TRUE,
				      WA_NoCareRefresh,	TRUE,
				      WA_PubScreenName,	(ULONG) "Workbench",
				      TAG_DONE))
	    {
		if (g = NewObject (NULL, "tapedeck.gadget",
#if SIZING
				   GA_RelRight,		-(win->BorderRight + 202),
				   GA_RelBottom,	-(win->BorderBottom + 16),
#else
				   GA_Left,		win->BorderLeft,
				   GA_Top,		win->BorderTop,
#endif
				   GA_Width,		202,
				   GA_Height,		16,
				   GA_RelVerify,	TRUE,
				   GA_FollowMouse,	TRUE,
				   GA_Immediate,	TRUE,

				   TDECK_Tape,		(argc > 1) ? TRUE : FALSE,

				   TAG_DONE))
		{
		    AddGList (win, g, -1, 1, NULL);
		    RefreshGList (g, win, NULL, 1);

		    printf ("press the close gadget to quit\n");
		    while (going)
		    {
			sigr = Wait ((1L << win->UserPort->mp_SigBit | SIGBREAKF_CTRL_C));

			if (sigr & SIGBREAKF_CTRL_C)
			    going = FALSE;

			while (imsg = (struct IntuiMessage *) GetMsg (win->UserPort))
			{
			    switch (imsg->Class)
			    {
				case IDCMP_CLOSEWINDOW:
				    going = FALSE;
				    break;

				case IDCMP_VANILLAKEY:
				    switch (imsg->Code)
				    {
					case '<':
					    printf ("rewind\n");
					    SetGadgetAttrs (g, win, NULL, TDECK_Mode, BUT_REWIND, TAG_DONE);
					    break;

					case 10:
					case 13:
					    printf ("play\n");
					    SetGadgetAttrs (g, win, NULL, TDECK_Mode, BUT_PLAY, TAG_DONE);
					    break;

					case '>':
					    printf ("fast forward\n");
					    SetGadgetAttrs (g, win, NULL, TDECK_Mode, BUT_FORWARD, TAG_DONE);
					    break;

					case 's':
					    printf ("stop\n");
					    SetGadgetAttrs (g, win, NULL, TDECK_Mode, BUT_STOP, TAG_DONE);
					    break;

					case 'p':
					    printf ("pause\n");
					    SetGadgetAttrs (g, win, NULL, TDECK_Mode, BUT_PAUSE, TAG_DONE);
					    break;

					case  27:
					case 'q':
					case 'Q':
					    going = FALSE;
					    break;
				    }
				    break;

				case IDCMP_MOUSEMOVE:
				case IDCMP_INTUITICKS:
				    if (g->Flags & GFLG_SELECTED)
					printf ("code=%04x : %04lx\n", imsg->Code, g->SpecialInfo);
				    break;

				case IDCMP_GADGETDOWN:
				case IDCMP_GADGETUP:
				    printf ("code=%04x : %04lx\n", imsg->Code, g->SpecialInfo);
				    break;
			    }

			    ReplyMsg ((struct Message *) imsg);
			}
		    }

		    RemoveGList (win, g, 1);

		    DisposeObject (g);
		}
		else
		    printf ("couldn't create tapedeck gadget\n");

		CloseWindow (win);
	    }
	    else
		printf ("couldn't open the window\n");

	    CloseLibrary (tapedeck);
	}
	else
	    printf ("couldn't open classes:gadgets/tapedeck.gadget\n");

	CloseLibrary (IntuitionBase);
    }
}
