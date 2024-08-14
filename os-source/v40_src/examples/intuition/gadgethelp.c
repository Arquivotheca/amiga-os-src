/*
 * gadgethelp.c - shows help and bounds on a prop gadget
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 * 
 * NB: If using GadTools, the correct bounds and help-properties
 * are already set in GadTools gadgets.
 *
 * This demo shows off the gadget help feature of V39.  With this
 * feature, an application can be notified when the user stops the
 * mouse over a gadget.  The intent is that the application would
 * then offer help (perhaps using AmigaGuide) on that gadget.
 * Gadget help can be set selectively for any user gadget.  The
 * application shows the correct handling of the IDCMP_GADGETHELP
 * message, including decisions about when the mouse is not in
 * the window, or when it's in the window but not over any help-reporting
 * gadget.
 *
 * Note that all GadTools gadgets correctly set their bounds and
 * the gadget help property.
 * 
 */

/*------------------------------------------------------------------------*/

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

/*------------------------------------------------------------------------*/

void bail_out(int code);

/*------------------------------------------------------------------------*/

struct Image im =
{
    0,0,0,0,0,
    0,
};

struct PropInfo mypinfo =
{
    AUTOKNOB|FREEVERT,
    0, 2730,	/*  VPot represents 1 on scale 0..28 with 5 visible */
    0, 9362,	/*  VBody represents 0..28 with 5 visible */
    0, 0, 0, 0, 0, 0,
};


struct ExtGadget myGad =
{
    NULL,	/*  NextGadget */
    20,12,	/*  LeftEdge, TopEdge */
    80,100,	/*  Width, Height */
    GFLG_EXTENDED | GADGHNONE | GADGIMAGE,		/*  Flags */
    GADGIMMEDIATE | FOLLOWMOUSE | RELVERIFY,	/*  Activation */
    PROPGADGET,	/*  GadgetType */
    &im,	/*  GadgetRender */
    NULL,	/*  SelectRender */
    NULL,	/*  GadgetText */
    NULL,	/*  MutualExclude */
    &mypinfo,	/*  SpecialInfo */
    NULL,	/*  GadgetID */
    NULL,	/*  UserData */
    GMORE_BOUNDS|GMORE_GADGETHELP,
    10,10,	/*  LeftEdge, TopEdge */
    100,120,	/*  Width, Height */
};

struct NewWindow testnewwindow =
{
    0,0,		/*  LeftEdge, TopEdge */
    320,150,            /*  Width, Height */
    -1, -1,             /*  DetailPen, BlockPen */
    IDCMP_GADGETHELP | IDCMP_RAWKEY | CLOSEWINDOW | MOUSEMOVE | GADGETUP | GADGETDOWN,	/*  IDCMPFlags */
    WINDOWDRAG | WINDOWSIZING | WINDOWDEPTH | WINDOWCLOSE |
    NOCAREREFRESH | SMART_REFRESH,	/* Flags */
    NULL,		/*  FirstGadget */
    NULL,		/*  CheckMark */
    (UBYTE *) "Gadget Help Demo",	/*  Title */
    NULL,		/*  Screen */
    NULL,		/*  BitMap */    
    100,50,		/*  MinWidth, MinHeight */
    640,200,		/*  MaxWidth, MaxHeight */
    WBENCHSCREEN,	/*  Type */
};

/*------------------------------------------------------------------------*/

struct IntuitionBase *IntuitionBase;
struct Window *mywin;

/*------------------------------------------------------------------------*/

main()
{
    BOOL terminated;
    struct IntuiMessage *imsg;

    terminated = FALSE;
    mywin = NULL;

    if (!(IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library",39L)))
	bail_out(20);

    if (!(mywin = OpenWindowTags(&testnewwindow,
	TAG_DONE)))
	bail_out(20);

    myGad.TopEdge += mywin->WScreen->Font->ta_YSize;
    AddGadget( mywin, &myGad, ~0 );
    RefreshGList( &myGad, mywin, NULL, 1 );

    printf("Window at %lx, gadget at %lx\n", mywin, &myGad);

    /* Turn on Gadget Help */
    HelpControl( mywin, HC_GADGETHELP );

    while (!terminated)
    {
	Wait (1 << mywin->UserPort->mp_SigBit);
	while (imsg = (struct IntuiMessage *) GetMsg(mywin->UserPort))
	{
	    switch ( imsg->Class )
	    {
	    case IDCMP_MOUSEMOVE:
		printf("MouseMove (Pot = %ld)\n", (LONG)mypinfo.VertPot);
		break;

	    case IDCMP_CLOSEWINDOW:
		terminated = TRUE;
		break;

	    case IDCMP_RAWKEY:
		printf("RAWKEY %lx\n", imsg->Code);
		break;

	    case IDCMP_GADGETUP:
		printf("Gadget Up (Pot = %ld)\n", (LONG)mypinfo.VertPot);
		break;

	    case IDCMP_GADGETDOWN:
		printf("Gadget Down (Pot = %ld)\n", (LONG)mypinfo.VertPot);
		break;

	    case IDCMP_GADGETHELP:
		if ( imsg->IAddress == NULL )
		{
		    printf("Gadget Help: Mouse not over window\n");
		}
		else if ( imsg->IAddress == (APTR) mywin )
		{
		    printf("Gadget Help: Mouse in window, not over a gadget\n");
		}
		else
		{
		    /* Detect system gadgets.  Figure out by looking at
		     * system-gadget-type bits in GadgetType field:
		     */
		    LONG sysgtype =
			((struct Gadget *)imsg->IAddress)->GadgetType & 0xF0;
		    switch ( sysgtype )
		    {
		    case GTYP_SIZING:
			printf("Gadget Help for window sizing gadget\n");
			break;

		    case GTYP_WDRAGGING:
			printf("Gadget Help for window drag-bar\n");
			break;

		    case GTYP_WUPFRONT:
			printf("Gadget Help for window depth gadget\n");
			break;

		    case GTYP_WDOWNBACK:
			printf("Gadget Help for window zoom gadget\n");
			break;

		    case GTYP_CLOSE:
			printf("Gadget Help for window close gadget\n");
			break;

		    case 0:
			/* In this example, we only have one gadget,
			 * so we know which one it is.  Normally, you'd
			 * have to figure that out here, using the
			 * usual techniques you already use for other
			 * gadget messages.
			 */
			printf("Gadget Help for proportional gadget, code %ld\n",
			    imsg->Code);
			break;

		    default:
			    printf("Gadget Help on some other system gadget\n");
			    break;
		    }
		}
	    }
	    ReplyMsg( imsg );
	}
    }
    bail_out(0);
}


/*------------------------------------------------------------------------*/

/*/ bail_out()
 *
 * Close any allocated or opened stuff.
 *
 */

void bail_out(code)

int code;
{
    if (mywin)
	CloseWindow(mywin);

    if (IntuitionBase)
	CloseLibrary(IntuitionBase);

    exit(code);
}

/*------------------------------------------------------------------------*/
