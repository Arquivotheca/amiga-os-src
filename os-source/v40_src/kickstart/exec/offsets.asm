*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		 System Library Table		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: offsets.asm,v 39.1 92/06/08 15:51:53 mks Exp $
*
*	$Log:	offsets.asm,v $
* Revision 39.1  92/06/08  15:51:53  mks
* Removed silly conditional for MCCASM
* 
* Revision 39.0  91/10/15  08:28:21  mks
* V39 Exec initial checkin
*
**********************************************************************



;Build table of relative vectors
FUNCDEF     MACRO   * function
	    XREF    \1
	    DC.W    \1+(*-SysLibTab)
FUNC_CNT    SET     FUNC_CNT-LIB_VECTSIZE
	    ENDM

;****** Included Files ***********************************************

	NOLIST
	INCLUDE "assembly.i"

	INCLUDE "types.i"
	INCLUDE "nodes.i"
	INCLUDE "libraries.i"
	INCLUDE "execbase.i"
	LIST

;****** Exported Globals *********************************************

	XDEF	SysLibTab
	XDEF	MAXSYSFUNC
	XDEF	EXECBASETOTAL

;****** Imported Functions *******************************************



;----------- relative pointers all Exec library functions -----------------
SysLibTab:
FUNC_CNT    SET     0
	    FUNCDEF open
	    FUNCDEF close
	    FUNCDEF expunge
	    FUNCDEF noharm
	    INCLUDE "exec_lib.i"        ;genlib'ed from exec_lib.fd
	    DC.W    -1			;End marker for relative vectors

;--------------------------------------------------------------------------
		IFNE (FUNC_CNT&3)
FUNC_CNT	SET  FUNC_CNT-2     ;Longword align
		ENDC
MAXSYSFUNC	EQU  FUNC_CNT			 ;(*-SysLibTab)*6
EXECBASETOTAL	EQU  SYSBASESIZE-MAXSYSFUNC	 ;MAXSYSFUNC is negative
		IFNE (EXECBASETOTAL&3)
		FAIL !!! EXECBASETOTAL must be a longword multiple !!!
		ENDC

	END
