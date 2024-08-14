*******************************************************************************
*
*	$Id: whichlayer.asm,v 38.2 91/08/02 10:17:50 mks Exp $
*
*******************************************************************************


******* layers.library/WhichLayer *********************************************
*
*    NAME
*	WhichLayer -- Which Layer is this point in?
*
*    SYNOPSIS
*	layer = WhichLayer( li, x, y )
*	d0                  a0  d0 d1
*
*	struct Layer *WhichLayer(struct Layer_Info*, WORD, WORD);
*
*    FUNCTION
*	Starting at the topmost layer check to see if this point (x,y)
*	    occurs in this layer.  If it does return the pointer to this
*	    layer. Return NULL if there is no layer at this point.
*
*    INPUTS
*	li  = pointer to LayerInfo structure
*	(x,y) = coordinate in the BitMap
*
*    RESULTS
*	layer - pointer to the topmost layer that this point is in
*	        NULL if this point is not in a layer
*
*    SEE ALSO
*	graphics/layers.h
*
*******************************************************************************

*struct Layer *whichlayer(wi,x,y)
*struct Layer_Info *wi;
*SHORT x,y;
*{
*    struct Layer *nl;
*	/* search through layers and get first one that contains (x,y) */
*	for (nl = wi->top_layer; nl ; nl = nl->back)
*		if ( (nl->bounds.MinX <= x) &&
*			 (nl->bounds.MaxX >= x) &&
*			 (nl->bounds.MinY <= y) &&
*			 (nl->bounds.MaxY >= y) )	break;
*#ifdef DEBUG
*	printf("whichlayer(%lx,%ld,%ld)=%lx\n",wi,x,y,nl);
*#endif
*	return(nl);
*}
	include "exec/types.i"
	include "graphics/layers.i"
	include "graphics/clip.i"

	xdef	WhichLayer
WhichLayer:
	move.l	li_top_layer(a0),a0		* get first layer
	sub.l	a1,a1					* stupid motorola 68k
;
	bra.s	test
loop:	cmp.w	lr_MinX(a0),d0
	blt.s	next
	cmp.w	lr_MaxX(a0),d0
	bgt.s	next
	cmp.w	lr_MinY(a0),d1
	blt.s	next
	cmp.w	lr_MaxY(a0),d1
	ble.s	found
next:	move.l	lr_back(a0),a0
test:	cmp.l	a0,a1			; (Use a1 as NULL for test)
	bne.s	loop
found:	move.l	a0,d0
	rts

	end
