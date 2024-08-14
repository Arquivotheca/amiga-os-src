*******************************************************************************
*
* $Id: rekickend.asm,v 39.1 92/06/07 15:16:49 mks Exp $
*
* $Log:	rekickend.asm,v $
* Revision 39.1  92/06/07  15:16:49  mks
* Initial release
* 
*
*******************************************************************************
*
* Since this code does all sorts of magic including a RESET instruction,
* we need to make sure we are in memory that will not go away.  This
* turns out to be CHIP memory on all current systems.  (MEMF_LOCAL under
* V37 and up, but that can not be counted on...)
*
	SECTION	ReKick,CODE,CHIP
*
		XDEF	EndOfMem
EndOfMem:	dc.b	'BOO!'
*
*******************************************************************************
*
	END
