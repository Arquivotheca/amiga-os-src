**********************************************************************
*								     *
*   Copyright 1984,1990 Commodore Amiga Inc.   All rights reserved.  *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore Amiga Incorporated, 3350 Scott Blvd, Bld #7,	     *
*   Santa Clara, CA 95051					     *
*								     *
**********************************************************************


**********************************************************************
*
*   Source Control:
*
*       $Id: absolute.asm,v 36.7 90/06/01 12:31:05 rsbx Exp $
*
**********************************************************************

	SECTION	cia

	INCLUDE	'assembly.i'

	INCLUDE	'exec/strings.i'
	INCLUDE	'exec/types.i'
	INCLUDE	'exec/nodes.i'
	INCLUDE	'exec/resident.i'

	INCLUDE	'cia.i'
	INCLUDE	'cia_rev.i'



	XREF	EndMarker
	XREF	InitCode

	XDEF	_CIAIdStr
	XDEF	CIAName
	XDEF	CIAAName
	XDEF	CIABName

*------ Resident Tag -------------------------------------------

residentTag:
	dc.w	RTC_MATCHWORD
	dc.l	residentTag
	dc.l	EndMarker
	dc.b	RTF_COLDSTART
	dc.b	VERSION			* RT_VERSION
	dc.b	NT_RESOURCE		* RT_TYPE
	dc.b	80			* low priority
	dc.l	CIAName
	dc.l	_CIAIdStr
	dc.l	InitCode


*------ Resource Name ---------------------------------------

CIAName
	STRING	'cia.resource'
CIAAName
	CIAANAME
CIABName
	CIABNAME


*------ ID String  ------------------------------------------

_CIAIdStr
	VSTRING

	END

