
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* blit.h
*
* Source Control
* ------ -------
* 
* $Id: blit.asm,v 27.3 90/06/01 23:14:37 jesup Exp $
*
* $Locker:  $
*
* $Log:	blit.asm,v $
* Revision 27.3  90/06/01  23:14:37  jesup
* Conform to include standard du jour
* 
* Revision 27.2  89/04/27  23:28:23  jesup
* fixed autodocs
* 
* Revision 27.1  85/06/24  13:36:20  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:12:55  neil
* *** empty log message ***
* 
* 
*************************************************************************

****** Included Files ***********************************************

	INCLUDE 'hardware/custom.i'

	SECTION section

****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------

*------ System Library Functions -------------------------------------

****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	TDDoBlit

*------ Data ---------------------------------------------------------

****** Local Definitions ********************************************


*****i* trackdisk.device/internal/TDDoBlit ***************************
*
*   NAME
*	TDDoBlit - set up the blitter with "standard" values
*
*   SYNOPSIS
*	TDDoBlit( custom ), TrackDiskDevice
*		  A0        A6
*
*   FUNCTION
*
*   INPUTS
*	custom -- pointer to special chips
*
*   OUTPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*


TDDoBlit:
		MOVEQ	#-1,D0
		MOVE.L	D0,bltafwm(A0)	; bltafwm, bltalwm

		MOVEQ	#0,D0
		LEA	bltbmod(A0),A1
		MOVE.L	D0,(A1)+	; bltbmod, bltamod
		MOVE.W	D0,(A1)+	; bltdmod

		ADDQ.L	#8,A1
		MOVE.W	#$5555,(A1)	; bltcdat

		RTS

	END
