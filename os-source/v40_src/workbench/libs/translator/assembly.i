	TTL	'$Header: assembly.i,v 36.0 89/05/24 10:11:36 kodiak Exp $'
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
* $Header: assembly.i,v 36.0 89/05/24 10:11:36 kodiak Exp $
*
* $Locker:  $
*
* $Log:	assembly.i,v $
*   Revision 36.0  89/05/24  10:11:36  kodiak
*   *** empty log message ***
*   
* Revision 32.1  86/01/22  01:18:06  sam
* placed under source control
* 
*
**********************************************************************
	SECTION reader		* must go before EQU's

OFF		EQU     0
ON		EQU     1


FINAL		EQU 	OFF	* final ROM form (not debug form)
MCCASM		EQU 	ON	* metacomco assembler

COLDCAPTURE	EQU	OFF	* allow system to coldcapture
CART_VERSION	EQU	ON	* make system work in cartridge memory

DEBUGAID        EQU     ON	* various debugging code
MEASUREMENT     EQU     ON	* emulator measurement support
EXEC_EXCEPTIONS EQU     ON	* should exec or emulator handle except

NEWLISTS	EQU 	ON	* new list structures
ALLOCABS	EQU 	ON	* absolute memory allocator


W		EQU	$0FFFF	* word arithmetic kludge


*------ Special Externals:

EXTERN_BASE MACRO
	    ENDM

EXTERN_SYS  MACRO
            XREF    _LVO\1
            ENDM

EXTERN_CODE MACRO
            XREF    \1
            ENDM

EXTERN_DATA MACRO
            XREF    \1
            ENDM

EXTERN_STR  MACRO
            XREF    \1
            ENDM



*------ pseudo-instruction macros:

CLEAR       MACRO
            MOVEQ.L #0,\1
            ENDM

EVEN	    MACRO
	    CNOP    0,2
	    ENDM


