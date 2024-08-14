	TTL	'SPNAME.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

	SECTION speech



    INCLUDE	'assembly.i'
    INCLUDE	'narrator_rev.i'
    INCLUDE	'exec/types.i'
    INCLUDE	'exec/nodes.i'
    INCLUDE	'exec/strings.i'
    INCLUDE	'exec/resident.i'



******* Exported *****************************************************

    XDEF	NarratorName
    XDEF	NarratorIdStr



NarratorIdStr    VSTRING


*------ Name String  -------------------------------------------

NarratorName   STRING  'narrator.device'



    END
