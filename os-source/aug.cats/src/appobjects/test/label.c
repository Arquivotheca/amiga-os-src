/* label.c
 * test label gadget class
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/apshattrs.h>
#include <utility/tagitem.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/utility.h>
#include <string.h>

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase, *GfxBase, *UtilityBase, *AppObjectsBase;

struct ExtNewWindow NewWindow =
{
    0, 0, 250, 125,		/* Window placement rectangle */
    0, 1,			/* DetailPen, BlockPen */

    CLOSEWINDOW |		/* IDCMP flags */
    GADGETDOWN | GADGETUP |
    ACTIVEWINDOW | INACTIVEWINDOW |
    IDCMPUPDATE,

    NOCAREREFRESH |		/* Window flags */
    SIMPLE_REFRESH | WINDOWDRAG |
    WINDOWDEPTH | WINDOWCLOSE |
    WFLG_SIZEBRIGHT | WFLG_SIZEBBOTTOM |
    WINDOWSIZING,

    NULL,			/* First gadget */
    NULL,			/* Menu checkmark */
    NULL,			/* Window title */
    NULL,			/* Screen pointer */
    NULL,			/* Custom bitmap */
    150, 40,			/* Minimum width & height */
    1024, 1024,			/* Maximum width & height */
    WBENCHSCREEN,		/* Screen type */
    NULL,
};

VOID
update (struct Window * win, ULONG line, LONG value)
{
    struct RastPort *rp = win->RPort;
    WORD xoff = win->BorderLeft;
    WORD yoff = win->BorderTop;
    UBYTE buf[10];

    sprintf (buf, "%6ld", value);

    SetDrMd (rp, JAM2);
    SetAPen (rp, 1);
    SetBPen (rp, 0);
    Move (rp, (xoff + 10), (yoff + (line * 10)));
    Text (rp, buf, strlen (buf));
}

BOOL OpenLibraries (VOID)
{

    if (IntuitionBase = OpenLibrary ("intuition.library", 36))
    {
	if (GfxBase = OpenLibrary ("graphics.library", 36))
	{
	    if (UtilityBase = OpenLibrary ("utility.library", 36))
	    {
		if (AppObjectsBase = OpenLibrary ("appobjects.library", 36))
		{
		    return (TRUE);
		}
		CloseLibrary (AppObjectsBase);
	    }
	    CloseLibrary (GfxBase);
	}
	CloseLibrary (IntuitionBase);
    }
    return (FALSE);
}

VOID CloseLibraries (VOID)
{

    if (IntuitionBase)
    {
	CloseLibrary (AppObjectsBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
    }
}

main (int argc, char **argv)
{
    struct Gadget *gad, *gad1, *gad2, *gad3, *gad4;
    ULONG freeTextIClass (Class * cl);
    Class *initTextIClass (VOID);
    UWORD flag = PLACETEXT_LEFT;
    struct IntuiMessage *imsg;
    struct Window *win;
    BOOL going = TRUE;
    struct TagItem *tags;
    ULONG gid;
    LONG curval;

    if (argc > 1)
    {
	flag = (UWORD) atoi (argv[1]);
    }

    /* Open the required libraries */
    if (OpenLibraries ())
    {
	    if (win = OpenWindow (&NewWindow))
	    {
		/* gadget... */
		gad1 = (struct Gadget *)
		    NewObject (NULL, "scrollgclass",
				GA_RelRight, -100,
				GA_Top, ((2 * win->BorderTop) + 2),
				GA_Width, 16L,
				GA_RelHeight, -((2 * win->BorderTop) + win->BorderBottom + 14L),
				GA_ID, 1L,
				GA_Immediate, TRUE,
				GA_RelVerify, TRUE,
				PGA_Visible, 2L,
				PGA_Total, 20L,
				PGA_Freedom, FREEVERT,
				ICA_TARGET, ICTARGET_IDCMP,
				TAG_DONE);

		gad2 = (struct Gadget *)
		    NewObject (NULL, "labelgclass",
				GA_Text, "Drag _Me",
				GA_Previous, gad1,
				APSH_GTFlags, (ULONG)flag,
				TAG_DONE);

		gad3 = (struct Gadget *)
		    NewObject (NULL, "scrollgclass",
				GA_RelRight, -17L,
				GA_Top, (win->BorderTop + 1),
				GA_Width, 18L,
				GA_RelHeight, -(win->BorderTop + win->BorderBottom + 1),
				GA_RightBorder, TRUE,
				GA_ID, 2L,
				GA_Previous, gad2,
				GA_Immediate, TRUE,
				GA_RelVerify, TRUE,
				PGA_Visible, 2L,
				PGA_Total, 20L,
				PGA_Freedom, FREEVERT,
				PGA_Borderless, TRUE,
				ICA_TARGET, ICTARGET_IDCMP,
				TAG_DONE);

		gad4 = (struct Gadget *)
		    NewObject (NULL, "scrollgclass",
				GA_Left, (win->BorderLeft - 1),
				GA_RelBottom, -9L,
				GA_RelWidth, -(win->BorderLeft + win->BorderRight - 1),
				GA_Height, 10L,
				GA_BottomBorder, TRUE,
				GA_ID, 3L,
				GA_Previous, gad3,
				GA_Immediate, TRUE,
				GA_RelVerify, TRUE,
				PGA_Visible, 2L,
				PGA_Total, 20L,
				PGA_Freedom, FREEHORIZ,
				PGA_Borderless, TRUE,
				ICA_TARGET, ICTARGET_IDCMP,
				TAG_DONE);

		if (gad1)
		{
		    AddGList (win, gad1, -1, -1, NULL);
		    RefreshGList (gad1, win, NULL, -1);

		    while (going)
		    {
			Wait (1L << win->UserPort->mp_SigBit);

			while (imsg = (struct IntuiMessage *) GetMsg (win->UserPort))
			{
			    /* Process the events */
			    switch (imsg->Class)
			    {
				case GADGETDOWN:
				    gad = (struct Gadget *) imsg->IAddress;
				    printf ("Gadget %d down\n",
					gad->GadgetID);
				    break;

				case IDCMPUPDATE:
				    tags = (struct TagItem *) imsg->IAddress;
				    curval = GetTagData (PGA_Top, 0, tags);
				    update (win, (ULONG)gad->GadgetID, curval);
				    break;

				case GADGETUP:
				    gad = (struct Gadget *) imsg->IAddress;
				    update (win, (ULONG)gad->GadgetID, (LONG)imsg->Code);
				    printf ("Gadget %d up with %d\n",
					gad->GadgetID, imsg->Code);
				    break;

				case CLOSEWINDOW:
				    going = FALSE;
				    break;
			    }

			    /* Reply to the message */
			    ReplyMsg ((struct Message *) imsg);
			}
		    }

		    /* Remove the gadgets from the window */
		    RemoveGList (win, gad1, 4);

		    /* Free the gadget */
		    DisposeObject (gad1);
		    DisposeObject (gad2);
		    DisposeObject (gad3);
		    DisposeObject (gad4);
		}

		/* Close the window */
		CloseWindow (win);
	    }

	CloseLibraries ();
    }
}
