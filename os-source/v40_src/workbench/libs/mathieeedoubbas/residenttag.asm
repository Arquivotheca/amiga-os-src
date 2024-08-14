    TTL    '$Id: residenttag.asm,v 37.1 91/01/21 12:01:47 mks Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*   $Id: residenttag.asm,v 37.1 91/01/21 12:01:47 mks Exp $
*
**********************************************************************

	SECTION		mathieeedoubbas

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/nodes.i'
	INCLUDE		'exec/resident.i'
	INCLUDE		'exec/strings.i'

	INCLUDE     'mathieeedoubbas_rev.i'

*------ Imported Names -----------------------------------------------

	XREF		MIInit
	XREF		MIEndModule


*------ Exported Names -----------------------------------------------

	XDEF		MIName


**********************************************************************
*		prevent self from being executed directly
		move.l	#-1,d0
		rts

residentMI:
		DC.W	RTC_MATCHWORD     /* word to match on (ILLEGAL)   */
		DC.L	residentMI        /* pointer to the above         */
		DC.L	MIEndModule       /* address to continue scan     */
		DC.B	RTW_COLDSTART     /* various tag flags
*		DC.B	RTW_COLDSTART!RTF_AUTOINIT /* various tag flags         */
*                             	  /* Exec will automatically initialize lib */
		DC.B	VERSION           /* release version number       */
		DC.B	NT_LIBRARY        /* type of module (NT_mumble)   */
		DC.B	0                 /* initialization priority      */
		DC.L	MIName            /* pointer to node name         */
		DC.L	identMI           /* pointer to ident string      */
		DC.L	MIInit            /* pointer to init code         */

MIName:
		STRING	<'mathieeedoubbas.library'>
identMI:
		VSTRING

		END
