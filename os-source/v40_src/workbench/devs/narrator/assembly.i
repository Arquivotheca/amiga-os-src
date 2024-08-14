
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


	TTL	'ASSEMBLY.I'


	SECTION speech		; must go before EQU's


OFF		EQU     0
ON		EQU     1


FINAL		EQU 	OFF	; final ROM form (not debug form)
MCCASM		EQU 	ON	; metacomco assembler

COLDCAPTURE	EQU	OFF	; allow system to coldcapture
CART_VERSION	EQU	ON	; make system work in cartridge memory

DEBUGAID        EQU     ON	; various debugging code
MEASUREMENT     EQU     ON	; emulator measurement support
EXEC_EXCEPTIONS EQU     ON	; should exec or emulator handle except

NEWLISTS	EQU 	ON	; new list structures
ALLOCABS	EQU 	ON	; absolute memory allocator


W		EQU	$0FFFF	; word arithmetic kludge


;------ Special Externals:

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



;------ pseudo-instruction macros:

CLEAR       MACRO
            MOVEQ.L #0,\1
            ENDM

;EVEN	    MACRO
;	    CNOP    0,2
;	    ENDM


