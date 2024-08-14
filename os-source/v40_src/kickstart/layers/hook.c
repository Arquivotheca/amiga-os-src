/******************************************************************************
*
*	$Id: hook.c,v 38.10 92/09/18 14:49:21 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <utility/hooks.h>
#include <graphics/clip.h>
#include <graphics/layers.h>

#include "layersbase.h"

/****** layers.library/InstallLayerHook ***************************************
*
*    NAME
*	InstallLayerHook -- safely install a new Layer->BackFill hook.    (V36)
*
*    SYNOPSIS
*	oldhook = InstallLayerHook( layer, hook )
*	d0                          a0     a1
*
*	struct Hook *InstallLayerHook( struct Layer *, struct Hook *);
*
*    FUNCTION
*	Installs a new Layer->Backfill Hook, waiting until it is safe to do
*	so. Locks the layer while substituting the new Hook and removing the
*	old one. If a new Hook is not provided, will install the default layer
*	BackFill Hook.
*
*    INPUTS
*	layer - pointer to the layer in which to install the Backfill Hook.
*	hook -  pointer to layer callback Hook which will be called
*	        with object == (struct RastPort *) result->RastPort
*	        and message == [ (Layer *) layer, (struct Rectangle) bounds,
*	                         (LONG) offsetx, (LONG) offsety ]
*
*	        This hook should fill the Rectangle in the RastPort
*	        with the BackFill pattern appropriate for offset x/y.
*
*	        If hook is LAYERS_BACKFILL, the default backfill is
*	        used for the layer.  (Same as pre-2.0)
*
*	        As of V39:
*		If hook is LAYERS_NOBACKFILL, the layer will not be
*	        backfilled (NO-OP).
*
*    RESULTS
*	oldhook - pointer to the Layer->BackFill Hook that was previously
*	          active.  Returns NULL if it was the default hook.
*		  In V39, it could return 1 if there was no hook.
*
*    EXAMPLE
*	The following hook is a very simple example that does rather little
*	but gives the basis idea of what is going on.
*
*	*
*	* This is the code called by the layer hook...
*	* Note that some other setup is required for this to work, including
*	* the definition of the PrivateData structure (pd_...) and the
*	* definition of the BitMapPattern structure (bmp_...)
*	*
*	CoolHook:	xdef	CoolHook
*			movem.l	d2-d7/a3-a6,-(sp)	; Save these...
*			move.l	h_SubEntry(a0),a4	; (my private data #1 here)
*			move.l	h_Data(a0),a5		; Put data into address reg
*	*
*	* Now, we do the rendering...
*	* Note that the layer may not be important...  But it is here...
*	*
*			move.l	(a1)+,a0		; Get the layer...
*	*
*	* a1 now points at the rectangle...
*	*
*			move.l	pd_GfxBase(a4),a6	; Point at GfxBase
*			move.l	bmp_Pattern(a5),d0	; Get PatternBitMap
*			beq	SimpleCase		; None?  Simple (0) case
*	*
*	* Now do the complex case of a pattern...
*	*
*			move.l	a1,a3			; Pointer to rectangle
*			addq.l	#8,a1			; Get past rectangle
*			move.l	(a1)+,d2		; X Offset (For pattern)
*			move.l	(a1)+,d3		; Y Offset
*		;
*		; Whatever complex blitting you would do in the complex case
*		; goes here
*		;
*	*
*	* No bitmap, so just do the simple (0) minterm case...
*	*
*	SimpleCase:	moveq.l	#0,d2			; Clear d2
*			move.w	ra_MinX(a1),d2		; Get X pos
*	*
*			moveq.l	#0,d3
*			move.w	ra_MinY(a1),d3		; Get Y pos
*	*
*			moveq.l	#0,d4
*			move.w	ra_MaxX(a1),d4
*			sub.l	d2,d4
*			addq.l	#1,d4			; Get X size
*	*
*			moveq.l	#0,d5
*			move.w	ra_MaxY(a1),d5
*			sub.l	d3,d5
*			addq.l	#1,d5			; Get Y size
*	*
*			move.l	d2,d0			; X Source
*			move.l	d3,d1			; Y Source
*			moveq.l	#0,d6			; NULL minterm
*			moveq.l	#-1,d7			; FF mask
*	*
*			move.l	rp_BitMap(a2),a1	; Get bitmap
*			move.l	a1,a0
*			CALLSYS	BltBitMap		; Do the backfill-0
*	*
*	HookDone:	movem.l	(sp)+,d2-d7/a3-a6	; Restore
*			rts
*
*    NOTES
*	The RastPort you are passed back is the same one passed to the
*	function.  You should *not* use "layered" rendering functions
*	on this RastPort.  Generally, you will wish to do BitMap operations
*	such as BltBitMap().  The callback is a raw, low-level rendering
*	call-back.  If you need to call a rendering operation with a
*	RastPort, make sure you use a copy of the RastPort and NULL the
*	Layer pointer.
*
*    BUGS
*
*    SEE ALSO
*	graphics/clip.h utility/hooks.h
*
*******************************************************************************/

