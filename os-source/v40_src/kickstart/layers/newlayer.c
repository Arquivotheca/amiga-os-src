/******************************************************************************
*
*	$Id: newlayer.c,v 38.20 92/04/22 08:56:20 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

#include "layersbase.h"

void addrect(struct Rectangle *r,struct Layer *lp)
{
struct ClipRect *op;
struct Layer_Info *wi = lp->LayerInfo;

	for (op = wi->obs; op; op = op->Next)
	{
		/* compare both MinX/Y in one test: save space, time... bart */
		if( *((ULONG *)(&op->bounds.MinX)) == *((ULONG *)(&r->MinX)) ) return;
	}

	op = AllocCR(lp->LayerInfo);
	op->bounds = *r;
	op->lobs = lp;
	/* put on list */
	op->Next = wi->obs;
	wi->obs = op;
}

void __stdargs addpiece(struct Layer *lp,struct Rectangle *r,struct Layer_Info *wi)
{
	addrect(r,wi->check_lp);
}

void __stdargs myaddpiece(struct Layer *lp,struct Rectangle *r,struct Layer_Info *wi)
{
struct	ClipRect	*op;

	op = lp->SuperSaveClipRects;
	lp->SuperSaveClipRects = op->Next;
	op->BitMap = 0;
	op->lobs = 0;
	op->bounds = *r;
	/* put on list */
	op->Next = lp->SuperClipRect;
	lp->SuperClipRect = op;
}

/* flags&LAYERSIMPLE --> this window can handle its own refresh if uncovered */
/* flags&LAYERSUPER --> this window has another BitMap to use for SuperBitMap */

void newlayer(struct Layer *newlp)
{
struct Layer_Info *wi=newlp->LayerInfo;
struct LayerInfo_extra *old_lie=wi->LayerInfo_extra;
struct LayerInfo_extra new_lie;
struct ClipRect *crob;  /* obscured ClipRects */
struct Layer *lp;
struct ClipRect *cr_deletelist;
struct ClipRect **crp;  /* let's speed this baby up a bit... bart */

	wi->obs = NULL;

	wi->LayerInfo_extra = &new_lie;
	if (aborted(wi))
	{	/* list is now completely mangled */
		/* attempt to restore damaged cliprects */
	    	for (lp = wi->top_layer ; lp ; lp = lp->back)
		{
			lp->ClipRect = lp->_cliprects;
            for (crob = lp->_cliprects ; crob ; crob = crob->prev)
                crob->Next = crob->prev;

		}
		wi->LayerInfo_extra = old_lie;
		abort(wi);
	}

	/* make a copy of the original list for error recovery*/
	for (lp = wi->top_layer ; lp ; lp = lp->back)
	{
		lp->_cliprects = lp->ClipRect;	/* save head */
		for (crob = lp->ClipRect ; crob ; crob = crob->Next)
		{
			crob->prev = crob->Next;	/* copy state of list */
		}
	}

	cr_deletelist = 0;

	for (lp = wi->top_layer ; lp ; lp = lp->back)
	{
	    for (crp = &lp->ClipRect ; crob = *crp; )
	    {
		if (crob->lobs)
		{
		    if (rectXrect(&newlp->bounds,&crob->bounds) &&
		    addobs(crob,&crob->bounds,&newlp->bounds,lp) )
		    {
			    /* unlink op from lp->obs */
			    *crp = crob->Next;

			    /* attach Cliprect to single savecliprect list */
			    crob->Next = cr_deletelist;
			    cr_deletelist = crob;
			    continue;
		    }
		}
		crp = (struct ClipRect **)*crp;
	    }
	}

	/* now add rectangles ClipRects for newlp */
	newlp->ClipRect = wi->obs;
	for(lp = wi->top_layer; lp ; lp = lp->back)
	{
	   	wi->check_lp = lp;
	   	layerop(newlp,addpiece,&lp->bounds,wi);
	}

	newlp->ClipRect = wi->obs;

	/* allocate offscreen buffers */
	if (!(newlp->Flags & LAYERSIMPLE))
	{
	struct RastPort myRP;

		InitRastPort(&myRP);

		for (crob=newlp->ClipRect;crob;crob=crob->Next)
		{
	    		if ((crob->lobs) && (!(crob->BitMap)))
	    		{
				get_concealed_rasters(newlp,crob);

				/*
				 * This code is here for
				 * visual reasons only.  It does
				 * not change the way layers
				 * works other than remove a flash
				 * from the display.
				 */
				myRP.BitMap=crob->BitMap;
				SetRast(&myRP,0);
	    		}
		}
	}

	if ( newlp->Flags & LAYERSUPER ) gen_sbitmap_cr(newlp);

	cleanup(wi);
	wi->LayerInfo_extra = old_lie;	/* pop back */

	/* put new layer at bottom of list */
	insertlayer(0,newlp);
	/* put lock node in central list */
	AddTail((struct List *)&wi->gs_Head,&newlp->Lock);
	/*bottomlayer(newlp,0);*/
	/* past point of no return */

	if ( (newlp->Flags&LAYERBACKDROP) == 0)
	{
	struct Layer *cw;

		for(cw = wi->top_layer; cw != newlp ; cw = cw->back)
			/* place it above all backdrops */
			if (cw->Flags & LAYERBACKDROP)
			{
				if (!movelayerinfrontof(newlp,cw,FALSE))
				{
					/* have to delete this guy now */
					deletelayer2(newlp, 1);
					abort(wi);
				}
				break;
			}
	}

	regen_display(wi);	/* this will work */

	if (newlp->Flags & LAYERSUPER) CopySBitMap(newlp);

	/* release cliprects no longer needed */
	Freecrlist(cr_deletelist);
}

