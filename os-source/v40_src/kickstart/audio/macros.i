**********************************************************************
*								     *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.	     *
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
* $Id: macros.i,v 36.2 90/08/29 17:54:12 mks Exp $
*
* $Locker:  $
*
* $Log:	macros.i,v $
* Revision 36.2  90/08/29  17:54:12  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* 
* Revision 32.1  86/01/14  21:23:15  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:48:38  sam
* Initial revision
*
*
**********************************************************************

*------ external definition macros -----------------------------------

XREF_EXE	MACRO
	XREF		_LVO\1
		ENDM

*------ library dispatch macros --------------------------------------

CALLEXE		MACRO
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO
		LINKLIB _LVO\1,_AbsExecBase
		ENDM
