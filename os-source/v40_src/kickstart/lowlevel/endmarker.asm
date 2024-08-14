******************************************************************************
*
*	$Id: endmarker.asm,v 40.0 93/04/20 14:20:15 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	endmarker.asm,v $
* Revision 40.0  93/04/20  14:20:15  Jim2
* Added header.
* 
*
******************************************************************************
*
*   Place holder for the label that defines the limit of the code for
*   lowlevel.library.
*
		XDEF	LowLevel_end
		ds.b	0
LowLevel_end:
		end
