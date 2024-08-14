	TTL    '$Id: iovec_table.asm,v 37.1 91/01/21 11:43:40 mks Exp $'
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
*   $Id: iovec_table.asm,v 37.1 91/01/21 11:43:40 mks Exp $
*
*   $Locker:  $
*
*   $Log:	iovec_table.asm,v $
*   Revision 37.1  91/01/21  11:43:40  mks
*   V37 checkin for Make Internal
*   
*   Revision 36.1  90/04/08  01:29:31  dale
*   new RCS
*
*   Revision 36.0  89/06/05  15:46:46  dale
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

* define macro
IOFUNCDEF		MACRO
	XREF	IOMI\1
		DC.W    IOMI\1+(*-iolibFuncInit)
		ENDM
HYBRIDFUNCDEF		MACRO
	XREF	MI\1
		DC.W    MI\1+(*-iolibFuncInit)
		ENDM

		xdef iolibFuncInit
iolibFuncInit:
		DC.W	-1	* use short form
		HYBRIDFUNCDEF	Open
		HYBRIDFUNCDEF	Close
		HYBRIDFUNCDEF	Expunge
		HYBRIDFUNCDEF	ExtFunc
	IOFUNCDEF	IEEESPAtan
	IOFUNCDEF	IEEESPSin
	IOFUNCDEF	IEEESPCos
	IOFUNCDEF	IEEESPTan
	IOFUNCDEF	IEEESPSincos
	IOFUNCDEF	IEEESPSinh
	IOFUNCDEF	IEEESPCosh
	IOFUNCDEF	IEEESPTanh
	IOFUNCDEF	IEEESPExp
	IOFUNCDEF	IEEESPLog
	IOFUNCDEF	IEEESPPow
	IOFUNCDEF	IEEESPSqrt
	IOFUNCDEF	IEEESPTieee
	IOFUNCDEF	IEEESPFieee
	IOFUNCDEF	IEEESPAsin
	IOFUNCDEF	IEEESPAcos
	IOFUNCDEF	IEEESPLog10

		DC.W	-1

		    END
