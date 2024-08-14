
**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
*   Source Control:
*
*	$Id: assembly.i,v 1.3 91/01/13 14:33:06 bryce Exp $
*
*	$Locker:  $
*
**********************************************************************

*	IDNT	exec
*	SECTION exec		* must go before EQU's


OFF		EQU     0
ON		EQU     1


FINAL		EQU 	OFF	* final ROM form (not debug form)
MCCASM		EQU 	ON	* metacomco assembler

MEASUREMENT     EQU     ON	* emulator measurement support

ALLOCABS	EQU 	OFF	* absolute memory allocator


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