void gen_sbitmap_cr(struct Layer *newlp)
{
struct BitMap *bitmap2=newlp->SuperBitMap;
struct Rectangle rect;
struct Layer cw;
struct ClipRect *cr;
struct ClipRect *crtmp;

	rect.MinX = 0;
	rect.MinY = 0;

	rect.MaxX = GetBitMapAttr(bitmap2,BMA_WIDTH)-1;
	rect.MaxY = GetBitMapAttr(bitmap2,BMA_HEIGHT)-1;

	default_layer(newlp->LayerInfo,&cw,&rect);

	/* delete all old Virtual ClipRects */
	cr = newlp->SuperClipRect;
	while (cr)
	{
		crtmp = cr->Next;
		cr->Next = newlp->SuperSaveClipRects;
		newlp->SuperSaveClipRects = cr;
		cr = crtmp;
	}
	cw.SuperSaveClipRects = newlp->SuperSaveClipRects;
	cr = cw.SuperSaveClipRects;
	cw.SuperSaveClipRects = cr->Next;
	cw.SuperClipRect = 0;
	cr->Next = 0;
	cr->lobs = &cw;
	cw.ClipRect = cr;
	cr->bounds.MinX = newlp->Scroll_X;
	cr->bounds.MinY = newlp->Scroll_Y;
	cr->bounds.MaxX = newlp->Scroll_X
			    + newlp->bounds.MaxX - newlp->bounds.MinX;
	cr->bounds.MaxY = newlp->Scroll_Y
			    + newlp->bounds.MaxY - newlp->bounds.MinY;

	layerop(&cw,myaddpiece,&rect,0);
	newlp->SuperClipRect = cw.SuperClipRect;
	/* return cr to save list and reinstall list*/
	cr->Next = cw.SuperSaveClipRects;
	newlp->SuperSaveClipRects = cr;
}

/****** layers.library/CreateBehindHookLayer ***********************************
*
*    NAME                                                              (V36)
*	CreateBehindHookLayer -- Create a new layer behind all existing layers,
*	                         using supplied callback BackFill hook.
*
*    SYNOPSIS
*	result = CreateBehindHookLayer(li,bm,x0,y0,x1,y1,flags,hook,[,bm2])
*	d0                             a0 a1 d0 d1 d2 d3 d4    a3   [ a2 ]
*
*	struct Layer *CreateBehindHookLayer(struct Layer_Info *, struct BitMap *,
*	    LONG, LONG, LONG, LONG, LONG, struct Hook *, ... );
*
*    FUNCTION
*	Create a new Layer of position and size (x0,y0)->(x1,y1)
*	Make this layer of type found in flags.
*	Install Layer->BackFill callback Hook.
*	If SuperBitMap, use bm2 as pointer to real SuperBitMap,
*	and copy contents of Superbitmap into display layer.
*	If this layer is a backdrop layer then place it behind all
*	other layers including other backdrop layers. If this is
*	not a backdrop layer then place it behind all nonbackdrop
*	layers.
*
*	Note: when using SUPERBITMAP, you should also set LAYERSMART flag.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*	bm - pointer to common BitMap used by all Layers
*	x0,y0 - upper left hand corner of layer
*	x1,y1 - lower right hand corner of layer
*	flags - various types of layers supported as bit sets.
*	        (for bit definitions, see graphics/layers.h )
*	hook -  Layer->BackFill callback Hook (see InstallLayerHook())
*
*	        If hook is LAYERS_BACKFILL, the default backfill is
*	        used for the layer.  (Same as pre-2.0)
*
*	        As of V39:
*		If hook is LAYERS_NOBACKFILL, the layer will not be
*	        backfilled (NO-OP).
*
*	bm2 - pointer to optional Super BitMap
*
*    RESULTS
*	result - pointer to Layer structure if successful
*	         NULL if not successful
*
*    BUGS
*
*    SEE ALSO
*	InstallLayerHook, DeleteLayer, graphics/layers.h, graphics/clip.h,
*	graphics/gfx.h, utility/hooks.h
*
*******************************************************************************/

