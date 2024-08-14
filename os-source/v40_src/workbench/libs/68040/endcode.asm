*******************************************************************************
*
* 68040 support library for 68881/68882 FPU emulation
*
* $Id: endcode.asm,v 1.2 91/05/22 17:11:26 mks Exp $
*
* $Log:	endcode.asm,v $
* Revision 1.2  91/05/22  17:11:26  mks
* Changed to use the assembly_options.i include file
* 
* Revision 1.1  91/05/21  16:19:46  mks
* Initial revision
*
*
*******************************************************************************
*
* Options for HX68 to turn on 68020 and 68881 FPU modes...
*
	INCLUDE	'assembly_options.i'
*
*******************************************************************************

		XDEF	EndCode
EndCode:	dc.b	'End',0
