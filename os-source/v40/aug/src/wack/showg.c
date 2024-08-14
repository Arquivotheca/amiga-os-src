/*
 * Amiga Grand Wack
 *
 * showg.c -- Show graphics.library structures.
 *
 * $Id: showg.c,v 39.3 93/05/21 17:35:02 peter Exp $
 *
 * This module contains code to display graphics.library structures on
 * the target Amiga, including ViewPorts, Views, ViewExtras, Layer_Infos,
 * Layers, and GfxBase.  Copper-list stuff is elsewhere.
 *
 */

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

#include "graphics/gfxbase.h"
#include "graphics/layers.h"
#include "graphics/clip.h"
#include "graphics/view.h"

#include "symbols.h"
#include "special.h"

#include "sys_proto.h"
#include "parse_proto.h"
#include "special_proto.h"
#include "link_proto.h"
#include "showg_proto.h"
#include "io_proto.h"
#include "ops_proto.h"

extern APTR CurrAddr;
extern APTR SpareAddr;


ULONG
dumpViewPort( APTR addr, struct ViewPort *vp )
{
    ShowAddress("ViewPort", addr);

    ShowAddress("Next", vp->Next);
    ShowAddress("ColorMap", vp->ColorMap);
    ShowAddress("DspIns", vp->DspIns);
    ShowAddress("SprIns", vp->SprIns);
    ShowAddress("UCopIns", vp->UCopIns);
    PPrintf("DWidth/Height\t0x%lx/0x%lx\t(%ld/%ld)\n",
	vp->DWidth, vp->DHeight, vp->DWidth, vp->DHeight);
    PPrintf("Dx/DyOffsets\t0x%lx/0x%lx\t(%ld/%ld)\n",
	vp->DxOffset, vp->DyOffset, vp->DxOffset, vp->DyOffset);
    PPrintf("Modes\t\t%lx\n", vp->Modes);
    PPrintf("Sprite priorities 0x%lx, EModes %lx\n", vp->SpritePriorities,
	vp->ExtendedModes);
    ShowAddress("RasInfo", vp->RasInfo);

    return (0);
}

SHOWSIMPLE(ViewPort,TRUE)

ULONG
dumpView( APTR addr, struct View *v )
{
    ShowAddress("View", addr);

    ShowAddress("ViewPort", v->ViewPort);
    ShowAddress("LOFCprList", v->LOFCprList);
    ShowAddress("SHFCprList", v->SHFCprList);
    PPrintf("Dx/DyOffsets\t0x%lx/0x%lx\t(%ld/%ld)\n",
	v->DxOffset, v->DyOffset, v->DxOffset, v->DyOffset);
    PPrintf("Modes\t\t%lx\n", v->Modes);

    return (0);
}

SHOWONE(View)

ULONG
dumpViewExtra( APTR addr, struct ViewExtra *ve )
{
    ShowAddress("ViewExtra", addr);

    ShowAddress("View (backlink)", ve->View);
    ShowAddress("MonitorSpec", ve->Monitor);
#if 0
    PPrintf("VideoFlags: %lx\n", ve->ve_VideoFlags );
#endif

    return (0);
}

SHOWONE(ViewExtra)

ULONG
dumpLayer_Info( APTR addr, struct Layer_Info *li )
{
    ShowAddress("Layer_Info", addr);
    PPrintf("Flags %lx\n", li->Flags);
    ShowAddress("top_layer", li->top_layer);
    ShowAddress("layer info lock", CADDR(addr, li, &li->Lock));
    ShowAddress("semaphore list gs_Head", CADDR(addr, li, &li->gs_Head));
    return (0);
}

SHOWONE(Layer_Info)

ULONG
dumpLayer( APTR addr, struct Layer *l )
{
    ShowAddress("Layer", addr);
    PPrintf("\tbounds: %ld %ld, %ld, %ld\n",
	l->bounds.MinX, l->bounds.MinY, l->bounds.MaxX, l->bounds.MaxY);
    PPrintf("\tFlags %lx\n", l->Flags);
    ShowAddress("\tWindow", l->Window);
    ShowAddress("\tLock", CADDR(addr, l, &l->Lock));
    ShowAddress("\tdamage list", l->DamageList);
    return(0);
}

SHOWONE(Layer)


STRPTR
ShowGfxBase( void )
{
    APTR gbase;			/* amiga address of graphics base */
    struct GfxBase GBase;	/* sun buffer for base */

    if (gbase =  FindBase ("graphics.library"))
    {
	SpareAddr = gbase;
	/* fill sun bugger with data from amiga */
	ReadBlock(gbase, &GBase, sizeof GBase);

	/* dump out some fields */
	PPrintf ("GfxBase at %08lx\n", gbase);
	PPrintf ("Modes %lx Flags %lx\n", GBase.Modes, GBase.Flags);
	PPrintf("DisplayFlags %lx\n", GBase.DisplayFlags);

	NewLine();
	ShowAddress ("ActiView", GBase.ActiView);
	ShowAddress ("LOFlist", GBase.LOFlist);
	ShowAddress ("SHFlist", GBase.SHFlist);
	PPrintf("NormalDisplayRows/Columns: 0x%lx/0x%lx (%ld/%ld)\n",
		GBase.NormalDisplayRows, GBase.NormalDisplayColumns,
		GBase.NormalDisplayRows, GBase.NormalDisplayColumns );

	PPrintf("MaxDisplayRow/Columns: 0x%lx/0x%lx (%ld/%ld)\n",
		GBase.MaxDisplayRow, GBase.MaxDisplayColumn,
		GBase.MaxDisplayRow, GBase.MaxDisplayColumn );

	ShowAddress ("Top of Display size stuff", 
	    CADDR(gbase, &GBase, &GBase.MaxDisplayRow));

	NewLine();
	PPrintf("SpriteReserved %lx\n", GBase.SpriteReserved);
	ShowAddress("LCM semaphore", GBase.LastChanceMemory );
	ShowAddress("ActiveView semaphore", GBase.ActiViewCprSemaphore );
	NewLine();

	PPrintf("BlitLock %ld\n", GBase.BlitLock);
	PPrintf("BlitNest %ld\n", GBase.BlitNest);
	PPrintf("BlitOwner %lx\n", GBase.BlitOwner);
	ShowAddress("BlitWaitQ", CADDR(gbase, &GBase, &GBase.BlitWaitQ));

	NewLine();
	ShowAddress("LCMem lock", CADDR(gbase,&GBase,&GBase.LastChanceMemory));
	PPrintf("LCMptr %lx\n", GBase.LCMptr);

	/* need address on amiga, not in sun buffer */
	ShowAddress ("Debug (byte)", CADDR(gbase, &GBase, &GBase.Debug));
	ShowAddress("TextFonts", CADDR(gbase,&GBase,&GBase.TextFonts));
	ShowAddress("DefaultFont", GBase.DefaultFont );
    }
    else
    {
	PPrintf("Couldn't find graphics.library.\n");
    }

    return( NULL );
}
