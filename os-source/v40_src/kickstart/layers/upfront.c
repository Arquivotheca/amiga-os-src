/******************************************************************************
*
*	$Id: upfront.c,v 38.7 91/11/15 20:15:56 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

#include "layersbase.h"

/****** layers.library/UpfrontLayer ********************************************
*
*    NAME
*	UpfrontLayer -- Put layer in front of all other layers.
*
*    SYNOPSIS
*	result = UpfrontLayer( dummy, l )
*	d0                     a0     a1
*
*	LONG UpfrontLayer( LONG, struct Layer *);
*
*    FUNCTION
*	Move this layer to the most upfront position swapping bits
*	in and out of the display with other layers.
*	If this is a refresh layer then collect damage list and
*	set the LAYERREFRESH bit in layer->Flags if redraw required.
*	By clearing the BACKDROP bit in the layers Flags you may
*	bring a Backdrop layer up to the front of all other layers.
*
*	Note: this operation may generate refresh events in other layers
*	   associated with this layer's Layer_Info structure.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a nonbackdrop layer
*
*    RESULTS
*	result - TRUE   if operation successful
*	         FALSE  if operation unsuccessful (probably out of memory)
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h
*
*******************************************************************************/

BOOL upfrontcode(struct Layer *l)
{
	struct Layer *target;

	if (l->Flags & LAYERBACKDROP)
	{
		/* only go above other BACKDROPS */
		/* find first backdrop */
		for (target = l->LayerInfo->top_layer; target ; target = target->back)
			if (target->Flags & LAYERBACKDROP) break;
	}
	else	target = l->LayerInfo->top_layer;
	if (target == l)	return (TRUE);
	return(movelayerinfrontof(l,target,FALSE));
}

LONG __stdargs __asm upfront(register __a1 struct Layer *l)
{
struct Layer_Info *li=l->LayerInfo;
LONG status=FALSE;

	if (fatten_lock(li))
	{
		if (upfrontcode(l))
		{
			regen_display(li);
			status = TRUE;
		}
		unlock_thin(li);
	}
	return(status);
}
