/******************************************************************************
*
*	$Id: behind.c,v 38.2 91/08/02 10:19:10 mks Exp $
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
#include <graphics/gfxbase.h>

#include "layersbase.h"

/****** layers.library/BehindLayer *********************************************
*
*    NAME
*	BehindLayer -- Put layer behind other layers.
*
*    SYNOPSIS
*	result = BehindLayer( dummy, l )
*	d0                    a0     a1
*
*	LONG BehindLayer( LONG, struct Layer *);
*
*    FUNCTION
*	Move this layer to the most behind position swapping bits
*	in and out of the display with other layers.
*	If other layers are REFRESH then collect their damage lists and
*	set the LAYERREFRESH bit in the Flags fields of those layers that
*	may be revealed.  If this layer is a backdrop layer then
*	put this layer behind all other backdrop layers.
*	If this layer is NOT a backdrop layer then put in front of the
*	top backdrop layer and behind all other layers.
*
*	Note: this operation may generate refresh events in other layers
*	   associated with this layer's Layer_Info structure.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a layer
*
*    RESULTS
*	result - TRUE    if operation successful
*	         FALSE   if operation unsuccessful (probably out of memory)
*
*    BUGS
*
*    SEE ALSO
*       graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

BOOL behindcode(struct Layer *l,UWORD stop)
{
struct Layer *p = 0;	/* assume behind everything */

	/* find the guy l will be placed in front of and call new routine */
	if (stop)
		for (p = l->back; p ; p = p->back)
		{
				if (p->Flags & stop)	break;
		}
	return(movelayerinfrontof(l,p,FALSE));
}

LONG __stdargs __asm behind(register __a1 struct Layer *l)
{
struct Layer_Info *li=l->LayerInfo;
LONG status = FALSE;

	if (fatten_lock(li))
	{
		if (behindcode(l,l->Flags & LAYERBACKDROP?0:LAYERBACKDROP))
		{
			regen_display(li);
			status = TRUE;
		}
	    	unlock_thin(li);
	}
	return(status);
}
