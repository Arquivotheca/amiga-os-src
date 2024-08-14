	TTL    '$Id: residenttag.asm,v 1.3 90/08/29 16:52:55 darren Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	printer device initialization tag
*
*   Source Control
*   --------------
*   $Id: residenttag.asm,v 1.3 90/08/29 16:52:55 darren Exp $
*
*   $Locker:  $
*
*   $Log:	residenttag.asm,v $
*   Revision 1.3  90/08/29  16:52:55  darren
*   Fix - the printer.device would
*   crash if run.  The BRA to its
*   initialization code is obviously not
*   enough, so ... we just get out
*   with a -1.
*   
*   Revision 1.2  90/07/27  02:19:21  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:25:17  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:28:14  daveb
*   added to rcs
*   
*   Revision 1.1  87/04/24  15:52:51  andy
*   back under rcs
*   
*   Revision 1.0  87/02/09  13:09:50  andy
*   added to rcs
*   
*   Revision 25.1  85/06/14  04:06:53  kodiak
*   6/14 morning checkin
*   
*   Revision 25.0  85/06/13  18:53:36  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"printer_rev.i"


*------ Imported Names -----------------------------------------------

	XREF		PInit
	XREF		PEndModule


*------ Exported Names -----------------------------------------------

	XDEF		PName
	XDEF		VERSION
	XDEF		REVISION


**********************************************************************
		MOVEQ	#-01,D0			;get out of here cleanly
		RTS				;if someone tries to run us

residentP:
		DC.W	RTC_MATCHWORD	; illegal instr. so Exec can find us
		DC.L	residentP		; address of this table
		DC.L	PEndModule		; address of end of code
		DC.B	RTW_COLDSTART	; tells exec to init us at cold start time
		DC.B	VERSION			; version # of this device
		DC.B	NT_DEVICE		; node type
		DC.B	-10				; priority
		DC.L	PName			; name of the device
		DC.L	identP			; an id string
		DC.L	PInit			; address of initialization code

PName:
		STRING	<'printer.device'>
identP:
		VSTRING

		END
