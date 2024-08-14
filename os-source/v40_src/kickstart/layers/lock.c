/******************************************************************************
*
*	$Id: lock.c,v 38.3 91/11/15 20:22:59 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/clip.h>
#include <graphics/layers.h>

#include "layersbase.h"

#define ObtainGfxSemaphoreList(x)	ObtainSemaphoreList(x)

/****** layers.library/LockLayers **********************************************
*
*    NAME
*	LockLayers -- lock all layers from graphics output.
*
*    SYNOPSIS
*	LockLayers( li )
*	            a0
*
*	void LockLayers( struct Layer_Info *);
*
*    FUNCTION
*	First calls LockLayerInfo()
*	Make all layers in this layer list locked.
*
*    INPUTS
*	li - pointer to Layer_Info structure
*
*    BUGS
*
*    SEE ALSO
*	LockLayer, LockLayerInfo, graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm locklayers(register __a0 struct Layer_Info *li)
{
	LockLayerInfo(li);
	li->LockLayersCount++;
	ObtainGfxSemaphoreList((struct List *)&li->gs_Head);
	if (li->LockLayersCount == 1)	pushusercliprects(li);
}

/****** layers.library/UnlockLayers ********************************************
*
*    NAME
*	UnlockLayers -- Unlock all layers from graphics output.
*	                Restart graphics output to layers that have been waiting
*
*    SYNOPSIS
*	UnlockLayers( li )
*	              a0
*
*	void UnlockLayers( struct Layer_Info *);
*
*    FUNCTION
*	Make all layers in this layer list unlocked.
*	Then call UnlockLayerInfo
*
*    INPUTS
*	li - pointer to the Layer_Info structure
*
*    BUGS
*
*    SEE ALSO
*	LockLayers, UnlockLayer, graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm unlocklayers(register __a0 struct Layer_Info *wi)
{
	wi->LockLayersCount--;
	if (wi->LockLayersCount == 0)	popusercliprects(wi);
	ReleaseSemaphoreList((struct List *)&wi->gs_Head);
	UnlockLayerInfo(wi);
}

/****** layers.library/LockLayer ***********************************************
*
*    NAME
*	LockLayer -- Lock layer to make changes to ClipRects.
*
*    SYNOPSIS
*	LockLayer( dummy, l )
*	           a0     a1
*
*	void LockLayer( LONG, struct Layer *);
*
*    FUNCTION
*	Make this layer unavailable for other tasks to use.
*	If another task is already using this layer then wait for
*	it to complete and then reserve the layer for your own use.
*	(this function does the same thing as graphics.library/LockLayerRom)
*
*	Note: if you wish to lock MORE THAN ONE layer at a time, you
*	    must call LockLayerInfo() before locking those layers and
*	    then call UnlockLayerInfo() when you have finished. This
*	    is to prevent system "deadlocks".
*
*	Further Note: while you hold the lock on a layer, Intuition will block
*	    on operations such as windowsizing, dragging, menus, and depth
*	    arranging windows in this layer's screen.  It is recommended that
*	    YOU do not make Intuition function calls while the layer is locked.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a layer
*
*    BUGS
*
*    SEE ALSO
*	UnlockLayer, LockLayerInfo, UnlockLayerInfo,
*	graphics.library/LockLayerRom, graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

/****** layers.library/UnlockLayer *********************************************
*
*    NAME
*	UnlockLayer -- Unlock layer and allow graphics routines to use it.
*
*    SYNOPSIS
*	UnlockLayer( l )
*	             a0
*
*	void UnlockLayer( struct Layer *);
*
*    FUNCTION
*	When finished changing the ClipRects or whatever you were
*	doing with this layer you must call UnlockLayer() to allow
*	other tasks to proceed with graphic output to the layer.
*
*    INPUTS
*	l - pointer to a layer
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

/****** layers.library/LockLayerInfo *******************************************
*
*    NAME
*	LockLayerInfo -- Lock the LayerInfo structure.
*
*    SYNOPSIS
*	LockLayerInfo( li )
*	               a0
*
*	void LockLayerInfo( struct Layer_Info *);
*
*    FUNCTION
*	Before doing an operation that requires the LayerInfo
*	structure, make sure that no other task is also using the
*	LayerInfo structure.  LockLayerInfo() returns when the
*	LayerInfo belongs to this task.  There should be
*	an UnlockLayerInfo for every LockLayerInfo.
*
*	Note: All layer routines presently LockLayerInfo() when they
*	start up and UnlockLayerInfo() as they exit.  Programmers
*	will need to use these Lock/Unlock routines if they wish
*	to do something with the LayerStructure that is not
*	supported by the layer library.
*
*    INPUTS
*	li - pointer to Layer_Info structure
*
*    BUGS
*
*    SEE ALSO
*	UnlockLayerInfo, graphics/layers.h
*
*******************************************************************************/

/****** layers.library/UnlockLayerInfo *****************************************
*
*    NAME
*	UnlockLayerInfo -- Unlock the LayerInfo structure.
*
*    SYNOPSIS
*	UnlockLayerInfo( li )
*	                 a0
*
*	void UnlockLayerInfo( struct Layer_Info *);
*
*    FUNCTION
*	After the operation is complete that required a LockLayerInfo,
*	unlock the LayerInfo structure so that  other tasks may
*	affect the layers.
*
*    INPUTS
*	li - pointer to the Layer_Info structure
*
*     BUGS
*
*     SEE ALSO
*	LockLayerInfo, graphics/layers.h
*
*******************************************************************************/
