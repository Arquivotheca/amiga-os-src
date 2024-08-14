/******************************************************************************
*
*   $Id: tomiddle.c,v 38.16 92/04/14 16:39:30 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/layers.h>

#include "layersbase.h"

/****** layers.library/MoveLayerInFrontOf **************************************
*
*    NAME
*	MoveLayerInFrontOf -- Put layer in front of another layer.
*
*    SYNOPSIS
*	result = MoveLayerInFrontOf( layertomove, targetlayer )
*                                    a0           a1
*
*	LONG MoveLayerInFrontOf( struct Layer *, struct Layer *);
*
*    FUNCTION
*	Move this layer in front of target layer, swapping bits
*	in and out of the display with other layers.
*	If this is a refresh layer then collect damage list and
*	set the LAYERREFRESH bit in layer->Flags if redraw required.
*
*	Note: this operation may generate refresh events in other layers
*	    associated with this layer's Layer_Info structure.
*
*    INPUTS
*	layertomove - pointer to layer which should be moved
*	targetlayer - pointer to target layer in front of which to move layer
*
*    RESULTS
*	result = TRUE    if operation successful
*	         FALSE   if operation unsuccessful (probably out of memory)
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h
*
*******************************************************************************/

long __stdargs __asm extern_movelayerinfrontof(register __a0 struct Layer *l,register __a1 struct Layer *target)
{
struct Layer_Info *li = l->LayerInfo;
long status = FALSE;

	if (fatten_lock(li))
	{
		if (movelayerinfrontof(l,target,FALSE))
		{
			regen_display(li);
			status = TRUE;
		}
		unlock_thin(li);
	}
	return(status);
}

/*
 * MKS:
 *
 * This is the new method to do movelayerinfrontof()
 * What I did was to start all over with a clean slate.
 * Since I know that the cliprects are complete
 * vertical cuts, some tricks can be used.  I will
 * also use the fact that they are sorted to make
 * use of faster search routines (or at least try to)
 *
 * Method:
 * First, find where the layer needs to be inserted...
 * This is done by setting layer priorities to all of
 * the layers and matching the priorities to the target.
 *
 * Next,...
 */
