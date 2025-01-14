	section mathieeedoubtrans
	TTL    '$Id: iovec_table.asm,v 37.1 91/01/21 15:19:11 mks Exp $'
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
*   $Id: iovec_table.asm,v 37.1 91/01/21 15:19:11 mks Exp $
*
**********************************************************************

        SECTION         mathieeedoubtrans

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
	IOFUNCDEF	IEEEDPAtan
	IOFUNCDEF	IEEEDPSin
	IOFUNCDEF	IEEEDPCos
	IOFUNCDEF	IEEEDPTan
	IOFUNCDEF	IEEEDPSincos
	IOFUNCDEF	IEEEDPSinh
	IOFUNCDEF	IEEEDPCosh
	IOFUNCDEF	IEEEDPTanh
	IOFUNCDEF	IEEEDPExp
	IOFUNCDEF	IEEEDPLog
	IOFUNCDEF	IEEEDPPow
	IOFUNCDEF	IEEEDPSqrt
	IOFUNCDEF	IEEEDPTieee
	IOFUNCDEF	IEEEDPFieee
	IOFUNCDEF	IEEEDPAsin
	IOFUNCDEF	IEEEDPAcos
	IOFUNCDEF	IEEEDPLog10

		DC.W	-1

		    END
