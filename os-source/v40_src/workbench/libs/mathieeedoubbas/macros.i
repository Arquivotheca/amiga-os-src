	TTL    '$Id: macros.i,v 37.1 91/01/21 12:02:36 mks Exp $'
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
*   $Id: macros.i,v 37.1 91/01/21 12:02:36 mks Exp $
*
*   $Log:	macros.i,v $
*   Revision 37.1  91/01/21  12:02:36  mks
*   Cleanup for V37 make internal
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