/****** layers.library/CreateBehindLayer ***************************************
*
*    NAME
*	CreateBehindLayer -- Create a new layer behind all existing layers.
*
*    SYNOPSIS
*	result = CreateBehindLayer(li,bm,x0,y0,x1,y1,flags [,bm2])
*	d0                         a0 a1 d0 d1 d2 d3   d4  [ a2 ]
*
*	struct Layer *CreateBehindLayer(struct Layer_Info *, struct BitMap *,
*	    LONG, LONG, LONG, LONG, LONG, ... );
*
*    FUNCTION
*	Create a new Layer of position and size (x0,y0)->(x1,y1)
*	Make this layer of type found in flags.
*	If SuperBitMap, use bm2 as pointer to real SuperBitMap,
*	and copy contents of Superbitmap into display layer.
*	If this layer is a backdrop layer then place it behind all
*	other layers including other backdrop layers. If this is
*	not a backdrop layer then place it behind all nonbackdrop
*	layers.
*
*	Note: when using SUPERBITMAP, you should also set LAYERSMART flag.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*	bm - pointer to common BitMap used by all Layers
*	x0,y0 - upper left hand corner of layer
*	x1,y1 - lower right hand corner of layer
*	flags - various types of layers supported as bit sets.
*	        (for bit definitions, see graphics/layers.h )
*	bm2 - pointer to optional Super BitMap
*
*    RESULTS
*	result - pointer to Layer structure if successful
*	         NULL if not successful
*
*    BUGS
*
*    SEE ALSO
*	DeleteLayer, graphics/layers.h, graphics/clip.h, graphics/gfx.h
*
*******************************************************************************/

/* there are too many arguments here now, we need to go to a structure */
struct Layer * __stdargs createbehindlayer(struct Layer_Info *wi,struct BitMap *bitmap,struct Rectangle bnds,long flags,struct BitMap *bitmap2,struct Hook *backfill)
{
struct Layer *newlp;
struct RastPort *rp;

	if (!fatten_lock(wi))	return(FALSE);

	if (!bitmap) return(FALSE);

	if (aborted(wi))
	{
		unlock_thin(wi);
		return(FALSE);
	}

	newlp = (struct Layer *)LayersAllocMem(sizeof(struct Layer),MEMF_CLEAR,wi);
	default_layer(wi,newlp,&bnds);

	/*
	 * Install backfill hook
	 */
	newlp->BackFill=backfill;

	if (flags & LAYERSUPER)
	{
	int i;
	struct ClipRect *cr;

		newlp->SuperBitMap = bitmap2;
		/* preallocate 5 ClipRects */
		for (i = 0; i < 5 ; i++)
		{
			cr = AllocCR(wi);
			cr->Next = newlp->SuperSaveClipRects;
			newlp->SuperSaveClipRects = cr;
		}
	}

	rp = (struct RastPort *)LayersAllocMem(sizeof(struct RastPort),MEMF_PUBLIC,wi);

	/* get damage region holder */
	if ( !(newlp->DamageList = NewRegion())
	||   !remember_to_free(wi,newlp->DamageList,LMN_REGION) )
	{
		if(newlp->DamageList) DisposeRegion(newlp->DamageList);
		abort(wi);
	};

	InitRastPort(rp);
	rp->BitMap = bitmap;
	newlp->rp = rp;
	rp->Layer = newlp;
	newlp->Flags = flags;

	newlayer(newlp);

