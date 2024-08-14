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
* $Id: vec_table.asm,v 40.4 93/03/11 17:22:30 mks Exp $
*
**********************************************************************

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
*------ Exported Functions -------------------------------------------

FUNCDEF		MACRO
	XREF	MI\1
		DC.W    MI\1+(*-libFuncInit)
		ENDM

		xdef libFuncInit

libFuncInit:
*
* Only build the non-FPU table for non-FPU compatible builds...
*
	IFND	CPU_FPU

		DC.W	-1	* use short form
		FUNCDEF	Open
		FUNCDEF	Close
		FUNCDEF	Expunge
		FUNCDEF	ExtFunc

        FUNCDEF IEEESPFix
        FUNCDEF IEEESPFlt
        FUNCDEF IEEESPCmp
        FUNCDEF IEEESPTst
        FUNCDEF IEEESPAbs
        FUNCDEF IEEESPNeg
        FUNCDEF IEEESPAdd
        FUNCDEF IEEESPSub
        FUNCDEF IEEESPMul
        FUNCDEF IEEESPDiv
        FUNCDEF IEEESPFloor
        FUNCDEF IEEESPCeil


		DC.W	-1

	ENDC
*
*****
*
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
	HARDFUNCDEF	IEEEDPFix
	HARDFUNCDEF	IEEEDPFlt
	HYBRIDFUNCDEF	IEEESPCmp
	HYBRIDFUNCDEF	IEEESPTst
	HYBRIDFUNCDEF	IEEESPAbs
	HYBRIDFUNCDEF	IEEESPNeg
	HARDFUNCDEF	IEEEDPAdd
	HARDFUNCDEF	IEEEDPSub
	HARDFUNCDEF	IEEEDPMul
	HARDFUNCDEF	IEEEDPDiv
	HARDFUNCDEF	IEEEDPFloor
	HYBRIDFUNCDEF	IEEESPCeil

		DC.W	-1

		    END
