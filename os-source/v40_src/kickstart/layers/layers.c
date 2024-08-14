/******************************************************************************
*
*	$Id: layers.c,v 39.1 92/06/05 11:47:06 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <utility/hooks.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>
#include <hardware/blit.h>

#include "layersbase.h"

#define BUFFER_CLIPRECTS	8

/* Prototypes for functions in this file */
BOOL fattenlayerinfo(struct Layer_Info *);
BOOL xfattenlayerinfo(struct Layer_Info *);
void thinlayerinfo(struct Layer_Info *);
void xthinlayerinfo(struct Layer_Info *);

#define	clear_struct(p,n)	memset(p,0,n)

/****** layers.library/InitLayers **********************************************
*
*    NAME
*	InitLayers -- Initialize Layer_Info structure
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    SYNOPSIS
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*	InitLayers( li )
*	            a0
*
*	void InitLayers( struct Layer_Info *);
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    FUNCTION
*	Initialize Layer_Info structure in preparation to use
*	other layer operations on this list of layers.
*	Make the Layers unlocked (open), available to layer operations.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*
*    BUGS
*
*    SEE ALSO
*	NewLayerInfo, DisposeLayerInfo, graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm initlayers(register __a0 struct Layer_Info *li)
{
	clear_struct(li,sizeof(struct Layer_Info));
	li->FreeClipRects=NULL;
	NewList((struct List *)&(li->gs_Head));	/* used for storage */
	InitSemaphore(&(li->Lock));
	li->fatten_count = -1;
}

/****** layers.library/FattenLayerInfo *****************************************
*
*    NAME
*	FattenLayerInfo -- convert 1.0 LayerInfo to 1.1 LayerInfo
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    SYNOPSIS
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*	FattenLayerInfo( li )
*	                 a0
*
*	LONG FattenLayerInfo( struct Layer_Info *);
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    FUNCTION
*	V1.1 software and any later releases need to have more info in the
*	Layer_Info structure. To do this in a 1.0 supportable manner requires
*	allocation and deallocation of the memory whenever most
*	layer library functions are called.  To prevent unnecessary
*	allocation/deallocation FattenLayerInfo will preallocate the
*	necessary data structures and fake out the layer library into
*	thinking it has a LayerInfo gotten from NewLayerInfo.
*	NewLayerInfo is the approved method for getting this structure.
*	When a program needs to give up the LayerInfo structure it
*	must call ThinLayerInfo before freeing the memory. ThinLayerInfo
*	is not necessary if New/DisposeLayerInfo are used however.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*
*    BUGS
*
*    SEE ALSO
*
*	NewLayerInfo, ThinLayerInfo, DisposeLayerInfo, graphics/layers.h
*
*******************************************************************************/

BOOL __stdargs __asm extern_fattenlayerinfo(register __a0 struct Layer_Info *li)
{
	if (xfattenlayerinfo(li))
	{
		li->Flags |= NEWLAYERINFO_CALLED;
		return(TRUE);
	}
	return(FALSE);
}

/****** layers.library/ThinLayerInfo *******************************************
*
*    NAME
*	ThinLayerInfo -- convert 1.1 LayerInfo to 1.0 LayerInfo.
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    SYNOPSIS
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*	ThinLayerInfo( li )
*	               a0
*
*	void ThinLayerInfo( struct Layer_Info *);
*	OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE
*
*    FUNCTION
*	return the extra memory needed that was allocated with
*	FattenLayerInfo. This is must be done prior to freeing
*	the Layer_Info structure itself. V1.1 software should be
*	using DisposeLayerInfo.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*
*    BUGS
*
*    SEE ALSO
*	DisposeLayerInfo, FattenLayerInfo, graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm extern_thinlayerinfo(register __a0 struct Layer_Info *li)
{
	xthinlayerinfo(li);
	li->Flags &= ~NEWLAYERINFO_CALLED;
}

/****** layers.library/NewLayerInfo ********************************************
*
*    NAME
*	NewLayerInfo -- Allocate and Initialize full Layer_Info structure.
*
*    SYNOPSIS
*	result = NewLayerInfo()
*	d0
*
*	struct Layer_Info *NewLayerInfo( void );
*
*    FUNCTION
*	Allocate memory required for full Layer_Info structure.
*	Initialize Layer_Info structure in preparation to use
*	other layer operations on this list of layers.
*	Make the Layer_Info unlocked (open).
*
*    INPUTS
*	None
*
*    RESULT
*	result- pointer to Layer_Info structure if successful
*	        NULL if not enough memory
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h
*
*******************************************************************************/

struct Layer_Info * __stdargs NewLayerInfo(void)
{
	struct Layer_Info *li;
	if (li = AllocMem(sizeof(struct Layer_Info),MEMF_PUBLIC))
	{
		initlayers(li);
		if (!xfattenlayerinfo(li))
		{
			FreeMem(li,sizeof(struct Layer_Info));
			return(FALSE);
		}
		li->Flags |= NEWLAYERINFO_CALLED;
	}
	return(li);
}

