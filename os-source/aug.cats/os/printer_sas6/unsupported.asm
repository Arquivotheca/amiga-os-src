	TTL    '$Id: unsupported.asm,v 1.2 90/07/27 02:19:40 bryce Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	printer device unsupported commands
*
*   Source Control
*   --------------
*   $Id: unsupported.asm,v 1.2 90/07/27 02:19:40 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	unsupported.asm,v $
*   Revision 1.2  90/07/27  02:19:40  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:25:54  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:29:06  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:48:20  daveb
*   added to rcs
*   
*   Revision 25.0  85/06/13  18:53:45  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/initializers.i"


*------ Imported Functions -------------------------------------------

	XREF		PCInvalid


*------ Exported Globals ---------------------------------------------

	XDEF		PCRead
	XDEF		PCUpdate
	XDEF		PCClear

	XDEF		PCBRead
	XDEF		PCBUpdate
	XDEF		PCBClear


**********************************************************************
*
*   INPUTS
*	iORequest -- the I/O Request for this command.
*
**********************************************************************
PCBRead		EQU	-1
PCBUpdate	EQU	-1
PCBClear	EQU	-1

PCRead:
PCUpdate:
PCClear:
		BRA	PCInvalid

		END
