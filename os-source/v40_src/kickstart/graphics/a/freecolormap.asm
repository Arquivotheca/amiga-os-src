*******************************************************************************
*
*	$Id: freecolormap.asm,v 38.3 92/07/01 10:19:58 chrisg Exp $
*
*******************************************************************************

	section	graphics
    xdef    _FreeColorMap
******* graphics.library/FreeColorMap ****************************************
*
*   NAME
*       FreeColorMap -- Free the ColorMap structure and return memory
*						to free memory pool.
*
*   SYNOPSIS
*       FreeColorMap( colormap )
*                       a0
*
*	void FreeColorMap(struct ColorMap *);
*
*   FUNCTION
*	Return the memory to the free memory pool that was allocated
*	with GetColorMap.
*
*   INPUTS
*	colormap - pointer to ColorMap allocated with GetColorMap.
*		
*		  Passing a NULL pointer (meaning "do nothing") is 
*		  acceptable (V39).
*
*   RESULT
*	The space is made available for others to use.
*
*   BUGS
*
*   SEE ALSO
*       SetRGB4() GetColorMap() graphics/view.h
******************************************************************************

	include 'exec/types.i'
	include '/view.i'
	include	'graphics/gfxbase.i'

	xref	_LVOFreeMem
	xref	get_genlock_byte_count

_FreeColorMap:
*	We now check for ptrs being there first before
*	freeing the memory. This is needed to GetColorMap
*	can use FreeColorMap for cleanup
	cmp.w	#0,a0
	beq.s	an_rts
	movem.l	a2/a6,-(sp)		* save my gfxbase
	move.l	gb_ExecBase(a6),a6	* get execbase

	move.l  a0,a2
	move.l	cm_ColorTable(a0),d1
	if <>
		moveq	#0,d0
		move.w	cm_Count(a0),d0
		add.l	d0,d0		* 1 word per entry
		move.l	d1,a1
		jsr	_LVOFreeMem(a6)	* free off colortable
	endif

	move.l	cm_LowColorBits(a2),d1
	if <>
		moveq	#0,d0
		move.w	cm_Count(a2),d0
		add.l	d0,d0		* 1 word per entry
		move.l	d1,a1
		jsr	_LVOFreeMem(a6)	* free off low colortable
	endif

	move.l	cm_PalExtra(a2),d1
	if	<>
		move.l	d1,a1
		moveq	#0,d0
		move.w	cm_Count(a2),d0
		move.l	d0,d1
		add.l	d0,d0	;*2
		add.l	d1,d0	; *3
		add.l	#pe_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
	endif

	lea	-4(a2),a1		* get back pointer to colormap structure
	moveq	#cm_SIZEOF+4,d0
	jsr	_LVOFreeMem(a6)	* free off colormap structure
	movem.l	(sp)+,a2/a6	* restore GfxBase
an_rts:
	rts

	end
