/* showg.c : wack "show graphics/layers" functions.
 * $Id: showg.c,v 1.7 91/04/24 20:50:37 peter Exp $
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

extern APTR CurrAddr;
extern APTR SpareAddr;


ULONG 
dumpViewPort( addr, vp)
APTR addr;
struct ViewPort *vp;
{
    ShowAddress("ViewPort", addr);

    ShowAddress("Next", vp->Next);
    ShowAddress("ColorMap", vp->ColorMap);
    ShowAddress("DspIns", vp->DspIns);
    ShowAddress("SprIns", vp->SprIns);
    ShowAddress("UCopIns", vp->UCopIns);
    printf("DWidth/Height\t0x%x/0x%x\t(%d/%d)\n",
	vp->DWidth, vp->DHeight, vp->DWidth, vp->DHeight);
    printf("Dx/DyOffsets\t0x%x/0x%x\t(%d/%d)\n",
	vp->DxOffset, vp->DyOffset, vp->DxOffset, vp->DyOffset);
    printf("Modes\t\t%x\n", vp->Modes);
    printf("Sprite priorities 0x%x, EModes %x\n", vp->SpritePriorities,
	vp->ExtendedModes);
    ShowAddress("RasInfo", vp->RasInfo);

    return (0);
}

SHOWSIMPLE(ViewPort)

ULONG 
dumpView( addr, v)
APTR addr;
struct View *v;
{
    ShowAddress("View", addr);

    ShowAddress("ViewPort", v->ViewPort);
    ShowAddress("LOFCprList", v->LOFCprList);
    ShowAddress("SHFCprList", v->SHFCprList);
    printf("Dx/DyOffsets\t0x%x/0x%x\t(%d/%d)\n",
	v->DxOffset, v->DyOffset, v->DxOffset, v->DyOffset);
    printf("Modes\t\t%x\n", v->Modes);

    return (0);
}

SHOWONE(View)

ULONG 
dumpViewExtra( addr, ve)
APTR addr;
struct ViewExtra *ve;
{
    ShowAddress("ViewExtra", addr);

    ShowAddress("View (backlink)", ve->View);
    ShowAddress("MonitorSpec", ve->Monitor);
#if 0
    printf("VideoFlags: %x\n", ve->ve_VideoFlags );
#endif

    return (0);
}

SHOWONE(ViewExtra)

ULONG
dumpLayer_Info(addr, li)
APTR addr;
struct Layer_Info *li;
{
    ShowAddress("Layer_Info", addr);
    printf("Flags %x\n", li->Flags);
    ShowAddress("top_layer", li->top_layer);
    ShowAddress("layer info lock", CADDR(addr, li, &li->Lock));
    ShowAddress("semaphore list gs_Head", CADDR(addr, li, &li->gs_Head));
    return (0);
}

SHOWONE(Layer_Info)

ULONG
dumpLayer(addr, l)
APTR addr;
struct Layer *l;
{
    ShowAddress("Layer", addr);
    printf("\tbounds: %d %d, %d, %d\n",
	l->bounds.MinX, l->bounds.MinY, l->bounds.MaxX, l->bounds.MaxY);
    printf("\tFlags %lx\n", l->Flags);
    ShowAddress("\tWindow", l->Window);
    ShowAddress("\tLock", CADDR(addr, l, &l->Lock));
    ShowAddress("\tdamage list", l->DamageList);
    printf("\n");
}

SHOWONE(Layer)


ShowGfxBase()
{
    APTR gbase;			/* amiga address of graphics base */
    struct GfxBase GBase;	/* sun buffer for base */

    if (gbase =  FindBase ("graphics.library"))
    {
	SpareAddr = gbase;
	/* fill sun bugger with data from amiga */
	GetBlock(gbase, &GBase, sizeof GBase);

	/* dump out some fields */
	Print (" at %08lx\n", gbase);
	Print ("Modes %lx Flags %lx\n", GBase.Modes, GBase.Flags);
	printf("DisplayFlags %lx\n", GBase.DisplayFlags);

	printf("\n");
	ShowAddress ("ActiView", GBase.ActiView);
	ShowAddress ("LOFlist", GBase.LOFlist);
	ShowAddress ("SHFlist", GBase.SHFlist);
	printf("NormalDisplayRows/Colums: 0x%x/0x%x (%d/%d)\n",
		GBase.NormalDisplayRows, GBase.NormalDisplayColumns,
		GBase.NormalDisplayRows, GBase.NormalDisplayColumns );

	printf("MaxDisplayRow/Colums: 0x%x/0x%x (%d/%d)\n",
		GBase.MaxDisplayRow, GBase.MaxDisplayColumn,
		GBase.MaxDisplayRow, GBase.MaxDisplayColumn );

	ShowAddress ("Top of Display size stuff", 
	    CADDR(gbase, &GBase, &GBase.MaxDisplayRow));

	printf("\n");
	printf("SpriteReserved %x\n", GBase.SpriteReserved);
	ShowAddress("LCM semaphore", GBase.LastChanceMemory );
	ShowAddress("ActiveView semaphore", GBase.ActiViewCprSemaphore );

	printf("\n");

	printf("BlitLock %d\n", GBase.BlitLock);
	printf("BlitNest %d\n", GBase.BlitNest);
	printf("BlitOwner %lx\n", GBase.BlitOwner);
	ShowAddress("BlitWaitQ", CADDR(gbase, &GBase, &GBase.BlitWaitQ));

	printf("\n");
	ShowAddress("LCMem lock", CADDR(gbase,&GBase,&GBase.LastChanceMemory));
	printf("LCMptr %lx\n", GBase.LCMptr);

	/* need address on amiga, not in sun buffer */
	ShowAddress ("Debug (byte)", CADDR(gbase, &GBase, &GBase.Debug));
	ShowAddress("TextFonts", CADDR(gbase,&GBase,&GBase.TextFonts));
	ShowAddress("DefaultFont", GBase.DefaultFont );

    }
    else
    {
	puts("\ncouldn't find graphics.library in exec library list.");
    }
}

