*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: endcode.asm,v 34.9 88/01/21 18:04:54 bart Exp $
*
*	$Locker:  $
*
*	$Log:	endcode.asm,v $
*   Revision 34.9  88/01/21  18:04:54  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.8  87/12/04  19:13:52  bart
*   checkpoint
*   
*   Revision 34.7  87/12/04  12:08:26  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.6  87/10/14  14:47:54  bart
*   10-13 rev 1
*   
*   Revision 34.5  87/10/14  14:15:55  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.4  87/06/11  15:47:58  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:59:00  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:35:44  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:39:21  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:37  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	LLEN	80
	PLEN	58
	LIST

*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************
	SECTION	section



	XDEF	EndCode


EndCode:
	DC.L	0

	END
