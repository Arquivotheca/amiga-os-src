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
*	printer device macro definitions
*
*   Source Control
*   --------------
*   $Id: macros.i,v 1.2 90/07/27 02:20:39 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	macros.i,v $
*   Revision 1.2  90/07/27  02:20:39  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:24:02  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:32:11  daveb
*   added to rcs
*   
*   Revision 25.2  85/06/16  01:04:51  kodiak
*   *** empty log message ***
*   
*   Revision 25.1  85/06/14  04:06:25  kodiak
*   6/14 morning checkin
*   
*   Revision 25.0  85/06/13  19:13:34  kodiak
*   added to rcs
*   
*
**********************************************************************

*------ external definition macros -----------------------------------

XREF_EXE	MACRO
	XREF		_LVO\1
		ENDM

XREF_DOS	MACRO
	XREF		_LVO\1
		ENDM

XREF_GFX	MACRO
	XREF		_LVO\1
		ENDM

XREF_ITU	MACRO
	XREF		_LVO\1
		ENDM

*------ library dispatch macros --------------------------------------

CALLEXE		MACRO
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO
		LINKLIB _LVO\1,_SysBase
		ENDM

LINKDOS		MACRO
		LINKLIB _LVO\1,_DOSBase
		ENDM

LINKGFX		MACRO
		LINKLIB _LVO\1,_GfxBase
		ENDM

LINKITU		MACRO
		LINKLIB _LVO\1,_IntuitionBase
		ENDM