	cleanup(wi);
	unlock_thin(wi);	/* also unlocklayer info*/
	if ((flags & LAYERSUPER)==0) FullBackFill(newlp);
					/* used to be SetRast(rp,0); */
	/* damage? we ain't got to show you no steenkeen' damage... */
	newlp->Flags &= ~(LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);

	WaitBlit();
	return(newlp);
}

/****** layers.library/CreateUpfrontHookLayer **********************************
*
*    NAME                                                              (V36)
*	CreateUpfrontHookLayer -- Create a new layer on top of existing layers,
*	                          using supplied callback BackFill hook.
*
*    SYNOPSIS
*	result = CreateUpfrontHookLayer(li,bm,x0,y0,x1,y1,flags,hook,[,bm2])
*	d0                              a0 a1 d0 d1 d2 d3   d4  a3   [ a2 ]
*
*	struct Layer *CreateUpfrontHookLayer(struct Layer_Info *, struct BitMap *,
*	    LONG, LONG, LONG, LONG, LONG, struct Hook *, ... );
*
*    FUNCTION
*	Create a new Layer of position and size (x0,y0)->(x1,y1)
*	and place it on top of all other layers.
*	Make this layer of type found in flags
*	Install Layer->BackFill callback hook.
*	if SuperBitMap, use bm2 as pointer to real SuperBitMap.
*	and copy contents of Superbitmap into display layer.
*
*	Note: when using SUPERBITMAP, you should also set LAYERSMART flag.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*	bm - pointer to common BitMap used by all Layers
*	x0,y0 - upper left hand corner of layer
*	x1,y1 - lower right hand corner of layer
*	flags - various types of layers supported as bit sets.
*	hook -  Layer->BackFill callback Hook (see InstallLayerHook())
*
*	        If hook is LAYERS_BACKFILL, the default backfill is
*	        used for the layer.  (Same as pre-2.0)
*
*	        As of V39:
*		If hook is LAYERS_NOBACKFILL, the layer will not be
*	        backfilled (NO-OP).
*
*	bm2 - pointer to optional Super BitMap
*
*    RESULTS
*	result - pointer to Layer structure if successful
*	         NULL if not successful
*
*    BUGS
*
*    SEE ALSO
*	InstallLayerHook, DeleteLayer, graphics/layers.h, graphics/clip.h,
*	graphics/gfx.h, utility/hooks.h
*
*******************************************************************************/

/****** layers.library/CreateUpfrontLayer **************************************
*
*    NAME
*	CreateUpfrontLayer -- Create a new layer on top of existing layers.
*
*    SYNOPSIS
*	result = CreateUpfrontLayer(li,bm,x0,y0,x1,y1,flags [,bm2])
*	d0                          a0 a1 d0 d1 d2 d3   d4  [ a2 ]
*
*	struct Layer *CreateUpfrontLayer(struct Layer_Info *, struct BitMap *,
*	    LONG, LONG, LONG, LONG, LONG, ... );
*
*    FUNCTION
*	Create a new Layer of position and size (x0,y0)->(x1,y1)
*	and place it on top of all other layers.
*	Make this layer of type found in flags
*	if SuperBitMap, use bm2 as pointer to real SuperBitMap.
*	and copy contents of Superbitmap into display layer.
*
*	Note: when using SUPERBITMAP, you should also set LAYERSMART flag.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*	bm - pointer to common BitMap used by all Layers
*	x0,y0 - upper left hand corner of layer
*	x1,y1 - lower right hand corner of layer
*	flags - various types of layers supported as bit sets.
*	bm2 - pointer to optional Super BitMap
*
*    RESULTS
*	result - pointer to Layer structure if successful
*	         NULL if not successful
*
*    BUGS
*
*    SEE ALSO
*       DeleteLayer, graphics/layers.h, graphics/clip.h, graphics/gfx.h
*
*******************************************************************************/

struct Layer * __stdargs createupfrontlayer(struct Layer_Info *wi,struct BitMap *bm,struct Rectangle bnds,long flags,struct BitMap *bitmap2,struct Hook *backfill)
{
struct Layer *cw;

	/* aborted does a lock screen */
	if (cw = createbehindlayer(wi,bm,bnds,flags,bitmap2,backfill))
	{
		if (upfront(cw))
		{
			ClearRegion(cw->DamageList);
			cw->Flags &= ~(LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
		}
		else
		{
			deletelayer(cw);
			cw = 0;
		}
	}
	return(cw);
}
