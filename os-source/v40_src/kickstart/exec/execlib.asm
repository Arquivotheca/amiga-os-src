*	   ************************************************
*	   **						 **
*	   **	     --=  AMIGA HOME COMPUTER  =--	 **
*	   **						 **
*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		   System Library		 **
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
*
*	$Id: execlib.asm,v 39.1 92/10/06 15:31:10 mks Exp $
*
*	$Log:	execlib.asm,v $
* Revision 39.1  92/10/06  15:31:10  mks
* Added two new reserved vectors and removed on for use in the
* Quick Interrupt code
* 
* Revision 39.0  91/10/15  08:27:18  mks
* V39 Exec initial checkin
*
*************************************************************************

    NOLIST
    INCLUDE	"types.i"
    INCLUDE	"assembly.i"
    INCLUDE	"libraries.i"
    LIST


	XDEF	open
	XDEF	close
	XDEF	expunge
	XDEF	noharm

open:
	    MOVE.L  A6,D0
	    ADDQ.W  #1,LIB_OPENCNT(A6)
	    RTS

close:
	    SUBQ.W  #1,LIB_OPENCNT(A6)
expunge:
noharm:
ExecReserved04:	XDEF	ExecReserved04
ExecReserved05:	XDEF	ExecReserved05
ExecReserved06:	XDEF	ExecReserved06
ExecReserved07:	XDEF	ExecReserved07
ExecReserved08:	XDEF	ExecReserved08
	    CLEAR   D0
	    RTS

	END
