*******************************************************************************
*
*	$Id: MakeVPort.asm,v 42.0 93/06/16 11:13:11 chrisg Exp $
*
*******************************************************************************

	section	graphics

******* graphics.library/MakeVPort ******************************************
*
*   NAME
*	MakeVPort -- generate display copper list for a viewport.
*
*   SYNOPSIS
*	error =  MakeVPort( view, viewport )
*	 d0                   a0     a1
*
*	ULONG MakeVPort( struct View *, struct ViewPort * );
*
*   FUNCTION
*	Uses information in the View, ViewPort, ViewPort->RasInfo to
*	construct and intermediate copper list for this ViewPort.
*
*   INPUTS
*	view - pointer to a View structure
*	viewport - pointer to a ViewPort structure
*		 The viewport must have valid pointer to a RasInfo.
*
*   RESULTS
*	constructs intermediate copper list and puts pointers in
*	viewport.DspIns
*	If the ColorMap ptr in ViewPort is NULL then it uses colors
*	from the default color table.
*	If DUALPF in Modes then there must be a second RasInfo pointed
*	to by the first RasInfo
*
*	From V39, MakeVPort can return a ULONG error value (previous versions
*	returned void), to indicate that either not enough memory could be
*	allocated for MakeVPort's use, or that the ViewPort mode
*	and bitplane alignments are incorrect for the bitplane's depth.
*
*	You should check for these error values - they are defined in
*	<graphics/view.h>.
*
*
*   BUGS
*	In V37 and earlier, narrow Viewports (whose righthand edge is 
*	less than 3/4 of the way across the display) do not work properly.
*
*   SEE ALSO
*	InitVPort() MrgCop() graphics/view.h intuition.library/MakeScreen()
*	intuition.library/RemakeDisplay() intuition.library/RethinkDisplay()
*
*****************************************************************************

* no longer any code it uses register params. see /c/makevp.c


	end
