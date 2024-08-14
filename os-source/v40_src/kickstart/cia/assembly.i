
**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
*   Source Control:
*
*	$Id: assembly.i,v 36.5 91/01/09 10:14:11 rsbx Exp $
*
**********************************************************************


OFF		EQU     0
ON		EQU     1


FINAL		EQU 	OFF	* final ROM form (not debug form)
MCCASM		EQU 	ON	* metacomco-land kludges (hate it)

DEBUGAID        EQU     ON	* various debugging code
MEASUREMENT     EQU     ON	* emulator measurement support

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
	    DS.W    0
	    ENDM


