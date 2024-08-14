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
*   $Id: vec_table.asm,v 39.1 92/04/20 13:28:57 mks Exp $
*
*   $Log:	vec_table.asm,v $
*   Revision 39.1  92/04/20  13:28:57  mks
*   Made changes to compile native!  (yup! :-)
*   
**********************************************************************

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
*------ Exported Functions -------------------------------------------

FUNCDEF		MACRO
	XREF	MF\1
		DC.W    MF\1+(*-libFuncInit)
		ENDM

		xdef libFuncInit

libFuncInit:
		DC.W	-1	* use short form
		FUNCDEF	Open
		FUNCDEF	Close
		FUNCDEF	Expunge
		FUNCDEF	ExtFunc

	INCLUDE		'mathffp_lib.i'

		DC.W	-1

		    END
