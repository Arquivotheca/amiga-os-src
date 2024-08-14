**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
*   Source Control:
*
*	$Id: privinfo.i,v 39.0 91/10/15 08:28:30 mks Exp $
*
*	$Log:	privinfo.i,v $
* Revision 39.0  91/10/15  08:28:30  mks
* V39 Exec initial checkin
* 
**********************************************************************


INFOMSG     MACRO   ; level,string
	IFGE	INFODEPTH-\1
	    LEA     INFO_\@(PC),A0
	    LEA     INFOL\@(PC),A5
	    BRA     BarePutStr
INFO_\@     DC.B    \2,13,10,0
	    DS.W    0
INFOL\@:
	ENDC

	    ENDM

