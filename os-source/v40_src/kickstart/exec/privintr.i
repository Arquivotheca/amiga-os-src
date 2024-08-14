**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*	Copyright 1985, 1989 Commodore-Amiga, Inc.
*
**********************************************************************
*
*   Source Control:
*
*	$Id: privintr.i,v 39.0 91/10/15 08:28:32 mks Exp $
*
*	$Log:	privintr.i,v $
* Revision 39.0  91/10/15  08:28:32  mks
* V39 Exec initial checkin
* 
**********************************************************************

*---------------------------------------------------------------------
*
*   Private Interrupt Server Control (server chain data area)
*
*---------------------------------------------------------------------

;Longword align this sucker... these are interrupts and need the speed!!
 STRUCTURE  IL,LH_SIZE
    UWORD   IL_SIntReq	 ;Mask to clear interrupt request (INF_XXXX)
    LABEL   IL_SIZE

;    UWORD   IL_Obsolete0 ;IL_SIntEna	;unused. (Mask to "Stop" interrupt)
;    UWORD   IL_Obsolete1 ;IL_EIntEna	;unused. (Mask to "Enable" interrupt)
;    UWORD   IL_Obsolete2 ;IL_EIntReq	;unused  (0)