/****** layers.library/DisposeLayerInfo ****************************************
*
*    NAME
*	DisposeLayerInfo -- Return all memory for LayerInfo to memory pool
*
*    SYNOPSIS
*	DisposeLayerInfo( li )
*	                  a0
*
*	void DisposeLayerInfo( struct Layer_Info *);
*
*    FUNCTION
*	return LayerInfo and any other memory attached to this LayerInfo
*	to memory allocator.
*
*	Note: if you wish to delete the layers associated with this Layer_Info
*	    structure, remember to call DeleteLayer() for each of the layers
*	    before calling DisposeLayerInfo().
*
*    INPUTS
*	li - pointer to LayerInfo structure
*
*    EXAMPLE
*
*	-- delete the layers associated this Layer_Info structure --
*
*	DeleteLayer(li,simple_layer);
*	DeleteLayer(li,smart_layer);
*
*	-- see documentation on DeleteLayer about deleting SuperBitMap layers --
*	my_super_bitmap_ptr = super_layer->SuperBitMap;
*	DeleteLayer(li,super_layer);
*
*	-- now dispose of the Layer_Info structure itself --
*	DisposeLayerInfo(li);
*
*
*    BUGS
*
*    SEE ALSO
*	DeleteLayer, graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm disposelayerinfo(register __a0 struct Layer_Info *li)
{
	xthinlayerinfo(li);
	FreeMem(li,sizeof(struct Layer_Info));
}

BOOL fatten_lock(struct Layer_Info *li)
{
	if (!fattenlayerinfo(li))	return (FALSE);
	locklayers(li);
	return(TRUE);
}

void unlock_thin(struct Layer_Info *li)
{
	unlocklayers(li);
	thinlayerinfo(li);
}

BOOL fattenlayerinfo(struct Layer_Info *li)
{
	LockLayerInfo(li);
	if ( (li->Flags & NEWLAYERINFO_CALLED) || (xfattenlayerinfo(li)) ) return(TRUE);
	UnlockLayerInfo(li);
	return(FALSE);
}

BOOL xfattenlayerinfo(struct Layer_Info *li)
{
struct LayerInfo_extra *lie;

	if (++li->fatten_count != 0)	return(TRUE);
	lie = li->LayerInfo_extra = AllocMem(sizeof(struct LayerInfo_extra),MEMF_PUBLIC|MEMF_CLEAR);
	if (lie)
	{
		NewList((struct List *)&(lie->mem));
		return(TRUE);
	}
	return(FALSE);
}

void thinlayerinfo(struct Layer_Info *li)
{
	if (0==(li->Flags & NEWLAYERINFO_CALLED)) xthinlayerinfo(li);
	UnlockLayerInfo(li);
}

void xthinlayerinfo(struct Layer_Info *li)
{
struct LayerInfo_extra *lie;

	if (--li->fatten_count < 0)
	{
		if (lie=li->LayerInfo_extra) FreeMem(lie,sizeof(struct LayerInfo_extra));
		li->LayerInfo_extra = 0;
	}
}

void default_layer(struct Layer_Info *li,struct Layer *l,struct Rectangle *r)
{
int i;

	/* all fields are assumed to already by zero */
	l->bounds = *r;
	l->Width = r->MaxX - r->MinX + 1;
	l->Height = r->MaxY - r->MinY +1;
	l->DamageList = 0;
	l->LayerInfo = li;
	InitSemaphore(&l->Lock);
	for (i = 0; i<li->LockLayersCount ; i++) LockLayerRom(l);
}

/* and call r_split */
/* generate additional list of ClipRects for the Display */
BOOL genrect(struct Layer *l)
{
	struct ClipRect *cr;

	if (!(cr = newAllocCR(l)))
	{
		return(FALSE);
	}

	cr->bounds = l->bounds;
	/* insert at head of list */
	cr->Next = l->ClipRect;
    	l->ClipRect = cr;

	return(r_split(l,l->front,cr));
}

void __stdargs __asm strip_onscreen_cliprects(register __a0 struct Layer_Info *li)
{
struct Layer *l;
struct ClipRect *cr, *crtmp;

	for (l = li->top_layer; l ; l = l->back)
	{
		l->_cliprects = NULL;

		/* remove all onscreen cliprects */
		for (cr = l->ClipRect; cr ; cr = crtmp)
		{
			crtmp = cr->Next;
			if (cr->lobs == 0)
			{
				Store_CR(l, cr);
				newDeleteCR(l, cr);
			}
    		}
	}
}

void __stdargs __asm gen_onscreen_cliprects(register __a0 struct Layer_Info *li)
{
struct Layer *l;
struct ClipRect *cr, *crtmp;
BOOL status;

	status=TRUE;
	for (l=li->top_layer;l;l=l->back) status &= genrect(l);

	if (!status)
	{
		/* we get here if genrect() failed [no memory] */

		/* remove partial results */
		for (l=li->top_layer;l;l=l->back)
		{
			/* remove all onscreen cliprects */
			for (cr=l->ClipRect;cr;cr=crtmp)
			{
				crtmp = cr->Next;
				if (!(cr->lobs)) newDeleteCR(l,cr);
			}
		}

		for (l=li->top_layer;l;l=l->back)
		{
			while (cr=UnStore_CR(l)) linkcr(l, cr);
		}
	}

	/* free any leftover cliprects */
	Freecrlist(li->FreeClipRects);
	li->FreeClipRects=NULL;
}


void regen_display(struct Layer_Info *wi)
{
	strip_onscreen_cliprects(wi);
	gen_onscreen_cliprects(wi);
}
