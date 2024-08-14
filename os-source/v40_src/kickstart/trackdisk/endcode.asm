
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* endcode.asm
*
* Source Control
* ------ -------
* 
* $Id: endcode.asm,v 27.4 90/06/01 23:15:15 jesup Exp $
*
* $Locker:  $
*
* $Log:	endcode.asm,v $
* Revision 27.4  90/06/01  23:15:15  jesup
* Conform to include standard du jour
* 
* Revision 27.3  89/04/27  23:31:07  jesup
* 
* 
* Revision 27.2  89/03/13  12:03:10  jesup
* changed dc.l 0 to ds.w 0
* 
* Revision 27.1  85/06/24  13:36:45  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:06  neil
* *** empty log message ***
* 
* 
*************************************************************************


	XDEF	EndCode

	SECTION	section

	DS.W	0
EndCode:

	END
