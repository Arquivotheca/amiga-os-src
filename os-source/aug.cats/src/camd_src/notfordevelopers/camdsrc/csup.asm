*************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                   *
*************************************************************************
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines				*
*                                                                       *
*************************************************************************
*
* csup.asm    - Compiler specific C function interface
*
* this version supports Lattice C v5.05
*
*************************************************************************
      nolist
	include "exec/types.i"
	include "exec/macros.i"
      list


	idnt	csup.asm

	section __MERGED,DATA

	xref	__BSSBAS,__BSSLEN

	section csup,CODE


****************************************************************
*
*   C environment initialization
*
****************************************************************

****************************************************************
*
*   InitCEnv
*
*   FUNCTION
*	For Lattice, just init BSS.
*
*   INPUTS
*	A5 - CamdBase (not used here)
*	A6 - SysBase (not used here)
*
*   RESULT
*	none
*
****************************************************************

	xdef	InitCEnv
InitCEnv
	lea	__BSSBAS,a0		; clear BSS
	move.l	#__BSSLEN,d1
	CLEAR	d0
	bra.s	ice_endloop
ice_clrloop
	move.l	d0,(a0)+
ice_endloop
	dbra	d1,ice_clrloop

	rts

	end
