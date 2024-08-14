*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: asmsupp.i,v 34.9 87/12/04 19:13:17 bart Exp $
*
*	$Locker:  $
*
*	$Log:	asmsupp.i,v $
*   Revision 34.9  87/12/04  19:13:17  bart
*   checkpoint
*   
*   Revision 34.8  87/12/04  12:07:51  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.7  87/10/14  14:25:58  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:15:20  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:00:18  bart
*   *** empty log message ***
*   
*   Revision 34.4  87/06/11  15:46:55  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:58:33  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:35:07  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:38:51  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:10  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

BHS	MACRO
	BCC.\0	\1
	ENDM

BLO	MACRO
	BCS.\0	\1
	ENDM

EVEN	MACRO
	DS.W	0
	ENDM

CLRA    MACRO
	SUBA.\0 \1,\1
	ENDM

LINKSYS MACRO
	LINKLIB _LVO\1,HD_SYSLIB(A6)
	ENDM

CALLSYS	MACRO
	CALLLIB	_LVO\1
	ENDM
