*************************************************************************
*
* $Id: internal.i,v 38.1 91/06/24 19:01:53 mks Exp $
*
* $Log:	internal.i,v $
* Revision 38.1  91/06/24  19:01:53  mks
* Changed to V38 source tree - Trimmed Log
* 
*************************************************************************

*********************************************************************
*
* Library structures
*
*********************************************************************

    STRUCTURE	il,LIB_SIZE
	APTR	il_SysBase
	APTR	il_DOSBase
	APTR	il_WorkbenchBase
	APTR	il_SegList
	LABEL	il_Sizeof

; sysBaseOffset EQU il_SysBase
; dosBaseOffset EQU il_DOSBase

INFOLEVEL	EQU	50
