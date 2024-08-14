	TTL    '$Header: macros.i,v 36.0 89/01/27 15:46:31 kodiak Exp $'
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
*   Source Control
*   --------------
*   $Header: macros.i,v 36.0 89/01/27 15:46:31 kodiak Exp $
*
*   $Locker:  $
*
*   $Log:	macros.i,v $
*   Revision 36.0  89/01/27  15:46:31  kodiak
*   *** empty log message ***
*   
*
**********************************************************************

*------ library function definition macro ----------------------------

XREF_EXE	MACRO
	XREF	_LVO\1
		ENDM

XREF_ML 	MACRO
	XREF	_LVO\1
		ENDM


*------ library dispatch macros --------------------------------------

LINKEXE		MACRO
		LINKLIB _LVO\1,_AbsExecBase
		ENDM

CALLEXE		MACRO
		CALLLIB _LVO\1
		ENDM
