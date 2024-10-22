*******************************************************************************
*
*	$Id: loadrgb4.asm,v 38.5 92/10/22 08:52:02 chrisg Exp $
*
*******************************************************************************


	section	graphics
	include	'/view.i'
	include	'hardware/custom.i'

	xref	_custom
	xref	_pokecolors,_pokevp
	xdef	_LoadRGB4

******* graphics.library/LoadRGB4 ********************************************
*
*   NAME
*	LoadRGB4 -- Load RGB color values from table.
*
*   SYNOPSIS
*	LoadRGB4( vp, colors , count )
*                 a0  	a1     d0:16
*
*	void LoadRGB4( struct ViewPort *, UWORD *, WORD);
*
*   FUNCTION
*   	load the count words of the colormap from table starting at
*	entry 0.
*
*   INPUTS
*	vp - pointer to ViewPort, whose colors you wish to change
*	colors - pointer to table of RGB values set up as an array
*	         of USHORTS
*		 	background--  0x0RGB
*			color1	  --  0x0RGB
*			color2    --  0x0RGB
*			 etc.         UWORD per value.
*		The colors are interpreted as 15 = maximum intensity.
*		                              0 = minimum intensity.
*	count	= number of UWORDs in the table to load into the
*	  colormap starting at color 0(background) and proceeding
*	  to the next higher color number
*
*   RESULTS
*	The ViewPort should have a pointer to a valid ColorMap to store
*	the colors in.
*	Updates the hardware copperlist to reflect the new colors.
*	Updates the intermediate copperlist with the new colors.
*
*   BUGS
*
*	NOTE: Under V36 and up, it is not safe to call this function
*	from an interrupt, due to semaphore protection of graphics
*	copper lists.
*
*   SEE ALSO
*	SetRGB4() GetRGB4() GetColorMap() graphics/view.h
*
*********************************************************************
_LoadRGB4:
	tst.w	d0
	beq.s	5$
	swap	d0
	clr.w	d0
	swap	d0
	cmp.w	#0,a0				; stupid Eye of the beholder jerks
	bne.s	1$
4$:	move.l	#_custom+color,a0
	bra.s	2$
3$:	move.w	(a1)+,(a0)+
2$:	dbra	d0,3$
5$:	rts
1$:	tst.l	vp_ColorMap(a0)
	beq.s	no_cm_kludge
	movem.l	d2/a2/a3/a4,-(a7)
	move.l	a0,a2
	move.l	vp_ColorMap(a2),a0
	cmp.w	cm_Count(a0),d0
	bls.s	no_too_many
	move.w	cm_Count(a0),d0
no_too_many:
	move.l	cm_ColorTable(a0),a3
	move.l	a3,a4
	tst.b	cm_Type(a0)
	beq.s	no_low
	tst.l	cm_LowColorBits(a0)
	beq.s	no_low
	move.l	cm_LowColorBits(a0),a4
no_low:	subq	#1,d0
1$:	move.w	(a1)+,d1
	and.w	#$7fff,d1
	move.w	(a3),d2
	and.w	#$8000,d2
	or.w	d1,d2
	move.w	d2,(a3)+
	move.w	(a4),d2
	and.w	#$8000,d2
	or.w	d1,d2
	move.w	d2,(a4)+
	dbra	d0,1$
	bsr	_pokecolors
	movem.l	(a7)+,d2/a2/a3/a4
no_loadrgb4:
	rts

no_cm_kludge:
; load directly to copper lists - no colormap to store in!
	movem.l	a2/d2/d6/d7,-(a7)
	cmp.w	#32,d0
	bls.s	no_limit_regnum
	moveq	#32,d0
no_limit_regnum:
	move.l	a1,a2
	move	d0,d7
	move.w	#color,d6
	move.w	#$fff,d2	; mask (leave genlock bits alone)
no_cm_lp:
	move.w	(a2)+,d1	; rgb word
	move.w	d6,d0		; reg index
	bsr	_pokevp			; pokevp(a0=vp,d0=regnum, d1=value, d2=mask)
	add.w	#2,d6		; next color reg
no_cm_kludge_end:
	dbra	d7,no_cm_lp
	movem.l	(a7)+,a2/d2/d6/d7
	rts

	end
