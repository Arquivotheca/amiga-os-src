*******************************************************************************
*
*	$Id: setapen.asm,v 38.2 92/09/03 15:36:57 spence Exp $
*
*******************************************************************************

	include 'exec/types.i'                  * Data type definitions
	include 'graphics/rastport.i'           * RastPort/AreaInfo structures
	include 'submacs.i'                     * Subroutine/function macros

	section	graphics
	xdef    _SetAPen                * Define public entry points
	xdef    _SetBPen
	xdef    _SetDrMd
	xdef    _VBeamPos

	xref    GenMinTerms                     * Define external symbols used
	xref    _vposr



	PAGE

******* graphics.library/SetAPen ***************************************
* 
*   NAME   
*	SetAPen -- Set the primary pen for a RastPort.
*
*   SYNOPSIS
*	SetAPen( rp, pen )
*		 a1  d0
*
*	void SetAPen( struct RastPort *, UBYTE );
*
*   FUNCTION
*	Set the primary drawing pen for lines, fills, and text.
*
*   INPUTS
*	rp - pointer to RastPort structure.
*	pen - (0-255)
*
*   RESULT
*	Changes the minterms in the RastPort to reflect new primary pen.
*	Sets line drawer to restart pattern.
*
*   BUGS
*
*   SEE ALSO
*	SetBPen() graphics/rastport.h
*
**************************************************************************


SetAPen:
_SetAPen:
	move.b  d0,rp_FgPen(a1)             * w->FgPen = p;
	bra.s   Com_Code_1                  * Execute common code



******* graphics.library/SetBPen ***************************************
* 
*   NAME   
*	SetBPen -- Set secondary pen for a RastPort
*
*   SYNOPSIS
*	SetBPen( rp, pen )
*		 a1  d0
*
*	void SetBPen( struct RastPort *, UBYTE );
*
*   FUNCTION
*	Set the secondary drawing pen for lines, fills, and text.
*
*   INPUTS
*	rp - pointer to RastPort structure.
*	pen - (0-255)
*
*   RESULT
*	Changes the minterms in the RastPort to reflect new secondary pen.
*	Sets line drawer to restart pattern.
*
*   BUGS
*
*    SEE ALSO
*	SetAPen() graphics/rastport.h
*
**************************************************************************

SetBPen:
_SetBPen:
	move.b  d0,rp_BgPen(a1)             * w->BgPen = p;
	bra.s   Com_Code_1                  * Execute common code


******* graphics.library/SetDrMd ***************************************
* 
*   NAME   
* 	SetDrMd -- Set drawing mode for a RastPort
*
*   SYNOPSIS
*	SetDrMd( rp, mode )
*		 a1  d0:8
*
*	void SetDrMd( struct RastPort *, UBYTE );
*
*   FUNCTION
*	Set the drawing mode for lines, fills and text.
*	Get the bit definitions from rastport.h
*
*   INPUTS
*	rp - pointer to RastPort structure.
*	mode - 0-255, some combinations may not make much sense.
*
*   RESULT
*	The mode set is dependent on the bits selected.
*	Changes minterms to reflect new drawing mode.
*	Sets line drawer to restart pattern.
*
*   BUGS
*
*   SEE ALSO
*	SetAPen() SetBPen() graphics/rastport.h
*
**************************************************************************


SetDrMd:
_SetDrMd:
	move.b  d0,rp_DrawMode(a1)          * w->DrawMode = m;

Com_Code_1:
	move.b  #15,rp_linpatcnt(a1)        * w->linpatcnt = 15;
	or      #RPF_FRST_DOT,rp_Flags(a1)  * w->Flags |= FRST_DOT;

	GENMINTERMS                         * genminterms(w);

	rts                                 * Exit to caller

	xdef    _vbeampos

	xdef	_SetABPenDrMd

_SetABPenDrMd:
******* graphics.library/SetABPenDrMd ***************************************
* 
*   NAME   
* 	SetABPenDrMd -- Set pen colors and draw mode for a RastPort.
*
*   SYNOPSIS
*	SetABPenDrMd( rp, apen, bpen, mode )
*		 		  a1  d0     d1    d2
*
*	void SetABPenDrMd( struct RastPort *, ULONG, ULONG, ULONG );
*
*   FUNCTION
*	Set the pen values and drawing mode for lines, fills and text.
*	Get the bit definitions from rastport.h
*
*   INPUTS
*	rp - pointer to RastPort structure.
*	apen - primary pen value
*	bpen - secondary pen value
*	mode - 0-255, some combinations may not make much sense.
*
*   RESULT
*	The mode set is dependent on the bits selected.
*	Changes minterms to reflect new drawing mode and colors.
*	Sets line drawer to restart pattern.
*
*	NOTES
*	This call is essentially the same as a sequence of
*	SetAPen()/SetBPen()/SetDrMD() calls, except that it is
*	significantly faster. The minterms will only be generated
*	once, or not at all if nothing changed (warning to illegal
*	RastPort pokers!).
*
*   BUGS
*
*   SEE ALSO
*	SetAPen() SetBPen() SetDrMd() graphics/rastport.h
*
**************************************************************************
	cmp.b	rp_DrawMode(a1),d2
	bne.s	something_changed
	cmp.b	rp_BgPen(a1),d1
	bne.s	something_changed_1
	cmp.b	rp_FgPen(a1),d0
	bne.s	something_changed_2
	rts
something_changed:
	move.b	d2,rp_DrawMode(a1)
something_changed_1:
	move.b	d1,rp_BgPen(a1)
something_changed_2:
	move.b	d0,rp_FgPen(a1)
	bra.s	Com_Code_1

	
******* graphics.library/VBeamPos **************************************
* 
*   NAME   
*	VBeamPos -- Get vertical beam position at this instant.
*
*   SYNOPSIS
*	pos = VBeamPos()
*	 d0
*
*	LONG VBeamPos( void );
*
*   FUNCTION
*	Get the vertical beam position from the hardware.
*
*   INPUTS
*	none
*
*   RESULT
*	interrogates hardware for beam position and returns value.
*	valid results in are the range of 0-511.
*	Because of multitasking, the actual value returned may have
*	no use. If you are the highest priority task then the value
*	returned should be close, within 1 line.
*
*   BUGS
*
*   SEE ALSO
*
*
**************************************************************************
_vbeampos:
_VBeamPos:
	move.l  _vposr,d0                   * d0.l = (*(long *) (&vposr))
	asr.l   #8,d0                       * d0.l = (*(long *)(&vposr))>>8
	and.l   #$7FF,d0                    * return(t &= 0x1ff); 

	rts                                 * Exit to caller


	end                                 * End of _InitRast
