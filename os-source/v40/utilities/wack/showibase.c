/* showibase.c : display IntuitionBase structure */
/* $Id: showibase.c,v 1.7 92/08/27 12:09:16 peter Exp $	*/

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "exec/semaphores.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

#if 0
#include "intuall.h"
#undef printf
/***
#include "intuition.h"
#include "intuinternal.h"
#include "intuitionbase.h"
#include "screens.h"
***/

#else
#include "intuition/iprefs.h"
#include "intuition/sc.h"
#include "intuition/i.h"
#include "intuition/intuinternal.h"
#include "intuition/ifunctions.h"
#endif

extern APTR CurrAddr;
extern APTR SpareAddr;

char *statename[] =
{
    "No-window",
    "Idle-window",
    "Requester",
    "Menu",
    "DMR-timeout",
    "Screen-drag",
    "Verify",
    "Gadget",
    "Window size/drag",
};

ShowIBase()
{
    struct IntuitionBase IBase;
    struct View	iview;
    APTR  ibase;

    if (ibase =  FindBase ("intuition.library"))
    {
	SpareAddr = ibase;
	GetBlock(ibase, &IBase, sizeof IBase);

	/*** Public Information ***/

	printf (" at %08lx\n", ibase);
#if 1
	printf( "ViewLord: %lx, ViewExtra: %lx\n",
	    IBADDR( &IBase.ViewLord ), IBase.ViewExtra);
	printf( "Active Window: %lx AScreen: %lx, FirstScreen: %lx\n",
	     IBase.ActiveWindow, IBase.ActiveScreen, IBase.FirstScreen);
	printf( "Flags: %x, DebugFlag at\n",
		IBase.Flags, IBADDR( &IBase.DebugFlag ) );
#else
	ShowAddress( "ViewLord", IBADDR( &IBase.ViewLord ) );
	ShowAddress ("ViewExtra", IBase.ViewExtra);
	ShowAddress ("ActiveWindow", IBase.ActiveWindow);
	ShowAddress ("ActiveScreen", IBase.ActiveScreen);
	ShowAddress ("FirstScreen", IBase.FirstScreen);
	Print ("Flags: %x\n", IBase.Flags);

	ShowAddress ("DebugFlag", IBADDR( &IBase.DebugFlag ) );
#endif

	/*** Private Information ***/

	/* State Machine */
	Print("-------- State Machine ------- State %d (%s-state)\n",
	     IBase.CurrentState, statename[ IBase.CurrentState] );
	Print( "Current InputToken %lx\n", IBase.InputToken );
	Print( "Return Pointers: ");
	Print( "  Gadget %lx", IBase.GadgetReturn );
	Print( "  SDrag %lx", IBase.SDragReturn );
	Print( "  WSD %lx", IBase.WSDReturn );
	Print( "  Verify %lx", IBase.VerifyReturn );
	Print( "  DMR %lx\n", IBase.DMRReturn );

	/* Lists, Queues, and Pools	*/
	Print("-------- Lists, Queues, and Pools -------\n");
	ShowAddressN( "TokenQueue", IBADDR( &IBase.TokenQueue ) );
	ShowAddress( "DeferredQueue", IBADDR( &IBase.DeferredQueue ) );

	ShowAddressN( "ITFreeList", IBADDR( &IBase.ITFreeList ) );
	ShowAddressN( "IEFoodList", IBADDR( &IBase.IEFoodList ) );
	ShowAddress( "IEQueue", IBADDR( &IBase.IEQueue ) );

	ShowAddressN( "IEFreeList", IBADDR( &IBase.IEFreeList ) );
	ShowAddress ("ISemaphore", IBADDR(IBase.ISemaphore));


	/*** Mouse ***/
	Print("-------- Mouse -------\n");
	Print ("Mouse X/Y %ld/%ld\n",	IBase.LongMouse.LX, IBase.LongMouse.LY);
	Print ("scale factors: %d(%d)/%d(%d)\n", 
		MOUSESCALEX, IBase.EffectiveScaleFactor.X,
		MOUSESCALEY, IBase.EffectiveScaleFactor.Y );

	dumpRect("MouseLimits", &IBase.MouseLimits);
	{   struct Rectangle	mlimits;

	    mlimits = IBase.MouseLimits;
	    mlimits.MinX /= MOUSESCALEX;
	    mlimits.MaxX /= MOUSESCALEX;
	    mlimits.MinY /= MOUSESCALEY;
	    mlimits.MaxY /= MOUSESCALEY;

	    dumpRect("(scaled back)", &mlimits);
	}

	/*** Gadgets ***/
	Print("-------- Gadgets -------\n");

	ShowAddress ("ActiveGadget", IBase.ActiveGadget);
	/* ShowAddress ("ActivePInfo", IBase.ActivePInfo); */
	ShowAddress ("GadgetEnv", IBADDR(&IBase.GadgetEnv));
	/* ShowAddress ("GadgetInfo", IBADDR(&IBase.GadgetInfo)); */

	/* omitting sysbase, gfxbase, layersbase, ... */

	Print("-------- Pointer -------\n");
	Print("\tPointer AX/YOffset %d, %d\n", IBase.AXOffset, IBase.AYOffset);

	Print("-------- Menu and Menu Rendering -------\n");
	Print ("MenuDrawn %x,\tMenuSelected %x\n", IBase.MenuDrawn,
		    IBase.MenuSelected);
	Print ("OptionList %x\n", IBase.OptionList );
	ShowAddressN("MenuRPort", IBADDR(&IBase.MenuRPort));

	ShowAddressN("ItemCRect", IBADDR(&IBase.ItemCRect));
	ShowAddress ("SubCRect", IBADDR(&IBase.SubCRect));

	Print("-------- Miscellany -------\n");
	Print("OrigNDRows/Cols: %d/%d\n", IBase.OrigNDRows, IBase.OrigNDCols);
	Print( "sizeof Private buffer: %d\n", sizeof IBase.Private );
	Print ("NewIControl %lx\n", IBase.NewIControl );
	Print ("ActiveMonitorSpec: %lx\n", IBase.ActiveMonitorSpec );
	ShowAddress( "WBPort", IBase.WBPort );
    }
    else
    {
	puts("\ncouldn't find intuition.library in exec library list.");
    }
}


/* no newline */
ShowAddressN(s, x)
char *s;
APTR x;
{
    printf("%s at", s);
    if (strlen(s) < 6) printf("\t");
    printf("\t%8lx  ", x);
}
