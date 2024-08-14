	TTL    '$Id: vec_table.asm,v 37.1 91/01/21 11:43:29 mks Exp $'
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
*   $Id: vec_table.asm,v 37.1 91/01/21 11:43:29 mks Exp $
*
*   $Locker:  $
*
*   $Log:	vec_table.asm,v $
*   Revision 37.1  91/01/21  11:43:29  mks
*   V37 checkin for Make Internal
*   
*   Revision 36.2  90/04/08  01:30:44  dale
*   new RCS
*
*   Revision 36.1  89/11/17  21:43:51  dale
*   make internal being wierd
*
*   Revision 36.0  89/06/05  15:45:40  dale
*   *** empty log message ***
*
*
**********************************************************************

        SECTION         mathieeesingtrans

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
*------ Exported Functions -------------------------------------------

FUNCDEF		MACRO
	XREF	MI\1
		DC.W    MI\1+(*-libFuncInit)
		ENDM

		xdef libFuncInit

libFuncInit:
		DC.W	-1	* use short form
		FUNCDEF	Open
		FUNCDEF	Close
		FUNCDEF	Expunge
		FUNCDEF	ExtFunc

	FUNCDEF	IEEESPAtan
	FUNCDEF	IEEESPSin
	FUNCDEF	IEEESPCos
	FUNCDEF	IEEESPTan
	FUNCDEF	IEEESPSincos
	FUNCDEF	IEEESPSinh
	FUNCDEF	IEEESPCosh
	FUNCDEF	IEEESPTanh
	FUNCDEF	IEEESPExp
	FUNCDEF	IEEESPLog
	FUNCDEF	IEEESPPow
	FUNCDEF	IEEESPSqrt
	FUNCDEF	IEEESPTieee
	FUNCDEF	IEEESPFieee
	FUNCDEF	IEEESPAsin
	FUNCDEF	IEEESPAcos
	FUNCDEF	IEEESPLog10

		DC.W	-1

* define macro
HARDFUNCDEF		MACRO
	XREF	HARDMI\1
		DC.W    HARDMI\1+(*-hardlibFuncInit)
		ENDM
HYBRIDFUNCDEF		MACRO
	XREF	MI\1
		DC.W    MI\1+(*-hardlibFuncInit)
		ENDM

		xdef hardlibFuncInit
hardlibFuncInit:
		DC.W	-1	* use short form
		HYBRIDFUNCDEF	Open
		HYBRIDFUNCDEF	Close
		HYBRIDFUNCDEF	Expunge
		HYBRIDFUNCDEF	ExtFunc
	HARDFUNCDEF	IEEESPAtan
	HARDFUNCDEF	IEEESPSin
	HARDFUNCDEF	IEEESPCos
	HARDFUNCDEF	IEEESPTan
	HARDFUNCDEF	IEEESPSincos
	HARDFUNCDEF	IEEESPSinh
	HARDFUNCDEF	IEEESPCosh
	HARDFUNCDEF	IEEESPTanh
	HARDFUNCDEF	IEEESPExp
	HARDFUNCDEF	IEEESPLog
	HARDFUNCDEF	IEEESPPow
	HARDFUNCDEF	IEEESPSqrt
	HARDFUNCDEF	IEEESPTieee
	HARDFUNCDEF	IEEESPFieee
	HARDFUNCDEF	IEEESPAsin
	HARDFUNCDEF	IEEESPAcos
	HARDFUNCDEF	IEEESPLog10

		DC.W	-1

		    END
