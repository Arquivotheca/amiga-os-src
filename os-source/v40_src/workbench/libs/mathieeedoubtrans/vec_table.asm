	section mathieeedoubtrans
	TTL    '$Id: vec_table.asm,v 37.1 91/01/21 15:18:35 mks Exp $'
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
*   $Id: vec_table.asm,v 37.1 91/01/21 15:18:35 mks Exp $
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

		xdef libFuncInit

libFuncInit:
		DC.W	-1	* use short form
		FUNCDEF	Open
		FUNCDEF	Close
		FUNCDEF	Expunge
		FUNCDEF	ExtFunc

        INCLUDE         'mathieeedoubtrans_lib.i'

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
	HARDFUNCDEF	IEEEDPAtan
	HARDFUNCDEF	IEEEDPSin
	HARDFUNCDEF	IEEEDPCos
	HARDFUNCDEF	IEEEDPTan
	HARDFUNCDEF	IEEEDPSincos
	HARDFUNCDEF	IEEEDPSinh
	HARDFUNCDEF	IEEEDPCosh
	HARDFUNCDEF	IEEEDPTanh
	HARDFUNCDEF	IEEEDPExp
	HARDFUNCDEF	IEEEDPLog
	HARDFUNCDEF	IEEEDPPow
	HARDFUNCDEF	IEEEDPSqrt
	HARDFUNCDEF	IEEEDPTieee
	HARDFUNCDEF	IEEEDPFieee
	HARDFUNCDEF	IEEEDPAsin
	HARDFUNCDEF	IEEEDPAcos
	HARDFUNCDEF	IEEEDPLog10

		DC.W	-1

		    END
