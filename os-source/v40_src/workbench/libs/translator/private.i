	TTL	'$Header: private.i,v 36.0 89/05/24 10:12:08 kodiak Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
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
* $Header: private.i,v 36.0 89/05/24 10:12:08 kodiak Exp $
*
* $Locker:  $
*
* $Log:	private.i,v $
*   Revision 36.0  89/05/24  10:12:08  kodiak
*   *** empty log message ***
*   
* Revision 32.1  86/01/22  01:18:14  sam
* placed under source control
* 
*
**********************************************************************

*		;------	System call macros

CALLSYS		MACRO	* &sysroutine
		JSR	_LVO\1(A6)
		ENDM

LINKSYS		MACRO	* &func
		LINKLIB _LVO\1,TR_SYSLIB(A6)
		ENDM


*		;------ Translator library node
 STRUCTURE TR,DD_SIZE
	APTR	TR_SYSLIB		;Pointer to system library
	APTR	TR_SEGLIST		;Pointer to segment list
	LABEL	TR_SIZE


*		;------	General equates

UNITB_EXPUNGE	EQU	5		;Delayed expunge bit
