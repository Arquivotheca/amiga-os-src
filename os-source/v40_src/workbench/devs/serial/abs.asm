   
	TTL '$Id: abs.asm,v 1.3 91/01/12 20:37:49 bryce Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	         SERIAL DEVICE DRIVER            **
*	   **	   --------------------------------	 **
*	   **						 **
*	   ************************************************
  

**********************************************************************
*								     *
*   Copyright 1984,1985,1989 Commodore Amiga Inc. All rights reserved*
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*								     *
**********************************************************************


**********************************************************************
*
*   Source Control:
*
*	$Id: abs.asm,v 1.3 91/01/12 20:37:49 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	abs.asm,v $
*   Revision 1.3  91/01/12  20:37:49  bryce
*   #@&!~($%! RCS 4.0 switch.   Change Header to Id
*   
*   Revision 1.2  89/04/19  18:47:15  bryce
*   Fix some autodocs & short branches
*   
*	
*
**********************************************************************
 
    SECTION     serial.device
 
    NOLIST

****** Included Files ***********************************************

    INCLUDE	'exec/types.i'
    INCLUDE     'exec/nodes.i'
    INCLUDE	'exec/strings.i'
    INCLUDE	'exec/resident.i'
    INCLUDE     'serial_rev.i'  

    LIST

****** Imported *****************************************************

    XREF        serName
    XREF        endMarker
    XREF        devStructInit
    XREF        serInit
    XREF        functions

**********************************************************************
*
****** Exported *****************************************************

    XDEF        VERNUM
    XDEF        REVNUM
    XDEF	serIdent

**********************************************************************
*
*      Device Initial Data
*
**********************************************************************

;FIXED-bryce:Dump out with error is executed directly
	    MOVEQ.L #-1,D0
	    RTS
*------ Identification String ----------------------------------

serIdent:   VSTRING

*------ Resident Tag ---------------------------------------------

serTag:
	    DC.W    RTC_MATCHWORD
	    DC.L    serTag
	    DC.L    endMarker
	    DC.B    0
	    DC.B    VERSION             * version number
	    DC.B    NT_DEVICE		* node type
	    DC.B    60			* initialization priority
	    DC.L    serName
	    DC.L    serIdent
	    DC.L    serInit             * init entry point

VERNUM      EQU     VERSION
REVNUM      EQU     REVISION

   END