BOOL movelayerinfrontof(struct Layer *l,struct Layer *target,long flag)
{
struct Layer_Info *li=l->LayerInfo;
struct Layer *layer;
struct Layer *layer2;
struct ClipRect *cr;
struct ClipRect *cr2;
struct ClipRect *prealloc_cr=NULL;
struct ClipRect *stack_list=NULL;
struct LayerInfo_extra *old_lie;
struct LayerInfo_extra new_lie;
struct Rectangle damage;
ULONG l_flag;
ULONG layer_flag;
UWORD priority;

	if ((l->back!=target) && (l!=target)) /* not already there? */
	{
		/*
		 * Pre-generate a flag that will be used in a few loops...
		 */
		l_flag=(l->Flags & LAYERSIMPLE);

		/*
		 * First, we must set the priority value for each layer
		 * unless the target layer is NULL.  In this case, we
		 * will not need any priority settings other than the
		 * two below to indicate that we are moving to the back.
		 */
		priority=1;
		l->priority=0;
		if (target)
		{
			for (layer=li->top_layer;layer;layer=layer->back) layer->priority=priority++;

			/*
			 * Now, priority is past the last layer
			 * however, if we have a target layer,
			 * we get the priority of that layer as
			 * our target.  Our target is to have
			 * l->back->priority==priority or, in the
			 * case of no target, have layer->back==NULL
			 */
			priority=target->priority;
		}

		/*
		 * Set up the memory failure abort system
		 * before we continue...
		 */
		old_lie = li->LayerInfo_extra;
		li->LayerInfo_extra = &new_lie;
		if (aborted(li))
		{
			li->LayerInfo_extra = old_lie;

			/* actually only have to do this to obscured cr's */
			for (l=li->top_layer;l;l = l->back) if (l->Flags & LAYERSIMPLE) for (cr=l->ClipRect;cr;cr=cr->Next)
			{
				cr->BitMap = NULL;
				cr->Flags &= ~(CR_NEEDS_NO_LAYERBLIT_DAMAGE);
			}

			/***********************************/
			/**  WARNING!!!  Early RETURN!!!  **/
			/***********************************/
			return(FALSE);
		}

		/*
		 * So, we "know" where we need to go, but
		 * we also need to know how much memory we
		 * will need.  In fact, what we will do is
		 * allocate the cliprects/bitmaps that are
		 * needed for this operation to work.
		 * While we are doing this, we will also
		 * link the cliprects as needed to make for
		 * a much faster operation when we complete
		 * the examination.  Since the _p1 and _p2
		 * fields of the cliprect structure are
		 * "unused" we will use these to link together
		 * the cliprects.  _p1 will be used to point
		 * to the next cliprect below this one
		 * and _p2 will be used to point to the
		 * cliprect's layer.  (So we can find it)
		 */
		if (l->priority > priority)
		{
			/*
			 * Ok, so we are moving up in the layers...
			 * This means that we only need to deal with
			 * cliprect stacks that have our cliprect in it
			 * and are obscured by a layer that is now going
			 * to be obscured...
			 */
			for (cr2=(struct ClipRect *)&(l->ClipRect);cr=cr2->Next;cr2=cr2->Next) if (layer=cr->lobs) if (layer->priority >= priority)
			{
				/*
				 * Ok, so this cliprect is a match...  Lets
				 * build the VLink chain...
				 */
				cr->reserved=(LONG)stack_list;
				stack_list=cr;
				cr->prev=cr2;

				cr->_p1=NULL;
				cr->_p2=(void *)layer;
				vlink_cr(layer,cr,l);

				/*
				 * Now, check to see if we need to allocate some
				 * bitmaps:  If we are SIMPLE and the old obscuring
				 * layer is SMART, we will need to...
				 */
				if ((l_flag) && (!(layer->Flags & LAYERSIMPLE)))
				{
					get_concealed_rasters(l,cr);

					/*
					 * If we are MOVESIZE flag, we will also need this
					 */
					if (flag)
					{
						/*
						 * Now, also get a cliprect put together for later work...
						 */
						cr=AllocCR(li);
						cr->Next=prealloc_cr;
						prealloc_cr=cr;
					}
				}
			}
		}
		else
		{
			/*
			 * Ok, so we are going to go down... Clear the link status
			 * of the cliprects I will be looking at...
			 */
			for (layer=l->back;layer!=target;layer=layer->back)
			{
				for (cr=layer->ClipRect;cr;cr=cr->Next) cr->_p2=NULL;
			}

			/*
			 * Ok, so we are going down in the layer stack.
			 * This means that we will need to look at all of the layers
			 * starting with the one behind me and going to the target...
			 */
			for (layer=l->back;layer!=target;layer=layer->back) if (rectXrect(&(l->bounds),&(layer->bounds)))
			{
				layer_flag=(layer->Flags & LAYERSIMPLE);	/* Pregenerate the SIMPLE flag */

				/*
				 * Now, for all of the cliprects that are obscured by me...
				 * (We can skip ones that are already linked...
				 */
				for (cr2=(struct ClipRect *)&(layer->ClipRect);cr=cr2->Next;cr2=cr2->Next) if ((l==cr->lobs) && (!(cr->_p2)))
				{
					/*
					 * Well, we don't have this one linked yet so
					 * build the link chain...
					 */
					cr->reserved=(LONG)stack_list;
					stack_list=cr;
					cr->prev=cr2;

					cr->_p1=NULL;
					cr->_p2=(void *)layer;
					vlink_cr(layer,cr,NULL);

					/*
					 * Ok, so do we need to get some backing store?
					 */
					if ((!l_flag) && (layer_flag)) get_concealed_rasters(layer,cr);
				}
			}
		}

		/*
		 * Well, all the memory is here, now
		 * continue with the work after we uninstall
		 * the memory failure system...
		 */
		cleanup(li);
		li->LayerInfo_extra = old_lie;

		/*
		 * Ok, so, we now have a list of cliprects
		 * that point to cliprect stack that will change due
		 * to this great new layer position.  So, lets move
		 * the sucker...
		 */
		while (cr=stack_list)
		{
			stack_list=(struct ClipRect *)(cr->reserved);

			/*
			 * Now, if the obscuring layer is me, then the one
			 * that belongs to this cliprect will become obscuring
			 * if it is not me, then I will become obscuring.  These
			 * are the only two options at this point...
			 */
			cr2=cr;
			layer=cr2->lobs;
			layer2=l;
			if (l==layer) layer2=(struct Layer *)cr2->_p2;

			/*
			 * First we should fix up all of the clip rects to have
			 * the new Obscuring layer pointer...
			 */
			for (;cr;cr=(struct ClipRect *)(cr->_p1)) cr->lobs=layer2;

			/*
			 * At this point we have:
			 *
			 * cr2    == the cliprect of the layer that is going to be top...
			 * layer  == the layer that is going to be obscured...
			 * layer2 == the layer that is going to be on top...
			 *
			 * Now, update the screen as needed...
			 *
			 * If the layer coming to top is SIMPLE...
			 */
			if (layer2->Flags & LAYERSIMPLE)
			{
				if (layer->Flags & LAYERSIMPLE)
				{
					/*
					 * Check if MOVESIZE and set the information as needed
					 */
					if ((target==layer) && (flag)) cr2->Flags |= CR_NEEDS_NO_LAYERBLIT_DAMAGE;
				}
				else
				{
					/*
					 * If the layer on top is smart, we need to
					 * grab what is on the screen...
					 */
					screentocr(layer2->rp,cr2);

					/*
					 * So, we are in MOVESIZE and the
					 * layer will be simple...  We need a cliprect...
					 */
					if (flag)
					{
						cr=prealloc_cr;
						prealloc_cr=cr->Next;

						cr->bounds=cr2->bounds;
						if (cr->Next=cr2->Next) cr->Next->prev=cr;	/* Make sure pointers are ok... */
						cr2->Next=cr;
					}
				}

				/*
				 * The new top layer is simple, so it will do
				 * its own refresh once I let it...
				 *
				 * I will not do this if we have the MOVESIZE flag set...
				 */
				if (!flag)
				{
					damage.MinX=cr2->bounds.MinX - layer2->bounds.MinX;
					damage.MinY=cr2->bounds.MinY - layer2->bounds.MinY;
					damage.MaxX=cr2->bounds.MaxX - layer2->bounds.MinX;
					damage.MaxY=cr2->bounds.MaxY - layer2->bounds.MinY;

					CallBackFillHook(layer2,cr2);

					OrRectRegion(layer2->DamageList,&damage);
					layer2->Flags|=(LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
				}
			}
			else
			{
				if (layer->Flags & LAYERSIMPLE)
				{
					/*
					 * Layer on top is simple and
					 * layer on bottom is smart...
					 */
					crtoscreen(layer2->rp,cr2);

					/*
					 * Make sure we WaitBlit before we
					 * free the rasters...
					 */
					WaitBlit();
					free_concealed_rasters(cr2);
				}
				else
				{
					/*
					 * Just swap the screen with the
					 * layer's cliprect...
					 */
					screenswap(layer2->rp,cr2);
				}
			}

			/*
			 * Now move the cliprect to the new layer
			 */
			cr=cr2->prev; if (cr->Next=cr2->Next) cr->Next->prev=cr;	/* Unlink... */

			cr=(struct ClipRect *)&(layer->ClipRect);
			cr2->prev=cr; if (cr2->Next=cr->Next) cr2->Next->prev=cr2;	/* Link... */
			cr->Next=cr2;
		}

		/*
		 * Ok, now all the hard stuff is done, just remove
		 * the layer and insert in the right spot.
		 */
		unlinklayer(l);
		insertlayer(target,l);
	}
	return(TRUE);
}
