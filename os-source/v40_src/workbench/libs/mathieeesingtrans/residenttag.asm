    TTL    '$Id: residenttag.asm,v 37.1 91/01/21 11:43:02 mks Exp $'
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
*	Console device initialization tag
*
*   Source Control
*   --------------
*   $Id: residenttag.asm,v 37.1 91/01/21 11:43:02 mks Exp $
*
*   $Locker:  $
*
*   $Log:	residenttag.asm,v $
*   Revision 37.1  91/01/21  11:43:02  mks
*   V37 checkin for Make Internal
*   
*   Revision 36.1  90/04/08  01:29:57  dale
*   new RCS
*
*   Revision 36.0  89/06/05  15:45:28  dale
*   *** empty log message ***
*
*
**********************************************************************

	SECTION		mathieeesingtrans

*------ Included Files -----------------------------------------------

	INCLUDE		'exec/types.i'
	INCLUDE		'exec/nodes.i'
	INCLUDE		'exec/resident.i'
	INCLUDE		'exec/strings.i'

	INCLUDE     'mathieeesingtrans_rev.i'

*------ Imported Names -----------------------------------------------

	XREF		MIInit
	XREF		MIEndModule


*------ Exported Names -----------------------------------------------

	XDEF		MIName


**********************************************************************

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
		STRING	<'mathieeesingtrans.library'>
identMI:
		VSTRING

		END