struct Hook * __stdargs __asm install_backfill_hook(register __a0 struct Layer *layer,register __a1 struct Hook *hook)
{
struct Hook *old_hook = NULL;

	if (layer)
	{
		LockLayerRom(layer);

		old_hook=layer->BackFill;
		layer->BackFill = hook;

		UnlockLayerRom(layer);
	}

	return(old_hook);
}

/****** layers.library/InstallLayerInfoHook ***********************************
*
*    NAME
*	InstallLayerInfoHook - Install a backfill hook for non-layer      (V39)
*
*    SYNOPSIS
*	oldhook=InstallLayerInfoHook(li,hook)
*	d0                           a0 a1
*
*	struct Hook *InstallLayerInfoHook(struct Layer_Info *,struct Hook *);
*
*    FUNCTION
*	This function will install a backfill hook for the Layer_Info
*	structure passed.  This backfill hook will be used to clear the
*	background area where no layer exists.  The hook function is
*	passed the RastPort and the bounds just like the layer backfill
*	hook.  Note that this hook could be called for any layer.
*
*    INPUTS
*	li - pointer to LayerInfo structure
*
*	hook -  pointer to layer callback Hook which will be called
*	        with object == (struct RastPort *) result->RastPort
*	        and message == [ (ULONG) undefined, (struct Rectangle) bounds ]
*
*	        This hook should fill the Rectangle in the RastPort
*	        with the BackFill pattern appropriate for rectangle given.
*
*	        If hook is LAYERS_BACKFILL, the default backfill is
*	        used.  (Same as pre-2.0)
*
*		If hook is LAYERS_NOBACKFILL, there will be no
*	        backfill.  (NO-OP).
*
*    RESULTS
*	oldhook - Returns the backfill hook that was in the Layer_Info.
*	          Returns LAYERS_BACKFILL if the default was installed.
*	          Returns LAYERS_NOBACKFILL if there was a NO-OP hook.
*	          Returns -1 if there was some failure.
*
*    EXAMPLE
*	See the example in InstallLayerHook.  Note that both the Layer
*	pointer and the OffsetX/Y values are not available in the
*	LayerInfo backfill hook.
*
*    NOTES
*	When the hook is first installed, it is *NOT* called.  It is up
*	to the application to know if it is safe to fill in the area.
*	Since the hook will be called when a layer is deleted, the easiest
*	way to have layers call this hook is to create and delete a backdrop
*	layer that is the size of the area.
*
*	Also, note that currently the first long word of the hook message
*	contains an undefined value.  This value may look like a layer pointer.
*	It is *not* a layer pointer.
*
*	The RastPort you are passed back is the same one passed to the
*	function.  You should *not* use "layered" rendering functions
*	on this RastPort.  Generally, you will wish to do BitMap operations
*	such as BltBitMap().  The callback is a raw, low-level rendering
*	call-back.  If you need to call a rendering operation with a
*	RastPort, make sure you use a copy of the RastPort and NULL the
*	Layer pointer.
*
*    SEE ALSO
*	InstallLayerHook
*
*******************************************************************************/
struct Hook * __asm InstallLayerInfoHook(register __a0 struct Layer_Info *li,register __a1 struct Hook *hook)
{
struct Hook *oldHook;

	oldHook=(struct Hook *)(-1);

	if (fatten_lock(li))
	{
		oldHook=li->BlankHook;
		li->BlankHook=hook;

		unlock_thin(li);
	}
	return(oldHook);
}
