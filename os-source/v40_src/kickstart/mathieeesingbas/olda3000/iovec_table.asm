	section mathieeesingbas
	TTL    '$Id: iovec_table.asm,v 1.4 90/08/30 15:57:43 mks Exp $'
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
*   $Id: iovec_table.asm,v 1.4 90/08/30 15:57:43 mks Exp $
*
**********************************************************************

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
*------ Exported Functions -------------------------------------------

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
	HYBRIDFUNCDEF	IEEESPFix
	IOFUNCDEF	IEEEDPFlt
	HYBRIDFUNCDEF	IEEESPCmp
	HYBRIDFUNCDEF	IEEESPTst
	HYBRIDFUNCDEF	IEEESPAbs
	HYBRIDFUNCDEF	IEEESPNeg
	IOFUNCDEF	IEEEDPAdd
	IOFUNCDEF	IEEEDPSub
	IOFUNCDEF	IEEEDPMul
	IOFUNCDEF	IEEEDPDiv
	HYBRIDFUNCDEF	IEEESPFloor
	HYBRIDFUNCDEF	IEEESPCeil

		DC.W	-1

		    END
