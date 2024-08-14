	TTL	'$Header: trname.asm,v 36.0 89/05/24 10:11:02 kodiak Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
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
* $Header: trname.asm,v 36.0 89/05/24 10:11:02 kodiak Exp $
*
* $Locker:  $
*
* $Log:	trname.asm,v $
*   Revision 36.0  89/05/24  10:11:02  kodiak
*   *** empty log message ***
*   
* Revision 32.2  86/06/24  13:15:36  sam
* autodoc updates
* 
* Revision 32.1  86/01/22  01:18:01  sam
* placed under source control
* 
*
**********************************************************************
	SECTION reader

* ****** Included Files ***********************************************

    INCLUDE	'assembly.i'
    INCLUDE	'translator_rev.i'
    INCLUDE	'exec/types.i'
    INCLUDE	'exec/nodes.i'
    INCLUDE	'exec/strings.i'
    INCLUDE	'exec/resident.i'


* ****** Imported Globals *********************************************



* ****** Imported Functions *******************************************



* ****** Exported *****************************************************

    XDEF	TranslatorName
    XDEF	TranslatorIdStr



TranslatorIdStr:
		VSTRING


*------ Name String  -------------------------------------------

TranslatorName:
		STRING  'translator.library'



	END
