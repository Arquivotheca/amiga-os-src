*************************************************************************
*
* $Id: abs.asm,v 38.1 91/06/24 19:00:56 mks Exp $
*
* $Log:	abs.asm,v $
* Revision 38.1  91/06/24  19:00:56  mks
* Changed to V38 source tree - Trimmed Log
* 
*************************************************************************

;****** Included Files ***********************************************

	INCLUDE 'assembly.i'
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE	'exec/resident.i'

	INCLUDE	'icon.i'
	INCLUDE	'icon_rev.i'


;****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	Init
	XREF	MyInit
	XREF	EndCode

;****** Exported Names ***********************************************

*------ Data ---------------------------------------------------------

	XDEF	subsysName
	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	VERSTRING

;****** Local Definitions ********************************************

initDDescrip:
				  ;STRUCTURE RT,0
	  DC.W    RTC_MATCHWORD   ; UWORD RT_MATCHWORD
	  DC.L    initDDescrip    ; APTR  RT_MATCHTAG
	  DC.L    EndCode         ; APTR  RT_ENDSKIP
	  DC.B    RTF_AUTOINIT    ; UBYTE RT_FLAGS
	  DC.B    VERSION         ; UBYTE RT_VERSION
	  DC.B    NT_LIBRARY      ; UBYTE RT_TYPE
	  DC.B    -120            ; BYTE  RT_PRI
	  DC.L    subsysName      ; APTR  RT_NAME
	  DC.L    VERSTRING       ; APTR  RT_IDSTRING
	  DC.L    Init	          ; APTR  RT_INIT
				  ; LABEL RT_SIZE



subsysName:	ICONNAME
VERSTRING:	VSTRING
VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION

		END
