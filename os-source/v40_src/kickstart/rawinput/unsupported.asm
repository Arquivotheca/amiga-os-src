	TTL    '$Id: unsupported.asm,v 35.2 90/04/13 12:45:59 kodiak Exp $
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
*	raw input device unsupported functions
*
**********************************************************************

	SECTION		rawinput

*------ Imported Functions -------------------------------------------

	XREF		Invalid


*------ Exported Globals ---------------------------------------------

	XDEF		Read
	XDEF		Write
	XDEF		Update

	XDEF		CBRead
	XDEF		CBWrite
	XDEF		CBUpdate


**********************************************************************
*
*   INPUTS
*	iORequest -- the I/O Request for this command.
*
**********************************************************************
CBRead		EQU	-1
CBWrite		EQU	-1
CBUpdate	EQU	-1

Read:
Write:
Update:
		BRA	Invalid

		END
