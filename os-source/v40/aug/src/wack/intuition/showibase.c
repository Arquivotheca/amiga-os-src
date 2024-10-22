/* showibase.c : display IntuitionBase structure */
/* $Id: showibase.c,v 40.1 93/09/10 11:11:00 peter Exp $	*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>
#include <exec/resident.h>

#include <exec/semaphores.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include "iwack_proto.h"

#include "/protos/wack_protos.h"
#include "/pragmas/wack_pragmas.h"

#include "I:cghooks.h"
#include "I:sghooks.h"
#include "I:iprefs.h"
#include "I:sc.h"
#include "I:i.h"
#include "I:intuinternal.h"
#include "I:ifunctions.h"

#define IBADDR(a) ((APTR) ((ULONG) (a) - (ULONG)(&IBase) + (ULONG)(ibase)))

extern struct MsgPort *WackBase;

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

void
ShowIBase( void )
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  Wack_FindLibrary ("intuition.library"))
    {
	Wack_Printf ("IntuitionBase at %08lx\n", ibase);

	Wack_WriteSpareAddr( ibase );
	Wack_ReadBlock(ibase, &IBase, sizeof IBase);

	/*** Public Information ***/

	Wack_Printf( "ViewLord: %lx, ViewExtra: %lx\n",
	    IBADDR( &IBase.ViewLord ), IBase.ViewExtra);
	Wack_Printf( "Active Window: %lx AScreen: %lx, FirstScreen: %lx\n",
	     IBase.ActiveWindow, IBase.ActiveScreen, IBase.FirstScreen);
	Wack_Printf( "Active Requester: %lx\n", IBase.ActiveRequester);
	Wack_Printf( "Flags: %lx, DebugFlag at\n",
		IBase.Flags, IBADDR( &IBase.DebugFlag ) );

	/*** Private Information ***/

	/* State Machine */
	Wack_Printf("-------- State Machine ------- State %ld (%s-state)\n",
	     IBase.CurrentState, statename[ IBase.CurrentState] );
	Wack_Printf( "Current InputToken %lx\n", IBase.InputToken );
	Wack_Printf( "Return Pointers: ");
	Wack_Printf( "  Gadget %lx", IBase.GadgetReturn );
	Wack_Printf( "  SDrag %lx", IBase.SDragReturn );
	Wack_Printf( "  WSD %lx", IBase.WSDReturn );
	Wack_Printf( "  Verify %lx", IBase.VerifyReturn );
	Wack_Printf( "  DMR %lx\n", IBase.DMRReturn );

	/* Lists, Queues, and Pools	*/
	Wack_Printf("-------- Lists, Queues, and Pools -------\n");
	ShowAddressN( "TokenQueue", IBADDR( &IBase.TokenQueue ) );
	ShowAddress( "DeferredQueue", IBADDR( &IBase.DeferredQueue ) );

	ShowAddressN( "ITFreeList", IBADDR( &IBase.ITFreeList ) );
	ShowAddressN( "IEFoodList", IBADDR( &IBase.IEFoodList ) );
	ShowAddress( "IEQueue", IBADDR( &IBase.IEQueue ) );

	ShowAddressN( "IEFreeList", IBADDR( &IBase.IEFreeList ) );
	ShowAddress ("ISemaphore", IBADDR(IBase.ISemaphore));


	/*** Mouse ***/
	Wack_Printf("-------- Mouse -------\n");
	Wack_Printf ("Mouse X/Y %ld/%ld\n",	IBase.LongMouse.LX, IBase.LongMouse.LY);
	Wack_Printf ("MOUSESCALE %ld/%ld, MouseScale %ld/%ld, EffectiveScaleFactor %ld/%ld\n",
		MOUSESCALEX, MOUSESCALEY,
		IBase.MouseScaleX, IBase.MouseScaleY,
		IBase.EffectiveScaleFactor.X, IBase.EffectiveScaleFactor.Y );

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
	Wack_Printf("-------- Gadgets -------\n");

	ShowAddress ("ActiveGadget", IBase.ActiveGadget);
	/* ShowAddress ("ActivePInfo", IBase.ActivePInfo); */
	ShowAddress ("GadgetEnv", IBADDR(&IBase.GadgetEnv));
	/* ShowAddress ("GadgetInfo", IBADDR(&IBase.GadgetInfo)); */

	/* omitting sysbase, gfxbase, layersbase, ... */

	Wack_Printf("-------- Pointer -------\n");
	Wack_Printf("SimpleSprite: %08lx\tCustomExtSprite: %08lx\n",
	    IBase.SimpleSprite, IBase.CustomExtSprite );
	Wack_Printf("DefaultExtSprite: %08lx %08lx %08lx\n",
	    IBase.DefaultExtSprite[0], IBase.DefaultExtSprite[1], IBase.DefaultExtSprite[2] ); 
	Wack_Printf("MousePointers:  Default: %08lx\tBusy: %08lx\tCustom: %08lx\n",
	    IBase.DefaultMousePointer, IBase.DefaultBusyPointer,
	    IBase.CustomMousePointer);
	Wack_Printf("AX/YOffset: %ld/%ld\tPointerXRes: %lx\tPointerYRes: %lx\n",
	    IBase.AXOffset, IBase.AYOffset, IBase.PointerXRes, IBase.PointerYRes );

	Wack_Printf("-------- Menu and Menu Rendering -------\n");
	Wack_Printf ("MenuDrawn %lx,\tMenuSelected %lx\n", IBase.MenuDrawn,
		    IBase.MenuSelected);
	Wack_Printf ("OptionList %lx\n", IBase.OptionList );
	ShowAddressN("MenuRPort", IBADDR(&IBase.MenuRPort));

	ShowAddressN("ItemCRect", IBADDR(&IBase.ItemCRect));
	ShowAddress ("SubCRect", IBADDR(&IBase.SubCRect));

	Wack_Printf("-------- Miscellany -------\n");
	Wack_Printf("OrigNDRCols/Rows: %ld/%ld\n", IBase.OrigNDCols, IBase.OrigNDRows);
	Wack_Printf ("NewIControl %lx\n", IBase.NewIControl );
	Wack_Printf ("ActiveMonitorSpec: %lx\n", IBase.ActiveMonitorSpec );
	ShowAddress( "WBPort", IBase.WBPort );
	ShowAddress( "IClassList", &((struct IntuitionBase *)ibase)->IClassList );

	ShowAddress( "Input Device Interrupt", IBADDR( &IBase.InputInterrupt ) );
    }
    else
    {
	Wack_Printf("Couldn't find intuition.library.\n");
    }
}

void
IDebug( char *arg )
{
    ULONG value;
    struct IntuitionBase *ibase;

    if ( ibase = (struct IntuitionBase *)Wack_FindLibrary("intuition.library") )
    {
	if ( ScanArg( arg, (ULONG *) &value ) )
	{
	    Wack_WriteWord( &ibase->DebugFlag, value );
	}
	else
	{
	    Wack_Printf( "IBase->DebugFlag %lx\n", Wack_ReadWord( &ibase->DebugFlag ) );
	}
    }
    else
    {
	Wack_Printf("Couldn't find intuition.library.\n");
    }
}
